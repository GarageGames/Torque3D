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

#include "platform/platform.h"
#include "gfx/sim/debugDraw.h"

#include "gfx/gFont.h"
#include "gfx/gfxDrawUtil.h"
#include "gfx/gfxTransformSaver.h"
#include "gfx/gfxDebugEvent.h"
#include "math/mathUtils.h"
#include "math/util/frustum.h"
#include "console/console.h"
#include "scene/sceneManager.h"
#include "core/module.h"
#include "console/engineAPI.h"

#include "math/mPolyhedron.impl.h"


MODULE_BEGIN( DebugDrawer )

   MODULE_INIT_AFTER( Sim )
   MODULE_INIT_AFTER( GFX )
   
   // DebugDrawer will register itself as a SimObject and
   // thus get automatically shut down with Sim.

   MODULE_INIT
   {
      DebugDrawer::init();
   }

MODULE_END;


DebugDrawer* DebugDrawer::sgDebugDrawer = NULL;

IMPLEMENT_CONOBJECT(DebugDrawer);

ConsoleDocClass( DebugDrawer, 
   "@brief A debug helper for rendering debug primitives to the scene.\n\n"

   "The DebugDrawer is used to render debug primitives to the scene for testing.  It is "
   "often useful when debugging collision code or complex 3d algorithms to have "
   "them draw debug information, like culling hulls or bounding volumes, normals, "
   "simple lines, and so forth.\n\n"

   "A key feature of the DebugDrawer is that each primitive gets a \"time to live\" (TTL) "
   "which allows them to continue to render to the scene for a fixed period of time.  You "
   "can freeze or resume the system at any time to allow you to examine the output.\n"

   "@tsexample\n"
   "DebugDraw.drawLine( %player.getMuzzlePoint( 0 ), %hitPoint );\n"
   "DebugDraw.setLastTTL( 5000 ); // 5 seconds.\n"
   "@endtsexample\n"

   "The DebugDrawer renders solely in world space and all primitives are rendered with the "
   "cull mode disabled.\n"

   "@note This feature can easily be used to cheat in online games, so you should be sure "
   "it is disabled in your shipping game.  By default the DebugDrawer is disabled in all "
   "TORQUE_SHIPPING builds.\n"

   "@ingroup GFX\n" );

DebugDrawer::DebugDrawer()
{
   mHead = NULL;
   isFrozen = false;
   shouldToggleFreeze = false;
   
#ifdef ENABLE_DEBUGDRAW
   isDrawing = true;
#else
   isDrawing = false;
#endif
}

DebugDrawer::~DebugDrawer()
{
   if( sgDebugDrawer == this )
      sgDebugDrawer = NULL;
}

DebugDrawer* DebugDrawer::get()
{
   if (sgDebugDrawer)
   {   
      return sgDebugDrawer;
   } else {
      DebugDrawer::init();
      return sgDebugDrawer;
   }
}

void DebugDrawer::init()
{
#ifdef ENABLE_DEBUGDRAW
   sgDebugDrawer = new DebugDrawer();
   sgDebugDrawer->registerObject("DebugDraw");
   Sim::getRootGroup()->addObject( sgDebugDrawer );
   Con::warnf( "DebugDrawer Enabled!" );
#endif
}

void DebugDrawer::setupStateBlocks()
{
   GFXStateBlockDesc d;

   d.setCullMode(GFXCullNone);
   mRenderZOnSB = GFX->createStateBlock(d);
   
   d.setZReadWrite(false);
   mRenderZOffSB = GFX->createStateBlock(d);
}

void DebugDrawer::render()
{
#ifdef ENABLE_DEBUGDRAW
   if(!isDrawing)
      return;

   GFXDEBUGEVENT_SCOPE( DebugDrawer, ColorI::GREEN );

   if (!mRenderZOnSB)
   {
      setupStateBlocks();
      String fontCacheDir = Con::getVariable("$GUI::fontCacheDirectory");
      mFont = GFont::create("Arial", 12, fontCacheDir);
   }

   SimTime curTime = Sim::getCurrentTime();
  
   GFX->disableShaders();   
   
   for(DebugPrim **walk = &mHead; *walk; )
   {
      DebugPrim *p = *walk;

      // Set up the state block...
      GFXStateBlockRef currSB;
      if(p->useZ)
         currSB = mRenderZOnSB;
      else
         currSB = mRenderZOffSB;
      GFX->setStateBlock( currSB );

      Point3F d;

      switch(p->type)
      {
      case DebugPrim::Tri:
         PrimBuild::begin( GFXLineStrip, 4);

         PrimBuild::color(p->color);

         PrimBuild::vertex3fv(p->a);
         PrimBuild::vertex3fv(p->b);
         PrimBuild::vertex3fv(p->c);
         PrimBuild::vertex3fv(p->a);

         PrimBuild::end();
         break;
      case DebugPrim::Box:
         d = p->a - p->b;
         GFX->getDrawUtil()->drawCube(currSB->getDesc(), d * 0.5, (p->a + p->b) * 0.5, p->color);
         break;
      case DebugPrim::Line:
         PrimBuild::begin( GFXLineStrip, 2);

         PrimBuild::color(p->color);

         PrimBuild::vertex3fv(p->a);
         PrimBuild::vertex3fv(p->b);

         PrimBuild::end();
         break;
      case DebugPrim::Text:
         {
            GFXTransformSaver saver;            
            Point3F result;
            if (MathUtils::mProjectWorldToScreen(p->a, &result, GFX->getViewport(), GFX->getWorldMatrix(), GFX->getProjectionMatrix()))
            {
               GFX->setClipRect(GFX->getViewport());
               GFX->getDrawUtil()->setBitmapModulation(p->color); 
               GFX->getDrawUtil()->drawText(mFont, Point2I(result.x, result.y), p->mText);
            }
         }
         break;
      }

      // Ok, we've got data, now freeze here if needed.
      if (shouldToggleFreeze)
      {
         isFrozen = !isFrozen;
         shouldToggleFreeze = false;
      }

      if(p->dieTime <= curTime && !isFrozen && p->dieTime != U32_MAX)
      {
         *walk = p->next;
         mPrimChunker.free(p);
      }
      else
         walk = &((*walk)->next);
   }
#endif
}

void DebugDrawer::drawBox(const Point3F &a, const Point3F &b, const ColorF &color)
{
   if(isFrozen || !isDrawing)
      return;

   DebugPrim *n = mPrimChunker.alloc();

   n->useZ = true;
   n->dieTime = 0;
   n->a = a;
   n->b = b;
   n->color = color;
   n->type = DebugPrim::Box;

   n->next = mHead;
   mHead = n;
}

void DebugDrawer::drawLine(const Point3F &a, const Point3F &b, const ColorF &color)
{
   if(isFrozen || !isDrawing)
      return;

   DebugPrim *n = mPrimChunker.alloc();

   n->useZ = true;
   n->dieTime = 0;
   n->a = a;
   n->b = b;
   n->color = color;
   n->type = DebugPrim::Line;

   n->next = mHead;
   mHead = n;
}

void DebugDrawer::drawTri(const Point3F &a, const Point3F &b, const Point3F &c, const ColorF &color)
{
   if(isFrozen || !isDrawing)
      return;

   DebugPrim *n = mPrimChunker.alloc();

   n->useZ = true;
   n->dieTime = 0;
   n->a = a;
   n->b = b;
   n->c = c;
   n->color = color;
   n->type = DebugPrim::Tri;

   n->next = mHead;
   mHead = n;
}

void DebugDrawer::drawPolyhedron( const AnyPolyhedron& polyhedron, const ColorF& color )
{
   const PolyhedronData::Edge* edges = polyhedron.getEdges();
   const Point3F* points = polyhedron.getPoints();
   const U32 numEdges = polyhedron.getNumEdges();

   for( U32 i = 0; i < numEdges; ++ i )
   {
      const PolyhedronData::Edge& edge = edges[ i ];
      drawLine( points[ edge.vertex[ 0 ] ], points[ edge.vertex[ 1 ] ], color );
   }
}

void DebugDrawer::drawPolyhedronDebugInfo( const AnyPolyhedron& polyhedron, const MatrixF& transform, const Point3F& scale )
{
   Point3F center = polyhedron.getCenterPoint();
   center.convolve( scale );
   transform.mulP( center );

   // Render plane indices and normals.

   const U32 numPlanes = polyhedron.getNumPlanes();
   for( U32 i = 0; i < numPlanes; ++ i )
   {
      const AnyPolyhedron::PlaneType& plane = polyhedron.getPlanes()[ i ];

      Point3F planePos = plane.getPosition();
      planePos.convolve( scale );
      transform.mulP( planePos );

      Point3F normal = plane.getNormal();
      transform.mulV( normal );

      drawText( planePos, String::ToString( i ), ColorI::BLACK );
      drawLine( planePos, planePos + normal, ColorI::GREEN );
   }

   // Render edge indices and direction indicators.

   const U32 numEdges = polyhedron.getNumEdges();
   for( U32 i = 0; i < numEdges; ++ i )
   {
      const AnyPolyhedron::EdgeType& edge = polyhedron.getEdges()[ i ];

      Point3F v1 = polyhedron.getPoints()[ edge.vertex[ 0 ] ];
      Point3F v2 = polyhedron.getPoints()[ edge.vertex[ 1 ] ];

      v1.convolve( scale );
      v2.convolve( scale );
      transform.mulP( v1 );
      transform.mulP( v2 );

      const Point3F midPoint = v1 + ( v2 - v1 ) / 2.f;

      drawText( midPoint, String::ToString( "%i (%i, %i)", i, edge.face[ 0 ], edge.face[ 1 ] ), ColorI::WHITE );

      // Push out the midpoint away from the center to place the direction indicator.

      Point3F pushDir = midPoint - center;
      pushDir.normalize();
      const Point3F dirPoint = midPoint + pushDir;
      const Point3F lineDir = ( v2 - v1 ) / 2.f;

      drawLine( dirPoint, dirPoint + lineDir, ColorI::RED );
   }

   // Render point indices and coordinates.

   const U32 numPoints = polyhedron.getNumPoints();
   for( U32 i = 0; i < numPoints; ++ i )
   {
      Point3F p = polyhedron.getPoints()[ i ];

      p.convolve( scale );
      transform.mulP( p );

      drawText( p, String::ToString( "%i: (%.2f, %.2f, %.2f)", i, p.x, p.y, p.z ), ColorF::WHITE );
   }
}

void DebugDrawer::drawText(const Point3F& pos, const String& text, const ColorF &color)
{
   if(isFrozen || !isDrawing)
      return;

   DebugPrim *n = mPrimChunker.alloc();

   n->useZ = false;
   n->dieTime = 0;
   n->a = pos;
   n->color = color;
   dStrncpy(n->mText, text.c_str(), 256);   
   n->type = DebugPrim::Text;

   n->next = mHead;
   mHead = n;
}

void DebugDrawer::setLastTTL(U32 ms)
{
   AssertFatal(mHead, "Tried to set last with nothing in the list!");
   if (ms != U32_MAX)
      mHead->dieTime = Sim::getCurrentTime() + ms;
   else
      mHead->dieTime = U32_MAX;
}

void DebugDrawer::setLastZTest(bool enabled)
{
   AssertFatal(mHead, "Tried to set last with nothing in the list!");
   mHead->useZ = enabled;
}

DefineEngineMethod( DebugDrawer, drawLine, void, ( Point3F a, Point3F b, ColorF color ), ( ColorF::WHITE ),
   "Draws a line primitive between two 3d points." )
{
   object->drawLine( a, b, color );
}

DefineEngineMethod( DebugDrawer, drawBox, void, ( Point3F a, Point3F b, ColorF color ), ( ColorF::WHITE ),
   "Draws an axis aligned box primitive within the two 3d points." )
{
   object->drawBox( a, b, color );
}

DefineEngineMethod( DebugDrawer, setLastTTL, void, ( U32 ms ),,
   "Sets the \"time to live\" (TTL) for the last rendered primitive." )
{
   object->setLastTTL( ms );
}

DefineEngineMethod( DebugDrawer, setLastZTest, void, ( bool enabled ),,
   "Sets the z buffer reading state for the last rendered primitive." )
{
   object->setLastZTest( enabled );
}

DefineEngineMethod( DebugDrawer, toggleFreeze, void, (),,
   "Toggles freeze mode which keeps the currently rendered primitives from expiring." )
{
   object->toggleFreeze();
}

DefineEngineMethod( DebugDrawer, toggleDrawing, void, (),,
   "Toggles the rendering of DebugDrawer primitives." )
{
   object->toggleDrawing();
}

