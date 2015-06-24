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

#include "gui/worldEditor/guiMissionAreaEditor.h"
#include "gui/core/guiCanvas.h"
#include "console/engineAPI.h"

IMPLEMENT_CONOBJECT(GuiMissionAreaEditorCtrl);

ConsoleDocClass( GuiMissionAreaEditorCtrl,
   "@brief Specialized GUI used for editing the MissionArea in a level\n\n"
   "Editor use only.\n\n"
   "@internal"
);

GuiMissionAreaEditorCtrl::GuiMissionAreaEditorCtrl()
{
}

GuiMissionAreaEditorCtrl::~GuiMissionAreaEditorCtrl()
{
}

bool GuiMissionAreaEditorCtrl::onAdd()
{
   if( !Parent::onAdd() )
      return false;

   return true;
}

void GuiMissionAreaEditorCtrl::initPersistFields()
{
   Parent::initPersistFields();
}

void GuiMissionAreaEditorCtrl::get3DCursor( GuiCursor *&cursor, 
                                       bool &visible, 
                                       const Gui3DMouseEvent &event_ )
{
   //cursor = mAddNodeCursor;
   //visible = false;
   
   cursor = NULL;
   visible = false;

   GuiCanvas *root = getRoot();
   if ( !root )
      return;

   S32 currCursor = PlatformCursorController::curArrow;

   if ( root->mCursorChanged == currCursor )
      return;

   PlatformWindow *window = root->getPlatformWindow();
   PlatformCursorController *controller = window->getCursorController();
   
   // We've already changed the cursor, 
   // so set it back before we change it again.
   if( root->mCursorChanged != -1)
      controller->popCursor();

   // Now change the cursor shape
   controller->pushCursor(currCursor);
   root->mCursorChanged = currCursor;   
}

void GuiMissionAreaEditorCtrl::setSelectedMissionArea( MissionArea *missionArea )
{
   mSelMissionArea = missionArea;

   if ( mSelMissionArea != NULL )
      Con::executef( this, "onMissionAreaSelected", missionArea->getIdString() );
   else
      Con::executef( this, "onMissionAreaSelected" );
}

DefineConsoleMethod( GuiMissionAreaEditorCtrl, setSelectedMissionArea, void, (const char * missionAreaName), (""), "" )
{
   if ( dStrcmp( missionAreaName, "" )==0 )
      object->setSelectedMissionArea(NULL);
   else
   {
      MissionArea *missionArea = NULL;
      if ( Sim::findObject( missionAreaName, missionArea ) )
         object->setSelectedMissionArea(missionArea);
   }
}

DefineConsoleMethod( GuiMissionAreaEditorCtrl, getSelectedMissionArea, const char*, (), , "" )
{
   MissionArea *missionArea = object->getSelectedMissionArea();
   if ( !missionArea )
      return NULL;

   return missionArea->getIdString();
}
