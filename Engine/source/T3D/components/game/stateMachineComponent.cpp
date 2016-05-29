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

#include "T3D/components/game/stateMachineComponent.h"

#include "platform/platform.h"
#include "console/consoleTypes.h"
#include "core/util/safeDelete.h"
#include "core/resourceManager.h"
#include "core/stream/fileStream.h"
#include "console/consoleTypes.h"
#include "console/consoleObject.h"
#include "ts/tsShapeInstance.h"
#include "core/stream/bitStream.h"
#include "gfx/gfxTransformSaver.h"
#include "console/engineAPI.h"
#include "lighting/lightQuery.h"

IMPLEMENT_CALLBACK( StateMachineComponent, onStateChange, void, (), (),
                   "@brief Called when we collide with another object.\n\n"
                   "@param obj The ShapeBase object\n"
                   "@param collObj The object we collided with\n"
                   "@param vec Collision impact vector\n"
                   "@param len Length of the impact vector\n" );

//////////////////////////////////////////////////////////////////////////
// Constructor/Destructor
//////////////////////////////////////////////////////////////////////////

StateMachineComponent::StateMachineComponent() : Component()
{
   mFriendlyName = "State Machine";
   mComponentType = "Game";

   mDescription = getDescriptionText("A generic state machine.");

   mStateMachineFile = "";

   //doesn't need to be networked
   mNetworked = false;
   mNetFlags.clear();
}

StateMachineComponent::~StateMachineComponent()
{
   for(S32 i = 0;i < mFields.size();++i)
   {
      ComponentField &field = mFields[i];
      SAFE_DELETE_ARRAY(field.mFieldDescription);
   }

   SAFE_DELETE_ARRAY(mDescription);
}

IMPLEMENT_CO_NETOBJECT_V1(StateMachineComponent);

bool StateMachineComponent::onAdd()
{
   if(! Parent::onAdd())
      return false;

   // Register for the resource change signal.
   ResourceManager::get().getChangedSignal().notify(this, &StateMachineComponent::_onResourceChanged);

   mStateMachine.onStateChanged.notify(this, &StateMachineComponent::onStateChanged);

   return true;
}

void StateMachineComponent::onRemove()
{
   Parent::onRemove();
}

U32 StateMachineComponent::packUpdate(NetConnection *con, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);
   return retMask;
}

void StateMachineComponent::unpackUpdate(NetConnection *con, BitStream *stream)
{
   Parent::unpackUpdate(con, stream);
}

//This is mostly a catch for situations where the behavior is re-added to the object and the like and we may need to force an update to the behavior
void StateMachineComponent::onComponentAdd()
{
   Parent::onComponentAdd();
}

void StateMachineComponent::onComponentRemove()
{
   Parent::onComponentRemove();
}

void StateMachineComponent::initPersistFields()
{
   Parent::initPersistFields();

   addProtectedField("stateMachineFile", TypeFilename, Offset(mStateMachineFile, StateMachineComponent), 
      &_setSMFile, &defaultProtectedGetFn, "The sim time of when we started this state");
}

bool StateMachineComponent::_setSMFile(void *object, const char *index, const char *data)
{
   StateMachineComponent* smComp = static_cast<StateMachineComponent*>(object);
   if (smComp)
   {
      smComp->setStateMachineFile(data);
      smComp->loadStateMachineFile();

      return true;
   }

   return false;
}

void StateMachineComponent::_onResourceChanged(const Torque::Path &path)
{
   if (path != Torque::Path(mStateMachineFile))
      return;

   loadStateMachineFile();
}

void StateMachineComponent::loadStateMachineFile()
{
   if (!dStrIsEmpty(mStateMachineFile))
   {
      mStateMachine.mStateMachineFile = mStateMachineFile;
      mStateMachine.loadStateMachineFile();

      //now that it's loaded, we need to parse the SM's fields and set them as script vars on ourselves
      S32 smFieldCount = mStateMachine.getFieldsCount();

      for (U32 i = 0; i < smFieldCount; i++)
      {
         StateMachine::StateField field = mStateMachine.getField(i);

         char buffer[128];

         if (field.fieldType == StateMachine::StateField::BooleanType)
         {
            dSprintf(buffer, sizeof(buffer), "%b", field.triggerBoolVal);
            setDataField(field.name, NULL, buffer);
         }
         else if (field.fieldType == StateMachine::StateField::NumberType)
         {
            dSprintf(buffer, sizeof(buffer), "%g", field.triggerNumVal);
            setDataField(field.name, NULL, buffer);
         }
         else if (field.fieldType == StateMachine::StateField::StringType)
         {
            setDataField(field.name, NULL, field.triggerStringVal);
         }
      }
   }
}

void StateMachineComponent::processTick()
{
   if (!isServerObject() || !isActive())
      return;

   mStateMachine.update();
}

void StateMachineComponent::onDynamicModified( const char* slotName, const char* newValue )
{
   Parent::onDynamicModified(slotName, newValue);

   StringTableEntry fieldName = StringTable->insert(slotName);
   mStateMachine.checkTransitions(fieldName, newValue);
}

void StateMachineComponent::onStaticModified( const char* slotName, const char* newValue )
{
   Parent::onStaticModified(slotName, newValue);

   StringTableEntry fieldName = StringTable->insert(slotName);
   mStateMachine.checkTransitions(fieldName, newValue);
}

void StateMachineComponent::onStateChanged(StateMachine* sm, S32 stateIdx)
{
   //do a script callback, if we have one
   //check if we have a function for that, and then also check if our owner does
   StringTableEntry callbackName = mStateMachine.getCurrentState().callbackName;

   if (isMethod(callbackName))
         Con::executef(this, callbackName);

   if (mOwner->isMethod(callbackName))
      Con::executef(mOwner, callbackName);
}