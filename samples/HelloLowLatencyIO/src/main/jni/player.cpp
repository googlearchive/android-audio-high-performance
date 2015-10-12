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
#include <string.h>
#include <array>
#include <cmath>
#include "player.h"
#include "howie.h"


struct Params {
  // System transform
  float A[2][2];

  // input-side vector
  float B[2];

  // output-side vector
  float C[3];

  float gain;
};

struct State {

  // current state vector
  float y[2];

};

void CookParameters(float frequency, float resonance, Params *out_params) {
  float f = frequency;
  float q = resonance;

  float g = tan(M_PI * f);
  float k = 2.f - q * 2.f;
  float a1 = 1.f / (1.f + g * (g + k));
  float a2 = g * a1;
  float a3 = g * a3;

  out_params->A[0][0] = 2.f * a1 - 1.f;
  out_params->A[0][1] = -2.f * a2;
  out_params->A[1][0] = 2.f * a2;
  out_params->A[1][1] = 1 - 2.f * a3;

  out_params->B[0] = 2.f * a2;
  out_params->B[1] = 2.f * a3;

  out_params->C[0] = a3;
  out_params->C[1] = a2;
  out_params->C[2] = 1.f - a3;
}

HowieError onDeviceChanged(
    const HowieDeviceCharacteristics* hdc,
    const HowieBuffer *state,
    const HowieBuffer *params) {
  State* pState = reinterpret_cast<State *>(state->data);
  Params *pParams = reinterpret_cast<Params *>(params->data);

  pState->y[0] = 0;
  pState->y[1] = 0;

  CookParameters(.2f, .9f, pParams);

  return HOWIE_SUCCESS;
}


HowieError onProcess(
    const HowieStream *stream,
    const HowieBuffer *in,
    const HowieBuffer *out,
    const HowieBuffer *state,
    const HowieBuffer *params) {
  State *pState = reinterpret_cast<State *>(state->data);
  Params *pParams = reinterpret_cast<Params *>(params->data);


  short *input_samples = reinterpret_cast<short *>(in->data);
  short *output_samples = reinterpret_cast<short *>(out->data);

  // alias all the struct members for convenience
  float (&A)[2][2] = pParams->A;
  float (&B)[2] = pParams->B;
  float (&C)[3] = pParams->C;
  float (&y)[2] = pState->y;

  for (int i = 0; i < in->byteCount /2; ++i) {
    // y[n+1] = A*y[n] + B * x[n]
    float x = (float)input_samples[i] / 32767.f;
    float next_y[2] = {
        A[0][0] * y[0] + A[0][1] * y[1] + B[0] * x,
        A[1][0] * y[0] + A[1][1] * y[1] + B[1] * x
    };

    // out[n] = C * concat([x[n]], y[n])
    float out_n = C[0] * x + C[1] * y[0] + C[2] * y[1];
    y[0] = next_y[0];
    y[1] = next_y[1];
    out_n *= pParams->gain;
    // clamp
    out_n = std::max(-1.0f, std::min(1.0f, out_n));
    output_samples[i] = (short)(out_n * 32767.f);
  }
  return HOWIE_SUCCESS;
}

HowieError onCleanup(
    const HowieStream *stream,
    const HowieBuffer *state) {
  return HOWIE_SUCCESS;
}

JNIEXPORT jlong JNICALL
Java_com_example_ilewis_hellolowlatencyio_MainActivity_initStream(
    JNIEnv *env,
    jobject instance) {
  HowieStreamCreationParams hscp = {
      sizeof(HowieStreamCreationParams), HOWIE_STREAM_DIRECTION_BOTH,
      onDeviceChanged,
      onProcess,
      onCleanup,
      sizeof(State),
      sizeof(Params),
      HOWIE_STREAM_STATE_PLAYING};

  HowieStream *pStream = nullptr;
  HowieStreamCreate(&hscp, &pStream);
  return reinterpret_cast<jlong>(pStream);
}

JNIEXPORT void JNICALL
Java_com_example_ilewis_hellolowlatencyio_MainActivity_setParams(
    JNIEnv *env,
    jobject instance,
    jlong stream,
    jfloat frequency,
    jfloat resonance,
    jfloat gain) {
  HowieStream *pStream = reinterpret_cast<HowieStream *>(stream);
  Params params;
  CookParameters(frequency, resonance, &params);
  params.gain = gain;
  HowieStreamSendParameters(pStream, &params, sizeof(params), 1);
}