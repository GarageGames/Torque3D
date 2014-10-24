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

#ifndef _MESH_PARSING_H_
#define _MESH_PARSING_H_

#ifndef _TSSHAPE_H_
#include "ts/tsShape.h"
#endif

#include "console/engineAPI.h"  
#include "console/consoleTypes.h" 
#include "math/mPoint3.h"

// TODO: Use stock methods where applicable.
class psMeshParsing {
public:
   /// A triangular "face" in a mesh.
   struct face
   {
      S32 triStart;
      S32 meshIndex;
      F32 area;
   };

   // Calculates the area of a triangle using Herons formula
   static F32 HeronsF(VectorF a, VectorF b, VectorF c);
};

/// An interface for objects that has meshes that need to interact
/// with a ParticleSystem (e.g. for use with @ref MeshEmitter)
class psMeshInterface {
public:
   /// Transforms a vertex position to match the current 
   /// transform of the mesh.
   virtual void transformVertex(Point3F &p) = 0;

   /// Get the @ref TSShapeInstance that interacts with the
   /// system.
   virtual TSShapeInstance* getShapeInstance() const = 0;

   /// Get position of the shape.
   virtual Point3F getShapePosition() const = 0;

   /// Get shape of the shape.
   virtual Point3F getShapeScale() const = 0;

   psMeshInterface() {};
   virtual ~psMeshInterface() {};
};

#endif