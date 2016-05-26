//-----------------------------------------------------------------------------
// Torque Game Engine
// Copyright (C) GarageGames.com, Inc.
//-----------------------------------------------------------------------------
#ifndef _TRIGGER_COMPONENT_H_
#define _TRIGGER_COMPONENT_H_

#ifndef _COMPONENT_H_
#include "T3D/components/component.h"
#endif

#ifndef _ENTITY_H_
#include "T3D/entity.h"
#endif

#ifndef _COLLISION_INTERFACES_H_
#include "T3D/components/collision/collisionInterfaces.h"
#endif

//////////////////////////////////////////////////////////////////////////
/// 
/// 
//////////////////////////////////////////////////////////////////////////
class TriggerComponent : public Component
{
   typedef Component Parent;

protected:
   Vector<SceneObject*> mObjectList;

   bool mVisible;

   String mEnterCommand;
   String mOnExitCommand;
   String mOnUpdateInViewCmd;

public:
   TriggerComponent();
   virtual ~TriggerComponent();
   DECLARE_CONOBJECT(TriggerComponent);

   virtual bool onAdd();
   virtual void onRemove();
   static void initPersistFields();

   virtual void onComponentAdd();
   virtual void onComponentRemove();

   virtual void componentAddedToOwner(Component *comp);
   virtual void componentRemovedFromOwner(Component *comp);

   virtual U32 packUpdate(NetConnection *con, U32 mask, BitStream *stream);
   virtual void unpackUpdate(NetConnection *con, BitStream *stream);

   void potentialEnterObject(SceneObject *collider);

   bool testObject(SceneObject* enter);

   virtual void processTick();

   GameConnection* getConnection(S32 connectionID);

   void addClient(S32 clientID);
   void removeClient(S32 clientID);

   void visualizeFrustums(F32 renderTimeMS);

   DECLARE_CALLBACK(void, onEnterViewCmd, (Entity* cameraEnt, bool firstTimeSeeing));
   DECLARE_CALLBACK(void, onExitViewCmd, (Entity* cameraEnt));
   DECLARE_CALLBACK(void, onUpdateInViewCmd, (Entity* cameraEnt));
   DECLARE_CALLBACK(void, onUpdateOutOfViewCmd, (Entity* cameraEnt));
};

#endif // _EXAMPLEBEHAVIOR_H_
