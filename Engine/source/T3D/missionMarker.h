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

#ifndef _MISSIONMARKER_H_
#define _MISSIONMARKER_H_

#ifndef _BITSTREAM_H_
#include "core/stream/bitStream.h"
#endif
#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif
#ifndef _SHAPEBASE_H_
#include "T3D/shapeBase.h"
#endif
#ifndef _MATHIO_H_
#include "math/mathIO.h"
#endif
#ifndef _COLOR_H_
#include "core/color.h"
#endif

class MissionMarkerData : public ShapeBaseData
{
   private:
      typedef ShapeBaseData      Parent;
   public:
      DECLARE_CONOBJECT(MissionMarkerData);
};

//------------------------------------------------------------------------------
// Class: MissionMarker
//------------------------------------------------------------------------------
class MissionMarker : public ShapeBase
{
   private:
      typedef ShapeBase          Parent;

   protected:
      enum MaskBits {
         PositionMask = Parent::NextFreeMask,
         NextFreeMask = Parent::NextFreeMask << 1
      };
      MissionMarkerData *        mDataBlock;
      bool                       mAddedToScene;

   public:
      MissionMarker();

      // GameBase
      bool onNewDataBlock( GameBaseData *dptr, bool reload );

      // SceneObject
      void setTransform(const MatrixF &mat);

      // SimObject
      bool onAdd();
      void onRemove();
      void onEditorEnable();
      void onEditorDisable();

      void inspectPostApply();

      // NetObject
      U32  packUpdate  (NetConnection *conn, U32 mask, BitStream *stream);
      void unpackUpdate(NetConnection *conn,           BitStream *stream);

      DECLARE_CONOBJECT(MissionMarker);
      static void initPersistFields();
};

//------------------------------------------------------------------------------
// Class: WayPoint
//------------------------------------------------------------------------------
class WayPoint;

class WayPoint : public MissionMarker
{
   private:
      typedef MissionMarker         Parent;

   public:
      enum WayPointMasks {
         UpdateNameMask    = Parent::NextFreeMask,
         UpdateTeamMask    = Parent::NextFreeMask << 1,
         UpdateHiddenMask  = Parent::NextFreeMask << 2,
         NextFreeMask      = Parent::NextFreeMask << 3
      };

      WayPoint();

      // ShapeBase: only ever added to scene if in the editor
      void setHidden(bool hidden);

      // SimObject
      bool onAdd();
      void inspectPostApply();

      // NetObject
      U32 packUpdate(NetConnection *conn, U32 mask, BitStream *stream);
      void unpackUpdate(NetConnection *conn, BitStream *stream);

      // field data
      StringTableEntry              mName;

      static void initPersistFields();

      DECLARE_CONOBJECT(WayPoint);
};

//------------------------------------------------------------------------------
// Class: SpawnSphere
//------------------------------------------------------------------------------

class SpawnSphere : public MissionMarker
{
   private:
      typedef MissionMarker         Parent;

   public:
      SpawnSphere();

      // SimObject
      bool onAdd();
      void inspectPostApply();

      // NetObject
      enum SpawnSphereMasks
      {
         UpdateSphereMask = Parent::NextFreeMask,
         NextFreeMask     = Parent::NextFreeMask << 1
      };

      U32  packUpdate  (NetConnection *conn, U32 mask, BitStream *stream);
      void unpackUpdate(NetConnection *conn,           BitStream *stream);

      // ProcessObject
      void advanceTime( F32 timeDelta );
      void processTick( const Move *move );

      // Spawn info
      String   mSpawnClass;
      String   mSpawnDataBlock;
      String   mSpawnName;
      String   mSpawnProperties;
      String   mSpawnScript;
      bool     mAutoSpawn;
      bool     mSpawnTransform;

      // Radius/weight info
      F32      mRadius;
      F32      mSphereWeight;
      F32      mIndoorWeight;
      F32      mOutdoorWeight;

      SimObject* spawnObject(String additionalProps = String::EmptyString);

      static void initPersistFields();

      DECLARE_CONOBJECT(SpawnSphere);
	  DECLARE_CALLBACK( void, onAdd, ( U32 objectId ) );
};


//------------------------------------------------------------------------------
// Class: CameraBookmark
//------------------------------------------------------------------------------

class CameraBookmark : public MissionMarker
{
   private:
      typedef MissionMarker         Parent;

   public:
      enum WayPointMasks {
         UpdateNameMask    = Parent::NextFreeMask,
         NextFreeMask      = Parent::NextFreeMask << 1
      };

      CameraBookmark();

      // SimObject
      virtual bool onAdd();
      virtual void onRemove();
      virtual void onGroupAdd();
      virtual void onGroupRemove();
      void inspectPostApply();

      // NetObject
      U32 packUpdate(NetConnection *conn, U32 mask, BitStream *stream);
      void unpackUpdate(NetConnection *conn, BitStream *stream);

      // field data
      StringTableEntry              mName;

      static void initPersistFields();

      DECLARE_CONOBJECT(CameraBookmark);
	  /*DECLARE_CALLBACK( void, onAdd, () );
	  DECLARE_CALLBACK( void, onRemove, () );
	  DECLARE_CALLBACK( void, onGroupAdd, () );
	  DECLARE_CALLBACK( void, onGroupRemove, () );
	  DECLARE_CALLBACK( void, onInspectPostApply, () );*/
};

#endif // _MISSIONMARKER_H_