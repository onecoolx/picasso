# Picasso - a vector graphics library
# 
# Copyright (C) 2013 Zhang Ji Peng
# Contact: onecoolx@gmail.com

{
  'configurations': {
    'Debug': {
      'defines': [
        '_DEBUG',
      ],
      'conditions': [
        ['OS=="win"', {
          'msvs_configuration_attributes': {
            'ConfigurationType': '2',
            'CharacterSet': '1',
          },
          'msvs_settings': {
            'VCCLCompilerTool': {
              'Optimization': '0',
              'MinimalRebuild': 'true',
              'ExceptionHandling': '0',
              'BasicRuntimeChecks': '3',
              'RuntimeLibrary': '1',
              'TreatWChar_tAsBuiltInType': 'false',
              'RuntimeTypeInfo': 'false',
              'WarningLevel': '3',
              'WarnAsError': 'true',
              'SuppressStartupBanner': 'true',
              'DebugInformationFormat': '3',
            },
            'VCLinkerTool': {
                'LinkIncremental': '1',
                'SuppressStartupBanner': 'true',
                'GenerateDebugInformation': 'true',
                'RandomizedBaseAddress': '1',
                'DataExecutionPrevention': '0',
                'TargetMachine': '1',
            },
          },
        }],
        ['OS=="macosx" or OS=="ios"', {
          'xcode_settings': {
            'ALWAYS_SEARCH_USER_PATHS': 'NO',
            'CLANG_ENABLE_OBJC_WEAK': 'YES',
            'CLANG_ADDRESS_SANITIZER_CONTAINER_OVERFLOW': 'YES',
            'DEAD_CODE_STRIPPING': 'NO',
            'ONLY_ACTIVE_ARCH': 'YES',
            'ENABLE_TESTABILITY': 'YES',
            'CODE_SIGN_IDENTITY': "-",
            'GCC_UNROLL_LOOPS': 'NO',
            'GCC_OPTIMIZATION_LEVEL': '0',
            'GCC_ENABLE_CPP_EXCEPTIONS': "NO",
            'GCC_ENABLE_CPP_RTTI': "NO",
          },
        }],
        ['OS=="linux"', {
          'cflags_cc': [
            '-O0',
            '-Wall',
            '-g',
            '-fPIC',
            '-fno-rtti',
            '-fno-exceptions',
            '-Wno-unused-result',
          ],
          'cflags': [
            '-O0',
            '-Wall',
            '-g',
            '-fPIC',
            '-Wno-unused-result',
          ],
        }],
      ],
    },
    'Release': {
      'defines': [
        'NDEBUG',
      ],
      'conditions': [
        ['OS=="win"', {
          'msvs_configuration_attributes': {
            'ConfigurationType': '2',
            'CharacterSet': '1',
          },
          'msvs_settings': {
            'VCCLCompilerTool': {
              'Optimization': '3',
              'InlineFunctionExpansion': '2',
              'FavorSizeOrSpeed': '1',
              'WholeProgramOptimization': 'true',
              'StringPooling': 'true',
              'ExceptionHandling': '0',
              'RuntimeLibrary': '0',
              'EnableFunctionLevelLinking': 'true',
              'EnableEnhancedInstructionSet': '2',
              'FloatingPointModel': '2',
              'TreatWChar_tAsBuiltInType': 'false',
              'RuntimeTypeInfo': 'false',
              'WarningLevel': '3',
              'WarnAsError': 'true',
              'SuppressStartupBanner': 'true',
              'DebugInformationFormat': '3',
            },
            'VCLinkerTool': {
                'LinkIncremental': '1',
                'SuppressStartupBanner': 'true',
                'GenerateDebugInformation': 'false',
                'GenerateMapFile': 'true',
                'LinkTimeCodeGeneration': '1',
                'RandomizedBaseAddress': '1',
                'DataExecutionPrevention': '0',
                'TargetMachine': '1',
            },
          },
        }],
        ['OS=="macosx" or OS=="ios"', {
          'xcode_settings': {
            'ALWAYS_SEARCH_USER_PATHS': 'NO',
            'CLANG_ENABLE_CODE_COVERAGE': 'NO',
            'GCC_GENERATE_DEBUGGING_SYMBOLS': 'NO',
            'GCC_INLINES_ARE_PRIVATE_EXTERN': 'YES',
            'CLANG_ENABLE_OBJC_WEAK': 'YES',
            'CLANG_ADDRESS_SANITIZER_CONTAINER_OVERFLOW': 'NO',
            'DEAD_CODE_STRIPPING': 'YES',
            'ONLY_ACTIVE_ARCH': 'NO',
            'CODE_SIGN_IDENTITY': "-",
            'LLVM_LTO': 'YES',
            'ENABLE_TESTABILITY': 'NO',
            'GCC_UNROLL_LOOPS': 'YES',
            'GCC_OPTIMIZATION_LEVEL': '3',
            'GCC_ENABLE_CPP_EXCEPTIONS': "NO",
            'GCC_ENABLE_CPP_RTTI': "NO",
          },
        }],
        ['OS=="linux"', {
          'cflags_cc': [
            '-O3',
            '-Wall',
            '-fPIC',
            '-fno-rtti',
            '-fno-exceptions',
            '-Wno-unused-result',
          ],
          'cflags': [
            '-O3',
            '-Wall',
            '-fPIC',
            '-Wno-unused-result',
          ],
        }],
      ],
    },
  },
}
