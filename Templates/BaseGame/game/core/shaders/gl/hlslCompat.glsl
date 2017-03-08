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

// These are some simple wrappers for simple 
// HLSL compatibility.

#define float4 vec4
#define float3 vec3
#define float2 vec2

#define half float
#define half2 vec2
#define half3 vec3
#define half4 vec4

#define float4x4 mat4
#define float3x3 mat3
#define float2x2 mat2

#define texCUBE texture
#define tex2D texture
#define tex1D texture
#define tex2Dproj textureProj
#define tex2Dlod( sampler, texCoord ) textureLod(sampler, texCoord.xy, texCoord.w)

#define samplerCUBE samplerCube

#define frac fract

#define lerp mix

void tSetMatrixRow(inout float3x3 m, int row, float3 value)
{
   m[0][row] = value.x;
   m[1][row] = value.y;
   m[2][row] = value.z;
}

void tSetMatrixRow(inout float4x4 m, int row, float4 value)
{
   m[0][row] = value.x;
   m[1][row] = value.y;
   m[2][row] = value.z;
   m[3][row] = value.w;
}

#define tGetMatrix3Row(matrix, row) float3(matrix[0][row], matrix[1][row], matrix[2][row])
#define tGetMatrix4Row(matrix, row) float4(matrix[0][row], matrix[1][row], matrix[2][row], matrix[3][row])

float3x3 float4x4to3x3(float4x4 m)
{   
   return float3x3( vec3(m[0]).xyz, m[1].xyz, m[2].xyz);
}

float3x3 float4x4to3x3_(float4x4 m)
{   
   return float3x3( vec3(m[0]), m[1].xyz, m[2].xyz);
}

mat4 mat4FromRow( float r0c0, float r0c1, float r0c2, float r0c3,
                  float r1c0, float r1c1, float r1c2, float r1c3,
                  float r2c0, float r2c1, float r2c2, float r2c3,
                  float r3c0, float r3c1, float r3c2, float r3c3 )
{
   return mat4( r0c0, r1c0, r2c0, r3c0,
                r0c1, r1c1, r2c1, r3c1,
                r0c2, r1c2, r2c2, r3c2,
                r0c3, r1c3, r2c3, r3c3 );
}


#define saturate( val ) clamp( val, 0.0, 1.0 )

#define round( n ) (sign( n ) * floor( abs( n ) + 0.5 ))

#define tMul(a, b) (a*b)

#define inversesqrt( n ) inversesqrt( n )

#define correctSSP(vec) vec.y *= -1

#ifdef TORQUE_PIXEL_SHADER
	void clip(float a) { if(a < 0) discard;}
#endif
