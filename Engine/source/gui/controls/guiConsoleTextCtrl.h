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

#ifndef _GUICONSOLETEXTCTRL_H_
#define _GUICONSOLETEXTCTRL_H_

#ifndef _GFONT_H_
#include "gfx/gFont.h"
#endif
#ifndef _GUITYPES_H_
#include "gui/core/guiTypes.h"
#endif
#ifndef _GUICONTROL_H_
#include "gui/core/guiControl.h"
#endif

class GuiConsoleTextCtrl : public GuiControl
{
private:
   typedef GuiControl Parent;

public:
   enum Constants { MAX_STRING_LENGTH = 255 };


protected:

   String mConsoleExpression;
   String mResult;
   Resource<GFont> mFont;

   Vector<U32> mStartLineOffset;
   Vector<U32> mLineLen;

public:

   //creation methods
   DECLARE_CONOBJECT(GuiConsoleTextCtrl);
   DECLARE_CATEGORY( "Gui Editor" );
   
   GuiConsoleTextCtrl();
   virtual ~GuiConsoleTextCtrl();
   static void initPersistFields();

   //Parental methods
   bool onWake();
   void onSleep();

   //text methods
   virtual void setText( const char *txt = NULL );
   const char* getText() { return mConsoleExpression.c_str(); }

   //rendering methods
   void calcResize();
   void onPreRender();    // do special pre render processing
   void onRender( Point2I offset, const RectI &updateRect );

   //Console methods
   const char* getScriptValue();
   void setScriptValue( const char *value );
};

#endif //_GUI_TEXT_CONTROL_H_
