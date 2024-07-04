#!/bin/sh

mkdir proj
cd proj
#  cmake -DCMAKE_BUILD_TYPE=Release ..
cmake -DCMAKE_BUILD_TYPE=Debug ..

echo "Please change dir to \"proj\" and type \"make\""
echo "...\n"		
