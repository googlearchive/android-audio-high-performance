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

#ifndef SIMPLESYNTH_SYNTHESIZER_H
#define SIMPLESYNTH_SYNTHESIZER_H

#include <stdint.h>
#include <math.h>
#include "audio_renderer.h"

#define MAXIMUM_AMPLITUDE_VALUE 10000


class Synthesizer : public AudioRenderer {

public:
  Synthesizer(int num_audio_channels, int frame_rate);

  virtual int render(int num_samples, int16_t *audio_buffer);

  void setVolume(int volume);

  void setWaveFrequency(float wave_frequency);

  void noteOn();

  void noteOff();

  void setWorkCycles(int work_cycles);

private:
  int num_audio_channels_;
  int frame_rate_;
  double phase_increment_;
  double current_phase_ = 0.0;
  int current_volume_ = MAXIMUM_AMPLITUDE_VALUE;
  bool is_playing_ = false;
  int work_cycles_ = 0;
};

#endif //SIMPLESYNTH_SYNTHESIZER_H
