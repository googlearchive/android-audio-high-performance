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
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.widget.Spinner;

import com.google.sample.aaudio.common.AudioDeviceAdapter;
import com.google.sample.aaudio.common.AudioDeviceListener;
import com.google.sample.aaudio.common.AudioDeviceNotifier;
import com.google.sample.aaudio.common.DeviceListEntry;

import java.util.List;

public class MainActivity extends Activity {
    boolean engineCreated = false;

    Spinner playbackDeviceSpinner;

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
                if (engineCreated)
                    PlaybackEngine.setToneOn(true);
                break;
            case (MotionEvent.ACTION_UP) :
                if (engineCreated)
                    PlaybackEngine.setToneOn(false);
                break;
        }
        return super.onTouchEvent(event);
    }
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        playbackDeviceSpinner = findViewById(R.id.playbackDevicesSpinner);
        AudioDeviceNotifier playbackDeviceNotifier = new AudioDeviceNotifier(this,
                AudioManager.GET_DEVICES_OUTPUTS);

        playbackDeviceNotifier.registerListener(new AudioDeviceListener() {
            @Override
            public void onDevicesUpdated(List<DeviceListEntry> deviceEntries) {
                AudioDeviceAdapter deviceAdapter =
                        new AudioDeviceAdapter(MainActivity.super.getBaseContext(),
                                deviceEntries);
                playbackDeviceSpinner.setSelection(0); // Select first item in list
                playbackDeviceSpinner.setAdapter(deviceAdapter);
            }
        });

        // initialize native audio system
        engineCreated = PlaybackEngine.createEngine();
    }

    private int getPlaybackDeviceId(){
        return ((DeviceListEntry)playbackDeviceSpinner.getSelectedItem()).getId();
    }

    @Override
    protected void onDestroy() {
        PlaybackEngine.deleteEngine();
        super.onDestroy();
    }



}
