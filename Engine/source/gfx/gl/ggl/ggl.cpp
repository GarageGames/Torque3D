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

#include "core/strings/stringFunctions.h"

#include "platform/platformDlibrary.h"
#include "console/console.h"

namespace GL
{

//-----------------------------------------------------------------------------
// Current active extensions
struct GLExtensionPtrs;
GLExtensionPtrs* _GGLptr;

struct GLExtensionFlags;
GLExtensionFlags* _GGLflag;


//-----------------------------------------------------------------------------

static inline int versionCombine( int major, int minor )
{
   return major * 100 + minor;
}

bool bindFunction( DLibrary *dll, void *&fnAddress, const char *name )
{
   fnAddress = dll->bind( name );

   if (!fnAddress)
      Con::warnf( "GLExtensions: DLL bind failed for %s", name );

   return fnAddress != 0;
}

bool hasExtension( const char *name, const char *extensions )
{
   // Extensions are compared against the extension strings
   if (extensions && *extensions) {
      const char* ptr = dStrstr(extensions,name);
      if (ptr) {
         char end = ptr[dStrlen(name)];
         if (end == ' ' || end == 0)
            return true;
      }
   }
   return false;
}

bool hasVersion( const char *name, const char* prefix, int major, int minor )
{
   // Extension group names are compared against the version number. The prefix
   // string should be "GL_VERSION" or "GLX_VERSION".. group names are
   // "GL_VERSION_1_1", "GL_VERSION_1_2", "GLX_VERSION_1_2", etc.
   while (*name && *prefix && *name == *prefix)
      name++, prefix++;
   if (*name == '_' && *prefix == '\0') {
      int maj = dAtoi(++name);
      while (dIsdigit(*name))
         *name++;
      int min = dAtoi(++name);
      return versionCombine(maj,min) <= versionCombine(major,minor);
   }
   return false;
}

} // Namespace

