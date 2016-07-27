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
#include "ts/tsMesh.h"

#include "ts/tsMeshIntrinsics.h"
#include "ts/tsDecal.h"
#include "ts/tsSortedMesh.h"
#include "ts/tsShape.h"
#include "ts/tsShapeInstance.h"
#include "ts/tsRenderState.h"
#include "ts/tsMaterialList.h"
#include "ts/instancingMatHook.h"
#include "math/mMath.h"
#include "math/mathIO.h"
#include "math/mathUtils.h"
#include "console/console.h"
#include "scene/sceneObject.h"
#include "core/bitRender.h"
#include "collision/convex.h"
#include "collision/optimizedPolyList.h"
#include "core/frameAllocator.h"
#include "platform/profiler.h"
#include "materials/sceneData.h"
#include "materials/materialManager.h"
#include "scene/sceneManager.h"
#include "scene/sceneRenderState.h"
#include "materials/matInstance.h"
#include "renderInstance/renderPassManager.h"
#include "materials/customMaterialDefinition.h"
#include "gfx/util/triListOpt.h"
#include "util/triRayCheck.h"

#include "opcode/Opcode.h"

#if defined(TORQUE_OS_XENON)
#  include "platformXbox/platformXbox.h"
#endif

GFXPrimitiveType drawTypes[] = { GFXTriangleList, GFXTriangleStrip };
#define getDrawType(a) (drawTypes[a])


// structures used to share data between detail levels...
// used (and valid) during load only
Vector<Point3F*> TSMesh::smVertsList;
Vector<Point3F*> TSMesh::smNormsList;
Vector<U8*>      TSMesh::smEncodedNormsList;
Vector<Point2F*> TSMesh::smTVertsList;
Vector<Point2F*> TSMesh::smTVerts2List;
Vector<ColorI*> TSMesh::smColorsList;

Vector<bool>     TSMesh::smDataCopied;

Vector<MatrixF*> TSSkinMesh::smInitTransformList;
Vector<S32*>     TSSkinMesh::smVertexIndexList;
Vector<S32*>     TSSkinMesh::smBoneIndexList;
Vector<F32*>     TSSkinMesh::smWeightList;
Vector<S32*>     TSSkinMesh::smNodeIndexList;

Vector<Point3F> gNormalStore;

bool TSMesh::smUseTriangles = false; // convert all primitives to triangle lists on load
bool TSMesh::smUseOneStrip  = true; // join triangle strips into one long strip on load
S32  TSMesh::smMinStripSize = 1;     // smallest number of _faces_ allowed per strip (all else put in tri list)
bool TSMesh::smUseEncodedNormals = false;

const F32 TSMesh::VISIBILITY_EPSILON = 0.0001f;

S32 TSMesh::smMaxInstancingVerts = 200;

// quick function to force object to face camera -- currently throws out roll :(
void tsForceFaceCamera( MatrixF *mat, const Point3F *objScale )
{
   Point4F p;
   mat->getColumn( 3, &p );
   mat->identity();
   mat->setColumn( 3, p );

   if ( objScale )
   {
      MatrixF scale( true );
      scale.scale( *objScale );
      mat->mul( scale );
   }
}

//-----------------------------------------------------
// TSMesh render methods
//-----------------------------------------------------

void TSMesh::render( TSVertexBufferHandle &instanceVB, GFXPrimitiveBufferHandle &instancePB )
{
   // A TSMesh never uses the instanceVB.
   TORQUE_UNUSED( instanceVB ); 
   TORQUE_UNUSED( instancePB );

   innerRender( mVB, mPB );
}

void TSMesh::innerRender( TSVertexBufferHandle &vb, GFXPrimitiveBufferHandle &pb )
{
   if ( !vb.isValid() || !pb.isValid() )
      return;

   GFX->setVertexBuffer( vb );
   GFX->setPrimitiveBuffer( pb );
   
   for( U32 p = 0; p < primitives.size(); p++ )
      GFX->drawPrimitive( p );
}


void TSMesh::render( TSMaterialList *materials, 
                     const TSRenderState &rdata, 
                     bool isSkinDirty,
                     const Vector<MatrixF> &transforms, 
                     TSVertexBufferHandle &vertexBuffer,
                     GFXPrimitiveBufferHandle &primitiveBuffer )
{
   // These are only used by TSSkinMesh.
   TORQUE_UNUSED( isSkinDirty );   
   TORQUE_UNUSED( transforms );
   TORQUE_UNUSED( vertexBuffer );
   TORQUE_UNUSED( primitiveBuffer );

   // Pass our shared VB.
   innerRender( materials, rdata, mVB, mPB );
}

void TSMesh::innerRender( TSMaterialList *materials, const TSRenderState &rdata, TSVertexBufferHandle &vb, GFXPrimitiveBufferHandle &pb )
{
   PROFILE_SCOPE( TSMesh_InnerRender );

   if( vertsPerFrame <= 0 ) 
      return;

   F32 meshVisibility = rdata.getFadeOverride() * mVisibility;
   if ( meshVisibility < VISIBILITY_EPSILON )
      return;

   const SceneRenderState *state = rdata.getSceneState();
   RenderPassManager *renderPass = state->getRenderPass();

   MeshRenderInst *coreRI = renderPass->allocInst<MeshRenderInst>();
   coreRI->type = RenderPassManager::RIT_Mesh;

   // Pass accumulation texture along.
   coreRI->accuTex = rdata.getAccuTex();

   const MatrixF &objToWorld = GFX->getWorldMatrix();

   // Sort by the center point or the bounds.
   if ( rdata.useOriginSort() )
      coreRI->sortDistSq = ( objToWorld.getPosition() - state->getCameraPosition() ).lenSquared();
   else
   {
      Box3F rBox = mBounds;
      objToWorld.mul( rBox );
      coreRI->sortDistSq = rBox.getSqDistanceToPoint( state->getCameraPosition() );      
   }

   if (getFlags(Billboard))
   {
      Point3F camPos = state->getDiffuseCameraPosition();
      Point3F objPos;
      objToWorld.getColumn(3, &objPos);
      Point3F targetVector = camPos - objPos;
      if(getFlags(BillboardZAxis))
         targetVector.z = 0.0f;
      targetVector.normalize();
      MatrixF orient = MathUtils::createOrientFromDir(targetVector);
      orient.setPosition(objPos);
      orient.scale(objToWorld.getScale());

      coreRI->objectToWorld = renderPass->allocUniqueXform( orient );
   }
   else
      coreRI->objectToWorld = renderPass->allocUniqueXform( objToWorld );

   coreRI->worldToCamera = renderPass->allocSharedXform(RenderPassManager::View);
   coreRI->projection = renderPass->allocSharedXform(RenderPassManager::Projection);

   AssertFatal( vb.isValid(), "TSMesh::innerRender() - Got invalid vertex buffer!" );
   AssertFatal( pb.isValid(), "TSMesh::innerRender() - Got invalid primitive buffer!" );

   coreRI->vertBuff = &vb;
   coreRI->primBuff = &pb;
   coreRI->defaultKey2 = (uintptr_t) coreRI->vertBuff;

   coreRI->materialHint = rdata.getMaterialHint();

   coreRI->visibility = meshVisibility;  
   coreRI->cubemap = rdata.getCubemap();

   // NOTICE: SFXBB is removed and refraction is disabled!
   //coreRI->backBuffTex = GFX->getSfxBackBuffer();

   for ( S32 i = 0; i < primitives.size(); i++ )
   {
      const TSDrawPrimitive &draw = primitives[i];

      // We need to have a material.
      if ( draw.matIndex & TSDrawPrimitive::NoMaterial )
         continue;

#ifdef TORQUE_DEBUG_BREAK_INSPECT
      // for inspection if you happen to be running in a debugger and can't do bit 
      // operations in your head.
      S32 triangles = draw.matIndex & TSDrawPrimitive::Triangles;
      S32 strip = draw.matIndex & TSDrawPrimitive::Strip;
      S32 fan = draw.matIndex & TSDrawPrimitive::Fan;
      S32 indexed = draw.matIndex & TSDrawPrimitive::Indexed;
      S32 type = draw.matIndex & TSDrawPrimitive::TypeMask;
      TORQUE_UNUSED(triangles);
      TORQUE_UNUSED(strip);
      TORQUE_UNUSED(fan);
      TORQUE_UNUSED(indexed);
      TORQUE_UNUSED(type);
      //define TORQUE_DEBUG_BREAK_INSPECT, and insert debug break here to inspect the above elements at runtime
#endif

      const U32 matIndex = draw.matIndex & TSDrawPrimitive::MaterialMask;
      BaseMatInstance *matInst = materials->getMaterialInst( matIndex );

#ifndef TORQUE_OS_MAC

      // Get the instancing material if this mesh qualifies.
      if ( meshType != SkinMeshType && pb->mPrimitiveArray[i].numVertices < smMaxInstancingVerts )
         matInst = InstancingMaterialHook::getInstancingMat( matInst );

#endif

      // If we don't have a material instance after the overload then
      // there is nothing to render... skip this primitive.
      matInst = state->getOverrideMaterial( matInst );
      if ( !matInst || !matInst->isValid())
         continue;

      // If the material needs lights then gather them
      // here once and set them on the core render inst.
      if ( matInst->isForwardLit() && !coreRI->lights[0] && rdata.getLightQuery() )
         rdata.getLightQuery()->getLights( coreRI->lights, 8 );

      MeshRenderInst *ri = renderPass->allocInst<MeshRenderInst>();
      *ri = *coreRI;

      ri->matInst = matInst;
      ri->defaultKey = matInst->getStateHint();
      ri->primBuffIndex = i;

      // Translucent materials need the translucent type.
      if ( matInst->getMaterial()->isTranslucent() )
      {
         ri->type = RenderPassManager::RIT_Translucent;
         ri->translucentSort = true;
      }

      renderPass->addInst( ri );
   }
}

const Point3F * TSMesh::getNormals( S32 firstVert )
{
   if ( getFlags( UseEncodedNormals ) )
   {
      gNormalStore.setSize( vertsPerFrame );
      for ( S32 i = 0; i < encodedNorms.size(); i++ )
         gNormalStore[i] = decodeNormal( encodedNorms[ i + firstVert ] );

      return gNormalStore.address();
   }

   return &norms[firstVert];
}

//-----------------------------------------------------
// TSMesh collision methods
//-----------------------------------------------------

bool TSMesh::buildPolyList( S32 frame, AbstractPolyList *polyList, U32 &surfaceKey, TSMaterialList *materials )
{
   S32 firstVert  = vertsPerFrame * frame, i, base = 0;

   // add the verts...
   if ( vertsPerFrame )
   {
      if ( mVertexData.isReady() )
      {
         OptimizedPolyList* opList = dynamic_cast<OptimizedPolyList*>(polyList);
         if ( opList )
         {
            base = opList->mVertexList.size();
            for ( i = 0; i < vertsPerFrame; i++ )
            {
               // Don't use vertex() method as we want to retain the original indices
               OptimizedPolyList::VertIndex vert;
               vert.vertIdx   = opList->insertPoint( mVertexData[ i + firstVert ].vert() );
               vert.normalIdx = opList->insertNormal( mVertexData[ i + firstVert ].normal() );
               vert.uv0Idx    = opList->insertUV0( mVertexData[ i + firstVert ].tvert() );
               if ( mHasTVert2 )
                  vert.uv1Idx = opList->insertUV1( mVertexData[ i + firstVert ].tvert2() );

               opList->mVertexList.push_back( vert );
            }
         }
         else
         {
            base = polyList->addPointAndNormal( mVertexData[firstVert].vert(), mVertexData[firstVert].normal() );
            for ( i = 1; i < vertsPerFrame; i++ )
            {
               polyList->addPointAndNormal( mVertexData[ i + firstVert ].vert(), mVertexData[ i + firstVert ].normal() );
            }
         }
      }
      else
      {
         OptimizedPolyList* opList = dynamic_cast<OptimizedPolyList*>(polyList);
         if ( opList )
         {
            base = opList->mVertexList.size();
            for ( i = 0; i < vertsPerFrame; i++ )
            {
               // Don't use vertex() method as we want to retain the original indices
               OptimizedPolyList::VertIndex vert;
               vert.vertIdx   = opList->insertPoint( verts[ i + firstVert ] );
               vert.normalIdx = opList->insertNormal( norms[ i + firstVert ] );
               vert.uv0Idx    = opList->insertUV0( tverts[ i + firstVert ] );
               if ( mHasTVert2 )
                  vert.uv1Idx = opList->insertUV1( tverts2[ i + firstVert ] );

               opList->mVertexList.push_back( vert );
            }
         }
         else
         {
            base = polyList->addPointAndNormal( verts[firstVert], norms[firstVert] );
            for ( i = 1; i < vertsPerFrame; i++ )
               polyList->addPointAndNormal( verts[ i + firstVert ], norms[ i + firstVert ] );
         }
      }
   }

   // add the polys...
   for ( i = 0; i < primitives.size(); i++ )
   {
      TSDrawPrimitive & draw = primitives[i];
      U32 start = draw.start;

      AssertFatal( draw.matIndex & TSDrawPrimitive::Indexed,"TSMesh::buildPolyList (1)" );

      U32 matIndex = draw.matIndex & TSDrawPrimitive::MaterialMask;
      BaseMatInstance* material = ( materials ? materials->getMaterialInst( matIndex ) : 0 );

      // gonna depend on what kind of primitive it is...
      if ( (draw.matIndex & TSDrawPrimitive::TypeMask) == TSDrawPrimitive::Triangles )
      {
         for ( S32 j = 0; j < draw.numElements; )
         {
            U32 idx0 = base + indices[start + j + 0];
            U32 idx1 = base + indices[start + j + 1];
            U32 idx2 = base + indices[start + j + 2];
            polyList->begin(material,surfaceKey++);
            polyList->vertex( idx0 );
            polyList->vertex( idx1 );
            polyList->vertex( idx2 );
            polyList->plane( idx0, idx1, idx2 );
            polyList->end();
            j += 3;
         }
      }
      else
      {
         AssertFatal( (draw.matIndex & TSDrawPrimitive::TypeMask) == TSDrawPrimitive::Strip,"TSMesh::buildPolyList (2)" );

         U32 idx0 = base + indices[start + 0];
         U32 idx1;
         U32 idx2 = base + indices[start + 1];
         U32 * nextIdx = &idx1;
         for ( S32 j = 2; j < draw.numElements; j++ )
         {
            *nextIdx = idx2;
            // nextIdx = (j%2)==0 ? &idx0 : &idx1;
            nextIdx = (U32*) ( (dsize_t)nextIdx ^ (dsize_t)&idx0 ^ (dsize_t)&idx1);
            idx2 = base + indices[start + j];
            if ( idx0 == idx1 || idx0 == idx2 || idx1 == idx2 )
               continue;

            polyList->begin( material, surfaceKey++ );
            polyList->vertex( idx0 );
            polyList->vertex( idx1 );
            polyList->vertex( idx2 );
            polyList->plane( idx0, idx1, idx2 );
            polyList->end();
         }
      }
   }
   return true;
}

bool TSMesh::getFeatures( S32 frame, const MatrixF& mat, const VectorF&, ConvexFeature* cf, U32& )
{
   S32 firstVert = vertsPerFrame * frame;
   S32 i;
   S32 base = cf->mVertexList.size();

   for ( i = 0; i < vertsPerFrame; i++ ) 
   {
      cf->mVertexList.increment();
      mat.mulP( mVertexData[firstVert + i].vert(), &cf->mVertexList.last() );
   }

   // add the polys...
   for ( i = 0; i < primitives.size(); i++ )
   {
      TSDrawPrimitive & draw = primitives[i];
      U32 start = draw.start;

      AssertFatal( draw.matIndex & TSDrawPrimitive::Indexed,"TSMesh::buildPolyList (1)" );

      // gonna depend on what kind of primitive it is...
      if ( (draw.matIndex & TSDrawPrimitive::TypeMask) == TSDrawPrimitive::Triangles)
      {
         for ( S32 j = 0; j < draw.numElements; j += 3 )
         {
            PlaneF plane( cf->mVertexList[base + indices[start + j + 0]],
                          cf->mVertexList[base + indices[start + j + 1]],
                          cf->mVertexList[base + indices[start + j + 2]]);

            cf->mFaceList.increment();

            ConvexFeature::Face& lastFace = cf->mFaceList.last();
            lastFace.normal = plane;

            lastFace.vertex[0] = base + indices[start + j + 0];
            lastFace.vertex[1] = base + indices[start + j + 1];
            lastFace.vertex[2] = base + indices[start + j + 2];

            for ( U32 l = 0; l < 3; l++ ) 
            {
               U32 newEdge0, newEdge1;
               U32 zero = base + indices[start + j + l];
               U32 one  = base + indices[start + j + ((l+1)%3)];
               newEdge0 = getMin( zero, one );
               newEdge1 = getMax( zero, one );
               bool found = false;
               for ( S32 k = 0; k < cf->mEdgeList.size(); k++ ) 
               {
                  if ( cf->mEdgeList[k].vertex[0] == newEdge0 &&
                       cf->mEdgeList[k].vertex[1] == newEdge1) 
                  {
                     found = true;
                     break;
                  }
               }

               if ( !found ) 
               {
                  cf->mEdgeList.increment();
                  cf->mEdgeList.last().vertex[0] = newEdge0;
                  cf->mEdgeList.last().vertex[1] = newEdge1;
               }
            }
         }
      }
      else
      {
         AssertFatal( (draw.matIndex & TSDrawPrimitive::TypeMask) == TSDrawPrimitive::Strip,"TSMesh::buildPolyList (2)" );

         U32 idx0 = base + indices[start + 0];
         U32 idx1;
         U32 idx2 = base + indices[start + 1];
         U32 * nextIdx = &idx1;
         for ( S32 j = 2; j < draw.numElements; j++ )
         {
            *nextIdx = idx2;
            nextIdx = (U32*) ( (dsize_t)nextIdx ^ (dsize_t)&idx0 ^ (dsize_t)&idx1);
            idx2 = base + indices[start + j];
            if ( idx0 == idx1 || idx0 == idx2 || idx1 == idx2 )
               continue;

            PlaneF plane( cf->mVertexList[idx0],
                          cf->mVertexList[idx1],
                          cf->mVertexList[idx2] );

            cf->mFaceList.increment();
            cf->mFaceList.last().normal = plane;

            cf->mFaceList.last().vertex[0] = idx0;
            cf->mFaceList.last().vertex[1] = idx1;
            cf->mFaceList.last().vertex[2] = idx2;

            U32 newEdge0, newEdge1;
            newEdge0 = getMin( idx0, idx1 );
            newEdge1 = getMax( idx0, idx1 );
            bool found = false;
            S32 k;
            for ( k = 0; k < cf->mEdgeList.size(); k++ ) 
            {
               ConvexFeature::Edge currentEdge = cf->mEdgeList[k];
               if (currentEdge.vertex[0] == newEdge0 &&
                  currentEdge.vertex[1] == newEdge1)
               {
                  found = true;
                  break;
               }
            }

            if ( !found ) 
            {
               cf->mEdgeList.increment();
               cf->mEdgeList.last().vertex[0] = newEdge0;
               cf->mEdgeList.last().vertex[1] = newEdge1;
            }

            newEdge0 = getMin( idx1, idx2 );
            newEdge1 = getMax( idx1, idx2 );
            found = false;
            for ( k = 0; k < cf->mEdgeList.size(); k++ ) 
            {
               if ( cf->mEdgeList[k].vertex[0] == newEdge0 &&
                    cf->mEdgeList[k].vertex[1] == newEdge1 ) 
               {
                  found = true;
                  break;
               }
            }

            if ( !found ) 
            {
               cf->mEdgeList.increment();
               cf->mEdgeList.last().vertex[0] = newEdge0;
               cf->mEdgeList.last().vertex[1] = newEdge1;
            }

            newEdge0 = getMin(idx0, idx2);
            newEdge1 = getMax(idx0, idx2);
            found = false;
            for ( k = 0; k < cf->mEdgeList.size(); k++ ) 
            {
               if ( cf->mEdgeList[k].vertex[0] == newEdge0 &&
                    cf->mEdgeList[k].vertex[1] == newEdge1 ) 
               {
                  found = true;
                  break;
               }
            }

            if ( !found ) 
            {
               cf->mEdgeList.increment();
               cf->mEdgeList.last().vertex[0] = newEdge0;
               cf->mEdgeList.last().vertex[1] = newEdge1;
            }
         }
      }
   }

   return false;
}


void TSMesh::support( S32 frame, const Point3F &v, F32 *currMaxDP, Point3F *currSupport )
{
   if ( vertsPerFrame == 0 )
      return;

   U32 waterMark = FrameAllocator::getWaterMark();
   F32* pDots = (F32*)FrameAllocator::alloc( sizeof(F32) * vertsPerFrame );

   S32 firstVert = vertsPerFrame * frame;
   m_point3F_bulk_dot( &v.x,
                       &mVertexData[firstVert].vert().x,
                       vertsPerFrame,
                       mVertexData.vertSize(),
                       pDots );

   F32 localdp = *currMaxDP;
   S32 index   = -1;

   for ( S32 i = 0; i < vertsPerFrame; i++ )
   {
      if ( pDots[i] > localdp )
      {
         localdp = pDots[i];
         index   = i;
      }
   }

   FrameAllocator::setWaterMark(waterMark);

   if ( index != -1 )
   {
      *currMaxDP   = localdp;
      *currSupport = mVertexData[index + firstVert].vert();
   }
}

bool TSMesh::castRay( S32 frame, const Point3F & start, const Point3F & end, RayInfo * rayInfo, TSMaterialList* materials )
{
   if ( planeNormals.empty() )
      buildConvexHull(); // if haven't done it yet...

   // Keep track of startTime and endTime.  They start out at just under 0 and just over 1, respectively.
   // As we check against each plane, prune start and end times back to represent current intersection of
   // line with all the planes (or rather with all the half-spaces defined by the planes).
   // But, instead of explicitly keeping track of startTime and endTime, keep track as numerator and denominator
   // so that we can avoid as many divisions as possible.

   //   F32 startTime = -0.01f;
   F32 startNum = -0.01f;
   F32 startDen =  1.00f;
   //   F32 endTime   = 1.01f;
   F32 endNum = 1.01f;
   F32 endDen = 1.00f;

   S32 curPlane = 0;
   U32 curMaterial = 0;
   bool found = false;

   // the following block of code is an optimization...
   // it isn't necessary if the longer version of the main loop is used
   bool tmpFound;
   S32 tmpPlane;
   F32 sgn = -1.0f;
   F32 * pnum = &startNum;
   F32 * pden = &startDen;
   S32 * pplane = &curPlane;
   bool * pfound = &found;

   S32 startPlane = frame * planesPerFrame;
   for ( S32  i = startPlane; i < startPlane + planesPerFrame; i++ )
   {
      // if start & end outside, no collision
      // if start & end inside, continue
      // if start outside, end inside, or visa versa, find intersection of line with plane
      //    then update intersection of line with hull (using startTime and endTime)
      F32 dot1 = mDot( planeNormals[i], start ) - planeConstants[i];
      F32 dot2 = mDot( planeNormals[i], end) - planeConstants[i];
      if ( dot1 * dot2 > 0.0f )
      {
         // same side of the plane...which side -- dot==0 considered inside
         if ( dot1 > 0.0f )
            return false; // start and end outside of this plane, no collision
         
         // start and end inside plane, continue
         continue;
      }

      //AssertFatal( dot1 / ( dot1 - dot2 ) >= 0.0f && dot1 / ( dot1 - dot2 ) <= 1.0f,"TSMesh::castRay (1)" );

      // find intersection (time) with this plane...
      // F32 time = dot1 / (dot1-dot2);
      F32 num = mFabs( dot1 );
      F32 den = mFabs( dot1 - dot2 );

      // the following block of code is an optimized version...
      // this can be commented out and the following block of code used instead
      // if debugging a problem in this code, that should probably be done
      // if you want to see how this works, look at the following block of code,
      // not this one...
      // Note that this does not get optimized appropriately...it is included this way
      // as an idea for future optimization.
      if ( sgn * dot1 >= 0 )
      {
         sgn *= -1.0f;
         pnum = (F32*) ((dsize_t)pnum ^ (dsize_t)&endNum ^ (dsize_t)&startNum);
         pden = (F32*) ((dsize_t)pden ^ (dsize_t)&endDen ^ (dsize_t)&startDen);
         pplane = (S32*) ((dsize_t)pplane ^ (dsize_t)&tmpPlane ^ (dsize_t)&curPlane);
         pfound = (bool*) ((dsize_t)pfound ^ (dsize_t)&tmpFound ^ (dsize_t)&found);
      }

      bool noCollision = num * endDen * sgn < endNum * den * sgn && num * startDen * sgn < startNum * den * sgn;
      if (num * *pden * sgn < *pnum * den * sgn && !noCollision)
      {
         *pnum = num;
         *pden = den;
         *pplane = i;
         *pfound = true;
      }
      else if ( noCollision )
         return false;

//      if (dot1<=0.0f)
//      {
//         // start is inside plane, end is outside...chop off end
//         if (num*endDen<endNum*den) // if (time<endTime)
//         {
//            if (num*startDen<startNum*den) //if (time<startTime)
//               // no intersection of line and hull
//               return false;
//            // endTime = time;
//            endNum = num;
//            endDen = den;
//         }
//         // else, no need to do anything, just continue (we've been more inside than this)
//      }
//      else // dot2<=0.0f
//      {
//         // end is inside poly, start is outside...chop off start
//         AssertFatal(dot2<=0.0f,"TSMesh::castRay (2)");
//         if (num*startDen>startNum*den) // if (time>startTime)
//        {
//            if (num*endDen>endNum*den) //if (time>endTime)
//               // no intersection of line and hull
//               return false;
//            // startTime   = time;
//            startNum = num;
//            startDen = den;
//            curPlane    = i;
//            curMaterial = planeMaterials[i-startPlane];
//            found = true;
//         }
//         // else, no need to do anything, just continue (we've been more inside than this)
//      }
   }

   // setup rayInfo
   if ( found && rayInfo )
   {
      curMaterial       = planeMaterials[ curPlane - startPlane ];

      rayInfo->t        = (F32)startNum/(F32)startDen; // finally divide...
      rayInfo->normal   = planeNormals[curPlane];

      if (materials && materials->size() > 0)
         rayInfo->material = materials->getMaterialInst( curMaterial );
      else
         rayInfo->material = NULL;

      rayInfo->setContactPoint( start, end );

      return true;
   }
   else if ( found )
      return true;

   // only way to get here is if start is inside hull...
   // we could return null and just plug in garbage for the material and normal...
   return false;
}

bool TSMesh::castRayRendered( S32 frame, const Point3F & start, const Point3F & end, RayInfo * rayInfo, TSMaterialList* materials )
{
   if( vertsPerFrame <= 0 ) 
      return false;

   if( mNumVerts == 0 )
      return false;

   S32 firstVert  = vertsPerFrame * frame;

	bool found = false;
	F32 best_t = F32_MAX;
   U32 bestIdx0 = 0, bestIdx1 = 0, bestIdx2 = 0;
   BaseMatInstance* bestMaterial = NULL;
	Point3F dir = end - start;

   for ( S32 i = 0; i < primitives.size(); i++ )
   {
      TSDrawPrimitive & draw = primitives[i];
      U32 drawStart = draw.start;

      AssertFatal( draw.matIndex & TSDrawPrimitive::Indexed,"TSMesh::castRayRendered (1)" );

      U32 matIndex = draw.matIndex & TSDrawPrimitive::MaterialMask;
      BaseMatInstance* material = ( materials ? materials->getMaterialInst( matIndex ) : 0 );

      U32 idx0, idx1, idx2;

      // gonna depend on what kind of primitive it is...
      if ( (draw.matIndex & TSDrawPrimitive::TypeMask) == TSDrawPrimitive::Triangles )
      {
         for ( S32 j = 0; j < draw.numElements-2; j += 3 )
         {
            idx0 = indices[drawStart + j + 0];
            idx1 = indices[drawStart + j + 1];
            idx2 = indices[drawStart + j + 2];

			   F32 cur_t = 0;
			   Point2F b;

			   if(castRayTriangle(start, dir, mVertexData[firstVert + idx0].vert(),
               mVertexData[firstVert + idx1].vert(), mVertexData[firstVert + idx2].vert(), cur_t, b))
			   {
				   if(cur_t < best_t)
				   {
					   best_t = cur_t;
                  bestIdx0 = idx0;
                  bestIdx1 = idx1;
                  bestIdx2 = idx2;
                  bestMaterial = material;
					   found = true;
				   }
			   }
         }
      }
      else
      {
         AssertFatal( (draw.matIndex & TSDrawPrimitive::TypeMask) == TSDrawPrimitive::Strip,"TSMesh::castRayRendered (2)" );

         idx0 = indices[drawStart + 0];
         idx2 = indices[drawStart + 1];
         U32 * nextIdx = &idx1;
         for ( S32 j = 2; j < draw.numElements; j++ )
         {
            *nextIdx = idx2;
            // nextIdx = (j%2)==0 ? &idx0 : &idx1;
            nextIdx = (U32*) ( (dsize_t)nextIdx ^ (dsize_t)&idx0 ^ (dsize_t)&idx1);
            idx2 = indices[drawStart + j];
            if ( idx0 == idx1 || idx0 == idx2 || idx1 == idx2 )
               continue;

			   F32 cur_t = 0;
			   Point2F b;

			   if(castRayTriangle(start, dir, mVertexData[firstVert + idx0].vert(), 
               mVertexData[firstVert + idx1].vert(), mVertexData[firstVert + idx2].vert(), cur_t, b))
			   {
				   if(cur_t < best_t)
				   {
					   best_t = cur_t;
                  bestIdx0 = firstVert + idx0;
                  bestIdx1 = firstVert + idx1;
                  bestIdx2 = firstVert + idx2;
                  bestMaterial = material;
					   found = true;
				   }
			   }
         }
      }
   }

   // setup rayInfo
   if ( found && rayInfo )
   {
      rayInfo->t = best_t;

      Point3F normal;
      mCross(mVertexData[bestIdx2].vert()-mVertexData[bestIdx0].vert(),mVertexData[bestIdx1].vert()-mVertexData[bestIdx0].vert(),&normal);
      if ( mDot( normal, normal ) < 0.001f )
      {
         mCross( mVertexData[bestIdx0].vert() - mVertexData[bestIdx1].vert(), mVertexData[bestIdx2].vert() - mVertexData[bestIdx1].vert(), &normal );
         if ( mDot( normal, normal ) < 0.001f )
         {
            mCross( mVertexData[bestIdx1].vert() - mVertexData[bestIdx2].vert(), mVertexData[bestIdx0].vert() - mVertexData[bestIdx2].vert(), &normal );
         }
      }
      normal.normalize();
      rayInfo->normal = normal;

      rayInfo->material = bestMaterial;

      rayInfo->setContactPoint( start, end );

      return true;
   }
   else if ( found )
      return true;

   return false;
}

bool TSMesh::addToHull( U32 idx0, U32 idx1, U32 idx2 )
{
   // calculate the normal of this triangle... remember, we lose precision
   // when we subtract two large numbers that are very close to each other,
   // so depending on how we calculate the normal, we could get a 
   // different result. so, we will calculate the normal three different
   // ways and take the one that gives us the largest vector before we
   // normalize.
   Point3F normal1, normal2, normal3;
   mCross(mVertexData[idx2].vert()-mVertexData[idx0].vert(),mVertexData[idx1].vert()-mVertexData[idx0].vert(),&normal1);
   mCross(mVertexData[idx0].vert()-mVertexData[idx1].vert(),mVertexData[idx2].vert()-mVertexData[idx1].vert(),&normal2);
   mCross(mVertexData[idx1].vert()-mVertexData[idx2].vert(),mVertexData[idx0].vert()-mVertexData[idx2].vert(),&normal3);
   Point3F normal = normal1;
   F32 greatestMagSquared = mDot(normal1, normal1);
   F32 magSquared = mDot(normal2, normal2);
   if (magSquared > greatestMagSquared)
   {
      normal = normal2;
      greatestMagSquared = magSquared;
   }
   magSquared = mDot(normal3, normal3);
   if (magSquared > greatestMagSquared)
   {
      normal = normal3;
      greatestMagSquared = magSquared;
   }
   if (mDot(normal, normal) < 0.00000001f)
       return false;

   normal.normalize();
   F32 k = mDot( normal, mVertexData[idx0].vert() );
   for ( S32 i = 0; i < planeNormals.size(); i++ ) 
   {
      if ( mDot( planeNormals[i], normal ) > 0.99f && mFabs( k-planeConstants[i] ) < 0.01f )
         return false;          // this is a repeat...
   }
   // new plane, add it to the list...
   planeNormals.push_back( normal );
   planeConstants.push_back( k );
   return true;
}

bool TSMesh::buildConvexHull()
{
   // already done, return without error
   if ( planeNormals.size() )
      return true;

   bool error = false;

   // should probably only have 1 frame, but just in case...
   planesPerFrame = 0;
   S32 frame, i, j;
   for ( frame = 0; frame < numFrames; frame++ )
   {
      S32 firstVert  = vertsPerFrame * frame;
      S32 firstPlane = planeNormals.size();
      for ( i = 0; i < primitives.size(); i++ )
      {
         TSDrawPrimitive & draw = primitives[i];
         U32 start = draw.start;

         AssertFatal( draw.matIndex & TSDrawPrimitive::Indexed,"TSMesh::buildConvexHull (1)" );

         // gonna depend on what kind of primitive it is...
         if ( (draw.matIndex & TSDrawPrimitive::TypeMask) == TSDrawPrimitive::Triangles )
         {
            for ( j = 0;  j < draw.numElements; j += 3 )
               if ( addToHull(   indices[start + j + 0] + firstVert, 
                                 indices[start + j + 1] + firstVert, 
                                 indices[start + j + 2] + firstVert ) && frame == 0 )
                  planeMaterials.push_back( draw.matIndex & TSDrawPrimitive::MaterialMask );
         }
         else
         {
            AssertFatal( (draw.matIndex&TSDrawPrimitive::Strip) == TSDrawPrimitive::Strip,"TSMesh::buildConvexHull (2)" );

            U32 idx0 = indices[start + 0] + firstVert;
            U32 idx1;
            U32 idx2 = indices[start + 1] + firstVert;
            U32 * nextIdx = &idx1;
            for ( j = 2; j < draw.numElements; j++ )
            {
               *nextIdx = idx2;
//               nextIdx = (j%2)==0 ? &idx0 : &idx1;
               nextIdx = (U32*) ( (dsize_t)nextIdx ^ (dsize_t)&idx0 ^ (dsize_t)&idx1 );
               idx2 = indices[start + j] + firstVert;
               if ( addToHull( idx0, idx1, idx2 ) && frame == 0 )
                  planeMaterials.push_back( draw.matIndex & TSDrawPrimitive::MaterialMask );
            }
         }
      }
      // make sure all the verts on this frame are inside all the planes
      for ( i = 0; i < vertsPerFrame; i++ )
         for ( j = firstPlane; j < planeNormals.size(); j++ )
            if ( mDot( mVertexData[firstVert + i].vert(), planeNormals[j] ) - planeConstants[j] < 0.01 ) // .01 == a little slack
               error = true;

      if ( frame == 0 )
         planesPerFrame = planeNormals.size();

      if ( (frame + 1) * planesPerFrame != planeNormals.size() )
      {
         // eek, not all frames have same number of planes...
         while ( (frame + 1) * planesPerFrame > planeNormals.size() )
         {
            // we're short, duplicate last plane till we match
            U32 sz = planeNormals.size();
            planeNormals.increment();
            planeNormals.last() = planeNormals[sz-1];
            planeConstants.increment();
            planeConstants.last() = planeConstants[sz-1];
         }
         while ( (frame + 1) * planesPerFrame < planeNormals.size() )
         {
            // harsh -- last frame has more than other frames
            // duplicate last plane in each frame
            for ( S32 k = frame - 1; k >= 0; k-- )
            {
               planeNormals.insert( k * planesPerFrame + planesPerFrame );
               planeNormals[k * planesPerFrame + planesPerFrame] = planeNormals[k * planesPerFrame + planesPerFrame - 1];
               planeConstants.insert( k * planesPerFrame + planesPerFrame );
               planeConstants[k * planesPerFrame + planesPerFrame] = planeConstants[k * planesPerFrame + planesPerFrame - 1];
               if ( k == 0 )
               {
                  planeMaterials.increment();
                  planeMaterials.last() = planeMaterials[planeMaterials.size() - 2];
               }
            }
            planesPerFrame++;
         }
      }
      AssertFatal( (frame + 1) * planesPerFrame == planeNormals.size(),"TSMesh::buildConvexHull (3)" );
   }
   return !error;
}

//-----------------------------------------------------
// TSMesh bounds methods
//-----------------------------------------------------

void TSMesh::computeBounds()
{
   MatrixF mat(true);
   computeBounds( mat, mBounds, -1, &mCenter, &mRadius );
}

void TSMesh::computeBounds( const MatrixF &transform, Box3F &bounds, S32 frame, Point3F *center, F32 *radius )
{
   const Point3F *baseVert = NULL;
   S32 stride = 0;
   S32 numVerts = 0;

   if(mVertexData.isReady())
   {
      baseVert = &mVertexData[0].vert();
      stride = mVertexData.vertSize();

      if ( frame < 0 )
         numVerts = mNumVerts;
      else
      {
         baseVert = &mVertexData[frame * vertsPerFrame].vert();
         numVerts = vertsPerFrame;
      }
   }
   else
   {
      baseVert = verts.address();
      stride = sizeof(Point3F);

      if ( frame < 0 )
         numVerts = verts.size();
      else
      {
         baseVert += frame * vertsPerFrame;
         numVerts = vertsPerFrame;
      }
   }
   computeBounds( baseVert, numVerts, stride, transform, bounds, center, radius );
}

void TSMesh::computeBounds( const Point3F *v, S32 numVerts, S32 stride, const MatrixF &transform, Box3F &bounds, Point3F *center, F32 *radius )
{
   const U8 *_vb = reinterpret_cast<const U8 *>(v);

   if ( !numVerts )
   {
      bounds.minExtents = Point3F::Zero;
      bounds.maxExtents = Point3F::Zero;
      if ( center )
         *center = Point3F::Zero;
      if ( radius )
         *radius = 0;
      return;
   }

   S32 i;
   Point3F p;
   transform.mulP( *v, &bounds.minExtents );
   bounds.maxExtents = bounds.minExtents;
   for ( i = 0; i < numVerts; i++ )
   {
      const Point3F &curVert = *reinterpret_cast<const Point3F *>(_vb + i * stride);
      transform.mulP( curVert, &p );
      bounds.maxExtents.setMax( p );
      bounds.minExtents.setMin( p );
   }
   Point3F c;
   if ( !center )
      center = &c;
   center->x = 0.5f * (bounds.minExtents.x + bounds.maxExtents.x);
   center->y = 0.5f * (bounds.minExtents.y + bounds.maxExtents.y);
   center->z = 0.5f * (bounds.minExtents.z + bounds.maxExtents.z);
   if ( radius )
   {
      *radius = 0.0f;
      for ( i = 0; i < numVerts; i++ )
      {
         const Point3F &curVert = *reinterpret_cast<const Point3F *>(_vb + i * stride);
         transform.mulP( curVert, &p );
         p -= *center;
         *radius = getMax( *radius, mDot( p, p ) );
      }
      *radius = mSqrt( *radius );
   }
}

//-----------------------------------------------------

S32 TSMesh::getNumPolys() const
{
   S32 count = 0;
   for ( S32 i = 0; i < primitives.size(); i++ )
   {
      switch (primitives[i].matIndex & TSDrawPrimitive::TypeMask)
      {
         case TSDrawPrimitive::Triangles:
            count += primitives[i].numElements / 3;
            break;

         case TSDrawPrimitive::Fan:
            count += primitives[i].numElements - 2;
            break;

         case TSDrawPrimitive::Strip:
            // Don't count degenerate triangles
            for ( S32 j = primitives[i].start;
                  j < primitives[i].start+primitives[i].numElements-2;
                  j++ )
            {
               if ((indices[j] != indices[j+1]) &&
                   (indices[j] != indices[j+2]) &&
                   (indices[j+1] != indices[j+2]))
                  count++;
            }
            break;
      }
   }
   return count;
}

//-----------------------------------------------------

TSMesh::TSMesh() : meshType( StandardMeshType )
{
   VECTOR_SET_ASSOCIATION( planeNormals );
   VECTOR_SET_ASSOCIATION( planeConstants );
   VECTOR_SET_ASSOCIATION( planeMaterials );
   parentMesh = -1;

   mOptTree = NULL;
   mOpMeshInterface = NULL;
   mOpTris = NULL;
   mOpPoints = NULL;

   mDynamic = false;
   mVisibility = 1.0f;
   mHasTVert2 = false;
   mHasColor = false;

   mNumVerts = 0;
}

//-----------------------------------------------------
// TSMesh destructor
//-----------------------------------------------------

TSMesh::~TSMesh()
{
   SAFE_DELETE( mOptTree );
   SAFE_DELETE( mOpMeshInterface );
   SAFE_DELETE_ARRAY( mOpTris );
   SAFE_DELETE_ARRAY( mOpPoints );

   mNumVerts = 0;
}

//-----------------------------------------------------
// TSSkinMesh methods
//-----------------------------------------------------

void TSSkinMesh::updateSkin( const Vector<MatrixF> &transforms, TSVertexBufferHandle &instanceVB, GFXPrimitiveBufferHandle &instancePB )
{
   PROFILE_SCOPE( TSSkinMesh_UpdateSkin );

   AssertFatal(batchDataInitialized, "Batch data not initialized. Call createBatchData() before any skin update is called.");

   // set arrays
#if defined(TORQUE_MAX_LIB)
   verts.setSize(batchData.initialVerts.size());
   norms.setSize(batchData.initialNorms.size());
#else
   if ( !batchDataInitialized && encodedNorms.size() )
   {
      // we co-opt responsibility for updating encoded normals from mesh
      gNormalStore.setSize( vertsPerFrame );
      for ( S32 i = 0; i < vertsPerFrame; i++ )
         gNormalStore[i] = decodeNormal( encodedNorms[i] );

      batchData.initialNorms.set( gNormalStore.address(), vertsPerFrame );
   }
#endif

   static Vector<MatrixF> sBoneTransforms;
   sBoneTransforms.setSize( batchData.nodeIndex.size() );

   // set up bone transforms
   PROFILE_START(TSSkinMesh_UpdateTransforms);
   for( S32 i=0; i<batchData.nodeIndex.size(); i++ )
   {
      S32 node = batchData.nodeIndex[i];
      sBoneTransforms[i].mul( transforms[node], batchData.initialTransforms[i] );
   }
   const MatrixF * matrices = &sBoneTransforms[0];
   PROFILE_END();

   // Perform skinning
   const bool bBatchByVert = !batchData.vertexBatchOperations.empty();
   if(bBatchByVert)
   {
      const Point3F *inVerts = &batchData.initialVerts[0];
      const Point3F *inNorms = &batchData.initialNorms[0];

      Point3F srcVtx, srcNrm;

      AssertFatal( batchData.vertexBatchOperations.size() == batchData.initialVerts.size(), "Assumption failed!" );

      register Point3F skinnedVert;
      register Point3F skinnedNorm;

      for( Vector<BatchData::BatchedVertex>::const_iterator itr = batchData.vertexBatchOperations.begin(); 
         itr != batchData.vertexBatchOperations.end(); itr++ )
      {
         const BatchData::BatchedVertex &curVert = *itr;

         skinnedVert.zero();
         skinnedNorm.zero();

         for( S32 tOp = 0; tOp < curVert.transformCount; tOp++ )
         {      
            const BatchData::TransformOp &transformOp = curVert.transform[tOp];

            const MatrixF& deltaTransform = matrices[transformOp.transformIndex];

            deltaTransform.mulP( inVerts[curVert.vertexIndex], &srcVtx );
            skinnedVert += ( srcVtx * transformOp.weight );

            deltaTransform.mulV( inNorms[curVert.vertexIndex], &srcNrm );
            skinnedNorm += srcNrm * transformOp.weight;
         }

         // Assign results 
         __TSMeshVertexBase &dest = mVertexData[curVert.vertexIndex];
         dest.vert(skinnedVert);
         dest.normal(skinnedNorm);
      }
   }
   else // Batch by transform
   {
      U8 *outPtr = reinterpret_cast<U8 *>(mVertexData.address());
      dsize_t outStride = mVertexData.vertSize();

#if defined(USE_MEM_VERTEX_BUFFERS)
      // Initialize it if NULL. 
      // Skinning includes readbacks from memory (argh) so don't allocate with PAGE_WRITECOMBINE
      if( instanceVB.isNull() )
         instanceVB.set( GFX, outStride, mVertexFormat, mNumVerts, GFXBufferTypeDynamic );

      // Grow if needed
      if( instanceVB.getPointer()->mNumVerts < mNumVerts )
         instanceVB.resize( mNumVerts );

      // Lock, and skin directly into the final memory destination
      outPtr = (U8 *)instanceVB.lock();
      if(!outPtr) return;
#endif
      // Set position/normal to zero so we can accumulate
      zero_vert_normal_bulk(mNumVerts, outPtr, outStride);

      // Iterate over transforms, and perform batch transform x skin_vert
      for(Vector<S32>::const_iterator itr = batchData.transformKeys.begin();
          itr != batchData.transformKeys.end(); itr++)
      {
         const S32 boneXfmIdx = *itr;
         const BatchData::BatchedTransform &curTransform = *batchData.transformBatchOperations.retreive(boneXfmIdx);
         const MatrixF &curBoneMat = matrices[boneXfmIdx];
         const S32 numVerts = curTransform.numElements;

         // Bulk transform points/normals by this transform
         m_matF_x_BatchedVertWeightList(curBoneMat, numVerts, curTransform.alignedMem,
            outPtr, outStride);
      }
#if defined(USE_MEM_VERTEX_BUFFERS)
      instanceVB.unlock();
#endif
   }
}

S32 QSORT_CALLBACK _sort_BatchedVertWeight( const void *a, const void *b )
{
   // Sort by vertex index
   const TSSkinMesh::BatchData::BatchedVertWeight &_a = *reinterpret_cast<const TSSkinMesh::BatchData::BatchedVertWeight *>(a);
   const TSSkinMesh::BatchData::BatchedVertWeight &_b = *reinterpret_cast<const TSSkinMesh::BatchData::BatchedVertWeight *>(b);
   return ( _a.vidx - _b.vidx );
}

// Batch by vertex is useful to emulate the old skinning, or to build batch data
// sutable for GPU skinning.
//#define _BATCH_BY_VERTEX

void TSSkinMesh::createBatchData()
{
   if(batchDataInitialized)
      return;

   batchDataInitialized = true;
   S32 * curVtx = vertexIndex.begin();
   S32 * curBone = boneIndex.begin();
   F32 * curWeight = weight.begin();
   const S32 * endVtx = vertexIndex.end();

   // Temp vector to build batch operations
   Vector<BatchData::BatchedVertex> batchOperations;

   bool issuedWeightWarning = false;

   // Build the batch operations
   while( curVtx != endVtx )
   {
      const S32 vidx = *curVtx;
      ++curVtx;

      const S32 midx = *curBone;
      ++curBone;

      const F32 w = *curWeight;
      ++curWeight;

      // Ignore empty weights
      if ( vidx < 0 || midx < 0 || w == 0 )
         continue;

      if( !batchOperations.empty() &&
         batchOperations.last().vertexIndex == vidx )
      {
         AssertFatal( batchOperations.last().transformCount > 0, "Not sure how this happened!" );

         S32 opIdx = batchOperations.last().transformCount++;

         // Limit the number of weights per bone (keep the N largest influences)
         if ( opIdx >= TSSkinMesh::BatchData::maxBonePerVert )
         {
            if ( !issuedWeightWarning )
            {
               issuedWeightWarning = true;
               Con::warnf( "At least one vertex has too many bone weights - limiting "
                  "to the largest %d influences (see maxBonePerVert in tsMesh.h).",
                  TSSkinMesh::BatchData::maxBonePerVert );
            }

            // Too many weights => find and replace the smallest one
            S32 minIndex = 0;
            F32 minWeight = batchOperations.last().transform[0].weight;
            for ( S32 i = 1; i < batchOperations.last().transformCount; i++ )
            {
               if ( batchOperations.last().transform[i].weight < minWeight )
               {
                  minWeight = batchOperations.last().transform[i].weight;
                  minIndex = i;
               }
            }

            opIdx = minIndex;
            batchOperations.last().transformCount = TSSkinMesh::BatchData::maxBonePerVert;
         }

         batchOperations.last().transform[opIdx].transformIndex = midx;
         batchOperations.last().transform[opIdx].weight = w;
      }
      else
      {
         batchOperations.increment();
         batchOperations.last().vertexIndex = vidx;
         batchOperations.last().transformCount = 1;

         batchOperations.last().transform[0].transformIndex = midx;
         batchOperations.last().transform[0].weight = w;
      }
      //Con::printf( "[%d] transform idx %d, weight %1.5f", vidx, midx, w );
   }
   //Con::printf("End skin update");

   // Normalize vertex weights (force weights for each vert to sum to 1)
   if ( issuedWeightWarning )
   {
      for ( S32 i = 0; i < batchOperations.size(); i++ )
      {
         BatchData::BatchedVertex& batchOp = batchOperations[i];

         // Sum weights for this vertex
         F32 invTotalWeight = 0;
         for ( S32 j = 0; j < batchOp.transformCount; j++ )
            invTotalWeight += batchOp.transform[j].weight;

         // Then normalize the vertex weights
         invTotalWeight = 1.0f / invTotalWeight;
         for ( S32 j = 0; j < batchOp.transformCount; j++ )
            batchOp.transform[j].weight *= invTotalWeight;
      }
   }

#ifdef _BATCH_BY_VERTEX
   // Copy data to member, and be done
   batchData.vertexBatchOperations.set(batchOperations.address(), batchOperations.size());

   // Convert to batch-by-transform, which is better for CPU skinning, 
   // where-as GPU skinning would data for batch-by-vertex operation
#else
   // Iterate the batch-by-vertex, and populate the batch-by-transform structs
   for( Vector<BatchData::BatchedVertex>::const_iterator itr = batchOperations.begin(); 
      itr != batchOperations.end(); itr++ )
   {
      const BatchData::BatchedVertex &curTransform = *itr;
      for( S32 i = 0; i < curTransform.transformCount; i++ )
      {
         const BatchData::TransformOp &transformOp = curTransform.transform[i];

         // Find the proper batched transform, and add this vertex/weight to the
         // list of verts affected by the transform
         BatchData::BatchedTransform *bt = batchData.transformBatchOperations.retreive(transformOp.transformIndex);
         if(!bt)
         {
            bt = new BatchData::BatchedTransform;
            batchData.transformBatchOperations.insert(bt, transformOp.transformIndex);
            bt->_tmpVec = new Vector<BatchData::BatchedVertWeight>;
            batchData.transformKeys.push_back(transformOp.transformIndex);
         }

         bt->_tmpVec->increment();

         BatchData::BatchedVertWeight& tempLast = bt->_tmpVec->last();
         tempLast.vert = batchData.initialVerts[curTransform.vertexIndex];
         tempLast.normal = batchData.initialNorms[curTransform.vertexIndex];
         tempLast.weight = transformOp.weight;
         tempLast.vidx = curTransform.vertexIndex;
      }
   }

   // Now iterate the resulting operations and convert the vectors to aligned
   // memory locations
   const S32 numBatchOps = batchData.transformKeys.size();
   for(S32 i = 0; i < numBatchOps; i++)
   {
      BatchData::BatchedTransform &curTransform = *batchData.transformBatchOperations.retreive(batchData.transformKeys[i]);
      const S32 numVerts = curTransform._tmpVec->size();

      // Allocate a chunk of aligned memory and copy in values
      curTransform.numElements = numVerts;
      curTransform.alignedMem = reinterpret_cast<BatchData::BatchedVertWeight *>(dMalloc_aligned(sizeof(BatchData::BatchedVertWeight) * numVerts, 16));
      AssertFatal(curTransform.alignedMem, "Aligned malloc failed! Debug!");
      constructArrayInPlace(curTransform.alignedMem, numVerts);
      dMemcpy(curTransform.alignedMem, curTransform._tmpVec->address(), numVerts * sizeof(BatchData::BatchedVertWeight));

      // Now free the vector memory
      delete curTransform._tmpVec;
      curTransform._tmpVec = NULL;
   }

   // Now sort the batch data so that the skin function writes close to linear output
   for(S32 i = 0; i < numBatchOps; i++)
   {
      BatchData::BatchedTransform &curTransform = *batchData.transformBatchOperations.retreive(batchData.transformKeys[i]);
      dQsort(curTransform.alignedMem, curTransform.numElements, sizeof(BatchData::BatchedVertWeight), _sort_BatchedVertWeight);
   }
#endif
}

void TSSkinMesh::render( TSVertexBufferHandle &instanceVB, GFXPrimitiveBufferHandle &instancePB )
{
   innerRender( instanceVB, instancePB );
}

void TSSkinMesh::render(   TSMaterialList *materials, 
                           const TSRenderState &rdata,
                           bool isSkinDirty,
                           const Vector<MatrixF> &transforms, 
                           TSVertexBufferHandle &vertexBuffer,
                           GFXPrimitiveBufferHandle &primitiveBuffer )
{
   PROFILE_SCOPE(TSSkinMesh_render);

   if( mNumVerts == 0 )
      return;

   // Initialize the vertex data if it needs it
   if(!mVertexData.isReady() )
      _convertToAlignedMeshData(mVertexData, batchData.initialVerts, batchData.initialNorms);
   AssertFatal(mVertexData.size() == mNumVerts, "Vert # mismatch");

   // Initialize the skin batch if that isn't ready
   if(!batchDataInitialized)
      createBatchData();

   const bool vertsChanged = vertexBuffer.isNull() || vertexBuffer->mNumVerts != mNumVerts;
   const bool primsChanged = primitiveBuffer.isNull() || primitiveBuffer->mIndexCount != indices.size();

   if ( primsChanged || vertsChanged || isSkinDirty )
   {
      // Perform skinning
      updateSkin( transforms, vertexBuffer, primitiveBuffer );
      
      // Update GFX vertex buffer
      _createVBIB( vertexBuffer, primitiveBuffer );
   }

   // render...
   innerRender( materials, rdata, vertexBuffer, primitiveBuffer );   
}

bool TSSkinMesh::buildPolyList( S32 frame, AbstractPolyList *polyList, U32 &surfaceKey, TSMaterialList *materials )
{
   // UpdateSkin() here may not be needed... 
   // we don't capture skinned 
   // verts in the polylist.
   
   // update verts and normals...
   //if( !smGlowPass && !smRefractPass ) 
   // updateSkin();

   // render...
   //Parent::buildPolyList( frame,polyList,surfaceKey, materials );
   return false;
}

bool TSSkinMesh::castRay( S32 frame, const Point3F &start, const Point3F &end, RayInfo *rayInfo, TSMaterialList *materials )
{
   TORQUE_UNUSED(frame);
   TORQUE_UNUSED(start);
   TORQUE_UNUSED(end);
   TORQUE_UNUSED(rayInfo);
   TORQUE_UNUSED(materials);

   return false;
}

bool TSSkinMesh::buildConvexHull()
{
   return false; // no error, but we don't do anything either...
}

void TSSkinMesh::computeBounds( const MatrixF &transform, Box3F &bounds, S32 frame, Point3F *center, F32 *radius )
{
   TORQUE_UNUSED(frame);

   if (frame < 0)
   {
      // Use unskinned verts
      TSMesh::computeBounds( batchData.initialVerts.address(), batchData.initialVerts.size(), sizeof(Point3F), transform, bounds, center, radius );
   }
   else
   {
      Point3F *vertStart = reinterpret_cast<Point3F *>(mVertexData.address());
      TSMesh::computeBounds( vertStart, mVertexData.size(), mVertexData.vertSize(), transform, bounds, center, radius );
   }
}

//-----------------------------------------------------
// encoded normals
//-----------------------------------------------------

const Point3F TSMesh::smU8ToNormalTable[] =
{
      Point3F( 0.565061f, -0.270644f, -0.779396f ),
      Point3F( -0.309804f, -0.731114f, 0.607860f ),
      Point3F( -0.867412f, 0.472957f, 0.154619f ),
      Point3F( -0.757488f, 0.498188f, -0.421925f ),
      Point3F( 0.306834f, -0.915340f, 0.260778f ),
      Point3F( 0.098754f, 0.639153f, -0.762713f ),
      Point3F( 0.713706f, -0.558862f, -0.422252f ),
      Point3F( -0.890431f, -0.407603f, -0.202466f ),
      Point3F( 0.848050f, -0.487612f, -0.207475f ),
      Point3F( -0.232226f, 0.776855f, 0.585293f ),
      Point3F( -0.940195f, 0.304490f, -0.152706f ),
      Point3F( 0.602019f, -0.491878f, -0.628991f ),
      Point3F( -0.096835f, -0.494354f, -0.863850f ),
      Point3F( 0.026630f, -0.323659f, -0.945799f ),
      Point3F( 0.019208f, 0.909386f, 0.415510f ),
      Point3F( 0.854440f, 0.491730f, 0.167731f ),
      Point3F( -0.418835f, 0.866521f, -0.271512f ),
      Point3F( 0.465024f, 0.409667f, 0.784809f ),
      Point3F( -0.674391f, -0.691087f, -0.259992f ),
      Point3F( 0.303858f, -0.869270f, -0.389922f ),
      Point3F( 0.991333f, 0.090061f, -0.095640f ),
      Point3F( -0.275924f, -0.369550f, 0.887298f ),
      Point3F( 0.426545f, -0.465962f, 0.775202f ),
      Point3F( -0.482741f, -0.873278f, -0.065920f ),
      Point3F( 0.063616f, 0.932012f, -0.356800f ),
      Point3F( 0.624786f, -0.061315f, 0.778385f ),
      Point3F( -0.530300f, 0.416850f, 0.738253f ),
      Point3F( 0.312144f, -0.757028f, -0.573999f ),
      Point3F( 0.399288f, -0.587091f, -0.704197f ),
      Point3F( -0.132698f, 0.482877f, 0.865576f ),
      Point3F( 0.950966f, 0.306530f, 0.041268f ),
      Point3F( -0.015923f, -0.144300f, 0.989406f ),
      Point3F( -0.407522f, -0.854193f, 0.322925f ),
      Point3F( -0.932398f, 0.220464f, 0.286408f ),
      Point3F( 0.477509f, 0.876580f, 0.059936f ),
      Point3F( 0.337133f, 0.932606f, -0.128796f ),
      Point3F( -0.638117f, 0.199338f, 0.743687f ),
      Point3F( -0.677454f, 0.445349f, 0.585423f ),
      Point3F( -0.446715f, 0.889059f, -0.100099f ),
      Point3F( -0.410024f, 0.909168f, 0.072759f ),
      Point3F( 0.708462f, 0.702103f, -0.071641f ),
      Point3F( -0.048801f, -0.903683f, -0.425411f ),
      Point3F( -0.513681f, -0.646901f, 0.563606f ),
      Point3F( -0.080022f, 0.000676f, -0.996793f ),
      Point3F( 0.066966f, -0.991150f, -0.114615f ),
      Point3F( -0.245220f, 0.639318f, -0.728793f ),
      Point3F( 0.250978f, 0.855979f, 0.452006f ),
      Point3F( -0.123547f, 0.982443f, -0.139791f ),
      Point3F( -0.794825f, 0.030254f, -0.606084f ),
      Point3F( -0.772905f, 0.547941f, 0.319967f ),
      Point3F( 0.916347f, 0.369614f, -0.153928f ),
      Point3F( -0.388203f, 0.105395f, 0.915527f ),
      Point3F( -0.700468f, -0.709334f, 0.078677f ),
      Point3F( -0.816193f, 0.390455f, 0.425880f ),
      Point3F( -0.043007f, 0.769222f, -0.637533f ),
      Point3F( 0.911444f, 0.113150f, 0.395560f ),
      Point3F( 0.845801f, 0.156091f, -0.510153f ),
      Point3F( 0.829801f, -0.029340f, 0.557287f ),
      Point3F( 0.259529f, 0.416263f, 0.871418f ),
      Point3F( 0.231128f, -0.845982f, 0.480515f ),
      Point3F( -0.626203f, -0.646168f, 0.436277f ),
      Point3F( -0.197047f, -0.065791f, 0.978184f ),
      Point3F( -0.255692f, -0.637488f, -0.726794f ),
      Point3F( 0.530662f, -0.844385f, -0.073567f ),
      Point3F( -0.779887f, 0.617067f, -0.104899f ),
      Point3F( 0.739908f, 0.113984f, 0.662982f ),
      Point3F( -0.218801f, 0.930194f, -0.294729f ),
      Point3F( -0.374231f, 0.818666f, 0.435589f ),
      Point3F( -0.720250f, -0.028285f, 0.693137f ),
      Point3F( 0.075389f, 0.415049f, 0.906670f ),
      Point3F( -0.539724f, -0.106620f, 0.835063f ),
      Point3F( -0.452612f, -0.754669f, -0.474991f ),
      Point3F( 0.682822f, 0.581234f, -0.442629f ),
      Point3F( 0.002435f, -0.618462f, -0.785811f ),
      Point3F( -0.397631f, 0.110766f, -0.910835f ),
      Point3F( 0.133935f, -0.985438f, 0.104754f ),
      Point3F( 0.759098f, -0.608004f, 0.232595f ),
      Point3F( -0.825239f, -0.256087f, 0.503388f ),
      Point3F( 0.101693f, -0.565568f, 0.818408f ),
      Point3F( 0.386377f, 0.793546f, -0.470104f ),
      Point3F( -0.520516f, -0.840690f, 0.149346f ),
      Point3F( -0.784549f, -0.479672f, 0.392935f ),
      Point3F( -0.325322f, -0.927581f, -0.183735f ),
      Point3F( -0.069294f, -0.428541f, 0.900861f ),
      Point3F( 0.993354f, -0.115023f, -0.004288f ),
      Point3F( -0.123896f, -0.700568f, 0.702747f ),
      Point3F( -0.438031f, -0.120880f, -0.890795f ),
      Point3F( 0.063314f, 0.813233f, 0.578484f ),
      Point3F( 0.322045f, 0.889086f, -0.325289f ),
      Point3F( -0.133521f, 0.875063f, -0.465228f ),
      Point3F( 0.637155f, 0.564814f, 0.524422f ),
      Point3F( 0.260092f, -0.669353f, 0.695930f ),
      Point3F( 0.953195f, 0.040485f, -0.299634f ),
      Point3F( -0.840665f, -0.076509f, 0.536124f ),
      Point3F( -0.971350f, 0.202093f, 0.125047f ),
      Point3F( -0.804307f, -0.396312f, -0.442749f ),
      Point3F( -0.936746f, 0.069572f, 0.343027f ),
      Point3F( 0.426545f, -0.465962f, 0.775202f ),
      Point3F( 0.794542f, -0.227450f, 0.563000f ),
      Point3F( -0.892172f, 0.091169f, -0.442399f ),
      Point3F( -0.312654f, 0.541264f, 0.780564f ),
      Point3F( 0.590603f, -0.735618f, -0.331743f ),
      Point3F( -0.098040f, -0.986713f, 0.129558f ),
      Point3F( 0.569646f, 0.283078f, -0.771603f ),
      Point3F( 0.431051f, -0.407385f, -0.805129f ),
      Point3F( -0.162087f, -0.938749f, -0.304104f ),
      Point3F( 0.241533f, -0.359509f, 0.901341f ),
      Point3F( -0.576191f, 0.614939f, 0.538380f ),
      Point3F( -0.025110f, 0.085740f, 0.996001f ),
      Point3F( -0.352693f, -0.198168f, 0.914515f ),
      Point3F( -0.604577f, 0.700711f, 0.378802f ),
      Point3F( 0.465024f, 0.409667f, 0.784809f ),
      Point3F( -0.254684f, -0.030474f, -0.966544f ),
      Point3F( -0.604789f, 0.791809f, 0.085259f ),
      Point3F( -0.705147f, -0.399298f, 0.585943f ),
      Point3F( 0.185691f, 0.017236f, -0.982457f ),
      Point3F( 0.044588f, 0.973094f, 0.226052f ),
      Point3F( -0.405463f, 0.642367f, 0.650357f ),
      Point3F( -0.563959f, 0.599136f, -0.568319f ),
      Point3F( 0.367162f, -0.072253f, -0.927347f ),
      Point3F( 0.960429f, -0.213570f, -0.178783f ),
      Point3F( -0.192629f, 0.906005f, 0.376893f ),
      Point3F( -0.199718f, -0.359865f, -0.911378f ),
      Point3F( 0.485072f, 0.121233f, -0.866030f ),
      Point3F( 0.467163f, -0.874294f, 0.131792f ),
      Point3F( -0.638953f, -0.716603f, 0.279677f ),
      Point3F( -0.622710f, 0.047813f, -0.780990f ),
      Point3F( 0.828724f, -0.054433f, -0.557004f ),
      Point3F( 0.130241f, 0.991080f, 0.028245f ),
      Point3F( 0.310995f, -0.950076f, -0.025242f ),
      Point3F( 0.818118f, 0.275336f, 0.504850f ),
      Point3F( 0.676328f, 0.387023f, 0.626733f ),
      Point3F( -0.100433f, 0.495114f, -0.863004f ),
      Point3F( -0.949609f, -0.240681f, -0.200786f ),
      Point3F( -0.102610f, 0.261831f, -0.959644f ),
      Point3F( -0.845732f, -0.493136f, 0.203850f ),
      Point3F( 0.672617f, -0.738838f, 0.041290f ),
      Point3F( 0.380465f, 0.875938f, 0.296613f ),
      Point3F( -0.811223f, 0.262027f, -0.522742f ),
      Point3F( -0.074423f, -0.775670f, -0.626736f ),
      Point3F( -0.286499f, 0.755850f, -0.588735f ),
      Point3F( 0.291182f, -0.276189f, -0.915933f ),
      Point3F( -0.638117f, 0.199338f, 0.743687f ),
      Point3F( 0.439922f, -0.864433f, -0.243359f ),
      Point3F( 0.177649f, 0.206919f, 0.962094f ),
      Point3F( 0.277107f, 0.948521f, 0.153361f ),
      Point3F( 0.507629f, 0.661918f, -0.551523f ),
      Point3F( -0.503110f, -0.579308f, -0.641313f ),
      Point3F( 0.600522f, 0.736495f, -0.311364f ),
      Point3F( -0.691096f, -0.715301f, -0.103592f ),
      Point3F( -0.041083f, -0.858497f, 0.511171f ),
      Point3F( 0.207773f, -0.480062f, -0.852274f ),
      Point3F( 0.795719f, 0.464614f, 0.388543f ),
      Point3F( -0.100433f, 0.495114f, -0.863004f ),
      Point3F( 0.703249f, 0.065157f, -0.707951f ),
      Point3F( -0.324171f, -0.941112f, 0.096024f ),
      Point3F( -0.134933f, -0.940212f, 0.312722f ),
      Point3F( -0.438240f, 0.752088f, -0.492249f ),
      Point3F( 0.964762f, -0.198855f, 0.172311f ),
      Point3F( -0.831799f, 0.196807f, 0.519015f ),
      Point3F( -0.508008f, 0.819902f, 0.263986f ),
      Point3F( 0.471075f, -0.001146f, 0.882092f ),
      Point3F( 0.919512f, 0.246162f, -0.306435f ),
      Point3F( -0.960050f, 0.279828f, -0.001187f ),
      Point3F( 0.110232f, -0.847535f, -0.519165f ),
      Point3F( 0.208229f, 0.697360f, 0.685806f ),
      Point3F( -0.199680f, -0.560621f, 0.803637f ),
      Point3F( 0.170135f, -0.679985f, -0.713214f ),
      Point3F( 0.758371f, -0.494907f, 0.424195f ),
      Point3F( 0.077734f, -0.755978f, 0.649965f ),
      Point3F( 0.612831f, -0.672475f, 0.414987f ),
      Point3F( 0.142776f, 0.836698f, -0.528726f ),
      Point3F( -0.765185f, 0.635778f, 0.101382f ),
      Point3F( 0.669873f, -0.419737f, 0.612447f ),
      Point3F( 0.593549f, 0.194879f, 0.780847f ),
      Point3F( 0.646930f, 0.752173f, 0.125368f ),
      Point3F( 0.837721f, 0.545266f, -0.030127f ),
      Point3F( 0.541505f, 0.768070f, 0.341820f ),
      Point3F( 0.760679f, -0.365715f, -0.536301f ),
      Point3F( 0.381516f, 0.640377f, 0.666605f ),
      Point3F( 0.565794f, -0.072415f, -0.821361f ),
      Point3F( -0.466072f, -0.401588f, 0.788356f ),
      Point3F( 0.987146f, 0.096290f, 0.127560f ),
      Point3F( 0.509709f, -0.688886f, -0.515396f ),
      Point3F( -0.135132f, -0.988046f, -0.074192f ),
      Point3F( 0.600499f, 0.476471f, -0.642166f ),
      Point3F( -0.732326f, -0.275320f, -0.622815f ),
      Point3F( -0.881141f, -0.470404f, 0.048078f ),
      Point3F( 0.051548f, 0.601042f, 0.797553f ),
      Point3F( 0.402027f, -0.763183f, 0.505891f ),
      Point3F( 0.404233f, -0.208288f, 0.890624f ),
      Point3F( -0.311793f, 0.343843f, 0.885752f ),
      Point3F( 0.098132f, -0.937014f, 0.335223f ),
      Point3F( 0.537158f, 0.830585f, -0.146936f ),
      Point3F( 0.725277f, 0.298172f, -0.620538f ),
      Point3F( -0.882025f, 0.342976f, -0.323110f ),
      Point3F( -0.668829f, 0.424296f, -0.610443f ),
      Point3F( -0.408835f, -0.476442f, -0.778368f ),
      Point3F( 0.809472f, 0.397249f, -0.432375f ),
      Point3F( -0.909184f, -0.205938f, -0.361903f ),
      Point3F( 0.866930f, -0.347934f, -0.356895f ),
      Point3F( 0.911660f, -0.141281f, -0.385897f ),
      Point3F( -0.431404f, -0.844074f, -0.318480f ),
      Point3F( -0.950593f, -0.073496f, 0.301614f ),
      Point3F( -0.719716f, 0.626915f, -0.298305f ),
      Point3F( -0.779887f, 0.617067f, -0.104899f ),
      Point3F( -0.475899f, -0.542630f, 0.692151f ),
      Point3F( 0.081952f, -0.157248f, -0.984153f ),
      Point3F( 0.923990f, -0.381662f, -0.024025f ),
      Point3F( -0.957998f, 0.120979f, -0.260008f ),
      Point3F( 0.306601f, 0.227975f, -0.924134f ),
      Point3F( -0.141244f, 0.989182f, 0.039601f ),
      Point3F( 0.077097f, 0.186288f, -0.979466f ),
      Point3F( -0.630407f, -0.259801f, 0.731499f ),
      Point3F( 0.718150f, 0.637408f, 0.279233f ),
      Point3F( 0.340946f, 0.110494f, 0.933567f ),
      Point3F( -0.396671f, 0.503020f, -0.767869f ),
      Point3F( 0.636943f, -0.245005f, 0.730942f ),
      Point3F( -0.849605f, -0.518660f, -0.095724f ),
      Point3F( -0.388203f, 0.105395f, 0.915527f ),
      Point3F( -0.280671f, -0.776541f, -0.564099f ),
      Point3F( -0.601680f, 0.215451f, -0.769131f ),
      Point3F( -0.660112f, -0.632371f, -0.405412f ),
      Point3F( 0.921096f, 0.284072f, 0.266242f ),
      Point3F( 0.074850f, -0.300846f, 0.950731f ),
      Point3F( 0.943952f, -0.067062f, 0.323198f ),
      Point3F( -0.917838f, -0.254589f, 0.304561f ),
      Point3F( 0.889843f, -0.409008f, 0.202219f ),
      Point3F( -0.565849f, 0.753721f, -0.334246f ),
      Point3F( 0.791460f, 0.555918f, -0.254060f ),
      Point3F( 0.261936f, 0.703590f, -0.660568f ),
      Point3F( -0.234406f, 0.952084f, 0.196444f ),
      Point3F( 0.111205f, 0.979492f, -0.168014f ),
      Point3F( -0.869844f, -0.109095f, -0.481113f ),
      Point3F( -0.337728f, -0.269701f, -0.901777f ),
      Point3F( 0.366793f, 0.408875f, -0.835634f ),
      Point3F( -0.098749f, 0.261316f, 0.960189f ),
      Point3F( -0.272379f, -0.847100f, 0.456324f ),
      Point3F( -0.319506f, 0.287444f, -0.902935f ),
      Point3F( 0.873383f, -0.294109f, 0.388203f ),
      Point3F( -0.088950f, 0.710450f, 0.698104f ),
      Point3F( 0.551238f, -0.786552f, 0.278340f ),
      Point3F( 0.724436f, -0.663575f, -0.186712f ),
      Point3F( 0.529741f, -0.606539f, 0.592861f ),
      Point3F( -0.949743f, -0.282514f, 0.134809f ),
      Point3F( 0.155047f, 0.419442f, -0.894443f ),
      Point3F( -0.562653f, -0.329139f, -0.758346f ),
      Point3F( 0.816407f, -0.576953f, 0.024576f ),
      Point3F( 0.178550f, -0.950242f, -0.255266f ),
      Point3F( 0.479571f, 0.706691f, 0.520192f ),
      Point3F( 0.391687f, 0.559884f, -0.730145f ),
      Point3F( 0.724872f, -0.205570f, -0.657496f ),
      Point3F( -0.663196f, -0.517587f, -0.540624f ),
      Point3F( -0.660054f, -0.122486f, -0.741165f ),
      Point3F( -0.531989f, 0.374711f, -0.759328f ),
      Point3F( 0.194979f, -0.059120f, 0.979024f )
};

U8 TSMesh::encodeNormal( const Point3F &normal )
{
   U8 bestIndex = 0;
   F32 bestDot = -10E30f;
   for ( U32 i = 0; i < 256; i++ )
   {
      F32 dot = mDot( normal, smU8ToNormalTable[i] );
      if ( dot > bestDot )
      {
         bestIndex = i;
         bestDot = dot;
      }
   }
   return bestIndex;
}

//-----------------------------------------------------
// TSMesh assemble from/ dissemble to memory buffer
//-----------------------------------------------------

#define tsalloc TSShape::smTSAlloc

TSMesh* TSMesh::assembleMesh( U32 meshType, bool skip )
{
   static TSMesh tempStandardMesh;
   static TSSkinMesh tempSkinMesh;
   static TSDecalMesh tempDecalMesh;
   static TSSortedMesh tempSortedMesh;

   bool justSize = skip || !tsalloc.allocShape32(0); // if this returns NULL, we're just sizing memory block

   // a little funny business because we pretend decals are derived from meshes
   S32 * ret = NULL;
   TSMesh * mesh = NULL;
   TSDecalMesh * decal = NULL;

   if ( justSize )
   {
      switch ( meshType )
      {
         case StandardMeshType :
         {
            ret = (S32*)&tempStandardMesh;
            mesh = &tempStandardMesh;
            tsalloc.allocShape32( sizeof(TSMesh) >> 2 );
            break;
         }
         case SkinMeshType     :
         {
            ret = (S32*)&tempSkinMesh;
            mesh = &tempSkinMesh;
            tsalloc.allocShape32( sizeof(TSSkinMesh) >> 2 );
            break;
         }
         case DecalMeshType    :
         {
            ret = (S32*)&tempDecalMesh;
            decal = &tempDecalMesh;
            tsalloc.allocShape32( sizeof(TSDecalMesh) >> 2 );
            break;
         }
         case SortedMeshType   :
         {
            ret = (S32*)&tempSortedMesh;
            mesh = &tempSortedMesh;
            tsalloc.allocShape32( sizeof(TSSortedMesh) >> 2 );
            break;
         }
      }
   }
   else
   {
      switch ( meshType )
      {
         case StandardMeshType :
         {
            ret = tsalloc.allocShape32( sizeof(TSMesh) >> 2 );
            constructInPlace( (TSMesh*)ret );
            mesh = (TSMesh*)ret;
            break;
         }
         case SkinMeshType     :
         {
            ret = tsalloc.allocShape32( sizeof(TSSkinMesh) >> 2 );
            constructInPlace( (TSSkinMesh*)ret );
            mesh = (TSSkinMesh*)ret;
            break;
         }
         case DecalMeshType    :
         {
            ret = tsalloc.allocShape32( sizeof(TSDecalMesh) >> 2 );
            constructInPlace((TSDecalMesh*)ret);
            decal = (TSDecalMesh*)ret;
            break;
         }
         case SortedMeshType   :
         {
            ret = tsalloc.allocShape32( sizeof(TSSortedMesh) >> 2 );
            constructInPlace( (TSSortedMesh*)ret );
            mesh = (TSSortedMesh*)ret;
            break;
         }
      }
   }

   tsalloc.setSkipMode( skip );

   if ( mesh )
      mesh->assemble( skip );

   if ( decal )
      decal->assemble( skip );

   tsalloc.setSkipMode( false );

   return (TSMesh*)ret;
}

void TSMesh::convertToTris(	const TSDrawPrimitive *primitivesIn,
							         const S32 *indicesIn,
                           	S32 numPrimIn,
         							S32 &numPrimOut, 
         							S32 &numIndicesOut,
                           	TSDrawPrimitive *primitivesOut, 
							         S32 *indicesOut ) const
{
   S32 prevMaterial = -99999;
   TSDrawPrimitive * newDraw = NULL;
   numPrimOut = 0;
   numIndicesOut = 0;
   for ( S32 i = 0; i < numPrimIn; i++ )
   {
      S32 newMat = primitivesIn[i].matIndex;
      newMat &= ~TSDrawPrimitive::TypeMask;

      U32 start = primitivesIn[i].start;
      U32 prevStart = (i > 0) ? primitivesIn[i-1].start : start;
      U32 numElements = primitivesIn[i].numElements;

      // Add a new primitive if changing materials, or if this primitive
      // indexes vertices in a different 16-bit range
      if ( ( newMat != prevMaterial ) ||
           ((indicesIn[prevStart] ^ indicesIn[start]) & 0xFFFF0000) )
      {
         if ( primitivesOut )
         {
            newDraw = &primitivesOut[numPrimOut];
            newDraw->start = numIndicesOut;
            newDraw->numElements = 0;
            newDraw->matIndex = newMat | TSDrawPrimitive::Triangles;
         }
         numPrimOut++;
         prevMaterial = newMat;
      }

      // gonna depend on what kind of primitive it is...
      if ( (primitivesIn[i].matIndex & TSDrawPrimitive::TypeMask) == TSDrawPrimitive::Triangles)
      {
         for ( S32 j = 0; j < numElements; j += 3 )
         {
            if ( indicesOut )
            {
               indicesOut[numIndicesOut + 0] = indicesIn[start + j + 0];
               indicesOut[numIndicesOut + 1] = indicesIn[start + j + 1];
               indicesOut[numIndicesOut + 2] = indicesIn[start + j + 2];
            }
            if ( newDraw )
               newDraw->numElements += 3;

            numIndicesOut += 3;
         }
      }
      else
      {
         U32 idx0 = indicesIn[start + 0];
         U32 idx1;
         U32 idx2 = indicesIn[start + 1];
         U32 * nextIdx = &idx1;
         for ( S32 j = 2; j < numElements; j++ )
         {
            *nextIdx = idx2;
            nextIdx = (U32*) ( (dsize_t)nextIdx ^ (dsize_t)&idx0 ^ (dsize_t)&idx1);
            idx2 = indicesIn[start + j];
            if ( idx0 == idx1 || idx1 == idx2 || idx2 == idx0 )
               continue;

            if ( indicesOut )
            {
               indicesOut[numIndicesOut+0] = idx0;
               indicesOut[numIndicesOut+1] = idx1;
               indicesOut[numIndicesOut+2] = idx2;
            }

            if ( newDraw )
               newDraw->numElements += 3;
            numIndicesOut += 3;
         }
      }
   }
}

void unwindStrip( const S32 * indices, S32 numElements, Vector<S32> &triIndices )
{
   U32 idx0 = indices[0];
   U32 idx1;
   U32 idx2 = indices[1];
   U32 * nextIdx = &idx1;
   for ( S32 j = 2; j < numElements; j++ )
   {
      *nextIdx = idx2;
      nextIdx = (U32*) ( (dsize_t)nextIdx ^ (dsize_t)&idx0 ^ (dsize_t)&idx1);
      idx2 = indices[j];
      if ( idx0 == idx1 || idx1 == idx2 || idx2 == idx0 )
         continue;

      triIndices.push_back( idx0 );
      triIndices.push_back( idx1 );
      triIndices.push_back( idx2 );
   }
}

void TSMesh::convertToSingleStrip(	const TSDrawPrimitive *primitivesIn, 
									         const S32 *indicesIn,
                                  	S32 numPrimIn, 
         									S32 &numPrimOut, 
         									S32 &numIndicesOut,
         									TSDrawPrimitive *primitivesOut,
         									S32 *indicesOut ) const
{
   S32 prevMaterial = -99999;
   TSDrawPrimitive * newDraw = NULL;
   TSDrawPrimitive * newTris = NULL;
   Vector<S32> triIndices;
   S32 curDrawOut = 0;
   numPrimOut = 0;
   numIndicesOut = 0;
   for ( S32 i = 0; i < numPrimIn; i++ )
   {
      S32 newMat = primitivesIn[i].matIndex;

      U32 start = primitivesIn[i].start;
      U32 prevStart = (i > 0) ? primitivesIn[i-1].start : start;
      U32 numElements = primitivesIn[i].numElements;

      // Add a new primitive if changing materials, or if this primitive
      // indexes vertices in a different 16-bit range
      if ( ( newMat != prevMaterial ) ||
           ((indicesIn[prevStart] ^ indicesIn[start]) & 0xFFFF0000) )
      {
         // before adding the new primitive, transfer triangle indices
         if ( triIndices.size() )
         {
            if ( newTris && indicesOut )
            {
               newTris->start = numIndicesOut;
               newTris->numElements = triIndices.size();
               dMemcpy(&indicesOut[numIndicesOut],triIndices.address(),triIndices.size()*sizeof(U32));
            }
            numIndicesOut += triIndices.size();
            triIndices.clear();
            newTris = NULL;
         }

         if ( primitivesOut )
         {
            newDraw = &primitivesOut[numPrimOut];
            newDraw->start = numIndicesOut;
            newDraw->numElements = 0;
            newDraw->matIndex = newMat;
         }

         numPrimOut++;
         curDrawOut = 0;
         prevMaterial = newMat;
      }

      // gonna depend on what kind of primitive it is...
      // from above we know it's the same kind as the one we're building...
      if ( (primitivesIn[i].matIndex & TSDrawPrimitive::TypeMask) == TSDrawPrimitive::Triangles)
      {
         // triangles primitive...add to it
         for ( S32 j = 0; j < numElements; j += 3 )
         {
            if ( indicesOut )
            {
               indicesOut[numIndicesOut + 0] = indicesIn[start + j + 0];
               indicesOut[numIndicesOut + 1] = indicesIn[start + j + 1];
               indicesOut[numIndicesOut + 2] = indicesIn[start + j + 2];
            }

            if ( newDraw )
               newDraw->numElements += 3;
            
            numIndicesOut += 3;
         }
      }
      else
      {
         // strip primitive...
         // if numElements less than smSmallestStripSize, add to triangles...
         if ( numElements < smMinStripSize + 2 )
         {
            // put triangle indices aside until material changes...
            if ( triIndices.empty() )
            {
               // set up for new triangle primitive and add it if we are copying data right now
               if ( primitivesOut )
               {
                  newTris = &primitivesOut[numPrimOut];
                  newTris->matIndex  = newMat;
                  newTris->matIndex &= ~(TSDrawPrimitive::Triangles|TSDrawPrimitive::Strip);
                  newTris->matIndex |= TSDrawPrimitive::Triangles;
               }

               numPrimOut++;
            }
            unwindStrip( indicesIn + start, numElements, triIndices );
         }
         else
         {
            // strip primitive...add to it
            if ( indicesOut )
            {
               if ( curDrawOut & 1 )
               {
                  indicesOut[numIndicesOut + 0] = indicesOut[numIndicesOut - 1];
                  indicesOut[numIndicesOut + 1] = indicesOut[numIndicesOut - 1];
                  indicesOut[numIndicesOut + 2] = indicesIn[start];
                  dMemcpy(indicesOut+numIndicesOut+3,indicesIn+start,numElements*sizeof(U32));
               }
               else if ( curDrawOut )
               {
                  indicesOut[numIndicesOut + 0] = indicesOut[numIndicesOut - 1];
                  indicesOut[numIndicesOut + 1] = indicesIn[start];
                  dMemcpy(indicesOut+numIndicesOut+2,indicesIn+start,numElements*sizeof(U32));
               }
               else
                  dMemcpy(indicesOut+numIndicesOut,indicesIn+start,numElements*sizeof(U32));
            }
            S32 added = numElements;
            added += curDrawOut ? (curDrawOut&1 ? 3 : 2) : 0;
            if ( newDraw )
               newDraw->numElements += added;
            
            numIndicesOut += added;
            curDrawOut += added;
         }
      }
   }
   // spit out tris before leaving
   // before adding the new primitive, transfer triangle indices
   if ( triIndices.size() )
   {
      if ( newTris && indicesOut )
      {
         newTris->start = numIndicesOut;
         newTris->numElements = triIndices.size();
         dMemcpy(&indicesOut[numIndicesOut],triIndices.address(),triIndices.size()*sizeof(U32));
      }

      numIndicesOut += triIndices.size();
      triIndices.clear();
      newTris = NULL;
   }
}

// this method does none of the converting that the above methods do, except that small strips are converted
// to triangle lists...
void TSMesh::leaveAsMultipleStrips(	const TSDrawPrimitive *primitivesIn, 
									         const S32 *indicesIn,
                                   	S32 numPrimIn, 
         									S32 &numPrimOut, 
         									S32 &numIndicesOut,
                                   	TSDrawPrimitive *primitivesOut, 
									         S32 *indicesOut ) const
{
   S32 prevMaterial = -99999;
   TSDrawPrimitive * newDraw = NULL;
   Vector<S32> triIndices;
   numPrimOut = 0;
   numIndicesOut = 0;
   for ( S32 i = 0; i < numPrimIn; i++ )
   {
      S32 newMat = primitivesIn[i].matIndex;

      U32 start = primitivesIn[i].start;
      U32 prevStart = (i > 0) ? primitivesIn[i-1].start : start;
      U32 numElements = primitivesIn[i].numElements;

      // Add a new primitive if changing materials, or if this primitive
      // indexes vertices in a different 16-bit range
      if ( triIndices.size() &&
           (( newMat != prevMaterial ) ||
           ((indicesIn[prevStart] ^ indicesIn[start]) & 0xFFFF0000) ))
      {
         // material just changed and we have triangles lying around
         // add primitive and indices for triangles and clear triIndices
         if ( indicesOut )
         {
            TSDrawPrimitive * newTris = &primitivesOut[numPrimOut];
            newTris->matIndex = prevMaterial;
            newTris->matIndex &= ~(TSDrawPrimitive::Triangles|TSDrawPrimitive::Strip);
            newTris->matIndex |= TSDrawPrimitive::Triangles;
            newTris->start = numIndicesOut;
            newTris->numElements = triIndices.size();
            dMemcpy(&indicesOut[numIndicesOut],triIndices.address(),triIndices.size()*sizeof(U32));
         }
         numPrimOut++;
         numIndicesOut += triIndices.size();
         triIndices.clear();
      }

      // this is a little convoluted because this code was adapted from convertToSingleStrip
      // but we will need a new primitive only if it is a triangle primitive coming in
      // or we have more elements than the min strip size...
      if ( (primitivesIn[i].matIndex & TSDrawPrimitive::TypeMask) == TSDrawPrimitive::Triangles || numElements>=smMinStripSize+2)
      {
         if ( primitivesOut )
         {
            newDraw = &primitivesOut[numPrimOut];
            newDraw->start = numIndicesOut;
            newDraw->numElements = 0;
            newDraw->matIndex = newMat;
         }
         numPrimOut++;
      }
      prevMaterial = newMat;

      // gonna depend on what kind of primitive it is...
      // from above we know it's the same kind as the one we're building...
      if ( (primitivesIn[i].matIndex & TSDrawPrimitive::TypeMask) == TSDrawPrimitive::Triangles)
      {
         // triangles primitive...add to it
         for ( S32 j = 0; j < numElements; j += 3 )
         {
            if ( indicesOut )
            {
               indicesOut[numIndicesOut + 0] = indicesIn[start + j + 0];
               indicesOut[numIndicesOut + 1] = indicesIn[start + j + 1];
               indicesOut[numIndicesOut + 2] = indicesIn[start + j + 2];
            }
            if ( newDraw )
               newDraw->numElements += 3;
            
            numIndicesOut += 3;
         }
      }
      else
      {
         // strip primitive...
         // if numElements less than smSmallestStripSize, add to triangles...
         if ( numElements < smMinStripSize + 2 )
            // put triangle indices aside until material changes...
            unwindStrip( indicesIn + start, numElements, triIndices );
         else
         {
            // strip primitive...add to it
            if ( indicesOut )
               dMemcpy(indicesOut+numIndicesOut,indicesIn+start,numElements*sizeof(U32));
            if ( newDraw )
               newDraw->numElements = numElements;
            
            numIndicesOut += numElements;
         }
      }
   }
   // spit out tris before leaving
   if ( triIndices.size() )
   {
      // material just changed and we have triangles lying around
      // add primitive and indices for triangles and clear triIndices
      if ( indicesOut )
      {
         TSDrawPrimitive *newTris = &primitivesOut[numPrimOut];
         newTris->matIndex = prevMaterial;
         newTris->matIndex &= ~(TSDrawPrimitive::Triangles|TSDrawPrimitive::Strip);
         newTris->matIndex |= TSDrawPrimitive::Triangles;
         newTris->start = numIndicesOut;
         newTris->numElements = triIndices.size();
         dMemcpy(&indicesOut[numIndicesOut],triIndices.address(),triIndices.size()*sizeof(U32));
      }
      numPrimOut++;
      numIndicesOut += triIndices.size();
      triIndices.clear();
   }
}

// This method retrieves data that is shared (or possibly shared) between different meshes.
// This adds an extra step to the copying of data from the memory buffer to the shape data buffer.
// If we have no parentMesh, then we either return a pointer to the data in the memory buffer
// (in the case that we skip this mesh) or copy the data into the shape data buffer and return
// that pointer (in the case that we don't skip this mesh).
// If we do have a parent mesh, then we return a pointer to the data in the shape buffer,
// copying the data in there ourselves if our parent didn't already do it (i.e., if it was skipped).
S32 * TSMesh::getSharedData32( S32 parentMesh, S32 size, S32 **source, bool skip )
{
   S32 * ptr;
   if( parentMesh < 0 )
      ptr = skip ? tsalloc.getPointer32( size ) : tsalloc.copyToShape32( size );
   else
   {
      ptr = source[parentMesh];
      // if we skipped the previous mesh (and we're not skipping this one) then
      // we still need to copy points into the shape...
      if ( !smDataCopied[parentMesh] && !skip )
      {
         S32 * tmp = ptr;
         ptr = tsalloc.allocShape32( size );
         if ( ptr && tmp )
            dMemcpy(ptr, tmp, size * sizeof(S32) );
      } 
   }
   return ptr;
}

S8 * TSMesh::getSharedData8( S32 parentMesh, S32 size, S8 **source, bool skip )
{
   S8 * ptr;
   if( parentMesh < 0 )
      ptr = skip ? tsalloc.getPointer8( size ) : tsalloc.copyToShape8( size );
   else
   {
      ptr = source[parentMesh];
      // if we skipped the previous mesh (and we're not skipping this one) then
      // we still need to copy points into the shape...
      if ( !smDataCopied[parentMesh] && !skip )
      {
         S8 * tmp = ptr;
         ptr = tsalloc.allocShape8( size );
         if ( ptr && tmp )
            dMemcpy( ptr, tmp, size * sizeof(S32) );
      }
   }
   return ptr;
}

void TSMesh::createVBIB()
{
   AssertFatal( getMeshType() != SkinMeshType, "TSMesh::createVBIB() - Invalid call for skinned mesh type!" );
   _createVBIB( mVB, mPB );
}

void TSMesh::_createVBIB( TSVertexBufferHandle &vb, GFXPrimitiveBufferHandle &pb )
{
   AssertFatal(mVertexData.isReady(), "Call convertToAlignedMeshData() before calling _createVBIB()");

   if ( mNumVerts == 0 || !GFXDevice::devicePresent() )
      return;

   PROFILE_SCOPE( TSMesh_CreateVBIB );

   // Number of verts can change in LOD skinned mesh
   const bool vertsChanged = ( vb && vb->mNumVerts < mNumVerts );

#if defined(USE_MEM_VERTEX_BUFFERS)
   if(!mDynamic)
   {
#endif
      // Create the vertex buffer
      if( vertsChanged || vb == NULL )
         vb.set( GFX, mVertSize, mVertexFormat, mNumVerts, mDynamic ? 
#if defined(TORQUE_OS_XENON)
         // Skinned meshes still will occasionally re-skin more than once per frame.
         // This cannot happen on the Xbox360. Until this issue is resolved, use
         // type volatile instead. [1/27/2010 Pat]
            GFXBufferTypeVolatile : GFXBufferTypeStatic );
#else
            GFXBufferTypeDynamic : GFXBufferTypeStatic );
#endif

      // Copy from aligned memory right into GPU memory
      U8 *vertData = (U8*)vb.lock();
      if(!vertData) return;
#if defined(TORQUE_OS_XENON)
      XMemCpyStreaming_WriteCombined( vertData, mVertexData.address(), mVertexData.mem_size() );
#else
      dMemcpy( vertData, mVertexData.address(), mVertexData.mem_size() );
#endif
      vb.unlock();
#if defined(USE_MEM_VERTEX_BUFFERS)
   }
#endif

   const bool primsChanged = ( pb.isValid() && pb->mIndexCount != indices.size() );
   if( primsChanged || pb.isNull() )
   {
      // go through and create PrimitiveInfo array
      Vector <GFXPrimitive> piArray;
      GFXPrimitive pInfo;

      U32 primitivesSize = primitives.size();
      for ( U32 i = 0; i < primitivesSize; i++ )
      {
         const TSDrawPrimitive & draw = primitives[i];

         GFXPrimitiveType drawType = getDrawType( draw.matIndex >> 30 );

         switch( drawType )
         {
         case GFXTriangleList:
            pInfo.type = drawType;
            pInfo.numPrimitives = draw.numElements / 3;
            pInfo.startIndex = draw.start;
            // Use the first index to determine which 16-bit address space we are operating in
            pInfo.startVertex = indices[draw.start] & 0xFFFF0000;
            pInfo.minIndex = 0; // minIndex are zero based index relative to startVertex. See @GFXDevice
            pInfo.numVertices = getMin((U32)0x10000, mNumVerts - pInfo.startVertex);
            break;

         case GFXTriangleStrip:
            pInfo.type = drawType;
            pInfo.numPrimitives = draw.numElements - 2;
            pInfo.startIndex = draw.start;
            // Use the first index to determine which 16-bit address space we are operating in
            pInfo.startVertex = indices[draw.start] & 0xFFFF0000;
            pInfo.minIndex = 0; // minIndex are zero based index relative to startVertex. See @GFXDevice
            pInfo.numVertices = getMin((U32)0x10000, mNumVerts - pInfo.startVertex);
            break;

         default:
            AssertFatal( false, "WTF?!" );
         }

         piArray.push_back( pInfo );
      }

      pb.set( GFX, indices.size(), piArray.size(), GFXBufferTypeStatic );

      U16 *ibIndices = NULL;
      GFXPrimitive *piInput = NULL;
      pb.lock( &ibIndices, &piInput );

      dCopyArray( ibIndices, indices.address(), indices.size() );
      dMemcpy( piInput, piArray.address(), piArray.size() * sizeof(GFXPrimitive) );

      pb.unlock();
   }
}

void TSMesh::assemble( bool skip )
{
   tsalloc.checkGuard();

   numFrames = tsalloc.get32();
   numMatFrames = tsalloc.get32();
   parentMesh = tsalloc.get32();
   tsalloc.get32( (S32*)&mBounds, 6 );
   tsalloc.get32( (S32*)&mCenter, 3 );
   mRadius = (F32)tsalloc.get32();

   S32 numVerts = tsalloc.get32();
   S32 *ptr32 = getSharedData32( parentMesh, 3 * numVerts, (S32**)smVertsList.address(), skip );
   verts.set( (Point3F*)ptr32, numVerts );

   S32 numTVerts = tsalloc.get32();
   ptr32 = getSharedData32( parentMesh, 2 * numTVerts, (S32**)smTVertsList.address(), skip );
   tverts.set( (Point2F*)ptr32, numTVerts );

   if ( TSShape::smReadVersion > 25 )
   {
      numTVerts = tsalloc.get32();
      ptr32 = getSharedData32( parentMesh, 2 * numTVerts, (S32**)smTVerts2List.address(), skip );
      tverts2.set( (Point2F*)ptr32, numTVerts );

      S32 numVColors = tsalloc.get32();
      ptr32 = getSharedData32( parentMesh, numVColors, (S32**)smColorsList.address(), skip );
      colors.set( (ColorI*)ptr32, numVColors );
   }

   S8 *ptr8;
   if ( TSShape::smReadVersion > 21 && TSMesh::smUseEncodedNormals)
   {
      // we have encoded normals and we want to use them...
      if ( parentMesh < 0 )
         tsalloc.getPointer32( numVerts * 3 ); // advance past norms, don't use
      norms.set( NULL, 0 );

      ptr8 = getSharedData8( parentMesh, numVerts, (S8**)smEncodedNormsList.address(), skip );
      encodedNorms.set( ptr8, numVerts );
   }
   else if ( TSShape::smReadVersion > 21 )
   {
      // we have encoded normals but we don't want to use them...
      ptr32 = getSharedData32( parentMesh, 3 * numVerts, (S32**)smNormsList.address(), skip );
      norms.set( (Point3F*)ptr32, numVerts );

      if ( parentMesh < 0 )
         tsalloc.getPointer8( numVerts ); // advance past encoded normls, don't use
      encodedNorms.set( NULL, 0 );
   }
   else
   {
      // no encoded normals...
      ptr32 = getSharedData32( parentMesh, 3 * numVerts, (S32**)smNormsList.address(), skip );
      norms.set( (Point3F*)ptr32, numVerts );
      encodedNorms.set( NULL, 0 );
   }

   // copy the primitives and indices...how we do this depends on what
   // form we want them in when copied...just get pointers to data for now
   S32 szPrimIn, szIndIn;
   TSDrawPrimitive *primIn;
   S32 *indIn;
   bool deleteInputArrays = false;

   if (TSShape::smReadVersion > 25)
   {
      // mesh primitives (start, numElements) and indices are stored as 32 bit values
      szPrimIn = tsalloc.get32();
      primIn = (TSDrawPrimitive*)tsalloc.getPointer32(szPrimIn*3);
      szIndIn = tsalloc.get32();
      indIn = tsalloc.getPointer32(szIndIn);
   }
   else
   {
      // mesh primitives (start, numElements) indices are stored as 16 bit values
      szPrimIn = tsalloc.get32();
      S16 *prim16 = tsalloc.getPointer16(szPrimIn*2);   // primitive: start, numElements
      S32 *prim32 = tsalloc.getPointer32(szPrimIn);     // primitive: matIndex
      szIndIn = tsalloc.get32();

      // warn about non-addressable indices
      if ( !skip && szIndIn >= 0x10000 )
      {
         Con::warnf("Mesh contains non-addressable indices, and may not render "
            "correctly. Either split this mesh into pieces of no more than 65k "
            "unique verts prior to export, or use COLLADA.");
      }

      S16 *ind16 = tsalloc.getPointer16(szIndIn);

      // need to copy to temporary arrays
      deleteInputArrays = true;
      primIn = new TSDrawPrimitive[szPrimIn];
      for (S32 i = 0; i < szPrimIn; i++)
      {
         primIn[i].start = prim16[i*2];
         primIn[i].numElements = prim16[i*2+1];
         primIn[i].matIndex = prim32[i];
      }

      indIn = new S32[szIndIn];
      dCopyArray(indIn, ind16, szIndIn);
   }

   // count the number of output primitives and indices
   S32 szPrimOut = szPrimIn, szIndOut = szIndIn;
   if (smUseTriangles)
      convertToTris(primIn, indIn, szPrimIn, szPrimOut, szIndOut, NULL, NULL);
   else if (smUseOneStrip)
      convertToSingleStrip(primIn, indIn, szPrimIn, szPrimOut, szIndOut, NULL, NULL);
   else
      leaveAsMultipleStrips(primIn, indIn, szPrimIn, szPrimOut, szIndOut, NULL, NULL);

   // allocate enough space for the new primitives and indices (all 32 bits)
   TSDrawPrimitive *primOut = (TSDrawPrimitive*)tsalloc.allocShape32(3*szPrimOut);
   S32 *indOut = tsalloc.allocShape32(szIndOut);

   // copy output primitives and indices
   S32 chkPrim = szPrimOut, chkInd = szIndOut;
   if (smUseTriangles)
      convertToTris(primIn, indIn, szPrimIn, chkPrim, chkInd, primOut, indOut);
   else if (smUseOneStrip)
      convertToSingleStrip(primIn, indIn, szPrimIn, chkPrim, chkInd, primOut, indOut);
   else
      leaveAsMultipleStrips(primIn, indIn, szPrimIn, chkPrim, chkInd, primOut, indOut);
   AssertFatal(chkPrim==szPrimOut && chkInd==szIndOut,"TSMesh::primitive conversion");

   // store output
   primitives.set(primOut, szPrimOut);
   indices.set(indOut, szIndOut);

   // delete temporary arrays if necessary
   if (deleteInputArrays)
   {
      delete [] primIn;
      delete [] indIn;
   }

   S32 sz = tsalloc.get32();
   tsalloc.getPointer16( sz ); // skip deprecated merge indices
   tsalloc.align32();

   vertsPerFrame = tsalloc.get32();
   U32 flags = (U32)tsalloc.get32();
   if ( encodedNorms.size() )
      flags |= UseEncodedNormals;
   
   setFlags( flags );

   tsalloc.checkGuard();

   if ( tsalloc.allocShape32( 0 ) && TSShape::smReadVersion < 19 )
      computeBounds(); // only do this if we copied the data...

   if(getMeshType() != SkinMeshType)
      createTangents(verts, norms);
}

void TSMesh::disassemble()
{
   tsalloc.setGuard();

   tsalloc.set32( numFrames );
   tsalloc.set32( numMatFrames );
   tsalloc.set32( parentMesh );
   tsalloc.copyToBuffer32( (S32*)&mBounds, 6 );
   tsalloc.copyToBuffer32( (S32*)&mCenter, 3 );
   tsalloc.set32( (S32)mRadius );

   // Re-create the vectors
   if(mVertexData.isReady())
   {
      verts.setSize(mNumVerts);
      tverts.setSize(mNumVerts);
      norms.setSize(mNumVerts);

      if(mHasColor)
         colors.setSize(mNumVerts);
      if(mHasTVert2)
         tverts2.setSize(mNumVerts);

      // Fill arrays
      for(U32 i = 0; i < mNumVerts; i++)
      {
         const __TSMeshVertexBase &cv = mVertexData[i];
         verts[i] = cv.vert();
         tverts[i] = cv.tvert();
         norms[i] = cv.normal();

         if(mHasColor)
            cv.color().getColor(&colors[i]);
         if(mHasTVert2)
            tverts2[i] = cv.tvert2();
      }
   }

   // verts...
   tsalloc.set32( verts.size() );
   if ( parentMesh < 0 )
      tsalloc.copyToBuffer32( (S32*)verts.address(), 3 * verts.size() ); // if no parent mesh, then save off our verts

   // tverts...
   tsalloc.set32( tverts.size() );
   if ( parentMesh < 0 )
      tsalloc.copyToBuffer32( (S32*)tverts.address(), 2 * tverts.size() ); // if no parent mesh, then save off our tverts

   if (TSShape::smVersion > 25)
   {
      // tverts2...
      tsalloc.set32( tverts2.size() );
      if ( parentMesh < 0 )
         tsalloc.copyToBuffer32( (S32*)tverts2.address(), 2 * tverts2.size() ); // if no parent mesh, then save off our tverts

      // colors
      tsalloc.set32( colors.size() );
      if ( parentMesh < 0 )
         tsalloc.copyToBuffer32( (S32*)colors.address(), colors.size() ); // if no parent mesh, then save off our tverts
   }

   // norms...
   if ( parentMesh < 0 ) // if no parent mesh, then save off our norms
      tsalloc.copyToBuffer32( (S32*)norms.address(), 3 * norms.size() ); // norms.size()==verts.size() or error...

   // encoded norms...
   if ( parentMesh < 0 )
   {
      // if no parent mesh, compute encoded normals and copy over
      for ( S32 i = 0; i < norms.size(); i++ )
      {
         U8 normIdx = encodedNorms.size() ? encodedNorms[i] : encodeNormal( norms[i] );
         tsalloc.copyToBuffer8( (S8*)&normIdx, 1 );
      }
   }

   // optimize triangle draw order during disassemble
   {
      FrameTemp<TriListOpt::IndexType> tmpIdxs(indices.size());
      for ( S32 i = 0; i < primitives.size(); i++ )
      {
         const TSDrawPrimitive& prim = primitives[i];

         // only optimize triangle lists (strips and fans are assumed to be already optimized)
         if ( (prim.matIndex & TSDrawPrimitive::TypeMask) == TSDrawPrimitive::Triangles )
         {
            TriListOpt::OptimizeTriangleOrdering(verts.size(), prim.numElements,
               indices.address() + prim.start, tmpIdxs.address());
            dCopyArray(indices.address() + prim.start, tmpIdxs.address(), 
               prim.numElements);
         }
      }
   }

   if (TSShape::smVersion > 25)
   {
      // primitives...
      tsalloc.set32( primitives.size() );
      tsalloc.copyToBuffer32((S32*)primitives.address(),3*primitives.size());

      // indices...
      tsalloc.set32(indices.size());
      tsalloc.copyToBuffer32((S32*)indices.address(),indices.size());
   }
   else
   {
      // primitives
      tsalloc.set32( primitives.size() );
      for (S32 i=0; i<primitives.size(); i++)
      {
         S16 start = (S16)primitives[i].start;
         S16 numElements = (S16)primitives[i].numElements;

         tsalloc.copyToBuffer16(&start, 1);
         tsalloc.copyToBuffer16(&numElements, 1);
         tsalloc.copyToBuffer32(&(primitives[i].matIndex), 1);
      }

      // indices
      tsalloc.set32(indices.size());
      Vector<S16> s16_indices(indices.size());
      for (S32 i=0; i<indices.size(); i++)
         s16_indices.push_back((S16)indices[i]);
      tsalloc.copyToBuffer16(s16_indices.address(), s16_indices.size());
   }

   // merge indices...DEPRECATED
   tsalloc.set32( 0 );

   // small stuff...
   tsalloc.set32( vertsPerFrame );
   tsalloc.set32( getFlags() );

   tsalloc.setGuard();
}

//-----------------------------------------------------------------------------
// TSSkinMesh assemble from/ dissemble to memory buffer
//-----------------------------------------------------------------------------
void TSSkinMesh::assemble( bool skip )
{
   // avoid a crash on computeBounds...
   batchData.initialVerts.set( NULL, 0 );

   TSMesh::assemble( skip );

   S32 sz = tsalloc.get32();
   S32 numVerts = sz;
   S32 * ptr32 = getSharedData32( parentMesh, 3 * numVerts, (S32**)smVertsList.address(), skip );
   batchData.initialVerts.set( (Point3F*)ptr32, sz );

   S8 * ptr8;
   if ( TSShape::smReadVersion>21 && TSMesh::smUseEncodedNormals )
   {
      // we have encoded normals and we want to use them...
      if ( parentMesh < 0 )
         tsalloc.getPointer32( numVerts * 3 ); // advance past norms, don't use
      batchData.initialNorms.set( NULL, 0 );

      ptr8 = getSharedData8( parentMesh, numVerts, (S8**)smEncodedNormsList.address(), skip );
      encodedNorms.set( ptr8, numVerts );
      // Note: we don't set the encoded normals flag because we handle them in updateSkin and
      //       hide the fact that we are using them from base class (TSMesh)
   }
   else if ( TSShape::smReadVersion > 21 )
   {
      // we have encoded normals but we don't want to use them...
      ptr32 = getSharedData32( parentMesh, 3 * numVerts, (S32**)smNormsList.address(), skip );
      batchData.initialNorms.set( (Point3F*)ptr32, numVerts );

      if ( parentMesh < 0 )
         tsalloc.getPointer8( numVerts ); // advance past encoded normls, don't use
      
      encodedNorms.set( NULL, 0 );
   }
   else
   {
      // no encoded normals...
      ptr32 = getSharedData32( parentMesh, 3 * numVerts, (S32**)smNormsList.address(), skip );
      batchData.initialNorms.set( (Point3F*)ptr32, numVerts );
      encodedNorms.set( NULL, 0 );
   }

   sz = tsalloc.get32();
   ptr32 = getSharedData32( parentMesh, 16 * sz, (S32**)smInitTransformList.address(), skip );
   batchData.initialTransforms.set( ptr32, sz );

   sz = tsalloc.get32();
   ptr32 = getSharedData32( parentMesh, sz, (S32**)smVertexIndexList.address(), skip );
   vertexIndex.set( ptr32, sz );

   ptr32 = getSharedData32( parentMesh, sz, (S32**)smBoneIndexList.address(), skip );
   boneIndex.set( ptr32, sz );

   ptr32 = getSharedData32( parentMesh, sz, (S32**)smWeightList.address(), skip );
   weight.set( (F32*)ptr32, sz );

   sz = tsalloc.get32();
   ptr32 = getSharedData32( parentMesh, sz, (S32**)smNodeIndexList.address(), skip );
   batchData.nodeIndex.set( ptr32, sz );

   tsalloc.checkGuard();

   if ( tsalloc.allocShape32( 0 ) && TSShape::smReadVersion < 19 )
      TSMesh::computeBounds(); // only do this if we copied the data...

   createTangents(batchData.initialVerts, batchData.initialNorms);
}

//-----------------------------------------------------------------------------
// disassemble
//-----------------------------------------------------------------------------
void TSSkinMesh::disassemble()
{
   TSMesh::disassemble();

   tsalloc.set32( batchData.initialVerts.size() );
   // if we have no parent mesh, then save off our verts & norms
   if ( parentMesh < 0 )
   {
      tsalloc.copyToBuffer32( (S32*)batchData.initialVerts.address(), 3 * batchData.initialVerts.size() );

      // no longer do this here...let tsmesh handle this
      tsalloc.copyToBuffer32( (S32*)batchData.initialNorms.address(), 3 * batchData.initialNorms.size() );

      // if no parent mesh, compute encoded normals and copy over
      for ( S32 i = 0; i < batchData.initialNorms.size(); i++ )
      {
         U8 normIdx = encodedNorms.size() ? encodedNorms[i] : encodeNormal( batchData.initialNorms[i] );
         tsalloc.copyToBuffer8( (S8*)&normIdx, 1 );
      }
   }

   tsalloc.set32( batchData.initialTransforms.size() );
   if ( parentMesh < 0 )
      tsalloc.copyToBuffer32( (S32*)batchData.initialTransforms.address(), batchData.initialTransforms.size() * 16 );

   tsalloc.set32( vertexIndex.size() );
   if ( parentMesh < 0 )
   {
      tsalloc.copyToBuffer32( (S32*)vertexIndex.address(), vertexIndex.size() );

      tsalloc.copyToBuffer32( (S32*)boneIndex.address(), boneIndex.size() );

      tsalloc.copyToBuffer32( (S32*)weight.address(), weight.size() );
   }

   tsalloc.set32( batchData.nodeIndex.size() );
   if ( parentMesh < 0 )
      tsalloc.copyToBuffer32( (S32*)batchData.nodeIndex.address(), batchData.nodeIndex.size() );

   tsalloc.setGuard();
}

TSSkinMesh::TSSkinMesh()
{
   meshType = SkinMeshType;
   mDynamic = true;
   batchDataInitialized = false;
}

//-----------------------------------------------------------------------------
// find tangent vector
//-----------------------------------------------------------------------------
inline void TSMesh::findTangent( U32 index1, 
                                 U32 index2, 
                                 U32 index3, 
                                 Point3F *tan0, 
                                 Point3F *tan1,
                                 const Vector<Point3F> &_verts)
{
   const Point3F &v1 = _verts[index1];
   const Point3F &v2 = _verts[index2];
   const Point3F &v3 = _verts[index3];

   const Point2F &w1 = tverts[index1];
   const Point2F &w2 = tverts[index2];
   const Point2F &w3 = tverts[index3];

   F32 x1 = v2.x - v1.x;
   F32 x2 = v3.x - v1.x;
   F32 y1 = v2.y - v1.y;
   F32 y2 = v3.y - v1.y;
   F32 z1 = v2.z - v1.z;
   F32 z2 = v3.z - v1.z;

   F32 s1 = w2.x - w1.x;
   F32 s2 = w3.x - w1.x;
   F32 t1 = w2.y - w1.y;
   F32 t2 = w3.y - w1.y;

   F32 denom = (s1 * t2 - s2 * t1);

   if( mFabs( denom ) < 0.0001f )
   {
	   // handle degenerate triangles from strips
	   if (denom<0) denom = -0.0001f;
	   else denom = 0.0001f;
   }
   F32 r = 1.0f / denom;

   Point3F sdir(  (t2 * x1 - t1 * x2) * r, 
                  (t2 * y1 - t1 * y2) * r, 
                  (t2 * z1 - t1 * z2) * r );

   Point3F tdir(  (s1 * x2 - s2 * x1) * r, 
                  (s1 * y2 - s2 * y1) * r, 
                  (s1 * z2 - s2 * z1) * r );


   tan0[index1]  += sdir;
   tan1[index1]  += tdir;

   tan0[index2]  += sdir;
   tan1[index2]  += tdir;

   tan0[index3]  += sdir;
   tan1[index3]  += tdir;
}

//-----------------------------------------------------------------------------
// create array of tangent vectors
//-----------------------------------------------------------------------------
void TSMesh::createTangents(const Vector<Point3F> &_verts, const Vector<Point3F> &_norms)
{
   U32 numVerts = _verts.size();
   U32 numNorms = _norms.size();
   if ( numVerts <= 0 || numNorms <= 0 )
      return;

   if( numVerts != numNorms)
      return;

   Vector<Point3F> tan0;
   tan0.setSize( numVerts * 2 );

   Point3F *tan1 = tan0.address() + numVerts;
   dMemset( tan0.address(), 0, sizeof(Point3F) * 2 * numVerts );
   
   U32   numPrimatives = primitives.size();

   for (S32 i = 0; i < numPrimatives; i++ )
   {
      const TSDrawPrimitive & draw = primitives[i];
      GFXPrimitiveType drawType = getDrawType( draw.matIndex >> 30 );

      U32 p1Index = 0;
      U32 p2Index = 0;

      U32 *baseIdx = &indices[draw.start];

      const U32 numElements = (U32)draw.numElements;

      switch( drawType )
      {
      case GFXTriangleList:
         {
            for( U32 j = 0; j < numElements; j += 3 )
               findTangent( baseIdx[j], baseIdx[j + 1], baseIdx[j + 2], tan0.address(), tan1, _verts );
            break;
         }

      case GFXTriangleStrip:
         {
            p1Index = baseIdx[0];
            p2Index = baseIdx[1];
            for( U32 j = 2; j < numElements; j++ )
            {
               findTangent( p1Index, p2Index, baseIdx[j], tan0.address(), tan1, _verts );
               p1Index = p2Index;
               p2Index = baseIdx[j];
            }
            break;
         }

      default:
         AssertFatal( false, "TSMesh::createTangents: unknown primitive type!" );
      }
   }

   tangents.setSize( numVerts );

   // fill out final info from accumulated basis data
   for( U32 i = 0; i < numVerts; i++ )
   {
      const Point3F &n = _norms[i];
      const Point3F &t = tan0[i];
      const Point3F &b = tan1[i];

      Point3F tempPt = t - n * mDot( n, t );
      tempPt.normalize();
      tangents[i] = tempPt;

      Point3F cp;
      mCross( n, t, &cp );
      
      tangents[i].w = (mDot( cp, b ) < 0.0f) ? -1.0f : 1.0f;
   }
}

void TSMesh::convertToAlignedMeshData()
{
   if(!mVertexData.isReady())
      _convertToAlignedMeshData(mVertexData, verts, norms);
}


void TSSkinMesh::convertToAlignedMeshData()
{
   if(!mVertexData.isReady())
      _convertToAlignedMeshData(mVertexData, batchData.initialVerts, batchData.initialNorms);
}

void TSMesh::_convertToAlignedMeshData( TSMeshVertexArray &vertexData, const Vector<Point3F> &_verts, const Vector<Point3F> &_norms )
{
   // If mVertexData is ready, and the input array is different than mVertexData
   // use mVertexData to quickly initialize the input array
   if(mVertexData.isReady() && vertexData.address() != mVertexData.address())
   {
      AssertFatal(mVertexData.size() == mNumVerts, "Vertex data length mismatch; no idea how this happened.");

      // There doesn't seem to be an _mm_realloc, even though there is an _aligned_realloc
      // We really shouldn't be re-allocating anyway. Should TSShapeInstance be
      // storing an array of the data structures? That would certainly bloat memory.
      void *aligned_mem = dMalloc_aligned(mVertSize * mNumVerts, 16);
      AssertFatal(aligned_mem, "Aligned malloc failed! Debug!");

      vertexData.set(aligned_mem, mVertSize, mNumVerts);
      vertexData.setReady(true);

#if defined(TORQUE_OS_XENON)
      XMemCpyStreaming(vertexData.address(), mVertexData.address(), vertexData.mem_size() );
#else
      dMemcpy(vertexData.address(), mVertexData.address(), vertexData.mem_size());
#endif
      return;
   }


   AssertFatal(!vertexData.isReady(), "Mesh already converted to aligned data! Re-check code!");
   AssertFatal(_verts.size() == _norms.size() &&
               _verts.size() == tangents.size(), 
               "Vectors: verts, norms, tangents must all be the same size");
   mNumVerts = _verts.size();

   // Initialize the vertex data
   vertexData.set(NULL, 0, 0);
   vertexData.setReady(true);

   if(mNumVerts == 0)
      return;

   mHasColor = !colors.empty();
   AssertFatal(!mHasColor || colors.size() == _verts.size(), "Vector of color elements should be the same size as other vectors");

   mHasTVert2 = !tverts2.empty();
   AssertFatal(!mHasTVert2 || tverts2.size() == _verts.size(), "Vector of tvert2 elements should be the same size as other vectors");

   // Create the proper array type
   void *aligned_mem = dMalloc_aligned(mVertSize * mNumVerts, 16);
   AssertFatal(aligned_mem, "Aligned malloc failed! Debug!");

   dMemset(aligned_mem, 0, mNumVerts * mVertSize);
   vertexData.set(aligned_mem, mVertSize, mNumVerts);

   for(U32 i = 0; i < mNumVerts; i++)
   {
      __TSMeshVertexBase &v = vertexData[i];
      v.vert(_verts[i]);
      v.normal(_norms[i]);
      v.tangent(tangents[i]);

      if(i < tverts.size())
         v.tvert(tverts[i]);
      if(mHasTVert2 && i < tverts2.size())
         v.tvert2(tverts2[i]);
      if(mHasColor && i < colors.size())
         v.color(colors[i]);
   }

   // Now that the data is in the aligned struct, free the Vector memory
   verts.free_memory();
   norms.free_memory();
   tangents.free_memory();
   tverts.free_memory();
   tverts2.free_memory();
   colors.free_memory();
}