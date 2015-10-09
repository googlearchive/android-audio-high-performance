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
#ifndef HIGH_PERFORMANCE_JNI_H
#define HIGH_PERFORMANCE_JNI_H
#include <jni.h>

#ifdef __cplusplus
extern"C" {
#endif // __cplusplus

JNIEXPORT jlong JNICALL
Java_com_example_ilewis_hellolowlatencyio_MainActivity_initStream(
    JNIEnv *env,
    jobject instance);

JNIEXPORT void JNICALL
Java_com_example_ilewis_hellolowlatencyio_MainActivity_setParams(
    JNIEnv *env,
    jobject instance,
    jlong stream,
    jfloat frequency,
    jfloat resonance,
    jfloat gain);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus
#endif //HIGH_PERFORMANCE_JNI_H
