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
#include "PlayAudioEngine.h"
#include "logging_macros.h"

extern "C" {

/**
 * Creates the audio engine
 *
 * @return a pointer to the audio engine. This should be passed to other methods
 */
JNIEXPORT jlong JNICALL
Java_com_google_sample_oboe_hellooboe_PlaybackEngine_native_1createEngine(JNIEnv *env,
                                                                          jclass) {

    // We use std::nothrow so `new` returns a nullptr if the engine creation fails
    PlayAudioEngine *engine = new(std::nothrow) PlayAudioEngine();
    return (jlong) engine;
}

JNIEXPORT void JNICALL
Java_com_google_sample_oboe_hellooboe_PlaybackEngine_native_1deleteEngine(JNIEnv *env,
                                                                          jclass,
                                                                          jlong engineHandle) {
    delete (PlayAudioEngine *) engineHandle;
}

JNIEXPORT void JNICALL
Java_com_google_sample_oboe_hellooboe_PlaybackEngine_native_1setToneOn(JNIEnv *env,
                                                                       jclass,
                                                                       jlong engineHandle,
                                                                       jboolean isToneOn) {
    PlayAudioEngine *engine = (PlayAudioEngine *) engineHandle;
    if (engine == nullptr) {
        LOGE("Engine handle is invalid, call createHandle() to create a new one");
        return;
    }
    engine->setToneOn(isToneOn);
}

JNIEXPORT void JNICALL
Java_com_google_sample_oboe_hellooboe_PlaybackEngine_native_1setAudioApi(JNIEnv *env, jclass type,
                                                                         jlong engineHandle,
                                                                         jint audioApi) {
    PlayAudioEngine *engine = (PlayAudioEngine *) engineHandle;
    if (engine == nullptr) {
        LOGE("Engine handle is invalid, call createHandle() to create a new one");
        return;
    }

    oboe::AudioApi api = static_cast<oboe::AudioApi>(audioApi);
    engine->setAudioApi(api);
}

JNIEXPORT void JNICALL
Java_com_google_sample_oboe_hellooboe_PlaybackEngine_native_1setAudioDeviceId(JNIEnv *env,
                                                                              jclass,
                                                                              jlong engineHandle,
                                                                              jint deviceId) {

    PlayAudioEngine *engine = (PlayAudioEngine *) engineHandle;
    if (engine == nullptr) {
        LOGE("Engine handle is invalid, call createHandle() to create a new one");
        return;
    }
    engine->setDeviceId(deviceId);
}

JNIEXPORT void JNICALL
Java_com_google_sample_oboe_hellooboe_PlaybackEngine_native_1setBufferSizeInBursts(
        JNIEnv *env,
        jclass,
        jlong engineHandle,
        jint bufferSizeInBursts) {

    PlayAudioEngine *engine = (PlayAudioEngine *) engineHandle;
    if (engine == nullptr) {
        LOGE("Engine handle is invalid, call createHandle() to create a new one");
        return;
    }
    engine->setBufferSizeInBursts(bufferSizeInBursts);
}


JNIEXPORT jdouble JNICALL
Java_com_google_sample_oboe_hellooboe_PlaybackEngine_native_1getCurrentOutputLatencyMillis(
        JNIEnv *env,
        jclass,
        jlong engineHandle) {

    PlayAudioEngine *engine = (PlayAudioEngine *) engineHandle;
    if (engine == nullptr) {
        LOGE("Engine is null, you must call createEngine before calling this method");
        return -1;
    }
    return (jdouble) engine->getCurrentOutputLatencyMillis();
}

JNIEXPORT jboolean JNICALL
Java_com_google_sample_oboe_hellooboe_PlaybackEngine_native_1isLatencyDetectionSupported(
        JNIEnv *env, jclass type, jlong engineHandle) {

    PlayAudioEngine *engine = (PlayAudioEngine *) engineHandle;
    if (engine == nullptr) {
        LOGE("Engine is null, you must call createEngine before calling this method");
        return (jboolean) false;
    }
    return (jboolean) engine->isLatencyDetectionSupported();
}

}