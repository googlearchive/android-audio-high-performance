//
// Created by ilewis on 9/25/15.
//

#ifndef HELLOLOWLATENCYOUTPUT_STREAMIMPL_H
#define HELLOLOWLATENCYOUTPUT_STREAMIMPL_H


#include <SLES/OpenSLES_Android.h>
#include <bits/unique_ptr.h>
#include <android/log.h>
#include "../howie.h"
#include "unique_buffer.h"
#include "ParameterPipe.h"

namespace howie {

  class StreamImpl : public HowieStream {
  public:

    StreamImpl(
        const HowieDeviceCharacteristics &deviceCharacteristics,
        const HowieStreamCreationParams &params)
        : deviceCharacteristics(deviceCharacteristics),
          deviceChangedCallback_(params.deviceChangedCallback),
          processCallback_(params.processCallback),
          cleanupCallback_(params.cleanupCallback),
          state_(params.sizeofStateBlock),
          params_(params.sizeofParameterBlock) {
      __android_log_print(ANDROID_LOG_DEBUG, "HOWIE", "%s %d", __func__, __LINE__);
      version = sizeof(*this);
    }

    HowieError init(SLEngineItf engineItf, SLObjectItf outputMixObject);

    bool PushParameterBlock(const void* data, size_t size);


  private:

    HowieDeviceCharacteristics deviceCharacteristics;
    HowieDirection direction_;

    unique_buffer input_;
    unique_buffer output_;
    unique_buffer state_;
    ParameterPipe params_;


    SLObjectItf bqPlayerObject_ = NULL;
    SLPlayItf bqPlayerItf_ = NULL;
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue_ = NULL;

    static void bqPlayerCallback(SLAndroidSimpleBufferQueueItf, void*);
    static HowieError lastErr_;

    HowieDeviceChangedCallback deviceChangedCallback_;
    HowieProcessCallback processCallback_;
    HowieCleanupCallback cleanupCallback_;

    HowieError process(SLAndroidSimpleBufferQueueItf bq);

  };

} // namespace howie


#endif //HELLOLOWLATENCYOUTPUT_STREAMIMPL_H
