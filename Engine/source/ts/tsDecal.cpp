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

#include "ts/tsDecal.h"
#include "math/mMath.h"
#include "math/mathIO.h"
#include "ts/tsShapeInstance.h"
#include "core/frameAllocator.h"

//-----------------------------------------------------
// TSDecalMesh assembly/dissembly methods
// used for transfer to/from memory buffers
//-----------------------------------------------------

#define tsalloc TSShape::smTSAlloc

void TSDecalMesh::assemble(bool)
{
   if (TSShape::smReadVersion<20)
   {
      // read empty mesh...decals used to be derived from meshes
      tsalloc.checkGuard();
      tsalloc.getPointer32(15);
   }

   S32 sz = tsalloc.get32();
   S32 * ptr32 = tsalloc.copyToShape32(0); // get current shape address w/o doing anything
   for (S32 i=0; i<sz; i++)
   {
      tsalloc.getPointer16(2);
      tsalloc.getPointer32(1);
   }
   tsalloc.align32();
   primitives.set(ptr32,sz);

   sz = tsalloc.get32();
   S16 * ptr16 = tsalloc.getPointer16(sz);
   tsalloc.align32();
   indices.set(ptr16,sz);

   if (TSShape::smReadVersion<20)
   {
      // read more empty mesh stuff...decals used to be derived from meshes
      tsalloc.getPointer32(3);
      tsalloc.checkGuard();
   }

   sz = tsalloc.get32();
   ptr32 = tsalloc.getPointer32(sz);
   startPrimitive.set(ptr32,sz);

   if (TSShape::smReadVersion>=19)
   {
   ptr32 = tsalloc.getPointer32(sz*4);
   texgenS.set(ptr32,startPrimitive.size());
   ptr32 = tsalloc.getPointer32(sz*4);
   texgenT.set(ptr32,startPrimitive.size());
   }
   else
   {
      texgenS.set(NULL,0);
      texgenT.set(NULL,0);
   }

   materialIndex = tsalloc.get32();

   tsalloc.checkGuard();
}

void TSDecalMesh::disassemble()
{
   tsalloc.set32(primitives.size());
   tsalloc.copyToBuffer32((S32*)primitives.address(),primitives.size());

   tsalloc.set32(indices.size());
   tsalloc.copyToBuffer32((S32*)indices.address(),indices.size());

   tsalloc.set32(startPrimitive.size());
   tsalloc.copyToBuffer32((S32*)startPrimitive.address(),startPrimitive.size());

   tsalloc.copyToBuffer32((S32*)texgenS.address(),texgenS.size()*4);
   tsalloc.copyToBuffer32((S32*)texgenT.address(),texgenT.size()*4);

   tsalloc.set32(materialIndex);

   tsalloc.setGuard();
}

