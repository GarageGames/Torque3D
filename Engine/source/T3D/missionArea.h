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

#ifndef _MISSIONAREA_H_
#define _MISSIONAREA_H_

#ifndef _NETOBJECT_H_
#include "sim/netObject.h"
#endif

class MissionArea : public NetObject
{
  private:
   typedef NetObject Parent;
   RectI             mArea;

   F32 mFlightCeiling;
   F32 mFlightCeilingRange;

   static MissionArea * smServerObject;

  public:
   MissionArea();

   static RectI      smMissionArea;

   static MissionArea * getServerObject();

   F32 getFlightCeiling()      const { return mFlightCeiling;      }
   F32 getFlightCeilingRange() const { return mFlightCeilingRange; }

   //
   const RectI & getArea(){return(mArea);}
   void setArea(const RectI & area);

   /// @name SimObject Inheritance
   /// @{
   bool onAdd();
   void onRemove();

   void inspectPostApply();

   static void initPersistFields();
   /// @}

   /// @name NetObject Inheritance
   /// @{
   enum NetMaskBits {
      UpdateMask     = BIT(0)
   };

   U32  packUpdate  (NetConnection *conn, U32 mask, BitStream *stream);
   void unpackUpdate(NetConnection *conn,           BitStream *stream);
   /// @}

   DECLARE_CONOBJECT(MissionArea);
};

#endif
