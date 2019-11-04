# Picasso - a vector graphics library
# 
# Copyright (C) 2016 Zhang Ji Peng
# Contact: onecoolx@gmail.com

{
  'includes': [
    './psx_image.gypi',
  ],
  'conditions': [
    ['OS!="macosx" and OS!="ios"', {
      'includes': [
        './png/png.gypi',
        './jpeg/jpeg.gypi',
        './gif/gif.gypi',
        './webp/webp.gypi',
      ],
    }],
  ],
}
