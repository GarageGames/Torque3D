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

#ifndef _T3D_PHYSICS_PHYSICSPLAYER_H_
#define _T3D_PHYSICS_PHYSICSPLAYER_H_

#ifndef _T3D_PHYSICS_PHYSICSOBJECT_H_
#include "T3D/physics/physicsObject.h"
#endif
#ifndef _MMATH_H_
#include "math/mMath.h"
#endif

class CollisionList;
//struct ObjectRenderInst;
//class BaseMatInstance;
//class Player;
//class SceneState;
class SceneObject;


///
class PhysicsPlayer : public PhysicsObject
{
public:

   PhysicsPlayer() {}

   virtual ~PhysicsPlayer() {};

   ///
   virtual void init(   const char *type, 
                        const Point3F &size,
                        F32 runSurfaceCos,
                        F32 stepHeight,
                        SceneObject *obj, 
                        PhysicsWorld *world ) = 0;

   virtual void findContact(  SceneObject **contactObject, 
                              VectorF *contactNormal,
                              Vector<SceneObject*> *outOverlapObjects ) const = 0;

   virtual Point3F move( const VectorF &displacement, CollisionList &outCol ) = 0;

   virtual bool testSpacials( const Point3F &nPos, const Point3F &nSize ) const = 0;

   virtual void setSpacials( const Point3F &nPos, const Point3F &nSize ) = 0;

   virtual void enableCollision() = 0;

   virtual void disableCollision() = 0;
};


#endif // _T3D_PHYSICS_PHYSICSPLAYER_H_