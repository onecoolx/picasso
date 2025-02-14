#!/bin/sh

mkdir proj
cd proj
cmake -G "Xcode"  ..

echo "Please change dir to \"proj\" and open picasso.xcodeproj with Xcode"
echo "...\n"
