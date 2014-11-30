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

#include "gui/core/guiControl.h"
#include "console/consoleTypes.h"
#include "T3D/shapeBase.h"
#include "gfx/gfxDrawUtil.h"
#include "console/engineAPI.h"

//-----------------------------------------------------------------------------

/// Vary basic HUD clock.
/// Displays the current simulation time offset from some base. The base time
/// is usually synchronized with the server as mission start time.  This hud
/// currently only displays minutes:seconds.
class GuiClockHud : public GuiControl
{
   typedef GuiControl Parent;

   bool     mShowFrame;
   bool     mShowFill;
   bool     mTimeReversed;

   ColorF   mFillColor;
   ColorF   mFrameColor;
   ColorF   mTextColor;

   S32      mTimeOffset;
   
   // Copyright (C) 2013 WinterLeaf Entertainment LLC.
   //  @Copyright start

   /// Copy information
   ColorF mClockFillColorCopy;
   ColorF mClockFrameColorCopy;
   ColorF mClockTextColorCopy;

   void applyProfileSettings();

   void copyProfileSettings();

   void resetProfileSettings();

   // @Copyright end

public:
   GuiClockHud();

   void setTime(F32 newTime);
   void setReverseTime(F32 reverseTime);
   F32  getTime();
   void onStaticModified( const char *slotName, const char *newValue );    // Copyright (C) 2013 WinterLeaf Entertainment LLC.
   void onRender( Point2I, const RectI &);
   static void initPersistFields();
   DECLARE_CONOBJECT( GuiClockHud );
   DECLARE_CATEGORY( "Gui Game" );
   DECLARE_DESCRIPTION( "Basic HUD clock. Displays the current simulation time offset from some base." );
   
};
