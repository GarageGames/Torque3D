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

#ifndef _T3D_PHYSICS_PHYSICSCOLLISION_H_
#define _T3D_PHYSICS_PHYSICSCOLLISION_H_

#ifndef _REFBASE_H_
#include "core/util/refBase.h"
#endif

class Point3F;
class MatrixF;
class PlaneF;


/// The shared collision representation for a instance of a 
/// static or dynamic physics body.
///
/// Note that making very big convex primitives can cause bad
/// queries and collisions in some physics providers.
///
/// @see PhysicsBody
///
class PhysicsCollision : public StrongRefBase
{
public:

   /// Add an infinite plane to the collision shape.
   ///
   /// This shape is assumed to be static in some physics
   /// providers and will at times be faked with a large box.
   ///
   virtual void addPlane( const PlaneF &plane ) = 0;

   /// Add a box to the collision shape.
   virtual void addBox( const Point3F &halfWidth,
                        const MatrixF &localXfm ) = 0;

   /// Add a sphere to the collision shape.
   virtual void addSphere( F32 radius,
                           const MatrixF &localXfm ) = 0;

   /// Add a Y axis capsule to the collision shape.
   virtual void addCapsule(   F32 radius,
                              F32 height,
                              const MatrixF &localXfm ) = 0;

   /// Add a point cloud convex hull to the collision shape.
   virtual bool addConvex( const Point3F *points, 
                           U32 count,
                           const MatrixF &localXfm ) = 0;

   /// Add a triangle mesh to the collision shape.
   virtual bool addTriangleMesh( const Point3F *vert,
                                 U32 vertCount,
                                 const U32 *index,
                                 U32 triCount,
                                 const MatrixF &localXfm ) = 0;

   /// Add a heightfield to the collision shape.
   virtual bool addHeightfield(  const U16 *heights,
                                 const bool *holes,
                                 U32 blockSize,
                                 F32 metersPerSample,
                                 const MatrixF &localXfm ) = 0;
};

#endif // _T3D_PHYSICS_PHYSICSCOLLISION_H_