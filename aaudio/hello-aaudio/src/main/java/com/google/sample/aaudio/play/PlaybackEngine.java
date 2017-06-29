package com.google.sample.aaudio.play;
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

import com.google.sample.aaudio.common.AudioDeviceNotifier;

/**
 * TODO: This singleton feels kind of redundant as it simply provides native methods to the MainActivity, consider removing
 *
 */
public enum PlaybackEngine {

    INSTANCE;

    public static boolean selectOutputDevice(int deviceId){

        if (deviceId == AudioDeviceNotifier.AUTO_SELECT_DEVICE_ID){

        }

        return false;
    };

    /*
     * Loading Native lib(s)
     */
    static {
        System.loadLibrary("hello-aaudio");
    }

    /*
    * jni function implementations...
    */
    //private static native boolean native_createOutputStream();
    static native boolean createEngine();
    static native void deleteEngine();
    static native void setToneOn(boolean isToneOn);
    //public static native boolean start();
    //public static native void stop();
}
