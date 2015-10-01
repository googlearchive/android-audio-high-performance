package com.example.android.howie;

import android.content.Context;
import android.media.AudioManager;

/**
 * Created by ilewis on 9/25/15.
 */
public class HowieEngine {

    public static long init(Context ctx) {
        System.loadLibrary("howie");

        //Check for optimal output sample rate and buffer size
        AudioManager am = (AudioManager) ctx.getSystemService(Context
                .AUDIO_SERVICE);

        String frameRate = am.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE);
        String framesPerBuffer = am.getProperty(AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER);

        //Convert to ints
        int frameRateInt = Integer.parseInt(frameRate);
        int framesPerBufferInt = Integer.parseInt(framesPerBuffer);
        return create(frameRateInt, 16, 2, 0xffff, false, 1, 1,
                framesPerBufferInt);
    }

    private static native long create(
            int sampleRate,
            int bitsPerSample,      // not including padding
            int bytesPerSample,     // including padding
            int sampleMask,         // mask significant bits from padding
            boolean floatingPoint,  // true for IEEE float; signed int otherwise
            int channelCount,
            int samplesPerFrame,    // for interleaved multichannel,
                                    // should be same as channel count
            int framesPerBuffer     // determines granularity of callbacks
    );
    private static native void destroy(long engine);
}
