//
// Created by ilewis on 9/29/15.
//

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
