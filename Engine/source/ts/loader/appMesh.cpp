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

#include "ts/loader/appMesh.h"
#include "ts/loader/tsShapeLoader.h"

Vector<AppMaterial*> AppMesh::appMaterials;

AppMesh::AppMesh()
   : flags(0), numFrames(0), numMatFrames(0), vertsPerFrame(0)
{
}

AppMesh::~AppMesh()
{
}

void AppMesh::computeBounds(Box3F& bounds)
{
   bounds = Box3F::Invalid;

   if ( isSkin() )
   {
      // Need to skin the mesh before we can compute the bounds

      // Setup bone transforms
      Vector<MatrixF> boneTransforms;
      boneTransforms.setSize( nodeIndex.size() );
      for (S32 iBone = 0; iBone < boneTransforms.size(); iBone++)
      {
         MatrixF nodeMat = bones[iBone]->getNodeTransform( TSShapeLoader::DefaultTime );
         TSShapeLoader::zapScale(nodeMat);
         boneTransforms[iBone].mul( nodeMat, initialTransforms[iBone] );
      }

      // Multiply verts by weighted bone transforms
      for (S32 iVert = 0; iVert < initialVerts.size(); iVert++)
         points[iVert].set( Point3F::Zero );

      for (S32 iWeight = 0; iWeight < vertexIndex.size(); iWeight++)
      {
         const S32& vertIndex = vertexIndex[iWeight];
         const MatrixF& deltaTransform = boneTransforms[ boneIndex[iWeight] ];

         Point3F v;
         deltaTransform.mulP( initialVerts[vertIndex], &v );
         v *= weight[iWeight];

         points[vertIndex] += v;
      }

      // compute bounds for the skinned mesh
      for (S32 iVert = 0; iVert < initialVerts.size(); iVert++)
         bounds.extend( points[iVert] );
   }
   else
   {
      MatrixF transform = getMeshTransform(TSShapeLoader::DefaultTime);
      TSShapeLoader::zapScale(transform);

      for (S32 iVert = 0; iVert < points.size(); iVert++)
      {
         Point3F p;
         transform.mulP(points[iVert], &p);
         bounds.extend(p);
      }
   }
}

void AppMesh::computeNormals()
{
   // Clear normals
   normals.setSize( points.size() );
   for (S32 iNorm = 0; iNorm < normals.size(); iNorm++)
      normals[iNorm] = Point3F::Zero;

   // Sum triangle normals for each vertex
   for (S32 iPrim = 0; iPrim < primitives.size(); iPrim++)
   {
      const TSDrawPrimitive& prim = primitives[iPrim];

      for (S32 iInd = 0; iInd < prim.numElements; iInd += 3)
      {
         // Compute the normal for this triangle
         S32 idx0 = indices[prim.start + iInd + 0];
         S32 idx1 = indices[prim.start + iInd + 1];
         S32 idx2 = indices[prim.start + iInd + 2];

         const Point3F& v0 = points[idx0];
         const Point3F& v1 = points[idx1];
         const Point3F& v2 = points[idx2];

         Point3F n;
         mCross(v2 - v0, v1 - v0, &n);
         n.normalize();    // remove this to use 'weighted' normals (large triangles will have more effect)

         normals[idx0] += n;
         normals[idx1] += n;
         normals[idx2] += n;
      }
   }

   // Normalize the vertex normals (this takes care of averaging the triangle normals)
   for (S32 iNorm = 0; iNorm < normals.size(); iNorm++)
      normals[iNorm].normalize();
}

TSMesh* AppMesh::constructTSMesh()
{
   TSMesh* tsmesh;
   if (isSkin())
   {
      TSSkinMesh* tsskin = new TSSkinMesh();
      tsmesh = tsskin;

      // Copy skin elements
      tsskin->weight = weight;
      tsskin->boneIndex = boneIndex;
      tsskin->vertexIndex = vertexIndex;
      tsskin->batchData.nodeIndex = nodeIndex;
      tsskin->batchData.initialTransforms = initialTransforms;
      tsskin->batchData.initialVerts = initialVerts;
      tsskin->batchData.initialNorms = initialNorms;
   }
   else
   {
      tsmesh = new TSMesh();
   }

   // Copy mesh elements
   tsmesh->verts = points;
   tsmesh->norms = normals;
   tsmesh->tverts = uvs;
   tsmesh->primitives = primitives;
   tsmesh->indices = indices;
   tsmesh->colors = colors;
   tsmesh->tverts2 = uv2s;

   // Finish initializing the shape
   tsmesh->setFlags(flags);
   tsmesh->computeBounds();
   tsmesh->numFrames = numFrames;
   tsmesh->numMatFrames = numMatFrames;
   tsmesh->vertsPerFrame = vertsPerFrame;
   tsmesh->createTangents(tsmesh->verts, tsmesh->norms);
   tsmesh->encodedNorms.set(NULL,0);

   return tsmesh;
}

bool AppMesh::isBillboard()
{
   return !dStrnicmp(getName(),"BB::",4) || !dStrnicmp(getName(),"BB_",3) || isBillboardZAxis();
}

bool AppMesh::isBillboardZAxis()
{
   return !dStrnicmp(getName(),"BBZ::",5) || !dStrnicmp(getName(),"BBZ_",4);
}
