//-----------------------------------------------------------------------------
// Copyright (c) 2013 GarageGames, LLC
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

#include "torqueRecast.h"
#include "duDebugDrawTorque.h"

#include "gfx/gfxDevice.h"
#include "gfx/primBuilder.h"
#include "gfx/gfxStateBlock.h"

/// @class duDebugDrawTorque
/// This class uses the primitive builder (gfx/primBuild.h) to render navmeshes
/// and other Recast data. To facilitate the primbuilder's requirement to know
/// the number of vertices to render beforehand, this class stores all vertices
/// in a buffer of its own, then passes on that known-size buffer.
/// This means that you only need to call the duDebugDraw functions when your
/// data changes. At other times, you can cache the duDebugDrawTorque object
/// and call its render() method, which actually renders its buffered data.

duDebugDrawTorque::duDebugDrawTorque()
{
   mPrimType = 0;
   mQuadsMode = false;
   mVertCount = 0;
   mGroup = 0;
   mCurrColor = 0;
   mOverrideColor = 0;
   mOverride = false;
}

duDebugDrawTorque::~duDebugDrawTorque()
{
   clear();
}

void duDebugDrawTorque::depthMask(bool state)
{
   mDesc.setZReadWrite(state, state);
}

void duDebugDrawTorque::texture(bool state)
{
}

/// Begin drawing primitives.
/// @param prim [in] primitive type to draw, one of rcDebugDrawPrimitives.
/// @param size [in] size of a primitive, applies to point size and line width only.
void duDebugDrawTorque::begin(duDebugDrawPrimitives prim, float size)
{
   mCurrColor = -1;
   mQuadsMode = false;
   mVertCount = 0;
   mPrimType = 0;
   switch(prim)
   {
   case DU_DRAW_POINTS: mPrimType = GFXPointList;    break;
   case DU_DRAW_LINES:  mPrimType = GFXLineList;     break;
   case DU_DRAW_TRIS:   mPrimType = GFXTriangleList; break;
   case DU_DRAW_QUADS:  mPrimType = GFXTriangleList;
                        mQuadsMode = true;           break;
   }
   mBuffers.push_back(Buffer(mPrimType));
   mBuffers.last().group = mGroup;
   mDesc.setCullMode(GFXCullNone);
   mDesc.setBlend(true);
}

void duDebugDrawTorque::beginGroup(U32 group)
{
   mGroup = group;
}

/// Submit a vertex
/// @param pos [in] position of the verts.
/// @param color [in] color of the verts.
void duDebugDrawTorque::vertex(const float* pos, unsigned int color)
{
   vertex(pos[0], pos[1], pos[2], color);
}

/// Submit a vertex
/// @param x,y,z [in] position of the verts.
/// @param color [in] color of the verts.
void duDebugDrawTorque::vertex(const float x, const float y, const float z, unsigned int color)
{
   if(mQuadsMode)
   {
      if(mVertCount == 3)
      {
         _vertex(x, -z, y, color);
         _vertex(mStore[0][0], mStore[0][1], mStore[0][2], color);
         _vertex(mStore[1][0], mStore[1][1], mStore[1][2], color);
         _vertex(mStore[1][0], mStore[1][1], mStore[1][2], color);
         _vertex(mStore[2][0], mStore[2][1], mStore[2][2], color);
         _vertex(x, -z, y, color);
         mVertCount = 0;
      }
      else
      {
         mStore[mVertCount][0] = x;
         mStore[mVertCount][1] = -z;
         mStore[mVertCount][2] = y;
         mVertCount++;
      }
   }
   else 
   {
      _vertex(x, -z, y, color);
   }
}

/// Submit a vertex
/// @param pos [in] position of the verts.
/// @param color [in] color of the verts.
void duDebugDrawTorque::vertex(const float* pos, unsigned int color, const float* uv)
{
   vertex(pos[0], pos[1], pos[2], color);
}

/// Submit a vertex
/// @param x,y,z [in] position of the verts.
/// @param color [in] color of the verts.
void duDebugDrawTorque::vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v)
{
   vertex(x, y, z, color);
}

/// Push a vertex onto the buffer.
void duDebugDrawTorque::_vertex(const float x, const float y, const float z, unsigned int color)
{
   // Use override color if we must.
   //if(mOverride)
      //color = mOverrideColor;
   if(mCurrColor != color || !mBuffers.last().buffer.size())
   {
      U8 r, g, b, a;
      // Convert color integer to components.
      rcCol(color, r, g, b, a);
      mBuffers.last().buffer.push_back(Instruction(r, g, b, a));
      mCurrColor = color;
   }
   // Construct vertex data.
   mBuffers.last().buffer.push_back(Instruction(x, y, z));
}

/// End drawing primitives.
void duDebugDrawTorque::end()
{
}

void duDebugDrawTorque::overrideColor(unsigned int col)
{
   mOverride = true;
   mOverrideColor = col;
}

void duDebugDrawTorque::cancelOverride()
{
   mOverride = false;
}

void duDebugDrawTorque::renderBuffer(Buffer &b)
{
   PrimBuild::begin(b.primType, b.buffer.size());
   Vector<Instruction> &buf = b.buffer;
   for(U32 i = 0; i < buf.size(); i++)
   {
      switch(buf[i].type)
      {
      case Instruction::POINT:
         PrimBuild::vertex3f(buf[i].data.point.x,
                             buf[i].data.point.y,
                             buf[i].data.point.z);
         break;

      case Instruction::COLOR:
         if(mOverride)
            break;
         PrimBuild::color4i(buf[i].data.color.r,
                            buf[i].data.color.g,
                            buf[i].data.color.b,
                            buf[i].data.color.a);
         break;
      }
   }
   PrimBuild::end();
}

void duDebugDrawTorque::render()
{
   GFXStateBlockRef sb = GFX->createStateBlock(mDesc);
   GFX->setStateBlock(sb);
   // Use override color for all rendering.
   if(mOverride)
   {
      U8 r, g, b, a;
      rcCol(mOverrideColor, r, g, b, a);
      PrimBuild::color4i(r, g, b, a);
   }
   for(U32 b = 0; b < mBuffers.size(); b++)
   {
      renderBuffer(mBuffers[b]);
   }
}

void duDebugDrawTorque::renderGroup(U32 group)
{
   GFXStateBlockRef sb = GFX->createStateBlock(mDesc);
   GFX->setStateBlock(sb);
   // Use override color for all rendering.
   if(mOverride)
   {
      U8 r, g, b, a;
      rcCol(mOverrideColor, r, g, b, a);
      PrimBuild::color4i(r, g, b, a);
   }
   for(U32 b = 0; b < mBuffers.size(); b++)
   {
      if(mBuffers[b].group == group)
         renderBuffer(mBuffers[b]);
   }
}

void duDebugDrawTorque::clear()
{
   for(U32 b = 0; b < mBuffers.size(); b++)
      mBuffers[b].buffer.clear();
   mBuffers.clear();
}
