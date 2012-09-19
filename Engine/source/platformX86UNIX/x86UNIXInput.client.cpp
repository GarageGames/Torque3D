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

#include "platformX86UNIX/platformX86UNIX.h"
#include "platform/platformInput.h"
#include "platform/platformVideo.h"
#include "platform/event.h"
#include "platform/gameInterface.h"
#include "console/console.h"
#include "platformX86UNIX/x86UNIXState.h"
#include "platformX86UNIX/x86UNIXInputManager.h"

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

#include <SDL/SDL.h>

#ifdef LOG_INPUT
#include <time.h>
#include <stdarg.h>
#include <fcntl.h>
#include <platformX86UNIX/x86UNIXUtils.h>

extern int x86UNIXOpen(const char *path, int oflag);
extern int x86UNIXClose(int fd);
extern ssize_t x86UNIXWrite(int fd, const void *buf, size_t nbytes);
#endif

class XClipboard
{
   private:
      Atom mClipboardProperty;
      Atom mClipboard;
      Atom mPrimary;
      bool mInitialized;
      U8 *mXData;
      char *mTData;
      S32 mTDataSize;

      void init();
      void freeXData();
      void freeTData();
      void checkTDataSize(S32 requestedSize);
   public:
      XClipboard();
      ~XClipboard();

      bool setClipboard(const char *text);
      const char* getClipboard();
      void handleSelectionRequest(XSelectionRequestEvent& request);
};

// Static class variables:
InputManager*  Input::smManager;

// smActive is not maintained under unix.  Use Input::isActive()
// instead
bool           Input::smActive = false;

// unix platform state
extern x86UNIXPlatformState * x86UNIXState;

extern AsciiData AsciiTable[NUM_KEYS];

static XClipboard xclipboard;

#ifdef LOG_INPUT
S32 gInputLog = -1;
#endif 

//------------------------------------------------------------------------------
void Input::init()
{
   Con::printf( "Input Init:" );

   destroy();

#ifdef LOG_INPUT
   struct tm* newTime;
   time_t aclock;
   time( &aclock );
   newTime = localtime( &aclock );
   asctime( newTime );

   gInputLog = x86UNIXOpen("input.log", O_WRONLY | O_CREAT);
   log("Input log opened at %s\n", asctime( newTime ) );
   log("Operating System:\n" );
   log("  %s", UUtils->getOSName());
   log("\n");
#endif

   smActive = false;
   smManager = NULL;

   UInputManager *uInputManager = new UInputManager;
   if ( !uInputManager->enable() )
   {
      Con::errorf( "   Failed to enable Input Manager." );
      delete uInputManager;
      return;
   }

   uInputManager->init();

   smManager = uInputManager;

   Con::printf("   Input initialized");
   Con::printf(" ");
}

//------------------------------------------------------------------------------
ConsoleFunction( isJoystickDetected, bool, 1, 1, "isJoystickDetected()" )
{
   argc; argv;
   UInputManager* manager = dynamic_cast<UInputManager*>(Input::getManager());
   if (manager)
      return manager->joystickDetected();
   else
      return false;
}

//------------------------------------------------------------------------------
ConsoleFunction( getJoystickAxes, const char*, 2, 2, "getJoystickAxes( instance )" )
{
   argc; argv;
   UInputManager* manager = dynamic_cast<UInputManager*>(Input::getManager());
   if (manager)
      return manager->getJoystickAxesString(dAtoi(argv[1]));
   else
      return "";
}

//------------------------------------------------------------------------------
U16 Input::getKeyCode( U16 asciiCode )
{
   U16 keyCode = 0;
   U16 i;
   
   // This is done three times so the lowerkey will always
   // be found first. Some foreign keyboards have duplicate
   // chars on some keys.
   for ( i = KEY_FIRST; i < NUM_KEYS && !keyCode; i++ )
   {
      if ( AsciiTable[i].lower.ascii == asciiCode )
      {
         keyCode = i;
         break;
      };
   }

   for ( i = KEY_FIRST; i < NUM_KEYS && !keyCode; i++ )
   {
      if ( AsciiTable[i].upper.ascii == asciiCode )
      {
         keyCode = i;
         break;
      };
   }

   for ( i = KEY_FIRST; i < NUM_KEYS && !keyCode; i++ )
   {
      if ( AsciiTable[i].goofy.ascii == asciiCode )
      {
         keyCode = i;
         break;
      };
   }

   return( keyCode );
}

//-----------------------------------------------------------------------------
//
// This function gets the standard ASCII code corresponding to our key code
// and the existing modifier key state.
//
//-----------------------------------------------------------------------------
U16 Input::getAscii( U16 keyCode, KEY_STATE keyState )
{
   if ( keyCode >= NUM_KEYS )
      return 0;

   switch ( keyState )
   {
      case STATE_LOWER:
         return AsciiTable[keyCode].lower.ascii;
      case STATE_UPPER:
         return AsciiTable[keyCode].upper.ascii;
      case STATE_GOOFY:
         return AsciiTable[keyCode].goofy.ascii;
      default:
         return(0);
            
   }
}

//------------------------------------------------------------------------------
void Input::destroy()
{   
#ifdef LOG_INPUT
   if ( gInputLog != -1 )
   {
      log( "*** CLOSING LOG ***\n" );
      x86UNIXClose(gInputLog);
      gInputLog = -1;
   }
#endif

   if ( smManager && smManager->isEnabled() )
   {
      smManager->disable();
      delete smManager;
      smManager = NULL;
   }
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
   if ( smManager && smManager->isEnabled() && !isActive())
   {
#ifdef LOG_INPUT
      Input::log( "Activating Input...\n" );
#endif
      UInputManager* uInputManager = dynamic_cast<UInputManager*>( smManager );
      if ( uInputManager )
         uInputManager->activate();
   }
}

//------------------------------------------------------------------------------
void Input::deactivate()
{
   if ( smManager && smManager->isEnabled() && isActive() )
   {
#ifdef LOG_INPUT
      Input::log( "Deactivating Input...\n" );
#endif
      UInputManager* uInputManager = dynamic_cast<UInputManager*>( smManager );
      if ( uInputManager )
         uInputManager->deactivate();
   }
}

//------------------------------------------------------------------------------
void Input::reactivate()
{
   Input::deactivate();
   Input::activate();
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
   UInputManager* uInputManager = dynamic_cast<UInputManager*>( smManager );
   if ( uInputManager )
      return uInputManager->isActive();
   return false;
}

//------------------------------------------------------------------------------
void Input::process()
{
   if ( smManager )
      smManager->process();
}

//------------------------------------------------------------------------------
InputManager* Input::getManager()
{
   return smManager;
}

#ifdef LOG_INPUT
//------------------------------------------------------------------------------
void Input::log( const char* format, ... )
{
   if ( gInputLog == -1)
      return;
   
   va_list argptr;
   va_start( argptr, format );

   const int BufSize = 4096;
   char buffer[BufSize];
   dVsprintf( buffer, BufSize, format, argptr );
   x86UNIXWrite(gInputLog, buffer, dStrlen( buffer ));
   va_end( argptr );
}

ConsoleFunction( inputLog, void, 2, 2, "inputLog( string )" )
{
   argc;
   Input::log( "%s\n", argv[1] );
}
#endif // LOG_INPUT

//------------------------------------------------------------------------------
void NotifySelectionEvent(XEvent& event)
{
   // somebody sent us a select event
   if (event.type == SelectionRequest)
      xclipboard.handleSelectionRequest(event.xselectionrequest);
}

//------------------------------------------------------------------------------
const char* Platform::getClipboard()
{
   return xclipboard.getClipboard();
}

//------------------------------------------------------------------------------
bool Platform::setClipboard(const char *text)
{
   return xclipboard.setClipboard(text);
}

//-----------------------------------------------------------------------------
// XClipboard members
XClipboard::XClipboard()
{
   mInitialized = false;
}

//------------------------------------------------------------------------------
XClipboard::~XClipboard()
{
   freeXData();
   freeTData();
}

//------------------------------------------------------------------------------
void XClipboard::init()
{
   DisplayPtrManager xdisplay;
   Display* display = xdisplay.getDisplayPointer();

   mClipboardProperty = XInternAtom(display, 
      "TORQUE_CLIPBOARD_ATOM", False);
   mClipboard = XInternAtom(display, "CLIPBOARD", 
      False);
   mPrimary = XA_PRIMARY; //XInternAtom(display, "PRIMARY", False);
   mXData = NULL;
   mTData = NULL;
   mTDataSize = 0;

   mInitialized = true;
}

//------------------------------------------------------------------------------
inline void XClipboard::freeXData()
{
   if (mXData != NULL)
   {
      XFree(mXData);
      mXData = NULL;
   }
}

//------------------------------------------------------------------------------
inline void XClipboard::freeTData()
{
   if (mTData != NULL)
   {
      dRealFree(mTData);
      mTData = NULL;
      mTDataSize = 0;
   }
}

//
// JMQ: As you might expect, X clipboard usage is bizarre.  I 
// found this document to be useful.
//
// http://www.freedesktop.org/standards/clipboards.txt 
//
// JMQ: later note: programming the X clipboard is not just
// bizarre, it SUCKS.  No wonder so many apps have
// clipboard problems.
//
//------------------------------------------------------------------------------
const char* XClipboard::getClipboard()
{
   DisplayPtrManager xdisplay;
   Display* display = xdisplay.getDisplayPointer();

   if (!mInitialized)
      init();

   // find the owner of the clipboard
   Atom targetSelection = mClipboard;
   Window clipOwner = XGetSelectionOwner(display, 
      targetSelection);
   if (clipOwner == None)
   {
      // It seems like KDE/QT reads the clipboard but doesn't set it.
      // This is a bug, that supposedly will be fixed in QT3.
      // I tried working around this by using
      // PRIMARY instead of CLIPBOARD, but this has some nonintuitive
      // side effects.  So, no pasting from KDE apps for now.
      //targetSelection = mPrimary;
      //clipOwner = XGetSelectionOwner(display, targetSelection);
   }

   if (clipOwner == None)
      // oh well
      return "";

   // request that the owner convert the selection to a string
   XConvertSelection(display, targetSelection, 
      XA_STRING, mClipboardProperty, x86UNIXState->getWindow(), CurrentTime);

   // flush the output buffer to make sure the selection request event gets 
   // sent now
   XFlush(display);

   XEvent xevent;

   // if our window is the current owner, (e.g. copy from one part of
   // torque and paste to another), then we just sent an event to our
   // window that won't get processed until we get back to the event
   // loop in x86Unixwindow.  So look for selection request events in
   // the event queue immediately and handle them.
   while (XCheckTypedWindowEvent(display, 
             x86UNIXState->getWindow(), SelectionRequest, &xevent))
      handleSelectionRequest(xevent.xselectionrequest);
  
   // poll for the SelectionNotify event for 5 seconds.  in most cases 
   // we should get the event very quickly
   U32 startTime = Platform::getRealMilliseconds();
   bool timeOut = false;
   while (!XCheckTypedWindowEvent(display, 
             x86UNIXState->getWindow(), SelectionNotify, &xevent) &&
      !timeOut)
   {
      // we'll be spinning here, but who cares
      if ((Platform::getRealMilliseconds() - startTime) > 5000)
         timeOut = true;
   }

   if (timeOut)
   {
      Con::warnf(ConsoleLogEntry::General, 
         "XClipboard: waited too long for owner to convert selection");
      return "";
   }

   if (xevent.xselection.property == None)
      return "";

   // free the X data from a previous get
   freeXData();

   // grab the string data from the property 
   Atom actual_type;
   int actual_format;
   unsigned long bytes_after;
   unsigned long nitems;
   // query the property length the 250000 is "the length in 32-bit
   // multiples of the data to be retrieved".  so we support up to a
   // million bytes of returned data.
   int numToRetrieve = 250000;
   int status = XGetWindowProperty(display, 
      x86UNIXState->getWindow(),
      mClipboardProperty, 0, numToRetrieve, True, XA_STRING, 
      &actual_type, &actual_format, &nitems, &bytes_after, &mXData);

   // we should have returned OK, with string type, 8bit data,
   // and > 0 items.
   if ((status != Success) || (actual_type != XA_STRING) || 
      (actual_format != 8) || (nitems == 0))
      return "";

   // if there is data left in the clipboard, warn about it
   if (bytes_after > 0)
      Con::warnf(ConsoleLogEntry::General, 
         "XClipboard: some data was not retrieved");

   return reinterpret_cast<const char *>(mXData);
}

//------------------------------------------------------------------------------
void XClipboard::checkTDataSize(S32 requestedSize)
{
   if (mTDataSize < requestedSize)
   {
      freeTData();
      mTData = static_cast<char*>(dRealMalloc(sizeof(char) * requestedSize));
      AssertFatal(mTData, "unable to allocate clipboard buffer data!");
      mTDataSize = requestedSize;
   }
}

//------------------------------------------------------------------------------
bool XClipboard::setClipboard(const char *text)
{
   DisplayPtrManager xdisplay;
   Display* display = xdisplay.getDisplayPointer();

   if (!mInitialized)
      init();

   // get the length of the text
   S32 len = dStrlen(text) + 1;
   
   // reallocate the storage buffer if necessary
   checkTDataSize(len);

   // copy the data into the storage buffer
   dStrcpy(mTData, text);

   // tell X that we own the clipboard.  (we'll get events
   // if an app tries to paste)
   XSetSelectionOwner(display, mClipboard, 
      x86UNIXState->getWindow(), CurrentTime);

   return true;
}

//------------------------------------------------------------------------------
void XClipboard::handleSelectionRequest(XSelectionRequestEvent& request)
{
   DisplayPtrManager xdisplay;
   Display* display = xdisplay.getDisplayPointer();

   // init our response
   XSelectionEvent notify;

   notify.type = SelectionNotify;
   notify.display = display;
   notify.requestor = request.requestor;
   notify.selection = request.selection;
   notify.target = XA_STRING;
   notify.property = None;
   notify.time = CurrentTime;

   // make sure the owner is our window, and that the
   // requestor wants the clipboard
   if (request.owner == x86UNIXState->getWindow() && 
      request.selection == mClipboard)
   {
      notify.property = request.property;
      // check to see if they did not set the property
      if (notify.property == None)
         notify.property = mClipboardProperty;

      // get the length of the data in the clipboard
      S32 length = dStrlen(mTData);
      // set the property on the requestor window
      XChangeProperty(display, request.requestor, 
         notify.property, XA_STRING,
         8, PropModeReplace, reinterpret_cast<const unsigned char*>(mTData), 
         length);
   }
   XSendEvent(display, notify.requestor, False, 0, 
      reinterpret_cast<XEvent*>(&notify));

   // flush the output buffer to send the event now
   XFlush(display);
}
