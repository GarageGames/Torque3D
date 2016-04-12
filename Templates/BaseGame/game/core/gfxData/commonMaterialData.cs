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

//-----------------------------------------------------------------------------
// Anim flag settings - must match material.h
// These cannot be enumed through script becuase it cannot
// handle the "|" operation for combining them together
// ie. Scroll | Wave does not work.
//-----------------------------------------------------------------------------
$scroll = 1;
$rotate = 2;
$wave   = 4;
$scale  = 8;
$sequence = 16;


// Common stateblock definitions
new GFXSamplerStateData(SamplerClampLinear)
{
   textureColorOp = GFXTOPModulate;
   addressModeU = GFXAddressClamp;
   addressModeV = GFXAddressClamp;
   addressModeW = GFXAddressClamp;
   magFilter = GFXTextureFilterLinear;
   minFilter = GFXTextureFilterLinear;
   mipFilter = GFXTextureFilterLinear;
};

new GFXSamplerStateData(SamplerClampPoint)
{
   textureColorOp = GFXTOPModulate;
   addressModeU = GFXAddressClamp;
   addressModeV = GFXAddressClamp;
   addressModeW = GFXAddressClamp;
   magFilter = GFXTextureFilterPoint;
   minFilter = GFXTextureFilterPoint;
   mipFilter = GFXTextureFilterPoint;
};

new GFXSamplerStateData(SamplerWrapLinear)
{
   textureColorOp = GFXTOPModulate;
   addressModeU = GFXTextureAddressWrap;
   addressModeV = GFXTextureAddressWrap;
   addressModeW = GFXTextureAddressWrap;
   magFilter = GFXTextureFilterLinear;
   minFilter = GFXTextureFilterLinear;
   mipFilter = GFXTextureFilterLinear;
};

new GFXSamplerStateData(SamplerWrapPoint)
{
   textureColorOp = GFXTOPModulate;
   addressModeU = GFXTextureAddressWrap;
   addressModeV = GFXTextureAddressWrap;
   addressModeW = GFXTextureAddressWrap;
   magFilter = GFXTextureFilterPoint;
   minFilter = GFXTextureFilterPoint;
   mipFilter = GFXTextureFilterPoint;
};
