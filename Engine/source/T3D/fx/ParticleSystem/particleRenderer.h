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

#ifndef _PARTICLE_RENDERER_H
#define _PARTICLE_RENDERER_H

#ifndef _GAMEBASE_H_
#include "T3D/gameBase/gameBase.h"
#endif
#include <T3D/fx/ParticleSystem/particle.h>

class ParticleSystem;
class ParticleRenderer;

/// @defgroup particlerenderers Renderers
/// @ingroup particlesystem
/// @{

//-----------------------------------------------
//! The interface for ParticleRenderer datablocks.
//! Any class implementing this interface can be
//! assigned to a @ref ParticleSystem.
//! @ingroup particlerenderers
//-----------------------------------------------
class ParticleRendererData : public GameBaseData
{
   typedef GameBaseData Parent;

public:
   ParticleRendererData()
   {
   }

   /// Creates an instance of the ParticleRenderer class associated with
   /// this datablock.
   /// @returns The new ParticleRenderer
   /// @public
   virtual ParticleRenderer* CreateRenderer(ParticleSystem* system) = 0;

   DECLARE_ABSTRACT_CLASS(ParticleRendererData, GameBaseData);
};

//-----------------------------------------------
//! The interface for ParticleRenderers.
//! Any class implementing this interface can act
//! as a ParticleRenderer.
//! @ingroup particlerenderers
//-----------------------------------------------
class ParticleRenderer
{
public:
   ParticleRenderer(ParticleSystem* parentSystem) : mParentSystem(parentSystem) {}
   virtual ~ParticleRenderer() {}

   /// Renders a @ref ParticlePool.
   virtual bool renderPool(ParticlePool *pool, SceneRenderState* state) = 0;

   /// Returns this object's datablock.
   /// @returns The datablock.
   virtual ParticleRendererData* getDataBlock() { return mDataBlock; };

   /// Sets the datablock of this renderer.
   virtual void setDataBlock(ParticleRendererData *data) { mDataBlock = data; };

protected:
   ParticleSystem* mParentSystem; ///< The ParticleSystem which owns this instance.
   ParticleRendererData* mDataBlock; ///< The datablock for this instance.
};

/// @}

#endif //__PARTICLE_RENDERER_H_EMITTER_H