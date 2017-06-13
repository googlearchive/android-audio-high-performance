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
#include "load_stabilizer.h"
#include "cpu_relax.h"
#include "android_log.h"
#include "audio_common.h"

#define LOAD_GENERATION_STEP_SIZE_IN_NANOS 1000
#define PERCENTAGE_OF_CALLBACK_TO_USE 0.8

LoadStabilizer::LoadStabilizer(AudioRenderer *audio_renderer, int64_t callback_period_ns) :
    audio_renderer_(audio_renderer),
    callback_period_(callback_period_ns),
    is_stabilization_enabled_(false),
    callback_count_(0){

  assert(callback_period_ns > 0);

  LOGV("Creating load stabilizer with callback period %lld", (long long)callback_period_);
}

int LoadStabilizer::render(int num_samples, int16_t *audio_buffer) {

  Trace::beginSection("LoadStabilizer::render start");
  int rendered_samples = 0;

  if (is_stabilization_enabled_){

    int64_t start_time = get_time();
    if (callback_count_ == 0) callback_epoch_ = start_time;

    // get the deadline for this callback by calculating the periods since the first callback
    int64_t time_since_epoch = start_time - callback_epoch_;
    int64_t periods_since_epoch = time_since_epoch / callback_period_;

    if (periods_since_epoch < callback_count_){

      // Previous epoch was set using a late callback
      // reset to this new earlier (more accurate) callback time
      callback_epoch_ = start_time;
      callback_count_ = 0;
      time_since_epoch = 0;
      periods_since_epoch = 0;
    }

    int64_t started_late_duration = time_since_epoch - (periods_since_epoch * callback_period_);
    int64_t target_duration = (int64_t)(callback_period_ *
        PERCENTAGE_OF_CALLBACK_TO_USE) - started_late_duration;

    Trace::beginSection("Actual load");
    rendered_samples = audio_renderer_->render(num_samples, audio_buffer);
    Trace::endSection();

    int64_t real_execution_duration = get_time() - start_time;
    int64_t stabilizing_load_duration = target_duration - real_execution_duration;

    if (stabilizing_load_duration > 0){
      Trace::beginSection("Stabilizing load");
      generateLoad(stabilizing_load_duration);
      Trace::endSection();
    }

    callback_count_++;

  } else {

    // just call the wrapped function directly, no load stabilization
    Trace::beginSection("Actual load");
    rendered_samples = audio_renderer_->render(num_samples, audio_buffer);
    Trace::endSection();
  }

  Trace::endSection();

  return rendered_samples;
}

// Generates a stabilizing load by executing cpu instructions for the specified time
void LoadStabilizer::generateLoad(int64_t duration_in_nanos){

  int64_t current_time = get_time();
  int64_t deadline_time = current_time + duration_in_nanos;

  // ops_per_step gives us an estimated number of operations which need to be run to fully utilize
  // the CPU for a fixed amount of time (specified by LOAD_GENERATION_STEP_SIZE_IN_NANOS).
  // After each step the ops_per_step value is re-calculated based on the actual time taken to
  // execute those operations.
  int ops_per_step = (int)(ops_per_nano_ * LOAD_GENERATION_STEP_SIZE_IN_NANOS);
  int64_t step_duration = 0;
  int64_t previous_time = 0;

  while (current_time <= deadline_time){

    for (int i = 0; i < ops_per_step; i++){
      cpu_relax();
    }

    previous_time = current_time;
    current_time = get_time();
    step_duration = current_time - previous_time;
    ops_per_nano_ = ops_per_step / step_duration;
    ops_per_step = (int)(ops_per_nano_ * LOAD_GENERATION_STEP_SIZE_IN_NANOS);
  }
}

void LoadStabilizer::setStabilizationEnabled(bool is_enabled){
  LOGV("Load stabilization set to %d", is_enabled);
  is_stabilization_enabled_ = is_enabled;
}
