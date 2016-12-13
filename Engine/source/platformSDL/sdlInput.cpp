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
#include "console/console.h"
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
U8             Input::smModifierKeys;
bool           Input::smLastKeyboardActivated;
bool           Input::smLastMouseActivated;
bool           Input::smLastJoystickActivated;
InputEvent     Input::smInputEvent;

#ifdef LOG_INPUT
static HANDLE gInputLog;
#endif

static void fillAsciiTable() {}

//------------------------------------------------------------------------------
//
// This function gets the standard ASCII code corresponding to our key code
// and the existing modifier key state.
//
//------------------------------------------------------------------------------
struct AsciiData
{
   struct KeyData
   {
      U16   ascii;
      bool  isDeadChar;
   };

   KeyData upper;
   KeyData lower;
   KeyData goofy;
};


#define NUM_KEYS ( KEY_OEM_102 + 1 )
#define KEY_FIRST KEY_ESCAPE

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
   fillAsciiTable();
   Con::printf( "" );

   // Set ourselves to participate in per-frame processing.
   Process::notify(Input::process, PROCESS_INPUT_ORDER);

}

//------------------------------------------------------------------------------
ConsoleFunction( isJoystickDetected, bool, 1, 1, "isJoystickDetected()" )
{
   return( SDL_NumJoysticks() > 0 );
}

//------------------------------------------------------------------------------
ConsoleFunction( getJoystickAxes, const char*, 2, 2, "getJoystickAxes( instance )" )
{
   // TODO SDL
   return( "" );
}

//------------------------------------------------------------------------------
U16 Input::getKeyCode( U16 asciiCode )
{
    if( asciiCode > 255 )
        return 0;

    char c[2];
    c[0]= asciiCode;
    c[1] = NULL;
    return KeyMapSDL::getTorqueScanCodeFromSDL( SDL_GetScancodeFromName( c ) );
}

//------------------------------------------------------------------------------
U16 Input::getAscii( U16 keyCode, KEY_STATE keyState )
{
   if ( keyCode >= NUM_KEYS )
      return 0;

   U32 SDLKey = KeyMapSDL::getSDLScanCodeFromTorque( keyCode );
   SDLKey = SDL_GetKeyFromScancode( (SDL_Scancode)SDLKey );

   const char *text = SDL_GetKeyName( SDLKey );
   if(text[1] != 0)
      return 0;
   U8 ret = text[0];

   if( !dIsalpha(ret) )
      return ret;

   switch ( keyState )
   {
      case STATE_LOWER:
         return dTolower( ret );
      case STATE_UPPER:
         return dToupper( ret );
      case STATE_GOOFY:
         return 0; // TODO SDL
      default:
         return(0);

   }
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


namespace
{
   const int TableSize = 256;
   U32 SDL_T3D[256];
   U32 T3D_SDL[256];
   static bool _buildScanCode = true;
}

void mapScanCode(U32 sdl, U32 torque)
{
   SDL_T3D[sdl] = torque;
   T3D_SDL[torque] = sdl;
}



void buildScanCodeArray()
{
   _buildScanCode = false;

   for(int i = 0; i < TableSize; ++i)
   {
      SDL_T3D[i] = 0;
      T3D_SDL[i] = 0;
   }

   // SDL, Torque
   mapScanCode(SDL_SCANCODE_A, KEY_A);
   mapScanCode(SDL_SCANCODE_B, KEY_B);
   mapScanCode(SDL_SCANCODE_C, KEY_C);
   mapScanCode(SDL_SCANCODE_D, KEY_D);
   mapScanCode(SDL_SCANCODE_E, KEY_E);
   mapScanCode(SDL_SCANCODE_F, KEY_F);
   mapScanCode(SDL_SCANCODE_G, KEY_G);
   mapScanCode(SDL_SCANCODE_H, KEY_H);
   mapScanCode(SDL_SCANCODE_I, KEY_I);
   mapScanCode(SDL_SCANCODE_J, KEY_J);
   mapScanCode(SDL_SCANCODE_K, KEY_K);
   mapScanCode(SDL_SCANCODE_L, KEY_L);
   mapScanCode(SDL_SCANCODE_M, KEY_M);
   mapScanCode(SDL_SCANCODE_N, KEY_N);
   mapScanCode(SDL_SCANCODE_O, KEY_O);
   mapScanCode(SDL_SCANCODE_P, KEY_P);
   mapScanCode(SDL_SCANCODE_Q, KEY_Q);
   mapScanCode(SDL_SCANCODE_R, KEY_R);
   mapScanCode(SDL_SCANCODE_S, KEY_S);
   mapScanCode(SDL_SCANCODE_T, KEY_T);
   mapScanCode(SDL_SCANCODE_U, KEY_U);
   mapScanCode(SDL_SCANCODE_V, KEY_V);
   mapScanCode(SDL_SCANCODE_W, KEY_W);
   mapScanCode(SDL_SCANCODE_X, KEY_X);
   mapScanCode(SDL_SCANCODE_Y, KEY_Y);
   mapScanCode(SDL_SCANCODE_Z, KEY_Z);

   mapScanCode(SDL_SCANCODE_1, KEY_1);
   mapScanCode(SDL_SCANCODE_2, KEY_2);
   mapScanCode(SDL_SCANCODE_3, KEY_3);
   mapScanCode(SDL_SCANCODE_4, KEY_4);
   mapScanCode(SDL_SCANCODE_5, KEY_5);
   mapScanCode(SDL_SCANCODE_6, KEY_6);
   mapScanCode(SDL_SCANCODE_7, KEY_7);
   mapScanCode(SDL_SCANCODE_8, KEY_8);
   mapScanCode(SDL_SCANCODE_9, KEY_9);
   mapScanCode(SDL_SCANCODE_0, KEY_0);

   mapScanCode(SDL_SCANCODE_BACKSPACE, KEY_BACKSPACE);
   mapScanCode(SDL_SCANCODE_TAB, KEY_TAB);
   mapScanCode(SDL_SCANCODE_RETURN, KEY_RETURN);
   mapScanCode(SDL_SCANCODE_LCTRL, KEY_CONTROL);
   mapScanCode(SDL_SCANCODE_RCTRL, KEY_CONTROL);
   mapScanCode(SDL_SCANCODE_LALT, KEY_ALT);
   mapScanCode(SDL_SCANCODE_RALT, KEY_ALT);
   mapScanCode(SDL_SCANCODE_LSHIFT, KEY_SHIFT);
   mapScanCode(SDL_SCANCODE_RSHIFT, KEY_SHIFT);
   mapScanCode(SDL_SCANCODE_PAUSE, KEY_PAUSE);
   mapScanCode(SDL_SCANCODE_CAPSLOCK, KEY_CAPSLOCK);
   mapScanCode(SDL_SCANCODE_ESCAPE, KEY_ESCAPE);
   mapScanCode(SDL_SCANCODE_SPACE, KEY_SPACE);
   mapScanCode(SDL_SCANCODE_PAGEDOWN, KEY_PAGE_DOWN);
   mapScanCode(SDL_SCANCODE_PAGEUP, KEY_PAGE_UP);
   mapScanCode(SDL_SCANCODE_END, KEY_END);
   mapScanCode(SDL_SCANCODE_HOME, KEY_HOME);
   mapScanCode(SDL_SCANCODE_LEFT, KEY_LEFT);
   mapScanCode(SDL_SCANCODE_UP, KEY_UP);
   mapScanCode(SDL_SCANCODE_RIGHT, KEY_RIGHT);
   mapScanCode(SDL_SCANCODE_DOWN, KEY_DOWN);
   mapScanCode(SDL_SCANCODE_PRINTSCREEN, KEY_PRINT);
   mapScanCode(SDL_SCANCODE_INSERT, KEY_INSERT);
   mapScanCode(SDL_SCANCODE_DELETE, KEY_DELETE);
   mapScanCode(SDL_SCANCODE_HELP, KEY_HELP);

   mapScanCode(SDL_SCANCODE_GRAVE, KEY_TILDE);
   mapScanCode(SDL_SCANCODE_MINUS, KEY_MINUS);
   mapScanCode(SDL_SCANCODE_EQUALS, KEY_EQUALS);
   mapScanCode(SDL_SCANCODE_LEFTBRACKET, KEY_LBRACKET);
   mapScanCode(SDL_SCANCODE_RIGHTBRACKET, KEY_RBRACKET);
   mapScanCode(SDL_SCANCODE_BACKSLASH, KEY_BACKSLASH);
   mapScanCode(SDL_SCANCODE_SEMICOLON, KEY_SEMICOLON);
   mapScanCode(SDL_SCANCODE_APOSTROPHE, KEY_APOSTROPHE);
   mapScanCode(SDL_SCANCODE_COMMA, KEY_COMMA);
   mapScanCode(SDL_SCANCODE_PERIOD, KEY_PERIOD);
   mapScanCode(SDL_SCANCODE_SLASH, KEY_SLASH);
   mapScanCode(SDL_SCANCODE_KP_0, KEY_NUMPAD0);
   mapScanCode(SDL_SCANCODE_KP_1, KEY_NUMPAD1);
   mapScanCode(SDL_SCANCODE_KP_2, KEY_NUMPAD2);
   mapScanCode(SDL_SCANCODE_KP_3, KEY_NUMPAD3);
   mapScanCode(SDL_SCANCODE_KP_4, KEY_NUMPAD4);
   mapScanCode(SDL_SCANCODE_KP_5, KEY_NUMPAD5);
   mapScanCode(SDL_SCANCODE_KP_6, KEY_NUMPAD6);
   mapScanCode(SDL_SCANCODE_KP_7, KEY_NUMPAD7);
   mapScanCode(SDL_SCANCODE_KP_8, KEY_NUMPAD8);
   mapScanCode(SDL_SCANCODE_KP_9, KEY_NUMPAD9);
   mapScanCode(SDL_SCANCODE_KP_MULTIPLY, KEY_MULTIPLY);
   mapScanCode(SDL_SCANCODE_KP_PLUS, KEY_ADD);
   mapScanCode(SDL_SCANCODE_KP_EQUALS, KEY_SEPARATOR);
   mapScanCode(SDL_SCANCODE_KP_MINUS, KEY_SUBTRACT);
   mapScanCode(SDL_SCANCODE_KP_PERIOD, KEY_DECIMAL);
   mapScanCode(SDL_SCANCODE_KP_DIVIDE, KEY_DIVIDE);
   mapScanCode(SDL_SCANCODE_KP_ENTER, KEY_NUMPADENTER);

   mapScanCode(SDL_SCANCODE_F1, KEY_F1);
   mapScanCode(SDL_SCANCODE_F2, KEY_F2);
   mapScanCode(SDL_SCANCODE_F3, KEY_F3);
   mapScanCode(SDL_SCANCODE_F4, KEY_F4);
   mapScanCode(SDL_SCANCODE_F5, KEY_F5);
   mapScanCode(SDL_SCANCODE_F6, KEY_F6);
   mapScanCode(SDL_SCANCODE_F7, KEY_F7);
   mapScanCode(SDL_SCANCODE_F8, KEY_F8);
   mapScanCode(SDL_SCANCODE_F9, KEY_F9);
   mapScanCode(SDL_SCANCODE_F10, KEY_F10);
   mapScanCode(SDL_SCANCODE_F11, KEY_F11);
   mapScanCode(SDL_SCANCODE_F12, KEY_F12);
   mapScanCode(SDL_SCANCODE_F13, KEY_F13);
   mapScanCode(SDL_SCANCODE_F14, KEY_F14);
   mapScanCode(SDL_SCANCODE_F15, KEY_F15);
   mapScanCode(SDL_SCANCODE_F16, KEY_F16);
   mapScanCode(SDL_SCANCODE_F17, KEY_F17);
   mapScanCode(SDL_SCANCODE_F18, KEY_F18);
   mapScanCode(SDL_SCANCODE_F19, KEY_F19);
   mapScanCode(SDL_SCANCODE_F20, KEY_F20);
   mapScanCode(SDL_SCANCODE_F21, KEY_F21);
   mapScanCode(SDL_SCANCODE_F22, KEY_F22);
   mapScanCode(SDL_SCANCODE_F23, KEY_F23);
   mapScanCode(SDL_SCANCODE_F24, KEY_F24);

   //mapScanCode(SDL_SCANCODE_LOCKINGNUMLOCK, KEY_NUMLOCK);
   //mapScanCode(SDL_SCANCODE_LOCKINGSCROLLLOCK, KEY_SCROLLLOCK);
   mapScanCode(SDL_SCANCODE_LCTRL, KEY_LCONTROL);
   mapScanCode(SDL_SCANCODE_RCTRL, KEY_RCONTROL);
   mapScanCode(SDL_SCANCODE_LALT, KEY_LALT);
   mapScanCode(SDL_SCANCODE_RALT, KEY_RALT);
   mapScanCode(SDL_SCANCODE_LSHIFT, KEY_LSHIFT);
   mapScanCode(SDL_SCANCODE_RSHIFT, KEY_RSHIFT);
   //mapScanCode(____, KEY_WIN_LWINDOW);
   //mapScanCode(____, KEY_WIN_RWINDOW);
   //mapScanCode(____, KEY_WIN_APPS);
   //mapScanCode(____, KEY_OEM_102);

   //mapScanCode(____, KEY_MAC_OPT);
   //mapScanCode(____, KEY_MAC_LOPT);
   //mapScanCode(____, KEY_MAC_ROPT);

   //for(int i = 0; i < 48; ++i)
   //   mapScanCode(____, KEY_BUTTON0 + i );

   //mapScanCode(____, KEY_ANYKEY);   
}

U32 KeyMapSDL::getTorqueScanCodeFromSDL(U32 sdl)
{
   if(_buildScanCode)
      buildScanCodeArray();

   return SDL_T3D[sdl];
}

U32 KeyMapSDL::getSDLScanCodeFromTorque(U32 torque)
{
   if(_buildScanCode)
      buildScanCodeArray();

   return T3D_SDL[torque];
}