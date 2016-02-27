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
#include "forest/editor/forestEditorCtrl.h"

#include "forest/editor/forestBrushTool.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "gui/core/guiCanvas.h"
#include "windowManager/platformCursorController.h"
#include "forest/editor/forestUndo.h"
#include "gui/worldEditor/undoActions.h"


IMPLEMENT_CONOBJECT( ForestEditorCtrl );

ConsoleDocClass( ForestEditorCtrl,
   "@brief The actual Forest Editor control\n\n"
   "Editor use only, should not be modified.\n\n"
   "@internal"
);

ForestEditorCtrl::ForestEditorCtrl()
{   
   dMemset( &mLastEvent, 0, sizeof(Gui3DMouseEvent) );
}

ForestEditorCtrl::~ForestEditorCtrl()
{
}

bool ForestEditorCtrl::onAdd()
{
   if ( !Parent::onAdd() )
      return false;

   return true;
}

void ForestEditorCtrl::initPersistFields()
{
   Parent::initPersistFields();
}

bool ForestEditorCtrl::onWake()
{
   if ( !Parent::onWake() )
      return false;

   // Push our default cursor on here once.
   GuiCanvas *root = getRoot();
   if ( root )
   {
      S32 currCursor = PlatformCursorController::curArrow;

      PlatformWindow *window = root->getPlatformWindow();
      PlatformCursorController *controller = window->getCursorController();
      controller->pushCursor( currCursor );
   }

   return true;
}

void ForestEditorCtrl::onSleep()
{
   // Pop our default cursor off.
   GuiCanvas *root = getRoot();
   if ( root )
   {
      PlatformWindow *window = root->getPlatformWindow();
      PlatformCursorController *controller = window->getCursorController();
      controller->popCursor();
   }

   Parent::onSleep();
}

bool ForestEditorCtrl::updateActiveForest( bool createNew )
{
   Con::executef( this, "onActiveForestUpdated", mForest ? mForest->getIdString() : "", createNew ? "1" : "0" );  

   if ( mTool )
      mTool->setActiveForest( mForest );

   return mForest;
}

void ForestEditorCtrl::setActiveTool( ForestTool *tool )
{ 
   if ( mTool )
   {
      mTool->onDeactivated();
   }

   mTool = tool;

   if ( mTool )   
   {
      mTool->setActiveForest( mForest );
      mTool->setParentEditor( this );
      mTool->onActivated( mLastEvent );
   }
}

void ForestEditorCtrl::onMouseUp( const GuiEvent &event_ )
{
   Parent::onMouseUp( event_ );
}

void ForestEditorCtrl::get3DCursor( GuiCursor *&cursor, 
                                       bool &visible, 
                                       const Gui3DMouseEvent &event_ )
{
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

void ForestEditorCtrl::on3DMouseDown( const Gui3DMouseEvent &evt )
{   
   if ( !mForest && !updateActiveForest( true ) )
      return;

   if ( mTool )
      mTool->on3DMouseDown( evt );

   mouseLock();
}

void ForestEditorCtrl::on3DMouseUp( const Gui3DMouseEvent &evt )
{
   if ( !isMouseLocked() )
      return;

   if ( mTool )
      mTool->on3DMouseUp( evt );

   mouseUnlock();
}

void ForestEditorCtrl::on3DMouseMove( const Gui3DMouseEvent &evt )
{
   if ( !mForest )
      updateActiveForest( false );

   if ( mTool )
      mTool->on3DMouseMove( evt );
}

void ForestEditorCtrl::on3DMouseDragged( const Gui3DMouseEvent &evt )
{   
   if ( mTool )
      mTool->on3DMouseDragged( evt );
}

void ForestEditorCtrl::on3DMouseEnter( const Gui3DMouseEvent &evt )
{
   if ( mTool )
      mTool->on3DMouseEnter( evt );
}

void ForestEditorCtrl::on3DMouseLeave( const Gui3DMouseEvent &evt )
{
   if ( mTool )
      mTool->on3DMouseLeave( evt );
}

void ForestEditorCtrl::on3DRightMouseDown( const Gui3DMouseEvent &evt )
{
}

void ForestEditorCtrl::on3DRightMouseUp( const Gui3DMouseEvent &evt )
{
}

bool ForestEditorCtrl::onMouseWheelUp(const GuiEvent &event_)
{
   if ( mTool )
      return mTool->onMouseWheel( event_ );

   return Parent::onMouseWheelUp( event_ );
}

bool ForestEditorCtrl::onMouseWheelDown(const GuiEvent &event_)
{
   if ( mTool )
      return mTool->onMouseWheel( event_ );

   return Parent::onMouseWheelDown( event_ );
}

void ForestEditorCtrl::updateGuiInfo()
{
   // Note: This is intended to be used for updating
   // GuiControls with info before they are rendered.	

   SimObject *statusbar;
   Sim::findObject( "EditorGuiStatusBar", statusbar );

   SimObject *selectionBar;
   Sim::findObject( "EWorldEditorStatusBarSelection", selectionBar );

   String text;

   if ( !mForest )
   {
      if ( statusbar )
         Con::executef( statusbar, "setInfo", "Forest Editor. You have no Forest in your level; click anywhere in the scene to create one." );
      if ( selectionBar )
         Con::executef( selectionBar, "setInfo", "" );
      return;
   }

   if ( mTool )
   {
      if ( mTool->updateGuiInfo() )
         return;
   }

   // Tool did not handle the update so we will.

   if ( statusbar )
      Con::executef( statusbar, "setInfo", "Forest Editor." );
   if ( selectionBar )
      Con::executef( selectionBar, "setInfo", "" );
}
            
void ForestEditorCtrl::renderScene( const RectI &updateRect )
{
   if ( mTool )
      mTool->onRender3D();
} 

void ForestEditorCtrl::updateGizmo()
{
   if ( mTool )
      mTool->updateGizmo();
}

void ForestEditorCtrl::renderGui( Point2I offset, const RectI &updateRect )
{
   if ( mTool )
      mTool->onRender2D();
}

static ForestItemData* sKey = NULL;

bool findMeshReferences( SimObject *obj )
{
   ForestBrushElement *element = dynamic_cast<ForestBrushElement*>(obj);

   if ( element && element->mData == sKey )
      return true;
   
   return false;
}

void ForestEditorCtrl::onUndoAction()
{
   if ( mTool )
      mTool->onUndoAction();

   updateCollision();
}

void ForestEditorCtrl::deleteMeshSafe( ForestItemData *mesh )
{
   UndoManager *undoMan = NULL;
   if ( !Sim::findObject( "EUndoManager", undoMan ) )
   {
      Con::errorf( "ForestEditorCtrl::deleteMeshSafe() - EUndoManager not found." );
      return;     
   }

   // CompoundUndoAction which will delete the ForestItemData, ForestItem(s), and ForestBrushElement(s).
   CompoundUndoAction *compoundAction = new CompoundUndoAction( "Delete Forest Mesh" );
    
   // Find ForestItem(s) referencing this datablock and add their deletion
   // to the undo action.
   if ( mForest )
   {      
      Vector<ForestItem> foundItems;
      mForest->getData()->getItems( mesh, &foundItems );

      ForestDeleteUndoAction *itemAction = new ForestDeleteUndoAction( mForest->getData(), this );
      itemAction->removeItem( foundItems );
      compoundAction->addAction( itemAction );
   }

   // Find ForestBrushElement(s) referencing this datablock.
   SimGroup *brushGroup = ForestBrush::getGroup();
   sKey = mesh;
   Vector<SimObject*> foundElements;   
   brushGroup->findObjectByCallback( &findMeshReferences, foundElements );   

   // Add UndoAction to delete the ForestBrushElement(s) and the ForestItemData.
   MEDeleteUndoAction *elementAction = new MEDeleteUndoAction();
   elementAction->deleteObject( foundElements );
   elementAction->deleteObject( mesh );
   
   // Add compound action to the UndoManager. Done.
   undoMan->addAction( compoundAction );

   updateCollision();
}

void ForestEditorCtrl::updateCollision()
{
   if ( mForest )
   {
      mForest->updateCollision();

      if ( mForest->getClientObject() )
         ((Forest*)(mForest->getClientObject()))->updateCollision();
   }
}

void FindDirtyForests( SceneObject *obj, void *key )
{
   Forest *forest = dynamic_cast<Forest*>(obj);
   if ( forest && forest->getData()->isDirty() )   
      *((bool*)(key)) = true;
}

bool ForestEditorCtrl::isDirty()
{   
   bool foundDirty = false;
   gServerContainer.findObjects( EnvironmentObjectType, FindDirtyForests, (void*)&foundDirty );

   return foundDirty;   
}

DefineConsoleMethod( ForestEditorCtrl, updateActiveForest, void, (), , "()" )
{
   object->updateActiveForest( true );
}

DefineConsoleMethod( ForestEditorCtrl, setActiveTool, void, ( const char * toolName ), , "( ForestTool tool )" )
{
   ForestTool *tool = dynamic_cast<ForestTool*>( Sim::findObject( toolName ) );
   object->setActiveTool( tool );
}

DefineConsoleMethod( ForestEditorCtrl, getActiveTool, S32, (), , "()" )
{
   ForestTool *tool = object->getActiveTool();
   return tool ? tool->getId() : 0;
}

DefineConsoleMethod( ForestEditorCtrl, deleteMeshSafe, void, ( const char * obj ), , "( ForestItemData obj )" )
{
   ForestItemData *db;
   if ( !Sim::findObject( obj, db ) )
      return;

   object->deleteMeshSafe( db );   
}

DefineConsoleMethod( ForestEditorCtrl, isDirty, bool, (), , "" )
{
   return object->isDirty();
}

DefineConsoleMethod(ForestEditorCtrl, setActiveForest, void, (const char * obj), , "( Forest obj )")
{
   Forest *forestObject;
   if (!Sim::findObject(obj, forestObject))
      return;

   object->setActiveForest(forestObject);
}