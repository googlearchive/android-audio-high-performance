//
// Created by ilewis on 9/29/15.
//
#include "player.h"
#include "howie.h"



HowieError onDeviceChanged(
    const HowieDeviceCharacteristics* hdc,
    const HowieBuffer *state,
    const HowieBuffer *params) {
  return HOWIE_SUCCESS;
}

HowieError onProcess(
    const HowieStream *stream,
    const HowieBuffer *in,
    const HowieBuffer *out,
    const HowieBuffer *state,
    const HowieBuffer *params) {
  return HOWIE_SUCCESS;
}

HowieError onCleanup(
    const HowieStream *stream,
    const HowieBuffer *state) {
  return HOWIE_SUCCESS;
}

void Java_com_example_ilewis_hellolowlatencyio_MainActivity_init(
    JNIEnv *env,
    jobject instance) {
  HowieStreamCreationParams hscp = {
      sizeof(HowieStreamCreationParams),
      HOWIE_DIRECTION_BOTH,
      onDeviceChanged,
      onProcess,
      onCleanup,
      0,
      0};

  HowieStreamCreate(&hscp, NULL);
}
