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

#ifndef _PXPLAYER_H
#define _PXPLAYER_H

#ifndef _PHYSX_H_
#include "T3D/physics/physX/px.h"
#endif
#ifndef _T3D_PHYSICS_PHYSICSPLAYER_H_
#include "T3D/physics/physicsPlayer.h"
#endif
#ifndef _T3D_PHYSICSCOMMON_H_
#include "T3D/physics/physicsCommon.h"
#endif


class PxWorld;
class NxController;


class PxPlayer : public PhysicsPlayer, public NxUserControllerHitReport
{
protected:

   NxController *mController;

   F32 mSkinWidth;

   PxWorld *mWorld;

   SceneObject *mObject;

   /// Used to get collision info out of the
   /// NxUserControllerHitReport callbacks.
   CollisionList *mCollisionList;

   ///
   F32 mOriginOffset;

   ///
   F32 mStepHeight;

   ///
   void _releaseController();

   // NxUserControllerHitReport
   virtual NxControllerAction onShapeHit( const NxControllerShapeHit& hit );
   virtual NxControllerAction onControllerHit( const NxControllersHit& hit );

   void _findContact( SceneObject **contactObject, VectorF *contactNormal ) const;

   void _onPhysicsReset( PhysicsResetEvent reset );

   void _onStaticChanged();

public:
   
	PxPlayer();
	virtual ~PxPlayer();	

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
   virtual bool testSpacials( const Point3F &nPos, const Point3F &nSize ) const { return true; }
   virtual void setSpacials( const Point3F &nPos, const Point3F &nSize ) {}
   virtual void enableCollision();
   virtual void disableCollision();
};


#endif // _PXPLAYER_H