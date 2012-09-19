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
#ifndef _CUSTOMMATERIALDEFINITION_H_
#define _CUSTOMMATERIALDEFINITION_H_

#ifndef _MATERIALDEFINITION_H_
#include "materials/materialDefinition.h"
#endif

class ShaderData;
class GFXStateBlockData;

//**************************************************************************
// Custom Material
//**************************************************************************
class CustomMaterial : public Material
{
   typedef Material Parent;
public:
   enum CustomConsts
   {
      MAX_PASSES = 8,
      NUM_FALLBACK_VERSIONS = 2,
   };
   
   FileName mTexFilename[MAX_TEX_PER_PASS];
   String mSamplerNames[MAX_TEX_PER_PASS];
   String mOutputTarget;
   Material* mFallback;
   bool mForwardLit;

   F32 mVersion;   // 0 = legacy, 1 = DX 8.1, 2 = DX 9.0   
   bool mRefract;   
   ShaderData* mShaderData;

   CustomMaterial();       
   const GFXStateBlockData* getStateBlockData() const;

   //
   // SimObject interface
   //
   virtual bool onAdd();
   virtual void onRemove();

   //
   // ConsoleObject interface
   //
   static void initPersistFields();
   DECLARE_CONOBJECT(CustomMaterial);
protected:
   U32 mMaxTex;
   String mShaderDataName;
   U32 mFlags[MAX_TEX_PER_PASS];   
   GFXStateBlockData* mStateBlockData;

   virtual void _mapMaterial();
};

#endif
