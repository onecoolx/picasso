# HTML5 runtime
# 
# Copyright (C) 2015 Zhang Ji Peng
# Contact: onecoolx@gmail.com

{
  'variables': {
    'lib_dir': 'libjpeg-turbo-1.4.1',
    'lib_name': 'jpeg',
  },
  'target_name': '<(lib_name)',
  'type': 'shared_library',
  'dependencies': [
  ],
  'defines': [
  ],
  'include_dirs': [
    '<(lib_dir)',
    '<(lib_dir)/build',
    '<(lib_dir)/simd',
  ],
  'sources': [
    '<(lib_dir)/jcapimin.c',
    '<(lib_dir)/jcapistd.c',
    '<(lib_dir)/jccoefct.c',
    '<(lib_dir)/jccolor.c',
    '<(lib_dir)/jcdctmgr.c',
    '<(lib_dir)/jchuff.c',
    '<(lib_dir)/jcinit.c',
    '<(lib_dir)/jcmainct.c',
    '<(lib_dir)/jcmarker.c',
    '<(lib_dir)/jcmaster.c',
    '<(lib_dir)/jcomapi.c',
    '<(lib_dir)/jcparam.c',
    '<(lib_dir)/jcphuff.c',
    '<(lib_dir)/jcprepct.c',
    '<(lib_dir)/jcsample.c',
    '<(lib_dir)/jctrans.c',
    '<(lib_dir)/jdapimin.c',
    '<(lib_dir)/jdapistd.c',
    '<(lib_dir)/jdatadst.c',
    '<(lib_dir)/jdatasrc.c',
    '<(lib_dir)/jdcoefct.c',
    '<(lib_dir)/jdcolor.c',
    '<(lib_dir)/jddctmgr.c',
    '<(lib_dir)/jdhuff.c',
    '<(lib_dir)/jdinput.c',
    '<(lib_dir)/jdmainct.c',
    '<(lib_dir)/jdmarker.c',
    '<(lib_dir)/jdmaster.c',
    '<(lib_dir)/jdmerge.c',
    '<(lib_dir)/jdphuff.c',
    '<(lib_dir)/jdpostct.c',
    '<(lib_dir)/jdsample.c',
    '<(lib_dir)/jdtrans.c',
    '<(lib_dir)/jerror.c',
    '<(lib_dir)/jfdctflt.c',
    '<(lib_dir)/jfdctfst.c',
    '<(lib_dir)/jfdctint.c',
    '<(lib_dir)/jidctflt.c',
    '<(lib_dir)/jidctfst.c',
    '<(lib_dir)/jidctint.c',
    '<(lib_dir)/jidctred.c',
    '<(lib_dir)/jquant1.c',
    '<(lib_dir)/jquant2.c',
    '<(lib_dir)/jutils.c',
    '<(lib_dir)/jmemmgr.c',
    '<(lib_dir)/jmemnobs.c',
    '<(lib_dir)/jaricom.c',
    '<(lib_dir)/jcarith.c',
    '<(lib_dir)/jdarith.c',
    '<(lib_dir)/turbojpeg.c',
    '<(lib_dir)/transupp.c',
    '<(lib_dir)/jdatadst-tj.c',
    '<(lib_dir)/jdatasrc-tj.c',
    '<(lib_dir)/jsimd_none.c',
  ],
  'conditions': [
    ['OS=="win"', {
      'defines': [
        'HAVE_BOOLEAN',
        'XMD_H',
      ],
      'actions': [
       {
        'action_name': 'install_header1',
        'inputs': [
          '<(lib_dir)/jpeglib.h',
         ],
        'outputs': [
          '$(OutDir)/include/jpeglib.h',
         ],
        'action': [
          'python',
          'tools/cp.py',
          '<(_inputs)',
          '$(OutDir)/include/jpeglib.h',
        ],
        'msvs_cygwin_shell': 0,
       },
       {
        'action_name': 'install_header2',
        'inputs': [
          '<(lib_dir)/build/jconfig.h',
         ],
        'outputs': [
          '$(OutDir)/include/jconfig.h',
         ],
        'action': [
          'python',
          'tools/cp.py',
          '<(_inputs)',
          '$(OutDir)/include/jconfig.h',
        ],
        'msvs_cygwin_shell': 0,
       },
       {
        'action_name': 'install_header3',
        'inputs': [
          '<(lib_dir)/jmorecfg.h',
         ],
        'outputs': [
          '$(OutDir)/include/jmorecfg.h',
         ],
        'action': [
          'python',
          'tools/cp.py',
          '<(_inputs)',
          '$(OutDir)/include/jmorecfg.h',
        ],
        'msvs_cygwin_shell': 0,
       },
      ],
      'sources': [
        '<(lib_dir)/win/jpeg8.def',
      ],
    }],
    ['OS=="linux"', {
      'actions': [
       {
        'action_name': 'install_header1',
        'inputs': [
          '<(lib_dir)/jpeglib.h',
        ],
        'outputs': [
          '$(builddir)/include/jpeglib.h',
        ],
        'action': [
        'python',
        'tools/cp.py',
        '<(_inputs)',
        '$(builddir)/include/jpeglib.h',
        ],
       },
       {
        'action_name': 'install_header2',
        'inputs': [
          '<(lib_dir)/build/jconfig.h',
        ],
        'outputs': [
          '$(builddir)/include/jconfig.h',
        ],
        'action': [
        'python',
        'tools/cp.py',
        '<(_inputs)',
        '$(builddir)/include/jconfig.h',
        ],
       },
       {
        'action_name': 'install_header3',
        'inputs': [
          '<(lib_dir)/jmorecfg.h',
        ],
        'outputs': [
          '$(builddir)/include/jmorecfg.h',
        ],
        'action': [
        'python',
        'tools/cp.py',
        '<(_inputs)',
        '$(builddir)/include/jmorecfg.h',
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

