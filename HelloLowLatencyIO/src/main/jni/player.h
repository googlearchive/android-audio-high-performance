//
// Created by ilewis on 9/29/15.
//

#ifndef HIGH_PERFORMANCE_JNI_H
#define HIGH_PERFORMANCE_JNI_H
#include <jni.h>

#ifdef __cplusplus
extern"C" {
#endif // __cplusplus

JNIEXPORT void JNICALL
Java_com_example_ilewis_hellolowlatencyio_MainActivity_init(
    JNIEnv *env,
    jobject instance);


#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus
#endif //HIGH_PERFORMANCE_JNI_H
