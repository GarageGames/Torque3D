//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#ifdef WGL_3DFX_multisample
#define WGL_SAMPLE_BUFFERS_3DFX 0x2060
#define WGL_SAMPLES_3DFX 0x2061
#endif

#ifdef WGL_ARB_buffer_region
#define WGL_FRONT_COLOR_BUFFER_BIT_ARB 0x00000001
#define WGL_BACK_COLOR_BUFFER_BIT_ARB 0x00000002
#define WGL_DEPTH_BUFFER_BIT_ARB 0x00000004
#define WGL_STENCIL_BUFFER_BIT_ARB 0x00000008
#define wglCreateBufferRegionARB XGL_FUNCPTR(wglCreateBufferRegionARB)
#define wglDeleteBufferRegionARB XGL_FUNCPTR(wglDeleteBufferRegionARB)
#define wglRestoreBufferRegionARB XGL_FUNCPTR(wglRestoreBufferRegionARB)
#define wglSaveBufferRegionARB XGL_FUNCPTR(wglSaveBufferRegionARB)
#endif

#ifdef WGL_ARB_extensions_string
#define wglGetExtensionsStringARB XGL_FUNCPTR(wglGetExtensionsStringARB)
#endif

#ifdef WGL_ARB_make_current_read
#define wglGetCurrentReadDCARB XGL_FUNCPTR(wglGetCurrentReadDCARB)
#define wglMakeContextCurrentARB XGL_FUNCPTR(wglMakeContextCurrentARB)
#endif

#ifdef WGL_ARB_multisample
#define WGL_SAMPLE_BUFFERS_ARB 0x2041
#define WGL_SAMPLES_ARB 0x2042
#endif

#ifdef WGL_ARB_pbuffer
DECLARE_HANDLE(HPBUFFERARB);
#define WGL_DRAW_TO_PBUFFER_ARB 0x202D
#define WGL_MAX_PBUFFER_PIXELS_ARB 0x202E
#define WGL_MAX_PBUFFER_WIDTH_ARB 0x202F
#define WGL_MAX_PBUFFER_HEIGHT_ARB 0x2030
#define WGL_PBUFFER_LARGEST_ARB 0x2033
#define WGL_PBUFFER_WIDTH_ARB 0x2034
#define WGL_PBUFFER_HEIGHT_ARB 0x2035
#define WGL_PBUFFER_LOST_ARB 0x2036
#define wglCreatePbufferARB XGL_FUNCPTR(wglCreatePbufferARB)
#define wglDestroyPbufferARB XGL_FUNCPTR(wglDestroyPbufferARB)
#define wglGetPbufferDCARB XGL_FUNCPTR(wglGetPbufferDCARB)
#define wglQueryPbufferARB XGL_FUNCPTR(wglQueryPbufferARB)
#define wglReleasePbufferDCARB XGL_FUNCPTR(wglReleasePbufferDCARB)
#endif

#ifdef WGL_ARB_pixel_format
#define WGL_NUMBER_PIXEL_FORMATS_ARB 0x2000
#define WGL_DRAW_TO_WINDOW_ARB 0x2001
#define WGL_DRAW_TO_BITMAP_ARB 0x2002
#define WGL_ACCELERATION_ARB 0x2003
#define WGL_NEED_PALETTE_ARB 0x2004
#define WGL_NEED_SYSTEM_PALETTE_ARB 0x2005
#define WGL_SWAP_LAYER_BUFFERS_ARB 0x2006
#define WGL_SWAP_METHOD_ARB 0x2007
#define WGL_NUMBER_OVERLAYS_ARB 0x2008
#define WGL_NUMBER_UNDERLAYS_ARB 0x2009
#define WGL_TRANSPARENT_ARB 0x200A
#define WGL_SHARE_DEPTH_ARB 0x200C
#define WGL_SHARE_STENCIL_ARB 0x200D
#define WGL_SHARE_ACCUM_ARB 0x200E
#define WGL_SUPPORT_GDI_ARB 0x200F
#define WGL_SUPPORT_OPENGL_ARB 0x2010
#define WGL_DOUBLE_BUFFER_ARB 0x2011
#define WGL_STEREO_ARB 0x2012
#define WGL_PIXEL_TYPE_ARB 0x2013
#define WGL_COLOR_BITS_ARB 0x2014
#define WGL_RED_BITS_ARB 0x2015
#define WGL_RED_SHIFT_ARB 0x2016
#define WGL_GREEN_BITS_ARB 0x2017
#define WGL_GREEN_SHIFT_ARB 0x2018
#define WGL_BLUE_BITS_ARB 0x2019
#define WGL_BLUE_SHIFT_ARB 0x201A
#define WGL_ALPHA_BITS_ARB 0x201B
#define WGL_ALPHA_SHIFT_ARB 0x201C
#define WGL_ACCUM_BITS_ARB 0x201D
#define WGL_ACCUM_RED_BITS_ARB 0x201E
#define WGL_ACCUM_GREEN_BITS_ARB 0x201F
#define WGL_ACCUM_BLUE_BITS_ARB 0x2020
#define WGL_ACCUM_ALPHA_BITS_ARB 0x2021
#define WGL_DEPTH_BITS_ARB 0x2022
#define WGL_STENCIL_BITS_ARB 0x2023
#define WGL_AUX_BUFFERS_ARB 0x2024
#define WGL_NO_ACCELERATION_ARB 0x2025
#define WGL_GENERIC_ACCELERATION_ARB 0x2026
#define WGL_FULL_ACCELERATION_ARB 0x2027
#define WGL_SWAP_EXCHANGE_ARB 0x2028
#define WGL_SWAP_COPY_ARB 0x2029
#define WGL_SWAP_UNDEFINED_ARB 0x202A
#define WGL_TYPE_RGBA_ARB 0x202B
#define WGL_TYPE_COLORINDEX_ARB 0x202C
#define WGL_TRANSPARENT_RED_VALUE_ARB 0x2037
#define WGL_TRANSPARENT_GREEN_VALUE_ARB 0x2038
#define WGL_TRANSPARENT_BLUE_VALUE_ARB 0x2039
#define WGL_TRANSPARENT_ALPHA_VALUE_ARB 0x203A
#define WGL_TRANSPARENT_INDEX_VALUE_ARB 0x203B
#define wglChoosePixelFormatARB XGL_FUNCPTR(wglChoosePixelFormatARB)
#define wglGetPixelFormatAttribfvARB XGL_FUNCPTR(wglGetPixelFormatAttribfvARB)
#define wglGetPixelFormatAttribivARB XGL_FUNCPTR(wglGetPixelFormatAttribivARB)
#endif

#ifdef WGL_ARB_pixel_format_float
#define WGL_TYPE_RGBA_FLOAT_ARB 0x21A0
#endif

#ifdef WGL_ARB_render_texture
#define WGL_BIND_TO_TEXTURE_RGB_ARB 0x2070
#define WGL_BIND_TO_TEXTURE_RGBA_ARB 0x2071
#define WGL_TEXTURE_FORMAT_ARB 0x2072
#define WGL_TEXTURE_TARGET_ARB 0x2073
#define WGL_MIPMAP_TEXTURE_ARB 0x2074
#define WGL_TEXTURE_RGB_ARB 0x2075
#define WGL_TEXTURE_RGBA_ARB 0x2076
#define WGL_NO_TEXTURE_ARB 0x2077
#define WGL_TEXTURE_CUBE_MAP_ARB 0x2078
#define WGL_TEXTURE_1D_ARB 0x2079
#define WGL_TEXTURE_2D_ARB 0x207A
#define WGL_MIPMAP_LEVEL_ARB 0x207B
#define WGL_CUBE_MAP_FACE_ARB 0x207C
#define WGL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB 0x207D
#define WGL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB 0x207E
#define WGL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB 0x207F
#define WGL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB 0x2080
#define WGL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB 0x2081
#define WGL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB 0x2082
#define WGL_FRONT_LEFT_ARB 0x2083
#define WGL_FRONT_RIGHT_ARB 0x2084
#define WGL_BACK_LEFT_ARB 0x2085
#define WGL_BACK_RIGHT_ARB 0x2086
#define WGL_AUX0_ARB 0x2087
#define WGL_AUX1_ARB 0x2088
#define WGL_AUX2_ARB 0x2089
#define WGL_AUX3_ARB 0x208A
#define WGL_AUX4_ARB 0x208B
#define WGL_AUX5_ARB 0x208C
#define WGL_AUX6_ARB 0x208D
#define WGL_AUX7_ARB 0x208E
#define WGL_AUX8_ARB 0x208F
#define WGL_AUX9_ARB 0x2090
#define wglBindTexImageARB XGL_FUNCPTR(wglBindTexImageARB)
#define wglReleaseTexImageARB XGL_FUNCPTR(wglReleaseTexImageARB)
#define wglSetPbufferAttribARB XGL_FUNCPTR(wglSetPbufferAttribARB)
#endif

#ifdef WGL_ATI_pixel_format_float
#define WGL_TYPE_RGBA_FLOAT_ATI 0x21A0
#define GL_RGBA_FLOAT_MODE_ATI 0x8820
#define GL_COLOR_CLEAR_UNCLAMPED_VALUE_ATI 0x8835
#endif

#ifdef WGL_ATI_render_texture_rectangle
#define WGL_TEXTURE_RECTANGLE_ATI 0x21A5
#endif

#ifdef WGL_EXT_depth_float
#define WGL_DEPTH_FLOAT_EXT 0x2040
#endif

#ifdef WGL_EXT_display_color_table
#define wglBindDisplayColorTableEXT XGL_FUNCPTR(wglBindDisplayColorTableEXT)
#define wglCreateDisplayColorTableEXT XGL_FUNCPTR(wglCreateDisplayColorTableEXT)
#define wglDestroyDisplayColorTableEXT XGL_FUNCPTR(wglDestroyDisplayColorTableEXT)
#define wglLoadDisplayColorTableEXT XGL_FUNCPTR(wglLoadDisplayColorTableEXT)
#endif

#ifdef WGL_EXT_extensions_string
#define wglGetExtensionsStringEXT XGL_FUNCPTR(wglGetExtensionsStringEXT)
#endif

#ifdef WGL_EXT_make_current_read
#define wglGetCurrentReadDCEXT XGL_FUNCPTR(wglGetCurrentReadDCEXT)
#define wglMakeContextCurrentEXT XGL_FUNCPTR(wglMakeContextCurrentEXT)
#endif

#ifdef WGL_EXT_multisample
#define WGL_SAMPLE_BUFFERS_EXT 0x2041
#define WGL_SAMPLES_EXT 0x2042
#endif

#ifdef WGL_EXT_pbuffer
DECLARE_HANDLE(HPBUFFEREXT);;
#define WGL_DRAW_TO_PBUFFER_EXT 0x202D
#define WGL_MAX_PBUFFER_PIXELS_EXT 0x202E
#define WGL_MAX_PBUFFER_WIDTH_EXT 0x202F
#define WGL_MAX_PBUFFER_HEIGHT_EXT 0x2030
#define WGL_OPTIMAL_PBUFFER_WIDTH_EXT 0x2031
#define WGL_OPTIMAL_PBUFFER_HEIGHT_EXT 0x2032
#define WGL_PBUFFER_LARGEST_EXT 0x2033
#define WGL_PBUFFER_WIDTH_EXT 0x2034
#define WGL_PBUFFER_HEIGHT_EXT 0x2035
#define wglCreatePbufferEXT XGL_FUNCPTR(wglCreatePbufferEXT)
#define wglDestroyPbufferEXT XGL_FUNCPTR(wglDestroyPbufferEXT)
#define wglGetPbufferDCEXT XGL_FUNCPTR(wglGetPbufferDCEXT)
#define wglQueryPbufferEXT XGL_FUNCPTR(wglQueryPbufferEXT)
#define wglReleasePbufferDCEXT XGL_FUNCPTR(wglReleasePbufferDCEXT)
#endif

#ifdef WGL_EXT_pixel_format
#define WGL_NUMBER_PIXEL_FORMATS_EXT 0x2000
#define WGL_DRAW_TO_WINDOW_EXT 0x2001
#define WGL_DRAW_TO_BITMAP_EXT 0x2002
#define WGL_ACCELERATION_EXT 0x2003
#define WGL_NEED_PALETTE_EXT 0x2004
#define WGL_NEED_SYSTEM_PALETTE_EXT 0x2005
#define WGL_SWAP_LAYER_BUFFERS_EXT 0x2006
#define WGL_SWAP_METHOD_EXT 0x2007
#define WGL_NUMBER_OVERLAYS_EXT 0x2008
#define WGL_NUMBER_UNDERLAYS_EXT 0x2009
#define WGL_TRANSPARENT_EXT 0x200A
#define WGL_TRANSPARENT_VALUE_EXT 0x200B
#define WGL_SHARE_DEPTH_EXT 0x200C
#define WGL_SHARE_STENCIL_EXT 0x200D
#define WGL_SHARE_ACCUM_EXT 0x200E
#define WGL_SUPPORT_GDI_EXT 0x200F
#define WGL_SUPPORT_OPENGL_EXT 0x2010
#define WGL_DOUBLE_BUFFER_EXT 0x2011
#define WGL_STEREO_EXT 0x2012
#define WGL_PIXEL_TYPE_EXT 0x2013
#define WGL_COLOR_BITS_EXT 0x2014
#define WGL_RED_BITS_EXT 0x2015
#define WGL_RED_SHIFT_EXT 0x2016
#define WGL_GREEN_BITS_EXT 0x2017
#define WGL_GREEN_SHIFT_EXT 0x2018
#define WGL_BLUE_BITS_EXT 0x2019
#define WGL_BLUE_SHIFT_EXT 0x201A
#define WGL_ALPHA_BITS_EXT 0x201B
#define WGL_ALPHA_SHIFT_EXT 0x201C
#define WGL_ACCUM_BITS_EXT 0x201D
#define WGL_ACCUM_RED_BITS_EXT 0x201E
#define WGL_ACCUM_GREEN_BITS_EXT 0x201F
#define WGL_ACCUM_BLUE_BITS_EXT 0x2020
#define WGL_ACCUM_ALPHA_BITS_EXT 0x2021
#define WGL_DEPTH_BITS_EXT 0x2022
#define WGL_STENCIL_BITS_EXT 0x2023
#define WGL_AUX_BUFFERS_EXT 0x2024
#define WGL_NO_ACCELERATION_EXT 0x2025
#define WGL_GENERIC_ACCELERATION_EXT 0x2026
#define WGL_FULL_ACCELERATION_EXT 0x2027
#define WGL_SWAP_EXCHANGE_EXT 0x2028
#define WGL_SWAP_COPY_EXT 0x2029
#define WGL_SWAP_UNDEFINED_EXT 0x202A
#define WGL_TYPE_RGBA_EXT 0x202B
#define WGL_TYPE_COLORINDEX_EXT 0x202C
#define wglChoosePixelFormatEXT XGL_FUNCPTR(wglChoosePixelFormatEXT)
#define wglGetPixelFormatAttribfvEXT XGL_FUNCPTR(wglGetPixelFormatAttribfvEXT)
#define wglGetPixelFormatAttribivEXT XGL_FUNCPTR(wglGetPixelFormatAttribivEXT)
#endif

#ifdef WGL_EXT_swap_control
#define wglGetSwapIntervalEXT XGL_FUNCPTR(wglGetSwapIntervalEXT)
#define wglSwapIntervalEXT XGL_FUNCPTR(wglSwapIntervalEXT)
#endif

#ifdef WGL_I3D_digital_video_control
#define WGL_DIGITAL_VIDEO_CURSOR_ALPHA_FRAMEBUFFER_I3D 0x2050
#define WGL_DIGITAL_VIDEO_CURSOR_ALPHA_VALUE_I3D 0x2051
#define WGL_DIGITAL_VIDEO_CURSOR_INCLUDED_I3D 0x2052
#define WGL_DIGITAL_VIDEO_GAMMA_CORRECTED_I3D 0x2053
#define wglGetDigitalVideoParametersI3D XGL_FUNCPTR(wglGetDigitalVideoParametersI3D)
#define wglSetDigitalVideoParametersI3D XGL_FUNCPTR(wglSetDigitalVideoParametersI3D)
#endif

#ifdef WGL_I3D_gamma
#define WGL_GAMMA_TABLE_SIZE_I3D 0x204E
#define WGL_GAMMA_EXCLUDE_DESKTOP_I3D 0x204F
#define wglGetGammaTableI3D XGL_FUNCPTR(wglGetGammaTableI3D)
#define wglGetGammaTableParametersI3D XGL_FUNCPTR(wglGetGammaTableParametersI3D)
#define wglSetGammaTableI3D XGL_FUNCPTR(wglSetGammaTableI3D)
#define wglSetGammaTableParametersI3D XGL_FUNCPTR(wglSetGammaTableParametersI3D)
#endif

#ifdef WGL_I3D_genlock
#define WGL_GENLOCK_SOURCE_MULTIVIEW_I3D 0x2044
#define WGL_GENLOCK_SOURCE_EXTERNAL_SYNC_I3D 0x2045
#define WGL_GENLOCK_SOURCE_EXTERNAL_FIELD_I3D 0x2046
#define WGL_GENLOCK_SOURCE_EXTERNAL_TTL_I3D 0x2047
#define WGL_GENLOCK_SOURCE_DIGITAL_SYNC_I3D 0x2048
#define WGL_GENLOCK_SOURCE_DIGITAL_FIELD_I3D 0x2049
#define WGL_GENLOCK_SOURCE_EDGE_FALLING_I3D 0x204A
#define WGL_GENLOCK_SOURCE_EDGE_RISING_I3D 0x204B
#define WGL_GENLOCK_SOURCE_EDGE_BOTH_I3D 0x204C
#define wglDisableGenlockI3D XGL_FUNCPTR(wglDisableGenlockI3D)
#define wglEnableGenlockI3D XGL_FUNCPTR(wglEnableGenlockI3D)
#define wglGenlockSampleRateI3D XGL_FUNCPTR(wglGenlockSampleRateI3D)
#define wglGenlockSourceDelayI3D XGL_FUNCPTR(wglGenlockSourceDelayI3D)
#define wglGenlockSourceEdgeI3D XGL_FUNCPTR(wglGenlockSourceEdgeI3D)
#define wglGenlockSourceI3D XGL_FUNCPTR(wglGenlockSourceI3D)
#define wglGetGenlockSampleRateI3D XGL_FUNCPTR(wglGetGenlockSampleRateI3D)
#define wglGetGenlockSourceDelayI3D XGL_FUNCPTR(wglGetGenlockSourceDelayI3D)
#define wglGetGenlockSourceEdgeI3D XGL_FUNCPTR(wglGetGenlockSourceEdgeI3D)
#define wglGetGenlockSourceI3D XGL_FUNCPTR(wglGetGenlockSourceI3D)
#define wglIsEnabledGenlockI3D XGL_FUNCPTR(wglIsEnabledGenlockI3D)
#define wglQueryGenlockMaxSourceDelayI3D XGL_FUNCPTR(wglQueryGenlockMaxSourceDelayI3D)
#endif

#ifdef WGL_I3D_image_buffer
#define WGL_IMAGE_BUFFER_MIN_ACCESS_I3D 0x00000001
#define WGL_IMAGE_BUFFER_LOCK_I3D 0x00000002
#define wglAssociateImageBufferEventsI3D XGL_FUNCPTR(wglAssociateImageBufferEventsI3D)
#define wglCreateImageBufferI3D XGL_FUNCPTR(wglCreateImageBufferI3D)
#define wglDestroyImageBufferI3D XGL_FUNCPTR(wglDestroyImageBufferI3D)
#define wglReleaseImageBufferEventsI3D XGL_FUNCPTR(wglReleaseImageBufferEventsI3D)
#endif

#ifdef WGL_I3D_swap_frame_lock
#define wglDisableFrameLockI3D XGL_FUNCPTR(wglDisableFrameLockI3D)
#define wglEnableFrameLockI3D XGL_FUNCPTR(wglEnableFrameLockI3D)
#define wglIsEnabledFrameLockI3D XGL_FUNCPTR(wglIsEnabledFrameLockI3D)
#define wglQueryFrameLockMasterI3D XGL_FUNCPTR(wglQueryFrameLockMasterI3D)
#endif

#ifdef WGL_I3D_swap_frame_usage
#define wglBeginFrameTrackingI3D XGL_FUNCPTR(wglBeginFrameTrackingI3D)
#define wglEndFrameTrackingI3D XGL_FUNCPTR(wglEndFrameTrackingI3D)
#define wglGetFrameUsageI3D XGL_FUNCPTR(wglGetFrameUsageI3D)
#define wglQueryFrameTrackingI3D XGL_FUNCPTR(wglQueryFrameTrackingI3D)
#endif

#ifdef WGL_NV_float_buffer
#define WGL_FLOAT_COMPONENTS_NV 0x20B0
#define WGL_BIND_TO_TEXTURE_RECTANGLE_FLOAT_R_NV 0x20B1
#define WGL_BIND_TO_TEXTURE_RECTANGLE_FLOAT_RG_NV 0x20B2
#define WGL_BIND_TO_TEXTURE_RECTANGLE_FLOAT_RGB_NV 0x20B3
#define WGL_BIND_TO_TEXTURE_RECTANGLE_FLOAT_RGBA_NV 0x20B4
#define WGL_TEXTURE_FLOAT_R_NV 0x20B5
#define WGL_TEXTURE_FLOAT_RG_NV 0x20B6
#define WGL_TEXTURE_FLOAT_RGB_NV 0x20B7
#define WGL_TEXTURE_FLOAT_RGBA_NV 0x20B8
#endif

#ifdef WGL_NV_render_depth_texture
#define WGL_NO_TEXTURE_ARB 0x2077
#define WGL_BIND_TO_TEXTURE_DEPTH_NV 0x20A3
#define WGL_BIND_TO_TEXTURE_RECTANGLE_DEPTH_NV 0x20A4
#define WGL_DEPTH_TEXTURE_FORMAT_NV 0x20A5
#define WGL_TEXTURE_DEPTH_COMPONENT_NV 0x20A6
#define WGL_DEPTH_COMPONENT_NV 0x20A7
#endif

#ifdef WGL_NV_render_texture_rectangle
#define WGL_BIND_TO_TEXTURE_RECTANGLE_RGB_NV 0x20A0
#define WGL_BIND_TO_TEXTURE_RECTANGLE_RGBA_NV 0x20A1
#define WGL_TEXTURE_RECTANGLE_NV 0x20A2
#endif

#ifdef WGL_NV_vertex_array_range
#define wglAllocateMemoryNV XGL_FUNCPTR(wglAllocateMemoryNV)
#define wglFreeMemoryNV XGL_FUNCPTR(wglFreeMemoryNV)
#endif

#ifdef WGL_OML_sync_control
#define wglGetMscRateOML XGL_FUNCPTR(wglGetMscRateOML)
#define wglGetSyncValuesOML XGL_FUNCPTR(wglGetSyncValuesOML)
#define wglSwapBuffersMscOML XGL_FUNCPTR(wglSwapBuffersMscOML)
#define wglSwapLayerBuffersMscOML XGL_FUNCPTR(wglSwapLayerBuffersMscOML)
#define wglWaitForMscOML XGL_FUNCPTR(wglWaitForMscOML)
#define wglWaitForSbcOML XGL_FUNCPTR(wglWaitForSbcOML)
#endif

