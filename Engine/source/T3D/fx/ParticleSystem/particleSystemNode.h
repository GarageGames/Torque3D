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

#ifndef _PARTICLESYSTEMDUMMY_H_
#define _PARTICLESYSTEMDUMMY_H_

#ifndef _GAMEBASE_H_
#include "T3D/gameBase/gameBase.h"
#endif

class ParticleSystemData;
class ParticleSystem;

//-----------------------------------------------
//! The datablock class for ParticleSystem's.
//! @ingroup particlesystem
//-----------------------------------------------
class ParticleSystemNodeData : public GameBaseData
{
   typedef GameBaseData Parent;

protected:
   bool onAdd();

public:
   F32 timeMultiple; //!< How much to scale emit time with. E.g. make particles emit faster/slower.

   ParticleSystemNodeData();
   ~ParticleSystemNodeData();

   void packData(BitStream*);
   void unpackData(BitStream*);
   bool preload(bool server, String &errorStr);

   DECLARE_CONOBJECT(ParticleSystemNodeData);
   static void initPersistFields();
};


//-----------------------------------------------
//! A node which holds a @ref ParticleSystem and
//! emits particles at the node's position.
//! @ingroup particlesystem
//-----------------------------------------------
class ParticleSystemNode : public GameBase
{
   typedef GameBase Parent;

   enum MaskBits
   {
      StateMask = Parent::NextFreeMask << 0,
      SystemDBMask = Parent::NextFreeMask << 1,
      NextFreeMask = Parent::NextFreeMask << 2,
   };

private:
   ParticleSystemNodeData* mDataBlock;

protected:
   bool onAdd();
   void onRemove();
   bool onNewDataBlock(GameBaseData *dptr, bool reload);
   void inspectPostApply();

   ParticleSystemData*  mSystemDatablock;
   S32                  mSystemDatablockId;

   bool                 mActive; //!< If true, this node is currently emitting particles.

   SimObjectPtr<ParticleSystem> mSystem; //!< The ParticleSystem.
   F32                  mVelocity; //!< The velocity to use when emitting particles.

public:
   ParticleSystemNode();
   ~ParticleSystemNode();

   ParticleSystem *getParticleSystem() { return mSystem; }

   // Time/Move Management
public:
   void processTick(const Move* move);
   void advanceTime(F32 dt);

   DECLARE_CONOBJECT(ParticleSystemNode);
   static void initPersistFields();

   U32  packUpdate(NetConnection *conn, U32 mask, BitStream* stream);
   void unpackUpdate(NetConnection *conn, BitStream* stream);

   inline bool getActive(void)        { return mActive; };
   inline void setActive(bool active) { mActive = active; setMaskBits(StateMask); };

   void setSystemDataBlock(ParticleSystemData* data);
};

#endif // _PARTICLESYSTEMDUMMY_H_
