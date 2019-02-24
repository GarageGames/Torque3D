#pragma once

#include "T3D/components/component.h"

#include "T3D/gameBase/gameConnection.h"

class ControlObjectComponent : public Component
{
   typedef Component Parent;

   GameConnection* mOwnerConnection;
   S32 mOwnerConnectionId;

public:
   ControlObjectComponent();
   ~ControlObjectComponent();

   DECLARE_CONOBJECT(ControlObjectComponent);

   virtual bool onAdd();
   virtual void onRemove();
   static void initPersistFields();

   virtual void onComponentAdd();
   virtual void onComponentRemove();

   void onClientConnect(GameConnection* conn);
   void onClientDisconnect(GameConnection* conn);
   void setConnectionControlObject(GameConnection* conn);
};