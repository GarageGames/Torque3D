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

#include "gfx/D3D9/gfxD3D9Shader.h"

#include "gfx/gfxEnums.h"

//------------------------------------------------------------------------------

namespace GFXD3D9EnumTranslate
{
   void init();
};

//------------------------------------------------------------------------------

extern _D3DFORMAT GFXD3D9IndexFormat[GFXIndexFormat_COUNT];
extern _D3DSAMPLERSTATETYPE GFXD3D9SamplerState[GFXSAMP_COUNT];
extern _D3DFORMAT GFXD3D9TextureFormat[GFXFormat_COUNT];
#ifdef TORQUE_OS_XENON
extern _D3DFORMAT GFXD3D9RenderTargetFormat[GFXFormat_COUNT];
#endif
extern _D3DRENDERSTATETYPE GFXD3D9RenderState[GFXRenderState_COUNT];
extern _D3DTEXTUREFILTERTYPE GFXD3D9TextureFilter[GFXTextureFilter_COUNT];
extern _D3DBLEND GFXD3D9Blend[GFXBlend_COUNT];
extern _D3DBLENDOP GFXD3D9BlendOp[GFXBlendOp_COUNT];
extern _D3DSTENCILOP GFXD3D9StencilOp[GFXStencilOp_COUNT];
extern _D3DCMPFUNC GFXD3D9CmpFunc[GFXCmp_COUNT];
extern _D3DCULL GFXD3D9CullMode[GFXCull_COUNT];
extern _D3DFILLMODE GFXD3D9FillMode[GFXFill_COUNT];
extern _D3DPRIMITIVETYPE GFXD3D9PrimType[GFXPT_COUNT];
extern _D3DTEXTURESTAGESTATETYPE GFXD3D9TextureStageState[GFXTSS_COUNT];
extern _D3DTEXTUREADDRESS GFXD3D9TextureAddress[GFXAddress_COUNT];
extern _D3DTEXTUREOP GFXD3D9TextureOp[GFXTOP_COUNT];
extern _D3DDECLTYPE GFXD3D9DeclType[GFXDeclType_COUNT];

#define GFXREVERSE_LOOKUP( tablearray, enumprefix, val ) \
   for( S32 i = enumprefix##_FIRST; i < enumprefix##_COUNT; i++ ) \
      if( (S32)tablearray[i] == val ) \
      { \
         val = i; \
         break; \
      } \

