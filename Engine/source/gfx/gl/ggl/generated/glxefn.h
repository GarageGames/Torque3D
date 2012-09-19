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
GL_GROUP_BEGIN(GLX_3DFX_multisample)
GL_GROUP_END()
#endif

#ifdef GLX_ARB_fbconfig_float
GL_GROUP_BEGIN(GLX_ARB_fbconfig_float)
GL_GROUP_END()
#endif

#ifdef GLX_ARB_get_proc_address
GL_GROUP_BEGIN(GLX_ARB_get_proc_address)
GL_FUNCTION(glXGetProcAddressARB, GLFunction, (const GLubyte*))
GL_GROUP_END()
#endif

#ifdef GLX_ARB_multisample
GL_GROUP_BEGIN(GLX_ARB_multisample)
GL_GROUP_END()
#endif

#ifdef GLX_ATI_pixel_format_float
GL_GROUP_BEGIN(GLX_ATI_pixel_format_float)
GL_GROUP_END()
#endif

#ifdef GLX_ATI_render_texture
GL_GROUP_BEGIN(GLX_ATI_render_texture)
GL_FUNCTION(glXBindTexImageATI,void,(Display *dpy, GLXPbuffer pbuf, int buffer))
GL_FUNCTION(glXReleaseTexImageATI,void,(Display *dpy, GLXPbuffer pbuf, int buffer))
GL_FUNCTION(glXDrawableAttribATI,void,(Display *dpy, GLXDrawable draw, const int *attrib_list))
GL_GROUP_END()
#endif

#ifdef GLX_EXT_import_context
GL_GROUP_BEGIN(GLX_EXT_import_context)
GL_FUNCTION(glXFreeContextEXT,void,(Display* dpy, GLXContext context))
GL_FUNCTION(glXGetContextIDEXT,GLXContextID,(const GLXContext context))
GL_FUNCTION(glXImportContextEXT,GLXContext,(Display* dpy, GLXContextID contextID))
GL_FUNCTION(glXQueryContextInfoEXT,int,(Display* dpy, GLXContext context, int attribute,int *value))
GL_GROUP_END()
#endif

#ifdef GLX_EXT_scene_marker
GL_GROUP_BEGIN(GLX_EXT_scene_marker)
GL_GROUP_END()
#endif

#ifdef GLX_EXT_visual_info
GL_GROUP_BEGIN(GLX_EXT_visual_info)
GL_GROUP_END()
#endif

#ifdef GLX_EXT_visual_rating
GL_GROUP_BEGIN(GLX_EXT_visual_rating)
GL_GROUP_END()
#endif

#ifdef GLX_MESA_agp_offset
GL_GROUP_BEGIN(GLX_MESA_agp_offset)
GL_FUNCTION(glXGetAGPOffsetMESA,unsigned int,(const void* pointer))
GL_GROUP_END()
#endif

#ifdef GLX_MESA_copy_sub_buffer
GL_GROUP_BEGIN(GLX_MESA_copy_sub_buffer)
GL_FUNCTION(glXCopySubBufferMESA,void,(Display* dpy, GLXDrawable drawable, int x, int y, int width, int height))
GL_GROUP_END()
#endif

#ifdef GLX_MESA_pixmap_colormap
GL_GROUP_BEGIN(GLX_MESA_pixmap_colormap)
GL_FUNCTION(glXCreateGLXPixmapMESA,GLXPixmap,(Display* dpy, XVisualInfo *visual, Pixmap pixmap, Colormap cmap))
GL_GROUP_END()
#endif

#ifdef GLX_MESA_release_buffers
GL_GROUP_BEGIN(GLX_MESA_release_buffers)
GL_FUNCTION(glXReleaseBuffersMESA,Bool,(Display* dpy, GLXDrawable d))
GL_GROUP_END()
#endif

#ifdef GLX_MESA_set_3dfx_mode
GL_GROUP_BEGIN(GLX_MESA_set_3dfx_mode)
GL_FUNCTION(glXSet3DfxModeMESA,GLboolean,(GLint mode))
GL_GROUP_END()
#endif

#ifdef GLX_NV_float_buffer
GL_GROUP_BEGIN(GLX_NV_float_buffer)
GL_GROUP_END()
#endif

#ifdef GLX_NV_vertex_array_range
GL_GROUP_BEGIN(GLX_NV_vertex_array_range)
GL_FUNCTION(glXAllocateMemoryNV,void *,(GLsizei size, GLfloat readFrequency, GLfloat writeFrequency, GLfloat priority))
GL_FUNCTION(glXFreeMemoryNV,void,(void *pointer))
GL_GROUP_END()
#endif

#ifdef GLX_OML_swap_method
GL_GROUP_BEGIN(GLX_OML_swap_method)
GL_GROUP_END()
#endif

#ifdef GLX_OML_sync_control
GL_GROUP_BEGIN(GLX_OML_sync_control)
GL_FUNCTION(glXGetMscRateOML,Bool,(Display* dpy, GLXDrawable drawable, int32_t* numerator, int32_t* denominator))
GL_FUNCTION(glXGetSyncValuesOML,Bool,(Display* dpy, GLXDrawable drawable, int64_t* ust, int64_t* msc, int64_t* sbc))
GL_FUNCTION(glXSwapBuffersMscOML,int64_t,(Display* dpy, GLXDrawable drawable, int64_t target_msc, int64_t divisor, int64_t remainder))
GL_FUNCTION(glXWaitForMscOML,Bool,(Display* dpy, GLXDrawable drawable, int64_t target_msc, int64_t divisor, int64_t remainder, int64_t* ust, int64_t* msc, int64_t* sbc))
GL_FUNCTION(glXWaitForSbcOML,Bool,(Display* dpy, GLXDrawable drawable, int64_t target_sbc, int64_t* ust, int64_t* msc, int64_t* sbc))
GL_GROUP_END()
#endif

#ifdef GLX_SGIS_blended_overlay
GL_GROUP_BEGIN(GLX_SGIS_blended_overlay)
GL_GROUP_END()
#endif

#ifdef GLX_SGIS_color_range
GL_GROUP_BEGIN(GLX_SGIS_color_range)
GL_GROUP_END()
#endif

#ifdef GLX_SGIS_multisample
GL_GROUP_BEGIN(GLX_SGIS_multisample)
GL_GROUP_END()
#endif

#ifdef GLX_SGIS_shared_multisample
GL_GROUP_BEGIN(GLX_SGIS_shared_multisample)
GL_GROUP_END()
#endif

#ifdef GLX_SGIX_fbconfig
GL_GROUP_BEGIN(GLX_SGIX_fbconfig)
GL_FUNCTION(glXChooseFBConfigSGIX,GLXFBConfigSGIX*,(Display *dpy, int screen, const int *attrib_list, int *nelements))
GL_FUNCTION(glXCreateContextWithConfigSGIX,GLXContext,(Display* dpy, GLXFBConfig config, int render_type, GLXContext share_list, Bool direct))
GL_FUNCTION(glXCreateGLXPixmapWithConfigSGIX,GLXPixmap,(Display* dpy, GLXFBConfig config, Pixmap pixmap))
GL_FUNCTION(glXGetFBConfigAttribSGIX,int,(Display* dpy, GLXFBConfigSGIX config, int attribute, int *value))
GL_FUNCTION(glXGetFBConfigFromVisualSGIX,GLXFBConfigSGIX,(Display* dpy, XVisualInfo *vis))
GL_FUNCTION(glXGetVisualFromFBConfigSGIX,XVisualInfo*,(Display *dpy, GLXFBConfig config))
GL_GROUP_END()
#endif

#ifdef GLX_SGIX_pbuffer
GL_GROUP_BEGIN(GLX_SGIX_pbuffer)
GL_FUNCTION(glXCreateGLXPbufferSGIX,GLXPbuffer,(Display* dpy, GLXFBConfig config, unsigned int width, unsigned int height, int *attrib_list))
GL_FUNCTION(glXDestroyGLXPbufferSGIX,void,(Display* dpy, GLXPbuffer pbuf))
GL_FUNCTION(glXGetSelectedEventSGIX,void,(Display* dpy, GLXDrawable drawable, unsigned long *mask))
GL_FUNCTION(glXQueryGLXPbufferSGIX,void,(Display* dpy, GLXPbuffer pbuf, int attribute, unsigned int *value))
GL_FUNCTION(glXSelectEventSGIX,void,(Display* dpy, GLXDrawable drawable, unsigned long mask))
GL_GROUP_END()
#endif

#ifdef GLX_SGIX_swap_barrier
GL_GROUP_BEGIN(GLX_SGIX_swap_barrier)
GL_FUNCTION(glXBindSwapBarrierSGIX,void,(Display *dpy, GLXDrawable drawable, int barrier))
GL_FUNCTION(glXQueryMaxSwapBarriersSGIX,Bool,(Display *dpy, int screen, int *max))
GL_GROUP_END()
#endif

#ifdef GLX_SGIX_swap_group
GL_GROUP_BEGIN(GLX_SGIX_swap_group)
GL_FUNCTION(glXJoinSwapGroupSGIX,void,(Display *dpy, GLXDrawable drawable, GLXDrawable member))
GL_GROUP_END()
#endif

#ifdef GLX_SGIX_video_resize
GL_GROUP_BEGIN(GLX_SGIX_video_resize)
GL_FUNCTION(glXBindChannelToWindowSGIX,int,(Display* display, int screen, int channel, Window window))
GL_FUNCTION(glXChannelRectSGIX,int,(Display* display, int screen, int channel, int x, int y, int w, int h))
GL_FUNCTION(glXChannelRectSyncSGIX,int,(Display* display, int screen, int channel, GLenum synctype))
GL_FUNCTION(glXQueryChannelDeltasSGIX,int,(Display* display, int screen, int channel, int *x, int *y, int *w, int *h))
GL_FUNCTION(glXQueryChannelRectSGIX,int,(Display* display, int screen, int channel, int *dx, int *dy, int *dw, int *dh))
GL_GROUP_END()
#endif

#ifdef GLX_SGIX_visual_select_group
GL_GROUP_BEGIN(GLX_SGIX_visual_select_group)
GL_GROUP_END()
#endif

#ifdef GLX_SGI_cushion
GL_GROUP_BEGIN(GLX_SGI_cushion)
GL_FUNCTION(glXCushionSGI,void,(Display* dpy, Window window, float cushion))
GL_GROUP_END()
#endif

#ifdef GLX_SGI_make_current_read
GL_GROUP_BEGIN(GLX_SGI_make_current_read)
GL_FUNCTION(glXGetCurrentReadDrawableSGI,GLXDrawable,(void))
GL_FUNCTION(glXMakeCurrentReadSGI,Bool,(Display* dpy, GLXDrawable draw, GLXDrawable read, GLXContext ctx))
GL_GROUP_END()
#endif

#ifdef GLX_SGI_swap_control
GL_GROUP_BEGIN(GLX_SGI_swap_control)
GL_FUNCTION(glXSwapIntervalSGI,int,(int interval))
GL_GROUP_END()
#endif

#ifdef GLX_SGI_video_sync
GL_GROUP_BEGIN(GLX_SGI_video_sync)
GL_FUNCTION(glXGetVideoSyncSGI,int,(uint* count))
GL_FUNCTION(glXWaitVideoSyncSGI,int,(int divisor, int remainder, unsigned int* count))
GL_GROUP_END()
#endif

#ifdef GLX_SUN_get_transparent_index
GL_GROUP_BEGIN(GLX_SUN_get_transparent_index)
GL_FUNCTION(glXGetTransparentIndexSUN,Status,(Display* dpy, Window overlay, Window underlay, unsigned long *pTransparentIndex))
GL_GROUP_END()
#endif

#ifdef GLX_SUN_video_resize
GL_GROUP_BEGIN(GLX_SUN_video_resize)
GL_FUNCTION(glXVideoResizeSUN,int,(Display* display, GLXDrawable window, float factor))
GL_FUNCTION(glXGetVideoResizeSUN,int,(Display* display, GLXDrawable window, float* factor))
GL_GROUP_END()
#endif

#ifdef GLX_VERSION_1_1
GL_GROUP_BEGIN(GLX_VERSION_1_1)
GL_FUNCTION(glXQueryExtensionsString, const char*, (Display *dpy, int screen))
GL_FUNCTION(glXGetClientString, const char*, (Display *dpy, int name))
GL_FUNCTION(glXQueryServerString, const char*, (Display *dpy, int screen, int name))
GL_GROUP_END()
#endif

#ifdef GLX_VERSION_1_2
GL_GROUP_BEGIN(GLX_VERSION_1_2)
GL_FUNCTION(glXGetCurrentDisplay, Display*, (void))
GL_GROUP_END()
#endif

#ifdef GLX_VERSION_1_3
GL_GROUP_BEGIN(GLX_VERSION_1_3)
GL_FUNCTION(glXChooseFBConfig,GLXFBConfig*,(Display *dpy, int screen, const int *attrib_list, int *nelements))
GL_FUNCTION(glXGetFBConfigs,GLXFBConfig*,(Display *dpy, int screen, int *nelements))
GL_FUNCTION(glXGetVisualFromFBConfig,XVisualInfo*,(Display *dpy, GLXFBConfig config))
GL_FUNCTION(glXGetFBConfigAttrib,int,(Display *dpy, GLXFBConfig config, int attribute, int *value))
GL_FUNCTION(glXCreateWindow,GLXWindow,(Display *dpy, GLXFBConfig config, Window win, const int *attrib_list))
GL_FUNCTION(glXDestroyWindow,void,(Display *dpy, GLXWindow win))
GL_FUNCTION(glXCreatePixmap,GLXPixmap,(Display *dpy, GLXFBConfig config, Pixmap pixmap, const int *attrib_list))
GL_FUNCTION(glXDestroyPixmap,void,(Display *dpy, GLXPixmap pixmap))
GL_FUNCTION(glXCreatePbuffer,GLXPbuffer,(Display *dpy, GLXFBConfig config, const int *attrib_list))
GL_FUNCTION(glXDestroyPbuffer,void,(Display *dpy, GLXPbuffer pbuf))
GL_FUNCTION(glXQueryDrawable,void,(Display *dpy, GLXDrawable draw, int attribute, unsigned int *value))
GL_FUNCTION(glXCreateNewContext,GLXContext,(Display *dpy, GLXFBConfig config, int render_type, GLXContext share_list, Bool direct))
GL_FUNCTION(glXMakeContextCurrent,Bool,(Display *display, GLXDrawable draw, GLXDrawable read, GLXContext ctx))
GL_FUNCTION(glXGetCurrentReadDrawable,GLXDrawable,(void))
GL_FUNCTION(glXQueryContext,int,(Display *dpy, GLXContext ctx, int attribute, int *value))
GL_FUNCTION(glXSelectEvent,void,(Display *dpy, GLXDrawable draw, unsigned long event_mask))
GL_FUNCTION(glXGetSelectedEvent,void,(Display *dpy, GLXDrawable draw, unsigned long *event_mask))
GL_GROUP_END()
#endif

#ifdef GLX_VERSION_1_4
GL_GROUP_BEGIN(GLX_VERSION_1_4)
GL_FUNCTION(glXGetProcAddress, GLFunction, (const GLubyte *procName))
GL_GROUP_END()
#endif


