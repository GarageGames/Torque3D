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

#include "console/console.h"
#include "console/consoleTypes.h"
#include "gfx/bitmap/gBitmap.h"
#include "gui/core/guiControl.h"
#include "gfx/gfxDevice.h"
#include "gfx/gfxTextureHandle.h"
#include "gfx/gfxDrawUtil.h"
#include "console/engineAPI.h"


class GuiChunkedBitmapCtrl : public GuiControl
{
private:
   typedef GuiControl Parent;
   void renderRegion(const Point2I &offset, const Point2I &extent);

protected:
   StringTableEntry mBitmapName;
   GFXTexHandle mTexHandle;
   bool  mUseVariable;
   bool  mTile;

public:
   //creation methods
   DECLARE_CONOBJECT(GuiChunkedBitmapCtrl);
   DECLARE_CATEGORY( "Gui Images" );
   
   GuiChunkedBitmapCtrl();
   static void initPersistFields();

   //Parental methods
   bool onWake();
   void onSleep();

   void setBitmap(const char *name);

   void onRender(Point2I offset, const RectI &updateRect);
};

IMPLEMENT_CONOBJECT(GuiChunkedBitmapCtrl);

ConsoleDocClass( GuiChunkedBitmapCtrl,
   "@brief This is a control that will render a specified bitmap or a bitmap specified in a referenced variable.\n\n"

   "This control allows you to either set a bitmap with the \"bitmap\" field or with the setBitmap method.  You can also choose "
   "to reference a variable in the \"variable\" field such as \"$image\" and then set \"useVariable\" to true.  This will cause it to "
   "synchronize the variable with the bitmap displayed (if the variable holds a valid image).  You can then change the variable and "
   "effectively changed the displayed image.\n\n"

   "@tsexample\n"
   "$image = \"anotherbackground.png\";\n"
   "new GuiChunkedBitmapCtrl(ChunkedBitmap)\n"
   "{\n"
   "   bitmap = \"background.png\";\n"
   "   variable = \"$image\";\n"
   "   useVariable = false;\n"
   "}\n\n"
   "// This will result in the control rendering \"background.png\"\n"
   "// If we now set the useVariable to true it will now render \"anotherbackground.png\"\n"
   "ChunkedBitmap.useVariable = true;\n"
   "@endtsexample\n\n"

   "@see GuiControl::variable\n\n"

   "@ingroup GuiImages\n"
);


void GuiChunkedBitmapCtrl::initPersistFields()
{
   addGroup("GuiChunkedBitmapCtrl");		
   addField( "bitmap",        TypeFilename,  Offset( mBitmapName, GuiChunkedBitmapCtrl ), "This is the bitmap to render to the control." );
   addField( "useVariable",   TypeBool,      Offset( mUseVariable, GuiChunkedBitmapCtrl ), "This decides whether to use the \"bitmap\" file "
	                                                                                      "or a bitmap stored in \"variable\"");
   addField( "tile",          TypeBool,      Offset( mTile, GuiChunkedBitmapCtrl ), "This is no longer in use");
   endGroup("GuiChunkedBitmapCtrl");
   Parent::initPersistFields();
}

DefineEngineMethod( GuiChunkedBitmapCtrl, setBitmap, void, (const char* filename),,
   "@brief Set the image rendered in this control.\n\n"
   "@param filename The image name you want to set\n"
   "@tsexample\n"
   "ChunkedBitmap.setBitmap(\"images/background.png\");"
   "@endtsexample\n\n")
{
   object->setBitmap( filename );
}

GuiChunkedBitmapCtrl::GuiChunkedBitmapCtrl()
{
   mBitmapName = StringTable->insert("");
   mUseVariable = false;
   mTile = false;
}

void GuiChunkedBitmapCtrl::setBitmap(const char *name)
{
   bool awake = mAwake;
   if(awake)
      onSleep();

   mBitmapName = StringTable->insert(name);
   if(awake)
      onWake();
   setUpdate();
}

bool GuiChunkedBitmapCtrl::onWake()
{
   if(!Parent::onWake())
      return false;

   if( !mTexHandle
       && ( ( mBitmapName && mBitmapName[ 0 ] )
            || ( mUseVariable && mConsoleVariable && mConsoleVariable[ 0 ] ) ) )
   {
      if ( mUseVariable )
         mTexHandle.set( Con::getVariable( mConsoleVariable ), &GFXDefaultGUIProfile, avar("%s() - mTexHandle (line %d)", __FUNCTION__, __LINE__) );
      else
         mTexHandle.set( mBitmapName, &GFXDefaultGUIProfile, avar("%s() - mTexHandle (line %d)", __FUNCTION__, __LINE__) );
   }

   return true;
}

void GuiChunkedBitmapCtrl::onSleep()
{
   mTexHandle = NULL;
   Parent::onSleep();
}

void GuiChunkedBitmapCtrl::renderRegion(const Point2I &offset, const Point2I &extent)
{
/*
   U32 widthCount = mTexHandle.getTextureCountWidth();
   U32 heightCount = mTexHandle.getTextureCountHeight();
   if(!widthCount || !heightCount)
      return;

   F32 widthScale = F32(extent.x) / F32(mTexHandle.getWidth());
   F32 heightScale = F32(extent.y) / F32(mTexHandle.getHeight());
   GFX->setBitmapModulation(ColorF(1,1,1));
   for(U32 i = 0; i < widthCount; i++)
   {
      for(U32 j = 0; j < heightCount; j++)
      {
         GFXTexHandle t = mTexHandle.getSubTexture(i, j);
         RectI stretchRegion;
         stretchRegion.point.x = (S32)(i * 256 * widthScale  + offset.x);
         stretchRegion.point.y = (S32)(j * 256 * heightScale + offset.y);
         if(i == widthCount - 1)
            stretchRegion.extent.x = extent.x + offset.x - stretchRegion.point.x;
         else
            stretchRegion.extent.x = (S32)((i * 256 + t.getWidth() ) * widthScale  + offset.x - stretchRegion.point.x);
         if(j == heightCount - 1)
            stretchRegion.extent.y = extent.y + offset.y - stretchRegion.point.y;
         else
            stretchRegion.extent.y = (S32)((j * 256 + t.getHeight()) * heightScale + offset.y - stretchRegion.point.y);
         GFX->drawBitmapStretch(t, stretchRegion);
      }
   }
*/
}


void GuiChunkedBitmapCtrl::onRender(Point2I offset, const RectI &updateRect)
{

   if( mTexHandle )
   {
      RectI boundsRect( offset, getExtent());
      GFX->getDrawUtil()->drawBitmapStretch( mTexHandle, boundsRect, GFXBitmapFlip_None, GFXTextureFilterLinear );
   }

   renderChildControls(offset, updateRect);
}
