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
#include <SLES/OpenSLES.h>
#include <android/log.h>
#include "howie-private.h"
#include "EngineImpl.h"
#include "howie_jni.h"
#include "StreamImpl.h"



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
                                                      jint framesPerBuffer) {
  __android_log_print(ANDROID_LOG_VERBOSE, "HOWIE", __func__);
  howie::EngineImpl *pEngine = howie::EngineImpl::get();
  if (!pEngine) {
    pEngine = new howie::EngineImpl(
        sampleRate,
        bitsPerSample,
        bytesPerSample,
        sampleMask,
        floatingPoint == 0 ? false : true,
        channelCount,
        samplesPerFrame,
        framesPerBuffer);
  }
  pEngine->init();

  return reinterpret_cast<long>(pEngine);
}

JNIEXPORT void JNICALL
Java_com_example_android_howie_HowieEngine_destroy(JNIEnv *env,
                                                   jclass type,
                                                   jlong engine) {
  __android_log_print(ANDROID_LOG_VERBOSE, "HOWIE", __func__);
  howie::EngineImpl * pEngine = reinterpret_cast<howie::EngineImpl *>(engine);
  delete pEngine;
}

namespace howie {
  EngineImpl* EngineImpl::instance_ = nullptr;

  EngineImpl::EngineImpl(int sampleRate,
                         int bitsPerSample,
                         int bytesPerSample,
                         int sampleMask,
                         bool floatingPoint,
                         int channelCount,
                         int samplesPerFrame,
                         int framesPerBuffer) {
    __android_log_print(ANDROID_LOG_VERBOSE, "HOWIE", __func__);
    deviceCharacteristics.version = sizeof(deviceCharacteristics);
    deviceCharacteristics.sampleRate = sampleRate;
    deviceCharacteristics.bitsPerSample = bitsPerSample;
    deviceCharacteristics.bytesPerSample = bytesPerSample;
    deviceCharacteristics.sampleMask = sampleMask;
    deviceCharacteristics.floatingPoint = floatingPoint;
    deviceCharacteristics.samplesPerFrame = samplesPerFrame;
    deviceCharacteristics.channelCount = channelCount;
    deviceCharacteristics.framesPerPeriod = framesPerBuffer;
    instance_ = this;
  }

  EngineImpl::~EngineImpl() {
    __android_log_print(ANDROID_LOG_VERBOSE, "HOWIE", __func__);
    instance_ = NULL;
  }

  HowieError EngineImpl::init() {
    __android_log_print(ANDROID_LOG_VERBOSE, "HOWIE", __func__);
    SLresult result;

    // create EngineImpl
    HOWIE_CHECK(slCreateEngine(&engineObject_, 0, NULL, 0, NULL, NULL));

    // realize the EngineImpl
    HOWIE_CHECK((*engineObject_)->Realize(engineObject_, SL_BOOLEAN_FALSE));

    // get the EngineImpl interface, which is needed in order to create other objects
    HOWIE_CHECK((*engineObject_)->GetInterface(engineObject_, SL_IID_ENGINE,
                                         &engineItf_));

    // create output mix,
    HOWIE_CHECK((*engineItf_)->CreateOutputMix(engineItf_, &outputMixObject_, 0, NULL,
                                         NULL));

    // realize the output mix
    HOWIE_CHECK((*outputMixObject_)->Realize(outputMixObject_, SL_BOOLEAN_FALSE));

    return HOWIE_SUCCESS;
  }

  HowieError EngineImpl::createStream(
      const HowieStreamCreationParams &params,
      HowieStream **out_stream) {
    HowieError result = HOWIE_ERROR_UNKNOWN;

    __android_log_print(ANDROID_LOG_VERBOSE, "HOWIE", __func__);
    if (out_stream) {
      *out_stream = nullptr;
    }

    StreamImpl *stream = new StreamImpl(deviceCharacteristics, params);
    if (stream) {
      result = stream->init(engineItf_, outputMixObject_);
      HOWIE_CHECK(result);
    }

    if (out_stream && HOWIE_SUCCEEDED(result)) {
      *out_stream = stream;
    }
    return result;
  }

} // namespace howie