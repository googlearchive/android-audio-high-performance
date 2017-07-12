package com.google.sample.aaudio.common;
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

import android.content.res.Resources;
import android.media.AudioDeviceCallback;
import android.media.AudioDeviceInfo;
import android.media.AudioManager;
import android.util.Log;

import java.util.List;
import java.util.Vector;

/**
 * Responsible for displaying and updating the list of possible audio devices for
 * recording and playback
 */
public class AudioDeviceNotifier extends AudioDeviceCallback {

    /**
     * The following constant is used to define a default {@link AudioDeviceListEntry}.
     * When this entry is selected by the user it means "automatically select an audio device".
     * There is a constant in the AAudio C API (in aaudio/AAudio.h) which is AAUDIO_UNSPECIFIED = 0
     * which is used the same way. This is why we set AUTO_SELECT_DEVICE_ID to 0.
     */
    private static final int AUTO_SELECT_DEVICE_ID = 0;
    private static final String TAG = AudioDeviceNotifier.class.getSimpleName();
    private String mAutoSelectDeviceDescription;
    private AudioDeviceListener mListener;
    private AudioManager mAudioManager;
    private int mDeviceDirectionType;

    /**
     * Constructs an object which will notify its {@link AudioDeviceListener mListener} when audio
     * devices of the given type (input or output) are connected or disconnected from the Android
     * device.
     *
     * Example: If you want to know when headphones (or other output audio devices) are connected:
     *
     * outputNotifier = new AudioDeviceNotifier(context, AudioManager.GET_DEVICES_OUTPUTS)
     * outputNotifier.setListener(new AudioDeviceListener(){...});
     *
     * @param resources The {@link Resources resources} for the application
     * @param deviceDirectionType The device direction which we want to monitor changes for i.e.
     *                            either {@link AudioManager#GET_DEVICES_INPUTS} for recording
     *                            devices or {@link AudioManager#GET_DEVICES_OUTPUTS} for
     *                            playback devices
     */
    public AudioDeviceNotifier(Resources resources,
                               AudioManager audioManager,
                               int deviceDirectionType) {

        mAutoSelectDeviceDescription = resources.getString(R.string.auto_select);
        mAudioManager = audioManager;
        mAudioManager.registerAudioDeviceCallback(this, null);
        mDeviceDirectionType = deviceDirectionType;
        updateListener();
    }

    public void registerListener(AudioDeviceListener listener){
        this.mListener = listener;
    }

    public void unregisterListener(){
        this.mListener = null;
    }

    private void updateListener() {
        if (mListener != null){
            List<AudioDeviceListEntry> deviceList = getDeviceList();
            mListener.onDevicesUpdated(deviceList);
        }
    }

    private List<AudioDeviceListEntry> getDeviceList(){

        Log.d(TAG, "Obtaining " + ((mDeviceDirectionType == AudioManager.GET_DEVICES_INPUTS) ?
                "input" : "output") + " devices");
        AudioDeviceInfo[] devices = mAudioManager.getDevices(mDeviceDirectionType);
        List<AudioDeviceListEntry> listEntries = new Vector<>();

        // Add "Auto select" as the first entry
        listEntries.add(new AudioDeviceListEntry(AUTO_SELECT_DEVICE_ID,
                mAutoSelectDeviceDescription));

        for (AudioDeviceInfo info : devices) {

            Log.d(TAG, AudioDeviceInfoConverter.toString(info));

            listEntries.add(new AudioDeviceListEntry(
                    info.getId(),
                    info.getProductName() + " " +
                            AudioDeviceInfoConverter.typeToString(info.getType())));
        }

        return listEntries;
    }

    @Override
    public void onAudioDevicesAdded(AudioDeviceInfo[] addedDevices) {
        if (mListener != null && haveDevicesChanged(addedDevices)) updateListener();
    }

    @Override
    public void onAudioDevicesRemoved(AudioDeviceInfo[] removedDevices) {
        if (mListener != null && haveDevicesChanged(removedDevices)) updateListener();
    }

    private boolean haveDevicesChanged(AudioDeviceInfo[] changedDevices){

        for (AudioDeviceInfo device : changedDevices){
            if ((mDeviceDirectionType == AudioManager.GET_DEVICES_INPUTS && device.isSource()) ||
                    (mDeviceDirectionType == AudioManager.GET_DEVICES_OUTPUTS && device.isSink())){

                return true;
            }
        }
        return false;
    }
}
