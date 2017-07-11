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

#ifndef AAUDIO_ECHOAUDIOENGINE_H
#define AAUDIO_ECHOAUDIOENGINE_H

#include <thread>
#include "audio_common.h"
#include "audio_effect.h"

class EchoAudioEngine {

public:
  EchoAudioEngine();
  ~EchoAudioEngine();
  void setRecordingDeviceId(int32_t deviceId);
  void setPlaybackDeviceId(int32_t deviceId);
  void setEchoOn(bool isEchoOn);
  aaudio_data_callback_result_t dataCallback(AAudioStream *stream,
                                             void *audioData,
                                             int32_t numFrames);
  void errorCallback(AAudioStream *stream,
                     aaudio_result_t  __unused error);

private:

  bool isEchoOn_ = false;
  bool isFirstDataCallback_ = true;
  int32_t recordingDeviceId_ = AAUDIO_UNSPECIFIED;
  int32_t playbackDeviceId_ = AAUDIO_UNSPECIFIED;
  int32_t sampleRate_;
  int16_t sampleChannels_;
  int16_t bitsPerSample_;
  aaudio_format_t sampleFormat_;

  AAudioStream *recordingStream_ = nullptr;
  AAudioStream *playStream_ = nullptr;

  int32_t playStreamUnderrunCount_;
  int32_t bufSizeInFrames_;
  int32_t framesPerBurst_;
  int32_t defaultBufSizeInFrames_;

  std::thread* streamRestartThread_;
  std::mutex restartingLock_;
  AudioEffect *audioEffect_;

  void createRecordingStream();
  void drainRecordingStream(void *audioData, int32_t numFrames);
  void createPlaybackStream();

  void startStream(AAudioStream* stream);
  void stopStream(AAudioStream* stream);
  void closeStream(AAudioStream* stream);

  void startStreams();
  void stopStreams();
  void restartStreams();
  AAudioStreamBuilder* createStreamBuilder();

  void setupRecordingStreamParameters(AAudioStreamBuilder *builder);
  void setupPlaybackStreamParameters(AAudioStreamBuilder *builder);
  void setupCommonStreamParameters(AAudioStreamBuilder *builder);


};

#endif //AAUDIO_ECHOAUDIOENGINE_H
