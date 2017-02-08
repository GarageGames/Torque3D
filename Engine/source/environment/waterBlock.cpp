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
#include "environment/waterBlock.h"

#include "core/util/safeDelete.h"
#include "scene/sceneRenderState.h"
#include "scene/sceneManager.h"
#include "lighting/lightInfo.h"
#include "core/stream/bitStream.h"
#include "math/mathIO.h"
#include "console/consoleTypes.h"
#include "gui/3d/guiTSControl.h"
#include "gfx/primBuilder.h"
#include "gfx/gfxTransformSaver.h"
#include "gfx/gfxDebugEvent.h"
#include "gfx/gfxOcclusionQuery.h"
#include "renderInstance/renderPassManager.h"
#include "sim/netConnection.h"   
#include "scene/reflectionManager.h"
#include "ts/tsShapeInstance.h"
#include "postFx/postEffect.h"
#include "math/util/matrixSet.h"

IMPLEMENT_CO_NETOBJECT_V1(WaterBlock);

ConsoleDocClass( WaterBlock,
   "@brief A block shaped water volume defined by a 3D scale and orientation.\n\n"      

   "@see WaterObject for inherited functionality.\n\n"
   
   "@ingroup Water"
);

WaterBlock::WaterBlock()
{
   mGridElementSize = 5.0f;
   mObjScale.set( 100.0f, 100.0f, 10.0f );

   mNetFlags.set(Ghostable | ScopeAlways);

   mObjBox.minExtents.set( -0.5f, -0.5f, -0.5f );
   mObjBox.maxExtents.set(  0.5f,  0.5f,  0.5f );

   mElapsedTime = 0.0f;   
   mGenerateVB = true;
}

WaterBlock::~WaterBlock()
{
}

bool WaterBlock::onAdd()
{
   if ( !Parent::onAdd() )
      return false;
   
   resetWorldBox();
   addToScene();

   return true;
}

void WaterBlock::onRemove()
{
   clearVertBuffers();

   removeFromScene();

   Parent::onRemove();
}

//-----------------------------------------------------------------------------
// packUpdate
//-----------------------------------------------------------------------------
U32 WaterBlock::packUpdate(NetConnection* con, U32 mask, BitStream* stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   stream->write( mGridElementSize );

   if ( stream->writeFlag( mask & UpdateMask ) )
   {
      // This is set to allow the user to modify the size of the water dynamically
      // in the editor
      mathWrite( *stream, mObjScale );
      stream->writeAffineTransform( mObjToWorld );      
   }

   return retMask;
}

//-----------------------------------------------------------------------------
// unpackUpdate
//-----------------------------------------------------------------------------
void WaterBlock::unpackUpdate(NetConnection* con, BitStream* stream)
{
   Parent::unpackUpdate(con, stream);

   F32 gridSize = mGridElementSize;
   stream->read( &mGridElementSize );
   if ( gridSize != mGridElementSize )
      mGenerateVB = true;

   if( stream->readFlag() ) // UpdateMask
   {
      Point3F scale;
      mathRead( *stream, &scale );
            
      setScale( scale );

      MatrixF objToWorld;
      stream->readAffineTransform( &objToWorld );      

      setTransform( objToWorld );
   } 
}

//-----------------------------------------------------------------------------
// Setup vertex and index buffers
//-----------------------------------------------------------------------------
void WaterBlock::setupVBIB()
{
   clearVertBuffers();

   const U32 maxIndexedVerts = 65536; // max number of indexed verts with U16 size indices

   if( mObjScale.x < mGridElementSize ||
       mObjScale.y < mGridElementSize )
   {
      F32 oldGridSize = mGridElementSize;
      mGridElementSize = getMin(mObjScale.x, mObjScale.y);
      logWarning("gridElementSize %g is larger than scale (%g, %g), clamping gridElementSize to %g",
         oldGridSize, mObjScale.x, mObjScale.y, mGridElementSize);
   }

   Point3F div = getScale() / mGridElementSize;
   // Add one to width and height for the edge.
   mWidth   = (U32)mCeil(div.x) + 1;
   mHeight  = (U32)mCeil(div.y) + 1;
   
   if( mWidth > maxIndexedVerts / 2 )
      mWidth = maxIndexedVerts / 2;

   // figure out how many blocks are needed and their size
   U32 maxBlockRows = maxIndexedVerts / mWidth;
   U32 rowOffset = 0;
   
   while( (rowOffset+1) < mHeight )
   {
      U32 numRows = mHeight - rowOffset;
      if( numRows == 1 ) numRows++;
      if( numRows > maxBlockRows )
      {
         numRows = maxBlockRows;
      }

      setupVertexBlock( mWidth, numRows, rowOffset );
      setupPrimitiveBlock( mWidth, numRows );

      rowOffset += numRows - 1;
   }

}

//-----------------------------------------------------------------------------
// Set up a block of vertices - the width is always the width of the entire
// waterBlock, so this is a block of full rows.
//-----------------------------------------------------------------------------
void WaterBlock::setupVertexBlock( U32 width, U32 height, U32 rowOffset )
{
   RayInfo rInfo;
   VectorF sunVector(-0.61f, 0.354f, 0.707f);

   if ( LIGHTMGR )
   {
      LightInfo* linfo = LIGHTMGR->getSpecialLight( LightManager::slSunLightType );
      if ( linfo )
         sunVector = linfo->getDirection();
   }

   sunVector.normalize();

   U32 numVerts = width * height;

   GFXWaterVertex *verts = new GFXWaterVertex[ numVerts ];

   U32 index = 0;
   for( U32 i=0; i<height; i++ )
   {
      for( U32 j=0; j<width; j++, index++ )
      {
         F32 vertX = getMin((-mObjScale.x / 2.0f) + mGridElementSize * j, mObjScale.x / 2.0f);
         F32 vertY = getMin((-mObjScale.y / 2.0f) + mGridElementSize * (i + rowOffset), mObjScale.y / 2.0f);
         GFXWaterVertex *vert = &verts[index];
         vert->point.x = vertX;
         vert->point.y = vertY;
         vert->point.z = 0.0;
         vert->normal.set(0,0,1);
         vert->undulateData.set( vertX, vertY );
         vert->horizonFactor.set( 0, 0, 0, 0 );

         // Calculate the water depth

         /*
         vert->depthData.set( 0.0f, 0.0f );

         Point3F start, end;
         Point3F worldPoint = vert->point + pos;

         start.x = end.x = worldPoint.x;
         start.y = end.y = worldPoint.y;
         start.z = -2000; // Really high, might be over kill
         end.z = 2000; // really low, might be overkill

         // Cast a ray to see how deep the water is. We are
         // currently just testing for terrain and atlas
         // objects, but potentially any object that responds
         // to a ray cast could detected.
         if(gClientContainer.castRay(start, end, 
            //StaticObjectType | 
            //InteriorObjectType | 
            //ShapeBaseObjectType | 
            //StaticShapeObjectType | 
            //ItemObjectType |
            //StaticTSObjectType |
            TerrainObjectType
            , &rInfo))
         {
            F32 depth = -(rInfo.point.z - pos.z);

            if(depth <= 0.0f)
            {
               depth = 1.0f;
            }
            else
            {
               depth = depth / mVisibilityDepth;
               if(depth > 1.0f)
               {
                  depth = 1.0f;
               }

               depth = 1.0f - depth;
            }

            vert->depthData.x = depth;
         }
         else
         {
            vert->depthData.x = 0.0f;
         }

         // Cast a ray to do some AO-style shadowing.
         F32 &shadow = vert->depthData.y;

         if(gClientContainer.castRay(worldPoint, worldPoint + sunVector * 9000.f, 
            //StaticObjectType | 
            //InteriorObjectType | 
            //ShapeBaseObjectType | 
            //StaticShapeObjectType | 
            //ItemObjectType |
            //StaticTSObjectType |
            TerrainObjectType
            , &rInfo))
         {
            shadow = 0.f;
         }
         else
         {
            shadow = 1.f;
         }
         */
      }
   }

   // copy to vertex buffer
   GFXVertexBufferHandle <GFXWaterVertex> * vertBuff = new GFXVertexBufferHandle <GFXWaterVertex>;

   vertBuff->set( GFX, numVerts, GFXBufferTypeStatic );
   GFXWaterVertex *vbVerts = vertBuff->lock();
   dMemcpy( vbVerts, verts, sizeof(GFXWaterVertex) * numVerts );
   vertBuff->unlock();
   mVertBuffList.push_back( vertBuff );
   

   delete [] verts;

}

//-----------------------------------------------------------------------------
// Set up a block of indices to match the block of vertices. The width is 
// always the width of the entire waterBlock, so this is a block of full rows.
//-----------------------------------------------------------------------------
void WaterBlock::setupPrimitiveBlock( U32 width, U32 height )
{
   AssertFatal( height > 1, "WaterBlock::setupPrimitiveBlock() - invalid height" );
   
   // setup vertex / primitive buffers
   U32 numIndices = (width-1) * (height-1) * 6;
   U16 *indices = new U16[ numIndices ];
   U32 numVerts = width * height;

   // This uses indexed triangle lists instead of strips, but it shouldn't be
   // significantly slower if the indices cache well.
   
   // Rough diagram of the index order
   //   0----2----+ ...
   //   |  / |    |
   //   |/   |    |
   //   1----3----+ ...
   //   |    |    |
   //   |    |    |
   //   +----+----+ ...

   U32 index = 0;
   for( U32 i=0; i<(height-1); i++ )
   {
      for( U32 j=0; j<(width-1); j++, index+=6 )
      {
         // Process one quad at a time.  Note it will re-use the same indices from
         // previous quad, thus optimizing vert cache.  Cache will run out at
         // end of each row with this implementation however.
         indices[index+0] = (i) * mWidth + j;         // 0
         indices[index+1] = (i+1) * mWidth + j;       // 1
         indices[index+2] =  i * mWidth + j+1;        // 2
         indices[index+3] = (i+1) * mWidth + j;       // 1
         indices[index+4] = (i+1) * mWidth + j+1;     // 3
         indices[index+5] =  i * mWidth + j+1;        // 2
      }

   }

   GFXPrimitiveBufferHandle *indexBuff = new GFXPrimitiveBufferHandle;
   
   GFXPrimitive pInfo;
   pInfo.type = GFXTriangleList;
   pInfo.numPrimitives = numIndices / 3;
   pInfo.startIndex = 0;
   pInfo.minIndex = 0;
   pInfo.numVertices = numVerts;

   U16 *ibIndices;
   GFXPrimitive *piInput;
   indexBuff->set( GFX, numIndices, 1, GFXBufferTypeStatic );
   indexBuff->lock( &ibIndices, &piInput );
   dMemcpy( ibIndices, indices, numIndices * sizeof(U16) );
   dMemcpy( piInput, &pInfo, sizeof(GFXPrimitive) );
   indexBuff->unlock();
   mPrimBuffList.push_back( indexBuff );


   delete [] indices;
}

//------------------------------------------------------------------------------
// Setup scenegraph data structure for materials
//------------------------------------------------------------------------------
SceneData WaterBlock::setupSceneGraphInfo( SceneRenderState *state )
{
   SceneData sgData;

   sgData.lights[0] = LIGHTMGR->getSpecialLight( LightManager::slSunLightType );

   // fill in water's transform
   sgData.objTrans = &getRenderTransform();

   // fog
   sgData.setFogParams( state->getSceneManager()->getFogData() );

   // misc
   sgData.backBuffTex = REFLECTMGR->getRefractTex();
   sgData.reflectTex = mPlaneReflector.reflectTex;
   sgData.wireframe = GFXDevice::getWireframe() || smWireframe;

   return sgData;
}

//-----------------------------------------------------------------------------
// set shader parameters
//-----------------------------------------------------------------------------
void WaterBlock::setShaderParams( SceneRenderState *state, BaseMatInstance *mat, const WaterMatParams &paramHandles)
{
   // Set variables that will be assigned to shader consts within WaterCommon
   // before calling Parent::setShaderParams

   mUndulateMaxDist = F32_MAX;

   Parent::setShaderParams( state, mat, paramHandles );   

   // Now set the rest of the shader consts that are either unique to this
   // class or that WaterObject leaves to us to handle...

   MaterialParameters* matParams = mat->getMaterialParameters();
   
   // set vertex shader constants
   //-----------------------------------   
   
   MatrixF modelMat( getRenderTransform() );
   if ( paramHandles.mModelMatSC->isValid() )
      matParams->set(paramHandles.mModelMatSC, modelMat, GFXSCT_Float4x4);
   matParams->setSafe(paramHandles.mGridElementSizeSC, (F32)mGridElementSize);

   // set pixel shader constants
   //-----------------------------------

   ColorF c( mWaterFogData.color );
   matParams->setSafe( paramHandles.mBaseColorSC, c );
      
   // By default we need to show a true reflection is fullReflect is enabled and
   // we are above water.
   F32 reflect = mPlaneReflector.isEnabled() && !isUnderwater( state->getCameraPosition() );
   
   // If we were occluded the last frame a query was fetched ( not necessarily last frame )
   // and we weren't updated last frame... we don't have a valid texture to show
   // so use the cubemap / fake reflection color this frame.
   if ( mPlaneReflector.lastUpdateMs != REFLECTMGR->getLastUpdateMs() && mPlaneReflector.isOccluded() )
      reflect = false;

   Point4F reflectParams( mWaterPos.z, 0.0f, 1000.0f, !reflect );   
   matParams->setSafe( paramHandles.mReflectParamsSC, reflectParams );

   VectorF reflectNorm = mReflectNormalUp ? VectorF(0,0,1) : static_cast<VectorF>(mPlaneReflector.refplane);
   matParams->setSafe(paramHandles.mReflectNormalSC, reflectNorm ); 
}

void WaterBlock::innerRender( SceneRenderState *state )
{      
   GFXDEBUGEVENT_SCOPE( WaterBlock_innerRender, ColorI( 255, 0, 0 ) );

   if ( mGenerateVB )
   {
      setupVBIB();
      mGenerateVB = false;
   }

   // Setup SceneData
   SceneData sgData = setupSceneGraphInfo( state );
   const Point3F &camPosition = state->getCameraPosition();

   // set the material

   S32 matIdx = getMaterialIndex( camPosition );

   if ( !initMaterial( matIdx ) )
      return;

   BaseMatInstance *mat = mMatInstances[matIdx];
   WaterMatParams matParams = mMatParamHandles[matIdx];

   // render the geometry
   if ( mat )
   {
      // setup proj/world transform
      mMatrixSet->setWorld(getRenderTransform());
      mMatrixSet->restoreSceneViewProjection();

      setShaderParams( state, mat, matParams );

      while ( mat->setupPass( state, sgData ) )
      {      
         mat->setSceneInfo(state, sgData);
         mat->setTransforms(*mMatrixSet, state);
         setCustomTextures( matIdx, mat->getCurPass(), matParams );

         for ( U32 i = 0; i < mVertBuffList.size(); i++ )
         {
            GFX->setVertexBuffer( *mVertBuffList[i] );
            GFXPrimitiveBuffer *primBuff = *mPrimBuffList[i];
            GFX->setPrimitiveBuffer( primBuff );
            GFX->drawPrimitives();
         }
      }
   }   
}

bool WaterBlock::setGridSizeProperty( void *obj, const char *index, const char *data )
{
   WaterBlock* object = static_cast<WaterBlock*>(obj);
   F32 gridSize = dAtof(data);
   
   Point3F scale = object->getScale();
   
   if(gridSize < 0.001f)
   {
      object->logWarning("gridSize cannot be <= 0, clamping to scale");
      gridSize = getMin(scale.x, scale.y);
   }
   
   if(gridSize > scale.x || gridSize > scale.y)
   {
      object->logWarning("gridSize cannot be > scale.  Your scale is (%g, %g) and your gridsize is %g", 
                 scale.x, scale.y, gridSize);
      gridSize = getMin(scale.x, scale.y);
   }
   
   object->mGridElementSize = gridSize;
               
   // This is a hack so the console system doesn't go in and set our variable
   // again, after we've already set it (possibly with a different value...)
   return false;
}

//-----------------------------------------------------------------------------
// initPersistFields
//-----------------------------------------------------------------------------
void WaterBlock::initPersistFields()
{
   addGroup( "WaterBlock" );
      addProtectedField( "gridElementSize", TypeF32,  Offset( mGridElementSize, WaterBlock ), 
         &setGridSizeProperty, &defaultProtectedGetFn, "Spacing between vertices in the WaterBlock mesh" );
      addProtectedField( "gridSize", TypeF32,  Offset( mGridElementSize, WaterBlock ), 
         &setGridSizeProperty, &defaultProtectedGetFn, "Duplicate of gridElementSize for backwards compatility" );
   endGroup( "WaterBlock" );

   Parent::initPersistFields();
}     

bool WaterBlock::isUnderwater( const Point3F &pnt ) const
{
   // Transform point into object space so we can test if it is within
   // the WaterBlock's object box, include rotation/scale.

   Point3F objPnt = pnt;
   mWorldToObj.mulP( pnt, &objPnt );

   objPnt.z -= 0.1f;
         
   Box3F testBox = mObjBox;
   testBox.scale( mObjScale );     

   // We already tested if below the surface plane,
   // so clamping the z height of the box is not really necessary.
   testBox.maxExtents.z = testBox.getCenter().z;      

   if ( testBox.isContained( objPnt ) )
      return true;

   return false;
}

void WaterBlock::clearVertBuffers()
{
   for( U32 i=0; i<mVertBuffList.size(); i++ )
      delete mVertBuffList[i];
   mVertBuffList.clear();

   for( U32 i=0; i<mPrimBuffList.size(); i++ )
      delete mPrimBuffList[i];
   mPrimBuffList.clear();
}

void WaterBlock::inspectPostApply()
{
   Parent::inspectPostApply();

   VectorF scale = getScale();
   if( scale.x < mGridElementSize )
      scale.x = mGridElementSize;
   if( scale.y < mGridElementSize )
      scale.y = mGridElementSize;
   
   if( scale != getScale() )
      setScale( scale );

   setMaskBits( UpdateMask );
}

void WaterBlock::setTransform( const MatrixF &mat )
{
   // If our transform changes we need to recalculate the 
   // per vertex depth/shadow info.  Would be nice if this could
   // be done independently of generating the whole VBIB...   
   Parent::setTransform( mat );

   // We don't need to regen our vb anymore since we aren't calculating
   // per vert depth/shadow on the cpu anymore.
   //if ( oldMat != mObjToWorld )   
   //   mGenerateVB = true;   

   // Keep mWaterPlane up to date.
   mWaterFogData.plane.set( 0, 0, 1, -getPosition().z ); 
}

void WaterBlock::setScale( const Point3F &scale )
{
   Point3F oldScale = mObjScale;
   
   Parent::setScale( scale );
   
   if ( oldScale != mObjScale )
      mGenerateVB = true;
}

void WaterBlock::onStaticModified( const char* slotName, const char*newValue )
{
   Parent::onStaticModified( slotName, newValue );

   if ( dStricmp( slotName, "surfMaterial" ) == 0 )
      setMaskBits( MaterialMask );
   if ( dStricmp( slotName, "gridElementSize" ) == 0 )
   {
      mGenerateVB = true;
      setMaskBits( UpdateMask );
   }
}

bool WaterBlock::castRay( const Point3F &start, const Point3F &end, RayInfo *info )
{
   // Simply look for the hit on the water plane
   // and ignore any future issues with waves, etc.
   const Point3F norm(0,0,1);
   PlaneF plane( Point3F::Zero, norm );

   F32 hit = plane.intersect( start, end );
   if ( hit < 0.0f || hit > 1.0f )
      return false;
   
   info->t = hit;
   info->object = this;
   info->point = start + ( ( end - start ) * hit );
   info->normal = norm;
   info->material = mMatInstances[WaterMat];

   return mObjBox.isContained(info->point);
}

bool WaterBlock::buildPolyList( PolyListContext context, AbstractPolyList* polyList, const Box3F& box, const SphereF& )
{
   if(context == PLC_Navigation && box.isOverlapped(mWorldBox))
   {
      polyList->setObject( this );
      MatrixF mat(true);
      Point3F pos = getPosition();
      pos.x = pos.y = 0;
      mat.setPosition(pos);
      polyList->setTransform( &mat, Point3F(1, 1, 1) );

      Box3F ov = box.getOverlap(mWorldBox);
      Point3F
         p0(ov.minExtents.x, ov.maxExtents.y, 0),
         p1(ov.maxExtents.x, ov.maxExtents.y, 0),
         p2(ov.maxExtents.x, ov.minExtents.y, 0),
         p3(ov.minExtents.x, ov.minExtents.y, 0);

      // Add vertices to poly list.
      U32 v0 = polyList->addPoint(p0);
      polyList->addPoint(p1);
      polyList->addPoint(p2);
      polyList->addPoint(p3);

      // Add plane between first three vertices.
      polyList->begin(0, 0);
      polyList->vertex(v0);
      polyList->vertex(v0+1);
      polyList->vertex(v0+2);
      polyList->plane(v0, v0+1, v0+2);
      polyList->end();

      // Add plane between last three vertices.
      polyList->begin(0, 1);
      polyList->vertex(v0+2);
      polyList->vertex(v0+3);
      polyList->vertex(v0);
      polyList->plane(v0+2, v0+3, v0);
      polyList->end();

      return true;
   }

   return false;
}

F32 WaterBlock::getWaterCoverage( const Box3F &testBox ) const
{
   Box3F wbox = getWorldBox();
   wbox.maxExtents.z = wbox.getCenter().z;
   
   F32 coverage = 0.0f;

   if ( wbox.isOverlapped(testBox) ) 
   {
      if (wbox.maxExtents.z < testBox.maxExtents.z)
         coverage = (wbox.maxExtents.z - testBox.minExtents.z) / (testBox.maxExtents.z - testBox.minExtents.z);
      else
         coverage = 1.0f;
   }

   return coverage;
}

F32 WaterBlock::getSurfaceHeight( const Point2F &pos ) const
{
   if ( !mWorldBox.isContained( pos ) )
      return -1.0f;

   return getPosition().z;   
}

void WaterBlock::_getWaterPlane( const Point3F &camPos, PlaneF &outPlane, Point3F &outPos )
{
   outPos = getPosition();
   if ( mReflectNormalUp )
      outPlane.set( outPos, Point3F(0,0,1) );
   else
   {
      Point3F normal;
      getRenderTransform().getColumn( 2, &normal );
      outPlane.set( outPos, normal );
   }
}

F32 WaterBlock::distanceTo( const Point3F& point ) const
{
   Box3F waterBox = getWorldBox();
   waterBox.maxExtents.z = getPosition().z;
   
   return waterBox.getDistanceToPoint( point );
}
