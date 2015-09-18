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

#ifndef _RIVER_H_
#define _RIVER_H_

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
#ifndef _GFXSTATEBLOCK_H_
#include "gfx/gfxStateBlock.h"
#endif
#ifndef _WATEROBJECT_H_
#include "waterObject.h"
#endif



//-------------------------------------------------------------------------
// RiverSplineNode Class
//-------------------------------------------------------------------------

class Path;
class TerrainBlock;
struct ObjectRenderInst;

class RiverSplineNode
{
public:
   RiverSplineNode() {}

   F32 x;
   F32 y;
   F32 z;
   F32 width;
   F32 depth;
   VectorF normal;

   RiverSplineNode& operator=(const RiverSplineNode&);
   RiverSplineNode operator+(const RiverSplineNode&) const;
   RiverSplineNode operator-(const RiverSplineNode&) const;
   RiverSplineNode operator*(const F32) const;

   F32 len();
};

inline F32 RiverSplineNode::len()
{
   return mSqrt(F32(x*x + y*y + z*z));
}

inline RiverSplineNode& RiverSplineNode::operator=(const RiverSplineNode &_node)
{
   x = _node.x;
   y = _node.y;
   z = _node.z;
   width = _node.width;
   depth = _node.depth;
   normal = _node.normal;
   
   return *this;
}

inline RiverSplineNode RiverSplineNode::operator+(const RiverSplineNode& _add) const
{
   RiverSplineNode result;
   result.x = x + _add.x;
   result.y = y + _add.y;
   result.z = z + _add.z;
   result.width = width + _add.width;
   result.depth = depth + _add.depth;
   result.normal = normal + _add.normal;
   
   return result;
}


inline RiverSplineNode RiverSplineNode::operator-(const RiverSplineNode& _rSub) const
{
   RiverSplineNode result;
   result.x = x - _rSub.x;
   result.y = y - _rSub.y;
   result.z = z - _rSub.z;
   result.width = width - _rSub.width;
   result.depth = depth - _rSub.depth;
   result.normal = normal - _rSub.normal;
   
   return result;
}

inline RiverSplineNode operator*(const F32 mul, const RiverSplineNode& multiplicand)
{
   return multiplicand * mul;
}

inline RiverSplineNode RiverSplineNode::operator*(const F32 _mul) const
{
   RiverSplineNode result;
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

struct RiverRenderBatch
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

typedef Vector<RiverRenderBatch> RiverBatchVector;

struct RiverNode
{
   // The 3D position of the node
   Point3F point;

   // The width of the River at this node (meters)
   F32 width;

   // The depth of the River at this node (meters)
   F32 depth;

   VectorF normal;
};

typedef Vector<RiverNode> RiverNodeVector;

struct RiverSlice
{
   RiverSlice() 
   {
      p0.zero();
      p1.zero();
      p2.zero();
      pb0.zero();
      pb2.zero();

      uvec.zero();
      fvec.zero();
      rvec.zero();

      normal.zero();

      width = 0.0f;
      depth = 0.0f;
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

   VectorF normal;

   F32 width;    
   F32 depth;

   F32 texCoordV;

   U32 parentNodeIdx;
};
typedef Vector<RiverSlice> RiverSliceVector;

//-------------------------------------------------------------------------
// RiverSegment Class
//-------------------------------------------------------------------------

static Point3F sSegmentPointCompareReference;

class RiverSegment
{
public:

   RiverSegment();
   RiverSegment( RiverSlice *rs0, RiverSlice *rs1 );   

   void set( RiverSlice *rs0, RiverSlice *rs1 );

   F32 TexCoordStart() { return slice0->texCoordV; }
   F32 TexCoordEnd() { return slice1->texCoordV; }

   Point3F getP00() const { return slice0->p0; } 
   Point3F getP01() const { return slice1->p0; }
   Point3F getP11() const { return slice1->p2; }
   Point3F getP10() const { return slice0->p2; }

   // NOTE:
   // For purposes of collision testing against a RiverSegment we represent
   // it as a set of 6 planes (one for each face) much like a frustum.
   // This is actually a flawed representation that will be more incorrect
   // the more twist/bend exists between the two RiverSlices making up 
   // the segment. Basically we treat the four points that make up a "face"
   // as if they were coplanar.

   Point3F getSurfaceCenter() const { return (slice0->p1 + slice1->p1) / 2; }
   Point3F getSurfaceNormal() const { return ( -mPlanes[4].getNormal() ); }
   Point3F getFaceCenter( U32 faceIdx ) const;

   bool intersectBox( const Box3F &bounds ) const;
   bool containsPoint( const Point3F &pnt ) const;
   F32 distanceToSurface( const Point3F &pnt ) const;

	// Quick access to the segment's points
	Point3F& operator[](U32);
	const Point3F& operator[](U32) const;
	Point3F& operator[](S32 i)              { return operator[](U32(i)); }
	const Point3F& operator[](S32 i ) const { return operator[](U32(i)); }
   
   RiverSlice *slice0;
   RiverSlice *slice1;

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

   Box3F worldbounds;

protected:

   PlaneF _getBestPlane( const Point3F *p0, const Point3F *p1, const Point3F *p2, const Point3F *p3 );
};
typedef Vector<RiverSegment> RiverSegmentVector;

inline Point3F& RiverSegment::operator[](U32 index)
{
	AssertFatal(index < 8, "RiverSegment::operator[] - out of bounds array access!");

	RiverSlice *slice = NULL;
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

inline const Point3F& RiverSegment::operator[](U32 index) const
{
	AssertFatal(index < 8, "RiverSegment::operator[] - out of bounds array access!");

	RiverSlice *slice = NULL;
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
// River Class
//------------------------------------------------------------------------------

class ParticleEmitter;
class ParticleEmitterData;
struct RiverNodeList;

class River : public WaterObject
{
private:

   friend class GuiRiverEditorCtrl;
   friend class GuiRiverEditorUndoAction;

   typedef WaterObject Parent;      

protected:

   enum 
   { 
      RiverMask       = Parent::NextFreeMask,
      NodeMask          = Parent::NextFreeMask << 1,      
      RegenMask         = Parent::NextFreeMask << 2,
      InitialUpdateMask = Parent::NextFreeMask << 3,
      SelectedMask      = Parent::NextFreeMask << 4,
      MaterialMask      = Parent::NextFreeMask << 5,
      NextFreeMask      = Parent::NextFreeMask << 6,
   };  

public:   

   River();
   ~River();

   DECLARE_CONOBJECT(River);
   
   // ConObject.
   static void initPersistFields();
   static void consoleInit();

   // SimObject      
   bool onAdd();
   void onRemove();
   void inspectPostApply();
   void onStaticModified(const char* slotName, const char*newValue = NULL);
   void writeFields(Stream &stream, U32 tabStop);
   bool writeField( StringTableEntry fieldname, const char *value );

   // NetObject
   U32 packUpdate(NetConnection *, U32, BitStream *);
   void unpackUpdate(NetConnection *, BitStream *);

   // SceneObject   
   virtual void setTransform( const MatrixF &mat );
   virtual void setScale( const VectorF &scale );
	virtual bool castRay(const Point3F &start, const Point3F &end, RayInfo* info);
	virtual bool collideBox(const Point3F &start, const Point3F &end, RayInfo* info);
   virtual bool containsPoint( const Point3F& point ) const { return containsPoint( point, NULL ); }
   virtual bool buildPolyList( PolyListContext context, AbstractPolyList* polyList, const Box3F& box, const SphereF& sphere );

   // WaterObject
   virtual F32 getWaterCoverage( const Box3F &worldBox ) const;   
   virtual F32 getSurfaceHeight( const Point2F &pos ) const;
   virtual VectorF getFlow( const Point3F &pos ) const;   
   virtual void onReflectionInfoChanged();
   virtual void updateUnderwaterEffect( SceneRenderState *state );
   
   virtual bool isUnderwater( const Point3F &pnt ) const;
   F32 distanceToSurface( const Point3F &pnt, U32 segmentIdx );
   bool containsPoint( const Point3F &worldPos, U32 *nodeIdx ) const;

   // Cast a ray against the river -- THE TOP SURFACE ONLY
   bool collideRay( const Point3F &origin, const Point3F &direction, U32 *nodeIdx = NULL, Point3F *collisionPnt = NULL ) const;

   void regenerate();
   void setMetersPerSegment( F32 meters );
   void setBatchSize( U32 level );
   void setShowNodes( bool enabled );
   void setMaxDivisionSize( F32 meters );
	void setColumnCount( S32 count );

   U32 insertNode( const Point3F &pos, const F32 &width, const F32 &depth, const VectorF &normal, const U32 &idx );
   U32 addNode( const Point3F &pos, const F32 &width, const F32 &depth, const VectorF &normal );   
   void setNode( const Point3F &pos, const F32 &width, const F32 &depth, const VectorF &normal, const U32 &idx );
   void deleteNode( U32 idx );

   void buildNodesFromList( RiverNodeList* list );

   bool getClosestNode( const Point3F &pos, U32 &idx ) const;

   Point3F getNodePosition( U32 idx ) const;
   void setNodePosition( U32 idx, const Point3F &pos );

   MatrixF getNodeTransform( U32 idx ) const;

   F32 getNodeWidth( U32 idx ) const;
   void setNodeWidth( U32 idx, F32 width );   

   F32 getNodeDepth( U32 idx ) const;
   void setNodeDepth( U32 idx, F32 depth );

   void setNodeHeight( U32 idx, F32 height );

	void setNodeNormal( U32 idx, const VectorF &normal );
   VectorF getNodeNormal( U32 idx ) const;   

   /// Protected 'Component' Field setter that will add a component to the list.
   static bool addNodeFromField( void *object, const char *index, const char *data );

   static SimSet* getServerSet();

   static bool smEditorOpen;
   static bool smWireframe;
   static bool smShowWalls;
   static bool smShowNodes;
   static bool smShowSpline;
   static bool smShowRiver;
   static SimObjectPtr<SimSet> smServerRiverSet;

protected:

   void _loadRenderData();
   bool _setRenderData();

   void _render( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *matInst );

   void _makeRenderBatches( const Point3F &cameraPos );
   void _makeHighLODBuffers();

   U32 _insertNode( const Point3F &pos, const F32 &width, const F32 &depth, const VectorF &normal, const U32 &idx );
   U32 _addNode( const Point3F &pos, const F32 &width, const F32 &depth, const VectorF &normal );

   void _regenerate();
   void _generateSlices();
   void _generateSegments();
   void _generateVerts();

   bool _getTerrainHeight( const Point2F &pos, F32 &height );
   bool _getTerrainHeight( F32 x, F32 y, F32 &height ); 

   // WaterObject
   virtual void setShaderParams( SceneRenderState *state, BaseMatInstance *mat, const WaterMatParams &paramHandles );   
   virtual void innerRender( SceneRenderState *state );
   virtual void _getWaterPlane( const Point3F &camPos, PlaneF &outPlane, Point3F &outPos );

protected:

   RiverSliceVector mSlices;

   RiverNodeVector mNodes;

   RiverSegmentVector mSegments;

   GFXVertexBufferHandle<GFXWaterVertex> mVB_high;  // the high detail vertex buffer
   GFXVertexBufferHandle<GFXWaterVertex> mVB_low;   // the low detail vertex buffer

   GFXPrimitiveBufferHandle mPB_high;	// the high detail prim buffer
   GFXPrimitiveBufferHandle mPB_low;	// the low detail prim buffer  

   RiverBatchVector mHighLODBatches;
   RiverBatchVector mLowLODBatches;

   U32 mLowVertCount;
   U32 mHighVertCount;

   U32 mLowTriangleCount;   
   U32 mHighTriangleCount;

   // Fields.
   U32 mSegmentsPerBatch;
   F32 mMetersPerSegment;
   F32 mDepthScale;
   F32 mFlowMagnitude;
   F32 mLodDistance;

   // Divide segments that are greater than this length
   // into sections of this size, or as close as possible
   // while maintaining an equal division size.
   F32 mMaxDivisionSize;
   F32 mMinDivisionSize;
   U32 mColumnCount;    
};

#endif // _RIVER_H_