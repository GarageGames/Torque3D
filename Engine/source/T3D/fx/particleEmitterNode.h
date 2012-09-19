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

#ifndef _PARTICLEEMITTERDUMMY_H_
#define _PARTICLEEMITTERDUMMY_H_

#ifndef _GAMEBASE_H_
#include "T3D/gameBase/gameBase.h"
#endif

class ParticleEmitterData;
class ParticleEmitter;

//*****************************************************************************
// ParticleEmitterNodeData
//*****************************************************************************
class ParticleEmitterNodeData : public GameBaseData
{
   typedef GameBaseData Parent;

  protected:
   bool onAdd();

   //-------------------------------------- Console set variables
  public:
   F32 timeMultiple;

   //-------------------------------------- load set variables
  public:

  public:
   ParticleEmitterNodeData();
   ~ParticleEmitterNodeData();

   void packData(BitStream*);
   void unpackData(BitStream*);
   bool preload(bool server, String &errorStr);

   DECLARE_CONOBJECT(ParticleEmitterNodeData);
   static void initPersistFields();
};


//*****************************************************************************
// ParticleEmitterNode
//*****************************************************************************
class ParticleEmitterNode : public GameBase
{
   typedef GameBase Parent;

   enum MaskBits
   {
      StateMask      = Parent::NextFreeMask << 0,
      EmitterDBMask  = Parent::NextFreeMask << 1,
      NextFreeMask   = Parent::NextFreeMask << 2,
   };

  private:
   ParticleEmitterNodeData* mDataBlock;

  protected:
   bool onAdd();
   void onRemove();
   bool onNewDataBlock( GameBaseData *dptr, bool reload );
   void inspectPostApply();

   ParticleEmitterData* mEmitterDatablock;
   S32                  mEmitterDatablockId;

   bool             mActive;

   SimObjectPtr<ParticleEmitter> mEmitter;
   F32              mVelocity;

  public:
   ParticleEmitterNode();
   ~ParticleEmitterNode();
   
   ParticleEmitter *getParticleEmitter() {return mEmitter;}
   
   // Time/Move Management
  public:
   void processTick(const Move* move);
   void advanceTime(F32 dt);

   DECLARE_CONOBJECT(ParticleEmitterNode);
   static void initPersistFields();

   U32  packUpdate  (NetConnection *conn, U32 mask, BitStream* stream);
   void unpackUpdate(NetConnection *conn,           BitStream* stream);

   inline bool getActive( void )        { return mActive;                             };
   inline void setActive( bool active ) { mActive = active; setMaskBits( StateMask ); };

   void setEmitterDataBlock(ParticleEmitterData* data);
};

#endif // _H_PARTICLEEMISSIONDUMMY

