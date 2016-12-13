//-----------------------------------------------------------------------------
// Copyright (c) 2015 GarageGames, LLC
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

#include "gfx/D3D11/gfxD3D11Device.h"
#include "gfx/D3D11/gfxD3D11EnumTranslate.h"
#include "console/console.h"

//------------------------------------------------------------------------------

DXGI_FORMAT GFXD3D11TextureFormat[GFXFormat_COUNT];
D3D11_FILTER GFXD3D11TextureFilter[GFXTextureFilter_COUNT];
D3D11_BLEND GFXD3D11Blend[GFXBlend_COUNT];
D3D11_BLEND_OP GFXD3D11BlendOp[GFXBlendOp_COUNT];
D3D11_STENCIL_OP GFXD3D11StencilOp[GFXStencilOp_COUNT];
D3D11_COMPARISON_FUNC GFXD3D11CmpFunc[GFXCmp_COUNT];
D3D11_CULL_MODE GFXD3D11CullMode[GFXCull_COUNT];
D3D11_FILL_MODE GFXD3D11FillMode[GFXFill_COUNT];
D3D11_PRIMITIVE_TOPOLOGY GFXD3D11PrimType[GFXPT_COUNT];
D3D11_TEXTURE_ADDRESS_MODE GFXD3D11TextureAddress[GFXAddress_COUNT];
DXGI_FORMAT GFXD3D11DeclType[GFXDeclType_COUNT];

//------------------------------------------------------------------------------

void GFXD3D11EnumTranslate::init()
{
   GFXD3D11TextureFormat[GFXFormatR8G8B8] = DXGI_FORMAT_B8G8R8X8_UNORM;
   GFXD3D11TextureFormat[GFXFormatR8G8B8A8] = DXGI_FORMAT_B8G8R8A8_UNORM;
   GFXD3D11TextureFormat[GFXFormatR8G8B8X8] = DXGI_FORMAT_B8G8R8X8_UNORM;
   GFXD3D11TextureFormat[GFXFormatB8G8R8A8] = DXGI_FORMAT_B8G8R8A8_UNORM;
   GFXD3D11TextureFormat[GFXFormatR5G6B5] = DXGI_FORMAT_B5G6R5_UNORM;
   GFXD3D11TextureFormat[GFXFormatR5G5B5A1] = DXGI_FORMAT_B5G5R5A1_UNORM;
   GFXD3D11TextureFormat[GFXFormatR5G5B5X1] = DXGI_FORMAT_UNKNOWN;
   GFXD3D11TextureFormat[GFXFormatR32F] = DXGI_FORMAT_R32_FLOAT;
   GFXD3D11TextureFormat[GFXFormatA4L4] = DXGI_FORMAT_UNKNOWN;
   GFXD3D11TextureFormat[GFXFormatA8L8] = DXGI_FORMAT_R8G8_UNORM;
   GFXD3D11TextureFormat[GFXFormatA8] = DXGI_FORMAT_A8_UNORM;
   GFXD3D11TextureFormat[GFXFormatL8] = DXGI_FORMAT_R8_UNORM;
   GFXD3D11TextureFormat[GFXFormatDXT1] = DXGI_FORMAT_BC1_UNORM;
   GFXD3D11TextureFormat[GFXFormatDXT2] = DXGI_FORMAT_BC1_UNORM;
   GFXD3D11TextureFormat[GFXFormatDXT3] = DXGI_FORMAT_BC2_UNORM;
   GFXD3D11TextureFormat[GFXFormatDXT4] = DXGI_FORMAT_BC2_UNORM;
   GFXD3D11TextureFormat[GFXFormatDXT5] = DXGI_FORMAT_BC3_UNORM;
   GFXD3D11TextureFormat[GFXFormatR32G32B32A32F] = DXGI_FORMAT_R32G32B32A32_FLOAT;
   GFXD3D11TextureFormat[GFXFormatR16G16B16A16F] = DXGI_FORMAT_R16G16B16A16_FLOAT;
   GFXD3D11TextureFormat[GFXFormatL16] = DXGI_FORMAT_R16_UNORM;
   GFXD3D11TextureFormat[GFXFormatR16G16B16A16] = DXGI_FORMAT_R16G16B16A16_UNORM;
   GFXD3D11TextureFormat[GFXFormatR16G16] = DXGI_FORMAT_R16G16_UNORM;
   GFXD3D11TextureFormat[GFXFormatR16F] = DXGI_FORMAT_R16_FLOAT;
   GFXD3D11TextureFormat[GFXFormatR16G16F] = DXGI_FORMAT_R16G16_FLOAT;
   GFXD3D11TextureFormat[GFXFormatR10G10B10A2] = DXGI_FORMAT_R10G10B10A2_UNORM;
   GFXD3D11TextureFormat[GFXFormatD32] = DXGI_FORMAT_UNKNOWN;
   GFXD3D11TextureFormat[GFXFormatD24X8] = DXGI_FORMAT_UNKNOWN;
   GFXD3D11TextureFormat[GFXFormatD24S8] = DXGI_FORMAT_D24_UNORM_S8_UINT;
   GFXD3D11TextureFormat[GFXFormatD24FS8] = DXGI_FORMAT_UNKNOWN;
   GFXD3D11TextureFormat[GFXFormatD16] = DXGI_FORMAT_D16_UNORM;
   GFXD3D11TextureFormat[GFXFormatR8G8B8A8_SRGB] = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
   GFXD3D11TextureFormat[GFXFormatR8G8B8A8_LINEAR_FORCE] = DXGI_FORMAT_R8G8B8A8_UNORM;
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
   GFXD3D11TextureFilter[GFXTextureFilterNone] = D3D11_FILTER_MIN_MAG_MIP_POINT;
   GFXD3D11TextureFilter[GFXTextureFilterPoint] = D3D11_FILTER_MIN_MAG_MIP_POINT;
   GFXD3D11TextureFilter[GFXTextureFilterLinear] = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
   GFXD3D11TextureFilter[GFXTextureFilterAnisotropic] = D3D11_FILTER_ANISOTROPIC;
   GFXD3D11TextureFilter[GFXTextureFilterPyramidalQuad] = D3D11_FILTER_ANISOTROPIC;
   GFXD3D11TextureFilter[GFXTextureFilterGaussianQuad] = D3D11_FILTER_ANISOTROPIC;
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
   GFXD3D11Blend[GFXBlendZero] = D3D11_BLEND_ZERO;
   GFXD3D11Blend[GFXBlendOne] = D3D11_BLEND_ONE;
   GFXD3D11Blend[GFXBlendSrcColor] = D3D11_BLEND_SRC_COLOR;
   GFXD3D11Blend[GFXBlendInvSrcColor] = D3D11_BLEND_INV_SRC_COLOR;
   GFXD3D11Blend[GFXBlendSrcAlpha] = D3D11_BLEND_SRC_ALPHA;
   GFXD3D11Blend[GFXBlendInvSrcAlpha] = D3D11_BLEND_INV_SRC_ALPHA;
   GFXD3D11Blend[GFXBlendDestAlpha] = D3D11_BLEND_DEST_ALPHA;
   GFXD3D11Blend[GFXBlendInvDestAlpha] = D3D11_BLEND_INV_DEST_ALPHA;
   GFXD3D11Blend[GFXBlendDestColor] = D3D11_BLEND_DEST_COLOR;
   GFXD3D11Blend[GFXBlendInvDestColor] = D3D11_BLEND_INV_DEST_COLOR;
   GFXD3D11Blend[GFXBlendSrcAlphaSat] = D3D11_BLEND_SRC_ALPHA_SAT;
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
   GFXD3D11BlendOp[GFXBlendOpAdd] = D3D11_BLEND_OP_ADD;
   GFXD3D11BlendOp[GFXBlendOpSubtract] = D3D11_BLEND_OP_SUBTRACT;
   GFXD3D11BlendOp[GFXBlendOpRevSubtract] = D3D11_BLEND_OP_REV_SUBTRACT;
   GFXD3D11BlendOp[GFXBlendOpMin] = D3D11_BLEND_OP_MIN;
   GFXD3D11BlendOp[GFXBlendOpMax] = D3D11_BLEND_OP_MAX;
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
   GFXD3D11StencilOp[GFXStencilOpKeep] = D3D11_STENCIL_OP_KEEP;
   GFXD3D11StencilOp[GFXStencilOpZero] = D3D11_STENCIL_OP_ZERO;
   GFXD3D11StencilOp[GFXStencilOpReplace] = D3D11_STENCIL_OP_REPLACE;
   GFXD3D11StencilOp[GFXStencilOpIncrSat] = D3D11_STENCIL_OP_INCR_SAT;
   GFXD3D11StencilOp[GFXStencilOpDecrSat] = D3D11_STENCIL_OP_DECR_SAT;
   GFXD3D11StencilOp[GFXStencilOpInvert] = D3D11_STENCIL_OP_INVERT;
   GFXD3D11StencilOp[GFXStencilOpIncr] = D3D11_STENCIL_OP_INCR;
   GFXD3D11StencilOp[GFXStencilOpDecr] = D3D11_STENCIL_OP_DECR;
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
   GFXD3D11CmpFunc[GFXCmpNever] = D3D11_COMPARISON_NEVER;
   GFXD3D11CmpFunc[GFXCmpLess] = D3D11_COMPARISON_LESS;
   GFXD3D11CmpFunc[GFXCmpEqual] = D3D11_COMPARISON_EQUAL;
   GFXD3D11CmpFunc[GFXCmpLessEqual] = D3D11_COMPARISON_LESS_EQUAL;
   GFXD3D11CmpFunc[GFXCmpGreater] = D3D11_COMPARISON_GREATER;
   GFXD3D11CmpFunc[GFXCmpNotEqual] = D3D11_COMPARISON_NOT_EQUAL;
   GFXD3D11CmpFunc[GFXCmpGreaterEqual] = D3D11_COMPARISON_GREATER_EQUAL;
   GFXD3D11CmpFunc[GFXCmpAlways] = D3D11_COMPARISON_ALWAYS;
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
   GFXD3D11CullMode[GFXCullNone] = D3D11_CULL_NONE;
   GFXD3D11CullMode[GFXCullCW] = D3D11_CULL_FRONT;
   GFXD3D11CullMode[GFXCullCCW] = D3D11_CULL_BACK;
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
   GFXD3D11FillMode[GFXFillPoint] = D3D11_FILL_SOLID;
   GFXD3D11FillMode[GFXFillWireframe] = D3D11_FILL_WIREFRAME;
   GFXD3D11FillMode[GFXFillSolid] = D3D11_FILL_SOLID;
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
   GFXD3D11PrimType[GFXPointList] = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
   GFXD3D11PrimType[GFXLineList] = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
   GFXD3D11PrimType[GFXLineStrip] = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
   GFXD3D11PrimType[GFXTriangleList] = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
   GFXD3D11PrimType[GFXTriangleStrip] = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
   GFXD3D11TextureAddress[GFXAddressWrap] = D3D11_TEXTURE_ADDRESS_WRAP;
   GFXD3D11TextureAddress[GFXAddressMirror] = D3D11_TEXTURE_ADDRESS_MIRROR;
   GFXD3D11TextureAddress[GFXAddressClamp] = D3D11_TEXTURE_ADDRESS_CLAMP;
   GFXD3D11TextureAddress[GFXAddressBorder] = D3D11_TEXTURE_ADDRESS_BORDER;
   GFXD3D11TextureAddress[GFXAddressMirrorOnce] = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
   GFXD3D11DeclType[GFXDeclType_Float] = DXGI_FORMAT_R32_FLOAT;
   GFXD3D11DeclType[GFXDeclType_Float2] = DXGI_FORMAT_R32G32_FLOAT;
   GFXD3D11DeclType[GFXDeclType_Float3] = DXGI_FORMAT_R32G32B32_FLOAT;
   GFXD3D11DeclType[GFXDeclType_Float4] = DXGI_FORMAT_R32G32B32A32_FLOAT;
   GFXD3D11DeclType[GFXDeclType_Color] =  DXGI_FORMAT_B8G8R8A8_UNORM; // DXGI_FORMAT_R8G8B8A8_UNORM;
   GFXD3D11DeclType[GFXDeclType_UByte4] = DXGI_FORMAT_R8G8B8A8_UINT;
}

