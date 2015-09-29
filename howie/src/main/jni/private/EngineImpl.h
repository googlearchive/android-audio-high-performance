//
// Created by ilewis on 9/25/15.
//

#ifndef HELLOLOWLATENCYOUTPUT_ENGINE_H
#define HELLOLOWLATENCYOUTPUT_ENGINE_H

#include "howie-private.h"

namespace howie {


  class EngineImpl {
  public:
    // Engine is a singleton. There doesn't seem to be a compelling reason
    // to support more than one, and making it a singleton
    static EngineImpl *get() { return instance_; }

    EngineImpl(int sampleRate,
               int bitsPerSample,
               int bytesPerSample,
               int sampleMask,
               bool floatingPoint,
               int channelCount,
               int samplesPerFrame,
               int framesPerBuffer);

    ~EngineImpl();

    HowieError init();

    HowieError createStream(
        const HowieStreamCreationParams &params,
        HowieStream **out_stream);

  private:
    HowieDeviceCharacteristics deviceCharacteristics;

    SLObjectItf engineObject_ = NULL;
    SLEngineItf engineItf_ = NULL;
    SLObjectItf outputMixObject_ = NULL;
    static EngineImpl *instance_;

  };

} // namespace howie

#endif //HELLOLOWLATENCYOUTPUT_ENGINE_H
