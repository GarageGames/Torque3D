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

// This vertex and pixel shader applies a 3 x 3 blur to the image in  
// colorMapSampler, which is the same size as the render target.  
// The sample weights are 1/16 in the corners, 2/16 on the edges,  
// and 4/16 in the center.  
#include "../../shaderModel.hlsl"

TORQUE_UNIFORM_SAMPLER2D(colorSampler, 0); // Output of DofNearCoc()

struct Pixel
{  
   float4 position : TORQUE_POSITION;  
   float4 texCoords : TEXCOORD0;  
};  

float4 main( Pixel IN ) : TORQUE_TARGET0
{  
   float4 color;  
   color = 0.0;  
   color += TORQUE_TEX2D( colorSampler, IN.texCoords.xz );  
   color += TORQUE_TEX2D( colorSampler, IN.texCoords.yz );  
   color += TORQUE_TEX2D( colorSampler, IN.texCoords.xw );  
   color += TORQUE_TEX2D( colorSampler, IN.texCoords.yw );  
   return color / 4.0;  
}  