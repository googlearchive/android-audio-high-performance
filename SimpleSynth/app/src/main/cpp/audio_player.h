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

#ifndef SIMPLESYNTH_AUDIO_PLAYER_H
#define SIMPLESYNTH_AUDIO_PLAYER_H

#include <stdint.h>

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <vector>
#include <jni.h>
#include "audio_renderer.h"
#include "audio_common.h"
#include "OpenSLES_Android_API24.h"


typedef void (*sl_player_callback_function)(SLAndroidSimpleBufferQueueItf buffer_queue_itf,
                                            void *context);

class AudioPlayer {

public:
  AudioPlayer(SLEngineItf sl_engine_itf,
              SLObjectItf output_mix_itf,
              AudioRenderer *renderer,
              AudioStreamFormat stream_format,
              int api_level);

  virtual ~AudioPlayer();

  void processSLCallback(SLAndroidSimpleBufferQueueItf buffer_queue_itf);

  void play();

  void stop();

  void setCallbackThreadCPUIds(std::vector<int> core_ids);

  jobject getAudioTrack();

private:

  // Methods
  void initAudioBuffer(int frames_per_buffer,
                       int num_audio_channels,
                       int16_t *&audio_buffer);

  void initDataLocatorBufferQueue(SLuint32 num_buffers,
                                  SLDataLocator_AndroidSimpleBufferQueue *data_locator);

  void initDataFormat(SLuint32 frame_rate,
                      SLuint32 num_channels,
                      SLDataFormat_PCM *data_format);

  void initDataSource(SLDataLocator_AndroidSimpleBufferQueue *data_locator,
                      SLDataFormat_PCM *data_format,
                      SLDataSource *data_source);

  void initDataLocatorOutputMix(SLObjectItf output_mix_itf,
                                SLDataLocator_OutputMix *data_locator);

  void initDataSink(SLDataLocator_OutputMix *data_locator,
                    SLDataSink *data_sink);

  void createPlayer(SLEngineItf engine_itf,
                    SLDataSource *data_source,
                    SLDataSink *data_sink,
                    SLObjectItf *player_object_itf);

  void getAndroidConfigurationInterface(SLObjectItf player_object_itf,
                                        SLAndroidConfigurationItf *config_itf);

  void getAndroidConfigurationInterfaceAPI24(SLObjectItf player_object_itf,
                                             SLAndroidConfigurationItfAPI24 *config_itf);

  void realizePlayer(SLObjectItf player_object_itf);

  void getPlayInterface(SLObjectItf player_object_itf, SLPlayItf *play_itf);

  void getBufferQueueInterface(SLObjectItf player_object_itf,
                               SLAndroidSimpleBufferQueueItf *buffer_queue_itf);

  void registerCallback(SLAndroidSimpleBufferQueueItf buffer_queue_itf,
                        sl_player_callback_function callback_function,
                        void *context);

  void setThreadAffinity();

  void acquireJavaProxy(SLAndroidConfigurationItfAPI24 config_itf, jobject *java_proxy);

  // Member variables
  AudioRenderer *renderer_ = nullptr;
  AudioStreamFormat stream_format_;
  int16_t *audio_buffer_;
  jobject java_proxy_ = nullptr;

  // OpenSL objects
  SLObjectItf sl_player_object_itf_ = nullptr;
  SLAndroidConfigurationItf sl_android_config_itf_ = nullptr;
  SLAndroidConfigurationItfAPI24 sl_android_config_itf_api24_ = nullptr;
  SLPlayItf sl_play_itf_ = nullptr;
  SLAndroidSimpleBufferQueueItf sl_buffer_queue_itf_ = nullptr;

  // Performance options
  bool is_thread_affinity_set_ = false;
  std::vector<int> callback_cpu_ids_;
};


#endif //SIMPLESYNTH_AUDIO_PLAYER_H
