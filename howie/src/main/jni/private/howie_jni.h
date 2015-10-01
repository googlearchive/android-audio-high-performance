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
#ifndef HOWIE_JNI_H
#define HOWIE_JNI_H

#include <jni.h>


#ifdef __cplusplus
extern"C" {
#endif // __cplusplus

JNIEXPORT jlong JNICALL
Java_com_example_android_howie_HowieEngine_create(JNIEnv *env,
                                                  jclass type,
                                                  jint sampleRate,
                                                  jint bitsPerSample,
                                                  jint bytesPerSample,
                                                  jint sampleMask,
                                                  jboolean floatingPoint,
                                                  jint channelCount,
                                                  jint samplesPerFrame,
                                                  jint framesPerBuffer);



JNIEXPORT void JNICALL
Java_com_example_android_howie_HowieEngine_destroy(JNIEnv *env,
                                                   jclass type,
                                                   jlong engine);


#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif //HOWIE_JNI_H
