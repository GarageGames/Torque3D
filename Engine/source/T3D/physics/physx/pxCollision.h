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

#ifndef _T3D_PHYSICS_PXCOLLISION_H_
#define _T3D_PHYSICS_PXCOLLISION_H_

#ifndef _T3D_PHYSICS_PHYSICSCOLLISION_H_
#include "T3D/physics/physicsCollision.h"
#endif
#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif

class NxShapeDesc;


class PxCollision : public PhysicsCollision
{
protected:

   /// The collision representation.
   Vector<NxShapeDesc*> mColShapes;

   /// Helper for adding shapes.
   //void _addShape( btCollisionShape *shape, const MatrixF &localXfm );

public:

   PxCollision();
   virtual ~PxCollision();

   /// Return the PhysX shape descriptions.
   const Vector<NxShapeDesc*>& getShapes() const { return mColShapes; }

   // PhysicsCollision
   virtual void addPlane( const PlaneF &plane );
   virtual void addBox( const Point3F &halfWidth,
                        const MatrixF &localXfm );
   virtual void addSphere( F32 radius,
                           const MatrixF &localXfm );
   virtual void addCapsule(   F32 radius,
                              F32 height,
                              const MatrixF &localXfm );
   virtual bool addConvex( const Point3F *points, 
                           U32 count,
                           const MatrixF &localXfm );
   virtual bool addTriangleMesh( const Point3F *vert,
                                 U32 vertCount,
                                 const U32 *index,
                                 U32 triCount,
                                 const MatrixF &localXfm );
   virtual bool addHeightfield(  const U16 *heights,
                                 const bool *holes,
                                 U32 blockSize,
                                 F32 metersPerSample,
                                 const MatrixF &localXfm );
};

#endif // _T3D_PHYSICS_PXCOLLISION_H_