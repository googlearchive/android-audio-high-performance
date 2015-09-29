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
 */

#include <assert.h>
#include <jni.h>
#include <math.h>
#include <malloc.h>

// for native audio
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

// for native asset manager
#include <sys/types.h>

// logging
#include <android/log.h>
#include <string.h>
#include "../../../../howie/src/main/jni/howie.h"

#define APPNAME "HelloLowLatencyOutput"


#define TWO_PI (3.14159 * 2)

struct PlayerState {
  short* sineWaveBuffer;
};

struct PlayerParams {
  unsigned int playing;
};

#define MAXIMUM_AMPLITUDE_VALUE 32767

/**
 * Create wave tables with the specified number of frames
 */
void createWaveTables(
    unsigned int frames,
    unsigned int channels,
    struct PlayerState *state){

    // First figure out how many samples we need and allocate memory for the tables
    unsigned int numSamples = frames * channels;
    state->sineWaveBuffer = malloc(sizeof(*state->sineWaveBuffer) * numSamples);
    size_t bufferSizeInBytes = numSamples * 2;

    __android_log_print(ANDROID_LOG_VERBOSE,
                        APPNAME,
                        "Creating wave tables. Frames: %i Channels: %i Total samples: %i Buffer size (bytes): %i",
                        frames,
                        channels,
                        numSamples,
                        bufferSizeInBytes);

    // Now create the sine wave - we'll just create a single cycle which fills the entire table
    float phaseIncrement = (float) TWO_PI/frames;
    float currentPhase = 0.0;

    unsigned int i;
    unsigned int j;

    for (i = 0; i < frames; i++) {

        short sampleValue = (short) (sin(currentPhase) * MAXIMUM_AMPLITUDE_VALUE);

        for (j = 0; j < channels; j++){
            state->sineWaveBuffer[(i* channels)+j] = sampleValue;
        }

        currentPhase += phaseIncrement;
    }
}


HowieError onDeviceChanged(
    const HowieDeviceCharacteristics * pHDC,
    const HowieBuffer *state,
    const HowieBuffer *params) {
  __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, __func__);

  struct PlayerState* playerState = (struct PlayerState*)state->data;
  struct PlayerParams* playerParams = (struct PlayerParams*)params->data;

  createWaveTables(pHDC->framesPerPeriod, pHDC->channelCount, playerState);
  playerParams->playing = 0;
  return HOWIE_SUCCESS;
}

HowieError onProcess(
    const HowieStream *stream,
    const HowieBuffer *in,
    const HowieBuffer *out,
    const HowieBuffer *state,
    const HowieBuffer *params) {
  struct PlayerState* playerState = (struct PlayerState*)state->data;
  struct PlayerParams* playerParams = (struct PlayerParams*)params->data;
  if (playerParams->playing) {
    memcpy(out->data, playerState->sineWaveBuffer, out->byteCount);
  } else {
    memset(out->data, 0, out->byteCount);
  }
  return HOWIE_SUCCESS;
}

HowieError onCleanup(
    const HowieStream *stream,
    const HowieBuffer *state) {
  return HOWIE_SUCCESS;
}

JNIEXPORT jlong JNICALL
Java_com_example_hellolowlatencyoutput_MainActivity_initPlayback(JNIEnv *env,
                                                                 jclass type) {

   __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "Trying to create stream");
  HowieStreamCreationParams hscp = {
      sizeof(HowieStreamCreationParams),
      HOWIE_DIRECTION_PLAYBACK,
      onDeviceChanged,
      onProcess,
      onCleanup,
      sizeof(struct PlayerState),
      sizeof(struct PlayerParams)
  };
  jlong stream;
  HowieStreamCreate(&hscp, (HowieStream**)&stream);
  __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "Created stream, I think?");
  return stream;
}

JNIEXPORT void JNICALL
Java_com_example_hellolowlatencyoutput_MainActivity_playTone(
    JNIEnv* env, jclass clazz, jlong streamId){
  HowieStream* stream = (HowieStream*)(void*)streamId;
  __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "Playing tone");
  struct PlayerParams params = { 1 };
  HowieStreamSendParameters(stream, &params, sizeof(params));
}

JNIEXPORT void JNICALL
Java_com_example_hellolowlatencyoutput_MainActivity_stopPlaying(
    JNIEnv *env,
    jclass type,
    jlong streamId) {
  HowieStream* stream = (HowieStream*)(void*)streamId;
  __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "Stopping tone");
  struct PlayerParams params = { 0 };
  HowieStreamSendParameters(stream, &params, sizeof(params));
}

