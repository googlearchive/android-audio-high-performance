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

### INCLUDE OBOE LIBRARY ###

# Set the path to the Oboe library directory (may not exist yet)
set (OBOE_RELATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../lib-oboe)

# Get the absolute path to the Oboe library directory
get_filename_component(OBOE_DIR
                       ${OBOE_RELATIVE_PATH}
                       ABSOLUTE)

# If the Oboe library directory doesn't exist, clone it from github
if ((NOT EXISTS ${OBOE_DIR}) OR
    (NOT EXISTS ${OBOE_DIR}/CMakeLists.txt))
    execute_process(COMMAND git clone
                            https://github.com/google/oboe.git
                            ${OBOE_DIR}
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..)
endif()

# Add the Oboe library as a subproject. Since Oboe is an out-of-tree source library we must also
# specify a binary directory
add_subdirectory(${OBOE_DIR} ./oboe)

# Include the Oboe headers
include_directories(${OBOE_DIR}/include)

### END OBOE INCLUDE SECTION ###

# Debug utilities
set (DEBUG_UTILS_PATH "../../debug-utils")
set (DEBUG_UTILS_SOURCES ${DEBUG_UTILS_PATH}/trace.cpp)
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
target_link_libraries(hello-oboe android log oboe)
