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

#define IN_GLSL
#include "../../../shdrConsts.h"
#include "../../../gl/hlslCompat.glsl"
#include "shadergen:/autogenConditioners.h"

in vec4 texCoords[8];
#define IN_texCoords texCoords

uniform sampler2D inputTex;

out vec4 OUT_col;

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------
void main()
{
   // We calculate the texture coords
   // in the vertex shader as an optimization.
   vec4 _sample = vec4(0.0f);
   for ( int i = 0; i < 8; i++ )
   {
      _sample += texture( inputTex, IN_texCoords[i].xy );
      _sample += texture( inputTex, IN_texCoords[i].zw );
   }
   
	OUT_col = _sample / 16;
}