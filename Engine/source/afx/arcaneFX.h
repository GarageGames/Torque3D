
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// Arcane-FX for MIT Licensed Open Source version of Torque 3D from GarageGames
// Copyright (C) 2015 Faust Logic, Inc.
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
//
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#ifndef _ARCANE_FX_H_
#define _ARCANE_FX_H_

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif

#define AFX_VERSION_STRING "2.0"
#define AFX_VERSION         2.0

// #define AFX_CUSTOMIZED_BRANCH

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//
#if defined(AFX_CUSTOMIZED_BRANCH)

#elif (TORQUE_GAME_ENGINE == 1100 || TORQUE_GAME_ENGINE >= 3000)

#define AFX_CAP_SCOPE_TRACKING
#define AFX_CAP_ROLLOVER_RAYCASTS
//#define AFX_CAP_AFXMODEL_TYPE
//#define BROKEN_POINT_IN_WATER
#define BROKEN_DAMAGEFLASH_WHITEOUT_BLACKOUT

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//
#else

// This version of AFX source only supports T3D 1.1

#endif

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

#ifndef _CONSOLETYPES_H_
#include "console/consoleTypes.h"
#endif

#ifndef _ENGINEAPI_H_
#include "console/engineAPI.h"
#endif

#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif

#ifndef _BITSTREAM_H_
#include "core/stream/bitStream.h"
#endif

#ifndef _GAMEBASE_H_
#include "T3D/gameBase/gameBase.h"
#endif

#if defined(DGL_GRAPHICS_LAYER)
#ifndef _DGL_H_
#include "dgl/dgl.h"
#endif
#endif

class afxChoreographer;
class afxSelectronData;
class GameConnection;
class SceneObject;

class arcaneFX
{
public:
  enum {
    TARGETING_OFF,
    TARGETING_STANDARD,
    TARGETING_FREE
  };
  enum {
    TARGET_CHECK_POLL,
    TARGET_CHECK_ON_MOUSE_MOVE
  };

private:
  static Vector<afxChoreographer*> active_choreographers;
  static Vector<afxChoreographer*> client_choreographers;
  static Vector<afxSelectronData*> selectrons;
  static Vector<SceneObject*>      scoped_objs;
  static bool                      is_shutdown;

public:
  static StringTableEntry   NULLSTRING;
  static U32                sTargetSelectionMask;
  static U32                sFreeTargetSelectionMask;
  static bool               sIsFreeTargeting;
  static Point3F            sFreeTargetPos;
  static bool               sFreeTargetPosValid;
  static F32                sTargetSelectionRange;
  static U32                sTargetSelectionTimeoutMS;
  static bool               sClickToTargetSelf;
  static U32                sMissileCollisionMask;
  static StringTableEntry   sParameterFieldPrefix;
  static F32                sTerrainZodiacZBias;
  static F32                sInteriorZodiacZBias;
  static F32                sPolysoupZodiacZBias;
  static U32                master_choreographer_id;
  static U16                master_scope_id;

public:
  static void init();
  static void shutdown();
  static void advanceTime(U32 delta);

  static U32  registerChoreographer(afxChoreographer*);
  static void unregisterChoreographer(afxChoreographer*);
  static void registerClientChoreographer(afxChoreographer*);
  static void unregisterClientChoreographer(afxChoreographer*);
  static afxChoreographer* findClientChoreographer(U32 id);

  static void registerSelectronData(afxSelectronData*);
  static void unregisterSelectronData(afxSelectronData*);
  static afxSelectronData* findSelectronData(U32 obj_type_mask, U8 code);

  static U16            generateScopeId();
  static void           registerScopedObject(SceneObject*);
  static SceneObject*   findScopedObject(U16 scope_id);
  static void           unregisterScopedObject(SceneObject*);

  static void syncToNewConnection(GameConnection* conn);
  static void endMissionNotify();
  static S32  rolloverRayCast(Point3F start, Point3F end, U32 mask);
  static bool freeTargetingRayCast(Point3F start, Point3F end, U32 mask);

  static bool isShutdown() { return is_shutdown; }
  static StringTableEntry convertLightingModelName(StringTableEntry lm_name);

private:
  static bool sUsePlayerCentricListener;
public:
  static bool usePlayerCentricListener() { return sUsePlayerCentricListener; }
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

class ByteRange
{
public:
  U8     low;
  U8     high;

public:
  /*C*/  ByteRange() { low = 0; high = 255; }
  /*C*/  ByteRange(U8 l, U8 h=255) { low = l; high = h; }

  void   set(U8 l, U8 h=255) { low = l; high = h; }
  bool   outOfRange(U8 v) { return (v < low || v > high); }
  bool   inRange(U8 v) { return !outOfRange(v); }
  S32    getSpan() const { return high - low; }
};

DefineConsoleType(TypeByteRange, ByteRange)
DefineConsoleType(TypeByteRange2, ByteRange)

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

inline void writeDatablockID(BitStream* s, SimObject* simobj, bool packed=false)
{
  if (s->writeFlag(simobj))
    s->writeRangedU32(packed ? SimObjectId((uintptr_t)simobj) : simobj->getId(),
                      DataBlockObjectIdFirst, DataBlockObjectIdLast);
}

inline S32 readDatablockID(BitStream* s)
{
  return (!s->readFlag()) ? 0 : ((S32)s->readRangedU32(DataBlockObjectIdFirst,
          DataBlockObjectIdLast));
}

inline void registerForCleanup(SimObject* obj)
{
  SimGroup* cleanup_grp = dynamic_cast<SimGroup*>(Sim::findObject("MissionCleanup"));
  if (cleanup_grp)
    cleanup_grp->addObject(obj);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

#define ST_NULLSTRING (arcaneFX::NULLSTRING)


//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#endif // _ARCANE_FX_H_

