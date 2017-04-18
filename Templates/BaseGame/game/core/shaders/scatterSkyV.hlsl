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

#include "shaderModel.hlsl"

// The scale equation calculated by Vernier's Graphical Analysis
float vernierScale(float fCos)
{
	float x = 1.0 - fCos;
	float x5 = x * 5.25;
	float x5p6 = (-6.80 + x5);
	float xnew = (3.83 + x * x5p6);
	float xfinal = (0.459 + x * xnew);
	float xfinal2 = -0.00287 + x * xfinal;
	float outx = exp( xfinal2 ); 
	return 0.25 * outx;
}

// This is the shader input vertex structure.
struct Vert
{
   // .xyz  = point
   float3 position : POSITION; 
};

// This is the shader output data.
struct Conn
{
   float4 position : TORQUE_POSITION;
   float4 rayleighColor : TEXCOORD0;
   float4 mieColor : TEXCOORD1;
   float3 v3Direction : TEXCOORD2;
   float3 pos : TEXCOORD3;
};
 
float3 desaturate(const float3 color, const float desaturation) 
{  
   const float3 gray_conv = float3 (0.30, 0.59, 0.11);  
   return lerp(color, dot(gray_conv , color), desaturation);  
}  
  
uniform float4x4 modelView;
uniform float4 misc;
uniform float4 sphereRadii;
uniform float4 scatteringCoeffs;
uniform float4 colorize;
uniform float3 camPos;
uniform float3 lightDir;
uniform float4 invWaveLength;

Conn main(  Vert In  )	         
{
   // Pull some variables out:
   float camHeight = misc.x;
   float camHeightSqr = misc.y;
   
   float scale = misc.z;
   float scaleOverScaleDepth = misc.w;
   
   float outerRadius = sphereRadii.x;
   float outerRadiusSqr = sphereRadii.y;
   
   float innerRadius = sphereRadii.z;
   float innerRadiusSqr = sphereRadii.w;
   
   float rayleighBrightness = scatteringCoeffs.x; // Kr * ESun
   float rayleigh4PI = scatteringCoeffs.y; // Kr * 4 * PI

   float mieBrightness = scatteringCoeffs.z; // Km * ESun
   float mie4PI = scatteringCoeffs.w; // Km * 4 * PI
   
   // Get the ray from the camera to the vertex, 
   // and its length (which is the far point of the ray 
   // passing through the atmosphere).
   float4 v3Pos = float4(In.position,1.0) / 6378000.0; // outerRadius;
   float3 newCamPos = float3( 0, 0, camHeight );
   v3Pos.z += innerRadius;
   float3 v3Ray = v3Pos.xyz - newCamPos;
   float fFar = length(v3Ray);
   v3Ray /= fFar;

   // Calculate the ray's starting position, 
   // then calculate its scattering offset.
   float3 v3Start = newCamPos;
   float fHeight = length(v3Start); 
   float fDepth = exp(scaleOverScaleDepth * (innerRadius - camHeight));
   float fStartAngle = dot(v3Ray, v3Start) / fHeight;

   float fStartOffset = fDepth * vernierScale( fStartAngle );

   // Initialize the scattering loop variables.
   float fSampleLength = fFar / 2;
   float fScaledLength = fSampleLength * scale;
   float3 v3SampleRay = v3Ray * fSampleLength;
   float3 v3SamplePoint = v3Start + v3SampleRay * 0.5;

   // Now loop through the sample rays
   float3 v3FrontColor = float3(0.0, 0.0, 0.0);
   for(int i=0; i<2; i++)
   {
      float fHeight = length(v3SamplePoint);
      float fDepth = exp(scaleOverScaleDepth * (innerRadius - fHeight));
      float fLightAngle = dot(lightDir, v3SamplePoint) / fHeight;
      float fCameraAngle = dot(v3Ray, v3SamplePoint) / fHeight;

      float vscale3 = vernierScale( fCameraAngle );
      float vscale2 = vernierScale( fLightAngle );

      float fScatter = (fStartOffset + fDepth*(vscale2 - vscale3));
      float3 v3Attenuate = exp(-fScatter * (invWaveLength.xyz * rayleigh4PI + mie4PI));
      v3FrontColor += v3Attenuate * (fDepth * fScaledLength);
      v3SamplePoint += v3SampleRay;
   } 

   Conn Out;   

   // Finally, scale the Mie and Rayleigh colors 
   // and set up the varying variables for the pixel shader.
   Out.position = mul( modelView, float4(In.position,1.0) );
   Out.mieColor.rgb = v3FrontColor * mieBrightness;
   Out.mieColor.a = 1.0f;
   Out.rayleighColor.rgb = v3FrontColor * (invWaveLength.xyz * rayleighBrightness);
   Out.rayleighColor.a = 1.0f;
   Out.v3Direction = newCamPos - v3Pos.xyz;
   Out.pos = In.position;

#ifdef USE_COLORIZE  
  
   Out.rayleighColor.rgb = desaturate(Out.rayleighColor.rgb, 1) * colorize.a;  
     
   Out.rayleighColor.r *= colorize.r;  
   Out.rayleighColor.g *= colorize.g;  
   Out.rayleighColor.b *= colorize.b;  
     
#endif  

   return Out;
}