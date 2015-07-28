package com.example.audio.generatetone;

import android.content.Context;
import android.content.pm.PackageManager;
import android.media.AudioManager;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;

public class MainActivity extends AppCompatActivity {

    static {
        System.loadLibrary("generate_tone");
    }

    public static native void playTone();
    public static native void createEngine();
    public static native void createBufferQueueAudioPlayer(int sampleRate, int framesPerBuffer);

    public static final String TAG = MainActivity.class.getName();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);


        //Check for low latency feature
        PackageManager pm = getPackageManager();
        boolean claimsFeature = pm.hasSystemFeature(PackageManager.FEATURE_AUDIO_LOW_LATENCY);
        Log.d(TAG, "Has low latency audio feature? " + claimsFeature);

        //Check for optimal output sample rate and buffer size
        AudioManager am = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        String sampleRate = am.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE);
        String framesPerBuffer = am.getProperty(AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER);
        Log.d(TAG, "Optimal sample rate is: "+sampleRate);
        Log.d(TAG, "Optimal frames per buffer is: "+framesPerBuffer);

        //Convert to ints
        int sampleRateInt = Integer.parseInt(sampleRate);
        int framesPerBufferInt = Integer.parseInt(framesPerBuffer);

        //Create the audio engine
        createEngine();
        createBufferQueueAudioPlayer(sampleRateInt, framesPerBufferInt);

        Log.d(TAG, "Audio engine created");

        setContentView(R.layout.activity_main);

        View layoutMain = findViewById(R.id.layoutMain);
        layoutMain.setOnTouchListener(new View.OnTouchListener() {

            @Override
            public boolean onTouch(View v, MotionEvent event) {

                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    Log.d(TAG, "Playing sound");
                    playTone();
                }

                return true;
            }
        });
    }
}
