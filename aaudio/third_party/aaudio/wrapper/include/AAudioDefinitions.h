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

#ifndef AAUDIO_AAUDIODEFINITIONS_H
#define AAUDIO_AAUDIODEFINITIONS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * This is used to represent a value that has not been specified.
 * For example, an application could use AAUDIO_UNSPECIFIED to indicate
 * that is did not not care what the specific value of a parameter was
 * and would accept whatever it was given.
 */
#define AAUDIO_UNSPECIFIED           0
#define AAUDIO_DEVICE_UNSPECIFIED    ((int32_t) -1)

enum {
    AAUDIO_DIRECTION_OUTPUT,
    AAUDIO_DIRECTION_INPUT
};
typedef int32_t aaudio_direction_t;

enum {
    AAUDIO_FORMAT_INVALID = -1,
    AAUDIO_FORMAT_UNSPECIFIED = 0,
    AAUDIO_FORMAT_PCM_I16,
    AAUDIO_FORMAT_PCM_FLOAT,
    AAUDIO_FORMAT_PCM_I8_24,
    AAUDIO_FORMAT_PCM_I32
};
typedef int32_t aaudio_audio_format_t;

enum {
    AAUDIO_OK,
    AAUDIO_ERROR_BASE = -900, // TODO review
    AAUDIO_ERROR_DISCONNECTED,
    AAUDIO_ERROR_ILLEGAL_ARGUMENT,
    AAUDIO_ERROR_INCOMPATIBLE,
    AAUDIO_ERROR_INTERNAL, // an underlying API returned an error code
    AAUDIO_ERROR_INVALID_STATE,
    AAUDIO_ERROR_UNEXPECTED_STATE,
    AAUDIO_ERROR_UNEXPECTED_VALUE,
    AAUDIO_ERROR_INVALID_HANDLE,
    AAUDIO_ERROR_INVALID_QUERY,
    AAUDIO_ERROR_UNIMPLEMENTED,
    AAUDIO_ERROR_UNAVAILABLE,
    AAUDIO_ERROR_NO_FREE_HANDLES,
    AAUDIO_ERROR_NO_MEMORY,
    AAUDIO_ERROR_NULL,
    AAUDIO_ERROR_TIMEOUT,
    AAUDIO_ERROR_WOULD_BLOCK,
    AAUDIO_ERROR_INVALID_ORDER,
    AAUDIO_ERROR_OUT_OF_RANGE,
    AAUDIO_ERROR_NO_SERVICE
};
typedef int32_t  aaudio_result_t;

enum
{
    AAUDIO_STREAM_STATE_UNINITIALIZED = 0,
    AAUDIO_STREAM_STATE_UNKNOWN,
    AAUDIO_STREAM_STATE_OPEN,
    AAUDIO_STREAM_STATE_STARTING,
    AAUDIO_STREAM_STATE_STARTED,
    AAUDIO_STREAM_STATE_PAUSING,
    AAUDIO_STREAM_STATE_PAUSED,
    AAUDIO_STREAM_STATE_FLUSHING,
    AAUDIO_STREAM_STATE_FLUSHED,
    AAUDIO_STREAM_STATE_STOPPING,
    AAUDIO_STREAM_STATE_STOPPED,
    AAUDIO_STREAM_STATE_CLOSING,
    AAUDIO_STREAM_STATE_CLOSED,
};
typedef int32_t aaudio_stream_state_t;

enum {
    /**
     * This will be the only stream using a particular source or sink.
     * This mode will provide the lowest possible latency.
     * You should close EXCLUSIVE streams immediately when you are not using them.
     */
    AAUDIO_SHARING_MODE_EXCLUSIVE,
    /**
     * Multiple applications will be mixed by the AAudio Server.
     * This will have higher latency than the EXCLUSIVE mode.
     */
    AAUDIO_SHARING_MODE_SHARED
};
typedef int32_t aaudio_sharing_mode_t;

#ifdef __cplusplus
}
#endif

#endif // AAUDIO_AAUDIODEFINITIONS_H
