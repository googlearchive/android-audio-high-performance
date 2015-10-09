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
