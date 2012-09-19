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
#include "environment/waterPlane.h"

#include "core/util/safeDelete.h"
#include "scene/sceneRenderState.h"
#include "scene/sceneManager.h"
#include "lighting/lightInfo.h"
#include "core/stream/bitStream.h"
#include "math/mathIO.h"
#include "math/mathUtils.h"
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
#include "T3D/gameFunctions.h"
#include "postFx/postEffect.h"
#include "math/util/matrixSet.h"

extern ColorI gCanvasClearColor;

#define BLEND_TEX_SIZE 256
#define V_SHADER_PARAM_OFFSET 50

IMPLEMENT_CO_NETOBJECT_V1(WaterPlane);

ConsoleDocClass( WaterPlane,
   "@brief Represents a large body of water stretching to the horizon in all directions.\n\n"

   "WaterPlane's position is defined only height, the z element of position, "
   "it is infinite in xy and depth. %WaterPlane is designed to represent the "
   "ocean on an island scene and viewed from ground level; other uses may not "
   "be appropriate and a WaterBlock may be used.\n\n"

   "@see WaterObject for inherited functionality.\n\n"

   "Limitations:\n\n"
   
   "Because %WaterPlane cannot be projected exactly to the far-clip distance, "
   "other objects nearing this distance can have noticible artifacts as they "
   "clip through first the %WaterPlane and then the far plane.\n\n"
   
   "To avoid this large objects should be positioned such that they will not line up with "
   "the far-clip from vantage points the player is expected to be. In particular, "
   "your TerrainBlock should be completely contained by the far-clip distance.\n\n"
   
   "Viewing %WaterPlane from a high altitude with a tight far-clip distance "
   "will accentuate this limitation. %WaterPlane is primarily designed to "
   "be viewed from ground level.\n\n"
      
   "@ingroup Water"
);

WaterPlane::WaterPlane()
{
   mGridElementSize = 1.0f;
   mGridSize = 101;
   mGridSizeMinusOne = mGridSize - 1;

   mNetFlags.set(Ghostable | ScopeAlways);

   mVertCount = 0;
   mIndxCount = 0;
   mPrimCount = 0;   
}

WaterPlane::~WaterPlane()
{
}

bool WaterPlane::onAdd()
{
   if ( !Parent::onAdd() )
      return false;
   
   setGlobalBounds();
   resetWorldBox();
   addToScene();

   mWaterFogData.plane.set( 0, 0, 1, -getPosition().z );

   return true;
}

void WaterPlane::onRemove()
{   
   removeFromScene();

   Parent::onRemove();
}

void WaterPlane::initPersistFields()
{
   addGroup( "WaterPlane" );     

      addProtectedField( "gridSize", TypeS32, Offset( mGridSize, WaterPlane ), &protectedSetGridSize, &defaultProtectedGetFn,
		  "Spacing between vertices in the WaterBlock mesh" );

      addProtectedField( "gridElementSize", TypeF32, Offset( mGridElementSize, WaterPlane ), &protectedSetGridElementSize, &defaultProtectedGetFn,
		  "Duplicate of gridElementSize for backwards compatility");

   endGroup( "WaterPlane" );

   Parent::initPersistFields();

   removeField( "rotation" );
   removeField( "scale" );
}

U32 WaterPlane::packUpdate(NetConnection* con, U32 mask, BitStream* stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   stream->write( mGridSize );
   stream->write( mGridElementSize );   

   if ( stream->writeFlag( mask & UpdateMask ) )
   {
      stream->write( getPosition().z );        
   }

   return retMask;
}

void WaterPlane::unpackUpdate(NetConnection* con, BitStream* stream)
{
   Parent::unpackUpdate(con, stream);

   U32 inGridSize;
   stream->read( &inGridSize );
   setGridSize( inGridSize );

   F32 inGridElementSize;
   stream->read( &inGridElementSize );
   setGridElementSize( inGridElementSize );

   if( stream->readFlag() ) // UpdateMask
   {
      float posZ;
      stream->read( &posZ );
      Point3F newPos = getPosition();
      newPos.z = posZ;
      setPosition( newPos );
   }  
}

void WaterPlane::setupVBIB( SceneRenderState *state )
{
   const Frustum &frustum = state->getFrustum();
   
   // Water base-color, assigned as color for all verts.
   const GFXVertexColor vertCol(mWaterFogData.color);

   // World-Up vector, assigned as normal for all verts.
   const Point3F worldUp( 0.0f, 0.0f, 1.0f );

   // World-unit size of a grid cell.
   const F32 squareSize = mGridElementSize;

   // Column/Row count.
   // So we don't neet to access class-specific member variables
   // in the code below.
   const U32 gridSize = mGridSize;

   // Number of verts in one column / row
   const U32 gridStride = gridSize + 1;

   // Grid is filled in this order...
   
   // Ex. Grid with gridSize of 2.
   //
   // Letters are cells.
   // Numbers are verts, enumerated in their order within the vert buffer.       
   //
   // 6  7  8   
   // (c) (d)
   // 3  4  5
   // (a) (b)
   // 0  1  2
   //   
   // Note...
   //   Camera would be positioned at vert 4 ( in this particular grid not a constant ).
   //   Positive Y points UP the diagram ( verts 0, 3, 6 ).
   //   Positive X points RIGHT across the diagram ( verts 0, 1, 2 ).

   // Length of a grid row/column divided by two.
   F32 gridSideHalfLen = squareSize * gridSize * 0.5f;

   // Position of the first vertex in the grid.
   // Relative to the camera this is the "Back Left" corner vert.
   const Point3F cornerPosition( -gridSideHalfLen, -gridSideHalfLen, 0.0f );   

   // Number of verts in the grid centered on the camera.
   const U32 gridVertCount = gridStride * gridStride;

   // Number of verts surrounding the grid, projected by the frustum.
   const U32 borderVertCount = gridSize * 4;

   // Number of verts in the front-most row which are raised to the horizon.
   const U32 horizonVertCount = gridStride;

   // Total number of verts. Calculation explained above.
   mVertCount = gridVertCount + borderVertCount + horizonVertCount;

   
   // Fill the vertex buffer...

   mVertBuff.set( GFX, mVertCount, GFXBufferTypeStatic );

   GFXWaterVertex *vertPtr = mVertBuff.lock();

   // Fill verts in the camera centered grid...

   // Temorary storage for calculation of vert position.
   F32 xVal, yVal;

   for ( U32 i = 0; i < gridStride; i++ )
   {
      yVal = cornerPosition.y + (F32)( i * squareSize );

      for ( U32 j = 0; j < gridStride; j++ )
      {
         xVal = cornerPosition.x + (F32)( j * squareSize );

         vertPtr->point.set( xVal, yVal, 0.0f );
         vertPtr->color = vertCol;
         vertPtr->normal = worldUp;
         vertPtr->undulateData.set( xVal, yVal );
         vertPtr->horizonFactor.set( 0, 0, 0, 0 );
         vertPtr++;
      }
   }

   // Fill in 'border' verts, surrounding the grid, projected by the frustum.

   // Ex. Grid with gridSize of 2.
   //
   // Letters in parenthesis are cells.
   // x's are grid-verts ( we have already filled ).
   // Numbers are border verts, enumerated in their order within the vert buffer.       
   // 
   // Lines connecting verts explained in the code below.
   //
   //      Front
   //
   //  L   0------1      2   R
   //  e       x  x  x   |   i
   //  f       (c) (d)   |   g
   //  t   7   x  x  x   3   h
   //      |   (a) (b)       t
   //      |   x  x  x       
   //      6      5------4
   //
   //      Back
   //   
   // As in previous diagram...
   //   Camera would be positioned at vert 4 ( in this particular grid not a constant ).
   //   Positive Y points UP the diagram ( verts 6, 7, 0 ).
   //   Positive X points RIGHT across the diagram ( verts 0, 1, 2 ).


   // Iterator i is looping through the 4 'sides' of the grid.
   // Inner loop ( using iterator j ) will fill in a number of verts   
   // where that count is 'gridSize'.
   //    
   // 
   // Ex. Given the grid with gridSize of 2 diagramed above,
   // Outer loop iterates through: Front, Right, Back, Left
   // Inner loop fills 2 verts per iteration of the outer loop: { 0, 1 }, { 2, 3 }, { 4, 5 }, { 6, 7 }

   // Grid-space vectors indexed by 'side'.
   // Each vector describes the direction we iterate when
   // filling in verts ( mathematically the tangent ).
   const Point2F sBorderTangentVec [4] = {
      Point2F(  1,  0 ), // Front ( 0 )
      Point2F(  0, -1 ), // Right ( 1 )
      Point2F( -1,  0 ), // Back  ( 2 )
      Point2F(  0,  1 )  // Left  ( 3 )
   };
   

   // Normalized positions indexed by 'side'
   // Defines the 'start' position of each side, eg. the position of the first vert.
   // See Diagram below.
   const Point2F sBorderStartPos [4] = {
      Point2F( -1,  1 ), // Front ( 0 )
      Point2F(  1,  1 ), // Right ( 1 )
      Point2F(  1, -1 ), // Back  ( 2 )
      Point2F( -1, -1 )  // Left  ( 3 )
   };

   // Diagram of Start vert position per Side.
   //
   // Labeling convention for verts is 'As' where A is the first letter of 
   // that side's descriptive name and lower-case s indicates 'start'. 
   // 
   // 
   //
   //          Front
   //          (-1,1)
   //          Fs------o-----Rs(1,1)R
   //          |             |      i
   //          |             |      g
   //          o     (0,0)   o      h
   //          |             |      t
   //          |             |     
   //  L(-1,-1)Ls------o-----Bs
   //  e                     (1,-1)   
   //  f                     Back
   //  t

   // Calculate the world-space dimension of the border-vert-ring...

   // Diagram shows overall layout of the WaterPlane with a gridSize of 1,
   // with all 'quads' enumerated.
   // center-grid ( 1 ), border ( 2, 3, 4, 5 ), and horizon ( 6 ).
   //
   //
   //   x------------x
   //    \      6     \    <- horizon quad is really 'above' the front border
   //      x ---------  x     not in front of it
   //      | \    2   / |
   //      |   x---- x  |
   //      |   |     |  |
   //      |  5|  1  |3 |
   //      |   x --- x  |
   //      | /    4    \|
   //      x------------x


   // WaterPlane renders relative to the camera rotation around z and xy position.
   //
   // That is, it rotates around the z-up axis with the camera such that the
   // camera is always facing towards the front border unless looking straight
   // down or up.
   //
   // Also note that the horizon verts are pulled straight up from the front
   // border verts.
   //
   // Therefore...
   //
   // The front border must be as close to the farclip plane as possible 
   // so distant objects clip through the horizon and  farplane at the same time.
   // 
   // The left and right borders must be pulled outward a distance such
   // that water extends horizontally across the entire viewable area while
   // looking straight forward +y or straight down -z.
   //

   // 
   const F32 farDistScale = 0.99f;

   // 
   F32 farDist = frustum.getFarDist() * farDistScale;
   
   //
   F32 farWidth = (F32)state->getViewport().extent.x * farDist / state->getWorldToScreenScale().x;

   Point2F borderExtents( farWidth * 2.0f, farDist * 2.0f );
   Point2F borderHalfExtents( farWidth, farDist );   

   Point2F borderDir;
   Point2F borderStart;

   for ( U32 i = 0; i < 4; i++ )
   {
      borderDir = sBorderTangentVec[i];
      borderStart = sBorderStartPos[i];

      for ( U32 j = 0; j < gridSize; j++ )
      {
         F32 frac = (F32)j / (F32)gridSize;

         Point2F pos( borderStart * borderHalfExtents );
         pos += borderDir * borderExtents * frac;

         vertPtr->point.set( pos.x, pos.y, 0.0f );
         vertPtr->undulateData.set( pos.x, pos.y );
         vertPtr->horizonFactor.set( 0, 0, 0, 0 );
         vertPtr->color = vertCol;
         vertPtr->normal = worldUp;         
         vertPtr++;
      }
   }

   // Fill in row of horizion verts.
   // Verts are positioned identical to the front border, but will be
   // manipulated in z within the shader.
   //
   // Z position of 50.0f is unimportant unless you want to disable
   // shader manipulation and render in wireframe for debugging.

   for ( U32 i = 0; i < gridStride; i++ )
   {
      F32 frac = (F32)i / (F32)gridSize;
      
      Point2F pos( sBorderStartPos[0] * borderHalfExtents );
      pos += sBorderTangentVec[0] * borderExtents * frac;

      vertPtr->point.set( pos.x, pos.y, 50.0f );
      vertPtr->undulateData.set( pos.x, pos.y );
      vertPtr->horizonFactor.set( 1, 0, 0, 0 );
      vertPtr->color = vertCol;
      vertPtr->normal = worldUp;
      vertPtr++;
   }

   mVertBuff.unlock();


   // Fill in the PrimitiveBuffer...

   // 2 triangles per cell/quad   
   const U32 gridTriCount = gridSize * gridSize * 2;

   // 4 sides, mGridSize quads per side, 2 triangles per quad
   const U32 borderTriCount = 4 * gridSize * 2;

   // 1 quad per gridSize, 2 triangles per quad
   // i.e. an extra row of 'cells' leading the front side of the grid
   const U32 horizonTriCount = gridSize * 2;
   
   mPrimCount = gridTriCount + borderTriCount + horizonTriCount;

   // 3 indices per triangle.
   mIndxCount = mPrimCount * 3; 

   mPrimBuff.set( GFX, mIndxCount, mPrimCount, GFXBufferTypeStatic );
   U16 *idxPtr;
   mPrimBuff.lock(&idxPtr);        

   // Temporaries to hold indices for the corner points of a quad.
   U32 p00, p01, p11, p10;
   U32 offset = 0;

   // Given a single cell of the grid diagramed below,
   // quad indice variables are in this orientation.
   //
   // p01 --- p11
   //  |       |
   //  |       |
   // p00 --- p10
   //
   //   Positive Y points UP the diagram ( p00, p01 ).
   //   Positive X points RIGHT across the diagram ( p00, p10 )
   //

   // i iterates bottom to top "column-wise"
   for ( U32 i = 0; i < mGridSize; i++ )
   {
      // j iterates left to right "row-wise"
      for ( U32 j = 0; j < mGridSize; j++ )
      {
         // where (j,i) is a particular cell.
         p00 = offset;
         p10 = offset + 1;
         p01 = offset + gridStride;
         p11 = offset + 1 + gridStride;

         // Top Left Triangle

         *idxPtr = p00;
         idxPtr++;
         *idxPtr = p01;
         idxPtr++;
         *idxPtr = p11;
         idxPtr++;

         // Bottom Right Triangle

         *idxPtr = p00;
         idxPtr++;
         *idxPtr = p11;
         idxPtr++;
         *idxPtr = p10;
         idxPtr++;

         offset += 1;
      }

      offset += 1;
   }

   // Fill border indices...

   // Given a grid size of 1, 
   // the grid / border verts are in the vertex buffer in this order.
   //
   //
   // 4           5  
   //    2 --- 3
   //    |     |
   //    |     |
   //    0 --- 1
   // 7           6
   //
   //   Positive Y points UP the diagram ( p00, p01 ).
   //   Positive X points RIGHT across the diagram ( p00, p10 )
   //
   //  Note we duplicate the first border vert ( 4 ) since it is also the last
   //  and this makes our loop easier.

   const U32 sBorderStartVert [4] = {
      gridStride * gridSize,              // Index to the Top-Left grid vert.
      gridStride * gridSize + gridSize,   // Index to the Top-Right grid vert.
      gridSize,                           // Index to the Bottom-Right grid vert.
      0,                                  // Index to the Bottom-Left grid vert.
   };

   const S32 sBorderStepSize [4] = {
      // Step size to the next grid vert along the specified side....
      1,             // Top
      -(S32)gridStride,   // Right
      -1,             // Bottom
      gridStride,    // Left
   };

   const U32 firstBorderVert = gridStride * gridSize + gridStride;
   const U32 lastBorderVert = firstBorderVert + ( borderVertCount - 1 );
   U32 startBorderVert = firstBorderVert;
   U32 startGridVert;
   U32 curStepSize;
   
   
   for ( U32 i = 0; i < 4; i++ )
   {
      startGridVert = sBorderStartVert[i];
      curStepSize = sBorderStepSize[i];
      
      for ( U32 j = 0; j < gridSize; j++ )
      {
         // Each border cell is 1 quad, 2 triangles.

         p00 = startGridVert;
         p10 = startGridVert + curStepSize;
         p01 = startBorderVert;
         p11 = startBorderVert + 1;
         
         if ( p11 > lastBorderVert )
            p11 = firstBorderVert;

         // Top Left Triangle

         *idxPtr = p00;
         idxPtr++;
         *idxPtr = p01;
         idxPtr++;
         *idxPtr = p11;
         idxPtr++;

         // Bottom Right Triangle

         *idxPtr = p00;
         idxPtr++;
         *idxPtr = p11;
         idxPtr++;
         *idxPtr = p10;
         idxPtr++;

         startBorderVert++;
         startGridVert += curStepSize;
      }
   }

   // Fill in 'horizon' triangles.

   U32 curHorizonVert = lastBorderVert + 1;
   U32 curBorderVert = firstBorderVert;

   for ( U32 i = 0; i < gridSize; i++ )
   {
      p00 = curBorderVert;
      p10 = curBorderVert + 1;
      p01 = curHorizonVert;
      p11 = curHorizonVert + 1;
      
      // Top Left Triangle

      *idxPtr = p00;
      idxPtr++;
      *idxPtr = p01;
      idxPtr++;
      *idxPtr = p11;
      idxPtr++;

      // Bottom Right Triangle

      *idxPtr = p00;
      idxPtr++;
      *idxPtr = p11;
      idxPtr++;
      *idxPtr = p10;
      idxPtr++;

      curBorderVert++;
      curHorizonVert++;
   }

   mPrimBuff.unlock();
}

SceneData WaterPlane::setupSceneGraphInfo( SceneRenderState *state )
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

void WaterPlane::setShaderParams( SceneRenderState *state, BaseMatInstance* mat, const WaterMatParams& paramHandles)
{
   // Set variables that will be assigned to shader consts within WaterCommon
   // before calling Parent::setShaderParams

   mUndulateMaxDist = mGridElementSize * mGridSizeMinusOne * 0.5f;

   Parent::setShaderParams( state, mat, paramHandles );   

   // Now set the rest of the shader consts that are either unique to this
   // class or that WaterObject leaves to us to handle...    

   MaterialParameters* matParams = mat->getMaterialParameters();

   // set vertex shader constants
   //-----------------------------------   
   matParams->setSafe(paramHandles.mGridElementSizeSC, (F32)mGridElementSize);
   //matParams->setSafe( paramHandles.mReflectTexSizeSC, mReflectTexSize );
   if ( paramHandles.mModelMatSC->isValid() )
      matParams->set(paramHandles.mModelMatSC, getRenderTransform(), GFXSCT_Float4x4);

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

   //Point4F reflectParams( getRenderPosition().z, mReflectMinDist, mReflectMaxDist, reflect );
   Point4F reflectParams( getRenderPosition().z, 0.0f, 1000.0f, !reflect );
   
   // TODO: This is a hack... why is this broken... check after
   // we merge advanced lighting with trunk!
   //
   reflectParams.z = 0.0f;
   matParams->setSafe( paramHandles.mReflectParamsSC, reflectParams );

   VectorF reflectNorm( 0, 0, 1 );
   matParams->setSafe(paramHandles.mReflectNormalSC, reflectNorm ); 
}

void WaterPlane::prepRenderImage( SceneRenderState *state )
{
   PROFILE_SCOPE(WaterPlane_prepRenderImage);

   if( !state->isDiffusePass() )
      return;

   mBasicLighting = dStricmp( LIGHTMGR->getId(), "BLM" ) == 0;
   mUnderwater = isUnderwater( state->getCameraPosition() );

   mMatrixSet->setSceneView(GFX->getWorldMatrix());
   
   const Frustum &frustum = state->getFrustum();

   if ( mPrimBuff.isNull() || 
        mGenerateVB ||         
        frustum != mFrustum )
   {      
      mFrustum = frustum;
      setupVBIB( state );
      mGenerateVB = false;

      MatrixF proj( true );
      MathUtils::getZBiasProjectionMatrix( 0.0001f, mFrustum, &proj );
      mMatrixSet->setSceneProjection(proj);
   }

   _getWaterPlane( state->getCameraPosition(), mWaterPlane, mWaterPos );
   mWaterFogData.plane = mWaterPlane;
   mPlaneReflector.refplane = mWaterPlane;
   updateUnderwaterEffect( state );

   ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
   ri->renderDelegate.bind( this, &WaterObject::renderObject );
   ri->type = RenderPassManager::RIT_Water;
   state->getRenderPass()->addInst( ri );

   //mRenderUpdateCount++;
}

void WaterPlane::innerRender( SceneRenderState *state )
{
   GFXDEBUGEVENT_SCOPE( WaterPlane_innerRender, ColorI( 255, 0, 0 ) );

   const Point3F &camPosition = state->getCameraPosition();

   Point3F rvec, fvec, uvec, pos;

   const MatrixF &objMat = getTransform(); //getRenderTransform();
   const MatrixF &camMat = state->getCameraTransform();

   MatrixF renderMat( true );

   camMat.getColumn( 1, &fvec );
   uvec.set( 0, 0, 1 );
   rvec = mCross( fvec, uvec );
   rvec.normalize();   
   fvec = mCross( uvec, rvec );
   pos = camPosition;
   pos.z = objMat.getPosition().z;      

   renderMat.setColumn( 0, rvec );
   renderMat.setColumn( 1, fvec );
   renderMat.setColumn( 2, uvec );
   renderMat.setColumn( 3, pos );

   setRenderTransform( renderMat );

   // Setup SceneData
   SceneData sgData = setupSceneGraphInfo( state );   

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
      mMatrixSet->restoreSceneViewProjection();
      mMatrixSet->setWorld(getRenderTransform());

      setShaderParams( state, mat, matParams );     

      while( mat->setupPass( state, sgData ) )
      {    
         mat->setSceneInfo(state, sgData);
         mat->setTransforms(*mMatrixSet, state);
         setCustomTextures( matIdx, mat->getCurPass(), matParams );

         // set vert/prim buffer
         GFX->setVertexBuffer( mVertBuff );
         GFX->setPrimitiveBuffer( mPrimBuff );
         GFX->drawIndexedPrimitive( GFXTriangleList, 0, 0, mVertCount, 0, mPrimCount );
      }
   }
}

bool WaterPlane::isUnderwater( const Point3F &pnt ) const
{
   F32 height = getPosition().z;

   F32 diff = pnt.z - height;

   return ( diff < 0.1 );
}

F32 WaterPlane::distanceTo( const Point3F& point ) const
{
   if( isUnderwater( point ) )
      return 0.f;
   else
      return ( point.z - getPosition().z );
}

void WaterPlane::inspectPostApply()
{
   Parent::inspectPostApply();

   setMaskBits( UpdateMask );
}

void WaterPlane::setTransform( const MatrixF &mat )
{
   // We only accept the z value from the new transform.

   MatrixF newMat( true );
   
   Point3F newPos = getPosition();
   newPos.z = mat.getPosition().z;  
   newMat.setPosition( newPos );

   Parent::setTransform( newMat );

   // Parent::setTransforms ends up setting our worldBox to something other than
   // global, so we have to set it back... but we can't actually call setGlobalBounds
   // again because it does extra work adding and removing us from the container.

   mGlobalBounds = true;
   mObjBox.minExtents.set(-1e10, -1e10, -1e10);
   mObjBox.maxExtents.set( 1e10,  1e10,  1e10);

   // Keep mWaterPlane up to date.
   mWaterFogData.plane.set( 0, 0, 1, -getPosition().z );   
}

void WaterPlane::onStaticModified( const char* slotName, const char*newValue )
{
   Parent::onStaticModified( slotName, newValue );

   if ( dStricmp( slotName, "surfMaterial" ) == 0 )
      setMaskBits( MaterialMask );
}

bool WaterPlane::castRay(const Point3F& start, const Point3F& end, RayInfo* info )
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
   info->material = mMatInstances[ WaterMat ];

   return true;
}

F32 WaterPlane::getWaterCoverage( const Box3F &testBox ) const
{
   F32 posZ = getPosition().z;
   
   F32 coverage = 0.0f;

   if ( posZ > testBox.minExtents.z ) 
   {
      if ( posZ < testBox.maxExtents.z )
         coverage = (posZ - testBox.minExtents.z) / (testBox.maxExtents.z - testBox.minExtents.z);
      else
         coverage = 1.0f;
   }

   return coverage;
}

F32 WaterPlane::getSurfaceHeight( const Point2F &pos ) const
{
   return getPosition().z;   
}

void WaterPlane::onReflectionInfoChanged()
{
   /*
   if ( isClientObject() && GFX->getPixelShaderVersion() >= 1.4 )
   {
      if ( mFullReflect )
         REFLECTMGR->registerObject( this, ReflectDelegate( this, &WaterPlane::updateReflection ), mReflectPriority, mReflectMaxRateMs, mReflectMaxDist );
      else
      {
         REFLECTMGR->unregisterObject( this );
         mReflectTex = NULL;
      }
   }
   */
}

void WaterPlane::setGridSize( U32 inSize )
{
   if ( inSize == mGridSize )
      return;

   // GridSize must be an odd number.
   //if ( inSize % 2 == 0 )
   //   inSize++;

   // GridSize must be at least 1
   inSize = getMax( inSize, (U32)1 );

   mGridSize = inSize;
   mGridSizeMinusOne = mGridSize - 1;
   mGenerateVB = true;
   setMaskBits( UpdateMask );
}

void WaterPlane::setGridElementSize( F32 inSize )
{
   if ( inSize == mGridElementSize )
      return;

   // GridElementSize must be greater than 0
   inSize = getMax( inSize, 0.0001f );

   mGridElementSize = inSize;
   mGenerateVB = true;
   setMaskBits( UpdateMask );
}

bool WaterPlane::protectedSetGridSize( void *obj, const char *index, const char *data )
{
   WaterPlane *object = static_cast<WaterPlane*>(obj);
   S32 size = dAtoi( data );

   object->setGridSize( size );

   // We already set the field.
   return false;
}

bool WaterPlane::protectedSetGridElementSize( void *obj, const char *index, const char *data )
{
   WaterPlane *object = static_cast<WaterPlane*>(obj);
   F32 size = dAtof( data );

   object->setGridElementSize( size );

   // We already set the field.
   return false;
}

void WaterPlane::_getWaterPlane( const Point3F &camPos, PlaneF &outPlane, Point3F &outPos )
{
   outPos = getPosition();   
   outPlane.set( outPos, Point3F(0,0,1) );   
}