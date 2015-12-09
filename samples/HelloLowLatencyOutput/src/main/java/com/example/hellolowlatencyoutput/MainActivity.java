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

package com.example.hellolowlatencyoutput;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.TextView;

import com.example.android.howie.HowieEngine;

public class MainActivity extends Activity {


  private long streamId;

  public static native long initPlayback();

  public static native void playTone(long stream);

  public static native void stopTone(long stream);

  public static native void startStream(long stream);

  public static native void stopStream(long stream);

  public static native void destroyPlayback(long stream);

  public static final String TAG = MainActivity.class.getName();

  private TextView textLog;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    System.loadLibrary("hello_low_latency_output");

    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_main);

    HowieEngine.init(this);
    streamId = initPlayback();

    View layoutMain = findViewById(R.id.layoutMain);
    layoutMain.setOnTouchListener(new View.OnTouchListener() {

      @Override
      public boolean onTouch(View v, MotionEvent event) {
        if (event.getAction() == MotionEvent.ACTION_DOWN) {
          Log.d(TAG, "Playing sound");
          playTone(streamId);
        } else if (event.getAction() == MotionEvent.ACTION_UP) {
          Log.d(TAG, "Stopping playback");
          stopTone(streamId);
        }

        return true;
      }
    });
  }

  @Override
  protected void onPause() {
    super.onPause();
    stopStream(streamId);
  }

  @Override
  protected void onResume() {
    super.onResume();
    startStream(streamId);
  }

  private void log(String message) {

    Log.d(TAG, message);
    textLog.setText(textLog.getText() + "\n" + message);
  }
}
