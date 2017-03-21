/*
 * Copyright (C) 2016 The Android Open Source Project
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

/**
 * This is the 'C' ABI for AAudio.
 */
#ifndef AAUDIO_AAUDIO_WRAPPER_H
#define AAUDIO_AAUDIO_WRAPPER_H

#include <time.h>
#include "AAudioDefinitions.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AAudioStreamStruct         AAudioStream;
typedef struct AAudioStreamBuilderStruct  AAudioStreamBuilder;

#ifndef AAUDIO_API
#define AAUDIO_API /* export this symbol */
#endif

// ============================================================
// Audio System
// ============================================================

/**
 * The text is the ASCII symbol corresponding to the returnCode,
 * or an English message saying the returnCode is unrecognized.
 * This is intended for developers to use when debugging.
 * It is not for display to users.
 *
 * @return pointer to a text representation of an AAudio result code.
 */
extern AAUDIO_API const char * (*AAudio_convertResultToText)(aaudio_result_t returnCode);

/**
 * The text is the ASCII symbol corresponding to the stream state,
 * or an English message saying the state is unrecognized.
 * This is intended for developers to use when debugging.
 * It is not for display to users.
 *
 * @return pointer to a text representation of an AAudio state.
 */
extern AAUDIO_API const char * (*AAudio_convertStreamStateToText)(aaudio_stream_state_t state);

// ============================================================
// StreamBuilder
// ============================================================

/**
 * Create a StreamBuilder that can be used to open a Stream.
 *
 * The deviceId is initially unspecified, meaning that the current default device will be used.
 *
 * The default direction is AAUDIO_DIRECTION_OUTPUT.
 * The default sharing mode is AAUDIO_SHARING_MODE_SHARED.
 * The data format, samplesPerFrames and sampleRate are unspecified and will be
 * chosen by the device when it is opened.
 *
 * AAudioStreamBuilder_delete() must be called when you are done using the builder.
 */
extern AAUDIO_API aaudio_result_t (*AAudio_createStreamBuilder)(AAudioStreamBuilder** builder);

/**
 * Request an audio device identified device using an ID.
 * On Android, for example, the ID could be obtained from the Java AudioManager.
 *
 * By default, the primary device will be used.
 *
 * @param builder reference provided by AAudio_createStreamBuilder()
 * @param deviceId device identifier or AAUDIO_DEVICE_UNSPECIFIED
 */
extern AAUDIO_API void (*AAudioStreamBuilder_setDeviceId)(AAudioStreamBuilder* builder,
                                                     int32_t deviceId);

/**
 * Request a sample rate in Hz.
 * The stream may be opened with a different sample rate.
 * So the application should query for the actual rate after the stream is opened.
 *
 * Technically, this should be called the "frame rate" or "frames per second",
 * because it refers to the number of complete frames transferred per second.
 * But it is traditionally called "sample rate". Se we use that term.
 *
 * Default is AAUDIO_UNSPECIFIED.

 */
extern AAUDIO_API void (*AAudioStreamBuilder_setSampleRate)(AAudioStreamBuilder* builder,
                                                       int32_t sampleRate);

/**
 * Request a number of samples per frame.
 * The stream may be opened with a different value.
 * So the application should query for the actual value after the stream is opened.
 *
 * Default is AAUDIO_UNSPECIFIED.
 *
 * Note, this quantity is sometimes referred to as "channel count".
 */
extern AAUDIO_API void (*AAudioStreamBuilder_setSamplesPerFrame)(AAudioStreamBuilder* builder,
                                                   int32_t samplesPerFrame);

/**
 * Request a sample data format, for example AAUDIO_FORMAT_PCM_I16.
 * The application should query for the actual format after the stream is opened.
 */
extern AAUDIO_API void (*AAudioStreamBuilder_setFormat)(AAudioStreamBuilder* builder,
                                                   aaudio_audio_format_t format);

/**
 * Request a mode for sharing the device.
 * The requested sharing mode may not be available.
 * So the application should query for the actual mode after the stream is opened.
 *
 * @param builder reference provided by AAudio_createStreamBuilder()
 * @param sharingMode AAUDIO_SHARING_MODE_LEGACY or AAUDIO_SHARING_MODE_EXCLUSIVE
 */
extern AAUDIO_API void (*AAudioStreamBuilder_setSharingMode)(AAudioStreamBuilder* builder,
                                                        aaudio_sharing_mode_t sharingMode);

/**
 * Request the direction for a stream. The default is AAUDIO_DIRECTION_OUTPUT.
 *
 * @param builder reference provided by AAudio_createStreamBuilder()
 * @param direction AAUDIO_DIRECTION_OUTPUT or AAUDIO_DIRECTION_INPUT
 */
extern AAUDIO_API void (*AAudioStreamBuilder_setDirection)(AAudioStreamBuilder* builder,
                                                            aaudio_direction_t direction);

/**
 * Set the requested maximum buffer capacity in frames.
 * The final AAudioStream capacity may differ, but will probably be at least this big.
 *
 * Default is AAUDIO_UNSPECIFIED.
 *
 * @param builder reference provided by AAudio_createStreamBuilder()
 * @param frames the desired buffer capacity in frames or AAUDIO_UNSPECIFIED
 */
extern AAUDIO_API void (*AAudioStreamBuilder_setBufferCapacityInFrames)(AAudioStreamBuilder* builder,
                                                                 int32_t frames);

/**
 * Open a stream based on the options in the StreamBuilder.
 *
 * AAudioStream_close must be called when finished with the stream to recover
 * the memory and to free the associated resources.
 *
 * @param builder reference provided by AAudio_createStreamBuilder()
 * @param stream pointer to a variable to receive the new stream reference
 * @return AAUDIO_OK or a negative error.
 */
extern AAUDIO_API aaudio_result_t  (*AAudioStreamBuilder_openStream)(AAudioStreamBuilder* builder,
                                                     AAudioStream** stream);

/**
 * Delete the resources associated with the StreamBuilder.
 *
 * @param builder reference provided by AAudio_createStreamBuilder()
 * @return AAUDIO_OK or a negative error.
 */
extern AAUDIO_API aaudio_result_t  (*AAudioStreamBuilder_delete)(AAudioStreamBuilder* builder);

// ============================================================
// Stream Control
// ============================================================

/**
 * Free the resources associated with a stream created by AAudioStreamBuilder_openStream()
 *
 * @param stream reference provided by AAudioStreamBuilder_openStream()
 * @return AAUDIO_OK or a negative error.
 */
extern AAUDIO_API aaudio_result_t  (*AAudioStream_close)(AAudioStream* stream);

/**
 * Asynchronously request to start playing the stream. For output streams, one should
 * write to the stream to fill the buffer before starting.
 * Otherwise it will underflow.
 * After this call the state will be in AAUDIO_STREAM_STATE_STARTING or AAUDIO_STREAM_STATE_STARTED.
 *
 * @param stream reference provided by AAudioStreamBuilder_openStream()
 * @return AAUDIO_OK or a negative error.
 */
extern AAUDIO_API aaudio_result_t  (*AAudioStream_requestStart)(AAudioStream* stream);

/**
 * Asynchronous request for the stream to pause.
 * Pausing a stream will freeze the data flow but not flush any buffers.
 * Use AAudioStream_Start() to resume playback after a pause.
 * After this call the state will be in AAUDIO_STREAM_STATE_PAUSING or AAUDIO_STREAM_STATE_PAUSED.
 *
 * @param stream reference provided by AAudioStreamBuilder_openStream()
 * @return AAUDIO_OK or a negative error.
 */
extern AAUDIO_API aaudio_result_t  (*AAudioStream_requestPause)(AAudioStream* stream);

/**
 * Asynchronous request for the stream to flush.
 * Flushing will discard any pending data.
 * This call only works if the stream is pausing or paused. TODO review
 * Frame counters are not reset by a flush. They may be advanced.
 * After this call the state will be in AAUDIO_STREAM_STATE_FLUSHING or AAUDIO_STREAM_STATE_FLUSHED.
 *
 * @param stream reference provided by AAudioStreamBuilder_openStream()
 * @return AAUDIO_OK or a negative error.
 */
extern AAUDIO_API aaudio_result_t  (*AAudioStream_requestFlush)(AAudioStream* stream);

/**
 * Asynchronous request for the stream to stop.
 * The stream will stop after all of the data currently buffered has been played.
 * After this call the state will be in AAUDIO_STREAM_STATE_STOPPING or AAUDIO_STREAM_STATE_STOPPED.
 *
 * @param stream reference provided by AAudioStreamBuilder_openStream()
 * @return AAUDIO_OK or a negative error.
 */
extern AAUDIO_API aaudio_result_t  (*AAudioStream_requestStop)(AAudioStream* stream);

/**
 * Query the current state of the client, eg. AAUDIO_STREAM_STATE_PAUSING
 *
 * This function will immediately return the state without updating the state.
 * If you want to update the client state based on the server state then
 * call AAudioStream_waitForStateChange() with currentState
 * set to AAUDIO_STREAM_STATE_UNKNOWN and a zero timeout.
 *
 * @param stream reference provided by AAudioStreamBuilder_openStream()
 * @param state pointer to a variable that will be set to the current state
 */
extern AAUDIO_API aaudio_stream_state_t (*AAudioStream_getState)(AAudioStream* stream);

/**
 * Wait until the current state no longer matches the input state.
 *
 * This will update the current client state.
 *
 * <pre><code>
 * aaudio_stream_state_t currentState;
 * aaudio_result_t result = AAudioStream_getState(stream, &currentState);
 * while (result == AAUDIO_OK && currentState != AAUDIO_STREAM_STATE_PAUSING) {
 *     result = AAudioStream_waitForStateChange(
 *                                   stream, currentState, &currentState, MY_TIMEOUT_NANOS);
 * }
 * </code></pre>
 *
 * @param stream A reference provided by AAudioStreamBuilder_openStream()
 * @param inputState The state we want to avoid.
 * @param nextState Pointer to a variable that will be set to the new state.
 * @param timeoutNanoseconds Maximum number of nanoseconds to wait for completion.
 * @return AAUDIO_OK or a negative error.
 */
extern AAUDIO_API aaudio_result_t (*AAudioStream_waitForStateChange)(AAudioStream* stream,
                                            aaudio_stream_state_t inputState,
                                            aaudio_stream_state_t *nextState,
                                            int64_t timeoutNanoseconds);

// ============================================================
// Stream I/O
// ============================================================

/**
 * Read data from the stream.
 *
 * The call will wait until the read is complete or until it runs out of time.
 * If timeoutNanos is zero then this call will not wait.
 *
 * Note that timeoutNanoseconds is a relative duration in wall clock time.
 * Time will not stop if the thread is asleep.
 * So it will be implemented using CLOCK_BOOTTIME.
 *
 * This call is "strong non-blocking" unless it has to wait for data.
 *
 * @param stream A stream created using AAudioStreamBuilder_openStream().
 * @param buffer The address of the first sample.
 * @param numFrames Number of frames to read. Only complete frames will be written.
 * @param timeoutNanoseconds Maximum number of nanoseconds to wait for completion.
 * @return The number of frames actually read or a negative error.
 */
extern AAUDIO_API aaudio_result_t (*AAudioStream_read)(AAudioStream* stream,
                               void *buffer,
                               int32_t numFrames,
                               int64_t timeoutNanoseconds);

/**
 * Write data to the stream.
 *
 * The call will wait until the write is complete or until it runs out of time.
 * If timeoutNanos is zero then this call will not wait.
 *
 * Note that timeoutNanoseconds is a relative duration in wall clock time.
 * Time will not stop if the thread is asleep.
 * So it will be implemented using CLOCK_BOOTTIME.
 *
 * This call is "strong non-blocking" unless it has to wait for room in the buffer.
 *
 * @param stream A stream created using AAudioStreamBuilder_openStream().
 * @param buffer The address of the first sample.
 * @param numFrames Number of frames to write. Only complete frames will be written.
 * @param timeoutNanoseconds Maximum number of nanoseconds to wait for completion.
 * @return The number of frames actually written or a negative error.
 */
extern AAUDIO_API aaudio_result_t (*AAudioStream_write)(AAudioStream* stream,
                               const void *buffer,
                               int32_t numFrames,
                               int64_t timeoutNanoseconds);


// ============================================================
// High priority audio threads
// ============================================================

typedef void *(*aaudio_audio_thread_proc_t)(void *);

/**
 * Create a thread associated with a stream. The thread has special properties for
 * low latency audio performance. This thread can be used to implement a callback API.
 *
 * Only one thread may be associated with a stream.
 *
 * If you are using multiple streams then we recommend that you only do
 * blocking reads or writes on one stream. You can do non-blocking I/O on the
 * other streams by setting the timeout to zero.
 * This thread should be created for the stream that you will block on.
 *
 * Note that this API is in flux.
 *
 * @param stream A stream created using AAudioStreamBuilder_openStream().
 * @param periodNanoseconds the estimated period at which the audio thread will need to wake up
 * @param threadProc your thread entry point
 * @param arg an argument that will be passed to your thread entry point
 * @return AAUDIO_OK or a negative error.
 */
extern AAUDIO_API aaudio_result_t (*AAudioStream_createThread)(AAudioStream* stream,
                                     int64_t periodNanoseconds,
                                     aaudio_audio_thread_proc_t threadProc,
                                     void *arg);

/**
 * Wait until the thread exits or an error occurs.
 *
 * @param stream A stream created using AAudioStreamBuilder_openStream().
 * @param returnArg a pointer to a variable to receive the return value
 * @param timeoutNanoseconds Maximum number of nanoseconds to wait for completion.
 * @return AAUDIO_OK or a negative error.
 */
extern AAUDIO_API aaudio_result_t (*AAudioStream_joinThread)(AAudioStream* stream,
                                   void **returnArg,
                                   int64_t timeoutNanoseconds);

// ============================================================
// Stream - queries
// ============================================================


/**
 * This can be used to adjust the latency of the buffer by changing
 * the threshold where blocking will occur.
 * By combining this with AAudioStream_getXRunCount(), the latency can be tuned
 * at run-time for each device.
 *
 * This cannot be set higher than AAudioStream_getBufferCapacityInFrames().
 *
 * Note that you will probably not get the exact size you request.
 * Call AAudioStream_getBufferSizeInFrames() to see what the actual final size is.
 *
 * @param stream reference provided by AAudioStreamBuilder_openStream()
 * @param requestedFrames requested number of frames that can be filled without blocking
 * @return actual buffer size in frames or a negative error
 */
extern AAUDIO_API aaudio_result_t (*AAudioStream_setBufferSizeInFrames)(AAudioStream* stream,
                                                      int32_t requestedFrames);

/**
 * Query the maximum number of frames that can be filled without blocking.
 *
 * @param stream reference provided by AAudioStreamBuilder_openStream()
 * @return buffer size in frames.
 */
extern AAUDIO_API int32_t (*AAudioStream_getBufferSizeInFrames)(AAudioStream* stream);

/**
 * Query the number of frames that the application should read or write at
 * one time for optimal performance. It is OK if an application writes
 * a different number of frames. But the buffer size may need to be larger
 * in order to avoid underruns or overruns.
 *
 * Note that this may or may not match the actual device burst size.
 * For some endpoints, the burst size can vary dynamically.
 * But these tend to be devices with high latency.
 *
 * @param stream reference provided by AAudioStreamBuilder_openStream()
 * @return burst size
 */
extern AAUDIO_API int32_t (*AAudioStream_getFramesPerBurst)(AAudioStream* stream);

/**
 * Query maximum buffer capacity in frames.
 *
 * @param stream reference provided by AAudioStreamBuilder_openStream()
 * @return  the buffer capacity in frames
 */
extern AAUDIO_API int32_t (*AAudioStream_getBufferCapacityInFrames)(AAudioStream* stream);

/**
 * An XRun is an Underrun or an Overrun.
 * During playing, an underrun will occur if the stream is not written in time
 * and the system runs out of valid data.
 * During recording, an overrun will occur if the stream is not read in time
 * and there is no place to put the incoming data so it is discarded.
 *
 * An underrun or overrun can cause an audible "pop" or "glitch".
 *
 * @param stream reference provided by AAudioStreamBuilder_openStream()
 * @return the underrun or overrun count
 */
extern AAUDIO_API int32_t (*AAudioStream_getXRunCount)(AAudioStream* stream);

/**
 * @param stream reference provided by AAudioStreamBuilder_openStream()
 * @return actual sample rate
 */
extern AAUDIO_API int32_t (*AAudioStream_getSampleRate)(AAudioStream* stream);

/**
 * The samplesPerFrame is also known as channelCount.
 *
 * @param stream reference provided by AAudioStreamBuilder_openStream()
 * @return actual samples per frame
 */
extern AAUDIO_API int32_t (*AAudioStream_getSamplesPerFrame)(AAudioStream* stream);

/**
 * @param stream reference provided by AAudioStreamBuilder_openStream()
 * @return actual device ID
 */
extern AAUDIO_API int32_t (*AAudioStream_getDeviceId)(AAudioStream* stream);

/**
 * @param stream reference provided by AAudioStreamBuilder_openStream()
 * @return actual data format
 */
extern AAUDIO_API aaudio_audio_format_t (*AAudioStream_getFormat)(AAudioStream* stream);

/**
 * Provide actual sharing mode.
 * @param stream reference provided by AAudioStreamBuilder_openStream()
 * @return  actual sharing mode
 */
extern AAUDIO_API aaudio_sharing_mode_t (*AAudioStream_getSharingMode)(AAudioStream* stream);

/**
 * @param stream reference provided by AAudioStreamBuilder_openStream()
 * @return direction
 */
extern AAUDIO_API aaudio_direction_t (*AAudioStream_getDirection)(AAudioStream* stream);

/**
 * Passes back the number of frames that have been written since the stream was created.
 * For an output stream, this will be advanced by the application calling write().
 * For an input stream, this will be advanced by the endpoint.
 *
 * The frame position is monotonically increasing.
 *
 * @param stream reference provided by AAudioStreamBuilder_openStream()
 * @return frames written
 */
extern AAUDIO_API int64_t (*AAudioStream_getFramesWritten)(AAudioStream* stream);

/**
 * Passes back the number of frames that have been read since the stream was created.
 * For an output stream, this will be advanced by the endpoint.
 * For an input stream, this will be advanced by the application calling read().
 *
 * The frame position is monotonically increasing.
 *
 * @param stream reference provided by AAudioStreamBuilder_openStream()
 * @return frames read
 */
extern AAUDIO_API int64_t (*AAudioStream_getFramesRead)(AAudioStream* stream);

/**
 * Passes back the time at which a particular frame was presented.
 * This can be used to synchronize audio with video or MIDI.
 * It can also be used to align a recorded stream with a playback stream.
 *
 * Timestamps are only valid when the stream is in AAUDIO_STREAM_STATE_STARTED.
 * AAUDIO_ERROR_INVALID_STATE will be returned if the stream is not started.
 * Note that because requestStart() is asynchronous, timestamps will not be valid until
 * a short time after calling requestStart().
 * So AAUDIO_ERROR_INVALID_STATE should not be considered a fatal error.
 * Just try calling again later.
 *
 * If an error occurs, then the position and time will not be modified.
 *
 * The position and time passed back are monotonically increasing.
 *
 * @param stream reference provided by AAudioStreamBuilder_openStream()
 * @param clockid AAUDIO_CLOCK_MONOTONIC or AAUDIO_CLOCK_BOOTTIME
 * @param framePosition pointer to a variable to receive the position
 * @param timeNanoseconds pointer to a variable to receive the time
 * @return AAUDIO_OK or a negative error
 */
extern AAUDIO_API aaudio_result_t (*AAudioStream_getTimestamp)(AAudioStream* stream,
                                      clockid_t clockid,
                                      int64_t *framePosition,
                                      int64_t *timeNanoseconds);

int32_t InitAAudio(void);

#ifdef __cplusplus
}
#endif

#endif // AAUDIO_AAUDIO_WRAPPER_H
