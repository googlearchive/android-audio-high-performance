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

class DeviceListEntry {

    private int mId;
    private String mName;

    DeviceListEntry(int deviceId, String deviceName){
        mId = deviceId;
        mName = deviceName;
    }

    public int getId() {
        return mId;
    }

    public String getName(){
        return mName;
    }

    public String toString(){
        return getName();
    }
}
