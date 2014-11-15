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

#include "../terrain.glsl"
#include "../../gl/hlslCompat.glsl"

in vec2 layerCoord;
#define IN_layerCoord layerCoord
in vec2 texCoord;
#define IN_texCoord texCoord

uniform sampler2D layerTex;
uniform sampler2D textureMap;
uniform float texId;
uniform float layerSize;

out vec4 OUT_FragColor0;

void main()
{
   vec4 layerSample = round(texture( layerTex, IN_layerCoord ) * 255.0);

   float blend = calcBlend( texId, IN_layerCoord, layerSize, layerSample );

   if(blend - 0.0001 < 0.0)
      discard;

   OUT_FragColor0 = vec4( texture( textureMap, IN_texCoord ).rgb, blend );
}
