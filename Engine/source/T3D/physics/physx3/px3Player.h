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

#ifndef _PX3PLAYER_H
#define _PX3PLAYER_H

#ifndef _PHYSX3_H_
#include "T3D/physics/physx3/px3.h"
#endif
#ifndef _T3D_PHYSICS_PHYSICSPLAYER_H_
#include "T3D/physics/physicsPlayer.h"
#endif
#ifndef _T3D_PHYSICSCOMMON_H_
#include "T3D/physics/physicsCommon.h"
#endif

class Px3World;

class Px3Player : public PhysicsPlayer, public physx::PxUserControllerHitReport
{
protected:

   physx::PxController *mController;
   physx::PxCapsuleGeometry mGeometry;
   F32 mSkinWidth;

   Px3World *mWorld;

   SceneObject *mObject;

   /// Used to get collision info out of the
   /// PxUserControllerHitReport callbacks.
   CollisionList *mCollisionList;

   ///
   F32 mOriginOffset;

   ///
   F32 mStepHeight;
   U32 mElapsed;
   ///
   void _releaseController();


   virtual void onShapeHit( const physx::PxControllerShapeHit &hit );
   virtual void onControllerHit( const physx::PxControllersHit &hit );
   virtual void onObstacleHit(const physx::PxControllerObstacleHit &){}

   void _findContact( SceneObject **contactObject, VectorF *contactNormal ) const;

   void _onPhysicsReset( PhysicsResetEvent reset );

   void _onStaticChanged();

public:
   
	Px3Player();
	virtual ~Px3Player();	

   // PhysicsObject
   virtual PhysicsWorld* getWorld();
   virtual void setTransform( const MatrixF &transform );
   virtual MatrixF& getTransform( MatrixF *outMatrix );
   virtual void setScale( const Point3F &scale );
   virtual Box3F getWorldBounds();
   virtual void setSimulationEnabled( bool enabled ) {}
   virtual bool isSimulationEnabled() { return true; }

   // PhysicsPlayer
   virtual void init(   const char *type, 
                        const Point3F &size,
                        F32 runSurfaceCos,
                        F32 stepHeight,
                        SceneObject *obj, 
                        PhysicsWorld *world );
   virtual Point3F move( const VectorF &displacement, CollisionList &outCol );
   virtual void findContact( SceneObject **contactObject, VectorF *contactNormal, Vector<SceneObject*> *outOverlapObjects ) const;
   virtual bool testSpacials( const Point3F &nPos, const Point3F &nSize ) const;
   virtual void setSpacials( const Point3F &nPos, const Point3F &nSize );
   virtual void enableCollision();
   virtual void disableCollision();
};


#endif // _PX3PLAYER_H_
