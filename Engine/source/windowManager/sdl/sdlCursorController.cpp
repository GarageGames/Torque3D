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

#include "core/strings/unicode.h"
#include "math/mMath.h"
#include "windowManager/sdl/sdlWindow.h"
#include "windowManager/sdl/sdlWindowMgr.h"
#include "windowManager/sdl/sdlCursorController.h"
#include "platform/platformInput.h"

#include "SDL.h"

static struct { U32 id; SDL_SystemCursor resourceID; SDL_Cursor *cursor;} sgCursorShapeMap[]=
{
   { PlatformCursorController::curArrow,       SDL_SYSTEM_CURSOR_ARROW  ,     NULL },
   { PlatformCursorController::curWait,        SDL_SYSTEM_CURSOR_WAIT,        NULL },
   { PlatformCursorController::curPlus,        SDL_SYSTEM_CURSOR_CROSSHAIR,   NULL },
   { PlatformCursorController::curResizeVert,  SDL_SYSTEM_CURSOR_SIZEWE,      NULL },
   { PlatformCursorController::curResizeHorz,  SDL_SYSTEM_CURSOR_SIZENS,      NULL },
   { PlatformCursorController::curResizeAll,   SDL_SYSTEM_CURSOR_SIZEALL,     NULL },
   { PlatformCursorController::curIBeam,       SDL_SYSTEM_CURSOR_IBEAM,       NULL },
   { PlatformCursorController::curResizeNESW,  SDL_SYSTEM_CURSOR_SIZENESW,    NULL },
   { PlatformCursorController::curResizeNWSE,  SDL_SYSTEM_CURSOR_SIZENWSE,    NULL },
   { PlatformCursorController::curHand,        SDL_SYSTEM_CURSOR_HAND,        NULL },
   { 0,                                        SDL_SYSTEM_CURSOR_NO,          NULL },
};


U32 PlatformCursorControllerSDL::getDoubleClickTime()
{
   // TODO SDL
   return 500;
}
S32 PlatformCursorControllerSDL::getDoubleClickWidth()
{
   // TODO SDL
   return 32;
}
S32 PlatformCursorControllerSDL::getDoubleClickHeight()
{
   // TODO SDL
   return 32;
}

void PlatformCursorControllerSDL::setCursorPosition( S32 x, S32 y )
{
   if( PlatformWindowManager::get() && PlatformWindowManager::get()->getFirstWindow() )
   {
      AssertFatal( dynamic_cast<PlatformWindowSDL*>( PlatformWindowManager::get()->getFirstWindow() ), "");
      PlatformWindowSDL *window = static_cast<PlatformWindowSDL*>( PlatformWindowManager::get()->getFirstWindow() );
      SDL_WarpMouseInWindow(window->getSDLWindow(), x, y);
   }
}

void PlatformCursorControllerSDL::getCursorPosition( Point2I &point )
{
   SDL_GetMouseState( &point.x, &point.y );
}

void PlatformCursorControllerSDL::setCursorVisible( bool visible )
{
   SDL_ShowCursor( visible );
}

bool PlatformCursorControllerSDL::isCursorVisible()
{
   return SDL_ShowCursor( -1 );;
}

void PlatformCursorControllerSDL::setCursorShape(U32 cursorID)
{
   SDL_Cursor* cursor = NULL;

   for(S32 i = 0; sgCursorShapeMap[i].resourceID != SDL_SYSTEM_CURSOR_NO; ++i)
   {
      if(cursorID == sgCursorShapeMap[i].id)
      {  
         if( !sgCursorShapeMap[i].cursor )
            sgCursorShapeMap[i].cursor = SDL_CreateSystemCursor( sgCursorShapeMap[i].resourceID );

         cursor = sgCursorShapeMap[i].cursor;
         break;
      }
   }

   if( !cursor )
      return;   
  
   SDL_SetCursor(cursor);
}


void PlatformCursorControllerSDL::setCursorShape( const UTF8 *fileName, bool reload )
{
   // TODO SDL
   AssertWarn(0, "PlatformCursorControllerSDL::setCursorShape - Not implemented");
}
