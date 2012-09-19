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

#ifndef _DEBUGDRAW_H_
#define _DEBUGDRAW_H_

#ifndef _SIMOBJECT_H_
#include "console/simObject.h"
#endif

#ifndef _GFXDEVICE_H_
#include "gfx/gfxDevice.h"
#endif

#ifndef _PRIMBUILDER_H_
#include "gfx/primBuilder.h"
#endif

#ifndef _GFONT_H_
#include "gfx/gFont.h"
#endif

#ifndef _DATACHUNKER_H_
#include "core/dataChunker.h"
#endif

#ifndef _MPOLYHEDRON_H_
#include "math/mPolyhedron.h"
#endif


class GFont;


// We enable the debug drawer for non-shipping
// builds.... you better be using shipping builds
// for your final release.
#ifndef TORQUE_SHIPPING
#define ENABLE_DEBUGDRAW
#endif


/// Debug output class.
///
/// This class provides you with a flexible means of drawing debug output. It is
/// often useful when debugging collision code or complex 3d algorithms to have
/// them draw debug information, like culling hulls or bounding volumes, normals,
/// simple lines, and so forth. In TGE1.2, which was based directly on a simple
/// OpenGL rendering layer, it was a simple matter to do debug rendering directly
/// inline.
///
/// Unfortunately, this doesn't hold true with more complex rendering scenarios,
/// where render modes and targets may be in abritrary states. In addition, it is
/// often useful to be able to freeze frame debug information for closer inspection.
///
/// Therefore, Torque provides a global DebugDrawer instance, called gDebugDraw, which
/// you can use to draw debug information. It exposes a number of methods for drawing
/// a variety of debug primitives, including lines, triangles and boxes.
/// Internally, DebugDrawer maintains a list of active debug primitives, and draws the
/// contents of the list after each frame is done rendering. This way, you can be
/// assured that your debug rendering won't interfere with TSE's various effect
/// rendering passes or render-to-target calls.
///
/// The DebugDrawer can also be used for more interesting uses, like freezing its
/// primitive list so you can look at a situation more closely, or dumping the
/// primitive list to disk for closer analysis.
///
/// DebugDrawer is accessible by script under the name DebugDrawer, and by C++ under
/// the symbol gDebugDraw. There are a variety of methods available for drawing
/// different sorts of output; see the class reference for more information.
///
/// DebugDrawer works solely in worldspace. Primitives are rendered with cull mode of
/// none.
///
class DebugDrawer : public SimObject
{
public:
   DECLARE_CONOBJECT(DebugDrawer);

   DebugDrawer();
   ~DebugDrawer();

   static DebugDrawer* get();
   
   /// Called at engine init to set up the global debug draw object.
   static void init();

   /// Called globally to render debug draw state. Also does state updates.
   void render();

   void toggleFreeze()  { shouldToggleFreeze = true; };
   void toggleDrawing() 
   {
#ifdef ENABLE_DEBUGDRAW
      isDrawing = !isDrawing;
#endif
   };


   /// @name ddrawmeth Debug Draw Methods
   ///
   /// @{

   void drawBox(const Point3F &a, const Point3F &b, const ColorF &color = ColorF(1.0f,1.0f,1.0f));
   void drawLine(const Point3F &a, const Point3F &b, const ColorF &color = ColorF(1.0f,1.0f,1.0f));	
   void drawTri(const Point3F &a, const Point3F &b, const Point3F &c, const ColorF &color = ColorF(1.0f,1.0f,1.0f));
   void drawText(const Point3F& pos, const String& text, const ColorF &color = ColorF(1.0f,1.0f,1.0f));

   /// Render a wireframe view of the given polyhedron.
   void drawPolyhedron( const AnyPolyhedron& polyhedron, const ColorF& color = ColorF( 1.f, 1.f, 1.f ) );

   /// Render the plane indices, edge indices, edge direction indicators, and point coordinates
   /// of the given polyhedron for debugging.
   ///
   /// Green lines are plane normals.  Red lines point from edge midpoints along the edge direction (i.e. to the
   /// second vertex).  This shows if the orientation is correct to yield CW ordering for face[0].  Indices and
   /// coordinates of vertices are shown in white.  Plane indices are rendered in black.  Edge indices and their
   /// plane indices are rendered in white.
   void drawPolyhedronDebugInfo( const AnyPolyhedron& polyhedron, const MatrixF& transform, const Point3F& scale );

   /// Set the TTL for the last item we entered...
   ///
   /// Primitives default to lasting one frame (ie, ttl=0)
   enum {
      DD_INFINITE = U32_MAX
   };
   // How long should this primitive be draw for, 0 = one frame, DD_INFINITE = draw forever
   void setLastTTL(U32 ms);

   /// Disable/enable z testing on the last primitive.
   ///
   /// Primitives default to z testing on.
   void setLastZTest(bool enabled);

   /// @}
private:
   typedef SimObject Parent;

   static DebugDrawer* sgDebugDrawer;

   struct DebugPrim
   {
      /// Color used for this primitive.
      ColorF color;

      /// Points used to store positional data. Exact semantics determined by type.
      Point3F a, b, c;
      enum {
         Tri,
         Box,
         Line,
         Text
      } type;	   ///< Type of the primitive. The meanings of a,b,c are determined by this.

      SimTime dieTime;   ///< Time at which we should remove this from the list.
      bool useZ; ///< If true, do z-checks for this primitive.      
      char mText[256];      // Text to display

      DebugPrim *next;
   };


   FreeListChunker<DebugPrim> mPrimChunker;
   DebugPrim *mHead;

   bool isFrozen;
   bool shouldToggleFreeze;
   bool isDrawing;   

   GFXStateBlockRef mRenderZOffSB;
   GFXStateBlockRef mRenderZOnSB;

   Resource<GFont> mFont;

   void setupStateBlocks();
};

#endif // _DEBUGDRAW_H_
