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

//-----------------------------------------------------------------------------
// getD3DXVer.exe
//
// Checks for the existance of the correct D3DX library.  Automatically
// uses the D3DX library version this executable was compiled against.
// (As contained in D3dx9core.h)
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include <D3dx9core.h>
#include <windows.h>

int _tmain(int argc, _TCHAR* argv[])
{
   // Method documented at http://msdn.microsoft.com/en-us/library/bb172717(VS.85).aspx
   // NOTE: Went with a different check below as this one requires linking with
   // the correct D3DX library to begin with.
   //if ( !D3DXCheckVersion(D3D_SDK_VERSION, D3DX_SDK_VERSION) )
   //{
   //   return 1;
   //}

   // Perform a simple LoadLibrary to check for the correct D3DX
   _TCHAR name[64];
   _stprintf(name, _T("d3dx9_%d.dll"), D3DX_SDK_VERSION);
   HINSTANCE hinstLib = LoadLibrary(name);
   if( !hinstLib )
   {
      //_tprintf(_T("'%s' lib NOT found"), name);
      return 1;
   }

   FreeLibrary(hinstLib);

   //_tprintf(_T("'%s' lib was found"), name);
   return 0;
}

