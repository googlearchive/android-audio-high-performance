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

#include <assert.h>
#include "synthesizer.h"
#include "trace.h"

#define DEFAULT_SINE_WAVE_FREQUENCY 440.0
#define TWO_PI (3.14159 * 2)

Synthesizer::Synthesizer(int num_audio_channels, int frame_rate):
    num_audio_channels_(num_audio_channels),
    frame_rate_(frame_rate){
  setWaveFrequency(DEFAULT_SINE_WAVE_FREQUENCY);
}

int Synthesizer::render(int num_samples, int16_t *audio_buffer) {

  Trace::beginSection("Synthesizer::render");

  assert(audio_buffer != nullptr);

  // Do some floating point operations to simulate the load required to produce complex
  // synthesizer voices
  float x = 0;
  for (int i = 1; i <= work_cycles_; i++) {
    float y = 1 / i;
    float z = 2 / i;
    x = x / (y * z);
  }

  // render an interleaved output with the same sample value per channel
  // For example: 6 samples of a 2 channel output stream could look like this
  // 1,1,2,2,3,3

  // Only render full frames
  int frames = num_samples / num_audio_channels_;
  int sample_count = 0;

  for (int i = 0; i < frames; i++){

    int16_t value = (int16_t) ((is_playing_) ? sin(current_phase_) * current_volume_ : 0);

    for (int j = 0; j < num_audio_channels_; j++){
      audio_buffer[sample_count] = value;
      sample_count++;
    }

    if (current_phase_ > TWO_PI) current_phase_ -= TWO_PI;
    current_phase_ += phase_increment_;
  }

  Trace::endSection();

  return sample_count;
}

void Synthesizer::setVolume(int volume) {
  current_volume_ = (volume < MAXIMUM_AMPLITUDE_VALUE) ? volume : MAXIMUM_AMPLITUDE_VALUE;
}

void Synthesizer::setWaveFrequency(float wave_frequency) {
  phase_increment_ = TWO_PI * wave_frequency / frame_rate_;
}

void Synthesizer::noteOn() {
  is_playing_ = true;
}

void Synthesizer::noteOff() {
  is_playing_ = false;
}

void Synthesizer::setWorkCycles(int work_cycles){
  work_cycles_ = work_cycles;
}
