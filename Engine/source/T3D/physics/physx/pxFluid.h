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

#ifndef _PXFLUID_H_
#define _PXFLUID_H_

#ifndef _SCENEOBJECT_H_
#include "scene/sceneObject.h"
#endif
#ifndef _PHYSX_H_
#include "T3D/physics/physx/px.h"
#endif

class BaseMatInstance;
class PxWorld;
class NxScene;


class PxFluid : public SceneObject
{
   typedef SceneObject Parent;

protected:
   
   enum NetMasks
   {
      UpdateMask     = Parent::NextFreeMask,
      ResetMask      = Parent::NextFreeMask << 1,
      NextFreeMask   = Parent::NextFreeMask << 2
   };

   struct FluidParticle
   {
      NxVec3   position;
      NxVec3   velocity;
      NxReal	density;
      NxReal   lifetime;
      NxU32	   id;
      NxVec3	collisionNormal;
   };

   #define MAX_PARTICLES 100

public:

   PxFluid();
   virtual ~PxFluid();

   DECLARE_CONOBJECT( PxFluid );      

   // SimObject
   virtual bool onAdd();
   virtual void onRemove();
   static void initPersistFields();
   virtual void inspectPostApply();

   // NetObject
   virtual U32 packUpdate( NetConnection *conn, U32 mask, BitStream *stream );
   virtual void unpackUpdate( NetConnection *conn, BitStream *stream );

   // SceneObject
   virtual void setTransform( const MatrixF &mat );
   virtual void setScale( const VectorF &scale );
   virtual void prepRenderImage( SceneRenderState *state );

   void resetParticles();
   void setRate( F32 rate );

protected:

   void renderObject( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat );

   void _createFluid();
   void _destroyFluid();

protected:

   PxWorld *mWorld;
   NxScene *mScene;

   FluidParticle *mParticles;
   //NxParticleData *mParticleData;
   NxFluid *mFluid;
   U32 mParticleCount;
   NxFluidEmitter *mEmitter;
};

#endif // _PXFLUID_H_