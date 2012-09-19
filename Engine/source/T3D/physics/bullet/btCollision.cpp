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

#include "platform/platform.h"
#include "T3D/physics/bullet/btCollision.h"

#include "math/mPoint3.h"
#include "math/mMatrix.h"
#include "T3D/physics/bullet/bt.h"
#include "T3D/physics/bullet/btCasts.h"


BtCollision::BtCollision() 
   :  mCompound( NULL ),
      mLocalXfm( true )
{
}

BtCollision::~BtCollision()
{
   SAFE_DELETE( mCompound );

   for ( U32 i=0; i < mShapes.size(); i++ )
      delete mShapes[i];

   for ( U32 i=0; i < mMeshInterfaces.size(); i++ )
      delete mMeshInterfaces[i];
}

btCollisionShape* BtCollision::getShape() 
{   
   if ( mCompound )
      return mCompound;
   
   if ( mShapes.empty() )
      return NULL;

   return mShapes.first();
}

void BtCollision::_addShape( btCollisionShape *shape, const MatrixF &localXfm )
{
   AssertFatal( !shape->isCompound(), "BtCollision::_addShape - Shape should not be a compound!" );

   // Stick the shape into the array to delete later.  Remember
   // that the compound shape doesn't delete its children.
   mShapes.push_back( shape );

   // If this is the first shape then just store the
   // local transform and we're done.
   if ( mShapes.size() == 1 )
   {
      mLocalXfm = localXfm;
      return;
   }

   // We use a compound to store the shapes with their
   // local transforms... so create it if we haven't already.
   if ( !mCompound )
   {
      mCompound = new btCompoundShape();

      // There should only be one shape now... add it and
      // clear the local transform.
      mCompound->addChildShape( btCast<btTransform>( mLocalXfm ), mShapes.first() );
      mLocalXfm = MatrixF::Identity;
   }

   // Add the new shape to the compound.
   mCompound->addChildShape( btCast<btTransform>( localXfm ), shape );
}

void BtCollision::addPlane( const PlaneF &plane )
{
   // NOTE: Torque uses a negative D... thats why we flip it here.
   btStaticPlaneShape *shape = new btStaticPlaneShape( btVector3( plane.x, plane.y, plane.z ), -plane.d );
   _addShape( shape, MatrixF::Identity );
}

void BtCollision::addBox(  const Point3F &halfWidth,
                           const MatrixF &localXfm )
{
   btBoxShape *shape = new btBoxShape( btVector3( halfWidth.x, halfWidth.y, halfWidth.z ) );
   shape->setMargin( 0.01f );
   _addShape( shape, localXfm );
}

void BtCollision::addSphere(  const F32 radius,
                              const MatrixF &localXfm )
{
   btSphereShape *shape = new btSphereShape( radius );
   shape->setMargin( 0.01f );
   _addShape( shape, localXfm );
}

void BtCollision::addCapsule( F32 radius,
                              F32 height,
                              const MatrixF &localXfm )
{
   btCapsuleShape *shape = new btCapsuleShape( radius, height );
   shape->setMargin( 0.01f );
   _addShape( shape, localXfm );
}

bool BtCollision::addConvex(  const Point3F *points, 
                              U32 count,
                              const MatrixF &localXfm )
{
   btConvexHullShape *shape = new btConvexHullShape( (btScalar*)points, count, sizeof( Point3F ) );
   shape->setMargin( 0.01f );
   _addShape( shape, localXfm );
   return true;
}

bool BtCollision::addTriangleMesh(  const Point3F *vert,
                                    U32 vertCount,
                                    const U32 *index,
                                    U32 triCount,
                                    const MatrixF &localXfm )
{
   // Setup the interface for loading the triangles.
   btTriangleMesh *meshInterface = new btTriangleMesh( true, false );
   for ( ; triCount-- ; )
   {
      meshInterface->addTriangle(   btCast<btVector3>( vert[ *( index + 0 ) ] ),
                                    btCast<btVector3>( vert[ *( index + 1 ) ] ),
                                    btCast<btVector3>( vert[ *( index + 2 ) ] ),
                                    false );

      index += 3;
   }
   mMeshInterfaces.push_back( meshInterface );

   btBvhTriangleMeshShape *shape = new btBvhTriangleMeshShape( meshInterface, true, true );
   shape->setMargin( 0.01f );
   _addShape( shape, localXfm );
   
   return true;
}

bool BtCollision::addHeightfield(   const U16 *heights,
                                    const bool *holes,   // TODO: Bullet height fields do not support holes
                                    U32 blockSize,
                                    F32 metersPerSample,
                                    const MatrixF &localXfm )
{
   // We pass the absolute maximum and minimum of a U16 height
   // field and not the actual min and max.  This helps with
   // placement.
   const F32 heightScale = 0.03125f;
   const F32 minHeight = 0;
   const F32 maxHeight = 65535 * heightScale;

   btHeightfieldTerrainShape *shape = new btHeightfieldTerrainShape( blockSize, blockSize,
                                                                     (void*)heights,
                                                                     heightScale,
                                                                     minHeight, maxHeight,
                                                                     2, // Z up! 
                                                                     PHY_SHORT, 
                                                                     false );
   shape->setMargin( 0.01f );
   shape->setLocalScaling( btVector3( metersPerSample, metersPerSample, 1.0f ) );
   shape->setUseDiamondSubdivision( true );

   // The local axis of the heightfield is the exact center of
   // its bounds defined as...
   //
   // ( blockSize * samplesPerMeter, blockSize * samplesPerMeter, maxHeight ) / 2.0f
   //
   // So we create a local transform to move it to the min point 
   // of the bounds so it matched Torque terrain.
   Point3F offset(   (F32)blockSize * metersPerSample / 2.0f,
                     (F32)blockSize * metersPerSample / 2.0f,
                     maxHeight / 2.0f );

   // And also bump it by half a sample square size.
   offset.x -= metersPerSample / 2.0f;
   offset.y -= metersPerSample / 2.0f;

   MatrixF offsetXfm( true );
   offsetXfm.setPosition( offset );

   _addShape( shape, offsetXfm );

   return true;
}
