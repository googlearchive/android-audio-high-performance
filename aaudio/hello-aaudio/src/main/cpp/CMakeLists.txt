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
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Werror")

# Debug utilities
set (DEBUG_UTILS_PATH "../../../../../debug-utils")
set (DEBUG_UTILS_SOURCES ${DEBUG_UTILS_PATH}/trace.cpp)

# Code shared between AAudio samples
set (AAUDIO_COMMON_PATH "../../../../common")
set (AAUDIO_COMMON_SOURCES ${AAUDIO_COMMON_PATH}/audio_common.cpp)

# Build the shared library for this sample
add_library(hello-aaudio SHARED
            PlayAudioEngine.cpp
            jni_bridge.cpp
            ${DEBUG_UTILS_SOURCES}
            ${AAUDIO_COMMON_SOURCES})

# Includes
target_include_directories(hello-aaudio PRIVATE
            ${AAUDIO_COMMON_PATH}
            ${DEBUG_UTILS_PATH})

# Library dependencies
target_link_libraries(hello-aaudio android atomic log aaudio)
