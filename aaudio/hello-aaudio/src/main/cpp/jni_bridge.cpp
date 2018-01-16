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


#include <jni.h>
#include <logging_macros.h>
#include "PlayAudioEngine.h"

static PlayAudioEngine *engine = nullptr;

extern "C" {

JNIEXPORT bool JNICALL
Java_com_google_sample_aaudio_play_PlaybackEngine_create(JNIEnv *env,
                                                               jclass) {
  if (engine == nullptr) {
    engine = new PlayAudioEngine();
  }

  return (engine != nullptr);
}

JNIEXPORT void JNICALL
Java_com_google_sample_aaudio_play_PlaybackEngine_delete(JNIEnv *env,
                                                               jclass) {
  delete engine;
  engine = nullptr;
}

JNIEXPORT void JNICALL
Java_com_google_sample_aaudio_play_PlaybackEngine_setToneOn(JNIEnv *env,
                                                               jclass, jboolean isToneOn) {
  if (engine == nullptr) {
    LOGE("Engine is null, you must call createEngine before calling this method");
    return;
  }

  engine->setToneOn(isToneOn);
}

JNIEXPORT void JNICALL
Java_com_google_sample_aaudio_play_PlaybackEngine_setAudioDeviceId(JNIEnv *env,
                                                            jclass, jint deviceId) {
  if (engine == nullptr) {
    LOGE("Engine is null, you must call createEngine before calling this method");
    return;
  }

  engine->setDeviceId(deviceId);
}

JNIEXPORT void JNICALL
Java_com_google_sample_aaudio_play_PlaybackEngine_setBufferSizeInBursts(
    JNIEnv *env, jclass, jint bufferSizeInBursts) {
  if (engine == nullptr) {
    LOGE("Engine is null, you must call createEngine before calling this method");
    return;
  }

  engine->setBufferSizeInBursts(bufferSizeInBursts);
}


JNIEXPORT jdouble JNICALL
Java_com_google_sample_aaudio_play_PlaybackEngine_getCurrentOutputLatencyMillis(JNIEnv *env,
                                                                            jclass) {
  if (engine == nullptr) {
    LOGE("Engine is null, you must call createEngine before calling this method");
    return -1;
  }
  return (jdouble)engine->getCurrentOutputLatencyMillis();
}


}
