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
#define APPNAME "HelloLowLatency"

// engine interfaces
static SLObjectItf engineObject = NULL;
static SLEngineItf engineEngine;

// output mix interfaces
static SLObjectItf outputMixObject = NULL;

// buffer queue player interfaces
static SLObjectItf bqPlayerObject = NULL;
static SLPlayItf bqPlayerPlay;
static SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;

#define CHANNELS 1
#define TWO_PI (3.14159 * 2)

// Each short represents a 16-bit audio sample
static short* squareWaveBuffer;
static short* sineWaveBuffer;
static short* silenceBuffer;
static unsigned int bufferSizeInBytes;

#define MAXIMUM_AMPLITUDE_VALUE 32767

// how many times to play the wave table (so we can actually hear it)
#define BUFFERS_TO_PLAY 100

static unsigned buffersRemaining = 0;

void createWaveTables(int frames){

    // Create a sine wave using 16-bit samples (shorts) which has one cycle in the supplied number of frames

    // First figure out how many samples we need and create an array for it
    int numSamples = frames * CHANNELS;
    silenceBuffer = malloc(sizeof(*silenceBuffer) * numSamples);
    sineWaveBuffer = malloc(sizeof(*sineWaveBuffer) * numSamples);
    bufferSizeInBytes = numSamples * 2;

    __android_log_print(ANDROID_LOG_VERBOSE,
                        APPNAME,
                        "Creating wave tables. Frames: %i Channels: %i Total samples: %i Buffer size (bytes): %i",
                        frames,
                        CHANNELS,
                        numSamples,
                        bufferSizeInBytes);

    // Now create the sine wave
    float phaseIncrement = TWO_PI/frames;
    float currentPhase = 0.0;

    unsigned int i;
    unsigned int j;

    for (i = 0; i < frames; i++) {

        short sampleValue = sin(currentPhase) * MAXIMUM_AMPLITUDE_VALUE;

        for (j = 0; j < CHANNELS; j++){
            sineWaveBuffer[(i*CHANNELS)+j] = sampleValue;
            silenceBuffer[(i*CHANNELS)+j] = 0;
        }

        currentPhase += phaseIncrement;
    }
}

// this callback handler is called every time a buffer finishes playing
void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
    assert(bq == bqPlayerBufferQueue);
    assert(NULL == context);

    SLresult result;

    short* bufferPtr;

    if (buffersRemaining > 0) {

        buffersRemaining--;
        bufferPtr = sineWaveBuffer;

    } else {

        //Enqueue silence to keep the player in warmed up state
        bufferPtr = silenceBuffer;
    }

    result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, bufferPtr, bufferSizeInBytes);
    assert(SL_RESULT_SUCCESS == result);
}

void Java_com_example_audio_generatetone_MainActivity_playTone(JNIEnv* env, jclass clazz){

    __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "Playing tone");
    buffersRemaining = BUFFERS_TO_PLAY;
}

// create the engine and output mix objects
void Java_com_example_audio_generatetone_MainActivity_createEngine(JNIEnv* env, jclass clazz)
{
    __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "Creating audio engine");

    SLresult result;

    // create engine
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    // realize the engine
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    // get the engine interface, which is needed in order to create other objects
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    // create output mix,
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0, NULL, NULL);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    // realize the output mix
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;
}


// create buffer queue audio player
void Java_com_example_audio_generatetone_MainActivity_createBufferQueueAudioPlayer(JNIEnv* env,
        jclass clazz, jint optimalFrameRate, jint optimalFramesPerBuffer)
{
    __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "Creating audio player with frame rate %d and frames per buffer %d",
                        optimalFrameRate, optimalFramesPerBuffer);

    // create the wave tables which we'll use as the audio signal source
    createWaveTables(optimalFramesPerBuffer);

    SLresult result;

    // configure the audio source (supply data through a buffer queue in PCM format)
    SLDataLocator_AndroidSimpleBufferQueue locator_bufferqueue_source;
    SLDataFormat_PCM format_pcm;
    SLDataSource audio_source;

    // source location
    locator_bufferqueue_source.locatorType = SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE;
    locator_bufferqueue_source.numBuffers = 1;

    // source format
    format_pcm.formatType = SL_DATAFORMAT_PCM;
    format_pcm.numChannels = CHANNELS;

    // Note: this shouldn't be called samplesPerSec it should be called *framesPerSec*
    // because when channels = 2 then there are 2 samples per frame.
    format_pcm.samplesPerSec = (SLuint32) optimalFrameRate * 1000;
    format_pcm.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
    format_pcm.containerSize = 16;
    format_pcm.channelMask = (CHANNELS == 1) ? SL_SPEAKER_FRONT_CENTER :
                             SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
    format_pcm.endianness = SL_BYTEORDER_LITTLEENDIAN;

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
    const SLInterfaceID interface_ids[2] = { SL_IID_ANDROIDSIMPLEBUFFERQUEUE, SL_IID_VOLUME };
    const SLboolean interfaces_required[2] = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };

    result = (*engineEngine)->CreateAudioPlayer(
        engineEngine,
        &bqPlayerObject,
        &audio_source,
        &audio_sink,
        2, // Number of interfaces
        interface_ids,
        interfaces_required
    );

    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    // realize the player
    result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    // get the play interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    // get the buffer queue interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE,
            &bqPlayerBufferQueue);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    // register callback on the buffer queue
    result = (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, NULL);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    // set the player's state to playing
    result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    // enqueue some silence
    result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, silenceBuffer, bufferSizeInBytes);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

}
