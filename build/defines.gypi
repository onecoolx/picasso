# Picasso - a vector graphics library
# 
# Copyright (C) 2013 Zhang Ji Peng
# Contact: onecoolx@gmail.com

{
  'conditions': [
    ['OS=="win"', {
      'defines': [
        'WIN32',
        'DLL_EXPORT',
        'ENABLE_FAST_COPY=1',
        'ENABLE_SYSTEM_MALLOC=1',
        '__SSE2__=1',
        '_HAS_EXCEPTIONS=0',
        '_USE_MATH_DEFINES',
        '_CRT_SECURE_NO_WARNINGS',
      ],
    }],
    ['OS=="linux"', {
      'defines': [
        'ENABLE_FREE_TYPE2=1',
        'ENABLE_FONT_CONFIG=1',
      ],
    }],
    ['OS=="macosx" or OS=="ios"', {
      'defines': [
        'ENABLE_SYSTEM_MALLOC=1',
      ],
    }],
  ],
}
