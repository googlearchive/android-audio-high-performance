---
layout: page
title: Audio Output Latency
permalink: /guides/audio-output-latency.html
site_nav_category_order: 120
is_site_nav_category2: true
site_nav_category: guides
---

{::options toc_levels="1,2,3"/}

* TOC
{:toc}

## Introduction
Many apps need to generate audio as quickly as possible following some input event (e.g. the user tapping on the screen). Common examples include:

- Digital Audio Workstations (DAWs)
- Video/Audio conferencing
- Synthesizers
- Drum machines
- Karaoke apps
- DJ mixing
- Audio effects

Whatever the purpose of your app this guide will help you achieve the lowest possible audio output latency.

## Prerequisites
Low latency audio is currently only supported using OpenSL ES and the Android NDK.

1. Download and install the [Android NDK](https://developer.android.com/tools/sdk/ndk/index.html)
2. Read the [OpenSL ES documentation]({{ site.baseurl }}/guides/opensl_es.html)

## Obtain a low latency track
To obtain the lowest latency you must supply audio data which matches the device’s optimal sample rate and buffer size. [More info](https://source.android.com/devices/audio/latency_design.html).

### Obtain the optimal sample rate
In Java you can obtain the optimal sample rate from AudioManager as follows:

    AudioManager am = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
    String frameRate = am.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE);
    int frameRateInt = Integer.parseInt(frameRate); // Convert to int
    if (frameRateInt == 0) frameRateInt = 44100; // Use a default value if property not found

Note: The sample rate refers to the rate of each stream. If your source audio has 2 channels
(stereo) then you will have 1 stream playing a pair of samples (frame) at `PROPERTY_OUTPUT_SAMPLE_RATE`.

### Use the optimal sample rate when creating your audio player
Once you have the optimal sample output rate you can supply it when creating your player using OpenSL ES:

    // create buffer queue audio player
    void Java_com_example_audio_generatetone_MainActivity_createBufferQueueAudioPlayer(JNIEnv* env,
           jclass clazz, jint sampleRate, jint framesPerBuffer)
    {
       ...
       // specify the audio source format
       SLDataFormat_PCM format_pcm;
       format_pcm.numChannels = 2;
       format_pcm.samplesPerSec = (SLuint32) sampleRate * 1000;
       ...
    }

Note: `samplesPerSec` refers to the *sample rate per channel in milli hertz* (1Hz = 1000mHz).


### Avoid adding output interfaces which involve signal processing
Only certain interfaces are supported by the fast mixer. These are:

- SL_IID_ANDROIDSIMPLEBUFFERQUEUE
- SL_IID_VOLUME
- SL_IID_MUTESOLO

The following interfaces are not allowed because they involve signal processing, and will cause your request for a fast track to be rejected:

- SL_IID_BASSBOOST
- SL_IID_EFFECTSEND
- SL_IID_ENVIRONMENTALREVERB
- SL_IID_EQUALIZER
- SL_IID_PLAYBACKRATE
- SL_IID_PRESETREVERB
- SL_IID_VIRTUALIZER
- SL_IID_ANDROIDEFFECT
- SL_IID_ANDROIDEFFECTSEND

So, when you create your player make sure you only add “fast” interfaces. Example source code:

    const SLInterfaceID interface_ids[2] = { SL_IID_ANDROIDSIMPLEBUFFERQUEUE, SL_IID_VOLUME };

### Test you’re using a low latency track
To verify that you have successfully obtained a low latency track run your app, then run the following command:

    adb shell ps | grep your_app_name

Make a note of your app’s process id. Now play some audio from your app. You have now have approximately 3 seconds to run the following command from the terminal:

    adb shell dumpsys media.audio_flinger

Scan for your process ID.  If you see an "F" under the Name column it is on a low latency track
(the "F" stands for "Fast track").

Example:



## Use the optimal buffer size when enqueuing audio data
You can obtain the optimal buffer size in a similar way to the optimal frame rate, using the AudioManager API:

    AudioManager am = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
    String framesPerBuffer = am.getProperty(AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER);
    int framesPerBufferInt = Integer.parseInt(framesPerBuffer); // Convert to int
    if (framesPerBufferInt == 0) framesPerBufferInt = 256; // Use default

The `PROPERTY_OUTPUT_FRAMES_PER_BUFFER` property indicates the number of audio frames which the HAL (Hardware Abstraction Layer) buffer can hold. You should construct your audio buffers so that they contain an exact multiple of this number. Failure to do this will result in callbacks at irregular times, greatly increasing the chance of jitter.

For example, on a Nexus 5 device the buffer size is 240 frames on build LMY48M so for this device/build combination you should ideally supply a single buffer containing 240 frames during every queue callback. If 240 frames is too small to maintain a reliable audio stream you should use 480 frames (at the cost of increased latency) and so on, increasing buffer size by 240 until you reach a good trade-off between latency and reliability.

HAL buffer sizes differ across devices and across Android builds, this is why it’s so important to use the API to determine buffer size rather than using a hardcoded value.

## Avoid warm-up latency
When you enqueue audio data for the first time it takes the system a small, but still significant, amount of time for the device audio circuit to warm up. To avoid this warm up latency you should enqueue buffers of audio data containing silence.

At the point when audio should be produced you can switch to enqueuing buffers containing real audio data.

Example code to create a buffer of silence:


    #define CHANNELS 1
    static short* silenceBuffer;
    int numSamples = frames * CHANNELS;
    silenceBuffer = malloc(sizeof(*silenceBuffer) * numSamples);
    for (i = 0; i < numSamples; i++) silenceBuffer[i] = 0;

Note: Constantly outputting audio incurs significant power consumption. Ensure you stop the output in the onPause() method. Also consider pausing the silent output after some period of user inactivity.
