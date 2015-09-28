//
// Created by ilewis on 9/25/15.
//

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
