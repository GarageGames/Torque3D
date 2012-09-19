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

#ifndef _T3D_PHYSICS_BTDEBUGDRAW_H_
#define _T3D_PHYSICS_BTDEBUGDRAW_H_

#ifndef _BULLET_H_
#include "T3D/physics/bullet/bt.h"
#endif

class Frustum;


class BtDebugDraw : public btIDebugDraw
{
protected:

   /// The number of verts we've used in rendering.
   U32 mVertexCount;

   /// The frustum to use for culling or NULL.
   const Frustum *mCuller;

public:

   BtDebugDraw()
      :  mVertexCount( 0 ),
         mCuller( NULL )
   {
   }

   /// Sets the culler which we use to cull out primitives
   /// that are completely offscreen.
   void setCuller( const Frustum *culler ) { mCuller = culler; }

   /// Call this after debug drawing to submit any
   /// remaining primitives for rendering.
   void flush();

   // btIDebugDraw
	virtual void drawLine( const btVector3 &from, const btVector3 &to, const btVector3 &color );
   virtual void drawTriangle(const btVector3& v0,const btVector3& v1,const btVector3& v2,const btVector3& color, btScalar /*alpha*/);
   virtual void drawContactPoint( const btVector3 &PointOnB, const btVector3 &normalOnB, btScalar distance, int lifeTime, const btVector3 &color );
   virtual void reportErrorWarning( const char *warningString ) {}
   virtual void draw3dText( const btVector3 &location, const char *textString ) {}
   virtual void setDebugMode( int debugMode ) {}
   virtual int getDebugMode() const { return DBG_DrawWireframe; }
};


#endif // _T3D_PHYSICS_BTDEBUGDRAW_H_