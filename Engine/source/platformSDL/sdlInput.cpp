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

#include "platform/platformInput.h"
#include "console/engineAPI.h"
#include "core/util/journal/process.h"
#include "windowManager/platformWindowMgr.h"

#include "sdlInput.h"
#include "platform/platformInput.h"
#include "SDL.h"

#ifdef LOG_INPUT
#include <time.h>
#include <stdarg.h>
#endif

// Static class variables:
InputManager*  Input::smManager;
bool           Input::smActive;
U16            Input::smModifierKeys;
bool           Input::smLastKeyboardActivated;
bool           Input::smLastMouseActivated;
bool           Input::smLastJoystickActivated;
InputEvent     Input::smInputEvent;

#ifdef LOG_INPUT
static HANDLE gInputLog;
#endif

//------------------------------------------------------------------------------
void Input::init()
{
   Con::printf( "Input Init:" );

   destroy();

   smActive = false;
   smLastKeyboardActivated = true;
   smLastMouseActivated = true;
   smLastJoystickActivated = true;

   SDL_InitSubSystem( SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS );

   // Init the current modifier keys
   setModifierKeys(0);
   Con::printf( "" );

   // Set ourselves to participate in per-frame processing.
   Process::notify(Input::process, PROCESS_INPUT_ORDER);

}

//------------------------------------------------------------------------------
DefineEngineFunction(isJoystickDetected, bool, (),, "")
{
   return(SDL_NumJoysticks() > 0);
}

//------------------------------------------------------------------------------
DefineEngineFunction(getJoystickAxes, const char*, (const char* instance), , "")
{
   // TODO SDL
   return("");
}

//------------------------------------------------------------------------------
void Input::destroy()
{
   Process::remove(Input::process);

   SDL_QuitSubSystem( SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC | SDL_INIT_GAMECONTROLLER );

}

//------------------------------------------------------------------------------
bool Input::enable()
{
   if ( smManager && !smManager->isEnabled() )
      return( smManager->enable() );

   return( false );
}

//------------------------------------------------------------------------------
void Input::disable()
{
   if ( smManager && smManager->isEnabled() )
      smManager->disable();
}

//------------------------------------------------------------------------------

void Input::activate()
{
#ifdef UNICODE
   //winState.imeHandle = ImmGetContext( getWin32WindowHandle() );
   //ImmReleaseContext( getWin32WindowHandle(), winState.imeHandle );
#endif

   if ( !Con::getBoolVariable( "$enableDirectInput" ) )
      return;

   if ( smManager && smManager->isEnabled() && !smActive )
   {
      Con::printf( "Activating Input..." );
      smActive = true;
   }
}

//------------------------------------------------------------------------------
void Input::deactivate()
{
   if ( smManager && smManager->isEnabled() && smActive )
   {
      smActive = false;
      Con::printf( "Input deactivated." );
   }
}

//------------------------------------------------------------------------------
bool Input::isEnabled()
{
   if ( smManager )
      return smManager->isEnabled();
   return false;
}

//------------------------------------------------------------------------------
bool Input::isActive()
{
   return smActive;
}

//------------------------------------------------------------------------------
void Input::process()
{
   if ( smManager && smManager->isEnabled() && smActive )
      smManager->process();
}

//------------------------------------------------------------------------------
InputManager* Input::getManager()
{
   return( smManager );
}

//-----------------------------------------------------------------------------
// Clipboard functions
const char* Platform::getClipboard()
{
   //note - this function never returns NULL
	return SDL_HasClipboardText() ? SDL_GetClipboardText() : "";
}

//-----------------------------------------------------------------------------
bool Platform::setClipboard(const char *text)
{
	if (!text)
		return false;

	SDL_SetClipboardText(text);

	return true;
}
