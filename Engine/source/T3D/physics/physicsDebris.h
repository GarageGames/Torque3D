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

#ifndef _PHYSICS_DEBRIS_H_
#define _PHYSICS_DEBRIS_H_

#ifndef __RESOURCE_H__
#include "core/resource.h"
#endif
#ifndef _GAMEBASE_H_
#include "T3D/gameBase/gameBase.h"
#endif
#ifndef _T3D_PHYSICSCOMMON_H_
#include "T3D/physics/physicsCommon.h"
#endif


class TSShapeInstance;
class TSShape;

//**************************************************************************
// Debris Data
//**************************************************************************
class PhysicsDebrisData : public GameBaseData
{
   typedef GameBaseData Parent;

public:


   F32      lifetime;
   F32      lifetimeVariance;

   ///
   F32 mass;

   /// 
   F32 dynamicFriction;

   /// 
   F32 staticFriction;

   ///
   F32 restitution;

   ///
   F32 linearDamping;

   ///
   F32 angularDamping;

   /// 
   F32 linearSleepThreshold;

   ///
   F32 angularSleepThreshold;

   // A scale applied to the normal linear and angular damping
   // when the object enters a water volume.
   F32 waterDampingScale;

   // The density of this object used for water buoyancy effects.
   F32 buoyancyDensity;

   /// Is rendererd during shadow passes.
   bool castShadows;

   const char* shapeName;
   Resource<TSShape> shape;

   PhysicsDebrisData();

   bool        onAdd();
   bool        preload( bool server, String &errorStr );
   static void initPersistFields();
   void        packData( BitStream *stream );
   void        unpackData( BitStream *stream );

   DECLARE_CONOBJECT( PhysicsDebrisData );

};



class PhysicsBody;
class PhysicsWorld;


class PhysicsDebris : public GameBase
{
   typedef GameBase Parent;

public:

   /// Helper method which creates debris based on the 
   /// datablock and initial state.
   ///
   /// It can return NULL if the system quality settings 
   /// are set to disable the debris.
   ///
   static PhysicsDebris* create( PhysicsDebrisData *datablock,
                                 const MatrixF &transform,
                                 const VectorF &linVel );

   PhysicsDebris();
   virtual ~PhysicsDebris();

   DECLARE_CONOBJECT(PhysicsDebris);
   static void initPersistFields();

   bool onAdd();
   void onRemove(); 

   void applyImpulse( const Point3F &pos, const VectorF &vec );
   void applyRadialImpulse( const Point3F &origin, F32 radius, F32 magnitude );

protected:

   /// The global object lifetime scalar.
   static F32 smLifetimeScale;

   void processTick( const Move *move );
   void advanceTime( F32 dt );
   void interpolateTick( F32 delta );
   bool onNewDataBlock( GameBaseData *dptr, bool reload );

   void prepRenderImage( SceneRenderState *state );
   void prepBatchRender( SceneRenderState *state );

   void _deleteFragments();
   void _createFragments();

   void _updateForces( PhysicsBody *body, const Box3F &bounds );

   void _findNodes( U32 objId, Vector<U32> &nodeIds );

   void _onPhysicsReset( PhysicsResetEvent reset );
   
protected:

   F32 mLifetime;

   Point3F mInitialLinVel;

   PhysicsDebrisData *mDataBlock;

   TSShapeInstance *mShapeInstance;      
   
   PhysicsWorld *mWorld;

   struct Fragment
   {
      Vector<U32> nodeIds;

      PhysicsBody *body;

      // The delta state.
      Point3F pos;
      Point3F lastPos;
      QuatF rot;
      QuatF lastRot;
   };

   typedef Vector<Fragment> FragmentVector;

   FragmentVector mFragments;

};


#endif // _PHYSICS_DEBRIS_H_
