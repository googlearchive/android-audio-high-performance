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

package com.google.sample.aaudio.play;

import android.app.Activity;
import android.content.Context;
import android.media.AudioManager;
import android.os.Bundle;
import android.support.v4.view.MotionEventCompat;
import android.view.MotionEvent;
import android.view.View;
import android.widget.AdapterView;
import android.widget.Spinner;
import android.widget.TextView;

import com.google.sample.aaudio.common.AudioDeviceAdapter;
import com.google.sample.aaudio.common.AudioDeviceListener;
import com.google.sample.aaudio.common.AudioDeviceNotifier;
import com.google.sample.aaudio.common.AudioDeviceListEntry;

import java.util.List;
import java.util.Locale;
import java.util.Timer;
import java.util.TimerTask;

public class MainActivity extends Activity {

    private static final long UPDATE_LATENCY_EVERY_MILLIS = 1000;
    private boolean mEngineCreated = false;
    private Spinner mPlaybackDeviceSpinner;
    private TextView mLatencyText;
    private Timer mLatencyUpdater;

    /*
     * Hook to user control to start / stop audio playback:
     *    touch-down: start, and keeps on playing
     *    touch-up: stop.
     * simply pass the events to native side.
     */
    @Override
    public boolean onTouchEvent(MotionEvent event) {
        int action = MotionEventCompat.getActionMasked(event);
        switch(action) {
            case (MotionEvent.ACTION_DOWN) :
                if (mEngineCreated)
                    PlaybackEngine.setToneOn(true);
                break;
            case (MotionEvent.ACTION_UP) :
                if (mEngineCreated)
                    PlaybackEngine.setToneOn(false);
                break;
        }
        return super.onTouchEvent(event);
    }
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mPlaybackDeviceSpinner = findViewById(R.id.playbackDevicesSpinner);
        mPlaybackDeviceSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> adapterView, View view, int i, long l) {
                PlaybackEngine.setAudioDeviceId(getPlaybackDeviceId());
            }

            @Override
            public void onNothingSelected(AdapterView<?> adapterView) {

            }
        });

        setupPlaybackDeviceNotifier();

        // initialize native audio system
        mEngineCreated = PlaybackEngine.create();

        // Periodically update the UI with the output stream latency
        mLatencyText = findViewById(R.id.latencyText);
        setupLatencyUpdater();
    }

    private void setupPlaybackDeviceNotifier() {

        AudioManager audioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        AudioDeviceNotifier playbackDeviceNotifier = new AudioDeviceNotifier(
                getResources(),
                audioManager,
                AudioManager.GET_DEVICES_OUTPUTS);

        playbackDeviceNotifier.registerListener(new AudioDeviceListener() {
            @Override
            public void onDevicesUpdated(List<AudioDeviceListEntry> deviceEntries) {
                AudioDeviceAdapter deviceAdapter =
                        new AudioDeviceAdapter(MainActivity.super.getBaseContext(),
                                deviceEntries);
                mPlaybackDeviceSpinner.setSelection(0); // Select first item in list
                mPlaybackDeviceSpinner.setAdapter(deviceAdapter);
            }
        });
    }

    private int getPlaybackDeviceId(){
        return ((AudioDeviceListEntry) mPlaybackDeviceSpinner.getSelectedItem()).getId();
    }

    private void setupLatencyUpdater() {

        //Update the latency every 1s
        TimerTask latencyUpdateTask = new TimerTask() {
            @Override
            public void run() {

                double latency = PlaybackEngine.getCurrentOutputLatencyMillis();
                final String latencyStr;
                if (latency >= 0){
                    latencyStr = String.format(Locale.getDefault(), "%.2fms", latency);
                } else {
                    latencyStr = "Unknown";
                }
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        mLatencyText.setText(getString(R.string.latency, latencyStr));
                    }
                });
            }
        };

        mLatencyUpdater = new Timer();
        mLatencyUpdater.schedule(latencyUpdateTask, 0, UPDATE_LATENCY_EVERY_MILLIS);
    }

    @Override
    protected void onDestroy() {
        if (mLatencyUpdater != null) mLatencyUpdater.cancel();
        PlaybackEngine.delete();
        super.onDestroy();
    }
}
