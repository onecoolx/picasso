# HTML5 runtime
# 
# Copyright (C) 2016 Zhang Ji Peng
# Contact: onecoolx@gmail.com

{
  'variables': {
    'lib_dir': 'giflib-5.1.3',
    'lib_name': 'gif',
  },
  'target_name': '<(lib_name)',
  'type': 'shared_library',
  'dependencies': [
  ],
  'defines': [
  ],
  'include_dirs': [
    '<(lib_dir)',
    '<(lib_dir)/lib',
  ],
  'sources': [
    '<(lib_dir)/lib/gif_hash.h',
    '<(lib_dir)/lib/gif_lib.h',
    '<(lib_dir)/lib/gif_lib_private.h',
    '<(lib_dir)/lib/dgif_lib.c',
    '<(lib_dir)/lib/egif_lib.c',
    '<(lib_dir)/lib/gif_err.c',
    '<(lib_dir)/lib/gif_font.c',
    '<(lib_dir)/lib/gif_hash.c',
    '<(lib_dir)/lib/gifalloc.c',
    '<(lib_dir)/lib/openbsd-reallocarray.c',
    '<(lib_dir)/lib/quantize.c',
  ],
  'conditions': [
    ['OS=="win"', {
      'sources': [
      '<(lib_dir)/gif_lib.def',
      ],
      'defines': [
      ],
      'actions': [
       {
        'action_name': 'install_header1',
        'inputs': [
          '<(lib_dir)/lib/gif_lib.h',
         ],
        'outputs': [
          '$(OutDir)/include/gif_lib.h',
         ],
        'action': [
          'python',
          'tools/cp.py',
          '<(_inputs)',
          '$(OutDir)/include/gif_lib.h',
        ],
        'msvs_cygwin_shell': 0,
       },
      ],
    }],
    ['OS=="linux"', {
      'actions': [
       {
        'action_name': 'install_header1',
        'inputs': [
          '<(lib_dir)/lib/gif_lib.h',
        ],
        'outputs': [
          '$(builddir)/include/gif_lib.h',
        ],
        'action': [
        'python',
        'tools/cp.py',
        '<(_inputs)',
        '$(builddir)/include/gif_lib.h',
        ],
       },
      ],
    }],
  ],
  'includes': [
    '../build/configs.gypi',
    '../build/defines.gypi',
  ],
}

