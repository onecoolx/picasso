#!/bin/sh

./tools/gyp/gyp --depth=./ picasso.gyp --generator-output=proj
echo "Please change dir to \"proj\" and type \"make -i\" or \"make BUILDTYPE=Release -i\""
echo "...\n"		
