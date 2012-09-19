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

#include <windows.h>
#include <tchar.h>
#include "core/strings/unicode.h"
#include "math/mMath.h"
#include "windowManager/win32/win32Window.h"
#include "windowManager/win32/win32WindowMgr.h"
#include "windowManager/win32/winDispatch.h"
#include "windowManager/win32/win32CursorController.h"
#include "platform/platformInput.h"
#include <zmouse.h>

static struct { U32 id; LPTSTR resourceID; } sgCursorShapeMap[]=
{
   { PlatformCursorController::curArrow,       IDC_ARROW },
   { PlatformCursorController::curWait,        IDC_WAIT },
   { PlatformCursorController::curPlus,        IDC_CROSS },
   { PlatformCursorController::curResizeVert,  IDC_SIZEWE },
   { PlatformCursorController::curResizeHorz,  IDC_SIZENS },
   { PlatformCursorController::curResizeAll,   IDC_SIZEALL },
   { PlatformCursorController::curIBeam,       IDC_IBEAM },
   { PlatformCursorController::curResizeNESW,  IDC_SIZENESW },
   { PlatformCursorController::curResizeNWSE,  IDC_SIZENWSE },
   { PlatformCursorController::curHand,        IDC_HAND },
   { 0,                             0 },
};

//static const EnumTable::Enums curManagerShapesEnums[] = 
//{
//   { Win32CursorController::curArrow, "Arrow" },
//   { Win32CursorController::curWait, "Wait" },
//   { Win32CursorController::curPlus, "Plus" },
//   { Win32CursorController::curResizeVert, "ResizeVert" },
//   { Win32CursorController::curResizeHorz, "ResizeHorz" },
//   { Win32CursorController::curResizeAll, "ResizeAll" },
//   { Win32CursorController::curIBeam, "ibeam" },
//   { Win32CursorController::curResizeNESW, "ResizeNESW" },
//   { Win32CursorController::curResizeNWSE, "ResizeNWSE" },
//};
//
//static const EnumTable gCurManagerShapesTable(8, &curManagerShapesEnums[0]); 

// CodeReview I've duplicated this 'cache' trick for system settings
// because they're unlikely to change and calling into the OS for values
// repeatedly is just silly to begin with.  [6/29/2007 justind]
U32 Win32CursorController::getDoubleClickTime()
{
   static S32 sPlatWinDoubleClicktime = -1;
   if( sPlatWinDoubleClicktime == -1 )
      sPlatWinDoubleClicktime = GetDoubleClickTime();
   return sPlatWinDoubleClicktime;
}
S32 Win32CursorController::getDoubleClickWidth()
{
   static S32 sPlatWinDoubleClickwidth = -1;
   if( sPlatWinDoubleClickwidth == -1 )
      sPlatWinDoubleClickwidth = GetSystemMetrics(SM_CXDOUBLECLK);
   return sPlatWinDoubleClickwidth;
}
S32 Win32CursorController::getDoubleClickHeight()
{
   static S32 sPlatWinDoubleClickheight = -1;
   if( sPlatWinDoubleClickheight == -1 )
      sPlatWinDoubleClickheight = GetSystemMetrics(SM_CYDOUBLECLK);
   return sPlatWinDoubleClickheight;
}

void Win32CursorController::setCursorPosition( S32 x, S32 y )
{
   ::SetCursorPos(x, y);
}

void Win32CursorController::getCursorPosition( Point2I &point )
{
   POINT rPoint;
   ::GetCursorPos( &rPoint );

   // Return 
   point.x = rPoint.x;
   point.y = rPoint.y;
}

void Win32CursorController::setCursorVisible( bool visible )
{
   if( visible )
      ShowCursor( true );
   else
      while( ShowCursor(false) > 0 );
}

bool Win32CursorController::isCursorVisible()
{
   CURSORINFO rCursorInfo;
   rCursorInfo.cbSize = sizeof(CURSORINFO);
   if( !GetCursorInfo( &rCursorInfo ) )
   {
      //DWORD error = GetLastError();
      return false;
   }

   // rCursorInfo.flags values :
   // 0 == Cursor is hidden
   // CURSOR_SHOWING == cursor is visible
   return (bool)(rCursorInfo.flags == CURSOR_SHOWING);
}

void Win32CursorController::setCursorShape(U32 cursorID)
{
   LPTSTR resourceID = NULL;

   for(S32 i = 0;sgCursorShapeMap[i].resourceID != NULL;++i)
   {
      if(cursorID == sgCursorShapeMap[i].id)
      {
         resourceID = sgCursorShapeMap[i].resourceID;
         break;
      }
   }

   if(resourceID == NULL)
      return;

   HCURSOR cur = LoadCursor(NULL, resourceID);
   if(cur)
      SetCursor(cur);
}

static HCURSOR gCursorShape = NULL;
void Win32CursorController::setCursorShape( const UTF8 *fileName, bool reload )
{
#ifdef UNICODE
   const UTF16 *lFileName = convertUTF8toUTF16( fileName );
#else
   const UTF8  *lFileName = fileName;
#endif

   if ( !gCursorShape || reload )
      gCursorShape = LoadCursorFromFile( lFileName );
   
   if ( gCursorShape )
      SetCursor( gCursorShape );
}

// Console function to set the current cursor shape given the cursor shape
// name as defined in the enum above.
//ConsoleFunction( inputPushCursor, void, 2, 2, "inputPushCursor(cursor shape name)" )
//{
//   S32 val = 0;
//
//   // Find the cursor shape
//   if(argc == 2)
//   {
//      for (S32 i = 0; i < gCurManagerShapesTable.size; i++)
//      {
//         if (! dStricmp(argv[1], gCurManagerShapesTable.table[i].label))
//         {
//            val = gCurManagerShapesTable.table[i].index;
//            break;
//         }
//      }
//   }
//
//   // Now set it
//   Win32CursorController* cm = Input::getCursorManager();
//   if(cm)
//   {
//      cm->pushCursor(val);
//   }
//}
//// Function to pop the current cursor shape
//ConsoleFunction( inputPopCursor, void, 1, 1, "inputPopCursor()" )
//{
//   argc;
//   argv;
//
//   Win32CursorController* cm = Input::getCursorManager();
//   if(cm)
//   {
//      cm->popCursor();
//   }
//}
