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

#ifndef _DU_DEBUG_DRAW_TORQUE_H_
#define _DU_DEBUG_DRAW_TORQUE_H_

#include "core/util/tVector.h"
#include <DebugDraw.h>
#include "gfx/gfxStateBlock.h"

/// @brief Implements the duDebugDraw interface in Torque.
class duDebugDrawTorque: public duDebugDraw {
public:
   duDebugDrawTorque();
   ~duDebugDrawTorque();

   /// Enable/disable Z read.
   void depthMask(bool state);

   /// Enable/disable texturing. Not used.
   void texture(bool state);

   /// Special colour overwrite for when I get picky about the colours Mikko chose.
   void overrideColor(unsigned int col);

   /// Stop the colour override.
   void cancelOverride();

   /// Begin drawing primitives.
   /// @param prim [in] primitive type to draw, one of rcDebugDrawPrimitives.
   /// @param size [in] size of a primitive, applies to point size and line width only.
   void begin(duDebugDrawPrimitives prim, float size = 1.0f);

   /// All new buffers go into this group.
   void beginGroup(U32 group);

   /// Submit a vertex
   /// @param pos [in] position of the verts.
   /// @param color [in] color of the verts.
   void vertex(const float* pos, unsigned int color);

   /// Submit a vertex
   /// @param x,y,z [in] position of the verts.
   /// @param color [in] color of the verts.
   void vertex(const float x, const float y, const float z, unsigned int color);

   /// Submit a vertex
   /// @param pos [in] position of the verts.
   /// @param color [in] color of the verts.
   void vertex(const float* pos, unsigned int color, const float* uv);

   /// Submit a vertex
   /// @param x,y,z [in] position of the verts.
   /// @param color [in] color of the verts.
   void vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v);

   /// End drawing primitives.
   void end();

   /// Render buffered primitive.
   void render();

   /// Render buffered primitives in a group.
   void renderGroup(U32 group);

   /// Delete buffered primitive.
   void clear();
      
private:
   GFXStateBlockDesc mDesc;

   U32 mPrimType;
   bool mQuadsMode;

   U32 mVertCount;
   F32 mStore[3][3];

   U32 mGroup;

   struct Instruction {
      // Contain either a point or a color command.
      union {
         struct {
            U8 r, g, b, a;
         } color;
         struct {
            float x, y, z;
         } point;
         U32 primType;
      } data;
      // Which type of data do we store?
      enum {
         COLOR,
         POINT,
         PRIMTYPE,
      } type;
      // Construct as color instruction.
      Instruction(U8 r, U8 g, U8 b, U8 a) {
         type = COLOR;
         data.color.r = r;
         data.color.g = g;
         data.color.b = b;
         data.color.a = a;
      }
      // Construct as point.
      Instruction(float x, float y, float z) {
         type = POINT;
         data.point.x = x;
         data.point.y = y;
         data.point.z = z;
      }
      Instruction(U32 t = 0) {
         type = PRIMTYPE;
         data.primType = t;
      }
   };

   struct Buffer {
      U32 group;
      Vector<Instruction> buffer;
      GFXPrimitiveType primType;
      Buffer(U32 type = 0) {
         primType = (GFXPrimitiveType)type;
         group = 0;
      }
   };
   Vector<Buffer> mBuffers;

   U32 mCurrColor;
   U32 mOverrideColor;
   bool mOverride;

   void _vertex(const float x, const float y, const float z, unsigned int color);

   void renderBuffer(Buffer &b);
};

#endif
