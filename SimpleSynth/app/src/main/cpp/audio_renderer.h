/*
 * Copyright 2017 The Android Open Source Project
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
 */

#ifndef SIMPLESYNTH_AUDIO_RENDERER_H
#define SIMPLESYNTH_AUDIO_RENDERER_H

#include <stdint.h>

class AudioRenderer {

public:
  /**
    * Render signed 16-bit audio samples into the supplied audio buffer
    *
    * @param num_samples number of samples to render
    * @param audio_buffer array into which samples should be rendered
    * @return number of samples which were actually rendered
    */
  virtual int render(int num_samples, int16_t *audio_buffer) = 0;
};


#endif //SIMPLESYNTH_AUDIO_RENDERER_H
