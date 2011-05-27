# Microsoft Developer Studio Project File - Name="picasso" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=picasso - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "picasso.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "picasso.mak" CFG="picasso - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "picasso - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "picasso - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "picasso - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "picasso_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /O2 /I "../include" /I "../build" /I "../src" /I "../src/include" /D "WIN32" /D "EXPORT" /D "DLL_EXPORT" /FD /Zm200 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"Release/proj.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 gdi32.lib user32.lib /nologo /dll /pdb:"../Release/picasso.pdb" /machine:I386 /out:"../Release/picasso.dll" /implib:"../Release/picasso.lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "picasso - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "picasso_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /ZI /Od /I "../include" /I "../build" /I "../src" /I "../src/include" /D "WIN32" /D "EXPORT" /D "DLL_EXPORT" /FD /Zm200 /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"Debug/proj.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 gdi32.lib user32.lib /nologo /dll /pdb:"../Debug/picasso.pdb" /debug /machine:I386 /out:"../Debug/picasso.dll" /implib:"../Debug/picasso.lib" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "picasso - Win32 Release"
# Name "picasso - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\lib\agg_arc.cpp
# End Source File
# Begin Source File

SOURCE=..\src\lib\agg_bezier_arc.cpp
# End Source File
# Begin Source File

SOURCE=..\src\lib\agg_bspline.cpp
# End Source File
# Begin Source File

SOURCE=..\src\lib\agg_curves.cpp
# End Source File
# Begin Source File

SOURCE=..\src\lib\agg_font_freetype.cpp
# End Source File
# Begin Source File

SOURCE=..\src\lib\agg_font_win32_tt.cpp
# End Source File
# Begin Source File

SOURCE=..\src\lib\agg_image_filters.cpp
# End Source File
# Begin Source File

SOURCE=..\src\lib\agg_line_aa_basics.cpp
# End Source File
# Begin Source File

SOURCE=..\src\lib\agg_line_profile_aa.cpp
# End Source File
# Begin Source File

SOURCE=..\src\lib\agg_rounded_rect.cpp
# End Source File
# Begin Source File

SOURCE=..\src\lib\agg_sqrt_tables.cpp
# End Source File
# Begin Source File

SOURCE=..\src\lib\agg_trans_affine.cpp
# End Source File
# Begin Source File

SOURCE=..\src\lib\agg_vcgen_bspline.cpp
# End Source File
# Begin Source File

SOURCE=..\src\lib\agg_vcgen_dash.cpp
# End Source File
# Begin Source File

SOURCE=..\src\lib\agg_vcgen_smooth_poly1.cpp
# End Source File
# Begin Source File

SOURCE=..\src\lib\agg_vcgen_stroke.cpp
# End Source File
# Begin Source File

SOURCE=..\src\picasso.cpp
# End Source File
# Begin Source File

SOURCE=..\src\picasso_canvas.cpp
# End Source File
# Begin Source File

SOURCE=..\src\picasso_engine_freetype2.cpp
# End Source File
# Begin Source File

SOURCE=..\src\picasso_engine_win32.cpp
# End Source File
# Begin Source File

SOURCE=..\src\picasso_font.cpp
# End Source File
# Begin Source File

SOURCE=..\src\picasso_freetype2_load.cpp
# End Source File
# Begin Source File

SOURCE=..\src\picasso_gpc.cpp
# End Source File
# Begin Source File

SOURCE=..\src\picasso_gradient.cpp
# End Source File
# Begin Source File

SOURCE=..\src\picasso_image.cpp
# End Source File
# Begin Source File

SOURCE=..\src\picasso_mask.cpp
# End Source File
# Begin Source File

SOURCE=..\src\picasso_matrix.cpp
# End Source File
# Begin Source File

SOURCE=..\src\picasso_path.cpp
# End Source File
# Begin Source File

SOURCE=..\src\picasso_pattern.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\src\include\agg_alpha_mask_u8.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_arc.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_array.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_basics.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_bezier_arc.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_bitset_iterator.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_blur.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_bounding_rect.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_bspline.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_clip_liang_barsky.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_color_conv.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_color_conv_rgb16.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_color_conv_rgb8.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_color_gray.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_color_rgba.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_config.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_conv_adaptor_vcgen.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_conv_adaptor_vpgen.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_conv_bspline.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_conv_clip_polygon.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_conv_clip_polyline.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_conv_close_polygon.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_conv_concat.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_conv_contour.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_conv_curve.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_conv_dash.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_conv_gpc.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_conv_segmentator.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_conv_shorten_path.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_conv_smooth_poly1.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_conv_stroke.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_conv_transform.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_conv_unclose_polygon.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_curves.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_dda_line.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_ellipse.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_ellipse_bresenham.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_font_cache_manager.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_font_freetype.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_font_win32_tt.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_gamma_functions.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_gamma_lut.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_glyph_raster_bin.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_gradient_lut.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_image_accessors.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_image_filters.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_line_aa_basics.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_math.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_math_stroke.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_path_length.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_path_storage.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_path_storage_integer.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_pattern_filters_rgba.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_pixfmt_amask_adaptor.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_pixfmt_gray.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_pixfmt_rgb.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_pixfmt_rgb_packed.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_pixfmt_rgba.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_pixfmt_transposer.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_rasterizer_cells_aa.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_rasterizer_compound_aa.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_rasterizer_outline.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_rasterizer_outline_aa.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_rasterizer_scanline_aa.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_rasterizer_sl_clip.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_renderer_base.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_renderer_mclip.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_renderer_outline_aa.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_renderer_outline_image.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_renderer_primitives.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_renderer_scanline.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_rendering_buffer.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_rendering_buffer_dynarow.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_rounded_rect.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_scanline_bin.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_scanline_boolean_algebra.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_scanline_p.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_scanline_storage_aa.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_scanline_storage_bin.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_scanline_u.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_shorten_path.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_simul_eq.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_span_allocator.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_span_converter.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_span_gouraud.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_span_gouraud_gray.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_span_gouraud_rgba.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_span_gradient.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_span_gradient_alpha.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_span_image_filter.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_span_image_filter_gray.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_span_image_filter_rgb.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_span_image_filter_rgb_packed.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_span_image_filter_rgba.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_span_interpolator_adaptor.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_span_interpolator_linear.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_span_interpolator_persp.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_span_interpolator_trans.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_span_pattern_gray.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_span_pattern_rgb.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_span_pattern_rgba.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_span_solid.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_span_subdiv_adaptor.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_trans_affine.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_trans_bilinear.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_trans_perspective.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_trans_single_path.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_trans_viewport.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_vcgen_bspline.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_vcgen_contour.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_vcgen_dash.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_vcgen_smooth_poly1.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_vcgen_stroke.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_vcgen_vertex_sequence.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_vertex_sequence.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_vpgen_clip_polygon.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_vpgen_clip_polyline.h
# End Source File
# Begin Source File

SOURCE=..\src\include\agg_vpgen_segmentator.h
# End Source File
# Begin Source File

SOURCE=..\src\aggheader.h
# End Source File
# Begin Source File

SOURCE=..\pconfig.h
# End Source File
# Begin Source File

SOURCE=..\include\picasso.h
# End Source File
# Begin Source File

SOURCE=..\src\picasso_gpc.h
# End Source File
# Begin Source File

SOURCE=..\src\picasso_helper.h
# End Source File
# Begin Source File

SOURCE=..\src\picasso_p.h
# End Source File
# Begin Source File

SOURCE=..\src\picasso_painter.h
# End Source File
# Begin Source File

SOURCE=..\src\picasso_wrapper.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
