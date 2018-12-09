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

// The purpose of this file is to get all of our HLSL structures into one place.
// Please use the structures here instead of redefining input and output structures
// in each shader file. If structures are added, please adhere to the naming convention.

//------------------------------------------------------------------------------
// Vertex Input Structures
//
// These structures map to FVFs/Vertex Declarations in Torque. See gfxStructs.h
//------------------------------------------------------------------------------

// Notes
//
// Position should be specified as a float3 as our vertex structures in 
// the engine output float3s for position.

struct VertexIn_P
{
   float3 pos        : POSITION;
};

struct VertexIn_PT
{
   float3 pos        : POSITION;
   float2 uv0        : TEXCOORD0;
};

struct VertexIn_PTTT
{
   float3 pos        : POSITION;
   float2 uv0        : TEXCOORD0;
   float2 uv1        : TEXCOORD1;
   float2 uv2        : TEXCOORD2;
};

struct VertexIn_PC
{
   float3 pos        : POSITION;
   float4 color      : DIFFUSE;
};

struct VertexIn_PNC
{
   float3 pos        : POSITION;
   float3 normal     : NORMAL;
   float4 color      : DIFFUSE;
};

struct VertexIn_PCT
{
   float3 pos        : POSITION;
   float4 color      : DIFFUSE;
   float2 uv0        : TEXCOORD0;
};

struct VertexIn_PN
{
   float3 pos        : POSITION;
   float3 normal     : NORMAL;
};

struct VertexIn_PNT
{
   float3 pos        : POSITION;
   float3 normal     : NORMAL;
   float2 uv0        : TEXCOORD0;
};

struct VertexIn_PNTT
{
   float3 pos        : POSITION;
   float3 normal     : NORMAL;
   float3 tangent    : TANGENT;
   float2 uv0        : TEXCOORD0;
};

struct VertexIn_PNCT
{
   float3 pos        : POSITION;
   float3 normal     : NORMAL;
   float4 color      : DIFFUSE;
   float2 uv0        : TEXCOORD0;
};

struct VertexIn_PNTTTB
{
   float3 pos        : POSITION;
   float3 normal     : NORMAL; 
   float2 uv0        : TEXCOORD0;
   float2 uv1        : TEXCOORD1;   
   float3 T          : TEXCOORD2;
   float3 B          : TEXCOORD3;
};