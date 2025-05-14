#!/bin/sh

# change to real android NDK path
export NDK_ROOT=/home/jipeng/android-ndk-r27c

if [ ! -d "./proj" ]; then
    mkdir proj
fi

cd proj

export PROJECT_ROOT=`pwd`

build_type="-DCMAKE_BUILD_TYPE=Debug"

if [ "$1" = "release" ]; then
    build_type="-DCMAKE_BUILD_TYPE=Release"
fi

cmake ${build_type} \
    -DCMAKE_TOOLCHAIN_FILE=${NDK_ROOT}/build/cmake/android.toolchain.cmake \
    -DANDROID_NDK=${NDK_ROOT} \
    -DANDROID_ABI=armeabi-v7a \
    -DANDROID_PLATFORM=android-21 \
    -DOPT_FORMAT_ABGR=OFF \
    -DOPT_FORMAT_BGRA=OFF \
    -DOPT_FORMAT_RGBA=OFF \
    -DOPT_FORMAT_RGB=OFF \
    -DOPT_FORMAT_BGR=OFF \
    -DOPT_FORMAT_RGB555=OFF \
    -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=${PROJECT_ROOT}/lib \
    ..

echo "Please change dir to \"proj\" and type \"make\""
echo "...\n"
