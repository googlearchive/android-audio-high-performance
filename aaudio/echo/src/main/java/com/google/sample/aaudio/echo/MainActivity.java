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
import android.content.Context;
import android.content.pm.PackageManager;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioRecord;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

public class MainActivity extends Activity
        implements ActivityCompat.OnRequestPermissionsResultCallback {
    private static final int AUDIO_ECHO_REQUEST = 0;

    TextView status_view;
    Button   commandButton;
    boolean  supportRecording;
    boolean  isPlaying;
    boolean  nativeAAudioInitialized;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        status_view = (TextView)findViewById(R.id.statusView);
        commandButton = (Button)findViewById(R.id.button_start_capture);
        commandButton.setText(getString(R.string.StartEcho));

        isPlaying = false;
        nativeAAudioInitialized = false;

        queryNativeAudioParameters();

        // initialize native audio system
        updateNativeAudioUI();
    }
    @Override
    protected void onDestroy() {
        if (supportRecording) {
            if (isPlaying) {
                stop();
            }
            deleteEngine();
        }
        isPlaying = false;
        super.onDestroy();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    private void startEcho() {
        if (!supportRecording)
            return;

        if (!nativeAAudioInitialized) {
            supportRecording = createEngine();
            nativeAAudioInitialized = true;
        }

        if(!supportRecording || isPlaying) {
            return;
        }
        start();
        isPlaying = true;
        status_view.setText("Engine Echoing ....");
        commandButton.setText(R.string.StopEcho);
    }
    public void startEcho(View view) {
        if (!isPlaying) {
            if (ActivityCompat.checkSelfPermission(this, Manifest.permission.RECORD_AUDIO) !=
                    PackageManager.PERMISSION_GRANTED) {
                ActivityCompat.requestPermissions(
                        this,
                        new String[]{Manifest.permission.RECORD_AUDIO},
                        AUDIO_ECHO_REQUEST);
                status_view.setText("Requesting RECORD_AUDIO Permission...");
                return;
            }
            startEcho();
        } else {
            // currently echoing, asking to stop
            stop();
            updateNativeAudioUI();
            isPlaying = false;
            commandButton.setText(R.string.StartEcho);
        }
    }

    private void queryNativeAudioParameters() {
        AudioManager myAudioMgr = (AudioManager)
                getSystemService(Context.AUDIO_SERVICE);
        String nativeSampleRate  =  myAudioMgr.
                getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE);
        String nativeSampleBufSize =myAudioMgr.
                getProperty(AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER);
        int recBufSize = AudioRecord.getMinBufferSize(
                Integer.parseInt(nativeSampleRate),
                AudioFormat.CHANNEL_IN_MONO,
                AudioFormat.ENCODING_PCM_16BIT);
        // This could be done from native side too: input stream creation would fail
        // if not the system does not support recording. Just do it here for convenience
        supportRecording = true;
        if (recBufSize == AudioRecord.ERROR ||
            recBufSize == AudioRecord.ERROR_BAD_VALUE) {
            supportRecording = false;
            commandButton.setEnabled(false);
        }
    }
    private void updateNativeAudioUI() {
        if (!supportRecording) {
            status_view.setText("Error: Audio recording is not supported");
            return;
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

        if (grantResults.length != 1  ||
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


        // The callback runs on app's thread, so we are safe to resume the action
        startEcho();
    }

    /*
     * Loading our Libs
     */
    static {
        System.loadLibrary("echo");
    }

    /*
     * jni function implementations...
     */
    public static native boolean createEngine();
    public static native void deleteEngine();

    public static native boolean start();
    public static native boolean stop();
}
