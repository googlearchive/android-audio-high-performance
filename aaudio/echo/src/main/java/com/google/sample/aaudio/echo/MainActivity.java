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
 */

package com.google.sample.aaudio.echo;

import android.Manifest;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.media.AudioManager;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import java.util.List;

/**
 * TODO: Remove xml for settings menu
 * TODO: Update member variable names
 */

public class MainActivity extends Activity
        implements ActivityCompat.OnRequestPermissionsResultCallback {

    private static final String TAG = MainActivity.class.getName();
    private static final int AUDIO_ECHO_REQUEST = 0;

    private TextView status_view;
    private Button toggleEchoButton;
    private AudioDeviceNotifier recordingDeviceNotifier;
    private AudioDeviceNotifier playbackDeviceNotifier;
    private EchoManager echoManager;
    private Spinner recordingDeviceSpinner;
    private Spinner playbackDeviceSpinner;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        echoManager = EchoManager.getInstance(this);

        status_view = (TextView) findViewById(R.id.statusView);
        toggleEchoButton = (Button) findViewById(R.id.button_start_capture);
        toggleEchoButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                toggleEcho();
            }
        });
        toggleEchoButton.setText(getString(R.string.StartEcho));

        recordingDeviceSpinner = findViewById(R.id.recordingDevicesSpinner);
        playbackDeviceSpinner = findViewById(R.id.playbackDevicesSpinner);

        recordingDeviceNotifier = new AudioDeviceNotifier(this,
                AudioManager.GET_DEVICES_INPUTS);
        playbackDeviceNotifier = new AudioDeviceNotifier(this,
                AudioManager.GET_DEVICES_OUTPUTS);

        recordingDeviceNotifier.registerListener(new AudioDeviceListener() {
            @Override
            public void onDevicesUpdated(List<DeviceListEntry> deviceEntries) {
                AudioDeviceAdapter deviceAdapter =
                        new AudioDeviceAdapter(MainActivity.super.getBaseContext(),
                                R.layout.audio_devices, deviceEntries);
                recordingDeviceSpinner.setSelection(0); // Select first item in list
                recordingDeviceSpinner.setAdapter(deviceAdapter);
            }
        });

        playbackDeviceNotifier.registerListener(new AudioDeviceListener() {
            @Override
            public void onDevicesUpdated(List<DeviceListEntry> deviceEntries) {
                AudioDeviceAdapter deviceAdapter =
                        new AudioDeviceAdapter(MainActivity.super.getBaseContext(),
                                R.layout.audio_devices, deviceEntries);
                playbackDeviceSpinner.setSelection(0); // Select first item in list
                playbackDeviceSpinner.setAdapter(deviceAdapter);
            }
        });

        // initialize native audio system
        updateNativeAudioUI();
    }

    @Override
    protected void onDestroy() {
        echoManager.destroy();
        super.onDestroy();
    }

    public void toggleEcho() {

        if (echoManager.isPlaying()) {
            Log.d(TAG, "Playing, attempting to stop");
            // currently echoing, asking to stop
            echoManager.stop();
            updateNativeAudioUI();
            toggleEchoButton.setText(R.string.StartEcho);

        } else {
            Log.d(TAG, "Attempting to start");

            if (checkPermissions()) return;

            if (echoManager.start(getRecordingDeviceId(), getPlaybackDeviceId())){
                status_view.setText("Engine Echoing ....");
                toggleEchoButton.setText(R.string.StopEcho);
            } else {
                status_view.setText("Failed to start");
            }
        }
    }

    // TODO: Add start after permissions granted

    private int getRecordingDeviceId(){
        return ((DeviceListEntry)recordingDeviceSpinner.getSelectedItem()).getId();
    }

    private int getPlaybackDeviceId(){
        return ((DeviceListEntry)playbackDeviceSpinner.getSelectedItem()).getId();
    }

    private boolean checkPermissions() {
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.RECORD_AUDIO) !=
                PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(
                    this,
                    new String[]{Manifest.permission.RECORD_AUDIO},
                    AUDIO_ECHO_REQUEST);
            return true;
        }
        return false;
    }

    private void updateNativeAudioUI() {

        if (!echoManager.isRecordingSupported()){
            toggleEchoButton.setEnabled(false);
            status_view.setText("Error: Audio recording is not supported");
        }

        status_view.setText("Warning:\n" +
                "    This sample must be run with headphone connected\n" +
                "     -- otherwise the sound could be pretty DISTURBING.\n");

    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions,
                                           @NonNull int[] grantResults) {
        /*
         * if any permission failed, the sample could not play
         */
        if (AUDIO_ECHO_REQUEST != requestCode) {
            super.onRequestPermissionsResult(requestCode, permissions, grantResults);
            return;
        }

        if (grantResults.length != 1 ||
                grantResults[0] != PackageManager.PERMISSION_GRANTED) {
            /*
             * When user denied the permission, throw a Toast to prompt that RECORD_AUDIO
             * is necessary; on UI, we display the current status as permission was denied so
             * user know what is going on.
             * This application go back to the original state: it behaves as if the button
             * was not clicked. The assumption is that user will re-click the "start" button
             * (to retry), or shutdown the app in normal way.
             */
            status_view.setText("Error: Permission for RECORD_AUDIO was denied");
            Toast.makeText(getApplicationContext(),
                    getString(R.string.NeedRecordAudioPermission),
                    Toast.LENGTH_SHORT)
                    .show();
            return;
        }

        /*
         * When permissions are granted, we prompt the user the status. User would
         * re-try the "start" button to perform the normal operation. This saves us the extra
         * logic in code for async processing of the button listener.
         */
        status_view.setText("RECORD_AUDIO permission granted, touch " +
                getString(R.string.StartEcho) + " to begin");

    }
}
