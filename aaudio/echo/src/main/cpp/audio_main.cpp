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

struct AAudioEchoEngine {
  uint32_t sampleRate_;
  uint16_t sampleChannels_;
  uint16_t bitsPerSample_;
  aaudio_audio_format_t sampleFormat_;

  AAudioStream *playStream_;
  AAudioStream *recordingStream_;

  std::atomic_bool playAudio_;

  int32_t underRunCount_;
  int32_t bufSizeInFrames_;
  int32_t framesPerBurst_;
  int32_t defaultBufSizeInFrames_;

  std::mutex mutex_;
  AAudioEchoEngine()
      : sampleRate_(0),
        sampleChannels_(0),
        bitsPerSample_(0),
        sampleFormat_(AAUDIO_FORMAT_UNSPECIFIED),
        playStream_(nullptr),
        recordingStream_(nullptr),
        playAudio_(false),
        underRunCount_(0),
        bufSizeInFrames_(0),
        framesPerBurst_(0),
        defaultBufSizeInFrames_(0) {}
  ~AAudioEchoEngine() {}
};

static AAudioEchoEngine engine;

extern "C" {
JNIEXPORT jboolean JNICALL
Java_com_google_sample_aaudio_echo_MainActivity_createEngine(JNIEnv *env,
                                                             jclass);
JNIEXPORT void JNICALL
Java_com_google_sample_aaudio_echo_MainActivity_deleteEngine(JNIEnv *env,
                                                             jclass type);
JNIEXPORT jboolean JNICALL
Java_com_google_sample_aaudio_echo_MainActivity_start(JNIEnv *env, jclass type);
JNIEXPORT jboolean JNICALL
Java_com_google_sample_aaudio_echo_MainActivity_stop(JNIEnv *env, jclass type);
}

aaudio_data_callback_result_t dataCallback(AAudioStream *stream, void *userData,
                                           void *audioData, int32_t numFrames) {
  assert(userData && audioData);
  AAudioEchoEngine *eng = reinterpret_cast<AAudioEchoEngine *>(userData);
  assert(stream == eng->playStream_);

  // Tuning the buffer size for low latency...
  int32_t underRun = AAudioStream_getXRunCount(eng->playStream_);
  if (underRun > eng->underRunCount_) {
    /* Underrun happened since last callback:
     * try to increase the buffer size.
     */
    eng->underRunCount_ = underRun;

    aaudio_result_t actSize = AAudioStream_setBufferSizeInFrames(
        stream, eng->bufSizeInFrames_ + eng->framesPerBurst_);
    if (actSize > 0) {
      eng->bufSizeInFrames_ = actSize;
    } else {
      LOGE("***** Output stream buffer tuning error: %s",
           AAudio_convertResultToText(actSize));
    }
  }

  int32_t samplesPerFrame = eng->sampleChannels_;
  // frameCount could be
  //    < 0 : error code
  //    >= 0 : actual value read from stream
  if (eng->playAudio_) {
    aaudio_result_t frameCount;
    frameCount = AAudioStream_read(eng->recordingStream_, audioData, numFrames,
                                   static_cast<int64_t>(0));
    if (frameCount < 0) {
      LOGE("****AAudioStream_read() returns %s",
           AAudio_convertResultToText(frameCount));
      assert(false);
      frameCount = 0;  // continue to play silent audio
    }
    /*
     * Not enough audio data from input stream, fill the rest of buffer with
     * 0 ( silence ) and continue to loop
     */
    numFrames -= frameCount;
    if (numFrames) {
      memset(static_cast<int16_t *>(audioData) + frameCount * samplesPerFrame,
             0, sizeof(int16_t) * numFrames * samplesPerFrame);
    }
  } else {
    memset(audioData, 0, sizeof(int16_t) * numFrames * samplesPerFrame);
  }

  return AAUDIO_CALLBACK_RESULT_CONTINUE;
}

/*
 * Initialize AAudioEchoEngine object, create streams etc:
 *    output stream state:  STARTED or STARTING
 *    input  stream state:  OPENED
 * silent audio is playing so no sound is generated
 */
JNIEXPORT jboolean JNICALL
Java_com_google_sample_aaudio_echo_MainActivity_createEngine(JNIEnv *env,
                                                             jclass type) {
  engine.sampleChannels_ = AUDIO_SAMPLE_CHANNELS;
  engine.sampleFormat_ = AAUDIO_FORMAT_PCM_I16;
  engine.bitsPerSample_ = SampleFormatToBpp(engine.sampleFormat_);

  // create an output stream
  StreamBuilder builder;
  engine.playStream_ = builder.CreateStream(
      engine.sampleFormat_, engine.sampleChannels_, AAUDIO_SHARING_MODE_SHARED,
      AAUDIO_DIRECTION_OUTPUT, INVALID_AUDIO_PARAM, dataCallback, &engine);
  // this sample only supports PCM_I16 format
  if (!engine.playStream_ ||
      engine.sampleFormat_ != AAudioStream_getFormat(engine.playStream_)) {
    assert(false);
    return JNI_FALSE;
  }
  engine.sampleRate_ = AAudioStream_getSampleRate(engine.playStream_);
  engine.framesPerBurst_ = AAudioStream_getFramesPerBurst(engine.playStream_);
  engine.defaultBufSizeInFrames_ =
      AAudioStream_getBufferSizeInFrames(engine.playStream_);
  AAudioStream_setBufferSizeInFrames(engine.playStream_,
                                     engine.framesPerBurst_);
  engine.bufSizeInFrames_ = engine.framesPerBurst_;

  // create an input Stream that matches the output stream
  engine.recordingStream_ = builder.CreateStream(
      engine.sampleFormat_, engine.sampleChannels_, AAUDIO_SHARING_MODE_SHARED,
      AAUDIO_DIRECTION_INPUT, engine.sampleRate_);
  if (!engine.recordingStream_ ||
      engine.sampleFormat_ != AAudioStream_getFormat(engine.recordingStream_)) {
    assert(false);
    return JNI_FALSE;
  }

  aaudio_result_t result = AAudioStream_requestStart(engine.playStream_);
  if (result != AAUDIO_OK) {
    assert(false);
    return JNI_FALSE;
  }

  engine.underRunCount_ = AAudioStream_getXRunCount(engine.playStream_);

  return JNI_TRUE;
}

/*
 * start():
 *   start to play audio from the input stream.
 */
JNIEXPORT jboolean JNICALL
Java_com_google_sample_aaudio_echo_MainActivity_start(JNIEnv *env,
                                                      jclass type) {
  aaudio_result_t result = AAUDIO_OK;
  // acquire lock to synchronize start() and stop()
  Lock lock(&engine.mutex_);
  if (engine.playStream_ && engine.recordingStream_) {
    if (!engine.playAudio_) {
      result = AAudioStream_requestStart(engine.recordingStream_);
      if (result == AAUDIO_OK) {
        engine.playAudio_ = true;
        return JNI_TRUE;
      }
    }
  }
  if (result != AAUDIO_OK) {
    LOGE("*****start() function failed with error code %s",
         AAudio_convertResultToText(result));
    assert(false);
  }
  return JNI_FALSE;
}

/*
 * stop():
 *   stop playing audio from the input stream.
 */
JNIEXPORT jboolean JNICALL
Java_com_google_sample_aaudio_echo_MainActivity_stop(JNIEnv *env, jclass type) {
  aaudio_result_t result = AAUDIO_OK;
  Lock lock(&engine.mutex_);
  if (engine.playStream_ && engine.recordingStream_) {
    if (engine.playAudio_) {
      engine.playAudio_ = false;
      result = AAudioStream_requestStop(engine.recordingStream_);
      if (result == AAUDIO_OK) {
        return JNI_TRUE;
      }
    }
  }

  if (result != AAUDIO_OK) {
    LOGE("*****requestStop() function failed with error code %s",
         AAudio_convertResultToText(result));
    assert(false);
  }

  // something is wrong, clean up errors
  if (engine.playStream_) {
    AAudioStream_requestStop(engine.playStream_);
    AAudioStream_close(engine.playStream_);
    engine.playStream_ = nullptr;
  }

  if (engine.recordingStream_) {
    AAudioStream_requestStop(engine.recordingStream_);
    AAudioStream_close(engine.recordingStream_);
    engine.recordingStream_ = nullptr;
  }
  return JNI_FALSE;
}

/*
 * delete(): close streams
 */
JNIEXPORT void JNICALL
Java_com_google_sample_aaudio_echo_MainActivity_deleteEngine(JNIEnv *env,
                                                             jclass type) {
  // streams could be closed from any state, so no need to stop before closing
  if (engine.playStream_) {
    AAudioStream_close(engine.playStream_);
    engine.playStream_ = nullptr;
  }

  if (engine.recordingStream_) {
    AAudioStream_close(engine.recordingStream_);
    engine.recordingStream_ = nullptr;
  }
}
