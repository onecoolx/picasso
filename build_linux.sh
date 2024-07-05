#!/bin/sh

mkdir proj
cd proj

build_type="-DCMAKE_BUILD_TYPE=Debug"

if [ "$1" = "release" ]; then
    build_type="-DCMAKE_BUILD_TYPE=Release"
fi

cmake ${build_type} ..

echo "Please change dir to \"proj\" and type \"make\""
echo "...\n"		
