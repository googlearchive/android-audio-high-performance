/*
 * Copyright 2017 The Android Open Source Project
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
#include <thread>
#include <cassert>
#include <jni.h>

#include "audio_common.h"
#include "SineGenerator.h"
#include "stream_builder.h"

/*
 * This Sample's Engine Structure
 */
struct HelloAAudioEngine {
  uint32_t sampleRate_;
  uint16_t sampleChannels_;
  uint16_t bitsPerSample_;
  aaudio_audio_format_t sampleFormat_;

  SineGenerator *sineOscLeft;
  SineGenerator *sineOscRight;

  AAudioStream *playStream_;
  bool playAudio_;

  int32_t underRunCount_;
  int32_t bufSizeInFrames_;
  int32_t framesPerBurst_;
  int32_t defaultBufSizeInFrames_;
};
static HelloAAudioEngine engine;

/*
 * Functions exposed to Java code...
 */
extern "C" {
JNIEXPORT jboolean JNICALL
Java_com_google_sample_aaudio_play_MainActivity_createEngine(JNIEnv *env,
                                                             jclass);
JNIEXPORT void JNICALL
Java_com_google_sample_aaudio_play_MainActivity_deleteEngine(JNIEnv *env,
                                                             jclass type);
JNIEXPORT jboolean JNICALL
Java_com_google_sample_aaudio_play_MainActivity_start(JNIEnv *env, jclass type);
JNIEXPORT jboolean JNICALL
Java_com_google_sample_aaudio_play_MainActivity_stop(JNIEnv *env, jclass type);
}

aaudio_data_callback_result_t dataCallback(AAudioStream *stream, void *userData,
                                           void *audioData, int32_t numFrames) {
  assert(userData && audioData);
  HelloAAudioEngine *eng = reinterpret_cast<HelloAAudioEngine *>(userData);
  assert(stream == eng->playStream_);

  int32_t underRun = AAudioStream_getXRunCount(eng->playStream_);
  if (underRun > eng->underRunCount_) {
    eng->underRunCount_ = underRun;

    aaudio_result_t actSize = AAudioStream_setBufferSizeInFrames(
        stream, eng->bufSizeInFrames_ + eng->framesPerBurst_);
    if (actSize > 0) {
      eng->bufSizeInFrames_ = actSize;
    } else {
      LOGE("*****: Error from dataCallback  -- %s",
           AAudio_convertResultToText(actSize));
    }
  }

  int32_t samplesPerFrame = eng->sampleChannels_;
  if (eng->playAudio_) {
    eng->sineOscRight->render(static_cast<int16_t *>(audioData),
                              samplesPerFrame, numFrames);
    if (samplesPerFrame == 2) {
      eng->sineOscLeft->render(static_cast<int16_t *>(audioData) + 1,
                               samplesPerFrame, numFrames);
    }
  } else {
    memset(static_cast<uint8_t *>(audioData), 0,
           sizeof(int16_t) * samplesPerFrame * numFrames);
  }

  return AAUDIO_CALLBACK_RESULT_CONTINUE;
}

JNIEXPORT jboolean JNICALL
Java_com_google_sample_aaudio_play_MainActivity_createEngine(JNIEnv *env,
                                                             jclass type) {
  memset(&engine, 0, sizeof(engine));

  engine.sampleChannels_ = AUDIO_SAMPLE_CHANNELS;
  engine.sampleFormat_ = AAUDIO_FORMAT_PCM_I16;
  engine.bitsPerSample_ = SampleFormatToBpp(engine.sampleFormat_);

  // Create an Output Stream
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

  PrintAudioStreamInfo(engine.playStream_);
  engine.sampleRate_ = AAudioStream_getSampleRate(engine.playStream_);
  engine.framesPerBurst_ = AAudioStream_getFramesPerBurst(engine.playStream_);
  engine.defaultBufSizeInFrames_ =
      AAudioStream_getBufferSizeInFrames(engine.playStream_);
  AAudioStream_setBufferSizeInFrames(engine.playStream_,
                                     engine.framesPerBurst_);
  engine.bufSizeInFrames_ = engine.framesPerBurst_;

  // prepare for data generator
  engine.sineOscLeft = new SineGenerator;
  engine.sineOscLeft->setup(440.0, engine.sampleRate_, 0.25);
  engine.sineOscRight = new SineGenerator;
  engine.sineOscRight->setup(660.0, engine.sampleRate_, 0.25);

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
 *   start to render sine wave audio.
 */
JNIEXPORT jboolean JNICALL
Java_com_google_sample_aaudio_play_MainActivity_start(JNIEnv *env,
                                                      jclass type) {
  if (!engine.playStream_) return JNI_FALSE;

  engine.playAudio_ = true;
  return JNI_TRUE;
}

/*
 * stop():
 *   stop rendering sine wave audio
 */
JNIEXPORT jboolean JNICALL
Java_com_google_sample_aaudio_play_MainActivity_stop(JNIEnv *env, jclass type) {
  if (!engine.playStream_) return JNI_TRUE;

  engine.playAudio_ = false;
  return JNI_TRUE;
}

/*
 * delete(): clean-up
 */
JNIEXPORT void JNICALL
Java_com_google_sample_aaudio_play_MainActivity_deleteEngine(JNIEnv *env,
                                                             jclass type) {
  if (!engine.playStream_) {
    return;
  }

  AAudioStream_requestStop(engine.playStream_);
  AAudioStream_close(engine.playStream_);
  engine.playStream_ = nullptr;

  delete (engine.sineOscLeft);
  delete (engine.sineOscRight);

  engine.sineOscLeft = nullptr;
  engine.sineOscRight = nullptr;
}
