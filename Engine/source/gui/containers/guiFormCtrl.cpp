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

#include "console/engineAPI.h"
#include "platform/platform.h"
#include "gui/containers/guiFormCtrl.h"

#include "gui/core/guiDefaultControlRender.h"
#include "gfx/gfxDrawUtil.h"

#ifdef TORQUE_TOOLS

IMPLEMENT_CONOBJECT(GuiFormCtrl);

ConsoleDocClass( GuiFormCtrl,
   "@brief A generic form control.\n\n"
   "Currently editor use only.\n\n "
   "@internal"
);

IMPLEMENT_CALLBACK( GuiFormCtrl, onResize, void, (), (),
   "Called when the control is resized." );

GuiFormCtrl::GuiFormCtrl()
{
   setMinExtent(Point2I(200,100));
   mActive        = true;
   mMouseOver     = false;
   mDepressed     = false;
   mCanMove       = false;
   mCaption       = "[none]";
   mUseSmallCaption = false;

   mContentLibrary = StringTable->insert("");
   mContent = StringTable->insert("");

   mCanSaveFieldDictionary = true;
   mIsContainer = true;

   // The attached menu bar
   mHasMenu = false;
   mMenuBar = NULL;
}

GuiFormCtrl::~GuiFormCtrl()
{
   // If we still have a menu bar, delete it.

   if( mMenuBar )
      mMenuBar->deleteObject();
}

bool GuiFormCtrl::_setHasMenu( void *object, const char *index, const char *data )
{
   GuiFormCtrl* ctrl = reinterpret_cast< GuiFormCtrl* >( object );
   ctrl->setHasMenu( dAtob( data ) );
   return false;
}

void GuiFormCtrl::initPersistFields()
{
   addField("caption",        TypeRealString, Offset(mCaption,        GuiFormCtrl));
   addField("contentLibrary",TypeString,     Offset(mContentLibrary, GuiFormCtrl));
   addField("content",       TypeString,     Offset(mContent,        GuiFormCtrl));
   addField("movable",        TypeBool,       Offset(mCanMove,        GuiFormCtrl));
   
   addProtectedField( "hasMenu", TypeBool,  Offset(mHasMenu, GuiFormCtrl),
      &_setHasMenu, &defaultProtectedGetFn,
      "" );

   Parent::initPersistFields();
}

void GuiFormCtrl::setHasMenu( bool value )
{
   if( mHasMenu == value )
      return;
      
   if( !value )
   {
      mMenuBar->deleteObject();
      mMenuBar = NULL;
   }
   else
   {
      if( !mMenuBar )
      {
         mMenuBar = new GuiMenuBar();
         mMenuBar->setField( "profile", "GuiFormMenuBarProfile" );
         mMenuBar->setField( "horizSizing", "right" );
         mMenuBar->setField( "vertSizing", "bottom" );
         mMenuBar->setField( "extent", "16 16" );
         mMenuBar->setField( "minExtent", "16 16" );
         mMenuBar->setField( "position", "0 0" );
         mMenuBar->setField( "class", "FormMenuBarClass "); // Give a generic class to the menu bar so that one set of functions may be used for all of them.

         mMenuBar->registerObject();
         mMenuBar->setProcessTicks(true); // Activate the processing of ticks to track if the mouse pointer has been hovering within the menu
      }
      
      addObject( mMenuBar ); // Add the menu bar to the form
   }
   
   mHasMenu = value;
}

bool GuiFormCtrl::onWake()
{
   if ( !Parent::onWake() )
      return false;

   mFont = mProfile->mFont;
   AssertFatal(mFont, "GuiFormCtrl::onWake: invalid font in profile" );

   mProfile->constructBitmapArray();

   if(mProfile->mUseBitmapArray && mProfile->mBitmapArrayRects.size())
   {
      mThumbSize.set(   mProfile->mBitmapArrayRects[0].extent.x, mProfile->mBitmapArrayRects[0].extent.y );
      mThumbSize.setMax( mProfile->mBitmapArrayRects[1].extent );

      if(mFont->getHeight() > mThumbSize.y)
         mThumbSize.y = mFont->getHeight();
   }
   else
   {
      mThumbSize.set(20, 20);
   }

   return true;
}


void GuiFormCtrl::addObject(SimObject *newObj )
{
   if( ( mHasMenu && size() > 1) || (!mHasMenu && size() > 0 ) )
   {
      Con::warnf("GuiFormCtrl::addObject - Forms may only have one *direct* child - Placing on Parent!");
      
      GuiControl* parent = getParent();
      if ( parent )
   	   parent->addObject( newObj );

      return;
   }

   GuiControl *newCtrl = dynamic_cast<GuiControl*>( newObj );
   GuiFormCtrl*formCtrl = dynamic_cast<GuiFormCtrl*>( newObj );
   
   if( newCtrl && formCtrl )
      newCtrl->setCanSave( true );
   else if ( newCtrl )
      newCtrl->setCanSave( false );

   Parent::addObject( newObj );
}

void GuiFormCtrl::removeObject( SimObject* object )
{
   if( object == mMenuBar )
   {
      mHasMenu = false;
      mMenuBar = NULL;
   }
   
   Parent::removeObject( object );
}

bool GuiFormCtrl::acceptsAsChild( SimObject* object ) const
{
   return Parent::acceptsAsChild( object ) &&
      ( ( mHasMenu && size() == 1 ) || ( !mHasMenu && !size() ) ); // Only accept a single child.
}

void GuiFormCtrl::onSleep()
{
   Parent::onSleep();
   mFont = NULL;
}

bool GuiFormCtrl::resize(const Point2I &newPosition, const Point2I &newExtent)
{
   if( !Parent::resize(newPosition, newExtent) ) 
      return false;

   if( !mAwake || !mProfile->mBitmapArrayRects.size() )
      return false;

   // Should the caption be modified because the title bar is too small?
   S32 textWidth = mProfile->mFont->getStrWidth(mCaption);
   S32 newTextArea = getWidth() - mThumbSize.x - mProfile->mBitmapArrayRects[4].extent.x;
   if(newTextArea < textWidth)
   {
      static char buf[256];

      mUseSmallCaption = true;
      mSmallCaption = StringTable->insert("");

      S32 strlen = dStrlen((const char*)mCaption);
      for(S32 i=strlen; i>=0; --i)
      {
         dStrcpy(buf, "");
         dStrncat(buf, (const char*)mCaption, i);
         dStrcat(buf, "...");

         textWidth = mProfile->mFont->getStrWidth(buf);

         if(textWidth < newTextArea)
         {
            mSmallCaption = StringTable->insert(buf, true);
            break;
         }
      }

   } else
   {
      mUseSmallCaption = false;
   }

   onResize_callback();

   return true;
}

void GuiFormCtrl::onRender(Point2I offset, const RectI &updateRect)
{
   // Fill in the control's child area
   RectI boundsRect(offset, getExtent());
   boundsRect.point.y += mThumbSize.y;
   boundsRect.extent.y -= mThumbSize.y;

   // draw the border of the form if specified
   if (mProfile->mOpaque)
      GFX->getDrawUtil()->drawRectFill(boundsRect, mProfile->mFillColor);

   if (mProfile->mBorder)
      renderBorder(boundsRect, mProfile);

   // If we don't have a child, put some text in the child area
   if( empty() )
   {
      GFX->getDrawUtil()->setBitmapModulation(ColorI(0,0,0));
      renderJustifiedText(boundsRect.point, boundsRect.extent, "[none]");
   }

   S32 textWidth = 0;

   // Draw our little bar, too
   if (mProfile->mBitmapArrayRects.size() >= 5)
   {
      GFX->getDrawUtil()->clearBitmapModulation();

      S32 barStart = offset.x + textWidth;
      S32 barTop   = mThumbSize.y / 2 + offset.y - mProfile->mBitmapArrayRects[3].extent.y / 2;

      Point2I barOffset(barStart, barTop);

      // Draw the start of the bar...
      GFX->getDrawUtil()->drawBitmapStretchSR(mProfile->mTextureObject ,RectI(barOffset, mProfile->mBitmapArrayRects[2].extent), mProfile->mBitmapArrayRects[2] );

      // Now draw the middle...
      barOffset.x += mProfile->mBitmapArrayRects[2].extent.x;

      S32 barMiddleSize = (getExtent().x - (barOffset.x - offset.x)) - mProfile->mBitmapArrayRects[4].extent.x + 1;

      if (barMiddleSize > 0)
      {
         // We have to do this inset to prevent nasty stretching artifacts
         RectI foo = mProfile->mBitmapArrayRects[3];
         foo.inset(1,0);

         GFX->getDrawUtil()->drawBitmapStretchSR(
            mProfile->mTextureObject,
            RectI(barOffset, Point2I(barMiddleSize, mProfile->mBitmapArrayRects[3].extent.y)),
            foo
            );
      }

      // And the end
      barOffset.x += barMiddleSize;

      GFX->getDrawUtil()->drawBitmapStretchSR( mProfile->mTextureObject, RectI(barOffset, mProfile->mBitmapArrayRects[4].extent),
         mProfile->mBitmapArrayRects[4]);

      GFX->getDrawUtil()->setBitmapModulation((mMouseOver ? mProfile->mFontColorHL : mProfile->mFontColor));
      renderJustifiedText(Point2I(mThumbSize.x, 0) + offset, Point2I(getWidth() - mThumbSize.x - mProfile->mBitmapArrayRects[4].extent.x, mThumbSize.y), (mUseSmallCaption ? mSmallCaption : mCaption) );

   }

   // Render the children
   renderChildControls(offset, updateRect);
}

void GuiFormCtrl::onMouseMove(const GuiEvent &event)
{
   Point2I localMove = globalToLocalCoord(event.mousePoint);

   // If we're clicking in the header then resize
   mMouseOver = (localMove.y < mThumbSize.y);
   if(isMouseLocked())
      mDepressed = mMouseOver;

}

void GuiFormCtrl::onMouseEnter(const GuiEvent &event)
{
   setUpdate();
   if(isMouseLocked())
   {
      mDepressed = true;
      mMouseOver = true;
   }
   else
   {
      mMouseOver = true;
   }

}

void GuiFormCtrl::onMouseLeave(const GuiEvent &event)
{
   setUpdate();
   if(isMouseLocked())
      mDepressed = false;
   mMouseOver = false;
}

void GuiFormCtrl::onMouseDown(const GuiEvent &event)
{
   Point2I localClick = globalToLocalCoord(event.mousePoint);

   // If we're clicking in the header then resize
   if(localClick.y < mThumbSize.y)
   {
      mouseLock();
      mDepressed = true;
      mMouseMovingWin = mCanMove;

      //update
      setUpdate();
   }

   mOrigBounds = getBounds();

   mMouseDownPosition = event.mousePoint;

   if (mMouseMovingWin )
   {
      mouseLock();
   }
   else
   {
      GuiControl *ctrl = findHitControl(localClick);
      if (ctrl && ctrl != this)
         ctrl->onMouseDown(event);
   }
}

void GuiFormCtrl::onMouseUp(const GuiEvent &event)
{
   // Make sure we only get events we ought to be getting...
   if (! mActive)
      return; 

   mouseUnlock();
   setUpdate();

   Point2I localClick = globalToLocalCoord(event.mousePoint);

   // If we're clicking in the header then resize
   //if(localClick.y < mThumbSize.y && mDepressed)
   //   setCollapsed(!mCollapsed);
}

DefineEngineMethod( GuiFormCtrl, getMenuID, S32, (),,
   "Get the ID of this form's menu.\n\n"
   "@return The ID of the form menu\n" )
{
   return object->getMenuBarID();
}

U32 GuiFormCtrl::getMenuBarID()
{
   return mMenuBar ? mMenuBar->getId() : 0;
}

DefineEngineMethod( GuiFormCtrl, setCaption, void, ( const char* caption ),,
   "Sets the title of the form.\n\n"
   "@param caption Form caption\n" )
{
   object->setCaption( caption );
}

#endif
