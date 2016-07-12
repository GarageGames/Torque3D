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


#ifndef _GFXD3D11ENUMTRANSLATE_H_
#define _GFXD3D11ENUMTRANSLATE_H_

#include "gfx/D3D11/gfxD3D11Shader.h"
#include "gfx/gfxEnums.h"

//------------------------------------------------------------------------------

namespace GFXD3D11EnumTranslate
{
   void init();
};

//------------------------------------------------------------------------------

extern DXGI_FORMAT GFXD3D11TextureFormat[GFXFormat_COUNT];
extern D3D11_FILTER GFXD3D11TextureFilter[GFXTextureFilter_COUNT];
extern D3D11_BLEND GFXD3D11Blend[GFXBlend_COUNT];
extern D3D11_BLEND_OP GFXD3D11BlendOp[GFXBlendOp_COUNT];
extern D3D11_STENCIL_OP GFXD3D11StencilOp[GFXStencilOp_COUNT];
extern D3D11_COMPARISON_FUNC GFXD3D11CmpFunc[GFXCmp_COUNT];
extern D3D11_CULL_MODE GFXD3D11CullMode[GFXCull_COUNT];
extern D3D11_FILL_MODE GFXD3D11FillMode[GFXFill_COUNT];
extern D3D11_PRIMITIVE_TOPOLOGY GFXD3D11PrimType[GFXPT_COUNT];
extern D3D11_TEXTURE_ADDRESS_MODE GFXD3D11TextureAddress[GFXAddress_COUNT];
extern DXGI_FORMAT GFXD3D11DeclType[GFXDeclType_COUNT];

#endif