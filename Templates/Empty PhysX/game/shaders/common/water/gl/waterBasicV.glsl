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

#include "../../gl/hlslCompat.glsl"

//-----------------------------------------------------------------------------
// Defines                                                                  
//-----------------------------------------------------------------------------

// TexCoord 0 and 1 (xy,zw) for ripple texture lookup
varying vec4 rippleTexCoord01;

// TexCoord 2 for ripple texture lookup
varying vec2 rippleTexCoord2;

// Screenspace vert position BEFORE wave transformation
varying vec4 posPreWave;

// Screenspace vert position AFTER wave transformation
varying vec4 posPostWave;  

// Worldspace unit distance/depth of this vertex/pixel
varying float  pixelDist;   

varying vec4 objPos;

varying vec3 misc;

//-----------------------------------------------------------------------------
// Uniforms                                                                  
//-----------------------------------------------------------------------------
uniform mat4 modelMat;
uniform mat4 modelview;
uniform vec4   rippleMat[3];
uniform vec3   eyePos;       
uniform vec2   waveDir[3];
uniform vec2   waveData[3];
uniform vec2   rippleDir[3];
uniform vec2   rippleTexScale[3];                    
uniform vec3   rippleSpeed;
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
   vec4 position = gl_Vertex;
   vec3 normal = gl_Normal;
   vec2 undulateData = gl_MultiTexCoord0.st;
   vec4 horizonFactor = gl_MultiTexCoord1;
   
   // use projection matrix for reflection / refraction texture coords
   mat4 texGen = mat4(0.5, 0.0, 0.0, 0.0,
                      0.0, 0.5, 0.0, 0.0,
                      0.0, 0.0, 1.0, 0.0,
                      0.5, 0.5, 0.0, 1.0);

   // Move the vertex based on the horizonFactor if specified to do so for this vert.
   //if ( horizonFactor.z > 0.0 )
   //{
      //vec2 offsetXY = eyePos.xy - mod(eyePos.xy, gridElementSize);         
      //position.xy += offsetXY;
      //undulateData += offsetXY; 
   //}      
   
   vec4 worldPos = modelMat * position;
   //fogPos = position.xyz;
   position.z = mix( position.z, eyePos.z, horizonFactor.x );

   objPos.xyz = position.xyz;
   objPos.w = worldPos.z;
   
   // Send pre-undulation screenspace position
   posPreWave = modelview * position;
   posPreWave = texGen * posPreWave;
      
   // Calculate the undulation amount for this vertex.   
   vec2 undulatePos = (modelMat * vec4( undulateData.xy, 0, 1 )).xy;
   
   //if ( undulatePos.x < 0.0 )
      //undulatePos = position.xy;
   
   float undulateAmt = 0.0;
   
   undulateAmt += waveData[0].y * sin( elapsedTime * waveData[0].x + 
                                       undulatePos.x * waveDir[0].x +
                                       undulatePos.y * waveDir[0].y );
   undulateAmt += waveData[1].y * sin( elapsedTime * waveData[1].x + 
                                       undulatePos.x * waveDir[1].x +
                                       undulatePos.y * waveDir[1].y );
   undulateAmt += waveData[2].y * sin( elapsedTime * waveData[2].x + 
                                       undulatePos.x * waveDir[2].x +
                                       undulatePos.y * waveDir[2].y );

   float undulateFade = 1.0;
 
   // Scale down wave magnitude amount based on distance from the camera.   
   float dist = length( position.xyz - eyePos );
   dist = clamp( dist, 1.0, undulateMaxDist );          
   undulateFade *= ( 1.0 - dist / undulateMaxDist ); 
   
   // Also scale down wave magnitude if the camera is very very close.
   undulateFade *= saturate( ( length( position.xyz - eyePos ) - 0.5 ) / 10.0 );

   undulateAmt *= undulateFade;
   
   //undulateAmt = 0;
   
   // Apply wave undulation to the vertex.
   posPostWave = position;
   posPostWave.xyz += normal.xyz * undulateAmt;   
   
   // Save worldSpace position of this pixel/vert
   //worldPos = posPostWave.xyz;   
   
   //worldSpaceZ = ( modelMat * vec4(fogPos,1.0) ).z;
   //if ( horizonFactor.x > 0.0 )
   //{
      //vec3 awayVec = normalize( fogPos.xyz - eyePos );
      //fogPos.xy += awayVec.xy * 1000.0;
   //}
   
   // Convert to screen 
   posPostWave = modelview * posPostWave;   
   
   // Setup the OUT position symantic variable
   gl_Position = posPostWave;
   //gl_Position.z = mix(gl_Position.z, gl_Position.w, horizonFactor.x);
   
   // Save world space camera dist/depth of the outgoing pixel
   pixelDist = gl_Position.z;              

   // Convert to reflection texture space   
   posPostWave = texGen * posPostWave;
        
   vec2 txPos = undulatePos;
   if ( horizonFactor.x > 0.0 )
      txPos = normalize( txPos ) * 50000.0;

   
   // set up tex coordinates for the 3 interacting normal maps
   rippleTexCoord01.xy = txPos * rippleTexScale[0];
   rippleTexCoord01.xy += rippleDir[0] * elapsedTime * rippleSpeed.x;
            
   mat2 texMat;   
   texMat[0][0] = rippleMat[0].x;
   texMat[1][0] = rippleMat[0].y;
   texMat[0][1] = rippleMat[0].z;
   texMat[1][1] = rippleMat[0].w;
   rippleTexCoord01.xy = texMat * rippleTexCoord01.xy ;      

   rippleTexCoord01.zw = txPos * rippleTexScale[1];
   rippleTexCoord01.zw += rippleDir[1] * elapsedTime * rippleSpeed.y;
   
   texMat[0][0] = rippleMat[1].x;
   texMat[1][0] = rippleMat[1].y;
   texMat[0][1] = rippleMat[1].z;
   texMat[1][1] = rippleMat[1].w;
   rippleTexCoord01.zw = texMat * rippleTexCoord01.zw ;         

   rippleTexCoord2.xy = txPos * rippleTexScale[2];
   rippleTexCoord2.xy += rippleDir[2] * elapsedTime * rippleSpeed.z; 
   
   texMat[0][0] = rippleMat[2].x;
   texMat[1][0] = rippleMat[2].y;
   texMat[0][1] = rippleMat[2].z;
   texMat[1][1] = rippleMat[2].w;
   rippleTexCoord2.xy = texMat * rippleTexCoord2.xy ;


   /*rippleTexCoord01.xy = mix( position.xy * rippleTexScale[0], txPos.xy * rippleTexScale[0], horizonFactor.x );
   rippleTexCoord01.xy += rippleDir[0] * elapsedTime * rippleSpeed.x;

   rippleTexCoord01.zw = mix( position.xy * rippleTexScale[1], txPos.xy * rippleTexScale[1], horizonFactor.x );
   rippleTexCoord01.zw += rippleDir[1] * elapsedTime * rippleSpeed.y;

   rippleTexCoord2.xy = mix( position.xy * rippleTexScale[2], txPos.xy * rippleTexScale[2], horizonFactor.x );
   rippleTexCoord2.xy += rippleDir[2] * elapsedTime * rippleSpeed.z; */


   /*rippleTexCoord01.xy = mix( position.xy * rippleTexScale[0], txPos.xy * rippleTexScale[0], horizonFactor.x );
   rippleTexCoord01.xy += rippleDir[0] * elapsedTime * rippleSpeed.x;
   mat2 texMat;   
   texMat[0][0] = rippleMat[0].x;
   texMat[1][0] = rippleMat[0].y;
   texMat[0][1] = rippleMat[0].z;
   texMat[1][1] = rippleMat[0].w;
   rippleTexCoord01.xy = texMat * rippleTexCoord01.xy ;      

   rippleTexCoord01.zw = mix( position.xy * rippleTexScale[1], txPos.xy * rippleTexScale[1], horizonFactor.x );
   rippleTexCoord01.zw += rippleDir[1] * elapsedTime * rippleSpeed.y;
   texMat[0][0] = rippleMat[1].x;
   texMat[1][0] = rippleMat[1].y;
   texMat[0][1] = rippleMat[1].z;
   texMat[1][1] = rippleMat[1].w;
   rippleTexCoord01.zw = texMat * rippleTexCoord01.zw ;         

   rippleTexCoord2.xy = mix( position.xy * rippleTexScale[2], txPos.xy * rippleTexScale[2], horizonFactor.x );
   rippleTexCoord2.xy += rippleDir[2] * elapsedTime * rippleSpeed.z;
   texMat[0][0] = rippleMat[2].x;
   texMat[1][0] = rippleMat[2].y;
   texMat[0][1] = rippleMat[2].z;
   texMat[1][1] = rippleMat[2].w;
   rippleTexCoord2.xy = texMat * rippleTexCoord2.xy ;*/

#ifdef WATER_SPEC
   
   vec3 binormal = vec3( 1, 0, 0 );
   vec3 tangent = vec3( 0, 1, 0 );
   vec3 normal;
   for ( int i = 0; i < 3; i++ )
   {
      binormal.z += undulateFade * waveDir[i].x * waveData[i].y * cos( waveDir[i].x * undulateData.x + waveDir[i].y * undulateData.y + elapsedTime * waveData[i].x );
	  tangent.z += undulateFade * waveDir[i].y * waveData[i].y * cos( waveDir[i].x * undulateData.x + waveDir[i].y * undulateData.y + elapsedTime * waveData[i].x );
   } 
      
   binormal = normalize( binormal );
   tangent = normalize( tangent );
   normal = cross( binormal, tangent );
   
   mat3 worldToTangent;
   worldToTangent[0] = binormal;
   worldToTangent[1] = tangent;
   worldToTangent[2] = normal;
      
   misc.xyz = inLightVec * modelMat;
   misc.xyz = worldToTangent * misc.xyz;   
   
#else

   misc.xyz = inLightVec;

#endif

}

