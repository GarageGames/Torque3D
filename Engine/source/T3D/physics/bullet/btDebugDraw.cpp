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
#include "T3D/physics/bullet/btDebugDraw.h"

#include "T3D/physics/bullet/btCasts.h"
#include "gfx/gfxDevice.h"
#include "math/util/frustum.h"
#include "gfx/primBuilder.h"


void BtDebugDraw::drawLine( const btVector3 &fromBt, const btVector3 &toBt, const btVector3 &color )
{
   Point3F from = btCast<Point3F>( fromBt );
   Point3F to = btCast<Point3F>( toBt );

   // Cull first if we have a frustum.
   //F32 distSquared = ( mCuller->getPosition() - from ).lenSquared();
   //if ( mCuller && distSquared > ( 150 * 150 ) ) //!mCuller->clipSegment( from, to ) )
      //return;

   // Do we need to flush the builder?
   if ( mVertexCount + 2 >= 1000 )
      flush();

   // Are we starting a new primitive?
   if ( mVertexCount == 0 )
      PrimBuild::begin( GFXLineList, 1000 );

   PrimBuild::color3f( color.x(), color.y(), color.z() );
   PrimBuild::vertex3f( from.x, from.y, from.z );
   PrimBuild::vertex3f( to.x, to.y, to.z );

   mVertexCount += 2;
}

void BtDebugDraw::drawTriangle(  const btVector3 &v0,
                                 const btVector3 &v1,
                                 const btVector3 &v2,
                                 const btVector3 &color, 
                                 btScalar /*alpha*/ )
{
	drawLine(v0,v1,color);
	drawLine(v1,v2,color);
	drawLine(v2,v0,color);
}

void BtDebugDraw::drawContactPoint( const btVector3 &pointOnB, 
                                    const btVector3 &normalOnB, 
                                    btScalar distance, 
                                    int lifeTime, const 
                                    btVector3 &color )
{
   drawLine( pointOnB, pointOnB+normalOnB*distance, color );
}

void BtDebugDraw::flush()
{
   // Do we have verts to render?
   if ( mVertexCount == 0 )
      return;

   PrimBuild::end();
   mVertexCount = 0;
}
