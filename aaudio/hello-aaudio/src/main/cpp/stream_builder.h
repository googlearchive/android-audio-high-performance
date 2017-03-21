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

//Replace this include with NDK's AAudio.h when available
#include <include/AAudio_Wrapper.h>

class StreamBuilder {
public:
    StreamBuilder() :
        sampleRate_(0),
        samplesPerFrame_(0),
        format_(AAUDIO_FORMAT_PCM_I16),
        sharing_(AAUDIO_SHARING_MODE_SHARED),
        direction_(AAUDIO_DIRECTION_OUTPUT),
        stream_(nullptr) {
      aaudio_result_t result = AAudio_createStreamBuilder(&builder_);
      assert(result == AAUDIO_OK && builder_);
    };
    StreamBuilder(
                  int32_t sampleRate,
                  int32_t samplesPerFrame,
                  aaudio_audio_format_t format,
                  aaudio_sharing_mode_t sharing,
                  aaudio_direction_t dir) :
          sampleRate_(sampleRate),
          samplesPerFrame_(samplesPerFrame),
          format_(format),
          sharing_(sharing),
          direction_(dir) {
      aaudio_result_t  result = AAudio_createStreamBuilder(&builder_);
      assert(result == AAUDIO_OK && builder_);
      AAudioStreamBuilder_setSampleRate(builder_, sampleRate_);
      AAudioStreamBuilder_setSamplesPerFrame(builder_, samplesPerFrame_);
      AAudioStreamBuilder_setFormat(builder_, format_);
      AAudioStreamBuilder_setSharingMode(builder_, sharing_);
      AAudioStreamBuilder_setDirection(builder_, direction_);
      result = AAudioStreamBuilder_openStream(builder_, &stream_);
      assert(result == AAUDIO_OK && stream_);
    }
    ~StreamBuilder() {
      if (builder_)
        AAudioStreamBuilder_delete(builder_);
      builder_ = nullptr;
    };
    int32_t DeviceId(void) {
      return stream_ ? AAudioStream_getDeviceId(stream_) : 0 ;
    }
    void SampleRate(int32_t rate) { sampleRate_ = rate; }
    int32_t SampleRate(void) const { return sampleRate_; }
    void SamplesPerFrame(int32_t samplesPerFrame) { samplesPerFrame_ = samplesPerFrame; }
    int32_t SamplesPerFrame(void) { return samplesPerFrame_; }
    void Format(aaudio_audio_format_t format) { format_ = format; }
    aaudio_audio_format_t Format(void)  const { return format_; }
    void SharingMode(aaudio_sharing_mode_t sharing) { sharing_ = sharing; }
    aaudio_sharing_mode_t SharingMode(void) const { return  sharing_; }
    void Direction(aaudio_direction_t dir) { direction_ = dir; }
    aaudio_direction_t Direction(void) const { return direction_; }
    int32_t BufferCapacity(void) {
      return stream_ ? AAudioStream_getBufferCapacityInFrames(stream_) : 0;
    }
    AAudioStream* Stream(void) {
      if (!stream_) {
        AAudioStreamBuilder_setSampleRate(builder_, sampleRate_);
        AAudioStreamBuilder_setSamplesPerFrame(builder_, samplesPerFrame_);
        AAudioStreamBuilder_setFormat(builder_, format_);
        AAudioStreamBuilder_setSharingMode(builder_, sharing_);
        AAudioStreamBuilder_setDirection(builder_, direction_);
        aaudio_result_t result = AAudioStreamBuilder_openStream(builder_, &stream_);
        assert(stream_ && result == AAUDIO_OK);
      }
      return stream_;
    }

private:
    AAudioStreamBuilder* builder_;
    int32_t sampleRate_;
    int32_t samplesPerFrame_;
    aaudio_audio_format_t format_;
    aaudio_sharing_mode_t sharing_;
    aaudio_direction_t direction_;
    AAudioStream* stream_;
};

#endif //AAUDIO_STREAM_BIULDER_H
