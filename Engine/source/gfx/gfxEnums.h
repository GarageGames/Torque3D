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

#ifndef _GFXENUMS_H_
#define _GFXENUMS_H_

#include "core/util/fourcc.h"

// These are for the enum translation. It will help with porting to other platforms
// and API's.
#define GFX_UNSUPPORTED_VAL 0xDEADBEEF
#define GFX_UNINIT_VAL 0xDECAFBAD

// Adjust these pools to your app's needs.  Be aware dynamic vertices are much more
// expensive than static vertices. These are in gfxEnums because they should be
// consistant across all APIs/platforms so that the dynamic buffer performance
// and behavior is also consistant. -patw
#define MAX_DYNAMIC_VERTS   (8192*2)
#define MAX_DYNAMIC_INDICES (8192*4)

enum GFXBufferType
{
      GFXBufferTypeStatic,   ///< Static vertex buffers are created and rarely updated.
                   ///< Updating might incur a performance penalty.  Resizing a static vertex buffer is not
                   ///< allowed.
      GFXBufferTypeDynamic,  ///< Dynamic vertex buffers are meant for vertices that can be changed
                   ///< often.  Vertices written into dynamic vertex buffers will remain valid
                   ///< until the dynamic vertex buffer is released.  Resizing a dynamic vertex buffer is not
                   ///< allowed.
      GFXBufferTypeVolatile, ///< Volatile vertex or index buffers are meant for vertices or indices that are essentially
                   ///< only used once.  They can be resized without any performance penalty.

      GFXBufferTypeImmutable, ///< Immutable buffers must specify the data when creating the buffer. Cannot be modified.

      GFXBufferType_COUNT ///< Number of buffer types.
};

enum GFXTexCallbackCode
{
   GFXZombify,
   GFXResurrect,
};


enum GFXPrimitiveType 
{
   GFXPT_FIRST = 0,
   GFXPointList = 0,
   GFXLineList,
   GFXLineStrip,
   GFXTriangleList,
   GFXTriangleStrip,
   GFXPT_COUNT
};

enum GFXTextureType 
{
   GFXTextureType_Normal,
   GFXTextureType_KeepBitmap,
   GFXTextureType_Dynamic,
   GFXTextureType_RenderTarget,
   GFXTextureType_Count
};

enum GFXBitmapFlip 
{
   GFXBitmapFlip_None = 0,
   GFXBitmapFlip_X    = 1 << 0,
   GFXBitmapFlip_Y    = 1 << 1,
   GFXBitmapFlip_XY   = GFXBitmapFlip_X | GFXBitmapFlip_Y
};

enum GFXTextureOp 
{
   GFXTOP_FIRST = 0,
   GFXTOPDisable = 0,
   GFXTOPSelectARG1,
   GFXTOPSelectARG2,
   GFXTOPModulate,
   GFXTOPModulate2X,
   GFXTOPModulate4X,
   GFXTOPAdd,
   GFXTOPAddSigned,
   GFXTOPAddSigned2X,
   GFXTOPSubtract,
   GFXTOPAddSmooth, 
   GFXTOPBlendDiffuseAlpha,
   GFXTOPBlendTextureAlpha,
   GFXTOPBlendFactorAlpha,
   GFXTOPBlendTextureAlphaPM,
   GFXTOPBlendCURRENTALPHA,
   GFXTOPPreModulate,
   GFXTOPModulateAlphaAddColor,
   GFXTOPModulateColorAddAlpha,
   GFXTOPModulateInvAlphaAddColor,
   GFXTOPModulateInvColorAddAlpha,
   GFXTOPBumpEnvMap,
   GFXTOPBumpEnvMapLuminance,
   GFXTOPDotProduct3,
   GFXTOPLERP,
   GFXTOP_COUNT
};

enum GFXTextureAddressMode 
{
   GFXAddress_FIRST = 0,
   GFXAddressWrap = 0,
   GFXAddressMirror,
   GFXAddressClamp,
   GFXAddressBorder,
   GFXAddressMirrorOnce,
   GFXAddress_COUNT
};

enum GFXTextureFilterType 
{
   GFXTextureFilter_FIRST = 0,
   GFXTextureFilterNone = 0,
   GFXTextureFilterPoint,
   GFXTextureFilterLinear,
   GFXTextureFilterAnisotropic,
   GFXTextureFilterPyramidalQuad,
   GFXTextureFilterGaussianQuad,
   GFXTextureFilter_COUNT
};

enum GFXFillMode 
{
   GFXFill_FIRST = 1,
   GFXFillPoint = 1,
   GFXFillWireframe,
   GFXFillSolid,
   GFXFill_COUNT
};

enum GFXFormat 
{
   // when adding formats make sure to place
   // them in the correct group!
   //
   // if displacing the first entry in the group
   // make sure to update the GFXFormat_xBIT entries!
   //
   GFXFormat_FIRST = 0,

   // 8 bit texture formats...
   GFXFormatA8 = 0,// first in group...
   GFXFormatL8,
   GFXFormatA4L4,

   // 16 bit texture formats...
   GFXFormatR5G6B5,// first in group...
   GFXFormatR5G5B5A1,
   GFXFormatR5G5B5X1,
   GFXFormatA8L8,
   GFXFormatL16,
   GFXFormatR16F,
   GFXFormatD16,

   // 24 bit texture formats...
   GFXFormatR8G8B8,// first in group...

   // 32 bit texture formats...
   GFXFormatR8G8B8A8,// first in group...
   GFXFormatR8G8B8X8,
   GFXFormatB8G8R8A8,
   GFXFormatR32F,
   GFXFormatR16G16,
   GFXFormatR16G16F,
   GFXFormatR10G10B10A2,
   GFXFormatD32,
   GFXFormatD24X8,
   GFXFormatD24S8,   
   GFXFormatD24FS8,

   // sRGB formats
   GFXFormatR8G8B8A8_SRGB,

   // Guaranteed RGBA8 (for apis which really dont like bgr)
   GFXFormatR8G8B8A8_LINEAR_FORCE,

   // 64 bit texture formats...
   GFXFormatR16G16B16A16,// first in group...
   GFXFormatR16G16B16A16F,

   // 128 bit texture formats...
   GFXFormatR32G32B32A32F,// first in group...

   // unknown size...
   GFXFormatDXT1,// first in group...
   GFXFormatDXT2,
   GFXFormatDXT3,
   GFXFormatDXT4,
   GFXFormatDXT5,

   GFXFormat_COUNT,

   GFXFormat_8BIT = GFXFormatA8,
   GFXFormat_16BIT = GFXFormatR5G6B5,
   GFXFormat_24BIT = GFXFormatR8G8B8,
   GFXFormat_32BIT = GFXFormatR8G8B8A8,
   GFXFormat_64BIT = GFXFormatR16G16B16A16,
   GFXFormat_128BIT = GFXFormatR32G32B32A32F,
   GFXFormat_UNKNOWNSIZE = GFXFormatDXT1,
};

/// Returns the byte size of the pixel for non-compressed formats.
inline U32 GFXFormat_getByteSize( GFXFormat format )
{
   AssertFatal( format < GFXFormat_UNKNOWNSIZE, 
      "GFXDevice::formatByteSize - Cannot size a compressed format!" );

   if ( format < GFXFormat_16BIT )
      return 1;// 8 bit...
   else if ( format < GFXFormat_24BIT )
      return 2;// 16 bit...
   else if ( format < GFXFormat_32BIT )
      return 3;// 24 bit...
   else if ( format < GFXFormat_64BIT )
      return 4;// 32 bit...
   else if ( format < GFXFormat_128BIT )
      return 8;// 64 bit...

   // This should be 128bits... else its a DDS and 
   // the assert should have gone off above.
   return 16;
}

enum GFXShadeMode 
{
   GFXShadeFlat = 1,
   GFXShadeGouraud,
   GFXShadePhong,
};

enum GFXClearFlags 
{
   GFXClearTarget = 1 << 0,
   GFXClearZBuffer = 1 << 1,
   GFXClearStencil = 1 << 2,
};

/// The supported blend modes.
enum GFXBlend 
{
   GFXBlend_FIRST = 0,
   GFXBlendZero = 0, /// (0, 0, 0, 0)
   GFXBlendOne, /// (1, 1, 1, 1)
   GFXBlendSrcColor, /// (Rs, Gs, Bs, As)
   GFXBlendInvSrcColor, /// (1 - Rs, 1 - Gs, 1 - Bs, 1 - As)
   GFXBlendSrcAlpha, /// (As, As, As, As)
   GFXBlendInvSrcAlpha, /// ( 1 - As, 1 - As, 1 - As, 1 - As)
   GFXBlendDestAlpha, /// (Ad Ad Ad Ad)
   GFXBlendInvDestAlpha, /// (1 - Ad 1 - Ad 1 - Ad 1 - Ad)
   GFXBlendDestColor, /// (Rd, Gd, Bd, Ad)
   GFXBlendInvDestColor, /// (1 - Rd, 1 - Gd, 1 - Bd, 1 - Ad)
   GFXBlendSrcAlphaSat, /// (f, f, f, 1) where f = min(As, 1 - Ad)
   GFXBlend_COUNT
};

/// Constants that name each GFXDevice type. Any new GFXDevice subclass must be
/// added to this enum. A string representing its name must also be added to
/// GFXInit::getAdapterNameFromType().
enum GFXAdapterType 
{
   OpenGL = 0,
   Direct3D11,
   Direct3D9,
   NullDevice,
   Direct3D9_360,
   GFXAdapterType_Count
};

enum GFXCullMode
{
   GFXCull_FIRST = 0,
   GFXCullNone = 0,
   GFXCullCW,
   GFXCullCCW,
   GFXCull_COUNT
};

enum GFXCmpFunc 
{
   GFXCmp_FIRST = 0,
   GFXCmpNever = 0,
   GFXCmpLess,
   GFXCmpEqual,
   GFXCmpLessEqual,
   GFXCmpGreater,
   GFXCmpNotEqual,
   GFXCmpGreaterEqual,
   GFXCmpAlways,
   GFXCmp_COUNT
};

enum GFXStencilOp 
{
   GFXStencilOp_FIRST = 0,
   GFXStencilOpKeep = 0,
   GFXStencilOpZero,
   GFXStencilOpReplace,
   GFXStencilOpIncrSat,
   GFXStencilOpDecrSat,
   GFXStencilOpInvert,
   GFXStencilOpIncr,
   GFXStencilOpDecr,
   GFXStencilOp_COUNT
};

enum GFXMaterialColorSource 
{
   GFXMCSMaterial = 0,
   GFXMCSColor1,
   GFXMCSColor2,
};

enum GFXBlendOp 
{ 
   GFXBlendOp_FIRST = 0,
   GFXBlendOpAdd = 0,
   GFXBlendOpSubtract,
   GFXBlendOpRevSubtract,
   GFXBlendOpMin,
   GFXBlendOpMax,
   GFXBlendOp_COUNT
};

enum GFXRenderState 
{
   GFXRenderState_FIRST = 0,
   GFXRSZEnable = 0,
   GFXRSFillMode,
   GFXRSShadeMode,
   GFXRSZWriteEnable,
   GFXRSAlphaTestEnable,
   GFXRSLastPixel,
   GFXRSSrcBlend,
   GFXRSDestBlend,
   GFXRSCullMode,
   GFXRSZFunc,
   GFXRSAlphaRef,
   GFXRSAlphaFunc,
   GFXRSDitherEnable,
   GFXRSAlphaBlendEnable,
   GFXRSFogEnable,
   GFXRSSpecularEnable,
   GFXRSFogColor,
   GFXRSFogTableMode,
   GFXRSFogStart,
   GFXRSFogEnd,
   GFXRSFogDensity,
   GFXRSRangeFogEnable,
   GFXRSStencilEnable,
   GFXRSStencilFail,
   GFXRSStencilZFail,
   GFXRSStencilPass,
   GFXRSStencilFunc,
   GFXRSStencilRef,
   GFXRSStencilMask,
   GFXRSStencilWriteMask,
   GFXRSTextureFactor,
   GFXRSWrap0,
   GFXRSWrap1,
   GFXRSWrap2,
   GFXRSWrap3,
   GFXRSWrap4,
   GFXRSWrap5,
   GFXRSWrap6,
   GFXRSWrap7,
   GFXRSClipping,
   GFXRSLighting,
   GFXRSAmbient,
   GFXRSFogVertexMode,
   GFXRSColorVertex,
   GFXRSLocalViewer,
   GFXRSNormalizeNormals,
   GFXRSDiffuseMaterialSource,
   GFXRSSpecularMaterialSource,
   GFXRSAmbientMaterialSource,
   GFXRSEmissiveMaterialSource,
   GFXRSVertexBlend,
   GFXRSClipPlaneEnable,
   GFXRSPointSize,
   GFXRSPointSizeMin,
   GFXRSPointSpriteEnable,
   GFXRSPointScaleEnable,
   GFXRSPointScale_A,
   GFXRSPointScale_B,
   GFXRSPointScale_C,
   GFXRSMultiSampleantiAlias,
   GFXRSMultiSampleMask,
   GFXRSPatchEdgeStyle,
   GFXRSDebugMonitorToken,
   GFXRSPointSize_Max,
   GFXRSIndexedVertexBlendEnable,
   GFXRSColorWriteEnable,
   GFXRSTweenFactor,
   GFXRSBlendOp,
   GFXRSPositionDegree,
   GFXRSNormalDegree,
   GFXRSScissorTestEnable,
   GFXRSSlopeScaleDepthBias,
   GFXRSAntiAliasedLineEnable,
   GFXRSMinTessellationLevel,
   GFXRSMaxTessellationLevel,
   GFXRSAdaptiveTess_X,
   GFXRSAdaptiveTess_Y,
   GFXRSdaptiveTess_Z,
   GFXRSAdaptiveTess_W,
   GFXRSEnableAdaptiveTesselation,
   GFXRSTwoSidedStencilMode,
   GFXRSCCWStencilFail,
   GFXRSCCWStencilZFail,
   GFXRSCCWStencilPass,
   GFXRSCCWStencilFunc,
   GFXRSColorWriteEnable1,
   GFXRSColorWriteEnable2,
   GFXRSolorWriteEnable3,
   GFXRSBlendFactor,
   GFXRSSRGBWriteEnable,
   GFXRSDepthBias,
   GFXRSWrap8,
   GFXRSWrap9,
   GFXRSWrap10,
   GFXRSWrap11,
   GFXRSWrap12,
   GFXRSWrap13,
   GFXRSWrap14,
   GFXRSWrap15,
   GFXRSSeparateAlphaBlendEnable,
   GFXRSSrcBlendAlpha,
   GFXRSDestBlendAlpha,
   GFXRSBlendOpAlpha,
   GFXRenderState_COUNT          ///< Don't use this one, this is a counter
};

#define GFXCOLORWRITEENABLE_RED     1
#define GFXCOLORWRITEENABLE_GREEN   2
#define GFXCOLORWRITEENABLE_BLUE    4
#define GFXCOLORWRITEENABLE_ALPHA   8

enum GFXTextureStageState 
{
   GFXTSS_FIRST = 0,
   GFXTSSColorOp = 0,
   GFXTSSColorArg1,
   GFXTSSColorArg2,
   GFXTSSAlphaOp,
   GFXTSSAlphaArg1,
   GFXTSSAlphaArg2,
   GFXTSSBumpEnvMat00,
   GFXTSSBumpEnvMat01,
   GFXTSSBumpEnvMat10,
   GFXTSSBumpEnvMat11,
   GFXTSSTexCoordIndex,
   GFXTSSBumpEnvlScale,
   GFXTSSBumpEnvlOffset,
   GFXTSSTextureTransformFlags,
   GFXTSSColorArg0,
   GFXTSSAlphaArg0,
   GFXTSSResultArg,
   GFXTSSConstant,
   GFXTSS_COUNT            ///< Don't use this one, this is a counter
};

enum GFXTextureTransformFlags
{
   GFXTTFFDisable = 0,
   GFXTTFFCoord1D = 1,
   GFXTTFFCoord2D = 2,
   GFXTTFFCoord3D = 3,
   GFXTTFFCoord4D = 4,
   GFXTTFFProjected = 256,
};

// CodeReview: This number is used for the declaration of variables, but it
// should *not* be used for any run-time purposes [7/2/2007 Pat]
#define TEXTURE_STAGE_COUNT 16

enum GFXSamplerState 
{
   GFXSAMP_FIRST = 0,
   GFXSAMPAddressU = 0,
   GFXSAMPAddressV,
   GFXSAMPAddressW,
   GFXSAMPBorderColor,
   GFXSAMPMagFilter,
   GFXSAMPMinFilter,
   GFXSAMPMipFilter,
   GFXSAMPMipMapLODBias,
   GFXSAMPMaxMipLevel,
   GFXSAMPMaxAnisotropy,
   GFXSAMPSRGBTexture,
   GFXSAMPElementIndex,
   GFXSAMPDMapOffset,
   GFXSAMP_COUNT          ///< Don't use this one, this is a counter
};

enum GFXTextureArgument 
{
   GFXTA_FIRST = 0,
   GFXTADiffuse = 0,
   GFXTACurrent,
   GFXTATexture,
   GFXTATFactor,
   GFXTASpecular,
   GFXTATemp,
   GFXTAConstant,
   GFXTA_COUNT,
   GFXTAComplement = 0x00000010,       // take 1.0 - x (read modifier)
   GFXTAAlphaReplicate = 0x00000020,   // replicate alpha to color components (read modifier)
};

// Matrix stuff
#define WORLD_STACK_MAX 24

enum GFXMatrixType 
{
   GFXMatrixWorld = 256,
   GFXMatrixView = 2,
   GFXMatrixProjection = 3,
   GFXMatrixTexture = 16,     // This value is texture matrix for sampler 0, can use this for offset
   GFXMatrixTexture0 = 16,
   GFXMatrixTexture1 = 17,
   GFXMatrixTexture2 = 18,
   GFXMatrixTexture3 = 19,
   GFXMatrixTexture4 = 20,
   GFXMatrixTexture5 = 21,
   GFXMatrixTexture6 = 22,
   GFXMatrixTexture7 = 23,
};

// Light define
#define LIGHT_STAGE_COUNT 8

#define GFXVERTEXFLAG_F32     3
#define GFXVERTEXFLAG_POINT2F 0
#define GFXVERTEXFLAG_POINT3F 1 
#define GFXVERTEXFLAG_POINT4F 2

#define GFXVERTEXFLAG_TEXCOORD_F32(CoordIndex)     ( GFXVERTEXFLAG_F32     << ( CoordIndex * 2 + 16 ) ) 
#define GFXVERTEXFLAG_TEXCOORD_POINT2F(CoordIndex) ( GFXVERTEXFLAG_POINT2F ) 
#define GFXVERTEXFLAG_TEXCOORD_POINT3F(CoordIndex) ( GFXVERTEXFLAG_POINT3F << ( CoordIndex * 2 + 16 ) ) 
#define GFXVERTEXFLAG_TEXCOORD_POINT4F(CoordIndex) ( GFXVERTEXFLAG_POINT4F << ( CoordIndex * 2 + 16 ) )

#define STATE_STACK_SIZE 32

// Index Formats
enum GFXIndexFormat 
{
   GFXIndexFormat_FIRST = 0,
   GFXIndexFormat16 = 0,
   GFXIndexFormat32,
   GFXIndexFormat_COUNT
};

enum GFXShaderConstType
{
   /// GFX"S"hader"C"onstant"T"ype
   // Scalar
   GFXSCT_Float, 
   // Vectors
   GFXSCT_Float2, 
   GFXSCT_Float3, 
   GFXSCT_Float4, 
   // Matrices
   GFXSCT_Float2x2, 
   GFXSCT_Float3x3,
   GFXSCT_Float3x4,
   GFXSCT_Float4x3,
   GFXSCT_Float4x4, 
   // Scalar
   GFXSCT_Int, 
   // Vectors
   GFXSCT_Int2, 
   GFXSCT_Int3, 
   GFXSCT_Int4, 
   // Samplers
   GFXSCT_Sampler,
   GFXSCT_SamplerCube
};


/// Defines a vertex declaration type.
/// @see GFXVertexElement
/// @see GFXVertexFormat
enum GFXDeclType
{
   GFXDeclType_FIRST = 0,

   /// A single component F32.
   GFXDeclType_Float = 0,

   /// A two-component F32.
   /// @see Point2F
   GFXDeclType_Float2,

   /// A three-component F32. 
   /// @see Point3F
   GFXDeclType_Float3,

   /// A four-component F32.
   /// @see Point4F
   GFXDeclType_Float4,

   /// A four-component, packed, unsigned bytes mapped to 0 to 1 range.
   /// @see GFXVertexColor
   GFXDeclType_Color,

   /// Four-component, packed, unsigned bytes ranged 0-255
   GFXDeclType_UByte4,

   /// The count of total GFXDeclTypes.
   GFXDeclType_COUNT,
};

#endif // _GFXENUMS_H_
