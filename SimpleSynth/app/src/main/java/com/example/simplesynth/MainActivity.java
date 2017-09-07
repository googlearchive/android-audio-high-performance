/*
 * Copyright 2016 The Android Open Source Project
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

package com.example.simplesynth;


import android.annotation.TargetApi;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.os.Build;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.WindowManager;
import android.widget.CompoundButton;
import android.widget.SeekBar;
import android.widget.Switch;
import android.widget.TextView;

import java.util.Timer;
import java.util.TimerTask;


public class MainActivity extends AppCompatActivity {

    private static final int NUM_BUFFERS = 2;
    private static final int UPDATE_UNDERRUNS_EVERY_MS = 1000;
    private static final float VARIABLE_LOAD_LOW_PERCENTAGE = 0.1F;
    private static final int VARIABLE_LOAD_LOW_DURATION = 2000;
    private static final int VARIABLE_LOAD_HIGH_DURATION = 2000;
    public static final int MAXIMUM_WORK_CYCLES = 500000;
    private static final int SEEKBAR_STEPS = 100;
    private static final float WORK_CYCLES_PER_STEP = MAXIMUM_WORK_CYCLES / SEEKBAR_STEPS;
    private static final String PREFERENCES_KEY_WORK_CYCLES = "work_cycles";

    private static int workCycles = 0;

    static {
        System.loadLibrary("SimpleSynth");
    }

    private TextView mDeviceInfoText, mWorkCyclesText;
    private AudioTrack mAudioTrack;
    private VariableLoadGenerator mLoadThread;
    private SharedPreferences mSettings;

    // Native methods
    private static native void native_createEngine(int apiLevel);
    private static native AudioTrack native_createAudioPlayer(int frameRate,
                                                        int framesPerBuffer,
                                                        int numBuffers,
                                                        int[] exclusiveCores);
    private static native void native_noteOn();
    private static native void native_noteOff();
    private static native void native_setWorkCycles(int workCycles);
    private static native void native_setLoadStabilizationEnabled(boolean isEnabled);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        // Lock to portrait to avoid onCreate being called more than once
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);

        // Load any previously saved values
        mSettings = getPreferences(MODE_PRIVATE);
        workCycles = mSettings.getInt(PREFERENCES_KEY_WORK_CYCLES, workCycles);

        initDeviceInfoUI();
        initPerformanceConfigurationUI();

        setSustainedPerformanceMode();

        // Create a synthesizer whose callbacks are affined to the exclusive core(s) (if available)
        int exclusiveCores[] = getExclusiveCores();
        mAudioTrack = createSynth(exclusiveCores);

        // Update the UI when there are underruns
        initUnderrunUpdater();

        setWorkCycles(workCycles);
    }

    @Override
    protected void onStop(){
        super.onStop();
        SharedPreferences.Editor editor = mSettings.edit();
        editor.putInt(PREFERENCES_KEY_WORK_CYCLES, workCycles);
        editor.apply();
    }

    private void initDeviceInfoUI(){

        mDeviceInfoText = (TextView) findViewById(R.id.deviceInfoText);

        String deviceInfo = "";
        deviceInfo += "API " + Build.VERSION.SDK_INT + "\n";
        deviceInfo += "Build " + Build.ID + " " +
                Build.VERSION.CODENAME + " " +
                Build.VERSION.RELEASE + " " +
                Build.VERSION.INCREMENTAL + "\n";

        PackageManager pm = getPackageManager();
        boolean claimsLowLatencyFeature = pm.hasSystemFeature(PackageManager.FEATURE_AUDIO_LOW_LATENCY);
        deviceInfo += "Hardware flag audio.low_latency: " + claimsLowLatencyFeature + "\n";

        String claimsProFeature = "Not supported. Only available on API " +
                Build.VERSION_CODES.M + "+";
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M){
            claimsProFeature = (pm.hasSystemFeature(PackageManager.FEATURE_AUDIO_PRO)) ?
                    "true" : "false";
        }
        deviceInfo += "Hardware flag audio.pro: " + claimsProFeature + "\n";

        mDeviceInfoText.append(deviceInfo);
    }

    private void setSustainedPerformanceMode(){

        mDeviceInfoText.append("Sustained performance mode: ");

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            getWindow().setSustainedPerformanceMode(true);
            mDeviceInfoText.append("On");
        } else {
            mDeviceInfoText.append("Not supported. Only available on API " +
                    Build.VERSION_CODES.N + "+");
        }
        mDeviceInfoText.append("\n");
    }

    // Obtain CPU cores which are reserved for the foreground app
    private int[] getExclusiveCores(){
        int exclusiveCores[] = {};

        mDeviceInfoText.append("Exclusive core ids: ");

        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.N) {
            mDeviceInfoText.append("Not supported. Only available on API " +
                    Build.VERSION_CODES.N + "+");
        } else {
            exclusiveCores = android.os.Process.getExclusiveCores();
            for (int i : exclusiveCores){
                mDeviceInfoText.append(i + " ");
            }
        }
        mDeviceInfoText.append("\n");

        return exclusiveCores;
    }

    private AudioTrack createSynth(int[] exclusiveCores){

        // Obtain the optimal output sample rate and buffer size
        AudioManager am = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        String frameRateString = am.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE);
        String framesPerBufferString =
                am.getProperty(AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER);
        int mFrameRate = Integer.parseInt(frameRateString);
        int mFramesPerBuffer = Integer.parseInt(framesPerBufferString);

        native_createEngine(Build.VERSION.SDK_INT);

        return native_createAudioPlayer(
                mFrameRate, mFramesPerBuffer, NUM_BUFFERS, exclusiveCores);
    }

    private void initPerformanceConfigurationUI(){

        Switch testToneSwitch = (Switch) findViewById(R.id.testToneSwitch);
        testToneSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                if (b){
                    native_noteOn();
                } else {
                    native_noteOff();
                }
            }
        });

        Switch variableLoadSwitch = (Switch) findViewById(R.id.variableLoadSwitch);
        variableLoadSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                if (b) {
                    if (mLoadThread == null){
                        mLoadThread = new VariableLoadGenerator();
                    }
                    mLoadThread.start();
                } else {
                    if (mLoadThread != null){
                        mLoadThread.terminate();
                        mLoadThread = null;
                    }
                }

            }
        });

        Switch stabilizedLoadSwitch = (Switch) findViewById(R.id.stabilizedLoadSwitch);
        stabilizedLoadSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                native_setLoadStabilizationEnabled(b);
            }
        });

        mWorkCyclesText = (TextView) findViewById(R.id.workCyclesText);

        SeekBar workCyclesSeekBar = (SeekBar) findViewById(R.id.workCycles);
        workCyclesSeekBar.setProgress((int)(workCycles / WORK_CYCLES_PER_STEP));

        workCyclesSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                workCycles = (int)(progress * WORK_CYCLES_PER_STEP);
                setWorkCycles(workCycles);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });
    }

    private void setWorkCycles(final int workCycles){

        native_setWorkCycles(workCycles);

        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mWorkCyclesText.setText("Work cycles " + workCycles);
            }
        });
    }

    private void initUnderrunUpdater(){

        final TextView mUnderrunCountText = (TextView) findViewById(R.id.underrunCountText);

        if (mAudioTrack == null){
            mUnderrunCountText.setText("Underruns: Feature not supported on API <"+
                    Build.VERSION_CODES.N +
                    " see README for more");
        } else {

            Timer underrunUpdater = new Timer();
            underrunUpdater.schedule(new TimerTask() {
                @Override
                @TargetApi(Build.VERSION_CODES.N)
                public void run() {
                    final int underrunCount;
                    underrunCount = ((AudioTrack) mAudioTrack).getUnderrunCount();
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            mUnderrunCountText.setText("Underruns: " + underrunCount);
                        }
                    });
                }
            },0, UPDATE_UNDERRUNS_EVERY_MS);
        }
    }

    private class VariableLoadGenerator extends Thread {

        private boolean isRunning = false;

        synchronized void terminate(){
            isRunning = false;
        }

        @Override
        public void run(){

            isRunning = true;

            try {
                while (isRunning) {

                    int lowCycles = (int)(MainActivity.workCycles * VARIABLE_LOAD_LOW_PERCENTAGE);
                    MainActivity.this.setWorkCycles(lowCycles);
                    Thread.sleep(VARIABLE_LOAD_LOW_DURATION);
                    MainActivity.this.setWorkCycles(MainActivity.workCycles);
                    Thread.sleep(VARIABLE_LOAD_HIGH_DURATION);
                }
            } catch (InterruptedException e){
                e.printStackTrace();
            }
        }
    }
}
