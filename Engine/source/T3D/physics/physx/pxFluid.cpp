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
#include "T3D/physics/physx/pxFluid.h"

#include "console/consoleTypes.h"
#include "scene/sceneRenderState.h"
#include "renderInstance/renderPassManager.h"
#include "T3D/physics/physicsPlugin.h"
#include "T3D/physics/physx/pxWorld.h"
#include "T3D/physics/physx/pxCasts.h"
#include "gfx/gfxDrawUtil.h"
#include "math/mathIO.h"
#include "core/stream/bitStream.h"


IMPLEMENT_CO_NETOBJECT_V1( PxFluid );

ConsoleDocClass( PxFluid,
   "@brief Experimental and unfinished Torque wrapper class for NxFluid.\n\n"
   "@internal\n"
);

PxFluid::PxFluid()
 : mWorld( NULL ),
   mScene( NULL ),
   mParticles( NULL ),
   mFluid( NULL ),
   mEmitter( NULL ),
   mParticleCount( 0 )
{
   mNetFlags.set( Ghostable | ScopeAlways );
   mTypeMask |= StaticObjectType | StaticShapeObjectType;
}

PxFluid::~PxFluid()
{

}

bool PxFluid::onAdd()
{
   if ( !Parent::onAdd() )
      return false;

   mWorld = dynamic_cast<PxWorld*>( PHYSICSMGR->getWorld( isServerObject() ? "server" : "client" ) );

   if ( !mWorld || !mWorld->getScene() )
   {
      Con::errorf( "PxMultiActor::onAdd() - PhysXWorld not initialized!" );
      return false;
   }

   mScene = mWorld->getScene();

   if ( isClientObject() )
      _createFluid();
   
   Point3F halfScale = Point3F::One * 0.5f;
   mObjBox.minExtents = -halfScale;
   mObjBox.maxExtents = halfScale;
   resetWorldBox();

   addToScene();

   return true;
}

void PxFluid::onRemove()
{
   if ( isClientObject() )
      _destroyFluid();

   removeFromScene();

   Parent::onRemove();
}

void PxFluid::initPersistFields()
{
   Parent::initPersistFields();
}

void PxFluid::inspectPostApply()
{
   Parent::inspectPostApply();

   setMaskBits( UpdateMask );
}

U32 PxFluid::packUpdate( NetConnection *conn, U32 mask, BitStream *stream )
{
   U32 retMask = Parent::packUpdate( conn, mask, stream );

   if ( stream->writeFlag( mask & UpdateMask ) )
   {
      mathWrite( *stream, getTransform() );
      mathWrite( *stream, getScale() );

      stream->write( mEmitter ? mEmitter->getRate() : 0 );
   }

   stream->writeFlag( isProperlyAdded() && mask & ResetMask );

   return retMask;
}

void PxFluid::unpackUpdate( NetConnection *conn, BitStream *stream )
{
   Parent::unpackUpdate( conn, stream );

   // UpdateMask
   if ( stream->readFlag() )
   {
      MatrixF mat;
      mathRead( *stream, &mat );
      Point3F scale;
      mathRead( *stream, &scale );

      setScale( scale );
      setTransform( mat );

      F32 rate;
      stream->read( &rate );
      setRate( rate );
   }

   // ResetMask
   if ( stream->readFlag() )
      resetParticles();
}

void PxFluid::setTransform( const MatrixF &mat )
{
   Parent::setTransform( mat );

   if ( mEmitter )
   {
      NxMat34 nxMat;
      nxMat.setRowMajor44( mat );
      mEmitter->setGlobalPose( nxMat );
   }
}

void PxFluid::setScale( const VectorF &scale )
{
   Point3F lastScale = getScale();

   Point3F halfScale = Point3F::One * 0.5f;
   mObjBox.minExtents = -halfScale;
   mObjBox.maxExtents = halfScale;
   resetWorldBox();

   Parent::setScale( scale );

   if ( lastScale != getScale() &&
        mEmitter )
   {      
      _destroyFluid();
      _createFluid();      
   }
}

void PxFluid::prepRenderImage( SceneRenderState *state )
{
   if ( !state->isDiffusePass() )
      return;

   ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
   ri->renderDelegate.bind( this, &PxFluid::renderObject );
   ri->type = RenderPassManager::RIT_Object;
   state->getRenderPass()->addInst( ri );
}

void PxFluid::resetParticles()
{
   if ( mEmitter )
      mEmitter->resetEmission( MAX_PARTICLES );
   setMaskBits( ResetMask );
}

void PxFluid::setRate( F32 rate )
{
   if ( mEmitter )
      mEmitter->setRate( rate );
   setMaskBits( UpdateMask );
}

void PxFluid::renderObject( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat )
{
   GFXStateBlockDesc desc;
   desc.setBlend( true );
   desc.setZReadWrite( true, false );

   for ( U32 i = 0; i < mParticleCount; i++ )
   {
      FluidParticle &particle = mParticles[i];
      Point3F pnt = pxCast<Point3F>( particle.position );

      Box3F box( 0.2f );
      box.minExtents += pnt;
      box.maxExtents += pnt;

      GFX->getDrawUtil()->drawCube( desc, box, ColorI::BLUE );
   }
}

void PxFluid::_createFluid()
{
   /*
   // Set structure to pass particles, and receive them after every simulation step    
   NxParticleData particleData;
   particleData.numParticlesPtr		   = &mParticleCount;
   particleData.bufferPos				   = &mParticles[0].position.x;
   particleData.bufferPosByteStride	   = sizeof(FluidParticle);
   particleData.bufferVel				   = &mParticles[0].velocity.x;
   particleData.bufferVelByteStride	   = sizeof(FluidParticle);
   particleData.bufferLife				   = &mParticles[0].lifetime;
   particleData.bufferLifeByteStride	= sizeof(FluidParticle);   

   // Create a fluid descriptor
   NxFluidDesc fluidDesc;
   fluidDesc.kernelRadiusMultiplier = 2.3f;    
   fluidDesc.restParticlesPerMeter = 10.0f;    
   fluidDesc.stiffness = 200.0f;    
   fluidDesc.viscosity = 22.0f;    
   fluidDesc.restDensity = 1000.0f;    
   fluidDesc.damping = 0.0f;    
   fluidDesc.simulationMethod = NX_F_SPH;    
   fluidDesc.initialParticleData = particleData;    
   fluidDesc.particlesWriteData = particleData;
   */

   NxFluidDesc fluidDesc;    
   fluidDesc.setToDefault();  
   fluidDesc.simulationMethod = NX_F_SPH;               
   fluidDesc.maxParticles = MAX_PARTICLES;      
   fluidDesc.restParticlesPerMeter = 50;       
   fluidDesc.stiffness = 1;       
   fluidDesc.viscosity = 6;
   fluidDesc.flags = NX_FF_VISUALIZATION|NX_FF_ENABLED;
   
   mParticles = new FluidParticle[MAX_PARTICLES];
   dMemset( mParticles, 0, sizeof(FluidParticle) * MAX_PARTICLES );

   NxParticleData &particleData = fluidDesc.particlesWriteData;

   particleData.numParticlesPtr		   = &mParticleCount;
   particleData.bufferPos				   = &mParticles[0].position.x;
   particleData.bufferPosByteStride	   = sizeof(FluidParticle);
   particleData.bufferVel				   = &mParticles[0].velocity.x;
   particleData.bufferVelByteStride	   = sizeof(FluidParticle);
   particleData.bufferLife				   = &mParticles[0].lifetime;
   particleData.bufferLifeByteStride	= sizeof(FluidParticle);   

   mFluid = mScene->createFluid( fluidDesc );

   
   //Create Emitter.
   NxFluidEmitterDesc emitterDesc;
   emitterDesc.setToDefault();
   emitterDesc.dimensionX = getScale().x;
   emitterDesc.dimensionY = getScale().y;
   emitterDesc.relPose.setColumnMajor44( getTransform() );
   emitterDesc.rate = 5.0f;
   emitterDesc.randomAngle = 0.1f;
   emitterDesc.fluidVelocityMagnitude = 6.5f;
   emitterDesc.maxParticles = 0;
   emitterDesc.particleLifetime = 4.0f;
   emitterDesc.type = NX_FE_CONSTANT_FLOW_RATE;
   emitterDesc.shape = NX_FE_ELLIPSE;
   mEmitter = mFluid->createEmitter(emitterDesc);
}

void PxFluid::_destroyFluid()
{
   delete[] mParticles;
   mScene->releaseFluid( *mFluid );
   mEmitter = NULL;
}

ConsoleMethod( PxFluid, resetParticles, void, 2, 2, "" )
{
   object->resetParticles();
}

ConsoleMethod( PxFluid, setRate, void, 2, 2, "" )
{
   object->setRate( dAtof(argv[2]) );
}