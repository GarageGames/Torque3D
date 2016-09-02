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

#include "console/consoleTypes.h"
#include "core/resourceManager.h"
#include "ts/tsShapeConstruct.h"
#include "console/engineAPI.h"

// define macros required for ConvexDecomp headers
#if defined( _WIN32 ) && !defined( WIN32 )
#define WIN32
#elif defined( __MACOSX__ ) && !defined( APPLE )
#define APPLE
#endif

#include "convexDecomp/NvFloatMath.h"
#include "convexDecomp/NvConvexDecomposition.h"
#include "convexDecomp/NvStanHull.h"

//-----------------------------------------------------------------------------
static const Point3F sFacePlanes[] = {
   Point3F( -1.0f,  0.0f,  0.0f ),
   Point3F(  1.0f,  0.0f,  0.0f ),
   Point3F(  0.0f, -1.0f,  0.0f ),
   Point3F(  0.0f,  1.0f,  0.0f ),
   Point3F(  0.0f,  0.0f, -1.0f ),
   Point3F(  0.0f,  0.0f,  1.0f ),
};

static const Point3F sXEdgePlanes[] = {
   Point3F( 0.0f, -0.7071f, -0.7071f ),
   Point3F( 0.0f, -0.7071f,  0.7071f ),
   Point3F( 0.0f,  0.7071f, -0.7071f ),
   Point3F( 0.0f,  0.7071f,  0.7071f ),
};

static const Point3F sYEdgePlanes[] = {
   Point3F( -0.7071f, 0.0f, -0.7071f ),
   Point3F( -0.7071f, 0.0f,  0.7071f ),
   Point3F(  0.7071f, 0.0f, -0.7071f ),
   Point3F(  0.7071f, 0.0f,  0.7071f ),
};

static const Point3F sZEdgePlanes[] = {
   Point3F( -0.7071f, -0.7071f, 0.0f ),
   Point3F( -0.7071f,  0.7071f, 0.0f ),
   Point3F(  0.7071f, -0.7071f, 0.0f ),
   Point3F(  0.7071f,  0.7071f, 0.0f ),
};

static const Point3F sCornerPlanes[] = {
   Point3F( -0.5774f, -0.5774f, -0.5774f ),
   Point3F( -0.5774f, -0.5774f,  0.5774f ),
   Point3F( -0.5774f,  0.5774f, -0.5774f ),
   Point3F( -0.5774f,  0.5774f,  0.5774f ),
   Point3F(  0.5774f, -0.5774f, -0.5774f ),
   Point3F(  0.5774f, -0.5774f,  0.5774f ),
   Point3F(  0.5774f,  0.5774f, -0.5774f ),
   Point3F(  0.5774f,  0.5774f,  0.5774f ),
};

//-----------------------------------------------------------------------------

/** A helper class for fitting primitives (Box, Sphere, Capsule) to a triangulated mesh */
struct PrimFit
{
   MatrixF     mBoxTransform;
   Point3F     mBoxSides;

   Point3F     mSphereCenter;
   F32         mSphereRadius;

   MatrixF     mCapTransform;
   F32         mCapRadius;
   F32         mCapHeight;

public:
   PrimFit() :
      mBoxTransform(true), mBoxSides(1,1,1),
      mSphereCenter(0,0,0), mSphereRadius(1),
      mCapTransform(true), mCapRadius(1), mCapHeight(1)
   {
   }

   inline F32 getBoxVolume() const { return mBoxSides.x * mBoxSides.y * mBoxSides.z; }
   inline F32 getSphereVolume() const { return 4.0f / 3.0f * M_PI * mPow( mSphereRadius, 3 ); }
   inline F32 getCapsuleVolume() const { return 2 * M_PI * mPow( mCapRadius, 2 ) * (4.0f / 3.0f * mCapRadius + mCapHeight); }

   void fitBox( U32 vertCount, const F32* verts )
   {
      CONVEX_DECOMPOSITION::fm_computeBestFitOBB( vertCount, verts, sizeof(F32)*3, (F32*)mBoxSides, (F32*)mBoxTransform );
      mBoxTransform.transpose();
   }

   void fitSphere( U32 vertCount, const F32* verts )
   {
      mSphereRadius = CONVEX_DECOMPOSITION::fm_computeBestFitSphere( vertCount, verts, sizeof(F32)*3, (F32*)mSphereCenter );
   }

   void fitCapsule( U32 vertCount, const F32* verts )
   {
      CONVEX_DECOMPOSITION::fm_computeBestFitCapsule( vertCount, verts, sizeof(F32)*3, mCapRadius, mCapHeight, (F32*)mCapTransform );
      mCapTransform.transpose();
   }
};

class MeshFit
{
public:
   enum eMeshType
   {
      Box = 0,
      Sphere,
      Capsule,
      Hull,
   };

   struct Mesh
   {
      eMeshType   type;
      MatrixF     transform;
      TSMesh      *tsmesh;
   };

private:
   TSShape           *mShape;    ///!< Source geometry shape
   Vector<Point3F>   mVerts;     ///!< Source geometry verts (all meshes)
   Vector<U32>       mIndices;   ///!< Source geometry indices (triangle lists, all meshes)

   bool              mIsReady;   ///!< Flag indicating whether we are ready to fit/create meshes

   Vector<Mesh>      mMeshes;    ///!< Fitted meshes

   void addSourceMesh( const TSShape::Object& obj, const TSMesh* mesh );
   TSMesh* initMeshFromFile( const String& filename ) const;
   TSMesh* createTriMesh( F32* verts, S32 numVerts, U32* indices, S32 numTris ) const;
   F32 maxDot( const VectorF& v ) const;
   void fitK_DOP( const Vector<Point3F>& planes );

public:
   MeshFit(TSShape* shape) : mShape(shape), mIsReady(false) { }

   void setReady() { mIsReady = true; }
   bool isReady() const { return mIsReady; }

   void initSourceGeometry( const String& target );

   S32 getMeshCount() const { return mMeshes.size(); }
   Mesh* getMesh( S32 index ) { return &(mMeshes[index]); }

   // Box
   void addBox( const Point3F& sides, const MatrixF& mat );
   void fitOBB();

   // Sphere
   void addSphere( F32 radius, const Point3F& center );
   void fitSphere();

   // Capsule
   void addCapsule( F32 radius, F32 height, const MatrixF& mat );
   void fitCapsule();

   // k-DOP
   void fit10_DOP_X();
   void fit10_DOP_Y();
   void fit10_DOP_Z();
   void fit18_DOP();
   void fit26_DOP();

   // Convex Hulls
   void fitConvexHulls( U32 depth, F32 mergeThreshold, F32 concavityThreshold, U32 maxHullVerts,
                        F32 boxMaxError, F32 sphereMaxError, F32 capsuleMaxError );
};


void MeshFit::initSourceGeometry( const String& target )
{
   mMeshes.clear();
   mVerts.clear();
   mIndices.clear();

   if ( target.equal( "bounds", String::NoCase ) )
   {
      // Add all geometry in the highest detail level
      S32 dl = 0;
      S32 ss = mShape->details[dl].subShapeNum;
      if ( ss < 0 )
         return;

      S32 od = mShape->details[dl].objectDetailNum;
      S32 start = mShape->subShapeFirstObject[ss];
      S32 end   = start + mShape->subShapeNumObjects[ss];

      for ( S32 i = start; i < end; i++ )
      {
         const TSShape::Object &obj = mShape->objects[i];
         const TSMesh* mesh = ( od < obj.numMeshes ) ? mShape->meshes[obj.startMeshIndex + od] : NULL;
         if ( mesh )
            addSourceMesh( obj, mesh );
      }
   }
   else
   {
      // Add highest detail mesh from this object
      S32 objIndex = mShape->findObject( target );
      if ( objIndex == -1 )
         return;

      const TSShape::Object &obj = mShape->objects[objIndex];
      for ( S32 i = 0; i < obj.numMeshes; i++ )
      {
         const TSMesh* mesh = mShape->meshes[obj.startMeshIndex + i];
         if ( mesh )
         {
            addSourceMesh( obj, mesh );
            break;
         }
      }
   }

   mIsReady = ( !mVerts.empty() && !mIndices.empty() );
}

void MeshFit::addSourceMesh( const TSShape::Object& obj, const TSMesh* mesh )
{
   // Add indices
   S32 indicesBase = mIndices.size();
   for ( S32 i = 0; i < mesh->primitives.size(); i++ )
   {
      const TSDrawPrimitive& draw = mesh->primitives[i];
      if ( (draw.matIndex & TSDrawPrimitive::TypeMask) == TSDrawPrimitive::Triangles )
      {
         mIndices.merge( &mesh->indices[draw.start], draw.numElements );
      }
      else
      {
         U32 idx0 = mesh->indices[draw.start + 0];
         U32 idx1;
         U32 idx2 = mesh->indices[draw.start + 1];
         U32 *nextIdx = &idx1;
         for ( S32 j = 2; j < draw.numElements; j++ )
         {
            *nextIdx = idx2;
            nextIdx = (U32*) ( (dsize_t)nextIdx ^ (dsize_t)&idx0 ^ (dsize_t)&idx1);
            idx2 = mesh->indices[draw.start + j];
            if ( idx0 == idx1 || idx0 == idx2 || idx1 == idx2 )
               continue;

            mIndices.push_back( idx0 );
            mIndices.push_back( idx1 );
            mIndices.push_back( idx2 );
         }
      }
   }

   // Offset indices for already added verts
   for ( S32 j = indicesBase; j < mIndices.size(); j++ )
      mIndices[j] += mVerts.size();

   // Add verts
   S32 count, stride;
   U8* pVert;

   if ( mesh->mVertexData.isReady() && mesh->verts.size() == 0 )
   {
      count = mesh->mVertexData.size();
      stride = mesh->mVertexData.vertSize();
      pVert = (U8*)mesh->mVertexData.address();
   }
   else
   {
      count = mesh->verts.size();
      stride = sizeof(Point3F);
      pVert = (U8*)mesh->verts.address();
   }

   MatrixF objMat;
   mShape->getNodeWorldTransform( obj.nodeIndex, &objMat );

   mVerts.reserve( mVerts.size() + count );
   for ( S32 j = 0; j < count; j++, pVert += stride )
   {
      mVerts.increment();
      objMat.mulP( *(Point3F*)pVert, &mVerts.last() );
   }
}

TSMesh* MeshFit::initMeshFromFile( const String& filename ) const
{
   // Open the source shape file and make a copy of the mesh
   Resource<TSShape> hShape = ResourceManager::get().load(filename);
   if (!bool(hShape) || !((TSShape*)hShape)->meshes.size())
   {
      Con::errorf("TSShape::createMesh: Could not load source mesh from %s", filename.c_str());
      return NULL;
   }

   TSMesh* srcMesh = ((TSShape*)hShape)->meshes[0];
   return mShape->copyMesh( srcMesh );
}

TSMesh* MeshFit::createTriMesh( F32* verts, S32 numVerts, U32* indices, S32 numTris ) const
{
   TSMesh* mesh = mShape->copyMesh( NULL );
   mesh->numFrames = 1;
   mesh->numMatFrames = 1;
   mesh->vertsPerFrame = numVerts;
   mesh->setFlags(0);
   mesh->mNumVerts = numVerts;

   mesh->indices.reserve( numTris * 3 );
   for ( S32 i = 0; i < numTris; i++ )
   {
      mesh->indices.push_back( indices[i*3 + 0] );
      mesh->indices.push_back( indices[i*3 + 2] );
      mesh->indices.push_back( indices[i*3 + 1] );
   }

   mesh->verts.set( verts, numVerts );

   // Compute mesh normals
   mesh->norms.setSize( mesh->verts.size() );
   for (S32 iNorm = 0; iNorm < mesh->norms.size(); iNorm++)
      mesh->norms[iNorm] = Point3F::Zero;

   // Sum triangle normals for each vertex
   for (S32 iInd = 0; iInd < mesh->indices.size(); iInd += 3)
   {
      // Compute the normal for this triangle
      S32 idx0 = mesh->indices[iInd + 0];
      S32 idx1 = mesh->indices[iInd + 1];
      S32 idx2 = mesh->indices[iInd + 2];

      const Point3F& v0 = mesh->verts[idx0];
      const Point3F& v1 = mesh->verts[idx1];
      const Point3F& v2 = mesh->verts[idx2];

      Point3F n;
      mCross(v2 - v0, v1 - v0, &n);
      n.normalize();    // remove this to use 'weighted' normals (large triangles will have more effect)

      mesh->norms[idx0] += n;
      mesh->norms[idx1] += n;
      mesh->norms[idx2] += n;
   }

   // Normalize the vertex normals (this takes care of averaging the triangle normals)
   for (S32 iNorm = 0; iNorm < mesh->norms.size(); iNorm++)
      mesh->norms[iNorm].normalize();

   // Set some dummy UVs
   mesh->tverts.setSize( numVerts );
   for ( S32 j = 0; j < mesh->tverts.size(); j++ )
      mesh->tverts[j].set( 0, 0 );

   // Add a single triangle-list primitive
   mesh->primitives.increment();
   mesh->primitives.last().start = 0;
   mesh->primitives.last().numElements = mesh->indices.size();
   mesh->primitives.last().matIndex =  TSDrawPrimitive::Triangles |
                                       TSDrawPrimitive::Indexed |
                                       TSDrawPrimitive::NoMaterial;

   mesh->createTangents( mesh->verts, mesh->norms );
   mesh->encodedNorms.set( NULL,0 );

   return mesh;
}

F32 MeshFit::maxDot( const VectorF& v ) const
{
   F32 maxDot = -FLT_MAX;
   for ( S32 i = 0; i < mVerts.size(); i++ )
      maxDot = getMax( maxDot, mDot( v, mVerts[i] ) );
   return maxDot;
}

//---------------------------
// Best-fit oriented bounding box
void MeshFit::addBox( const Point3F& sides, const MatrixF& mat )
{
   TSMesh* mesh = initMeshFromFile( TSShapeConstructor::getCubeShapePath() );
   if ( !mesh )
      return;

   if (mesh->verts.size() > 0)
   {
      for (S32 i = 0; i < mesh->verts.size(); i++)
      {
         Point3F v = mesh->verts[i];
         v.convolve(sides);
         mesh->verts[i] = v;
      }

      mesh->mVertexData.setReady(false);
   }
   else
   {
      for (S32 i = 0; i < mesh->mVertexData.size(); i++)
      {
         TSMesh::__TSMeshVertexBase &vdata = mesh->mVertexData.getBase(i);
         Point3F v = vdata.vert();
         v.convolve(sides);
         vdata.vert(v);
      }
   }

   mesh->computeBounds();

   mMeshes.increment();
   mMeshes.last().type = MeshFit::Box;
   mMeshes.last().transform = mat;
   mMeshes.last().tsmesh = mesh;
}

void MeshFit::fitOBB()
{
   PrimFit primFitter;
   primFitter.fitBox( mVerts.size(), (F32*)mVerts.address() );
   addBox( primFitter.mBoxSides, primFitter.mBoxTransform );
}

//---------------------------
// Best-fit sphere
void MeshFit::addSphere( F32 radius, const Point3F& center )
{
   TSMesh* mesh = initMeshFromFile( TSShapeConstructor::getSphereShapePath() );
   if ( !mesh )
      return;

   for ( S32 i = 0; i < mesh->mVertexData.size(); i++ )
   {
      TSMesh::__TSMeshVertexBase &vdata = mesh->mVertexData.getBase(i);
      Point3F v = vdata.vert();
      vdata.vert( v * radius );
   }
   mesh->computeBounds();

   mMeshes.increment();
   MeshFit::Mesh& lastMesh = mMeshes.last();
   lastMesh.type = MeshFit::Sphere;
   lastMesh.transform.identity();
   lastMesh.transform.setPosition(center);
   lastMesh.tsmesh = mesh;
}

void MeshFit::fitSphere()
{
   PrimFit primFitter;
   primFitter.fitSphere( mVerts.size(), (F32*)mVerts.address() );
   addSphere( primFitter.mSphereRadius, primFitter.mSphereCenter );
}

//---------------------------
// Best-fit capsule
void MeshFit::addCapsule( F32 radius, F32 height, const MatrixF& mat )
{
   TSMesh* mesh = initMeshFromFile( TSShapeConstructor::getCapsuleShapePath() );
   if ( !mesh )
      return;

   // Translate and scale the mesh verts
   height = mMax( 0, height );
   F32 offset = ( height / ( 2 * radius ) ) - 0.5f;
   for ( S32 i = 0; i < mesh->mVertexData.size(); i++ )
   {
      Point3F v = mesh->mVertexData.getBase(i).vert();
      v.y += ( ( v.y > 0 ) ? offset : -offset );
      mesh->mVertexData.getBase(i).vert( v * radius );
   }
   mesh->computeBounds();

   mMeshes.increment();
   mMeshes.last().type = MeshFit::Capsule;
   mMeshes.last().transform = mat;
   mMeshes.last().tsmesh = mesh;
}

void MeshFit::fitCapsule()
{
   PrimFit primFitter;
   primFitter.fitCapsule( mVerts.size(), (F32*)mVerts.address() );
   addCapsule( primFitter.mCapRadius, primFitter.mCapHeight, primFitter.mCapTransform );
}

//---------------------------
// Best-fit k-discrete-oriented-polytope (where k is the number of axis-aligned planes)

// All faces + 4 edges (aligned to X axis) of the unit cube
void MeshFit::fit10_DOP_X()
{
   Vector<Point3F> planes;
   planes.setSize( 10 );
   dCopyArray( planes.address(), sFacePlanes, 6 );
   dCopyArray( planes.address()+6, sXEdgePlanes, 4 );
   fitK_DOP( planes );
}

// All faces + 4 edges (aligned to Y axis)  of the unit cube
void MeshFit::fit10_DOP_Y()
{
   Vector<Point3F> planes;
   planes.setSize( 10 );
   dCopyArray( planes.address(), sFacePlanes, 6 );
   dCopyArray( planes.address()+6, sYEdgePlanes, 4 );
   fitK_DOP( planes );
}

// All faces + 4 edges (aligned to Z axis)  of the unit cube
void MeshFit::fit10_DOP_Z()
{
   Vector<Point3F> planes;
   planes.setSize( 10 );
   dCopyArray( planes.address(), sFacePlanes, 6 );
   dCopyArray( planes.address()+6, sZEdgePlanes, 4 );
   fitK_DOP( planes );
}

// All faces and edges of the unit cube
void MeshFit::fit18_DOP()
{
   Vector<Point3F> planes;
   planes.setSize( 18 );
   dCopyArray( planes.address(), sFacePlanes, 6 );
   dCopyArray( planes.address()+6, sXEdgePlanes, 4 );
   dCopyArray( planes.address()+10, sYEdgePlanes, 4 );
   dCopyArray( planes.address()+14, sZEdgePlanes, 4 );
   fitK_DOP( planes );
}

// All faces, edges and corners of the unit cube
void MeshFit::fit26_DOP()
{
   Vector<Point3F> planes;
   planes.setSize( 26 );
   dCopyArray( planes.address(), sFacePlanes, 6 );
   dCopyArray( planes.address()+6, sXEdgePlanes, 4 );
   dCopyArray( planes.address()+10, sYEdgePlanes, 4 );
   dCopyArray( planes.address()+14, sZEdgePlanes, 4 );
   dCopyArray( planes.address()+18, sCornerPlanes, 8 );
   fitK_DOP( planes );
}

void MeshFit::fitK_DOP( const Vector<Point3F>& planes )
{
   // Push the planes up against the mesh
   Vector<F32> planeDs;
   for ( S32 i = 0; i < planes.size(); i++ )
      planeDs.push_back( maxDot( planes[i] ) );

   // Collect the intersection points of any 3 planes that lie inside
   // the maximum distances found above
   Vector<Point3F> points;
   for ( S32 i = 0; i < planes.size()-2; i++ )
   {
      for ( S32 j = i+1; j < planes.size()-1; j++ )
      {
         for ( S32 k = j+1; k < planes.size(); k++ )
         {
            Point3F v23 = mCross( planes[j], planes[k] );
            F32 denom = mDot( planes[i], v23 );
            if ( denom == 0 )
               continue;

            Point3F v31 = mCross( planes[k], planes[i] );
            Point3F v12 = mCross( planes[i], planes[j] );
            Point3F p = ( planeDs[i]*v23 + planeDs[j]*v31 + planeDs[k]*v12 ) / denom;

            // Ignore intersection points outside the volume
            // described by the planes
            bool addPoint = true;
            for ( S32 n = 0; n < planes.size(); n++ )
            {
               if ( ( mDot( p, planes[n] ) - planeDs[n] ) > 0.005f )
               {
                  addPoint = false;
                  break;
               }
            }

            if ( addPoint )
               points.push_back( p );
         }
      }
   }

   // Create a convex hull from the point set
   CONVEX_DECOMPOSITION::HullDesc hd;
   hd.mVcount 			= points.size();
   hd.mVertices 		= (F32*)points.address();
   hd.mVertexStride 	= sizeof(Point3F);
   hd.mMaxVertices 	= 64;
   hd.mSkinWidth		= 0.0f;

   CONVEX_DECOMPOSITION::HullLibrary hl;
   CONVEX_DECOMPOSITION::HullResult result;
   hl.CreateConvexHull( hd, result );

   // Create TSMesh from convex hull
   mMeshes.increment();
   MeshFit::Mesh& lastMesh = mMeshes.last();
   lastMesh.type = MeshFit::Hull;
   lastMesh.transform.identity();
   lastMesh.tsmesh = createTriMesh(result.mOutputVertices, result.mNumOutputVertices,
                              result.mIndices, result.mNumFaces );
   lastMesh.tsmesh->computeBounds();
}

//---------------------------
// Best-fit set of convex hulls
void MeshFit::fitConvexHulls( U32 depth, F32 mergeThreshold, F32 concavityThreshold, U32 maxHullVerts,
                              F32 boxMaxError, F32 sphereMaxError, F32 capsuleMaxError )
{
   const F32 SkinWidth      = 0.0f;
   const F32 SplitThreshold = 2.0f;

   CONVEX_DECOMPOSITION::iConvexDecomposition *ic = CONVEX_DECOMPOSITION::createConvexDecomposition();

   for ( S32 i = 0; i < mIndices.size(); i += 3 )
   {
      ic->addTriangle(  (F32*)mVerts[mIndices[i]],
                        (F32*)mVerts[mIndices[i+1]],
                        (F32*)mVerts[mIndices[i+2]] );
   }

   ic->computeConvexDecomposition(
      SkinWidth,
      depth,
      maxHullVerts,
      concavityThreshold,
      mergeThreshold,
      SplitThreshold,
      true,
      false,
      false );

   // Add a TSMesh for each hull
   for ( S32 i = 0; i < ic->getHullCount(); i++ )
   {
      CONVEX_DECOMPOSITION::ConvexHullResult result;
      ic->getConvexHullResult( i, result );

      eMeshType meshType = MeshFit::Hull;

      // Check if we can use a box, sphere or capsule primitive for this hull
      if (( boxMaxError > 0 ) || ( sphereMaxError > 0 ) || ( capsuleMaxError > 0 ))
      {
         // Compute error between actual mesh and fitted primitives
         F32 meshVolume = CONVEX_DECOMPOSITION::fm_computeMeshVolume( result.mVertices, result.mTcount, result.mIndices );
         PrimFit primFitter;

         F32 boxError = 100.0f, sphereError = 100.0f, capsuleError = 100.0f;
         if ( boxMaxError > 0 )
         {
            primFitter.fitBox( result.mVcount, result.mVertices );
            boxError = 100.0f * ( 1.0f - ( meshVolume / primFitter.getBoxVolume() ) );
         }
         if ( sphereMaxError > 0 )
         {
            primFitter.fitSphere( result.mVcount, result.mVertices );
            sphereError = 100.0f * ( 1.0f - ( meshVolume / primFitter.getSphereVolume() ) );
         }
         if ( capsuleMaxError > 0 )
         {
            primFitter.fitCapsule( result.mVcount, result.mVertices );
            capsuleError = 100.0f * ( 1.0f - ( meshVolume / primFitter.getCapsuleVolume() ) );
         }

         // Use the primitive type with smallest error less than the respective
         // max error, or Hull if none
         F32 minError = FLT_MAX;
         if ( ( boxError < boxMaxError ) && ( boxError < minError ) )
         {
            meshType = MeshFit::Box;
            minError = boxError;
         }
         if ( ( sphereError < sphereMaxError ) && ( sphereError < minError ) )
         {
            meshType = MeshFit::Sphere;
            minError = sphereError;
         }
         if ( ( capsuleError < capsuleMaxError ) && ( capsuleError < minError ) )
         {
            meshType = MeshFit::Capsule;
            minError = capsuleError;
         }

         if ( meshType == MeshFit::Box )
            addBox( primFitter.mBoxSides, primFitter.mBoxTransform );
         else if ( meshType == MeshFit::Sphere )
            addSphere( primFitter.mSphereRadius, primFitter.mSphereCenter );
         else if ( meshType == MeshFit::Capsule )
            addCapsule( primFitter.mCapRadius, primFitter.mCapHeight, primFitter.mCapTransform );
         // else fall through to Hull processing
      }

      if ( meshType == MeshFit::Hull )
      {
         // Create TSMesh from convex hull
         mMeshes.increment();
         MeshFit::Mesh& lastMesh = mMeshes.last();
         lastMesh.type = MeshFit::Hull;
         lastMesh.transform.identity();
         lastMesh.tsmesh = createTriMesh(result.mVertices, result.mVcount, result.mIndices, result.mTcount);
         lastMesh.tsmesh->computeBounds();
      }
   }

   CONVEX_DECOMPOSITION::releaseConvexDecomposition( ic );
}

//-----------------------------------------------------------------------------
DefineTSShapeConstructorMethod( addPrimitive, bool, ( const char* meshName, const char* type, const char* params, TransformF txfm, const char* nodeName ),,
   ( meshName, type, params, txfm, nodeName ), false,
   "Add a new mesh primitive to the shape.\n"
   "@param meshName full name (object name + detail size) of the new mesh. If "
      "no detail size is present at the end of the name, a value of 2 is used.<br>"
      "An underscore before the number at the end of the name will be interpreted as "
      "a negative sign. eg. \"MyMesh_4\" will be interpreted as \"MyMesh-4\".\n"
   "@param type one of: \"box\", \"sphere\", \"capsule\"\n"
   "@param params mesh primitive parameters:\n"
      "<ul>"
         "<li>for box: \"size_x size_y size_z\"</li>"
         "<li>for sphere: \"radius\"</li>"
         "<li>for capsule: \"height radius\"</li>"
      "</ul>"
   "</ul>\n"
   "@param txfm local transform offset from the node for this mesh\n"
   "@param nodeName name of the node to attach the new mesh to (will change the "
   "object's node if adding a new mesh to an existing object)\n"
   "@return true if successful, false otherwise\n\n"
   "@tsexample\n"
   "%this.addMesh( \"Box4\", \"box\", \"2 4 2\", \"0 2 0 0 0 1 0\", \"eye\" );\n"
   "%this.addMesh( \"Sphere256\", \"sphere\", \"2\", \"0 0 0 0 0 1 0\", \"root\" );\n"
   "%this.addMesh( \"MyCapsule-1\", \"capsule\", \"2 5\", \"0 0 2 0 0 1 0\", \"base01\" );\n"
   "@endtsexample\n" )
{
   MeshFit fit( mShape );
   if ( !dStricmp( type, "box" ) )
   {
      // Parse box parameters
      Point3F sides;
      if ( dSscanf( params, "%g %g %g", &sides.x, &sides.y, &sides.z ) == 3 )
      {
         fit.addBox( sides, MatrixF::Identity );
         fit.setReady();
      }
   }
   else if ( !dStricmp( type, "sphere" ) )
   {
      // Parse sphere parameters
      F32 radius;
      if ( dSscanf( params, "%g", &radius ) == 1)
      {
         fit.addSphere( radius, Point3F::Zero );
         fit.setReady();
      }
   }
   else if ( !dStricmp( type, "capsule" ) )
   {
      // Parse capsule parameters
      F32 radius, height;
      if ( dSscanf( params, "%g %g", &radius, &height ) == 1)
      {
         fit.addCapsule( radius, height, MatrixF::Identity );
         fit.setReady();
      }
   }

   if ( !fit.isReady() )
   {
      Con::errorf( "TSShapeConstructor::addPrimitive: Invalid params: '%s' for type '%s'",
         params, type );
      return false;
   }

   TSMesh* mesh = fit.getMesh( 0 )->tsmesh;
   MatrixF mat( txfm.getMatrix() );

   // Transform the mesh vertices
   if ( mesh->mVertexData.isReady() && mesh->verts.size() == 0 )
   {
      for (S32 i = 0; i < mesh->mVertexData.size(); i++)
      {
         TSMesh::__TSMeshVertexBase &vdata = mesh->mVertexData.getBase(i);
         Point3F v;
         mat.mulP( vdata.vert(), &v );
         vdata.vert( v );
      }
   }
   else
   {
      for (S32 i = 0; i < mesh->verts.size(); i++)
      {
         Point3F v(mesh->verts[i]);
         mat.mulP( v, &mesh->verts[i] );
      }
   }

   // Add the mesh to the shape at the right node
   mShape->addMesh( mesh, meshName );

   S32 dummy;
   String objName = String::GetTrailingNumber( meshName, dummy );
   setObjectNode( objName, nodeName );

   mShape->init();

   ADD_TO_CHANGE_SET();
   return true;
}}

DefineTSShapeConstructorMethod( addCollisionDetail, bool, ( S32 size, const char* type, const char* target, S32 depth, F32 merge, F32 concavity, S32 maxVerts, F32 boxMaxError, F32 sphereMaxError, F32 capsuleMaxError ), ( 4, 30, 30, 32, 0, 0, 0 ),
   ( size, type, target, depth, merge, concavity, maxVerts, boxMaxError, sphereMaxError, capsuleMaxError ), false,
   "Autofit a mesh primitive or set of convex hulls to the shape geometry. Hulls "
   "may optionally be converted to boxes, spheres and/or capsules based on their "
   "volume.\n"
   "@param size size for this detail level\n"
   "@param type one of: box, sphere, capsule, 10-dop x, 10-dop y, 10-dop z, 18-dop, "
      "26-dop, convex hulls. See the Shape Editor documentation for more details "
      "about these types.\n"
   "@param target geometry to fit collision mesh(es) to; either \"bounds\" (for the "
      "whole shape), or the name of an object in the shape\n"
   "@param depth maximum split recursion depth (hulls only)\n"
   "@param merge volume % threshold used to merge hulls together (hulls only)\n"
   "@param concavity volume % threshold used to detect concavity (hulls only)\n"
   "@param maxVerts maximum number of vertices per hull (hulls only)\n"
   "@param boxMaxError max % volume difference for a hull to be converted to a "
      "box (hulls only)\n"
   "@param sphereMaxError max % volume difference for a hull to be converted to "
      "a sphere (hulls only)\n"
   "@param capsuleMaxError max % volume difference for a hull to be converted to "
      "a capsule (hulls only)\n"
   "@return true if successful, false otherwise\n\n"
   "@tsexample\n"
   "%this.addCollisionDetail( -1, \"box\", \"bounds\" );\n"
   "%this.addCollisionDetail( -1, \"convex hulls\", \"bounds\", 4, 30, 30, 32, 0, 0, 0 );\n"
   "%this.addCollisionDetail( -1, \"convex hulls\", \"bounds\", 4, 30, 30, 32, 50, 50, 50 );\n"
   "@endtsexample\n" )
{
   MeshFit fit( mShape );
   fit.initSourceGeometry( target );
   if ( !fit.isReady() )
   {
      Con::errorf( "TSShapeConstructor::addCollisionDetail: Failed to initialise mesh fitter "
         "using target: %s", target );
      return false;
   }

   if ( !dStricmp( type, "box" ) )
      fit.fitOBB();
   else if ( !dStricmp( type, "sphere" ) )
      fit.fitSphere();
   else if ( !dStricmp( type, "capsule" ) )
      fit.fitCapsule();
   else if ( !dStricmp( type, "10-dop x" ) )
      fit.fit10_DOP_X();
   else if ( !dStricmp( type, "10-dop y" ) )
      fit.fit10_DOP_Y();
   else if ( !dStricmp( type, "10-dop z" ) )
      fit.fit10_DOP_Z();
   else if ( !dStricmp( type, "18-dop" ) )
      fit.fit18_DOP();
   else if ( !dStricmp( type, "26-dop" ) )
      fit.fit26_DOP();
   else if ( !dStricmp( type, "convex hulls" ) )
   {
      fit.fitConvexHulls( depth, merge, concavity, maxVerts,
                           boxMaxError, sphereMaxError, capsuleMaxError );
   }
   else
   {
      Con::errorf( "TSShape::addCollisionDetail: Invalid type: '%s'", type );
      return false;
   }

   // Now add the fitted meshes to the shape:
   // - primitives (box, sphere, capsule) need their own node (with appropriate
   //   transform set) so that we can use the mesh bounds to compute the real
   //   collision primitive at load time without having to examine the geometry.
   // - convex meshes may be added at the default node, with identity transform
   // - since all meshes are in the same detail level, they all get a unique
   //   object name

   const String colNodeName( String::ToString( "Col%d", size ) );

   // Add the default node with identity transform
   S32 nodeIndex = mShape->findNode( colNodeName );
   if ( nodeIndex == -1 )
   {
      addNode( colNodeName, "" );
   }
   else
   {
      MatrixF mat;
      mShape->getNodeWorldTransform( nodeIndex, &mat );
      if ( !mat.isIdentity() )
         setNodeTransform( colNodeName, TransformF::Identity );
   }

   // Add the meshes to the shape => 
   for ( S32 i = 0; i < fit.getMeshCount(); i++ )
   {
      MeshFit::Mesh* mesh = fit.getMesh( i );

      // Determine a unique name for this mesh
      String objName;
      switch ( mesh->type )
      {
         case MeshFit::Box:      objName = "ColBox";       break;
         case MeshFit::Sphere:   objName = "ColSphere";    break;
         case MeshFit::Capsule:  objName = "ColCapsule";   break;
         default:                objName = "ColConvex";    break;
      }

      for ( S32 suffix = i; suffix != 0; suffix /= 26 )
         objName += ('A' + ( suffix % 26 ) );
      String meshName = objName + String::ToString( "%d", size );

      mShape->addMesh( mesh->tsmesh, meshName );

      // Add a node for this object if needed (non-identity transform)
      if ( mesh->transform.isIdentity() )
      {
         mShape->setObjectNode( objName, colNodeName );
      }
      else
      {
         addNode( meshName, colNodeName, TransformF( mesh->transform ) );
         mShape->setObjectNode( objName, meshName );
      }
   }

   mShape->init();

   ADD_TO_CHANGE_SET();
   return true;
}}
