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

#ifndef C_CONTROLINTERFACE_H
#define C_CONTROLINTERFACE_H
#include "platform/platformDlibrary.h"
#include "console/engineFunctions.h"

TORQUE_API void torque_reset();
TORQUE_API bool torque_engineinit(S32 argc, const char **argv);
TORQUE_API S32 torque_enginetick();
TORQUE_API S32 torque_getreturnstatus();
TORQUE_API void torque_enginesignalshutdown();
TORQUE_API S32 torque_engineshutdown();
TORQUE_API bool torque_isdebugbuild();
TORQUE_API void torque_setwebdeployment();
TORQUE_API void torque_resizewindow(S32 width, S32 height);

#if defined(TORQUE_OS_WIN) && !defined(TORQUE_SDL)
TORQUE_API void* torque_gethwnd();
TORQUE_API void torque_directmessage(U32 message, U32 wparam, U32 lparam);
#endif
#ifdef TORQUE_OS_WIN
TORQUE_API void torque_inputevent(S32 type, S32 value1, S32 value2);
#endif

TORQUE_API const char* torque_getexecutablepath();

#endif // C_CONTROLINTERFACE_H
