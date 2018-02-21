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

#ifndef AAUDIO_PLAYAUDIOENGINE_H
#define AAUDIO_PLAYAUDIOENGINE_H

#include <thread>
#include "audio_common.h"
#include "SineGenerator.h"

#define BUFFER_SIZE_AUTOMATIC 0

class PlayAudioEngine {

public:
  PlayAudioEngine();
  ~PlayAudioEngine();
  void setDeviceId(int32_t deviceId);
  void setToneOn(bool isToneOn);
  void setBufferSizeInBursts(int32_t numBursts);
  aaudio_data_callback_result_t dataCallback(AAudioStream *stream,
                                             void *audioData,
                                             int32_t numFrames);
  void errorCallback(AAudioStream *stream,
                     aaudio_result_t  __unused error);
  double getCurrentOutputLatencyMillis();

private:

  int32_t playbackDeviceId_ = AAUDIO_UNSPECIFIED;
  int32_t sampleRate_;
  int16_t sampleChannels_;
  aaudio_format_t sampleFormat_;

  SineGenerator *sineOscLeft_;
  SineGenerator *sineOscRight_;

  AAudioStream *playStream_;
  bool isToneOn_ = false;

  int32_t playStreamUnderrunCount_;
  int32_t bufSizeInFrames_;
  int32_t framesPerBurst_;
  double currentOutputLatencyMillis_ = 0;
  int32_t bufferSizeSelection_ = BUFFER_SIZE_AUTOMATIC;

private:

  std::mutex restartingLock_;

  void createPlaybackStream();
  void closeOutputStream();
  void restartStream();

  AAudioStreamBuilder* createStreamBuilder();
  void setupPlaybackStreamParameters(AAudioStreamBuilder *builder);
  void prepareOscillators();

  aaudio_result_t calculateCurrentOutputLatencyMillis(AAudioStream *stream, double *latencyMillis);

};


#endif //AAUDIO_PLAYAUDIOENGINE_H
