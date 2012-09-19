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

#ifndef _GUICONSOLE_H_
#define _GUICONSOLE_H_

#ifndef _GUIARRAYCTRL_H_
#include "gui/core/guiArrayCtrl.h"
#endif

#ifndef _CONSOLE_LOGGER_H_
#include "console/consoleLogger.h"
#endif


class GuiConsole : public GuiArrayCtrl
{
   private:
      typedef GuiArrayCtrl Parent;

      Resource<GFont> mFont;

      S32 getMaxWidth(S32 startIndex, S32 endIndex);

   protected:

      /// @name Callbacks
      /// @{

      DECLARE_CALLBACK( void, onMessageSelected, ( ConsoleLogEntry::Level level, const char* message ) );

      /// @}

      // GuiArrayCtrl.
      virtual void onCellSelected( Point2I cell );

   public:
      GuiConsole();
      DECLARE_CONOBJECT(GuiConsole);
      DECLARE_CATEGORY( "Gui Editor" );
      DECLARE_DESCRIPTION( "Control that displays the console log text." );

      // GuiArrayCtrl.
      virtual bool onWake();
      virtual void onPreRender();
      virtual void onRenderCell(Point2I offset, Point2I cell, bool selected, bool mouseOver);
};

#endif
