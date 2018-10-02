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
#include <unistd.h>
#include <sched.h>
#include <cstring>

#include <android/log.h>

#include "audio_player.h"
#include "android_log.h"

#define MILLIHERTZ_IN_HERTZ 1000
#define JAVA_PROXY_AVAILABLE_FROM_API_LEVEL 24

void SLPlayerCallback(SLAndroidSimpleBufferQueueItf buffer_queue_itf, void *context) {
  (static_cast<AudioPlayer *>(context))->processSLCallback(buffer_queue_itf);
}

AudioPlayer::AudioPlayer(SLEngineItf engine_itf,
                         SLObjectItf output_mix_object_itf,
                         AudioRenderer *renderer,
                         AudioStreamFormat stream_format,
                         int api_level) :
    renderer_(renderer),
    stream_format_(stream_format),
    is_thread_affinity_set_(false) {

  assert(renderer_ != nullptr);

  LOGV("Creating AudioPlayer with frame rate %d, "
           "frames per buffer %d, "
           "buffers %d",
       stream_format.frame_rate,
       stream_format.frames_per_buffer,
       stream_format.num_buffers);

  SLDataLocator_AndroidSimpleBufferQueue sl_data_locator_bufferqueue_source;
  SLDataFormat_PCM sl_data_format_pcm;
  SLDataSource sl_data_source;
  SLDataLocator_OutputMix sl_data_locator_output_mix;
  SLDataSink sl_data_sink;

  initAudioBuffer(stream_format_.frames_per_buffer,
                  stream_format_.num_audio_channels,
                  audio_buffer_);
  initDataLocatorBufferQueue((SLuint32) stream_format_.num_buffers,
                             &sl_data_locator_bufferqueue_source);
  initDataFormat((SLuint32) stream_format_.frame_rate,
                 (SLuint32) stream_format_.num_audio_channels,
                 &sl_data_format_pcm);
  initDataSource(&sl_data_locator_bufferqueue_source, &sl_data_format_pcm, &sl_data_source);
  initDataLocatorOutputMix(output_mix_object_itf, &sl_data_locator_output_mix);
  initDataSink(&sl_data_locator_output_mix, &sl_data_sink);

  // Now we have a data source and sink we are able to create the OpenSL Player
  createPlayer(engine_itf, &sl_data_source, &sl_data_sink, &sl_player_object_itf_);
  realizePlayer(sl_player_object_itf_);

  // If the API level is 24+ we can obtain the newer Android configuration interface which
  // allows us to obtain the Java AudioTrack object associated with playback, this can be used
  // to query the number of underruns which have occurred on the stream
  if (api_level >= JAVA_PROXY_AVAILABLE_FROM_API_LEVEL) {
    getAndroidConfigurationInterfaceAPI24(sl_player_object_itf_, &sl_android_config_itf_api24_);
    acquireJavaProxy(sl_android_config_itf_api24_, &java_proxy_);
  } else {
    // On API <24 we don't do anything with the Android configuration interface, however,
    // it is left here to demonstrate how it can be obtained
    getAndroidConfigurationInterface(sl_player_object_itf_, &sl_android_config_itf_);
  }

  getPlayInterface(sl_player_object_itf_, &sl_play_itf_);
  getBufferQueueInterface(sl_player_object_itf_, &sl_buffer_queue_itf_);
  registerCallback(sl_buffer_queue_itf_, SLPlayerCallback, this);
}

AudioPlayer::~AudioPlayer() {
  if (sl_player_object_itf_ != nullptr){
    (*sl_player_object_itf_)->Destroy(sl_player_object_itf_);
    sl_player_object_itf_ = nullptr;
  }
  delete[] audio_buffer_;
}

void AudioPlayer::initAudioBuffer(int frames_per_buffer,
                                  int num_audio_channels,
                                  int16_t *&audio_buffer) {

  int samples_per_buffer = frames_per_buffer * num_audio_channels;
  audio_buffer = new int16_t[samples_per_buffer];
  memset(audio_buffer, 0, samples_per_buffer * sizeof(int16_t));
  LOGV("audio buffer array allocated %d samples", samples_per_buffer);
}

void AudioPlayer::initDataLocatorBufferQueue(SLuint32 num_buffers,
                                             SLDataLocator_AndroidSimpleBufferQueue *data_locator) {

  data_locator->locatorType = SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE;
  data_locator->numBuffers = num_buffers;
}

void AudioPlayer::initDataFormat(SLuint32 frame_rate,
                                 SLuint32 num_channels,
                                 SLDataFormat_PCM *data_format) {

  data_format->formatType = SL_DATAFORMAT_PCM;
  data_format->numChannels = num_channels;

  // Note: samplesPerSec is an inaccurate name for this property since the expected
  // value is the "sample rate in milliHz"
  data_format->samplesPerSec = frame_rate * MILLIHERTZ_IN_HERTZ;
  data_format->bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
  data_format->containerSize = SL_PCMSAMPLEFORMAT_FIXED_16;

  // Note: because of an Android bug (id: 35749641) attempting to use indexed channel
  // masks results in a non-fast mixer thread. Uncomment this line when bug fixed.
  // data_format->channelMask = SL_ANDROID_MAKE_INDEXED_CHANNEL_MASK((1 << num_channels) - 1);
  data_format->channelMask = (SLuint32) (1 << num_channels) - 1;
  data_format->endianness = SL_BYTEORDER_LITTLEENDIAN;
}

void AudioPlayer::initDataSource(SLDataLocator_AndroidSimpleBufferQueue *data_locator,
                                 SLDataFormat_PCM *data_format,
                                 SLDataSource *data_source) {

  data_source->pLocator = data_locator;
  data_source->pFormat = data_format;
}

void AudioPlayer::initDataLocatorOutputMix(SLObjectItf output_mix_itf,
                                           SLDataLocator_OutputMix *data_locator) {

  data_locator->locatorType = SL_DATALOCATOR_OUTPUTMIX;
  data_locator->outputMix = output_mix_itf;
}

void AudioPlayer::initDataSink(SLDataLocator_OutputMix *data_locator, SLDataSink *data_sink) {

  data_sink->pLocator = data_locator;
  data_sink->pFormat = NULL;
}

void AudioPlayer::createPlayer(SLEngineItf engine_itf,
                               SLDataSource *data_source,
                               SLDataSink *data_sink,
                               SLObjectItf *player_object_itf) {

  // Note: Adding other output interfaces here may result in your audio being routed using the
  // normal path NOT the fast path
  const int numInterfaces = 2;
  const SLInterfaceID interfaceIds[numInterfaces] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                                                     SL_IID_ANDROIDCONFIGURATION};
  const SLboolean interfacesRequired[numInterfaces] = {SL_BOOLEAN_TRUE,
                                                       SL_BOOLEAN_TRUE};

  SLresult result = (*engine_itf)->CreateAudioPlayer(
      engine_itf,
      player_object_itf,
      data_source,
      data_sink,
      numInterfaces,
      interfaceIds,
      interfacesRequired
  );
  assert(SL_RESULT_SUCCESS == result);
}

void AudioPlayer::getAndroidConfigurationInterface(SLObjectItf player_object_itf,
                                                   SLAndroidConfigurationItf *config_itf) {

  SLresult result = (*player_object_itf)->GetInterface(player_object_itf,
                                                       SL_IID_ANDROIDCONFIGURATION,
                                                       config_itf);
  assert(SL_RESULT_SUCCESS == result);
}

void AudioPlayer::getAndroidConfigurationInterfaceAPI24(SLObjectItf player_object_itf,
                                                        SLAndroidConfigurationItfAPI24 *config_itf) {

  SLresult result = (*player_object_itf)->GetInterface(player_object_itf,
                                                       SL_IID_ANDROIDCONFIGURATION,
                                                       config_itf);
  assert(SL_RESULT_SUCCESS == result);
}

void AudioPlayer::realizePlayer(SLObjectItf player_object_itf) {

  SLresult result = (*player_object_itf)->Realize(player_object_itf,
                                                  SL_BOOLEAN_FALSE /* async */);
  assert(SL_RESULT_SUCCESS == result);
}

void AudioPlayer::getPlayInterface(SLObjectItf player_object_itf, SLPlayItf *play_itf) {

  SLresult result = (*player_object_itf)->GetInterface(player_object_itf, SL_IID_PLAY, play_itf);
  assert(SL_RESULT_SUCCESS == result);
}

void AudioPlayer::getBufferQueueInterface(SLObjectItf player_object_itf,
                                          SLAndroidSimpleBufferQueueItf *buffer_queue_itf) {

  SLresult result = (*player_object_itf)->GetInterface(player_object_itf,
                                                       SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                                                       buffer_queue_itf);
  assert(SL_RESULT_SUCCESS == result);
}

void AudioPlayer::registerCallback(SLAndroidSimpleBufferQueueItf buffer_queue_itf,
                                   sl_player_callback_function callback_function,
                                   void *context) {

  SLresult result = (*buffer_queue_itf)->RegisterCallback(buffer_queue_itf,
                                                          callback_function,
                                                          context);
  assert(SL_RESULT_SUCCESS == result);
}

void AudioPlayer::play() {

  if (sl_play_itf_ == nullptr) {
    LOGE("SLPlayItf was null");
  } else if (renderer_ == nullptr) {
    LOGE("Renderer was null");
  } else if (audio_buffer_ == nullptr) {
    LOGE("Audio buffer is null");
  } else {

    // set the player's state to playing
    SLresult result = (*sl_play_itf_)->SetPlayState(sl_play_itf_, SL_PLAYSTATE_PLAYING);
    assert(SL_RESULT_SUCCESS == result);
    LOGV("play state set to playing");

    (void) result;

    // Enqueue buffers of audio data to kick off the callbacks
    for (int i = 0; i < stream_format_.num_buffers; i++) {
      int samples_rendered = renderer_->render(
          stream_format_.frames_per_buffer * stream_format_.num_audio_channels,
          audio_buffer_);
      LOGV("Enqueuing buffer %d, samples rendered %d ", i, samples_rendered);

      result = (*sl_buffer_queue_itf_)->Enqueue(
          sl_buffer_queue_itf_,
          audio_buffer_,
          samples_rendered * sizeof(audio_buffer_[0]));
      assert(SL_RESULT_SUCCESS == result);
    }
  }
}

void AudioPlayer::stop() {

  if (sl_play_itf_ == nullptr) {
    LOGE("SLPlayItf was null");
  } else {
    SLresult result = (*sl_play_itf_)->SetPlayState(sl_play_itf_, SL_PLAYSTATE_STOPPED);
    assert(SL_RESULT_SUCCESS == result);
    LOGV("play state set to stopped");
  }
}

void AudioPlayer::processSLCallback(SLAndroidSimpleBufferQueueItf buffer_queue_itf) {

  if (callback_cpu_ids_.size() > 0 && !is_thread_affinity_set_) setThreadAffinity();

  int num_requested_samples = stream_format_.frames_per_buffer *
                              stream_format_.num_audio_channels;
  int num_rendered_samples = renderer_->render(num_requested_samples, audio_buffer_);
  SLresult result = (*buffer_queue_itf)->Enqueue(buffer_queue_itf,
                                                 audio_buffer_,
                                                 num_rendered_samples * sizeof(int16_t));
  assert(SL_RESULT_SUCCESS == result);
}

void AudioPlayer::setThreadAffinity() {

  pid_t current_thread_id = gettid();
  cpu_set_t cpu_set;
  CPU_ZERO(&cpu_set);

  // If the callback cpu ids aren't specified then bind to the current cpu
  if (callback_cpu_ids_.empty()) {
    int current_cpu_id = sched_getcpu();
    LOGV("Current CPU ID is %d", current_cpu_id);
    CPU_SET(current_cpu_id, &cpu_set);
  } else {

    for (size_t i = 0; i < callback_cpu_ids_.size(); i++) {
      int cpu_id = callback_cpu_ids_.at(i);
      LOGV("CPU ID %d added to cores set", cpu_id);
      CPU_SET(cpu_id, &cpu_set);
    }
  }

  int result = sched_setaffinity(current_thread_id, sizeof(cpu_set_t), &cpu_set);
  if (result == 0) {
    LOGV("Thread affinity set");
  } else {
    LOGW("Error setting thread affinity. Error no: %d", result);
  }

  is_thread_affinity_set_ = true;
}

void AudioPlayer::setCallbackThreadCPUIds(std::vector<int> cpu_ids) {

  is_thread_affinity_set_ = false;
  callback_cpu_ids_ = cpu_ids;
}

void AudioPlayer::acquireJavaProxy(SLAndroidConfigurationItfAPI24 config_itf, jobject *java_proxy) {

  SLresult result = (*config_itf)->AcquireJavaProxy(config_itf, SL_ANDROID_JAVA_PROXY_ROUTING,
                                                    java_proxy);
  assert(SL_RESULT_SUCCESS == result);
  (void) result;
}

jobject AudioPlayer::getAudioTrack() {
  return java_proxy_;
}
