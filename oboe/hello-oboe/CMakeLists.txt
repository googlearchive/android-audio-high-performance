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

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Werror")

# Build the Oboe library
set (OBOE_DIR ../../../oboe)

set (OBOE_SOURCES
        ${OBOE_DIR}/src/aaudio/AAudioLoader.cpp
        ${OBOE_DIR}/src/aaudio/AudioStreamAAudio.cpp
        ${OBOE_DIR}/src/common/LatencyTuner.cpp
        ${OBOE_DIR}/src/common/AudioStream.cpp
        ${OBOE_DIR}/src/common/AudioStreamBuilder.cpp
        ${OBOE_DIR}/src/common/Utilities.cpp
        ${OBOE_DIR}/src/fifo/FifoBuffer.cpp
        ${OBOE_DIR}/src/fifo/FifoController.cpp
        ${OBOE_DIR}/src/fifo/FifoControllerBase.cpp
        ${OBOE_DIR}/src/fifo/FifoControllerIndirect.cpp
        ${OBOE_DIR}/src/opensles/OpenSLESUtilities.cpp
        ${OBOE_DIR}/src/opensles/AudioStreamBuffered.cpp
        ${OBOE_DIR}/src/opensles/AudioStreamOpenSLES.cpp
        )

include_directories(${OBOE_DIR}/include ${OBOE_DIR}/src)
add_library(oboe STATIC ${OBOE_SOURCES})

# Debug utilities
set (DEBUG_UTILS_PATH "../../debug-utils")
set (DEBUG_UTILS_SOURCES ${DEBUG_UTILS_PATH}/trace.cc)
include_directories(${DEBUG_UTILS_PATH})

# App specific sources
set (APP_DIR src/main/cpp)
file (GLOB_RECURSE APP_SOURCES
    ${APP_DIR}/jni_bridge.cpp
    ${APP_DIR}/PlayAudioEngine.cpp
    ${APP_DIR}/SineGenerator.cpp
)

# Build the libhello-oboe library
add_library(hello-oboe SHARED
            ${DEBUG_UTILS_SOURCES}
            ${APP_SOURCES}
            )

# Specify the libraries needed for hello-oboe
target_link_libraries(hello-oboe android log oboe OpenSLES)
