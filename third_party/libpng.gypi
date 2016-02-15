# HTML5 runtime
# 
# Copyright (C) 2015 Zhang Ji Peng
# Contact: onecoolx@gmail.com

{
  'variables': {
    'lib_dir': 'libpng-1.6.17',
    'lib_name': 'png',
  },
  'target_name': '<(lib_name)',
  'type': 'shared_library',
  'dependencies': [
    'zlib',
  ],
  'defines': [
    'ZLIB_DLL',
  ],
  'include_dirs': [
    '<(lib_dir)',
  ],
  'sources': [
  '<(lib_dir)/png.c',
  '<(lib_dir)/pngerror.c',
  '<(lib_dir)/pngget.c',
  '<(lib_dir)/pngmem.c',
  '<(lib_dir)/pngpread.c',
  '<(lib_dir)/pngread.c',
  '<(lib_dir)/pngrio.c',
  '<(lib_dir)/pngrtran.c',
  '<(lib_dir)/pngrutil.c',
  '<(lib_dir)/pngset.c',
  '<(lib_dir)/pngtrans.c',
  '<(lib_dir)/pngwio.c',
  '<(lib_dir)/pngwrite.c',
  '<(lib_dir)/pngwtran.c',
  '<(lib_dir)/pngwutil.c',
  ],
  'conditions': [
    ['OS=="win"', {
      'sources': [
        '<(lib_dir)/scripts/pngwin.rc',
        '<(lib_dir)/scripts/symbols.def',
      ],
    }],
  ],
  'includes': [
    '../build/configs.gypi',
    '../build/defines.gypi',
  ],
  'conditions': [
    ['OS=="win"', {
      'include_dirs': [
        '$(OutDir)/include',
      ],
      'actions': [
       {
        'action_name': 'prebuild',
        'inputs': [
          '<(lib_dir)/scripts/pnglibconf.h.prebuilt', 
         ],
        'outputs': [
          '$(OutDir)/include/pnglibconf.h' 
         ],
        'action': [
          'python',
          'tools/cp.py',
          '<(_inputs)',
          '$(OutDir)/include/pnglibconf.h',
        ],
        'msvs_cygwin_shell': 0,
       },
       {
        'action_name': 'install_header',
        'inputs': [
          '<(lib_dir)/png.h',
         ],
        'outputs': [
          '$(OutDir)/include/png.h',
         ],
        'action': [
          'python',
          'tools/cp.py',
          '<(_inputs)',
          '$(OutDir)/include/png.h',
        ],
        'msvs_cygwin_shell': 0,
       },
       {
        'action_name': 'install_header1',
        'inputs': [
          '<(lib_dir)/pngconf.h',
         ],
        'outputs': [
          '$(OutDir)/include/pngconf.h',
         ],
        'action': [
          'python',
          'tools/cp.py',
          '<(_inputs)',
          '$(OutDir)/include/pngconf.h',
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
        'action_name': 'prebuild',
        'inputs': [
          '<(lib_dir)/scripts/pnglibconf.h.prebuilt', 
        ],
        'outputs': [
          '$(builddir)/include/pnglibconf.h', 
        ],
        'action': [
        'python',
        'tools/cp.py',
        '<(_inputs)',
        '$(builddir)/include/pnglibconf.h',
        ],
       },
       {
        'action_name': 'install_header',
        'inputs': [
          '<(lib_dir)/png.h',
        ],
        'outputs': [
          '$(builddir)/include/png.h',
        ],
        'action': [
        'python',
        'tools/cp.py',
        '<(_inputs)',
        '$(builddir)/include/png.h',
        ],
       },
       {
        'action_name': 'install_header1',
        'inputs': [
          '<(lib_dir)/pngconf.h',
        ],
        'outputs': [
          '$(builddir)/include/pngconf.h',
        ],
        'action': [
        'python',
        'tools/cp.py',
        '<(_inputs)',
        '$(builddir)/include/pngconf.h',
        ],
       },
      ],
    }],
  ],
}

