//-----------------------------------------------------------------------------
// Copyright (c) 2014 GarageGames, LLC
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

#ifndef _RIBBON_NODE_H_
#define _RIBBON_NODE_H_

#ifndef _GAMEBASE_H_
#include "T3D/gameBase/gameBase.h"
#endif

class RibbonData;
class Ribbon;

//*****************************************************************************
// ParticleEmitterNodeData
//*****************************************************************************
class RibbonNodeData : public GameBaseData
{
   typedef GameBaseData Parent;

public:
   F32 timeMultiple;

public:
   RibbonNodeData();
   ~RibbonNodeData();

   DECLARE_CONOBJECT(RibbonNodeData);
   static void initPersistFields();
};


//*****************************************************************************
// ParticleEmitterNode
//*****************************************************************************
class RibbonNode : public GameBase
{
   typedef GameBase Parent;

   enum MaskBits
   {
      StateMask      = Parent::NextFreeMask << 0,
      EmitterDBMask  = Parent::NextFreeMask << 1,
      NextFreeMask   = Parent::NextFreeMask << 2,
   };

   RibbonNodeData* mDataBlock;

protected:
   bool onAdd();
   void onRemove();
   bool onNewDataBlock( GameBaseData *dptr, bool reload );
   void inspectPostApply();

   RibbonData* mRibbonDatablock;
   S32 mRibbonDatablockId;

   SimObjectPtr<Ribbon> mRibbon;

   bool mActive;

public:
   RibbonNode();
   ~RibbonNode();

   Ribbon *getRibbonEmitter() {return mRibbon;}

   // Time/Move Management

   void processTick(const Move* move);
   void advanceTime(F32 dt);

   DECLARE_CONOBJECT(RibbonNode);
   static void initPersistFields();

   U32  packUpdate  (NetConnection *conn, U32 mask, BitStream* stream);
   void unpackUpdate(NetConnection *conn,           BitStream* stream);

   inline bool getActive( void )        { return mActive;                             };
   inline void setActive( bool active ) { mActive = active; setMaskBits( StateMask ); };

   void setRibbonDatablock(RibbonData* data);
};

#endif // _RIBBON_NODE_H_

