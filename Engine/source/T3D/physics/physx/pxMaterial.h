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

#ifndef _PHYSX_MATERIAL_H
#define _PHYSX_MATERIAL_H

#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif
#ifndef _DYNAMIC_CONSOLETYPES_H_
#include "console/dynamicTypes.h"
#endif

class NxMaterial;

class PxMaterial : public SimDataBlock 
{
   typedef SimDataBlock Parent;

protected:

   F32 restitution;
   F32 staticFriction;
   F32 dynamicFriction;

   NxMaterial *mNxMat;
   S32 mNxMatId;

   bool mServer;

public:

   DECLARE_CONOBJECT( PxMaterial );

   PxMaterial();
   ~PxMaterial();

   static void consoleInit();
   static void initPersistFields();
   virtual void onStaticModified( const char *slotName, const char *newValue );

   bool preload( bool server, String &errorBuffer );
   virtual void packData( BitStream* stream );
   virtual void unpackData( BitStream* stream );

   S32 getMaterialId() const { return mNxMatId; }

};

#endif // _PHYSX_MATERIAL_H