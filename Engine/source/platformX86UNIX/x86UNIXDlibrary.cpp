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

class x86UNIXDLibrary: public DLibrary
{
    void*   mLibHandle;
public:
    x86UNIXDLibrary();
    virtual ~x86UNIXDLibrary();

    bool open(const char* file);
    void close();
    virtual void *bind(const char *name);
};

x86UNIXDLibrary::x86UNIXDLibrary()
{
    mLibHandle = 0;
}

x86UNIXDLibrary::~x86UNIXDLibrary()
{
    close();
}

bool x86UNIXDLibrary::open(const char* file)
{
    mLibHandle = dlopen(file, RTLD_LAZY);
    if( !mLibHandle )
        return false;

    return true;
}

void x86UNIXDLibrary::close()
{
    if( mLibHandle )
    {
        dlclose(mLibHandle);
        mLibHandle = 0;
    }
}

void* x86UNIXDLibrary::bind(const char *name)
{
    return mLibHandle ? dlsym(mLibHandle, name) : 0;
}

DLibraryRef OsLoadLibrary(const char* file)
{
    x86UNIXDLibrary* library = new x86UNIXDLibrary();
    if (!library->open(file)) 
    {
        delete library;
        library = 0;
    }
    return library;
}

