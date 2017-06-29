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

import android.app.Activity;
import android.content.Context;
import android.media.AudioDeviceCallback;
import android.media.AudioDeviceInfo;
import android.media.AudioManager;
import android.util.Log;
import android.widget.Spinner;

import java.util.List;
import java.util.Vector;

/**
 * Responsible for displaying and updating the list of possible audio devices for
 * recording and playback
 */
class AudioDeviceNotifier extends AudioDeviceCallback {

    private AudioDeviceListener listener;

    private static final String TAG = AudioDeviceNotifier.class.getName();
    private static final int AUTO_SELECT_DEVICE_ID = 0; // Same value as AAUDIO_UNSPECIFIED
    private static final String AUTO_SELECT_DEVICE_NAME = "Auto select";

    private AudioManager mAudioManager;
    private int mDeviceType;
    private int selectedDeviceId = AUTO_SELECT_DEVICE_ID;

    public AudioDeviceNotifier(Context context, int getDeviceType) {

        mAudioManager = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
        mAudioManager.registerAudioDeviceCallback(this, null);
        mDeviceType = getDeviceType;
        updateListener();
    }

    public void registerListener(AudioDeviceListener listener){
        this.listener = listener;
    }

    public void unregisterListener(){
        this.listener = null;
    }

    public void updateListener() {

        if (listener != null){
            List<DeviceListEntry> deviceList = getDeviceList();
            listener.onDevicesUpdated(deviceList);
        }
    }

    private List<DeviceListEntry> getDeviceList(){

        AudioDeviceInfo[] devices = mAudioManager.getDevices(mDeviceType);
        List<DeviceListEntry> listEntries = new Vector<>();

        // Add "Auto select" as the first entry
        listEntries.add(new DeviceListEntry(AUTO_SELECT_DEVICE_ID, AUTO_SELECT_DEVICE_NAME));

        for (AudioDeviceInfo info : devices) {

            listEntries.add(new DeviceListEntry(
                    info.getId(),
                    info.getProductName() + " " +
                            AudioDeviceInfoConverter.toReadableString(info.getType())));
        }

        return listEntries;
    }

    private void printAudioDevices(AudioDeviceInfo[] deviceInfos) {
        for (AudioDeviceInfo adi : deviceInfos) {
            Log.d(TAG,
                    adi.getProductName() +
                            " " +
                            AudioDeviceInfoConverter.toReadableString(adi.getType()));
        }
    }

    @Override
    public void onAudioDevicesAdded(AudioDeviceInfo[] addedDevices) {
        if (haveDevicesChanged(addedDevices)) updateListener();
    }

    @Override
    public void onAudioDevicesRemoved(AudioDeviceInfo[] removedDevices) {
        if (haveDevicesChanged(removedDevices)) updateListener();
    }

    private boolean haveDevicesChanged(AudioDeviceInfo[] changedDevices){

        for (AudioDeviceInfo device : changedDevices){
            if ((mDeviceType == AudioManager.GET_DEVICES_INPUTS && device.isSource()) ||
                    (mDeviceType == AudioManager.GET_DEVICES_OUTPUTS && device.isSink())){

                return true;
            }
        }
        return false;
    }
}
