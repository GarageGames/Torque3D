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

#ifndef _MESH_EMITTER_H
#define _MESH_EMITTER_H

#ifndef _GAMEBASE_H_
#include "T3D/gameBase/gameBase.h"
#endif

#include "renderInstance/renderPassManager.h"

#include <T3D/fx/ParticleSystem/particle.h>
#include <T3D/fx/ParticleSystem/particleEmitter.h>
#include <T3D/fx/ParticleSystem/particleSystem.h>
#include <T3D/fx/ParticleSystem/Utility/psMeshParsing.h>

//-----------------------------------------------
//! A datablock for @ref SphereEmitter's.
//! @ingroup particleemitters
//-----------------------------------------------
class MeshEmitterData : public ParticleEmitterData
{
   typedef ParticleEmitterData Parent;

public:
   MeshEmitterData();

   virtual ParticleEmitter* CreateEmitter(ParticleSystem *system);

   // Script interface
   static void  initPersistFields();

   // Networking
   virtual void packData(BitStream* stream);
   virtual void unpackData(BitStream* stream);

   /// @name Getters and setters
   /// @{
   F32 getEjectionVelocity() { return mEjectionVelocity; };
   F32 getVelocityVariance() { return mVelocityVariance; };
   F32 getEjectionOffset() { return mEjectionOffset; };
   F32 getEjectionOffsetVariance() { return mEjectionOffsetVariance; };
   /// @}

public:
   /// @name Mesh Fields
   /// @{
   StringTableEntry		mEmitMesh;				///< Id of the object that has a mesh that we want to emit particles on
   bool					mEvenEmission;			///< Even the emission
   bool					mEmitOnFaces;			///< If true, emits particles on faces rather than vertices
   /// @}

   /// @name Emission Fields
   /// @{
   F32 mEjectionVelocity; //!< Particle ejection velocity.
   F32 mVelocityVariance; //!< Variance for ejection velocity, from 0 - ejectionVelocity.
   F32 mEjectionOffset; //!< Distance along ejection Z axis from which to eject particles.
   // TODO: Implement OffsetVariance
   F32 mEjectionOffsetVariance; //!< Distance Padding along ejection Z axis from which to eject particles.
   /// @}

   DECLARE_CONOBJECT(MeshEmitterData);
};

//-----------------------------------------------
//! A ParticleEmitter that emits particles along
//! the geometry of a mesh.
//! @ingroup particleemitters
//! @see psMeshInterface
//-----------------------------------------------
class MeshEmitter : public ParticleEmitter
{
public:
   MeshEmitter(ParticleSystem* parentSystem);

   virtual bool addParticle(Point3F const& pos,
      Point3F const& axis,
      Point3F const& vel,
      Point3F const& axisx);

   /// Get a point distributed amongst the vertices of the mesh.
   /// @param[out] pNew particle to attach the point to.
   virtual bool getPointOnVertex(SimObject *SB, psMeshInterface* psMesh, Particle *pNew);

   /// Get a point distributed amongst the surfaces of the mesh.
   /// @param[out] pNew particle to attach the point to.
   virtual bool getPointOnFace(SimObject *SB, psMeshInterface* psMesh, Particle *pNew);

   /// Prefetch the surfaces of the mesh into the emitFaces buffer.
   virtual void loadFaces(SimObject *SB, psMeshInterface* psMesh);

   /// Prefetch the surfaces of the mesh into the emitFaces buffer.
   virtual void loadFaces();

   /// Delegate for debugrendering (TODO: enable debug rendering)
   virtual void debugRenderDelegate(ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance* overrideMat) { };
   virtual void bindDelegate(ObjectRenderInst *ori) { ori->renderDelegate.bind(this, &MeshEmitter::debugRenderDelegate); };


   virtual MeshEmitterData* getDataBlock() { return dynamic_cast<MeshEmitterData*>(mDataBlock); };

private:
   Vector<psMeshParsing::face>	emitfaces;				///< Faces to emit particles on
   U32						vertexCount;			///< Amount of vertices in the mesh

   SimObjectId currentMesh;   ///< ID of the current mesh

   S32						mainTime; ///< Logical time
};

#endif //_MESH_EMITTER_H