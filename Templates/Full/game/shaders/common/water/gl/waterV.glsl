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

#include "shadergen:/autogenConditioners.h"

//-----------------------------------------------------------------------------
// Defines                                                                  
//-----------------------------------------------------------------------------

// waveData
#define WAVE_SPEED(i)      waveData[i].x
#define WAVE_MAGNITUDE(i)  waveData[i].y

// Outgoing data
// Worldspace position of this pixel
varying vec3 worldPos;

// TexCoord 0 and 1 (xy,zw) for ripple texture lookup
varying vec4 rippleTexCoord01;

// TexCoord 2 for ripple texture lookup
varying vec2 rippleTexCoord2;

// Screenspace vert position BEFORE wave transformation
varying vec4 posPreWave;

// Screenspace vert position AFTER wave transformation
varying vec4 posPostWave;

// Worldspace unit distance/depth of this vertex/pixel
varying float pixelDist;

varying vec3 fogPos;

varying float worldSpaceZ;

varying vec4 foamTexCoords;

//-----------------------------------------------------------------------------
// Uniforms                                                                  
//-----------------------------------------------------------------------------
uniform mat4 modelMat;
uniform mat4 modelview;
uniform mat3 cubeTrans;
uniform mat4 objTrans;
uniform vec3   cubeEyePos;
uniform vec3   eyePos;       
uniform vec2   waveDir[3];
uniform vec2   waveData[3];
uniform vec2   rippleDir[3];
uniform vec2   rippleTexScale[3];                    
uniform vec3   rippleSpeed;
uniform vec2   reflectTexSize;
uniform vec3   inLightVec;
uniform vec3   reflectNormal;
uniform float    gridElementSize;
uniform float    elapsedTime;
uniform float    undulateMaxDist;

//-----------------------------------------------------------------------------
// Main                                                                        
//-----------------------------------------------------------------------------
void main()
{
   // Copy incoming attributes into locals so we can modify them in place.
   vec4 position = gl_Vertex.xyzw;
   vec3 normal = gl_Normal.xyz;
   vec2 undulateData = gl_MultiTexCoord0.st;
   vec4 horizonFactor = gl_MultiTexCoord1.xyzw;
   
   // use projection matrix for reflection / refraction texture coords
   mat4 texGen = { 0.5, 0.0, 0.0, 0.5, //+ 0.5 / reflectTexSize.x,
                   0.0, 0.5, 0.0, 0.5, //+ 1.0 / reflectTexSize.y,
                   0.0, 0.0, 1.0, 0.0,
                   0.0, 0.0, 0.0, 1.0 };

   // Move the vertex based on the horizonFactor if specified to do so for this vert.
   if ( horizonFactor.z > 0 )
   {
      vec2 offsetXY = eyePos.xy - eyePos.xy % gridElementSize;         
      position.xy += offsetXY;
      undulateData += offsetXY;
   }      
      
   fogPos = position;
   position.z = mix( position.z, eyePos.z, horizonFactor.x );
   
   // Send pre-undulation screenspace position
   posPreWave = modelview * position;
   posPreWave = texGen * posPreWave;
      
   // Calculate the undulation amount for this vertex.   
   vec2 undulatePos = undulateData; 
   float undulateAmt = 0;
   
   for ( int i = 0; i < 3; i++ )
   {
      undulateAmt += WAVE_MAGNITUDE(i) * sin( elapsedTime * WAVE_SPEED(i) + 
                                              undulatePos.x * waveDir[i].x +
                                              undulatePos.y * waveDir[i].y );
   }      
   
   // Scale down wave magnitude amount based on distance from the camera.   
   float dist = distance( position, eyePos );
   dist = clamp( dist, 1.0, undulateMaxDist );          
   undulateAmt *= ( 1 - dist / undulateMaxDist ); 
   
   // Also scale down wave magnitude if the camera is very very close.
   undulateAmt *= clamp( ( distance( IN.position, eyePos ) - 0.5 ) / 10.0, 0.0, 1.0 );
   
   // Apply wave undulation to the vertex.
   posPostWave = position;
   posPostWave.xyz += normal.xyz * undulateAmt;   
   
   // Save worldSpace position of this pixel/vert
   worldPos = posPostWave.xyz;   
   
   // Convert to screen 
   posPostWave = modelview * posPostWave;
   
   // Setup the OUT position symantic variable
   gl_Position = posPostWave;
   gl_Position.z = mix(gl_Position.z, gl_Position.w, horizonFactor.x);
   
   worldSpaceZ = modelMat * vec4(fogPos, 1.0) ).z;
   if ( horizonFactor.x > 0.0 )
   {
      vec3 awayVec = normalize( fogPos.xyz - eyePos );
      fogPos.xy += awayVec.xy * 1000.0;
   }
   
   // Save world space camera dist/depth of the outgoing pixel
   pixelDist = gl_Position.z;              

   // Convert to reflection texture space   
   posPostWave = texGen * posPostWave;
              
   float2 ripplePos = undulateData;     
   float2 txPos = normalize( ripplePos );
   txPos *= 50000.0;   
   ripplePos = mix( ripplePos, txPos, IN.horizonFactor.x );
      
   // set up tex coordinates for the 3 interacting normal maps   
   rippleTexCoord01.xy = mix( ripplePos * rippleTexScale[0], txPos.xy * rippleTexScale[0], IN.horizonFactor.x );
   rippleTexCoord01.xy += rippleDir[0] * elapsedTime * rippleSpeed.x;

   rippleTexCoord01.zw = mix( ripplePos * rippleTexScale[1], txPos.xy * rippleTexScale[1], IN.horizonFactor.x );
   rippleTexCoord01.zw += rippleDir[1] * elapsedTime * rippleSpeed.y;

   rippleTexCoord2.xy = mix( ripplePos * rippleTexScale[2], txPos.xy * rippleTexScale[2], IN.horizonFactor.x );
   rippleTexCoord2.xy += rippleDir[2] * elapsedTime * rippleSpeed.z; 

   foamTexCoords.xy = mix( ripplePos * 0.2, txPos.xy * rippleTexScale[0], IN.horizonFactor.x );   
   foamTexCoords.xy += rippleDir[0] * sin( ( elapsedTime + 500.0 ) * -0.4 ) * 0.15;

   foamTexCoords.zw = mix( ripplePos * 0.3, txPos.xy * rippleTexScale[1], IN.horizonFactor.x );   
   foamTexCoords.zw += rippleDir[1] * sin( elapsedTime * 0.4 ) * 0.15;
}
