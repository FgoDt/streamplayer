project(bsptst)

cmake_minimum_required(VERSION 3.2)

set(CMAKE_CXX_FLAGS "-g")
set(CMAKE_C_FLAGS "-g")

include_directories("../../ThirdPartyLibs/FFmpeg/include")
link_directories("../../ThirdPartyLibs/FFmpeg/lib")

add_library( 
    BSPD 
    STATIC 
    BSPD.cpp 
    bspd_coder.c
    )

add_executable(bsptst test.cpp)
target_link_libraries(
    bsptst
    BSPD
    avformat
    swscale
    avcodec
    avfilter
    swresample
    avutil
    z
    m
    pthread
    dl)
