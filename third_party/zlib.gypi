# HTML5 runtime
# 
# Copyright (C) 2015 Zhang Ji Peng
# Contact: onecoolx@gmail.com

{
  'variables': {
    'lib_dir': 'zlib-1.2.8',
    'lib_name': 'zlib',
  },
  'target_name': '<(lib_name)',
  'type': 'shared_library',
  'dependencies': [
  ],
  'defines': [
    'ZLIB_DLL',
  ],
  'include_dirs': [
    '<(lib_dir)',
  ],
  'sources': [
    '<(lib_dir)/adler32.c',
    '<(lib_dir)/compress.c',
    '<(lib_dir)/crc32.c',
    '<(lib_dir)/deflate.c',
    '<(lib_dir)/infback.c',
    '<(lib_dir)/inffast.c',
    '<(lib_dir)/inflate.c',
    '<(lib_dir)/inftrees.c',
    '<(lib_dir)/trees.c',
    '<(lib_dir)/uncompr.c',
    '<(lib_dir)/zutil.c',
  ],
  'conditions': [
    ['OS=="win"', {
      'sources': [
        '<(lib_dir)/gzclose.c',
        '<(lib_dir)/gzlib.c',
        '<(lib_dir)/gzread.c',
        '<(lib_dir)/gzwrite.c',
        '<(lib_dir)/win32/zlib.def',
        '<(lib_dir)/win32/zlib1.rc',
      ],
    }],
  ],
  'conditions': [
    ['OS=="win"', {
      'include_dirs': [
        '$(OutDir)/include',
      ],
      'actions': [
       {
        'action_name': 'install_header1',
        'inputs': [
          '<(lib_dir)/zconf.h',
         ],
        'outputs': [
          '$(OutDir)/include/zconf.h',
         ],
        'action': [
          'python',
          'tools/cp.py',
          '<(_inputs)',
          '$(OutDir)/include/zconf.h',
        ],
        'msvs_cygwin_shell': 0,
       },
       {
        'action_name': 'install_header2',
        'inputs': [
          '<(lib_dir)/zlib.h',
         ],
        'outputs': [
          '$(OutDir)/include/zlib.h',
         ],
        'action': [
          'python',
          'tools/cp.py',
          '<(_inputs)',
          '$(OutDir)/include/zlib.h',
        ],
        'msvs_cygwin_shell': 0,
       },
      ],
    }],
    ['OS=="linux"', {
      'include_dirs': [
        '$(builddir)/include',
      ],
      'actions': [
       {
        'action_name': 'install_header1',
        'inputs': [
          '<(lib_dir)/zconf.h',
        ],
        'outputs': [
          '$(builddir)/include/zconf.h',
        ],
        'action': [
        'python',
        'tools/cp.py',
        '<(_inputs)',
        '$(builddir)/include/zconf.h',
        ],
       },
       {
        'action_name': 'install_header2',
        'inputs': [
          '<(lib_dir)/zlib.h',
        ],
        'outputs': [
          '$(builddir)/include/zlib.h',
        ],
        'action': [
        'python',
        'tools/cp.py',
        '<(_inputs)',
        '$(builddir)/include/zlib.h',
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

