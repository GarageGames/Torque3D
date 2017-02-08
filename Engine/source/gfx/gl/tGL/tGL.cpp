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

#include "tGL.h"

#include "core/strings/stringFunctions.h"
#include "console/console.h"

#ifdef TORQUE_OS_WIN
   #include "tWGL.h"
#endif

namespace GL
{
   void gglPerformBinds()
   {
      if (!gladLoadGL()) 
      {
	     AssertFatal(false, "Unable to load GLAD. Make sure your OpenGL drivers are up to date!");
      }
   }

   void gglPerformExtensionBinds(void *context)
   {
   #ifdef  TORQUE_OS_WIN 
      if (!gladLoadWGL(wglGetCurrentDC()))
      {
         AssertFatal(false, "Unable to load GLAD WGL extensions. Make sure your OpenGL drivers are up to date!");
      }
   #endif
   }
}

