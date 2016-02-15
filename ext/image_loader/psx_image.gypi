# Picasso - a vector graphics library
# 
# Copyright (C) 2016 Zhang Ji Peng
# Contact: onecoolx@gmail.com

{
  'targets': [
    {
      'target_name': 'psx_image',
      'type': 'shared_library',
      'dependencies': [
        'picasso2_sw',
      ],
      'defines': [
        'EXPORT',
      ],
      'include_dirs': [
        '../../include',
        '../../build',
        './',
      ],
      'sources': [
        'psx_list.h',
        'psx_image_io.h',
        'psx_image_io.c',
        'psx_image_loader.h',
        'psx_image_loader.c',
        'psx_image_modules.h',
        'psx_image_modules.c',
      ],
      'conditions': [
        ['OS=="win"', {
          'sources': [
            'psx_image.rc',
            'psx_image.def',
            'resource.h',
          ],
        }],
      ],
      'includes': [
        '../../build/configs.gypi',
        '../../build/defines.gypi',
      ],
    },
  ],
}
