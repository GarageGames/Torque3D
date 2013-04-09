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

#ifndef _GFXD3D9CARDPROFILER_H_
#define _GFXD3D9CARDPROFILER_H_


#include "gfx/gfxCardProfile.h"

struct IDirect3DDevice9;
class GFXD3D9CardProfiler : public GFXCardProfiler
{
private:
   typedef GFXCardProfiler Parent;

   IDirect3DDevice9 *mD3DDevice;
   UINT mAdapterOrdinal;

public:
   GFXD3D9CardProfiler(U32 adapterIndex);
   ~GFXD3D9CardProfiler();
   void init();

protected:
   const String &getRendererString() const { static String sRS("D3D9"); return sRS; }

   void setupCardCapabilities();
   bool _queryCardCap(const String &query, U32 &foundResult);
   bool _queryFormat(const GFXFormat fmt, const GFXTextureProfile *profile, bool &inOutAutogenMips);
};

#endif
