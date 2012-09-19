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

#ifndef _H_TRIGGER
#define _H_TRIGGER

#ifndef _GAMEBASE_H_
#include "T3D/gameBase/gameBase.h"
#endif
#ifndef _MBOX_H_
#include "math/mBox.h"
#endif
#ifndef _EARLYOUTPOLYLIST_H_
#include "collision/earlyOutPolyList.h"
#endif
#ifndef _MPOLYHEDRON_H_
#include "math/mPolyhedron.h"
#endif

class Convex;
class PhysicsBody;
class Trigger;

DefineConsoleType( TypeTriggerPolyhedron, Polyhedron )


struct TriggerData: public GameBaseData {
   typedef GameBaseData Parent;

  public:
   S32  tickPeriodMS;
   bool isClientSide;

   TriggerData();
   
   DECLARE_CONOBJECT(TriggerData);

   DECLARE_CALLBACK( void, onEnterTrigger, ( Trigger* trigger, GameBase* obj ) );
   DECLARE_CALLBACK( void, onTickTrigger, ( Trigger* trigger ) );
   DECLARE_CALLBACK( void, onLeaveTrigger, ( Trigger* trigger, GameBase* obj ) );

   bool onAdd();
   static void initPersistFields();
   virtual void packData  (BitStream* stream);
   virtual void unpackData(BitStream* stream);
};

class Trigger : public GameBase
{
   typedef GameBase Parent;

   /// Trigger polyhedron with *outward* facing normals and CCW ordered
   /// vertices.
   Polyhedron mTriggerPolyhedron;

   EarlyOutPolyList  mClippedList;
   Vector<GameBase*> mObjects;

   PhysicsBody      *mPhysicsRep;

   TriggerData*      mDataBlock;

   U32               mLastThink;
   U32               mCurrTick;
   Convex            *mConvexList;

   String            mEnterCommand;
   String            mLeaveCommand;
   String            mTickCommand;

   enum TriggerUpdateBits
   {
      TransformMask = Parent::NextFreeMask << 0,
      PolyMask      = Parent::NextFreeMask << 1,
      EnterCmdMask  = Parent::NextFreeMask << 2,
      LeaveCmdMask  = Parent::NextFreeMask << 3,
      TickCmdMask   = Parent::NextFreeMask << 4,
      NextFreeMask  = Parent::NextFreeMask << 5,
   };

   static const U32 CMD_SIZE = 1024;

  protected:

   static bool smRenderTriggers;
   bool testObject(GameBase* enter);
   void processTick(const Move *move);

   void buildConvex(const Box3F& box, Convex* convex);

   static bool setEnterCmd(void *object, const char *index, const char *data);
   static bool setLeaveCmd(void *object, const char *index, const char *data);
   static bool setTickCmd(void *object, const char *index, const char *data);

  public:
   Trigger();
   ~Trigger();

   // SimObject
   DECLARE_CONOBJECT(Trigger);

   DECLARE_CALLBACK( void, onAdd, ( U32 objectId ) );
   DECLARE_CALLBACK( void, onRemove, ( U32 objectId ) );

   static void consoleInit();
   static void initPersistFields();
   bool onAdd();
   void onRemove();
   void onDeleteNotify(SimObject*);
   void inspectPostApply();

   // NetObject
   U32  packUpdate  (NetConnection *conn, U32 mask, BitStream* stream);
   void unpackUpdate(NetConnection *conn,           BitStream* stream);

   // SceneObject
   void setTransform(const MatrixF &mat);
   void prepRenderImage( SceneRenderState* state );

   // GameBase
   bool onNewDataBlock( GameBaseData *dptr, bool reload );

   // Trigger
   void setTriggerPolyhedron(const Polyhedron&);

   void      potentialEnterObject(GameBase*);
   U32       getNumTriggeringObjects() const;
   GameBase* getObject(const U32);   
   const Vector<GameBase*>& getObjects() const { return mObjects; }

   void renderObject( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat );

   bool castRay(const Point3F &start, const Point3F &end, RayInfo* info);
};

inline U32 Trigger::getNumTriggeringObjects() const
{
   return mObjects.size();
}

inline GameBase* Trigger::getObject(const U32 index)
{
   AssertFatal(index < getNumTriggeringObjects(), "Error, out of range object index");

   return mObjects[index];
}

#endif // _H_TRIGGER

