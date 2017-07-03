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
#include <stream_builder.h>
#include "audio_engine.h"


/**
 * AAudio callback functions
 * These call the equivalent method in the AudioEngine instance
 */
extern "C" {

aaudio_data_callback_result_t dataCallback(AAudioStream *stream, void *userData,
                                           void *audioData, int32_t numFrames) {
  assert(userData && audioData);
  AudioEngine *audioEngine = reinterpret_cast<AudioEngine *>(userData);
  return audioEngine->dataCallback(stream, audioData, numFrames);
}

void errorCallback(AAudioStream *stream,
                   void *userData,
                   aaudio_result_t error) {
  assert(userData);
  AudioEngine *audioEngine = reinterpret_cast<AudioEngine *>(userData);
  audioEngine->errorCallback(stream, error);
}

} // end extern "C"

AudioEngine::AudioEngine() {

  sampleChannels_ = AUDIO_SAMPLE_CHANNELS;
  sampleFormat_ = AAUDIO_FORMAT_PCM_I16;
  bitsPerSample_ = SampleFormatToBpp(sampleFormat_);

  // Create the output stream. By not specifying an audio device id we are telling AAudio that
  // we want the stream to be created using the default playback audio device.
  createPlaybackStream();
}

AudioEngine::~AudioEngine(){

  stopOutputStream();
  delete sineOscLeft;
  delete sineOscRight;
}

void AudioEngine::setDeviceId(int32_t deviceId){

  deviceId_ = deviceId;

  // If this is a different device from the one currently in use then restart the stream
  int32_t currentDeviceId = AAudioStream_getDeviceId(playStream_);
  if (deviceId != currentDeviceId) restartStream();
}

void AudioEngine::createPlaybackStream(){

  AAudioStreamBuilder* builder;
  aaudio_result_t  result = AAudio_createStreamBuilder(&builder);
  if (result != AAUDIO_OK && !builder) {
    assert(false);
  }

  /**
   * The following properties are not set because the builder will pick the values
   * automatically for us:
   *
   * - Sample rate (can be set using AAudioStreamBuilder_setSampleRate())
   *
   */
  AAudioStreamBuilder_setDeviceId(builder, deviceId_);
  AAudioStreamBuilder_setFormat(builder, sampleFormat_);
  AAudioStreamBuilder_setSamplesPerFrame(builder, sampleChannels_);
  AAudioStreamBuilder_setSharingMode(builder, AAUDIO_SHARING_MODE_SHARED);
  AAudioStreamBuilder_setPerformanceMode(builder, AAUDIO_PERFORMANCE_MODE_LOW_LATENCY);
  AAudioStreamBuilder_setDirection(builder, AAUDIO_DIRECTION_OUTPUT);
  AAudioStreamBuilder_setDataCallback(builder, ::dataCallback, this);
  AAudioStreamBuilder_setErrorCallback(builder, ::errorCallback, this);

  result = AAudioStreamBuilder_openStream(builder, &playStream_);
  if (result != AAUDIO_OK || playStream_ == nullptr) {
    LOGE("Failed to create stream. Error: %s", AAudio_convertResultToText(result));
    return;
  }

  // check that we got PCM_I16 format
  if (sampleFormat_ != AAudioStream_getFormat(playStream_)) {
    LOGE("Sample format is not PCM_I16");
    return;
  }

  sampleRate_ = AAudioStream_getSampleRate(playStream_);
  framesPerBurst_ = AAudioStream_getFramesPerBurst(playStream_);
  defaultBufSizeInFrames_ = AAudioStream_getBufferSizeInFrames(playStream_);

  // Set the buffer size to the burst size - this will give us the minimum possible latency
  AAudioStream_setBufferSizeInFrames(playStream_, framesPerBurst_);
  bufSizeInFrames_ = framesPerBurst_;

  PrintAudioStreamInfo(playStream_);
  AAudioStreamBuilder_delete(builder);

  // Prepare the oscillators
  sineOscLeft = new SineGenerator();
  sineOscLeft->setup(440.0, sampleRate_, 0.25);
  sineOscRight = new SineGenerator();
  sineOscRight->setup(660.0, sampleRate_, 0.25);

  // Start the stream - the dataCallback function will start being called
  result = AAudioStream_requestStart(playStream_);
  if (result != AAUDIO_OK) {
    LOGE("Error starting stream. %s", AAudio_convertResultToText(result));
  }

  playStreamUnderrunCount_ = AAudioStream_getXRunCount(playStream_);
}

void AudioEngine::stopOutputStream(){

  if (playStream_ != nullptr){
    aaudio_result_t result = AAudioStream_requestStop(playStream_);
    if (result != AAUDIO_OK){
      LOGE("Error stopping output stream. %s", AAudio_convertResultToText(result));
    }

    result = AAudioStream_close(playStream_);
    if (result != AAUDIO_OK){
      LOGE("Error closing output stream. %s", AAudio_convertResultToText(result));
    }
  }
}

void AudioEngine::setToneOn(bool isToneOn){
  isToneOn_ = isToneOn;
}

aaudio_data_callback_result_t AudioEngine::dataCallback(AAudioStream *stream,
                                                        void *audioData,
                                                        int32_t numFrames) {
  assert(stream == playStream_);
  int32_t underRun = AAudioStream_getXRunCount(playStream_);
  if (underRun > playStreamUnderrunCount_) {
    playStreamUnderrunCount_ = underRun;

    aaudio_result_t actSize = AAudioStream_setBufferSizeInFrames(
        stream, bufSizeInFrames_ + framesPerBurst_);
    if (actSize > 0) {
      bufSizeInFrames_ = actSize;
    } else {
      LOGE("*****: Error from dataCallback  -- %s",
           AAudio_convertResultToText(actSize));
    }
  }

  int32_t samplesPerFrame = sampleChannels_;

  if (isToneOn_) {
    sineOscRight->render(static_cast<int16_t *>(audioData),
                                      samplesPerFrame, numFrames);
    if (sampleChannels_ == 2) {
      sineOscLeft->render(static_cast<int16_t *>(audioData) + 1,
                                       samplesPerFrame, numFrames);
    }
  } else {
    memset(static_cast<uint8_t *>(audioData), 0,
           sizeof(int16_t) * samplesPerFrame * numFrames);
  }

  return AAUDIO_CALLBACK_RESULT_CONTINUE;
}

void AudioEngine::errorCallback(AAudioStream *stream,
                   aaudio_result_t error){

  assert(stream == playStream_);
  LOGD("errorCallback result: %s", AAudio_convertResultToText(error));

  aaudio_stream_state_t streamState = AAudioStream_getState(playStream_);
  if (streamState == AAUDIO_STREAM_STATE_DISCONNECTED){

    // Handle stream restart on a separate thread
    std::function<void(void)> restartStream = std::bind(&AudioEngine::restartStream, this);
    streamRestartThread_ = new std::thread(restartStream);
  }
}

void AudioEngine::restartStream(){

  LOGI("Restarting stream");

  if (restartingLock_.try_lock()){
    stopOutputStream();
    createPlaybackStream();
    restartingLock_.unlock();
  } else {
    LOGW("Restart stream operation already in progress - ignoring this request");
    // We were unable to obtain the restarting lock which means the restart operation is currently
    // active. This is probably because we received successive "stream disconnected" events.
    // Internal issue b/63087953
  }
}
