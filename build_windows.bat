rem set GYP_MSVS_VERSION=2019
rem ./tools/gyp/gyp --depth=./ picasso.gyp --generator-output=vcproj

mkdir proj
cd proj
cmake ../
