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

#ifndef _EDITOR_H_
#define _EDITOR_H_

#ifndef _MMATRIX_H_
#include "math/mMatrix.h"
#endif
#ifndef _GUICONTROL_H_
#include "gui/core/guiControl.h"
#endif

class GameBase;

//------------------------------------------------------------------------------

class EditManager : public GuiControl
{
   private:
      typedef GuiControl Parent;

   public:
      EditManager();
      ~EditManager();

      bool onWake();
      void onSleep();

      // SimObject
      bool onAdd();

      /// Perform the onEditorEnabled callback on all SimObjects
      /// and set gEditingMission true.
      void editorEnabled();

      /// Perform the onEditorDisabled callback on all SimObjects
      /// and set gEditingMission false.
      void editorDisabled();

      MatrixF mBookmarks[10];
      DECLARE_CONOBJECT(EditManager);
      DECLARE_CATEGORY( "Gui Editor" );
};

extern bool gEditingMission;

//------------------------------------------------------------------------------

#endif
