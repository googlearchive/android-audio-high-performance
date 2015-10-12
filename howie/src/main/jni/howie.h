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
 *
 */
#pragma once
#ifndef HOWIE_H
#define HOWIE_H


#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif // CPLUSPLUS

typedef enum HowieError_t {
  HOWIE_SUCCESS = 0,
  HOWIE_ERROR_UNKNOWN,
  HOWIE_ERROR_INVALID_PARAMETER,
  HOWIE_ERROR_NULL,
  HOWIE_ERROR_IO,
  HOWIE_ERROR_INVALID_OBJECT,
  HOWIE_ERROR_ENGINE_NOT_INITIALIZED,
  HOWIE_ERROR_AGAIN, // the current operation would block
} HowieError;
#define HOWIE_SUCCEEDED(result) (result == HOWIE_SUCCESS)

typedef enum HowieStreamDirection_t {
  HOWIE_STREAM_DIRECTION_RECORD = 0x01,
  HOWIE_STREAM_DIRECTION_PLAYBACK = 0x02,
  HOWIE_STREAM_DIRECTION_BOTH = 0x03,
} HowieDirection;

typedef enum HowieStreamState_t {
  HOWIE_STREAM_STATE_STOPPED = 0,
  HOWIE_STREAM_STATE_PLAYING
} HowieStreamState;

typedef struct AudioDeviceCharacteristics_t {
  size_t version;

  // The sample rate of each individual stream, in Hz.
  int sampleRate;

  // The number of significant bits per sample, excluding
  // padding
  int bitsPerSample;

  // The total number of bytes per sample. This number includes
  // padding, if any.
  int bytesPerSample;

  // Identifies the significant bits in the sample. For instance,
  // the mask for 24 bit audio in 32 bit samples, left justified,
  // would be 0x00ffffff.
  int sampleMask;

  // TRUE if samples should be interpreted as IEEE floating point
  // numbers. FALSE if samples should be interpreted as signed
  // twos-complement integer values.
  bool floatingPoint;

  // The number of channels supported by this device
  int channelCount;

  // The number of samples in each complete frame. For interleaved
  // multichannel audio, this will equal the number of channels.
  int samplesPerFrame;

  // The expected number of frames that will be presented to the
  // process callback each time it is called. This number is not a
  // guarantee; variations in timing may cause a greater or lesser
  // number of frames to be presented in some intervals.
  int framesPerPeriod;
} HowieDeviceCharacteristics;

typedef struct HowieStream_t {
  size_t version;
} HowieStream;


typedef struct HowieBuffer_t {
  size_t version;
  unsigned char* data;
  size_t byteCount;
} HowieBuffer;

// Called by the Howie system when the device a stream is assigned to
// is changed. This callback will always be called at least once per
// stream, and the first call to this callback will always occur before
// the first call to the process callback.
//
// This call runs on a user thread. It is safe to call blocking operations,
// including memory allocation/deallocation, within this function.
typedef HowieError (*HowieDeviceChangedCallback)(
    const HowieDeviceCharacteristics *,
    const HowieBuffer *state,
    const HowieBuffer *params);

// Called by the Howie system to allow the application to process samples
// for the specified stream. The application is responsible for processing
// the entire buffer, including inserting silence if necessary.
//
// This call runs on the audio thread. It is NOT safe to call blocking
// operations within this function. Examples of unsafe operations include,
// but are not limited to:
// * Memory allocation / deallocation
// * File access
// * Locking (including mutex/futex acquisition)
// * JNI calls
// * Access to memory outside of the provided buffers (input, output, state,
//   params)
typedef HowieError (*HowieProcessCallback)(
    const HowieStream *stream,
    const HowieBuffer *input,
    const HowieBuffer *output,
    const HowieBuffer *state,
    const HowieBuffer *params);

// Called by the Howie system when a stream is destroyed, so that any
// dynamically allocated state can be cleaned up.
//
// This call runs on a user thread. It is safe to call blocking operations,
// including memory allocation/deallocation, within this function.
typedef HowieError (*HowieCleanupCallback)(
    const HowieStream *stream,
    const HowieBuffer *state );

// Create a stream
typedef struct HowieStreamCreationParams_ {
  size_t version;
  // Determines which buffers the stream will present for processing.
  // RECORD implies the "input" buffer will be valid, PLAY implies
  // that the "output" buffer will be valid. BOTH implies that both
  // buffers are valid.
  HowieDirection direction;

  // Called when the device is changed.
  HowieDeviceChangedCallback deviceChangedCallback;

  // Called to process incoming and outgoing buffers.
  HowieProcessCallback processCallback;

  // Called when the stream is destroyed
  HowieCleanupCallback cleanupCallback;

  // Size of the memory to be reserved for processing state. This memory
  // is intended to store all user-defined state between calls to
  // the process callback. The stream implementation guarantees that
  // this block of memory can be accessed without locking from the
  // process function. Accessing state outside this block is strongly
  // discouraged, as it can lead to concurrency issues.
  size_t sizeofStateBlock;

  // Size of the memory to be reserved for processing parameters. The stream
  // facilitates thread-safe exchange of parameter data between user threads
  // and the audio thread. Exchanging data between the two threads in any other
  // way is discouraged, as it can lead to concurrency issues.
  size_t sizeofParameterBlock;

  // Initial state that the stream should be in after the creation function
  // returns.
  HowieStreamState initialState;
} HowieStreamCreationParams;

HowieError HowieGetDeviceCharacteristics(HowieDeviceCharacteristics *dest);

HowieError HowieStreamCreate(
    const HowieStreamCreationParams *params,
    HowieStream **out_stream);


// Releases a previously created stream
HowieError HowieStreamDestroy(HowieStream *stream);

HowieError HowieStreamSetState(HowieStream *stream, HowieStreamState newState);
HowieError HowieStreamGetState(HowieStream *stream, HowieStreamState *state);

// Enqueues a parameter block for the next processing cycle. The stream
// guarantees that the parameter block will be available to the process
// callback at the beginning of the next processing cycle. It also guarantees
// thread safety for the parameter block. If the processing cycle is currently
// running, the new parameter block will not become visible to the audio
// thread until the current cycle finishes and a new cycle begins.
HowieError HowieStreamSendParameters(
    HowieStream* stream,
    const void *parameters,
    size_t size,
    int timeoutMs);



#ifdef __cplusplus
} // extern "C"
#endif // CPLUSPLUS

#endif //HOWIE_H
