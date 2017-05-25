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

#ifndef SIMPLESYNTH_LOAD_STABILIZER_H
#define SIMPLESYNTH_LOAD_STABILIZER_H

#include <SLES/OpenSLES_Android.h>
#include "trace.h"
#include "audio_renderer.h"

class LoadStabilizer : public AudioRenderer {

public:
  LoadStabilizer(AudioRenderer *audio_renderer, int64_t callback_period_ns);
  int render(int num_samples, int16_t *audio_buffer);
  void generateLoad(int64_t duration_in_nanos);
  void setStabilizationEnabled(bool is_enabled);

private:
  AudioRenderer *audio_renderer_;
  int64_t callback_period_;
  double ops_per_nano_ = 1;
  bool is_stabilization_enabled_;
  int64_t callback_count_;
  int64_t callback_epoch_;
};

#endif //SIMPLESYNTH_LOAD_STABILIZER_H
