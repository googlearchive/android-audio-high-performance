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

#include <dlfcn.h>
#include <assert.h>

#include <include/AAudio_Wrapper.h>

AAUDIO_API aaudio_result_t (*AAudio_createStreamBuilder)(AAudioStreamBuilder** builder);
const char * (*AAudio_convertResultToText)(aaudio_result_t returnCode);
const char * (*AAudio_convertStreamStateToText)(aaudio_stream_state_t state);
// ============================================================
// StreamBuilder
// ============================================================

aaudio_result_t (*AAudio_createStreamBuilder )(AAudioStreamBuilder** builder);
void (*AAudioStreamBuilder_setDeviceId )(AAudioStreamBuilder* builder,
                                     int32_t deviceId);

void (*AAudioStreamBuilder_setSampleRate )(AAudioStreamBuilder* builder,
                                       int32_t sampleRate);
void (*AAudioStreamBuilder_setChannelCount )(AAudioStreamBuilder* builder,
                                            int32_t channelCount);
void (*AAudioStreamBuilder_setFormat )(AAudioStreamBuilder* builder,
                                   aaudio_format_t format);
void (*AAudioStreamBuilder_setSharingMode )(AAudioStreamBuilder* builder,
                                        aaudio_sharing_mode_t sharingMode);
void (*AAudioStreamBuilder_setDirection )(AAudioStreamBuilder* builder,
                                      aaudio_direction_t direction);
void (*AAudioStreamBuilder_setBufferCapacityInFrames )(AAudioStreamBuilder* builder,
                                                   int32_t frames);

void (*AAudioStreamBuilder_setDataCallback)(AAudioStreamBuilder* builder,
                                            AAudioStream_dataCallback callback,
                                            void *userData);
void (*AAudioStreamBuilder_setFramesPerDataCallback)(AAudioStreamBuilder* builder,
                                                     int32_t numFrames);
void (*AAudioStreamBuilder_setErrorCallback)(AAudioStreamBuilder* builder,
                                                        AAudioStream_errorCallback callback,
                                                        void *userData);
aaudio_result_t  (*AAudioStreamBuilder_openStream )(AAudioStreamBuilder* builder,
                                                AAudioStream** stream);
aaudio_result_t  (*AAudioStreamBuilder_delete )(AAudioStreamBuilder* builder);
// ============================================================
// Stream Control
// ============================================================
aaudio_result_t  (*AAudioStream_close )(AAudioStream* stream);
aaudio_result_t  (*AAudioStream_requestStart )(AAudioStream* stream);
aaudio_result_t  (*AAudioStream_requestPause )(AAudioStream* stream);

aaudio_result_t  (*AAudioStream_requestFlush )(AAudioStream* stream);
aaudio_result_t  (*AAudioStream_requestStop )(AAudioStream* stream);

aaudio_stream_state_t (*AAudioStream_getState )(AAudioStream* stream);

aaudio_result_t (*AAudioStream_waitForStateChange )(AAudioStream* stream,
                                                aaudio_stream_state_t inputState,
                                                aaudio_stream_state_t *nextState,
                                                int64_t timeoutNanoseconds);
// ============================================================
// Stream I/O
// ============================================================

aaudio_result_t (*AAudioStream_read )(AAudioStream* stream,
                                  void *buffer,
                                  int32_t numFrames,
                                  int64_t timeoutNanoseconds);

aaudio_result_t (*AAudioStream_write )(AAudioStream* stream,
                                   const void *buffer,
                                   int32_t numFrames,
                                   int64_t timeoutNanoseconds);

// ============================================================
// Stream - queries
// ============================================================

aaudio_result_t (*AAudioStream_setBufferSizeInFrames )(AAudioStream* stream,
                                                   int32_t requestedFrames);
int32_t (*AAudioStream_getBufferSizeInFrames )(AAudioStream* stream);

int32_t (*AAudioStream_getFramesPerBurst )(AAudioStream* stream);
int32_t (*AAudioStream_getBufferCapacityInFrames )(AAudioStream* stream);
int32_t (*AAudioStream_getFramesPerDataCallback)(AAudioStream* stream);
int32_t (*AAudioStream_getXRunCount )(AAudioStream* stream);
int32_t (*AAudioStream_getSampleRate )(AAudioStream* stream);
int32_t (*AAudioStream_getChannelCount )(AAudioStream* stream);
int32_t (*AAudioStream_getDeviceId )(AAudioStream* stream);
aaudio_format_t (*AAudioStream_getFormat )(AAudioStream* stream);
aaudio_sharing_mode_t (*AAudioStream_getSharingMode )(AAudioStream* stream);
aaudio_direction_t (*AAudioStream_getDirection )(AAudioStream* stream);

int64_t (*AAudioStream_getFramesWritten )(AAudioStream* stream);

int64_t (*AAudioStream_getFramesRead )(AAudioStream* stream);

aaudio_result_t (*AAudioStream_getTimestamp )(AAudioStream* stream,
                                          clockid_t clockid,
                                          int64_t *framePosition,
                                          int64_t *timeNanoseconds);

int32_t InitAAudio(void) {

  //Load the guessed audio lib file...
  void* handle = dlopen("libaaudio.so", RTLD_NOW);
  assert(handle);

  #define GET_PROC(s)  s = dlsym(handle, #s)

  GET_PROC(AAudio_createStreamBuilder);
  GET_PROC(AAudio_convertResultToText);
  GET_PROC(AAudio_convertStreamStateToText);
  GET_PROC(AAudio_createStreamBuilder);
  GET_PROC(AAudioStreamBuilder_setDeviceId);
  GET_PROC(AAudioStreamBuilder_setSampleRate);
  GET_PROC(AAudioStreamBuilder_setChannelCount);
  GET_PROC(AAudioStreamBuilder_setFormat);
  GET_PROC(AAudioStreamBuilder_setSharingMode);
  GET_PROC(AAudioStreamBuilder_setDirection);
  GET_PROC(AAudioStreamBuilder_setBufferCapacityInFrames);
  GET_PROC(AAudioStreamBuilder_setDataCallback);
  GET_PROC(AAudioStreamBuilder_setFramesPerDataCallback);
  GET_PROC(AAudioStreamBuilder_setErrorCallback);
  GET_PROC(AAudioStreamBuilder_openStream);
  GET_PROC(AAudioStreamBuilder_delete);
  GET_PROC(AAudioStream_close);
  GET_PROC(AAudioStream_requestStart);
  GET_PROC(AAudioStream_requestPause);

  GET_PROC(AAudioStream_requestFlush);
  GET_PROC(AAudioStream_requestStop);
  GET_PROC(AAudioStream_getState);

  GET_PROC(AAudioStream_waitForStateChange);
  GET_PROC(AAudioStream_read);
  GET_PROC(AAudioStream_write);

  GET_PROC(AAudioStream_setBufferSizeInFrames);
  GET_PROC(AAudioStream_getBufferSizeInFrames);

  GET_PROC(AAudioStream_getFramesPerBurst);
  GET_PROC(AAudioStream_getBufferCapacityInFrames);

  GET_PROC(AAudioStream_getFramesPerDataCallback);

  GET_PROC(AAudioStream_getXRunCount);
  GET_PROC(AAudioStream_getSampleRate);
  GET_PROC(AAudioStream_getChannelCount);
  GET_PROC(AAudioStream_getDeviceId);
  GET_PROC(AAudioStream_getFormat);
  GET_PROC(AAudioStream_getSharingMode);
  GET_PROC(AAudioStream_getDirection);

  GET_PROC(AAudioStream_getFramesWritten);
  GET_PROC(AAudioStream_getFramesRead);

  GET_PROC(AAudioStream_getTimestamp);
  
  if (AAudio_createStreamBuilder  &&
      AAudio_convertResultToText  &&
      AAudio_convertStreamStateToText  &&
      AAudio_createStreamBuilder &&
      AAudioStreamBuilder_setDeviceId &&
      AAudioStreamBuilder_setSampleRate &&
      AAudioStreamBuilder_setChannelCount &&
      AAudioStreamBuilder_setFormat &&
      AAudioStreamBuilder_setSharingMode &&
      AAudioStreamBuilder_setDirection &&
      AAudioStreamBuilder_setBufferCapacityInFrames &&
      AAudioStreamBuilder_setDataCallback &&
      AAudioStreamBuilder_setFramesPerDataCallback &&
      AAudioStreamBuilder_setErrorCallback &&
      AAudioStreamBuilder_openStream &&
      AAudioStreamBuilder_delete &&
      AAudioStream_close &&
      AAudioStream_requestStart &&
      AAudioStream_requestPause &&
      AAudioStream_requestFlush &&
      AAudioStream_requestStop &&
      AAudioStream_getState &&
      AAudioStream_waitForStateChange &&
      AAudioStream_read &&
      AAudioStream_write &&
      AAudioStream_setBufferSizeInFrames &&
      AAudioStream_getBufferSizeInFrames &&
      AAudioStream_getFramesPerBurst &&
      AAudioStream_getBufferCapacityInFrames &&
      AAudioStream_getFramesPerDataCallback &&
      AAudioStream_getXRunCount &&
      AAudioStream_getSampleRate &&
      AAudioStream_getChannelCount &&
      AAudioStream_getDeviceId &&
      AAudioStream_getFormat &&
      AAudioStream_getSharingMode &&
      AAudioStream_getDirection &&
      AAudioStream_getFramesWritten &&
      AAudioStream_getFramesRead &&
      AAudioStream_getTimestamp) {
    return 1;
  }
  return 0;
}
