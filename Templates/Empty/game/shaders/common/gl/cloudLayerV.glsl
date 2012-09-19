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

varying vec4 texCoord12;
varying vec4 texCoord34;
varying vec3 vLightTS; // light vector in tangent space, denormalized
varying vec3 vViewTS;  // view vector in tangent space, denormalized
varying vec3 vNormalWS; // Normal vector in world space
varying float worldDist;

//-----------------------------------------------------------------------------
// Uniforms                                                                        
//-----------------------------------------------------------------------------
uniform mat4  modelview;
uniform vec3  eyePosWorld;
uniform vec3  sunVec;
uniform vec2  texOffset0;
uniform vec2  texOffset1;
uniform vec2  texOffset2;
uniform vec3  texScale;

//-----------------------------------------------------------------------------
// Main                                                                        
//-----------------------------------------------------------------------------
void main()
{   
   vec4 pos = gl_Vertex;
   vec3 normal = gl_Normal;
   vec3 binormal = gl_MultiTexCoord0.xyz;
   vec3 tangent = gl_MultiTexCoord1.xyz;
   vec2 uv0 = gl_MultiTexCoord2.st;

   gl_Position = modelview * pos;
   
   // Offset the uv so we don't have a seam directly over our head.
   vec2 uv = uv0 + vec2( 0.5, 0.5 );
   
   texCoord12.xy = uv * texScale.x;
   texCoord12.xy += texOffset0;
   
   texCoord12.zw = uv * texScale.y;
   texCoord12.zw += texOffset1;
   
   texCoord34.xy = uv * texScale.z;
   texCoord34.xy += texOffset2;  
   
   texCoord34.z = pos.z;
   texCoord34.w = 0.0;

   // Transform the normal, tangent and binormal vectors from object space to 
   // homogeneous projection space:
   vNormalWS   = -normal; 
   vec3 vTangentWS  = -tangent;
   vec3 vBinormalWS = -binormal;
   
   // Compute position in world space:
   vec4 vPositionWS = pos + vec4( eyePosWorld, 1 ); //mul( pos, objTrans );

   // Compute and output the world view vector (unnormalized):
   vec3 vViewWS = eyePosWorld - vPositionWS.xyz;

   // Compute denormalized light vector in world space:
   vec3 vLightWS = -sunVec;

   // Normalize the light and view vectors and transform it to the tangent space:
   mat3 mWorldToTangent = mat3( vTangentWS, vBinormalWS, vNormalWS );

   // Propagate the view and the light vectors (in tangent space):
   vLightTS = mWorldToTangent * vLightWS;
   vViewTS  = vViewWS * mWorldToTangent;
   
   worldDist = clamp( pow( pos.z, 2.0 ), 0.0, 1.0 );
}
