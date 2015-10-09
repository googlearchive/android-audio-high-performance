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

/**
 * Implements the C interface for stream creation
 */
HowieError HowieStreamCreate(
    const HowieStreamCreationParams *params,
    HowieStream **out_stream) {
  HOWIE_CHECK_ENGINE_INITIALIZED();
  HOWIE_CHECK_NOT_NULL(params)
  HOWIE_CHECK(howie::checkCast<const HowieStreamCreationParams*>(params));
  __android_log_print(ANDROID_LOG_VERBOSE, "HOWIE", __func__);

  return howie::EngineImpl::get()->createStream(*params, out_stream);
}

/**
 * Implements the C interface for stream destruction
 */
HowieError HowieStreamDestroy(HowieStream *stream) {
  HOWIE_CHECK_ENGINE_INITIALIZED();
  HOWIE_CHECK_NOT_NULL(stream);
  HOWIE_CHECK(howie::checkCast<const howie::StreamImpl*>(stream));
  __android_log_print(ANDROID_LOG_VERBOSE, "HOWIE", __func__);

  delete(reinterpret_cast<howie::StreamImpl*>(stream));
}

/**
 * Implements the C interface for pushing parameters to the stream
 */
HowieError HowieStreamSendParameters(
    HowieStream *stream,
    const void* parameters,
    size_t size){
  HowieError result = HOWIE_SUCCESS;
  HOWIE_CHECK_ENGINE_INITIALIZED();
  HOWIE_CHECK_NOT_NULL(stream);
  HOWIE_CHECK(howie::checkCast<const howie::StreamImpl*>(stream));
  __android_log_print(ANDROID_LOG_VERBOSE, "HOWIE", __func__);

  howie::StreamImpl *pStream = reinterpret_cast<howie::StreamImpl *>(stream);

  if (!pStream->PushParameterBlock(parameters, size)) {
    result = HOWIE_ERROR_AGAIN;
  }
  HOWIE_CHECK(result);
  return result;
}


namespace howie {
  HowieError StreamImpl::lastPlaybackError_ = HOWIE_SUCCESS;
  HowieError StreamImpl::lastRecordError_ = HOWIE_SUCCESS;

  StreamImpl::~StreamImpl() {
    cleanupObjects();
  }

  /**
   * Delete SL Recorder and Player objects
   */
  HowieError StreamImpl::cleanupObjects(void) {
    if (direction_ & HOWIE_DIRECTION_RECORD) {
      HOWIE_CHECK((*recorderItf_)->SetRecordState(recorderItf_,
                                      SL_RECORDSTATE_STOPPED));
    }
    if (direction_ & HOWIE_DIRECTION_PLAYBACK) {
      HOWIE_CHECK((*playerItf_)->SetPlayState(playerItf_,
                                              SL_PLAYSTATE_STOPPED));
    }
    if (cleanupCallback_) {
      HowieBuffer state { sizeof(HowieBuffer), state_.get(), state_.size() };
      HOWIE_CHECK(cleanupCallback_(this, &state));
    }
    (*recorderObject_)->Destroy(recorderObject_);
    (*playerObject_)->Destroy(playerObject_);
  }

  /**
   * Initialize the OpenSL inputs and outputs for this stream.
   */
  HowieError StreamImpl::init(SLEngineItf engineItf,
                              SLObjectItf outputMixObject) {
    SLresult result;
    __android_log_print(ANDROID_LOG_VERBOSE, "HOWIE", __func__);

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


    if (direction_ & HOWIE_DIRECTION_RECORD) {
      HOWIE_CHECK(initRecording(engineItf, format_pcm));
    }

    if (direction_ & HOWIE_DIRECTION_PLAYBACK) {
      HOWIE_CHECK(initPlayback(engineItf,
                   outputMixObject,
                   format_pcm));
    }

  }

  HowieError StreamImpl::initRecording(SLEngineItf engineItf,
                                       SLDataFormat_PCM& format_pcm) {
    __android_log_print(ANDROID_LOG_VERBOSE, "HOWIE", __func__);
    // configure audio source
    SLDataLocator_IODevice loc_dev = {SL_DATALOCATOR_IODEVICE, SL_IODEVICE_AUDIOINPUT,
                                      SL_DEFAULTDEVICEID_AUDIOINPUT, NULL};
    SLDataSource audioSrc = {&loc_dev, NULL};

    // configure record buffer queue
    SLDataLocator_AndroidSimpleBufferQueue loc_bq = {
        SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
        nRecordBuffers_ };

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
    HOWIE_CHECK((*recorderItf_)->SetRecordState(recorderItf_, SL_RECORDSTATE_RECORDING));
    SLAndroidSimpleBufferQueueState qState;
    HOWIE_CHECK((*recorderBufferQueueItf_)->GetState(recorderBufferQueueItf_, &qState));
    __android_log_print(ANDROID_LOG_INFO, "HOWIE", "Get Recording status right after start recording: (%d, %d)", qState.count, qState.index);

    // Create the recording buffer and submit the first chunk
    input_.reset(bufferQuantum_ * nRecordBuffers_);
    input_.clear();

    return submitRecordBuffer();
  }

  HowieError StreamImpl::initPlayback(
        SLEngineItf engineItf,
        SLObjectItf outputMixObject,
        SLDataFormat_PCM &format_pcm)
  {
    __android_log_print(ANDROID_LOG_VERBOSE, "HOWIE", __func__);
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

    // set the player's state to playing
    HOWIE_CHECK((*playerItf_)->SetPlayState(playerItf_, SL_PLAYSTATE_PLAYING));

    // Last thing before actually starting playback: call the deviceChanged
    // callback
    HowieBuffer state { sizeof(HowieBuffer), state_.get(), state_.size() };
    HowieBuffer params { sizeof(HowieBuffer), params_.get(), params_.size()};
    deviceChangedCallback_(&deviceCharacteristics, &state, &params);

    // enqueue some silence
    HOWIE_CHECK((*playerBufferQueueItf_)->Enqueue(
        playerBufferQueueItf_,
        output_.get(),
        output_.size()));
  }

  HowieError StreamImpl::submitRecordBuffer() {
    while(countFreeBuffers() < nRecordBuffers_) {
      size_t offset = (recordBuffersSubmitted_ % nRecordBuffers_) * bufferQuantum_;
      ++recordBuffersSubmitted_;
      HOWIE_CHECK((*recorderBufferQueueItf_)->Enqueue(recorderBufferQueueItf_,
                                                      input_.get() + offset,
                                                      bufferQuantum_));
    }
  }


  void StreamImpl::bqRecorderCallback(SLAndroidSimpleBufferQueueItf itf,
                                      void *context) {
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

    if (HOWIE_SUCCEEDED(lastPlaybackError_)) {
      lastPlaybackError_ = checkCast<const StreamImpl *>(context);
    }

    if(HOWIE_SUCCEEDED(lastPlaybackError_)) {
      lastPlaybackError_ = reinterpret_cast<StreamImpl *>(context)->process(bq);
    }
  }


  HowieError StreamImpl::process(SLAndroidSimpleBufferQueueItf bq) {
    HOWIE_CHECK_NOT_NULL(bq);

    if (bq != playerBufferQueueItf_) {
      return HOWIE_ERROR_INVALID_OBJECT;
    }

    HowieBuffer in { sizeof(HowieBuffer), nullptr, 0 };
    if (direction_ & HOWIE_DIRECTION_RECORD) {
      size_t inputOffset = 0;
      int recordBuffersFinished = recordBuffersFinished_.load(
          std::memory_order_acquire);

      int nBuffersAvailable = countFreeBuffers();
      if (nBuffersAvailable <= 0) {
        __android_log_write(
            ANDROID_LOG_WARN,
            "HOWIE",
            "GLITCH: missed a record buffer");
      }
      inputOffset = (recordBuffersFinished % nRecordBuffers_) * bufferQuantum_;
      in.data = input_.get() + inputOffset;
      in.byteCount = bufferQuantum_;
    }

    HowieBuffer out { sizeof(HowieBuffer), output_.get(), output_.size() };
    HowieBuffer state { sizeof(HowieBuffer), state_.get(), state_.size() };

    if (params_.pop()) {
      paramsContentionCounter_ = 0;
    } else {
      if (++paramsContentionCounter_ > maxContentions_) {
        __android_log_print(
            ANDROID_LOG_WARN,
            "HOWIE",
            "Read thread has missed %d parameter updates in a row. "
            "This is probably because you are making too many calls to "
            "HowieStreamSendParameters().");
      }
    }
    HowieBuffer params { sizeof(HowieBuffer), params_.get(), params_.size()};


    HOWIE_CHECK(processCallback_(this, &in, &out, &state, &params));

    if (direction_ & HOWIE_DIRECTION_PLAYBACK) {
      HOWIE_CHECK((*playerBufferQueueItf_)->Enqueue(playerBufferQueueItf_,
                                                   output_.get(),
                                                   output_.size()));
    }

    if (direction_ & HOWIE_DIRECTION_RECORD) {
      HOWIE_CHECK(submitRecordBuffer());
    }

    return HOWIE_SUCCESS;
  }


  bool StreamImpl::PushParameterBlock(const void *data, size_t size) {
    __android_log_print(ANDROID_LOG_VERBOSE, "HOWIE", __func__);

    // spin, because the reader is lock-free and high-priority and doesn't
    // have a lot of work to do
    bool result = false;
    for ( int i = 0; i < 5 && !result; ++i) {
      result = params_.push(data, size);
    }

    // still not finished? Try again, but with yields this time.
    for ( int i = 0; i < 5 && !result; ++i) {
      std::this_thread::yield();
      result = params_.push(data, size);
    }

    // At this point we're not going to wait any longer, so the
    // result is what it is.
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
    unsigned int finished = recordBuffersFinished_.load(std::memory_order_acquire);
    unsigned int result = recordBuffersSubmitted_ - finished;
    return result;
  }
} // namespace howie

