cmake_minimum_required(VERSION 3.4.1)

set(HOWIE_NATIVE_DIR  ../../../../../howie/src/main/jni)
add_subdirectory(${HOWIE_NATIVE_DIR}/private
                 ${HOWIE_NATIVE_DIR}/build)

add_library(hello_low_latency_output SHARED
            player.c)

target_include_directories(hello_low_latency_output PRIVATE
            ${HOWIE_NATIVE_DIR})

target_link_libraries(hello_low_latency_output
            OpenSLES
            log
            howie)


