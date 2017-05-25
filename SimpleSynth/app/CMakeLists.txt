# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

add_library( SimpleSynth SHARED
             src/main/cpp/jni_bridge.cc
             src/main/cpp/audio_player.cc
             src/main/cpp/synthesizer.cc
             src/main/cpp/load_stabilizer.cc
             src/main/cpp/trace.cc
             src/main/cpp/audio_common.cc
           )

target_link_libraries( SimpleSynth
                       log OpenSLES android)
