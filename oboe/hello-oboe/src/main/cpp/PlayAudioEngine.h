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

#ifndef OBOE_HELLOOBOE_PLAYAUDIOENGINE_H
#define OBOE_HELLOOBOE_PLAYAUDIOENGINE_H

#include <thread>
#include "oboe/Oboe.h"
#include "SineGenerator.h"

constexpr int32_t kBufferSizeAutomatic = 0;

class PlayAudioEngine : OboeStreamCallback {

public:
    PlayAudioEngine();

    ~PlayAudioEngine();

    void setDeviceId(int32_t deviceId);

    void setToneOn(bool isToneOn);

    void setBufferSizeInBursts(int32_t numBursts);

    double getCurrentOutputLatencyMillis();

    // OboeStreamCallback methods
    oboe_data_callback_result_t
    onAudioReady(OboeStream *audioStream, void *audioData, int32_t numFrames) override;

    void onError(OboeStream *audioStream, oboe_result_t error) override;

private:
    int32_t mPlaybackDeviceId = OBOE_UNSPECIFIED;
    int32_t mSampleRate;
    int16_t mSampleChannels;
    bool mIsToneOn = false;
    int32_t mFramesPerBurst;
    double mCurrentOutputLatencyMillis = 0;
    int32_t mBufferSizeSelection = kBufferSizeAutomatic;
    bool mIsLatencyDetectionSupported = false;
    OboeStream *mPlayStream;
    OboeLatencyTuner *mLatencyTuner;
    std::thread *mStreamRestartThread;
    std::mutex mRestartingLock;

    // The SineGenerators generate audio data, feel free to replace with your own audio generators
    SineGenerator mSineOscLeft;
    SineGenerator mSineOscRight;

    void createPlaybackStream();

    void closeOutputStream();

    void restartStream();

    void setupPlaybackStreamParameters(OboeStreamBuilder *builder);

    void prepareOscillators();

    oboe_result_t calculateCurrentOutputLatencyMillis(OboeStream *stream, double *latencyMillis);
};

#endif //OBOE_HELLOOBOE_PLAYAUDIOENGINE_H
