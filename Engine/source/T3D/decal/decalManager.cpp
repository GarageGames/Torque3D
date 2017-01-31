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
#include "T3D/decal/decalManager.h"

#include "scene/sceneManager.h"
#include "scene/sceneRenderState.h"
#include "ts/tsShapeInstance.h"
#include "console/console.h"
#include "console/dynamicTypes.h"
#include "gfx/primBuilder.h"
#include "console/consoleTypes.h"
#include "platform/profiler.h"
#include "gfx/gfxTransformSaver.h"
#include "lighting/lightManager.h"
#include "lighting/lightInfo.h"
#include "gfx/gfxDrawUtil.h"
#include "gfx/sim/gfxStateBlockData.h"
#include "materials/shaderData.h"
#include "materials/matInstance.h"
#include "renderInstance/renderPassManager.h"
#include "core/resourceManager.h"
#include "core/stream/fileStream.h"
#include "gfx/gfxDebugEvent.h"
#include "math/util/quadTransforms.h"
#include "math/mathUtils.h"
#include "core/volume.h"
#include "core/module.h"
#include "T3D/decal/decalData.h"
#include "console/engineAPI.h"


extern bool gEditingMission;


MODULE_BEGIN( DecalManager )

   MODULE_INIT_AFTER( Scene )
   MODULE_SHUTDOWN_BEFORE( Scene )
   
   MODULE_INIT
   {
      gDecalManager = new DecalManager;
      gClientSceneGraph->addObjectToScene( gDecalManager );
   }
   
   MODULE_SHUTDOWN
   {
      gClientSceneGraph->removeObjectFromScene( gDecalManager );
      SAFE_DELETE( gDecalManager );
   }

MODULE_END;


/// A bias applied to the nearPlane for Decal and DecalRoad rendering.
/// Is set by by LevelInfo.
F32 gDecalBias = 0.0015f;

bool      DecalManager::smDecalsOn = true;
bool      DecalManager::smDebugRender = false;
F32       DecalManager::smDecalLifeTimeScale = 1.0f;
bool      DecalManager::smPoolBuffers = true;
const U32 DecalManager::smMaxVerts = 6000;
const U32 DecalManager::smMaxIndices = 10000;

DecalManager *gDecalManager = NULL;

IMPLEMENT_CONOBJECT(DecalManager);

ConsoleDoc(
   "@defgroup Decals\n"
   "@brief Decals are non-SimObject derived objects that are stored and loaded "
   "separately from the normal mission file.\n\n"

   "The DecalManager handles all aspects of decal management including loading, "
   "creation, saving, and automatically deleting decals that have exceeded their "
   "lifeSpan.\n\n"

   "The static decals associated with a mission are normally loaded immediately "
   "after the mission itself has loaded as shown below.\n"

   "@tsexample\n"
   "// Load the static mission decals.\n"
   "decalManagerLoad( %missionName @ \".decals\" );\n"
   "@endtsexample\n"

   "@ingroup FX\n"
);

ConsoleDocClass( DecalManager,
   "@brief The object that manages all of the decals in the active mission.\n\n"
   "@see Decals\n"
   "@ingroup Decals\n"
   "@ingroup FX\n"
);

namespace {

S32 QSORT_CALLBACK cmpDecalInstance(const void* p1, const void* p2)
{
   const DecalInstance** pd1 = (const DecalInstance**)p1;
   const DecalInstance** pd2 = (const DecalInstance**)p2;

   return int(((char *)(*pd1)->mDataBlock) - ((char *)(*pd2)->mDataBlock));
}

S32 QSORT_CALLBACK cmpPointsXY( const void *p1, const void *p2 )
{
   const Point3F *pnt1 = (const Point3F*)p1;
   const Point3F *pnt2 = (const Point3F*)p2;

   if ( pnt1->x < pnt2->x )
      return -1;
   else if ( pnt1->x > pnt2->x )
      return 1;
   else if ( pnt1->y < pnt2->y )
      return -1;
   else if ( pnt1->y > pnt2->y )
      return 1;
   else
      return 0;
}

S32 QSORT_CALLBACK cmpQuadPointTheta( const void *p1, const void *p2 )
{
   const Point4F *pnt1 = (const Point4F*)p1;
   const Point4F *pnt2 = (const Point4F*)p2;

   if ( mFabs( pnt1->w ) > mFabs( pnt2->w ) )
      return 1;
   else if ( mFabs( pnt1->w ) < mFabs( pnt2->w ) )
      return -1;
   else
      return 0;
}

static Point3F gSortPoint;

S32 QSORT_CALLBACK cmpDecalDistance( const void *p1, const void *p2 )
{
   const DecalInstance** pd1 = (const DecalInstance**)p1;
   const DecalInstance** pd2 = (const DecalInstance**)p2;

   F32 dist1 = ( (*pd1)->mPosition - gSortPoint ).lenSquared();
   F32 dist2 = ( (*pd2)->mPosition - gSortPoint ).lenSquared();

   return mSign( dist1 - dist2 );
}

S32 QSORT_CALLBACK cmpDecalRenderOrder( const void *p1, const void *p2 )
{   
   const DecalInstance** pd1 = (const DecalInstance**)p1;
   const DecalInstance** pd2 = (const DecalInstance**)p2;

   if ( ( (*pd2)->mFlags & SaveDecal ) && !( (*pd1)->mFlags & SaveDecal ) )
      return -1;
   else if ( !( (*pd2)->mFlags & SaveDecal ) && ( (*pd1)->mFlags & SaveDecal ) )
      return 1;
   else
   {
      S32 priority = (*pd1)->getRenderPriority() - (*pd2)->getRenderPriority();

      if ( priority != 0 )
         return priority;

      if ( (*pd2)->mFlags & SaveDecal )
      {
         S32 id = ( (*pd1)->mDataBlock->getMaterial()->getId() - (*pd2)->mDataBlock->getMaterial()->getId() );      
         if ( id != 0 )
            return id;

         return (*pd1)->mCreateTime - (*pd2)->mCreateTime;
      }
      else
         return (*pd1)->mCreateTime - (*pd2)->mCreateTime;
   }
}

} // namespace {}

// These numbers should be tweaked to get as many dynamically placed decals
// as possible to allocate buffer arrays with the FreeListChunker.
enum
{
   SIZE_CLASS_0 = 256,
   SIZE_CLASS_1 = 512,
   SIZE_CLASS_2 = 1024,
   
   NUM_SIZE_CLASSES = 3
};

//-------------------------------------------------------------------------
// DecalManager
//-------------------------------------------------------------------------
DecalManager::DecalManager()
{
   #ifdef DECALMANAGER_DEBUG
   VECTOR_SET_ASSOCIATION( mDebugPlanes );
   #endif

   setGlobalBounds();

   mDataFileName = NULL;

   mTypeMask |= EnvironmentObjectType;

   mDirty = false;

   mChunkers[0] = new FreeListChunkerUntyped( SIZE_CLASS_0 * sizeof( U8 ) );
   mChunkers[1] = new FreeListChunkerUntyped( SIZE_CLASS_1 * sizeof( U8 ) );
   mChunkers[2] = new FreeListChunkerUntyped( SIZE_CLASS_2 * sizeof( U8 ) );

   GFXDevice::getDeviceEventSignal().notify(this, &DecalManager::_handleGFXEvent);
}

DecalManager::~DecalManager()
{
   GFXDevice::getDeviceEventSignal().remove(this, &DecalManager::_handleGFXEvent);

   clearData();

   for( U32 i = 0; i < NUM_SIZE_CLASSES; ++ i )
      delete mChunkers[ i ];
}

void DecalManager::consoleInit()
{
   Con::addVariable( "$pref::Decals::enabled", TypeBool, &smDecalsOn,
      "Controls whether decals are rendered.\n"
      "@ingroup Decals" );

   Con::addVariable( "$pref::Decals::lifeTimeScale", TypeF32, &smDecalLifeTimeScale,
      "@brief Lifetime that decals will last after being created in the world.\n"
      "Deprecated. Use DecalData::lifeSpan instead.\n"
      "@ingroup Decals" );

   Con::addVariable( "$Decals::poolBuffers", TypeBool, &smPoolBuffers,
      "If true, will merge all PrimitiveBuffers and VertexBuffers into a pair "
      "of pools before clearing them at the end of a frame.\n"
      "If false, will just clear them at the end of a frame.\n"
      "@ingroup Decals" );

   Con::addVariable( "$Decals::debugRender", TypeBool, &smDebugRender,
      "If true, the decal spheres will be visualized when in the editor.\n\n"
      "@ingroup Decals" );

   Con::addVariable( "$Decals::sphereDistanceTolerance", TypeF32, &DecalSphere::smDistanceTolerance,
      "The distance at which the decal system will start breaking up decal "
      "spheres when adding new decals.\n\n"
      "@ingroup Decals" );

   Con::addVariable( "$Decals::sphereRadiusTolerance", TypeF32, &DecalSphere::smRadiusTolerance,
      "The radius beyond which the decal system will start breaking up decal "
      "spheres when adding new decals.\n\n"
      "@ingroup Decals" );
}

bool DecalManager::_handleGFXEvent(GFXDevice::GFXDeviceEventType event)
{
   switch(event)
   {
   case GFXDevice::deEndOfFrame:

      // Return PrimitiveBuffers and VertexBuffers used this frame to the pool.

      if ( smPoolBuffers )
      {
         mPBPool.merge( mPBs );
         mPBs.clear();

         mVBPool.merge( mVBs );
         mVBs.clear();
      }
      else
      {
         _freePools();
      }

      break;
      
   default: ;
   }

   return true;
}

bool DecalManager::clipDecal( DecalInstance *decal, Vector<Point3F> *edgeVerts, const Point2F *clipDepth )
{
   PROFILE_SCOPE( DecalManager_clipDecal );

   // Free old verts and indices.
   _freeBuffers( decal );

   F32 halfSize = decal->mSize * 0.5f;
   
   // Ugly hack for ProjectedShadow!
   F32 halfSizeZ = clipDepth ? clipDepth->x : halfSize;
   F32 negHalfSize = clipDepth ? clipDepth->y : halfSize;
   Point3F decalHalfSize( halfSize, halfSize, halfSize );
   Point3F decalHalfSizeZ( halfSizeZ, halfSizeZ, halfSizeZ );

   MatrixF projMat( true );
   decal->getWorldMatrix( &projMat );

   const VectorF &crossVec = decal->mNormal;
   const Point3F &decalPos = decal->mPosition;

   VectorF newFwd, newRight;
   projMat.getColumn( 0, &newRight );
   projMat.getColumn( 1, &newFwd );   

   VectorF objRight( 1.0f, 0, 0 );
   VectorF objFwd( 0, 1.0f, 0 );
   VectorF objUp( 0, 0, 1.0f );

   // See above re: decalHalfSizeZ hack.
   mClipper.clear();
   mClipper.mPlaneList.setSize(6);
   mClipper.mPlaneList[0].set( ( decalPos + ( -newRight * halfSize ) ), -newRight );
   mClipper.mPlaneList[1].set( ( decalPos + ( -newFwd * halfSize ) ), -newFwd );
   mClipper.mPlaneList[2].set( ( decalPos + ( -crossVec * decalHalfSizeZ ) ), -crossVec );
   mClipper.mPlaneList[3].set( ( decalPos + ( newRight * halfSize ) ), newRight );
   mClipper.mPlaneList[4].set( ( decalPos + ( newFwd * halfSize ) ), newFwd );
   mClipper.mPlaneList[5].set( ( decalPos + ( crossVec * negHalfSize ) ), crossVec );

   mClipper.mNormal = decal->mNormal;

   const DecalData *decalData = decal->mDataBlock;

   mClipper.mNormalTolCosineRadians = mCos( mDegToRad( decalData->clippingAngle ) );

   Box3F box( -decalHalfSizeZ, decalHalfSizeZ );

   projMat.mul( box );

   PROFILE_START( DecalManager_clipDecal_buildPolyList );
   getContainer()->buildPolyList( PLC_Decal, box, decalData->clippingMasks, &mClipper );   
   PROFILE_END();

   mClipper.cullUnusedVerts();
   mClipper.triangulate();
   
   const U32 numVerts = mClipper.mVertexList.size();
   const U32 numIndices = mClipper.mIndexList.size();

   if ( !numVerts || !numIndices )
      return false;

   // Fail if either of the buffer metrics exceeds our limits
   // on dynamic geometry buffers.
   if ( numVerts > smMaxVerts ||
        numIndices > smMaxIndices )
      return false;

   if ( !decalData->skipVertexNormals )
      mClipper.generateNormals();
   
#ifdef DECALMANAGER_DEBUG
   mDebugPlanes.clear();
   mDebugPlanes.merge( mClipper.mPlaneList );
#endif

   decal->mVertCount = numVerts;
   decal->mIndxCount = numIndices;
   
   Vector<Point3F> tmpPoints;

   tmpPoints.push_back(( objFwd * decalHalfSize ) + ( objRight * decalHalfSize ));
   tmpPoints.push_back(( objFwd * decalHalfSize ) + ( -objRight * decalHalfSize ));
   tmpPoints.push_back(( -objFwd * decalHalfSize ) + ( -objRight * decalHalfSize ));
   
   Point3F lowerLeft(( -objFwd * decalHalfSize ) + ( objRight * decalHalfSize ));

   projMat.inverse();

   _generateWindingOrder( lowerLeft, &tmpPoints );

   BiQuadToSqr quadToSquare( Point2F( lowerLeft.x, lowerLeft.y ),
                             Point2F( tmpPoints[0].x, tmpPoints[0].y ), 
                             Point2F( tmpPoints[1].x, tmpPoints[1].y ), 
                             Point2F( tmpPoints[2].x, tmpPoints[2].y ) );  

   Point2F uv( 0, 0 );
   Point3F vecX(0.0f, 0.0f, 0.0f);

   // Allocate memory for vert and index arrays
   _allocBuffers( decal );  

   // Mark this so that the color will be assigned on these verts the next
   // time it renders, since we just threw away the previous verts.
   decal->mLastAlpha = -1;
   
   Point3F vertPoint( 0, 0, 0 );

   for ( U32 i = 0; i < mClipper.mVertexList.size(); i++ )
   {
      const ClippedPolyList::Vertex &vert = mClipper.mVertexList[i];
      vertPoint = vert.point;

      // Transform this point to
      // object space to look up the
      // UV coordinate for this vertex.
      projMat.mulP( vertPoint );

      // Clamp the point to be within the quad.
      vertPoint.x = mClampF( vertPoint.x, -decalHalfSize.x, decalHalfSize.x );
      vertPoint.y = mClampF( vertPoint.y, -decalHalfSize.y, decalHalfSize.y );

      // Get our UV.
      uv = quadToSquare.transform( Point2F( vertPoint.x, vertPoint.y ) );

      const RectF &rect = decal->mDataBlock->texRect[decal->mTextureRectIdx];

      uv *= rect.extent;
      uv += rect.point;      

      // Set the world space vertex position.
      decal->mVerts[i].point = vert.point;
      
      decal->mVerts[i].texCoord.set( uv.x, uv.y );
      
      if ( mClipper.mNormalList.empty() )
         continue;

      decal->mVerts[i].normal = mClipper.mNormalList[i];
      decal->mVerts[i].normal.normalize();

      if( mFabs( decal->mVerts[i].normal.z ) > 0.8f ) 
         mCross( decal->mVerts[i].normal, Point3F( 1.0f, 0.0f, 0.0f ), &vecX );
      else if ( mFabs( decal->mVerts[i].normal.x ) > 0.8f )
         mCross( decal->mVerts[i].normal, Point3F( 0.0f, 1.0f, 0.0f ), &vecX );
      else if ( mFabs( decal->mVerts[i].normal.y ) > 0.8f )
         mCross( decal->mVerts[i].normal, Point3F( 0.0f, 0.0f, 1.0f ), &vecX );
   
      decal->mVerts[i].tangent = mCross( decal->mVerts[i].normal, vecX );
   }

   U32 curIdx = 0;
   for ( U32 j = 0; j < mClipper.mPolyList.size(); j++ )
   {
      // Write indices for each Poly
      ClippedPolyList::Poly *poly = &mClipper.mPolyList[j];                  

      AssertFatal( poly->vertexCount == 3, "Got non-triangle poly!" );

      decal->mIndices[curIdx] = mClipper.mIndexList[poly->vertexStart];         
      curIdx++;
      decal->mIndices[curIdx] = mClipper.mIndexList[poly->vertexStart + 1];            
      curIdx++;
      decal->mIndices[curIdx] = mClipper.mIndexList[poly->vertexStart + 2];                
      curIdx++;
   } 

   if ( !edgeVerts )
      return true;

   Point3F tmpHullPt( 0, 0, 0 );
   Vector<Point3F> tmpHullPts;

   for ( U32 i = 0; i < mClipper.mVertexList.size(); i++ )
   {
      const ClippedPolyList::Vertex &vert = mClipper.mVertexList[i];
      tmpHullPt = vert.point;
      projMat.mulP( tmpHullPt );
      tmpHullPts.push_back( tmpHullPt );
   }

   edgeVerts->clear();
   U32 verts = _generateConvexHull( tmpHullPts, edgeVerts );
   edgeVerts->setSize( verts );

   projMat.inverse();
   for ( U32 i = 0; i < edgeVerts->size(); i++ )
      projMat.mulP( (*edgeVerts)[i] );

   return true;
}

DecalInstance* DecalManager::addDecal( const Point3F &pos,
                                       const Point3F &normal,
                                       F32 rotAroundNormal,
                                       DecalData *decalData,
                                       F32 decalScale,
                                       S32 decalTexIndex,
                                       U8 flags )
{      
   MatrixF mat( true );
   MathUtils::getMatrixFromUpVector( normal, &mat );

   AngAxisF rot( normal, rotAroundNormal );
   MatrixF rotmat;
   rot.setMatrix( &rotmat );
   mat.mul( rotmat );
 
   Point3F tangent;
   mat.getColumn( 1, &tangent );

   return addDecal( pos, normal, tangent, decalData, decalScale, decalTexIndex, flags );   
}

DecalInstance* DecalManager::addDecal( const Point3F& pos,
                                       const Point3F& normal,
                                       const Point3F& tangent,
                                       DecalData* decalData,
                                       F32 decalScale,
                                       S32 decalTexIndex,
                                       U8 flags )
{
   if ( !mData && !_createDataFile() )
      return NULL;

   // only dirty the manager if this decal should be saved
   if ( flags & SaveDecal )
      mDirty = true;

   return mData->addDecal( pos, normal, tangent, decalData, decalScale, decalTexIndex, flags );
}

void DecalManager::removeDecal( DecalInstance *inst )
{
   // If this is a decal we save then we need 
   // to set the dirty flag.
   if ( inst->mFlags & SaveDecal )
      mDirty = true;

   // Remove the decal from the instance vector.
   
	if( inst->mId != -1 && inst->mId < mDecalInstanceVec.size() )
      mDecalInstanceVec[ inst->mId ] = NULL;
   
   // Release its geometry (if it has any).

   _freeBuffers( inst );
   
   // Remove it from the decal file.

   if ( mData )      
      mData->removeDecal( inst );
}

DecalInstance* DecalManager::getDecal( S32 id )
{
   if( id < 0 || id >= mDecalInstanceVec.size() )
      return NULL;

   return mDecalInstanceVec[id];
}

void DecalManager::notifyDecalModified( DecalInstance *inst )
{
   // If this is a decal we save then we need 
   // to set the dirty flag.
   if ( inst->mFlags & SaveDecal )
      mDirty = true;

   if ( mData )
      mData->notifyDecalModified( inst );
}

DecalInstance* DecalManager::getClosestDecal( const Point3F &pos )
{
   if ( !mData )
      return NULL;

   const Vector<DecalSphere*> &grid = mData->getSphereList();

   DecalInstance *inst = NULL;
   SphereF worldPickSphere( pos, 0.5f );
   SphereF worldInstSphere( Point3F( 0, 0, 0 ), 1.0f );

   Vector<DecalInstance*> collectedInsts;

   for ( U32 i = 0; i < grid.size(); i++ )
   {
      DecalSphere *decalSphere = grid[i];
      const SphereF &worldSphere = decalSphere->mWorldSphere;
      if (  !worldSphere.isIntersecting( worldPickSphere ) && 
            !worldSphere.isContained( pos ) )
         continue;

      const Vector<DecalInstance*> &items = decalSphere->mItems;
      for ( U32 n = 0; n < items.size(); n++ )
      {
         inst = items[n];
         if ( !inst )
            continue;

         worldInstSphere.center = inst->mPosition;
         worldInstSphere.radius = inst->mSize;

         if ( !worldInstSphere.isContained( inst->mPosition ) )
            continue;

         collectedInsts.push_back( inst );
      }
   }

   F32 closestDistance = F32_MAX;
   F32 currentDist = 0;
   U32 closestIndex = 0;
   for ( U32 i = 0; i < collectedInsts.size(); i++ )
   {
      inst = collectedInsts[i];
      currentDist = (inst->mPosition - pos).len();
      if ( currentDist < closestDistance )
      {
         closestIndex = i;
         closestDistance = currentDist;
         worldInstSphere.center = inst->mPosition;
         worldInstSphere.radius = inst->mSize;
      }
   }

   if (  (!collectedInsts.empty() && 
         collectedInsts[closestIndex] && 
         closestDistance < 1.0f) ||
         worldInstSphere.isContained( pos ) )
      return collectedInsts[closestIndex];
   else
      return NULL;
}

DecalInstance* DecalManager::raycast( const Point3F &start, const Point3F &end, bool savedDecalsOnly )
{
   if ( !mData )
      return NULL;

   const Vector<DecalSphere*> &grid = mData->getSphereList();

   DecalInstance *inst = NULL;
   SphereF worldSphere( Point3F( 0, 0, 0 ), 1.0f );

   Vector<DecalInstance*> hitDecals;

   for ( U32 i = 0; i < grid.size(); i++ )
   {
      DecalSphere *decalSphere = grid[i];
      if ( !decalSphere->mWorldSphere.intersectsRay( start, end ) )
         continue;

      const Vector<DecalInstance*> &items = decalSphere->mItems;
      for ( U32 n = 0; n < items.size(); n++ )
      {
         inst = items[n];
         if ( !inst )
            continue;

         if ( savedDecalsOnly && !(inst->mFlags & SaveDecal) )
            continue;

         worldSphere.center = inst->mPosition;
         worldSphere.radius = inst->mSize;

         if ( !worldSphere.intersectsRay( start, end ) )
            continue;
			
			RayInfo ri;
			bool containsPoint = false;
			if ( gServerContainer.castRayRendered( start, end, STATIC_COLLISION_TYPEMASK, &ri ) )
			{        
				Point2F poly[4];
				poly[0].set( inst->mPosition.x - (inst->mSize / 2), inst->mPosition.y + (inst->mSize / 2));
				poly[1].set( inst->mPosition.x - (inst->mSize / 2), inst->mPosition.y - (inst->mSize / 2));
				poly[2].set( inst->mPosition.x + (inst->mSize / 2), inst->mPosition.y - (inst->mSize / 2));
				poly[3].set( inst->mPosition.x + (inst->mSize / 2), inst->mPosition.y + (inst->mSize / 2));
				
				if ( MathUtils::pointInPolygon( poly, 4, Point2F(ri.point.x, ri.point.y) ) )
					containsPoint = true;
			}

			if( !containsPoint )
				continue;

         hitDecals.push_back( inst );
      }
   }

   if ( hitDecals.empty() )
      return NULL;

   gSortPoint = start;
   dQsort( hitDecals.address(), hitDecals.size(), sizeof(DecalInstance*), cmpDecalDistance );
   return hitDecals[0];
}

U32 DecalManager::_generateConvexHull( const Vector<Point3F> &points, Vector<Point3F> *outPoints )
{
   PROFILE_SCOPE( DecalManager_generateConvexHull );

   // chainHull_2D(): Andrew's monotone chain 2D convex hull algorithm
   //     Input:  P[] = an array of 2D points
   //                   presorted by increasing x- and y-coordinates
   //             n = the number of points in P[]
   //     Output: H[] = an array of the convex hull vertices (max is n)
   //     Return: the number of points in H[]
   //int

   if ( points.size() < 3 )
   {
      outPoints->merge( points );
      return outPoints->size();
   }

   // Sort our input points.
   dQsort( points.address(), points.size(), sizeof( Point3F ), cmpPointsXY ); 

   U32 n = points.size();

   Vector<Point3F> tmpPoints;
   tmpPoints.setSize( n );

   // the output array H[] will be used as the stack
   S32    bot=0, top=(-1);  // indices for bottom and top of the stack
   S32    i;                // array scan index
   S32 toptmp = 0;

   // Get the indices of points with min x-coord and min|max y-coord
   S32 minmin = 0, minmax;
   F32 xmin = points[0].x;
   for ( i = 1; i < n; i++ )
     if (points[i].x != xmin) 
        break;

   minmax = i - 1;
   if ( minmax == n - 1 ) 
   {       
      // degenerate case: all x-coords == xmin
      toptmp = top + 1;
      if ( toptmp < n )
         tmpPoints[++top] = points[minmin];

      if ( points[minmax].y != points[minmin].y ) // a nontrivial segment
      {
         toptmp = top + 1;
         if ( toptmp < n )
            tmpPoints[++top] = points[minmax];
      }

      toptmp = top + 1;
      if ( toptmp < n )
         tmpPoints[++top] = points[minmin];           // add polygon endpoint

      return top+1;
   }

   // Get the indices of points with max x-coord and min|max y-coord
   S32 maxmin, maxmax = n-1;
   F32 xmax = points[n-1].x;

   for ( i = n - 2; i >= 0; i-- )
     if ( points[i].x != xmax ) 
        break;
   
   maxmin = i + 1;

   // Compute the lower hull on the stack H
   toptmp = top + 1;
   if ( toptmp < n )
      tmpPoints[++top] = points[minmin];      // push minmin point onto stack

   i = minmax;
   while ( ++i <= maxmin )
   {
      // the lower line joins P[minmin] with P[maxmin]
      if ( isLeft( points[minmin], points[maxmin], points[i]) >= 0 && i < maxmin )
         continue;          // ignore P[i] above or on the lower line

      while (top > 0)        // there are at least 2 points on the stack
      {
         // test if P[i] is left of the line at the stack top
         if ( isLeft( tmpPoints[top-1], tmpPoints[top], points[i]) > 0)
             break;         // P[i] is a new hull vertex
         else
            top--;         // pop top point off stack
      }

      toptmp = top + 1;
      if ( toptmp < n )
         tmpPoints[++top] = points[i];       // push P[i] onto stack
   }

   // Next, compute the upper hull on the stack H above the bottom hull
   if (maxmax != maxmin)      // if distinct xmax points
   {
      toptmp = top + 1;
      if ( toptmp < n )
         tmpPoints[++top] = points[maxmax];  // push maxmax point onto stack
   }

   bot = top;                 // the bottom point of the upper hull stack
   i = maxmin;
   while (--i >= minmax)
   {
      // the upper line joins P[maxmax] with P[minmax]
      if ( isLeft( points[maxmax], points[minmax], points[i] ) >= 0 && i > minmax )
         continue;          // ignore P[i] below or on the upper line

      while ( top > bot )    // at least 2 points on the upper stack
      { 
         // test if P[i] is left of the line at the stack top
         if ( isLeft( tmpPoints[top-1], tmpPoints[top], points[i] ) > 0 )
             break;         // P[i] is a new hull vertex
         else
            top--;         // pop top point off stack
      }

      toptmp = top + 1;
      if ( toptmp < n )
         tmpPoints[++top] = points[i];       // push P[i] onto stack
   }

   if (minmax != minmin)
   {
      toptmp = top + 1;
      if ( toptmp < n )
         tmpPoints[++top] = points[minmin];  // push joining endpoint onto stack
   }

   outPoints->merge( tmpPoints );

   return top + 1;
}

void DecalManager::_generateWindingOrder( const Point3F &cornerPoint, Vector<Point3F> *sortPoints )
{
   // This block of code is used to find 
   // the winding order for the points in our quad.

   // First, choose an arbitrary corner point.
   // We'll use the "lowerRight" point.
   Point3F relPoint( 0, 0, 0 );

   // See comment below about radius.
   //F32 radius = 0;

   F32 theta = 0;
   
   Vector<Point4F> tmpPoints;

   for ( U32 i = 0; i < (*sortPoints).size(); i++ )
   {
      const Point3F &pnt = (*sortPoints)[i];
      relPoint = cornerPoint - pnt;

      // Get the radius (r^2 = x^2 + y^2).
      
      // This is commented because for a quad
      // you typically can't have the same values
      // for theta, which is the caveat which would
      // require sorting by the radius.
      //radius = mSqrt( (relPoint.x * relPoint.x) + (relPoint.y * relPoint.y) );

      // Get the theta value for the
      // interval -PI, PI.

      // This algorithm for determining the
      // theta value is defined by
      //          | arctan( y / x )  if x > 0
      //          | arctan( y / x )  if x < 0 and y >= 0
      // theta =  | arctan( y / x )  if x < 0 and y < 0
      //          | PI / 2           if x = 0 and y > 0
      //          | -( PI / 2 )      if x = 0 and y < 0
      if ( relPoint.x > 0.0f )
         theta = mAtan2( relPoint.y, relPoint.x );
      else if ( relPoint.x < 0.0f )
      {
         if ( relPoint.y >= 0.0f )
            theta = mAtan2( relPoint.y, relPoint.x ) + M_PI_F;
         else if ( relPoint.y < 0.0f )
            theta = mAtan2( relPoint.y, relPoint.x ) - M_PI_F;
      }
      else if ( relPoint.x == 0.0f )
      {
         if ( relPoint.y > 0.0f )
            theta = M_PI_F / 2.0f;
         else if ( relPoint.y < 0.0f )
            theta = -(M_PI_F / 2.0f);
      }

      tmpPoints.push_back( Point4F( pnt.x, pnt.y, pnt.z, theta ) );
   }

   dQsort( tmpPoints.address(), tmpPoints.size(), sizeof( Point4F ), cmpQuadPointTheta ); 

   for ( U32 i = 0; i < tmpPoints.size(); i++ )
   {
      const Point4F &tmpPoint = tmpPoints[i];
      (*sortPoints)[i].set( tmpPoint.x, tmpPoint.y, tmpPoint.z );
   }
}

void DecalManager::_allocBuffers( DecalInstance *inst )
{
   const S32 sizeClass = _getSizeClass( inst );
   
   void* data;
   if ( sizeClass == -1 )
      data = dMalloc( sizeof( DecalVertex ) * inst->mVertCount + sizeof( U16 ) * inst->mIndxCount );
   else
      data = mChunkers[sizeClass]->alloc();

   inst->mVerts = reinterpret_cast< DecalVertex* >( data );
   data = (U8*)data + sizeof( DecalVertex ) * inst->mVertCount;
   inst->mIndices = reinterpret_cast< U16* >( data );
}

void DecalManager::_freeBuffers( DecalInstance *inst )
{
   if ( inst->mVerts != NULL )
   {
      const S32 sizeClass = _getSizeClass( inst );
      
      if ( sizeClass == -1 )
         dFree( inst->mVerts );
      else
      {
         // Use FreeListChunker
         mChunkers[sizeClass]->free( inst->mVerts );      
      }

      inst->mVerts = NULL;
      inst->mVertCount = 0;
      inst->mIndices = NULL;
      inst->mIndxCount = 0;
   }   
}

void DecalManager::_freePools()
{
   while ( !mVBPool.empty() )
   {
      delete mVBPool.last();
      mVBPool.pop_back();
   }

   while ( !mVBs.empty() )
   {
      delete mVBs.last();
      mVBs.pop_back();
   }

   while ( !mPBPool.empty() )
   {
      delete mPBPool.last();
      mPBPool.pop_back();
   }

   while ( !mPBs.empty() )
   {
      delete mPBs.last();
      mPBs.pop_back();
   }
}

S32 DecalManager::_getSizeClass( DecalInstance *inst ) const
{
   U32 bytes = inst->mVertCount * sizeof( DecalVertex ) + inst->mIndxCount * sizeof ( U16 );

   if ( bytes <= SIZE_CLASS_0 )
      return 0;
   if ( bytes <= SIZE_CLASS_1 )
      return 1;
   if ( bytes <= SIZE_CLASS_2 )
      return 2;

   // Size is outside of the largest chunker.
   return -1;
}

void DecalManager::prepRenderImage( SceneRenderState* state )
{
   PROFILE_SCOPE( DecalManager_RenderDecals );

   if ( !smDecalsOn || !mData ) 
      return;

   // Decals only render in the diffuse pass!
   // We technically could render them into reflections but prefer to save
   // the performance. This would also break the DecalInstance::mLastAlpha 
   // optimization.
   if ( !state->isDiffusePass() )
      return;

   PROFILE_START( DecalManager_RenderDecals_SphereTreeCull );

   const Frustum& rootFrustum = state->getCameraFrustum();

   // Populate vector of decal instances to be rendered with all
   // decals from visible decal spheres.

   SceneManager* sceneManager = state->getSceneManager();
   SceneZoneSpaceManager* zoneManager = sceneManager->getZoneManager();
   AssertFatal( zoneManager, "DecalManager::prepRenderImage - No zone manager!" );
   const Vector<DecalSphere*> &grid = mData->getSphereList();
   const bool haveOnlyOutdoorZone = ( zoneManager->getNumActiveZones() == 1 );
   
   mDecalQueue.clear();
   for ( U32 i = 0; i < grid.size(); i++ )
   {
      DecalSphere* decalSphere = grid[i];
      const SphereF& worldSphere = decalSphere->mWorldSphere;

      // See if this decal sphere can be culled.

      const SceneCullingState& cullingState = state->getCullingState();
      if( haveOnlyOutdoorZone )
      {
         U32 outdoorZone = SceneZoneSpaceManager::RootZoneId;
         if( cullingState.isCulled( worldSphere, &outdoorZone, 1 ) )
            continue;
      }
      else
      {
         // Update the zoning state of the sphere, if we need to.

         if( decalSphere->mZones.size() == 0 )
            decalSphere->updateZoning( zoneManager );

         // Skip the sphere if it is not visible in any of its zones.

         if( cullingState.isCulled( worldSphere, decalSphere->mZones.address(), decalSphere->mZones.size() ) )
            continue;
      }  

      // TODO: If each sphere stored its largest decal instance we
      // could do an LOD step on it here and skip adding any of the
      // decals in the sphere.

      mDecalQueue.merge( decalSphere->mItems );
   }

   PROFILE_END();

   PROFILE_START( DecalManager_RenderDecals_Update );

   const U32 &curSimTime = Sim::getCurrentTime();   
   F32 pixelSize;
   U32 delta, diff;
   DecalInstance *dinst;
   DecalData *ddata;

   // Loop through DecalQueue once for preRendering work.
   // 1. Update DecalInstance fade (over time)
   // 2. Clip geometry if flagged to do so.
   // 3. Calculate lod - if decal is far enough away it will not render.
   for ( U32 i = 0; i < mDecalQueue.size(); i++ )
   {
      dinst = mDecalQueue[i];
      ddata = dinst->mDataBlock;

      // LOD calculation...

      pixelSize = dinst->calcPixelSize( state->getViewport().extent.y, state->getCameraPosition(), state->getWorldToScreenScale().y );

      if ( pixelSize != F32_MAX && pixelSize < ddata->fadeEndPixelSize )      
      {
         mDecalQueue.erase_fast( i );
         i--;
         continue;
      }

      // We will try to render this decal... so do any 
      // final adjustments to it before rendering.

      // Update fade and delete expired.
      if ( !( dinst->mFlags & PermanentDecal || dinst->mFlags & CustomDecal ) )
      {         
         delta = ( curSimTime - dinst->mCreateTime );
         if ( delta > dinst->mDataBlock->lifeSpan )         
         {            
            diff = delta - dinst->mDataBlock->lifeSpan;
            dinst->mVisibility = 1.0f - (F32)diff / (F32)dinst->mDataBlock->fadeTime;

            if ( dinst->mVisibility <= 0.0f )
            {
               mDecalQueue.erase_fast( i );
               removeDecal( dinst );               
               i--;
               continue;
            }
         }
      }

      // Build clipped geometry for this decal if needed.
      if ( dinst->mFlags & ClipDecal && !( dinst->mFlags & CustomDecal ) )
      {  
         // Turn off the flag so we don't continually try to clip
         // if it fails.
         dinst->mFlags = dinst->mFlags & ~ClipDecal;

         if ( !(dinst->mFlags & CustomDecal) && !clipDecal( dinst ) )
         {
            // Clipping failed to get any geometry...

            // Remove it from the render queue.
            mDecalQueue.erase_fast( i );
            i--;

            // If the decal is one placed at run-time (not the editor)
            // then we should also permanently delete the decal instance.
            if ( !(dinst->mFlags & SaveDecal) )
            {
               removeDecal( dinst );
            }

            // If this is a decal placed by the editor it will be
            // flagged to attempt clipping again the next time it is
            // modified. For now we just skip rendering it.      
            continue;
         }         
      }

      // If we get here and the decal still does not have any geometry
      // skip rendering it. It must be an editor placed decal that failed
      // to clip any geometry but has not yet been flagged to try again.
      if ( !dinst->mVerts || dinst->mVertCount == 0 || dinst->mIndxCount == 0 )
      {
         mDecalQueue.erase_fast( i );
         i--;
         continue;
      }
            
      // Calculate the alpha value for this decal and apply it to the verts.            
      {
         PROFILE_START( DecalManager_RenderDecals_Update_SetAlpha );

         F32 alpha = 1.0f;

         // Only necessary for decals which fade over time or distance.
         if ( !( dinst->mFlags & PermanentDecal ) || dinst->mDataBlock->fadeStartPixelSize >= 0.0f )
         {
            if ( pixelSize < ddata->fadeStartPixelSize )
            {
               const F32 range = ddata->fadeStartPixelSize - ddata->fadeEndPixelSize;
               alpha = 1.0f - mClampF( ( ddata->fadeStartPixelSize - pixelSize ) / range, 0.0f, 1.0f );
            }

            alpha *= dinst->mVisibility;
         }
            
         // If the alpha value has not changed since last render avoid
         // looping through all the verts!
         if ( alpha != dinst->mLastAlpha )
         {
            // calculate the swizzles color once, outside the loop.
            GFXVertexColor color;
            color.set( 255, 255, 255, (U8)(alpha * 255.0f) );

            for ( U32 v = 0; v < dinst->mVertCount; v++ )
               dinst->mVerts[v].color = color;

            dinst->mLastAlpha = alpha;
         }      

         PROFILE_END();
      }
   }

   PROFILE_END();      

   if ( mDecalQueue.empty() )
      return;

   // Sort queued decals...
   // 1. Editor decals - in render priority order first, creation time second, and material third.
   // 2. Dynamic decals - in render priority order first and creation time second.
   //
   // With the constraint that decals with different render priority cannot
   // be rendered together in the same draw call.

   PROFILE_START( DecalManager_RenderDecals_Sort );
   dQsort( mDecalQueue.address(), mDecalQueue.size(), sizeof(DecalInstance*), cmpDecalRenderOrder );
   PROFILE_END();

   PROFILE_SCOPE( DecalManager_RenderDecals_RenderBatch );

   RenderPassManager *renderPass = state->getRenderPass();

   // Base render instance we use for convenience.
   // Data shared by all instances we allocate below can be copied
   // from the base instance at the same time.
   MeshRenderInst baseRenderInst;
   baseRenderInst.clear();   

   MatrixF *tempMat = renderPass->allocUniqueXform( MatrixF( true ) );
   MathUtils::getZBiasProjectionMatrix( gDecalBias, rootFrustum, tempMat );
   baseRenderInst.projection = tempMat;

   baseRenderInst.objectToWorld = &MatrixF::Identity;
   baseRenderInst.worldToCamera = renderPass->allocSharedXform(RenderPassManager::View);

   baseRenderInst.type = RenderPassManager::RIT_Decal;      

   // Make it the sort distance the max distance so that 
   // it renders after all the other opaque geometry in 
   // the prepass bin.
   baseRenderInst.sortDistSq = F32_MAX;

   Vector<DecalBatch> batches;
   DecalBatch *currentBatch = NULL;

   // Loop through DecalQueue collecting them into render batches.
   for ( U32 i = 0; i < mDecalQueue.size(); i++ )
   {
      DecalInstance *decal = mDecalQueue[i];      
      DecalData *data = decal->mDataBlock;
      Material *mat = data->getMaterial();

      if ( currentBatch == NULL )
      {
         // Start a new batch, beginning with this decal.

         batches.increment();
         currentBatch = &batches.last();
         currentBatch->startDecal = i;
         currentBatch->decalCount = 1;

         // Shrink and warning: preventing a potential crash.
         currentBatch->iCount =
             (decal->mIndxCount > smMaxIndices) ? smMaxIndices : decal->mIndxCount;
         currentBatch->vCount =
             (decal->mVertCount > smMaxVerts) ? smMaxVerts : decal->mVertCount;
#ifdef TORQUE_DEBUG
         // we didn't mean send a spam to the console
         static U32 countMsgIndx = 0;
         if ( (decal->mIndxCount > smMaxIndices) && ((countMsgIndx++ % 1024) == 0) ) {
            Con::warnf(
               "DecalManager::prepRenderImage() - Shrinked indices of decal."
               " Lost %u.",  (decal->mIndxCount - smMaxIndices)
            );
         }
         static U32 countMsgVert = 0;
         if ( (decal->mVertCount > smMaxVerts) && ((countMsgVert++ % 1024) == 0) ) {
            Con::warnf(
               "DecalManager::prepRenderImage() - Shrinked vertices of decal."
               " Lost %u.",  (decal->mVertCount - smMaxVerts)
            );
         }
#endif

         currentBatch->mat = mat;
         currentBatch->matInst = decal->mDataBlock->getMaterialInstance();
         currentBatch->priority = decal->getRenderPriority();         
         currentBatch->dynamic = !(decal->mFlags & SaveDecal);

         continue;
      }

      if ( currentBatch->iCount + decal->mIndxCount >= smMaxIndices || 
           currentBatch->vCount + decal->mVertCount >= smMaxVerts ||
           currentBatch->mat != mat ||
           currentBatch->priority != decal->getRenderPriority() ||
           decal->mCustomTex )
      {
         // End batch.

         currentBatch = NULL;
         i--;
         continue;
      }

      // Add on to current batch.
      currentBatch->decalCount++;
      currentBatch->iCount += decal->mIndxCount;
      currentBatch->vCount += decal->mVertCount;
   }
   
   // Make sure our primitive and vertex buffer handle storage is
   // big enough to take all batches.  Doing this now avoids reallocation
   // later on which would invalidate all the pointers we already had
   // passed into render instances.
   
   //mPBs.reserve( batches.size() );
   //mVBs.reserve( batches.size() );

   // System memory array of verts and indices so we can fill them incrementally
   // and then memcpy to the graphics device buffers in one call.
   static DecalVertex vertData[smMaxVerts];
   static U16 indexData[smMaxIndices];

   // Loop through batches allocating buffers and submitting render instances.
   for ( U32 i = 0; i < batches.size(); i++ )
   {
      DecalBatch &currentBatch = batches[i];      

      // Copy data into the system memory arrays, from all decals in this batch...

      DecalVertex *vpPtr = vertData;
      U16 *pbPtr = indexData;            

      U32 lastDecal = currentBatch.startDecal + currentBatch.decalCount;

      U32 voffset = 0;
      U32 ioffset = 0;

      // This is an ugly hack for ProjectedShadow!
      GFXTextureObject *customTex = NULL;

      for ( U32 j = currentBatch.startDecal; j < lastDecal; j++ )
      {
         DecalInstance *dinst = mDecalQueue[j];

         const U32 indxCount =
             (dinst->mIndxCount > currentBatch.iCount) ?
             currentBatch.iCount : dinst->mIndxCount;
         for ( U32 k = 0; k < indxCount; k++ )
         {
            *( pbPtr + ioffset + k ) = dinst->mIndices[k] + voffset;            
         }

         ioffset += indxCount;

         const U32 vertCount =
             (dinst->mVertCount > currentBatch.vCount) ?
             currentBatch.vCount : dinst->mVertCount;
         dMemcpy( vpPtr + voffset, dinst->mVerts, sizeof( DecalVertex ) * vertCount );
         voffset += vertCount;

         // Ugly hack for ProjectedShadow!
         if ( (dinst->mFlags & CustomDecal) && dinst->mCustomTex != NULL )
            customTex = *dinst->mCustomTex;
      }

      AssertFatal( ioffset == currentBatch.iCount, "bad" );
      AssertFatal( voffset == currentBatch.vCount, "bad" );
        
      // Get handles to video memory buffers we will be filling...

      GFXVertexBufferHandle<DecalVertex> *vb = NULL;
      
      if ( mVBPool.empty() )
      {
         // If the Pool is empty allocate a new one.
         vb = new GFXVertexBufferHandle<DecalVertex>;
         vb->set( GFX, smMaxVerts, GFXBufferTypeDynamic );                  
      }      
      else 
      {
         // Otherwise grab from the pool.
         vb = mVBPool.last();
         mVBPool.pop_back();
      }

      // Push into our vector of 'in use' buffers.
      mVBs.push_back( vb );
      
      // Ready to start filling.
      vpPtr = vb->lock();

      // Same deal as above...      
      GFXPrimitiveBufferHandle *pb = NULL;
      if ( mPBPool.empty() )
      {
         pb = new GFXPrimitiveBufferHandle;
         pb->set( GFX, smMaxIndices, 0, GFXBufferTypeDynamic );   
      }
      else
      {
         pb = mPBPool.last();
         mPBPool.pop_back();
      }
      mPBs.push_back( pb );
      
      pb->lock( &pbPtr );

      // Memcpy from system to video memory.
      const U32 vpCount = sizeof( DecalVertex ) * currentBatch.vCount;
      dMemcpy( vpPtr, vertData, vpCount );
      const U32 pbCount = sizeof( U16 ) * currentBatch.iCount;
      dMemcpy( pbPtr, indexData, pbCount );

      pb->unlock();
      vb->unlock();

      // DecalManager must hold handles to these buffers so they remain valid,
      // we don't actually use them elsewhere.
      //mPBs.push_back( pb );
      //mVBs.push_back( vb );

      // Get the best lights for the current camera position
      // if the materail is forward lit and we haven't got them yet.
      if ( currentBatch.matInst->isForwardLit() && !baseRenderInst.lights[0] )
      {
         LightQuery query;
         query.init( rootFrustum.getPosition(),
                     rootFrustum.getTransform().getForwardVector(),
                     rootFrustum.getFarDist() );
		   query.getLights( baseRenderInst.lights, 8 );
      }

      // Submit render inst...
      MeshRenderInst *ri = renderPass->allocInst<MeshRenderInst>();
      *ri = baseRenderInst;

      ri->primBuff = pb;
      ri->vertBuff = vb;

      ri->matInst = currentBatch.matInst;

      ri->prim = renderPass->allocPrim();
      ri->prim->type = GFXTriangleList;
      ri->prim->minIndex = 0;
      ri->prim->startIndex = 0;
      ri->prim->numPrimitives = currentBatch.iCount / 3;
      ri->prim->startVertex = 0;
      ri->prim->numVertices = currentBatch.vCount;

      // Ugly hack for ProjectedShadow!
      if ( customTex )
         ri->miscTex = customTex;

      // The decal bin will contain render instances for both decals and decalRoad's.
      // Dynamic decals render last, then editor decals and roads in priority order.
      // DefaultKey is sorted in descending order.
      ri->defaultKey = currentBatch.dynamic ? 0xFFFFFFFF : (U32)currentBatch.priority;
      ri->defaultKey2 = 1;//(U32)lastDecal->mDataBlock;

      renderPass->addInst( ri );
   }

#ifdef TORQUE_GATHER_METRICS
   Con::setIntVariable( "$Decal::Batches", batches.size() );
   Con::setIntVariable( "$Decal::Buffers", mPBs.size() + mPBPool.size() );
   Con::setIntVariable( "$Decal::DecalsRendered", mDecalQueue.size() );
#endif

   if( smDebugRender && gEditingMission )
   {
      ObjectRenderInst* ri = state->getRenderPass()->allocInst< ObjectRenderInst >();

      ri->renderDelegate.bind( this, &DecalManager::_renderDecalSpheres );
      ri->type = RenderPassManager::RIT_Editor;
      ri->defaultKey = 0;
      ri->defaultKey2 = 0;

      state->getRenderPass()->addInst( ri );
   }
}

void DecalManager::_renderDecalSpheres( ObjectRenderInst* ri, SceneRenderState* state, BaseMatInstance* overrideMat )
{
   if( !mData )
      return;

   const Vector<DecalSphere*> &grid = mData->getSphereList();

   GFXDrawUtil *drawUtil = GFX->getDrawUtil();
   ColorI sphereColor( 0, 255, 0, 45 );

   GFXStateBlockDesc desc;
   desc.setBlend( true );
   desc.setZReadWrite( true, false );
   desc.setCullMode( GFXCullNone );

   for ( U32 i = 0; i < grid.size(); i++ )
   {
      DecalSphere *decalSphere = grid[i];
      const SphereF &worldSphere = decalSphere->mWorldSphere;

      if( state->getCullingFrustum().isCulled( worldSphere ) )
         continue;

      drawUtil->drawSphere( desc, worldSphere.radius, worldSphere.center, sphereColor );
   }
}

bool DecalManager::_createDataFile()
{
   AssertFatal( !mData, "DecalManager::tried to create duplicate data file?" );

   // We need to construct a default file name
   char fileName[1024];
   fileName[0] = 0;

   // See if we know our current mission name
   char missionName[1024];
   dStrcpy( missionName, Con::getVariable( "$Client::MissionFile" ) );
   char *dot = dStrstr((const char*)missionName, ".mis");
   if(dot)
      *dot = '\0';
   
   dSprintf( fileName, sizeof(fileName), "%s.mis.decals", missionName );

   mDataFileName = StringTable->insert( fileName );

   // If the file doesn't exist, create an empty file.

   if( !Torque::FS::IsFile( fileName ) )
   {
      FileStream stream;
      if( stream.open( mDataFileName, Torque::FS::File::Write ) )
      {
         DecalDataFile dataFile;
         dataFile.write( stream );
      }
   }

   mData = ResourceManager::get().load( mDataFileName );
   return (bool)mData;
}

void DecalManager::saveDecals( const UTF8* fileName )
{
   if( !mData )
      return;

   // Create the file.

   FileStream stream;
   if ( !stream.open( fileName, Torque::FS::File::Write ) )
   {
      Con::errorf( "DecalManager::saveDecals - Could not open '%s' for writing!", fileName );
      return;
   }

   // Write the data.

   if( !mData->write( stream ) )
   {
      Con::errorf( "DecalManager::saveDecals - Failed to write '%s'", fileName );
      return;
   }

   mDirty = false;
}

bool DecalManager::loadDecals( const UTF8 *fileName )
{
   if( mData )
      clearData();

   mData = ResourceManager::get().load( fileName );

   mDirty = false;

   return mData != NULL;
}

void DecalManager::clearData()
{
   mClearDataSignal.trigger();
   
   // Free all geometry buffers.
   
   if( mData )
   {
      const Vector< DecalSphere* > grid = mData->getSphereList();
      for( U32 i = 0; i < grid.size(); ++ i )
      {
         DecalSphere* sphere = grid[ i ];
         for( U32 n = 0; n < sphere->mItems.size(); ++ n )
            _freeBuffers( sphere->mItems[ n ] );
      }
   }
   
   mData = NULL;
	mDecalInstanceVec.clear();

   _freePools();   
}

bool DecalManager::onSceneAdd()
{
   if( !Parent::onSceneAdd() )
      return false;

   SceneZoneSpaceManager::getZoningChangedSignal().notify( this, &DecalManager::_handleZoningChangedEvent );

   return true;
}

void DecalManager::onSceneRemove()
{
   SceneZoneSpaceManager::getZoningChangedSignal().remove( this, &DecalManager::_handleZoningChangedEvent );
   Parent::onSceneRemove();
}

void DecalManager::_handleZoningChangedEvent( SceneZoneSpaceManager* zoneManager )
{
   if( zoneManager != getSceneManager()->getZoneManager() || !getDecalDataFile() )
      return;

   // Clear the zoning state of all DecalSpheres in the data file.

   const Vector< DecalSphere* > grid = getDecalDataFile()->getSphereList();
   const U32 numSpheres = grid.size();

   for( U32 i = 0; i < numSpheres; ++ i )
      grid[ i ]->mZones.clear();
}

DefineEngineFunction( decalManagerSave, void, ( String decalSaveFile ), ( "" ),
   "Saves the decals for the active mission in the entered filename.\n"
   "@param decalSaveFile Filename to save the decals to.\n"
   "@tsexample\n"
   "// Set the filename to save the decals in. If no filename is set, then the\n"
   "// decals will default to <activeMissionName>.mis.decals\n"
   "%fileName = \"./missionDecals.mis.decals\";\n"
   "// Inform the decal manager to save the decals for the active mission.\n"
   "decalManagerSave( %fileName );\n"
   "@endtsexample\n"
   "@ingroup Decals" )
{
   // If not given a file name, synthesize one.

   if( decalSaveFile.isEmpty() )
   {
      String fileName = String::ToString( "%s.decals", Con::getVariable( "$Client::MissionFile" ) );

      char fullName[ 4096 ];
      Platform::makeFullPathName( fileName, fullName, sizeof( fullName ) );

      decalSaveFile = String( fullName );
   }

   // Write the data.

   gDecalManager->saveDecals( decalSaveFile );
}

DefineEngineFunction( decalManagerLoad, bool, ( const char* fileName ),,
   "Clears existing decals and replaces them with decals loaded from the specified file.\n"
   "@param fileName Filename to load the decals from.\n"
   "@return True if the decal manager was able to load the requested file, "
   "false if it could not.\n"
   "@tsexample\n"
   "// Set the filename to load the decals from.\n"
   "%fileName = \"./missionDecals.mis.decals\";\n"
   "// Inform the decal manager to load the decals from the entered filename.\n"
   "decalManagerLoad( %fileName );\n"
   "@endtsexample\n"
   "@ingroup Decals" )
{
   return gDecalManager->loadDecals( fileName );
}

DefineEngineFunction( decalManagerDirty, bool, (),, 
   "Returns whether the decal manager has unsaved modifications.\n"
   "@return True if the decal manager has unsaved modifications, false if "
   "everything has been saved.\n"
   "@tsexample\n"
   "// Ask the decal manager if it has unsaved modifications.\n"
   "%hasUnsavedModifications = decalManagerDirty();\n"
   "@endtsexample\n"
   "@ingroup Decals" )
{
   return gDecalManager->isDirty();
}

DefineEngineFunction( decalManagerClear, void, (),,
   "Removes all decals currently loaded in the decal manager.\n"
   "@tsexample\n"
   "// Tell the decal manager to remove all existing decals.\n"
   "decalManagerClear();\n"
   "@endtsexample\n"
   "@ingroup Decals" )
{
   gDecalManager->clearData();
}

DefineEngineFunction( decalManagerAddDecal, S32,
   ( Point3F position, Point3F normal, F32 rot, F32 scale, DecalData* decalData, bool isImmortal), ( false ),
   "Adds a new decal to the decal manager.\n"
   "@param position World position for the decal.\n"
   "@param normal Decal normal vector (if the decal was a tire lying flat on a "
   "surface, this is the vector pointing in the direction of the axle).\n"
   "@param rot Angle (in radians) to rotate this decal around its normal vector.\n"
   "@param scale Scale factor applied to the decal.\n"
   "@param decalData DecalData datablock to use for the new decal.\n"
   "@param isImmortal Whether or not this decal is immortal. If immortal, it "
   "does not expire automatically and must be removed explicitly.\n"
   "@return Returns the ID of the new Decal object or -1 on failure.\n"
   "@tsexample\n"
   "// Specify the decal position\n"
   "%position = \"1.0 1.0 1.0\";\n\n"
   "// Specify the up vector\n"
   "%normal = \"0.0 0.0 1.0\";\n\n"
   "// Add the new decal.\n"
   "%decalObj = decalManagerAddDecal( %position, %normal, 0.5, 0.35, ScorchBigDecal, false );\n"
   "@endtsexample\n"
   "@ingroup Decals" )
{
   if( !decalData )
   {
      Con::errorf( "decalManagerAddDecal - Invalid Decal DataBlock" );
      return -1;
   }

   U8 flags = 0;
   if( isImmortal )
      flags |= PermanentDecal;

   DecalInstance* inst = gDecalManager->addDecal( position, normal, rot, decalData, scale, -1, flags );
   if( !inst )
   {
      Con::errorf( "decalManagerAddDecal - Unable to create decal instance." );
      return -1;
   }

   // Add the decal to the instance vector.
   inst->mId = gDecalManager->mDecalInstanceVec.size();
   gDecalManager->mDecalInstanceVec.push_back( inst );

   return inst->mId;
}

DefineEngineFunction( decalManagerRemoveDecal, bool, ( S32 decalID ),,
   "Remove specified decal from the scene.\n"
   "@param decalID ID of the decal to remove.\n"
   "@return Returns true if successful, false if decal ID not found.\n"
   "@tsexample\n"
   "// Specify a decal ID to be removed\n"
   "%decalID = 1;\n\n"
   "// Tell the decal manager to remove the specified decal ID.\n"
   "decalManagerRemoveDecal( %decalId )\n"
   "@endtsexample\n"
   "@ingroup Decals" )
{
   DecalInstance *inst = gDecalManager->getDecal( decalID );
   if( !inst )
      return false;

   gDecalManager->removeDecal(inst);
   return true;
}

DefineEngineFunction( decalManagerEditDecal, bool, ( S32 decalID, Point3F pos, Point3F normal, F32 rotAroundNormal, F32 decalScale ),,
   "Edit specified decal of the decal manager.\n"
   "@param decalID ID of the decal to edit.\n"
   "@param pos World position for the decal.\n"
   "@param normal Decal normal vector (if the decal was a tire lying flat on a "
   "surface, this is the vector pointing in the direction of the axle).\n"
   "@param rotAroundNormal Angle (in radians) to rotate this decal around its normal vector.\n"
   "@param decalScale Scale factor applied to the decal.\n"
   "@return Returns true if successful, false if decalID not found.\n"
   "" )
{
   DecalInstance *decalInstance = gDecalManager->getDecal( decalID );
   if( !decalInstance )
		return false;

   //Internally we need Point3F tangent instead of the user friendly F32 rotAroundNormal
   MatrixF mat( true );
   MathUtils::getMatrixFromUpVector( normal, &mat );

   AngAxisF rot( normal, rotAroundNormal );
   MatrixF rotmat;
   rot.setMatrix( &rotmat );
   mat.mul( rotmat );

   Point3F tangent;
   mat.getColumn( 1, &tangent );
   
   //if everything is unchanged just do nothing and  return "everything is ok"
   if ( pos.equal(decalInstance->mPosition) &&
        normal.equal(decalInstance->mNormal) &&
        tangent.equal(decalInstance->mTangent) &&
        mFabs( decalInstance->mSize - (decalInstance->mDataBlock->size * decalScale) ) < POINT_EPSILON )
           return true;

   decalInstance->mPosition = pos;
   decalInstance->mNormal = normal;
   decalInstance->mTangent = tangent;
   decalInstance->mSize = decalInstance->mDataBlock->size * decalScale;

   gDecalManager->clipDecal( decalInstance, NULL, NULL);
   
   gDecalManager->notifyDecalModified( decalInstance );
   return true;
}
