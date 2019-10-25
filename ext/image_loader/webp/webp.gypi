# Picasso - a vector graphics library
# 
# Copyright (C) 2016 Zhang Ji Peng
# Contact: onecoolx@gmail.com

{
  'targets': [
    {
      'target_name': 'psxm_image_webp',
      'type': 'loadable_module',
      'dependencies': [
        'psx_image',
        'webp',
      ],
      'defines': [
        'EXPORT',
      ],
      'include_dirs': [
        '../../../include',
        '../../../build',
        '../',
      ],
      'module_dir': 'image_modules',
      'sources': [
        '../psx_image_io.h',
        '../psx_image_io.c',
        'webp_module.c',
      ],
      'conditions': [
        ['OS=="win"', {
          'defines': [
          ],
          'include_dirs': [
            '$(OutDir)/include',
          ],
          'msvs_settings': {
            'VCLinkerTool' : {
                'OutputFile': '$(OutDir)modules\\$(ProjectName).dll',
            },
          },
        }],
        ['OS=="linux"', {
          'include_dirs': [
            '$(builddir)/include',
          ],
        }],
        ['OS=="macosx" or OS=="ios"', {
          'include_dirs': [
            '$(INTERMEDIATE_DIR)/include',
          ],
        }],
      ],
      'includes': [
        '../../../build/configs.gypi',
        '../../../build/defines.gypi',
      ],
    },
  ],
}
