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

#include "platform/platform.h"
#include "console/simBase.h"
#include "console/consoleTypes.h"
#include "console/scriptObjects.h"
#include "console/simBase.h"
#include "console/engineAPI.h"

//-----------------------------------------------------------------------------
// ScriptObject
//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(ScriptObject);

ConsoleDocClass( ScriptObject,
   "@brief A script-level OOP object which allows binding of a class, superClass and arguments along with declaration of methods.\n\n"   

   "ScriptObjects are extrodinarily powerful objects that allow defining of any type of data required. They can optionally have\n"
   "a class and a superclass defined for added control of multiple ScriptObjects through a simple class definition.\n\n"

    "@tsexample\n"
      "new ScriptObject(Game)\n"
      "{\n"
      "   class = \"DeathMatchGame\";\n"
      "   superClass = GameCore;\n"
      "   genre = \"Action FPS\"; // Note the new, non-Torque variable\n"
      "};\n"
    "@endtsexample\n"
   "@see SimObject\n"
   "@ingroup Console\n"
   "@ingroup Scripting"
);

IMPLEMENT_CALLBACK( ScriptObject, onAdd, void, ( SimObjectId ID ), ( ID ),
	"Called when this ScriptObject is added to the system.\n"
	"@param ID Unique object ID assigned when created (%this in script).\n"
);

IMPLEMENT_CALLBACK( ScriptObject, onRemove, void, ( SimObjectId ID ), ( ID ),
	"Called when this ScriptObject is removed from the system.\n"
	"@param ID Unique object ID assigned when created (%this in script).\n"
);

ScriptObject::ScriptObject()
{
}

bool ScriptObject::onAdd()
{
   if (!Parent::onAdd())
      return false;

   // Call onAdd in script!
   onAdd_callback(getId());
   return true;
}

void ScriptObject::onRemove()
{
   // We call this on this objects namespace so we unlink them after. - jdd
   //
   // Call onRemove in script!
   onRemove_callback(getId());
   
   Parent::onRemove();
}

//-----------------------------------------------------------------------------
// ScriptTickObject
//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(ScriptTickObject);

ConsoleDocClass( ScriptTickObject,
   "@brief A ScriptObject that responds to tick and frame events.\n\n"   

   "ScriptTickObject is a ScriptObject that adds callbacks for tick and frame events.  Use "
   "setProcessTicks() to enable or disable the onInterpolateTick() and onProcessTick() callbacks.  "
   "The callOnAdvanceTime property determines if the onAdvanceTime() callback is called.\n\n"

   "@see ScriptObject\n"
   "@ingroup Console\n"
   "@ingroup Scripting"
);

IMPLEMENT_CALLBACK( ScriptTickObject, onInterpolateTick, void, ( F32 delta ), ( delta ),
	"This is called every frame, but only if the object is set to process ticks.\n"
	"@param delta The time delta for this frame.\n"
);

IMPLEMENT_CALLBACK( ScriptTickObject, onProcessTick, void, (), (),
	"Called once every 32ms if this object is set to process ticks.\n"
);

IMPLEMENT_CALLBACK( ScriptTickObject, onAdvanceTime, void, ( F32 timeDelta ), ( timeDelta ),
	"This is called every frame regardless if the object is set to process ticks, but only "
   "if the callOnAdvanceTime property is set to true.\n"
	"@param timeDelta The time delta for this frame.\n"
   "@see callOnAdvanceTime\n"
);

ScriptTickObject::ScriptTickObject()
{
   mCallOnAdvanceTime = false;
}

void ScriptTickObject::initPersistFields()
{
   addField("callOnAdvanceTime", TypeBool,   Offset(mCallOnAdvanceTime,  ScriptTickObject), "Call the onAdvaceTime() callback.");

   Parent::initPersistFields();
}

bool ScriptTickObject::onAdd()
{
   if (!Parent::onAdd())
      return false;

   return true;
}

void ScriptTickObject::onRemove()
{
   Parent::onRemove();
}

void ScriptTickObject::interpolateTick( F32 delta )
{
   onInterpolateTick_callback(delta);
}

void ScriptTickObject::processTick()
{
   onProcessTick_callback();
}

void ScriptTickObject::advanceTime( F32 timeDelta )
{
   if(mCallOnAdvanceTime)
   {
      onAdvanceTime_callback(timeDelta);
   }
}

DefineEngineMethod( ScriptTickObject, setProcessTicks, void, ( bool tick ),,
   "@brief Sets this object as either tick processing or not.\n\n"

   "@param tick This object's onInterpolateTick() and onProcessTick() callbacks are called if set to true.\n\n")
{
   object->setProcessTicks(tick);
}

DefineEngineMethod( ScriptTickObject, isProcessingTicks, bool, ( ),,
   "@brief Is this object wanting to receive tick notifications.\n\n"

   "If this object is set to receive tick notifications then its onInterpolateTick() and "
   "onProcessTick() callbacks are called.\n"
   "@return True if object wants tick notifications\n\n" )
{
   return object->isProcessingTicks();
}

//-----------------------------------------------------------------------------
// ScriptGroup
//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(ScriptGroup);

ConsoleDocClass( ScriptGroup,
		"@brief Essentially a SimGroup, but with onAdd and onRemove script callbacks.\n\n"

		"@tsexample\n"
		"// First container, SimGroup containing a ScriptGroup\n"
		"new SimGroup(Scenes)\n"
		"{\n"
		"	// Subcontainer, ScriptGroup containing variables\n"
		"	// related to a cut scene and a starting WayPoint\n"
		"	new ScriptGroup(WelcomeScene)\n"
		"	{\n"
		"		class = \"Scene\";\n"
		"		pathName = \"Pathx\";\n"
		"		description = \"A small orc village set in the Hardesty mountains. This town and its surroundings will be used to illustrate some the Torque Game Engine\'s features.\";\n"
		"		pathTime = \"0\";\n"
		"		title = \"Welcome to Orc Town\";\n\n"
		"		new WayPoint(start)\n"
		"		{\n"
		"			position = \"163.873 -103.82 208.354\";\n"
		"			rotation = \"0.136165 -0.0544916 0.989186 44.0527\";\n"
		"			scale = \"1 1 1\";\n"
		"			dataBlock = \"WayPointMarker\";\n"
		"			team = \"0\";\n"
		"		};\n"
		"	};\n"
		"};\n"
		"@endtsexample\n\n"

		"@see SimGroup\n"

		"@ingroup Console\n"
		"@ingroup Scripting"
);

ScriptGroup::ScriptGroup()
{
}

IMPLEMENT_CALLBACK( ScriptGroup, onAdd, void, ( SimObjectId ID ), ( ID ),
	"Called when this ScriptGroup is added to the system.\n"
	"@param ID Unique object ID assigned when created (%this in script).\n" 
);

IMPLEMENT_CALLBACK( ScriptGroup, onRemove, void, ( SimObjectId ID ), ( ID ),
	"Called when this ScriptObject is removed from the system.\n"
	"@param ID Unique object ID assigned when created (%this in script).\n" 
);

bool ScriptGroup::onAdd()
{
   if (!Parent::onAdd())
      return false;

   // Call onAdd in script!
   onAdd_callback(getId());
   return true;
}

void ScriptGroup::onRemove()
{
   // Call onRemove in script!
	onRemove_callback(getId());

   Parent::onRemove();
}
