#!/bin/sh

./tools/gyp/gyp --depth=./ picasso.gyp -f xcode -DOS=macosx --generator-output=proj
echo "Please change dir to \"proj\" and open picasso.xcodeproj with Xcode"
echo "...\n"		
