//
// Created by ilewis on 9/25/15.
//
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
} HowieError;
#define HOWIE_SUCCEEDED(result) (result == HOWIE_SUCCESS)

typedef enum HowieDirection_t {
  HOWIE_DIRECTION_RECORD,
  HOWIE_DIRECTION_PLAYBACK
} HowieDirection;

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
typedef HowieError (*HowieDeviceChangedCallback)(
    const HowieDeviceCharacteristics *);

// Called by the Howie system to allow the application to process samples
// for the specified stream. The application is responsible for processing
// the entire buffer, including inserting silence if necessary.
typedef HowieError (*HowieProcessCallback)(
    HowieStream *stream,
    const HowieBuffer *input,
    const HowieBuffer *output);

// Creates a stream
 HowieError HowieCreateStream(
    HowieDirection direction,
    HowieDeviceChangedCallback deviceChangedCallback,
    HowieProcessCallback processCallback,
    HowieStream **out_stream );

// Releases a previously created stream
HowieError HowieDestroyStream(HowieStream *stream);

#ifdef __cplusplus
} // extern "C"
#endif // CPLUSPLUS

#endif //HOWIE_H
