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

#ifndef AAUDIO_AUDIOENGINE_H
#define AAUDIO_AUDIOENGINE_H

#include <thread>
#include "audio_common.h"
#include "SineGenerator.h"

class AudioEngine {

public:
  AudioEngine();
  ~AudioEngine();
  void setDeviceId(int32_t deviceId);
  void setToneOn(bool isToneOn);
  aaudio_data_callback_result_t dataCallback(AAudioStream *stream,
                                             void *audioData,
                                             int32_t numFrames);
  void errorCallback(AAudioStream *stream,
                     aaudio_result_t error);

private:

  int32_t deviceId_ = AAUDIO_UNSPECIFIED;
  int32_t sampleRate_;
  int16_t sampleChannels_;
  int16_t bitsPerSample_;
  aaudio_format_t sampleFormat_;

  SineGenerator *sineOscLeft;
  SineGenerator *sineOscRight;

  AAudioStream *playStream_;
  bool isToneOn_ = false;

  int32_t underRunCount_;
  int32_t bufSizeInFrames_;
  int32_t framesPerBurst_;
  int32_t defaultBufSizeInFrames_;

  std::thread* streamRestartThread_;
  std::mutex restartingLock_;

  void createOutputStream();
  void stopOutputStream();
  void restartStream();
};


#endif //AAUDIO_AUDIOENGINE_H
