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

#include "console/consoleTypes.h"
#include "console/console.h"
#include "console/engineAPI.h"
#include "gfx/gfxDevice.h"
#include "gui/core/guiCanvas.h"
#include "gui/controls/guiTabPageCtrl.h"
#include "gui/containers/guiTabBookCtrl.h"
#include "gui/core/guiDefaultControlRender.h"
#include "gui/editor/guiEditCtrl.h"

IMPLEMENT_CONOBJECT(GuiTabPageCtrl);

ConsoleDocClass( GuiTabPageCtrl,
   "@brief A single page in a GuiTabBookCtrl.\n\n"

   "@tsexample\n\n"
   "new GuiTabPageCtrl()\n"
   "{\n"
   "   fitBook = \"1\";\n"
   "   //Properties not specific to this control have been omitted from this example.\n"
   "};\n"
   "@endtsexample\n\n"

   "@ingroup GuiContainers"
);

GuiTabPageCtrl::GuiTabPageCtrl(void)
{
   setExtent(Point2I(100, 200));
   mFitBook = false;
   dStrcpy(mText,(UTF8*)"TabPage");
   mActive = true;
   mIsContainer = true;
}

void GuiTabPageCtrl::initPersistFields()
{
   addField( "fitBook", TypeBool, Offset( mFitBook, GuiTabPageCtrl ),
      "Determines whether to resize this page when it is added to the tab book. "
      "If true, the page will be resized according to the tab book extents and "
      "<i>tabPosition</i> property." );

   Parent::initPersistFields();
}

bool GuiTabPageCtrl::onWake()
{
   if (! Parent::onWake())
      return false;

   return true;
}

void GuiTabPageCtrl::onSleep()
{
   Parent::onSleep();
}

GuiControl* GuiTabPageCtrl::findHitControl(const Point2I &pt, S32 initialLayer)
{
   return Parent::findHitControl(pt, initialLayer);
}

void GuiTabPageCtrl::onMouseDown(const GuiEvent &event)
{
   setUpdate();
   Point2I localPoint = globalToLocalCoord( event.mousePoint );

   GuiControl *ctrl = findHitControl(localPoint);
   if (ctrl && ctrl != this)
   {
      ctrl->onMouseDown(event);
   }
}

bool GuiTabPageCtrl::onMouseDownEditor(const GuiEvent &event, Point2I offset )
{
#ifdef TORQUE_TOOLS
   // This shouldn't be called if it's not design time, but check just incase
   if ( GuiControl::smDesignTime )
   {
      GuiEditCtrl* edit = GuiControl::smEditorHandle;
      if( edit )
         edit->select( this );
   }

   return Parent::onMouseDownEditor( event, offset );
#else
   return false;
#endif
}


GuiControl *GuiTabPageCtrl::findNextTabable(GuiControl *curResponder, bool firstCall)
{
   //set the global if this is the first call (directly from the canvas)
   if (firstCall)
   {
      GuiControl::smCurResponder = NULL;
   }

   //if the window does not already contain the first responder, return false
   //ie.  Can't tab into or out of a window
   if (! controlIsChild(curResponder))
   {
      return NULL;
   }

   //loop through, checking each child to see if it is the one that follows the firstResponder
   GuiControl *tabCtrl = NULL;
   iterator i;
   for (i = begin(); i != end(); i++)
   {
      GuiControl *ctrl = static_cast<GuiControl *>(*i);
      tabCtrl = ctrl->findNextTabable(curResponder, false);
      if (tabCtrl) break;
   }

   //to ensure the tab cycles within the current window...
   if (! tabCtrl)
   {
      tabCtrl = findFirstTabable();
   }

   mFirstResponder = tabCtrl;
   return tabCtrl;
}

GuiControl *GuiTabPageCtrl::findPrevTabable(GuiControl *curResponder, bool firstCall)
{
   if (firstCall)
   {
      GuiControl::smPrevResponder = NULL;
   }

   //if the window does not already contain the first responder, return false
   //ie.  Can't tab into or out of a window
   if (! controlIsChild(curResponder))
   {
      return NULL;
   }

   //loop through, checking each child to see if it is the one that follows the firstResponder
   GuiControl *tabCtrl = NULL;
   iterator i;
   for (i = begin(); i != end(); i++)
   {
      GuiControl *ctrl = static_cast<GuiControl *>(*i);
      tabCtrl = ctrl->findPrevTabable(curResponder, false);
      if (tabCtrl) break;
   }

   //to ensure the tab cycles within the current window...
   if (! tabCtrl)
   {
      tabCtrl = findLastTabable();
   }

   mFirstResponder = tabCtrl;
   return tabCtrl;
}

void GuiTabPageCtrl::setText(const char *txt)
{
   Parent::setText( txt );

   GuiControl *parent = getParent();
   if( parent )
      parent->setUpdate();
};


void GuiTabPageCtrl::selectWindow(void)
{
   //first make sure this window is the front most of its siblings
   GuiControl *parent = getParent();
   if (parent)
   {
      parent->pushObjectToBack(this);
   }

   //also set the first responder to be the one within this window
   setFirstResponder(mFirstResponder);
}

void GuiTabPageCtrl::onRender(Point2I offset,const RectI &updateRect)
{
   // Call directly into GuiControl to skip the GuiTextCtrl parent render
   GuiControl::onRender( offset, updateRect );
}

void GuiTabPageCtrl::inspectPostApply()
{
   Parent::inspectPostApply();
   
   if( mFitBook )
   {
      GuiTabBookCtrl* book = dynamic_cast< GuiTabBookCtrl* >( getParent() );
      if( book )
         book->fitPage( this );
   }
}

DefineEngineMethod( GuiTabPageCtrl, select, void, (),,
   "Select this page in its tab book." )
{
   GuiTabBookCtrl* book = dynamic_cast< GuiTabBookCtrl* >( object->getParent() );
   if( !book )
      return;

   book->selectPage( object );
}
