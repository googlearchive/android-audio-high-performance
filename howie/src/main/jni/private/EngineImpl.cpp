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
  HOWIE_TRACE_FN(HOWIE_TRACE_LEVEL_CALLS)
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
  HOWIE_TRACE_FN(HOWIE_TRACE_LEVEL_CALLS)
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
    HOWIE_TRACE_FN(HOWIE_TRACE_LEVEL_CALLS)
    deviceCharacteristics_.version = sizeof(deviceCharacteristics_);
    deviceCharacteristics_.sampleRate = sampleRate;
    deviceCharacteristics_.bitsPerSample = bitsPerSample;
    deviceCharacteristics_.bytesPerSample = bytesPerSample;
    deviceCharacteristics_.sampleMask = sampleMask;
    deviceCharacteristics_.floatingPoint = floatingPoint;
    deviceCharacteristics_.samplesPerFrame = samplesPerFrame;
    deviceCharacteristics_.channelCount = channelCount;
    deviceCharacteristics_.framesPerPeriod = framesPerBuffer;
    instance_ = this;
  }

  EngineImpl::~EngineImpl() {
    HOWIE_TRACE_FN(HOWIE_TRACE_LEVEL_CALLS)
    instance_ = NULL;
  }

  HowieError EngineImpl::init() {
    HOWIE_TRACE_FN(HOWIE_TRACE_LEVEL_CALLS)

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
    HOWIE_TRACE_FN(HOWIE_TRACE_LEVEL_CALLS)
    HowieError result = HOWIE_ERROR_UNKNOWN;

    if (out_stream) {
      *out_stream = nullptr;
    }

    StreamImpl *stream = new StreamImpl(deviceCharacteristics_, params);
    if (stream) {
      result = DoAsync([=]{stream->init(engineItf_, outputMixObject_, params);});
      HOWIE_CHECK(result);
    }

    if (out_stream && HOWIE_SUCCEEDED(result)) {
      *out_stream = stream;
    }
    return result;
  }

  const HowieDeviceCharacteristics * EngineImpl::getDeviceCharacteristics() const {
    HOWIE_TRACE_FN(HOWIE_TRACE_LEVEL_CALLS)
    return &deviceCharacteristics_;
  }

  HowieError EngineImpl::DoAsync(const Worker::work_item_t &fn) {
    HOWIE_TRACE_FN(HOWIE_TRACE_LEVEL_CALLS)
    HowieError result = HOWIE_ERROR_AGAIN;
    if (openSLIsSlow_.push_work(fn)) {
      result = HOWIE_SUCCESS;
    }
    return result;
  }
} // namespace howie
