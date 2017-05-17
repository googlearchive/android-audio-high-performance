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

// Replace this include with NDK's AAudio.h when available
#include <aaudio/AAudio.h>
#define INVALID_AUDIO_PARAM 0

class StreamBuilder {
public:
    explicit StreamBuilder() {
      aaudio_result_t  result = AAudio_createStreamBuilder(&builder_);
      if (result != AAUDIO_OK && !builder_) {
          assert(false);
      }
    }
    ~StreamBuilder() {
      if (builder_)
        AAudioStreamBuilder_delete(builder_);
      builder_ = nullptr;
    };

    /*
     * purposely left
     *     deviceID
     *     bufferSize
     *     sampleRate
     * as default, expecting that AAudio will pick up the right one for us
     */
    AAudioStream *CreateStream(
            aaudio_audio_format_t format,
            int32_t samplesPerFrame,
            aaudio_sharing_mode_t sharing,
            aaudio_direction_t dir = AAUDIO_DIRECTION_OUTPUT,
            int32_t sampleRate = INVALID_AUDIO_PARAM,
            AAudioStream_dataCallback  callback = nullptr,
            void    *userData = nullptr) {

      AAudioStreamBuilder_setFormat(builder_, format);
      AAudioStreamBuilder_setSharingMode(builder_, sharing);
      AAudioStreamBuilder_setDirection(builder_, dir);
      AAudioStreamBuilder_setSampleRate(builder_, sampleRate);
      AAudioStreamBuilder_setSamplesPerFrame(builder_, samplesPerFrame);
      if (sampleRate != INVALID_AUDIO_PARAM) {
        AAudioStreamBuilder_setSampleRate(builder_, sampleRate);
      }
      AAudioStreamBuilder_setDataCallback(builder_, callback, userData);

      AAudioStream* stream;
      aaudio_result_t result = AAudioStreamBuilder_openStream(builder_, &stream);
      if (result != AAUDIO_OK) {
        assert(false);
        stream = nullptr;
      }
      return stream;
    }

private:
    AAudioStreamBuilder* builder_;
};

#endif //AAUDIO_STREAM_BIULDER_H
