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

#ifndef _MESHROAD_H_
#define _MESHROAD_H_

#ifndef _SCENEOBJECT_H_
#include "scene/sceneObject.h"
#endif
#ifndef _GFXTEXTUREHANDLE_H_
#include "gfx/gfxTextureHandle.h"
#endif
#ifndef _GFXVERTEXBUFFER_H_
#include "gfx/gfxVertexBuffer.h"
#endif
#ifndef _GFXPRIMITIVEBUFFER_H_
#include "gfx/gfxPrimitiveBuffer.h"
#endif
#ifndef _CLIPPEDPOLYLIST_H_
#include "collision/clippedPolyList.h"
#endif
#ifndef _MATINSTANCE_H_
#include "materials/matInstance.h"
#endif
#ifndef _CONVEX_H_
#include "collision/convex.h"
#endif


//extern U32 gIdxArray[6][2][3];

struct MeshRoadHitSegment
{
   U32 idx;
   F32 t;
};

class MeshRoad;

//-------------------------------------------------------------------------
// MeshRoadConvex Class
//-------------------------------------------------------------------------

class MeshRoadConvex : public Convex
{
   typedef Convex Parent;
   friend class MeshRoad;

protected:

   MeshRoad *pRoad;

public:

   U32 faceId;
   U32 triangleId;
   U32 segmentId;
   Point3F verts[4];
   PlaneF normal;
   Box3F box;

public:

   MeshRoadConvex() { mType = MeshRoadConvexType; }

   MeshRoadConvex( const MeshRoadConvex& cv ) {
      mType      = MeshRoadConvexType;
      mObject    = cv.mObject;
      pRoad      = cv.pRoad;
      faceId     = cv.faceId;
      triangleId = cv.triangleId;
      segmentId  = cv.segmentId;
      verts[0]   = cv.verts[0];
      verts[1]   = cv.verts[1];
      verts[2]   = cv.verts[2];
      verts[3]   = cv.verts[3];
      normal     = cv.normal;
      box        = cv.box;
   }

   const MatrixF& getTransform() const;
   Box3F getBoundingBox() const;
   Box3F getBoundingBox(const MatrixF& mat, const Point3F& scale) const;
   Point3F support(const VectorF& vec) const;
   void getFeatures(const MatrixF& mat,const VectorF& n, ConvexFeature* cf);
   void getPolyList(AbstractPolyList* list);
};


//-------------------------------------------------------------------------
// MeshRoadSplineNode Class
//-------------------------------------------------------------------------

class Path;
class TerrainBlock;
struct ObjectRenderInst;

class MeshRoadSplineNode
{
public:
   MeshRoadSplineNode() {}

   F32 x;
   F32 y;
   F32 z;
   F32 width;
   F32 depth;
   VectorF normal;

   MeshRoadSplineNode& operator=(const MeshRoadSplineNode&);
   MeshRoadSplineNode operator+(const MeshRoadSplineNode&) const;
   MeshRoadSplineNode operator-(const MeshRoadSplineNode&) const;
   MeshRoadSplineNode operator*(const F32) const;

   F32 len();
   Point3F getPosition() const { return Point3F(x,y,z); };
};

inline F32 MeshRoadSplineNode::len()
{
   return mSqrt(F32(x*x + y*y + z*z));
}

inline MeshRoadSplineNode& MeshRoadSplineNode::operator=(const MeshRoadSplineNode &_node)
{
   x = _node.x;
   y = _node.y;
   z = _node.z;
   width = _node.width;
   depth = _node.depth;
   normal = _node.normal;

   return *this;
}

inline MeshRoadSplineNode MeshRoadSplineNode::operator+(const MeshRoadSplineNode& _add) const
{
   MeshRoadSplineNode result;
   result.x = x + _add.x;
   result.y = y + _add.y;
   result.z = z + _add.z;
   result.width = width + _add.width;
   result.depth = depth + _add.depth;
   result.normal = normal + _add.normal;

   return result;
}


inline MeshRoadSplineNode MeshRoadSplineNode::operator-(const MeshRoadSplineNode& _rSub) const
{
   MeshRoadSplineNode result;
   result.x = x - _rSub.x;
   result.y = y - _rSub.y;
   result.z = z - _rSub.z;
   result.width = width - _rSub.width;
   result.depth = depth - _rSub.depth;
   result.normal = normal - _rSub.normal;

   return result;
}

inline MeshRoadSplineNode operator*(const F32 mul, const MeshRoadSplineNode& multiplicand)
{
   return multiplicand * mul;
}

inline MeshRoadSplineNode MeshRoadSplineNode::operator*(const F32 _mul) const
{
   MeshRoadSplineNode result;
   result.x = x * _mul;
   result.y = y * _mul;
   result.z = z * _mul;
   result.width = width * _mul;
   result.depth = depth * _mul;
   result.normal = normal * _mul;

   return result;
}

//-------------------------------------------------------------------------
// Structures
//-------------------------------------------------------------------------

struct MeshRoadRenderBatch
{
   U32 startSegmentIdx;
   U32 endSegmentIdx;
   U32 startVert;
   U32 endVert;
   U32 startIndex;
   U32 endIndex;
   U32 totalRows;
   U32 indexCount;

   U32 vertCount;
   U32 triangleCount; 
};

typedef Vector<MeshRoadRenderBatch> MeshRoadBatchVector;

struct MeshRoadNode
{
   // The 3D position of the node
   Point3F point;

   // The width of the River at this node (meters)
   F32 width;

   // The depth of the River at this node (meters)
   F32 depth;

   VectorF normal;   
};

typedef Vector<MeshRoadNode> MeshRoadNodeVector;

struct MeshRoadSlice
{
   MeshRoadSlice() 
   {
      p0.zero();
      p1.zero();
      p2.zero();
      pb0.zero();
      pb2.zero();

      uvec.zero();
      fvec.zero();
      rvec.zero();
      
      width = 0.0f;
      depth = 0.0f;
      normal.set(0,0,1);
      texCoordV = 0.0f;

      parentNodeIdx = -1;
   };

   Point3F p0; // upper left
   Point3F p1; // upper center
   Point3F p2; // upper right

   Point3F pb0; // bottom left
   Point3F pb2; // bottom right

   VectorF uvec;
   VectorF fvec;
   VectorF rvec;

   F32 width;    
   F32 depth;
   Point3F normal;

   F32 t;

   F32 texCoordV;

   U32 parentNodeIdx;
};
typedef Vector<MeshRoadSlice> MeshRoadSliceVector;


//-------------------------------------------------------------------------
// MeshRoadSegment Class
//-------------------------------------------------------------------------

class MeshRoadSegment
{
public:

   MeshRoadSegment();
   MeshRoadSegment( MeshRoadSlice *rs0, MeshRoadSlice *rs1, const MatrixF &roadMat );   

   void set( MeshRoadSlice *rs0, MeshRoadSlice *rs1 );

   F32 TexCoordStart() const { return slice0->texCoordV; }
   F32 TexCoordEnd() const { return slice1->texCoordV; }

   const Point3F& getP00() const { return slice0->p0; } 
   const Point3F& getP01() const { return slice1->p0; }
   const Point3F& getP11() const { return slice1->p2; }
   const Point3F& getP10() const { return slice0->p2; }

   Point3F getSurfaceCenter() const { return ( slice0->p1 + slice1->p1 ) / 2.0f; }
   Point3F getSurfaceNormal() const { return -mPlanes[4].getNormal(); }

   bool intersectBox( const Box3F &bounds ) const;
   bool containsPoint( const Point3F &pnt ) const;
   F32 distanceToSurface( const Point3F &pnt ) const;
   F32 length() const { return ( slice1->p1 - slice0->p1 ).len(); }

   // Quick access to the segment's points
   Point3F& operator[](U32);
   const Point3F& operator[](U32) const;
   Point3F& operator[](S32 i)              { return operator[](U32(i)); }
   const Point3F& operator[](S32 i ) const { return operator[](U32(i)); }

   const Box3F& getWorldBounds() const { return worldbounds; }

   MeshRoadSlice *slice0;
   MeshRoadSlice *slice1;

protected:

   PlaneF mPlanes[6];
   U32 mPlaneCount;

   U32 columns;
   U32 rows;

   U32 startVert;
   U32 endVert;
   U32 startIndex;
   U32 endIndex;   

   U32 numVerts;
   U32 numTriangles;

   Box3F objectbounds;
   Box3F worldbounds;
};

typedef Vector<MeshRoadSegment> MeshRoadSegmentVector;


inline Point3F& MeshRoadSegment::operator[](U32 index)
{
   AssertFatal(index < 8, "MeshRoadSegment::operator[] - out of bounds array access!");

   MeshRoadSlice *slice = NULL;
   if ( index > 3 )
   {
      slice = slice1;
      index -= 4;
   }
   else
   {
      slice = slice0;
   }

   if ( index == 0 )
      return slice->p0;

   if ( index == 1 )
      return slice->p2;

   if ( index == 2 )
      return slice->pb0;

   else //( index == 3 )
      return slice->pb2;
}

inline const Point3F& MeshRoadSegment::operator[](U32 index) const
{
   AssertFatal(index < 8, "MeshRoadSegment::operator[] - out of bounds array access!");

   MeshRoadSlice *slice = NULL;
   if ( index > 3 )
   {
      slice = slice1;
      index -= 4;
   }
   else
   {
      slice = slice0;
   }

   if ( index == 0 )
      return slice->p0;

   if ( index == 1 )
      return slice->p2;

   if ( index == 2 )
      return slice->pb0;

   else// ( index == 3 )
      return slice->pb2;
}


//------------------------------------------------------------------------------
// MeshRoad Class
//------------------------------------------------------------------------------

class PhysicsBody;
struct MeshRoadNodeList;

class MeshRoad : public SceneObject
{
private:

   friend class GuiMeshRoadEditorCtrl;
   friend class GuiMeshRoadEditorUndoAction;
   friend class MeshRoadConvex;

   typedef SceneObject		Parent;

   enum 
   { 
      MeshRoadMask      = Parent::NextFreeMask,
      NodeMask          = Parent::NextFreeMask << 1,      
      RegenMask         = Parent::NextFreeMask << 2,
      InitialUpdateMask = Parent::NextFreeMask << 3,
      SelectedMask      = Parent::NextFreeMask << 4,
      MaterialMask      = Parent::NextFreeMask << 5,
      NextFreeMask      = Parent::NextFreeMask << 6,
   };   

public:

   MeshRoad();
   ~MeshRoad();

   DECLARE_CONOBJECT(MeshRoad);

   // ConObject.
   static void initPersistFields();
   static void consoleInit();

   // SimObject      
   bool onAdd();
   void onRemove();
   void onEditorEnable();
   void onEditorDisable();
   void inspectPostApply();
   void onStaticModified(const char* slotName, const char*newValue = NULL);
   void writeFields(Stream &stream, U32 tabStop);
   bool writeField( StringTableEntry fieldname, const char *value );

   // NetObject
   U32 packUpdate(NetConnection *, U32, BitStream *);
   void unpackUpdate(NetConnection *, BitStream *);

   // SceneObject
   virtual void prepRenderImage( SceneRenderState* sceneState );
   virtual void setTransform( const MatrixF &mat );
   virtual void setScale( const VectorF &scale );

   // SceneObject - Collision
   virtual void buildConvex(const Box3F& box,Convex* convex);
   virtual bool buildPolyList(PolyListContext context, AbstractPolyList* polyList, const Box3F &box, const SphereF &sphere);
   virtual bool castRay(const Point3F &start, const Point3F &end, RayInfo* info);
   virtual bool collideBox(const Point3F &start, const Point3F &end, RayInfo* info);

   // MeshRoad
   void regenerate();   
   void setBatchSize( U32 level );
   void setTextureFile( StringTableEntry file );
   void setTextureRepeat( F32 meters );

   bool collideRay( const Point3F &origin, const Point3F &direction, U32 *nodeIdx = NULL, Point3F *collisionPnt = NULL );
   bool buildSegmentPolyList( AbstractPolyList* polyList, U32 startSegIdx, U32 endSegIdx, bool capFront, bool capEnd );

   void buildNodesFromList( MeshRoadNodeList* list );

   U32 insertNode( const Point3F &pos, const F32 &width, const F32 &depth, const Point3F &normal, const U32 &idx );
   U32 addNode( const Point3F &pos, const F32 &width, const F32 &depth, const Point3F &normal );   
   void deleteNode( U32 idx );

   void setNode( const Point3F &pos, const F32 &width, const F32 &depth, const Point3F &normal, const U32 &idx );

   const MeshRoadNode& getNode( U32 idx );
   VectorF getNodeNormal( U32 idx );
   void setNodeNormal( U32 idx, const VectorF &normal );
   Point3F getNodePosition( U32 idx );
   void setNodePosition( U32 idx, const Point3F &pos );
   F32 getNodeWidth( U32 idx );
   void setNodeWidth( U32 idx, F32 width );   
   F32 getNodeDepth( U32 idx );
   void setNodeDepth( U32 idx, F32 depth );
   MatrixF getNodeTransform( U32 idx );
   void calcSliceTransform( U32 idx, MatrixF &mat );
   bool isEndNode( U32 idx ) { return ( mNodes.size() > 0 && ( idx == 0 || idx == mNodes.size() - 1 ) ); }

   U32 getSegmentCount() { return mSegments.size(); }
   const MeshRoadSegment& getSegment( U32 idx ) { return mSegments[idx]; }

   F32 getRoadLength() const;

   static SimSet* getServerSet();

   /// Protected 'Component' Field setter that will add a component to the list.
   static bool addNodeFromField( void *object, const char *index, const char *data );

   static bool smEditorOpen;
   static bool smWireframe;
   static bool smShowBatches;
   static bool smShowSpline;
   static bool smShowRoad;
   static SimObjectPtr<SimSet> smServerMeshRoadSet;   

protected:

   void _initMaterial();

   void _debugRender( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance* );

   U32 _insertNode( const Point3F &pos, const F32 &width, const F32 &depth, const Point3F &normal, const U32 &idx );
   U32 _addNode( const Point3F &pos, const F32 &width, const F32 &depth, const Point3F &normal );

   void _regenerate();
   void _generateSlices();
   void _generateSegments();
   void _generateVerts();   

protected:

   MeshRoadSliceVector mSlices;
   MeshRoadNodeVector mNodes;
   MeshRoadSegmentVector mSegments;
   MeshRoadBatchVector mBatches;
   
   static GFXStateBlockRef smWireframeSB;

   enum { 
      Top = 0, 
      Bottom = 1, 
      Side = 2,
      SurfaceCount = 3
   };

   GFXVertexBufferHandle<GFXVertexPNTT> mVB[SurfaceCount];   
   GFXPrimitiveBufferHandle mPB[SurfaceCount];      

   String mMaterialName[SurfaceCount];   
   SimObjectPtr<Material> mMaterial[SurfaceCount];
   BaseMatInstance *mMatInst[SurfaceCount];

   U32 mVertCount[SurfaceCount];
   U32 mTriangleCount[SurfaceCount];   
      
   // Fields.
   F32 mTextureLength;
   F32 mBreakAngle;
   S32 mWidthSubdivisions;
   
   // Collision and Physics.
   Convex* mConvexList;
   Vector<MeshRoadConvex*> mDebugConvex;
   PhysicsBody *mPhysicsRep;
};


#endif // _MESHROAD_H_