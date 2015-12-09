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
#include "StreamImpl.h"
#include "howie-private.h"
#include "EngineImpl.h"
#include <thread>
#include <cstring>

/**
 * Implements the C accessor for device characteristics
 */
HowieError HowieGetDeviceCharacteristics(HowieDeviceCharacteristics *dest) {
  HOWIE_TRACE_FN(HOWIE_TRACE_LEVEL_CALLS)
  HOWIE_CHECK_ENGINE_INITIALIZED();
  HOWIE_CHECK_NOT_NULL(dest)
  memcpy(dest,
         howie::EngineImpl::get()->getDeviceCharacteristics(),
         sizeof(HowieDeviceCharacteristics));
  return HOWIE_SUCCESS;
}

/**
 * Implements the C interface for stream creation
 */
HowieError HowieStreamCreate(
    const HowieStreamCreationParams *params,
    HowieStream **out_stream) {
  HOWIE_TRACE_FN(HOWIE_TRACE_LEVEL_CALLS)
  HOWIE_CHECK_ENGINE_INITIALIZED();
  HOWIE_CHECK_NOT_NULL(params)
  HOWIE_CHECK(howie::checkCast<const HowieStreamCreationParams*>(params));


  return howie::EngineImpl::get()->createStream(*params, out_stream);
}

/**
 * Implements the C interface for stream destruction
 */
HowieError HowieStreamDestroy(HowieStream *stream) {
  HOWIE_TRACE_FN(HOWIE_TRACE_LEVEL_CALLS)
  HOWIE_CHECK_ENGINE_INITIALIZED();
  HOWIE_CHECK_NOT_NULL(stream);
  HOWIE_CHECK(howie::checkCast<const howie::StreamImpl*>(stream));


  howie::EngineImpl::get()->DoAsync([=]{
    delete(reinterpret_cast<howie::StreamImpl*>(stream));});
  return HOWIE_SUCCESS;
}

/**
 * Implements the C interface for pushing parameters to the stream
 */
HowieError HowieStreamSendParameters(
    HowieStream *stream,
    const void* parameters,
    size_t size,
    int timeoutMs){
  HOWIE_TRACE_FN(HOWIE_TRACE_LEVEL_CALLS)
  HowieError result = HOWIE_SUCCESS;
  HOWIE_CHECK_ENGINE_INITIALIZED();
  HOWIE_CHECK_NOT_NULL(stream);
  HOWIE_CHECK(howie::checkCast<const howie::StreamImpl*>(stream));


  howie::StreamImpl *pStream = reinterpret_cast<howie::StreamImpl *>(stream);

  if (!pStream->PushParameterBlock(parameters, size)) {
    result = HOWIE_ERROR_AGAIN;
  }
  HOWIE_CHECK(result);
  return result;
}

HowieError HowieStreamSetState(HowieStream *stream,
                               HowieStreamState newState) {
  HOWIE_TRACE_FN(HOWIE_TRACE_LEVEL_CALLS)
  HowieError result = HOWIE_SUCCESS;
  HOWIE_CHECK_ENGINE_INITIALIZED();
  HOWIE_CHECK_NOT_NULL(stream);
  HOWIE_CHECK(howie::checkCast<const howie::StreamImpl*>(stream));


  howie::StreamImpl *pStream = reinterpret_cast<howie::StreamImpl *>(stream);

  switch (newState) {
    case HOWIE_STREAM_STATE_PLAYING:
      result = howie::EngineImpl::get()->DoAsync([=] {
          pStream->run();
      });
      break;
    case HOWIE_STREAM_STATE_STOPPED:
      result = howie::EngineImpl::get()->DoAsync([=] {
        pStream->stop();
      });
      break;
  }
  HOWIE_CHECK(result);
  return result;
}

HowieError HowieStreamGetState(HowieStream *stream, HowieStreamState *state) {
  HOWIE_TRACE_FN(HOWIE_TRACE_LEVEL_CALLS)
  HowieError result = HOWIE_SUCCESS;
  HOWIE_CHECK_ENGINE_INITIALIZED();
  HOWIE_CHECK_NOT_NULL(stream);
  HOWIE_CHECK_NOT_NULL(state);
  HOWIE_CHECK(howie::checkCast<const howie::StreamImpl*>(stream));


  howie::StreamImpl *pStream = reinterpret_cast<howie::StreamImpl *>(stream);
  *state = pStream->getState();

  HOWIE_CHECK(result);
  return result;
}

namespace howie {
  HowieError StreamImpl::lastPlaybackError_ = HOWIE_SUCCESS;
  HowieError StreamImpl::lastRecordError_ = HOWIE_SUCCESS;

  StreamImpl::~StreamImpl() {
    HOWIE_TRACE_FN(HOWIE_TRACE_LEVEL_CALLS)
    cleanupObjects();
  }

  /**
   * Delete SL Recorder and Player objects
   */
  HowieError StreamImpl::cleanupObjects(void) {
    HOWIE_TRACE_FN(HOWIE_TRACE_LEVEL_CALLS)
    stop();
    if (cleanupCallback_) {
      HowieBuffer state { sizeof(HowieBuffer), state_.get(), state_.size() };
      HOWIE_CHECK(cleanupCallback_(this, &state));
    }

    if (direction_ & HOWIE_STREAM_DIRECTION_RECORD) {
      (*recorderObject_)->Destroy(recorderObject_);
    }
    if (direction_ & HOWIE_STREAM_DIRECTION_PLAYBACK) {
      (*playerObject_)->Destroy(playerObject_);
    }
    return HOWIE_SUCCESS;
  }

  /**
   * Initialize the OpenSL inputs and outputs for this stream.
   */
  HowieError StreamImpl::init(SLEngineItf engineItf,
       SLObjectItf outputMixObject,
       const HowieStreamCreationParams &creationParams_) {
    HOWIE_TRACE_FN(HOWIE_TRACE_LEVEL_CALLS)


    // Compute the buffer quantum
    bufferQuantum_ = deviceCharacteristics.framesPerPeriod
                     * deviceCharacteristics.bytesPerSample
                     * deviceCharacteristics.samplesPerFrame;

    // configure the audio source (supply data through a buffer queue in PCM format)
    SLDataFormat_PCM format_pcm;

    // source format
    format_pcm.formatType = SL_DATAFORMAT_PCM;
    format_pcm.numChannels = deviceCharacteristics.channelCount;
    format_pcm.samplesPerSec = (SLuint32) deviceCharacteristics.sampleRate *
                               1000;
    format_pcm.bitsPerSample = deviceCharacteristics.bitsPerSample;
    format_pcm.containerSize = deviceCharacteristics.bitsPerSample
                               * deviceCharacteristics.samplesPerFrame;
    format_pcm.channelMask = (format_pcm.numChannels == 1) ?
                             SL_SPEAKER_FRONT_CENTER :
                             SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
    format_pcm.endianness = SL_BYTEORDER_LITTLEENDIAN;


    if (direction_ & HOWIE_STREAM_DIRECTION_RECORD) {
      HOWIE_CHECK(initRecording(engineItf, format_pcm));
    }

    if (direction_ & HOWIE_STREAM_DIRECTION_PLAYBACK) {
      HOWIE_CHECK(initPlayback(engineItf,
                   outputMixObject,
                   format_pcm));
    }

    if (creationParams_.initialState == HOWIE_STREAM_STATE_PLAYING) {
      run();
    }
    return HOWIE_SUCCESS;
  }

  HowieError StreamImpl::initRecording(SLEngineItf engineItf,
                                       SLDataFormat_PCM& format_pcm) {
    HOWIE_TRACE_FN(HOWIE_TRACE_LEVEL_CALLS)
    // configure audio source
    SLDataLocator_IODevice loc_dev = {SL_DATALOCATOR_IODEVICE, SL_IODEVICE_AUDIOINPUT,
                                      SL_DEFAULTDEVICEID_AUDIOINPUT, NULL};
    SLDataSource audioSrc = {&loc_dev, NULL};

    // configure record buffer queue
    SLDataLocator_AndroidSimpleBufferQueue loc_bq = {
        SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, kRecordBufferCount};

    SLDataSink audioSnk = {&loc_bq, &format_pcm};

    // create audio recorder
    // (requires the RECORD_AUDIO permission)
    const SLInterfaceID id[1] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
    HOWIE_CHECK ((*engineItf)->CreateAudioRecorder(
        engineItf,
        &recorderObject_,
        &audioSrc,
        &audioSnk,
        1,
        id,
        req));

    // realize the audio recorder
    HOWIE_CHECK ((*recorderObject_)->Realize(recorderObject_,
                                             SL_BOOLEAN_FALSE));

    // get the record interface
    HOWIE_CHECK ((*recorderObject_)->GetInterface(
        recorderObject_,
        SL_IID_RECORD,
        &recorderItf_));


    // get the buffer queue interface
    HOWIE_CHECK ((*recorderObject_)->GetInterface(
        recorderObject_,
        SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
        &recorderBufferQueueItf_));

    // register callback on the buffer queue
    HOWIE_CHECK ((*recorderBufferQueueItf_)->RegisterCallback(
        recorderBufferQueueItf_,
        bqRecorderCallback,
        this));

    // start recording
    SLAndroidSimpleBufferQueueState qState;
    HOWIE_CHECK((*recorderBufferQueueItf_)->GetState(recorderBufferQueueItf_, &qState));

    // Create the recording buffer and submit the first chunk
    input_.reset(bufferQuantum_ * kRecordBufferCount);
    input_.clear();

    HOWIE_CHECK(submitRecordBuffer());
    HOWIE_CHECK((*recorderItf_)->SetRecordState(recorderItf_, SL_RECORDSTATE_PAUSED));
    return HOWIE_SUCCESS;
  }

  HowieError StreamImpl::initPlayback(
        SLEngineItf engineItf,
        SLObjectItf outputMixObject,
        SLDataFormat_PCM &format_pcm)
  {
    HOWIE_TRACE_FN(HOWIE_TRACE_LEVEL_CALLS)
    SLDataLocator_AndroidSimpleBufferQueue locator_bufferqueue_source;
    SLDataSource audio_source;

    // source location
    locator_bufferqueue_source.locatorType
        = SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE;
    locator_bufferqueue_source.numBuffers = 1;
    audio_source.pLocator = &locator_bufferqueue_source;
    audio_source.pFormat = &format_pcm;


    // configure the output: An output mix sink
    SLDataLocator_OutputMix locator_output_mix;
    SLDataSink audio_sink;

    locator_output_mix.locatorType = SL_DATALOCATOR_OUTPUTMIX;
    locator_output_mix.outputMix = outputMixObject;

    audio_sink.pLocator = &locator_output_mix;
    audio_sink.pFormat = NULL;

    // create audio player
    // Note: Adding other output interfaces here will result in your audio being routed using the
    // normal path NOT the fast path
    const SLInterfaceID
        interface_ids[2] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE, SL_IID_VOLUME};
    const SLboolean interfaces_required[2] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

    HOWIE_CHECK((*engineItf)->CreateAudioPlayer(
        engineItf,
        &playerObject_,
        &audio_source,
        &audio_sink,
        2, // Number of interfaces
        interface_ids,
        interfaces_required
    ));

    // realize the player
    HOWIE_CHECK((*playerObject_)->Realize(playerObject_, SL_BOOLEAN_FALSE));

    // get the play interface
    HOWIE_CHECK((*playerObject_)->GetInterface(playerObject_, SL_IID_PLAY,
                                           &playerItf_));


    // get the buffer queue interface
    HOWIE_CHECK((*playerObject_)->GetInterface(playerObject_, SL_IID_BUFFERQUEUE,
                                           &playerBufferQueueItf_));

    // register callback on the buffer queue
    HOWIE_CHECK((*playerBufferQueueItf_)->RegisterCallback(
        playerBufferQueueItf_,
        bqPlayerCallback,
        reinterpret_cast<void *>(this)));


    // Create the buffer
    output_.reset(bufferQuantum_);
    output_.clear();

    // Last thing before actually starting playback: call the deviceChanged
    // callback
    HowieBuffer state { sizeof(HowieBuffer), state_.get(), state_.size() };
    HowieBuffer params { sizeof(HowieBuffer), params_.top(),
                         params_.maxElementSize()};
    deviceChangedCallback_(&deviceCharacteristics, &state, &params);

    // enqueue some silence
    HOWIE_CHECK((*playerBufferQueueItf_)->Enqueue(
        playerBufferQueueItf_,
        output_.get(),
        output_.size()));
    HOWIE_CHECK((*playerItf_)->SetPlayState(playerItf_, SL_PLAYSTATE_PAUSED));
    return HOWIE_SUCCESS;
  }

  HowieError StreamImpl::submitRecordBuffer() {
    HOWIE_TRACE_FN(HOWIE_TRACE_LEVEL_REALTIME);
    while(countFreeBuffers() < kRecordBufferCount) {
      size_t offset = (recordBuffersSubmitted_ % kRecordBufferCount) * bufferQuantum_;
      ++recordBuffersSubmitted_;
      HOWIE_RTCHECK((*recorderBufferQueueItf_)->Enqueue(recorderBufferQueueItf_,
                                                      input_.get() + offset,
                                                      bufferQuantum_));
    }
    return HOWIE_SUCCESS;
  }


  void StreamImpl::bqRecorderCallback(SLAndroidSimpleBufferQueueItf itf,
                                      void *context) {
    HOWIE_TRACE_FN(HOWIE_TRACE_LEVEL_ALL);
    if (HOWIE_SUCCEEDED(lastRecordError_)) {
      lastPlaybackError_ = checkCast<const StreamImpl *>(context);
    }

    StreamImpl *pStream = reinterpret_cast<StreamImpl *>(context);
    if (HOWIE_SUCCEEDED(lastRecordError_)) {
      pStream->recordBuffersFinished_.fetch_add(1, std::memory_order_release);
    }
  }


  // this callback handler is called every time a buffer finishes playing
  void StreamImpl::bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void
      *context) {
    HOWIE_TRACE_FN(HOWIE_TRACE_LEVEL_ALL);

    if (HOWIE_SUCCEEDED(lastPlaybackError_)) {
      lastPlaybackError_ = checkCast<const StreamImpl *>(context);
    }

    if(HOWIE_SUCCEEDED(lastPlaybackError_)) {
      lastPlaybackError_ = reinterpret_cast<StreamImpl *>(context)->process(bq);
    }
  }


  HowieError StreamImpl::process(SLAndroidSimpleBufferQueueItf bq) {
    HOWIE_CHECK_NOT_NULL(bq);
    HOWIE_TRACE_FN(HOWIE_TRACE_LEVEL_REALTIME);

    if (bq != playerBufferQueueItf_) {
      return HOWIE_ERROR_INVALID_OBJECT;
    }

    HowieBuffer in { sizeof(HowieBuffer), nullptr, 0 };
    if (direction_ & HOWIE_STREAM_DIRECTION_RECORD) {
      size_t inputOffset = 0;
      int recordBuffersFinished = recordBuffersFinished_.load(
          std::memory_order_acquire);

      int nBuffersAvailable = countFreeBuffers();
      if (nBuffersAvailable <= 0) {
        __android_log_write(
            ANDROID_LOG_WARN,
            kLibName,
            "GLITCH: missed a record buffer");
      }
      inputOffset = (recordBuffersFinished % kRecordBufferCount) * bufferQuantum_;
      in.data = input_.get() + inputOffset;
      in.byteCount = bufferQuantum_;
    }

    HowieBuffer out { sizeof(HowieBuffer), output_.get(), output_.size() };
    HowieBuffer state { sizeof(HowieBuffer), state_.get(), state_.size() };

    if (params_.maxElementSize() == 0 || params_.pop()) {
      paramsContentionCounter_ = 0;
    } else {
      if (++paramsContentionCounter_ > kMaxContentions) {
        __android_log_print(
            ANDROID_LOG_WARN,
            kLibName,
            "Read thread has missed %d parameter updates in a row. "
            "This is probably because you are making too many calls to "
            "HowieStreamSendParameters().", paramsContentionCounter_);
      }
    }
    HowieBuffer params { sizeof(HowieBuffer), params_.top(),
                         params_.maxElementSize()};


    HOWIE_RTCHECK(processCallback_(this, &in, &out, &state, &params));

    if (direction_ & HOWIE_STREAM_DIRECTION_PLAYBACK) {
      HOWIE_RTCHECK((*playerBufferQueueItf_)->Enqueue(playerBufferQueueItf_,
                                                   output_.get(),
                                                   output_.size()));
    }

    if (direction_ & HOWIE_STREAM_DIRECTION_RECORD) {
      HOWIE_RTCHECK(submitRecordBuffer());
    }

    return HOWIE_SUCCESS;
  }


  bool StreamImpl::PushParameterBlock(const void *data, size_t size) {
    HOWIE_TRACE_FN(HOWIE_TRACE_LEVEL_CALLS)
    bool result = (params_.push(data, size) > 0);
    return result;
  }

  /**
   * Count the number of free (readable) recording buffers.
   *
   * NB: this function is only safe to call from the playback thread.
   *     If called from another thread it will likely give erroneous
   *     results, because recordBuffersSubmitted_ is not synchronized.
   */
  const unsigned int StreamImpl::countFreeBuffers() const {
    HOWIE_TRACE_FN(HOWIE_TRACE_LEVEL_REALTIME)
    unsigned int finished = recordBuffersFinished_.load(std::memory_order_acquire);
    unsigned int result = recordBuffersSubmitted_ - finished;
    return result;
  }

  HowieError StreamImpl::run() {
    HOWIE_TRACE_FN(HOWIE_TRACE_LEVEL_CALLS)
    if (direction_ & HOWIE_STREAM_DIRECTION_RECORD) {
      HOWIE_CHECK((*recorderItf_)->SetRecordState(
          recorderItf_, SL_RECORDSTATE_RECORDING));
    }
    if (direction_ & HOWIE_STREAM_DIRECTION_PLAYBACK) {
      HOWIE_CHECK((*playerItf_)->SetPlayState(
          playerItf_, SL_PLAYSTATE_PLAYING));
    }
    streamState_ = HOWIE_STREAM_STATE_PLAYING;
    return HOWIE_SUCCESS;
  }

  HowieError StreamImpl::stop() {
    HOWIE_TRACE_FN(HOWIE_TRACE_LEVEL_CALLS)
    if (direction_ & HOWIE_STREAM_DIRECTION_RECORD) {
      HOWIE_CHECK((*recorderItf_)->SetRecordState(
          recorderItf_, SL_RECORDSTATE_PAUSED));
    }
    if (direction_ & HOWIE_STREAM_DIRECTION_PLAYBACK) {
      HOWIE_CHECK((*playerItf_)->SetPlayState(
          playerItf_, SL_PLAYSTATE_PAUSED));
    }
    streamState_ = HOWIE_STREAM_STATE_STOPPED;
    return HOWIE_SUCCESS;

  }

  HowieStreamState_t StreamImpl::getState() {
    HOWIE_TRACE_FN(HOWIE_TRACE_LEVEL_CALLS)
    return streamState_;
  }
} // namespace howie

