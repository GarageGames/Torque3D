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

#include "../postFx.hlsl"
#include "shadergen:/autogenConditioners.h"

uniform sampler2D currLum : register( S0 );
uniform sampler2D lastAdaptedLum : register( S1 );

uniform float adaptRate;
uniform float deltaTime;

float4 main( PFXVertToPix IN ) : COLOR
{
   float fAdaptedLum = tex2D( lastAdaptedLum, float2(0.5f, 0.5f) ).r;
   float fCurrentLum = tex2D( currLum, float2(0.5f, 0.5f) ).r;

   // The user's adapted luminance level is simulated by closing the gap between
   // adapted luminance and current luminance by 2% every frame, based on a
   // 30 fps rate. This is not an accurate model of human adaptation, which can
   // take longer than half an hour.
   float diff = fCurrentLum - fAdaptedLum;
   float fNewAdaptation = fAdaptedLum + ( diff * ( 1.0 - exp( -deltaTime * adaptRate ) ) );

   return float4( fNewAdaptation, 0.0, 0.0, 1.0f );
}
