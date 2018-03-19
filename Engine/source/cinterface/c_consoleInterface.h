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

#ifndef C_CONSOLEINTERFACE_H
#define C_CONSOLEINTERFACE_H
#include "platform/platformDlibrary.h"
#include "console/consoleInternal.h"

extern "C" {
   DLL_DECL void Con_AddConsumer(ConsumerCallback cb);
   DLL_DECL void Con_RemoveConsumer(ConsumerCallback cb);

   DLL_DECL const char* Con_getConsoleString(const char* name);
   DLL_DECL void Con_setConsoleString(const char* name, const char* value);
   DLL_DECL S32 Con_getConsoleInt(const char* name);
   DLL_DECL void Con_setConsoleInt(const char* name, S32 value);
   DLL_DECL F32 Con_getConsoleFloat(const char* name);
   DLL_DECL void Con_setConsoleFloat(const char* name, F32 value);
   DLL_DECL bool Con_getConsoleBool(const char* name);
   DLL_DECL void Con_setConsoleBool(const char* name, bool value);
}
#endif // C_CONSOLEINTERFACE_H
