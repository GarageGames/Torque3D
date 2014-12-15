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
#include "T3D/physics/physx3/px3Collision.h"

#include "math/mPoint3.h"
#include "math/mMatrix.h"
#include "T3D/physics/physx3/px3.h"
#include "T3D/physics/physx3/px3Casts.h"
#include "T3D/physics/physx3/px3World.h"
#include "T3D/physics/physx3/px3Stream.h"


Px3Collision::Px3Collision()
{
}

Px3Collision::~Px3Collision()
{
	
	for ( U32 i=0; i < mColShapes.size(); i++ )
	{
		Px3CollisionDesc *desc = mColShapes[i];
		delete desc->pGeometry;
		// Delete the descriptor.
		delete desc;
	}

	mColShapes.clear();
}

void Px3Collision::addPlane( const PlaneF &plane )
{
	physx::PxVec3 pos = px3Cast<physx::PxVec3>(plane.getPosition());
	Px3CollisionDesc *desc = new Px3CollisionDesc;
   desc->pGeometry = new physx::PxPlaneGeometry();
   desc->pose = physx::PxTransform(pos, physx::PxQuat(physx::PxHalfPi, physx::PxVec3(0.0f, -1.0f, 0.0f)));
	mColShapes.push_back(desc);
}

void Px3Collision::addBox( const Point3F &halfWidth,const MatrixF &localXfm )
{
	Px3CollisionDesc *desc = new Px3CollisionDesc;
	desc->pGeometry = new physx::PxBoxGeometry(px3Cast<physx::PxVec3>(halfWidth));
	desc->pose = px3Cast<physx::PxTransform>(localXfm);
	mColShapes.push_back(desc);
}

void Px3Collision::addSphere(  F32 radius,
                              const MatrixF &localXfm )
{
	Px3CollisionDesc *desc = new Px3CollisionDesc;
	desc->pGeometry = new physx::PxSphereGeometry(radius);
	desc->pose = px3Cast<physx::PxTransform>(localXfm);
	mColShapes.push_back(desc);
}

void Px3Collision::addCapsule( F32 radius,
                              F32 height,
                              const MatrixF &localXfm )
{
	Px3CollisionDesc *desc = new Px3CollisionDesc;
	desc->pGeometry = new physx::PxCapsuleGeometry(radius,height*0.5);//uses half height
	desc->pose = px3Cast<physx::PxTransform>(localXfm);
	mColShapes.push_back(desc);
}

bool Px3Collision::addConvex(  const Point3F *points, 
                              U32 count,
                              const MatrixF &localXfm )
{
	physx::PxCooking *cooking = Px3World::getCooking();
	physx::PxConvexMeshDesc convexDesc;
	convexDesc.points.data = points;
	convexDesc.points.stride = sizeof(Point3F);
	convexDesc.points.count = count;
	convexDesc.flags = physx::PxConvexFlag::eFLIPNORMALS|physx::PxConvexFlag::eCOMPUTE_CONVEX | physx::PxConvexFlag::eINFLATE_CONVEX;

	Px3MemOutStream stream;
	if(!cooking->cookConvexMesh(convexDesc,stream))
		return false;

	physx::PxConvexMesh* convexMesh;
	Px3MemInStream in(stream.getData(), stream.getSize());
	convexMesh = gPhysics3SDK->createConvexMesh(in);

	Px3CollisionDesc *desc = new Px3CollisionDesc;
   physx::PxVec3 scale = px3Cast<physx::PxVec3>(localXfm.getScale());
   physx::PxQuat rotation = px3Cast<physx::PxQuat>(QuatF(localXfm));
   physx::PxMeshScale meshScale(scale,rotation);
	desc->pGeometry = new physx::PxConvexMeshGeometry(convexMesh,meshScale);
	desc->pose = px3Cast<physx::PxTransform>(localXfm);
	mColShapes.push_back(desc);
	return true;
}

bool Px3Collision::addTriangleMesh(  const Point3F *vert,
                                    U32 vertCount,
                                    const U32 *index,
                                    U32 triCount,
                                    const MatrixF &localXfm )
{
	physx::PxCooking *cooking = Px3World::getCooking();
	physx::PxTriangleMeshDesc meshDesc;
	meshDesc.points.count = vertCount;
	meshDesc.points.data = vert;
	meshDesc.points.stride = sizeof(Point3F);

	meshDesc.triangles.count = triCount;
	meshDesc.triangles.data = index;
	meshDesc.triangles.stride = 3*sizeof(U32);
	meshDesc.flags = physx::PxMeshFlag::eFLIPNORMALS;

	Px3MemOutStream stream;
	if(!cooking->cookTriangleMesh(meshDesc,stream))
		return false;

	physx::PxTriangleMesh *mesh;
	Px3MemInStream in(stream.getData(), stream.getSize());
	mesh = gPhysics3SDK->createTriangleMesh(in);

	Px3CollisionDesc *desc = new Px3CollisionDesc;
	desc->pGeometry = new physx::PxTriangleMeshGeometry(mesh);
	desc->pose = px3Cast<physx::PxTransform>(localXfm);
	mColShapes.push_back(desc);
	return true;
}

bool Px3Collision::addHeightfield(   const U16 *heights,
                                    const bool *holes,
                                    U32 blockSize,
                                    F32 metersPerSample,
                                    const MatrixF &localXfm )
{
	const F32 heightScale = 0.03125f;
	physx::PxHeightFieldSample* samples = (physx::PxHeightFieldSample*) new physx::PxHeightFieldSample[blockSize*blockSize];
	memset(samples,0,blockSize*blockSize*sizeof(physx::PxHeightFieldSample));

	physx::PxHeightFieldDesc heightFieldDesc;
	heightFieldDesc.nbColumns = blockSize;
	heightFieldDesc.nbRows = blockSize;
	heightFieldDesc.thickness = -10.f;
	heightFieldDesc.convexEdgeThreshold = 0;
	heightFieldDesc.format = physx::PxHeightFieldFormat::eS16_TM;
	heightFieldDesc.samples.data = samples;
	heightFieldDesc.samples.stride = sizeof(physx::PxHeightFieldSample);

	physx::PxU8 *currentByte = (physx::PxU8*)heightFieldDesc.samples.data;
   for ( U32 row = 0; row < blockSize; row++ )        
   {  
      const U32 tess = ( row + 1 ) % 2;

      for ( U32 column = 0; column < blockSize; column++ )            
      {
         physx::PxHeightFieldSample *currentSample = (physx::PxHeightFieldSample*)currentByte;

         U32 index = ( blockSize - row - 1 ) + ( column * blockSize );
         currentSample->height = (physx::PxI16)heights[ index ];


         if ( holes && holes[ getMax( (S32)index - 1, 0 ) ] )     // row index for holes adjusted so PhysX collision shape better matches rendered terrain
         {
            currentSample->materialIndex0 = physx::PxHeightFieldMaterial::eHOLE;
            currentSample->materialIndex1 = physx::PxHeightFieldMaterial::eHOLE;
         }
         else
         {
            currentSample->materialIndex0 = 0;
            currentSample->materialIndex1 = 0;
         }

		int flag = ( column + tess ) % 2;
		if(flag)
			currentSample->clearTessFlag();
		else
			currentSample->setTessFlag();

         currentByte += heightFieldDesc.samples.stride;    
      }
   }

	physx::PxHeightField * hf = gPhysics3SDK->createHeightField(heightFieldDesc);
	physx::PxHeightFieldGeometry *geom = new physx::PxHeightFieldGeometry(hf,physx::PxMeshGeometryFlags(),heightScale,metersPerSample,metersPerSample);
		
	physx::PxTransform pose= physx::PxTransform(physx::PxQuat(Float_HalfPi, physx::PxVec3(1, 0, 0 )));
	physx::PxTransform pose1= physx::PxTransform(physx::PxQuat(Float_Pi, physx::PxVec3(0, 0, 1 )));
	physx::PxTransform pose2 = pose1 * pose;
	pose2.p = physx::PxVec3(( blockSize - 1 ) * metersPerSample, 0, 0 );
	Px3CollisionDesc *desc = new Px3CollisionDesc;
	desc->pGeometry = geom;
	desc->pose = pose2;

	mColShapes.push_back(desc);

   SAFE_DELETE(samples);
	return true;
}
