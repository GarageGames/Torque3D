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

#include "platform/types.h"
#include "platform/platformDlibrary.h"
#include <dlfcn.h>

class MacDLibrary : public DLibrary
{
   void* _handle;
public:
   MacDLibrary();
   ~MacDLibrary();
   bool open(const char* file);
   void close();
   void* bind(const char* name);
};

MacDLibrary::MacDLibrary()
{
   _handle = NULL;
}

MacDLibrary::~MacDLibrary()
{
   close();
}

bool MacDLibrary::open(const char* file)
{
   Platform::getExecutablePath();
   _handle = dlopen(file, RTLD_LAZY | RTLD_LOCAL);
   return _handle != NULL;
}

void* MacDLibrary::bind(const char* name)
{
   return _handle ? dlsym(_handle, name) : NULL;
}

void MacDLibrary::close()
{
   if(_handle)
      dlclose(_handle);
   
   _handle = NULL;
}

DLibraryRef OsLoadLibrary(const char* file)
{
   MacDLibrary* library = new MacDLibrary();
   if(!library->open(file))
   {
      delete library;
      return NULL;
   }
   
   return library;
}
