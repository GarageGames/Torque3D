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

#ifndef _GFXDEBUGEVENT_H_
#define _GFXDEBUGEVENT_H_

#ifndef _GFXDEVICE_H_
#include "gfx/gfxDevice.h"
#endif

/// See TorqueConfig.h to enable this.
#ifdef TORQUE_ENABLE_GFXDEBUGEVENTS


/// You shouldn't use this class directly... use the 
/// following macros:
///
/// GFXDEBUGEVENT_START / GFXDEBUGEVENT_END
/// GFXDEBUGEVENT_SCOPE
///
class GFXDebugEventScope
{
public:
   GFXDebugEventScope( const char* name, const ColorI &color ) 
   {
      GFX->enterDebugEvent( color, name );
   }

   ~GFXDebugEventScope() 
   {
      GFX->leaveDebugEvent();
   }
};

#define GFXDEBUGEVENT_START( name, color ) GFX->enterDebugEvent( color, #name )

#define GFXDEBUGEVENT_END() GFX->leaveDebugEvent()

#define GFXDEBUGEVENT_MARKER( name, color ) GFX->setDebugMarker( color, #name )


///
/// Will add start/end GFX debug events around the 
/// current scope.
///
/// @param name   The unquoted name for the event.
/// @param color  A ColorI to associate with the event.
///
#define GFXDEBUGEVENT_SCOPE( name, color ) GFXDebugEventScope GFXDebugEventScope##name##Obj( #name, color )
#define GFXDEBUGEVENT_SCOPE_EX( name, color, desc ) GFXDebugEventScope GFXDebugEventScope##name##Obj( desc, color )

#else // !TORQUE_ENABLE_GFXDEBUGEVENTS

   /// These are disabled in shipping builds or maybe you
   /// forgot to include "platform/platform.h" first?
   #define GFXDEBUGEVENT_START(n,c)
   #define GFXDEBUGEVENT_END()
   #define GFXDEBUGEVENT_MARKER(n,c)
   #define GFXDEBUGEVENT_SCOPE(n,c)
   #define GFXDEBUGEVENT_SCOPE_EX(n,c,d)

#endif

#endif // _GFXDEBUGEVENT_H_
