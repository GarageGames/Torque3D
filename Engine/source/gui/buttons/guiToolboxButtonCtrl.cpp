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
#include "gui/buttons/guiToolboxButtonCtrl.h"

#include "console/console.h"
#include "console/engineAPI.h"
#include "gfx/gfxDevice.h"
#include "gfx/gfxDrawUtil.h"
#include "console/consoleTypes.h"
#include "gui/core/guiCanvas.h"
#include "gui/core/guiDefaultControlRender.h"


IMPLEMENT_CONOBJECT(GuiToolboxButtonCtrl);

ConsoleDocClass( GuiToolboxButtonCtrl,
   "@brief Unimplemented GUI control meant to interact with Toolbox.\n\n"
   "For Torque 3D editors only, soon to be deprecated\n\n"
   "@internal"
);

//-------------------------------------
GuiToolboxButtonCtrl::GuiToolboxButtonCtrl()
{
   mNormalBitmapName = StringTable->EmptyString();
   mLoweredBitmapName = StringTable->insert("sceneeditor/client/images/buttondown");
   mHoverBitmapName = StringTable->insert("sceneeditor/client/images/buttonup");
   setMinExtent(Point2I(16,16));
   setExtent(48, 48);
   mButtonType = ButtonTypeRadio;
   mTipHoverTime = 100;
   
}


//-------------------------------------
void GuiToolboxButtonCtrl::initPersistFields()
{
   addField("normalBitmap", TypeFilename, Offset(mNormalBitmapName, GuiToolboxButtonCtrl));
   addField("loweredBitmap", TypeFilename, Offset(mLoweredBitmapName, GuiToolboxButtonCtrl));
   addField("hoverBitmap", TypeFilename, Offset(mHoverBitmapName, GuiToolboxButtonCtrl));
   Parent::initPersistFields();
}


//-------------------------------------
bool GuiToolboxButtonCtrl::onWake()
{
   if (! Parent::onWake())
      return false;

   setActive( true );
   
   setNormalBitmap( mNormalBitmapName );
   setLoweredBitmap( mLoweredBitmapName );
   setHoverBitmap( mHoverBitmapName );

   return true;
}


//-------------------------------------
void GuiToolboxButtonCtrl::onSleep()
{
   mTextureNormal = NULL;
   mTextureLowered = NULL;
   mTextureHover = NULL;
   Parent::onSleep();
}


//-------------------------------------

DefineConsoleMethod( GuiToolboxButtonCtrl, setNormalBitmap, void, ( const char * name ), , "( filepath name ) sets the bitmap that shows when the button is active")
{
   object->setNormalBitmap(name);
}

DefineConsoleMethod( GuiToolboxButtonCtrl, setLoweredBitmap, void, ( const char * name ), , "( filepath name ) sets the bitmap that shows when the button is disabled")
{
   object->setLoweredBitmap(name);
}

DefineConsoleMethod( GuiToolboxButtonCtrl, setHoverBitmap, void, ( const char * name ), , "( filepath name ) sets the bitmap that shows when the button is disabled")
{
   object->setHoverBitmap(name);
}

//-------------------------------------
void GuiToolboxButtonCtrl::inspectPostApply()
{
   // if the extent is set to (0,0) in the gui editor and appy hit, this control will
   // set it's extent to be exactly the size of the normal bitmap (if present)
   Parent::inspectPostApply();

   if ((getWidth() == 0) && (getHeight() == 0) && mTextureNormal)
   {
      setExtent( mTextureNormal->getWidth(), mTextureNormal->getHeight());
   }
}


//-------------------------------------
void GuiToolboxButtonCtrl::setNormalBitmap( StringTableEntry bitmapName )
{
   mNormalBitmapName = StringTable->insert( bitmapName );
   
   if(!isAwake())
      return;

   if ( *mNormalBitmapName )
      mTextureNormal = GFXTexHandle( mNormalBitmapName, &GFXDefaultPersistentProfile, avar("%s() - mTextureNormal (line %d)", __FUNCTION__, __LINE__) );
   else
      mTextureNormal = NULL;
   
   setUpdate();
}   

void GuiToolboxButtonCtrl::setLoweredBitmap( StringTableEntry bitmapName )
{
   mLoweredBitmapName = StringTable->insert( bitmapName );
   
   if(!isAwake())
      return;

   if ( *mLoweredBitmapName )
      mTextureLowered = GFXTexHandle( mLoweredBitmapName, &GFXDefaultPersistentProfile, avar("%s() - mTextureLowered (line %d)", __FUNCTION__, __LINE__) );
   else
      mTextureLowered = NULL;
   
   setUpdate();
}   

void GuiToolboxButtonCtrl::setHoverBitmap( StringTableEntry bitmapName )
{
   mHoverBitmapName = StringTable->insert( bitmapName );

   if(!isAwake())
      return;

   if ( *mHoverBitmapName )
      mTextureHover = GFXTexHandle( mHoverBitmapName, &GFXDefaultPersistentProfile, avar("%s() - mTextureHover (line %d)", __FUNCTION__, __LINE__) );
   else
      mTextureHover = NULL;

   setUpdate();
}   



//-------------------------------------
void GuiToolboxButtonCtrl::onRender(Point2I offset, const RectI& updateRect)
{
   // Only render the state rect (hover/down) if we're active
   if (mActive)
   {
      RectI r(offset, getExtent());
      if ( mDepressed  || mStateOn )
         renderStateRect( mTextureLowered , r );
      else if ( mMouseOver )
         renderStateRect( mTextureHover , r );
   }

   // Now render the image
   if( mTextureNormal )
   {
      renderButton( mTextureNormal, offset, updateRect );
      return;
   }

   Point2I textPos = offset;
   if( mDepressed )
      textPos += Point2I(1,1);

   // Make sure we take the profile's textOffset into account.
   textPos += mProfile->mTextOffset;

   GFX->getDrawUtil()->setBitmapModulation( mProfile->mFontColor );
   renderJustifiedText(textPos, getExtent(), mButtonText);

}

void GuiToolboxButtonCtrl::renderStateRect( GFXTexHandle &texture, const RectI& rect )
{
   if (texture)
   {
      GFX->getDrawUtil()->clearBitmapModulation();
      GFX->getDrawUtil()->drawBitmapStretch( texture, rect );
   }
}

//------------------------------------------------------------------------------

void GuiToolboxButtonCtrl::renderButton(GFXTexHandle &texture, Point2I &offset, const RectI& updateRect)
{
   if (texture)
   {
      Point2I finalOffset = offset;

      finalOffset.x += ( ( getWidth() / 2 ) - ( texture.getWidth() / 2 ) );
      finalOffset.y += ( ( getHeight() / 2 ) - ( texture.getHeight() / 2 ) );

      GFX->getDrawUtil()->clearBitmapModulation();
      GFX->getDrawUtil()->drawBitmap(texture, finalOffset);
      renderChildControls( offset, updateRect);
   }
}
