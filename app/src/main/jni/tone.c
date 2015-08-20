#include <assert.h>
#include <jni.h>

// for native audio
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

// for native asset manager
#include <sys/types.h>

// logging
#include <android/log.h>
#define APPNAME "GenerateTone"

// engine interfaces
static SLObjectItf engineObject = NULL;
static SLEngineItf engineEngine;

// output mix interfaces
static SLObjectItf outputMixObject = NULL;

// buffer queue player interfaces
static SLObjectItf bqPlayerObject = NULL;
static SLPlayItf bqPlayerPlay;
static SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;

// synthesized square wave
// NOTE: Changing this to 128 results in an output of clicks
#define SQUARE_FRAMES 128
static short squareBuffer[SQUARE_FRAMES];
#define MAXIMUM_AMPLITUDE_VALUE 32767

// how many times to play the square wave buffer (so we can actually hear it)
#define BUFFERS_TO_PLAY 200

static SLuint32 bufferSize; // Amount of frames to feed to the player in each callback
static unsigned buffersRemaining = BUFFERS_TO_PLAY;

// synthesize a mono square wave and place it into a buffer (called automatically on load)
__attribute__((constructor)) static void onDlOpen(void)
{
    unsigned j;
    int sign = 1;
    for (j = 0; j < SQUARE_FRAMES; ++j) {

        /**
         * Changing the sign every 64 frames gives us a complete square wave every 128 frames.
         * At a sample rate of 48000Hz this gives us a square wave of 375Hz
         **/
        if (j % 64 == 0) sign = sign * -1;
        squareBuffer[j] = MAXIMUM_AMPLITUDE_VALUE * sign;
        __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "Square wave %d %d", j, squareBuffer[j]);
    }
}

// this callback handler is called every time a buffer finishes playing
void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
    __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "In bqPlayerCallback");

    assert(bq == bqPlayerBufferQueue);
    assert(NULL == context);

    __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "Buffers remaining %d", buffersRemaining);

    if (buffersRemaining > 0) {

        SLresult result;

        // enqueue another buffer
        __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "Enqueuing %d frames to be played. First frame value %d", bufferSize, *squareBuffer);

        buffersRemaining--;

        short* bufferPtr = squareBuffer;
        result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, bufferPtr, bufferSize);
        assert(SL_RESULT_SUCCESS == result);
    }
}

void Java_com_example_audio_generatetone_MainActivity_playTone(JNIEnv* env, jclass clazz){

    __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "Playing tone");

    buffersRemaining = BUFFERS_TO_PLAY;
    buffersRemaining--;

    // Ignore the ideal buffer size and just send the whole wave file in at once
    bufferSize = SQUARE_FRAMES;

    // Enqueue the first buffer
    short* bufferPtr = squareBuffer;
    SLresult result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, bufferPtr, bufferSize);
    assert(result == SL_RESULT_SUCCESS);

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
        jclass clazz, jint sampleRate, jint framesPerBuffer)
{

    __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "Creating audio player with sample rate %d and buffer size %d ", sampleRate, framesPerBuffer);

    // framesPerBuffer represents the *ideal* buffer size according to that reported by the hardware
    // We'll ignore this now since on Nexus 9 we can just hardcode it to 128.
    // bufferSize = framesPerBuffer;

    SLresult result;

    // configure the audio source (supply data through a buffer queue in PCM format)
    SLDataLocator_BufferQueue locator_bufferqueue_source;
    SLDataFormat_PCM format_pcm;
    SLDataSource audio_source;

    // source location
    locator_bufferqueue_source.locatorType = SL_DATALOCATOR_BUFFERQUEUE;
    locator_bufferqueue_source.numBuffers = 1; // 2 for double buffering

    // source format
    format_pcm.formatType = SL_DATAFORMAT_PCM;
    format_pcm.numChannels = 1;
    format_pcm.samplesPerSec = (SLuint32) sampleRate * 1000;
    format_pcm.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
    format_pcm.containerSize = 16;
    format_pcm.numChannels = 1;
    format_pcm.channelMask = SL_SPEAKER_FRONT_CENTER;
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
    const SLInterfaceID interface_ids[1] = { SL_IID_BUFFERQUEUE };
    const SLboolean interfaces_required[1] = { SL_BOOLEAN_TRUE };

    result = (*engineEngine)->CreateAudioPlayer(
        engineEngine, //engine
        &bqPlayerObject, //player object
        &audio_source, //input
        &audio_sink, //output
        1, // Number of interfaces
        interface_ids, // The interfaces
        interfaces_required // Whether the interfaces are required
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
}
