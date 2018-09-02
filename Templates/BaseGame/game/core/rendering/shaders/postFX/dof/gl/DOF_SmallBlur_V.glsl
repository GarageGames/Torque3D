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

#include "../../../gl/hlslCompat.glsl"
#include "../../../gl/torque.glsl"

in vec4 vPosition;
in vec2 vTexCoord0;

#define IN_position  vPosition
#define IN_texCoords vTexCoord0

#define OUT_position gl_Position
out vec4 texCoords;
#define OUT_texCoords texCoords

uniform vec2 oneOverTargetSize;  
uniform vec4 rtParams0;

void main()  
{  
   const vec4 halfPixel = vec4( -0.5, 0.5, -0.5, 0.5 );     
   OUT_position = IN_position; //Transform_ObjectToClip( IN_position );  
   
   //vec2 uv = IN_texCoords + rtParams0.xy;
   vec2 uv = viewportCoordToRenderTarget( IN_texCoords, rtParams0 );
   OUT_texCoords = uv.xxyy + halfPixel * oneOverTargetSize.xxyy; 
   
   correctSSP(gl_Position);
}  