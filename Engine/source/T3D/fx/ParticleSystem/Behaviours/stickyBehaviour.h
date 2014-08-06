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

#ifndef STICKY_BEHAVIOUR_H
#define STICKY_BEHAVIOUR_H

#ifndef PARTICLE_BEHAVIOUR_H
#include "../particleBehaviour.h"
#endif

//-----------------------------------------------
//! A @ref ParticleBehaviour that makes particles
//! use local coordinates.
//! @ingroup particlebehaviours
//-----------------------------------------------
class StickyBehaviour : public IParticleBehaviour
{
   typedef IParticleBehaviour Parent;
public:
   //--------------------------------------------
   // SimDataBlock
   //--------------------------------------------
   bool onAdd();
   void packData(BitStream*);
   void unpackData(BitStream*);
   virtual void packUpdate(BitStream* stream, NetConnection*) { packData(stream); };
   virtual void unpackUpdate(BitStream* stream, NetConnection*) { unpackData(stream); };
   bool preload(bool server, String &errorStr);
   static void initPersistFields();

   //--------------------------------------------
   // IParticleBehaviour
   //--------------------------------------------
   void updateParticle(ParticleSystem* emitter, Particle* part, F32 time);
   U8 getPriority() { return 21; };
   virtual behaviourType::behaviourType getType() { return behaviourType::Position; };

   //--------------------------------------------
   // StickyBehaviour
   //--------------------------------------------
   // Nothing yet
   StickyBehaviour();

   DECLARE_CONOBJECT(StickyBehaviour);
};

#endif // STICKY_BEHAVIOUR_H