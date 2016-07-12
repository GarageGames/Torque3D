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

#ifndef _GFXADAPTER_H_
#define _GFXADAPTER_H_

#ifndef _GFXSTRUCTS_H_
#include "gfx/gfxStructs.h"
#endif

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif

#ifndef _UTIL_DELEGATE_H_
#include "core/util/delegate.h"
#endif

struct GFXAdapterLUID
{
   unsigned long LowPart;
   long HighPart;
};

struct GFXAdapter 
{
public:
   typedef Delegate<GFXDevice* (U32 adapterIndex)> CreateDeviceInstanceDelegate;     

   enum
   {
      MaxAdapterNameLen = 512,
   };

   char mName[MaxAdapterNameLen];

   /// The name of the display output device for the adapter, if any.
   /// For example under Windows, this could be: \\.\DISPLAY1
   char mOutputName[MaxAdapterNameLen];

   /// List of available full-screen modes. Windows can be any size,
   /// so we do not enumerate them here.
   Vector<GFXVideoMode> mAvailableModes;

   /// Supported shader model. 0.f means none supported.
   F32 mShaderModel;

	/// LUID for windows oculus support
	GFXAdapterLUID mLUID;

   const char * getName() const { return mName; }
   const char * getOutputName() const { return mOutputName; }
   GFXAdapterType mType;
   U32            mIndex;
   CreateDeviceInstanceDelegate mCreateDeviceInstanceDelegate;

   GFXAdapter()
   {
      VECTOR_SET_ASSOCIATION( mAvailableModes );

      mName[0] = 0;
      mOutputName[0] = 0;
      mShaderModel = 0.f;
      mIndex = 0;
		dMemset(&mLUID, '\0', sizeof(mLUID));
   }

   ~GFXAdapter()
   {
      mAvailableModes.clear();
   }
private:
   // Disallow copying to prevent mucking with our data above.
   GFXAdapter(const GFXAdapter&);
};

#endif // _GFXADAPTER_H_
