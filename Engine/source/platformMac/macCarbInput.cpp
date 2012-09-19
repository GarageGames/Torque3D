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

#include <ApplicationServices/ApplicationServices.h>
#include <Carbon/Carbon.h>

#include "platform/platformInput.h"
#include "console/console.h"
#include "core/strings/unicode.h"
#include "core/util/tVector.h"

// Static class variables:
InputManager*  Input::smManager;
bool           Input::smActive;
U8             Input::smModifierKeys;
InputEvent     Input::smInputEvent;

//-----------------------------------------------------------------------------
// Keycode mapping.

struct KeyCode
{
   U32      mKeyCode;
   UniChar  mCharLower;
   UniChar  mCharUpper;
   
   KeyCode( U32 keyCode )
      : mKeyCode( keyCode ) {}
};

static KeyCode sOSToKeyCode[] =
{
   KEY_A, // 0x00
   KEY_S, // 0x01
   KEY_D, // 0x02
   KEY_F, // 0x03
   KEY_H, // 0x04
   KEY_G, // 0x05
   KEY_Z, // 0x06
   KEY_X, // 0x07
   KEY_C, // 0x08
   KEY_V, // 0x09
   0, // 0x0A
   KEY_B, // 0x0B
   KEY_Q, // 0x0C
   KEY_W, // 0x0D
   KEY_E, // 0x0E
   KEY_R, // 0x0F
   KEY_Y, // 0x10
   KEY_T, // 0x11
   KEY_1, // 0x12
   KEY_2, // 0x13
   KEY_3, // 0x14
   KEY_4, // 0x15
   KEY_6, // 0x16
   KEY_5, // 0x17
   KEY_EQUALS, // 0x18
   KEY_9, // 0x19
   KEY_7, // 0x1A
   KEY_MINUS, // 0x1B
   KEY_8, // 0x1C
   KEY_0, // 0x1D
   KEY_RBRACKET, // 0x1E
   KEY_O, // 0x1F
   KEY_U, // 0x20
   KEY_LBRACKET, // 0x21
   KEY_I, // 0x22
   KEY_P, // 0x23
   KEY_RETURN, // 0x24
   KEY_L, // 0x25
   KEY_J, // 0x26
   KEY_APOSTROPHE, // 0x27
   KEY_K, // 0x28
   KEY_SEMICOLON, // 0x29
   KEY_BACKSLASH, // 0x2A
   KEY_COMMA, // 0x2B
   KEY_SLASH, // 0x2C
   KEY_N, // 0x2D
   KEY_M, // 0x2E
   KEY_PERIOD, // 0x2F
   KEY_TAB, // 0x30
   KEY_SPACE, // 0x31
   KEY_TILDE, // 0x32
   KEY_BACKSPACE, // 0x33
   0, // 0x34
   KEY_ESCAPE, // 0x35
   KEY_RALT, // 0x36
   KEY_LALT, // 0x37
   KEY_LSHIFT, // 0x38
   KEY_CAPSLOCK, // 0x39
   KEY_MAC_LOPT, // 0x3A
   KEY_LCONTROL, // 0x3B
   KEY_RSHIFT, // 0x3C
   KEY_MAC_ROPT, // 0x3D
   KEY_RCONTROL, // 0x3E
   0, // 0x3F
   0, // 0x40
   KEY_DECIMAL, // 0x41
   0, // 0x42
   KEY_MULTIPLY, // 0x43
   0, // 0x44
   KEY_ADD, // 0x45
   0, // 0x46
   KEY_NUMLOCK, // 0x47
   0, // 0x48
   0, // 0x49
   0, // 0x4A
   KEY_DIVIDE, // 0x4B
   KEY_NUMPADENTER, // 0x4C
   0, // 0x4D
   KEY_SUBTRACT, // 0x4E
   0, // 0x4F
   0, // 0x50
   KEY_SEPARATOR, // 0x51
   KEY_NUMPAD0, // 0x52
   KEY_NUMPAD1, // 0x53
   KEY_NUMPAD2, // 0x54
   KEY_NUMPAD3, // 0x55
   KEY_NUMPAD4, // 0x56
   KEY_NUMPAD5, // 0x57
   KEY_NUMPAD6, // 0x58
   KEY_NUMPAD7, // 0x59
   0, // 0x5A
   KEY_NUMPAD8, // 0x5B
   KEY_NUMPAD9, // 0x5C
   0, // 0x5D
   0, // 0x5E
   0, // 0x5F
   KEY_F5, // 0x60
   KEY_F6, // 0x61
   KEY_F7, // 0x62
   KEY_F3, // 0x63
   KEY_F8, // 0x64
   KEY_F9, // 0x65
   0, // 0x66
   KEY_F11, // 0x67
   0, // 0x68
   KEY_F13, // 0x69
   KEY_F16, // 0x6A
   KEY_F14, // 0x6B
   0, // 0x6C
   KEY_F10, // 0x6D
   0, // 0x6E
   KEY_F12, // 0x6F
   0, // 0x70
   KEY_F15, // 0x71
   KEY_INSERT, // 0x72
   KEY_HOME, // 0x73
   KEY_PAGE_UP, // 0x74
   KEY_DELETE, // 0x75
   KEY_F4, // 0x76
   KEY_END, // 0x77
   KEY_F2, // 0x78
   KEY_PAGE_DOWN, // 0x79
   KEY_F1, // 0x7A
   KEY_LEFT, // 0x7B
   KEY_RIGHT, // 0x7C
   KEY_DOWN, // 0x7D
   KEY_UP, // 0x7E
};

static Vector< U8 > sKeyCodeToOS( __FILE__, __LINE__ );

#define NSShiftKeyMask ( 1 << 17 )

static KeyboardLayoutRef sKeyLayout;
static SInt32 sKeyLayoutKind = -1;
static SInt32 sKeyLayoutID = -1;
static SInt32 sLastKeyLayoutID = -1;

static void GetKeyboardLayout()
{
   KLGetCurrentKeyboardLayout( &sKeyLayout );
   KLGetKeyboardLayoutProperty( sKeyLayout, kKLKind, ( const void** ) &sKeyLayoutKind );
   KLGetKeyboardLayoutProperty( sKeyLayout, kKLIdentifier, ( const void** ) &sKeyLayoutID );
}

static bool KeyboardLayoutHasChanged()
{
   GetKeyboardLayout();
   return ( sKeyLayoutID != sLastKeyLayoutID );
}

static UniChar OSKeyCodeToUnicode( UInt16 osKeyCode, bool shift = false )
{
   // Translate the key code.
   
   UniChar uniChar = 0;
   if( sKeyLayoutKind == kKLKCHRKind )
   {
      // KCHR mapping.
      
      void* KCHRData;
      KLGetKeyboardLayoutProperty( sKeyLayout, kKLKCHRData, ( const void** ) & KCHRData );
      
      UInt16 key = ( osKeyCode & 0x7f );
      if( shift )
         key |= NSShiftKeyMask;
         
      UInt32 keyTranslateState = 0;
      UInt32 charCode = KeyTranslate( KCHRData, key, &keyTranslateState );
      charCode &= 0xff;
      
      if( keyTranslateState == 0 && charCode )
         uniChar = charCode;
   }
   else
   {
      // UCHR mapping.
      
      UCKeyboardLayout* uchrData;
      KLGetKeyboardLayoutProperty( sKeyLayout, kKLuchrData, ( const void** ) &uchrData );
      
      UInt32 deadKeyState;
      UniCharCount actualStringLength;
      UniChar unicodeString[ 4 ];
      UCKeyTranslate( uchrData,
                      osKeyCode,
                      kUCKeyActionDown,
                      ( shift ? 0x02 : 0 ), // Oh yeah... Apple docs are fun...
                      LMGetKbdType(),
                      0,
                      &deadKeyState,
                      sizeof( unicodeString ) / sizeof( unicodeString[ 0 ] ),
                      &actualStringLength,
                      unicodeString );
      
      if( actualStringLength )
         uniChar = unicodeString[ 0 ]; // Well, Unicode is something else, but...
   }
   
   return uniChar;
}

static void InitKeyCodeMapping()
{
   const U32 numOSKeyCodes = sizeof( sOSToKeyCode ) / sizeof( sOSToKeyCode[ 0 ] );
   GetKeyboardLayout();
   sLastKeyLayoutID = sKeyLayoutID;
   
   U32 maxKeyCode = 0;
   for( U32 i = 0; i < numOSKeyCodes; ++ i )
   {
      sOSToKeyCode[ i ].mCharLower = OSKeyCodeToUnicode( i, false );
      sOSToKeyCode[ i ].mCharUpper = OSKeyCodeToUnicode( i, true );
      
      if( sOSToKeyCode[ i ].mKeyCode > maxKeyCode )
         maxKeyCode = sOSToKeyCode[ i ].mKeyCode;
   }
   
   if( !sKeyCodeToOS.size() )
   {
      sKeyCodeToOS.setSize( maxKeyCode + 1 );
      dMemset( sKeyCodeToOS.address(), 0, sKeyCodeToOS.size() );
      for( U32 i = 0; i < numOSKeyCodes; ++ i )
         sKeyCodeToOS[ sOSToKeyCode[ i ].mKeyCode ] = i;
   }
}

U8 TranslateOSKeyCode(U8 macKeycode)
{
   AssertWarn(macKeycode < sizeof(sOSToKeyCode) / sizeof(sOSToKeyCode[0]), avar("TranslateOSKeyCode - could not translate code %i", macKeycode));
   if(macKeycode >= sizeof(sOSToKeyCode) / sizeof(sOSToKeyCode[0]))
      return KEY_NULL;
      
   return sOSToKeyCode[ macKeycode ].mKeyCode;
}

U8 TranslateKeyCodeToOS( U8 keycode )
{
   return sKeyCodeToOS[ keycode ];
}

#pragma mark ---- Clipboard functions ----
//-----------------------------------------------------------------------------
const char* Platform::getClipboard()
{
   // mac clipboards can contain multiple items,
   //  and each item can be in several differnt flavors, 
   //  such as unicode or plaintext or pdf, etc.
   // scan through the clipboard, and return the 1st piece of actual text.
   ScrapRef    clip;
   char        *retBuf = "";
   OSStatus    err = noErr;
   char        *dataBuf = "";
   
   // get a local ref to the system clipboard
   GetScrapByName( kScrapClipboardScrap, kScrapGetNamedScrap, &clip ); 
   
   
   // First try to get unicode data, then try to get plain text data.
   Size dataSize = 0;
   bool plaintext = false;
   err = GetScrapFlavorSize(clip, kScrapFlavorTypeUnicode, &dataSize);
   if( err != noErr || dataSize <= 0)
   {
      Con::errorf("some error getting unicode clip");
      plaintext = true;
      err = GetScrapFlavorSize(clip, kScrapFlavorTypeText, &dataSize);
   }

   // kick out if we don't have any data.
   if( err != noErr || dataSize <= 0)
   {
      Con::errorf("no data, kicking out. size = %i",dataSize);
      return "";
   }
   
   if( err == noErr && dataSize > 0 )
   {
      // ok, we've got something! allocate a buffer and copy it in.
      char buf[dataSize+1];
      dMemset(buf, 0, dataSize+1);
      dataBuf = buf;      
      // plain text needs no conversion.
      // unicode data needs to be converted to normalized utf-8 format.
      if(plaintext)
      { 
         GetScrapFlavorData(clip, kScrapFlavorTypeText, &dataSize, &buf);
         retBuf = Con::getReturnBuffer(dataSize + 1);
         dMemcpy(retBuf,buf,dataSize);
      }
      else
      {
         GetScrapFlavorData(clip, kScrapFlavorTypeUnicode, &dataSize, &buf);         

         // normalize
         CFStringRef cfBuf = CFStringCreateWithBytes(NULL, (const UInt8*)buf, dataSize, kCFStringEncodingUnicode, false);
         CFMutableStringRef normBuf = CFStringCreateMutableCopy(NULL, 0, cfBuf);
         CFStringNormalize(normBuf, kCFStringNormalizationFormC);

         // convert to utf-8
         U32 normBufLen = CFStringGetLength(normBuf);
         U32 retBufLen = CFStringGetMaximumSizeForEncoding(normBufLen,kCFStringEncodingUTF8) + 1; // +1 for the null terminator
         retBuf = Con::getReturnBuffer(retBufLen);
         CFStringGetCString( normBuf, retBuf, retBufLen, kCFStringEncodingUTF8);
         dataSize = retBufLen;
      }

      // manually null terminate, just in case.
      retBuf[dataSize] = 0;
   }
         
    // return the data, or the empty string if we did not find any data.
         return retBuf;
}

//-----------------------------------------------------------------------------
bool Platform::setClipboard(const char *text)
{
   ScrapRef       clip;
   U32            textSize;
   OSStatus       err = noErr;
   
   // make sure we have something to copy
   textSize = dStrlen(text);
   if(textSize == 0)
      return false;
   
   // get a local ref to the system clipboard
   GetScrapByName( kScrapClipboardScrap, kScrapClearNamedScrap, &clip );

   // put the data on the clipboard as text
   err = PutScrapFlavor( clip, kScrapFlavorTypeText, kScrapFlavorMaskNone, textSize, text);
   
   // put the data on the clipboard as unicode
   const UTF16 *utf16Data = convertUTF8toUTF16(text);
   err |= PutScrapFlavor( clip, kScrapFlavorTypeUnicode, kScrapFlavorMaskNone,
                        dStrlen(utf16Data) * sizeof(UTF16), utf16Data);
   delete [] utf16Data;

   // and see if we were successful.
   if( err == noErr )
      return true;
   else
      return false;
}

void Input::init()
{
   smManager = NULL;
   smActive = false;
   
   InitKeyCodeMapping();
}

U16 Input::getKeyCode( U16 asciiCode )
{
   if( KeyboardLayoutHasChanged() )
      InitKeyCodeMapping();
      
   for( U32 i = 0; i < ( sizeof( sOSToKeyCode ) / sizeof( sOSToKeyCode[ 0 ] ) ); ++ i )
      if( sOSToKeyCode[ i ].mCharLower == asciiCode
          || sOSToKeyCode[ i ].mCharUpper == asciiCode )
         return sOSToKeyCode[ i ].mKeyCode;
         
   return 0;
}

U16 Input::getAscii( U16 keyCode, KEY_STATE keyState )
{
   GetKeyboardLayout();
   return OSKeyCodeToUnicode( TranslateKeyCodeToOS( keyCode ), ( keyState == STATE_UPPER ? true : false ) );
}

void Input::destroy()
{
}

bool Input::enable()
{
   return true;
}

void Input::disable()
{
}

void Input::activate()
{
}

void Input::deactivate()
{
}

bool Input::isEnabled()
{
   return true;
}

bool Input::isActive()
{
   return true;
}

void Input::process()
{
}

InputManager* Input::getManager()
{
   return smManager;
}

ConsoleFunction( enableMouse, bool, 1, 1, "enableMouse()" )
{
   return true;
}

ConsoleFunction( disableMouse, void, 1, 1, "disableMouse()" )
{
}

ConsoleFunction( echoInputState, void, 1, 1, "echoInputState()" )
{
}

ConsoleFunction( toggleInputState, void, 1, 1, "toggleInputState()" )
{
}

ConsoleFunction( isJoystickDetected, bool, 1, 1, "Always false on the MAC." )
{
   return false;
}

ConsoleFunction( getJoystickAxes, const char*, 2, 2, "(handle instance)" )
{
   return "";
}

ConsoleFunction( deactivateKeyboard, void, 1, 1, "deactivateKeyboard();")
{
   // these are only useful on the windows side. They deal with some vagaries of win32 DirectInput.
}

ConsoleFunction( activateKeyboard, void, 1, 1, "activateKeyboard();")
{
   // these are only useful on the windows side. They deal with some vagaries of win32 DirectInput.
}
