# Copyright (C) 2015 Zhang Ji Peng
# Contact: onecoolx@gmail.com

{
  'conditions': [
    ['OS!="macosx" and OS!="ios"', {
      'targets': [
         {
            'includes': [
              './zlib.gypi'
            ],
         },
         {
            'includes': [
              './libpng.gypi'
            ],
         },
         {
            'includes': [
              './libjpeg.gypi'
            ],
         },
         {
            'includes': [
              './giflib.gypi'
            ],
         },
         {
            'includes': [
              './libwebp.gypi'
            ],
         },
      ],
    }],
  ],
}

