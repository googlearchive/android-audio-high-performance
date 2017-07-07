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

add_library(echo SHARED
            echo_audio_engine.cc
            jni_bridge.cc
            audio_effect.cc
            ${AAUDIO_COMMON_PATH}/utils/audio_common.cc
            ${AAUDIO_COMMON_PATH}/utils/debug_utils.cc)

target_include_directories(echo PRIVATE
            ${AAUDIO_COMMON_PATH}/utils
            ${AAUDIO_COMMON_PATH}/../android)

target_link_libraries(echo android atomic log aaudio)
