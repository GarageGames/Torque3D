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

#include "gui/worldEditor/editor.h"
#include "console/console.h"
#include "console/consoleInternal.h"
#include "gui/controls/guiTextListCtrl.h"
#include "T3D/shapeBase.h"
#include "T3D/gameBase/gameConnection.h"

#ifndef TORQUE_PLAYER
// See matching #ifdef in app/game.cpp
bool gEditingMission = false;
#endif

//------------------------------------------------------------------------------
// Class EditManager
//------------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(EditManager);

ConsoleDocClass( EditManager,
   "@brief For Editor use only, deprecated\n\n"
   "@internal"
);

EditManager::EditManager()
{
   for(U32 i = 0; i < 10; i++)
      mBookmarks[i] = MatrixF(true);
}

EditManager::~EditManager()
{
}

//------------------------------------------------------------------------------

bool EditManager::onWake()
{
   if(!Parent::onWake())
      return(false);

   return(true);
}

void EditManager::onSleep()
{
   Parent::onSleep();
}

//------------------------------------------------------------------------------

bool EditManager::onAdd()
{
   if(!Parent::onAdd())
      return(false);

   // hook the namespace
   const char * name = getName();
   if(name && name[0] && getClassRep())
   {
      Namespace * parent = getClassRep()->getNameSpace();
      Con::linkNamespaces(parent->mName, name);
      mNameSpace = Con::lookupNamespace(name);
   }

   return(true);
}

//------------------------------------------------------------------------------

// NOTE: since EditManager is not actually used as a gui anymore, onWake/Sleep
// were never called, which broke anyone hooking into onEditorEnable/onEditorDisable 
// and gEditingMission. So, moved these to happen in response to console methods
// which should be called at the appropriate time.
//
// This is a quick fix and this system is still "begging" for a remake.

void EditManager::editorEnabled()
{
   for(SimGroupIterator itr(Sim::getRootGroup());  *itr; ++itr)
      (*itr)->onEditorEnable();

   gEditingMission = true;
}

void EditManager::editorDisabled()
{
   for(SimGroupIterator itr(Sim::getRootGroup());  *itr; ++itr)
   {
      SimObject *so = *itr;
      AssertFatal(so->isProperlyAdded() && !so->isRemoved(), "bad");
      so->onEditorDisable();
   }

   gEditingMission = false;
}

//------------------------------------------------------------------------------

static GameBase * getControlObj()
{
   GameConnection * connection = GameConnection::getLocalClientConnection();
   ShapeBase* control = 0;
   if(connection)
      control = dynamic_cast<ShapeBase*>(connection->getControlObject());
   return(control);
}

ConsoleMethod( EditManager, setBookmark, void, 3, 3, "(int slot)")
{
   S32 val = dAtoi(argv[2]);
   if(val < 0 || val > 9)
      return;

   GameBase * control = getControlObj();
   if(control)
      object->mBookmarks[val] = control->getTransform();
}

ConsoleMethod( EditManager, gotoBookmark, void, 3, 3, "(int slot)")
{
   S32 val = dAtoi(argv[2]);
   if(val < 0 || val > 9)
      return;

   GameBase * control = getControlObj();
   if(control)
      control->setTransform(object->mBookmarks[val]);
}

ConsoleMethod( EditManager, editorEnabled, void, 2, 2, "Perform the onEditorEnabled callback on all SimObjects and set gEditingMission true" )
{
   object->editorEnabled();
}

ConsoleMethod( EditManager, editorDisabled, void, 2, 2, "Perform the onEditorDisabled callback on all SimObjects and set gEditingMission false" )
{
   object->editorDisabled();
}

ConsoleMethod( EditManager, isEditorEnabled, bool, 2, 2, "Return the value of gEditingMission." )
{
   return gEditingMission;
}