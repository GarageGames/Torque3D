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

//#define SM_Fmt_R8G8B8A8

#define pkDepthBitShft 65536.0
#define pkDepthChanMax 256.0
#define bias -0.5/255.0
#define coeff 0.9999991
//#define coeff 1.0

float4 encodeShadowMap( float depth )
{
#if defined(SM_Fmt_R8G8B8A8)   
   return frac( float4(1.0, 255.0, 65025.0, 160581375.0) * depth ) + bias;

   //float4 packedValue = frac((depth / coeff) * float4(16777216.0, 65536.0, 256.0, 1.0));
   //return (packedValue - packedValue.xxyz * float4(0, 1.0 / 256, 1.0 / 256, 1.0 / 256));
#else
   return depth;
#endif
}

float decodeShadowMap( float4 smSample )
{
#if defined(SM_Fmt_R8G8B8A8)
   return dot( smSample, float4(1.0, 1/255.0, 1/65025.0, 1/160581375.0) );
#else
   return smSample.x;  
#endif
}
