#
# Copyright 2017 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

cmake_minimum_required(VERSION 3.4.1)

get_filename_component(AAUDIO_COMMON_PATH "../../../../third_party/aaudio" ABSOLUTE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Werror")

add_library(hello-aaudio SHARED
            audio_main.cpp
            ${AAUDIO_COMMON_PATH}/utils/audio_common.cpp
            ${AAUDIO_COMMON_PATH}/utils/debug_utils.cpp
            ${AAUDIO_COMMON_PATH}/wrapper/src/aaudio_wrapper.c)

target_include_directories(hello-aaudio PRIVATE
                           ${AAUDIO_COMMON_PATH}/wrapper
                           ${AAUDIO_COMMON_PATH}/utils
                           ${AAUDIO_COMMON_PATH}/../android)

# include libraries needed for hello-aaudio
target_link_libraries(hello-aaudio
                      android
                      atomic
                      log)

# Android Studio 2.2.0 with CMake support does not pack stl shared libraries,
# so app needs to pack the right shared lib into APK. This sample uses solution
# from https://github.com/jomof/ndk-stl to find the right stl shared lib to use
# and copy it to the right place for Android Studio to pack
# Usage: download ndk-stl-config.cmake into app's directory hosting CMakeLists.txt
#        and just use it with the following line
#include(ndk-stl-config.cmake)
