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
 *
 */
#include <jni.h>
#include <android/log.h>

const char * MODULE_NAME="AUDIO-ECHO";

extern "C" {
JNIEXPORT  jlong JNICALL
   Java_com_google_sample_audio_1echo_EchoMainActivity_startEcho(JNIEnv *env, jobject instance);
JNIEXPORT void JNICALL
        Java_com_google_sample_audio_1echo_EchoMainActivity_stopEcho(JNIEnv *env, jobject instance,
                                                                     jlong streamId);
}

JNIEXPORT jlong JNICALL
Java_com_google_sample_audio_1echo_EchoMainActivity_startEcho(JNIEnv *env, jobject instance) {

    __android_log_print(ANDROID_LOG_INFO, MODULE_NAME, __FUNCTION__);
    return reinterpret_cast<jlong>((void*)0);
}

JNIEXPORT void JNICALL
Java_com_google_sample_audio_1echo_EchoMainActivity_stopEcho(JNIEnv *env, jobject instance,
                                                             jlong streamId) {
    __android_log_print(ANDROID_LOG_INFO, MODULE_NAME, "%s: id = %d", __FUNCTION__, streamId);
}