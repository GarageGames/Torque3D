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

#ifndef _TSDECAL_H_
#define _TSDECAL_H_

#ifndef _TSMESH_H_
#include "ts/tsMesh.h"
#endif

/// Decals!  The lovely detailing thingies, e.g. bullet hole marks.
/// DEPRECATED: This class is here for compatibility with old files only.
/// Performs no actual rendering.
class TSDecalMesh
{
public:

   /// The mesh that we are decaling
   TSMesh * targetMesh;

   /// @name Topology
   /// @{
   Vector<TSDrawPrimitive> primitives;
   Vector<U16> indices;
   /// @}

   /// @name Render Data
   /// indexed by decal frame...
   /// @{
   Vector<S32> startPrimitive;
   Vector<Point4F> texgenS;
   Vector<Point4F> texgenT;
   /// @}

   /// We only allow 1 material per decal...
   S32 materialIndex;

   /// DEPRECATED
   // void render(S32 frame, S32 decalFrame, TSMaterialList *);

   void disassemble();
   void assemble(bool skip);
};


#endif

