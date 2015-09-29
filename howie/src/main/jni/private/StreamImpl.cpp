//
// Created by ilewis on 9/25/15.
//

#include "StreamImpl.h"
#include "howie-private.h"
#include "EngineImpl.h"
#include <thread>


HowieError HowieStreamCreate(
    const HowieStreamCreationParams *params,
    HowieStream **out_stream) {
  __android_log_print(ANDROID_LOG_DEBUG, "HOWIE", "%s %d", __func__, __LINE__);
  HOWIE_CHECK_ENGINE_INITIALIZED();

  HOWIE_CHECK_NOT_NULL(params)
  HOWIE_CHECK(howie::checkCast<const HowieStreamCreationParams*>(params));
  return howie::EngineImpl::get()->createStream(*params, out_stream);
}

// Releases a previously created stream
HowieError HowieStreamDestroy(HowieStream *stream) {
  __android_log_print(ANDROID_LOG_DEBUG, "HOWIE", "%s %d", __func__, __LINE__);
  HOWIE_CHECK_ENGINE_INITIALIZED();
  HOWIE_CHECK_NOT_NULL(stream);
  HOWIE_CHECK(howie::checkCast<const howie::StreamImpl*>(stream));
  delete(reinterpret_cast<howie::StreamImpl*>(stream));
}

HowieError HowieStreamSendParameters(
    HowieStream *stream,
    const void* parameters,
    size_t size){
  HowieError result = HOWIE_SUCCESS;

  __android_log_print(ANDROID_LOG_DEBUG, "HOWIE", "%s %d", __func__, __LINE__);
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


namespace howie {
  HowieError StreamImpl::lastErr_ = HOWIE_SUCCESS;

  HowieError StreamImpl::init(SLEngineItf engineItf,
                              SLObjectItf outputMixObject) {
    __android_log_print(ANDROID_LOG_DEBUG, "HOWIE", "%s %d", __func__, __LINE__);
    SLresult result;

    // configure the audio source (supply data through a buffer queue in PCM format)
    SLDataLocator_AndroidSimpleBufferQueue locator_bufferqueue_source;
    SLDataFormat_PCM format_pcm;
    SLDataSource audio_source;

    // source location
    locator_bufferqueue_source.locatorType
        = SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE;
    locator_bufferqueue_source.numBuffers = 1;
    audio_source.pLocator = &locator_bufferqueue_source;

    // source format
    format_pcm.formatType = SL_DATAFORMAT_PCM;
    format_pcm.numChannels = deviceCharacteristics.channelCount;
    format_pcm.samplesPerSec = (SLuint32) deviceCharacteristics.sampleRate *
                               1000;
    format_pcm.bitsPerSample = deviceCharacteristics.bitsPerSample;
    format_pcm.containerSize = 16;
    format_pcm.channelMask = (format_pcm.numChannels == 1) ?
                             SL_SPEAKER_FRONT_CENTER :
                             SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
    format_pcm.endianness = SL_BYTEORDER_LITTLEENDIAN;
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
        &bqPlayerObject_,
        &audio_source,
        &audio_sink,
        2, // Number of interfaces
        interface_ids,
        interfaces_required
    ));

    // realize the player
    HOWIE_CHECK((*bqPlayerObject_)->Realize(bqPlayerObject_, SL_BOOLEAN_FALSE));

    // get the play interface
    HOWIE_CHECK((*bqPlayerObject_)->GetInterface(bqPlayerObject_, SL_IID_PLAY,
                                           &bqPlayerItf_));

    // get the buffer queue interface
    HOWIE_CHECK((*bqPlayerObject_)->GetInterface(bqPlayerObject_, SL_IID_BUFFERQUEUE,
                                           &bqPlayerBufferQueue_));

    // register callback on the buffer queue
    HOWIE_CHECK((*bqPlayerBufferQueue_)->RegisterCallback(
        bqPlayerBufferQueue_,
        bqPlayerCallback,
        reinterpret_cast<void *>(this)));


    // Create the buffer
    size_t bufferSize = deviceCharacteristics.framesPerPeriod
                 * deviceCharacteristics.bytesPerSample
                 * deviceCharacteristics.samplesPerFrame;
    output_.reset(bufferSize);
    output_.clear();

    // set the player's state to playing
    HOWIE_CHECK((*bqPlayerItf_)->SetPlayState(bqPlayerItf_, SL_PLAYSTATE_PLAYING));

    // Last thing before actually starting playback: call the deviceChanged
    // callback
    HowieBuffer state { sizeof(HowieBuffer), state_.get(), state_.size() };
    HowieBuffer params { sizeof(HowieBuffer), params_.get(), params_.size()};
    deviceChangedCallback_(&deviceCharacteristics, &state, &params);

    // enqueue some silence
    HOWIE_CHECK((*bqPlayerBufferQueue_)->Enqueue(
        bqPlayerBufferQueue_,
        output_.get(),
        output_.size()));
  }

// this callback handler is called every time a buffer finishes playing
  void StreamImpl::bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void
      *context) {

    if (HOWIE_SUCCEEDED(lastErr_)) {
      lastErr_ = checkCast<const StreamImpl *>(context);
    }

    if(HOWIE_SUCCEEDED(lastErr_)) {
      lastErr_ = reinterpret_cast<StreamImpl *>(context)->process(bq);
    }
  }


  HowieError StreamImpl::process(SLAndroidSimpleBufferQueueItf bq) {
    HOWIE_CHECK_NOT_NULL(bq);
    if (bq != bqPlayerBufferQueue_) {
      return HOWIE_ERROR_INVALID_OBJECT;
    }

    HowieBuffer in { sizeof(HowieBuffer), input_.get(), input_.size() };
    HowieBuffer out { sizeof(HowieBuffer), output_.get(), output_.size() };
    HowieBuffer state { sizeof(HowieBuffer), state_.get(), state_.size() };

    params_.pop(); // ignore the return value. We don't care.
    HowieBuffer params { sizeof(HowieBuffer), params_.get(), params_.size()};


    HOWIE_CHECK(processCallback_(this, &in, &out, &state, &params));

    HOWIE_CHECK((*bqPlayerBufferQueue_)->Enqueue(
        bqPlayerBufferQueue_,
        output_.get(),
        output_.size()));
    return HOWIE_SUCCESS;
  }


  bool StreamImpl::PushParameterBlock(const void *data, size_t size) {
    // spin, because the reader is lock-free and doesn't have a lot
    // of work to do
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
} // namespace howie

