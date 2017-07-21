
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

#include "afx/arcaneFX.h"

#include "scene/sceneObject.h"
#include "scene/sceneManager.h"
#include "T3D/gameBase/gameConnection.h"
#include "T3D/gameBase/gameProcess.h"
#include "T3D/player.h"
#include "math/mathUtils.h"
#include "console/compiler.h"
#include "console/engineAPI.h"

#include "afx/afxChoreographer.h"
#include "afx/afxSelectron.h"
#include "afx/afxResidueMgr.h"
#include "afx/ce/afxZodiacMgr.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#define N_LIGHTING_MODELS 6
//
// "SG - Original Advanced (Lighting Pack)"
// "SG - Original Stock (Lighting Pack)"
// "SG - Inverse Square (Lighting Pack)"
// "SG - Inverse Square Fast Falloff (Lighting Pack)"
// "SG - Near Linear (Lighting Pack)"
// "SG - Near Linear Fast Falloff (Lighting Pack)"
static StringTableEntry lm_old_names[N_LIGHTING_MODELS];
//
// "Original Advanced"
// "Original Stock"
// "Inverse Square"
// "Inverse Square Fast Falloff"
// "Near Linear"
// "Near Linear Fast Falloff"
static StringTableEntry lm_new_names[N_LIGHTING_MODELS];

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class ClientZoneInEvent : public NetEvent
{
  typedef NetEvent Parent;
public:
  ClientZoneInEvent() { mGuaranteeType = Guaranteed; }
  ~ClientZoneInEvent() { }

  virtual void pack(NetConnection*, BitStream*bstream) { }
  virtual void write(NetConnection*, BitStream *bstream) { }
  virtual void unpack(NetConnection* /*ps*/, BitStream *bstream) { }

  virtual void process(NetConnection* conn)
  {
    GameConnection* game_conn = dynamic_cast<GameConnection*>(conn);
    if (game_conn && !game_conn->isZonedIn())
    {
      arcaneFX::syncToNewConnection(game_conn);
    }
  }

  DECLARE_CONOBJECT(ClientZoneInEvent);
  DECLARE_CATEGORY("AFX");
};
IMPLEMENT_CO_SERVEREVENT_V1(ClientZoneInEvent);

ConsoleDocClass( ClientZoneInEvent,
				"@brief Event posted when player is fully loaded into the game and ready for interaction.\n\n"
				"@internal");

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

Vector<afxChoreographer*> arcaneFX::active_choreographers;
Vector<afxChoreographer*> arcaneFX::client_choreographers;
Vector<afxSelectronData*> arcaneFX::selectrons;
Vector<SceneObject*>      arcaneFX::scoped_objs;

StringTableEntry arcaneFX::NULLSTRING = 0;
U32              arcaneFX::sTargetSelectionMask = 0;
U32              arcaneFX::sFreeTargetSelectionMask = 0;
bool             arcaneFX::sIsFreeTargeting = false;
Point3F          arcaneFX::sFreeTargetPos = Point3F(0.0f, 0.0f, 0.0f);
bool             arcaneFX::sFreeTargetPosValid = false;
F32              arcaneFX::sTargetSelectionRange = 200.0f;
U32              arcaneFX::sTargetSelectionTimeoutMS = 500;
bool             arcaneFX::sClickToTargetSelf = false;
U32              arcaneFX::sMissileCollisionMask = 0;
StringTableEntry arcaneFX::sParameterFieldPrefix = 0;
F32              arcaneFX::sTerrainZodiacZBias = -0.00025f;
F32              arcaneFX::sInteriorZodiacZBias = -0.0001f;
F32              arcaneFX::sPolysoupZodiacZBias = -0.0001f;
U32              arcaneFX::master_choreographer_id = 1;
U16              arcaneFX::master_scope_id = 1;
bool             arcaneFX::is_shutdown = true;

bool             arcaneFX::sUsePlayerCentricListener = false;

void arcaneFX::init()
{
  NULLSTRING = StringTable->insert("");
  sParameterFieldPrefix = StringTable->insert("_");

#if defined(TORQUE_OS_MAC)
  arcaneFX::sTerrainZodiacZBias = -0.00025f;
  arcaneFX::sInteriorZodiacZBias = -0.00025f;
  arcaneFX::sPolysoupZodiacZBias = -0.00025f;
#endif

  Con::addVariable(  "pref::AFX::targetSelectionMask",      TypeS32,    &sTargetSelectionMask);
  Con::addVariable(  "pref::AFX::freeTargetSelectionMask",  TypeS32,    &sFreeTargetSelectionMask);
  Con::addVariable(  "pref::AFX::targetSelectionRange",     TypeF32,    &sTargetSelectionRange);
  Con::addVariable(  "pref::AFX::targetSelectionTimeoutMS", TypeS32,    &sTargetSelectionTimeoutMS);
  Con::addVariable(  "pref::AFX::missileCollisionMask",     TypeS32,    &sMissileCollisionMask);
  Con::addVariable(  "pref::AFX::clickToTargetSelf",        TypeBool,   &sClickToTargetSelf);
  Con::addVariable(  "Pref::Server::AFX::parameterFieldPrefix",     TypeString,  &sParameterFieldPrefix);

  Con::addVariable(  "pref::AFX::terrainZodiacZBias",       TypeF32,    &sTerrainZodiacZBias);
  Con::addVariable(  "pref::AFX::interiorZodiacZBias",      TypeF32,    &sInteriorZodiacZBias);
  Con::addVariable(  "pref::AFX::polysoupZodiacZBias",      TypeF32,    &sPolysoupZodiacZBias);

  Con::setIntVariable(    "$AFX::TARGETING_OFF",                TARGETING_OFF);
  Con::setIntVariable(    "$AFX::TARGETING_STANDARD",           TARGETING_STANDARD);
  Con::setIntVariable(    "$AFX::TARGETING_FREE",               TARGETING_FREE);
  Con::setIntVariable(    "$AFX::TARGET_CHECK_POLL",            TARGET_CHECK_POLL);
  Con::setIntVariable(    "$AFX::TARGET_CHECK_ON_MOUSE_MOVE",   TARGET_CHECK_ON_MOUSE_MOVE);

  Con::setIntVariable(    "$AFX::IMPACTED_SOMETHING", afxEffectDefs::IMPACTED_SOMETHING);
  Con::setIntVariable(    "$AFX::IMPACTED_TARGET",    afxEffectDefs::IMPACTED_TARGET);
  Con::setIntVariable(    "$AFX::IMPACTED_PRIMARY",   afxEffectDefs::IMPACTED_PRIMARY);
  Con::setIntVariable(    "$AFX::IMPACT_IN_WATER",    afxEffectDefs::IMPACT_IN_WATER);
  Con::setIntVariable(    "$AFX::CASTER_IN_WATER",    afxEffectDefs::CASTER_IN_WATER);

  Con::setIntVariable(    "$AFX::SERVER_ONLY",        afxEffectDefs::SERVER_ONLY);
  Con::setIntVariable(    "$AFX::SCOPE_ALWAYS",       afxEffectDefs::SCOPE_ALWAYS);
  Con::setIntVariable(    "$AFX::GHOSTABLE",          afxEffectDefs::GHOSTABLE);
  Con::setIntVariable(    "$AFX::CLIENT_ONLY",        afxEffectDefs::CLIENT_ONLY);
  Con::setIntVariable(    "$AFX::SERVER_AND_CLIENT",  afxEffectDefs::SERVER_AND_CLIENT);

  Con::setIntVariable(    "$AFX::DELAY",              afxEffectDefs::TIMING_DELAY);
  Con::setIntVariable(    "$AFX::LIFETIME",           afxEffectDefs::TIMING_LIFETIME);
  Con::setIntVariable(    "$AFX::FADE_IN_TIME",       afxEffectDefs::TIMING_FADE_IN);
  Con::setIntVariable(    "$AFX::FADE_OUT_TIME",      afxEffectDefs::TIMING_FADE_OUT);

  Con::setFloatVariable(  "$AFX::INFINITE_TIME",      -1.0f);
  Con::setIntVariable(    "$AFX::INFINITE_REPEATS",   -1);

  Con::setIntVariable(    "$AFX::PLAYER_MOVE_TRIGGER_0",      Player::PLAYER_MOVE_TRIGGER_0);
  Con::setIntVariable(    "$AFX::PLAYER_MOVE_TRIGGER_1",      Player::PLAYER_MOVE_TRIGGER_1);
  Con::setIntVariable(    "$AFX::PLAYER_MOVE_TRIGGER_2",      Player::PLAYER_MOVE_TRIGGER_2);
  Con::setIntVariable(    "$AFX::PLAYER_MOVE_TRIGGER_3",      Player::PLAYER_MOVE_TRIGGER_3);
  Con::setIntVariable(    "$AFX::PLAYER_MOVE_TRIGGER_4",      Player::PLAYER_MOVE_TRIGGER_4);
  Con::setIntVariable(    "$AFX::PLAYER_MOVE_TRIGGER_5",      Player::PLAYER_MOVE_TRIGGER_5);

  Con::setIntVariable(    "$AFX::PLAYER_FIRE_S_TRIGGER",      Player::PLAYER_FIRE_S_TRIGGER);
  Con::setIntVariable(    "$AFX::PLAYER_FIRE_ALT_S_TRIGGER",  Player::PLAYER_FIRE_ALT_S_TRIGGER);
  Con::setIntVariable(    "$AFX::PLAYER_JUMP_S_TRIGGER",      Player::PLAYER_JUMP_S_TRIGGER);
  Con::setIntVariable(    "$AFX::PLAYER_LANDING_S_TRIGGER",   Player::PLAYER_LANDING_S_TRIGGER);

  Con::setIntVariable(    "$AFX::PLAYER_LF_FOOT_C_TRIGGER",   Player::PLAYER_LF_FOOT_C_TRIGGER);
  Con::setIntVariable(    "$AFX::PLAYER_RT_FOOT_C_TRIGGER",   Player::PLAYER_RT_FOOT_C_TRIGGER);
  Con::setIntVariable(    "$AFX::PLAYER_LANDING_C_TRIGGER",   Player::PLAYER_LANDING_C_TRIGGER);
  Con::setIntVariable(    "$AFX::PLAYER_IDLE_C_TRIGGER",      Player::PLAYER_IDLE_C_TRIGGER);

  Con::setIntVariable(    "$AFX::ILLUM_TERRAIN",      0);
  Con::setIntVariable(    "$AFX::ILLUM_ATLAS",        0);
  Con::setIntVariable(    "$AFX::ILLUM_DIF",          0);
  Con::setIntVariable(    "$AFX::ILLUM_DTS",          0);
  Con::setIntVariable(    "$AFX::ILLUM_ALL",          0);

  Con::setIntVariable("$TypeMasks::TerrainLikeObjectType", TerrainLikeObjectType);
  Con::setIntVariable("$TypeMasks::InteriorLikeObjectType", InteriorLikeObjectType);
  Con::setIntVariable("$TypeMasks::PolysoupObjectType", InteriorLikeObjectType); // deprecated

  Con::addVariable("$pref::Audio::usePlayerCentricListener", TypeBool, &sUsePlayerCentricListener);

  afxResidueMgr* residue_mgr = new afxResidueMgr;
  afxResidueMgr::setMaster(residue_mgr);

  master_scope_id = 1;
  master_choreographer_id = 1;
  is_shutdown = false;

  if (lm_old_names[0] == 0)
  {
    lm_old_names[0] = StringTable->insert("SG - Original Advanced (Lighting Pack)");
    lm_old_names[1] = StringTable->insert("SG - Original Stock (Lighting Pack)");
    lm_old_names[2] = StringTable->insert("SG - Inverse Square (Lighting Pack)");
    lm_old_names[3] = StringTable->insert("SG - Inverse Square Fast Falloff (Lighting Pack)");
    lm_old_names[4] = StringTable->insert("SG - Near Linear (Lighting Pack)");
    lm_old_names[5] = StringTable->insert("SG - Near Linear Fast Falloff (Lighting Pack)");
    //
    lm_new_names[0] = StringTable->insert("Original Advanced");
    lm_new_names[1] = StringTable->insert("Original Stock");
    lm_new_names[2] = StringTable->insert("Inverse Square");
    lm_new_names[3] = StringTable->insert("Inverse Square Fast Falloff");
    lm_new_names[4] = StringTable->insert("Near Linear");
    lm_new_names[5] = StringTable->insert("Near Linear Fast Falloff");
  }
}

void arcaneFX::shutdown()
{
  is_shutdown = true;

  for (S32 i = 0; i < scoped_objs.size(); i++)
     if (scoped_objs[i])
       scoped_objs[i]->setScopeRegistered(false);
  scoped_objs.clear();

  for (S32 i = 0; i < client_choreographers.size(); i++)
     if (client_choreographers[i])
       client_choreographers[i]->clearChoreographerId();
  client_choreographers.clear();

  for (S32 i = 0; i < selectrons.size(); i++)
    if (selectrons[i])
       selectrons[i]->registered = false;
  selectrons.clear();

  afxResidueMgr* residue_mgr = afxResidueMgr::getMaster();
  delete residue_mgr;
  afxResidueMgr::setMaster(NULL);
}

MODULE_BEGIN( arcaneFX )

   MODULE_INIT
   {
      arcaneFX::init();
   }

   MODULE_SHUTDOWN
   {
      arcaneFX::shutdown();
   }

MODULE_END;

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

void arcaneFX::advanceTime(U32 delta)
{
  GameConnection* conn = GameConnection::getConnectionToServer();
  if (conn && !conn->isZonedIn() && conn->getCameraObject() != 0)
  {
    conn->setZonedIn();
    conn->postNetEvent(new ClientZoneInEvent());
  }

  afxZodiacMgr::frameReset();
  afxResidueMgr::getMaster()->residueAdvanceTime();
}

//

U32 arcaneFX::registerChoreographer(afxChoreographer* ch)
{
  if (!ch)
    return 0;

  active_choreographers.push_back(ch);

  //Con::printf("registerChoreographer() -- size=%d %s", active_choreographers.size(),
  //  (ch->isServerObject()) ? "server" : "client");

  return master_choreographer_id++;
}

void arcaneFX::unregisterChoreographer(afxChoreographer* ch)
{
  if (!ch)
    return;

  for (U32 i = 0; i < active_choreographers.size(); i++)
  {
    if (ch == active_choreographers[i])
    {
      active_choreographers.erase_fast(i);
      //Con::printf("unregisterChoreographer() -- size=%d %s", active_choreographers.size(),
      //  (ch->isServerObject()) ? "server" : "client");
      return;
    }
  }

  Con::errorf("arcaneFX::unregisterChoreographer() -- failed to find choreographer in list.");
}

void arcaneFX::registerClientChoreographer(afxChoreographer* ch)
{
  if (!ch || ch->getChoreographerId() == 0)
    return;

  client_choreographers.push_back(ch);
}

void arcaneFX::unregisterClientChoreographer(afxChoreographer* ch)
{
  if (!ch || ch->getChoreographerId() == 0)
    return;

  for (U32 i = 0; i < client_choreographers.size(); i++)
  {
    if (ch == client_choreographers[i])
    {
      client_choreographers.erase_fast(i);
      return;
    }
  }

  Con::errorf("arcaneFX::unregisterClientChoreographer() -- failed to find choreographer in list.");
}

afxChoreographer* arcaneFX::findClientChoreographer(U32 id)
{
  for (U32 i = 0; i < client_choreographers.size(); i++)
  {
    if (id == client_choreographers[i]->getChoreographerId())
      return client_choreographers[i];
  }

  return 0;
}

//

void arcaneFX::registerSelectronData(afxSelectronData* selectron)
{
  if (!selectron)
    return;

  selectrons.push_back(selectron);
}

void arcaneFX::unregisterSelectronData(afxSelectronData* selectron)
{
  if (!selectron)
    return;

  for (U32 i = 0; i < selectrons.size(); i++)
  {
    if (selectron == selectrons[i])
    {
      selectrons.erase_fast(i);
      return;
    }
  }

  Con::errorf("arcaneFX::unregisterSelectronData() -- failed to find selectron in list.");
}

afxSelectronData* arcaneFX::findSelectronData(U32 mask, U8 style)
{
  for (U32 i = 0; i < selectrons.size(); i++)
    if (selectrons[i]->matches(mask, style))
      return selectrons[i];

  return 0;
}

U16 arcaneFX::generateScopeId()
{
  U16 ret_id = master_scope_id++;
  if (master_scope_id >= BIT(GameBase::SCOPE_ID_BITS))
    master_scope_id = 1;
  return ret_id;
}

void arcaneFX::registerScopedObject(SceneObject* object)
{
  scoped_objs.push_back(object);
  object->setScopeRegistered(true);

  for (S32 i = 0; i < client_choreographers.size(); i++)
    if (client_choreographers[i])
      client_choreographers[i]->restoreScopedObject(object);
}

SceneObject* arcaneFX::findScopedObject(U16 scope_id)
{
  if (scoped_objs.size() > 0)
  {
    for (S32 i = scoped_objs.size()-1; i >= 0; i--)
      if (scoped_objs[i] && scoped_objs[i]->getScopeId() == scope_id)
        return scoped_objs[i];
  }
  return 0;
}

void arcaneFX::unregisterScopedObject(SceneObject* object)
{
  if (scoped_objs.size() > 0)
  {
    for (S32 i = scoped_objs.size()-1; i >= 0; i--)
      if (scoped_objs[i] == object)
      {
        scoped_objs.erase_fast(i);
        if (object)
          object->setScopeRegistered(false);
        return;
      }
  }
}

void arcaneFX::syncToNewConnection(GameConnection* conn)
{
  if (conn)
    conn->setZonedIn();

  for (U32 i = 0; i < active_choreographers.size(); i++)
  {
    if (active_choreographers[i])
      active_choreographers[i]->sync_with_clients();
  }
}

void arcaneFX::endMissionNotify()
{
  for (S32 i = 0; i < scoped_objs.size(); i++)
     if (scoped_objs[i])
       scoped_objs[i]->setScopeRegistered(false);
  scoped_objs.clear();

  for (S32 i = 0; i < client_choreographers.size(); i++)
     if (client_choreographers[i])
       client_choreographers[i]->clearChoreographerId();
  client_choreographers.clear();

  for (S32 i = 0; i < selectrons.size(); i++)
    if (selectrons[i])
       selectrons[i]->registered = false;
  selectrons.clear();

  if (afxResidueMgr::getMaster())
    afxResidueMgr::getMaster()->cleanup();
  afxZodiacMgr::missionCleanup();
}

S32 arcaneFX::rolloverRayCast(Point3F start, Point3F end, U32 mask)
{
  sIsFreeTargeting = false;
#if !defined(AFX_CAP_ROLLOVER_RAYCASTS)
  return -1;
#else
  GameConnection* conn = GameConnection::getConnectionToServer();
  SceneObject* ctrl_obj = NULL;

  if (!arcaneFX::sClickToTargetSelf && conn != NULL)
    ctrl_obj = conn->getControlObject();

  if (ctrl_obj)
    ctrl_obj->disableCollision();

  SceneObject* rollover_obj = (conn) ? conn->getRolloverObj() : 0;
  SceneObject* picked_obj = 0;

  RayInfo hit_info;
  if (gClientContainer.castRay(start, end, mask, &hit_info))
    picked_obj = dynamic_cast<SceneObject*>(hit_info.object);

  if (ctrl_obj)
    ctrl_obj->enableCollision();

  if (picked_obj != rollover_obj)
  {
    if (rollover_obj)
      rollover_obj->setSelectionFlags(rollover_obj->getSelectionFlags() & ~SceneObject::PRE_SELECTED);
    if (picked_obj)
      picked_obj->setSelectionFlags(picked_obj->getSelectionFlags() | SceneObject::PRE_SELECTED);
    rollover_obj = picked_obj;

    if (conn)
      conn->setRolloverObj(rollover_obj);
  }

  return (picked_obj) ? picked_obj->getId() : -1;
#endif
}

bool arcaneFX::freeTargetingRayCast(Point3F start, Point3F end, U32 mask)
{
  sIsFreeTargeting = true;

  RayInfo hit_info;
  if (!gClientContainer.castRay(start, end, mask, &hit_info))
  {
    sFreeTargetPosValid = false;
    return false;
  }

  sFreeTargetPosValid = true;
  sFreeTargetPos = hit_info.point;

  return true;
}

StringTableEntry arcaneFX::convertLightingModelName(StringTableEntry lm_name)
{
  for (U32 i = 0; i < N_LIGHTING_MODELS; i++)
  {
    if (lm_name == lm_old_names[i])
      return lm_new_names[i];
  }

  return lm_name;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// Console Functions

DefineEngineFunction(afxEndMissionNotify, void, (),,
                     "...\n\n"
                     "@ingroup AFX")
{
  arcaneFX::endMissionNotify();
}

DefineEngineFunction(afxGetVersion, const char*, (),,
                     "...\n\n"
                     "@ingroup AFX")
{
  return AFX_VERSION_STRING;
}

DefineEngineFunction(afxGetEngine, const char*, (),,
                     "...\n\n"
                     "@ingroup AFX")
{
  return "T3D";
}

#if defined(AFX_CAP_ROLLOVER_RAYCASTS)
DefineEngineFunction(rolloverRayCast, S32, (Point3F start, Point3F end, U32 mask),,
                     "Performs a raycast from points start to end and returns the ID of nearest "
                     "intersecting object with a type found in the specified mask. "
                     "Returns -1 if no object is found.\n\n"
                     "@ingroup AFX")
{
  return arcaneFX::rolloverRayCast(start, end, mask);
}
#endif

DefineEngineFunction(getRandomF, F32, (float a, float b), (F32_MAX, F32_MAX),
                "Get a random float number between a and b.\n\n"
                "@ingroup AFX")
{
  if (b == F32_MAX)
  {
    if (a == F32_MAX)
      return gRandGen.randF();

    return gRandGen.randF(0.0f, a);
  }

  return (a + (b-a)*gRandGen.randF());
}

DefineEngineFunction(getRandomDir, Point3F, (Point3F axis, float thetaMin, float thetaMax, float phiMin, float phiMax),
                     (Point3F(0.0f,0.0f,0.0f), 0.0f, 180.0f, 0.0f, 360.0f),
                     "Get a random direction vector.\n\n"
                     "@ingroup AFX")
{
  return MathUtils::randomDir(axis, thetaMin, thetaMax, phiMin, phiMax);
}

ConsoleFunction( MatrixInverseMulVector, const char*, 3, 3, "(MatrixF xfrm, Point3F vector)"
                "@brief Multiply the vector by the affine inverse of the transform.\n\n"
                "@ingroup AFX")
{
   Point3F pos1(0.0f,0.0f,0.0f);
   AngAxisF aa1(Point3F(0.0f,0.0f,0.0f),0.0f);
   dSscanf(argv[1], "%g %g %g %g %g %g %g", &pos1.x, &pos1.y, &pos1.z, &aa1.axis.x, &aa1.axis.y, &aa1.axis.z, &aa1.angle);

   MatrixF temp1(true);
   aa1.setMatrix(&temp1);
   temp1.setColumn(3, pos1);

   Point3F vec1(0.0f,0.0f,0.0f);
   dSscanf(argv[2], "%g %g %g", &vec1.x, &vec1.y, &vec1.z);

   temp1.affineInverse();

   Point3F result;
   temp1.mulV(vec1, &result);

   char* ret = Con::getReturnBuffer(256);
   dSprintf(ret, 255, "%g %g %g", result.x, result.y, result.z);
   return ret;
}

ConsoleFunction(moveTransformAbs, const char*, 3, 3, "(MatrixF xfrm, Point3F pos)"
                "@brief Move the transform to the new absolute position.\n\n"
                "@ingroup AFX")
{
   Point3F pos1(0.0f,0.0f,0.0f);
   AngAxisF aa1(Point3F(0.0f,0.0f,0.0f),0.0f);
   dSscanf(argv[1], "%g %g %g %g %g %g %g", &pos1.x, &pos1.y, &pos1.z, &aa1.axis.x, &aa1.axis.y, &aa1.axis.z, &aa1.angle);

   Point3F pos2(0.0f,0.0f,0.0f);
   dSscanf(argv[2], "%g %g %g", &pos2.x, &pos2.y, &pos2.z);

   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer, 255, "%g %g %g %g %g %g %g",
            pos2.x, pos2.y, pos2.z,
            aa1.axis.x, aa1.axis.y, aa1.axis.z,
            aa1.angle);
   return returnBuffer;
}

ConsoleFunction(moveTransformRel, const char*, 3, 3, "(MatrixF xfrm, Point3F pos)"
                "@brief Move the transform to the new relative position.\n\n"
                "@ingroup AFX")
{
   Point3F pos1(0.0f,0.0f,0.0f);
   AngAxisF aa1(Point3F(0.0f,0.0f,0.0f),0.0f);
   dSscanf(argv[1], "%g %g %g %g %g %g %g", &pos1.x, &pos1.y, &pos1.z, &aa1.axis.x, &aa1.axis.y, &aa1.axis.z, &aa1.angle);

   Point3F pos2(0.0f,0.0f,0.0f);
   dSscanf(argv[2], "%g %g %g", &pos2.x, &pos2.y, &pos2.z);

   pos2 += pos1;

   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer, 255, "%g %g %g %g %g %g %g",
            pos2.x, pos2.y, pos2.z,
            aa1.axis.x, aa1.axis.y, aa1.axis.z,
            aa1.angle);
   return returnBuffer;
}

DefineEngineFunction(getFreeTargetPosition, Point3F, (),,
                     "@brief Returns the current location of the free target.\n\n"
                     "@ingroup AFX")
{
  if (!arcaneFX::sFreeTargetPosValid)
    return Point3F(0.0f, 0.0f, 0.0f);
  return arcaneFX::sFreeTargetPos;
}

DefineEngineMethod(SceneObject, getSpeed, F32, (),,
                   "Returns the velocity of a scene-object.\n\n"
                   "@ingroup AFX")
{
   return object->getVelocity().len();
}

static S32 mark_modkey = -1;

DefineEngineFunction(markDataBlocks, void, (),,
                     "@brief Called before a series of datablocks are reloaded to "
                     "help distinguish reloaded datablocks from already loaded ones.\n\n"
                     "@ingroup AFX")
{
  mark_modkey = SimDataBlock::getNextModifiedKey();
}

DefineEngineFunction(touchDataBlocks, void, (),,
                     "@brief Called after a series of datablocks are reloaded to "
                     "trigger some important actions on the reloaded datablocks.\n\n"
                     "@ingroup AFX")
{
  if (mark_modkey < 0)
    return;

  SimDataBlockGroup* g = Sim::getDataBlockGroup();

  U32 groupCount = g->size();
  for (S32 i = groupCount-1; i >= 0; i--)
  {
    SimDataBlock* simdb = (SimDataBlock*)(*g)[i];
    if (simdb->getModifiedKey() > mark_modkey)
    {
      simdb->unregisterObject();
      simdb->registerObject();
    }
  }

  mark_modkey = -1;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//
// Syntax Error Checking
// (for checking eval() and compile() calls)

DefineEngineFunction(wasSyntaxError, bool, (),,
                     "@brief Returns true if script compiler had a syntax error. Useful "
                     "for detecting syntax errors after reloading a script.\n\n"
                     "@ingroup AFX")
{
  return Compiler::gSyntaxError;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//
// Network Object Identification

//  These useful console methods come from the following code resource:
//
//  How to Identify Objects from Client to Server or Server to Client by Nathan Davies
//    http://www.garagegames.com/index.php?sec=mg&mod=resource&page=view&qid=4852
//

DefineEngineMethod(NetConnection, GetGhostIndex, S32, (NetObject* obj),,
                   "Returns the ghost-index for an object.\n\n"
                   "@ingroup AFX")
{
  if (obj)
    return object->getGhostIndex(obj);
  return 0;
}

DefineEngineMethod(NetConnection, ResolveGhost, S32, (int ghostIndex),,
                   "Resolves a ghost-index into an object ID.\n\n"
                   "@ingroup AFX")
{
  if (ghostIndex != -1)
  {
    NetObject* pObject = NULL;
    if( object->isGhostingTo())
      pObject = object->resolveGhost(ghostIndex);
    else if( object->isGhostingFrom())
      pObject = object->resolveObjectFromGhostIndex(ghostIndex);
    if (pObject)
      return pObject->getId();
  }
  return 0;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

//////////////////////////////////////////////////////////////////////////
// TypeByteRange
//////////////////////////////////////////////////////////////////////////

IMPLEMENT_STRUCT( ByteRange, ByteRange,,
   "" )
END_IMPLEMENT_STRUCT;

ConsoleType( ByteRange, TypeByteRange, ByteRange, "")
ConsoleType( ByteRange, TypeByteRange2, ByteRange, "")

ConsoleGetType( TypeByteRange )
{
   ByteRange* pt = (ByteRange *) dptr;
   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer, 256, "%u %u", pt->low, pt->high);
   return returnBuffer;
}

ConsoleSetType( TypeByteRange )
{
  if(argc == 1)
  {
    ByteRange* range = (ByteRange*) dptr;
    U32 lo, hi;
    S32 args = dSscanf(argv[0], "%u %u", &lo, &hi);
    range->low = (args > 0) ? lo : 0;
    range->high = (args > 1) ? hi : 255;
  }
  else
    Con::printf("ByteRange must be set as \"low\" or \"low high\"");
}

ConsoleGetType( TypeByteRange2 )
{
   ByteRange* pt = (ByteRange *) dptr;
   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer, 256, "%u %u", pt->low, pt->high);
   return returnBuffer;
}

ConsoleSetType( TypeByteRange2 )
{
  if(argc == 1)
  {
    ByteRange* range = (ByteRange*) dptr;
    U32 lo, hi;
    S32 args = dSscanf(argv[0], "%u %u", &lo, &hi);
    range->low = (args > 0) ? lo : 0;
    range->high = (args > 1) ? hi : lo;
  }
  else
    Con::printf("ByteRange must be set as \"low\" or \"low high\"");
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

static void HSVtoRGB(F32 h, F32 s, F32 v, F32& r, F32& g, F32& b)
{
  h = mFmod(h, 360.0f);

  if (v == 0.0f)
    r = g = b = 0.0f;
  else if (s == 0.0f)
    r = g = b = v;
  else
  {
    F32 hf = h/60.0f;
    S32 i = (S32) mFloor(hf);
    F32 f = hf - i;

    F32 pv = v*(1.0f - s);
    F32 qv = v*(1.0f - s*f);
    F32 tv = v*(1.0f - s*(1.0f - f));

    switch (i)
    {
    case 0:
      r = v;  g = tv; b = pv;
      break;
    case 1:
      r = qv; g = v;  b = pv;
      break;
    case 2:
      r = pv; g = v;  b = tv;
      break;
    case 3:
      r = pv; g = qv; b = v;
      break;
    case 4:
      r = tv; g = pv; b = v;
      break;
    case 5:
      r = v;  g = pv; b = qv;
      break;
    default:
      r = g = b = 0.0f;
      break;
    }
  }
}

DefineEngineFunction(getColorFromHSV, const char*, (float hue, float sat, float val, float alpha), (0.0, 0.0, 1.0, 1.0),
                     "Coverts an HSV formatted color into an RBG color.\n\n"
                     "@param hue The hue of the color (0-360).\n"
                     "@param sat The saturation of the color (0-1).\n"
                     "@param val The value of the color (0-1).\n"
                     "@param alpha The alpha of the color (0-1).\n"
                     "@ingroup AFX")
{
  ColorF rgb;
  HSVtoRGB(hue, sat, val, rgb.red, rgb.green, rgb.blue);
  rgb.alpha = alpha;

  char* returnBuffer = Con::getReturnBuffer(256);
  dSprintf(returnBuffer, 256, "%g %g %g %g", rgb.red, rgb.green, rgb.blue, rgb.alpha);

  return returnBuffer;
}

DefineEngineFunction(ColorScale, const char*, ( ColorF color, float scalar ),,
                     "Returns color scaled by scalar (color*scalar).\n\n"
                     "@param color The color to be scaled.\n"
                     "@param scalar The amount to scale the color.\n"
                     "@ingroup AFX")
{
  color *= scalar;

  char* returnBuffer = Con::getReturnBuffer(256);
  dSprintf(returnBuffer, 256, "%g %g %g %g", color.red, color.green, color.blue, color.alpha);

  return returnBuffer;
}

DefineEngineFunction(getMinF, F32, (float a, float b),,
                     "Returns the lesser of the two arguments.\n\n"
                     "@ingroup AFX")
{
   return getMin(a, b);
}

DefineEngineFunction(getMaxF, F32, (float a, float b),,
                     "Returns the greater of the two arguments.\n\n"
                     "@ingroup AFX")
{
   return getMax(a, b);
}

ConsoleFunction(echoThru, const char*, 2, 0, "(string passthru, string text...)"
                "Like echo(), but first argument is returned.\n"
                "@ingroup AFX")
{
   U32 len = 0;
   S32 i;
   for(i = 2; i < argc; i++)
      len += dStrlen(argv[i]);

   char *ret = Con::getReturnBuffer(len + 1);
   ret[0] = 0;
   for(i = 2; i < argc; i++)
      dStrcat(ret, argv[i]);

   Con::printf("%s -- [%s]", ret, argv[1].getStringValue());
   ret[0] = 0;

   return argv[1];
}

ConsoleFunction(warnThru, const char*, 2, 0, "(string passthru, string text...)"
                "Like warn(), but first argument is returned.\n"
                "@ingroup AFX")
{
   U32 len = 0;
   S32 i;
   for(i = 2; i < argc; i++)
      len += dStrlen(argv[i]);

   char *ret = Con::getReturnBuffer(len + 1);
   ret[0] = 0;
   for(i = 2; i < argc; i++)
      dStrcat(ret, argv[i]);

   Con::warnf("%s -- [%s]", ret, argv[1].getStringValue());
   ret[0] = 0;

   return argv[1];
}

ConsoleFunction(errorThru, const char*, 2, 0, "(string passthru, string text...)"
                "Like error(), but first argument is returned.\n"
                "@ingroup AFX")
{
   U32 len = 0;
   S32 i;
   for(i = 2; i < argc; i++)
      len += dStrlen(argv[i]);

   char *ret = Con::getReturnBuffer(len + 1);
   ret[0] = 0;
   for(i = 2; i < argc; i++)
      dStrcat(ret, argv[i]);

   Con::errorf("%s -- [%s]", ret, argv[1].getStringValue());
   ret[0] = 0;

   return argv[1];
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
