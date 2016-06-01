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

#ifndef _BTPLAYER_H
#define _BTPLAYER_H

#ifndef _T3D_PHYSICS_PHYSICSPLAYER_H_
#include "T3D/physics/physicsPlayer.h"
#endif


class BtWorld;
//class btKinematicCharacterController;
class btPairCachingGhostObject;
class btConvexShape;
class btVector3;


class BtPlayer : public PhysicsPlayer
{
protected:

   //F32 mSkinWidth;

   BtWorld *mWorld;

   SceneObject *mObject;

   ///
   //btKinematicCharacterController *mController;
   
   ///
   btPairCachingGhostObject *mGhostObject;

   ///
   btConvexShape *mColShape;

   ///
   F32 mOriginOffset;

   ///
   F32 mSlopeAngle;
   ///

   ///
   F32 mStepHeight;
   ///
   void _releaseController();

   ///
   bool _recoverFromPenetration();

   ///
   bool _sweep( btVector3 *inOutCurrPos, const btVector3 &disp, CollisionList *outCol );

   ///
   void _stepForward( btVector3 *inOutCurrPos, const btVector3 &displacement, CollisionList *outCol );

public:

   BtPlayer();
   virtual ~BtPlayer();	

   // PhysicsObject
   virtual PhysicsWorld* getWorld();
   virtual void setTransform( const MatrixF &transform );
   virtual MatrixF& getTransform( MatrixF *outMatrix );
   virtual void setScale( const Point3F &scale );   
   virtual Box3F getWorldBounds() { return Box3F::Invalid; }
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


#endif // _BTPLAYER_H
