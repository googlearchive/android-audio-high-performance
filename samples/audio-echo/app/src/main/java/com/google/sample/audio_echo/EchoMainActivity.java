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
 *
 */
package com.google.sample.audio_echo;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.widget.CompoundButton;
import android.widget.ToggleButton;

public class EchoMainActivity extends AppCompatActivity {
    private   long streamId;
    private   ToggleButton toggle;
    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_echo_main);

        System.loadLibrary("audio-echo");

        toggle = (ToggleButton) findViewById(R.id.toggleEcho);
        toggle.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                startEcho(streamId, isChecked ? true : false);
            }
        });
    }

    @Override
    protected void onPause() {
        toggle.setChecked(false);
        destroyStream(streamId);
        super.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        streamId = createStream();
    }

    native public long createStream();
    native public void destroyStream(long streamId);
    native public void startEcho(long stream, boolean start);
}
