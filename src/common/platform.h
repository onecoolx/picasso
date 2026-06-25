/* Picasso - a vector graphics library
 *
 * Copyright (C) 2010 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _PLATFORM_H_
#define _PLATFORM_H_

/* ENABLE() - turn on a specific feature */
#define ENABLE(FEATURE) (defined ENABLE_##FEATURE && ENABLE_##FEATURE)

/* COMPILER() - the compiler being used to build the project */
#define COMPILER(FEATURE) (defined COMPILER_##FEATURE  && COMPILER_##FEATURE)

/* CPU() - the target CPU architecture */
#define CPU(FEATURE) (defined CPU_##FEATURE  && CPU_##FEATURE)

/* COMPILER(MSVC) - Microsoft Visual C++ */
#if defined(_MSC_VER)
    #define COMPILER_MSVC 1
#endif

/* COMPILER(GCC) - GNU Compiler Collection */
#if defined(__GNUC__)
    #define COMPILER_GCC 1
#endif

/* COMPILER(INTEL) - Intel C++ Compiler */
#if defined(__INTEL_COMPILER)
    #define COMPILER_INTEL 1
#endif

/* COMPILER(WATCOM) - Watcom C/C++ Compiler */
#if defined(__WATCOMC__)
    #define COMPILER_WATCOM 1
#endif

/* COMPILER(CLANG) - llvm/clang C/C++ Compiler */
#if defined(__clang__)
    #define COMPILER_CLANG 1
#endif

/* CPU(X86) - i386 / x86 32-bit */
#if   defined(__i386__) \
    || defined(i386)     \
    || defined(_M_IX86)  \
    || defined(_X86_)    \
    || defined(__THW_INTEL)
    #define CPU_X86 1
#endif

/* CPU(X86_64) - AMD64 / Intel64 / x86_64 64-bit */
#if   defined(__x86_64__) \
    || defined(_M_X64)
    #define CPU_X86_64 1
#endif

/* CPU(ARM) - ARM any version */
#if   defined(arm) \
    || defined(__arm__) \
    || defined(ARM) \
    || defined(_ARM_)
    #define CPU_ARM 1
#endif

/* CPU(ARM64) - ARM 64bit version */
#if defined(__aarch64__)
    #define CPU_ARM64 1
#endif

#if defined(__ARM_NEON__)
    #define CPU_ARM_NEON 1
#endif

#if (defined(__VFP_FP__) && !defined(__SOFTFP__))
    #define CPU_ARM_VFP 1
#endif

#if COMPILER(GCC)
    #if __GNUC__ == 2 && __GNUC_MINOR__ < 96
        #define __builtin_expect(x, expected_value) (x)
    #endif
    #define likely(x) __builtin_expect((x),1)
    #define unlikely(x) __builtin_expect((x),0)
#else
    #define likely(X)  (X)
    #define unlikely(X)  (X)
#endif

// aligned attribute
#if COMPILER(MSVC)
    #define ALIGNED(x)  __declspec(align(x))
#elif COMPILER(GCC)
    #define ALIGNED(x)  __attribute__((aligned((x))))
#else
    #define ALIGNED(x)
#endif

// force inline
#if COMPILER(MSVC)
    #define _FORCE_INLINE_  __forceinline
#elif COMPILER(GCC)
    #if __GNUC__ == 2 && __GNUC_MINOR__ < 96
        #define _FORCE_INLINE_  inline
    #else
        #define _FORCE_INLINE_  __attribute__((always_inline))
    #endif
#endif

#if __cplusplus >= 201103L
    #define _REGISTER_
#else
    #define _REGISTER_ register
#endif

#endif /*_PLATFORM_H_*/
