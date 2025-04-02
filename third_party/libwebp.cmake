# Picasso - a vector graphics library
# 
# Copyright (C) 2024 Zhang Ji Peng
# Contact: onecoolx@gmail.com

set(WEBP_DIR ${PROJECT_ROOT}/third_party/libwebp-0.5.1)

set(WEBP_SOURCES
  ${WEBP_DIR}/src/dec/alpha.c
  ${WEBP_DIR}/src/dec/buffer.c
  ${WEBP_DIR}/src/dec/frame.c
  ${WEBP_DIR}/src/dec/idec.c
  ${WEBP_DIR}/src/dec/io.c
  ${WEBP_DIR}/src/dec/quant.c
  ${WEBP_DIR}/src/dec/tree.c
  ${WEBP_DIR}/src/dec/vp8.c
  ${WEBP_DIR}/src/dec/vp8l.c
  ${WEBP_DIR}/src/dec/webp.c
  ${WEBP_DIR}/src/demux/anim_decode.c
  ${WEBP_DIR}/src/demux/demux.c
  ${WEBP_DIR}/src/dsp/alpha_processing.c
  ${WEBP_DIR}/src/dsp/alpha_processing_mips_dsp_r2.c
  ${WEBP_DIR}/src/dsp/alpha_processing_sse2.c
  ${WEBP_DIR}/src/dsp/alpha_processing_sse41.c
  ${WEBP_DIR}/src/dsp/cpu.c
  ${WEBP_DIR}/src/dsp/dec.c
  ${WEBP_DIR}/src/dsp/dec_clip_tables.c
  ${WEBP_DIR}/src/dsp/dec_mips32.c
  ${WEBP_DIR}/src/dsp/dec_mips_dsp_r2.c
  ${WEBP_DIR}/src/dsp/dec_msa.c
  ${WEBP_DIR}/src/dsp/dec_neon.c
  ${WEBP_DIR}/src/dsp/dec_sse2.c
  ${WEBP_DIR}/src/dsp/dec_sse41.c
  ${WEBP_DIR}/src/dsp/dfilters.c
  ${WEBP_DIR}/src/dsp/filters_mips_dsp_r2.c
  ${WEBP_DIR}/src/dsp/filters_sse2.c
  ${WEBP_DIR}/src/dsp/lossless.c
  ${WEBP_DIR}/src/dsp/lossless_mips_dsp_r2.c
  ${WEBP_DIR}/src/dsp/lossless_neon.c
  ${WEBP_DIR}/src/dsp/lossless_sse2.c
  ${WEBP_DIR}/src/dsp/drescaler.c
  ${WEBP_DIR}/src/dsp/rescaler_mips32.c
  ${WEBP_DIR}/src/dsp/rescaler_mips_dsp_r2.c
  ${WEBP_DIR}/src/dsp/rescaler_neon.c
  ${WEBP_DIR}/src/dsp/rescaler_sse2.c
  ${WEBP_DIR}/src/dsp/upsampling.c
  ${WEBP_DIR}/src/dsp/upsampling_mips_dsp_r2.c
  ${WEBP_DIR}/src/dsp/upsampling_neon.c
  ${WEBP_DIR}/src/dsp/upsampling_sse2.c
  ${WEBP_DIR}/src/dsp/yuv.c
  ${WEBP_DIR}/src/dsp/yuv_mips32.c
  ${WEBP_DIR}/src/dsp/yuv_mips_dsp_r2.c
  ${WEBP_DIR}/src/dsp/yuv_sse2.c
  ${WEBP_DIR}/src/dsp/argb.c
  ${WEBP_DIR}/src/dsp/argb_mips_dsp_r2.c
  ${WEBP_DIR}/src/dsp/argb_sse2.c
  ${WEBP_DIR}/src/dsp/dcost.c
  ${WEBP_DIR}/src/dsp/cost_mips32.c
  ${WEBP_DIR}/src/dsp/cost_mips_dsp_r2.c
  ${WEBP_DIR}/src/dsp/cost_sse2.c
  ${WEBP_DIR}/src/dsp/enc.c
  ${WEBP_DIR}/src/dsp/enc_avx2.c
  ${WEBP_DIR}/src/dsp/enc_mips32.c
  ${WEBP_DIR}/src/dsp/enc_mips_dsp_r2.c
  ${WEBP_DIR}/src/dsp/enc_neon.c
  ${WEBP_DIR}/src/dsp/enc_sse2.c
  ${WEBP_DIR}/src/dsp/enc_sse41.c
  ${WEBP_DIR}/src/dsp/lossless_enc.c
  ${WEBP_DIR}/src/dsp/lossless_enc_mips32.c
  ${WEBP_DIR}/src/dsp/lossless_enc_mips_dsp_r2.c
  ${WEBP_DIR}/src/dsp/lossless_enc_neon.c
  ${WEBP_DIR}/src/dsp/lossless_enc_sse2.c
  ${WEBP_DIR}/src/dsp/lossless_enc_sse41.c
  ${WEBP_DIR}/src/enc/ealpha.c
  ${WEBP_DIR}/src/enc/analysis.c
  ${WEBP_DIR}/src/enc/backward_references.c
  ${WEBP_DIR}/src/enc/config.c
  ${WEBP_DIR}/src/enc/cost.c
  ${WEBP_DIR}/src/enc/delta_palettization.c
  ${WEBP_DIR}/src/enc/filter.c
  ${WEBP_DIR}/src/enc/eframe.c
  ${WEBP_DIR}/src/enc/histogram.c
  ${WEBP_DIR}/src/enc/iterator.c
  ${WEBP_DIR}/src/enc/near_lossless.c
  ${WEBP_DIR}/src/enc/picture.c
  ${WEBP_DIR}/src/enc/picture_csp.c
  ${WEBP_DIR}/src/enc/picture_psnr.c
  ${WEBP_DIR}/src/enc/picture_rescale.c
  ${WEBP_DIR}/src/enc/picture_tools.c
  ${WEBP_DIR}/src/enc/equant.c
  ${WEBP_DIR}/src/enc/syntax.c
  ${WEBP_DIR}/src/enc/token.c
  ${WEBP_DIR}/src/enc/etree.c
  ${WEBP_DIR}/src/enc/evp8l.c
  ${WEBP_DIR}/src/enc/webpenc.c
  ${WEBP_DIR}/src/mux/anim_encode.c
  ${WEBP_DIR}/src/mux/muxedit.c
  ${WEBP_DIR}/src/mux/muxinternal.c
  ${WEBP_DIR}/src/mux/muxread.c
  ${WEBP_DIR}/src/utils/bit_reader.c
  ${WEBP_DIR}/src/utils/color_cache.c
  ${WEBP_DIR}/src/utils/filters.c
  ${WEBP_DIR}/src/utils/huffman.c
  ${WEBP_DIR}/src/utils/quant_levels_dec.c
  ${WEBP_DIR}/src/utils/random.c
  ${WEBP_DIR}/src/utils/rescaler.c
  ${WEBP_DIR}/src/utils/thread.c
  ${WEBP_DIR}/src/utils/utils.c
  ${WEBP_DIR}/src/utils/bit_writer.c
  ${WEBP_DIR}/src/utils/huffman_encode.c
  ${WEBP_DIR}/src/utils/quant_levels.c
  ${WEBP_DIR}/src/extras/extras.c
  ${WEBP_DIR}/src/dec/alphai.h
  ${WEBP_DIR}/src/dec/common.h
  ${WEBP_DIR}/src/dec/decode_vp8.h
  ${WEBP_DIR}/src/dec/vp8i.h
  ${WEBP_DIR}/src/dec/vp8li.h
  ${WEBP_DIR}/src/dec/webpi.h
  ${WEBP_DIR}/src/dsp/common_sse2.h
  ${WEBP_DIR}/src/dsp/dsp.h
  ${WEBP_DIR}/src/dsp/lossless.h
  ${WEBP_DIR}/src/dsp/mips_macro.h
  ${WEBP_DIR}/src/dsp/msa_macro.h
  ${WEBP_DIR}/src/dsp/neon.h
  ${WEBP_DIR}/src/dsp/yuv.h
  ${WEBP_DIR}/src/enc/backward_references.h
  ${WEBP_DIR}/src/enc/cost.h
  ${WEBP_DIR}/src/enc/delta_palettization.h
  ${WEBP_DIR}/src/enc/histogram.h
  ${WEBP_DIR}/src/enc/vp8enci.h
  ${WEBP_DIR}/src/enc/vp8li.h
  ${WEBP_DIR}/src/mux/muxi.h
  ${WEBP_DIR}/src/utils/bit_reader.h
  ${WEBP_DIR}/src/utils/bit_reader_inl.h
  ${WEBP_DIR}/src/utils/bit_writer.h
  ${WEBP_DIR}/src/utils/color_cache.h
  ${WEBP_DIR}/src/utils/endian_inl.h
  ${WEBP_DIR}/src/utils/filters.h
  ${WEBP_DIR}/src/utils/huffman.h
  ${WEBP_DIR}/src/utils/huffman_encode.h
  ${WEBP_DIR}/src/utils/quant_levels.h
  ${WEBP_DIR}/src/utils/quant_levels_dec.h
  ${WEBP_DIR}/src/utils/random.h
  ${WEBP_DIR}/src/utils/rescaler.h
  ${WEBP_DIR}/src/utils/thread.h
  ${WEBP_DIR}/src/utils/utils.h
  ${WEBP_DIR}/src/webp/format_constants.h
  ${WEBP_DIR}/src/webp/decode.h
  ${WEBP_DIR}/src/webp/demux.h
  ${WEBP_DIR}/src/webp/encode.h
  ${WEBP_DIR}/src/webp/mux.h
  ${WEBP_DIR}/src/webp/mux_types.h
  ${WEBP_DIR}/src/webp/types.h
)

add_definitions(-DWEBP_USE_THREAD)

if (WIN32)
    add_definitions(-DWIN32_EXPORT)
    add_definitions(-DHAVE_WINCODEC_H)
elseif (ANDROID)
    set(WEBP_SOURCES ${WEBP_SOURCES} ${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c)
    include_directories(${ANDROID_NDK}/sources/android/cpufeatures)
endif()

file(GLOB_RECURSE WEBP_HEADERS ${WEBP_DIR}/src/webp/*.h)
foreach(webp_header ${WEBP_HEADERS})
    get_filename_component(header_name ${webp_header} NAME)
    configure_file(${webp_header} ${CMAKE_CURRENT_BINARY_DIR}/include/webp/${header_name})
endforeach(webp_header ${WEBP_HEADERS})

add_library(webp ${WEBP_SOURCES})
if (UNIX AND NOT APPLE)
    include(GNUInstallDirs)
    install(TARGETS webp LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/picasso ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/picasso)
else
    install(TARGETS webp LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
endif

include_directories(${WEBP_DIR} ${CMAKE_CURRENT_BINARY_DIR}/include)

if (ANDROID)
    target_compile_options(webp PRIVATE -DANDROID -Wno-unused-but-set-variable)
endif()
