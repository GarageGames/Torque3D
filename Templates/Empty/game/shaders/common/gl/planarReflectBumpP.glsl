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
// Data
//-----------------------------------------------------------------------------
uniform sampler2D diffuseMap, refractMap, bumpMap;
uniform vec4 shadeColor;

in vec2 TEX0;
in vec4 TEX1;

out vec4 OUT_col;

//-----------------------------------------------------------------------------
// Fade edges of axis for texcoord passed in
//-----------------------------------------------------------------------------
float fadeAxis( float val )
{
   // Fades from 1.0 to 0.0 when less than 0.1
   float fadeLow = clamp( val * 10.0, 0.0, 1.0 );
   
   // Fades from 1.0 to 0.0 when greater than 0.9
   float fadeHigh = 1.0 - clamp( (val - 0.9) * 10.0, 0.0, 1.0 );

   return fadeLow * fadeHigh;
}


//-----------------------------------------------------------------------------
// Main                                                                        
//-----------------------------------------------------------------------------
void main()
{
   vec3 bumpNorm = texture( bumpMap, TEX0 ).rgb * 2.0 - 1.0;
   vec2 offset = vec2( bumpNorm.x, bumpNorm.y );
   vec4 texIndex = TEX1;

   // The fadeVal is used to "fade" the distortion at the edges of the screen.
   // This is done so it won't sample the reflection texture out-of-bounds and create artifacts
   // Note - this can be done more efficiently with a texture lookup
   float fadeVal = fadeAxis( texIndex.x / texIndex.w ) * fadeAxis( texIndex.y / texIndex.w );

   const float distortion = 0.2;
   texIndex.xy += offset * distortion * fadeVal;

   vec4 diffuseColor = texture( diffuseMap, TEX0 );
   vec4 reflectColor = textureProj( refractMap, texIndex );

   OUT_col = diffuseColor + reflectColor * diffuseColor.a;
}
