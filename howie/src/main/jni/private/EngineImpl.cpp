//
// Created by ilewis on 9/25/15.
//

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
    instance_ = NULL;
  }

  HowieError EngineImpl::init() {
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

  HowieError EngineImpl::createStream(HowieDirection direction,
                           HowieDeviceChangedCallback deviceChangedCallback,
                           HowieProcessCallback processCallback,
                           HowieStream **out_stream) {
    __android_log_print(ANDROID_LOG_VERBOSE, "HOWIE", __func__);
    HowieError result = HOWIE_ERROR_UNKNOWN;
    StreamImpl *stream = new StreamImpl(deviceCharacteristics,
                                 deviceChangedCallback,
                                 processCallback);
    if (stream) {
      result = stream->init(engineItf_, outputMixObject_);
    }

    if (out_stream) {
      *out_stream = nullptr;
      if (HOWIE_SUCCEEDED(result)) {
        *out_stream = stream;
      }
    }
    return result;
  }

} // namespace howie