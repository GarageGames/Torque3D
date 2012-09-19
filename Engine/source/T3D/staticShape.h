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

#ifndef _STATICSHAPE_H_
#define _STATICSHAPE_H_

#ifndef _SHAPEBASE_H_
#include "T3D/shapeBase.h"
#endif

//----------------------------------------------------------------------------

struct StaticShapeData: public ShapeBaseData {
   typedef ShapeBaseData Parent;

  public:
   StaticShapeData();

   bool  noIndividualDamage;
   S32   dynamicTypeField;
   bool  isShielded;
   F32   energyPerDamagePoint;

   //
   DECLARE_CONOBJECT(StaticShapeData);
   static void initPersistFields();
   virtual void packData(BitStream* stream);
   virtual void unpackData(BitStream* stream);
};


//----------------------------------------------------------------------------

class StaticShape: public ShapeBase
{
   typedef ShapeBase Parent;

   StaticShapeData*  mDataBlock;
   bool              mPowered;

   void onUnmount(ShapeBase* obj,S32 node);

protected:
   enum MaskBits {
      PositionMask = Parent::NextFreeMask,	  
      NextFreeMask = Parent::NextFreeMask << 1
   };

public:
   DECLARE_CONOBJECT(StaticShape);

   StaticShape();
   ~StaticShape();

   bool onAdd();
   void onRemove();
   bool onNewDataBlock(GameBaseData *dptr, bool reload);

   void processTick(const Move *move);
   void interpolateTick(F32 delta);
   void setTransform(const MatrixF &mat);

   U32  packUpdate  (NetConnection *conn, U32 mask, BitStream *stream);
   void unpackUpdate(NetConnection *conn,           BitStream *stream);

   // power
   void setPowered(bool power)      {mPowered = power;}
   bool isPowered()                 {return(mPowered);}

   static void initPersistFields();   
};


#endif
