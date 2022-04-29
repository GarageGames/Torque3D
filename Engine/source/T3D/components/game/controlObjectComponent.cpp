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

#include "T3D/components/game/controlObjectComponent.h"


//////////////////////////////////////////////////////////////////////////
// Constructor/Destructor
//////////////////////////////////////////////////////////////////////////
ControlObjectComponent::ControlObjectComponent() : Component(),
   mOwnerConnection(nullptr),
   mOwnerConnectionId(0)
{
   mFriendlyName = "Control Object";
   mComponentType = "Game";

   mDescription = getDescriptionText("Allows owner entity to be controlled by a client.");
}

ControlObjectComponent::~ControlObjectComponent()
{
}

IMPLEMENT_CO_NETOBJECT_V1(ControlObjectComponent);

bool ControlObjectComponent::onAdd()
{
   if (!Parent::onAdd())
      return false;

   return true;
}

void ControlObjectComponent::onRemove()
{
   Parent::onRemove();
}

//This is mostly a catch for situations where the behavior is re-added to the object and the like and we may need to force an update to the behavior
void ControlObjectComponent::onComponentAdd()
{
   Parent::onComponentAdd();

   if (mOwnerConnection && mOwnerConnection->getControlObject() == nullptr)
   {
      mOwnerConnection->setControlObject(mOwner);
      mOwnerConnectionId = mOwnerConnection->getId();
   }
}

void ControlObjectComponent::onComponentRemove()
{
   Parent::onComponentRemove();

   if (mOwnerConnection)
   {
      mOwnerConnection->setControlObject(nullptr);
      mOwnerConnectionId = 0;
   }
}

void ControlObjectComponent::initPersistFields()
{
   Parent::initPersistFields();

   addField("clientOwner", TypeS32, Offset(mOwnerConnectionId, ControlObjectComponent), "Client connection ID");
}

void ControlObjectComponent::setConnectionControlObject(GameConnection* conn)
{
   if (conn)
   {
      if (conn->getControlObject() == nullptr)
      {
         conn->setControlObject(mOwner);
         mOwnerConnectionId = conn->getId();
      }
      else
      {
         //Inform the old control object it's no longer in control?
         conn->setControlObject(mOwner);
         mOwnerConnectionId = conn->getId();
      }
   }
}

void ControlObjectComponent::onClientConnect(GameConnection* conn)
{
   if (conn && conn->getControlObject() == nullptr)
   {
      conn->setControlObject(mOwner);
      mOwnerConnectionId = conn->getId();
   }
}

void ControlObjectComponent::onClientDisconnect(GameConnection* conn)
{
   if (conn && conn->getControlObject() == mOwner)
   {
      conn->setControlObject(nullptr);
      mOwnerConnectionId = 0;
   }
}

DefineEngineMethod(ControlObjectComponent, onClientConnect, void, (GameConnection* conn), (nullAsType<GameConnection*>()),
"Triggers a signal call to all components for a certain function.")
{
   if (conn == nullptr)
      return;

   object->onClientConnect(conn);
}

DefineEngineMethod(ControlObjectComponent, onClientDisconnect, void, (GameConnection* conn), (nullAsType<GameConnection*>()),
   "Triggers a signal call to all components for a certain function.")
{
   if (conn == nullptr)
      return;

   object->onClientDisconnect(conn);
}

DefineEngineMethod(ControlObjectComponent, setConnectionControlObject, void, (GameConnection* conn), (nullAsType<GameConnection*>()),
   "Triggers a signal call to all components for a certain function.")
{
   if (conn == nullptr)
      return;

   object->setConnectionControlObject(conn);
}
