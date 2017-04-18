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
#include "shadergen:/autogenConditioners.h"

//-----------------------------------------------------------------------------
// Structures                                                                  
//-----------------------------------------------------------------------------
struct VertData
{
   vec4   position         ;// POSITION;
   vec3   normal           ;// NORMAL;
   vec2   undulateData     ;// TEXCOORD0;
   vec4   horizonFactor    ;// TEXCOORD1;
};

//-----------------------------------------------------------------------------
// Defines                                                                  
//-----------------------------------------------------------------------------
//VertData IN
in vec4 vPosition;
in vec3 vNormal;
in vec2 vTexCoord0;
in vec4 vTexCoord1;

#define IN_position_       vPosition
#define IN_normal          vNormal
#define IN_undulateData    vTexCoord0
#define IN_horizonFactor   vTexCoord1

//ConnectData OUT
//
   out vec4   hpos             ;

// TexCoord 0 and 1 (xy,zw) for ripple texture lookup
out vec4 rippleTexCoord01;

   // xy is TexCoord 2 for ripple texture lookup 
   // z is the Worldspace unit distance/depth of this vertex/pixel
   // w is amount of the crestFoam ( more at crest of waves ).
   out vec4   rippleTexCoord2  ;

// Screenspace vert position BEFORE wave transformation
out vec4 posPreWave;

// Screenspace vert position AFTER wave transformation
out vec4 posPostWave;

   // Objectspace vert position BEFORE wave transformation	
   // w coord is world space z position.
   out vec4   objPos           ;  

   out vec4   foamTexCoords    ;

   out mat3   tangentMat     ;
//

#define OUT_hpos hpos
#define OUT_rippleTexCoord01 rippleTexCoord01
#define OUT_rippleTexCoord2 rippleTexCoord2
#define OUT_posPreWave posPreWave
#define OUT_posPostWave posPostWave
#define OUT_objPos objPos
#define OUT_foamTexCoords foamTexCoords
#define OUT_tangentMat tangentMat

//-----------------------------------------------------------------------------
// Uniforms                                                                  
//-----------------------------------------------------------------------------
uniform mat4 modelMat;
uniform mat4 modelview;
uniform vec4     rippleMat[3];
uniform vec3   eyePos;       
uniform vec2   waveDir[3];
uniform vec2   waveData[3];
uniform vec2   rippleDir[3];
uniform vec2   rippleTexScale[3];                    
uniform vec3   rippleSpeed;
uniform vec4     foamDir;
uniform vec4     foamTexScale;
uniform vec2     foamSpeed;
uniform vec3   inLightVec;
uniform float    gridElementSize;
uniform float    elapsedTime;
uniform float    undulateMaxDist;

//-----------------------------------------------------------------------------
// Main                                                                        
//-----------------------------------------------------------------------------
void main()
{
   vec4 IN_position      = IN_position_;   
   
   // use projection matrix for reflection / refraction texture coords
   mat4 texGen = mat4FromRow( 0.5,  0.0,  0.0,  0.5,
                       0.0, -0.5,  0.0,  0.5,
                       0.0,  0.0,  1.0,  0.0,
                       0.0,  0.0,  0.0,  1.0 );   

   IN_position.z = mix( IN_position.z, eyePos.z, IN_horizonFactor.x );
      
   OUT_objPos = IN_position;
   OUT_objPos.w = tMul( modelMat, IN_position ).z;
   
   // Send pre-undulation screenspace position
   OUT_posPreWave = tMul( modelview, IN_position );
   OUT_posPreWave = tMul( texGen, OUT_posPreWave );
      
   // Calculate the undulation amount for this vertex.   
   vec2   undulatePos = tMul( modelMat, vec4  ( IN_undulateData.xy, 0, 1 ) ).xy;
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
   
   float undulateFade = 1;
   
   // Scale down wave magnitude amount based on distance from the camera.   
   float dist = distance( IN_position.xyz, eyePos );
   dist = clamp( dist, 1.0, undulateMaxDist );          
   undulateFade *= ( 1 - dist / undulateMaxDist ); 
   
   // Also scale down wave magnitude if the camera is very very close.
   undulateFade *= saturate( ( distance( IN_position.xyz, eyePos ) - 0.5 ) / 10.0 );
   
   undulateAmt *= undulateFade;
   
   OUT_rippleTexCoord2.w = undulateAmt / ( waveData[0].y + waveData[1].y + waveData[2].y );	
   OUT_rippleTexCoord2.w = saturate( OUT_rippleTexCoord2.w - 0.2 ) / 0.8;
   
   // Apply wave undulation to the vertex.
   OUT_posPostWave = IN_position;
   OUT_posPostWave.xyz += IN_normal.xyz * undulateAmt;         
   
   // Convert to screen 
   OUT_posPostWave = tMul( modelview, OUT_posPostWave );
   
   // Setup the OUT position symantic variable
   OUT_hpos = OUT_posPostWave; 
   //OUT_hpos.z = mix( OUT_hpos.z, OUT_hpos.w, IN_horizonFactor.x );
   
   // if ( IN_horizonFactor.x > 0 )
   // {
      // vec3   awayVec = normalize( OUT_objPos.xyz - eyePos );
      // OUT_objPos.xy += awayVec.xy * 1000.0;
   // }
   
   // Save world space camera dist/depth of the outgoing pixel
   OUT_rippleTexCoord2.z = OUT_hpos.z;              

   // Convert to reflection texture space   
   OUT_posPostWave = tMul( texGen, OUT_posPostWave );
              
   vec2   txPos = undulatePos;
   if ( bool(IN_horizonFactor.x) )
      txPos = normalize( txPos ) * 50000.0;
      
   // set up tex coordinates for the 3 interacting normal maps   
   OUT_rippleTexCoord01.xy = txPos * rippleTexScale[0];
   OUT_rippleTexCoord01.xy += rippleDir[0] * elapsedTime * rippleSpeed.x;

   mat2 texMat;   
   texMat[0][0] = rippleMat[0].x;
   texMat[1][0] = rippleMat[0].y;
   texMat[0][1] = rippleMat[0].z;
   texMat[1][1] = rippleMat[0].w;
   OUT_rippleTexCoord01.xy = tMul( texMat, OUT_rippleTexCoord01.xy );      

   OUT_rippleTexCoord01.zw = txPos * rippleTexScale[1];
   OUT_rippleTexCoord01.zw += rippleDir[1] * elapsedTime * rippleSpeed.y;
   
   texMat[0][0] = rippleMat[1].x;
   texMat[1][0] = rippleMat[1].y;
   texMat[0][1] = rippleMat[1].z;
   texMat[1][1] = rippleMat[1].w;
   OUT_rippleTexCoord01.zw = tMul( texMat, OUT_rippleTexCoord01.zw );         

   OUT_rippleTexCoord2.xy = txPos * rippleTexScale[2];
   OUT_rippleTexCoord2.xy += rippleDir[2] * elapsedTime * rippleSpeed.z; 

   texMat[0][0] = rippleMat[2].x;
   texMat[1][0] = rippleMat[2].y;
   texMat[0][1] = rippleMat[2].z;
   texMat[1][1] = rippleMat[2].w;
   OUT_rippleTexCoord2.xy = tMul( texMat, OUT_rippleTexCoord2.xy );    
   
   OUT_foamTexCoords.xy = txPos * foamTexScale.xy + foamDir.xy * foamSpeed.x * elapsedTime;
   OUT_foamTexCoords.zw = txPos * foamTexScale.zw + foamDir.zw * foamSpeed.y * elapsedTime;

   
   vec3   binormal = vec3  ( 1, 0, 0 );
   vec3   tangent = vec3  ( 0, 1, 0 );
   vec3   normal;
   for ( int i = 0; i < 3; i++ )
   {
      binormal.z += undulateFade * waveDir[i].x * waveData[i].y * cos( waveDir[i].x * undulatePos.x + waveDir[i].y * undulatePos.y + elapsedTime * waveData[i].x );
	  tangent.z += undulateFade * waveDir[i].y * waveData[i].y * cos( waveDir[i].x * undulatePos.x + waveDir[i].y * undulatePos.y + elapsedTime * waveData[i].x );
   } 

   binormal = binormal;
   tangent = tangent;
   normal = cross( binormal, tangent );
   
   mat3 worldToTangent;
   worldToTangent[0] = binormal;
   worldToTangent[1] = tangent;
   worldToTangent[2] = normal;

   OUT_tangentMat = transpose(worldToTangent);

   gl_Position = OUT_hpos;
   correctSSP(gl_Position);
}

