# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.


#include_directories("../../../../../../../BSPDSource/BSPD")

#这里配置你的FFMPEG库和头文件路径
#针对不同的平台FFMPEG库也应该不同
#如果要使用硬件加速你的FFMPEG库应该使用media_codec选项编译
include_directories("E:\\FFmpegBuild\\a16x86\\include")
LINK_DIRECTORIES("E:\\FFmpegBuild\\a16x86\\lib")
#include_directories("E:\\FFmpegBuild\\a19armMC\\include")
#LINK_DIRECTORIES("E:\\FFmpegBuild\\a19armMC\\lib")

add_library( # Sets the name of the library.
             BSPD

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             ../../../../../../../BSPDSource/BSPD/BSPD.cpp
             ../../../../../../../BSPDSource/BSPD/bspd_coder.c
             )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       BSPD
                       avformat
                       swscale
                       swresample
                       postproc
                       avcodec
                       avfilter
                       avutil
                       z
                       m
                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} )