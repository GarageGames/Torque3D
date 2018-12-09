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

float4 smoothCurve( float4 x )
{
   return x * x * ( 3.0 - 2.0 * x );
}

float4 triangleWave( float4 x )
{
   return abs( frac( x + 0.5 ) * 2.0 - 1.0 );
}

float4 smoothTriangleWave( float4 x )
{
   return smoothCurve( triangleWave( x ) );
}

float3 windTrunkBending( float3 vPos, float2 vWind, float fBendFactor )
{
   // Smooth the bending factor and increase 
   // the near by height limit.
   fBendFactor += 1.0;
   fBendFactor *= fBendFactor;
   fBendFactor = fBendFactor * fBendFactor - fBendFactor;

   // Displace the vert.
   float3 vNewPos = vPos;
   vNewPos.xy += vWind * fBendFactor;

   // Limit the length which makes the bend more 
   // spherical and prevents stretching.
   float fLength = length( vPos );
   vPos = normalize( vNewPos ) * fLength;

   return vPos;
}

float3 windBranchBending(  float3 vPos,
                           float3 vNormal,

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
   float fVertPhase = dot( vPos, fDetailPhase + fBranchPhase );

   float2 vWavesIn = fTime + float2( fVertPhase, fBranchPhase );

   float4 vWaves = ( frac( vWavesIn.xxyy *
                           float4( 1.975, 0.793, 0.375, 0.193 ) ) *
                           2.0 - 1.0 ) * fWindSpeed * fDetailFreq;

   vWaves = smoothTriangleWave( vWaves );

   float2 vWavesSum = vWaves.xz + vWaves.yw;

   // We want the branches to bend both up and down.
   vWavesSum.y = 1 - ( vWavesSum.y * 2 );

   vPos += vWavesSum.xxy * float3(  fEdgeAtten * fDetailAmp * vNormal.xy,
                                    fBranchAtten * fBranchAmp );

   return vPos;
}
