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
#include "environment/river.h"

#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "util/catmullRom.h"
#include "math/util/quadTransforms.h"
#include "scene/simPath.h"
#include "scene/sceneRenderState.h"
#include "scene/sceneManager.h"
#include "materials/sceneData.h"
#include "materials/baseMatInstance.h"
#include "scene/sgUtil.h"
#include "T3D/gameBase/gameConnection.h"
#include "core/stream/bitStream.h"
#include "gfx/gfxDrawUtil.h"
#include "gfx/gfxTransformSaver.h"
#include "gfx/primBuilder.h"
#include "gfx/gfxDebugEvent.h"
#include "gfx/gfxOcclusionQuery.h"
#include "math/mathIO.h"
#include "math/mathUtils.h"
#include "math/util/frustum.h"
#include "math/util/quadTransforms.h"
#include "gui/3d/guiTSControl.h"
#include "gfx/sim/debugDraw.h"
#include "T3D/fx/particleEmitter.h"
#include "scene/reflectionManager.h"
#include "ts/tsShapeInstance.h"
#include "postFx/postEffect.h"
#include "math/util/matrixSet.h"
#include "environment/nodeListManager.h"

ConsoleDocClass( River,
   "@brief A water volume defined by a 3D spline.\n\n"
   
   "User may control width and depth per node and overall spline shape in three "
   "dimensions.\n\n"   
      
   "%River supports dynamic planar reflections (fullReflect) like all WaterObject "
   "classes, but keep in mind it is not necessarily a planar surface. For best "
   "visual quality a %River should be less reflective the more it twists and "
   "bends. This caution only applies to %Rivers with fullReflect on.\n\n"

   "@see WaterObject for inherited functionality.\n\n"

   "@ingroup Water"
);

#define MIN_METERS_PER_SEGMENT 1.0f
#define MIN_NODE_DEPTH 0.25f
#define MAX_NODE_DEPTH 500.0f
#define MIN_NODE_WIDTH 0.25f
#define MAX_NODE_WIDTH 1000.0f
#define NODE_RADIUS 15.0f

static U32 gIdxArray[6][2][3] = {
	{ { 0, 4, 5 }, { 0, 5, 1 }, },   // Top Face
	{ { 2, 6, 4 }, { 2, 4, 0 }, },   // Left Face
	{ { 1, 5, 7 }, { 1, 7, 3 }, },   // Right Face
	{ { 2, 3, 7 }, { 2, 7, 6 }, },   // Bottom Face
	{ { 0, 1, 3 }, { 0, 3, 2 }, },   // Front Face
	{ { 4, 6, 7 }, { 4, 7, 5 }, },   // Back Face
};

struct RiverHitSegment
{
   U32 idx;
   F32 t;
};

static S32 QSORT_CALLBACK compareHitSegments(const void* a,const void* b)
{
	const RiverHitSegment *fa = (RiverHitSegment*)a;
	const RiverHitSegment *fb = (RiverHitSegment*)b;

	return mSign(fb->t - fa->t);
}

static Point3F sSegmentPointComparePoints[4];

//-----------------------------------------------------------------------------
// DecalRoadNodeList Struct
//-----------------------------------------------------------------------------

struct RiverNodeList : public NodeListManager::NodeList
{
   Vector<Point3F>   mPositions;
   Vector<F32>       mWidths;
   Vector<F32>       mDepths;
   Vector<VectorF>   mNormals;

   RiverNodeList() { }
   virtual ~RiverNodeList() { }
};

//-----------------------------------------------------------------------------
// RiverNodeEvent Class
//-----------------------------------------------------------------------------

class RiverNodeEvent : public NodeListEvent
{
   typedef NodeListEvent Parent;

public:
   Vector<Point3F>   mPositions;
   Vector<F32>       mWidths;
   Vector<F32>       mDepths;
   Vector<VectorF>   mNormals;

public:
   RiverNodeEvent() { mNodeList = NULL; }
   virtual ~RiverNodeEvent() { }

   virtual void pack(NetConnection*, BitStream*);
   virtual void unpack(NetConnection*, BitStream*);

   virtual void copyIntoList(NodeListManager::NodeList* copyInto);
   virtual void padListToSize();

   DECLARE_CONOBJECT(RiverNodeEvent);
};

void RiverNodeEvent::pack(NetConnection* conn, BitStream* stream)
{
   Parent::pack( conn, stream );

   stream->writeInt( mPositions.size(), 16 );

   for (U32 i=0; i<mPositions.size(); ++i)
   {
      mathWrite( *stream, mPositions[i] );
      stream->write( mWidths[i] );
      stream->write( mDepths[i] );
      mathWrite( *stream, mNormals[i] );         
   }
}

void RiverNodeEvent::unpack(NetConnection* conn, BitStream* stream)
{
   mNodeList = new RiverNodeList();

   Parent::unpack( conn, stream );

   U32 count = stream->readInt( 16 );

   Point3F pos;
   F32 width, depth;
   VectorF normal;

   RiverNodeList* list = static_cast<RiverNodeList*>(mNodeList);

   for (U32 i=0; i<count; ++i)
   {
      mathRead( *stream, &pos );
      stream->read( &width );   
      stream->read( &depth );
      mathRead( *stream, &normal );

      list->mPositions.push_back( pos );
      list->mWidths.push_back( width );
      list->mDepths.push_back( depth );
      list->mNormals.push_back( normal );
   }

   list->mTotalValidNodes = count;

   // Do we have a complete list?
   if (list->mPositions.size() >= mTotalNodes)
      list->mListComplete = true;
}

void RiverNodeEvent::copyIntoList(NodeListManager::NodeList* copyInto)
{
   RiverNodeList* prevList = dynamic_cast<RiverNodeList*>(copyInto);
   RiverNodeList* list = static_cast<RiverNodeList*>(mNodeList);

   // Merge our list with the old list.
   for (U32 i=mLocalListStart, index=0; i<mLocalListStart+list->mPositions.size(); ++i, ++index)
   {
      prevList->mPositions[i] = list->mPositions[index];
      prevList->mWidths[i] = list->mWidths[index];
      prevList->mDepths[i] = list->mDepths[index];
      prevList->mNormals[i] = list->mNormals[index];
   }
}

void RiverNodeEvent::padListToSize()
{
   RiverNodeList* list = static_cast<RiverNodeList*>(mNodeList);

   U32 totalValidNodes = list->mTotalValidNodes;

   // Pad our list front?
   if (mLocalListStart)
   {
      RiverNodeList* newlist = new RiverNodeList();
      newlist->mPositions.increment(mLocalListStart);
      newlist->mWidths.increment(mLocalListStart);
      newlist->mDepths.increment(mLocalListStart);
      newlist->mNormals.increment(mLocalListStart);

      newlist->mPositions.merge(list->mPositions);
      newlist->mWidths.merge(list->mWidths);
      newlist->mDepths.merge(list->mDepths);
      newlist->mNormals.merge(list->mNormals);

      mNodeList = newlist;
      delete list;
   }

   // Pad our list end?
   if (list->mPositions.size() < mTotalNodes)
   {
      U32 delta = mTotalNodes - list->mPositions.size();
      list->mPositions.increment(delta);
      list->mWidths.increment(delta);
      list->mDepths.increment(delta);
      list->mNormals.increment(delta);
   }

   list->mTotalValidNodes = totalValidNodes;
}

IMPLEMENT_CO_NETEVENT_V1(RiverNodeEvent);

ConsoleDocClass( RiverNodeEvent,
   "@brief Sends messages to the River Editor\n\n"
   "Editor use only.\n\n"
   "@internal"
);
//-----------------------------------------------------------------------------
// RiverNodeListNotify Class
//-----------------------------------------------------------------------------

class RiverNodeListNotify : public NodeListNotify
{
   typedef NodeListNotify Parent;

protected:
   SimObjectPtr<River> mRiver;

public:
   RiverNodeListNotify( River* river, U32 listId ) { mRiver = river; mListId = listId; }
   virtual ~RiverNodeListNotify() { mRiver = NULL; }

   virtual void sendNotification( NodeListManager::NodeList* list );
};

void RiverNodeListNotify::sendNotification( NodeListManager::NodeList* list )
{
   if (mRiver.isValid())
   {
      // Build the road's nodes
      RiverNodeList* riverList = dynamic_cast<RiverNodeList*>( list );
      if (riverList)
         mRiver->buildNodesFromList( riverList );
   }
}

//------------------------------------------------------------------------------
// Class: RiverSegment
//------------------------------------------------------------------------------

RiverSegment::RiverSegment()
{
   mPlaneCount = 0;
   columns = 0;
   rows = 0;
   numVerts = 0;
   numTriangles = 0;

   startVert = 0;
   endVert = 0;
   startIndex = 0;
   endIndex = 0;

   slice0 = NULL;
   slice1 = NULL;
}

RiverSegment::RiverSegment( RiverSlice *rs0, RiverSlice *rs1 )
{
   columns = 0;
   rows = 0;
   numVerts = 0;
   numTriangles = 0;

   startVert = 0;
   endVert = 0;
   startIndex = 0;
   endIndex = 0;

   slice0 = rs0;
   slice1 = rs1;

   // Calculate the planes for this segment
   // Will be used for intersection/buoyancy tests
    VectorF normal;
    mPlaneCount = 6;
 
    sSegmentPointCompareReference = getFaceCenter(6);

    // left
    mPlanes[0] = _getBestPlane( &slice1->p0, &slice1->pb0, &slice0->pb0, &slice0->p0 );
    
    // right
    mPlanes[1] = _getBestPlane( &slice0->pb2, &slice1->pb2, &slice1->p2, &slice0->p2 );    
 
    // near    
    mPlanes[2] = _getBestPlane( &slice0->pb0, &slice0->pb2, &slice0->p2, &slice0->p0 );
 
    // far    
    mPlanes[3] = _getBestPlane( &slice1->pb2, &slice1->pb0, &slice1->p0, &slice1->p2 );
 
    // top
    mPlanes[4] = _getBestPlane( &slice0->p2, &slice1->p2, &slice1->p0, &slice0->p0 );
 
    // bottom
    mPlanes[5] = _getBestPlane( &slice0->pb2, &slice0->pb0, &slice1->pb0, &slice1->pb2 );

   // Calculate the bounding box(s)
   worldbounds.minExtents = worldbounds.maxExtents = rs0->p0;
   worldbounds.extend( rs0->p2 );
   worldbounds.extend( rs0->pb0 );
   worldbounds.extend( rs0->pb2 );
   worldbounds.extend( rs1->p0 );
   worldbounds.extend( rs1->p2 );
   worldbounds.extend( rs1->pb0 );
   worldbounds.extend( rs1->pb2 );

   /*
   // Calculate tetrahedrons (for collision and buoyancy testing)
      // This is 0 in the diagram.
      mCubePoints[0] = cornerPoint;
      mCubePoints[1] = cornerPoint + (VectorF( 1.0f, 0.0f, 0.0f ) * size );
      mCubePoints[2] = cornerPoint + (VectorF( 0.0f, 1.0f, 0.0f ) * size );
      mCubePoints[3] = cornerPoint + (VectorF( 1.0f, 1.0f, 0.0f ) * size );
   
      mCubePoints[4] = cornerPoint + (VectorF( 0.0f, 0.0f, 1.0f );
      mCubePoints[5] = cornerPoint + (VectorF( 1.0f, 0.0f, 1.0f );
      mCubePoints[6] = cornerPoint + (VectorF( 0.0f, 1.0f, 1.0f );
      mCubePoints[7] = cornerPoint + (VectorF( 1.0f, 1.0f, 1.0f );
   
      // Center tetra.
      mTetras[0].p0 = &mCubePoints[1];
      mTetras[0].p1 = &mCubePoints[2];
      mTetras[0].p2 = &mCubePoints[4];
      mTetras[0].p3 = &mCubePoints[7];
   
   
   
      mTetras[1].p0 = &mCubePoints[0]; // this is the tip
      mTetras[1].p1 = &mCubePoints[1];
      mTetras[1].p2 = &mCubePoints[2];
      mTetras[1].p3 = &mCubePoints[4];
   
      mTetras[2].p0 = &mCubePoints[3]; // tip
      mTetras[2].p1 = &mCubePoints[2];
      mTetras[2].p2 = &mCubePoints[1];
      mTetras[2].p3 = &mCubePoints[7];
   
      mTetras[3].p0 = &mCubePoints[6]; // tip
      mTetras[3].p1 = &mCubePoints[7];
      mTetras[3].p2 = &mCubePoints[4];
      mTetras[3].p3 = &mCubePoints[2];
   
      mTetras[4].p0 = &mCubePoints[5]; // tip
      mTetras[4].p1 = &mCubePoints[7];
      mTetras[4].p2 = &mCubePoints[4];
      mTetras[4].p3 = &mCubePoints[3];*/
   
}

void RiverSegment::set( RiverSlice *rs0, RiverSlice *rs1 )
{
   columns = 0;
   rows = 0;
   numVerts = 0;
   numTriangles = 0;

   startVert = 0;
   endVert = 0;
   startIndex = 0;
   endIndex = 0;

   slice0 = rs0;
   slice1 = rs1;
}

static S32 QSORT_CALLBACK SegmentPointCompare(const void *aptr, const void *bptr)
{
   const U32 a = *(const U32*)aptr;
   const U32 b = *(const U32*)bptr;
   
   F32 lenA = ( sSegmentPointCompareReference - sSegmentPointComparePoints[a] ).lenSquared();
   F32 lenB = ( sSegmentPointCompareReference - sSegmentPointComparePoints[b] ).lenSquared();
   return ( lenB - lenA );   
}

PlaneF RiverSegment::_getBestPlane( const Point3F *p0, const Point3F *p1, const Point3F *p2, const Point3F *p3 )
{   
   sSegmentPointComparePoints[0] = *p0;
   sSegmentPointComparePoints[1] = *p1;
   sSegmentPointComparePoints[2] = *p2;
   sSegmentPointComparePoints[3] = *p3;

   Point3F points[4] = {
      *p0, *p1, *p2, *p3
   };

   U32 indices[4] = {
      0,1,2,3
   };

   dQsort(indices, 4, sizeof(U32), SegmentPointCompare);

   // Collect the best three points (in correct winding order)
   // To generate the plane's normal
   Vector<Point3F> normalPnts;   

   for ( U32 i = 0; i < 4; i++ )
   {
      if ( i == indices[3] )
         continue;

      normalPnts.push_back(points[i]);
   }

   PlaneF plane( normalPnts[0], normalPnts[1], normalPnts[2] );
   return plane;
}

Point3F RiverSegment::getFaceCenter( U32 faceIdx ) const
{
   Point3F center(0,0,0);

   switch ( faceIdx )
   {
   case 0: // left
      center = slice1->p0 + slice0->p0 + slice0->pb0 + slice1->pb0;
      center *= 0.25f;
      break;

   case 1: // right
      center = slice0->p2 + slice1->p2 + slice1->pb2 + slice0->pb2;
      center *= 0.25f;
      break;   

   case 2: // near    
      center = slice0->p0 + slice0->p2 + slice0->pb2 + slice0->pb0;
      center *= 0.25f;
      break;   

   case 3: // far    
      center = slice1->pb0 + slice1->p0 + slice1->pb0 + slice1->pb2;
      center *= 0.25f;
      break;   

   case 4: // top
      center = slice0->p0 + slice1->p0 + slice1->p2 + slice0->p2;
      center *= 0.25f;
      break;   

   case 5: // bottom
      center = slice1->pb2 + slice1->pb0 + slice0->pb0 + slice0->pb2;
      center *= 0.25f;
      break;

   case 6: // segment center
      center = slice0->p0 + slice0->p2 + slice1->p0 + slice1->p2 + slice0->pb0 + slice0->pb2 + slice1->pb0 + slice1->pb2;
      center /= 8;
      break;
   }

   return center;
}

bool RiverSegment::intersectBox( const Box3F &bounds ) const
{
   // This code copied from Frustum class.

   Point3F maxPoint;
   F32 maxDot;

   // Note the planes are ordered left, right, near, 
   // far, top, bottom for getting early rejections
   // from the typical horizontal scene.
   for ( S32 i = 0; i < mPlaneCount; i++ )
   {
      // This is pretty much as optimal as you can
      // get for a plane vs AABB test...
      // 
      // 4 comparisons
      // 3 multiplies
      // 2 adds
      // 1 negation
      //
      // It will early out as soon as it detects the
      // bounds is outside one of the planes.

      if ( mPlanes[i].x > 0 )
         maxPoint.x = bounds.maxExtents.x;
      else
         maxPoint.x = bounds.minExtents.x;

      if ( mPlanes[i].y > 0 )
         maxPoint.y = bounds.maxExtents.y;
      else
         maxPoint.y = bounds.minExtents.y;

      if ( mPlanes[i].z > 0 )
         maxPoint.z = bounds.maxExtents.z;
      else
         maxPoint.z = bounds.minExtents.z;

      maxDot = mDot( maxPoint, mPlanes[ i ] );

      if ( maxDot <= -mPlanes[ i ].d )
         return false;
   }

   return true;
}

bool RiverSegment::containsPoint( const Point3F &pnt ) const
{
   // NOTE: this code from Frustum class.

   F32 maxDot;

   // Note the planes are ordered left, right, near, 
   // far, top, bottom for getting early rejections
   // from the typical horizontal scene.
   for ( S32 i = 0; i < mPlaneCount; i++ )
   {
      const PlaneF &plane = mPlanes[ i ];

      // This is pretty much as optimal as you can
      // get for a plane vs point test...
      // 
      // 1 comparison
      // 2 multiplies
      // 1 adds
      //
      // It will early out as soon as it detects the
      // point is outside one of the planes.

      maxDot = mDot( pnt, plane ) + plane.d;
      if ( maxDot < -0.1f )
         return false;
   }

   return true;
}

F32 RiverSegment::distanceToSurface(const Point3F &pnt) const
{
   return mPlanes[4].distToPlane( pnt );
}

bool River::smEditorOpen = false;
bool River::smWireframe = false;
bool River::smShowWalls = false;
bool River::smShowNodes = false;
bool River::smShowSpline = true;
bool River::smShowRiver = true;
SimObjectPtr<SimSet> River::smServerRiverSet = NULL;

IMPLEMENT_CO_NETOBJECT_V1(River);


River::River()
 : mMetersPerSegment(10.0f),
   mSegmentsPerBatch(10),
   mDepthScale(1.0f),
   mMaxDivisionSize(2.5f),
   mMinDivisionSize(0.25f),
	mColumnCount(5),
   mFlowMagnitude(1.0f),
   mLodDistance( 50.0f )   
{   
   mNetFlags.set( Ghostable | ScopeAlways );

   mObjScale.set( 1, 1, 1 );

   mObjBox.minExtents.set( -0.5, -0.5, -0.5 );
   mObjBox.maxExtents.set(  0.5,  0.5,  0.5 );

   mReflectNormalUp = false;

   // We use the shader const miscParams.w to signify
   // that this object is a River.
   mMiscParamW = 1.0f;
}

River::~River()
{      
}

void River::initPersistFields()
{
   addGroup( "River" );

      addField( "SegmentLength",       TypeF32,    Offset( mMetersPerSegment, River ),
         "Divide the River lengthwise into segments of this length in meters. "
         "These geometric volumes are used for spacial queries like determining containment." );      

      addField( "SubdivideLength",     TypeF32,    Offset( mMaxDivisionSize, River ),
         "For purposes of generating the renderable geometry River segments are further subdivided "
         "such that no quad is of greater width or length than this distance in meters." );

      addField( "FlowMagnitude",       TypeF32,    Offset( mFlowMagnitude, River ),
         "Magnitude of the force vector applied to dynamic objects within the River." );

      addField( "LowLODDistance",      TypeF32,    Offset( mLodDistance, River ),
         "Segments of the river at this distance in meters or greater will "
         "render as a single unsubdivided without undulation effects." );      

   endGroup( "River" );   

   addGroup( "Internal" );

      addProtectedField( "Node", TypeString, NULL, &addNodeFromField, &emptyStringProtectedGetFn, "For internal use, do not modify." );

   endGroup( "Internal" );

   Parent::initPersistFields();
}

void River::consoleInit()
{
   Parent::consoleInit();

   Con::addVariable( "$River::EditorOpen", TypeBool, &River::smEditorOpen, "For editor use.\n"
	   "@ingroup Editors\n" );
   Con::addVariable( "$River::showWalls", TypeBool, &River::smShowWalls, "For editor use.\n"
	   "@ingroup Editors\n" );
   Con::addVariable( "$River::showNodes", TypeBool, &River::smShowNodes, "For editor use.\n"
	   "@ingroup Editors\n");
   Con::addVariable( "$River::showSpline", TypeBool, &River::smShowSpline, "For editor use.\n"
	   "@ingroup Editors\n" );
   Con::addVariable( "$River::showRiver", TypeBool, &River::smShowRiver, "For editor use.\n"
	   "@ingroup Editors\n" );
	Con::addVariable( "$River::showWireframe", TypeBool, &River::smWireframe, "For editor use.\n"
	   "@ingroup Editors\n");
}

bool River::addNodeFromField( void *object, const char *index, const char *data )
{
   River *pObj = static_cast<River*>(object);

   //if ( !pObj->isProperlyAdded() )
   //{      
   F32 x,y,z,width,depth;      
   VectorF normal;
   U32 result = dSscanf( data, "%f %f %f %f %f %f %f %f", &x, &y, &z, &width, &depth, &normal.x, &normal.y, &normal.z );      
   if ( result == 8 )   
      pObj->_addNode( Point3F(x,y,z), width, depth, normal );      
   //}

   return false;
}

bool River::onAdd()
{
   if ( !Parent::onAdd() ) 
      return false;

   // Reset the World Box.
   //setGlobalBounds();
   resetWorldBox();

   // Set the Render Transform.
   setRenderTransform(mObjToWorld);

   // Add to Scene.
   addToScene();
   
   if ( isServerObject() )
      getServerSet()->addObject( this );   

   _regenerate();

   return true;
}

void River::onRemove()
{
   removeFromScene();

   Parent::onRemove();
}

void River::inspectPostApply()
{
   // Set Parent.
   Parent::inspectPostApply();

   if ( mMetersPerSegment < MIN_METERS_PER_SEGMENT )
      mMetersPerSegment = MIN_METERS_PER_SEGMENT;

   mMaxDivisionSize = getMax( mMaxDivisionSize, mMinDivisionSize );      

   // Set fxPortal Mask.
   setMaskBits(RiverMask|RegenMask);
}

void River::onStaticModified( const char* slotName, const char*newValue )
{
   Parent::onStaticModified( slotName, newValue );

   if ( dStricmp( slotName, "surfMaterial" ) == 0 )
      setMaskBits( MaterialMask );
}

SimSet* River::getServerSet()
{
   if ( !smServerRiverSet )
   {
      smServerRiverSet = new SimSet();
      smServerRiverSet->registerObject( "ServerRiverSet" );
      Sim::getRootGroup()->addObject( smServerRiverSet );
   }

   return smServerRiverSet;
}

void River::writeFields( Stream &stream, U32 tabStop )
{
   Parent::writeFields( stream, tabStop );

   // Now write all nodes

   stream.write(2, "\r\n");   

   for ( U32 i = 0; i < mNodes.size(); i++ )
   {
      const RiverNode &node = mNodes[i];

      stream.writeTabs(tabStop);

      char buffer[1024];
      dMemset( buffer, 0, 1024 );
      dSprintf( buffer, 1024, "Node = \"%f %f %f %f %f %f %f %f\";", node.point.x, node.point.y, node.point.z, 
                                                                     node.width, 
                                                                     node.depth, 
                                                                     node.normal.x, node.normal.y, node.normal.z );      
      stream.writeLine( (const U8*)buffer );
   }
}

bool River::writeField( StringTableEntry fieldname, const char *value )
{   
   if ( fieldname == StringTable->insert("node") )
      return false;

   return Parent::writeField( fieldname, value );
}

void River::innerRender( SceneRenderState *state )
{   
   GFXDEBUGEVENT_SCOPE( River_innerRender, ColorI( 255, 0, 0 ) );

   PROFILE_SCOPE( River_innerRender );

   // Setup SceneData
   SceneData sgData;
   sgData.init( state );
   sgData.lights[0] = LIGHTMGR->getSpecialLight( LightManager::slSunLightType );
   sgData.backBuffTex = REFLECTMGR->getRefractTex();
   sgData.reflectTex = mPlaneReflector.reflectTex; 
   sgData.wireframe |= smWireframe;

   const Point3F &camPosition = state->getCameraPosition();

   // set the material

   S32 matIdx = getMaterialIndex( camPosition );

   if ( !initMaterial( matIdx ) )
      return;

   BaseMatInstance *mat = mMatInstances[matIdx];
   WaterMatParams matParams = mMatParamHandles[matIdx];

   if ( !mat )      
      return;

   // setup proj/world transform
   GFXTransformSaver saver;

   setShaderParams( state, mat, matParams );

   _makeRenderBatches( camPosition );

	if ( !River::smShowRiver )      
      return;

   // If no material... we're done.
   if ( mLowLODBatches.empty() && mHighLODBatches.empty() )      
      return;

   if ( !mHighLODBatches.empty() )
      _makeHighLODBuffers();

   mMatrixSet->restoreSceneViewProjection();
   mMatrixSet->setWorld( MatrixF::Identity );

   while( mat->setupPass( state, sgData ) )
   {
      mat->setSceneInfo(state, sgData);
      mat->setTransforms(*mMatrixSet, state);

      setCustomTextures( matIdx, mat->getCurPass(), matParams );      

      GFX->setVertexBuffer( mVB_low );
      GFX->setPrimitiveBuffer( mPB_low );

      for ( U32 i = 0; i < mLowLODBatches.size(); i++ )
      {
         const RiverRenderBatch &batch = mLowLODBatches[i];

         U32 startVert = batch.startSegmentIdx * 2;
         U32 endVert = ( batch.endSegmentIdx + 1 ) * 2 + 1;
         U32 startIdx = batch.startSegmentIdx * 6;
         U32 endIdx = batch.endSegmentIdx * 6 + 5;
          
         U32 vertCount = ( endVert - startVert ) + 1;
         U32 idxCount = ( endIdx - startIdx ) + 1;
         U32 triangleCount = idxCount / 3;
         				
         AssertFatal( startVert < mLowVertCount, "River, bad draw call!" );
         AssertFatal( startVert + vertCount <= mLowVertCount, "River, bad draw call!" );
         AssertFatal( triangleCount <= mLowTriangleCount, "River, bad draw call!" );

         GFX->drawIndexedPrimitive( GFXTriangleList, 0, startVert, vertCount, startIdx, triangleCount );
      }
      
      // Render all high detail batches.
      //
      // It is possible that the buffers could not be allocated because
      // the max number of verts/indices was exceeded.  We don't want to 
      // crash because that would be unhelpful for working in the editor.
      if ( mVB_high.isValid() && mPB_high.isValid() )
      {
         GFX->setVertexBuffer( mVB_high );
         GFX->setPrimitiveBuffer( mPB_high );

         for ( U32 i = 0; i < mHighLODBatches.size(); i++ )
         {
            const RiverRenderBatch &batch = mHighLODBatches[i];

            AssertFatal( batch.startVert < mHighVertCount, "River, bad draw call!" );
            AssertFatal( batch.startVert + batch.vertCount <= mHighVertCount, "River, bad draw call!" );            
            AssertFatal( batch.triangleCount <= mHighTriangleCount, "River, bad draw call!" );
            AssertFatal( batch.startIndex < mHighTriangleCount * 3, "River, bad draw call!" );
            AssertFatal( batch.startIndex + batch.triangleCount * 3 <= mHighTriangleCount * 3, "River, bad draw call!" );

            GFX->drawIndexedPrimitive( GFXTriangleList, 
                                       0, 
                                       0, 
                                       batch.vertCount, 
                                       batch.startIndex, 
                                       batch.triangleCount );
         }
      }

   } // while( mat->setupPass( sgData ) )      
}

void River::updateUnderwaterEffect( SceneRenderState *state )
{
   // Calculate mWaterPlane before calling updateUnderwaterEffect.
   Point3F dummy;
   _getWaterPlane( state->getCameraPosition(), mWaterFogData.plane, dummy );

   Parent::updateUnderwaterEffect( state );
}

void River::setShaderParams( SceneRenderState *state, BaseMatInstance* mat, const WaterMatParams& paramHandles )
{
   // Set variables that will be assigned to shader consts within WaterCommon
   // before calling Parent::setShaderParams

   mUndulateMaxDist = mLodDistance;

   Parent::setShaderParams( state, mat, paramHandles );   

   // Now set the rest of the shader consts that are either unique to this
   // class or that WaterObject leaves to us to handle...

   MaterialParameters* matParams = mat->getMaterialParameters();  

   // set vertex shader constants
   //-----------------------------------           

   matParams->setSafe(paramHandles.mGridElementSizeSC, 1.0f);
   if ( paramHandles.mModelMatSC->isValid() )
      matParams->set(paramHandles.mModelMatSC, MatrixF::Identity, GFXSCT_Float4x4);

   // set pixel shader constants
   //-----------------------------------

   ColorF c( mWaterFogData.color );
   matParams->setSafe(paramHandles.mBaseColorSC, c);

   // By default we need to show a true reflection is fullReflect is enabled and
   // we are above water.
   F32 reflect = mPlaneReflector.isEnabled() && !isUnderwater( state->getCameraPosition() );
   
   // If we were occluded the last frame a query was fetched ( not necessarily last frame )
   // and we weren't updated last frame... we don't have a valid texture to show
   // so use the cubemap / fake reflection color this frame.
   if ( mPlaneReflector.lastUpdateMs != REFLECTMGR->getLastUpdateMs() && mPlaneReflector.isOccluded() )
      reflect = false;

   Point4F reflectParams( mWaterPos.z, 0.0f, 1000.0f, !reflect );
   matParams->setSafe(paramHandles.mReflectParamsSC, reflectParams );

   matParams->setSafe(paramHandles.mReflectNormalSC, mPlaneReflector.refplane );   
}

bool River::isUnderwater( const Point3F &pnt ) const
{
   return containsPoint( pnt, NULL );
}

U32 River::packUpdate(NetConnection * con, U32 mask, BitStream * stream)
{  
   // Pack Parent.
   U32 retMask = Parent::packUpdate(con, mask, stream);

   if ( stream->writeFlag( mask & RiverMask ) )
   {
      // Write Object Transform.
      stream->writeAffineTransform(mObjToWorld);

      stream->write( mMetersPerSegment );      
      stream->write( mSegmentsPerBatch );
      stream->write( mDepthScale );
      stream->write( mMaxDivisionSize );
		stream->write( mColumnCount );

      stream->write( mFlowMagnitude );
      stream->write( mLodDistance );
   }   

   if ( stream->writeFlag( mask & NodeMask ) )
   {
      const U32 nodeByteSize = 32; // Based on sending all of a node's parameters

      // Test if we can fit all of our nodes within the current stream.
      // We make sure we leave 100 bytes still free in the stream for whatever
      // may follow us.
      S32 allowedBytes = stream->getWriteByteSize() - 100;
      if ( stream->writeFlag( (nodeByteSize * mNodes.size()) < allowedBytes ) )
      {
         // All nodes should fit, so send them out now.
         stream->writeInt( mNodes.size(), 16 );

         for ( U32 i = 0; i < mNodes.size(); i++ )
         {
            mathWrite( *stream, mNodes[i].point );
            stream->write( mNodes[i].width );
            stream->write( mNodes[i].depth );
            mathWrite( *stream, mNodes[i].normal );
         }
      }
      else
      {
         // There isn't enough space left in the stream for all of the
         // nodes.  Batch them up into NetEvents.
         U32 id = gServerNodeListManager->nextListId();
         U32 count = 0;
         U32 index = 0;
         while (count < mNodes.size())
         {
            count += NodeListManager::smMaximumNodesPerEvent;
            if (count > mNodes.size())
            {
               count = mNodes.size();
            }

            RiverNodeEvent* event = new RiverNodeEvent();
            event->mId = id;
            event->mTotalNodes = mNodes.size();
            event->mLocalListStart = index;

            for (; index<count; ++index)
            {
               event->mPositions.push_back( mNodes[index].point );
               event->mWidths.push_back( mNodes[index].width );
               event->mDepths.push_back( mNodes[index].depth );
               event->mNormals.push_back( mNodes[index].normal );
            }

            con->postNetEvent( event );
         }

         stream->write( id );
      }
   }
   
   if( stream->writeFlag( mask & ( RiverMask | InitialUpdateMask ) ) )
   {
      // This is set to allow the user to modify the size of the water dynamically
      // in the editor
      mathWrite( *stream, mObjScale );
      stream->writeAffineTransform( mObjToWorld );
   }

   stream->writeFlag( mask & RegenMask );

   return retMask;
}

void River::unpackUpdate(NetConnection * con, BitStream * stream)
{
   // Unpack Parent.
   Parent::unpackUpdate(con, stream);

   // RiverMask
   if(stream->readFlag())
   {
      MatrixF		ObjectMatrix;
      stream->readAffineTransform(&ObjectMatrix);
      Parent::setTransform(ObjectMatrix);
     
      stream->read( &mMetersPerSegment );    
      stream->read( &mSegmentsPerBatch );
      stream->read( &mDepthScale );
      stream->read( &mMaxDivisionSize );
		stream->read( &mColumnCount );

      stream->read( &mFlowMagnitude );
      stream->read( &mLodDistance );
   }

   // NodeMask
   if ( stream->readFlag() )
   {
      if (stream->readFlag())
      {
         // Nodes have been passed in this update
         U32 count = stream->readInt( 16 );

         mNodes.clear();

         Point3F pos;
         VectorF normal;
         F32 width,depth;

         for ( U32 i = 0; i < count; i++ )
         {
            mathRead( *stream, &pos );
            stream->read( &width );         
            stream->read( &depth );
            mathRead( *stream, &normal );
            _addNode( pos, width, depth, normal );         
         }
      }
      else
      {
         // Nodes will arrive as events
         U32 id;
         stream->read( &id );

         // Check if the road's nodes made it here before we did.
         NodeListManager::NodeList* list = NULL;
         if ( gClientNodeListManager->findListById( id, &list, true) )
         {
            // Work with the completed list
            RiverNodeList* riverList = dynamic_cast<RiverNodeList*>( list );
            if (riverList)
               buildNodesFromList( riverList );

            delete list;
         }
         else
         {
            // Nodes have not yet arrived, so register our interest in the list
            RiverNodeListNotify* notify = new RiverNodeListNotify( this, id );
            gClientNodeListManager->registerNotification( notify );
         }
      }
   }

   // RiverMask | InitialUpdateMask
   if( stream->readFlag() )
   {
      mathRead( *stream, &mObjScale );
      stream->readAffineTransform( &mObjToWorld );
   }

   // RegenMask
   if ( stream->readFlag() && isProperlyAdded() )  
      regenerate();
}

void River::_getWaterPlane( const Point3F &camPos, PlaneF &outPlane, Point3F &outPos )
{
   // Find the RiverSegment closest to the camera.   
   F32 closestDist = F32_MAX;
   S32 closestSegment = 0;
   Point3F projPnt(0.0f, 0.0f, 0.0f);

   VectorF normal(0,0,0);

   for ( U32 i = 0; i < mSegments.size(); i++ )
   {
      const RiverSegment &segment = mSegments[i];

      const Point3F pos = MathUtils::mClosestPointOnSegment( segment.slice0->p1, segment.slice1->p1, camPos );

      F32 dist = ( camPos - pos ).len();

      if ( dist < closestDist )
      {
         closestDist = dist;
         closestSegment = i;
         projPnt = pos;
      }

      normal += segment.getSurfaceNormal();
   }

   if ( mReflectNormalUp )
      normal.set(0,0,1);
   else
      normal.normalizeSafe();

   outPos = projPnt;
   outPlane.set( projPnt, normal );
}

void River::setTransform( const MatrixF &mat )
{
   
   for ( U32 i = 0; i < mNodes.size(); i++ )
   {
      mWorldToObj.mulP( mNodes[i].point );
      mat.mulP( mNodes[i].point );
   }

   /*
   // Get the amount of change in position.
   MatrixF oldMat = getTransform();
   Point3F oldPos = oldMat.getPosition();
   Point3F newPos = mat.getPosition();
   Point3F delta = newPos - oldPos;

   // Offset all nodes by that amount
   for ( U32 i = 0; i < mNodes.size(); i++ )
   {
      mNodes[i].point += delta;
   }

   // Assign the new position ( we ignore rotation )   
   MatrixF newMat( oldMat );
   newMat.setPosition( newPos );
   */
   
   Parent::setTransform( mat );

   // Regenerate and update the client
   _regenerate();
   setMaskBits( NodeMask | RegenMask );
}

void River::setScale( const VectorF &scale )
{
   // We ignore scale requests from the editor
   // right now.
}

bool River::castRay(const Point3F &s, const Point3F &e, RayInfo* info)
{
	Point3F start = s;
	Point3F end = e;
	mObjToWorld.mulP(start);
	mObjToWorld.mulP(end);

	F32 out = 1.0f;   // The output fraction/percentage along the line defined by s and e
	VectorF norm(0.0f, 0.0f, 0.0f);     // The normal of the face intersected

	Vector<RiverHitSegment> hitSegments;

	for ( U32 i = 0; i < mSegments.size(); i++ )
	{
		const RiverSegment &segment = mSegments[i];

		F32 t;
		VectorF n;

		if ( segment.worldbounds.collideLine( start, end, &t, &n ) )
		{
			hitSegments.increment();
			hitSegments.last().t = t;
			hitSegments.last().idx = i;         
		}
	}

	dQsort( hitSegments.address(), hitSegments.size(), sizeof(RiverHitSegment), compareHitSegments );

   U32 idx0, idx1, idx2;
   F32 t;

	for ( U32 i = 0; i < hitSegments.size(); i++ )
	{
		U32 segIdx = hitSegments[i].idx;
		const RiverSegment &segment = mSegments[segIdx];

		// Each segment has 6 faces
		for ( U32 j = 0; j < 6; j++ )
		{
			if ( j == 4 && segIdx != 0 )
				continue;

			if ( j == 5 && segIdx != mSegments.size() - 1 )
				continue;

			// Each face has 2 triangles
			for ( U32 k = 0; k < 2; k++ )
			{
				idx0 = gIdxArray[j][k][0];
				idx1 = gIdxArray[j][k][1];
				idx2 = gIdxArray[j][k][2];

            const Point3F &v0 = segment[idx0];
            const Point3F &v1 = segment[idx1];
            const Point3F &v2 = segment[idx2];

            if ( !MathUtils::mLineTriangleCollide( start, end, 
                                                   v2, v1, v0,
                                                   NULL,
                                                   &t ) )
					continue;

				if ( t >= 0.0f && t < 1.0f && t < out )
				{
					out = t;

               // optimize this, can be calculated easily within 
               // the collision test
               norm = PlaneF( v0, v1, v2 );
				}
			}
		}

		if (out >= 0.0f && out < 1.0f)
			break;
	}

	if (out >= 0.0f && out < 1.0f)
	{
		info->t = out;
		info->normal = norm;
		info->point.interpolate(start, end, out);
		info->face = -1;
		info->object = this;

		return true;
	}

	return false;
}

bool River::collideBox(const Point3F &start, const Point3F &end, RayInfo* info)
{
	return false;
}

F32 River::getWaterCoverage( const Box3F &worldBox ) const
{
   PROFILE_SCOPE( River_GetWaterCoverage );

   if ( !mWorldBox.isOverlapped(worldBox) )
      return 0.0f;

   Point3F bottomPnt = worldBox.getCenter();
   bottomPnt.z = worldBox.minExtents.z;

   F32 farthest = 0.0f;

   for ( U32 i = 0; i < mSegments.size(); i++ )
   {
      const RiverSegment &segment = mSegments[i];

      if ( !segment.worldbounds.isOverlapped(worldBox) )
         continue;

      if ( !segment.intersectBox( worldBox ) )
         continue;

      F32 distance = segment.distanceToSurface( bottomPnt );
     
      if ( distance > farthest )
         farthest = distance;
   }

   F32 height = worldBox.maxExtents.z - worldBox.minExtents.z;
   F32 distance = mClampF( farthest, 0.0f, height );
   F32 coverage = distance / height;

   return coverage;   
}

F32 River::getSurfaceHeight( const Point2F &pos ) const
{
   PROFILE_SCOPE( River_GetSurfaceHeight );

   Point3F origin( pos.x, pos.y, mWorldBox.maxExtents.z );
   Point3F direction(0,0,-1);
   U32 nodeIdx;
   Point3F collisionPnt;

   if ( !collideRay( origin, direction, &nodeIdx, &collisionPnt ) )
      return -1.0f;

   return collisionPnt.z;
}

VectorF River::getFlow( const Point3F &pos ) const
{
   PROFILE_SCOPE( River_GetFlow );

   for ( U32 i = 0; i < mSegments.size(); i++ )
   {
      const RiverSegment &segment = mSegments[i];

      if ( !segment.containsPoint(pos) )
         continue;

      VectorF flow = segment.slice0->p1 - segment.slice1->p1;
      flow.normalize();
      flow *= mFlowMagnitude;

      return flow;
   }

   return VectorF::Zero;
}

void River::onReflectionInfoChanged()
{
   /*
   if ( isClientObject() && GFX->getPixelShaderVersion() >= 1.4 )
   {
      if ( mFullReflect )
         REFLECTMGR->registerObject( this, ReflectDelegate( this, &River::updateReflection ), mReflectPriority, mReflectMaxRateMs, mReflectMaxDist );
      else
      {
         REFLECTMGR->unregisterObject( this );
         mReflectTex = NULL;
      }
   }
   */
}

void River::_regenerate()
{               
   if ( mNodes.size() == 0 )
      return;

   const Point3F &nodePt = mNodes.first().point;

   MatrixF mat( true );
   mat.setPosition( nodePt );
   Parent::setTransform( mat );

   _generateSlices();
}

void River::_generateSlices()
{      
   if ( mNodes.size() < 2 )
      return;

   U32 nodeCount = mNodes.size();
   RiverSplineNode *splineNodes = new RiverSplineNode[nodeCount];

   for ( U32 i = 0; i < nodeCount; i++ )
   {
      const RiverNode &node = mNodes[i];
      splineNodes[i].x = node.point.x;
      splineNodes[i].y = node.point.y;
      splineNodes[i].z = node.point.z;
      splineNodes[i].width = node.width;
      splineNodes[i].depth = node.depth;
      splineNodes[i].normal = node.normal;
   }

   CatmullRom<RiverSplineNode> spline;
   spline.initialize( nodeCount, splineNodes );
   delete [] splineNodes;

   mSlices.clear();

   for ( U32 i = 1; i < nodeCount; i++ )
   {
      F32 t0 = spline.getTime( i-1 );
      F32 t1 = spline.getTime( i );

      F32 segLength = spline.arcLength( t0, t1 );
    
      U32 numSegments = mCeil( segLength / mMetersPerSegment );
      numSegments = getMax( numSegments, (U32)1 );
      F32 tstep = ( t1 - t0 ) / numSegments;

      //AssertFatal( numSegments > 0, "River::_generateSlices, got zero segments!" );   

      U32 startIdx = 0;
      U32 endIdx = ( i == nodeCount - 1 ) ? numSegments + 1 : numSegments;

      for ( U32 j = startIdx; j < endIdx; j++ )
      {
         F32 t = t0 + tstep * j; //spline.findParameterByDistance( 0.0f, i * segLen );
         RiverSplineNode val = spline.evaluate(t);

         RiverSlice slice;
         slice.p1.set( val.x, val.y, val.z );
         slice.uvec.set( 0,0,1 );
         slice.width = val.width;
         slice.depth = val.depth;
         slice.parentNodeIdx = i-1;
         slice.normal = val.normal;
         slice.normal.normalize();
         mSlices.push_back( slice );
      }   
   }
   
   //
   // Calculate fvec and rvec for all slices
   //
   RiverSlice *pSlice = NULL;
   RiverSlice *pNextSlice = NULL;

   // Must do the first slice outside the loop
   {
      pSlice = &mSlices[0];
      pNextSlice = &mSlices[1];
      pSlice->fvec = pNextSlice->p1 - pSlice->p1;
      pSlice->fvec.normalize();
      pSlice->rvec = mCross( pSlice->fvec, pSlice->normal );
      pSlice->rvec.normalize();
      pSlice->uvec = mCross( pSlice->rvec, pSlice->fvec );            
      pSlice->uvec.normalize();
      pSlice->rvec = mCross( pSlice->fvec, pSlice->uvec );      
      pSlice->rvec.normalize();
   }

   for ( U32 i = 1; i < mSlices.size() - 1; i++ )
   {
      pSlice = &mSlices[i];
      pNextSlice = &mSlices[i+1];

      pSlice->fvec = pNextSlice->p1 - pSlice->p1;
      pSlice->fvec.normalize();

      pSlice->rvec = mCross( pSlice->fvec, pSlice->normal );
      pSlice->rvec.normalize();

      pSlice->uvec = mCross( pSlice->rvec, pSlice->fvec );      
      pSlice->uvec.normalize();

      pSlice->rvec = mCross( pSlice->fvec, pSlice->uvec );
      pSlice->rvec.normalize();
   }

   // Must do the last slice outside the loop
   {
      RiverSlice *lastSlice = &mSlices[mSlices.size()-1];
      RiverSlice *prevSlice = &mSlices[mSlices.size()-2];

      lastSlice->fvec = prevSlice->fvec;

      lastSlice->rvec = mCross( lastSlice->fvec, lastSlice->normal );
      lastSlice->rvec.normalize();

      lastSlice->uvec = mCross( lastSlice->rvec, lastSlice->fvec );      
      lastSlice->uvec.normalize();

      lastSlice->rvec = mCross( lastSlice->fvec, lastSlice->uvec );
      lastSlice->rvec.normalize();
   }


   //
   // Calculate p0/p2/pb0/pb2 for all slices
   //      
   for ( U32 i = 0; i < mSlices.size(); i++ )
   {
      RiverSlice *slice = &mSlices[i];
      slice->p0 = slice->p1 - slice->rvec * slice->width * 0.5f;
      slice->p2 = slice->p1 + slice->rvec * slice->width * 0.5f;
      slice->pb0 = slice->p0 - slice->uvec * slice->depth;
      slice->pb2 = slice->p2 - slice->uvec * slice->depth;
   }
   
   // Generate the object/world bounds
   Box3F box;
   for ( U32 i = 0; i < mSlices.size(); i++ )
   {
      const RiverSlice &slice = mSlices[i];

      if ( i == 0 )
      {
         box.minExtents = slice.p0;
         box.maxExtents = slice.p2;
         box.extend( slice.pb0 );
         box.extend( slice.pb2 );
      }
      else
      {
         box.extend( slice.p0 ); 
         box.extend( slice.p2 );
         box.extend( slice.pb0 );
         box.extend( slice.pb2 );
      }
   }

   Point3F pos = getPosition();

   mWorldBox = box;
   //mObjBox.minExtents -= pos;
   //mObjBox.maxExtents -= pos;
   resetObjectBox();

   // Make sure we are in the correct bins given our world box.
   if( getSceneManager() != NULL )
      getSceneManager()->notifyObjectDirty( this );

   _generateSegments();   
}

void River::_generateSegments()
{
   mSegments.clear();

   for ( U32 i = 0; i < mSlices.size() - 1; i++ )
   {
      RiverSegment seg( &mSlices[i], &mSlices[i+1] );
      
      mSegments.push_back( seg );
   }

   /*
   #ifdef TORQUE_DEBUG

      for ( U32 i = 0; i < mSegments.size(); i++ )
      {
         const RiverSegment &segment = mSegments[i];         
         PlaneF normal0 = MathUtils::mTriangleNormal( segment.slice0->p0, segment.slice1->p0, segment.slice1->p2 );
         PlaneF normal1 = MathUtils::mTriangleNormal( segment.slice0->p0, segment.slice1->p2, segment.slice0->p2 );
         AssertFatal( true || normal0 != normal1, "River::generateSegments, segment is not coplanar!" );
      }

   #endif // TORQUE_DEBUG
   */

   // We have to go back and generate normals for each slice
   // to be used in calculation of the reflect plane.  
   // The slice-normal we calculate are relative to the surface normal 
   // of the segments adjacent to the slice.
   /*
   if ( mSlices.size() >= 2 )
   {
      mSlices[0].normal = mSegments[0].getSurfaceNormal();
      for ( U32 i = 1; i < mSlices.size() - 1; i++ )
      {
         mSlices[i].normal = ( mSegments[i-1].getSurfaceNormal() + mSegments[i].getSurfaceNormal() ) / 2;
      }
      mSlices.last().normal = mSegments.last().getSurfaceNormal();
   }
   */
   
   _generateVerts();
}

void River::_generateVerts()
{           
   if ( isServerObject() )
      return;

   // These will depend on the level of subdivision per segment
   // calculated below.
   mHighVertCount = 0;
	mHighTriangleCount = 0;
	
   // Calculate the number of row/column subdivisions per each
   // RiverSegment.

   F32 greatestWidth = 0.1f;
   for ( U32 i = 0; i < mNodes.size(); i++ )
   {
      RiverNode &node = mNodes[i];
      if ( node.width > greatestWidth )
         greatestWidth = node.width;
   }

   mColumnCount = mCeil( greatestWidth / mMaxDivisionSize );

	for ( U32 i = 0; i < mSegments.size(); i++ )
	{
      RiverSegment &segment = mSegments[i];
      const RiverSlice *slice = segment.slice0;
		const RiverSlice *nextSlice = segment.slice1;

		// Calculate the size of divisions in the forward direction ( p00 -> p01 )      		
		F32 segLength = (nextSlice->p1 - slice->p1).len();		

		// A division count of one is actually NO subdivision,
		// the segment corners are the only verts in this segment.
		U32 numRows = 1;

      if ( segLength > 0.0f )
         numRows = mCeil( segLength / mMaxDivisionSize );

      // The problem with calculating num columns per segment is 
      // two adjacent - high lod segments of different width can have
      // verts that don't line up!  So even though RiverSegment HAS a 
      // column data member we initialize all segments in the river to
      // the same (River::mColumnCount)

		// Calculate the size of divisions in the right direction ( p00 -> p10 ) 
		// F32 segWidth = ( ( p11 - p01 ).len() + ( p10 - p00 ).len() ) * 0.5f;

		// U32 numColumns = 5;
		//F32 columnSize = segWidth / numColumns;

		//while ( columnSize > mMaxDivisionSize )
		//{
		//	numColumns++;
		//	columnSize = segWidth / numColumns;
		//}
		
      // Save the calculated numb of columns / rows for this segment.
      segment.columns = mColumnCount;
      segment.rows = numRows;
		
      // Save the corresponding number of verts/prims
      segment.numVerts = ( 1 + mColumnCount ) * ( 1 + numRows );
      segment.numTriangles = mColumnCount * numRows * 2;

		mHighVertCount += segment.numVerts;
		mHighTriangleCount += segment.numTriangles;
	}

   // Number of low detail verts/prims.
   mLowVertCount = mSlices.size() * 2;	
	mLowTriangleCount = mSegments.size() * 2;   

   // Allocate the low detail VertexBuffer, 
   // this will stay in memory and will never need to change.
   mVB_low.set( GFX, mLowVertCount, GFXBufferTypeStatic );   
   
   GFXWaterVertex *lowVertPtr = mVB_low.lock(); 
   U32 vertCounter = 0;

	// The texCoord.y value start/end for a segment
	// as we loop through them.	
   F32 textCoordV = 0;

   //
   // Fill the low-detail VertexBuffer
   //
   for ( U32 i = 0; i < mSlices.size(); i++ )
   {
      RiverSlice &slice = mSlices[i];      

      lowVertPtr->point = slice.p0;    
      lowVertPtr->normal = slice.normal;      
      lowVertPtr->undulateData.set( -slice.width*0.5f, textCoordV );   
      lowVertPtr->horizonFactor.set( 0, 0, 0, 0 );
      lowVertPtr++;
      vertCounter++;

      lowVertPtr->point = slice.p2;
      lowVertPtr->normal = slice.normal;
      lowVertPtr->undulateData.set( slice.width*0.5f, textCoordV );     
      lowVertPtr->horizonFactor.set( 0, 0, 0, 0 );
      lowVertPtr++;
      vertCounter++;

      // Save this so we can get it later.
      slice.texCoordV = textCoordV;

      if ( i < mSlices.size() - 1 )
      {         
         // Increment the textCoordV for the next slice.
         F32 segLen = ( mSlices[i+1].p1 - slice.p1 ).len();
		   textCoordV += segLen;         
      }
   }

   AssertFatal( vertCounter == mLowVertCount, "River, wrote incorrect number of verts in mBV_low!" );
   
   // Unlock the low-detail VertexBuffer, we are done filling it.
   mVB_low.unlock();

   //
   // Create the low-detail prim buffer(s)
   //	
	mPB_low.set( GFX, mLowTriangleCount * 3, mLowTriangleCount, GFXBufferTypeStatic );

   U16 *lowIdxBuff;
   mPB_low.lock(&lowIdxBuff);     
   U32 curLowIdx = 0; 

   // Temporaries to hold indices for the corner points of a quad.
   U32 p00, p01, p11, p10;

   U32 offset = 0;

   // Fill the low-detail PrimitiveBuffer   
	for ( U32 i = 0; i < mSegments.size(); i++ )
	{		
      //const RiverSegment &segment = mSegments[i];
		
      // Two triangles formed by the corner points of this segment
      // into the the low detail primitive buffer.
		p00 = offset;
      p01 = p00 + 2;
      p11 = p01 + 1;
      p10 = p00 + 1;

      // Upper-Left triangle
      lowIdxBuff[curLowIdx] = p00;
      curLowIdx++;
      lowIdxBuff[curLowIdx] = p01;
      curLowIdx++;
      lowIdxBuff[curLowIdx] = p11;
      curLowIdx++;

      // Lower-Right Triangle
      lowIdxBuff[curLowIdx] = p00;
      curLowIdx++;
      lowIdxBuff[curLowIdx] = p11;
      curLowIdx++;
      lowIdxBuff[curLowIdx] = p10;
      curLowIdx++;      

      offset += 2;
   }

   AssertFatal( curLowIdx == mLowTriangleCount * 3, "River, wrote incorrect number of indices in mPB_low!" );

   // Unlock the low-detail PrimitiveBuffer, we are done filling it.
   mPB_low.unlock();
}

bool River::getClosestNode( const Point3F &pos, U32 &idx ) const
{
   F32 closestDist = F32_MAX;

   for ( U32 i = 0; i < mNodes.size(); i++ )
   {
      F32 dist = ( mNodes[i].point - pos ).len();
      if ( dist < closestDist )
      {
         closestDist = dist;
         idx = i;
      }      
   }

   return closestDist != F32_MAX;
}

bool River::containsPoint( const Point3F &worldPos, U32 *nodeIdx ) const
{
   // If point isn't in the world box, 
   // it's definitely not in the River.
   //if ( !getWorldBox().isContained( worldPos ) )
   //   return false;   

   // Look through all edges, does the polygon
   // formed from adjacent edge's contain the worldPos?
   for ( U32 i = 0; i < mSegments.size(); i++ )
   {
      const RiverSegment &segment = mSegments[i];

      if ( segment.containsPoint( worldPos ) )
      {
         if ( nodeIdx )
            *nodeIdx = i;
         return true;
      }      
   }

   return false;
}

F32 River::distanceToSurface( const Point3F &pnt, U32 segmentIdx )
{
   return mSegments[segmentIdx].distanceToSurface( pnt );
}

bool River::collideRay( const Point3F &origin, const Point3F &direction, U32 *nodeIdx, Point3F *collisionPnt ) const
{
   Point3F p0 = origin;
   Point3F p1 = origin + direction * 2000.0f;

   // If the line segment does not collide with the river's world box, 
   // it definitely does not collide with any part of the river.
   if ( !getWorldBox().collideLine( p0, p1 ) )
      return false;
   
   if ( mSlices.size() < 2 )
      return false;

   MathUtils::Quad quad;
   MathUtils::Ray ray;
   F32 t;

   // Check each river segment (formed by a pair of slices) for collision
   // with the line segment.
   for ( U32 i = 0; i < mSlices.size() - 1; i++ )
   {
      const RiverSlice &slice0 = mSlices[i];
      const RiverSlice &slice1 = mSlices[i+1];

      // For simplicities sake we will only test for collision between the
      // line segment and the Top face of the river segment.

      // Clockwise starting with the leftmost/closest point.
      quad.p00 = slice0.p0;
      quad.p01 = slice1.p0;
      quad.p11 = slice1.p2;
      quad.p10 = slice0.p2;
      
      ray.origin = origin;
      ray.direction = direction;

      // NOTE: 
      // mRayQuadCollide is designed for a "real" quad in which all four points
      // are coplanar which is actually not the case here. The more twist 
      // and turn in-between two neighboring river slices the more incorrect 
      // this calculation will be.

      if ( MathUtils::mRayQuadCollide( quad, ray, NULL, &t ) )
      {
         if ( nodeIdx )
            *nodeIdx = slice0.parentNodeIdx;         
         if ( collisionPnt )
            *collisionPnt = ray.origin + ray.direction * t;
         return true;
      }
   }

   return false;
}

Point3F River::getNodePosition( U32 idx ) const
{
   if ( mNodes.size() - 1 < idx )
      return Point3F();

   return mNodes[idx].point;
}

void River::setNodePosition( U32 idx, const Point3F &pos )
{
   if ( mNodes.size() - 1 < idx )
      return;

   mNodes[idx].point = pos;

   regenerate();

   setMaskBits( NodeMask | RegenMask );
}

U32 River::addNode( const Point3F &pos, const F32 &width, const F32 &depth, const VectorF &normal )
{
   U32 idx = _addNode( pos, width, depth, normal );   

   regenerate();

   setMaskBits( NodeMask | RegenMask );

   return idx;
}

U32 River::insertNode(const Point3F &pos, const F32 &width, const F32 &depth, const VectorF &normal, const U32 &idx)
{
   U32 ret = _insertNode( pos, width, depth, normal, idx );

   regenerate();

   setMaskBits( NodeMask | RegenMask );

   return ret;
}

void River::setNode(const Point3F &pos, const F32 &width, const F32 &depth, const VectorF &normal, const U32 &idx)
{
   if ( mNodes.size() - 1 < idx )
      return;

   RiverNode &node = mNodes[idx];
   node.point = pos;   
   node.width = width;
   node.depth = depth;
   node.normal = normal;

   regenerate();

   setMaskBits( NodeMask | RegenMask );
}

void River::setNodeWidth( U32 idx, F32 meters )
{
   meters = mClampF( meters, MIN_NODE_WIDTH, MAX_NODE_WIDTH );

   if ( mNodes.size() - 1 < idx )
      return;

   mNodes[idx].width = meters;
   _regenerate();

   setMaskBits( RegenMask | NodeMask );
}

void River::setNodeHeight( U32 idx, F32 height )
{
   if ( mNodes.size() - 1 < idx )
      return;

   mNodes[idx].point.z = height;
   _regenerate();

   setMaskBits( RegenMask | NodeMask );
}

F32 River::getNodeWidth( U32 idx ) const
{
   if ( mNodes.size() - 1 < idx )
      return -1.0f;

   return mNodes[idx].width;
}

void River::setNodeDepth( U32 idx, F32 meters )
{
   meters = mClampF( meters, MIN_NODE_DEPTH, MAX_NODE_DEPTH );
   
   if ( mNodes.size() - 1 < idx )
      return;

   mNodes[idx].depth = meters;
   _regenerate();
   setMaskBits( RiverMask | RegenMask | NodeMask );
}

void River::setNodeNormal( U32 idx, const VectorF &normal )
{
   if ( mNodes.size() - 1 < idx )
      return;

   mNodes[idx].normal = normal;

   regenerate();

   setMaskBits( NodeMask | RegenMask );
}

F32 River::getNodeDepth( U32 idx ) const
{
   if ( mNodes.size() - 1 < idx )
      return -1.0f;

   return mNodes[idx].depth;
}

VectorF River::getNodeNormal( U32 idx ) const
{
   if ( mNodes.size() - 1 < idx )
      return VectorF::Zero;

   return mNodes[idx].normal;
}

MatrixF River::getNodeTransform( U32 idx ) const
{   
   MatrixF mat(true);   

   if ( mNodes.size() - 1 < idx )
      return mat;

   bool hasNext = idx + 1 < mNodes.size();
   bool hasPrev = (S32)idx - 1 >= 0;

   const RiverNode &node = mNodes[idx];   

   VectorF fvec( 0, 1, 0 );

   if ( hasNext )
   {
      fvec = mNodes[idx+1].point - node.point;      
      fvec.normalizeSafe();
   }
   else if ( hasPrev )
   {
      fvec = node.point - mNodes[idx-1].point;
      fvec.normalizeSafe();
   }
   else
      fvec = mPerp( node.normal );
   
   if ( fvec.isZero() )
      fvec = mPerp( node.normal );

   F32 dot = mDot( fvec, node.normal );
   if ( dot < -0.9f || dot > 0.9f )
      fvec = mPerp( node.normal );

   VectorF rvec = mCross( fvec, node.normal );
   if ( rvec.isZero() )
      rvec = mPerp( fvec );
   rvec.normalize();

   fvec = mCross( node.normal, rvec );
   fvec.normalize();

   mat.setColumn( 0, rvec );
   mat.setColumn( 1, fvec );
   mat.setColumn( 2, node.normal );
   mat.setColumn( 3, node.point );

   AssertFatal( m_matF_determinant( mat ) != 0.0f, "no inverse!");

   return mat; 
}

void River::deleteNode( U32 idx )
{
   if ( mNodes.size() - 1 < idx )
      return;

   mNodes.erase(idx);   
   _regenerate();

   setMaskBits( RegenMask | NodeMask );
}

void River::buildNodesFromList( RiverNodeList* list )
{
   mNodes.clear();

   for (U32 i=0; i<list->mPositions.size(); ++i)
   {
      _addNode( list->mPositions[i], list->mWidths[i], list->mDepths[i], list->mNormals[i] );
   }

   _regenerate();
}

void River::_makeRenderBatches( const Point3F &cameraPos )
{
   // Loop through each segment to determine if it is either 1 [not visible], 2 [high LOD], 3 [low LOD]

   mHighLODBatches.clear();
   mLowLODBatches.clear();      

   // Keeps track of what we batch type we are currently collecting.
   // -1 is uninitialized, 0 is low detail, 1 is high detail
   S32 lastDetail = -1;
   bool highDetail;

   U32 startSegmentIdx = -1;
   U32 endSegmentIdx = 0;

   F32 lodDistSquared = mLodDistance * mLodDistance;

   for ( U32 i = 0; i < mSegments.size(); i++ )   
   {
      const RiverSegment &segment = mSegments[i];
      const RiverSlice *slice = segment.slice0;
      const RiverSlice *nextSlice = segment.slice1;

      // TODO: add bounds BoxF to RiverSegment
      const bool isVisible = true; //frustum.intersects( segment.bounds );
      if ( isVisible )
      {
         F32 dist0 = MathUtils::mTriangleDistance( slice->p0, nextSlice->p0, nextSlice->p2, cameraPos );
         F32 dist1 = MathUtils::mTriangleDistance( slice->p0, nextSlice->p2, slice->p2, cameraPos );

         F32 dist = getMin( dist0, dist1 );
         highDetail = ( dist < lodDistSquared );
         if ( highDetail && lastDetail == 0 ||
            !highDetail && lastDetail == 1 )
         {
            // We hit a segment with a different lod than the previous.
            // Save what we have so far...

            RiverRenderBatch batch;

            batch.startSegmentIdx = startSegmentIdx;
            batch.endSegmentIdx = endSegmentIdx;               

            if ( lastDetail == 0 )
            {
               mLowLODBatches.push_back( batch );
            }
            else
            {
               mHighLODBatches.push_back( batch );
            }

            // Reset the batching
            startSegmentIdx = -1;
            lastDetail = -1;
            i--;

            continue;
         }

         // If this is the start of a set of batches.
         if ( startSegmentIdx == -1 )
         {
            endSegmentIdx = startSegmentIdx = i;
            lastDetail = ( highDetail ) ? 1 : 0;
         }

         // Else we're extending the end batch index.
         else
            ++endSegmentIdx; 

         // If this isn't the last batch then continue.
         if ( i < mSegments.size()-1 )
            continue;
      }

      // If we still don't have a start batch skip.
      if ( startSegmentIdx == -1 )
         continue;

      // Save what we have so far...

      RiverRenderBatch batch;

      batch.startSegmentIdx = startSegmentIdx;
      batch.endSegmentIdx = endSegmentIdx;               

      if ( lastDetail == 0 )
      {
         mLowLODBatches.push_back( batch );
      }
      else
      {
         mHighLODBatches.push_back( batch );
      }

      // Reset the batching.
      startSegmentIdx = -1;
      lastDetail = -1;
   }   
}

void River::_makeHighLODBuffers()
{
   PROFILE_SCOPE( River_makeHighLODBuffers );

   // This is the number of verts/triangles for ALL high lod batches combined.
   // eg. the size for the buffers.
   U32 numVerts = 0;
   U32 numTriangles = 0;

   for ( U32 i = 0; i < mHighLODBatches.size(); i++ )
   {
      RiverRenderBatch &batch = mHighLODBatches[i];

      for ( U32 j = batch.startSegmentIdx; j <= batch.endSegmentIdx; j++ )
      {
         const RiverSegment &segment = mSegments[j];

         numTriangles += segment.numTriangles;
         numVerts += segment.numVerts;
      }
   }

   if ( numVerts > MAX_DYNAMIC_VERTS || numTriangles * 3 > MAX_DYNAMIC_INDICES )
   {
      mVB_high = NULL;
      mPB_high = NULL;
      return;
   }

   mHighTriangleCount = numTriangles;
   mHighVertCount = numVerts;

   mVB_high.set( GFX, numVerts, GFXBufferTypeVolatile );
   GFXWaterVertex *vertPtr = mVB_high.lock();   
   U32 vertCounter = 0;

   // NOTE: this will break if different segments have different number
   // of columns, but that will also cause T-junction triangles so just don't
   // do that.

   // For each batch, loop through the segments contained by
   // that batch, and add their verts to the buffer.
   for ( U32 i = 0; i < mHighLODBatches.size(); i++ )
   {
      RiverRenderBatch &batch = mHighLODBatches[i];                        

      batch.startVert = vertCounter;
      batch.vertCount = 0;

      VectorF lastNormal(0,0,1);

      for ( U32 j = batch.startSegmentIdx; j <= batch.endSegmentIdx; j++ )
      {
         // Add the verts for this segment to the buffer.
         RiverSegment &segment = mSegments[j];

         BiSqrToQuad3D squareToQuad( segment.getP00(), 
                                     segment.getP10(), 
                                     segment.getP11(), 
                                     segment.getP01() );

         // We are duplicating the last row of verts in a segment on
         // the first row of the next segment.  This could be optimized but 
         // shouldn't cause any problems.

         VectorF normal = segment.getSurfaceNormal();

         for ( U32 k = 0; k <= segment.rows; k++ )
         {
            VectorF vertNormal = ( k == 0 && j != batch.startSegmentIdx ) ? lastNormal : normal;    

            F32 rowLen = mLerp( segment.slice0->width, segment.slice1->width, (F32)k / (F32)segment.rows );

            for ( U32 l = 0; l <= segment.columns; l++ )
            {                                    
               // We are generating a "row" of verts along the forwardDivision
               // Each l iteration is a step to the right along with row.

               Point2F uv( (F32)l / (F32)segment.columns, (F32)k / (F32)segment.rows );

               Point3F pnt = squareToQuad.transform( uv );

               // Assign the Vert
               vertPtr->point = pnt;            
               vertPtr->normal = vertNormal;
               vertPtr->undulateData.x = ( uv.x - 0.5f ) * rowLen;
               vertPtr->undulateData.y = ( segment.TexCoordEnd() - segment.TexCoordStart() ) * uv.y + segment.TexCoordStart();
               vertPtr->horizonFactor.set( 0, 0, 0, 0 );

               vertPtr++;
               vertCounter++;
               batch.vertCount++;                     
            }
         }

         lastNormal = normal;
      }
   }

   AssertFatal( vertCounter == mHighVertCount, "River, wrote incorrect number of verts in mVB_high" );

   mVB_high.unlock();

   //
   // Do the high lod primitive buffer.         
   //

   mPB_high.set( GFX, numTriangles * 3, numTriangles, GFXBufferTypeVolatile );         
   U16 *idxBuff;
   mPB_high.lock(&idxBuff);     
   U32 curIdx = 0; 

   U32 batchOffset = 0;         

   // For each high lod batch, we must add indices to the buffer
   // for each segment it contains ( and the count will depend on
   // the division level columns/rows for each segment ).

   // Temporaries for holding the indices of a quad
   U32 p00, p01, p11, p10;

   for ( U32 i = 0; i < mHighLODBatches.size(); i++ )
   {
      RiverRenderBatch &batch = mHighLODBatches[i];            

      batch.indexCount = 0;
      batch.triangleCount = 0;
      batch.startIndex = curIdx;

      U32 temp = 0;
      U32 segmentOffset = 0;            

      for ( U32 j = batch.startSegmentIdx; j <= batch.endSegmentIdx; j++ )
      {               
         const RiverSegment &segment = mSegments[j];               

         // Loop through all divisions adding the indices to the 
         // high detail primitive buffer.
         for ( U32 k = 0; k < segment.rows; k++ )
         {         
            for ( U32 l = 0; l < segment.columns; l++ )
            {                     
               // The indices for this quad.
               p00 = batchOffset + segmentOffset + l + k * ( segment.columns + 1 );
               p01 = p00 + segment.columns + 1;
               p11 = p01 + 1;
               p10 = p00 + 1;

               AssertFatal( p00 <= mHighTriangleCount * 3, "River, bad draw call!" );
               AssertFatal( p01 <= mHighTriangleCount * 3, "River, bad draw call!" );
               AssertFatal( p11 <= mHighTriangleCount * 3, "River, bad draw call!" );
               AssertFatal( p10 <= mHighTriangleCount * 3, "River, bad draw call!" );

               // Upper-Left triangle
               idxBuff[curIdx] = p00;
               curIdx++;
               idxBuff[curIdx] = p01;
               curIdx++;
               idxBuff[curIdx] = p11;
               curIdx++;

               // Lower-Right Triangle
               idxBuff[curIdx] = p00;
               curIdx++;
               idxBuff[curIdx] = p11;
               curIdx++;
               idxBuff[curIdx] = p10;
               curIdx++;                     

               batch.indexCount += 6;
               batch.triangleCount += 2;
            }
         }

         // Increment the sliceOffset by the number of verts 
         // used by this segment.  So the next segment will index
         // into new verts.
         segmentOffset += ( segment.columns + 1 ) * ( segment.rows + 1 );                              
         temp += ( segment.columns + 1 ) * ( segment.rows + 1 );
      }            

      batchOffset += temp;
   }

   // Unlock the PrimitiveBuffer, we are done filling it.
   mPB_high.unlock();
}

U32 River::_addNode( const Point3F &pos, const F32 &width, const F32 &depth, const VectorF &normal )
{
   mNodes.increment();
   RiverNode &node = mNodes.last();

   node.point = pos;   
   node.width = width;
   node.depth = depth;
   node.normal = normal;

   setMaskBits( NodeMask | RegenMask );

   return mNodes.size() - 1;
}

U32 River::_insertNode( const Point3F &pos, const F32 &width, const F32 &depth, const VectorF &normal, const U32 &idx )
{
   U32 ret;
   RiverNode *node;

   if ( idx == U32_MAX )
   {
      mNodes.increment();
      node = &mNodes.last();
      ret = mNodes.size() - 1;
   }
   else
   {
      mNodes.insert( idx );
      node = &mNodes[idx];
      ret = idx;
   }

   node->point = pos;
   node->depth = depth;
   node->width = width;     
   node->normal = normal;

   return ret;
}

void River::setMetersPerSegment( F32 meters )
{
   if ( meters < MIN_METERS_PER_SEGMENT )
   {
      Con::warnf( "River::setMetersPerSegment, specified meters (%g) is below the min meters (%g), NOT SET!", meters, MIN_METERS_PER_SEGMENT );
      return;
   }

   mMetersPerSegment = meters;
   _regenerate();
   setMaskBits( RiverMask | RegenMask );
}

void River::setBatchSize( U32 size )
{
   // Not functional
   //mSegmentsPerBatch = size;
   //_regenerate();
   //setMaskBits( RiverMask | RegenMask );
}

void River::regenerate()
{
   _regenerate();
   setMaskBits( RegenMask );
}

void River::setMaxDivisionSize( F32 meters )
{
   if ( meters < mMinDivisionSize )
      mMaxDivisionSize = mMinDivisionSize;
   else
      mMaxDivisionSize = meters;      

   _regenerate();
   setMaskBits( RiverMask | RegenMask );
}

//-------------------------------------------------------------------------
// Console Methods
//-------------------------------------------------------------------------

DefineEngineMethod( River, regenerate, void, (),,
                   "Intended as a helper to developers and editor scripts.\n"
                   "Force River to recreate its geometry."
                   )
{
   object->regenerate();
}

DefineEngineMethod( River, setMetersPerSegment, void, ( F32 meters ),,
                   "Intended as a helper to developers and editor scripts.\n"
                   "@see SegmentLength field."
                   )
{
   object->setMetersPerSegment( meters );
}

DefineEngineMethod( River, setBatchSize, void, ( F32 meters ),,
                   "Intended as a helper to developers and editor scripts.\n"
                   "BatchSize is not currently used."
                   )
{
   object->setBatchSize( meters );
}

DefineEngineMethod( River, setNodeDepth, void, ( S32 idx, F32 meters ),,
                   "Intended as a helper to developers and editor scripts.\n"
                   "Sets the depth in meters of a particular node."
                   )
{
   object->setNodeDepth( idx, meters );
}

DefineEngineMethod( River, setMaxDivisionSize, void, ( F32 meters ),,
                   "Intended as a helper to developers and editor scripts.\n"
                   "@see SubdivideLength field."
                   )
{
   object->setMaxDivisionSize( meters );
}
