package com.example.audio.generatetone;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;



public class MainActivity extends AppCompatActivity {

    static {
        System.loadLibrary("generate_tone");
    }

    public static native void playTone();
    public static native void createEngine();
    public static native void createBufferQueueAudioPlayer();

    public static final String TAG = MainActivity.class.getName();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        //Create the audio engine
        createEngine();
        createBufferQueueAudioPlayer();

        Log.d(TAG, "Audio engine created");

        setContentView(R.layout.activity_main);

        Button playToneButton = (Button) findViewById(R.id.button);
        playToneButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                //Play tone using OpenSLES
                playTone();
            }
        });
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
}
