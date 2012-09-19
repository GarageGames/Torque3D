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

#ifndef _GUITOOLBOXBUTTON_H_
#define _GUITOOLBOXBUTTON_H_

#ifndef _GUIBUTTONCTRL_H_
#include "gui/buttons/guiButtonCtrl.h"
#endif
#ifndef GFX_Texture_Manager_H_
#include "gfx/gfxTextureManager.h"
#endif

class GuiToolboxButtonCtrl : public GuiButtonCtrl
{
private:
   typedef GuiButtonCtrl Parent;

protected:
   StringTableEntry mNormalBitmapName;
   StringTableEntry mLoweredBitmapName;
   StringTableEntry mHoverBitmapName;

   GFXTexHandle mTextureNormal;
   GFXTexHandle mTextureLowered;
   GFXTexHandle mTextureHover;

   void renderButton(GFXTexHandle &texture, Point2I &offset, const RectI& updateRect);
   void renderStateRect( GFXTexHandle &texture, const RectI& rect );

public:   
   DECLARE_CONOBJECT(GuiToolboxButtonCtrl);
   GuiToolboxButtonCtrl();

   static void initPersistFields();

   //Parent methods
   bool onWake();
   void onSleep();
   void inspectPostApply();

   void setNormalBitmap( StringTableEntry bitmapName );
   void setLoweredBitmap( StringTableEntry bitmapName );
   void setHoverBitmap( StringTableEntry bitmapName );
   

   void onRender(Point2I offset, const RectI &updateRect);
};


#endif //_GUITOOLBOXBUTTON_H_
