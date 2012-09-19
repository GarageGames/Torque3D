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
#include "T3D/physics/physX/pxCollision.h"

#include "math/mPoint3.h"
#include "math/mMatrix.h"
#include "T3D/physics/physX/px.h"
#include "T3D/physics/physX/pxCasts.h"
#include "T3D/physics/physX/pxWorld.h"
#include "T3D/physics/physX/pxStream.h"


PxCollision::PxCollision() 
{
}

PxCollision::~PxCollision()
{
   // We may be deleteting SDK data... so make 
   // sure we have the the scene write lock.
   PxWorld::releaseWriteLocks();

   for ( U32 i=0; i < mColShapes.size(); i++ )
   {
      // Check for special types which need cleanup.
      NxShapeDesc *desc = mColShapes[i];

      if ( desc->getType() == NX_SHAPE_CONVEX )
         gPhysicsSDK->releaseConvexMesh( *((NxConvexShapeDesc*)desc)->meshData );
      else if ( desc->getType() == NX_SHAPE_MESH )
         gPhysicsSDK->releaseTriangleMesh( *((NxTriangleMeshShapeDesc*)desc)->meshData );
      else if ( desc->getType() == NX_SHAPE_HEIGHTFIELD )
         gPhysicsSDK->releaseHeightField( *((NxHeightFieldShapeDesc*)desc)->heightField );

      // Delete the descriptor.
      delete desc;
   }

   mColShapes.clear();
}

void PxCollision::addPlane( const PlaneF &plane )
{
   NxBoxShapeDesc *desc = new NxBoxShapeDesc;
   desc->skinWidth = 0.01f;
   desc->dimensions.set( 10000.0f, 10000.0f, 100.0f );
   desc->localPose.t.z = -100.0f;
   
   // TODO: Fix rotation to match plane normal!
   //boxDesc->localPose.M.setColumn( 0, NxVec3( plane.x, plane.y, plane.z ) );
   //boxDesc->localPose.M.setColumn( 1, NxVec3( plane.x, plane.y, plane.z ) );
   //boxDesc->localPose.M.setColumn( 2, NxVec3( plane.x, plane.y, plane.z ) );

   mColShapes.push_back( desc );
}

void PxCollision::addBox(  const Point3F &halfWidth,
                           const MatrixF &localXfm )
{
   NxBoxShapeDesc *desc = new NxBoxShapeDesc;
   desc->skinWidth = 0.01f;
   desc->dimensions.set( halfWidth.x, halfWidth.y, halfWidth.z );
   desc->localPose.setRowMajor44( localXfm );
   mColShapes.push_back( desc );
}

void PxCollision::addSphere(  F32 radius,
                              const MatrixF &localXfm )
{
   NxSphereShapeDesc *desc = new NxSphereShapeDesc;
   desc->skinWidth = 0.01f;
   desc->radius = radius;
   desc->localPose.setRowMajor44( localXfm );
   mColShapes.push_back( desc );
}

void PxCollision::addCapsule( F32 radius,
                              F32 height,
                              const MatrixF &localXfm )
{
   NxCapsuleShapeDesc *desc = new NxCapsuleShapeDesc;
   desc->skinWidth = 0.01f;
   desc->radius = radius;
   desc->height = height;
   desc->localPose.setRowMajor44( localXfm );
   mColShapes.push_back( desc );
}

bool PxCollision::addConvex(  const Point3F *points, 
                              U32 count,
                              const MatrixF &localXfm )
{
   // Mesh cooking requires that both 
   // scenes not be write locked!
   PxWorld::releaseWriteLocks();

   NxCookingInterface *cooker = PxWorld::getCooking();
   cooker->NxInitCooking();

   NxConvexMeshDesc meshDesc;
   meshDesc.numVertices                = count;
   meshDesc.pointStrideBytes           = sizeof(Point3F);
   meshDesc.points                     = points;
   meshDesc.flags                      = NX_CF_COMPUTE_CONVEX | NX_CF_INFLATE_CONVEX;

   // Cook it!
   NxCookingParams params;
   #ifdef TORQUE_OS_XENON 
      params.targetPlatform = PLATFORM_XENON;
   #else
      params.targetPlatform = PLATFORM_PC;
   #endif
   params.skinWidth = 0.01f;
   params.hintCollisionSpeed = true;
   cooker->NxSetCookingParams( params );

   PxMemStream stream;	
   bool cooked = cooker->NxCookConvexMesh( meshDesc, stream );
   cooker->NxCloseCooking();

   if ( !cooked )
      return false;

   stream.resetPosition();
   NxConvexMesh *meshData = gPhysicsSDK->createConvexMesh( stream );
   if ( !meshData )
      return false;

   NxConvexShapeDesc *desc = new NxConvexShapeDesc;
   desc->skinWidth = 0.01f;
   desc->meshData = meshData;
   desc->localPose.setRowMajor44( localXfm );
   mColShapes.push_back( desc );

   return true;
}

bool PxCollision::addTriangleMesh(  const Point3F *vert,
                                    U32 vertCount,
                                    const U32 *index,
                                    U32 triCount,
                                    const MatrixF &localXfm )
{
   // Mesh cooking requires that both 
   // scenes not be write locked!
   PxWorld::releaseWriteLocks();

   NxCookingInterface *cooker = PxWorld::getCooking();
   cooker->NxInitCooking();

   NxTriangleMeshDesc meshDesc;
   meshDesc.numVertices                = vertCount;
   meshDesc.numTriangles               = triCount;
   meshDesc.pointStrideBytes           = sizeof(Point3F);
   meshDesc.triangleStrideBytes        = 3*sizeof(U32);
   meshDesc.points                     = vert;
   meshDesc.triangles                  = index;
   meshDesc.flags                      = NX_MF_FLIPNORMALS;

   // Cook it!
   NxCookingParams params;
   #ifdef TORQUE_OS_XENON 
      params.targetPlatform = PLATFORM_XENON;
   #else
      params.targetPlatform = PLATFORM_PC;
   #endif
   params.skinWidth = 0.01f;
   params.hintCollisionSpeed = true;
   cooker->NxSetCookingParams( params );
   
   PxMemStream stream;	
   bool cooked = cooker->NxCookTriangleMesh( meshDesc, stream );
   cooker->NxCloseCooking();
   if ( !cooked )
      return false;

   stream.resetPosition();
   NxTriangleMesh *meshData = gPhysicsSDK->createTriangleMesh( stream );
   if ( !meshData )
      return false;

   NxTriangleMeshShapeDesc *desc = new NxTriangleMeshShapeDesc;
   desc->skinWidth = 0.01f;
   desc->meshData = meshData;
   desc->localPose.setRowMajor44( localXfm );
   mColShapes.push_back( desc );

   return true;
}

bool PxCollision::addHeightfield(   const U16 *heights,
                                    const bool *holes,
                                    U32 blockSize,
                                    F32 metersPerSample,
                                    const MatrixF &localXfm )
{
   // Since we're creating SDK level data we
   // have to have access to all active worlds.
   PxWorld::releaseWriteLocks();

   // Init the heightfield description.
   NxHeightFieldDesc heightFieldDesc;    
   heightFieldDesc.nbColumns           = blockSize;   
   heightFieldDesc.nbRows              = blockSize; 
   heightFieldDesc.thickness           = -10.0f;
   heightFieldDesc.convexEdgeThreshold = 0;

   // Allocate the samples.
   heightFieldDesc.samples       = new NxU32[ blockSize * blockSize ];
   heightFieldDesc.sampleStride  = sizeof(NxU32);   
   NxU8 *currentByte = (NxU8*)heightFieldDesc.samples;

   for ( U32 row = 0; row < blockSize; row++ )        
   {  
      const U32 tess = ( row + 1 ) % 2;

      for ( U32 column = 0; column < blockSize; column++ )            
      {
         NxHeightFieldSample *currentSample = (NxHeightFieldSample*)currentByte;

         U32 index = ( blockSize - row - 1 ) + ( column * blockSize );
         currentSample->height = heights[ index ];

         if ( holes && holes[ getMax( (S32)index - 1, 0 ) ] )     // row index for holes adjusted so PhysX collision shape better matches rendered terrain
         {
            currentSample->materialIndex0 = 0;
            currentSample->materialIndex1 = 0;
         }
         else
         {
            currentSample->materialIndex0 = 1; //materialIds[0];
            currentSample->materialIndex1 = 1; //materialIds[0];
         }

         currentSample->tessFlag = ( column + tess ) % 2;    

         currentByte += heightFieldDesc.sampleStride;     
      }
   }

   // Build it.
   NxHeightFieldShapeDesc *desc = new NxHeightFieldShapeDesc;
   desc->heightField = gPhysicsSDK->createHeightField( heightFieldDesc );

   // Destroy the temp sample array.
   delete [] heightFieldDesc.samples;   

   // TerrainBlock uses a 11.5 fixed point height format
   // giving it a maximum height range of 0 to 2048.
   desc->heightScale = 0.03125f;

   desc->rowScale = metersPerSample;  
   desc->columnScale = metersPerSample;  
   desc->materialIndexHighBits = 0;   
   desc->skinWidth = 0.01f;

   // Use the local pose to align the heightfield
   // to what Torque will expect.
   NxMat33 rotX;
   rotX.rotX( Float_HalfPi );
   NxMat33 rotZ;
   rotZ.rotZ( Float_Pi );
   NxMat34 rot;
   rot.M.multiply( rotZ, rotX );
   rot.t.set( ( blockSize - 1 ) * metersPerSample, 0, 0 );   
   desc->localPose = rot;

   mColShapes.push_back( desc );
   return true;
}
