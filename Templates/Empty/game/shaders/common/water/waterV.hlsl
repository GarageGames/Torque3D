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

#include "../shaderModel.hlsl"

//-----------------------------------------------------------------------------
// Structures                                                                  
//-----------------------------------------------------------------------------
struct VertData
{
   float3 position         : POSITION;
   float3 normal           : NORMAL;
   float2 undulateData     : TEXCOORD0;
   float4 horizonFactor    : TEXCOORD1;
};

struct ConnectData
{
   float4 hpos             : TORQUE_POSITION;   
   
   // TexCoord 0 and 1 (xy,zw) for ripple texture lookup
   float4 rippleTexCoord01 : TEXCOORD0;   
   
   // xy is TexCoord 2 for ripple texture lookup 
   // z is the Worldspace unit distance/depth of this vertex/pixel
   // w is amount of the crestFoam ( more at crest of waves ).
   float4 rippleTexCoord2  : TEXCOORD1;
   
   // Screenspace vert position BEFORE wave transformation
   float4 posPreWave       : TEXCOORD2;
   
   // Screenspace vert position AFTER wave transformation
   float4 posPostWave      : TEXCOORD3;   
    
   // Objectspace vert position BEFORE wave transformation	
   // w coord is world space z position.
   float4 objPos           : TEXCOORD4;   
   
   float4 foamTexCoords    : TEXCOORD5;
   
   float3x3 tangentMat     : TEXCOORD6;
};

//-----------------------------------------------------------------------------
// Uniforms                                                                  
//-----------------------------------------------------------------------------
uniform float4x4 modelMat;
uniform float4x4 modelview;
uniform float4   rippleMat[3];
uniform float3   eyePos;       
uniform float2   waveDir[3];
uniform float2   waveData[3];
uniform float2   rippleDir[3];
uniform float2   rippleTexScale[3];                    
uniform float3   rippleSpeed;
uniform float4   foamDir;
uniform float4   foamTexScale;
uniform float2   foamSpeed;
uniform float3   inLightVec;
uniform float    gridElementSize;
uniform float    elapsedTime;
uniform float    undulateMaxDist;

//-----------------------------------------------------------------------------
// Main                                                                        
//-----------------------------------------------------------------------------
ConnectData main( VertData IN )
{	
   ConnectData OUT;  
   
   // use projection matrix for reflection / refraction texture coords
   float4x4 texGen = { 0.5,  0.0,  0.0,  0.5,
                       0.0, -0.5,  0.0,  0.5,
                       0.0,  0.0,  1.0,  0.0,
                       0.0,  0.0,  0.0,  1.0 };
         
   IN.position.z = lerp( IN.position.z, eyePos.z, IN.horizonFactor.x );
   float4 inPos = float4( IN.position, 1.0);
   OUT.objPos = inPos;
   OUT.objPos.w = mul( modelMat, inPos ).z;
   
   // Send pre-undulation screenspace position
   OUT.posPreWave = mul( modelview, inPos );
   OUT.posPreWave = mul( texGen, OUT.posPreWave );
      
   // Calculate the undulation amount for this vertex.   
   float2 undulatePos = mul( modelMat, float4( IN.undulateData.xy, 0, 1 ) ).xy;
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
   float dist = distance( IN.position.xyz, eyePos );
   dist = clamp( dist, 1.0, undulateMaxDist );          
   undulateFade *= ( 1 - dist / undulateMaxDist ); 
   
   // Also scale down wave magnitude if the camera is very very close.
   undulateFade *= saturate( ( distance( IN.position.xyz, eyePos ) - 0.5 ) / 10.0 );
   
   undulateAmt *= undulateFade;
   
   OUT.rippleTexCoord2.w = undulateAmt / ( waveData[0].y + waveData[1].y + waveData[2].y );	
   OUT.rippleTexCoord2.w = saturate( OUT.rippleTexCoord2.w - 0.2 ) / 0.8;
   
   // Apply wave undulation to the vertex.
   OUT.posPostWave = inPos;
   OUT.posPostWave.xyz += IN.normal.xyz * undulateAmt;         
   
   // Convert to screen 
   OUT.posPostWave = mul( modelview, OUT.posPostWave );
   
   // Setup the OUT position symantic variable
   OUT.hpos = OUT.posPostWave; 
   //OUT.hpos.z = lerp( OUT.hpos.z, OUT.hpos.w, IN.horizonFactor.x );
      
   // if ( IN.horizonFactor.x > 0 )
   // {
      // float3 awayVec = normalize( OUT.objPos.xyz - eyePos );
      // OUT.objPos.xy += awayVec.xy * 1000.0;
   // }
   
   // Save world space camera dist/depth of the outgoing pixel
   OUT.rippleTexCoord2.z = OUT.hpos.z;              

   // Convert to reflection texture space   
   OUT.posPostWave = mul( texGen, OUT.posPostWave );
                 
   float2 txPos = undulatePos;
   if ( IN.horizonFactor.x )
      txPos = normalize( txPos ) * 50000.0;
      
   // set up tex coordinates for the 3 interacting normal maps   
   OUT.rippleTexCoord01.xy = txPos * rippleTexScale[0];
   OUT.rippleTexCoord01.xy += rippleDir[0] * elapsedTime * rippleSpeed.x;
            
   float2x2 texMat;   
   texMat[0][0] = rippleMat[0].x;
   texMat[0][1] = rippleMat[0].y;
   texMat[1][0] = rippleMat[0].z;
   texMat[1][1] = rippleMat[0].w;
   OUT.rippleTexCoord01.xy = mul( texMat, OUT.rippleTexCoord01.xy );      

   OUT.rippleTexCoord01.zw = txPos * rippleTexScale[1];
   OUT.rippleTexCoord01.zw += rippleDir[1] * elapsedTime * rippleSpeed.y;
   
   texMat[0][0] = rippleMat[1].x;
   texMat[0][1] = rippleMat[1].y;
   texMat[1][0] = rippleMat[1].z;
   texMat[1][1] = rippleMat[1].w;
   OUT.rippleTexCoord01.zw = mul( texMat, OUT.rippleTexCoord01.zw );         

   OUT.rippleTexCoord2.xy = txPos * rippleTexScale[2];
   OUT.rippleTexCoord2.xy += rippleDir[2] * elapsedTime * rippleSpeed.z; 

   texMat[0][0] = rippleMat[2].x;
   texMat[0][1] = rippleMat[2].y;
   texMat[1][0] = rippleMat[2].z;
   texMat[1][1] = rippleMat[2].w;
   OUT.rippleTexCoord2.xy = mul( texMat, OUT.rippleTexCoord2.xy );    
   
   OUT.foamTexCoords.xy = txPos * foamTexScale.xy + foamDir.xy * foamSpeed.x * elapsedTime;
   OUT.foamTexCoords.zw = txPos * foamTexScale.zw + foamDir.zw * foamSpeed.y * elapsedTime;

   
   float3 binormal = float3( 1, 0, 0 );
   float3 tangent = float3( 0, 1, 0 );
   float3 normal;
   for ( int i = 0; i < 3; i++ )
   {
      binormal.z += undulateFade * waveDir[i].x * waveData[i].y * cos( waveDir[i].x * undulatePos.x + waveDir[i].y * undulatePos.y + elapsedTime * waveData[i].x );
      tangent.z += undulateFade * waveDir[i].y * waveData[i].y * cos( waveDir[i].x * undulatePos.x + waveDir[i].y * undulatePos.y + elapsedTime * waveData[i].x );
   } 
      
   binormal = binormal;
   tangent = tangent;
   normal = cross( binormal, tangent );
   
   float3x3 worldToTangent;
   worldToTangent[0] = binormal;
   worldToTangent[1] = tangent;
   worldToTangent[2] = normal;

   OUT.tangentMat = worldToTangent;
   
   return OUT;
}

