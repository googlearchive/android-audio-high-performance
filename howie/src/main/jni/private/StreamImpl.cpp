//
// Created by ilewis on 9/25/15.
//

#include "StreamImpl.h"
#include "howie-private.h"
#include "EngineImpl.h"
#include <memory>


HowieError HowieCreateStream(
    HowieDirection direction,
    HowieDeviceChangedCallback deviceChangedCallback,
    HowieProcessCallback processCallback,
    HowieStream **out_stream ) {
  __android_log_print(ANDROID_LOG_VERBOSE, "HOWIE", __func__);
  HOWIE_CHECK_ENGINE_INITIALIZED();
  __android_log_print(ANDROID_LOG_VERBOSE, "HOWIE", "doin' it");

  return howie::EngineImpl::get()->createStream(
      direction,
      deviceChangedCallback,
      processCallback,
      out_stream);
}

// Releases a previously created stream
HowieError HowieDestroyStream(HowieStream *stream) {

}

namespace howie {
  HowieError StreamImpl::lastErr_ = HOWIE_SUCCESS;

  HowieError StreamImpl::init(SLEngineItf engineItf,
                              SLObjectItf outputMixObject) {
    __android_log_print(ANDROID_LOG_VERBOSE, "HOWIE", __func__);
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
    bufferSize_ = deviceCharacteristics.framesPerPeriod
                 * deviceCharacteristics.bytesPerSample
                 * deviceCharacteristics.samplesPerFrame;
    buffer_.reset(new unsigned char[bufferSize_]);
    __android_log_print(ANDROID_LOG_VERBOSE, "HOWIE", "%s line %d buffer size"
        " is %d", __func__,
                        __LINE__, bufferSize_);

    memset(buffer_.get(), 0, bufferSize_);


    __android_log_print(ANDROID_LOG_VERBOSE, "HOWIE", "%s line %d", __func__,
                        __LINE__);
    // set the player's state to playing
    HOWIE_CHECK((*bqPlayerItf_)->SetPlayState(bqPlayerItf_, SL_PLAYSTATE_PLAYING));
    __android_log_print(ANDROID_LOG_VERBOSE, "HOWIE", "%s line %d", __func__,
                        __LINE__);

    // Last thing before actually starting playback: call the deviceChanged
    // callback
    deviceChangedCallback_(&deviceCharacteristics);
    __android_log_print(ANDROID_LOG_VERBOSE, "HOWIE", "%s line %d", __func__,
                        __LINE__);

    // enqueue some silence
    HOWIE_CHECK((*bqPlayerBufferQueue_)->Enqueue(bqPlayerBufferQueue_, buffer_.get(),
                                           bufferSize_));
    __android_log_print(ANDROID_LOG_VERBOSE, "HOWIE", "%s line %d", __func__,
                        __LINE__);
  }

// this callback handler is called every time a buffer finishes playing
  void StreamImpl::bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void
      *context) {

    if (HOWIE_SUCCEEDED(lastErr_)) {
      lastErr_ = checkCast<StreamImpl *>(context);
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
    HowieBuffer buf {
      sizeof(HowieBuffer),
      buffer_.get(),
      bufferSize_
    };
    HOWIE_CHECK(processCallback_(this, nullptr, &buf));
    HOWIE_CHECK((*bqPlayerBufferQueue_)->Enqueue(bqPlayerBufferQueue_, buffer_.get(),
                                                 bufferSize_));
    return HOWIE_SUCCESS;
  }


} // namespace howie