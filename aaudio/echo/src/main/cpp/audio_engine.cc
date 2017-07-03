#include <climits>/*
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
}

AudioEngine::~AudioEngine(){

  closeStream(recordingStream_);
  closeStream(playStream_);

}

void AudioEngine::setRecordingDeviceId(int32_t deviceId){

  recordingDeviceId_ = deviceId;
}

void AudioEngine::setPlaybackDeviceId(int32_t deviceId){

  playbackDeviceId_ = deviceId;
}

void AudioEngine::setEchoOn(bool isEchoOn){

  isEchoOn_ = isEchoOn;

  if (isEchoOn){
    startStreams();
  } else {
    stopStreams();
  }
}

void AudioEngine::startStreams(){

  // Note: The order of stream creation is important. We create the playback stream first,
  // then use properties from the playback stream (e.g. sample rate) to create the
  // recording stream. By matching the properties we should get the lowest latency path
  createPlaybackStream();
  createRecordingStream();

  // Now start the recording stream so that we can read from it during the playback dataCallback
  startStream(recordingStream_);
  startStream(playStream_);
  playStreamUnderrunCount_ = AAudioStream_getXRunCount(playStream_);
}

void AudioEngine::stopStreams(){

  if (recordingStream_ != nullptr){
    stopStream(recordingStream_);
    closeStream(recordingStream_);
  }

  if (playStream_ != nullptr){
    stopStream(playStream_);
    closeStream(playStream_);
  }
}

AAudioStreamBuilder* AudioEngine::createStreamBuilder(){

  AAudioStreamBuilder* builder = nullptr;
  aaudio_result_t result = AAudio_createStreamBuilder(&builder);
  if (result != AAUDIO_OK && !builder) {
    LOGE("Error creating stream builder: %s", AAudio_convertResultToText(result));
  }
  return builder;
}

void AudioEngine::createRecordingStream() {

  AAudioStreamBuilder* builder = createStreamBuilder();
  AAudioStreamBuilder_setDeviceId(builder, recordingDeviceId_);
  AAudioStreamBuilder_setFormat(builder, sampleFormat_);
  AAudioStreamBuilder_setSamplesPerFrame(builder, sampleChannels_);
  AAudioStreamBuilder_setSharingMode(builder, AAUDIO_SHARING_MODE_SHARED);
  AAudioStreamBuilder_setPerformanceMode(builder, AAUDIO_PERFORMANCE_MODE_LOW_LATENCY);
  AAudioStreamBuilder_setDirection(builder, AAUDIO_DIRECTION_INPUT);
  AAudioStreamBuilder_setSampleRate(builder, sampleRate_);
  AAudioStreamBuilder_setErrorCallback(builder, ::errorCallback, this);

  aaudio_result_t result = AAudioStreamBuilder_openStream(builder, &recordingStream_);
  if (result != AAUDIO_OK || recordingStream_ == nullptr) {
    LOGE("Failed to create recording stream. Error: %s", AAudio_convertResultToText(result));
    return;
  }

  // Check that the stream format matches what we requested
  if (sampleFormat_ != AAudioStream_getFormat(recordingStream_)) {
    LOGW("Recording stream format doesn't match what was requested (format: %d). "
             "Higher latency expected.", sampleFormat_);
  }

  PrintAudioStreamInfo(recordingStream_);
}

void AudioEngine::createPlaybackStream(){

  AAudioStreamBuilder* builder = createStreamBuilder();
  AAudioStreamBuilder_setDeviceId(builder, playbackDeviceId_);
  AAudioStreamBuilder_setFormat(builder, sampleFormat_);
  AAudioStreamBuilder_setSamplesPerFrame(builder, sampleChannels_);
  AAudioStreamBuilder_setSharingMode(builder, AAUDIO_SHARING_MODE_SHARED);
  AAudioStreamBuilder_setPerformanceMode(builder, AAUDIO_PERFORMANCE_MODE_LOW_LATENCY);
  AAudioStreamBuilder_setDirection(builder, AAUDIO_DIRECTION_OUTPUT);

  // The :: here indicates that the function is in the global namespace
  // i.e. *not* AudioEngine::dataCallback, but dataCallback defined at the top of this class
  AAudioStreamBuilder_setDataCallback(builder, ::dataCallback, this);
  AAudioStreamBuilder_setErrorCallback(builder, ::errorCallback, this);

  aaudio_result_t result = AAudioStreamBuilder_openStream(builder, &playStream_);
  if (result != AAUDIO_OK || playStream_ == nullptr) {
    LOGE("Failed to create playback stream. Error: %s", AAudio_convertResultToText(result));
    return;
  }

  // check that we got PCM_I16 format
  if (sampleFormat_ != AAudioStream_getFormat(playStream_)) {
    LOGW("Playback stream sample format is not PCM_I16, higher latency expected");
  }

  sampleRate_ = AAudioStream_getSampleRate(playStream_);
  framesPerBurst_ = AAudioStream_getFramesPerBurst(playStream_);
  defaultBufSizeInFrames_ = AAudioStream_getBufferSizeInFrames(playStream_);

  // Set the buffer size to the burst size - this will give us the minimum possible latency
  AAudioStream_setBufferSizeInFrames(playStream_, framesPerBurst_);
  bufSizeInFrames_ = framesPerBurst_;

  PrintAudioStreamInfo(playStream_);
  AAudioStreamBuilder_delete(builder);
}

void AudioEngine::startStream(AAudioStream* stream){

  // Start the stream - if set the dataCallback function will start being called
  aaudio_result_t result = AAudioStream_requestStart(stream);
  if (result != AAUDIO_OK) {
    LOGE("Error starting stream. %s", AAudio_convertResultToText(result));
  }
}

void AudioEngine::stopStream(AAudioStream* stream){

  if (stream != nullptr){
    aaudio_result_t result = AAudioStream_requestStop(stream);
    if (result != AAUDIO_OK){
      LOGE("Error stopping stream. %s", AAudio_convertResultToText(result));
    }
  }
}

void AudioEngine::closeStream(AAudioStream *stream){

  if (stream != nullptr){
    aaudio_result_t result = AAudioStream_close(stream);
    if (result != AAUDIO_OK){
      LOGE("Error closing stream. %s", AAudio_convertResultToText(result));
    }
  }
}

aaudio_data_callback_result_t AudioEngine::dataCallback(AAudioStream *stream,
                                                        void *audioData,
                                                        int32_t numFrames) {
  if (isEchoOn_){
    // Tuning the buffer size for low latency...
    int32_t underRun = AAudioStream_getXRunCount(playStream_);
    if (underRun > playStreamUnderrunCount_) {
      /* Underrun happened since last callback:
       * try to increase the buffer size.
       */
      playStreamUnderrunCount_ = underRun;

      aaudio_result_t actSize = AAudioStream_setBufferSizeInFrames(
          stream, bufSizeInFrames_ + framesPerBurst_);
      if (actSize > 0) {
        bufSizeInFrames_ = actSize;
        LOGI("Buffer size increased to %d frames due to underruns", bufSizeInFrames_);
      } else {
        LOGE("***** Output stream buffer tuning error: %s",
             AAudio_convertResultToText(actSize));
      }
    }

    int32_t samplesPerFrame = sampleChannels_;
    // frameCount could be
    //    < 0 : error code
    //    >= 0 : actual value read from stream

    aaudio_result_t frameCount = 0;

    if (recordingStream_ != nullptr) {
      frameCount = AAudioStream_read(recordingStream_, audioData, numFrames,
                                     static_cast<int64_t>(0));
      if (frameCount < 0) {
        LOGE("****AAudioStream_read() returns %s",
             AAudio_convertResultToText(frameCount));
        frameCount = 0;  // continue to play silent audio
      }
    }

    /**
    * If there's not enough audio data from input stream, fill the rest of buffer with
    * 0 (silence) and continue to loop
    */
    numFrames -= frameCount;
    if (numFrames > 0) {
      memset(static_cast<int16_t *>(audioData) + frameCount * samplesPerFrame,
             0, sizeof(int16_t) * numFrames * samplesPerFrame);
    }
    return AAUDIO_CALLBACK_RESULT_CONTINUE;

  } else {
    return AAUDIO_CALLBACK_RESULT_STOP;
  }
}

void AudioEngine::errorCallback(AAudioStream *stream,
                                aaudio_result_t __unused error){

  aaudio_stream_state_t streamState = AAudioStream_getState(stream);
  if (streamState == AAUDIO_STREAM_STATE_DISCONNECTED){

    // Handle stream restart on a separate thread
    std::function<void(void)> restartStreams = std::bind(&AudioEngine::restartStreams, this);
    streamRestartThread_ = new std::thread(restartStreams);
  }
}

void AudioEngine::restartStreams(){

  LOGI("Restarting streams");

  if (restartingLock_.try_lock()){
    stopStreams();
    startStreams();
    restartingLock_.unlock();
  } else {
    LOGW("Restart stream operation already in progress - ignoring this request");
    // We were unable to obtain the restarting lock which means the restart operation is currently
    // active. This is probably because we received successive "stream disconnected" events.
    // Internal issue b/63087953
  }
}
