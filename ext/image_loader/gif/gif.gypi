# Picasso - a vector graphics library
# 
# Copyright (C) 2016 Zhang Ji Peng
# Contact: onecoolx@gmail.com

{
  'targets': [
    {
      'target_name': 'psxm_image_gif',
      'type': 'loadable_module',
      'dependencies': [
        'psx_image',
        'gif',
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
        'gif_module.c',
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
                'OutputFile': '$(OutDir)modules\\$(ProjectName)$(TargetExt)',
            },
          },
        }],
        ['OS=="linux"', {
          'include_dirs': [
            '$(builddir)/include',
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
