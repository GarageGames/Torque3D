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

// CornerId corresponds to this arrangement
// from the perspective of the camera.
//
//    3 ---- 2
//    |      |
//    0 ---- 1
//

#define MAX_COVERTYPES 8

uniform float2 gc_fadeParams;
uniform float2 gc_windDir;
uniform float3 gc_camRight;
uniform float3 gc_camUp;
uniform float4 gc_typeRects[MAX_COVERTYPES];

// .x = gust length
// .y = premultiplied simulation time and gust frequency
// .z = gust strength
uniform float3 gc_gustInfo;

// .x = premultiplied simulation time and turbulance frequency
// .y = turbulance strength
uniform float2 gc_turbInfo;


static float sCornerRight[4] = { -0.5, 0.5, 0.5, -0.5 };

static float sCornerUp[4] = { 0, 0, 1, 1 };

static float sMovableCorner[4] = { 0, 0, 1, 1 };

static float2 sUVCornerExtent[4] =
{ 
   float2( 0, 1 ),
   float2( 1, 1 ), 
   float2( 1, 0 ), 
   float2( 0, 0 )
};


///////////////////////////////////////////////////////////////////////////////
// The following wind effect was derived from the GPU Gems 3 chapter...
//
// "Vegetation Procedural Animation and Shading in Crysis"
// by Tiago Sousa, Crytek
//

float2 smoothCurve( float2 x )
{
   return x * x * ( 3.0 - 2.0 * x );
}

float2 triangleWave( float2 x )
{
   return abs( frac( x + 0.5 ) * 2.0 - 1.0 );
}

float2 smoothTriangleWave( float2 x )
{
   return smoothCurve( triangleWave( x ) );
}

float windTurbulence( float bbPhase, float frequency, float strength )
{
   // We create the input value for wave generation from the frequency and phase.
   float2 waveIn = bbPhase.xx + frequency.xx;

   // We use two square waves to generate the effect which
   // is then scaled by the overall strength.
   float2 waves = ( frac( waveIn.xy * float2( 1.975, 0.793 ) ) * 2.0 - 1.0 );
   waves = smoothTriangleWave( waves );

   // Sum up the two waves into a single wave.
   return ( waves.x + waves.y ) * strength;
}

float2 windEffect(   float bbPhase, 
                     float2 windDirection,
                     float gustLength,
                     float gustFrequency,
                     float gustStrength,
                     float turbFrequency,
                     float turbStrength )
{
   // Calculate the ambient wind turbulence.
   float turbulence = windTurbulence( bbPhase, turbFrequency, turbStrength );

   // We simulate the overall gust via a sine wave.
   float gustPhase = clamp( sin( ( bbPhase - gustFrequency ) / gustLength ) , 0, 1 );
   float gustOffset = ( gustPhase * gustStrength ) + ( ( 0.2 + gustPhase ) * turbulence );

   // Return the final directional wind effect.
   return gustOffset.xx * windDirection.xy;
}
   
void foliageProcessVert( inout float3 position, 
                         inout float4 diffuse, 
                         inout float4 texCoord, 
                         inout float3 normal, 
                         inout float3 T,
                         in float3 eyePos )
{  
   // Assign the normal and tagent values.
   //normal = float3( 0, 0, 1 );//cross( gc_camUp, gc_camRight );
   T = gc_camRight;
   
   // Pull out local vars we need for work.
   int corner = ( diffuse.a * 255.0f ) + 0.5f;
   float2 size = texCoord.xy;
   int type = texCoord.z;            
           
   // The billboarding is based on the camera direction.
   float3 rightVec   = gc_camRight * sCornerRight[corner];
   float3 upVec      = gc_camUp * sCornerUp[corner];               
   
   // Figure out the corner position.     
   float3 outPos = ( upVec * size.y ) + ( rightVec * size.x );
   float len = length( outPos.xyz );
   
   // We derive the billboard phase used for wind calculations from its position.
   float bbPhase = dot( position.xyz, 1 );

   // Get the overall wind gust and turbulence effects.
   float3 wind;
   wind.xy = windEffect(   bbPhase,
                           gc_windDir,
                           gc_gustInfo.x, gc_gustInfo.y, gc_gustInfo.z,
                           gc_turbInfo.x, gc_turbInfo.y );
   wind.z = 0;

   // Add the summed wind effect into the point.
   outPos.xyz += wind.xyz * texCoord.w;

   // Do a simple spherical clamp to keep the foliage
   // from stretching too much by wind effect.
   outPos.xyz = normalize( outPos.xyz ) * len;

   // Move the point into world space.
   position += outPos;      

   // Grab the uv set and setup the texture coord.
   float4 uvSet = gc_typeRects[type]; 
   texCoord.x = uvSet.x + ( uvSet.z * sUVCornerExtent[corner].x );
   texCoord.y = uvSet.y + ( uvSet.w * sUVCornerExtent[corner].y );

   // Animate the normal to get lighting changes
   // across the the wind swept foliage.
   // 
   // TODO: Expose the 10x as a factor to control
   // how much the wind effects the lighting on the grass.
   //
   normal.xy += wind.xy * ( 10.0 * texCoord.w );
   normal = normalize( normal );

   // Get the alpha fade value.
   
   float    fadeStart      = gc_fadeParams.x;
   float    fadeEnd        = gc_fadeParams.y;
   const float fadeRange   = fadeEnd - fadeStart;     
   
   float dist = distance( eyePos, position.xyz ) - fadeStart;
   diffuse.a = 1 - clamp( dist / fadeRange, 0, 1 );     
}