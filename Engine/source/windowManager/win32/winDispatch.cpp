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

#define NO_MINMAX
#define WIN32_LEAN_AND_MEAN

// This is a fix for mouse wheel support on 
// older versions of VC++.
#if _MSC_VER < 1500
#define _WIN32_WINNT 0x0400
#endif

#include <windows.h>

#include "platform/platformInput.h"
#include "windowManager/win32/winDispatch.h"
#include "windowManager/win32/win32Window.h"
#include "windowManager/win32/win32CursorController.h"
#include "platformWin32/winDirectInput.h"
#include "core/util/journal/process.h"
#include "core/util/journal/journaledSignal.h"

static U32 _ModifierKeys=0;
static BYTE keyboardState[256];
static bool initKBState = false;
static bool sgDoubleByteEnabled = false;

// is keyboard input a standard (non-changing) VK keycode
#define dIsStandardVK(c) (((0x08 <= (c)) && ((c) <= 0x12)) || \
	((c) == 0x1b) ||                    \
	((0x20 <= (c)) && ((c) <= 0x2e)) || \
	((0x30 <= (c)) && ((c) <= 0x39)) || \
	((0x41 <= (c)) && ((c) <= 0x5a)) || \
	((0x70 <= (c)) && ((c) <= 0x7B)))

extern InputObjectInstances DIK_to_Key( U8 dikCode );

extern U8 TranslateOSKeyCode(U8 vcode );

extern InputModifiers convertModifierBits(const U32 in);

static void _keyboardEvent(Win32Window* window,UINT message, WPARAM wParam, WPARAM lParam)
{
	if(!initKBState)
	{
		dMemset(keyboardState, 0, sizeof(keyboardState));
		initKBState = true;
	}

	// Extract windows key info:
	// S32 repeatCount = (lParam & 0xffff);
	U32 scanCode    = (lParam >> 16) & 0xff;
	bool extended   = lParam & (1 << 24);       // Enhanced keyboard key
	bool previous   = lParam & (1 << 30);       // Previously down
	bool make       = (message == WM_KEYDOWN || message == WM_SYSKEYDOWN);

	// Translate the OS virtual key code to a Torque KEY_XXXX.   
	S32 nVirtkey = TranslateOSKeyCode( wParam );

	S32 keyCode;
	if ( wParam == VK_PROCESSKEY && sgDoubleByteEnabled )
		keyCode = MapVirtualKey( scanCode, 1 );   // This is the REAL virtual key...
	else
		keyCode = wParam;

	// Convert alt/shift/ctrl to left or right variant if needed.
	S32 newVirtKey = nVirtkey;
	switch(nVirtkey)
	{
	case KEY_ALT:
		newVirtKey = extended ? KEY_RALT : KEY_LALT;
		break;
	case KEY_CONTROL:
		newVirtKey = extended ? KEY_RCONTROL : KEY_LCONTROL;
		break;
	case KEY_SHIFT:
		newVirtKey = (scanCode == 54) ? KEY_RSHIFT : KEY_LSHIFT;
		break;
	case KEY_RETURN:
		if ( extended )
			newVirtKey = KEY_NUMPADENTER;
		break;
	}

	// Track modifier keys
	U32 modifier = 0;
	switch (newVirtKey) 
	{
	case KEY_LALT:     modifier = IM_LALT;   break;
	case KEY_RALT:     modifier = IM_RALT;   break;
	case KEY_LSHIFT:   modifier = IM_LSHIFT; break;
	case KEY_RSHIFT:   modifier = IM_RSHIFT; break;
	case KEY_LCONTROL: modifier = IM_LCTRL;  break;
	case KEY_RCONTROL: modifier = IM_RCTRL;  break;
	}

	if (make)
	{
		_ModifierKeys |= modifier;
		keyboardState[keyCode] |= 0x80;
	}
	else
	{
		_ModifierKeys &= ~modifier;
		keyboardState[keyCode] &= 0x7f;
	}

   U32 torqueMods = convertModifierBits( _ModifierKeys );
	Input::setModifierKeys( torqueMods );
	
	// If character event translation is active and this isn't a key
	// mapped in the global action map, try converting the event into
	// a character event first.
	
	if(    make
       && window->getKeyboardTranslation()
	    && !window->shouldNotTranslate( torqueMods, newVirtKey ) )
	{
      U16 chars[ 64 ];
      dMemset( chars, 0, sizeof( chars ) );

      S32 res = ToUnicode( keyCode, scanCode, keyboardState, chars, sizeof( chars ) / sizeof( chars[ 0 ] ), 0 );

   	// This should only happen on Window 9x/ME systems
   	if( res == 0 )
   		res = ToAscii( keyCode, scanCode, keyboardState, chars, 0 );

      if( res >= 1 )
      {
      	// Post chars, but filter them to not be control codes... this is a bit hacky.

         bool handledCharEvent = false;
      	for( S32 i=0; i< res; i ++ )
      		if( chars[i] >= 32)
            {
      			window->charEvent.trigger(window->getWindowId(),_ModifierKeys,chars[i]);
               handledCharEvent = true;
            }
      		
         if( handledCharEvent )
            return;
      }
	}

	// Produce a key event.
	
	U32 action = make ? (previous ? IA_REPEAT : IA_MAKE ) : IA_BREAK;   
	window->keyEvent.trigger(window->getWindowId(),_ModifierKeys,action,newVirtKey);
}


//-----------------------------------------------------------------------------

static bool _dispatch(HWND hWnd,UINT message,WPARAM wParam,WPARAM lParam)
{
	static bool button[3] = {false,false,false};
	static S32 mouseNCState = -1; // -1 denotes unchanged, 
	// 0  denotes changed but was hidden
	// 1  denotes changed but was visible
	Win32Window* window = hWnd?(Win32Window*)GetWindowLong(hWnd, GWL_USERDATA): 0;
	const WindowId devId = window ? window->getWindowId() : 0;

	// State tracking for focus/lose focus cursor management
	static bool cursorLocked = false;
	static bool cursorVisible = true;

	switch(message) 
	{
	case WM_MOUSEMOVE:
		{
			// Skip it if we have no window!
			if (!window || !window->getCursorController())
				break;


			PlatformCursorController *pController = window->getCursorController();

			// If we're locked and unfocused, ignore it.
			if(window->shouldLockMouse() && !window->isFocused())
				break;

			// If the mouse was shown to accommodate a NC mouse move
			//  we need to change it back to what it was
			if( mouseNCState != -1 )
			{
				pController->setCursorVisible( mouseNCState );
				mouseNCState = -1; // reset to unchanged
			}

			// Let the cursor manager update the native cursor.
			pController->refreshCursor();

			// Grab the mouse pos so we can modify it.
			S32 mouseX = S16(LOWORD(lParam));
			S32 mouseY = S16(HIWORD(lParam));

			// Ensure mouse lock when appropriate
			window->setMouseLocked( window->shouldLockMouse() );

			// Are we locked?
			if(window->isMouseLocked())
			{
				// Always invisible when locked.
				if( window->isCursorVisible() )
					window->setCursorVisible( false );

				RECT r;
				GetWindowRect(window->getHWND(), &r);

				// See Win32Window::setMouseLocked for explanation
				RECT rCopy = r;
				rCopy.top  += 32; rCopy.bottom -= 64;
				rCopy.left += 32; rCopy.right  -= 64;
				ClipCursor(&rCopy);

				// Recenter the mouse if necessary (don't flood the message pump)
				Point2I curPos;
				pController->getCursorPosition( curPos );

				const S32 centerX = (r.right + r.left) / 2;
				const S32 centerY = (r.bottom + r.top) / 2;

				if( curPos.x != centerX || curPos.y != centerY )
					pController->setCursorPosition(centerX, centerY);

				// Convert the incoming client pos into a screen pos, so we can
				// accurately convert to relative coordinates.
				POINT mousePos;
				mousePos.x = mouseX;
				mousePos.y = mouseY;

				ClientToScreen(window->getHWND(), &mousePos);

				// Now we can calculate the position relative to the center we set.
				mouseX = mousePos.x - centerX;
				mouseY = mousePos.y - centerY;
			}
			else
			{
				// Probably don't need to call this all the time but better
				// safe than sorry...
				ClipCursor(NULL);
			}

			window->mouseEvent.trigger(devId,_ModifierKeys,mouseX,mouseY,window->isMouseLocked());
			break;
		}

		// We want to show the system cursor whenever we leave
		// our window, and it'd be simple, except for one problem:
		// showcursor isn't a toggle.  so, keep hammering it until
		// the cursor is *actually* going to be shown.
	case WM_NCMOUSEMOVE:
		{
			if( window )
			{
				mouseNCState = ( window->isCursorVisible() ? 1 : 0);
				window->setCursorVisible( true );
			}

			break;
		}

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN: {
		int index = (message - WM_LBUTTONDOWN) / 3;
		button[index] = true;

		// Capture the mouse on button down to allow dragging outside
		// of the window boundary.
		if (GetCapture() != hWnd)
			SetCapture(hWnd);

		if (window)
			window->buttonEvent.trigger(devId,_ModifierKeys,IA_MAKE,index);
		break;
						 }

	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP: {
		int index = (message - WM_LBUTTONUP) / 3;
		button[index] = false;

		// Release mouse capture from button down.
		if (!button[0] && !button[1] && !button[2])
			ReleaseCapture();

		if (window)
			window->buttonEvent.trigger(devId,_ModifierKeys,IA_BREAK,index);
		break;
					   }

	case WM_MOUSEWHEEL: // Vertical wheel.
		if (window)
			window->wheelEvent.trigger(devId,_ModifierKeys,0,GET_WHEEL_DELTA_WPARAM(wParam));
		break;

#ifdef WM_MOUSEHWHEEL // Vista
   case WM_MOUSEHWHEEL: // Horizontal wheel.
      if( window )
         window->wheelEvent.trigger( devId, _ModifierKeys, GET_WHEEL_DELTA_WPARAM( wParam ), 0 );
      break;
#endif

	case WM_KEYUP:
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		if (window)
			_keyboardEvent(window,message,wParam,lParam);
		break;

		// NOTE: if wParam is NOT equal to our window handle then we are GAINING focus
	case WM_SETFOCUS:

		// clear any key states 
		_ModifierKeys = 0;
		dMemset(keyboardState, 0, 256);
		Input::setModifierKeys(0);

		// We must have a window present; otherwise there's nothing further
		// we can do about this event.
		if (window && window->getHWND() != (HWND)wParam)
		{
			if (cursorVisible == false)
				window->setCursorVisible(false);

			if (cursorLocked == true)
				window->setMouseLocked(true);

			// Update window state.
			window->setBackground(false);

			// Fire event.
			window->appEvent.trigger(devId, GainFocus);

			if (!Input::isActive())
				Input::activate();
		}
		break;

		// NOTE: if wParam is NOT equal to our window handle then we are LOSING focus
	case WM_KILLFOCUS:

		// clear any key states 
		_ModifierKeys = 0;
		dMemset(keyboardState, 0, 256);
		Input::setModifierKeys(0);

		// We must have a window present; otherwise there's nothing further
		// we can do about this event.
		if (window && window->getHWND() != (HWND)wParam)
		{
			HWND hwnd = (HWND)wParam;
			UTF16 classBuf[256];

			if (hwnd)
				GetClassName(hwnd, classBuf, sizeof(classBuf));

			// We toggle the mouse lock when we become inactive 
			// causing the subsequent lock call to defer itself
			// until the window becomes active again.
			if (window && window->isMouseLocked())
			{
				window->setMouseLocked( false );
				window->setMouseLocked( true );
			}

			// FIXME [tom, 5/1/2007] Hard coding this is lame since there's a const in win32Window.cpp
			// CodeReview - this fails if there is a second jug app in the arena.
			if (hwnd == NULL || dStrcmp(classBuf, L"TorqueJuggernaughtWindow") != 0)
			{
				// We are being made inactive and the window being made active isn't
				// a jugg window. Thus, we need to deactivate input.
				if (Input::isActive())
					Input::deactivate();
			}

			cursorVisible = window->isCursorVisible();
			if (!cursorVisible)
				window->setCursorVisible(true);

			cursorLocked = window->isMouseLocked();
			if (cursorLocked)
				window->setMouseLocked(false);

			// Update window state.
			window->setBackground(true);

			// Fire event.
			window->appEvent.trigger(devId, LoseFocus);
		}
		break;      

	case WM_ACTIVATEAPP:
		if (wParam) 
		{
			// Could extract current modifier state from windows.
			_ModifierKeys = 0;
			Input::setModifierKeys(_ModifierKeys);
		}
		break;

	case WM_CLOSE:
		if (window)
			window->appEvent.trigger(devId,WindowClose);

		// Force a quit if we're in play mode, otherwise there would be
		// no way to stop a journal playback.(
		if (Journal::IsPlaying())
			Process::requestShutdown();
		break;

	case WM_TIMER: {
		if (window)
			window->appEvent.trigger(devId,Timer);
		break;
				   }

	case WM_DESTROY:{
		// Only people who care about this currently are web plugins, because
		// everyone else will just handle the WM_CLOSE app event.
		if(window)
			window->appEvent.trigger(devId,WindowDestroy);
		break;
					}

	case WM_QUIT: {
		// Quit indicates that we're not going to receive anymore Win32 messages.
		// Therefore, it's appropriate to flag our event loop for exit as well,
		// since we won't be getting any more messages.
		Process::requestShutdown();
		break;
				  }

				  // CodeReview - This is not used now and will incur an overhead for rendering 
				  //              since the renderThreadBlocked fix requires handling WM_PAINT and
				  //              triggering the displayEvent.  May need to revisit this at a later
				  //              time if we want event driven rendering.
				  //case WM_PAINT: {
				  //   // Checking for isOpen will keep us from generating an event
				  //   // during the window creation process, which can cause problems
				  //   // with the journaling.
				  //   if (window && window->isOpen() && !winState.renderThreadBlocked )
				  //      window->displayEvent.trigger(devId);
				  //}
	case WM_SIZE: {
		if (window && wParam != SIZE_MINIMIZED && !Journal::IsPlaying())
		{
			window->resizeEvent.trigger(window->getWindowId(), LOWORD(lParam),HIWORD(lParam));

			// Consume all existing mouse events and those posted to our own dispatch queue
			MSG msg;
			PeekMessage( &msg, 0,WM_MOUSEFIRST,WM_MOUSELAST , PM_QS_POSTMESSAGE | PM_NOYIELD | PM_REMOVE );
			RemoveMessages( NULL, WM_MOUSEMOVE, WM_MOUSEMOVE );

			if( window->isMouseLocked())
			{
				RECT r;
				GetWindowRect(window->getHWND(), &r);

				S32 centerX = (r.right + r.left) >> 1;
				S32 centerY = ((r.bottom + r.top) >> 1);
				window->setCursorPosition( centerX, centerY );

				// Set the CursorPos
				SetCursorPos(centerX, centerY);
			}
		}

				  }

	}
	return true;
}


//-----------------------------------------------------------------------------

// Structure used to store Windows events for delayed dispatching
struct WinMessageQueue
{
public:
	struct Message {
		HWND hWnd;
		UINT message;
		WPARAM wparam;
		WPARAM lparam;
	};

	WinMessageQueue()
	{
		VECTOR_SET_ASSOCIATION( _messageList );
	}
	bool isEmpty() {
		return !_messageList.size();
	}
	void post(HWND hWnd,UINT message,WPARAM wparam,WPARAM lparam) {
		Message msg;
		msg.hWnd = hWnd;
		msg.message = message;
		msg.wparam = wparam;
		msg.lparam = lparam;
		_messageList.push_back(msg);
	}
	bool next(Message* msg) {
		if (!_messageList.size())
			return false;
		*msg = _messageList.first();
		_messageList.pop_front();
		return true;
	}
	void remove(HWND hWnd, UINT msgBegin = -1, UINT msgEnd = -1) {
		for (S32 i = 0; i < _messageList.size(); i++)
		{
			// Match Window
			if( hWnd != NULL && _messageList[i].hWnd == hWnd)
				_messageList.erase_fast(i--);
			else if( msgBegin != -1 && msgEnd != -1 )
			{
				// CodeReview - Match Message Range [6/30/2007 justind]
				//
				// Word of caution : Use this only if you know what you're doing.
				//  I cannot be responsible for you blowing your leg off destroying
				//  a bunch of messages you didn't intend to if you specify a ridiculous
				//  range of messages values.
				//
				// To filter a single message, pass the message as the begin and end.
				if( _messageList[i].message >= msgBegin && _messageList[i].message <= msgEnd )
					_messageList.erase_fast(i--);
			}
		}
	}

private:
	Vector<Message> _messageList;
};

static WinMessageQueue _MessageQueue;


void RemoveMessages(HWND hWnd,UINT msgBegin,WPARAM msgEnd )
{
	_MessageQueue.remove( hWnd, msgBegin, msgEnd );
}

// Dispatch the window event, or queue up for later
void Dispatch(DispatchType type,HWND hWnd,UINT message,WPARAM wparam,WPARAM lparam)
{
	// If the message queue is not empty, then we'll need to delay
	// this dispatch in order to preserve message order.
	if (type == DelayedDispatch || !_MessageQueue.isEmpty())
		_MessageQueue.post(hWnd,message,wparam,lparam);
	else
		_dispatch(hWnd,message,wparam,lparam);
}

// Dispatch next even in the queue
bool DispatchNext()
{
	WinMessageQueue::Message msg;
	if (!_MessageQueue.next(&msg))
		return false;
	_dispatch(msg.hWnd,msg.message,msg.wparam,msg.lparam);
	return true;
}

// Remove events from the queue
void DispatchRemove(HWND hWnd)
{
	_MessageQueue.remove(hWnd);
}
