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
GL_GROUP_BEGIN(WGL_3DFX_multisample)
GL_GROUP_END()
#endif

#ifdef WGL_ARB_buffer_region
GL_GROUP_BEGIN(WGL_ARB_buffer_region)
GL_FUNCTION(wglCreateBufferRegionARB,HANDLE,(HDC hDC, int iLayerPlane, UINT uType))
GL_FUNCTION(wglDeleteBufferRegionARB,VOID,(HANDLE hRegion))
GL_FUNCTION(wglRestoreBufferRegionARB,BOOL,(HANDLE hRegion, int x, int y, int width, int height, int xSrc, int ySrc))
GL_FUNCTION(wglSaveBufferRegionARB,BOOL,(HANDLE hRegion, int x, int y, int width, int height))
GL_GROUP_END()
#endif

#ifdef WGL_ARB_extensions_string
GL_GROUP_BEGIN(WGL_ARB_extensions_string)
GL_FUNCTION(wglGetExtensionsStringARB,const char*,(HDC hdc))
GL_GROUP_END()
#endif

#ifdef WGL_ARB_make_current_read
GL_GROUP_BEGIN(WGL_ARB_make_current_read)
GL_FUNCTION(wglGetCurrentReadDCARB,HDC,(VOID))
GL_FUNCTION(wglMakeContextCurrentARB,BOOL,(HDC hDrawDC, HDC hReadDC, HGLRC hglrc))
GL_GROUP_END()
#endif

#ifdef WGL_ARB_multisample
GL_GROUP_BEGIN(WGL_ARB_multisample)
GL_GROUP_END()
#endif

#ifdef WGL_ARB_pbuffer
GL_GROUP_BEGIN(WGL_ARB_pbuffer)
GL_FUNCTION(wglCreatePbufferARB,HPBUFFERARB,(HDC hDC, int iPixelFormat, int iWidth, int iHeight, const int* piAttribList))
GL_FUNCTION(wglDestroyPbufferARB,BOOL,(HPBUFFERARB hPbuffer))
GL_FUNCTION(wglGetPbufferDCARB,HDC,(HPBUFFERARB hPbuffer))
GL_FUNCTION(wglQueryPbufferARB,BOOL,(HPBUFFERARB hPbuffer, int iAttribute, int* piValue))
GL_FUNCTION(wglReleasePbufferDCARB,int,(HPBUFFERARB hPbuffer, HDC hDC))
GL_GROUP_END()
#endif

#ifdef WGL_ARB_pixel_format
GL_GROUP_BEGIN(WGL_ARB_pixel_format)
GL_FUNCTION(wglChoosePixelFormatARB,BOOL,(HDC hdc, const int* piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats))
GL_FUNCTION(wglGetPixelFormatAttribfvARB,BOOL,(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int* piAttributes, FLOAT *pfValues))
GL_FUNCTION(wglGetPixelFormatAttribivARB,BOOL,(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int* piAttributes, int *piValues))
GL_GROUP_END()
#endif

#ifdef WGL_ARB_pixel_format_float
GL_GROUP_BEGIN(WGL_ARB_pixel_format_float)
GL_GROUP_END()
#endif

#ifdef WGL_ARB_render_texture
GL_GROUP_BEGIN(WGL_ARB_render_texture)
GL_FUNCTION(wglBindTexImageARB,BOOL,(HPBUFFERARB hPbuffer, int iBuffer))
GL_FUNCTION(wglReleaseTexImageARB,BOOL,(HPBUFFERARB hPbuffer, int iBuffer))
GL_FUNCTION(wglSetPbufferAttribARB,BOOL,(HPBUFFERARB hPbuffer, const int* piAttribList))
GL_GROUP_END()
#endif

#ifdef WGL_ATI_pixel_format_float
GL_GROUP_BEGIN(WGL_ATI_pixel_format_float)
GL_GROUP_END()
#endif

#ifdef WGL_ATI_render_texture_rectangle
GL_GROUP_BEGIN(WGL_ATI_render_texture_rectangle)
GL_GROUP_END()
#endif

#ifdef WGL_EXT_depth_float
GL_GROUP_BEGIN(WGL_EXT_depth_float)
GL_GROUP_END()
#endif

#ifdef WGL_EXT_display_color_table
GL_GROUP_BEGIN(WGL_EXT_display_color_table)
GL_FUNCTION(wglBindDisplayColorTableEXT,GLboolean,(GLushort id))
GL_FUNCTION(wglCreateDisplayColorTableEXT,GLboolean,(GLushort id))
GL_FUNCTION(wglDestroyDisplayColorTableEXT,void,(GLushort id))
GL_FUNCTION(wglLoadDisplayColorTableEXT,GLboolean,(GLushort* table, GLuint length))
GL_GROUP_END()
#endif

#ifdef WGL_EXT_extensions_string
GL_GROUP_BEGIN(WGL_EXT_extensions_string)
GL_FUNCTION(wglGetExtensionsStringEXT,const char*,(void))
GL_GROUP_END()
#endif

#ifdef WGL_EXT_make_current_read
GL_GROUP_BEGIN(WGL_EXT_make_current_read)
GL_FUNCTION(wglGetCurrentReadDCEXT,HDC,(VOID))
GL_FUNCTION(wglMakeContextCurrentEXT,BOOL,(HDC hDrawDC, HDC hReadDC, HGLRC hglrc))
GL_GROUP_END()
#endif

#ifdef WGL_EXT_multisample
GL_GROUP_BEGIN(WGL_EXT_multisample)
GL_GROUP_END()
#endif

#ifdef WGL_EXT_pbuffer
GL_GROUP_BEGIN(WGL_EXT_pbuffer)
GL_FUNCTION(wglCreatePbufferEXT,HPBUFFEREXT,(HDC hDC, int iPixelFormat, int iWidth, int iHeight, const int* piAttribList))
GL_FUNCTION(wglDestroyPbufferEXT,BOOL,(HPBUFFEREXT hPbuffer))
GL_FUNCTION(wglGetPbufferDCEXT,HDC,(HPBUFFEREXT hPbuffer))
GL_FUNCTION(wglQueryPbufferEXT,BOOL,(HPBUFFEREXT hPbuffer, int iAttribute, int* piValue))
GL_FUNCTION(wglReleasePbufferDCEXT,int,(HPBUFFEREXT hPbuffer, HDC hDC))
GL_GROUP_END()
#endif

#ifdef WGL_EXT_pixel_format
GL_GROUP_BEGIN(WGL_EXT_pixel_format)
GL_FUNCTION(wglChoosePixelFormatEXT,BOOL,(HDC hdc, const int* piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats))
GL_FUNCTION(wglGetPixelFormatAttribfvEXT,BOOL,(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, int* piAttributes, FLOAT *pfValues))
GL_FUNCTION(wglGetPixelFormatAttribivEXT,BOOL,(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, int* piAttributes, int *piValues))
GL_GROUP_END()
#endif

#ifdef WGL_EXT_swap_control
GL_GROUP_BEGIN(WGL_EXT_swap_control)
GL_FUNCTION(wglGetSwapIntervalEXT,int,(void))
GL_FUNCTION(wglSwapIntervalEXT,BOOL,(int interval))
GL_GROUP_END()
#endif

#ifdef WGL_I3D_digital_video_control
GL_GROUP_BEGIN(WGL_I3D_digital_video_control)
GL_FUNCTION(wglGetDigitalVideoParametersI3D,BOOL,(HDC hDC, int iAttribute, int* piValue))
GL_FUNCTION(wglSetDigitalVideoParametersI3D,BOOL,(HDC hDC, int iAttribute, const int* piValue))
GL_GROUP_END()
#endif

#ifdef WGL_I3D_gamma
GL_GROUP_BEGIN(WGL_I3D_gamma)
GL_FUNCTION(wglGetGammaTableI3D,BOOL,(HDC hDC, int iEntries, USHORT* puRed, USHORT *puGreen, USHORT *puBlue))
GL_FUNCTION(wglGetGammaTableParametersI3D,BOOL,(HDC hDC, int iAttribute, int* piValue))
GL_FUNCTION(wglSetGammaTableI3D,BOOL,(HDC hDC, int iEntries, const USHORT* puRed, const USHORT *puGreen, const USHORT *puBlue))
GL_FUNCTION(wglSetGammaTableParametersI3D,BOOL,(HDC hDC, int iAttribute, const int* piValue))
GL_GROUP_END()
#endif

#ifdef WGL_I3D_genlock
GL_GROUP_BEGIN(WGL_I3D_genlock)
GL_FUNCTION(wglDisableGenlockI3D,BOOL,(HDC hDC))
GL_FUNCTION(wglEnableGenlockI3D,BOOL,(HDC hDC))
GL_FUNCTION(wglGenlockSampleRateI3D,BOOL,(HDC hDC, UINT uRate))
GL_FUNCTION(wglGenlockSourceDelayI3D,BOOL,(HDC hDC, UINT uDelay))
GL_FUNCTION(wglGenlockSourceEdgeI3D,BOOL,(HDC hDC, UINT uEdge))
GL_FUNCTION(wglGenlockSourceI3D,BOOL,(HDC hDC, UINT uSource))
GL_FUNCTION(wglGetGenlockSampleRateI3D,BOOL,(HDC hDC, UINT* uRate))
GL_FUNCTION(wglGetGenlockSourceDelayI3D,BOOL,(HDC hDC, UINT* uDelay))
GL_FUNCTION(wglGetGenlockSourceEdgeI3D,BOOL,(HDC hDC, UINT* uEdge))
GL_FUNCTION(wglGetGenlockSourceI3D,BOOL,(HDC hDC, UINT* uSource))
GL_FUNCTION(wglIsEnabledGenlockI3D,BOOL,(HDC hDC, BOOL* pFlag))
GL_FUNCTION(wglQueryGenlockMaxSourceDelayI3D,BOOL,(HDC hDC, UINT* uMaxLineDelay, UINT *uMaxPixelDelay))
GL_GROUP_END()
#endif

#ifdef WGL_I3D_image_buffer
GL_GROUP_BEGIN(WGL_I3D_image_buffer)
GL_FUNCTION(wglAssociateImageBufferEventsI3D,BOOL,(HDC hdc, HANDLE* pEvent, LPVOID *pAddress, DWORD *pSize, UINT count))
GL_FUNCTION(wglCreateImageBufferI3D,LPVOID,(HDC hDC, DWORD dwSize, UINT uFlags))
GL_FUNCTION(wglDestroyImageBufferI3D,BOOL,(HDC hDC, LPVOID pAddress))
GL_FUNCTION(wglReleaseImageBufferEventsI3D,BOOL,(HDC hdc, LPVOID* pAddress, UINT count))
GL_GROUP_END()
#endif

#ifdef WGL_I3D_swap_frame_lock
GL_GROUP_BEGIN(WGL_I3D_swap_frame_lock)
GL_FUNCTION(wglDisableFrameLockI3D,BOOL,(VOID))
GL_FUNCTION(wglEnableFrameLockI3D,BOOL,(VOID))
GL_FUNCTION(wglIsEnabledFrameLockI3D,BOOL,(BOOL* pFlag))
GL_FUNCTION(wglQueryFrameLockMasterI3D,BOOL,(BOOL* pFlag))
GL_GROUP_END()
#endif

#ifdef WGL_I3D_swap_frame_usage
GL_GROUP_BEGIN(WGL_I3D_swap_frame_usage)
GL_FUNCTION(wglBeginFrameTrackingI3D,BOOL,(void))
GL_FUNCTION(wglEndFrameTrackingI3D,BOOL,(void))
GL_FUNCTION(wglGetFrameUsageI3D,BOOL,(float* pUsage))
GL_FUNCTION(wglQueryFrameTrackingI3D,BOOL,(DWORD* pFrameCount, DWORD *pMissedFrames, float *pLastMissedUsage))
GL_GROUP_END()
#endif

#ifdef WGL_NV_float_buffer
GL_GROUP_BEGIN(WGL_NV_float_buffer)
GL_GROUP_END()
#endif

#ifdef WGL_NV_render_depth_texture
GL_GROUP_BEGIN(WGL_NV_render_depth_texture)
GL_GROUP_END()
#endif

#ifdef WGL_NV_render_texture_rectangle
GL_GROUP_BEGIN(WGL_NV_render_texture_rectangle)
GL_GROUP_END()
#endif

#ifdef WGL_NV_vertex_array_range
GL_GROUP_BEGIN(WGL_NV_vertex_array_range)
GL_FUNCTION(wglAllocateMemoryNV,void *,(GLsizei size, GLfloat readFrequency, GLfloat writeFrequency, GLfloat priority))
GL_FUNCTION(wglFreeMemoryNV,void,(void *pointer))
GL_GROUP_END()
#endif

#ifdef WGL_OML_sync_control
GL_GROUP_BEGIN(WGL_OML_sync_control)
GL_FUNCTION(wglGetMscRateOML,BOOL,(HDC hdc, INT32* numerator, INT32 *denominator))
GL_FUNCTION(wglGetSyncValuesOML,BOOL,(HDC hdc, INT64* ust, INT64 *msc, INT64 *sbc))
GL_FUNCTION(wglSwapBuffersMscOML,INT64,(HDC hdc, INT64 target_msc, INT64 divisor, INT64 remainder))
GL_FUNCTION(wglSwapLayerBuffersMscOML,INT64,(HDC hdc, INT fuPlanes, INT64 target_msc, INT64 divisor, INT64 remainder))
GL_FUNCTION(wglWaitForMscOML,BOOL,(HDC hdc, INT64 target_msc, INT64 divisor, INT64 remainder, INT64* ust, INT64 *msc, INT64 *sbc))
GL_FUNCTION(wglWaitForSbcOML,BOOL,(HDC hdc, INT64 target_sbc, INT64* ust, INT64 *msc, INT64 *sbc))
GL_GROUP_END()
#endif

