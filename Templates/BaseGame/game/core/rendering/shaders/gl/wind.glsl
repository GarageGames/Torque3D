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

//
// A tip of the hat....
//
// The following wind effects were derived from the GPU Gems 
// 3 chapter "Vegetation Procedural Animation and Shading in Crysis"
// by Tiago Sousa of Crytek.
//

vec4 smoothCurve( vec4 x )
{
   return x * x * ( 3.0 - 2.0 * x );
}

vec4 triangleWave( vec4 x )
{
   return abs( fract( x + 0.5 ) * 2.0 - 1.0 );
}

vec4 smoothTriangleWave( vec4 x )
{
   return smoothCurve( triangleWave( x ) );
}

vec3 windTrunkBending( vec3 vPos, vec2 vWind, float fBendFactor )
{
   // Smooth the bending factor and increase 
   // the near by height limit.
   fBendFactor += 1.0;
   fBendFactor *= fBendFactor;
   fBendFactor = fBendFactor * fBendFactor - fBendFactor;

   // Displace the vert.
   vec3 vNewPos = vPos;
   vNewPos.xy += vWind * fBendFactor;

   // Limit the length which makes the bend more 
   // spherical and prevents stretching.
   float fLength = length( vPos );
   vPos = normalize( vNewPos ) * fLength;

   return vPos;
}

vec3 windBranchBending(  vec3 vPos,
                           vec3 vNormal,

                           float fTime, 
                           float fWindSpeed,

                           float fBranchPhase,
                           float fBranchAmp,
                           float fBranchAtten,

                           float fDetailPhase,
                           float fDetailAmp,
                           float fDetailFreq,

                           float fEdgeAtten )
{
   float fVertPhase = dot( vPos, vec3( fDetailPhase + fBranchPhase ) );

   vec2 vWavesIn = fTime + vec2( fVertPhase, fBranchPhase );

   vec4 vWaves = ( fract( vWavesIn.xxyy *
                           vec4( 1.975, 0.793, 0.375, 0.193 ) ) *
                           2.0 - 1.0 ) * fWindSpeed * fDetailFreq;

   vWaves = smoothTriangleWave( vWaves );

   vec2 vWavesSum = vWaves.xz + vWaves.yw;

   // We want the branches to bend both up and down.
   vWavesSum.y = 1.0 - ( vWavesSum.y * 2.0 );

   vPos += vWavesSum.xxy * vec3(  fEdgeAtten * fDetailAmp * vNormal.xy,
                                    fBranchAtten * fBranchAmp );

   return vPos;
}
