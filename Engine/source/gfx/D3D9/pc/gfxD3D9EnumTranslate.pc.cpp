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

#include <d3d9.h>
#include "gfx/D3D9/gfxD3D9EnumTranslate.h"
#include "console/console.h"

//------------------------------------------------------------------------------

_D3DFORMAT GFXD3D9IndexFormat[GFXIndexFormat_COUNT];
_D3DSAMPLERSTATETYPE GFXD3D9SamplerState[GFXSAMP_COUNT];
_D3DFORMAT GFXD3D9TextureFormat[GFXFormat_COUNT];
_D3DRENDERSTATETYPE GFXD3D9RenderState[GFXRenderState_COUNT];
_D3DTEXTUREFILTERTYPE GFXD3D9TextureFilter[GFXTextureFilter_COUNT];
_D3DBLEND GFXD3D9Blend[GFXBlend_COUNT];
_D3DBLENDOP GFXD3D9BlendOp[GFXBlendOp_COUNT];
_D3DSTENCILOP GFXD3D9StencilOp[GFXStencilOp_COUNT];
_D3DCMPFUNC GFXD3D9CmpFunc[GFXCmp_COUNT];
_D3DCULL GFXD3D9CullMode[GFXCull_COUNT];
_D3DFILLMODE GFXD3D9FillMode[GFXFill_COUNT];
_D3DPRIMITIVETYPE GFXD3D9PrimType[GFXPT_COUNT];
_D3DTEXTURESTAGESTATETYPE GFXD3D9TextureStageState[GFXTSS_COUNT];
_D3DTEXTUREADDRESS GFXD3D9TextureAddress[GFXAddress_COUNT];
_D3DTEXTUREOP GFXD3D9TextureOp[GFXTOP_COUNT];
_D3DDECLTYPE GFXD3D9DeclType[GFXDeclType_COUNT];

//------------------------------------------------------------------------------

#define INIT_LOOKUPTABLE( tablearray, enumprefix, type ) \
   for( S32 i = enumprefix##_FIRST; i < enumprefix##_COUNT; i++ ) \
      tablearray[i] = (type)GFX_UNINIT_VAL;

#define VALIDATE_LOOKUPTABLE( tablearray, enumprefix ) \
   for( S32 i = enumprefix##_FIRST; i < enumprefix##_COUNT; i++ ) \
      if( (S32)tablearray[i] == GFX_UNINIT_VAL ) \
         Con::warnf( "GFXD3D9EnumTranslate: Unassigned value in " #tablearray ": %i", i ); \
      else if( (S32)tablearray[i] == GFX_UNSUPPORTED_VAL ) \
         Con::warnf( "GFXD3D9EnumTranslate: Unsupported value in " #tablearray ": %i", i );

//------------------------------------------------------------------------------

void GFXD3D9EnumTranslate::init()
{
   INIT_LOOKUPTABLE( GFXD3D9IndexFormat, GFXIndexFormat, _D3DFORMAT );
   GFXD3D9IndexFormat[GFXIndexFormat16] = D3DFMT_INDEX16;
   GFXD3D9IndexFormat[GFXIndexFormat32] = D3DFMT_INDEX32;
   VALIDATE_LOOKUPTABLE( GFXD3D9IndexFormat, GFXIndexFormat );
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
   INIT_LOOKUPTABLE( GFXD3D9SamplerState, GFXSAMP, _D3DSAMPLERSTATETYPE );
   GFXD3D9SamplerState[GFXSAMPAddressU] = D3DSAMP_ADDRESSU;
   GFXD3D9SamplerState[GFXSAMPAddressV] = D3DSAMP_ADDRESSV;
   GFXD3D9SamplerState[GFXSAMPAddressW] = D3DSAMP_ADDRESSW;
   GFXD3D9SamplerState[GFXSAMPBorderColor] = D3DSAMP_BORDERCOLOR;
   GFXD3D9SamplerState[GFXSAMPMagFilter] = D3DSAMP_MAGFILTER;
   GFXD3D9SamplerState[GFXSAMPMinFilter] = D3DSAMP_MINFILTER;
   GFXD3D9SamplerState[GFXSAMPMipFilter] = D3DSAMP_MIPFILTER;
   GFXD3D9SamplerState[GFXSAMPMipMapLODBias] = D3DSAMP_MIPMAPLODBIAS;
   GFXD3D9SamplerState[GFXSAMPMaxMipLevel] = D3DSAMP_MAXMIPLEVEL;
   GFXD3D9SamplerState[GFXSAMPMaxAnisotropy] = D3DSAMP_MAXANISOTROPY;
   GFXD3D9SamplerState[GFXSAMPSRGBTexture] = D3DSAMP_SRGBTEXTURE;
   GFXD3D9SamplerState[GFXSAMPElementIndex] = D3DSAMP_ELEMENTINDEX;
   GFXD3D9SamplerState[GFXSAMPDMapOffset] = D3DSAMP_DMAPOFFSET;
   VALIDATE_LOOKUPTABLE( GFXD3D9SamplerState, GFXSAMP );
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
   INIT_LOOKUPTABLE( GFXD3D9TextureFormat, GFXFormat, _D3DFORMAT );
   GFXD3D9TextureFormat[GFXFormatR8G8B8] = D3DFMT_R8G8B8;
   GFXD3D9TextureFormat[GFXFormatR8G8B8A8] = D3DFMT_A8R8G8B8;
   GFXD3D9TextureFormat[GFXFormatR8G8B8X8] = D3DFMT_X8R8G8B8;
   GFXD3D9TextureFormat[GFXFormatB8G8R8A8] = D3DFMT_A8R8G8B8;
   GFXD3D9TextureFormat[GFXFormatR5G6B5] = D3DFMT_R5G6B5;
   GFXD3D9TextureFormat[GFXFormatR5G5B5A1] = D3DFMT_A1R5G5B5;
   GFXD3D9TextureFormat[GFXFormatR5G5B5X1] = D3DFMT_X1R5G5B5;
   GFXD3D9TextureFormat[GFXFormatR32F] = D3DFMT_R32F;
   GFXD3D9TextureFormat[GFXFormatA4L4] = D3DFMT_A4L4;
   GFXD3D9TextureFormat[GFXFormatA8L8] = D3DFMT_A8L8;
   GFXD3D9TextureFormat[GFXFormatA8] = D3DFMT_A8;
   GFXD3D9TextureFormat[GFXFormatL8] = D3DFMT_L8;
   GFXD3D9TextureFormat[GFXFormatDXT1] = D3DFMT_DXT1;
   GFXD3D9TextureFormat[GFXFormatDXT2] = D3DFMT_DXT2;
   GFXD3D9TextureFormat[GFXFormatDXT3] = D3DFMT_DXT3;
   GFXD3D9TextureFormat[GFXFormatDXT4] = D3DFMT_DXT4;
   GFXD3D9TextureFormat[GFXFormatDXT5] = D3DFMT_DXT5;
   GFXD3D9TextureFormat[GFXFormatR32G32B32A32F] = D3DFMT_A32B32G32R32F;
   GFXD3D9TextureFormat[GFXFormatR16G16B16A16F] = D3DFMT_A16B16G16R16F;
   GFXD3D9TextureFormat[GFXFormatL16] = D3DFMT_L16;
   GFXD3D9TextureFormat[GFXFormatR16G16B16A16] = D3DFMT_A16B16G16R16;
   GFXD3D9TextureFormat[GFXFormatR16G16] = D3DFMT_G16R16;
   GFXD3D9TextureFormat[GFXFormatR16F] = D3DFMT_R16F;
   GFXD3D9TextureFormat[GFXFormatR16G16F] = D3DFMT_G16R16F;
   GFXD3D9TextureFormat[GFXFormatR10G10B10A2] = D3DFMT_A2R10G10B10;
   GFXD3D9TextureFormat[GFXFormatD32] = D3DFMT_D32;
   GFXD3D9TextureFormat[GFXFormatD24X8] = D3DFMT_D24X8;
   GFXD3D9TextureFormat[GFXFormatD24S8] = D3DFMT_D24S8;
   GFXD3D9TextureFormat[GFXFormatD24FS8] = D3DFMT_D24FS8;
   GFXD3D9TextureFormat[GFXFormatD16] = D3DFMT_D16;
   GFXD3D9TextureFormat[GFXFormatR8G8B8A8_SRGB] = D3DFMT_UNKNOWN;

   GFXD3D9TextureFormat[GFXFormatR8G8B8A8_LINEAR_FORCE] = D3DFMT_A8R8G8B8;
   VALIDATE_LOOKUPTABLE( GFXD3D9TextureFormat, GFXFormat);
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
   INIT_LOOKUPTABLE( GFXD3D9RenderState, GFXRenderState, _D3DRENDERSTATETYPE );
   GFXD3D9RenderState[GFXRSZEnable] = D3DRS_ZENABLE;
   GFXD3D9RenderState[GFXRSFillMode] = D3DRS_FILLMODE;
   GFXD3D9RenderState[GFXRSZWriteEnable] = D3DRS_ZWRITEENABLE;
   GFXD3D9RenderState[GFXRSAlphaTestEnable] = D3DRS_ALPHATESTENABLE;
   GFXD3D9RenderState[GFXRSSrcBlend] = D3DRS_SRCBLEND;
   GFXD3D9RenderState[GFXRSDestBlend] = D3DRS_DESTBLEND;
   GFXD3D9RenderState[GFXRSCullMode] = D3DRS_CULLMODE;
   GFXD3D9RenderState[GFXRSZFunc] = D3DRS_ZFUNC;
   GFXD3D9RenderState[GFXRSAlphaRef] = D3DRS_ALPHAREF;
   GFXD3D9RenderState[GFXRSAlphaFunc] = D3DRS_ALPHAFUNC;
   GFXD3D9RenderState[GFXRSAlphaBlendEnable] = D3DRS_ALPHABLENDENABLE;
   GFXD3D9RenderState[GFXRSStencilEnable] = D3DRS_STENCILENABLE;
   GFXD3D9RenderState[GFXRSStencilFail] = D3DRS_STENCILFAIL;
   GFXD3D9RenderState[GFXRSStencilZFail] = D3DRS_STENCILZFAIL;
   GFXD3D9RenderState[GFXRSStencilPass] = D3DRS_STENCILPASS;
   GFXD3D9RenderState[GFXRSStencilFunc] = D3DRS_STENCILFUNC;
   GFXD3D9RenderState[GFXRSStencilRef] = D3DRS_STENCILREF;
   GFXD3D9RenderState[GFXRSStencilMask] = D3DRS_STENCILMASK;
   GFXD3D9RenderState[GFXRSStencilWriteMask] = D3DRS_STENCILWRITEMASK;
   GFXD3D9RenderState[GFXRSWrap0] = D3DRS_WRAP0;
   GFXD3D9RenderState[GFXRSWrap1] = D3DRS_WRAP1;
   GFXD3D9RenderState[GFXRSWrap2] = D3DRS_WRAP2;
   GFXD3D9RenderState[GFXRSWrap3] = D3DRS_WRAP3;
   GFXD3D9RenderState[GFXRSWrap4] = D3DRS_WRAP4;
   GFXD3D9RenderState[GFXRSWrap5] = D3DRS_WRAP5;
   GFXD3D9RenderState[GFXRSWrap6] = D3DRS_WRAP6;
   GFXD3D9RenderState[GFXRSWrap7] = D3DRS_WRAP7;
   GFXD3D9RenderState[GFXRSClipPlaneEnable] = D3DRS_CLIPPLANEENABLE;
   GFXD3D9RenderState[GFXRSPointSize] = D3DRS_POINTSIZE;
   GFXD3D9RenderState[GFXRSPointSizeMin] = D3DRS_POINTSIZE_MIN;
   GFXD3D9RenderState[GFXRSPointSize_Max] = D3DRS_POINTSIZE_MAX;
   GFXD3D9RenderState[GFXRSPointSpriteEnable] = D3DRS_POINTSPRITEENABLE;
   GFXD3D9RenderState[GFXRSMultiSampleantiAlias] = D3DRS_MULTISAMPLEANTIALIAS;
   GFXD3D9RenderState[GFXRSMultiSampleMask] = D3DRS_MULTISAMPLEMASK;
   GFXD3D9RenderState[GFXRSShadeMode] = D3DRS_SHADEMODE;
   GFXD3D9RenderState[GFXRSLastPixel] = D3DRS_LASTPIXEL;
   GFXD3D9RenderState[GFXRSClipping] = D3DRS_CLIPPING;
   GFXD3D9RenderState[GFXRSPointScaleEnable] = D3DRS_POINTSCALEENABLE;
   GFXD3D9RenderState[GFXRSPointScale_A] = D3DRS_POINTSCALE_A;
   GFXD3D9RenderState[GFXRSPointScale_B] = D3DRS_POINTSCALE_B;
   GFXD3D9RenderState[GFXRSPointScale_C] = D3DRS_POINTSCALE_C;
   GFXD3D9RenderState[GFXRSLighting] = D3DRS_LIGHTING;
   GFXD3D9RenderState[GFXRSAmbient] = D3DRS_AMBIENT;
   GFXD3D9RenderState[GFXRSFogVertexMode] = D3DRS_FOGVERTEXMODE;
   GFXD3D9RenderState[GFXRSColorVertex] = D3DRS_COLORVERTEX;
   GFXD3D9RenderState[GFXRSLocalViewer] = D3DRS_LOCALVIEWER;
   GFXD3D9RenderState[GFXRSNormalizeNormals] = D3DRS_NORMALIZENORMALS;
   GFXD3D9RenderState[GFXRSDiffuseMaterialSource] = D3DRS_DIFFUSEMATERIALSOURCE;
   GFXD3D9RenderState[GFXRSSpecularMaterialSource] = D3DRS_SPECULARMATERIALSOURCE;
   GFXD3D9RenderState[GFXRSAmbientMaterialSource] = D3DRS_AMBIENTMATERIALSOURCE;
   GFXD3D9RenderState[GFXRSEmissiveMaterialSource] = D3DRS_EMISSIVEMATERIALSOURCE;
   GFXD3D9RenderState[GFXRSVertexBlend] = D3DRS_VERTEXBLEND;
   GFXD3D9RenderState[GFXRSFogEnable] = D3DRS_FOGENABLE;
   GFXD3D9RenderState[GFXRSSpecularEnable] = D3DRS_SPECULARENABLE;
   GFXD3D9RenderState[GFXRSFogColor] = D3DRS_FOGCOLOR;
   GFXD3D9RenderState[GFXRSFogTableMode] = D3DRS_FOGTABLEMODE;
   GFXD3D9RenderState[GFXRSFogStart] = D3DRS_FOGSTART;
   GFXD3D9RenderState[GFXRSFogEnd] = D3DRS_FOGEND;
   GFXD3D9RenderState[GFXRSFogDensity] = D3DRS_FOGDENSITY;
   GFXD3D9RenderState[GFXRSRangeFogEnable] = D3DRS_RANGEFOGENABLE;
   GFXD3D9RenderState[GFXRSDebugMonitorToken] = D3DRS_DEBUGMONITORTOKEN;
   GFXD3D9RenderState[GFXRSIndexedVertexBlendEnable] = D3DRS_INDEXEDVERTEXBLENDENABLE;
   GFXD3D9RenderState[GFXRSTweenFactor] = D3DRS_TWEENFACTOR;
   GFXD3D9RenderState[GFXRSTextureFactor] = D3DRS_TEXTUREFACTOR;
   GFXD3D9RenderState[GFXRSPatchEdgeStyle] = D3DRS_PATCHEDGESTYLE;
   GFXD3D9RenderState[GFXRSPositionDegree] = D3DRS_POSITIONDEGREE;
   GFXD3D9RenderState[GFXRSNormalDegree] = D3DRS_NORMALDEGREE;
   GFXD3D9RenderState[GFXRSAntiAliasedLineEnable] = D3DRS_ANTIALIASEDLINEENABLE;
   GFXD3D9RenderState[GFXRSAdaptiveTess_X] = D3DRS_ADAPTIVETESS_X;
   GFXD3D9RenderState[GFXRSAdaptiveTess_Y] = D3DRS_ADAPTIVETESS_Y;
   GFXD3D9RenderState[GFXRSdaptiveTess_Z] = D3DRS_ADAPTIVETESS_Z;
   GFXD3D9RenderState[GFXRSAdaptiveTess_W] = D3DRS_ADAPTIVETESS_W;
   GFXD3D9RenderState[GFXRSEnableAdaptiveTesselation] = D3DRS_ENABLEADAPTIVETESSELLATION;
   GFXD3D9RenderState[GFXRSDitherEnable] = D3DRS_DITHERENABLE;
   GFXD3D9RenderState[GFXRSColorWriteEnable] = D3DRS_COLORWRITEENABLE;
   GFXD3D9RenderState[GFXRSBlendOp] = D3DRS_BLENDOP;
   GFXD3D9RenderState[GFXRSScissorTestEnable] = D3DRS_SCISSORTESTENABLE;
   GFXD3D9RenderState[GFXRSSlopeScaleDepthBias] = D3DRS_SLOPESCALEDEPTHBIAS;
   GFXD3D9RenderState[GFXRSMinTessellationLevel] = D3DRS_MINTESSELLATIONLEVEL;
   GFXD3D9RenderState[GFXRSMaxTessellationLevel] = D3DRS_MAXTESSELLATIONLEVEL;
   GFXD3D9RenderState[GFXRSTwoSidedStencilMode] = D3DRS_TWOSIDEDSTENCILMODE;
   GFXD3D9RenderState[GFXRSCCWStencilFail] = D3DRS_CCW_STENCILFAIL;
   GFXD3D9RenderState[GFXRSCCWStencilZFail] = D3DRS_CCW_STENCILZFAIL;
   GFXD3D9RenderState[GFXRSCCWStencilPass] = D3DRS_CCW_STENCILPASS;
   GFXD3D9RenderState[GFXRSCCWStencilFunc] = D3DRS_CCW_STENCILFUNC;
   GFXD3D9RenderState[GFXRSColorWriteEnable1] = D3DRS_COLORWRITEENABLE1;
   GFXD3D9RenderState[GFXRSColorWriteEnable2] = D3DRS_COLORWRITEENABLE2;
   GFXD3D9RenderState[GFXRSolorWriteEnable3] = D3DRS_COLORWRITEENABLE3;
   GFXD3D9RenderState[GFXRSBlendFactor] = D3DRS_BLENDFACTOR;
   GFXD3D9RenderState[GFXRSSRGBWriteEnable] = D3DRS_SRGBWRITEENABLE;
   GFXD3D9RenderState[GFXRSDepthBias] = D3DRS_DEPTHBIAS;
   GFXD3D9RenderState[GFXRSWrap8] = D3DRS_WRAP8;
   GFXD3D9RenderState[GFXRSWrap9] = D3DRS_WRAP9;
   GFXD3D9RenderState[GFXRSWrap10] = D3DRS_WRAP10;
   GFXD3D9RenderState[GFXRSWrap11] = D3DRS_WRAP11;
   GFXD3D9RenderState[GFXRSWrap12] = D3DRS_WRAP12;
   GFXD3D9RenderState[GFXRSWrap13] = D3DRS_WRAP13;
   GFXD3D9RenderState[GFXRSWrap14] = D3DRS_WRAP14;
   GFXD3D9RenderState[GFXRSWrap15] = D3DRS_WRAP15;
   GFXD3D9RenderState[GFXRSSeparateAlphaBlendEnable] = D3DRS_SEPARATEALPHABLENDENABLE;
   GFXD3D9RenderState[GFXRSSrcBlendAlpha] = D3DRS_SRCBLENDALPHA;
   GFXD3D9RenderState[GFXRSDestBlendAlpha] = D3DRS_DESTBLENDALPHA;
   GFXD3D9RenderState[GFXRSBlendOpAlpha] = D3DRS_BLENDOPALPHA;
   VALIDATE_LOOKUPTABLE( GFXD3D9RenderState, GFXRenderState );
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
   INIT_LOOKUPTABLE( GFXD3D9TextureFilter, GFXTextureFilter, _D3DTEXTUREFILTERTYPE );
   GFXD3D9TextureFilter[GFXTextureFilterNone] = D3DTEXF_NONE;
   GFXD3D9TextureFilter[GFXTextureFilterPoint] = D3DTEXF_POINT;
   GFXD3D9TextureFilter[GFXTextureFilterLinear] = D3DTEXF_LINEAR;
   GFXD3D9TextureFilter[GFXTextureFilterAnisotropic] = D3DTEXF_ANISOTROPIC;
   GFXD3D9TextureFilter[GFXTextureFilterPyramidalQuad] = D3DTEXF_PYRAMIDALQUAD;
   GFXD3D9TextureFilter[GFXTextureFilterGaussianQuad] = D3DTEXF_GAUSSIANQUAD;
   VALIDATE_LOOKUPTABLE( GFXD3D9TextureFilter, GFXTextureFilter );
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
   INIT_LOOKUPTABLE( GFXD3D9Blend, GFXBlend, _D3DBLEND );
   GFXD3D9Blend[GFXBlendZero] = D3DBLEND_ZERO;
   GFXD3D9Blend[GFXBlendOne] = D3DBLEND_ONE;
   GFXD3D9Blend[GFXBlendSrcColor] = D3DBLEND_SRCCOLOR;
   GFXD3D9Blend[GFXBlendInvSrcColor] = D3DBLEND_INVSRCCOLOR;
   GFXD3D9Blend[GFXBlendSrcAlpha] = D3DBLEND_SRCALPHA;
   GFXD3D9Blend[GFXBlendInvSrcAlpha] = D3DBLEND_INVSRCALPHA;
   GFXD3D9Blend[GFXBlendDestAlpha] = D3DBLEND_DESTALPHA;
   GFXD3D9Blend[GFXBlendInvDestAlpha] = D3DBLEND_INVDESTALPHA;
   GFXD3D9Blend[GFXBlendDestColor] = D3DBLEND_DESTCOLOR;
   GFXD3D9Blend[GFXBlendInvDestColor] = D3DBLEND_INVDESTCOLOR;
   GFXD3D9Blend[GFXBlendSrcAlphaSat] = D3DBLEND_SRCALPHASAT;
   VALIDATE_LOOKUPTABLE( GFXD3D9Blend, GFXBlend );
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
   INIT_LOOKUPTABLE( GFXD3D9BlendOp, GFXBlendOp, _D3DBLENDOP );
   GFXD3D9BlendOp[GFXBlendOpAdd] = D3DBLENDOP_ADD;
   GFXD3D9BlendOp[GFXBlendOpSubtract] = D3DBLENDOP_SUBTRACT;
   GFXD3D9BlendOp[GFXBlendOpRevSubtract] = D3DBLENDOP_REVSUBTRACT;
   GFXD3D9BlendOp[GFXBlendOpMin] = D3DBLENDOP_MIN;
   GFXD3D9BlendOp[GFXBlendOpMax] = D3DBLENDOP_MAX;
   VALIDATE_LOOKUPTABLE( GFXD3D9BlendOp, GFXBlendOp );
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
   INIT_LOOKUPTABLE( GFXD3D9StencilOp, GFXStencilOp, _D3DSTENCILOP );
   GFXD3D9StencilOp[GFXStencilOpKeep] = D3DSTENCILOP_KEEP;
   GFXD3D9StencilOp[GFXStencilOpZero] = D3DSTENCILOP_ZERO;
   GFXD3D9StencilOp[GFXStencilOpReplace] = D3DSTENCILOP_REPLACE;
   GFXD3D9StencilOp[GFXStencilOpIncrSat] = D3DSTENCILOP_INCRSAT;
   GFXD3D9StencilOp[GFXStencilOpDecrSat] = D3DSTENCILOP_DECRSAT;
   GFXD3D9StencilOp[GFXStencilOpInvert] = D3DSTENCILOP_INVERT;
   GFXD3D9StencilOp[GFXStencilOpIncr] = D3DSTENCILOP_INCR;
   GFXD3D9StencilOp[GFXStencilOpDecr] = D3DSTENCILOP_DECR;
   VALIDATE_LOOKUPTABLE( GFXD3D9StencilOp, GFXStencilOp );
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
   INIT_LOOKUPTABLE( GFXD3D9CmpFunc, GFXCmp, _D3DCMPFUNC );
   GFXD3D9CmpFunc[GFXCmpNever] = D3DCMP_NEVER;
   GFXD3D9CmpFunc[GFXCmpLess] = D3DCMP_LESS;
   GFXD3D9CmpFunc[GFXCmpEqual] = D3DCMP_EQUAL;
   GFXD3D9CmpFunc[GFXCmpLessEqual] = D3DCMP_LESSEQUAL;
   GFXD3D9CmpFunc[GFXCmpGreater] = D3DCMP_GREATER;
   GFXD3D9CmpFunc[GFXCmpNotEqual] = D3DCMP_NOTEQUAL;
   GFXD3D9CmpFunc[GFXCmpGreaterEqual] = D3DCMP_GREATEREQUAL;
   GFXD3D9CmpFunc[GFXCmpAlways] = D3DCMP_ALWAYS;
   VALIDATE_LOOKUPTABLE( GFXD3D9CmpFunc, GFXCmp );
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
   INIT_LOOKUPTABLE( GFXD3D9CullMode, GFXCull, _D3DCULL );
   GFXD3D9CullMode[GFXCullNone] = D3DCULL_NONE;
   GFXD3D9CullMode[GFXCullCW] = D3DCULL_CW;
   GFXD3D9CullMode[GFXCullCCW] = D3DCULL_CCW;
   VALIDATE_LOOKUPTABLE( GFXD3D9CullMode, GFXCull );
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
   INIT_LOOKUPTABLE( GFXD3D9FillMode, GFXFill, _D3DFILLMODE );
   GFXD3D9FillMode[GFXFillPoint] = D3DFILL_POINT;
   GFXD3D9FillMode[GFXFillWireframe] = D3DFILL_WIREFRAME;
   GFXD3D9FillMode[GFXFillSolid] = D3DFILL_SOLID;
   VALIDATE_LOOKUPTABLE( GFXD3D9FillMode, GFXFill );
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
   INIT_LOOKUPTABLE( GFXD3D9PrimType, GFXPT, _D3DPRIMITIVETYPE );
   GFXD3D9PrimType[GFXPointList] = D3DPT_POINTLIST;
   GFXD3D9PrimType[GFXLineList] = D3DPT_LINELIST;
   GFXD3D9PrimType[GFXLineStrip] = D3DPT_LINESTRIP;
   GFXD3D9PrimType[GFXTriangleList] = D3DPT_TRIANGLELIST;
   GFXD3D9PrimType[GFXTriangleStrip] = D3DPT_TRIANGLESTRIP;
   VALIDATE_LOOKUPTABLE( GFXD3D9PrimType, GFXPT );
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
   INIT_LOOKUPTABLE( GFXD3D9TextureStageState, GFXTSS, _D3DTEXTURESTAGESTATETYPE );
   GFXD3D9TextureStageState[GFXTSSColorOp] = D3DTSS_COLOROP;
   GFXD3D9TextureStageState[GFXTSSColorArg1] = D3DTSS_COLORARG1;
   GFXD3D9TextureStageState[GFXTSSColorArg2] = D3DTSS_COLORARG2;
   GFXD3D9TextureStageState[GFXTSSAlphaOp] = D3DTSS_ALPHAOP;
   GFXD3D9TextureStageState[GFXTSSAlphaArg1] = D3DTSS_ALPHAARG1;
   GFXD3D9TextureStageState[GFXTSSAlphaArg2] = D3DTSS_ALPHAARG2;
   GFXD3D9TextureStageState[GFXTSSBumpEnvMat00] = D3DTSS_BUMPENVMAT00;
   GFXD3D9TextureStageState[GFXTSSBumpEnvMat01] = D3DTSS_BUMPENVMAT01;
   GFXD3D9TextureStageState[GFXTSSBumpEnvMat10] = D3DTSS_BUMPENVMAT10;
   GFXD3D9TextureStageState[GFXTSSBumpEnvMat11] = D3DTSS_BUMPENVMAT11;
   GFXD3D9TextureStageState[GFXTSSTexCoordIndex] = D3DTSS_TEXCOORDINDEX;
   GFXD3D9TextureStageState[GFXTSSBumpEnvlScale] = D3DTSS_BUMPENVLSCALE;
   GFXD3D9TextureStageState[GFXTSSBumpEnvlOffset] = D3DTSS_BUMPENVLOFFSET;
   GFXD3D9TextureStageState[GFXTSSTextureTransformFlags] = D3DTSS_TEXTURETRANSFORMFLAGS;
   GFXD3D9TextureStageState[GFXTSSColorArg0] = D3DTSS_COLORARG0;
   GFXD3D9TextureStageState[GFXTSSAlphaArg0] = D3DTSS_ALPHAARG0;
   GFXD3D9TextureStageState[GFXTSSResultArg] = D3DTSS_RESULTARG;
   GFXD3D9TextureStageState[GFXTSSConstant] = D3DTSS_CONSTANT;
   VALIDATE_LOOKUPTABLE( GFXD3D9TextureStageState, GFXTSS );
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
   INIT_LOOKUPTABLE( GFXD3D9TextureAddress, GFXAddress, _D3DTEXTUREADDRESS );
   GFXD3D9TextureAddress[GFXAddressWrap] = D3DTADDRESS_WRAP ;
   GFXD3D9TextureAddress[GFXAddressMirror] = D3DTADDRESS_MIRROR;
   GFXD3D9TextureAddress[GFXAddressClamp] = D3DTADDRESS_CLAMP;
   GFXD3D9TextureAddress[GFXAddressBorder] = D3DTADDRESS_BORDER;
   GFXD3D9TextureAddress[GFXAddressMirrorOnce] = D3DTADDRESS_MIRRORONCE;
   VALIDATE_LOOKUPTABLE(GFXD3D9TextureAddress, GFXAddress );
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
   INIT_LOOKUPTABLE( GFXD3D9TextureOp, GFXTOP, _D3DTEXTUREOP );
   GFXD3D9TextureOp[GFXTOPDisable] = D3DTOP_DISABLE;
   GFXD3D9TextureOp[GFXTOPSelectARG1] = D3DTOP_SELECTARG1;
   GFXD3D9TextureOp[GFXTOPSelectARG2] = D3DTOP_SELECTARG2;
   GFXD3D9TextureOp[GFXTOPModulate] = D3DTOP_MODULATE;
   GFXD3D9TextureOp[GFXTOPModulate2X] = D3DTOP_MODULATE2X;
   GFXD3D9TextureOp[GFXTOPModulate4X] = D3DTOP_MODULATE4X;
   GFXD3D9TextureOp[GFXTOPAdd] = D3DTOP_ADD;
   GFXD3D9TextureOp[GFXTOPAddSigned] = D3DTOP_ADDSIGNED;
   GFXD3D9TextureOp[GFXTOPAddSigned2X] = D3DTOP_ADDSIGNED2X;
   GFXD3D9TextureOp[GFXTOPSubtract] = D3DTOP_SUBTRACT;
   GFXD3D9TextureOp[GFXTOPAddSmooth] = D3DTOP_ADDSMOOTH;
   GFXD3D9TextureOp[GFXTOPBlendDiffuseAlpha] = D3DTOP_BLENDDIFFUSEALPHA;
   GFXD3D9TextureOp[GFXTOPBlendTextureAlpha] = D3DTOP_BLENDTEXTUREALPHA;
   GFXD3D9TextureOp[GFXTOPBlendFactorAlpha] = D3DTOP_BLENDFACTORALPHA;
   GFXD3D9TextureOp[GFXTOPBlendTextureAlphaPM] = D3DTOP_BLENDTEXTUREALPHAPM;
   GFXD3D9TextureOp[GFXTOPBlendCURRENTALPHA] = D3DTOP_BLENDCURRENTALPHA;
   GFXD3D9TextureOp[GFXTOPPreModulate] = D3DTOP_PREMODULATE;
   GFXD3D9TextureOp[GFXTOPModulateAlphaAddColor] = D3DTOP_MODULATEALPHA_ADDCOLOR;
   GFXD3D9TextureOp[GFXTOPModulateColorAddAlpha] = D3DTOP_MODULATECOLOR_ADDALPHA;
   GFXD3D9TextureOp[GFXTOPModulateInvAlphaAddColor] = D3DTOP_MODULATEINVALPHA_ADDCOLOR;
   GFXD3D9TextureOp[GFXTOPModulateInvColorAddAlpha] = D3DTOP_MODULATEINVCOLOR_ADDALPHA;
   GFXD3D9TextureOp[GFXTOPBumpEnvMap] = D3DTOP_BUMPENVMAP;
   GFXD3D9TextureOp[GFXTOPBumpEnvMapLuminance] = D3DTOP_BUMPENVMAPLUMINANCE;
   GFXD3D9TextureOp[GFXTOPDotProduct3] = D3DTOP_DOTPRODUCT3;
   GFXD3D9TextureOp[GFXTOPLERP] = D3DTOP_LERP;
   VALIDATE_LOOKUPTABLE( GFXD3D9TextureOp, GFXTOP );
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
   INIT_LOOKUPTABLE( GFXD3D9DeclType, GFXDeclType, _D3DDECLTYPE );
   GFXD3D9DeclType[GFXDeclType_Float] = D3DDECLTYPE_FLOAT1;
   GFXD3D9DeclType[GFXDeclType_Float2] = D3DDECLTYPE_FLOAT2;
   GFXD3D9DeclType[GFXDeclType_Float3] = D3DDECLTYPE_FLOAT3;
   GFXD3D9DeclType[GFXDeclType_Float4] = D3DDECLTYPE_FLOAT4;
   GFXD3D9DeclType[GFXDeclType_Color] = D3DDECLTYPE_D3DCOLOR;
   GFXD3D9DeclType[GFXDeclType_UByte4] = D3DDECLTYPE_UBYTE4;
   VALIDATE_LOOKUPTABLE( GFXD3D9DeclType, GFXDeclType );
}

