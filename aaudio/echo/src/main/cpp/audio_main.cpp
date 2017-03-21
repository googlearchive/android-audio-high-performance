/*
 * Copyright 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <cassert>
#include <cstring>
#include <jni.h>
#include <sys/types.h>
#include <thread>

#include "audio_common.h"
#include "stream_builder.h"

struct AAudioEcho {
    uint32_t     sampleRate_;
    uint32_t     framesPerBuf_;
    uint16_t     sampleChannels_;
    uint16_t     bitsPerSample_;
    aaudio_audio_format_t sampleFormat_;

    AAudioStream *playStream_;
    AAudioStream *recordingStream_;

    bool   requestStop_;
    bool   playAudio_;
};
static AAudioEcho engine;


extern "C" {
  JNIEXPORT jboolean JNICALL
  Java_com_google_sample_aaudio_echo_MainActivity_createEngine(
      JNIEnv *env, jclass, jint, jint);
  JNIEXPORT void JNICALL
  Java_com_google_sample_aaudio_echo_MainActivity_deleteEngine(
      JNIEnv *env, jclass type);
  JNIEXPORT jboolean JNICALL
  Java_com_google_sample_aaudio_echo_MainActivity_start(
      JNIEnv *env, jclass type);
  JNIEXPORT jboolean JNICALL
  Java_com_google_sample_aaudio_echo_MainActivity_stop(
      JNIEnv *env, jclass type);
}

bool TunePlayerForLowLatency(AAudioStream* stream);

/*
 * AudioThreadProc()
 *   Rendering audio frames continuously; if user asks to echo audio, render
 *   read audio; if user asks to stop, renders silent audio (all 0s)
 *   in all cases, audio recording and playback are still functioning through
 *   apps life time.
 */
void AudioThreadProc(void* ctx) {
  AAudioEcho* eng = reinterpret_cast<AAudioEcho*>(ctx);

  // Tune up output stream ( input stream does not support tuning )
  bool status = TunePlayerForLowLatency(engine.playStream_);
  if (!status) {
    // if tune up is failed, audio could still play
    LOGW("Failed to tune up the audio buffer size,"
             "low latency audio may not be guaranteed");
  }
  // double check the tuning result: not necessary
  PrintAudioStreamInfo(engine.playStream_);

  // prepare for data generator
  int32_t framesPerBurst = AAudioStream_getFramesPerBurst(eng->playStream_);
  int32_t samplesPerFrame = AAudioStream_getSamplesPerFrame(eng->playStream_);

  // Writing it out as frames per burst, total 5 seconds
  int16_t *buf = new int16_t[framesPerBurst * samplesPerFrame];
  assert(buf);

  // calculate the timeout value:
  //   wait for 2 times application read buf size, hard coded to
  //        framePerBuffer
  uint64_t timeoutInNano = (framesPerBurst * 1000000000 * 16)/ eng->sampleRate_;
  timeoutInNano /= 8;

  // Count for each read/write. aaudio_result_t is typedef(ed) to int32_t
  // negative values means error code, defined in aaudio_result_t
  int32_t frameCount;
  while (!eng->requestStop_) {
    if (eng->playAudio_) {
      frameCount = AAudioStream_read(eng->recordingStream_,
                                 buf,
                                 framesPerBurst,
                                 timeoutInNano);
      assert(frameCount == framesPerBurst);
    } else {
      memset(buf, 0, sizeof(int16_t) * framesPerBurst * samplesPerFrame);
      frameCount = samplesPerFrame;
    }
    frameCount = AAudioStream_write(eng->playStream_,
                                buf,
                                frameCount,
                                timeoutInNano);
    assert(frameCount > 0);
  }

  delete [] buf;
  eng->requestStop_ = false;

  AAudioStream_requestStop(eng->playStream_);
  AAudioStream_requestStop(eng->recordingStream_);

  AAudioStream_close(eng->playStream_);
  AAudioStream_close(eng->recordingStream_);
  eng->playStream_ = nullptr;
  eng->playStream_ = nullptr;

  LOGV("====Echo is completed");
}

/*
 * Create sample engine and put application into started state:
 * audio is already rendering -- rendering silent audio.
 */
JNIEXPORT jboolean JNICALL
Java_com_google_sample_aaudio_echo_MainActivity_createEngine(
    JNIEnv *env, jclass type, jint sampleRate, jint framesPerBuf) {

  memset(&engine, 0, sizeof(engine));

  // Initialize AAudio wrapper
  if(!InitAAudio()) {
    LOGE("AAudio is not supported on your platform, cannot proceed");
    return JNI_FALSE;
  }

  engine.sampleRate_   = sampleRate;
  engine.framesPerBuf_ = static_cast<uint32_t>(framesPerBuf);
  engine.sampleChannels_   = AUDIO_SAMPLE_CHANNELS;
  engine.sampleFormat_ = AAUDIO_FORMAT_PCM_I16;
  engine.bitsPerSample_    = SampleFormatToBpp(engine.sampleFormat_);

  StreamBuilder builder(engine.sampleRate_,
                        engine.sampleChannels_,
                        engine.sampleFormat_,
                        AAUDIO_SHARING_MODE_SHARED,
                        AAUDIO_DIRECTION_OUTPUT);

  engine.playStream_ = builder.Stream();
  assert(engine.playStream_);

  PrintAudioStreamInfo(engine.playStream_);

  StreamBuilder recBuilder(engine.sampleRate_,
                        engine.sampleChannels_,
                        engine.sampleFormat_,
                        AAUDIO_SHARING_MODE_SHARED,
                        AAUDIO_DIRECTION_INPUT);

  engine.recordingStream_ = recBuilder.Stream();
  assert(engine.recordingStream_);
  PrintAudioStreamInfo(engine.recordingStream_);

  aaudio_stream_state_t result = AAudioStream_requestStart(engine.playStream_);
  if (result != AAUDIO_OK) {
    assert(result == AAUDIO_OK);
    return JNI_FALSE;
  }

  result = AAudioStream_requestStart(engine.recordingStream_);
  if (result != AAUDIO_OK) {
    assert(result == AAUDIO_OK);
    AAudioStream_requestStop(engine.playStream_);
    return JNI_FALSE;
  }

  std::thread t(AudioThreadProc, &engine);
  t.detach();
  return JNI_TRUE;
}

/*
 * start():
 *   start to render sine wave audio.
 */
JNIEXPORT jboolean JNICALL
Java_com_google_sample_aaudio_echo_MainActivity_start(
    JNIEnv *env, jclass type) {
  if (engine.playStream_ && engine.recordingStream_) {
    engine.playAudio_ = true;
    return JNI_TRUE;
  }
  return JNI_FALSE;
}

/*
 * stop():
 *   stop rendering sine wave audio ( resume rendering silent audio )
 */
JNIEXPORT jboolean JNICALL
Java_com_google_sample_aaudio_echo_MainActivity_stop(
    JNIEnv *env, jclass type) {
  if (engine.playStream_ && engine.recordingStream_) {
    engine.playAudio_ = false;
    return JNI_TRUE;
  }

  // something is wrong, cleaning up errors
  if (engine.playStream_) {
    AAudioStream_requestStop(engine.playStream_);
    AAudioStream_close(engine.playStream_);
    engine.playStream_ = nullptr;
    return JNI_FALSE;
  }

  AAudioStream_requestStop(engine.recordingStream_);
  AAudioStream_close(engine.recordingStream_);
  engine.recordingStream_ = nullptr;
  return JNI_FALSE;
}

/*
 * delete()
 *   clean-up sample: application is going away. Simply setup stop request
 *   flag and rendering thread will see it and perform clean-up
 */
JNIEXPORT void JNICALL
Java_com_google_sample_aaudio_echo_MainActivity_deleteEngine(
    JNIEnv *env, jclass type) {
  if (engine.playStream_ && engine.recordingStream_) {
    engine.requestStop_ = true;
    return;
  }

  // something is wrong, cleaning up errors
  if (engine.playStream_) {
    AAudioStream_requestStop(engine.playStream_);
    AAudioStream_close(engine.playStream_);
    engine.playStream_ = nullptr;
    return;
  }

  AAudioStream_requestStop(engine.recordingStream_);
  AAudioStream_close(engine.recordingStream_);
  engine.recordingStream_ = nullptr;
}

/*
 * TunePlayerForLowLatency()
 *   start from the framesPerBurst, find out the smallest size that has no
 *   underRan for buffer between Application and AAudio
 *  If tune-up failed, we still let it continue by restoring the value
 *  upon entering the function; the failure of the tuning is notified to
 *  caller with false return value.
 * Return:
 *   true:  tune-up is completed, AAudio is at its best
 *   false: tune-up is not complete, AAudio is at its default condition
 */
bool TunePlayerForLowLatency(AAudioStream* stream) {
  aaudio_stream_state_t state = AAudioStream_getState(stream);
  if (state == AAUDIO_STREAM_STATE_STARTING) {
    aaudio_result_t result;
    aaudio_stream_state_t nextState = AAUDIO_STREAM_STATE_UNINITIALIZED;
    do {
      result = AAudioStream_waitForStateChange(stream,
                                               state,
                                               &nextState,
                                               100 * 1000000);
    } while ((result == AAUDIO_OK || result == AAUDIO_ERROR_TIMEOUT)
             && nextState == AAUDIO_STREAM_STATE_UNINITIALIZED);
    state = AAudioStream_getState(stream);
  }

  if (state != AAUDIO_STREAM_STATE_STARTED) {
    LOGE("stream(%p) is in state %s, NOT in STARTED state, tuning is skipped",
         stream, AAudio_convertStreamStateToText(state));
    return false;
  }

  int32_t framesPerBurst = AAudioStream_getFramesPerBurst(stream);
  int32_t orgSize = AAudioStream_getBufferSizeInFrames(stream);

  int32_t bufSize = framesPerBurst;
  int32_t bufCap  = AAudioStream_getBufferCapacityInFrames(stream);

  uint8_t *buf = new uint8_t [bufCap * engine.bitsPerSample_ / 8];
  assert(buf);
  memset(buf, 0, bufCap * engine.bitsPerSample_ / 8);

  uint64_t timeoutInNano = (bufCap * 1000000000 * 16) /
                            engine.sampleRate_;
  timeoutInNano /= 8;

  int32_t prevXRun = AAudioStream_getXRunCount(stream);
  int32_t prevBufSize = 0;
  bool trainingError = false;
  while (bufSize <= bufCap) {
    aaudio_result_t  result = AAudioStream_setBufferSizeInFrames(stream, bufSize);
    if(result <= AAUDIO_OK) {
      trainingError = true;
      break;
    }

    // check whether we are really setting to our value
    // AAudio might already reached its optimized state
    // so we set-get-compare, then act accordingly
    bufSize = AAudioStream_getBufferSizeInFrames(stream);
    if (bufSize == prevBufSize) {
      // AAudio refuses to go up, tuning is complete
      break;
    }
    // remember the current buf size so we could continue for next round tuning up
    prevBufSize = bufSize;
    result = AAudioStream_write(stream, buf, bufCap, timeoutInNano);

    if (result < 0 ) {
      assert(result >= 0);
      trainingError = true;
      break;
    }
    int32_t curXRun = AAudioStream_getXRunCount(stream);
    if (curXRun <= prevXRun) {
      // no more errors, we are done
      break;
    }
    prevXRun = curXRun;
    bufSize += framesPerBurst;
  }

  delete [] buf;
  if (trainingError) {
    // we are playing conservative here: if anything wrong, we restore to default
    // size WHEN engine was created
    AAudioStream_setBufferSizeInFrames(stream, orgSize);
    return false;
  }
  return true;
}

