/* GL dispatch code.
 * This is code-generated from the GL API XML files from Khronos.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "dispatch_common.h"
#include "epoxy/wgl.h"

#ifdef __GNUC__
#define EPOXY_NOINLINE __attribute__((noinline))
#define EPOXY_INLINE inline
#elif defined (_MSC_VER)
#define EPOXY_NOINLINE __declspec(noinline)
#define EPOXY_INLINE
#endif
struct dispatch_table {
    PFNWGLALLOCATEMEMORYNVPROC epoxy_wglAllocateMemoryNV;
    PFNWGLASSOCIATEIMAGEBUFFEREVENTSI3DPROC epoxy_wglAssociateImageBufferEventsI3D;
    PFNWGLBEGINFRAMETRACKINGI3DPROC epoxy_wglBeginFrameTrackingI3D;
    PFNWGLBINDDISPLAYCOLORTABLEEXTPROC epoxy_wglBindDisplayColorTableEXT;
    PFNWGLBINDSWAPBARRIERNVPROC epoxy_wglBindSwapBarrierNV;
    PFNWGLBINDTEXIMAGEARBPROC epoxy_wglBindTexImageARB;
    PFNWGLBINDVIDEOCAPTUREDEVICENVPROC epoxy_wglBindVideoCaptureDeviceNV;
    PFNWGLBINDVIDEODEVICENVPROC epoxy_wglBindVideoDeviceNV;
    PFNWGLBINDVIDEOIMAGENVPROC epoxy_wglBindVideoImageNV;
    PFNWGLBLITCONTEXTFRAMEBUFFERAMDPROC epoxy_wglBlitContextFramebufferAMD;
    PFNWGLCHOOSEPIXELFORMATARBPROC epoxy_wglChoosePixelFormatARB;
    PFNWGLCHOOSEPIXELFORMATEXTPROC epoxy_wglChoosePixelFormatEXT;
    PFNWGLCOPYCONTEXTPROC epoxy_wglCopyContext;
    PFNWGLCOPYIMAGESUBDATANVPROC epoxy_wglCopyImageSubDataNV;
    PFNWGLCREATEAFFINITYDCNVPROC epoxy_wglCreateAffinityDCNV;
    PFNWGLCREATEASSOCIATEDCONTEXTAMDPROC epoxy_wglCreateAssociatedContextAMD;
    PFNWGLCREATEASSOCIATEDCONTEXTATTRIBSAMDPROC epoxy_wglCreateAssociatedContextAttribsAMD;
    PFNWGLCREATEBUFFERREGIONARBPROC epoxy_wglCreateBufferRegionARB;
    PFNWGLCREATECONTEXTPROC epoxy_wglCreateContext;
    PFNWGLCREATECONTEXTATTRIBSARBPROC epoxy_wglCreateContextAttribsARB;
    PFNWGLCREATEDISPLAYCOLORTABLEEXTPROC epoxy_wglCreateDisplayColorTableEXT;
    PFNWGLCREATEIMAGEBUFFERI3DPROC epoxy_wglCreateImageBufferI3D;
    PFNWGLCREATELAYERCONTEXTPROC epoxy_wglCreateLayerContext;
    PFNWGLCREATEPBUFFERARBPROC epoxy_wglCreatePbufferARB;
    PFNWGLCREATEPBUFFEREXTPROC epoxy_wglCreatePbufferEXT;
    PFNWGLDXCLOSEDEVICENVPROC epoxy_wglDXCloseDeviceNV;
    PFNWGLDXLOCKOBJECTSNVPROC epoxy_wglDXLockObjectsNV;
    PFNWGLDXOBJECTACCESSNVPROC epoxy_wglDXObjectAccessNV;
    PFNWGLDXOPENDEVICENVPROC epoxy_wglDXOpenDeviceNV;
    PFNWGLDXREGISTEROBJECTNVPROC epoxy_wglDXRegisterObjectNV;
    PFNWGLDXSETRESOURCESHAREHANDLENVPROC epoxy_wglDXSetResourceShareHandleNV;
    PFNWGLDXUNLOCKOBJECTSNVPROC epoxy_wglDXUnlockObjectsNV;
    PFNWGLDXUNREGISTEROBJECTNVPROC epoxy_wglDXUnregisterObjectNV;
    PFNWGLDELAYBEFORESWAPNVPROC epoxy_wglDelayBeforeSwapNV;
    PFNWGLDELETEASSOCIATEDCONTEXTAMDPROC epoxy_wglDeleteAssociatedContextAMD;
    PFNWGLDELETEBUFFERREGIONARBPROC epoxy_wglDeleteBufferRegionARB;
    PFNWGLDELETECONTEXTPROC epoxy_wglDeleteContext;
    PFNWGLDELETEDCNVPROC epoxy_wglDeleteDCNV;
    PFNWGLDESCRIBELAYERPLANEPROC epoxy_wglDescribeLayerPlane;
    PFNWGLDESTROYDISPLAYCOLORTABLEEXTPROC epoxy_wglDestroyDisplayColorTableEXT;
    PFNWGLDESTROYIMAGEBUFFERI3DPROC epoxy_wglDestroyImageBufferI3D;
    PFNWGLDESTROYPBUFFERARBPROC epoxy_wglDestroyPbufferARB;
    PFNWGLDESTROYPBUFFEREXTPROC epoxy_wglDestroyPbufferEXT;
    PFNWGLDISABLEFRAMELOCKI3DPROC epoxy_wglDisableFrameLockI3D;
    PFNWGLDISABLEGENLOCKI3DPROC epoxy_wglDisableGenlockI3D;
    PFNWGLENABLEFRAMELOCKI3DPROC epoxy_wglEnableFrameLockI3D;
    PFNWGLENABLEGENLOCKI3DPROC epoxy_wglEnableGenlockI3D;
    PFNWGLENDFRAMETRACKINGI3DPROC epoxy_wglEndFrameTrackingI3D;
    PFNWGLENUMGPUDEVICESNVPROC epoxy_wglEnumGpuDevicesNV;
    PFNWGLENUMGPUSFROMAFFINITYDCNVPROC epoxy_wglEnumGpusFromAffinityDCNV;
    PFNWGLENUMGPUSNVPROC epoxy_wglEnumGpusNV;
    PFNWGLENUMERATEVIDEOCAPTUREDEVICESNVPROC epoxy_wglEnumerateVideoCaptureDevicesNV;
    PFNWGLENUMERATEVIDEODEVICESNVPROC epoxy_wglEnumerateVideoDevicesNV;
    PFNWGLFREEMEMORYNVPROC epoxy_wglFreeMemoryNV;
    PFNWGLGENLOCKSAMPLERATEI3DPROC epoxy_wglGenlockSampleRateI3D;
    PFNWGLGENLOCKSOURCEDELAYI3DPROC epoxy_wglGenlockSourceDelayI3D;
    PFNWGLGENLOCKSOURCEEDGEI3DPROC epoxy_wglGenlockSourceEdgeI3D;
    PFNWGLGENLOCKSOURCEI3DPROC epoxy_wglGenlockSourceI3D;
    PFNWGLGETCONTEXTGPUIDAMDPROC epoxy_wglGetContextGPUIDAMD;
    PFNWGLGETCURRENTASSOCIATEDCONTEXTAMDPROC epoxy_wglGetCurrentAssociatedContextAMD;
    PFNWGLGETCURRENTCONTEXTPROC epoxy_wglGetCurrentContext;
    PFNWGLGETCURRENTDCPROC epoxy_wglGetCurrentDC;
    PFNWGLGETCURRENTREADDCARBPROC epoxy_wglGetCurrentReadDCARB;
    PFNWGLGETCURRENTREADDCEXTPROC epoxy_wglGetCurrentReadDCEXT;
    PFNWGLGETDEFAULTPROCADDRESSPROC epoxy_wglGetDefaultProcAddress;
    PFNWGLGETDIGITALVIDEOPARAMETERSI3DPROC epoxy_wglGetDigitalVideoParametersI3D;
    PFNWGLGETEXTENSIONSSTRINGARBPROC epoxy_wglGetExtensionsStringARB;
    PFNWGLGETEXTENSIONSSTRINGEXTPROC epoxy_wglGetExtensionsStringEXT;
    PFNWGLGETFRAMEUSAGEI3DPROC epoxy_wglGetFrameUsageI3D;
    PFNWGLGETGPUIDSAMDPROC epoxy_wglGetGPUIDsAMD;
    PFNWGLGETGPUINFOAMDPROC epoxy_wglGetGPUInfoAMD;
    PFNWGLGETGAMMATABLEI3DPROC epoxy_wglGetGammaTableI3D;
    PFNWGLGETGAMMATABLEPARAMETERSI3DPROC epoxy_wglGetGammaTableParametersI3D;
    PFNWGLGETGENLOCKSAMPLERATEI3DPROC epoxy_wglGetGenlockSampleRateI3D;
    PFNWGLGETGENLOCKSOURCEDELAYI3DPROC epoxy_wglGetGenlockSourceDelayI3D;
    PFNWGLGETGENLOCKSOURCEEDGEI3DPROC epoxy_wglGetGenlockSourceEdgeI3D;
    PFNWGLGETGENLOCKSOURCEI3DPROC epoxy_wglGetGenlockSourceI3D;
    PFNWGLGETLAYERPALETTEENTRIESPROC epoxy_wglGetLayerPaletteEntries;
    PFNWGLGETMSCRATEOMLPROC epoxy_wglGetMscRateOML;
    PFNWGLGETPBUFFERDCARBPROC epoxy_wglGetPbufferDCARB;
    PFNWGLGETPBUFFERDCEXTPROC epoxy_wglGetPbufferDCEXT;
    PFNWGLGETPIXELFORMATATTRIBFVARBPROC epoxy_wglGetPixelFormatAttribfvARB;
    PFNWGLGETPIXELFORMATATTRIBFVEXTPROC epoxy_wglGetPixelFormatAttribfvEXT;
    PFNWGLGETPIXELFORMATATTRIBIVARBPROC epoxy_wglGetPixelFormatAttribivARB;
    PFNWGLGETPIXELFORMATATTRIBIVEXTPROC epoxy_wglGetPixelFormatAttribivEXT;
    PFNWGLGETPROCADDRESSPROC epoxy_wglGetProcAddress;
    PFNWGLGETSWAPINTERVALEXTPROC epoxy_wglGetSwapIntervalEXT;
    PFNWGLGETSYNCVALUESOMLPROC epoxy_wglGetSyncValuesOML;
    PFNWGLGETVIDEODEVICENVPROC epoxy_wglGetVideoDeviceNV;
    PFNWGLGETVIDEOINFONVPROC epoxy_wglGetVideoInfoNV;
    PFNWGLISENABLEDFRAMELOCKI3DPROC epoxy_wglIsEnabledFrameLockI3D;
    PFNWGLISENABLEDGENLOCKI3DPROC epoxy_wglIsEnabledGenlockI3D;
    PFNWGLJOINSWAPGROUPNVPROC epoxy_wglJoinSwapGroupNV;
    PFNWGLLOADDISPLAYCOLORTABLEEXTPROC epoxy_wglLoadDisplayColorTableEXT;
    PFNWGLLOCKVIDEOCAPTUREDEVICENVPROC epoxy_wglLockVideoCaptureDeviceNV;
    PFNWGLMAKEASSOCIATEDCONTEXTCURRENTAMDPROC epoxy_wglMakeAssociatedContextCurrentAMD_unwrapped;
    PFNWGLMAKECONTEXTCURRENTARBPROC epoxy_wglMakeContextCurrentARB_unwrapped;
    PFNWGLMAKECONTEXTCURRENTEXTPROC epoxy_wglMakeContextCurrentEXT_unwrapped;
    PFNWGLMAKECURRENTPROC epoxy_wglMakeCurrent_unwrapped;
    PFNWGLQUERYCURRENTCONTEXTNVPROC epoxy_wglQueryCurrentContextNV;
    PFNWGLQUERYFRAMECOUNTNVPROC epoxy_wglQueryFrameCountNV;
    PFNWGLQUERYFRAMELOCKMASTERI3DPROC epoxy_wglQueryFrameLockMasterI3D;
    PFNWGLQUERYFRAMETRACKINGI3DPROC epoxy_wglQueryFrameTrackingI3D;
    PFNWGLQUERYGENLOCKMAXSOURCEDELAYI3DPROC epoxy_wglQueryGenlockMaxSourceDelayI3D;
    PFNWGLQUERYMAXSWAPGROUPSNVPROC epoxy_wglQueryMaxSwapGroupsNV;
    PFNWGLQUERYPBUFFERARBPROC epoxy_wglQueryPbufferARB;
    PFNWGLQUERYPBUFFEREXTPROC epoxy_wglQueryPbufferEXT;
    PFNWGLQUERYSWAPGROUPNVPROC epoxy_wglQuerySwapGroupNV;
    PFNWGLQUERYVIDEOCAPTUREDEVICENVPROC epoxy_wglQueryVideoCaptureDeviceNV;
    PFNWGLREALIZELAYERPALETTEPROC epoxy_wglRealizeLayerPalette;
    PFNWGLRELEASEIMAGEBUFFEREVENTSI3DPROC epoxy_wglReleaseImageBufferEventsI3D;
    PFNWGLRELEASEPBUFFERDCARBPROC epoxy_wglReleasePbufferDCARB;
    PFNWGLRELEASEPBUFFERDCEXTPROC epoxy_wglReleasePbufferDCEXT;
    PFNWGLRELEASETEXIMAGEARBPROC epoxy_wglReleaseTexImageARB;
    PFNWGLRELEASEVIDEOCAPTUREDEVICENVPROC epoxy_wglReleaseVideoCaptureDeviceNV;
    PFNWGLRELEASEVIDEODEVICENVPROC epoxy_wglReleaseVideoDeviceNV;
    PFNWGLRELEASEVIDEOIMAGENVPROC epoxy_wglReleaseVideoImageNV;
    PFNWGLRESETFRAMECOUNTNVPROC epoxy_wglResetFrameCountNV;
    PFNWGLRESTOREBUFFERREGIONARBPROC epoxy_wglRestoreBufferRegionARB;
    PFNWGLSAVEBUFFERREGIONARBPROC epoxy_wglSaveBufferRegionARB;
    PFNWGLSENDPBUFFERTOVIDEONVPROC epoxy_wglSendPbufferToVideoNV;
    PFNWGLSETDIGITALVIDEOPARAMETERSI3DPROC epoxy_wglSetDigitalVideoParametersI3D;
    PFNWGLSETGAMMATABLEI3DPROC epoxy_wglSetGammaTableI3D;
    PFNWGLSETGAMMATABLEPARAMETERSI3DPROC epoxy_wglSetGammaTableParametersI3D;
    PFNWGLSETLAYERPALETTEENTRIESPROC epoxy_wglSetLayerPaletteEntries;
    PFNWGLSETPBUFFERATTRIBARBPROC epoxy_wglSetPbufferAttribARB;
    PFNWGLSETSTEREOEMITTERSTATE3DLPROC epoxy_wglSetStereoEmitterState3DL;
    PFNWGLSHARELISTSPROC epoxy_wglShareLists;
    PFNWGLSWAPBUFFERSMSCOMLPROC epoxy_wglSwapBuffersMscOML;
    PFNWGLSWAPINTERVALEXTPROC epoxy_wglSwapIntervalEXT;
    PFNWGLSWAPLAYERBUFFERSPROC epoxy_wglSwapLayerBuffers;
    PFNWGLSWAPLAYERBUFFERSMSCOMLPROC epoxy_wglSwapLayerBuffersMscOML;
    PFNWGLUSEFONTBITMAPSAPROC epoxy_wglUseFontBitmapsA;
    PFNWGLUSEFONTBITMAPSWPROC epoxy_wglUseFontBitmapsW;
    PFNWGLUSEFONTOUTLINESPROC epoxy_wglUseFontOutlines;
    PFNWGLUSEFONTOUTLINESAPROC epoxy_wglUseFontOutlinesA;
    PFNWGLUSEFONTOUTLINESWPROC epoxy_wglUseFontOutlinesW;
    PFNWGLWAITFORMSCOMLPROC epoxy_wglWaitForMscOML;
    PFNWGLWAITFORSBCOMLPROC epoxy_wglWaitForSbcOML;
};

#if USING_DISPATCH_TABLE
static EPOXY_INLINE struct dispatch_table *
get_dispatch_table(void);

#endif
enum wgl_provider {
    wgl_provider_terminator = 0,
    WGL_10,
    WGL_extension_WGL_3DL_stereo_control,
    WGL_extension_WGL_AMD_gpu_association,
    WGL_extension_WGL_ARB_buffer_region,
    WGL_extension_WGL_ARB_create_context,
    WGL_extension_WGL_ARB_extensions_string,
    WGL_extension_WGL_ARB_make_current_read,
    WGL_extension_WGL_ARB_pbuffer,
    WGL_extension_WGL_ARB_pixel_format,
    WGL_extension_WGL_ARB_render_texture,
    WGL_extension_WGL_EXT_display_color_table,
    WGL_extension_WGL_EXT_extensions_string,
    WGL_extension_WGL_EXT_make_current_read,
    WGL_extension_WGL_EXT_pbuffer,
    WGL_extension_WGL_EXT_pixel_format,
    WGL_extension_WGL_EXT_swap_control,
    WGL_extension_WGL_I3D_digital_video_control,
    WGL_extension_WGL_I3D_gamma,
    WGL_extension_WGL_I3D_genlock,
    WGL_extension_WGL_I3D_image_buffer,
    WGL_extension_WGL_I3D_swap_frame_lock,
    WGL_extension_WGL_I3D_swap_frame_usage,
    WGL_extension_WGL_NV_DX_interop,
    WGL_extension_WGL_NV_copy_image,
    WGL_extension_WGL_NV_delay_before_swap,
    WGL_extension_WGL_NV_gpu_affinity,
    WGL_extension_WGL_NV_present_video,
    WGL_extension_WGL_NV_swap_group,
    WGL_extension_WGL_NV_vertex_array_range,
    WGL_extension_WGL_NV_video_capture,
    WGL_extension_WGL_NV_video_output,
    WGL_extension_WGL_OML_sync_control,
} PACKED;

static const char *enum_string =
    "WGL 10\0"
    "WGL extension \"WGL_3DL_stereo_control\"\0"
    "WGL extension \"WGL_AMD_gpu_association\"\0"
    "WGL extension \"WGL_ARB_buffer_region\"\0"
    "WGL extension \"WGL_ARB_create_context\"\0"
    "WGL extension \"WGL_ARB_extensions_string\"\0"
    "WGL extension \"WGL_ARB_make_current_read\"\0"
    "WGL extension \"WGL_ARB_pbuffer\"\0"
    "WGL extension \"WGL_ARB_pixel_format\"\0"
    "WGL extension \"WGL_ARB_render_texture\"\0"
    "WGL extension \"WGL_EXT_display_color_table\"\0"
    "WGL extension \"WGL_EXT_extensions_string\"\0"
    "WGL extension \"WGL_EXT_make_current_read\"\0"
    "WGL extension \"WGL_EXT_pbuffer\"\0"
    "WGL extension \"WGL_EXT_pixel_format\"\0"
    "WGL extension \"WGL_EXT_swap_control\"\0"
    "WGL extension \"WGL_I3D_digital_video_control\"\0"
    "WGL extension \"WGL_I3D_gamma\"\0"
    "WGL extension \"WGL_I3D_genlock\"\0"
    "WGL extension \"WGL_I3D_image_buffer\"\0"
    "WGL extension \"WGL_I3D_swap_frame_lock\"\0"
    "WGL extension \"WGL_I3D_swap_frame_usage\"\0"
    "WGL extension \"WGL_NV_DX_interop\"\0"
    "WGL extension \"WGL_NV_copy_image\"\0"
    "WGL extension \"WGL_NV_delay_before_swap\"\0"
    "WGL extension \"WGL_NV_gpu_affinity\"\0"
    "WGL extension \"WGL_NV_present_video\"\0"
    "WGL extension \"WGL_NV_swap_group\"\0"
    "WGL extension \"WGL_NV_vertex_array_range\"\0"
    "WGL extension \"WGL_NV_video_capture\"\0"
    "WGL extension \"WGL_NV_video_output\"\0"
    "WGL extension \"WGL_OML_sync_control\"\0"
     ;

static const uint16_t enum_string_offsets[] = {
    -1, /* wgl_provider_terminator, unused */
    0, /* WGL_10 */
    7, /* WGL_extension_WGL_3DL_stereo_control */
    46, /* WGL_extension_WGL_AMD_gpu_association */
    86, /* WGL_extension_WGL_ARB_buffer_region */
    124, /* WGL_extension_WGL_ARB_create_context */
    163, /* WGL_extension_WGL_ARB_extensions_string */
    205, /* WGL_extension_WGL_ARB_make_current_read */
    247, /* WGL_extension_WGL_ARB_pbuffer */
    279, /* WGL_extension_WGL_ARB_pixel_format */
    316, /* WGL_extension_WGL_ARB_render_texture */
    355, /* WGL_extension_WGL_EXT_display_color_table */
    399, /* WGL_extension_WGL_EXT_extensions_string */
    441, /* WGL_extension_WGL_EXT_make_current_read */
    483, /* WGL_extension_WGL_EXT_pbuffer */
    515, /* WGL_extension_WGL_EXT_pixel_format */
    552, /* WGL_extension_WGL_EXT_swap_control */
    589, /* WGL_extension_WGL_I3D_digital_video_control */
    635, /* WGL_extension_WGL_I3D_gamma */
    665, /* WGL_extension_WGL_I3D_genlock */
    697, /* WGL_extension_WGL_I3D_image_buffer */
    734, /* WGL_extension_WGL_I3D_swap_frame_lock */
    774, /* WGL_extension_WGL_I3D_swap_frame_usage */
    815, /* WGL_extension_WGL_NV_DX_interop */
    849, /* WGL_extension_WGL_NV_copy_image */
    883, /* WGL_extension_WGL_NV_delay_before_swap */
    924, /* WGL_extension_WGL_NV_gpu_affinity */
    960, /* WGL_extension_WGL_NV_present_video */
    997, /* WGL_extension_WGL_NV_swap_group */
    1031, /* WGL_extension_WGL_NV_vertex_array_range */
    1073, /* WGL_extension_WGL_NV_video_capture */
    1110, /* WGL_extension_WGL_NV_video_output */
    1146, /* WGL_extension_WGL_OML_sync_control */
};

static const char entrypoint_strings[] = {
   'w',
   'g',
   'l',
   'A',
   'l',
   'l',
   'o',
   'c',
   'a',
   't',
   'e',
   'M',
   'e',
   'm',
   'o',
   'r',
   'y',
   'N',
   'V',
   0, // wglAllocateMemoryNV
   'w',
   'g',
   'l',
   'A',
   's',
   's',
   'o',
   'c',
   'i',
   'a',
   't',
   'e',
   'I',
   'm',
   'a',
   'g',
   'e',
   'B',
   'u',
   'f',
   'f',
   'e',
   'r',
   'E',
   'v',
   'e',
   'n',
   't',
   's',
   'I',
   '3',
   'D',
   0, // wglAssociateImageBufferEventsI3D
   'w',
   'g',
   'l',
   'B',
   'e',
   'g',
   'i',
   'n',
   'F',
   'r',
   'a',
   'm',
   'e',
   'T',
   'r',
   'a',
   'c',
   'k',
   'i',
   'n',
   'g',
   'I',
   '3',
   'D',
   0, // wglBeginFrameTrackingI3D
   'w',
   'g',
   'l',
   'B',
   'i',
   'n',
   'd',
   'D',
   'i',
   's',
   'p',
   'l',
   'a',
   'y',
   'C',
   'o',
   'l',
   'o',
   'r',
   'T',
   'a',
   'b',
   'l',
   'e',
   'E',
   'X',
   'T',
   0, // wglBindDisplayColorTableEXT
   'w',
   'g',
   'l',
   'B',
   'i',
   'n',
   'd',
   'S',
   'w',
   'a',
   'p',
   'B',
   'a',
   'r',
   'r',
   'i',
   'e',
   'r',
   'N',
   'V',
   0, // wglBindSwapBarrierNV
   'w',
   'g',
   'l',
   'B',
   'i',
   'n',
   'd',
   'T',
   'e',
   'x',
   'I',
   'm',
   'a',
   'g',
   'e',
   'A',
   'R',
   'B',
   0, // wglBindTexImageARB
   'w',
   'g',
   'l',
   'B',
   'i',
   'n',
   'd',
   'V',
   'i',
   'd',
   'e',
   'o',
   'C',
   'a',
   'p',
   't',
   'u',
   'r',
   'e',
   'D',
   'e',
   'v',
   'i',
   'c',
   'e',
   'N',
   'V',
   0, // wglBindVideoCaptureDeviceNV
   'w',
   'g',
   'l',
   'B',
   'i',
   'n',
   'd',
   'V',
   'i',
   'd',
   'e',
   'o',
   'D',
   'e',
   'v',
   'i',
   'c',
   'e',
   'N',
   'V',
   0, // wglBindVideoDeviceNV
   'w',
   'g',
   'l',
   'B',
   'i',
   'n',
   'd',
   'V',
   'i',
   'd',
   'e',
   'o',
   'I',
   'm',
   'a',
   'g',
   'e',
   'N',
   'V',
   0, // wglBindVideoImageNV
   'w',
   'g',
   'l',
   'B',
   'l',
   'i',
   't',
   'C',
   'o',
   'n',
   't',
   'e',
   'x',
   't',
   'F',
   'r',
   'a',
   'm',
   'e',
   'b',
   'u',
   'f',
   'f',
   'e',
   'r',
   'A',
   'M',
   'D',
   0, // wglBlitContextFramebufferAMD
   'w',
   'g',
   'l',
   'C',
   'h',
   'o',
   'o',
   's',
   'e',
   'P',
   'i',
   'x',
   'e',
   'l',
   'F',
   'o',
   'r',
   'm',
   'a',
   't',
   'A',
   'R',
   'B',
   0, // wglChoosePixelFormatARB
   'w',
   'g',
   'l',
   'C',
   'h',
   'o',
   'o',
   's',
   'e',
   'P',
   'i',
   'x',
   'e',
   'l',
   'F',
   'o',
   'r',
   'm',
   'a',
   't',
   'E',
   'X',
   'T',
   0, // wglChoosePixelFormatEXT
   'w',
   'g',
   'l',
   'C',
   'o',
   'p',
   'y',
   'C',
   'o',
   'n',
   't',
   'e',
   'x',
   't',
   0, // wglCopyContext
   'w',
   'g',
   'l',
   'C',
   'o',
   'p',
   'y',
   'I',
   'm',
   'a',
   'g',
   'e',
   'S',
   'u',
   'b',
   'D',
   'a',
   't',
   'a',
   'N',
   'V',
   0, // wglCopyImageSubDataNV
   'w',
   'g',
   'l',
   'C',
   'r',
   'e',
   'a',
   't',
   'e',
   'A',
   'f',
   'f',
   'i',
   'n',
   'i',
   't',
   'y',
   'D',
   'C',
   'N',
   'V',
   0, // wglCreateAffinityDCNV
   'w',
   'g',
   'l',
   'C',
   'r',
   'e',
   'a',
   't',
   'e',
   'A',
   's',
   's',
   'o',
   'c',
   'i',
   'a',
   't',
   'e',
   'd',
   'C',
   'o',
   'n',
   't',
   'e',
   'x',
   't',
   'A',
   'M',
   'D',
   0, // wglCreateAssociatedContextAMD
   'w',
   'g',
   'l',
   'C',
   'r',
   'e',
   'a',
   't',
   'e',
   'A',
   's',
   's',
   'o',
   'c',
   'i',
   'a',
   't',
   'e',
   'd',
   'C',
   'o',
   'n',
   't',
   'e',
   'x',
   't',
   'A',
   't',
   't',
   'r',
   'i',
   'b',
   's',
   'A',
   'M',
   'D',
   0, // wglCreateAssociatedContextAttribsAMD
   'w',
   'g',
   'l',
   'C',
   'r',
   'e',
   'a',
   't',
   'e',
   'B',
   'u',
   'f',
   'f',
   'e',
   'r',
   'R',
   'e',
   'g',
   'i',
   'o',
   'n',
   'A',
   'R',
   'B',
   0, // wglCreateBufferRegionARB
   'w',
   'g',
   'l',
   'C',
   'r',
   'e',
   'a',
   't',
   'e',
   'C',
   'o',
   'n',
   't',
   'e',
   'x',
   't',
   0, // wglCreateContext
   'w',
   'g',
   'l',
   'C',
   'r',
   'e',
   'a',
   't',
   'e',
   'C',
   'o',
   'n',
   't',
   'e',
   'x',
   't',
   'A',
   't',
   't',
   'r',
   'i',
   'b',
   's',
   'A',
   'R',
   'B',
   0, // wglCreateContextAttribsARB
   'w',
   'g',
   'l',
   'C',
   'r',
   'e',
   'a',
   't',
   'e',
   'D',
   'i',
   's',
   'p',
   'l',
   'a',
   'y',
   'C',
   'o',
   'l',
   'o',
   'r',
   'T',
   'a',
   'b',
   'l',
   'e',
   'E',
   'X',
   'T',
   0, // wglCreateDisplayColorTableEXT
   'w',
   'g',
   'l',
   'C',
   'r',
   'e',
   'a',
   't',
   'e',
   'I',
   'm',
   'a',
   'g',
   'e',
   'B',
   'u',
   'f',
   'f',
   'e',
   'r',
   'I',
   '3',
   'D',
   0, // wglCreateImageBufferI3D
   'w',
   'g',
   'l',
   'C',
   'r',
   'e',
   'a',
   't',
   'e',
   'L',
   'a',
   'y',
   'e',
   'r',
   'C',
   'o',
   'n',
   't',
   'e',
   'x',
   't',
   0, // wglCreateLayerContext
   'w',
   'g',
   'l',
   'C',
   'r',
   'e',
   'a',
   't',
   'e',
   'P',
   'b',
   'u',
   'f',
   'f',
   'e',
   'r',
   'A',
   'R',
   'B',
   0, // wglCreatePbufferARB
   'w',
   'g',
   'l',
   'C',
   'r',
   'e',
   'a',
   't',
   'e',
   'P',
   'b',
   'u',
   'f',
   'f',
   'e',
   'r',
   'E',
   'X',
   'T',
   0, // wglCreatePbufferEXT
   'w',
   'g',
   'l',
   'D',
   'X',
   'C',
   'l',
   'o',
   's',
   'e',
   'D',
   'e',
   'v',
   'i',
   'c',
   'e',
   'N',
   'V',
   0, // wglDXCloseDeviceNV
   'w',
   'g',
   'l',
   'D',
   'X',
   'L',
   'o',
   'c',
   'k',
   'O',
   'b',
   'j',
   'e',
   'c',
   't',
   's',
   'N',
   'V',
   0, // wglDXLockObjectsNV
   'w',
   'g',
   'l',
   'D',
   'X',
   'O',
   'b',
   'j',
   'e',
   'c',
   't',
   'A',
   'c',
   'c',
   'e',
   's',
   's',
   'N',
   'V',
   0, // wglDXObjectAccessNV
   'w',
   'g',
   'l',
   'D',
   'X',
   'O',
   'p',
   'e',
   'n',
   'D',
   'e',
   'v',
   'i',
   'c',
   'e',
   'N',
   'V',
   0, // wglDXOpenDeviceNV
   'w',
   'g',
   'l',
   'D',
   'X',
   'R',
   'e',
   'g',
   'i',
   's',
   't',
   'e',
   'r',
   'O',
   'b',
   'j',
   'e',
   'c',
   't',
   'N',
   'V',
   0, // wglDXRegisterObjectNV
   'w',
   'g',
   'l',
   'D',
   'X',
   'S',
   'e',
   't',
   'R',
   'e',
   's',
   'o',
   'u',
   'r',
   'c',
   'e',
   'S',
   'h',
   'a',
   'r',
   'e',
   'H',
   'a',
   'n',
   'd',
   'l',
   'e',
   'N',
   'V',
   0, // wglDXSetResourceShareHandleNV
   'w',
   'g',
   'l',
   'D',
   'X',
   'U',
   'n',
   'l',
   'o',
   'c',
   'k',
   'O',
   'b',
   'j',
   'e',
   'c',
   't',
   's',
   'N',
   'V',
   0, // wglDXUnlockObjectsNV
   'w',
   'g',
   'l',
   'D',
   'X',
   'U',
   'n',
   'r',
   'e',
   'g',
   'i',
   's',
   't',
   'e',
   'r',
   'O',
   'b',
   'j',
   'e',
   'c',
   't',
   'N',
   'V',
   0, // wglDXUnregisterObjectNV
   'w',
   'g',
   'l',
   'D',
   'e',
   'l',
   'a',
   'y',
   'B',
   'e',
   'f',
   'o',
   'r',
   'e',
   'S',
   'w',
   'a',
   'p',
   'N',
   'V',
   0, // wglDelayBeforeSwapNV
   'w',
   'g',
   'l',
   'D',
   'e',
   'l',
   'e',
   't',
   'e',
   'A',
   's',
   's',
   'o',
   'c',
   'i',
   'a',
   't',
   'e',
   'd',
   'C',
   'o',
   'n',
   't',
   'e',
   'x',
   't',
   'A',
   'M',
   'D',
   0, // wglDeleteAssociatedContextAMD
   'w',
   'g',
   'l',
   'D',
   'e',
   'l',
   'e',
   't',
   'e',
   'B',
   'u',
   'f',
   'f',
   'e',
   'r',
   'R',
   'e',
   'g',
   'i',
   'o',
   'n',
   'A',
   'R',
   'B',
   0, // wglDeleteBufferRegionARB
   'w',
   'g',
   'l',
   'D',
   'e',
   'l',
   'e',
   't',
   'e',
   'C',
   'o',
   'n',
   't',
   'e',
   'x',
   't',
   0, // wglDeleteContext
   'w',
   'g',
   'l',
   'D',
   'e',
   'l',
   'e',
   't',
   'e',
   'D',
   'C',
   'N',
   'V',
   0, // wglDeleteDCNV
   'w',
   'g',
   'l',
   'D',
   'e',
   's',
   'c',
   'r',
   'i',
   'b',
   'e',
   'L',
   'a',
   'y',
   'e',
   'r',
   'P',
   'l',
   'a',
   'n',
   'e',
   0, // wglDescribeLayerPlane
   'w',
   'g',
   'l',
   'D',
   'e',
   's',
   't',
   'r',
   'o',
   'y',
   'D',
   'i',
   's',
   'p',
   'l',
   'a',
   'y',
   'C',
   'o',
   'l',
   'o',
   'r',
   'T',
   'a',
   'b',
   'l',
   'e',
   'E',
   'X',
   'T',
   0, // wglDestroyDisplayColorTableEXT
   'w',
   'g',
   'l',
   'D',
   'e',
   's',
   't',
   'r',
   'o',
   'y',
   'I',
   'm',
   'a',
   'g',
   'e',
   'B',
   'u',
   'f',
   'f',
   'e',
   'r',
   'I',
   '3',
   'D',
   0, // wglDestroyImageBufferI3D
   'w',
   'g',
   'l',
   'D',
   'e',
   's',
   't',
   'r',
   'o',
   'y',
   'P',
   'b',
   'u',
   'f',
   'f',
   'e',
   'r',
   'A',
   'R',
   'B',
   0, // wglDestroyPbufferARB
   'w',
   'g',
   'l',
   'D',
   'e',
   's',
   't',
   'r',
   'o',
   'y',
   'P',
   'b',
   'u',
   'f',
   'f',
   'e',
   'r',
   'E',
   'X',
   'T',
   0, // wglDestroyPbufferEXT
   'w',
   'g',
   'l',
   'D',
   'i',
   's',
   'a',
   'b',
   'l',
   'e',
   'F',
   'r',
   'a',
   'm',
   'e',
   'L',
   'o',
   'c',
   'k',
   'I',
   '3',
   'D',
   0, // wglDisableFrameLockI3D
   'w',
   'g',
   'l',
   'D',
   'i',
   's',
   'a',
   'b',
   'l',
   'e',
   'G',
   'e',
   'n',
   'l',
   'o',
   'c',
   'k',
   'I',
   '3',
   'D',
   0, // wglDisableGenlockI3D
   'w',
   'g',
   'l',
   'E',
   'n',
   'a',
   'b',
   'l',
   'e',
   'F',
   'r',
   'a',
   'm',
   'e',
   'L',
   'o',
   'c',
   'k',
   'I',
   '3',
   'D',
   0, // wglEnableFrameLockI3D
   'w',
   'g',
   'l',
   'E',
   'n',
   'a',
   'b',
   'l',
   'e',
   'G',
   'e',
   'n',
   'l',
   'o',
   'c',
   'k',
   'I',
   '3',
   'D',
   0, // wglEnableGenlockI3D
   'w',
   'g',
   'l',
   'E',
   'n',
   'd',
   'F',
   'r',
   'a',
   'm',
   'e',
   'T',
   'r',
   'a',
   'c',
   'k',
   'i',
   'n',
   'g',
   'I',
   '3',
   'D',
   0, // wglEndFrameTrackingI3D
   'w',
   'g',
   'l',
   'E',
   'n',
   'u',
   'm',
   'G',
   'p',
   'u',
   'D',
   'e',
   'v',
   'i',
   'c',
   'e',
   's',
   'N',
   'V',
   0, // wglEnumGpuDevicesNV
   'w',
   'g',
   'l',
   'E',
   'n',
   'u',
   'm',
   'G',
   'p',
   'u',
   's',
   'F',
   'r',
   'o',
   'm',
   'A',
   'f',
   'f',
   'i',
   'n',
   'i',
   't',
   'y',
   'D',
   'C',
   'N',
   'V',
   0, // wglEnumGpusFromAffinityDCNV
   'w',
   'g',
   'l',
   'E',
   'n',
   'u',
   'm',
   'G',
   'p',
   'u',
   's',
   'N',
   'V',
   0, // wglEnumGpusNV
   'w',
   'g',
   'l',
   'E',
   'n',
   'u',
   'm',
   'e',
   'r',
   'a',
   't',
   'e',
   'V',
   'i',
   'd',
   'e',
   'o',
   'C',
   'a',
   'p',
   't',
   'u',
   'r',
   'e',
   'D',
   'e',
   'v',
   'i',
   'c',
   'e',
   's',
   'N',
   'V',
   0, // wglEnumerateVideoCaptureDevicesNV
   'w',
   'g',
   'l',
   'E',
   'n',
   'u',
   'm',
   'e',
   'r',
   'a',
   't',
   'e',
   'V',
   'i',
   'd',
   'e',
   'o',
   'D',
   'e',
   'v',
   'i',
   'c',
   'e',
   's',
   'N',
   'V',
   0, // wglEnumerateVideoDevicesNV
   'w',
   'g',
   'l',
   'F',
   'r',
   'e',
   'e',
   'M',
   'e',
   'm',
   'o',
   'r',
   'y',
   'N',
   'V',
   0, // wglFreeMemoryNV
   'w',
   'g',
   'l',
   'G',
   'e',
   'n',
   'l',
   'o',
   'c',
   'k',
   'S',
   'a',
   'm',
   'p',
   'l',
   'e',
   'R',
   'a',
   't',
   'e',
   'I',
   '3',
   'D',
   0, // wglGenlockSampleRateI3D
   'w',
   'g',
   'l',
   'G',
   'e',
   'n',
   'l',
   'o',
   'c',
   'k',
   'S',
   'o',
   'u',
   'r',
   'c',
   'e',
   'D',
   'e',
   'l',
   'a',
   'y',
   'I',
   '3',
   'D',
   0, // wglGenlockSourceDelayI3D
   'w',
   'g',
   'l',
   'G',
   'e',
   'n',
   'l',
   'o',
   'c',
   'k',
   'S',
   'o',
   'u',
   'r',
   'c',
   'e',
   'E',
   'd',
   'g',
   'e',
   'I',
   '3',
   'D',
   0, // wglGenlockSourceEdgeI3D
   'w',
   'g',
   'l',
   'G',
   'e',
   'n',
   'l',
   'o',
   'c',
   'k',
   'S',
   'o',
   'u',
   'r',
   'c',
   'e',
   'I',
   '3',
   'D',
   0, // wglGenlockSourceI3D
   'w',
   'g',
   'l',
   'G',
   'e',
   't',
   'C',
   'o',
   'n',
   't',
   'e',
   'x',
   't',
   'G',
   'P',
   'U',
   'I',
   'D',
   'A',
   'M',
   'D',
   0, // wglGetContextGPUIDAMD
   'w',
   'g',
   'l',
   'G',
   'e',
   't',
   'C',
   'u',
   'r',
   'r',
   'e',
   'n',
   't',
   'A',
   's',
   's',
   'o',
   'c',
   'i',
   'a',
   't',
   'e',
   'd',
   'C',
   'o',
   'n',
   't',
   'e',
   'x',
   't',
   'A',
   'M',
   'D',
   0, // wglGetCurrentAssociatedContextAMD
   'w',
   'g',
   'l',
   'G',
   'e',
   't',
   'C',
   'u',
   'r',
   'r',
   'e',
   'n',
   't',
   'C',
   'o',
   'n',
   't',
   'e',
   'x',
   't',
   0, // wglGetCurrentContext
   'w',
   'g',
   'l',
   'G',
   'e',
   't',
   'C',
   'u',
   'r',
   'r',
   'e',
   'n',
   't',
   'D',
   'C',
   0, // wglGetCurrentDC
   'w',
   'g',
   'l',
   'G',
   'e',
   't',
   'C',
   'u',
   'r',
   'r',
   'e',
   'n',
   't',
   'R',
   'e',
   'a',
   'd',
   'D',
   'C',
   'A',
   'R',
   'B',
   0, // wglGetCurrentReadDCARB
   'w',
   'g',
   'l',
   'G',
   'e',
   't',
   'C',
   'u',
   'r',
   'r',
   'e',
   'n',
   't',
   'R',
   'e',
   'a',
   'd',
   'D',
   'C',
   'E',
   'X',
   'T',
   0, // wglGetCurrentReadDCEXT
   'w',
   'g',
   'l',
   'G',
   'e',
   't',
   'D',
   'e',
   'f',
   'a',
   'u',
   'l',
   't',
   'P',
   'r',
   'o',
   'c',
   'A',
   'd',
   'd',
   'r',
   'e',
   's',
   's',
   0, // wglGetDefaultProcAddress
   'w',
   'g',
   'l',
   'G',
   'e',
   't',
   'D',
   'i',
   'g',
   'i',
   't',
   'a',
   'l',
   'V',
   'i',
   'd',
   'e',
   'o',
   'P',
   'a',
   'r',
   'a',
   'm',
   'e',
   't',
   'e',
   'r',
   's',
   'I',
   '3',
   'D',
   0, // wglGetDigitalVideoParametersI3D
   'w',
   'g',
   'l',
   'G',
   'e',
   't',
   'E',
   'x',
   't',
   'e',
   'n',
   's',
   'i',
   'o',
   'n',
   's',
   'S',
   't',
   'r',
   'i',
   'n',
   'g',
   'A',
   'R',
   'B',
   0, // wglGetExtensionsStringARB
   'w',
   'g',
   'l',
   'G',
   'e',
   't',
   'E',
   'x',
   't',
   'e',
   'n',
   's',
   'i',
   'o',
   'n',
   's',
   'S',
   't',
   'r',
   'i',
   'n',
   'g',
   'E',
   'X',
   'T',
   0, // wglGetExtensionsStringEXT
   'w',
   'g',
   'l',
   'G',
   'e',
   't',
   'F',
   'r',
   'a',
   'm',
   'e',
   'U',
   's',
   'a',
   'g',
   'e',
   'I',
   '3',
   'D',
   0, // wglGetFrameUsageI3D
   'w',
   'g',
   'l',
   'G',
   'e',
   't',
   'G',
   'P',
   'U',
   'I',
   'D',
   's',
   'A',
   'M',
   'D',
   0, // wglGetGPUIDsAMD
   'w',
   'g',
   'l',
   'G',
   'e',
   't',
   'G',
   'P',
   'U',
   'I',
   'n',
   'f',
   'o',
   'A',
   'M',
   'D',
   0, // wglGetGPUInfoAMD
   'w',
   'g',
   'l',
   'G',
   'e',
   't',
   'G',
   'a',
   'm',
   'm',
   'a',
   'T',
   'a',
   'b',
   'l',
   'e',
   'I',
   '3',
   'D',
   0, // wglGetGammaTableI3D
   'w',
   'g',
   'l',
   'G',
   'e',
   't',
   'G',
   'a',
   'm',
   'm',
   'a',
   'T',
   'a',
   'b',
   'l',
   'e',
   'P',
   'a',
   'r',
   'a',
   'm',
   'e',
   't',
   'e',
   'r',
   's',
   'I',
   '3',
   'D',
   0, // wglGetGammaTableParametersI3D
   'w',
   'g',
   'l',
   'G',
   'e',
   't',
   'G',
   'e',
   'n',
   'l',
   'o',
   'c',
   'k',
   'S',
   'a',
   'm',
   'p',
   'l',
   'e',
   'R',
   'a',
   't',
   'e',
   'I',
   '3',
   'D',
   0, // wglGetGenlockSampleRateI3D
   'w',
   'g',
   'l',
   'G',
   'e',
   't',
   'G',
   'e',
   'n',
   'l',
   'o',
   'c',
   'k',
   'S',
   'o',
   'u',
   'r',
   'c',
   'e',
   'D',
   'e',
   'l',
   'a',
   'y',
   'I',
   '3',
   'D',
   0, // wglGetGenlockSourceDelayI3D
   'w',
   'g',
   'l',
   'G',
   'e',
   't',
   'G',
   'e',
   'n',
   'l',
   'o',
   'c',
   'k',
   'S',
   'o',
   'u',
   'r',
   'c',
   'e',
   'E',
   'd',
   'g',
   'e',
   'I',
   '3',
   'D',
   0, // wglGetGenlockSourceEdgeI3D
   'w',
   'g',
   'l',
   'G',
   'e',
   't',
   'G',
   'e',
   'n',
   'l',
   'o',
   'c',
   'k',
   'S',
   'o',
   'u',
   'r',
   'c',
   'e',
   'I',
   '3',
   'D',
   0, // wglGetGenlockSourceI3D
   'w',
   'g',
   'l',
   'G',
   'e',
   't',
   'L',
   'a',
   'y',
   'e',
   'r',
   'P',
   'a',
   'l',
   'e',
   't',
   't',
   'e',
   'E',
   'n',
   't',
   'r',
   'i',
   'e',
   's',
   0, // wglGetLayerPaletteEntries
   'w',
   'g',
   'l',
   'G',
   'e',
   't',
   'M',
   's',
   'c',
   'R',
   'a',
   't',
   'e',
   'O',
   'M',
   'L',
   0, // wglGetMscRateOML
   'w',
   'g',
   'l',
   'G',
   'e',
   't',
   'P',
   'b',
   'u',
   'f',
   'f',
   'e',
   'r',
   'D',
   'C',
   'A',
   'R',
   'B',
   0, // wglGetPbufferDCARB
   'w',
   'g',
   'l',
   'G',
   'e',
   't',
   'P',
   'b',
   'u',
   'f',
   'f',
   'e',
   'r',
   'D',
   'C',
   'E',
   'X',
   'T',
   0, // wglGetPbufferDCEXT
   'w',
   'g',
   'l',
   'G',
   'e',
   't',
   'P',
   'i',
   'x',
   'e',
   'l',
   'F',
   'o',
   'r',
   'm',
   'a',
   't',
   'A',
   't',
   't',
   'r',
   'i',
   'b',
   'f',
   'v',
   'A',
   'R',
   'B',
   0, // wglGetPixelFormatAttribfvARB
   'w',
   'g',
   'l',
   'G',
   'e',
   't',
   'P',
   'i',
   'x',
   'e',
   'l',
   'F',
   'o',
   'r',
   'm',
   'a',
   't',
   'A',
   't',
   't',
   'r',
   'i',
   'b',
   'f',
   'v',
   'E',
   'X',
   'T',
   0, // wglGetPixelFormatAttribfvEXT
   'w',
   'g',
   'l',
   'G',
   'e',
   't',
   'P',
   'i',
   'x',
   'e',
   'l',
   'F',
   'o',
   'r',
   'm',
   'a',
   't',
   'A',
   't',
   't',
   'r',
   'i',
   'b',
   'i',
   'v',
   'A',
   'R',
   'B',
   0, // wglGetPixelFormatAttribivARB
   'w',
   'g',
   'l',
   'G',
   'e',
   't',
   'P',
   'i',
   'x',
   'e',
   'l',
   'F',
   'o',
   'r',
   'm',
   'a',
   't',
   'A',
   't',
   't',
   'r',
   'i',
   'b',
   'i',
   'v',
   'E',
   'X',
   'T',
   0, // wglGetPixelFormatAttribivEXT
   'w',
   'g',
   'l',
   'G',
   'e',
   't',
   'P',
   'r',
   'o',
   'c',
   'A',
   'd',
   'd',
   'r',
   'e',
   's',
   's',
   0, // wglGetProcAddress
   'w',
   'g',
   'l',
   'G',
   'e',
   't',
   'S',
   'w',
   'a',
   'p',
   'I',
   'n',
   't',
   'e',
   'r',
   'v',
   'a',
   'l',
   'E',
   'X',
   'T',
   0, // wglGetSwapIntervalEXT
   'w',
   'g',
   'l',
   'G',
   'e',
   't',
   'S',
   'y',
   'n',
   'c',
   'V',
   'a',
   'l',
   'u',
   'e',
   's',
   'O',
   'M',
   'L',
   0, // wglGetSyncValuesOML
   'w',
   'g',
   'l',
   'G',
   'e',
   't',
   'V',
   'i',
   'd',
   'e',
   'o',
   'D',
   'e',
   'v',
   'i',
   'c',
   'e',
   'N',
   'V',
   0, // wglGetVideoDeviceNV
   'w',
   'g',
   'l',
   'G',
   'e',
   't',
   'V',
   'i',
   'd',
   'e',
   'o',
   'I',
   'n',
   'f',
   'o',
   'N',
   'V',
   0, // wglGetVideoInfoNV
   'w',
   'g',
   'l',
   'I',
   's',
   'E',
   'n',
   'a',
   'b',
   'l',
   'e',
   'd',
   'F',
   'r',
   'a',
   'm',
   'e',
   'L',
   'o',
   'c',
   'k',
   'I',
   '3',
   'D',
   0, // wglIsEnabledFrameLockI3D
   'w',
   'g',
   'l',
   'I',
   's',
   'E',
   'n',
   'a',
   'b',
   'l',
   'e',
   'd',
   'G',
   'e',
   'n',
   'l',
   'o',
   'c',
   'k',
   'I',
   '3',
   'D',
   0, // wglIsEnabledGenlockI3D
   'w',
   'g',
   'l',
   'J',
   'o',
   'i',
   'n',
   'S',
   'w',
   'a',
   'p',
   'G',
   'r',
   'o',
   'u',
   'p',
   'N',
   'V',
   0, // wglJoinSwapGroupNV
   'w',
   'g',
   'l',
   'L',
   'o',
   'a',
   'd',
   'D',
   'i',
   's',
   'p',
   'l',
   'a',
   'y',
   'C',
   'o',
   'l',
   'o',
   'r',
   'T',
   'a',
   'b',
   'l',
   'e',
   'E',
   'X',
   'T',
   0, // wglLoadDisplayColorTableEXT
   'w',
   'g',
   'l',
   'L',
   'o',
   'c',
   'k',
   'V',
   'i',
   'd',
   'e',
   'o',
   'C',
   'a',
   'p',
   't',
   'u',
   'r',
   'e',
   'D',
   'e',
   'v',
   'i',
   'c',
   'e',
   'N',
   'V',
   0, // wglLockVideoCaptureDeviceNV
   'w',
   'g',
   'l',
   'M',
   'a',
   'k',
   'e',
   'A',
   's',
   's',
   'o',
   'c',
   'i',
   'a',
   't',
   'e',
   'd',
   'C',
   'o',
   'n',
   't',
   'e',
   'x',
   't',
   'C',
   'u',
   'r',
   'r',
   'e',
   'n',
   't',
   'A',
   'M',
   'D',
   0, // wglMakeAssociatedContextCurrentAMD
   'w',
   'g',
   'l',
   'M',
   'a',
   'k',
   'e',
   'C',
   'o',
   'n',
   't',
   'e',
   'x',
   't',
   'C',
   'u',
   'r',
   'r',
   'e',
   'n',
   't',
   'A',
   'R',
   'B',
   0, // wglMakeContextCurrentARB
   'w',
   'g',
   'l',
   'M',
   'a',
   'k',
   'e',
   'C',
   'o',
   'n',
   't',
   'e',
   'x',
   't',
   'C',
   'u',
   'r',
   'r',
   'e',
   'n',
   't',
   'E',
   'X',
   'T',
   0, // wglMakeContextCurrentEXT
   'w',
   'g',
   'l',
   'M',
   'a',
   'k',
   'e',
   'C',
   'u',
   'r',
   'r',
   'e',
   'n',
   't',
   0, // wglMakeCurrent
   'w',
   'g',
   'l',
   'Q',
   'u',
   'e',
   'r',
   'y',
   'C',
   'u',
   'r',
   'r',
   'e',
   'n',
   't',
   'C',
   'o',
   'n',
   't',
   'e',
   'x',
   't',
   'N',
   'V',
   0, // wglQueryCurrentContextNV
   'w',
   'g',
   'l',
   'Q',
   'u',
   'e',
   'r',
   'y',
   'F',
   'r',
   'a',
   'm',
   'e',
   'C',
   'o',
   'u',
   'n',
   't',
   'N',
   'V',
   0, // wglQueryFrameCountNV
   'w',
   'g',
   'l',
   'Q',
   'u',
   'e',
   'r',
   'y',
   'F',
   'r',
   'a',
   'm',
   'e',
   'L',
   'o',
   'c',
   'k',
   'M',
   'a',
   's',
   't',
   'e',
   'r',
   'I',
   '3',
   'D',
   0, // wglQueryFrameLockMasterI3D
   'w',
   'g',
   'l',
   'Q',
   'u',
   'e',
   'r',
   'y',
   'F',
   'r',
   'a',
   'm',
   'e',
   'T',
   'r',
   'a',
   'c',
   'k',
   'i',
   'n',
   'g',
   'I',
   '3',
   'D',
   0, // wglQueryFrameTrackingI3D
   'w',
   'g',
   'l',
   'Q',
   'u',
   'e',
   'r',
   'y',
   'G',
   'e',
   'n',
   'l',
   'o',
   'c',
   'k',
   'M',
   'a',
   'x',
   'S',
   'o',
   'u',
   'r',
   'c',
   'e',
   'D',
   'e',
   'l',
   'a',
   'y',
   'I',
   '3',
   'D',
   0, // wglQueryGenlockMaxSourceDelayI3D
   'w',
   'g',
   'l',
   'Q',
   'u',
   'e',
   'r',
   'y',
   'M',
   'a',
   'x',
   'S',
   'w',
   'a',
   'p',
   'G',
   'r',
   'o',
   'u',
   'p',
   's',
   'N',
   'V',
   0, // wglQueryMaxSwapGroupsNV
   'w',
   'g',
   'l',
   'Q',
   'u',
   'e',
   'r',
   'y',
   'P',
   'b',
   'u',
   'f',
   'f',
   'e',
   'r',
   'A',
   'R',
   'B',
   0, // wglQueryPbufferARB
   'w',
   'g',
   'l',
   'Q',
   'u',
   'e',
   'r',
   'y',
   'P',
   'b',
   'u',
   'f',
   'f',
   'e',
   'r',
   'E',
   'X',
   'T',
   0, // wglQueryPbufferEXT
   'w',
   'g',
   'l',
   'Q',
   'u',
   'e',
   'r',
   'y',
   'S',
   'w',
   'a',
   'p',
   'G',
   'r',
   'o',
   'u',
   'p',
   'N',
   'V',
   0, // wglQuerySwapGroupNV
   'w',
   'g',
   'l',
   'Q',
   'u',
   'e',
   'r',
   'y',
   'V',
   'i',
   'd',
   'e',
   'o',
   'C',
   'a',
   'p',
   't',
   'u',
   'r',
   'e',
   'D',
   'e',
   'v',
   'i',
   'c',
   'e',
   'N',
   'V',
   0, // wglQueryVideoCaptureDeviceNV
   'w',
   'g',
   'l',
   'R',
   'e',
   'a',
   'l',
   'i',
   'z',
   'e',
   'L',
   'a',
   'y',
   'e',
   'r',
   'P',
   'a',
   'l',
   'e',
   't',
   't',
   'e',
   0, // wglRealizeLayerPalette
   'w',
   'g',
   'l',
   'R',
   'e',
   'l',
   'e',
   'a',
   's',
   'e',
   'I',
   'm',
   'a',
   'g',
   'e',
   'B',
   'u',
   'f',
   'f',
   'e',
   'r',
   'E',
   'v',
   'e',
   'n',
   't',
   's',
   'I',
   '3',
   'D',
   0, // wglReleaseImageBufferEventsI3D
   'w',
   'g',
   'l',
   'R',
   'e',
   'l',
   'e',
   'a',
   's',
   'e',
   'P',
   'b',
   'u',
   'f',
   'f',
   'e',
   'r',
   'D',
   'C',
   'A',
   'R',
   'B',
   0, // wglReleasePbufferDCARB
   'w',
   'g',
   'l',
   'R',
   'e',
   'l',
   'e',
   'a',
   's',
   'e',
   'P',
   'b',
   'u',
   'f',
   'f',
   'e',
   'r',
   'D',
   'C',
   'E',
   'X',
   'T',
   0, // wglReleasePbufferDCEXT
   'w',
   'g',
   'l',
   'R',
   'e',
   'l',
   'e',
   'a',
   's',
   'e',
   'T',
   'e',
   'x',
   'I',
   'm',
   'a',
   'g',
   'e',
   'A',
   'R',
   'B',
   0, // wglReleaseTexImageARB
   'w',
   'g',
   'l',
   'R',
   'e',
   'l',
   'e',
   'a',
   's',
   'e',
   'V',
   'i',
   'd',
   'e',
   'o',
   'C',
   'a',
   'p',
   't',
   'u',
   'r',
   'e',
   'D',
   'e',
   'v',
   'i',
   'c',
   'e',
   'N',
   'V',
   0, // wglReleaseVideoCaptureDeviceNV
   'w',
   'g',
   'l',
   'R',
   'e',
   'l',
   'e',
   'a',
   's',
   'e',
   'V',
   'i',
   'd',
   'e',
   'o',
   'D',
   'e',
   'v',
   'i',
   'c',
   'e',
   'N',
   'V',
   0, // wglReleaseVideoDeviceNV
   'w',
   'g',
   'l',
   'R',
   'e',
   'l',
   'e',
   'a',
   's',
   'e',
   'V',
   'i',
   'd',
   'e',
   'o',
   'I',
   'm',
   'a',
   'g',
   'e',
   'N',
   'V',
   0, // wglReleaseVideoImageNV
   'w',
   'g',
   'l',
   'R',
   'e',
   's',
   'e',
   't',
   'F',
   'r',
   'a',
   'm',
   'e',
   'C',
   'o',
   'u',
   'n',
   't',
   'N',
   'V',
   0, // wglResetFrameCountNV
   'w',
   'g',
   'l',
   'R',
   'e',
   's',
   't',
   'o',
   'r',
   'e',
   'B',
   'u',
   'f',
   'f',
   'e',
   'r',
   'R',
   'e',
   'g',
   'i',
   'o',
   'n',
   'A',
   'R',
   'B',
   0, // wglRestoreBufferRegionARB
   'w',
   'g',
   'l',
   'S',
   'a',
   'v',
   'e',
   'B',
   'u',
   'f',
   'f',
   'e',
   'r',
   'R',
   'e',
   'g',
   'i',
   'o',
   'n',
   'A',
   'R',
   'B',
   0, // wglSaveBufferRegionARB
   'w',
   'g',
   'l',
   'S',
   'e',
   'n',
   'd',
   'P',
   'b',
   'u',
   'f',
   'f',
   'e',
   'r',
   'T',
   'o',
   'V',
   'i',
   'd',
   'e',
   'o',
   'N',
   'V',
   0, // wglSendPbufferToVideoNV
   'w',
   'g',
   'l',
   'S',
   'e',
   't',
   'D',
   'i',
   'g',
   'i',
   't',
   'a',
   'l',
   'V',
   'i',
   'd',
   'e',
   'o',
   'P',
   'a',
   'r',
   'a',
   'm',
   'e',
   't',
   'e',
   'r',
   's',
   'I',
   '3',
   'D',
   0, // wglSetDigitalVideoParametersI3D
   'w',
   'g',
   'l',
   'S',
   'e',
   't',
   'G',
   'a',
   'm',
   'm',
   'a',
   'T',
   'a',
   'b',
   'l',
   'e',
   'I',
   '3',
   'D',
   0, // wglSetGammaTableI3D
   'w',
   'g',
   'l',
   'S',
   'e',
   't',
   'G',
   'a',
   'm',
   'm',
   'a',
   'T',
   'a',
   'b',
   'l',
   'e',
   'P',
   'a',
   'r',
   'a',
   'm',
   'e',
   't',
   'e',
   'r',
   's',
   'I',
   '3',
   'D',
   0, // wglSetGammaTableParametersI3D
   'w',
   'g',
   'l',
   'S',
   'e',
   't',
   'L',
   'a',
   'y',
   'e',
   'r',
   'P',
   'a',
   'l',
   'e',
   't',
   't',
   'e',
   'E',
   'n',
   't',
   'r',
   'i',
   'e',
   's',
   0, // wglSetLayerPaletteEntries
   'w',
   'g',
   'l',
   'S',
   'e',
   't',
   'P',
   'b',
   'u',
   'f',
   'f',
   'e',
   'r',
   'A',
   't',
   't',
   'r',
   'i',
   'b',
   'A',
   'R',
   'B',
   0, // wglSetPbufferAttribARB
   'w',
   'g',
   'l',
   'S',
   'e',
   't',
   'S',
   't',
   'e',
   'r',
   'e',
   'o',
   'E',
   'm',
   'i',
   't',
   't',
   'e',
   'r',
   'S',
   't',
   'a',
   't',
   'e',
   '3',
   'D',
   'L',
   0, // wglSetStereoEmitterState3DL
   'w',
   'g',
   'l',
   'S',
   'h',
   'a',
   'r',
   'e',
   'L',
   'i',
   's',
   't',
   's',
   0, // wglShareLists
   'w',
   'g',
   'l',
   'S',
   'w',
   'a',
   'p',
   'B',
   'u',
   'f',
   'f',
   'e',
   'r',
   's',
   'M',
   's',
   'c',
   'O',
   'M',
   'L',
   0, // wglSwapBuffersMscOML
   'w',
   'g',
   'l',
   'S',
   'w',
   'a',
   'p',
   'I',
   'n',
   't',
   'e',
   'r',
   'v',
   'a',
   'l',
   'E',
   'X',
   'T',
   0, // wglSwapIntervalEXT
   'w',
   'g',
   'l',
   'S',
   'w',
   'a',
   'p',
   'L',
   'a',
   'y',
   'e',
   'r',
   'B',
   'u',
   'f',
   'f',
   'e',
   'r',
   's',
   0, // wglSwapLayerBuffers
   'w',
   'g',
   'l',
   'S',
   'w',
   'a',
   'p',
   'L',
   'a',
   'y',
   'e',
   'r',
   'B',
   'u',
   'f',
   'f',
   'e',
   'r',
   's',
   'M',
   's',
   'c',
   'O',
   'M',
   'L',
   0, // wglSwapLayerBuffersMscOML
   'w',
   'g',
   'l',
   'U',
   's',
   'e',
   'F',
   'o',
   'n',
   't',
   'B',
   'i',
   't',
   'm',
   'a',
   'p',
   's',
   'A',
   0, // wglUseFontBitmapsA
   'w',
   'g',
   'l',
   'U',
   's',
   'e',
   'F',
   'o',
   'n',
   't',
   'B',
   'i',
   't',
   'm',
   'a',
   'p',
   's',
   'W',
   0, // wglUseFontBitmapsW
   'w',
   'g',
   'l',
   'U',
   's',
   'e',
   'F',
   'o',
   'n',
   't',
   'O',
   'u',
   't',
   'l',
   'i',
   'n',
   'e',
   's',
   0, // wglUseFontOutlines
   'w',
   'g',
   'l',
   'U',
   's',
   'e',
   'F',
   'o',
   'n',
   't',
   'O',
   'u',
   't',
   'l',
   'i',
   'n',
   'e',
   's',
   'A',
   0, // wglUseFontOutlinesA
   'w',
   'g',
   'l',
   'U',
   's',
   'e',
   'F',
   'o',
   'n',
   't',
   'O',
   'u',
   't',
   'l',
   'i',
   'n',
   'e',
   's',
   'W',
   0, // wglUseFontOutlinesW
   'w',
   'g',
   'l',
   'W',
   'a',
   'i',
   't',
   'F',
   'o',
   'r',
   'M',
   's',
   'c',
   'O',
   'M',
   'L',
   0, // wglWaitForMscOML
   'w',
   'g',
   'l',
   'W',
   'a',
   'i',
   't',
   'F',
   'o',
   'r',
   'S',
   'b',
   'c',
   'O',
   'M',
   'L',
   0, // wglWaitForSbcOML
    0 };

static void *wgl_provider_resolver(const char *name,
                                   const enum wgl_provider *providers,
                                   const uint32_t *entrypoints)
{
    int i;
    for (i = 0; providers[i] != wgl_provider_terminator; i++) {
        switch (providers[i]) {
        case WGL_10:
            if (true)
                return epoxy_gl_dlsym(entrypoint_strings + entrypoints[i]);
            break;
        case WGL_extension_WGL_3DL_stereo_control:
            if (epoxy_conservative_has_wgl_extension("WGL_3DL_stereo_control"))
                return wglGetProcAddress(entrypoint_strings + entrypoints[i]);
            break;
        case WGL_extension_WGL_AMD_gpu_association:
            if (epoxy_conservative_has_wgl_extension("WGL_AMD_gpu_association"))
                return wglGetProcAddress(entrypoint_strings + entrypoints[i]);
            break;
        case WGL_extension_WGL_ARB_buffer_region:
            if (epoxy_conservative_has_wgl_extension("WGL_ARB_buffer_region"))
                return wglGetProcAddress(entrypoint_strings + entrypoints[i]);
            break;
        case WGL_extension_WGL_ARB_create_context:
            if (epoxy_conservative_has_wgl_extension("WGL_ARB_create_context"))
                return wglGetProcAddress(entrypoint_strings + entrypoints[i]);
            break;
        case WGL_extension_WGL_ARB_extensions_string:
            if (epoxy_conservative_has_wgl_extension("WGL_ARB_extensions_string"))
                return wglGetProcAddress(entrypoint_strings + entrypoints[i]);
            break;
        case WGL_extension_WGL_ARB_make_current_read:
            if (epoxy_conservative_has_wgl_extension("WGL_ARB_make_current_read"))
                return wglGetProcAddress(entrypoint_strings + entrypoints[i]);
            break;
        case WGL_extension_WGL_ARB_pbuffer:
            if (epoxy_conservative_has_wgl_extension("WGL_ARB_pbuffer"))
                return wglGetProcAddress(entrypoint_strings + entrypoints[i]);
            break;
        case WGL_extension_WGL_ARB_pixel_format:
            if (epoxy_conservative_has_wgl_extension("WGL_ARB_pixel_format"))
                return wglGetProcAddress(entrypoint_strings + entrypoints[i]);
            break;
        case WGL_extension_WGL_ARB_render_texture:
            if (epoxy_conservative_has_wgl_extension("WGL_ARB_render_texture"))
                return wglGetProcAddress(entrypoint_strings + entrypoints[i]);
            break;
        case WGL_extension_WGL_EXT_display_color_table:
            if (epoxy_conservative_has_wgl_extension("WGL_EXT_display_color_table"))
                return wglGetProcAddress(entrypoint_strings + entrypoints[i]);
            break;
        case WGL_extension_WGL_EXT_extensions_string:
            if (epoxy_conservative_has_wgl_extension("WGL_EXT_extensions_string"))
                return wglGetProcAddress(entrypoint_strings + entrypoints[i]);
            break;
        case WGL_extension_WGL_EXT_make_current_read:
            if (epoxy_conservative_has_wgl_extension("WGL_EXT_make_current_read"))
                return wglGetProcAddress(entrypoint_strings + entrypoints[i]);
            break;
        case WGL_extension_WGL_EXT_pbuffer:
            if (epoxy_conservative_has_wgl_extension("WGL_EXT_pbuffer"))
                return wglGetProcAddress(entrypoint_strings + entrypoints[i]);
            break;
        case WGL_extension_WGL_EXT_pixel_format:
            if (epoxy_conservative_has_wgl_extension("WGL_EXT_pixel_format"))
                return wglGetProcAddress(entrypoint_strings + entrypoints[i]);
            break;
        case WGL_extension_WGL_EXT_swap_control:
            if (epoxy_conservative_has_wgl_extension("WGL_EXT_swap_control"))
                return wglGetProcAddress(entrypoint_strings + entrypoints[i]);
            break;
        case WGL_extension_WGL_I3D_digital_video_control:
            if (epoxy_conservative_has_wgl_extension("WGL_I3D_digital_video_control"))
                return wglGetProcAddress(entrypoint_strings + entrypoints[i]);
            break;
        case WGL_extension_WGL_I3D_gamma:
            if (epoxy_conservative_has_wgl_extension("WGL_I3D_gamma"))
                return wglGetProcAddress(entrypoint_strings + entrypoints[i]);
            break;
        case WGL_extension_WGL_I3D_genlock:
            if (epoxy_conservative_has_wgl_extension("WGL_I3D_genlock"))
                return wglGetProcAddress(entrypoint_strings + entrypoints[i]);
            break;
        case WGL_extension_WGL_I3D_image_buffer:
            if (epoxy_conservative_has_wgl_extension("WGL_I3D_image_buffer"))
                return wglGetProcAddress(entrypoint_strings + entrypoints[i]);
            break;
        case WGL_extension_WGL_I3D_swap_frame_lock:
            if (epoxy_conservative_has_wgl_extension("WGL_I3D_swap_frame_lock"))
                return wglGetProcAddress(entrypoint_strings + entrypoints[i]);
            break;
        case WGL_extension_WGL_I3D_swap_frame_usage:
            if (epoxy_conservative_has_wgl_extension("WGL_I3D_swap_frame_usage"))
                return wglGetProcAddress(entrypoint_strings + entrypoints[i]);
            break;
        case WGL_extension_WGL_NV_DX_interop:
            if (epoxy_conservative_has_wgl_extension("WGL_NV_DX_interop"))
                return wglGetProcAddress(entrypoint_strings + entrypoints[i]);
            break;
        case WGL_extension_WGL_NV_copy_image:
            if (epoxy_conservative_has_wgl_extension("WGL_NV_copy_image"))
                return wglGetProcAddress(entrypoint_strings + entrypoints[i]);
            break;
        case WGL_extension_WGL_NV_delay_before_swap:
            if (epoxy_conservative_has_wgl_extension("WGL_NV_delay_before_swap"))
                return wglGetProcAddress(entrypoint_strings + entrypoints[i]);
            break;
        case WGL_extension_WGL_NV_gpu_affinity:
            if (epoxy_conservative_has_wgl_extension("WGL_NV_gpu_affinity"))
                return wglGetProcAddress(entrypoint_strings + entrypoints[i]);
            break;
        case WGL_extension_WGL_NV_present_video:
            if (epoxy_conservative_has_wgl_extension("WGL_NV_present_video"))
                return wglGetProcAddress(entrypoint_strings + entrypoints[i]);
            break;
        case WGL_extension_WGL_NV_swap_group:
            if (epoxy_conservative_has_wgl_extension("WGL_NV_swap_group"))
                return wglGetProcAddress(entrypoint_strings + entrypoints[i]);
            break;
        case WGL_extension_WGL_NV_vertex_array_range:
            if (epoxy_conservative_has_wgl_extension("WGL_NV_vertex_array_range"))
                return wglGetProcAddress(entrypoint_strings + entrypoints[i]);
            break;
        case WGL_extension_WGL_NV_video_capture:
            if (epoxy_conservative_has_wgl_extension("WGL_NV_video_capture"))
                return wglGetProcAddress(entrypoint_strings + entrypoints[i]);
            break;
        case WGL_extension_WGL_NV_video_output:
            if (epoxy_conservative_has_wgl_extension("WGL_NV_video_output"))
                return wglGetProcAddress(entrypoint_strings + entrypoints[i]);
            break;
        case WGL_extension_WGL_OML_sync_control:
            if (epoxy_conservative_has_wgl_extension("WGL_OML_sync_control"))
                return wglGetProcAddress(entrypoint_strings + entrypoints[i]);
            break;
        case wgl_provider_terminator:
            abort(); /* Not reached */
        }
    }

    fprintf(stderr, "No provider of %s found.  Requires one of:\n", name);
    for (i = 0; providers[i] != wgl_provider_terminator; i++) {
        fprintf(stderr, "    %s\n", enum_string + enum_string_offsets[providers[i]]);
    }
    if (providers[0] == wgl_provider_terminator) {
        fprintf(stderr, "    No known providers.  This is likely a bug "
                        "in libepoxy code generation\n");
    }
    abort();
}

EPOXY_NOINLINE static void *
wgl_single_resolver(const enum wgl_provider provider, const uint32_t entrypoint_offset);

static void *
wgl_single_resolver(const enum wgl_provider provider, const uint32_t entrypoint_offset)
{
    const enum wgl_provider providers[] = {
        provider,
        wgl_provider_terminator
    };
    const uint32_t entrypoints[] = {
        entrypoint_offset
    };
    return wgl_provider_resolver(entrypoint_strings + entrypoint_offset,
                                providers, entrypoints);
}

static PFNWGLALLOCATEMEMORYNVPROC
epoxy_wglAllocateMemoryNV_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_NV_vertex_array_range, 0 /* wglAllocateMemoryNV */);
}

static PFNWGLASSOCIATEIMAGEBUFFEREVENTSI3DPROC
epoxy_wglAssociateImageBufferEventsI3D_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_I3D_image_buffer, 20 /* wglAssociateImageBufferEventsI3D */);
}

static PFNWGLBEGINFRAMETRACKINGI3DPROC
epoxy_wglBeginFrameTrackingI3D_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_I3D_swap_frame_usage, 53 /* wglBeginFrameTrackingI3D */);
}

static PFNWGLBINDDISPLAYCOLORTABLEEXTPROC
epoxy_wglBindDisplayColorTableEXT_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_EXT_display_color_table, 78 /* wglBindDisplayColorTableEXT */);
}

static PFNWGLBINDSWAPBARRIERNVPROC
epoxy_wglBindSwapBarrierNV_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_NV_swap_group, 106 /* wglBindSwapBarrierNV */);
}

static PFNWGLBINDTEXIMAGEARBPROC
epoxy_wglBindTexImageARB_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_ARB_render_texture, 127 /* wglBindTexImageARB */);
}

static PFNWGLBINDVIDEOCAPTUREDEVICENVPROC
epoxy_wglBindVideoCaptureDeviceNV_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_NV_video_capture, 146 /* wglBindVideoCaptureDeviceNV */);
}

static PFNWGLBINDVIDEODEVICENVPROC
epoxy_wglBindVideoDeviceNV_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_NV_present_video, 174 /* wglBindVideoDeviceNV */);
}

static PFNWGLBINDVIDEOIMAGENVPROC
epoxy_wglBindVideoImageNV_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_NV_video_output, 195 /* wglBindVideoImageNV */);
}

static PFNWGLBLITCONTEXTFRAMEBUFFERAMDPROC
epoxy_wglBlitContextFramebufferAMD_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_AMD_gpu_association, 215 /* wglBlitContextFramebufferAMD */);
}

static PFNWGLCHOOSEPIXELFORMATARBPROC
epoxy_wglChoosePixelFormatARB_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_ARB_pixel_format, 244 /* wglChoosePixelFormatARB */);
}

static PFNWGLCHOOSEPIXELFORMATEXTPROC
epoxy_wglChoosePixelFormatEXT_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_EXT_pixel_format, 268 /* wglChoosePixelFormatEXT */);
}

static PFNWGLCOPYCONTEXTPROC
epoxy_wglCopyContext_resolver(void)
{
    return wgl_single_resolver(WGL_10, 292 /* wglCopyContext */);
}

static PFNWGLCOPYIMAGESUBDATANVPROC
epoxy_wglCopyImageSubDataNV_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_NV_copy_image, 307 /* wglCopyImageSubDataNV */);
}

static PFNWGLCREATEAFFINITYDCNVPROC
epoxy_wglCreateAffinityDCNV_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_NV_gpu_affinity, 329 /* wglCreateAffinityDCNV */);
}

static PFNWGLCREATEASSOCIATEDCONTEXTAMDPROC
epoxy_wglCreateAssociatedContextAMD_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_AMD_gpu_association, 351 /* wglCreateAssociatedContextAMD */);
}

static PFNWGLCREATEASSOCIATEDCONTEXTATTRIBSAMDPROC
epoxy_wglCreateAssociatedContextAttribsAMD_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_AMD_gpu_association, 381 /* wglCreateAssociatedContextAttribsAMD */);
}

static PFNWGLCREATEBUFFERREGIONARBPROC
epoxy_wglCreateBufferRegionARB_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_ARB_buffer_region, 418 /* wglCreateBufferRegionARB */);
}

static PFNWGLCREATECONTEXTPROC
epoxy_wglCreateContext_resolver(void)
{
    return wgl_single_resolver(WGL_10, 443 /* wglCreateContext */);
}

static PFNWGLCREATECONTEXTATTRIBSARBPROC
epoxy_wglCreateContextAttribsARB_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_ARB_create_context, 460 /* wglCreateContextAttribsARB */);
}

static PFNWGLCREATEDISPLAYCOLORTABLEEXTPROC
epoxy_wglCreateDisplayColorTableEXT_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_EXT_display_color_table, 487 /* wglCreateDisplayColorTableEXT */);
}

static PFNWGLCREATEIMAGEBUFFERI3DPROC
epoxy_wglCreateImageBufferI3D_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_I3D_image_buffer, 517 /* wglCreateImageBufferI3D */);
}

static PFNWGLCREATELAYERCONTEXTPROC
epoxy_wglCreateLayerContext_resolver(void)
{
    return wgl_single_resolver(WGL_10, 541 /* wglCreateLayerContext */);
}

static PFNWGLCREATEPBUFFERARBPROC
epoxy_wglCreatePbufferARB_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_ARB_pbuffer, 563 /* wglCreatePbufferARB */);
}

static PFNWGLCREATEPBUFFEREXTPROC
epoxy_wglCreatePbufferEXT_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_EXT_pbuffer, 583 /* wglCreatePbufferEXT */);
}

static PFNWGLDXCLOSEDEVICENVPROC
epoxy_wglDXCloseDeviceNV_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_NV_DX_interop, 603 /* wglDXCloseDeviceNV */);
}

static PFNWGLDXLOCKOBJECTSNVPROC
epoxy_wglDXLockObjectsNV_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_NV_DX_interop, 622 /* wglDXLockObjectsNV */);
}

static PFNWGLDXOBJECTACCESSNVPROC
epoxy_wglDXObjectAccessNV_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_NV_DX_interop, 641 /* wglDXObjectAccessNV */);
}

static PFNWGLDXOPENDEVICENVPROC
epoxy_wglDXOpenDeviceNV_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_NV_DX_interop, 661 /* wglDXOpenDeviceNV */);
}

static PFNWGLDXREGISTEROBJECTNVPROC
epoxy_wglDXRegisterObjectNV_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_NV_DX_interop, 679 /* wglDXRegisterObjectNV */);
}

static PFNWGLDXSETRESOURCESHAREHANDLENVPROC
epoxy_wglDXSetResourceShareHandleNV_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_NV_DX_interop, 701 /* wglDXSetResourceShareHandleNV */);
}

static PFNWGLDXUNLOCKOBJECTSNVPROC
epoxy_wglDXUnlockObjectsNV_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_NV_DX_interop, 731 /* wglDXUnlockObjectsNV */);
}

static PFNWGLDXUNREGISTEROBJECTNVPROC
epoxy_wglDXUnregisterObjectNV_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_NV_DX_interop, 752 /* wglDXUnregisterObjectNV */);
}

static PFNWGLDELAYBEFORESWAPNVPROC
epoxy_wglDelayBeforeSwapNV_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_NV_delay_before_swap, 776 /* wglDelayBeforeSwapNV */);
}

static PFNWGLDELETEASSOCIATEDCONTEXTAMDPROC
epoxy_wglDeleteAssociatedContextAMD_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_AMD_gpu_association, 797 /* wglDeleteAssociatedContextAMD */);
}

static PFNWGLDELETEBUFFERREGIONARBPROC
epoxy_wglDeleteBufferRegionARB_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_ARB_buffer_region, 827 /* wglDeleteBufferRegionARB */);
}

static PFNWGLDELETECONTEXTPROC
epoxy_wglDeleteContext_resolver(void)
{
    return wgl_single_resolver(WGL_10, 852 /* wglDeleteContext */);
}

static PFNWGLDELETEDCNVPROC
epoxy_wglDeleteDCNV_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_NV_gpu_affinity, 869 /* wglDeleteDCNV */);
}

static PFNWGLDESCRIBELAYERPLANEPROC
epoxy_wglDescribeLayerPlane_resolver(void)
{
    return wgl_single_resolver(WGL_10, 883 /* wglDescribeLayerPlane */);
}

static PFNWGLDESTROYDISPLAYCOLORTABLEEXTPROC
epoxy_wglDestroyDisplayColorTableEXT_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_EXT_display_color_table, 905 /* wglDestroyDisplayColorTableEXT */);
}

static PFNWGLDESTROYIMAGEBUFFERI3DPROC
epoxy_wglDestroyImageBufferI3D_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_I3D_image_buffer, 936 /* wglDestroyImageBufferI3D */);
}

static PFNWGLDESTROYPBUFFERARBPROC
epoxy_wglDestroyPbufferARB_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_ARB_pbuffer, 961 /* wglDestroyPbufferARB */);
}

static PFNWGLDESTROYPBUFFEREXTPROC
epoxy_wglDestroyPbufferEXT_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_EXT_pbuffer, 982 /* wglDestroyPbufferEXT */);
}

static PFNWGLDISABLEFRAMELOCKI3DPROC
epoxy_wglDisableFrameLockI3D_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_I3D_swap_frame_lock, 1003 /* wglDisableFrameLockI3D */);
}

static PFNWGLDISABLEGENLOCKI3DPROC
epoxy_wglDisableGenlockI3D_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_I3D_genlock, 1026 /* wglDisableGenlockI3D */);
}

static PFNWGLENABLEFRAMELOCKI3DPROC
epoxy_wglEnableFrameLockI3D_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_I3D_swap_frame_lock, 1047 /* wglEnableFrameLockI3D */);
}

static PFNWGLENABLEGENLOCKI3DPROC
epoxy_wglEnableGenlockI3D_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_I3D_genlock, 1069 /* wglEnableGenlockI3D */);
}

static PFNWGLENDFRAMETRACKINGI3DPROC
epoxy_wglEndFrameTrackingI3D_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_I3D_swap_frame_usage, 1089 /* wglEndFrameTrackingI3D */);
}

static PFNWGLENUMGPUDEVICESNVPROC
epoxy_wglEnumGpuDevicesNV_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_NV_gpu_affinity, 1112 /* wglEnumGpuDevicesNV */);
}

static PFNWGLENUMGPUSFROMAFFINITYDCNVPROC
epoxy_wglEnumGpusFromAffinityDCNV_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_NV_gpu_affinity, 1132 /* wglEnumGpusFromAffinityDCNV */);
}

static PFNWGLENUMGPUSNVPROC
epoxy_wglEnumGpusNV_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_NV_gpu_affinity, 1160 /* wglEnumGpusNV */);
}

static PFNWGLENUMERATEVIDEOCAPTUREDEVICESNVPROC
epoxy_wglEnumerateVideoCaptureDevicesNV_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_NV_video_capture, 1174 /* wglEnumerateVideoCaptureDevicesNV */);
}

static PFNWGLENUMERATEVIDEODEVICESNVPROC
epoxy_wglEnumerateVideoDevicesNV_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_NV_present_video, 1208 /* wglEnumerateVideoDevicesNV */);
}

static PFNWGLFREEMEMORYNVPROC
epoxy_wglFreeMemoryNV_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_NV_vertex_array_range, 1235 /* wglFreeMemoryNV */);
}

static PFNWGLGENLOCKSAMPLERATEI3DPROC
epoxy_wglGenlockSampleRateI3D_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_I3D_genlock, 1251 /* wglGenlockSampleRateI3D */);
}

static PFNWGLGENLOCKSOURCEDELAYI3DPROC
epoxy_wglGenlockSourceDelayI3D_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_I3D_genlock, 1275 /* wglGenlockSourceDelayI3D */);
}

static PFNWGLGENLOCKSOURCEEDGEI3DPROC
epoxy_wglGenlockSourceEdgeI3D_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_I3D_genlock, 1300 /* wglGenlockSourceEdgeI3D */);
}

static PFNWGLGENLOCKSOURCEI3DPROC
epoxy_wglGenlockSourceI3D_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_I3D_genlock, 1324 /* wglGenlockSourceI3D */);
}

static PFNWGLGETCONTEXTGPUIDAMDPROC
epoxy_wglGetContextGPUIDAMD_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_AMD_gpu_association, 1344 /* wglGetContextGPUIDAMD */);
}

static PFNWGLGETCURRENTASSOCIATEDCONTEXTAMDPROC
epoxy_wglGetCurrentAssociatedContextAMD_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_AMD_gpu_association, 1366 /* wglGetCurrentAssociatedContextAMD */);
}

static PFNWGLGETCURRENTCONTEXTPROC
epoxy_wglGetCurrentContext_resolver(void)
{
    return wgl_single_resolver(WGL_10, 1400 /* wglGetCurrentContext */);
}

static PFNWGLGETCURRENTDCPROC
epoxy_wglGetCurrentDC_resolver(void)
{
    return wgl_single_resolver(WGL_10, 1421 /* wglGetCurrentDC */);
}

static PFNWGLGETCURRENTREADDCARBPROC
epoxy_wglGetCurrentReadDCARB_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_ARB_make_current_read, 1437 /* wglGetCurrentReadDCARB */);
}

static PFNWGLGETCURRENTREADDCEXTPROC
epoxy_wglGetCurrentReadDCEXT_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_EXT_make_current_read, 1460 /* wglGetCurrentReadDCEXT */);
}

static PFNWGLGETDEFAULTPROCADDRESSPROC
epoxy_wglGetDefaultProcAddress_resolver(void)
{
    static const enum wgl_provider providers[] = {
        wgl_provider_terminator
    };
    static const uint32_t entrypoints[] = {
        0 /* None */,
    };
    return wgl_provider_resolver(entrypoint_strings + 1483 /* "wglGetDefaultProcAddress" */,
                                providers, entrypoints);
}

static PFNWGLGETDIGITALVIDEOPARAMETERSI3DPROC
epoxy_wglGetDigitalVideoParametersI3D_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_I3D_digital_video_control, 1508 /* wglGetDigitalVideoParametersI3D */);
}

static PFNWGLGETEXTENSIONSSTRINGARBPROC
epoxy_wglGetExtensionsStringARB_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_ARB_extensions_string, 1540 /* wglGetExtensionsStringARB */);
}

static PFNWGLGETEXTENSIONSSTRINGEXTPROC
epoxy_wglGetExtensionsStringEXT_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_EXT_extensions_string, 1566 /* wglGetExtensionsStringEXT */);
}

static PFNWGLGETFRAMEUSAGEI3DPROC
epoxy_wglGetFrameUsageI3D_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_I3D_swap_frame_usage, 1592 /* wglGetFrameUsageI3D */);
}

static PFNWGLGETGPUIDSAMDPROC
epoxy_wglGetGPUIDsAMD_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_AMD_gpu_association, 1612 /* wglGetGPUIDsAMD */);
}

static PFNWGLGETGPUINFOAMDPROC
epoxy_wglGetGPUInfoAMD_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_AMD_gpu_association, 1628 /* wglGetGPUInfoAMD */);
}

static PFNWGLGETGAMMATABLEI3DPROC
epoxy_wglGetGammaTableI3D_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_I3D_gamma, 1645 /* wglGetGammaTableI3D */);
}

static PFNWGLGETGAMMATABLEPARAMETERSI3DPROC
epoxy_wglGetGammaTableParametersI3D_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_I3D_gamma, 1665 /* wglGetGammaTableParametersI3D */);
}

static PFNWGLGETGENLOCKSAMPLERATEI3DPROC
epoxy_wglGetGenlockSampleRateI3D_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_I3D_genlock, 1695 /* wglGetGenlockSampleRateI3D */);
}

static PFNWGLGETGENLOCKSOURCEDELAYI3DPROC
epoxy_wglGetGenlockSourceDelayI3D_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_I3D_genlock, 1722 /* wglGetGenlockSourceDelayI3D */);
}

static PFNWGLGETGENLOCKSOURCEEDGEI3DPROC
epoxy_wglGetGenlockSourceEdgeI3D_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_I3D_genlock, 1750 /* wglGetGenlockSourceEdgeI3D */);
}

static PFNWGLGETGENLOCKSOURCEI3DPROC
epoxy_wglGetGenlockSourceI3D_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_I3D_genlock, 1777 /* wglGetGenlockSourceI3D */);
}

static PFNWGLGETLAYERPALETTEENTRIESPROC
epoxy_wglGetLayerPaletteEntries_resolver(void)
{
    return wgl_single_resolver(WGL_10, 1800 /* wglGetLayerPaletteEntries */);
}

static PFNWGLGETMSCRATEOMLPROC
epoxy_wglGetMscRateOML_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_OML_sync_control, 1826 /* wglGetMscRateOML */);
}

static PFNWGLGETPBUFFERDCARBPROC
epoxy_wglGetPbufferDCARB_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_ARB_pbuffer, 1843 /* wglGetPbufferDCARB */);
}

static PFNWGLGETPBUFFERDCEXTPROC
epoxy_wglGetPbufferDCEXT_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_EXT_pbuffer, 1862 /* wglGetPbufferDCEXT */);
}

static PFNWGLGETPIXELFORMATATTRIBFVARBPROC
epoxy_wglGetPixelFormatAttribfvARB_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_ARB_pixel_format, 1881 /* wglGetPixelFormatAttribfvARB */);
}

static PFNWGLGETPIXELFORMATATTRIBFVEXTPROC
epoxy_wglGetPixelFormatAttribfvEXT_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_EXT_pixel_format, 1910 /* wglGetPixelFormatAttribfvEXT */);
}

static PFNWGLGETPIXELFORMATATTRIBIVARBPROC
epoxy_wglGetPixelFormatAttribivARB_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_ARB_pixel_format, 1939 /* wglGetPixelFormatAttribivARB */);
}

static PFNWGLGETPIXELFORMATATTRIBIVEXTPROC
epoxy_wglGetPixelFormatAttribivEXT_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_EXT_pixel_format, 1968 /* wglGetPixelFormatAttribivEXT */);
}

static PFNWGLGETPROCADDRESSPROC
epoxy_wglGetProcAddress_resolver(void)
{
    return wgl_single_resolver(WGL_10, 1997 /* wglGetProcAddress */);
}

static PFNWGLGETSWAPINTERVALEXTPROC
epoxy_wglGetSwapIntervalEXT_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_EXT_swap_control, 2015 /* wglGetSwapIntervalEXT */);
}

static PFNWGLGETSYNCVALUESOMLPROC
epoxy_wglGetSyncValuesOML_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_OML_sync_control, 2037 /* wglGetSyncValuesOML */);
}

static PFNWGLGETVIDEODEVICENVPROC
epoxy_wglGetVideoDeviceNV_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_NV_video_output, 2057 /* wglGetVideoDeviceNV */);
}

static PFNWGLGETVIDEOINFONVPROC
epoxy_wglGetVideoInfoNV_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_NV_video_output, 2077 /* wglGetVideoInfoNV */);
}

static PFNWGLISENABLEDFRAMELOCKI3DPROC
epoxy_wglIsEnabledFrameLockI3D_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_I3D_swap_frame_lock, 2095 /* wglIsEnabledFrameLockI3D */);
}

static PFNWGLISENABLEDGENLOCKI3DPROC
epoxy_wglIsEnabledGenlockI3D_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_I3D_genlock, 2120 /* wglIsEnabledGenlockI3D */);
}

static PFNWGLJOINSWAPGROUPNVPROC
epoxy_wglJoinSwapGroupNV_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_NV_swap_group, 2143 /* wglJoinSwapGroupNV */);
}

static PFNWGLLOADDISPLAYCOLORTABLEEXTPROC
epoxy_wglLoadDisplayColorTableEXT_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_EXT_display_color_table, 2162 /* wglLoadDisplayColorTableEXT */);
}

static PFNWGLLOCKVIDEOCAPTUREDEVICENVPROC
epoxy_wglLockVideoCaptureDeviceNV_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_NV_video_capture, 2190 /* wglLockVideoCaptureDeviceNV */);
}

static PFNWGLMAKEASSOCIATEDCONTEXTCURRENTAMDPROC
epoxy_wglMakeAssociatedContextCurrentAMD_unwrapped_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_AMD_gpu_association, 2218 /* wglMakeAssociatedContextCurrentAMD */);
}

static PFNWGLMAKECONTEXTCURRENTARBPROC
epoxy_wglMakeContextCurrentARB_unwrapped_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_ARB_make_current_read, 2253 /* wglMakeContextCurrentARB */);
}

static PFNWGLMAKECONTEXTCURRENTEXTPROC
epoxy_wglMakeContextCurrentEXT_unwrapped_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_EXT_make_current_read, 2278 /* wglMakeContextCurrentEXT */);
}

static PFNWGLMAKECURRENTPROC
epoxy_wglMakeCurrent_unwrapped_resolver(void)
{
    return wgl_single_resolver(WGL_10, 2303 /* wglMakeCurrent */);
}

static PFNWGLQUERYCURRENTCONTEXTNVPROC
epoxy_wglQueryCurrentContextNV_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_NV_present_video, 2318 /* wglQueryCurrentContextNV */);
}

static PFNWGLQUERYFRAMECOUNTNVPROC
epoxy_wglQueryFrameCountNV_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_NV_swap_group, 2343 /* wglQueryFrameCountNV */);
}

static PFNWGLQUERYFRAMELOCKMASTERI3DPROC
epoxy_wglQueryFrameLockMasterI3D_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_I3D_swap_frame_lock, 2364 /* wglQueryFrameLockMasterI3D */);
}

static PFNWGLQUERYFRAMETRACKINGI3DPROC
epoxy_wglQueryFrameTrackingI3D_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_I3D_swap_frame_usage, 2391 /* wglQueryFrameTrackingI3D */);
}

static PFNWGLQUERYGENLOCKMAXSOURCEDELAYI3DPROC
epoxy_wglQueryGenlockMaxSourceDelayI3D_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_I3D_genlock, 2416 /* wglQueryGenlockMaxSourceDelayI3D */);
}

static PFNWGLQUERYMAXSWAPGROUPSNVPROC
epoxy_wglQueryMaxSwapGroupsNV_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_NV_swap_group, 2449 /* wglQueryMaxSwapGroupsNV */);
}

static PFNWGLQUERYPBUFFERARBPROC
epoxy_wglQueryPbufferARB_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_ARB_pbuffer, 2473 /* wglQueryPbufferARB */);
}

static PFNWGLQUERYPBUFFEREXTPROC
epoxy_wglQueryPbufferEXT_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_EXT_pbuffer, 2492 /* wglQueryPbufferEXT */);
}

static PFNWGLQUERYSWAPGROUPNVPROC
epoxy_wglQuerySwapGroupNV_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_NV_swap_group, 2511 /* wglQuerySwapGroupNV */);
}

static PFNWGLQUERYVIDEOCAPTUREDEVICENVPROC
epoxy_wglQueryVideoCaptureDeviceNV_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_NV_video_capture, 2531 /* wglQueryVideoCaptureDeviceNV */);
}

static PFNWGLREALIZELAYERPALETTEPROC
epoxy_wglRealizeLayerPalette_resolver(void)
{
    return wgl_single_resolver(WGL_10, 2560 /* wglRealizeLayerPalette */);
}

static PFNWGLRELEASEIMAGEBUFFEREVENTSI3DPROC
epoxy_wglReleaseImageBufferEventsI3D_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_I3D_image_buffer, 2583 /* wglReleaseImageBufferEventsI3D */);
}

static PFNWGLRELEASEPBUFFERDCARBPROC
epoxy_wglReleasePbufferDCARB_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_ARB_pbuffer, 2614 /* wglReleasePbufferDCARB */);
}

static PFNWGLRELEASEPBUFFERDCEXTPROC
epoxy_wglReleasePbufferDCEXT_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_EXT_pbuffer, 2637 /* wglReleasePbufferDCEXT */);
}

static PFNWGLRELEASETEXIMAGEARBPROC
epoxy_wglReleaseTexImageARB_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_ARB_render_texture, 2660 /* wglReleaseTexImageARB */);
}

static PFNWGLRELEASEVIDEOCAPTUREDEVICENVPROC
epoxy_wglReleaseVideoCaptureDeviceNV_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_NV_video_capture, 2682 /* wglReleaseVideoCaptureDeviceNV */);
}

static PFNWGLRELEASEVIDEODEVICENVPROC
epoxy_wglReleaseVideoDeviceNV_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_NV_video_output, 2713 /* wglReleaseVideoDeviceNV */);
}

static PFNWGLRELEASEVIDEOIMAGENVPROC
epoxy_wglReleaseVideoImageNV_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_NV_video_output, 2737 /* wglReleaseVideoImageNV */);
}

static PFNWGLRESETFRAMECOUNTNVPROC
epoxy_wglResetFrameCountNV_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_NV_swap_group, 2760 /* wglResetFrameCountNV */);
}

static PFNWGLRESTOREBUFFERREGIONARBPROC
epoxy_wglRestoreBufferRegionARB_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_ARB_buffer_region, 2781 /* wglRestoreBufferRegionARB */);
}

static PFNWGLSAVEBUFFERREGIONARBPROC
epoxy_wglSaveBufferRegionARB_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_ARB_buffer_region, 2807 /* wglSaveBufferRegionARB */);
}

static PFNWGLSENDPBUFFERTOVIDEONVPROC
epoxy_wglSendPbufferToVideoNV_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_NV_video_output, 2830 /* wglSendPbufferToVideoNV */);
}

static PFNWGLSETDIGITALVIDEOPARAMETERSI3DPROC
epoxy_wglSetDigitalVideoParametersI3D_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_I3D_digital_video_control, 2854 /* wglSetDigitalVideoParametersI3D */);
}

static PFNWGLSETGAMMATABLEI3DPROC
epoxy_wglSetGammaTableI3D_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_I3D_gamma, 2886 /* wglSetGammaTableI3D */);
}

static PFNWGLSETGAMMATABLEPARAMETERSI3DPROC
epoxy_wglSetGammaTableParametersI3D_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_I3D_gamma, 2906 /* wglSetGammaTableParametersI3D */);
}

static PFNWGLSETLAYERPALETTEENTRIESPROC
epoxy_wglSetLayerPaletteEntries_resolver(void)
{
    return wgl_single_resolver(WGL_10, 2936 /* wglSetLayerPaletteEntries */);
}

static PFNWGLSETPBUFFERATTRIBARBPROC
epoxy_wglSetPbufferAttribARB_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_ARB_render_texture, 2962 /* wglSetPbufferAttribARB */);
}

static PFNWGLSETSTEREOEMITTERSTATE3DLPROC
epoxy_wglSetStereoEmitterState3DL_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_3DL_stereo_control, 2985 /* wglSetStereoEmitterState3DL */);
}

static PFNWGLSHARELISTSPROC
epoxy_wglShareLists_resolver(void)
{
    return wgl_single_resolver(WGL_10, 3013 /* wglShareLists */);
}

static PFNWGLSWAPBUFFERSMSCOMLPROC
epoxy_wglSwapBuffersMscOML_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_OML_sync_control, 3027 /* wglSwapBuffersMscOML */);
}

static PFNWGLSWAPINTERVALEXTPROC
epoxy_wglSwapIntervalEXT_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_EXT_swap_control, 3048 /* wglSwapIntervalEXT */);
}

static PFNWGLSWAPLAYERBUFFERSPROC
epoxy_wglSwapLayerBuffers_resolver(void)
{
    return wgl_single_resolver(WGL_10, 3067 /* wglSwapLayerBuffers */);
}

static PFNWGLSWAPLAYERBUFFERSMSCOMLPROC
epoxy_wglSwapLayerBuffersMscOML_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_OML_sync_control, 3087 /* wglSwapLayerBuffersMscOML */);
}

static PFNWGLUSEFONTBITMAPSAPROC
epoxy_wglUseFontBitmapsA_resolver(void)
{
    return wgl_single_resolver(WGL_10, 3113 /* wglUseFontBitmapsA */);
}

static PFNWGLUSEFONTBITMAPSWPROC
epoxy_wglUseFontBitmapsW_resolver(void)
{
    return wgl_single_resolver(WGL_10, 3132 /* wglUseFontBitmapsW */);
}

static PFNWGLUSEFONTOUTLINESPROC
epoxy_wglUseFontOutlines_resolver(void)
{
    return wgl_single_resolver(WGL_10, 3151 /* wglUseFontOutlines */);
}

static PFNWGLUSEFONTOUTLINESAPROC
epoxy_wglUseFontOutlinesA_resolver(void)
{
    return wgl_single_resolver(WGL_10, 3170 /* wglUseFontOutlinesA */);
}

static PFNWGLUSEFONTOUTLINESWPROC
epoxy_wglUseFontOutlinesW_resolver(void)
{
    return wgl_single_resolver(WGL_10, 3190 /* wglUseFontOutlinesW */);
}

static PFNWGLWAITFORMSCOMLPROC
epoxy_wglWaitForMscOML_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_OML_sync_control, 3210 /* wglWaitForMscOML */);
}

static PFNWGLWAITFORSBCOMLPROC
epoxy_wglWaitForSbcOML_resolver(void)
{
    return wgl_single_resolver(WGL_extension_WGL_OML_sync_control, 3227 /* wglWaitForSbcOML */);
}

GEN_THUNKS_RET(void *, wglAllocateMemoryNV, (GLsizei size, GLfloat readfreq, GLfloat writefreq, GLfloat priority), (size, readfreq, writefreq, priority))
GEN_THUNKS_RET(BOOL, wglAssociateImageBufferEventsI3D, (HDC hDC, const HANDLE * pEvent, const LPVOID * pAddress, const DWORD * pSize, UINT count), (hDC, pEvent, pAddress, pSize, count))
GEN_THUNKS_RET(BOOL, wglBeginFrameTrackingI3D, (void), ())
GEN_THUNKS_RET(GLboolean, wglBindDisplayColorTableEXT, (GLushort id), (id))
GEN_THUNKS_RET(BOOL, wglBindSwapBarrierNV, (GLuint group, GLuint barrier), (group, barrier))
GEN_THUNKS_RET(BOOL, wglBindTexImageARB, (HPBUFFERARB hPbuffer, int iBuffer), (hPbuffer, iBuffer))
GEN_THUNKS_RET(BOOL, wglBindVideoCaptureDeviceNV, (UINT uVideoSlot, HVIDEOINPUTDEVICENV hDevice), (uVideoSlot, hDevice))
GEN_THUNKS_RET(BOOL, wglBindVideoDeviceNV, (HDC hDC, unsigned int uVideoSlot, HVIDEOOUTPUTDEVICENV hVideoDevice, const int * piAttribList), (hDC, uVideoSlot, hVideoDevice, piAttribList))
GEN_THUNKS_RET(BOOL, wglBindVideoImageNV, (HPVIDEODEV hVideoDevice, HPBUFFERARB hPbuffer, int iVideoBuffer), (hVideoDevice, hPbuffer, iVideoBuffer))
GEN_THUNKS(wglBlitContextFramebufferAMD, (HGLRC dstCtx, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter), (dstCtx, srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter))
GEN_THUNKS_RET(BOOL, wglChoosePixelFormatARB, (HDC hdc, const int * piAttribIList, const FLOAT * pfAttribFList, UINT nMaxFormats, int * piFormats, UINT * nNumFormats), (hdc, piAttribIList, pfAttribFList, nMaxFormats, piFormats, nNumFormats))
GEN_THUNKS_RET(BOOL, wglChoosePixelFormatEXT, (HDC hdc, const int * piAttribIList, const FLOAT * pfAttribFList, UINT nMaxFormats, int * piFormats, UINT * nNumFormats), (hdc, piAttribIList, pfAttribFList, nMaxFormats, piFormats, nNumFormats))
GEN_THUNKS_RET(BOOL, wglCopyContext, (HGLRC hglrcSrc, HGLRC hglrcDst, UINT mask), (hglrcSrc, hglrcDst, mask))
GEN_THUNKS_RET(BOOL, wglCopyImageSubDataNV, (HGLRC hSrcRC, GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, HGLRC hDstRC, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei width, GLsizei height, GLsizei depth), (hSrcRC, srcName, srcTarget, srcLevel, srcX, srcY, srcZ, hDstRC, dstName, dstTarget, dstLevel, dstX, dstY, dstZ, width, height, depth))
GEN_THUNKS_RET(HDC, wglCreateAffinityDCNV, (const HGPUNV * phGpuList), (phGpuList))
GEN_THUNKS_RET(HGLRC, wglCreateAssociatedContextAMD, (UINT id), (id))
GEN_THUNKS_RET(HGLRC, wglCreateAssociatedContextAttribsAMD, (UINT id, HGLRC hShareContext, const int * attribList), (id, hShareContext, attribList))
GEN_THUNKS_RET(HANDLE, wglCreateBufferRegionARB, (HDC hDC, int iLayerPlane, UINT uType), (hDC, iLayerPlane, uType))
GEN_THUNKS_RET(HGLRC, wglCreateContext, (HDC hDc), (hDc))
GEN_THUNKS_RET(HGLRC, wglCreateContextAttribsARB, (HDC hDC, HGLRC hShareContext, const int * attribList), (hDC, hShareContext, attribList))
GEN_THUNKS_RET(GLboolean, wglCreateDisplayColorTableEXT, (GLushort id), (id))
GEN_THUNKS_RET(LPVOID, wglCreateImageBufferI3D, (HDC hDC, DWORD dwSize, UINT uFlags), (hDC, dwSize, uFlags))
GEN_THUNKS_RET(HGLRC, wglCreateLayerContext, (HDC hDc, int level), (hDc, level))
GEN_THUNKS_RET(HPBUFFERARB, wglCreatePbufferARB, (HDC hDC, int iPixelFormat, int iWidth, int iHeight, const int * piAttribList), (hDC, iPixelFormat, iWidth, iHeight, piAttribList))
GEN_THUNKS_RET(HPBUFFEREXT, wglCreatePbufferEXT, (HDC hDC, int iPixelFormat, int iWidth, int iHeight, const int * piAttribList), (hDC, iPixelFormat, iWidth, iHeight, piAttribList))
GEN_THUNKS_RET(BOOL, wglDXCloseDeviceNV, (HANDLE hDevice), (hDevice))
GEN_THUNKS_RET(BOOL, wglDXLockObjectsNV, (HANDLE hDevice, GLint count, HANDLE * hObjects), (hDevice, count, hObjects))
GEN_THUNKS_RET(BOOL, wglDXObjectAccessNV, (HANDLE hObject, GLenum access), (hObject, access))
GEN_THUNKS_RET(HANDLE, wglDXOpenDeviceNV, (void * dxDevice), (dxDevice))
GEN_THUNKS_RET(HANDLE, wglDXRegisterObjectNV, (HANDLE hDevice, void * dxObject, GLuint name, GLenum type, GLenum access), (hDevice, dxObject, name, type, access))
GEN_THUNKS_RET(BOOL, wglDXSetResourceShareHandleNV, (void * dxObject, HANDLE shareHandle), (dxObject, shareHandle))
GEN_THUNKS_RET(BOOL, wglDXUnlockObjectsNV, (HANDLE hDevice, GLint count, HANDLE * hObjects), (hDevice, count, hObjects))
GEN_THUNKS_RET(BOOL, wglDXUnregisterObjectNV, (HANDLE hDevice, HANDLE hObject), (hDevice, hObject))
GEN_THUNKS_RET(BOOL, wglDelayBeforeSwapNV, (HDC hDC, GLfloat seconds), (hDC, seconds))
GEN_THUNKS_RET(BOOL, wglDeleteAssociatedContextAMD, (HGLRC hglrc), (hglrc))
GEN_THUNKS(wglDeleteBufferRegionARB, (HANDLE hRegion), (hRegion))
GEN_THUNKS_RET(BOOL, wglDeleteContext, (HGLRC oldContext), (oldContext))
GEN_THUNKS_RET(BOOL, wglDeleteDCNV, (HDC hdc), (hdc))
GEN_THUNKS_RET(BOOL, wglDescribeLayerPlane, (HDC hDc, int pixelFormat, int layerPlane, UINT nBytes, const LAYERPLANEDESCRIPTOR * plpd), (hDc, pixelFormat, layerPlane, nBytes, plpd))
GEN_THUNKS(wglDestroyDisplayColorTableEXT, (GLushort id), (id))
GEN_THUNKS_RET(BOOL, wglDestroyImageBufferI3D, (HDC hDC, LPVOID pAddress), (hDC, pAddress))
GEN_THUNKS_RET(BOOL, wglDestroyPbufferARB, (HPBUFFERARB hPbuffer), (hPbuffer))
GEN_THUNKS_RET(BOOL, wglDestroyPbufferEXT, (HPBUFFEREXT hPbuffer), (hPbuffer))
GEN_THUNKS_RET(BOOL, wglDisableFrameLockI3D, (void), ())
GEN_THUNKS_RET(BOOL, wglDisableGenlockI3D, (HDC hDC), (hDC))
GEN_THUNKS_RET(BOOL, wglEnableFrameLockI3D, (void), ())
GEN_THUNKS_RET(BOOL, wglEnableGenlockI3D, (HDC hDC), (hDC))
GEN_THUNKS_RET(BOOL, wglEndFrameTrackingI3D, (void), ())
GEN_THUNKS_RET(BOOL, wglEnumGpuDevicesNV, (HGPUNV hGpu, UINT iDeviceIndex, PGPU_DEVICE lpGpuDevice), (hGpu, iDeviceIndex, lpGpuDevice))
GEN_THUNKS_RET(BOOL, wglEnumGpusFromAffinityDCNV, (HDC hAffinityDC, UINT iGpuIndex, HGPUNV * hGpu), (hAffinityDC, iGpuIndex, hGpu))
GEN_THUNKS_RET(BOOL, wglEnumGpusNV, (UINT iGpuIndex, HGPUNV * phGpu), (iGpuIndex, phGpu))
GEN_THUNKS_RET(UINT, wglEnumerateVideoCaptureDevicesNV, (HDC hDc, HVIDEOINPUTDEVICENV * phDeviceList), (hDc, phDeviceList))
GEN_THUNKS_RET(int, wglEnumerateVideoDevicesNV, (HDC hDC, HVIDEOOUTPUTDEVICENV * phDeviceList), (hDC, phDeviceList))
GEN_THUNKS(wglFreeMemoryNV, (void * pointer), (pointer))
GEN_THUNKS_RET(BOOL, wglGenlockSampleRateI3D, (HDC hDC, UINT uRate), (hDC, uRate))
GEN_THUNKS_RET(BOOL, wglGenlockSourceDelayI3D, (HDC hDC, UINT uDelay), (hDC, uDelay))
GEN_THUNKS_RET(BOOL, wglGenlockSourceEdgeI3D, (HDC hDC, UINT uEdge), (hDC, uEdge))
GEN_THUNKS_RET(BOOL, wglGenlockSourceI3D, (HDC hDC, UINT uSource), (hDC, uSource))
GEN_THUNKS_RET(UINT, wglGetContextGPUIDAMD, (HGLRC hglrc), (hglrc))
GEN_THUNKS_RET(HGLRC, wglGetCurrentAssociatedContextAMD, (void), ())
GEN_THUNKS_RET(HGLRC, wglGetCurrentContext, (void), ())
GEN_THUNKS_RET(HDC, wglGetCurrentDC, (void), ())
GEN_THUNKS_RET(HDC, wglGetCurrentReadDCARB, (void), ())
GEN_THUNKS_RET(HDC, wglGetCurrentReadDCEXT, (void), ())
GEN_THUNKS_RET(PROC, wglGetDefaultProcAddress, (LPCSTR lpszProc), (lpszProc))
GEN_THUNKS_RET(BOOL, wglGetDigitalVideoParametersI3D, (HDC hDC, int iAttribute, int * piValue), (hDC, iAttribute, piValue))
GEN_THUNKS_RET(const char *, wglGetExtensionsStringARB, (HDC hdc), (hdc))
GEN_THUNKS_RET(const char *, wglGetExtensionsStringEXT, (void), ())
GEN_THUNKS_RET(BOOL, wglGetFrameUsageI3D, (float * pUsage), (pUsage))
GEN_THUNKS_RET(UINT, wglGetGPUIDsAMD, (UINT maxCount, UINT * ids), (maxCount, ids))
GEN_THUNKS_RET(INT, wglGetGPUInfoAMD, (UINT id, int property, GLenum dataType, UINT size, void * data), (id, property, dataType, size, data))
GEN_THUNKS_RET(BOOL, wglGetGammaTableI3D, (HDC hDC, int iEntries, USHORT * puRed, USHORT * puGreen, USHORT * puBlue), (hDC, iEntries, puRed, puGreen, puBlue))
GEN_THUNKS_RET(BOOL, wglGetGammaTableParametersI3D, (HDC hDC, int iAttribute, int * piValue), (hDC, iAttribute, piValue))
GEN_THUNKS_RET(BOOL, wglGetGenlockSampleRateI3D, (HDC hDC, UINT * uRate), (hDC, uRate))
GEN_THUNKS_RET(BOOL, wglGetGenlockSourceDelayI3D, (HDC hDC, UINT * uDelay), (hDC, uDelay))
GEN_THUNKS_RET(BOOL, wglGetGenlockSourceEdgeI3D, (HDC hDC, UINT * uEdge), (hDC, uEdge))
GEN_THUNKS_RET(BOOL, wglGetGenlockSourceI3D, (HDC hDC, UINT * uSource), (hDC, uSource))
GEN_THUNKS_RET(int, wglGetLayerPaletteEntries, (HDC hdc, int iLayerPlane, int iStart, int cEntries, const COLORREF * pcr), (hdc, iLayerPlane, iStart, cEntries, pcr))
GEN_THUNKS_RET(BOOL, wglGetMscRateOML, (HDC hdc, INT32 * numerator, INT32 * denominator), (hdc, numerator, denominator))
GEN_THUNKS_RET(HDC, wglGetPbufferDCARB, (HPBUFFERARB hPbuffer), (hPbuffer))
GEN_THUNKS_RET(HDC, wglGetPbufferDCEXT, (HPBUFFEREXT hPbuffer), (hPbuffer))
GEN_THUNKS_RET(BOOL, wglGetPixelFormatAttribfvARB, (HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int * piAttributes, FLOAT * pfValues), (hdc, iPixelFormat, iLayerPlane, nAttributes, piAttributes, pfValues))
GEN_THUNKS_RET(BOOL, wglGetPixelFormatAttribfvEXT, (HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, int * piAttributes, FLOAT * pfValues), (hdc, iPixelFormat, iLayerPlane, nAttributes, piAttributes, pfValues))
GEN_THUNKS_RET(BOOL, wglGetPixelFormatAttribivARB, (HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int * piAttributes, int * piValues), (hdc, iPixelFormat, iLayerPlane, nAttributes, piAttributes, piValues))
GEN_THUNKS_RET(BOOL, wglGetPixelFormatAttribivEXT, (HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, int * piAttributes, int * piValues), (hdc, iPixelFormat, iLayerPlane, nAttributes, piAttributes, piValues))
GEN_THUNKS_RET(PROC, wglGetProcAddress, (LPCSTR lpszProc), (lpszProc))
GEN_THUNKS_RET(int, wglGetSwapIntervalEXT, (void), ())
GEN_THUNKS_RET(BOOL, wglGetSyncValuesOML, (HDC hdc, INT64 * ust, INT64 * msc, INT64 * sbc), (hdc, ust, msc, sbc))
GEN_THUNKS_RET(BOOL, wglGetVideoDeviceNV, (HDC hDC, int numDevices, HPVIDEODEV * hVideoDevice), (hDC, numDevices, hVideoDevice))
GEN_THUNKS_RET(BOOL, wglGetVideoInfoNV, (HPVIDEODEV hpVideoDevice, unsigned long * pulCounterOutputPbuffer, unsigned long * pulCounterOutputVideo), (hpVideoDevice, pulCounterOutputPbuffer, pulCounterOutputVideo))
GEN_THUNKS_RET(BOOL, wglIsEnabledFrameLockI3D, (BOOL * pFlag), (pFlag))
GEN_THUNKS_RET(BOOL, wglIsEnabledGenlockI3D, (HDC hDC, BOOL * pFlag), (hDC, pFlag))
GEN_THUNKS_RET(BOOL, wglJoinSwapGroupNV, (HDC hDC, GLuint group), (hDC, group))
GEN_THUNKS_RET(GLboolean, wglLoadDisplayColorTableEXT, (const GLushort * table, GLuint length), (table, length))
GEN_THUNKS_RET(BOOL, wglLockVideoCaptureDeviceNV, (HDC hDc, HVIDEOINPUTDEVICENV hDevice), (hDc, hDevice))
GEN_THUNKS_RET(BOOL, wglMakeAssociatedContextCurrentAMD_unwrapped, (HGLRC hglrc), (hglrc))
GEN_THUNKS_RET(BOOL, wglMakeContextCurrentARB_unwrapped, (HDC hDrawDC, HDC hReadDC, HGLRC hglrc), (hDrawDC, hReadDC, hglrc))
GEN_THUNKS_RET(BOOL, wglMakeContextCurrentEXT_unwrapped, (HDC hDrawDC, HDC hReadDC, HGLRC hglrc), (hDrawDC, hReadDC, hglrc))
GEN_THUNKS_RET(BOOL, wglMakeCurrent_unwrapped, (HDC hDc, HGLRC newContext), (hDc, newContext))
GEN_THUNKS_RET(BOOL, wglQueryCurrentContextNV, (int iAttribute, int * piValue), (iAttribute, piValue))
GEN_THUNKS_RET(BOOL, wglQueryFrameCountNV, (HDC hDC, GLuint * count), (hDC, count))
GEN_THUNKS_RET(BOOL, wglQueryFrameLockMasterI3D, (BOOL * pFlag), (pFlag))
GEN_THUNKS_RET(BOOL, wglQueryFrameTrackingI3D, (DWORD * pFrameCount, DWORD * pMissedFrames, float * pLastMissedUsage), (pFrameCount, pMissedFrames, pLastMissedUsage))
GEN_THUNKS_RET(BOOL, wglQueryGenlockMaxSourceDelayI3D, (HDC hDC, UINT * uMaxLineDelay, UINT * uMaxPixelDelay), (hDC, uMaxLineDelay, uMaxPixelDelay))
GEN_THUNKS_RET(BOOL, wglQueryMaxSwapGroupsNV, (HDC hDC, GLuint * maxGroups, GLuint * maxBarriers), (hDC, maxGroups, maxBarriers))
GEN_THUNKS_RET(BOOL, wglQueryPbufferARB, (HPBUFFERARB hPbuffer, int iAttribute, int * piValue), (hPbuffer, iAttribute, piValue))
GEN_THUNKS_RET(BOOL, wglQueryPbufferEXT, (HPBUFFEREXT hPbuffer, int iAttribute, int * piValue), (hPbuffer, iAttribute, piValue))
GEN_THUNKS_RET(BOOL, wglQuerySwapGroupNV, (HDC hDC, GLuint * group, GLuint * barrier), (hDC, group, barrier))
GEN_THUNKS_RET(BOOL, wglQueryVideoCaptureDeviceNV, (HDC hDc, HVIDEOINPUTDEVICENV hDevice, int iAttribute, int * piValue), (hDc, hDevice, iAttribute, piValue))
GEN_THUNKS_RET(BOOL, wglRealizeLayerPalette, (HDC hdc, int iLayerPlane, BOOL bRealize), (hdc, iLayerPlane, bRealize))
GEN_THUNKS_RET(BOOL, wglReleaseImageBufferEventsI3D, (HDC hDC, const LPVOID * pAddress, UINT count), (hDC, pAddress, count))
GEN_THUNKS_RET(int, wglReleasePbufferDCARB, (HPBUFFERARB hPbuffer, HDC hDC), (hPbuffer, hDC))
GEN_THUNKS_RET(int, wglReleasePbufferDCEXT, (HPBUFFEREXT hPbuffer, HDC hDC), (hPbuffer, hDC))
GEN_THUNKS_RET(BOOL, wglReleaseTexImageARB, (HPBUFFERARB hPbuffer, int iBuffer), (hPbuffer, iBuffer))
GEN_THUNKS_RET(BOOL, wglReleaseVideoCaptureDeviceNV, (HDC hDc, HVIDEOINPUTDEVICENV hDevice), (hDc, hDevice))
GEN_THUNKS_RET(BOOL, wglReleaseVideoDeviceNV, (HPVIDEODEV hVideoDevice), (hVideoDevice))
GEN_THUNKS_RET(BOOL, wglReleaseVideoImageNV, (HPBUFFERARB hPbuffer, int iVideoBuffer), (hPbuffer, iVideoBuffer))
GEN_THUNKS_RET(BOOL, wglResetFrameCountNV, (HDC hDC), (hDC))
GEN_THUNKS_RET(BOOL, wglRestoreBufferRegionARB, (HANDLE hRegion, int x, int y, int width, int height, int xSrc, int ySrc), (hRegion, x, y, width, height, xSrc, ySrc))
GEN_THUNKS_RET(BOOL, wglSaveBufferRegionARB, (HANDLE hRegion, int x, int y, int width, int height), (hRegion, x, y, width, height))
GEN_THUNKS_RET(BOOL, wglSendPbufferToVideoNV, (HPBUFFERARB hPbuffer, int iBufferType, unsigned long * pulCounterPbuffer, BOOL bBlock), (hPbuffer, iBufferType, pulCounterPbuffer, bBlock))
GEN_THUNKS_RET(BOOL, wglSetDigitalVideoParametersI3D, (HDC hDC, int iAttribute, const int * piValue), (hDC, iAttribute, piValue))
GEN_THUNKS_RET(BOOL, wglSetGammaTableI3D, (HDC hDC, int iEntries, const USHORT * puRed, const USHORT * puGreen, const USHORT * puBlue), (hDC, iEntries, puRed, puGreen, puBlue))
GEN_THUNKS_RET(BOOL, wglSetGammaTableParametersI3D, (HDC hDC, int iAttribute, const int * piValue), (hDC, iAttribute, piValue))
GEN_THUNKS_RET(int, wglSetLayerPaletteEntries, (HDC hdc, int iLayerPlane, int iStart, int cEntries, const COLORREF * pcr), (hdc, iLayerPlane, iStart, cEntries, pcr))
GEN_THUNKS_RET(BOOL, wglSetPbufferAttribARB, (HPBUFFERARB hPbuffer, const int * piAttribList), (hPbuffer, piAttribList))
GEN_THUNKS_RET(BOOL, wglSetStereoEmitterState3DL, (HDC hDC, UINT uState), (hDC, uState))
GEN_THUNKS_RET(BOOL, wglShareLists, (HGLRC hrcSrvShare, HGLRC hrcSrvSource), (hrcSrvShare, hrcSrvSource))
GEN_THUNKS_RET(INT64, wglSwapBuffersMscOML, (HDC hdc, INT64 target_msc, INT64 divisor, INT64 remainder), (hdc, target_msc, divisor, remainder))
GEN_THUNKS_RET(BOOL, wglSwapIntervalEXT, (int interval), (interval))
GEN_THUNKS_RET(BOOL, wglSwapLayerBuffers, (HDC hdc, UINT fuFlags), (hdc, fuFlags))
GEN_THUNKS_RET(INT64, wglSwapLayerBuffersMscOML, (HDC hdc, int fuPlanes, INT64 target_msc, INT64 divisor, INT64 remainder), (hdc, fuPlanes, target_msc, divisor, remainder))
GEN_THUNKS_RET(BOOL, wglUseFontBitmapsA, (HDC hDC, DWORD first, DWORD count, DWORD listBase), (hDC, first, count, listBase))
GEN_THUNKS_RET(BOOL, wglUseFontBitmapsW, (HDC hDC, DWORD first, DWORD count, DWORD listBase), (hDC, first, count, listBase))
GEN_THUNKS_RET(BOOL, wglUseFontOutlines, (HDC hDC, DWORD first, DWORD count, DWORD listBase, FLOAT deviation, FLOAT extrusion, int format, LPGLYPHMETRICSFLOAT lpgmf), (hDC, first, count, listBase, deviation, extrusion, format, lpgmf))
GEN_THUNKS_RET(BOOL, wglUseFontOutlinesA, (HDC hDC, DWORD first, DWORD count, DWORD listBase, FLOAT deviation, FLOAT extrusion, int format, LPGLYPHMETRICSFLOAT lpgmf), (hDC, first, count, listBase, deviation, extrusion, format, lpgmf))
GEN_THUNKS_RET(BOOL, wglUseFontOutlinesW, (HDC hDC, DWORD first, DWORD count, DWORD listBase, FLOAT deviation, FLOAT extrusion, int format, LPGLYPHMETRICSFLOAT lpgmf), (hDC, first, count, listBase, deviation, extrusion, format, lpgmf))
GEN_THUNKS_RET(BOOL, wglWaitForMscOML, (HDC hdc, INT64 target_msc, INT64 divisor, INT64 remainder, INT64 * ust, INT64 * msc, INT64 * sbc), (hdc, target_msc, divisor, remainder, ust, msc, sbc))
GEN_THUNKS_RET(BOOL, wglWaitForSbcOML, (HDC hdc, INT64 target_sbc, INT64 * ust, INT64 * msc, INT64 * sbc), (hdc, target_sbc, ust, msc, sbc))

#if USING_DISPATCH_TABLE
static struct dispatch_table resolver_table = {
    epoxy_wglAllocateMemoryNV_dispatch_table_rewrite_ptr, /* wglAllocateMemoryNV */
    epoxy_wglAssociateImageBufferEventsI3D_dispatch_table_rewrite_ptr, /* wglAssociateImageBufferEventsI3D */
    epoxy_wglBeginFrameTrackingI3D_dispatch_table_rewrite_ptr, /* wglBeginFrameTrackingI3D */
    epoxy_wglBindDisplayColorTableEXT_dispatch_table_rewrite_ptr, /* wglBindDisplayColorTableEXT */
    epoxy_wglBindSwapBarrierNV_dispatch_table_rewrite_ptr, /* wglBindSwapBarrierNV */
    epoxy_wglBindTexImageARB_dispatch_table_rewrite_ptr, /* wglBindTexImageARB */
    epoxy_wglBindVideoCaptureDeviceNV_dispatch_table_rewrite_ptr, /* wglBindVideoCaptureDeviceNV */
    epoxy_wglBindVideoDeviceNV_dispatch_table_rewrite_ptr, /* wglBindVideoDeviceNV */
    epoxy_wglBindVideoImageNV_dispatch_table_rewrite_ptr, /* wglBindVideoImageNV */
    epoxy_wglBlitContextFramebufferAMD_dispatch_table_rewrite_ptr, /* wglBlitContextFramebufferAMD */
    epoxy_wglChoosePixelFormatARB_dispatch_table_rewrite_ptr, /* wglChoosePixelFormatARB */
    epoxy_wglChoosePixelFormatEXT_dispatch_table_rewrite_ptr, /* wglChoosePixelFormatEXT */
    epoxy_wglCopyContext_dispatch_table_rewrite_ptr, /* wglCopyContext */
    epoxy_wglCopyImageSubDataNV_dispatch_table_rewrite_ptr, /* wglCopyImageSubDataNV */
    epoxy_wglCreateAffinityDCNV_dispatch_table_rewrite_ptr, /* wglCreateAffinityDCNV */
    epoxy_wglCreateAssociatedContextAMD_dispatch_table_rewrite_ptr, /* wglCreateAssociatedContextAMD */
    epoxy_wglCreateAssociatedContextAttribsAMD_dispatch_table_rewrite_ptr, /* wglCreateAssociatedContextAttribsAMD */
    epoxy_wglCreateBufferRegionARB_dispatch_table_rewrite_ptr, /* wglCreateBufferRegionARB */
    epoxy_wglCreateContext_dispatch_table_rewrite_ptr, /* wglCreateContext */
    epoxy_wglCreateContextAttribsARB_dispatch_table_rewrite_ptr, /* wglCreateContextAttribsARB */
    epoxy_wglCreateDisplayColorTableEXT_dispatch_table_rewrite_ptr, /* wglCreateDisplayColorTableEXT */
    epoxy_wglCreateImageBufferI3D_dispatch_table_rewrite_ptr, /* wglCreateImageBufferI3D */
    epoxy_wglCreateLayerContext_dispatch_table_rewrite_ptr, /* wglCreateLayerContext */
    epoxy_wglCreatePbufferARB_dispatch_table_rewrite_ptr, /* wglCreatePbufferARB */
    epoxy_wglCreatePbufferEXT_dispatch_table_rewrite_ptr, /* wglCreatePbufferEXT */
    epoxy_wglDXCloseDeviceNV_dispatch_table_rewrite_ptr, /* wglDXCloseDeviceNV */
    epoxy_wglDXLockObjectsNV_dispatch_table_rewrite_ptr, /* wglDXLockObjectsNV */
    epoxy_wglDXObjectAccessNV_dispatch_table_rewrite_ptr, /* wglDXObjectAccessNV */
    epoxy_wglDXOpenDeviceNV_dispatch_table_rewrite_ptr, /* wglDXOpenDeviceNV */
    epoxy_wglDXRegisterObjectNV_dispatch_table_rewrite_ptr, /* wglDXRegisterObjectNV */
    epoxy_wglDXSetResourceShareHandleNV_dispatch_table_rewrite_ptr, /* wglDXSetResourceShareHandleNV */
    epoxy_wglDXUnlockObjectsNV_dispatch_table_rewrite_ptr, /* wglDXUnlockObjectsNV */
    epoxy_wglDXUnregisterObjectNV_dispatch_table_rewrite_ptr, /* wglDXUnregisterObjectNV */
    epoxy_wglDelayBeforeSwapNV_dispatch_table_rewrite_ptr, /* wglDelayBeforeSwapNV */
    epoxy_wglDeleteAssociatedContextAMD_dispatch_table_rewrite_ptr, /* wglDeleteAssociatedContextAMD */
    epoxy_wglDeleteBufferRegionARB_dispatch_table_rewrite_ptr, /* wglDeleteBufferRegionARB */
    epoxy_wglDeleteContext_dispatch_table_rewrite_ptr, /* wglDeleteContext */
    epoxy_wglDeleteDCNV_dispatch_table_rewrite_ptr, /* wglDeleteDCNV */
    epoxy_wglDescribeLayerPlane_dispatch_table_rewrite_ptr, /* wglDescribeLayerPlane */
    epoxy_wglDestroyDisplayColorTableEXT_dispatch_table_rewrite_ptr, /* wglDestroyDisplayColorTableEXT */
    epoxy_wglDestroyImageBufferI3D_dispatch_table_rewrite_ptr, /* wglDestroyImageBufferI3D */
    epoxy_wglDestroyPbufferARB_dispatch_table_rewrite_ptr, /* wglDestroyPbufferARB */
    epoxy_wglDestroyPbufferEXT_dispatch_table_rewrite_ptr, /* wglDestroyPbufferEXT */
    epoxy_wglDisableFrameLockI3D_dispatch_table_rewrite_ptr, /* wglDisableFrameLockI3D */
    epoxy_wglDisableGenlockI3D_dispatch_table_rewrite_ptr, /* wglDisableGenlockI3D */
    epoxy_wglEnableFrameLockI3D_dispatch_table_rewrite_ptr, /* wglEnableFrameLockI3D */
    epoxy_wglEnableGenlockI3D_dispatch_table_rewrite_ptr, /* wglEnableGenlockI3D */
    epoxy_wglEndFrameTrackingI3D_dispatch_table_rewrite_ptr, /* wglEndFrameTrackingI3D */
    epoxy_wglEnumGpuDevicesNV_dispatch_table_rewrite_ptr, /* wglEnumGpuDevicesNV */
    epoxy_wglEnumGpusFromAffinityDCNV_dispatch_table_rewrite_ptr, /* wglEnumGpusFromAffinityDCNV */
    epoxy_wglEnumGpusNV_dispatch_table_rewrite_ptr, /* wglEnumGpusNV */
    epoxy_wglEnumerateVideoCaptureDevicesNV_dispatch_table_rewrite_ptr, /* wglEnumerateVideoCaptureDevicesNV */
    epoxy_wglEnumerateVideoDevicesNV_dispatch_table_rewrite_ptr, /* wglEnumerateVideoDevicesNV */
    epoxy_wglFreeMemoryNV_dispatch_table_rewrite_ptr, /* wglFreeMemoryNV */
    epoxy_wglGenlockSampleRateI3D_dispatch_table_rewrite_ptr, /* wglGenlockSampleRateI3D */
    epoxy_wglGenlockSourceDelayI3D_dispatch_table_rewrite_ptr, /* wglGenlockSourceDelayI3D */
    epoxy_wglGenlockSourceEdgeI3D_dispatch_table_rewrite_ptr, /* wglGenlockSourceEdgeI3D */
    epoxy_wglGenlockSourceI3D_dispatch_table_rewrite_ptr, /* wglGenlockSourceI3D */
    epoxy_wglGetContextGPUIDAMD_dispatch_table_rewrite_ptr, /* wglGetContextGPUIDAMD */
    epoxy_wglGetCurrentAssociatedContextAMD_dispatch_table_rewrite_ptr, /* wglGetCurrentAssociatedContextAMD */
    epoxy_wglGetCurrentContext_dispatch_table_rewrite_ptr, /* wglGetCurrentContext */
    epoxy_wglGetCurrentDC_dispatch_table_rewrite_ptr, /* wglGetCurrentDC */
    epoxy_wglGetCurrentReadDCARB_dispatch_table_rewrite_ptr, /* wglGetCurrentReadDCARB */
    epoxy_wglGetCurrentReadDCEXT_dispatch_table_rewrite_ptr, /* wglGetCurrentReadDCEXT */
    epoxy_wglGetDefaultProcAddress_dispatch_table_rewrite_ptr, /* wglGetDefaultProcAddress */
    epoxy_wglGetDigitalVideoParametersI3D_dispatch_table_rewrite_ptr, /* wglGetDigitalVideoParametersI3D */
    epoxy_wglGetExtensionsStringARB_dispatch_table_rewrite_ptr, /* wglGetExtensionsStringARB */
    epoxy_wglGetExtensionsStringEXT_dispatch_table_rewrite_ptr, /* wglGetExtensionsStringEXT */
    epoxy_wglGetFrameUsageI3D_dispatch_table_rewrite_ptr, /* wglGetFrameUsageI3D */
    epoxy_wglGetGPUIDsAMD_dispatch_table_rewrite_ptr, /* wglGetGPUIDsAMD */
    epoxy_wglGetGPUInfoAMD_dispatch_table_rewrite_ptr, /* wglGetGPUInfoAMD */
    epoxy_wglGetGammaTableI3D_dispatch_table_rewrite_ptr, /* wglGetGammaTableI3D */
    epoxy_wglGetGammaTableParametersI3D_dispatch_table_rewrite_ptr, /* wglGetGammaTableParametersI3D */
    epoxy_wglGetGenlockSampleRateI3D_dispatch_table_rewrite_ptr, /* wglGetGenlockSampleRateI3D */
    epoxy_wglGetGenlockSourceDelayI3D_dispatch_table_rewrite_ptr, /* wglGetGenlockSourceDelayI3D */
    epoxy_wglGetGenlockSourceEdgeI3D_dispatch_table_rewrite_ptr, /* wglGetGenlockSourceEdgeI3D */
    epoxy_wglGetGenlockSourceI3D_dispatch_table_rewrite_ptr, /* wglGetGenlockSourceI3D */
    epoxy_wglGetLayerPaletteEntries_dispatch_table_rewrite_ptr, /* wglGetLayerPaletteEntries */
    epoxy_wglGetMscRateOML_dispatch_table_rewrite_ptr, /* wglGetMscRateOML */
    epoxy_wglGetPbufferDCARB_dispatch_table_rewrite_ptr, /* wglGetPbufferDCARB */
    epoxy_wglGetPbufferDCEXT_dispatch_table_rewrite_ptr, /* wglGetPbufferDCEXT */
    epoxy_wglGetPixelFormatAttribfvARB_dispatch_table_rewrite_ptr, /* wglGetPixelFormatAttribfvARB */
    epoxy_wglGetPixelFormatAttribfvEXT_dispatch_table_rewrite_ptr, /* wglGetPixelFormatAttribfvEXT */
    epoxy_wglGetPixelFormatAttribivARB_dispatch_table_rewrite_ptr, /* wglGetPixelFormatAttribivARB */
    epoxy_wglGetPixelFormatAttribivEXT_dispatch_table_rewrite_ptr, /* wglGetPixelFormatAttribivEXT */
    epoxy_wglGetProcAddress_dispatch_table_rewrite_ptr, /* wglGetProcAddress */
    epoxy_wglGetSwapIntervalEXT_dispatch_table_rewrite_ptr, /* wglGetSwapIntervalEXT */
    epoxy_wglGetSyncValuesOML_dispatch_table_rewrite_ptr, /* wglGetSyncValuesOML */
    epoxy_wglGetVideoDeviceNV_dispatch_table_rewrite_ptr, /* wglGetVideoDeviceNV */
    epoxy_wglGetVideoInfoNV_dispatch_table_rewrite_ptr, /* wglGetVideoInfoNV */
    epoxy_wglIsEnabledFrameLockI3D_dispatch_table_rewrite_ptr, /* wglIsEnabledFrameLockI3D */
    epoxy_wglIsEnabledGenlockI3D_dispatch_table_rewrite_ptr, /* wglIsEnabledGenlockI3D */
    epoxy_wglJoinSwapGroupNV_dispatch_table_rewrite_ptr, /* wglJoinSwapGroupNV */
    epoxy_wglLoadDisplayColorTableEXT_dispatch_table_rewrite_ptr, /* wglLoadDisplayColorTableEXT */
    epoxy_wglLockVideoCaptureDeviceNV_dispatch_table_rewrite_ptr, /* wglLockVideoCaptureDeviceNV */
    epoxy_wglMakeAssociatedContextCurrentAMD_unwrapped_dispatch_table_rewrite_ptr, /* wglMakeAssociatedContextCurrentAMD_unwrapped */
    epoxy_wglMakeContextCurrentARB_unwrapped_dispatch_table_rewrite_ptr, /* wglMakeContextCurrentARB_unwrapped */
    epoxy_wglMakeContextCurrentEXT_unwrapped_dispatch_table_rewrite_ptr, /* wglMakeContextCurrentEXT_unwrapped */
    epoxy_wglMakeCurrent_unwrapped_dispatch_table_rewrite_ptr, /* wglMakeCurrent_unwrapped */
    epoxy_wglQueryCurrentContextNV_dispatch_table_rewrite_ptr, /* wglQueryCurrentContextNV */
    epoxy_wglQueryFrameCountNV_dispatch_table_rewrite_ptr, /* wglQueryFrameCountNV */
    epoxy_wglQueryFrameLockMasterI3D_dispatch_table_rewrite_ptr, /* wglQueryFrameLockMasterI3D */
    epoxy_wglQueryFrameTrackingI3D_dispatch_table_rewrite_ptr, /* wglQueryFrameTrackingI3D */
    epoxy_wglQueryGenlockMaxSourceDelayI3D_dispatch_table_rewrite_ptr, /* wglQueryGenlockMaxSourceDelayI3D */
    epoxy_wglQueryMaxSwapGroupsNV_dispatch_table_rewrite_ptr, /* wglQueryMaxSwapGroupsNV */
    epoxy_wglQueryPbufferARB_dispatch_table_rewrite_ptr, /* wglQueryPbufferARB */
    epoxy_wglQueryPbufferEXT_dispatch_table_rewrite_ptr, /* wglQueryPbufferEXT */
    epoxy_wglQuerySwapGroupNV_dispatch_table_rewrite_ptr, /* wglQuerySwapGroupNV */
    epoxy_wglQueryVideoCaptureDeviceNV_dispatch_table_rewrite_ptr, /* wglQueryVideoCaptureDeviceNV */
    epoxy_wglRealizeLayerPalette_dispatch_table_rewrite_ptr, /* wglRealizeLayerPalette */
    epoxy_wglReleaseImageBufferEventsI3D_dispatch_table_rewrite_ptr, /* wglReleaseImageBufferEventsI3D */
    epoxy_wglReleasePbufferDCARB_dispatch_table_rewrite_ptr, /* wglReleasePbufferDCARB */
    epoxy_wglReleasePbufferDCEXT_dispatch_table_rewrite_ptr, /* wglReleasePbufferDCEXT */
    epoxy_wglReleaseTexImageARB_dispatch_table_rewrite_ptr, /* wglReleaseTexImageARB */
    epoxy_wglReleaseVideoCaptureDeviceNV_dispatch_table_rewrite_ptr, /* wglReleaseVideoCaptureDeviceNV */
    epoxy_wglReleaseVideoDeviceNV_dispatch_table_rewrite_ptr, /* wglReleaseVideoDeviceNV */
    epoxy_wglReleaseVideoImageNV_dispatch_table_rewrite_ptr, /* wglReleaseVideoImageNV */
    epoxy_wglResetFrameCountNV_dispatch_table_rewrite_ptr, /* wglResetFrameCountNV */
    epoxy_wglRestoreBufferRegionARB_dispatch_table_rewrite_ptr, /* wglRestoreBufferRegionARB */
    epoxy_wglSaveBufferRegionARB_dispatch_table_rewrite_ptr, /* wglSaveBufferRegionARB */
    epoxy_wglSendPbufferToVideoNV_dispatch_table_rewrite_ptr, /* wglSendPbufferToVideoNV */
    epoxy_wglSetDigitalVideoParametersI3D_dispatch_table_rewrite_ptr, /* wglSetDigitalVideoParametersI3D */
    epoxy_wglSetGammaTableI3D_dispatch_table_rewrite_ptr, /* wglSetGammaTableI3D */
    epoxy_wglSetGammaTableParametersI3D_dispatch_table_rewrite_ptr, /* wglSetGammaTableParametersI3D */
    epoxy_wglSetLayerPaletteEntries_dispatch_table_rewrite_ptr, /* wglSetLayerPaletteEntries */
    epoxy_wglSetPbufferAttribARB_dispatch_table_rewrite_ptr, /* wglSetPbufferAttribARB */
    epoxy_wglSetStereoEmitterState3DL_dispatch_table_rewrite_ptr, /* wglSetStereoEmitterState3DL */
    epoxy_wglShareLists_dispatch_table_rewrite_ptr, /* wglShareLists */
    epoxy_wglSwapBuffersMscOML_dispatch_table_rewrite_ptr, /* wglSwapBuffersMscOML */
    epoxy_wglSwapIntervalEXT_dispatch_table_rewrite_ptr, /* wglSwapIntervalEXT */
    epoxy_wglSwapLayerBuffers_dispatch_table_rewrite_ptr, /* wglSwapLayerBuffers */
    epoxy_wglSwapLayerBuffersMscOML_dispatch_table_rewrite_ptr, /* wglSwapLayerBuffersMscOML */
    epoxy_wglUseFontBitmapsA_dispatch_table_rewrite_ptr, /* wglUseFontBitmapsA */
    epoxy_wglUseFontBitmapsW_dispatch_table_rewrite_ptr, /* wglUseFontBitmapsW */
    epoxy_wglUseFontOutlines_dispatch_table_rewrite_ptr, /* wglUseFontOutlines */
    epoxy_wglUseFontOutlinesA_dispatch_table_rewrite_ptr, /* wglUseFontOutlinesA */
    epoxy_wglUseFontOutlinesW_dispatch_table_rewrite_ptr, /* wglUseFontOutlinesW */
    epoxy_wglWaitForMscOML_dispatch_table_rewrite_ptr, /* wglWaitForMscOML */
    epoxy_wglWaitForSbcOML_dispatch_table_rewrite_ptr, /* wglWaitForSbcOML */
};

uint32_t wgl_tls_index;
uint32_t wgl_tls_size = sizeof(struct dispatch_table);

static EPOXY_INLINE struct dispatch_table *
get_dispatch_table(void)
{
	return TlsGetValue(wgl_tls_index);
}

void
wgl_init_dispatch_table(void)
{
    struct dispatch_table *dispatch_table = get_dispatch_table();
    memcpy(dispatch_table, &resolver_table, sizeof(resolver_table));
}

void
wgl_switch_to_dispatch_table(void)
{
    epoxy_wglAllocateMemoryNV = epoxy_wglAllocateMemoryNV_dispatch_table_thunk;
    epoxy_wglAssociateImageBufferEventsI3D = epoxy_wglAssociateImageBufferEventsI3D_dispatch_table_thunk;
    epoxy_wglBeginFrameTrackingI3D = epoxy_wglBeginFrameTrackingI3D_dispatch_table_thunk;
    epoxy_wglBindDisplayColorTableEXT = epoxy_wglBindDisplayColorTableEXT_dispatch_table_thunk;
    epoxy_wglBindSwapBarrierNV = epoxy_wglBindSwapBarrierNV_dispatch_table_thunk;
    epoxy_wglBindTexImageARB = epoxy_wglBindTexImageARB_dispatch_table_thunk;
    epoxy_wglBindVideoCaptureDeviceNV = epoxy_wglBindVideoCaptureDeviceNV_dispatch_table_thunk;
    epoxy_wglBindVideoDeviceNV = epoxy_wglBindVideoDeviceNV_dispatch_table_thunk;
    epoxy_wglBindVideoImageNV = epoxy_wglBindVideoImageNV_dispatch_table_thunk;
    epoxy_wglBlitContextFramebufferAMD = epoxy_wglBlitContextFramebufferAMD_dispatch_table_thunk;
    epoxy_wglChoosePixelFormatARB = epoxy_wglChoosePixelFormatARB_dispatch_table_thunk;
    epoxy_wglChoosePixelFormatEXT = epoxy_wglChoosePixelFormatEXT_dispatch_table_thunk;
    epoxy_wglCopyContext = epoxy_wglCopyContext_dispatch_table_thunk;
    epoxy_wglCopyImageSubDataNV = epoxy_wglCopyImageSubDataNV_dispatch_table_thunk;
    epoxy_wglCreateAffinityDCNV = epoxy_wglCreateAffinityDCNV_dispatch_table_thunk;
    epoxy_wglCreateAssociatedContextAMD = epoxy_wglCreateAssociatedContextAMD_dispatch_table_thunk;
    epoxy_wglCreateAssociatedContextAttribsAMD = epoxy_wglCreateAssociatedContextAttribsAMD_dispatch_table_thunk;
    epoxy_wglCreateBufferRegionARB = epoxy_wglCreateBufferRegionARB_dispatch_table_thunk;
    epoxy_wglCreateContext = epoxy_wglCreateContext_dispatch_table_thunk;
    epoxy_wglCreateContextAttribsARB = epoxy_wglCreateContextAttribsARB_dispatch_table_thunk;
    epoxy_wglCreateDisplayColorTableEXT = epoxy_wglCreateDisplayColorTableEXT_dispatch_table_thunk;
    epoxy_wglCreateImageBufferI3D = epoxy_wglCreateImageBufferI3D_dispatch_table_thunk;
    epoxy_wglCreateLayerContext = epoxy_wglCreateLayerContext_dispatch_table_thunk;
    epoxy_wglCreatePbufferARB = epoxy_wglCreatePbufferARB_dispatch_table_thunk;
    epoxy_wglCreatePbufferEXT = epoxy_wglCreatePbufferEXT_dispatch_table_thunk;
    epoxy_wglDXCloseDeviceNV = epoxy_wglDXCloseDeviceNV_dispatch_table_thunk;
    epoxy_wglDXLockObjectsNV = epoxy_wglDXLockObjectsNV_dispatch_table_thunk;
    epoxy_wglDXObjectAccessNV = epoxy_wglDXObjectAccessNV_dispatch_table_thunk;
    epoxy_wglDXOpenDeviceNV = epoxy_wglDXOpenDeviceNV_dispatch_table_thunk;
    epoxy_wglDXRegisterObjectNV = epoxy_wglDXRegisterObjectNV_dispatch_table_thunk;
    epoxy_wglDXSetResourceShareHandleNV = epoxy_wglDXSetResourceShareHandleNV_dispatch_table_thunk;
    epoxy_wglDXUnlockObjectsNV = epoxy_wglDXUnlockObjectsNV_dispatch_table_thunk;
    epoxy_wglDXUnregisterObjectNV = epoxy_wglDXUnregisterObjectNV_dispatch_table_thunk;
    epoxy_wglDelayBeforeSwapNV = epoxy_wglDelayBeforeSwapNV_dispatch_table_thunk;
    epoxy_wglDeleteAssociatedContextAMD = epoxy_wglDeleteAssociatedContextAMD_dispatch_table_thunk;
    epoxy_wglDeleteBufferRegionARB = epoxy_wglDeleteBufferRegionARB_dispatch_table_thunk;
    epoxy_wglDeleteContext = epoxy_wglDeleteContext_dispatch_table_thunk;
    epoxy_wglDeleteDCNV = epoxy_wglDeleteDCNV_dispatch_table_thunk;
    epoxy_wglDescribeLayerPlane = epoxy_wglDescribeLayerPlane_dispatch_table_thunk;
    epoxy_wglDestroyDisplayColorTableEXT = epoxy_wglDestroyDisplayColorTableEXT_dispatch_table_thunk;
    epoxy_wglDestroyImageBufferI3D = epoxy_wglDestroyImageBufferI3D_dispatch_table_thunk;
    epoxy_wglDestroyPbufferARB = epoxy_wglDestroyPbufferARB_dispatch_table_thunk;
    epoxy_wglDestroyPbufferEXT = epoxy_wglDestroyPbufferEXT_dispatch_table_thunk;
    epoxy_wglDisableFrameLockI3D = epoxy_wglDisableFrameLockI3D_dispatch_table_thunk;
    epoxy_wglDisableGenlockI3D = epoxy_wglDisableGenlockI3D_dispatch_table_thunk;
    epoxy_wglEnableFrameLockI3D = epoxy_wglEnableFrameLockI3D_dispatch_table_thunk;
    epoxy_wglEnableGenlockI3D = epoxy_wglEnableGenlockI3D_dispatch_table_thunk;
    epoxy_wglEndFrameTrackingI3D = epoxy_wglEndFrameTrackingI3D_dispatch_table_thunk;
    epoxy_wglEnumGpuDevicesNV = epoxy_wglEnumGpuDevicesNV_dispatch_table_thunk;
    epoxy_wglEnumGpusFromAffinityDCNV = epoxy_wglEnumGpusFromAffinityDCNV_dispatch_table_thunk;
    epoxy_wglEnumGpusNV = epoxy_wglEnumGpusNV_dispatch_table_thunk;
    epoxy_wglEnumerateVideoCaptureDevicesNV = epoxy_wglEnumerateVideoCaptureDevicesNV_dispatch_table_thunk;
    epoxy_wglEnumerateVideoDevicesNV = epoxy_wglEnumerateVideoDevicesNV_dispatch_table_thunk;
    epoxy_wglFreeMemoryNV = epoxy_wglFreeMemoryNV_dispatch_table_thunk;
    epoxy_wglGenlockSampleRateI3D = epoxy_wglGenlockSampleRateI3D_dispatch_table_thunk;
    epoxy_wglGenlockSourceDelayI3D = epoxy_wglGenlockSourceDelayI3D_dispatch_table_thunk;
    epoxy_wglGenlockSourceEdgeI3D = epoxy_wglGenlockSourceEdgeI3D_dispatch_table_thunk;
    epoxy_wglGenlockSourceI3D = epoxy_wglGenlockSourceI3D_dispatch_table_thunk;
    epoxy_wglGetContextGPUIDAMD = epoxy_wglGetContextGPUIDAMD_dispatch_table_thunk;
    epoxy_wglGetCurrentAssociatedContextAMD = epoxy_wglGetCurrentAssociatedContextAMD_dispatch_table_thunk;
    epoxy_wglGetCurrentContext = epoxy_wglGetCurrentContext_dispatch_table_thunk;
    epoxy_wglGetCurrentDC = epoxy_wglGetCurrentDC_dispatch_table_thunk;
    epoxy_wglGetCurrentReadDCARB = epoxy_wglGetCurrentReadDCARB_dispatch_table_thunk;
    epoxy_wglGetCurrentReadDCEXT = epoxy_wglGetCurrentReadDCEXT_dispatch_table_thunk;
    epoxy_wglGetDefaultProcAddress = epoxy_wglGetDefaultProcAddress_dispatch_table_thunk;
    epoxy_wglGetDigitalVideoParametersI3D = epoxy_wglGetDigitalVideoParametersI3D_dispatch_table_thunk;
    epoxy_wglGetExtensionsStringARB = epoxy_wglGetExtensionsStringARB_dispatch_table_thunk;
    epoxy_wglGetExtensionsStringEXT = epoxy_wglGetExtensionsStringEXT_dispatch_table_thunk;
    epoxy_wglGetFrameUsageI3D = epoxy_wglGetFrameUsageI3D_dispatch_table_thunk;
    epoxy_wglGetGPUIDsAMD = epoxy_wglGetGPUIDsAMD_dispatch_table_thunk;
    epoxy_wglGetGPUInfoAMD = epoxy_wglGetGPUInfoAMD_dispatch_table_thunk;
    epoxy_wglGetGammaTableI3D = epoxy_wglGetGammaTableI3D_dispatch_table_thunk;
    epoxy_wglGetGammaTableParametersI3D = epoxy_wglGetGammaTableParametersI3D_dispatch_table_thunk;
    epoxy_wglGetGenlockSampleRateI3D = epoxy_wglGetGenlockSampleRateI3D_dispatch_table_thunk;
    epoxy_wglGetGenlockSourceDelayI3D = epoxy_wglGetGenlockSourceDelayI3D_dispatch_table_thunk;
    epoxy_wglGetGenlockSourceEdgeI3D = epoxy_wglGetGenlockSourceEdgeI3D_dispatch_table_thunk;
    epoxy_wglGetGenlockSourceI3D = epoxy_wglGetGenlockSourceI3D_dispatch_table_thunk;
    epoxy_wglGetLayerPaletteEntries = epoxy_wglGetLayerPaletteEntries_dispatch_table_thunk;
    epoxy_wglGetMscRateOML = epoxy_wglGetMscRateOML_dispatch_table_thunk;
    epoxy_wglGetPbufferDCARB = epoxy_wglGetPbufferDCARB_dispatch_table_thunk;
    epoxy_wglGetPbufferDCEXT = epoxy_wglGetPbufferDCEXT_dispatch_table_thunk;
    epoxy_wglGetPixelFormatAttribfvARB = epoxy_wglGetPixelFormatAttribfvARB_dispatch_table_thunk;
    epoxy_wglGetPixelFormatAttribfvEXT = epoxy_wglGetPixelFormatAttribfvEXT_dispatch_table_thunk;
    epoxy_wglGetPixelFormatAttribivARB = epoxy_wglGetPixelFormatAttribivARB_dispatch_table_thunk;
    epoxy_wglGetPixelFormatAttribivEXT = epoxy_wglGetPixelFormatAttribivEXT_dispatch_table_thunk;
    epoxy_wglGetProcAddress = epoxy_wglGetProcAddress_dispatch_table_thunk;
    epoxy_wglGetSwapIntervalEXT = epoxy_wglGetSwapIntervalEXT_dispatch_table_thunk;
    epoxy_wglGetSyncValuesOML = epoxy_wglGetSyncValuesOML_dispatch_table_thunk;
    epoxy_wglGetVideoDeviceNV = epoxy_wglGetVideoDeviceNV_dispatch_table_thunk;
    epoxy_wglGetVideoInfoNV = epoxy_wglGetVideoInfoNV_dispatch_table_thunk;
    epoxy_wglIsEnabledFrameLockI3D = epoxy_wglIsEnabledFrameLockI3D_dispatch_table_thunk;
    epoxy_wglIsEnabledGenlockI3D = epoxy_wglIsEnabledGenlockI3D_dispatch_table_thunk;
    epoxy_wglJoinSwapGroupNV = epoxy_wglJoinSwapGroupNV_dispatch_table_thunk;
    epoxy_wglLoadDisplayColorTableEXT = epoxy_wglLoadDisplayColorTableEXT_dispatch_table_thunk;
    epoxy_wglLockVideoCaptureDeviceNV = epoxy_wglLockVideoCaptureDeviceNV_dispatch_table_thunk;
    epoxy_wglMakeAssociatedContextCurrentAMD_unwrapped = epoxy_wglMakeAssociatedContextCurrentAMD_unwrapped_dispatch_table_thunk;
    epoxy_wglMakeContextCurrentARB_unwrapped = epoxy_wglMakeContextCurrentARB_unwrapped_dispatch_table_thunk;
    epoxy_wglMakeContextCurrentEXT_unwrapped = epoxy_wglMakeContextCurrentEXT_unwrapped_dispatch_table_thunk;
    epoxy_wglMakeCurrent_unwrapped = epoxy_wglMakeCurrent_unwrapped_dispatch_table_thunk;
    epoxy_wglQueryCurrentContextNV = epoxy_wglQueryCurrentContextNV_dispatch_table_thunk;
    epoxy_wglQueryFrameCountNV = epoxy_wglQueryFrameCountNV_dispatch_table_thunk;
    epoxy_wglQueryFrameLockMasterI3D = epoxy_wglQueryFrameLockMasterI3D_dispatch_table_thunk;
    epoxy_wglQueryFrameTrackingI3D = epoxy_wglQueryFrameTrackingI3D_dispatch_table_thunk;
    epoxy_wglQueryGenlockMaxSourceDelayI3D = epoxy_wglQueryGenlockMaxSourceDelayI3D_dispatch_table_thunk;
    epoxy_wglQueryMaxSwapGroupsNV = epoxy_wglQueryMaxSwapGroupsNV_dispatch_table_thunk;
    epoxy_wglQueryPbufferARB = epoxy_wglQueryPbufferARB_dispatch_table_thunk;
    epoxy_wglQueryPbufferEXT = epoxy_wglQueryPbufferEXT_dispatch_table_thunk;
    epoxy_wglQuerySwapGroupNV = epoxy_wglQuerySwapGroupNV_dispatch_table_thunk;
    epoxy_wglQueryVideoCaptureDeviceNV = epoxy_wglQueryVideoCaptureDeviceNV_dispatch_table_thunk;
    epoxy_wglRealizeLayerPalette = epoxy_wglRealizeLayerPalette_dispatch_table_thunk;
    epoxy_wglReleaseImageBufferEventsI3D = epoxy_wglReleaseImageBufferEventsI3D_dispatch_table_thunk;
    epoxy_wglReleasePbufferDCARB = epoxy_wglReleasePbufferDCARB_dispatch_table_thunk;
    epoxy_wglReleasePbufferDCEXT = epoxy_wglReleasePbufferDCEXT_dispatch_table_thunk;
    epoxy_wglReleaseTexImageARB = epoxy_wglReleaseTexImageARB_dispatch_table_thunk;
    epoxy_wglReleaseVideoCaptureDeviceNV = epoxy_wglReleaseVideoCaptureDeviceNV_dispatch_table_thunk;
    epoxy_wglReleaseVideoDeviceNV = epoxy_wglReleaseVideoDeviceNV_dispatch_table_thunk;
    epoxy_wglReleaseVideoImageNV = epoxy_wglReleaseVideoImageNV_dispatch_table_thunk;
    epoxy_wglResetFrameCountNV = epoxy_wglResetFrameCountNV_dispatch_table_thunk;
    epoxy_wglRestoreBufferRegionARB = epoxy_wglRestoreBufferRegionARB_dispatch_table_thunk;
    epoxy_wglSaveBufferRegionARB = epoxy_wglSaveBufferRegionARB_dispatch_table_thunk;
    epoxy_wglSendPbufferToVideoNV = epoxy_wglSendPbufferToVideoNV_dispatch_table_thunk;
    epoxy_wglSetDigitalVideoParametersI3D = epoxy_wglSetDigitalVideoParametersI3D_dispatch_table_thunk;
    epoxy_wglSetGammaTableI3D = epoxy_wglSetGammaTableI3D_dispatch_table_thunk;
    epoxy_wglSetGammaTableParametersI3D = epoxy_wglSetGammaTableParametersI3D_dispatch_table_thunk;
    epoxy_wglSetLayerPaletteEntries = epoxy_wglSetLayerPaletteEntries_dispatch_table_thunk;
    epoxy_wglSetPbufferAttribARB = epoxy_wglSetPbufferAttribARB_dispatch_table_thunk;
    epoxy_wglSetStereoEmitterState3DL = epoxy_wglSetStereoEmitterState3DL_dispatch_table_thunk;
    epoxy_wglShareLists = epoxy_wglShareLists_dispatch_table_thunk;
    epoxy_wglSwapBuffersMscOML = epoxy_wglSwapBuffersMscOML_dispatch_table_thunk;
    epoxy_wglSwapIntervalEXT = epoxy_wglSwapIntervalEXT_dispatch_table_thunk;
    epoxy_wglSwapLayerBuffers = epoxy_wglSwapLayerBuffers_dispatch_table_thunk;
    epoxy_wglSwapLayerBuffersMscOML = epoxy_wglSwapLayerBuffersMscOML_dispatch_table_thunk;
    epoxy_wglUseFontBitmapsA = epoxy_wglUseFontBitmapsA_dispatch_table_thunk;
    epoxy_wglUseFontBitmapsW = epoxy_wglUseFontBitmapsW_dispatch_table_thunk;
    epoxy_wglUseFontOutlines = epoxy_wglUseFontOutlines_dispatch_table_thunk;
    epoxy_wglUseFontOutlinesA = epoxy_wglUseFontOutlinesA_dispatch_table_thunk;
    epoxy_wglUseFontOutlinesW = epoxy_wglUseFontOutlinesW_dispatch_table_thunk;
    epoxy_wglWaitForMscOML = epoxy_wglWaitForMscOML_dispatch_table_thunk;
    epoxy_wglWaitForSbcOML = epoxy_wglWaitForSbcOML_dispatch_table_thunk;
}

#endif /* !USING_DISPATCH_TABLE */
PUBLIC PFNWGLALLOCATEMEMORYNVPROC epoxy_wglAllocateMemoryNV = epoxy_wglAllocateMemoryNV_global_rewrite_ptr;

PUBLIC PFNWGLASSOCIATEIMAGEBUFFEREVENTSI3DPROC epoxy_wglAssociateImageBufferEventsI3D = epoxy_wglAssociateImageBufferEventsI3D_global_rewrite_ptr;

PUBLIC PFNWGLBEGINFRAMETRACKINGI3DPROC epoxy_wglBeginFrameTrackingI3D = epoxy_wglBeginFrameTrackingI3D_global_rewrite_ptr;

PUBLIC PFNWGLBINDDISPLAYCOLORTABLEEXTPROC epoxy_wglBindDisplayColorTableEXT = epoxy_wglBindDisplayColorTableEXT_global_rewrite_ptr;

PUBLIC PFNWGLBINDSWAPBARRIERNVPROC epoxy_wglBindSwapBarrierNV = epoxy_wglBindSwapBarrierNV_global_rewrite_ptr;

PUBLIC PFNWGLBINDTEXIMAGEARBPROC epoxy_wglBindTexImageARB = epoxy_wglBindTexImageARB_global_rewrite_ptr;

PUBLIC PFNWGLBINDVIDEOCAPTUREDEVICENVPROC epoxy_wglBindVideoCaptureDeviceNV = epoxy_wglBindVideoCaptureDeviceNV_global_rewrite_ptr;

PUBLIC PFNWGLBINDVIDEODEVICENVPROC epoxy_wglBindVideoDeviceNV = epoxy_wglBindVideoDeviceNV_global_rewrite_ptr;

PUBLIC PFNWGLBINDVIDEOIMAGENVPROC epoxy_wglBindVideoImageNV = epoxy_wglBindVideoImageNV_global_rewrite_ptr;

PUBLIC PFNWGLBLITCONTEXTFRAMEBUFFERAMDPROC epoxy_wglBlitContextFramebufferAMD = epoxy_wglBlitContextFramebufferAMD_global_rewrite_ptr;

PUBLIC PFNWGLCHOOSEPIXELFORMATARBPROC epoxy_wglChoosePixelFormatARB = epoxy_wglChoosePixelFormatARB_global_rewrite_ptr;

PUBLIC PFNWGLCHOOSEPIXELFORMATEXTPROC epoxy_wglChoosePixelFormatEXT = epoxy_wglChoosePixelFormatEXT_global_rewrite_ptr;

PUBLIC PFNWGLCOPYCONTEXTPROC epoxy_wglCopyContext = epoxy_wglCopyContext_global_rewrite_ptr;

PUBLIC PFNWGLCOPYIMAGESUBDATANVPROC epoxy_wglCopyImageSubDataNV = epoxy_wglCopyImageSubDataNV_global_rewrite_ptr;

PUBLIC PFNWGLCREATEAFFINITYDCNVPROC epoxy_wglCreateAffinityDCNV = epoxy_wglCreateAffinityDCNV_global_rewrite_ptr;

PUBLIC PFNWGLCREATEASSOCIATEDCONTEXTAMDPROC epoxy_wglCreateAssociatedContextAMD = epoxy_wglCreateAssociatedContextAMD_global_rewrite_ptr;

PUBLIC PFNWGLCREATEASSOCIATEDCONTEXTATTRIBSAMDPROC epoxy_wglCreateAssociatedContextAttribsAMD = epoxy_wglCreateAssociatedContextAttribsAMD_global_rewrite_ptr;

PUBLIC PFNWGLCREATEBUFFERREGIONARBPROC epoxy_wglCreateBufferRegionARB = epoxy_wglCreateBufferRegionARB_global_rewrite_ptr;

PUBLIC PFNWGLCREATECONTEXTPROC epoxy_wglCreateContext = epoxy_wglCreateContext_global_rewrite_ptr;

PUBLIC PFNWGLCREATECONTEXTATTRIBSARBPROC epoxy_wglCreateContextAttribsARB = epoxy_wglCreateContextAttribsARB_global_rewrite_ptr;

PUBLIC PFNWGLCREATEDISPLAYCOLORTABLEEXTPROC epoxy_wglCreateDisplayColorTableEXT = epoxy_wglCreateDisplayColorTableEXT_global_rewrite_ptr;

PUBLIC PFNWGLCREATEIMAGEBUFFERI3DPROC epoxy_wglCreateImageBufferI3D = epoxy_wglCreateImageBufferI3D_global_rewrite_ptr;

PUBLIC PFNWGLCREATELAYERCONTEXTPROC epoxy_wglCreateLayerContext = epoxy_wglCreateLayerContext_global_rewrite_ptr;

PUBLIC PFNWGLCREATEPBUFFERARBPROC epoxy_wglCreatePbufferARB = epoxy_wglCreatePbufferARB_global_rewrite_ptr;

PUBLIC PFNWGLCREATEPBUFFEREXTPROC epoxy_wglCreatePbufferEXT = epoxy_wglCreatePbufferEXT_global_rewrite_ptr;

PUBLIC PFNWGLDXCLOSEDEVICENVPROC epoxy_wglDXCloseDeviceNV = epoxy_wglDXCloseDeviceNV_global_rewrite_ptr;

PUBLIC PFNWGLDXLOCKOBJECTSNVPROC epoxy_wglDXLockObjectsNV = epoxy_wglDXLockObjectsNV_global_rewrite_ptr;

PUBLIC PFNWGLDXOBJECTACCESSNVPROC epoxy_wglDXObjectAccessNV = epoxy_wglDXObjectAccessNV_global_rewrite_ptr;

PUBLIC PFNWGLDXOPENDEVICENVPROC epoxy_wglDXOpenDeviceNV = epoxy_wglDXOpenDeviceNV_global_rewrite_ptr;

PUBLIC PFNWGLDXREGISTEROBJECTNVPROC epoxy_wglDXRegisterObjectNV = epoxy_wglDXRegisterObjectNV_global_rewrite_ptr;

PUBLIC PFNWGLDXSETRESOURCESHAREHANDLENVPROC epoxy_wglDXSetResourceShareHandleNV = epoxy_wglDXSetResourceShareHandleNV_global_rewrite_ptr;

PUBLIC PFNWGLDXUNLOCKOBJECTSNVPROC epoxy_wglDXUnlockObjectsNV = epoxy_wglDXUnlockObjectsNV_global_rewrite_ptr;

PUBLIC PFNWGLDXUNREGISTEROBJECTNVPROC epoxy_wglDXUnregisterObjectNV = epoxy_wglDXUnregisterObjectNV_global_rewrite_ptr;

PUBLIC PFNWGLDELAYBEFORESWAPNVPROC epoxy_wglDelayBeforeSwapNV = epoxy_wglDelayBeforeSwapNV_global_rewrite_ptr;

PUBLIC PFNWGLDELETEASSOCIATEDCONTEXTAMDPROC epoxy_wglDeleteAssociatedContextAMD = epoxy_wglDeleteAssociatedContextAMD_global_rewrite_ptr;

PUBLIC PFNWGLDELETEBUFFERREGIONARBPROC epoxy_wglDeleteBufferRegionARB = epoxy_wglDeleteBufferRegionARB_global_rewrite_ptr;

PUBLIC PFNWGLDELETECONTEXTPROC epoxy_wglDeleteContext = epoxy_wglDeleteContext_global_rewrite_ptr;

PUBLIC PFNWGLDELETEDCNVPROC epoxy_wglDeleteDCNV = epoxy_wglDeleteDCNV_global_rewrite_ptr;

PUBLIC PFNWGLDESCRIBELAYERPLANEPROC epoxy_wglDescribeLayerPlane = epoxy_wglDescribeLayerPlane_global_rewrite_ptr;

PUBLIC PFNWGLDESTROYDISPLAYCOLORTABLEEXTPROC epoxy_wglDestroyDisplayColorTableEXT = epoxy_wglDestroyDisplayColorTableEXT_global_rewrite_ptr;

PUBLIC PFNWGLDESTROYIMAGEBUFFERI3DPROC epoxy_wglDestroyImageBufferI3D = epoxy_wglDestroyImageBufferI3D_global_rewrite_ptr;

PUBLIC PFNWGLDESTROYPBUFFERARBPROC epoxy_wglDestroyPbufferARB = epoxy_wglDestroyPbufferARB_global_rewrite_ptr;

PUBLIC PFNWGLDESTROYPBUFFEREXTPROC epoxy_wglDestroyPbufferEXT = epoxy_wglDestroyPbufferEXT_global_rewrite_ptr;

PUBLIC PFNWGLDISABLEFRAMELOCKI3DPROC epoxy_wglDisableFrameLockI3D = epoxy_wglDisableFrameLockI3D_global_rewrite_ptr;

PUBLIC PFNWGLDISABLEGENLOCKI3DPROC epoxy_wglDisableGenlockI3D = epoxy_wglDisableGenlockI3D_global_rewrite_ptr;

PUBLIC PFNWGLENABLEFRAMELOCKI3DPROC epoxy_wglEnableFrameLockI3D = epoxy_wglEnableFrameLockI3D_global_rewrite_ptr;

PUBLIC PFNWGLENABLEGENLOCKI3DPROC epoxy_wglEnableGenlockI3D = epoxy_wglEnableGenlockI3D_global_rewrite_ptr;

PUBLIC PFNWGLENDFRAMETRACKINGI3DPROC epoxy_wglEndFrameTrackingI3D = epoxy_wglEndFrameTrackingI3D_global_rewrite_ptr;

PUBLIC PFNWGLENUMGPUDEVICESNVPROC epoxy_wglEnumGpuDevicesNV = epoxy_wglEnumGpuDevicesNV_global_rewrite_ptr;

PUBLIC PFNWGLENUMGPUSFROMAFFINITYDCNVPROC epoxy_wglEnumGpusFromAffinityDCNV = epoxy_wglEnumGpusFromAffinityDCNV_global_rewrite_ptr;

PUBLIC PFNWGLENUMGPUSNVPROC epoxy_wglEnumGpusNV = epoxy_wglEnumGpusNV_global_rewrite_ptr;

PUBLIC PFNWGLENUMERATEVIDEOCAPTUREDEVICESNVPROC epoxy_wglEnumerateVideoCaptureDevicesNV = epoxy_wglEnumerateVideoCaptureDevicesNV_global_rewrite_ptr;

PUBLIC PFNWGLENUMERATEVIDEODEVICESNVPROC epoxy_wglEnumerateVideoDevicesNV = epoxy_wglEnumerateVideoDevicesNV_global_rewrite_ptr;

PUBLIC PFNWGLFREEMEMORYNVPROC epoxy_wglFreeMemoryNV = epoxy_wglFreeMemoryNV_global_rewrite_ptr;

PUBLIC PFNWGLGENLOCKSAMPLERATEI3DPROC epoxy_wglGenlockSampleRateI3D = epoxy_wglGenlockSampleRateI3D_global_rewrite_ptr;

PUBLIC PFNWGLGENLOCKSOURCEDELAYI3DPROC epoxy_wglGenlockSourceDelayI3D = epoxy_wglGenlockSourceDelayI3D_global_rewrite_ptr;

PUBLIC PFNWGLGENLOCKSOURCEEDGEI3DPROC epoxy_wglGenlockSourceEdgeI3D = epoxy_wglGenlockSourceEdgeI3D_global_rewrite_ptr;

PUBLIC PFNWGLGENLOCKSOURCEI3DPROC epoxy_wglGenlockSourceI3D = epoxy_wglGenlockSourceI3D_global_rewrite_ptr;

PUBLIC PFNWGLGETCONTEXTGPUIDAMDPROC epoxy_wglGetContextGPUIDAMD = epoxy_wglGetContextGPUIDAMD_global_rewrite_ptr;

PUBLIC PFNWGLGETCURRENTASSOCIATEDCONTEXTAMDPROC epoxy_wglGetCurrentAssociatedContextAMD = epoxy_wglGetCurrentAssociatedContextAMD_global_rewrite_ptr;

PUBLIC PFNWGLGETCURRENTCONTEXTPROC epoxy_wglGetCurrentContext = epoxy_wglGetCurrentContext_global_rewrite_ptr;

PUBLIC PFNWGLGETCURRENTDCPROC epoxy_wglGetCurrentDC = epoxy_wglGetCurrentDC_global_rewrite_ptr;

PUBLIC PFNWGLGETCURRENTREADDCARBPROC epoxy_wglGetCurrentReadDCARB = epoxy_wglGetCurrentReadDCARB_global_rewrite_ptr;

PUBLIC PFNWGLGETCURRENTREADDCEXTPROC epoxy_wglGetCurrentReadDCEXT = epoxy_wglGetCurrentReadDCEXT_global_rewrite_ptr;

PUBLIC PFNWGLGETDEFAULTPROCADDRESSPROC epoxy_wglGetDefaultProcAddress = epoxy_wglGetDefaultProcAddress_global_rewrite_ptr;

PUBLIC PFNWGLGETDIGITALVIDEOPARAMETERSI3DPROC epoxy_wglGetDigitalVideoParametersI3D = epoxy_wglGetDigitalVideoParametersI3D_global_rewrite_ptr;

PUBLIC PFNWGLGETEXTENSIONSSTRINGARBPROC epoxy_wglGetExtensionsStringARB = epoxy_wglGetExtensionsStringARB_global_rewrite_ptr;

PUBLIC PFNWGLGETEXTENSIONSSTRINGEXTPROC epoxy_wglGetExtensionsStringEXT = epoxy_wglGetExtensionsStringEXT_global_rewrite_ptr;

PUBLIC PFNWGLGETFRAMEUSAGEI3DPROC epoxy_wglGetFrameUsageI3D = epoxy_wglGetFrameUsageI3D_global_rewrite_ptr;

PUBLIC PFNWGLGETGPUIDSAMDPROC epoxy_wglGetGPUIDsAMD = epoxy_wglGetGPUIDsAMD_global_rewrite_ptr;

PUBLIC PFNWGLGETGPUINFOAMDPROC epoxy_wglGetGPUInfoAMD = epoxy_wglGetGPUInfoAMD_global_rewrite_ptr;

PUBLIC PFNWGLGETGAMMATABLEI3DPROC epoxy_wglGetGammaTableI3D = epoxy_wglGetGammaTableI3D_global_rewrite_ptr;

PUBLIC PFNWGLGETGAMMATABLEPARAMETERSI3DPROC epoxy_wglGetGammaTableParametersI3D = epoxy_wglGetGammaTableParametersI3D_global_rewrite_ptr;

PUBLIC PFNWGLGETGENLOCKSAMPLERATEI3DPROC epoxy_wglGetGenlockSampleRateI3D = epoxy_wglGetGenlockSampleRateI3D_global_rewrite_ptr;

PUBLIC PFNWGLGETGENLOCKSOURCEDELAYI3DPROC epoxy_wglGetGenlockSourceDelayI3D = epoxy_wglGetGenlockSourceDelayI3D_global_rewrite_ptr;

PUBLIC PFNWGLGETGENLOCKSOURCEEDGEI3DPROC epoxy_wglGetGenlockSourceEdgeI3D = epoxy_wglGetGenlockSourceEdgeI3D_global_rewrite_ptr;

PUBLIC PFNWGLGETGENLOCKSOURCEI3DPROC epoxy_wglGetGenlockSourceI3D = epoxy_wglGetGenlockSourceI3D_global_rewrite_ptr;

PUBLIC PFNWGLGETLAYERPALETTEENTRIESPROC epoxy_wglGetLayerPaletteEntries = epoxy_wglGetLayerPaletteEntries_global_rewrite_ptr;

PUBLIC PFNWGLGETMSCRATEOMLPROC epoxy_wglGetMscRateOML = epoxy_wglGetMscRateOML_global_rewrite_ptr;

PUBLIC PFNWGLGETPBUFFERDCARBPROC epoxy_wglGetPbufferDCARB = epoxy_wglGetPbufferDCARB_global_rewrite_ptr;

PUBLIC PFNWGLGETPBUFFERDCEXTPROC epoxy_wglGetPbufferDCEXT = epoxy_wglGetPbufferDCEXT_global_rewrite_ptr;

PUBLIC PFNWGLGETPIXELFORMATATTRIBFVARBPROC epoxy_wglGetPixelFormatAttribfvARB = epoxy_wglGetPixelFormatAttribfvARB_global_rewrite_ptr;

PUBLIC PFNWGLGETPIXELFORMATATTRIBFVEXTPROC epoxy_wglGetPixelFormatAttribfvEXT = epoxy_wglGetPixelFormatAttribfvEXT_global_rewrite_ptr;

PUBLIC PFNWGLGETPIXELFORMATATTRIBIVARBPROC epoxy_wglGetPixelFormatAttribivARB = epoxy_wglGetPixelFormatAttribivARB_global_rewrite_ptr;

PUBLIC PFNWGLGETPIXELFORMATATTRIBIVEXTPROC epoxy_wglGetPixelFormatAttribivEXT = epoxy_wglGetPixelFormatAttribivEXT_global_rewrite_ptr;

PUBLIC PFNWGLGETPROCADDRESSPROC epoxy_wglGetProcAddress = epoxy_wglGetProcAddress_global_rewrite_ptr;

PUBLIC PFNWGLGETSWAPINTERVALEXTPROC epoxy_wglGetSwapIntervalEXT = epoxy_wglGetSwapIntervalEXT_global_rewrite_ptr;

PUBLIC PFNWGLGETSYNCVALUESOMLPROC epoxy_wglGetSyncValuesOML = epoxy_wglGetSyncValuesOML_global_rewrite_ptr;

PUBLIC PFNWGLGETVIDEODEVICENVPROC epoxy_wglGetVideoDeviceNV = epoxy_wglGetVideoDeviceNV_global_rewrite_ptr;

PUBLIC PFNWGLGETVIDEOINFONVPROC epoxy_wglGetVideoInfoNV = epoxy_wglGetVideoInfoNV_global_rewrite_ptr;

PUBLIC PFNWGLISENABLEDFRAMELOCKI3DPROC epoxy_wglIsEnabledFrameLockI3D = epoxy_wglIsEnabledFrameLockI3D_global_rewrite_ptr;

PUBLIC PFNWGLISENABLEDGENLOCKI3DPROC epoxy_wglIsEnabledGenlockI3D = epoxy_wglIsEnabledGenlockI3D_global_rewrite_ptr;

PUBLIC PFNWGLJOINSWAPGROUPNVPROC epoxy_wglJoinSwapGroupNV = epoxy_wglJoinSwapGroupNV_global_rewrite_ptr;

PUBLIC PFNWGLLOADDISPLAYCOLORTABLEEXTPROC epoxy_wglLoadDisplayColorTableEXT = epoxy_wglLoadDisplayColorTableEXT_global_rewrite_ptr;

PUBLIC PFNWGLLOCKVIDEOCAPTUREDEVICENVPROC epoxy_wglLockVideoCaptureDeviceNV = epoxy_wglLockVideoCaptureDeviceNV_global_rewrite_ptr;

PFNWGLMAKEASSOCIATEDCONTEXTCURRENTAMDPROC epoxy_wglMakeAssociatedContextCurrentAMD_unwrapped = epoxy_wglMakeAssociatedContextCurrentAMD_unwrapped_global_rewrite_ptr;

PFNWGLMAKECONTEXTCURRENTARBPROC epoxy_wglMakeContextCurrentARB_unwrapped = epoxy_wglMakeContextCurrentARB_unwrapped_global_rewrite_ptr;

PFNWGLMAKECONTEXTCURRENTEXTPROC epoxy_wglMakeContextCurrentEXT_unwrapped = epoxy_wglMakeContextCurrentEXT_unwrapped_global_rewrite_ptr;

PFNWGLMAKECURRENTPROC epoxy_wglMakeCurrent_unwrapped = epoxy_wglMakeCurrent_unwrapped_global_rewrite_ptr;

PUBLIC PFNWGLQUERYCURRENTCONTEXTNVPROC epoxy_wglQueryCurrentContextNV = epoxy_wglQueryCurrentContextNV_global_rewrite_ptr;

PUBLIC PFNWGLQUERYFRAMECOUNTNVPROC epoxy_wglQueryFrameCountNV = epoxy_wglQueryFrameCountNV_global_rewrite_ptr;

PUBLIC PFNWGLQUERYFRAMELOCKMASTERI3DPROC epoxy_wglQueryFrameLockMasterI3D = epoxy_wglQueryFrameLockMasterI3D_global_rewrite_ptr;

PUBLIC PFNWGLQUERYFRAMETRACKINGI3DPROC epoxy_wglQueryFrameTrackingI3D = epoxy_wglQueryFrameTrackingI3D_global_rewrite_ptr;

PUBLIC PFNWGLQUERYGENLOCKMAXSOURCEDELAYI3DPROC epoxy_wglQueryGenlockMaxSourceDelayI3D = epoxy_wglQueryGenlockMaxSourceDelayI3D_global_rewrite_ptr;

PUBLIC PFNWGLQUERYMAXSWAPGROUPSNVPROC epoxy_wglQueryMaxSwapGroupsNV = epoxy_wglQueryMaxSwapGroupsNV_global_rewrite_ptr;

PUBLIC PFNWGLQUERYPBUFFERARBPROC epoxy_wglQueryPbufferARB = epoxy_wglQueryPbufferARB_global_rewrite_ptr;

PUBLIC PFNWGLQUERYPBUFFEREXTPROC epoxy_wglQueryPbufferEXT = epoxy_wglQueryPbufferEXT_global_rewrite_ptr;

PUBLIC PFNWGLQUERYSWAPGROUPNVPROC epoxy_wglQuerySwapGroupNV = epoxy_wglQuerySwapGroupNV_global_rewrite_ptr;

PUBLIC PFNWGLQUERYVIDEOCAPTUREDEVICENVPROC epoxy_wglQueryVideoCaptureDeviceNV = epoxy_wglQueryVideoCaptureDeviceNV_global_rewrite_ptr;

PUBLIC PFNWGLREALIZELAYERPALETTEPROC epoxy_wglRealizeLayerPalette = epoxy_wglRealizeLayerPalette_global_rewrite_ptr;

PUBLIC PFNWGLRELEASEIMAGEBUFFEREVENTSI3DPROC epoxy_wglReleaseImageBufferEventsI3D = epoxy_wglReleaseImageBufferEventsI3D_global_rewrite_ptr;

PUBLIC PFNWGLRELEASEPBUFFERDCARBPROC epoxy_wglReleasePbufferDCARB = epoxy_wglReleasePbufferDCARB_global_rewrite_ptr;

PUBLIC PFNWGLRELEASEPBUFFERDCEXTPROC epoxy_wglReleasePbufferDCEXT = epoxy_wglReleasePbufferDCEXT_global_rewrite_ptr;

PUBLIC PFNWGLRELEASETEXIMAGEARBPROC epoxy_wglReleaseTexImageARB = epoxy_wglReleaseTexImageARB_global_rewrite_ptr;

PUBLIC PFNWGLRELEASEVIDEOCAPTUREDEVICENVPROC epoxy_wglReleaseVideoCaptureDeviceNV = epoxy_wglReleaseVideoCaptureDeviceNV_global_rewrite_ptr;

PUBLIC PFNWGLRELEASEVIDEODEVICENVPROC epoxy_wglReleaseVideoDeviceNV = epoxy_wglReleaseVideoDeviceNV_global_rewrite_ptr;

PUBLIC PFNWGLRELEASEVIDEOIMAGENVPROC epoxy_wglReleaseVideoImageNV = epoxy_wglReleaseVideoImageNV_global_rewrite_ptr;

PUBLIC PFNWGLRESETFRAMECOUNTNVPROC epoxy_wglResetFrameCountNV = epoxy_wglResetFrameCountNV_global_rewrite_ptr;

PUBLIC PFNWGLRESTOREBUFFERREGIONARBPROC epoxy_wglRestoreBufferRegionARB = epoxy_wglRestoreBufferRegionARB_global_rewrite_ptr;

PUBLIC PFNWGLSAVEBUFFERREGIONARBPROC epoxy_wglSaveBufferRegionARB = epoxy_wglSaveBufferRegionARB_global_rewrite_ptr;

PUBLIC PFNWGLSENDPBUFFERTOVIDEONVPROC epoxy_wglSendPbufferToVideoNV = epoxy_wglSendPbufferToVideoNV_global_rewrite_ptr;

PUBLIC PFNWGLSETDIGITALVIDEOPARAMETERSI3DPROC epoxy_wglSetDigitalVideoParametersI3D = epoxy_wglSetDigitalVideoParametersI3D_global_rewrite_ptr;

PUBLIC PFNWGLSETGAMMATABLEI3DPROC epoxy_wglSetGammaTableI3D = epoxy_wglSetGammaTableI3D_global_rewrite_ptr;

PUBLIC PFNWGLSETGAMMATABLEPARAMETERSI3DPROC epoxy_wglSetGammaTableParametersI3D = epoxy_wglSetGammaTableParametersI3D_global_rewrite_ptr;

PUBLIC PFNWGLSETLAYERPALETTEENTRIESPROC epoxy_wglSetLayerPaletteEntries = epoxy_wglSetLayerPaletteEntries_global_rewrite_ptr;

PUBLIC PFNWGLSETPBUFFERATTRIBARBPROC epoxy_wglSetPbufferAttribARB = epoxy_wglSetPbufferAttribARB_global_rewrite_ptr;

PUBLIC PFNWGLSETSTEREOEMITTERSTATE3DLPROC epoxy_wglSetStereoEmitterState3DL = epoxy_wglSetStereoEmitterState3DL_global_rewrite_ptr;

PUBLIC PFNWGLSHARELISTSPROC epoxy_wglShareLists = epoxy_wglShareLists_global_rewrite_ptr;

PUBLIC PFNWGLSWAPBUFFERSMSCOMLPROC epoxy_wglSwapBuffersMscOML = epoxy_wglSwapBuffersMscOML_global_rewrite_ptr;

PUBLIC PFNWGLSWAPINTERVALEXTPROC epoxy_wglSwapIntervalEXT = epoxy_wglSwapIntervalEXT_global_rewrite_ptr;

PUBLIC PFNWGLSWAPLAYERBUFFERSPROC epoxy_wglSwapLayerBuffers = epoxy_wglSwapLayerBuffers_global_rewrite_ptr;

PUBLIC PFNWGLSWAPLAYERBUFFERSMSCOMLPROC epoxy_wglSwapLayerBuffersMscOML = epoxy_wglSwapLayerBuffersMscOML_global_rewrite_ptr;

PUBLIC PFNWGLUSEFONTBITMAPSAPROC epoxy_wglUseFontBitmapsA = epoxy_wglUseFontBitmapsA_global_rewrite_ptr;

PUBLIC PFNWGLUSEFONTBITMAPSWPROC epoxy_wglUseFontBitmapsW = epoxy_wglUseFontBitmapsW_global_rewrite_ptr;

PUBLIC PFNWGLUSEFONTOUTLINESPROC epoxy_wglUseFontOutlines = epoxy_wglUseFontOutlines_global_rewrite_ptr;

PUBLIC PFNWGLUSEFONTOUTLINESAPROC epoxy_wglUseFontOutlinesA = epoxy_wglUseFontOutlinesA_global_rewrite_ptr;

PUBLIC PFNWGLUSEFONTOUTLINESWPROC epoxy_wglUseFontOutlinesW = epoxy_wglUseFontOutlinesW_global_rewrite_ptr;

PUBLIC PFNWGLWAITFORMSCOMLPROC epoxy_wglWaitForMscOML = epoxy_wglWaitForMscOML_global_rewrite_ptr;

PUBLIC PFNWGLWAITFORSBCOMLPROC epoxy_wglWaitForSbcOML = epoxy_wglWaitForSbcOML_global_rewrite_ptr;

