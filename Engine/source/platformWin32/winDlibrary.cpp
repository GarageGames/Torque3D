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

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "platform/types.h"
#include "platform/platformDlibrary.h"

class Win32DLibrary: public DLibrary
{
   HMODULE _handle;
public:
   Win32DLibrary();
   ~Win32DLibrary();
   bool open(const char* file);
   void close();
   void *bind(const char *name);
};

Win32DLibrary::Win32DLibrary()
{
   _handle = 0;
}

Win32DLibrary::~Win32DLibrary()
{
   close();
}

bool Win32DLibrary::open(const char* file)
{
   // dlopen should also include the RTLD_LOCAL flag, but it seems to be
   // missing from cygwin
   _handle = LoadLibraryA(file);
   if (!_handle)
      return false;
   bool (*open)() = (bool(*)())bind("dllopen");
   if (open && !(*open)()) {
      FreeLibrary(_handle);
      _handle = 0;
      return false;
   }
   return true;
}

void Win32DLibrary::close()
{
   if (_handle) {
      void (*close)() = (void(*)())bind("dllclose");
      if (close)
         (*close)();
      FreeLibrary(_handle);
      _handle = 0;
   }
}

void* Win32DLibrary::bind(const char *name)
{
   return _handle? (void*)GetProcAddress(_handle,name): 0;
}

DLibraryRef OsLoadLibrary(const char* file)
{
   Win32DLibrary* library = new Win32DLibrary();
   if (!library->open(file)) {
      delete library;
      return 0;
   }
   return library;
}

