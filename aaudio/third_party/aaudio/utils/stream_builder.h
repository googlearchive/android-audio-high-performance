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
 */

#ifndef AAUDIO_STREAM_BIULDER_H
#define AAUDIO_STREAM_BIULDER_H
#include <cassert>

#include <aaudio/AAudio.h>

class StreamBuilder {
public:
    explicit StreamBuilder() {}
    ~StreamBuilder() {};

    /*
     * purposely left
     *     deviceID
     *     bufferSize
     *     sampleRate
     * as default, expecting that AAudio will pick up the right one for us
     */
    AAudioStream *CreateStream(
            aaudio_format_t format,
            int32_t samplesPerFrame,
            aaudio_sharing_mode_t sharing,
            aaudio_performance_mode_t performanceMode = AAUDIO_PERFORMANCE_MODE_NONE,
            aaudio_direction_t dir = AAUDIO_DIRECTION_OUTPUT,
            int32_t sampleRate = AAUDIO_UNSPECIFIED,
            AAudioStream_dataCallback  callback = nullptr,
            void    *userData = nullptr,
            int32_t deviceId = AAUDIO_UNSPECIFIED) {

      AAudioStreamBuilder* builder;
      aaudio_result_t  result = AAudio_createStreamBuilder(&builder);
      if (result != AAUDIO_OK && !builder) {
        assert(false);
      }

      AAudioStreamBuilder_setFormat(builder, format);
      AAudioStreamBuilder_setSharingMode(builder, sharing);
      AAudioStreamBuilder_setPerformanceMode(builder, performanceMode);
      AAudioStreamBuilder_setDirection(builder, dir);
      AAudioStreamBuilder_setSampleRate(builder, sampleRate);
      AAudioStreamBuilder_setSamplesPerFrame(builder, samplesPerFrame);
      if (sampleRate != AAUDIO_UNSPECIFIED) {
        AAudioStreamBuilder_setSampleRate(builder, sampleRate);
      }
      AAudioStreamBuilder_setDataCallback(builder, callback, userData);
      AAudioStreamBuilder_setDeviceId(builder, deviceId);

      AAudioStream* stream;
      result = AAudioStreamBuilder_openStream(builder, &stream);
      if (result != AAUDIO_OK) {
        assert(false);
        stream = nullptr;
      }

      AAudioStreamBuilder_delete(builder);

      return stream;
    }
};

#endif //AAUDIO_STREAM_BIULDER_H
