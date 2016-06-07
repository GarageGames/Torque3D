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

#ifndef STATE_MACHINE_COMPONENT_H
#define STATE_MACHINE_COMPONENT_H

#ifndef COMPONENT_H
   #include "T3D/components/component.h"
#endif
#ifndef STATE_MACHINE_H
#include "T3D/components/game/stateMachine.h"
#endif

//////////////////////////////////////////////////////////////////////////
/// 
/// 
//////////////////////////////////////////////////////////////////////////
class StateMachineComponent : public Component
{
   typedef Component Parent;

public:
   StateMachine mStateMachine;

protected:
   StringTableEntry		mStateMachineFile;

public:
   StateMachineComponent();
   virtual ~StateMachineComponent();
   DECLARE_CONOBJECT(StateMachineComponent);

   virtual bool onAdd();
   virtual void onRemove();
   static void initPersistFields();

   virtual void onComponentAdd();
   virtual void onComponentRemove();

   void _onResourceChanged(const Torque::Path &path);

   virtual U32 packUpdate(NetConnection *con, U32 mask, BitStream *stream);
   virtual void unpackUpdate(NetConnection *con, BitStream *stream);

   virtual void processTick();

   virtual void onDynamicModified(const char* slotName, const char* newValue);
   virtual void onStaticModified(const char* slotName, const char* newValue);

   virtual void loadStateMachineFile();

   void setStateMachineFile(const char* fileName) { mStateMachineFile = StringTable->insert(fileName); }

   static bool _setSMFile(void *object, const char *index, const char *data);

   void onStateChanged(StateMachine* sm, S32 stateIdx);

   //Callbacks
   DECLARE_CALLBACK(void, onStateChange, ());
};

#endif