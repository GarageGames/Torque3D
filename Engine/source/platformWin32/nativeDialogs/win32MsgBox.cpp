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
#include "platformWin32/platformWin32.h" 
#include "platform/platformInput.h"
#include "platform/nativeDialogs/msgBox.h"

#include "console/console.h"
#include "core/strings/unicode.h"

#include "windowManager/platformWindowMgr.h"
#include "windowManager/win32/win32Window.h"

struct _FlagMap
{
   S32 num;
   U32 flag;
};

static _FlagMap sgButtonMap[] =
{
   { MBOk,                 MB_OK },
   { MBOkCancel,           MB_OKCANCEL },
   { MBRetryCancel,        MB_RETRYCANCEL },
   { MBSaveDontSave,       MB_YESNO },
   { MBSaveDontSaveCancel, MB_YESNOCANCEL },
   { 0xffffffff,           0xffffffff }
};

static _FlagMap sgIconMap[] =
{
   { MIWarning,            MB_ICONWARNING },
   { MIInformation,        MB_ICONINFORMATION },
   { MIQuestion,           MB_ICONQUESTION },
   { MIStop,               MB_ICONSTOP },
   { 0xffffffff,           0xffffffff }
};

static _FlagMap sgMsgBoxRetMap[] =
{
   { IDCANCEL,             MRCancel },
   { IDNO,                 MRDontSave },
   { IDOK,                 MROk},
   { IDRETRY,              MRRetry },
   { IDYES,                MROk },
   { 0xffffffff,           0xffffffff }
};

//-----------------------------------------------------------------------------

static U32 getMaskFromID(_FlagMap *map, S32 id)
{
   for(S32 i = 0;map[i].num != 0xffffffff && map[i].flag != 0xffffffff;++i)
   {
      if(map[i].num == id)
         return map[i].flag;
   }

   return 0;
}

//-----------------------------------------------------------------------------
#ifndef TORQUE_SDL
S32 Platform::messageBox(const UTF8 *title, const UTF8 *message, MBButtons buttons, MBIcons icon)
{
   PlatformWindow *pWindow = WindowManager->getFirstWindow();

   // Get us rendering while we're blocking.
   winState.renderThreadBlocked = true;

   // We don't keep a locked mouse or else we're going 
   // to end up possibly locking our mouse out of the 
   // message box area
   bool cursorLocked = pWindow && pWindow->isMouseLocked();
   if( cursorLocked )
      pWindow->setMouseLocked( false );

   // Need a visible cursor to click stuff accurately
   bool cursorVisible = !pWindow || pWindow->isCursorVisible();
   if( !cursorVisible )
      pWindow->setCursorVisible(true);

#ifdef UNICODE
   const UTF16 *msg = createUTF16string(message);
   const UTF16 *t = createUTF16string(title);
#else
   const UTF8 *msg = message;
   const UTF8 *t = title;
#endif

   HWND parent = pWindow ? static_cast<Win32Window*>(pWindow)->getHWND() : NULL;
   S32 ret = ::MessageBox( parent, msg, t, getMaskFromID(sgButtonMap, buttons) | getMaskFromID(sgIconMap, icon));

#ifdef UNICODE
   delete [] msg;
   delete [] t;
#endif

   // Dialog is gone.
   winState.renderThreadBlocked = false;

   if( cursorVisible == false )
      pWindow->setCursorVisible( false );

   if( cursorLocked == true )
      pWindow->setMouseLocked( true );

   return getMaskFromID(sgMsgBoxRetMap, ret);
}
#endif
