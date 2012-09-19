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

#ifndef OS_DLIBRARY_H
#define OS_DLIBRARY_H

#include "core/util/refBase.h"

// DLLs use the standard calling convention
#define DLL_CALL __stdcall
#define DLL_EXPORT_CALL __declspec(dllexport)
#define DLL_IMPORT_CALL __declspec(dllimport)

// Export functions from the DLL
#if defined(DLL_CODE)
   #define DLL_DECL DLL_EXPORT_CALL
#else
   #define DLL_DECL DLL_IMPORT_CALL
#endif


//-----------------------------------------------------------------------------

///@defgroup KernelDLL Loadable Libraries
/// Basic DLL handling and symbol resolving. When a library is first loaded
/// it's "open" function will be called, and it's "close" function is called
/// right before the library is unloaded.
///@ingroup OsModule
///@{

/// Dynamic Library
/// Interface for library objects loaded using the loadLibrary() function.
class DLibrary: public StrongRefBase
{
public:
   virtual ~DLibrary() {}
   virtual void *bind( const char *name ) = 0;
};
typedef StrongRefPtr<DLibrary> DLibraryRef;

/// Load a library
/// Returns 0 if the library fails to load. Symbols are
/// resolved through the DLibrary interface.
DLibraryRef OsLoadLibrary( const char *file );

///@}

//-----------------------------------------------------------------------------



#endif

