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

#ifdef GLX_3DFX_multisample
#define GLX_SAMPLE_BUFFERS_3DFX 0x8050
#define GLX_SAMPLES_3DFX 0x8051
#endif

#ifdef GLX_ARB_fbconfig_float
#define GLX_RGBA_FLOAT_BIT 0x00000004
#define GLX_RGBA_FLOAT_TYPE 0x20B9
#endif

#ifdef GLX_ARB_get_proc_address
#define glXGetProcAddressARB XGL_FUNCPTR(glXGetProcAddressARB)
#endif

#ifdef GLX_ARB_multisample
#define GLX_SAMPLE_BUFFERS_ARB 100000
#define GLX_SAMPLES_ARB 100001
#endif

#ifdef GLX_ATI_pixel_format_float
#define GLX_RGBA_FLOAT_ATI_BIT              0x00000100
#endif

#ifdef GLX_ATI_render_texture
#define GLX_BIND_TO_TEXTURE_RGB_ATI         0x9800
#define GLX_BIND_TO_TEXTURE_RGBA_ATI        0x9801
#define GLX_TEXTURE_FORMAT_ATI              0x9802
#define GLX_TEXTURE_TARGET_ATI              0x9803
#define GLX_MIPMAP_TEXTURE_ATI              0x9804
#define GLX_TEXTURE_RGB_ATI                 0x9805
#define GLX_TEXTURE_RGBA_ATI                0x9806
#define GLX_NO_TEXTURE_ATI                  0x9807
#define GLX_TEXTURE_CUBE_MAP_ATI            0x9808
#define GLX_TEXTURE_1D_ATI                  0x9809
#define GLX_TEXTURE_2D_ATI                  0x980A
#define GLX_MIPMAP_LEVEL_ATI                0x980B
#define GLX_CUBE_MAP_FACE_ATI               0x980C
#define GLX_TEXTURE_CUBE_MAP_POSITIVE_X_ATI 0x980D
#define GLX_TEXTURE_CUBE_MAP_NEGATIVE_X_ATI 0x980E
#define GLX_TEXTURE_CUBE_MAP_POSITIVE_Y_ATI 0x980F
#define GLX_TEXTURE_CUBE_MAP_NEGATIVE_Y_ATI 0x9810
#define GLX_TEXTURE_CUBE_MAP_POSITIVE_Z_ATI 0x9811
#define GLX_TEXTURE_CUBE_MAP_NEGATIVE_Z_ATI 0x9812
#define GLX_FRONT_LEFT_ATI                  0x9813
#define GLX_FRONT_RIGHT_ATI                 0x9814
#define GLX_BACK_LEFT_ATI                   0x9815
#define GLX_BACK_RIGHT_ATI                  0x9816
#define GLX_AUX0_ATI                        0x9817
#define GLX_AUX1_ATI                        0x9818
#define GLX_AUX2_ATI                        0x9819
#define GLX_AUX3_ATI                        0x981A
#define GLX_AUX4_ATI                        0x981B
#define GLX_AUX5_ATI                        0x981C
#define GLX_AUX6_ATI                        0x981D
#define GLX_AUX7_ATI                        0x981E
#define GLX_AUX8_ATI                        0x981F
#define GLX_AUX9_ATI                        0x9820
#define GLX_BIND_TO_TEXTURE_LUMINANCE_ATI   0x9821
#define GLX_BIND_TO_TEXTURE_INTENSITY_ATI   0x9822
#define glXBindTexImageATI XGL_FUNCPTR(glXBindTexImageATI)
#define glXReleaseTexImageATI XGL_FUNCPTR(glXReleaseTexImageATI)
#define glXDrawableAttribATI XGL_FUNCPTR(glXDrawableAttribATI)
#endif

#ifdef GLX_EXT_import_context
typedef XID GLXContextID;
#define GLX_SHARE_CONTEXT_EXT 0x800A
#define GLX_VISUAL_ID_EXT 0x800B
#define GLX_SCREEN_EXT 0x800C
#define glXFreeContextEXT XGL_FUNCPTR(glXFreeContextEXT)
#define glXGetContextIDEXT XGL_FUNCPTR(glXGetContextIDEXT)
#define glXImportContextEXT XGL_FUNCPTR(glXImportContextEXT)
#define glXQueryContextInfoEXT XGL_FUNCPTR(glXQueryContextInfoEXT)
#endif

#ifdef GLX_EXT_scene_marker
#endif

#ifdef GLX_EXT_visual_info
#define GLX_X_VISUAL_TYPE_EXT 0x22
#define GLX_TRANSPARENT_TYPE_EXT 0x23
#define GLX_TRANSPARENT_INDEX_VALUE_EXT 0x24
#define GLX_TRANSPARENT_RED_VALUE_EXT 0x25
#define GLX_TRANSPARENT_GREEN_VALUE_EXT 0x26
#define GLX_TRANSPARENT_BLUE_VALUE_EXT 0x27
#define GLX_TRANSPARENT_ALPHA_VALUE_EXT 0x28
#define GLX_NONE_EXT 0x8000
#define GLX_TRUE_COLOR_EXT 0x8002
#define GLX_DIRECT_COLOR_EXT 0x8003
#define GLX_PSEUDO_COLOR_EXT 0x8004
#define GLX_STATIC_COLOR_EXT 0x8005
#define GLX_GRAY_SCALE_EXT 0x8006
#define GLX_STATIC_GRAY_EXT 0x8007
#define GLX_TRANSPARENT_RGB_EXT 0x8008
#define GLX_TRANSPARENT_INDEX_EXT 0x8009
#endif

#ifdef GLX_EXT_visual_rating
#define GLX_VISUAL_CAVEAT_EXT 0x20
#define GLX_SLOW_VISUAL_EXT 0x8001
#define GLX_NON_CONFORMANT_VISUAL_EXT 0x800D
#endif

#ifdef GLX_MESA_agp_offset
#define glXGetAGPOffsetMESA XGL_FUNCPTR(glXGetAGPOffsetMESA)
#endif

#ifdef GLX_MESA_copy_sub_buffer
#define glXCopySubBufferMESA XGL_FUNCPTR(glXCopySubBufferMESA)
#endif

#ifdef GLX_MESA_pixmap_colormap
#define glXCreateGLXPixmapMESA XGL_FUNCPTR(glXCreateGLXPixmapMESA)
#endif

#ifdef GLX_MESA_release_buffers
#define glXReleaseBuffersMESA XGL_FUNCPTR(glXReleaseBuffersMESA)
#endif

#ifdef GLX_MESA_set_3dfx_mode
#define GLX_3DFX_WINDOW_MODE_MESA 0x1
#define GLX_3DFX_FULLSCREEN_MODE_MESA 0x2
#define glXSet3DfxModeMESA XGL_FUNCPTR(glXSet3DfxModeMESA)
#endif

#ifdef GLX_NV_float_buffer
#define GLX_FLOAT_COMPONENTS_NV 0x20B0
#endif

#ifdef GLX_NV_vertex_array_range
#define glXAllocateMemoryNV XGL_FUNCPTR(glXAllocateMemoryNV)
#define glXFreeMemoryNV XGL_FUNCPTR(glXFreeMemoryNV)
#endif

#ifdef GLX_OML_swap_method
#define GLX_SWAP_METHOD_OML 0x8060
#define GLX_SWAP_EXCHANGE_OML 0x8061
#define GLX_SWAP_COPY_OML 0x8062
#define GLX_SWAP_UNDEFINED_OML 0x8063
#endif

#ifdef GLX_OML_sync_control
#define glXGetMscRateOML XGL_FUNCPTR(glXGetMscRateOML)
#define glXGetSyncValuesOML XGL_FUNCPTR(glXGetSyncValuesOML)
#define glXSwapBuffersMscOML XGL_FUNCPTR(glXSwapBuffersMscOML)
#define glXWaitForMscOML XGL_FUNCPTR(glXWaitForMscOML)
#define glXWaitForSbcOML XGL_FUNCPTR(glXWaitForSbcOML)
#endif

#ifdef GLX_SGIS_blended_overlay
#define GLX_BLENDED_RGBA_SGIS 0x8025
#endif

#ifdef GLX_SGIS_color_range
#define GLX_MAX_GREEN_SGIS 0
#define GLX_MIN_RED_SGIS 0
#define GLX_MIN_BLUE_SGIS 0
#define GLX_MAX_RED_SGIS 0
#define GLX_MAX_ALPHA_SGIS 0
#define GLX_MIN_GREEN_SGIS 0
#define GLX_MIN_ALPHA_SGIS 0
#define GLX_EXTENDED_RANGE_SGIS 0
#define GLX_MAX_BLUE_SGIS 0
#endif

#ifdef GLX_SGIS_multisample
#define GLX_SAMPLE_BUFFERS_SGIS 100000
#define GLX_SAMPLES_SGIS 100001
#endif

#ifdef GLX_SGIS_shared_multisample
#define GLX_MULTISAMPLE_SUB_RECT_WIDTH_SGIS 0x8026
#define GLX_MULTISAMPLE_SUB_RECT_HEIGHT_SGIS 0x8027
#endif

#ifdef GLX_SGIX_fbconfig
typedef XID GLXFBConfigIDSGIX;
typedef struct __GLXFBConfigRec *GLXFBConfigSGIX;
#define GLX_WINDOW_BIT_SGIX 0x00000001
#define GLX_RGBA_BIT_SGIX 0x00000001
#define GLX_PIXMAP_BIT_SGIX 0x00000002
#define GLX_COLOR_INDEX_BIT_SGIX 0x00000002
#define GLX_SCREEN_EXT 0x800C
#define GLX_DRAWABLE_TYPE_SGIX 0x8010
#define GLX_RENDER_TYPE_SGIX 0x8011
#define GLX_X_RENDERABLE_SGIX 0x8012
#define GLX_FBCONFIG_ID_SGIX 0x8013
#define GLX_RGBA_TYPE_SGIX 0x8014
#define GLX_COLOR_INDEX_TYPE_SGIX 0x8015
#define glXChooseFBConfigSGIX XGL_FUNCPTR(glXChooseFBConfigSGIX)
#define glXCreateContextWithConfigSGIX XGL_FUNCPTR(glXCreateContextWithConfigSGIX)
#define glXCreateGLXPixmapWithConfigSGIX XGL_FUNCPTR(glXCreateGLXPixmapWithConfigSGIX)
#define glXGetFBConfigAttribSGIX XGL_FUNCPTR(glXGetFBConfigAttribSGIX)
#define glXGetFBConfigFromVisualSGIX XGL_FUNCPTR(glXGetFBConfigFromVisualSGIX)
#define glXGetVisualFromFBConfigSGIX XGL_FUNCPTR(glXGetVisualFromFBConfigSGIX)
#endif

#ifdef GLX_SGIX_pbuffer
typedef XID GLXPbufferSGIX;
typedef struct { int type; unsigned long serial; Bool send_event; Display *display; GLXDrawable drawable; int event_type; int draw_type; unsigned int mask; int x, y; int width, height; int count; } GLXBufferClobberEventSGIX;
#define GLX_FRONT_LEFT_BUFFER_BIT_SGIX 0x00000001
#define GLX_FRONT_RIGHT_BUFFER_BIT_SGIX 0x00000002
#define GLX_PBUFFER_BIT_SGIX 0x00000004
#define GLX_BACK_LEFT_BUFFER_BIT_SGIX 0x00000004
#define GLX_BACK_RIGHT_BUFFER_BIT_SGIX 0x00000008
#define GLX_AUX_BUFFERS_BIT_SGIX 0x00000010
#define GLX_DEPTH_BUFFER_BIT_SGIX 0x00000020
#define GLX_STENCIL_BUFFER_BIT_SGIX 0x00000040
#define GLX_ACCUM_BUFFER_BIT_SGIX 0x00000080
#define GLX_SAMPLE_BUFFERS_BIT_SGIX 0x00000100
#define GLX_MAX_PBUFFER_WIDTH_SGIX 0x8016
#define GLX_MAX_PBUFFER_HEIGHT_SGIX 0x8017
#define GLX_MAX_PBUFFER_PIXELS_SGIX 0x8018
#define GLX_OPTIMAL_PBUFFER_WIDTH_SGIX 0x8019
#define GLX_OPTIMAL_PBUFFER_HEIGHT_SGIX 0x801A
#define GLX_PRESERVED_CONTENTS_SGIX 0x801B
#define GLX_LARGEST_PBUFFER_SGIX 0x801C
#define GLX_WIDTH_SGIX 0x801D
#define GLX_HEIGHT_SGIX 0x801E
#define GLX_EVENT_MASK_SGIX 0x801F
#define GLX_DAMAGED_SGIX 0x8020
#define GLX_SAVED_SGIX 0x8021
#define GLX_WINDOW_SGIX 0x8022
#define GLX_PBUFFER_SGIX 0x8023
#define GLX_BUFFER_CLOBBER_MASK_SGIX 0x08000000
#define glXCreateGLXPbufferSGIX XGL_FUNCPTR(glXCreateGLXPbufferSGIX)
#define glXDestroyGLXPbufferSGIX XGL_FUNCPTR(glXDestroyGLXPbufferSGIX)
#define glXGetSelectedEventSGIX XGL_FUNCPTR(glXGetSelectedEventSGIX)
#define glXQueryGLXPbufferSGIX XGL_FUNCPTR(glXQueryGLXPbufferSGIX)
#define glXSelectEventSGIX XGL_FUNCPTR(glXSelectEventSGIX)
#endif

#ifdef GLX_SGIX_swap_barrier
#define glXBindSwapBarrierSGIX XGL_FUNCPTR(glXBindSwapBarrierSGIX)
#define glXQueryMaxSwapBarriersSGIX XGL_FUNCPTR(glXQueryMaxSwapBarriersSGIX)
#endif

#ifdef GLX_SGIX_swap_group
#define glXJoinSwapGroupSGIX XGL_FUNCPTR(glXJoinSwapGroupSGIX)
#endif

#ifdef GLX_SGIX_video_resize
#define GLX_SYNC_FRAME_SGIX 0x00000000
#define GLX_SYNC_SWAP_SGIX 0x00000001
#define glXBindChannelToWindowSGIX XGL_FUNCPTR(glXBindChannelToWindowSGIX)
#define glXChannelRectSGIX XGL_FUNCPTR(glXChannelRectSGIX)
#define glXChannelRectSyncSGIX XGL_FUNCPTR(glXChannelRectSyncSGIX)
#define glXQueryChannelDeltasSGIX XGL_FUNCPTR(glXQueryChannelDeltasSGIX)
#define glXQueryChannelRectSGIX XGL_FUNCPTR(glXQueryChannelRectSGIX)
#endif

#ifdef GLX_SGIX_visual_select_group
#define GLX_VISUAL_SELECT_GROUP_SGIX 0x8028
#endif

#ifdef GLX_SGI_cushion
#define glXCushionSGI XGL_FUNCPTR(glXCushionSGI)
#endif

#ifdef GLX_SGI_make_current_read
#define glXGetCurrentReadDrawableSGI XGL_FUNCPTR(glXGetCurrentReadDrawableSGI)
#define glXMakeCurrentReadSGI XGL_FUNCPTR(glXMakeCurrentReadSGI)
#endif

#ifdef GLX_SGI_swap_control
#define glXSwapIntervalSGI XGL_FUNCPTR(glXSwapIntervalSGI)
#endif

#ifdef GLX_SGI_video_sync
#define glXGetVideoSyncSGI XGL_FUNCPTR(glXGetVideoSyncSGI)
#define glXWaitVideoSyncSGI XGL_FUNCPTR(glXWaitVideoSyncSGI)
#endif

#ifdef GLX_SUN_get_transparent_index
#define glXGetTransparentIndexSUN XGL_FUNCPTR(glXGetTransparentIndexSUN)
#endif

#ifdef GLX_SUN_video_resize
#define GL_VIDEO_RESIZE_COMPENSATION_SUN 0x85CD
#define GLX_VIDEO_RESIZE_SUN 0x8171
#define glXVideoResizeSUN XGL_FUNCPTR(glXVideoResizeSUN)
#define glXGetVideoResizeSUN XGL_FUNCPTR(glXGetVideoResizeSUN)
#endif

#ifdef GLX_VERSION_1_1
#define glXQueryExtensionsString XGL_FUNCPTR(glXQueryExtensionsString)
#define glXGetClientString XGL_FUNCPTR(glXGetClientString)
#define glXQueryServerString XGL_FUNCPTR(glXQueryServerString)
#endif

#ifdef GLX_VERSION_1_2
#define glXGetCurrentDisplay XGL_FUNCPTR(glXGetCurrentDisplay)
#endif

#ifdef GLX_VERSION_1_3
typedef XID GLXWindow;
typedef XID GLXPbuffer;
typedef XID GLXFBConfigID;
typedef struct __GLXFBConfigRec *GLXFBConfig;
typedef struct { int event_type; int draw_type; unsigned long serial; Bool send_event; Display *display; GLXDrawable drawable; unsigned int buffer_mask; unsigned int aux_buffer; int x, y; int width, height; int count; } GLXPbufferClobberEvent;
typedef union __GLXEvent { GLXPbufferClobberEvent glxpbufferclobber; long pad[24]; } GLXEvent;
#define GLX_WINDOW_BIT 0x00000001
#define GLX_PIXMAP_BIT 0x00000002
#define GLX_PBUFFER_BIT 0x00000004
#define GLX_RGBA_BIT 0x00000001
#define GLX_COLOR_INDEX_BIT 0x00000002
#define GLX_PBUFFER_CLOBBER_MASK 0x08000000
#define GLX_FRONT_LEFT_BUFFER_BIT 0x00000001
#define GLX_FRONT_RIGHT_BUFFER_BIT 0x00000002
#define GLX_BACK_LEFT_BUFFER_BIT 0x00000004
#define GLX_BACK_RIGHT_BUFFER_BIT 0x00000008
#define GLX_AUX_BUFFERS_BIT 0x00000010
#define GLX_DEPTH_BUFFER_BIT 0x00000020
#define GLX_STENCIL_BUFFER_BIT 0x00000040
#define GLX_ACCUM_BUFFER_BIT 0x00000080
#define GLX_CONFIG_CAVEAT 0x20
#define GLX_X_VISUAL_TYPE 0x22
#define GLX_TRANSPARENT_TYPE 0x23
#define GLX_TRANSPARENT_INDEX_VALUE 0x24
#define GLX_TRANSPARENT_RED_VALUE 0x25
#define GLX_TRANSPARENT_GREEN_VALUE 0x26
#define GLX_TRANSPARENT_BLUE_VALUE 0x27
#define GLX_TRANSPARENT_ALPHA_VALUE 0x28
#define GLX_DONT_CARE 0xFFFFFFFF
#define GLX_NONE 0x8000
#define GLX_SLOW_CONFIG 0x8001
#define GLX_TRUE_COLOR 0x8002
#define GLX_DIRECT_COLOR 0x8003
#define GLX_PSEUDO_COLOR 0x8004
#define GLX_STATIC_COLOR 0x8005
#define GLX_GRAY_SCALE 0x8006
#define GLX_STATIC_GRAY 0x8007
#define GLX_TRANSPARENT_RGB 0x8008
#define GLX_TRANSPARENT_INDEX 0x8009
#define GLX_VISUAL_ID 0x800B
#define GLX_SCREEN 0x800C
#define GLX_NON_CONFORMANT_CONFIG 0x800D
#define GLX_DRAWABLE_TYPE 0x8010
#define GLX_RENDER_TYPE 0x8011
#define GLX_X_RENDERABLE 0x8012
#define GLX_FBCONFIG_ID 0x8013
#define GLX_RGBA_TYPE 0x8014
#define GLX_COLOR_INDEX_TYPE 0x8015
#define GLX_MAX_PBUFFER_WIDTH 0x8016
#define GLX_MAX_PBUFFER_HEIGHT 0x8017
#define GLX_MAX_PBUFFER_PIXELS 0x8018
#define GLX_PRESERVED_CONTENTS 0x801B
#define GLX_LARGEST_PBUFFER 0x801C
#define GLX_WIDTH 0x801D
#define GLX_HEIGHT 0x801E
#define GLX_EVENT_MASK 0x801F
#define GLX_DAMAGED 0x8020
#define GLX_SAVED 0x8021
#define GLX_WINDOW 0x8022
#define GLX_PBUFFER 0x8023
#define GLX_PBUFFER_HEIGHT 0x8040
#define GLX_PBUFFER_WIDTH 0x8041
#define glXChooseFBConfig XGL_FUNCPTR(glXChooseFBConfig)
#define glXGetFBConfigs XGL_FUNCPTR(glXGetFBConfigs)
#define glXGetVisualFromFBConfig XGL_FUNCPTR(glXGetVisualFromFBConfig)
#define glXGetFBConfigAttrib XGL_FUNCPTR(glXGetFBConfigAttrib)
#define glXCreateWindow XGL_FUNCPTR(glXCreateWindow)
#define glXDestroyWindow XGL_FUNCPTR(glXDestroyWindow)
#define glXCreatePixmap XGL_FUNCPTR(glXCreatePixmap)
#define glXDestroyPixmap XGL_FUNCPTR(glXDestroyPixmap)
#define glXCreatePbuffer XGL_FUNCPTR(glXCreatePbuffer)
#define glXDestroyPbuffer XGL_FUNCPTR(glXDestroyPbuffer)
#define glXQueryDrawable XGL_FUNCPTR(glXQueryDrawable)
#define glXCreateNewContext XGL_FUNCPTR(glXCreateNewContext)
#define glXMakeContextCurrent XGL_FUNCPTR(glXMakeContextCurrent)
#define glXGetCurrentReadDrawable XGL_FUNCPTR(glXGetCurrentReadDrawable)
#define glXQueryContext XGL_FUNCPTR(glXQueryContext)
#define glXSelectEvent XGL_FUNCPTR(glXSelectEvent)
#define glXGetSelectedEvent XGL_FUNCPTR(glXGetSelectedEvent)
#endif

#ifdef GLX_VERSION_1_4
#define GLX_SAMPLE_BUFFERS 100000
#define GLX_SAMPLES 100001
#define glXGetProcAddress XGL_FUNCPTR(glXGetProcAddress)
#endif


