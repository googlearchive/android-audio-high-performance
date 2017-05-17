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


#ifndef AAUDIO_AUDIO_COMMON_H
#define AAUDIO_AUDIO_COMMON_H

#include <chrono>
#include <aaudio/AAudio.h>
#include "android_debug.h"
#include "debug_utils.h"

/*
 * Audio Sample Controls...
 */
#define AUDIO_SAMPLE_CHANNELS               1

uint16_t SampleFormatToBpp(aaudio_audio_format_t format);
/*
 * GetSystemTicks(void):  return the time in micro sec
 */
__inline__ uint64_t GetSystemTicks(void) {
    struct timeval Time;
    gettimeofday( &Time, NULL );

    return (static_cast<uint64_t>(1000000) * Time.tv_sec + Time.tv_usec);
}

/*
 * flag to enable file dumping
 */
//#define ENABLE_LOG  1

void PrintAudioStreamInfo(const AAudioStream * stream);
#endif // AAUDIO_AUDIO_COMMON_H
