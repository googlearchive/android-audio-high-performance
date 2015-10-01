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

package com.example.ilewis.hellolowlatencyio;

import android.app.Activity;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.Range;
import android.view.MotionEvent;
import android.view.View;
import android.view.Menu;
import android.view.MenuItem;

import com.example.android.howie.HowieEngine;

public class MainActivity extends Activity {

    private static final double lowNote = -24;
    private static final double noteRange = 36;
    private static final double referenceFreq = 440;
    private static final double lowRes = .2f;
    private static final double resRange = .7f;

    private static final double noteBase = Math.pow(2.0, 1.0 / 12.0);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        System.loadLibrary("hello_low_latency_io");
        HowieEngine.init(this);
        stream = initStream();

        final View main = findViewById(R.id.main);
        main.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View view, MotionEvent motionEvent) {
                float x = motionEvent.getX() / (float) main.getWidth();
                float y = motionEvent.getY() / (float) main.getHeight();

                double note = y * noteRange + lowNote;
                double freq = referenceFreq * Math.pow(noteBase, note);
                setParams(
                        stream,
                        (float) (freq / 48000.0),
                        .95f,
                        x * 3.f
                );
                return true;
            }
        });
    }

    private long stream;

    private native long initStream();

    private native void setParams(long stream, float frequency, float resonance, float gain);
}
