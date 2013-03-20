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

#ifndef _SCRIPTOBJECTS_H_
#define _SCRIPTOBJECTS_H_

#ifndef _CONSOLEINTERNAL_H_
#include "console/consoleInternal.h"
#endif

#ifndef _ITICKABLE_H_
#include "core/iTickable.h"
#endif

//-----------------------------------------------------------------------------
// ScriptObject
//-----------------------------------------------------------------------------

class ScriptObject : public SimObject
{
   typedef SimObject Parent;

public:
   ScriptObject();
   bool onAdd();
   void onRemove();

   DECLARE_CONOBJECT(ScriptObject);

   DECLARE_CALLBACK(void, onAdd, (SimObjectId ID) );
   DECLARE_CALLBACK(void, onRemove, (SimObjectId ID));
};

//-----------------------------------------------------------------------------
// ScriptTickObject
//-----------------------------------------------------------------------------

class ScriptTickObject : public ScriptObject, public virtual ITickable
{
   typedef ScriptObject Parent;

protected:
   bool mCallOnAdvanceTime;

public:
   ScriptTickObject();
   static void initPersistFields();
   bool onAdd();
   void onRemove();

   virtual void interpolateTick( F32 delta );
   virtual void processTick();
   virtual void advanceTime( F32 timeDelta );

   DECLARE_CONOBJECT(ScriptTickObject);

   DECLARE_CALLBACK(void, onInterpolateTick, (F32 delta) );
   DECLARE_CALLBACK(void, onProcessTick, () );
   DECLARE_CALLBACK(void, onAdvanceTime, (F32 timeDelta) );
};

//-----------------------------------------------------------------------------
// ScriptGroup
//-----------------------------------------------------------------------------

class ScriptGroup : public SimGroup
{
   typedef SimGroup Parent;
   
public:
   ScriptGroup();
   bool onAdd();
   void onRemove();

   DECLARE_CONOBJECT(ScriptGroup);

   DECLARE_CALLBACK(void, onAdd, (SimObjectId ID) );
   DECLARE_CALLBACK(void, onRemove, (SimObjectId ID));
};

#endif