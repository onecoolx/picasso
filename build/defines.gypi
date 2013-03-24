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
        '_USE_MATH_DEFINES',
        '_CRT_SECURE_NO_WARNINGS' 
      ],
    }],
  ],
}
