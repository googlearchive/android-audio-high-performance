cmake_minimum_required(VERSION 3.4.1)


file(GLOB HOWIE_SRC ./*.cpp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Werror")

# Uncomment this line to get more debug spew.
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHOWIE_TRACE_LEVEL=1")

add_library(howie SHARED
            ${HOWIE_SRC})

target_include_directories(howie PRIVATE
            ../)
target_link_libraries(howie
            OpenSLES
            log)
