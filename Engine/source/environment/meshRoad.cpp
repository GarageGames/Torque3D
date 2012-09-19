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
#include "environment/meshRoad.h"

#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "util/catmullRom.h"
#include "math/util/quadTransforms.h"
#include "scene/simPath.h"
#include "scene/sceneRenderState.h"
#include "scene/sceneManager.h"
#include "scene/sgUtil.h"
#include "renderInstance/renderPassManager.h"
#include "T3D/gameBase/gameConnection.h"
#include "core/stream/bitStream.h"
#include "gfx/gfxDrawUtil.h"
#include "gfx/gfxTransformSaver.h"
#include "gfx/primBuilder.h"
#include "gfx/gfxDebugEvent.h"
#include "materials/materialManager.h"
#include "math/mathIO.h"
#include "math/mathUtils.h"
#include "math/util/frustum.h"
#include "gui/3d/guiTSControl.h"
#include "materials/shaderData.h"
#include "gfx/sim/gfxStateBlockData.h"
#include "gfx/sim/debugDraw.h"
#include "collision/concretePolyList.h"
#include "T3D/physics/physicsPlugin.h"
#include "T3D/physics/physicsBody.h"
#include "environment/nodeListManager.h"

#define MIN_METERS_PER_SEGMENT 1.0f
#define MIN_NODE_DEPTH 0.25f
#define MAX_NODE_DEPTH 50.0f
#define MIN_NODE_WIDTH 0.25f
#define MAX_NODE_WIDTH 50.0f


static U32 gIdxArray[6][2][3] = {
   { { 0, 4, 5 }, { 0, 5, 1 }, },   // Top Face
   { { 2, 6, 4 }, { 2, 4, 0 }, },   // Left Face
   { { 1, 5, 7 }, { 1, 7, 3 }, },   // Right Face
   { { 2, 3, 7 }, { 2, 7, 6 }, },   // Bottom Face
   { { 0, 1, 3 }, { 0, 3, 2 }, },   // Front Face
   { { 4, 6, 7 }, { 4, 7, 5 }, },   // Back Face
};

static S32 QSORT_CALLBACK compareHitSegments(const void* a,const void* b)
{
   const MeshRoadHitSegment *fa = (MeshRoadHitSegment*)a;
   const MeshRoadHitSegment *fb = (MeshRoadHitSegment*)b;

   return mSign(fb->t - fa->t);
}


//-----------------------------------------------------------------------------
// MeshRoadNodeList Struct
//-----------------------------------------------------------------------------

struct MeshRoadNodeList : public NodeListManager::NodeList
{
   Vector<Point3F>   mPositions;
   Vector<F32>       mWidths;
   Vector<F32>       mDepths;
   Vector<VectorF>   mNormals;

   MeshRoadNodeList() { }
   virtual ~MeshRoadNodeList() { }
};

//-----------------------------------------------------------------------------
// MeshRoadNodeEvent Class
//-----------------------------------------------------------------------------

class MeshRoadNodeEvent : public NodeListEvent
{
   typedef NodeListEvent Parent;

public:
   Vector<Point3F>   mPositions;
   Vector<F32>       mWidths;
   Vector<F32>       mDepths;
   Vector<VectorF>   mNormals;

public:
   MeshRoadNodeEvent() { mNodeList = NULL; }
   virtual ~MeshRoadNodeEvent() { }

   virtual void pack(NetConnection*, BitStream*);
   virtual void unpack(NetConnection*, BitStream*);

   virtual void copyIntoList(NodeListManager::NodeList* copyInto);
   virtual void padListToSize();

   DECLARE_CONOBJECT(MeshRoadNodeEvent);
};

void MeshRoadNodeEvent::pack(NetConnection* conn, BitStream* stream)
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

void MeshRoadNodeEvent::unpack(NetConnection* conn, BitStream* stream)
{
   mNodeList = new MeshRoadNodeList();

   Parent::unpack( conn, stream );

   U32 count = stream->readInt( 16 );

   Point3F pos;
   F32 width, depth;
   VectorF normal;

   MeshRoadNodeList* list = static_cast<MeshRoadNodeList*>(mNodeList);

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

void MeshRoadNodeEvent::copyIntoList(NodeListManager::NodeList* copyInto)
{
   MeshRoadNodeList* prevList = dynamic_cast<MeshRoadNodeList*>(copyInto);
   MeshRoadNodeList* list = static_cast<MeshRoadNodeList*>(mNodeList);

   // Merge our list with the old list.
   for (U32 i=mLocalListStart, index=0; i<mLocalListStart+list->mPositions.size(); ++i, ++index)
   {
      prevList->mPositions[i] = list->mPositions[index];
      prevList->mWidths[i] = list->mWidths[index];
      prevList->mDepths[i] = list->mDepths[index];
      prevList->mNormals[i] = list->mNormals[index];
   }
}

void MeshRoadNodeEvent::padListToSize()
{
   MeshRoadNodeList* list = static_cast<MeshRoadNodeList*>(mNodeList);

   U32 totalValidNodes = list->mTotalValidNodes;

   // Pad our list front?
   if (mLocalListStart)
   {
      MeshRoadNodeList* newlist = new MeshRoadNodeList();
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

IMPLEMENT_CO_NETEVENT_V1(MeshRoadNodeEvent);

ConsoleDocClass( MeshRoadNodeEvent,
   "@brief Sends messages to the Mesh Road Editor\n\n"
   "Editor use only.\n\n"
   "@internal"
);

//-----------------------------------------------------------------------------
// MeshRoadNodeListNotify Class
//-----------------------------------------------------------------------------

class MeshRoadNodeListNotify : public NodeListNotify
{
   typedef NodeListNotify Parent;

protected:
   SimObjectPtr<MeshRoad> mRoad;

public:
   MeshRoadNodeListNotify( MeshRoad* road, U32 listId ) { mRoad = road; mListId = listId; }
   virtual ~MeshRoadNodeListNotify() { mRoad = NULL; }

   virtual void sendNotification( NodeListManager::NodeList* list );
};

void MeshRoadNodeListNotify::sendNotification( NodeListManager::NodeList* list )
{
   if (mRoad.isValid())
   {
      // Build the road's nodes
      MeshRoadNodeList* roadList = dynamic_cast<MeshRoadNodeList*>( list );
      if (roadList)
         mRoad->buildNodesFromList( roadList );
   }
}

//------------------------------------------------------------------------------
// MeshRoadConvex Class
//------------------------------------------------------------------------------

const MatrixF& MeshRoadConvex::getTransform() const
{
   return MatrixF::Identity; //mObject->getTransform();    
}

Box3F MeshRoadConvex::getBoundingBox() const
{
   return box;
}

Box3F MeshRoadConvex::getBoundingBox(const MatrixF& mat, const Point3F& scale) const
{
   Box3F newBox = box;
   newBox.minExtents.convolve(scale);
   newBox.maxExtents.convolve(scale);
   mat.mul(newBox);
   return newBox;
}

Point3F MeshRoadConvex::support(const VectorF& vec) const
{
   F32 bestDot = mDot( verts[0], vec );

   const Point3F *bestP = &verts[0];
   for(S32 i=1; i<4; i++)
   {
      F32 newD = mDot(verts[i], vec);
      if(newD > bestDot)
      {
         bestDot = newD;
         bestP = &verts[i];
      }
   }

   return *bestP;
}

void MeshRoadConvex::getFeatures(const MatrixF& mat, const VectorF& n, ConvexFeature* cf)
{
   cf->material = 0;
   cf->object = mObject;

   // For a tetrahedron this is pretty easy... first
   // convert everything into world space.
   Point3F tverts[4];
   mat.mulP(verts[0], &tverts[0]);
   mat.mulP(verts[1], &tverts[1]);
   mat.mulP(verts[2], &tverts[2]);
   mat.mulP(verts[3], &tverts[3]);

   // Points...
   S32 firstVert = cf->mVertexList.size();
   cf->mVertexList.increment(); cf->mVertexList.last() = tverts[0];
   cf->mVertexList.increment(); cf->mVertexList.last() = tverts[1];
   cf->mVertexList.increment(); cf->mVertexList.last() = tverts[2];
   cf->mVertexList.increment(); cf->mVertexList.last() = tverts[3];

   // Edges...
   cf->mEdgeList.increment();
   cf->mEdgeList.last().vertex[0] = firstVert+0;
   cf->mEdgeList.last().vertex[1] = firstVert+1;

   cf->mEdgeList.increment();
   cf->mEdgeList.last().vertex[0] = firstVert+1;
   cf->mEdgeList.last().vertex[1] = firstVert+2;

   cf->mEdgeList.increment();
   cf->mEdgeList.last().vertex[0] = firstVert+2;
   cf->mEdgeList.last().vertex[1] = firstVert+0;

   cf->mEdgeList.increment();
   cf->mEdgeList.last().vertex[0] = firstVert+3;
   cf->mEdgeList.last().vertex[1] = firstVert+0;

   cf->mEdgeList.increment();
   cf->mEdgeList.last().vertex[0] = firstVert+3;
   cf->mEdgeList.last().vertex[1] = firstVert+1;

   cf->mEdgeList.increment();
   cf->mEdgeList.last().vertex[0] = firstVert+3;
   cf->mEdgeList.last().vertex[1] = firstVert+2;

   // Triangles...
   cf->mFaceList.increment();
   cf->mFaceList.last().normal = PlaneF(tverts[2], tverts[1], tverts[0]);
   cf->mFaceList.last().vertex[0] = firstVert+2;
   cf->mFaceList.last().vertex[1] = firstVert+1;
   cf->mFaceList.last().vertex[2] = firstVert+0;

   cf->mFaceList.increment();
   cf->mFaceList.last().normal = PlaneF(tverts[1], tverts[0], tverts[3]);
   cf->mFaceList.last().vertex[0] = firstVert+1;
   cf->mFaceList.last().vertex[1] = firstVert+0;
   cf->mFaceList.last().vertex[2] = firstVert+3;

   cf->mFaceList.increment();
   cf->mFaceList.last().normal = PlaneF(tverts[2], tverts[1], tverts[3]);
   cf->mFaceList.last().vertex[0] = firstVert+2;
   cf->mFaceList.last().vertex[1] = firstVert+1;
   cf->mFaceList.last().vertex[2] = firstVert+3;

   cf->mFaceList.increment();
   cf->mFaceList.last().normal = PlaneF(tverts[0], tverts[2], tverts[3]);
   cf->mFaceList.last().vertex[0] = firstVert+0;
   cf->mFaceList.last().vertex[1] = firstVert+2;
   cf->mFaceList.last().vertex[2] = firstVert+3;
}


void MeshRoadConvex::getPolyList( AbstractPolyList* list )
{
   // Transform the list into object space and set the pointer to the object
   //MatrixF i( mObject->getTransform() );
   //Point3F iS( mObject->getScale() );
   //list->setTransform(&i, iS);

   list->setTransform( &MatrixF::Identity, Point3F::One );
   list->setObject(mObject);

   // Points...
   S32 base =  list->addPoint(verts[1]);
   list->addPoint(verts[2]);
   list->addPoint(verts[0]);
   list->addPoint(verts[3]);

   // Planes...
   list->begin(0,0);
   list->vertex(base + 2);
   list->vertex(base + 1);
   list->vertex(base + 0);
   list->plane(base + 2, base + 1, base + 0);
   list->end();
   list->begin(0,0);
   list->vertex(base + 2);
   list->vertex(base + 1);
   list->vertex(base + 3);
   list->plane(base + 2, base + 1, base + 3);
   list->end();
   list->begin(0,0);
   list->vertex(base + 3);
   list->vertex(base + 1);
   list->vertex(base + 0);
   list->plane(base + 3, base + 1, base + 0);
   list->end();
   list->begin(0,0);
   list->vertex(base + 2);
   list->vertex(base + 3);
   list->vertex(base + 0);
   list->plane(base + 2, base + 3, base + 0);
   list->end();
}


//------------------------------------------------------------------------------
// MeshRoadSegment Class
//------------------------------------------------------------------------------

MeshRoadSegment::MeshRoadSegment()
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

MeshRoadSegment::MeshRoadSegment( MeshRoadSlice *rs0, MeshRoadSlice *rs1, const MatrixF &roadMat )
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

   // Calculate the bounding box(s)
   worldbounds.minExtents = worldbounds.maxExtents = rs0->p0;
   worldbounds.extend( rs0->p2 );
   worldbounds.extend( rs0->pb0 );
   worldbounds.extend( rs0->pb2 );
   worldbounds.extend( rs1->p0 );
   worldbounds.extend( rs1->p2 );
   worldbounds.extend( rs1->pb0 );
   worldbounds.extend( rs1->pb2 );

   objectbounds = worldbounds;
   roadMat.mul( objectbounds );

   // Calculate the planes for this segment
   // Will be used for intersection/buoyancy tests

   mPlaneCount = 6;
   mPlanes[0].set( slice0->pb0, slice0->p0, slice1->p0 ); // left   
   mPlanes[1].set( slice1->pb2, slice1->p2, slice0->p2 ); // right   
   mPlanes[2].set( slice0->pb2, slice0->p2, slice0->p0 ); // near   
   mPlanes[3].set( slice1->p0, slice1->p2, slice1->pb2 ); // far   
   mPlanes[4].set( slice1->p2, slice1->p0, slice0->p0 ); // top   
   mPlanes[5].set( slice0->pb0, slice1->pb0, slice1->pb2 ); // bottom
}

void MeshRoadSegment::set( MeshRoadSlice *rs0, MeshRoadSlice *rs1 )
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

bool MeshRoadSegment::intersectBox( const Box3F &bounds ) const
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

bool MeshRoadSegment::containsPoint( const Point3F &pnt ) const
{
   // This code from Frustum class.

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
      if ( maxDot < 0.0f )
         return false;
   }

   return true;
}

F32 MeshRoadSegment::distanceToSurface(const Point3F &pnt) const
{
   return mPlanes[4].distToPlane( pnt );
}

//------------------------------------------------------------------------------
// MeshRoad Class
//------------------------------------------------------------------------------

ConsoleDocClass( MeshRoad,
   "@brief A strip of rectangular mesh segments defined by a 3D spline "
   "for prototyping road-shaped objects in your scene.\n\n"
   
   "User may control width and depth per node, overall spline shape in three "
   "dimensions, and seperate Materials for rendering the top, bottom, and side surfaces.\n\n"
   
   "MeshRoad is not capable of handling intersections, branches, curbs, or other "
   "desirable features in a final 'road' asset and is therefore intended for "
   "prototyping and experimentation.\n\n"

   "Materials assigned to MeshRoad should tile vertically.\n\n"

   "@ingroup Terrain"
);

bool MeshRoad::smEditorOpen = false;
bool MeshRoad::smShowBatches = false;
bool MeshRoad::smShowSpline = true;
bool MeshRoad::smShowRoad = true;
bool MeshRoad::smWireframe = true;
SimObjectPtr<SimSet> MeshRoad::smServerMeshRoadSet = NULL;

GFXStateBlockRef MeshRoad::smWireframeSB;

IMPLEMENT_CO_NETOBJECT_V1(MeshRoad);

MeshRoad::MeshRoad()
: mTextureLength( 5.0f ),
  mBreakAngle( 3.0f ),
  mPhysicsRep( NULL ),
  mWidthSubdivisions( 0 )
{
   mConvexList = new Convex;

   // Setup NetObject.
   mTypeMask |= StaticObjectType | StaticShapeObjectType;
   mNetFlags.set(Ghostable);

	mMatInst[Top] = NULL;
   mMatInst[Bottom] = NULL;
   mMatInst[Side] = NULL;
}

MeshRoad::~MeshRoad()
{   
   delete mConvexList;
   mConvexList = NULL;
}

void MeshRoad::initPersistFields()
{
   addGroup( "MeshRoad" );

      addField( "topMaterial", TypeMaterialName, Offset( mMaterialName[Top], MeshRoad ),
         "Material for the upper surface of the road." );

      addField( "bottomMaterial", TypeMaterialName, Offset( mMaterialName[Bottom], MeshRoad ),
         "Material for the bottom surface of the road." );

      addField( "sideMaterial", TypeMaterialName, Offset( mMaterialName[Side], MeshRoad ),
         "Material for the left, right, front, and back surfaces of the road." );

      addField( "textureLength", TypeF32, Offset( mTextureLength, MeshRoad ), 
         "The length in meters of textures mapped to the MeshRoad." );      

      addField( "breakAngle", TypeF32, Offset( mBreakAngle, MeshRoad ), 
         "Angle in degrees - MeshRoad will subdivide the spline if its curve is greater than this threshold." ); 

      addField( "widthSubdivisions", TypeS32, Offset( mWidthSubdivisions, MeshRoad ), 
         "Subdivide segments widthwise this many times when generating vertices." );

   endGroup( "MeshRoad" );

   addGroup( "Internal" );

      addProtectedField( "Node", TypeString, NULL, &addNodeFromField, &emptyStringProtectedGetFn, 
         "Do not modify, for internal use." );

   endGroup( "Internal" );

   Parent::initPersistFields();
}

void MeshRoad::consoleInit()
{
   Parent::consoleInit();

   Con::addVariable( "$MeshRoad::EditorOpen", TypeBool, &MeshRoad::smEditorOpen, "True if the MeshRoad editor is open, otherwise false.\n"
	   "@ingroup Editors\n");
   Con::addVariable( "$MeshRoad::wireframe", TypeBool, &MeshRoad::smWireframe, "If true, will render the wireframe of the road.\n"
	   "@ingroup Editors\n");
   Con::addVariable( "$MeshRoad::showBatches", TypeBool, &MeshRoad::smShowBatches, "Determines if the debug rendering of the batches cubes is displayed or not.\n"
	   "@ingroup Editors\n");
   Con::addVariable( "$MeshRoad::showSpline", TypeBool, &MeshRoad::smShowSpline, "If true, the spline on which the curvature of this road is based will be rendered.\n"
	   "@ingroup Editors\n");
   Con::addVariable( "$MeshRoad::showRoad", TypeBool, &MeshRoad::smShowRoad, "If true, the road will be rendered. When in the editor, roads are always rendered regardless of this flag.\n"
	   "@ingroup Editors\n");
}

bool MeshRoad::addNodeFromField( void *object, const char *index, const char *data )
{
   MeshRoad *pObj = static_cast<MeshRoad*>(object);

   //if ( !pObj->isProperlyAdded() )
   //{      
   F32 width, depth;
   Point3F pos, normal;      
   U32 result = dSscanf( data, "%g %g %g %g %g %g %g %g", &pos.x, &pos.y, &pos.z, &width, &depth, &normal.x, &normal.y, &normal.z );      
   if ( result == 8 )
      pObj->_addNode( pos, width, depth, normal );      
   //}

   return false;
}

bool MeshRoad::onAdd()
{
   if ( !Parent::onAdd() ) 
      return false;

   // Reset the World Box.
   //setGlobalBounds();
   resetWorldBox();

   // Set the Render Transform.
   setRenderTransform(mObjToWorld);

   // Add to ServerMeshRoadSet
   if ( isServerObject() )
   {
      getServerSet()->addObject( this );      
   }

   if ( isClientObject() )
      _initMaterial();

   // Generate the Vert/Index buffers and everything else.
   _regenerate();

   // Add to Scene.
   addToScene();

   return true;
}

void MeshRoad::onRemove()
{
   SAFE_DELETE( mPhysicsRep );

   mConvexList->nukeList();

   for ( U32 i = 0; i < SurfaceCount; i++ )
   {
      SAFE_DELETE( mMatInst[i] );
   }

   removeFromScene();
   Parent::onRemove();
}

void MeshRoad::inspectPostApply()
{
   // Set Parent.
   Parent::inspectPostApply();

   //if ( mMetersPerSegment < MIN_METERS_PER_SEGMENT )
   //   mMetersPerSegment = MIN_METERS_PER_SEGMENT;

   setMaskBits(MeshRoadMask);
}

void MeshRoad::onStaticModified( const char* slotName, const char*newValue )
{
   Parent::onStaticModified( slotName, newValue );

   if ( dStricmp( slotName, "breakAngle" ) == 0 )
   {
      setMaskBits( RegenMask );
   }
}

void MeshRoad::writeFields( Stream &stream, U32 tabStop )
{
   Parent::writeFields( stream, tabStop );

   // Now write all nodes

   stream.write(2, "\r\n");   

   for ( U32 i = 0; i < mNodes.size(); i++ )
   {
      const MeshRoadNode &node = mNodes[i];

      stream.writeTabs(tabStop);

      char buffer[1024];
      dMemset( buffer, 0, 1024 );
      dSprintf( buffer, 1024, "Node = \"%g %g %g %g %g %g %g %g\";", node.point.x, node.point.y, node.point.z, node.width, node.depth, node.normal.x, node.normal.y, node.normal.z );      
      stream.writeLine( (const U8*)buffer );
   }
}

bool MeshRoad::writeField( StringTableEntry fieldname, const char *value )
{   
   if ( fieldname == StringTable->insert("Node") )
      return false;

   return Parent::writeField( fieldname, value );
}

void MeshRoad::onEditorEnable()
{
}

void MeshRoad::onEditorDisable()
{
}

SimSet* MeshRoad::getServerSet()
{
   if ( !smServerMeshRoadSet )
   {
      smServerMeshRoadSet = new SimSet();
      smServerMeshRoadSet->registerObject( "ServerMeshRoadSet" );
		Sim::getRootGroup()->addObject( smServerMeshRoadSet );
   }

   return smServerMeshRoadSet;
}

void MeshRoad::prepRenderImage( SceneRenderState* state )
{
   if ( mNodes.size() <= 1 )
      return;

   RenderPassManager *renderPass = state->getRenderPass();
	
   // Normal Road RenderInstance
   // Always rendered when the editor is not open
   // otherwise obey the smShowRoad flag
   if ( smShowRoad || !smEditorOpen )
   {
      MeshRenderInst coreRI;
      coreRI.clear();
      coreRI.objectToWorld = &MatrixF::Identity;
      coreRI.worldToCamera = renderPass->allocSharedXform(RenderPassManager::View);
      coreRI.projection = renderPass->allocSharedXform(RenderPassManager::Projection);
      coreRI.type = RenderPassManager::RIT_Mesh;      
		
      BaseMatInstance *matInst;
      for ( U32 i = 0; i < SurfaceCount; i++ )
      {             
         matInst = state->getOverrideMaterial( mMatInst[i] );   
         if ( !matInst )
            continue;

         // Get the lights if we haven't already.
         if ( matInst->isForwardLit() && !coreRI.lights[0] )
         {
            LightQuery query;
            query.init( getWorldSphere() );
	         query.getLights( coreRI.lights, 8 );
         }

         MeshRenderInst *ri = renderPass->allocInst<MeshRenderInst>();
         *ri = coreRI;

         // Currently rendering whole road, fix to cull and batch
         // per segment.

         // Set the correct material for rendering.
         ri->matInst = matInst;
         ri->vertBuff = &mVB[i];
         ri->primBuff = &mPB[i];

         ri->prim = renderPass->allocPrim();
         ri->prim->type = GFXTriangleList;
         ri->prim->minIndex = 0;
         ri->prim->startIndex = 0;
         ri->prim->numPrimitives = mTriangleCount[i];
         ri->prim->startVertex = 0;
         ri->prim->numVertices = mVertCount[i];

         // We sort by the material then vertex buffer.
         ri->defaultKey = matInst->getStateHint();
         ri->defaultKey2 = (U32)ri->vertBuff; // Not 64bit safe!

         renderPass->addInst( ri );  
      }
   }

   // Debug RenderInstance
   // Only when editor is open.
   if ( smEditorOpen )
   {
      ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
      ri->renderDelegate.bind( this, &MeshRoad::_debugRender );
		ri->type = RenderPassManager::RIT_Editor;
      state->getRenderPass()->addInst( ri );
   }
}

void MeshRoad::_initMaterial()
{
   for ( U32 i = 0; i < SurfaceCount; i++ )
   {
      if ( mMatInst[i] )
         SAFE_DELETE( mMatInst[i] );

      if ( mMaterial[i] )
         mMatInst[i] = mMaterial[i]->createMatInstance();
      else
         mMatInst[i] = MATMGR->createMatInstance( "WarningMaterial" );

      mMatInst[i]->init( MATMGR->getDefaultFeatures(), getGFXVertexFormat<GFXVertexPNTT>() );
   }
}

void MeshRoad::_debugRender( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance* )
{
   //MeshRoadConvex convex;
   //buildConvex( Box3F(true), convex );
   //convex.render();   
   //GFXDrawUtil *drawer = GFX->getDrawUtil();

   //GFX->setStateBlock( smStateBlock );
   return;
	/*
   U32 convexCount = mDebugConvex.size();

   PrimBuild::begin( GFXTriangleList, convexCount * 12 );
   PrimBuild::color4i( 0, 0, 255, 155 );

   for ( U32 i = 0; i < convexCount; i++ )
   {
      MeshRoadConvex *convex = mDebugConvex[i];

      Point3F a = convex->verts[0];
      Point3F b = convex->verts[1];
      Point3F c = convex->verts[2];
      Point3F p = convex->verts[3];

      //mObjToWorld.mulP(a);
      //mObjToWorld.mulP(b);
      //mObjToWorld.mulP(c);
      //mObjToWorld.mulP(p);
      
      PrimBuild::vertex3fv( c );
      PrimBuild::vertex3fv( b );
      PrimBuild::vertex3fv( a );      

      PrimBuild::vertex3fv( b );
      PrimBuild::vertex3fv( a );
      PrimBuild::vertex3fv( p );

      PrimBuild::vertex3fv( c );
      PrimBuild::vertex3fv( b );
      PrimBuild::vertex3fv( p );

      PrimBuild::vertex3fv( a );
      PrimBuild::vertex3fv( c );
      PrimBuild::vertex3fv( p );
   }

   PrimBuild::end();

   for ( U32 i = 0; i < mSegments.size(); i++ )
   {
      ///GFX->getDrawUtil()->drawWireBox( mSegments[i].worldbounds, ColorI(255,0,0,255) );
   }

   GFX->enterDebugEvent( ColorI( 255, 0, 0 ), "DecalRoad_debugRender" );
   GFXTransformSaver saver;

   GFX->setStateBlock( smStateBlock );      

   Point3F size(1,1,1);
   ColorI color( 255, 0, 0, 255 );

   if ( smShowBatches )  
   {
      for ( U32 i = 0; i < mBatches.size(); i++ )   
      {
         const Box3F &box = mBatches[i].bounds;
         Point3F center;
         box.getCenter( &center );

         GFX->getDrawUtil()->drawWireCube( ( box.maxExtents - box.minExtents ) * 0.5f, center, ColorI(255,100,100,255) );         
      }
   }

   GFX->leaveDebugEvent();
   */
}

U32 MeshRoad::packUpdate(NetConnection * con, U32 mask, BitStream * stream)
{  
   U32 retMask = Parent::packUpdate(con, mask, stream);

   if ( stream->writeFlag( mask & MeshRoadMask ) )
   {
      // Write Object Transform.
      stream->writeAffineTransform( mObjToWorld );

      // Write Materials      
      stream->write( mMaterialName[0] );      
      stream->write( mMaterialName[1] );
      stream->write( mMaterialName[2] );

      stream->write( mTextureLength );      
      stream->write( mBreakAngle );
      stream->write( mWidthSubdivisions );
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

            MeshRoadNodeEvent* event = new MeshRoadNodeEvent();
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

   stream->writeFlag( mask & RegenMask );

   // Were done ...
   return retMask;
}

void MeshRoad::unpackUpdate(NetConnection * con, BitStream * stream)
{
   // Unpack Parent.
   Parent::unpackUpdate(con, stream);

   // MeshRoadMask
   if(stream->readFlag())
   {
      MatrixF		ObjectMatrix;
      stream->readAffineTransform(&ObjectMatrix);
      Parent::setTransform(ObjectMatrix);

      // Read Materials...
      Material *pMat = NULL;

      for ( U32 i = 0; i < SurfaceCount; i++ )
      {
         stream->read( &mMaterialName[i] );
        
         if ( !Sim::findObject( mMaterialName[i], pMat ) )
            Con::printf( "DecalRoad::unpackUpdate, failed to find Material of name &s!", mMaterialName[i].c_str() );
         else         
            mMaterial[i] = pMat;         
      }

      if ( isProperlyAdded() )
         _initMaterial(); 

      stream->read( &mTextureLength );

      stream->read( &mBreakAngle );

      stream->read( &mWidthSubdivisions );
   }

   // NodeMask
   if ( stream->readFlag() )
   {
      if (stream->readFlag())
      {
         // Nodes have been passed in this update
         U32 count = stream->readInt( 16 );

         mNodes.clear();

         Point3F pos, normal;
         F32 width, depth;
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
            MeshRoadNodeList* roadList = dynamic_cast<MeshRoadNodeList*>( list );
            if (roadList)
               buildNodesFromList( roadList );

            delete list;
         }
         else
         {
            // Nodes have not yet arrived, so register our interest in the list
            MeshRoadNodeListNotify* notify = new MeshRoadNodeListNotify( this, id );
            gClientNodeListManager->registerNotification( notify );
         }
      }
   }

   if ( stream->readFlag() && isProperlyAdded() )   
      _regenerate();
}

void MeshRoad::setTransform( const MatrixF &mat )
{
   for ( U32 i = 0; i < mNodes.size(); i++ )
   {
      mWorldToObj.mulP( mNodes[i].point );
      mat.mulP( mNodes[i].point );
   }

   Parent::setTransform( mat );

   if ( mPhysicsRep )
      mPhysicsRep->setTransform( mat );

   // Regenerate and update the client
   _regenerate();
   setMaskBits( NodeMask | RegenMask );
}

void MeshRoad::setScale( const VectorF &scale )
{
   // We ignore scale requests from the editor
   // right now.

   //Parent::setScale( scale );
}

void MeshRoad::buildConvex(const Box3F& box, Convex* convex)
{
   if ( mSlices.size() < 2 )
      return;

   mConvexList->collectGarbage();
   mDebugConvex.clear();

   Box3F realBox = box;
   mWorldToObj.mul(realBox);
   realBox.minExtents.convolveInverse(mObjScale);
   realBox.maxExtents.convolveInverse(mObjScale);

   if (realBox.isOverlapped(getObjBox()) == false)
      return;

   U32 segmentCount = mSegments.size();

   // Create convex(s) for each segment
   for ( U32 i = 0; i < segmentCount; i++ )
   {
      const MeshRoadSegment &segment = mSegments[i];

      // Is this segment overlapped?
      if ( !segment.getWorldBounds().isOverlapped( box ) )
         continue;

      // Each segment has 6 faces
      for ( U32 j = 0; j < 6; j++ )
      {
         // Only first segment has front face
         if ( j == 4 && i != 0 )
            continue;
         // Only last segment has back face
         if ( j == 5 && i != segmentCount-1 )
            continue;

         // Each face has 2 convex(s)
         for ( U32 k = 0; k < 2; k++ )
         {
            // See if this convex exists in the working set already...
            Convex* cc = 0;
            CollisionWorkingList& wl = convex->getWorkingList();
            for ( CollisionWorkingList* itr = wl.wLink.mNext; itr != &wl; itr = itr->wLink.mNext ) 
            {
               if ( itr->mConvex->getType() == MeshRoadConvexType )
               {
                  MeshRoadConvex *pConvex = static_cast<MeshRoadConvex*>(itr->mConvex);

                  if ( pConvex->pRoad == this &&
                       pConvex->segmentId == i &&
                       pConvex->faceId == j &&
                       pConvex->triangleId == k )           
                  {
                     cc = itr->mConvex;
                     break;
                  }
               }
            }
            if (cc)
               continue;

            // Get the triangle...
            U32 idx0 = gIdxArray[j][k][0];
            U32 idx1 = gIdxArray[j][k][1];
            U32 idx2 = gIdxArray[j][k][2];

            Point3F a = segment[idx0];
            Point3F b = segment[idx1];
            Point3F c = segment[idx2];
            
            // Transform the result into object space!
            //mWorldToObj.mulP( a );
            //mWorldToObj.mulP( b );
            //mWorldToObj.mulP( c );            

            PlaneF p( c, b, a );
            Point3F peak = ((a + b + c) / 3.0f) + (p * 0.15f);

            // Set up the convex...
            MeshRoadConvex *cp = new MeshRoadConvex();

            mConvexList->registerObject( cp );
            convex->addToWorkingList( cp );

            cp->mObject    = this;
            cp->pRoad   = this;
            cp->segmentId = i;
            cp->faceId = j;
            cp->triangleId = k;

            cp->normal = p;
            cp->verts[0] = c;
            cp->verts[1] = b;
            cp->verts[2] = a;
            cp->verts[3] = peak;

            // Update the bounding box.
            Box3F &bounds = cp->box;
            bounds.minExtents.set( F32_MAX,  F32_MAX,  F32_MAX );
            bounds.maxExtents.set( -F32_MAX, -F32_MAX, -F32_MAX );

            bounds.minExtents.setMin( a );
            bounds.minExtents.setMin( b );
            bounds.minExtents.setMin( c );
            bounds.minExtents.setMin( peak );

            bounds.maxExtents.setMax( a );
            bounds.maxExtents.setMax( b );
            bounds.maxExtents.setMax( c );
            bounds.maxExtents.setMax( peak );

            mDebugConvex.push_back(cp);
         }
      }
   }
}

bool MeshRoad::buildPolyList( PolyListContext, AbstractPolyList* polyList, const Box3F &box, const SphereF & )
{
   if ( mSlices.size() < 2 )
      return false;

   polyList->setTransform( &MatrixF::Identity, Point3F::One );
   polyList->setObject(this);

   // JCF: optimize this to not always add everything.

   return buildSegmentPolyList( polyList, 0, mSegments.size() - 1, true, true );
}

bool MeshRoad::buildSegmentPolyList( AbstractPolyList* polyList, U32 startSegIdx, U32 endSegIdx, bool capFront, bool capEnd )
{
   if ( mSlices.size() < 2 )
      return false;

   // Add verts
   for ( U32 i = startSegIdx; i <= endSegIdx; i++ )
   {
      const MeshRoadSegment &seg = mSegments[i];

      if ( i == startSegIdx )
      {
         polyList->addPoint( seg.slice0->p0 );
         polyList->addPoint( seg.slice0->p2 );
         polyList->addPoint( seg.slice0->pb0 );
         polyList->addPoint( seg.slice0->pb2 );
      }

      polyList->addPoint( seg.slice1->p0 );
      polyList->addPoint( seg.slice1->p2 );
      polyList->addPoint( seg.slice1->pb0 );
      polyList->addPoint( seg.slice1->pb2 );
   }

   // Temporaries to hold indices for the corner points of a quad.
   S32 p00, p01, p11, p10;
   S32 pb00, pb01, pb11, pb10;
   U32 offset = 0;

   DebugDrawer *ddraw = NULL;//DebugDrawer::get();
   ClippedPolyList *cpolyList = dynamic_cast<ClippedPolyList*>(polyList);
   MatrixF mat;
   Point3F scale;
   if ( cpolyList )
      cpolyList->getTransform( &mat, &scale );


   for ( U32 i = startSegIdx; i <= endSegIdx; i++ )
   {		
      p00 = offset;
      p10 = offset + 1;
      pb00 = offset + 2;
      pb10 = offset + 3;
      p01 = offset + 4;
      p11 = offset + 5;
      pb01 = offset + 6;
      pb11 = offset + 7;

      // Top Face

      polyList->begin( 0,0 );
      polyList->vertex( p00 );
      polyList->vertex( p01 );
      polyList->vertex( p11 );
      polyList->plane( p00, p01, p11 );
      polyList->end();
      if ( ddraw && cpolyList )
      {
         Point3F v0 = cpolyList->mVertexList[p00].point;
         mat.mulP( v0 );
         Point3F v1 = cpolyList->mVertexList[p01].point;
         mat.mulP( v1 );
         Point3F v2 = cpolyList->mVertexList[p11].point;
         mat.mulP( v2 );
         ddraw->drawTri( v0, v1, v2 );
         ddraw->setLastZTest( false );
         ddraw->setLastTTL( 0 );
      }

      polyList->begin( 0,0 );
      polyList->vertex( p00 );
      polyList->vertex( p11 );
      polyList->vertex( p10 );
      polyList->plane( p00, p11, p10 );
      polyList->end();
      if ( ddraw && cpolyList )
      {
         ddraw->drawTri( cpolyList->mVertexList[p00].point, cpolyList->mVertexList[p11].point, cpolyList->mVertexList[p10].point );
         ddraw->setLastTTL( 0 );
      }

      // Left Face

      polyList->begin( 0,0 );
      polyList->vertex( pb00 );
      polyList->vertex( pb01 );
      polyList->vertex( p01 );
      polyList->plane( pb00, pb01, p01 );
      polyList->end();

      polyList->begin( 0,0 );
      polyList->vertex( pb00 );
      polyList->vertex( p01 );
      polyList->vertex( p00 );
      polyList->plane( pb00, p01, p00 );
      polyList->end();   

      // Right Face

      polyList->begin( 0,0 );
      polyList->vertex( p10 );
      polyList->vertex( p11 );
      polyList->vertex( pb11 );
      polyList->plane( p10, p11, pb11 );
      polyList->end();

      polyList->begin( 0,0 );
      polyList->vertex( p10 );
      polyList->vertex( pb11 );
      polyList->vertex( pb10 );
      polyList->plane( p10, pb11, pb10 );
      polyList->end();  

      // Bottom Face

      polyList->begin( 0,0 );
      polyList->vertex( pb00 );
      polyList->vertex( pb10 );
      polyList->vertex( pb11 );
      polyList->plane( pb00, pb10, pb11 );
      polyList->end();

      polyList->begin( 0,0 );
      polyList->vertex( pb00 );
      polyList->vertex( pb11 );
      polyList->vertex( pb01 );
      polyList->plane( pb00, pb11, pb01 );
      polyList->end();  

      // Front Face

      if ( i == startSegIdx && capFront )
      {
         polyList->begin( 0,0 );
         polyList->vertex( p00 );
         polyList->vertex( p10 );
         polyList->vertex( pb10 );
         polyList->plane( p00, p10, pb10 );
         polyList->end();

         polyList->begin( 0,0 );
         polyList->vertex( p00 );
         polyList->vertex( pb10 );
         polyList->vertex( pb00 );
         polyList->plane( p00, pb10, pb00 );
         polyList->end();  
      }

      // Back Face
      if ( i == endSegIdx && capEnd )
      {
         polyList->begin( 0,0 );
         polyList->vertex( p01 );
         polyList->vertex( pb01 );
         polyList->vertex( pb11 );
         polyList->plane( p01, pb01, pb11 );
         polyList->end();

         polyList->begin( 0,0 );
         polyList->vertex( p01 );
         polyList->vertex( pb11 );
         polyList->vertex( p11 );
         polyList->plane( p01, pb11, p11 );
         polyList->end();  
      }

      offset += 4;
   }

   return true;
}

bool MeshRoad::castRay( const Point3F &s, const Point3F &e, RayInfo *info )
{
   Point3F start = s;
   Point3F end = e;
   mObjToWorld.mulP(start);
   mObjToWorld.mulP(end);

   F32 out = 1.0f;   // The output fraction/percentage along the line defined by s and e
   VectorF norm(0.0f, 0.0f, 0.0f);     // The normal of the face intersected
   
   Vector<MeshRoadHitSegment> hitSegments;

   for ( U32 i = 0; i < mSegments.size(); i++ )
   {
      const MeshRoadSegment &segment = mSegments[i];

      F32 t;
      VectorF n;

      if ( segment.getWorldBounds().collideLine( start, end, &t, &n ) )
      {
         hitSegments.increment();
         hitSegments.last().t = t;
         hitSegments.last().idx = i;         
      }
   }

   dQsort( hitSegments.address(), hitSegments.size(), sizeof(MeshRoadHitSegment), compareHitSegments );

   U32 idx0, idx1, idx2;
   F32 t;

   for ( U32 i = 0; i < hitSegments.size(); i++ )
   {
      U32 segIdx = hitSegments[i].idx;
      const MeshRoadSegment &segment = mSegments[segIdx];

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

bool MeshRoad::collideBox(const Point3F &start, const Point3F &end, RayInfo* info)
{   
   Con::warnf( "MeshRoad::collideBox() - not yet implemented!" );
   return Parent::collideBox( start, end, info );
}

void MeshRoad::_regenerate()
{               
   if ( mNodes.size() == 0 )
      return;

   const Point3F &nodePt = mNodes.first().point;

   MatrixF mat( true );
   mat.setPosition( nodePt );
   Parent::setTransform( mat );

   _generateSlices();

   // Make sure we are in the correct bins given our world box.
   if( getSceneManager() != NULL )
      getSceneManager()->notifyObjectDirty( this );
}

void MeshRoad::_generateSlices()
{      
   if ( mNodes.size() < 2 )
      return;

   // Create the spline, initialized with the MeshRoadNode(s)
   U32 nodeCount = mNodes.size();
   MeshRoadSplineNode *splineNodes = new MeshRoadSplineNode[nodeCount];

   for ( U32 i = 0; i < nodeCount; i++ )
   {
      MeshRoadSplineNode &splineNode = splineNodes[i];
      const MeshRoadNode &node = mNodes[i];

      splineNode.x = node.point.x;
      splineNode.y = node.point.y;
      splineNode.z = node.point.z;
      splineNode.width = node.width;
      splineNode.depth = node.depth;
      splineNode.normal = node.normal;
   }

   CatmullRom<MeshRoadSplineNode> spline;
   spline.initialize( nodeCount, splineNodes );
   delete [] splineNodes;

   mSlices.clear();
      
   VectorF lastBreakVector(0,0,0);      
   MeshRoadSlice slice;
   MeshRoadSplineNode lastBreakNode;
   lastBreakNode = spline.evaluate(0.0f);

   for ( U32 i = 1; i < mNodes.size(); i++ )
   {
      F32 t1 = spline.getTime(i);
      F32 t0 = spline.getTime(i-1);
      
      F32 segLength = spline.arcLength( t0, t1 );

      U32 numSegments = mCeil( segLength / MIN_METERS_PER_SEGMENT );
      numSegments = getMax( numSegments, (U32)1 );
      F32 tstep = ( t1 - t0 ) / numSegments; 

      U32 startIdx = 0;
      U32 endIdx = ( i == nodeCount - 1 ) ? numSegments + 1 : numSegments;

      for ( U32 j = startIdx; j < endIdx; j++ )
      {
         F32 t = t0 + tstep * j;
         MeshRoadSplineNode splineNode = spline.evaluate(t);

         VectorF toNodeVec = splineNode.getPosition() - lastBreakNode.getPosition();
         toNodeVec.normalizeSafe();

         if ( lastBreakVector.isZero() )
            lastBreakVector = toNodeVec;

         F32 angle = mRadToDeg( mAcos( mDot( toNodeVec, lastBreakVector ) ) );

         if ( j == startIdx || 
            ( j == endIdx - 1 && i == mNodes.size() - 1 ) ||
              angle > mBreakAngle )
         {
            // Push back a spline node
            slice.p1.set( splineNode.x, splineNode.y, splineNode.z );            
            slice.width = splineNode.width;
            slice.depth = splineNode.depth;
            slice.normal = splineNode.normal;   
            slice.normal.normalize();
            slice.parentNodeIdx = i-1;
            slice.t = t;
            mSlices.push_back( slice );         

            lastBreakVector = splineNode.getPosition() - lastBreakNode.getPosition();
            lastBreakVector.normalizeSafe();

            lastBreakNode = splineNode;
         }          
      }
   }

   //
   // Calculate uvec, fvec, and rvec for all slices
   //   

   MatrixF mat(true);

   for ( U32 i = 0; i < mSlices.size(); i++ )
   {
      calcSliceTransform( i, mat );
      mat.getColumn( 0, &mSlices[i].rvec );
      mat.getColumn( 1, &mSlices[i].fvec );
      mat.getColumn( 2, &mSlices[i].uvec );
   } 

   //
   // Calculate p0/p2/pb0/pb2 for all slices
   //      
   for ( U32 i = 0; i < mSlices.size(); i++ )
   {
      MeshRoadSlice *slice = &mSlices[i];
      slice->p0 = slice->p1 - slice->rvec * slice->width * 0.5f;
      slice->p2 = slice->p1 + slice->rvec * slice->width * 0.5f;
      slice->pb0 = slice->p0 - slice->uvec * slice->depth;
      slice->pb2 = slice->p2 - slice->uvec * slice->depth;
   }

   // Generate the object/world bounds
   Box3F box;
   for ( U32 i = 0; i < mSlices.size(); i++ )
   {
      const MeshRoadSlice &slice = mSlices[i];

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
   resetObjectBox();

   _generateSegments();   
}

void MeshRoad::_generateSegments()
{
   mSegments.clear();

   for ( U32 i = 0; i < mSlices.size() - 1; i++ )
   {
      MeshRoadSegment seg( &mSlices[i], &mSlices[i+1], getWorldTransform() );

      mSegments.push_back( seg );
   }

   if ( isClientObject() )
      _generateVerts();

   if ( PHYSICSMGR )
   {
      SAFE_DELETE( mPhysicsRep );
      //mPhysicsRep = PHYSICSMGR->createBody();
   }
}

void MeshRoad::_generateVerts()
{           
   const U32 widthDivisions = getMax( 0, mWidthSubdivisions );
   const F32 divisionStep = 1.0f / (F32)( widthDivisions + 1 );
   const U32 sliceCount = mSlices.size();
   const U32 segmentCount = mSegments.size();

   mVertCount[Top] = ( 2 + widthDivisions ) * sliceCount;
   mTriangleCount[Top] = segmentCount * 2 * ( widthDivisions + 1 );

   mVertCount[Bottom] = sliceCount * 2;
   mTriangleCount[Bottom] = segmentCount * 2;

   mVertCount[Side] = sliceCount * 4;
   mTriangleCount[Side] = segmentCount * 4 + 4;
   
   // Calculate TexCoords for Slices

   F32 texCoordV = 0.0f;
   mSlices[0].texCoordV = 0.0f;

   for ( U32 i = 1; i < sliceCount; i++ )
   {
      MeshRoadSlice &slice = mSlices[i]; 
      MeshRoadSlice &prevSlice = mSlices[i-1];
             
      // Increment the textCoordV for the next slice.
      F32 len = ( slice.p1 - prevSlice.p1 ).len();
      texCoordV += len / mTextureLength;         

      slice.texCoordV = texCoordV;
   }

   // Make Vertex Buffers
   GFXVertexPNTT *pVert = NULL;
   U32 vertCounter = 0;

   // Top Buffers...

   mVB[Top].set( GFX, mVertCount[Top], GFXBufferTypeStatic );   
   pVert = mVB[Top].lock(); 
   vertCounter = 0;
   
   for ( U32 i = 0; i < sliceCount; i++ )
   {
      MeshRoadSlice &slice = mSlices[i];      
      
      pVert->point = slice.p0;    
      pVert->normal = slice.uvec;
      pVert->tangent = slice.fvec;
      pVert->texCoord.set(1,slice.texCoordV);      
      pVert++;
      vertCounter++;

      for ( U32 j = 0; j < widthDivisions; j++ )
      {
         const F32 t = divisionStep * (F32)( j + 1 );

         pVert->point.interpolate( slice.p0, slice.p2, t );    
         pVert->normal = slice.uvec;
         pVert->tangent = slice.fvec;
         pVert->texCoord.set( 1.0f - t, slice.texCoordV );      
         pVert++;
         vertCounter++;
      }

      pVert->point = slice.p2;    
      pVert->normal = slice.uvec;
      pVert->tangent = slice.fvec;
      pVert->texCoord.set( 0, slice.texCoordV );      
      pVert++;
      vertCounter++;
   }

   AssertFatal( vertCounter == mVertCount[Top], "MeshRoad, wrote incorrect number of verts in mVB[Top]!" );

   mVB[Top].unlock();

   // Bottom Buffer...

   mVB[Bottom].set( GFX, mVertCount[Bottom], GFXBufferTypeStatic );   
   pVert = mVB[Bottom].lock(); 
   vertCounter = 0;

   for ( U32 i = 0; i < sliceCount; i++ )
   {
      MeshRoadSlice &slice = mSlices[i];      

      pVert->point = slice.pb2;    
      pVert->normal = -slice.uvec;
      pVert->tangent = slice.fvec;
      pVert->texCoord.set(0,slice.texCoordV);      
      pVert++;
      vertCounter++;

      pVert->point = slice.pb0;    
      pVert->normal = -slice.uvec;
      pVert->tangent = slice.fvec;
      pVert->texCoord.set(1,slice.texCoordV);      
      pVert++;
      vertCounter++;
   }

   AssertFatal( vertCounter == mVertCount[Bottom], "MeshRoad, wrote incorrect number of verts in mVB[Bottom]!" );

   mVB[Bottom].unlock();

   // Side Buffers...

   mVB[Side].set( GFX, mVertCount[Side], GFXBufferTypeStatic );   
   pVert = mVB[Side].lock(); 
   vertCounter = 0;

   for ( U32 i = 0; i < sliceCount; i++ )
   {
      MeshRoadSlice &slice = mSlices[i];      

      pVert->point = slice.p0;    
      pVert->normal = -slice.rvec;
      pVert->tangent = slice.fvec;
      pVert->texCoord.set(1,slice.texCoordV);      
      pVert++;
      vertCounter++;

      pVert->point = slice.p2;    
      pVert->normal = slice.rvec;
      pVert->tangent = slice.fvec;
      pVert->texCoord.set(1,slice.texCoordV);      
      pVert++;
      vertCounter++;

      pVert->point = slice.pb0;    
      pVert->normal = -slice.rvec;
      pVert->tangent = slice.fvec;
      pVert->texCoord.set(0,slice.texCoordV);      
      pVert++;
      vertCounter++;

      pVert->point = slice.pb2;    
      pVert->normal = slice.rvec;
      pVert->tangent = slice.fvec;
      pVert->texCoord.set(0,slice.texCoordV);      
      pVert++;
      vertCounter++;
   }

   AssertFatal( vertCounter == mVertCount[Side], "MeshRoad, wrote incorrect number of verts in mVB[Side]!" );

   mVB[Side].unlock();

   // Make Primitive Buffers   
   U32 p00, p01, p11, p10;
   U32 pb00, pb01, pb11, pb10;
   U32 offset = 0;
   U16 *pIdx = NULL;   
   U32 curIdx = 0; 

   // Top Primitive Buffer

   mPB[Top].set( GFX, mTriangleCount[Top] * 3, mTriangleCount[Top], GFXBufferTypeStatic );

   mPB[Top].lock(&pIdx);     
   curIdx = 0; 
   offset = 0;

   const U32 rowStride = 2 + widthDivisions;
 
   for ( U32 i = 0; i < mSegments.size(); i++ )
   {		
      for ( U32 j = 0; j < widthDivisions + 1; j++ )
      {
         p00 = offset;
         p10 = offset + 1;
         p01 = offset + rowStride;
         p11 = offset + rowStride + 1;

         pIdx[curIdx] = p00;
         curIdx++;
         pIdx[curIdx] = p01;
         curIdx++;
         pIdx[curIdx] = p11;
         curIdx++;

         pIdx[curIdx] = p00;
         curIdx++;
         pIdx[curIdx] = p11;
         curIdx++;
         pIdx[curIdx] = p10;
         curIdx++;  

         offset += 1;
      }

      offset += 1;
   }


   AssertFatal( curIdx == mTriangleCount[Top] * 3, "MeshRoad, wrote incorrect number of indices in mPB[Top]!" );

   mPB[Top].unlock();

   // Bottom Primitive Buffer

   mPB[Bottom].set( GFX, mTriangleCount[Bottom] * 3, mTriangleCount[Bottom], GFXBufferTypeStatic );

   mPB[Bottom].lock(&pIdx);     
   curIdx = 0; 
   offset = 0;

   for ( U32 i = 0; i < mSegments.size(); i++ )
   {		
      p00 = offset;
      p10 = offset + 1;
      p01 = offset + 2;
      p11 = offset + 3;

      pIdx[curIdx] = p00;
      curIdx++;
      pIdx[curIdx] = p01;
      curIdx++;
      pIdx[curIdx] = p11;
      curIdx++;

      pIdx[curIdx] = p00;
      curIdx++;
      pIdx[curIdx] = p11;
      curIdx++;
      pIdx[curIdx] = p10;
      curIdx++;      

      offset += 2;
   }

   AssertFatal( curIdx == mTriangleCount[Bottom] * 3, "MeshRoad, wrote incorrect number of indices in mPB[Bottom]!" );

   mPB[Bottom].unlock();

   // Side Primitive Buffer

   mPB[Side].set( GFX, mTriangleCount[Side] * 3, mTriangleCount[Side], GFXBufferTypeStatic );

   mPB[Side].lock(&pIdx);     
   curIdx = 0; 
   offset = 0;

   for ( U32 i = 0; i < mSegments.size(); i++ )
   {		
      p00 = offset;
      p10 = offset + 1;
      pb00 = offset + 2;
      pb10 = offset + 3;
      p01 = offset + 4;
      p11 = offset + 5;
      pb01 = offset + 6;
      pb11 = offset + 7;

      // Left Side

      pIdx[curIdx] = pb00;
      curIdx++;
      pIdx[curIdx] = pb01;
      curIdx++;
      pIdx[curIdx] = p01;
      curIdx++;

      pIdx[curIdx] = pb00;
      curIdx++;
      pIdx[curIdx] = p01;
      curIdx++;
      pIdx[curIdx] = p00;
      curIdx++;      

      // Right Side

      pIdx[curIdx] = p10;
      curIdx++;
      pIdx[curIdx] = p11;
      curIdx++;
      pIdx[curIdx] = pb11;
      curIdx++;

      pIdx[curIdx] = p10;
      curIdx++;
      pIdx[curIdx] = pb11;
      curIdx++;
      pIdx[curIdx] = pb10;
      curIdx++;      

      offset += 4;
   }

   // Cap the front and back ends
   pIdx[curIdx++] = 0;
   pIdx[curIdx++] = 1;
   pIdx[curIdx++] = 3;
   pIdx[curIdx++] = 0;
   pIdx[curIdx++] = 3;
   pIdx[curIdx++] = 2;
   
   pIdx[curIdx++] = offset + 0;
   pIdx[curIdx++] = offset + 3;
   pIdx[curIdx++] = offset + 1;
   pIdx[curIdx++] = offset + 0;
   pIdx[curIdx++] = offset + 2;
   pIdx[curIdx++] = offset + 3;


   AssertFatal( curIdx == mTriangleCount[Side] * 3, "MeshRoad, wrote incorrect number of indices in mPB[Side]!" );

   mPB[Side].unlock();
}

const MeshRoadNode& MeshRoad::getNode( U32 idx )
{
   return mNodes[idx];
}

VectorF MeshRoad::getNodeNormal( U32 idx )
{
   if ( mNodes.size() - 1 < idx )
      return VectorF::Zero;

   return mNodes[idx].normal;
}

void MeshRoad::setNodeNormal( U32 idx, const VectorF &normal )
{
   if ( mNodes.size() - 1 < idx )
      return;

   mNodes[idx].normal = normal;

   regenerate();

   setMaskBits( NodeMask | RegenMask );
}

Point3F MeshRoad::getNodePosition( U32 idx )
{
   if ( mNodes.size() - 1 < idx )
      return Point3F::Zero;

   return mNodes[idx].point;
}

void MeshRoad::setNodePosition( U32 idx, const Point3F &pos )
{
   if ( mNodes.size() - 1 < idx )
      return;

   mNodes[idx].point = pos;

   regenerate();

   setMaskBits( NodeMask | RegenMask );
}

U32 MeshRoad::addNode( const Point3F &pos, const F32 &width, const F32 &depth, const VectorF &normal )
{
   U32 idx = _addNode( pos, width, depth, normal );   

   regenerate();

   setMaskBits( NodeMask | RegenMask );

   return idx;
}

void MeshRoad::buildNodesFromList( MeshRoadNodeList* list )
{
   mNodes.clear();

   for (U32 i=0; i<list->mPositions.size(); ++i)
   {
      _addNode( list->mPositions[i], list->mWidths[i], list->mDepths[i], list->mNormals[i] );
   }

   _regenerate();
}

U32 MeshRoad::insertNode( const Point3F &pos, const F32 &width, const F32 &depth, const VectorF &normal, const U32 &idx )
{
   U32 ret = _insertNode( pos, width, depth, normal, idx );

   regenerate();

   setMaskBits( NodeMask | RegenMask );

   return ret;
}

void MeshRoad::setNode( const Point3F &pos, const F32 &width, const F32 &depth, const VectorF &normal, const U32 &idx )
{
   if ( mNodes.size() - 1 < idx )
      return;

   MeshRoadNode &node = mNodes[idx];

   node.point = pos;
   node.width = width;
   node.depth = depth;
   node.normal = normal;

   regenerate();

   setMaskBits( NodeMask | RegenMask );
}

void MeshRoad::setNodeWidth( U32 idx, F32 meters )
{
   meters = mClampF( meters, MIN_NODE_WIDTH, MAX_NODE_WIDTH );

   if ( mNodes.size() - 1 < idx )
      return;

   mNodes[idx].width = meters;
   _regenerate();

   setMaskBits( RegenMask | NodeMask );
}

F32 MeshRoad::getNodeWidth( U32 idx )
{
   if ( mNodes.size() - 1 < idx )
      return -1.0f;

   return mNodes[idx].width;
}

void MeshRoad::setNodeDepth( U32 idx, F32 meters )
{
   meters = mClampF( meters, MIN_NODE_DEPTH, MAX_NODE_DEPTH );

   if ( mNodes.size() - 1 < idx )
      return;

   mNodes[idx].depth = meters;

   _regenerate();

   setMaskBits( MeshRoadMask | RegenMask | NodeMask );
}

F32 MeshRoad::getNodeDepth( U32 idx )
{
   if ( mNodes.size() - 1 < idx )
      return -1.0f;

   return mNodes[idx].depth;
}

MatrixF MeshRoad::getNodeTransform( U32 idx )
{   
   MatrixF mat(true);   

   if ( mNodes.size() - 1 < idx )
      return mat;

   bool hasNext = idx + 1 < mNodes.size();
   bool hasPrev = (S32)idx - 1 > 0;

   const MeshRoadNode &node = mNodes[idx];   

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

void MeshRoad::calcSliceTransform( U32 idx, MatrixF &mat )
{   
   if ( mSlices.size() - 1 < idx )
      return;

   bool hasNext = idx + 1 < mSlices.size();
   bool hasPrev = (S32)idx - 1 >= 0;

   const MeshRoadSlice &slice = mSlices[idx];   

   VectorF fvec( 0, 1, 0 );

   if ( hasNext )
   {
      fvec = mSlices[idx+1].p1 - slice.p1;      
      fvec.normalizeSafe();
   }
   else if ( hasPrev )
   {
      fvec = slice.p1 - mSlices[idx-1].p1;
      fvec.normalizeSafe();
   }
   else
      fvec = mPerp( slice.normal );

   if ( fvec.isZero() )
      fvec = mPerp( slice.normal );

   F32 dot = mDot( fvec, slice.normal );
   if ( dot < -0.9f || dot > 0.9f )
      fvec = mPerp( slice.normal );

   VectorF rvec = mCross( fvec, slice.normal );
   if ( rvec.isZero() )
      rvec = mPerp( fvec );
   rvec.normalize();

   fvec = mCross( slice.normal, rvec );
   fvec.normalize();

   mat.setColumn( 0, rvec );
   mat.setColumn( 1, fvec );
   mat.setColumn( 2, slice.normal );
   mat.setColumn( 3, slice.p1 );

   AssertFatal( m_matF_determinant( mat ) != 0.0f, "no inverse!");
}

F32 MeshRoad::getRoadLength() const
{
   F32 length = 0.0f;

   for ( U32 i = 0; i < mSegments.size(); i++ )
   {
      length += mSegments[i].length();
   }

   return length;
}

void MeshRoad::deleteNode( U32 idx )
{
   if ( mNodes.size() - 1 < idx )
      return;

   mNodes.erase(idx);   
   _regenerate();

   setMaskBits( RegenMask | NodeMask );
}

U32 MeshRoad::_addNode( const Point3F &pos, const F32 &width, const F32 &depth, const VectorF &normal )
{
   mNodes.increment();
   MeshRoadNode &node = mNodes.last();

   node.point = pos;   
   node.width = width;
   node.depth = depth;
   node.normal = normal;

   setMaskBits( NodeMask | RegenMask );

   return mNodes.size() - 1;
}

U32 MeshRoad::_insertNode( const Point3F &pos, const F32 &width, const F32 &depth, const VectorF &normal, const U32 &idx )
{
   U32 ret;
   MeshRoadNode *node;

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

bool MeshRoad::collideRay( const Point3F &origin, const Point3F &direction, U32 *nodeIdx, Point3F *collisionPnt )
{
   Point3F p0 = origin;
   Point3F p1 = origin + direction * 2000.0f;

   // If the line segment does not collide with the MeshRoad's world box, 
   // it definitely does not collide with any part of the river.
   if ( !getWorldBox().collideLine( p0, p1 ) )
      return false;

   if ( mSlices.size() < 2 )
      return false;

   MathUtils::Quad quad;
   MathUtils::Ray ray;
   F32 t;

   // Check each road segment (formed by a pair of slices) for collision
   // with the line segment.
   for ( U32 i = 0; i < mSlices.size() - 1; i++ )
   {
      const MeshRoadSlice &slice0 = mSlices[i];
      const MeshRoadSlice &slice1 = mSlices[i+1];

      // For simplicities sake we will only test for collision between the
      // line segment and the Top face of the river segment.

      // Clockwise starting with the leftmost/closest point.
      quad.p00 = slice0.p0;
      quad.p01 = slice1.p0;
      quad.p11 = slice1.p2;
      quad.p10 = slice0.p2;

      ray.origin = origin;
      ray.direction = direction;

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

void MeshRoad::regenerate()
{
   _regenerate();
   setMaskBits( RegenMask );
}

//-------------------------------------------------------------------------
// Console Methods
//-------------------------------------------------------------------------

DefineEngineMethod( MeshRoad, setNodeDepth, void, ( S32 idx, F32 meters ),,
                   "Intended as a helper to developers and editor scripts.\n"
                   "Sets the depth in meters of a particular node."
                   )
{
   object->setNodeDepth( idx, meters );
}

DefineEngineMethod( MeshRoad, regenerate, void, (),,
                   "Intended as a helper to developers and editor scripts.\n"
                   "Force MeshRoad to recreate its geometry."
                   )
{
   object->regenerate();
}

DefineEngineMethod( MeshRoad, postApply, void, (),,
                   "Intended as a helper to developers and editor scripts.\n"
                   "Force trigger an inspectPostApply. This will transmit "
                   "material and other fields ( not including nodes ) to client objects."
                   )
{
   object->inspectPostApply();
}