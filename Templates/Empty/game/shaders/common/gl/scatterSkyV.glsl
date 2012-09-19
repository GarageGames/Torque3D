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

const int nSamples = 4;
const float fSamples = 4.0;

// The scale depth (the altitude at which the average atmospheric density is found)
const float fScaleDepth = 0.25;
const float fInvScaleDepth = 1.0 / 0.25;

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

// This is the shader output data.
varying vec4 rayleighColor;
varying vec4 mieColor;
varying vec3 v3Direction;
varying float zPosition;
varying vec3 pos;
 
uniform mat4 modelView;
uniform vec4 misc;
uniform vec4 sphereRadii;
uniform vec4 scatteringCoeffs;
uniform vec3 camPos;
uniform vec3 lightDir;
uniform vec4 invWaveLength;
 
void main()        
{
   vec4 position = gl_Vertex.xyzw;
   vec3 normal = gl_Normal.xyz;
   vec4 color = gl_MultiTexCoord0.xyzw;

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
   vec3 v3Pos = position.xyz / 6378000.0;// / outerRadius;
   vec3 newCamPos = vec3( 0, 0, camHeight );
   v3Pos.z += innerRadius;
   vec3 v3Ray = v3Pos.xyz - newCamPos;
   float fFar = length(v3Ray);
   v3Ray /= fFar;

   // Calculate the ray's starting position, 
   // then calculate its scattering offset.
   vec3 v3Start = newCamPos;
   float fHeight = length(v3Start); 
   float fDepth = exp(scaleOverScaleDepth * (innerRadius - camHeight));
   float fStartAngle = dot(v3Ray, v3Start) / fHeight;

   float x = 1.0 - fStartAngle;
   float x5 = x * 5.25;
   float x5p6 = (-6.80 + x5);
   float xnew = (3.83 + x * x5p6);
   float xfinal = (0.459 + x * xnew);
   float xfinal2 = -0.00287 + x * xfinal;
   float othx = exp( xfinal2 ); 
   float vscale1 = 0.25 * othx;

   float fStartOffset = fDepth * vscale1;//vernierScale(fStartAngle);

   // Initialize the scattering loop variables.
   float fSampleLength = fFar / 2.0;
   float fScaledLength = fSampleLength * scale;
   vec3 v3SampleRay = v3Ray * fSampleLength;
   vec3 v3SamplePoint = v3Start + v3SampleRay * 0.5;

   // Now loop through the sample rays
   vec3 v3FrontColor = vec3(0.0, 0.0, 0.0);
   for(int i=0; i<2; i++)
   {
      float fHeight = length(v3SamplePoint);
      float fDepth = exp(scaleOverScaleDepth * (innerRadius - fHeight));
      float fLightAngle = dot(lightDir, v3SamplePoint) / fHeight;
      float fCameraAngle = dot(v3Ray, v3SamplePoint) / fHeight;

      x = 1.0 - fCameraAngle;
      x5 = x * 5.25;
      x5p6 = (-6.80 + x5);
      xnew = (3.83 + x * x5p6);
      xfinal = (0.459 + x * xnew);
      xfinal2 = -0.00287 + x * xfinal;
      othx = exp( xfinal2 ); 
      float vscale3 = 0.25 * othx;


      x = 1.0 - fLightAngle;
      x5 = x * 5.25;
      x5p6 = (-6.80 + x5);
      xnew = (3.83 + x * x5p6);
      xfinal = (0.459 + x * xnew);
      xfinal2 = -0.00287 + x * xfinal;
      othx = exp( xfinal2 ); 
      float vscale2 = 0.25 * othx;

      float fScatter = (fStartOffset + fDepth*(vscale2 - vscale3));
      vec3 v3Attenuate = exp(-fScatter * (invWaveLength.xyz * rayleigh4PI + mie4PI));
      v3FrontColor += v3Attenuate * (fDepth * fScaledLength);
      v3SamplePoint += v3SampleRay;
   } 
   
   // Finally, scale the Mie and Rayleigh colors 
   // and set up the varying variables for the pixel shader.
   gl_Position = modelView * position;
   mieColor.rgb = v3FrontColor * mieBrightness;
   mieColor.a = 1.0;
	rayleighColor.rgb = v3FrontColor * (invWaveLength.xyz * rayleighBrightness);
	rayleighColor.a = 1.0;
   v3Direction = newCamPos - v3Pos.xyz;      
   
   // This offset is to get rid of the black line between the atmosky and the waterPlane
   // along the horizon.
   zPosition = position.z + 4000.0;
   pos = position.xyz;
}

