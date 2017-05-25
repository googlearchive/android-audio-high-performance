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

#ifndef SIMPLESYNTH_AUDIO_COMMON_H
#define SIMPLESYNTH_AUDIO_COMMON_H

#include <stdint.h>
#include <time.h>

#define NANOS_IN_SECOND 1000000000

#define SLASSERT(x)   do {\
    assert(SL_RESULT_SUCCESS == (x));\
    (void) (x);\
    } while (0)

struct AudioStreamFormat {
  uint32_t   frame_rate;
  uint32_t   frames_per_buffer;
  uint16_t   num_audio_channels;
  uint16_t   num_buffers;
};

int64_t timestamp_to_nanos(timespec ts);
int64_t get_time();

#endif //SIMPLESYNTH_AUDIO_COMMON_H
