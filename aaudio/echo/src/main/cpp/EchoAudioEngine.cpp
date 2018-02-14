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

#include <logging_macros.h>
#include <climits>
#include <assert.h>
#include <audio_common.h>
#include "EchoAudioEngine.h"


/**
 * Every time the playback stream requires data this method will be called.
 *
 * @param stream the audio stream which is requesting data, this is the playStream_ object
 * @param userData the context in which the function is being called, in this case it will be the
 * EchoAudioEngine instance
 * @param audioData an empty buffer into which we can write our audio data
 * @param numFrames the number of audio frames which are required
 * @return Either AAUDIO_CALLBACK_RESULT_CONTINUE if the stream should continue requesting data
 * or AAUDIO_CALLBACK_RESULT_STOP if the stream should stop.
 *
 * @see EchoAudioEngine#dataCallback
 */
aaudio_data_callback_result_t dataCallback(AAudioStream *stream, void *userData,
                                           void *audioData, int32_t numFrames) {
  assert(userData && audioData);
  EchoAudioEngine *audioEngine = reinterpret_cast<EchoAudioEngine *>(userData);
  return audioEngine->dataCallback(stream, audioData, numFrames);
}

/**
 * If there is an error with a stream this function will be called. A common example of an error
 * is when an audio device (such as headphones) is disconnected. In this case you should not
 * restart the stream within the callback, instead use a separate thread to perform the stream
 * recreation and restart.
 *
 * @param stream the stream with the error
 * @param userData the context in which the function is being called, in this case it will be the
 * EchoAudioEngine instance
 * @param error the error which occured, a human readable string can be obtained using
 * AAudio_convertResultToText(error);
 *
 * @see EchoAudioEngine#errorCallback
 */
void errorCallback(AAudioStream *stream,
                   void *userData,
                   aaudio_result_t error) {
  assert(userData);
  EchoAudioEngine *audioEngine = reinterpret_cast<EchoAudioEngine *>(userData);
  audioEngine->errorCallback(stream, error);
}

EchoAudioEngine::~EchoAudioEngine() {
  stopStream(playStream_);
  stopStream(recordingStream_);
  closeStream(playStream_);
  closeStream(recordingStream_);
}

void EchoAudioEngine::setRecordingDeviceId(int32_t deviceId) {

  recordingDeviceId_ = deviceId;
}

void EchoAudioEngine::setPlaybackDeviceId(int32_t deviceId) {

  playbackDeviceId_ = deviceId;
}

void EchoAudioEngine::setEchoOn(bool isEchoOn) {

  if (isEchoOn != isEchoOn_) {
    isEchoOn_ = isEchoOn;

    if (isEchoOn) {
      openAllStreams();
    } else {
      closeAllStreams();
    }
  }
}

void EchoAudioEngine::openAllStreams() {

  // Note: The order of stream creation is important. We create the playback stream first,
  // then use properties from the playback stream (e.g. sample rate) to create the
  // recording stream. By matching the properties we should get the lowest latency path
  openPlaybackStream();
  openRecordingStream();

  // Now start the recording stream first so that we can read from it during the playback
  // stream's dataCallback
  if (recordingStream_ != nullptr && playStream_ != nullptr) {
    startStream(recordingStream_);
    startStream(playStream_);
  } else {
    LOGE("Failed to create recording and/or playback stream");
  }
}

/**
 * Stops and closes the playback and recording streams.
 */
void EchoAudioEngine::closeAllStreams() {

 /**
  * Note: The order of events is important here.
  * The playback stream must be closed before the recording stream. If the recording stream were to
  * be closed first the playback stream's callback may attempt to read from the recording stream
  * which would cause the app to crash since the recording stream would be null.
  */

  if (playStream_ != nullptr) {
    closeStream(playStream_); // Calling close will also stop the stream
    playStream_ = nullptr;
  }

  if (recordingStream_ != nullptr) {
    closeStream(recordingStream_);
    recordingStream_ = nullptr;
  }
}

/**
 * Creates a stream builder which can be used to construct streams
 * @return a new stream builder object
 */
AAudioStreamBuilder *EchoAudioEngine::createStreamBuilder() {

  AAudioStreamBuilder *builder = nullptr;
  aaudio_result_t result = AAudio_createStreamBuilder(&builder);
  if (result != AAUDIO_OK) {
    LOGE("Error creating stream builder: %s", AAudio_convertResultToText(result));
  }
  return builder;
}

/**
 * Creates an audio stream for recording. The audio device used will depend on recordingDeviceId_.
 * If the value is set to AAUDIO_UNSPECIFIED then the default recording device will be used.
 */
void EchoAudioEngine::openRecordingStream() {

  // To create a stream we use a stream builder. This allows us to specify all the parameters
  // for the stream prior to opening it
  AAudioStreamBuilder *builder = createStreamBuilder();

  if (builder != nullptr) {
    setupRecordingStreamParameters(builder);

    // Now that the parameters are set up we can open the stream
    aaudio_result_t result = AAudioStreamBuilder_openStream(builder, &recordingStream_);
    if (result == AAUDIO_OK && recordingStream_ != nullptr) {
      warnIfNotLowLatency(recordingStream_);
      PrintAudioStreamInfo(recordingStream_);
    } else {
      LOGE("Failed to create recording stream. Error: %s", AAudio_convertResultToText(result));
    }
    AAudioStreamBuilder_delete(builder);
  } else {
    LOGE("Unable to obtain an AAudioStreamBuilder object");
  }
}

/**
 * Creates an audio stream for playback. The audio device used will depend on playbackDeviceId_.
 * If the value is set to AAUDIO_UNSPECIFIED then the default playback device will be used.
 */
void EchoAudioEngine::openPlaybackStream() {

  AAudioStreamBuilder *builder = createStreamBuilder();

  if (builder != nullptr) {
    setupPlaybackStreamParameters(builder);
    aaudio_result_t result = AAudioStreamBuilder_openStream(builder, &playStream_);
    if (result == AAUDIO_OK && playStream_ != nullptr) {

      sampleRate_ = AAudioStream_getSampleRate(playStream_);
      framesPerBurst_ = AAudioStream_getFramesPerBurst(playStream_);

      warnIfNotLowLatency(playStream_);
      
      // Set the buffer size to the burst size - this will give us the minimum possible latency
      AAudioStream_setBufferSizeInFrames(playStream_, framesPerBurst_);
      PrintAudioStreamInfo(playStream_);

    } else {
      LOGE("Failed to create playback stream. Error: %s", AAudio_convertResultToText(result));
    }
    AAudioStreamBuilder_delete(builder);
  } else {
    LOGE("Unable to obtain an AAudioStreamBuilder object");
  }
}

/**
 * Sets the stream parameters which are specific to recording, including the sample rate which
 * is determined from the playback stream.
 * @param builder The recording stream builder
 */
void EchoAudioEngine::setupRecordingStreamParameters(AAudioStreamBuilder *builder) {
  AAudioStreamBuilder_setDeviceId(builder, recordingDeviceId_);
  AAudioStreamBuilder_setDirection(builder, AAUDIO_DIRECTION_INPUT);
  AAudioStreamBuilder_setSampleRate(builder, sampleRate_);
  AAudioStreamBuilder_setChannelCount(builder, inputChannelCount_);
  setupCommonStreamParameters(builder);
}

/**
 * Sets the stream parameters which are specific to playback, including device id and the
 * dataCallback function, which must be set for low latency playback.
 * @param builder The playback stream builder
 */
void EchoAudioEngine::setupPlaybackStreamParameters(AAudioStreamBuilder *builder) {

  AAudioStreamBuilder_setDeviceId(builder, playbackDeviceId_);
  AAudioStreamBuilder_setDirection(builder, AAUDIO_DIRECTION_OUTPUT);
  AAudioStreamBuilder_setChannelCount(builder, outputChannelCount_);

  // The :: here indicates that the function is in the global namespace
  // i.e. *not* EchoAudioEngine::dataCallback, but dataCallback defined at the top of this class
  AAudioStreamBuilder_setDataCallback(builder, ::dataCallback, this);
  setupCommonStreamParameters(builder);
}

/**
 * Set the stream parameters which are common to both recording and playback streams.
 * @param builder The playback or recording stream builder
 */
void EchoAudioEngine::setupCommonStreamParameters(AAudioStreamBuilder *builder) {
  AAudioStreamBuilder_setFormat(builder, format_);
  // We request EXCLUSIVE mode since this will give us the lowest possible latency.
  // If EXCLUSIVE mode isn't available the builder will fall back to SHARED mode.
  AAudioStreamBuilder_setSharingMode(builder, AAUDIO_SHARING_MODE_EXCLUSIVE);
  AAudioStreamBuilder_setPerformanceMode(builder, AAUDIO_PERFORMANCE_MODE_LOW_LATENCY);
  AAudioStreamBuilder_setErrorCallback(builder, ::errorCallback, this);
}

void EchoAudioEngine::startStream(AAudioStream *stream) {

  aaudio_result_t result = AAudioStream_requestStart(stream);
  if (result != AAUDIO_OK) {
    LOGE("Error starting stream. %s", AAudio_convertResultToText(result));
  }
}

void EchoAudioEngine::stopStream(AAudioStream *stream) {

  if (stream != nullptr) {
    aaudio_result_t result = AAudioStream_requestStop(stream);
    if (result != AAUDIO_OK) {
      LOGE("Error stopping stream. %s", AAudio_convertResultToText(result));
    }
  }
}

/**
 * Close the stream. After the stream is closed it is deleted and subesequent AAudioStream_* calls
 * will return an error. AAudioStream_close() also checks and waits for any outstanding dataCallback
 * calls to complete before closing the stream. This means the application does not need to add
 * synchronization between the dataCallback function and the thread calling AAudioStream_close()
 * [the closing thread is the UI thread in this sample].
 * @param stream the stream to close
 */
void EchoAudioEngine::closeStream(AAudioStream *stream) {

  if (stream != nullptr) {
    aaudio_result_t result = AAudioStream_close(stream);
    if (result != AAUDIO_OK) {
      LOGE("Error closing stream. %s", AAudio_convertResultToText(result));
    }
  }
}

/**
 * @see the C method dataCallback at the top of this file
 */
aaudio_data_callback_result_t EchoAudioEngine::dataCallback(AAudioStream *stream,
                                                            void *audioData,
                                                            int32_t numFrames) {
  if (isEchoOn_) {

    // frameCount could be
    //    < 0 : error code
    //    >= 0 : actual value read from stream
    aaudio_result_t frameCount = 0;

    if (recordingStream_ != nullptr) {

      // If this is the first data callback we want to drain the recording buffer so we're getting
      // the most up to date data
      if (isFirstDataCallback_) {
        drainRecordingStream(audioData, numFrames);
        isFirstDataCallback_ = false;
      }

      frameCount = AAudioStream_read(recordingStream_, audioData, numFrames,
                                     static_cast<int64_t>(0));

      ConvertMonoToStereo(static_cast<int16_t *>(audioData), frameCount);

      audioEffect_.process(static_cast<int16_t *>(audioData), outputChannelCount_, frameCount);

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
      memset(static_cast<int16_t *>(audioData) + frameCount * outputChannelCount_,
             0, sizeof(int16_t) * numFrames * outputChannelCount_);
    }
    return AAUDIO_CALLBACK_RESULT_CONTINUE;

  } else {
    return AAUDIO_CALLBACK_RESULT_STOP;
  }
}

/**
 * Drain the recording stream of any existing data by reading from it until it's empty. This is
 * usually run to clear out any stale data before performing an actual read operation, thereby
 * obtaining the most recently recorded data and the best possbile recording latency.
 *
 * @param audioData A buffer which the existing data can be read into
 * @param numFrames The number of frames to read in a single read operation, this is typically the
 * size of `audioData`.
 */
void EchoAudioEngine::drainRecordingStream(void *audioData, int32_t numFrames) {

  aaudio_result_t clearedFrames = 0;
  do {
    clearedFrames = AAudioStream_read(recordingStream_, audioData, numFrames, 0);
  } while (clearedFrames > 0);
}

/**
 * See the C method errorCallback at the top of this file
 */
void EchoAudioEngine::errorCallback(AAudioStream *stream,
                                    aaudio_result_t error) {

  LOGI("errorCallback has result: %s", AAudio_convertResultToText(error));
  aaudio_stream_state_t streamState = AAudioStream_getState(stream);
  if (streamState == AAUDIO_STREAM_STATE_DISCONNECTED) {

    // Handle stream restart on a separate thread
    std::function<void(void)> restartStreams = std::bind(&EchoAudioEngine::restartStreams, this);
    std::thread streamRestartThread(restartStreams);
    streamRestartThread.detach();
  }
}

/**
 * Restart the streams. During the restart operation subsequent calls to this method will output
 * a warning.
 */
void EchoAudioEngine::restartStreams() {

  LOGI("Restarting streams");

  if (restartingLock_.try_lock()) {
    closeAllStreams();
    openAllStreams();
    restartingLock_.unlock();
  } else {
    LOGW("Restart stream operation already in progress - ignoring this request");
    // We were unable to obtain the restarting lock which means the restart operation is currently
    // active. This is probably because we received successive "stream disconnected" events.
    // Internal issue b/63087953
  }
}

void EchoAudioEngine::warnIfNotLowLatency(AAudioStream *stream) {

  if (AAudioStream_getPerformanceMode(stream) != AAUDIO_PERFORMANCE_MODE_LOW_LATENCY){
    LOGW("Stream is NOT low latency. Check your requested format, sample rate and channel count");
  }
}
