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
#ifndef HELLOLOWLATENCYOUTPUT_STREAMIMPL_H
#define HELLOLOWLATENCYOUTPUT_STREAMIMPL_H


#include <SLES/OpenSLES_Android.h>
#include <memory>
#include <android/log.h>
#include "../howie.h"
#include "unique_buffer.h"
#include "ParameterPipe.h"

namespace howie {

  class StreamImpl : public HowieStream {
  public:
    // Defines the size of the parameter pipe ringbuffer. Larger
    // values reduce the chance of contention between the audio
    // thread and the rest of the app. Adjust this number higher
    // if the audio thread seems to be missing a large number of
    // parameter updates. See the documentation for ParameterPipe
    // for more information.
    static constexpr size_t kParameterPipeSafetyMargin = 8;
    // This is the maximum number of times the parameter pipe read function
    // can fail before we start to get worried.
    static constexpr int kMaxContentions = 5;

    StreamImpl(
        const HowieDeviceCharacteristics &deviceCharacteristics,
        const HowieStreamCreationParams &params)
        : deviceCharacteristics(deviceCharacteristics),
          direction_(params.direction),
          state_(params.sizeofStateBlock),
          params_(params.sizeofParameterBlock, kParameterPipeSafetyMargin),
          deviceChangedCallback_(params.deviceChangedCallback),
          processCallback_(params.processCallback),
          cleanupCallback_(params.cleanupCallback),
          streamState_(HOWIE_STREAM_STATE_STOPPED) {
      __android_log_print(ANDROID_LOG_DEBUG,
                          "HOWIE",
                          "%s %d",
                          __func__,
                          __LINE__);
      version = sizeof(*this);
    }

    HowieError init(SLEngineItf engineItf,
         SLObjectItf outputMixObject,
         const HowieStreamCreationParams &creationParams_);
    ~StreamImpl();

    bool PushParameterBlock(const void *data, size_t size);

    HowieError run();
    HowieError stop();
    HowieStreamState getState();


  private:
    // Defines the number of buffers used for recording. In the absence of
    // predictably ordered, synchronized I/O, we use three:
    // one to write, one to read, and one to compensate for i/o
    // being out of phase.
    static constexpr unsigned int kRecordBufferCount = 3;

    HowieDeviceCharacteristics deviceCharacteristics;
    HowieDirection direction_;

    // The smallest size a buffer can be. All of the buffers need to be
    // multiples of this number.
    size_t bufferQuantum_ = 0;

    unique_buffer input_;
    unsigned int recordBuffersSubmitted_ = 0;
    std::atomic<unsigned int> recordBuffersFinished_ = {0};

    unique_buffer output_;
    unique_buffer state_;

    ParameterPipe params_;
    int paramsContentionCounter_ = 0;


    SLObjectItf playerObject_ = nullptr;
    SLPlayItf playerItf_ = nullptr;
    SLAndroidSimpleBufferQueueItf playerBufferQueueItf_ = nullptr;
    static void bqPlayerCallback(SLAndroidSimpleBufferQueueItf, void *);

    SLObjectItf recorderObject_ = nullptr;
    SLRecordItf recorderItf_ = nullptr;
    SLAndroidSimpleBufferQueueItf recorderBufferQueueItf_ = nullptr;
    static void bqRecorderCallback(SLAndroidSimpleBufferQueueItf, void*);

    static HowieError lastPlaybackError_;
    static HowieError lastRecordError_;

    HowieDeviceChangedCallback deviceChangedCallback_;
    HowieProcessCallback processCallback_;
    HowieCleanupCallback cleanupCallback_;

    HowieError process(SLAndroidSimpleBufferQueueItf bq);

    HowieError initPlayback(SLEngineItf engineItf,
                            SLObjectItf outputMixObject,
                            SLDataFormat_PCM &format_pcm);

    HowieError initRecording(
        SLEngineItf engineItf,
        SLDataFormat_PCM &pcm);

    HowieError submitRecordBuffer();

    HowieError cleanupObjects(void);
    const unsigned int countFreeBuffers() const;

    HowieStreamState_t streamState_;

  };

} // namespace howie


#endif //HELLOLOWLATENCYOUTPUT_STREAMIMPL_H
