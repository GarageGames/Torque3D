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
#ifndef __GFXSTATEBLOCKDATA_H_
#define __GFXSTATEBLOCKDATA_H_

#ifndef _GFXSTATEBLOCK_H_
#include "gfx/gfxStateBlock.h"
#endif
#ifndef _SIMOBJECT_H_
#include "console/simObject.h"
#endif


class GFXSamplerStateData;

/// Allows definition of render state via script, basically wraps a GFXStateBlockDesc
class GFXStateBlockData : public SimObject
{
   typedef SimObject Parent;

   GFXStateBlockDesc mState;
   GFXSamplerStateData* mSamplerStates[TEXTURE_STAGE_COUNT];
public:
   GFXStateBlockData();

   // SimObject
   virtual bool onAdd();
   static void initPersistFields();  

   // GFXStateBlockData
   const GFXStateBlockDesc getState() const { return mState; }
        
   DECLARE_CONOBJECT(GFXStateBlockData);   
};

/// Allows definition of sampler state via script, basically wraps a GFXSamplerStateDesc
class GFXSamplerStateData : public SimObject
{
   typedef SimObject Parent;
   GFXSamplerStateDesc mState;
public:
   // SimObject
   static void initPersistFields();  

   /// Copies the data of this object into desc
   void setSamplerState(GFXSamplerStateDesc& desc);

   DECLARE_CONOBJECT(GFXSamplerStateData);
};


#endif // __GFXSTATEBLOCKDATA_H_
