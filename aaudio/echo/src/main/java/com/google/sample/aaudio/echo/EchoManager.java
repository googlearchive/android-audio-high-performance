package com.google.sample.aaudio.echo;
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

import android.content.Context;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioRecord;
import android.util.Log;

public class EchoManager {

    private static final String TAG = EchoManager.class.getSimpleName();

    private boolean isRecordingSupported;
    private boolean isPlaying = false;
    private boolean nativeAAudioInitialized = false;
    private static EchoManager instance = null;
    private Context context;

    /*
     * Loading our Libs
     */
    static {
        System.loadLibrary("echo");
    }

    /*
     * jni function implementations...
     */
    public static native boolean native_createEngine();

    public static native void native_deleteEngine();

    public static native boolean native_start();

    public static native boolean native_stop();

    private EchoManager(Context context){
        this.context = context.getApplicationContext();
        checkRecordingSupported();
    }

    public static EchoManager getInstance(Context context){
        if (instance == null){
            instance = new EchoManager(context);
        }
        return instance;
    }

    /**
     * Start playback
     *
     * @return true if started, false otherwise
     */
    public boolean start(int recordingDeviceId, int playbackDeviceId){

        Log.d(TAG, "Attempting to start with recording id " + recordingDeviceId + " and playback" +
                "device id " + playbackDeviceId);

        if (!isRecordingSupported)
            return false;

        if (!nativeAAudioInitialized) {
            isRecordingSupported = native_createEngine();
            nativeAAudioInitialized = true;
        }

        if (!isRecordingSupported || isPlaying) {
            return false;
        }
        native_start();
        isPlaying = true;
        return true;
    }

    private void checkRecordingSupported(){
        AudioManager myAudioMgr = (AudioManager)
                context.getSystemService(Context.AUDIO_SERVICE);
        String nativeSampleRate = myAudioMgr.
                getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE);
        String nativeSampleBufSize = myAudioMgr.
                getProperty(AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER);
        int recBufSize = AudioRecord.getMinBufferSize(
                Integer.parseInt(nativeSampleRate),
                AudioFormat.CHANNEL_IN_MONO,
                AudioFormat.ENCODING_PCM_16BIT);
        // This could be done from native side too: input stream creation would fail
        // if not the system does not support recording. Just do it here for convenience
        isRecordingSupported = true;
        if (recBufSize == AudioRecord.ERROR ||
                recBufSize == AudioRecord.ERROR_BAD_VALUE) {
            isRecordingSupported = false;
        }
    }

    public boolean isRecordingSupported() {
        return isRecordingSupported;
    }

    public boolean isPlaying() {
        return isPlaying;
    }

    public boolean stop(){
        native_stop();
        isPlaying = false;
        return true;
    }


    public void destroy(){
        if (isRecordingSupported) {
            if (isPlaying) {
                stop();
            }
            native_deleteEngine();
        }
        isPlaying = false;
    }
}
