//
// Created by ilewis on 9/25/15.
//

#ifndef HELLOLOWLATENCYOUTPUT_STREAMIMPL_H
#define HELLOLOWLATENCYOUTPUT_STREAMIMPL_H


#include <SLES/OpenSLES_Android.h>
#include <bits/unique_ptr.h>
#include "../howie.h"

namespace howie {

  class StreamImpl : public HowieStream {
  public:

    StreamImpl(const HowieDeviceCharacteristics &deviceCharacteristics,
               HowieDeviceChangedCallback deviceChangedCallback,
               HowieProcessCallback processCallback)
          : deviceCharacteristics(deviceCharacteristics),
          deviceChangedCallback_(deviceChangedCallback),
          processCallback_(processCallback) {
      version = sizeof(*this);
    }

    HowieError init(SLEngineItf engineItf, SLObjectItf outputMixObject);

  private:
    HowieDeviceCharacteristics deviceCharacteristics;

    std::unique_ptr<unsigned char> buffer_;
    size_t bufferSize_;

    SLObjectItf bqPlayerObject_ = NULL;
    SLPlayItf bqPlayerItf_ = NULL;
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue_ = NULL;

    static void bqPlayerCallback(SLAndroidSimpleBufferQueueItf, void*);
    static HowieError lastErr_;

    HowieDeviceChangedCallback deviceChangedCallback_;
    HowieProcessCallback processCallback_;

    HowieError process(SLAndroidSimpleBufferQueueItf bq);
  };

} // namespace howie


#endif //HELLOLOWLATENCYOUTPUT_STREAMIMPL_H
