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

#ifndef _GuiProgressBitmapCtrl_H_
#define _GuiProgressBitmapCtrl_H_

#ifndef _GUICONTROL_H_
#include "gui/core/guiControl.h"
#endif

#ifndef _GUITEXTCTRL_H_
#include "gui/controls/guiTextCtrl.h"
#endif


//FIXME: WTH is this derived from GuiTextCtrl??  should be a GuiControl


/// A control that renders a horizontal progress bar from a repeating bitmap image.
class GuiProgressBitmapCtrl : public GuiTextCtrl
{
   public:
   
      typedef GuiTextCtrl Parent;
      
   protected:

      F32 mProgress;
      StringTableEntry mBitmapName;
      bool mUseVariable;
      bool mTile;
      S32 mNumberOfBitmaps;
      S32 mDim;
      
      static bool _setBitmap( void* object, const char* index, const char* data )
      {
         static_cast< GuiProgressBitmapCtrl* >( object )->setBitmap( data );
         return false;
      }

   public:
         
      GuiProgressBitmapCtrl();

      void setBitmap( const char* name );
      
      //console related methods
      virtual const char *getScriptValue();
      virtual void setScriptValue(const char *value);

      // GuiTextCtrl.
      virtual void onPreRender();
      virtual void onRender( Point2I offset, const RectI &updateRect );
      virtual bool onWake();

      DECLARE_CONOBJECT( GuiProgressBitmapCtrl );
      DECLARE_CATEGORY( "Gui Values" );
      DECLARE_DESCRIPTION( "A control that shows a horizontal progress bar that is rendered\n"
         "by repeating a bitmap." );

      static void initPersistFields();
};

#endif
