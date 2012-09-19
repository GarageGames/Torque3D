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

#ifndef _DECALROAD_H_
#define _DECALROAD_H_

#ifndef _SCENEOBJECT_H_
#include "scene/sceneObject.h"
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

class Path;
class TerrainBlock;
struct ObjectRenderInst;
class Material;
struct DecalRoadNodeList;


class DecalRoadUpdateEvent : public SimEvent
{
   typedef SimEvent Parent;
public:

   DecalRoadUpdateEvent( U32 mask, U32 ms ) { mMask = mask; mMs = ms; }
   virtual void process( SimObject *object );

   U32 mMask;
   U32 mMs;
};


struct RoadNode
{
   /// The 3D position of the node.
   Point3F point;

   /// The width of the road at this node.
   F32 width;

   /// Alpha of the road at this node.
   //F32 alpha;
};
typedef Vector<RoadNode> RoadNodeVector;

struct RoadEdge
{
   RoadEdge() 
   {
      p0.zero();
      p1.zero();
      p2.zero();

      uvec.zero();
      fvec.zero();
      rvec.zero();

      width = 0.0f;

      parentNodeIdx = -1;
   };

   Point3F p0;
   Point3F p1;
   Point3F p2;      

   VectorF uvec;
   VectorF fvec;
   VectorF rvec;

   F32 width;      

   U32 parentNodeIdx;
};
typedef Vector<RoadEdge> RoadEdgeVector;

struct RoadBatch
{
   U32 startVert;
   U32 endVert;

   U32 startIndex;
   U32 endIndex;

   Box3F bounds;
};
typedef Vector<RoadBatch> RoadBatchVector;


//------------------------------------------------------------------------------
// DecalRoad Class
//------------------------------------------------------------------------------
class DecalRoad : public SceneObject
{
private:

   friend class DecalRoadUpdateEvent;
   friend class GuiRoadEditorCtrl;
   friend class GuiRoadEditorUndoAction;
	typedef SceneObject		Parent;

protected:
   // Internal defines, enums, structs, classes

   struct Triangle
   {
      GFXVertexPT v0, v1, v2;
   };

   enum 
   { 
      DecalRoadMask        = Parent::NextFreeMask,
      NodeMask             = Parent::NextFreeMask << 1,      
      GenEdgesMask         = Parent::NextFreeMask << 2,
      ReClipMask           = Parent::NextFreeMask << 3,
      TerrainChangedMask   = Parent::NextFreeMask << 4,
      NextFreeMask         = Parent::NextFreeMask << 5,
   };   

   #define StepSize_Normal 10.0f  
   #define MIN_METERS_PER_SEGMENT 1.0f

public:

	DecalRoad();
	~DecalRoad();

	DECLARE_CONOBJECT(DecalRoad);

   // ConsoleObject
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
	virtual void prepRenderImage( SceneRenderState* state );
   virtual void setTransform( const MatrixF &mat );
   virtual void setScale( const VectorF &scale );
   virtual bool containsPoint( const Point3F& point ) const { return containsPoint( point, NULL ); } 

   // fxRoad Public Methods
   void scheduleUpdate( U32 updateMask );
   void scheduleUpdate( U32 updateMask, U32 delayMs, bool restartTimer );
   void regenerate();   
   void setTextureLength( F32 meters );
   void setBreakAngle( F32 degrees );

   /// Insert a node anywhere in the road.
   /// Pass idx zero to add to the front and idx U32_MAX to add to the end
   U32 insertNode( const Point3F &pos, const F32 &width, const U32 &idx );

   U32 addNode( const Point3F &pos, F32 width = 10.0f );   
   void deleteNode( U32 idx );

   void buildNodesFromList( DecalRoadNodeList* list );

   bool getClosestNode( const Point3F &pos, U32 &idx );

   bool containsPoint( const Point3F &worldPos, U32 *nodeIdx ) const;
   bool castray( const Point3F &start, const Point3F &end ) const;
   
   Point3F getNodePosition( U32 idx );
   void setNodePosition( U32 idx, const Point3F &pos );

   F32 getNodeWidth( U32 idx );
   void setNodeWidth( U32 idx, F32 width );   

   /// Protected 'Node' Field setter that will add a node to the list.
   static bool addNodeFromField( void *object, const char *index, const char *data );  

   static SimSet* getServerSet();
  
protected:
         
   // Internal Helper Methods   

   void _initMaterial();   
   void _debugRender( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *matInst );

   U32 _insertNode( const Point3F &pos, const F32 &width, const U32 &idx );
   U32 _addNode( const Point3F &pos, F32 width );
   void _generateEdges();
   void _captureVerts();

   bool _getTerrainHeight( Point3F &pos );
   bool _getTerrainHeight( const Point2F &pos, F32 &height );
   bool _getTerrainHeight( const F32 &x, const F32 &y, F32 &height );

   void _onTerrainChanged( U32 type, TerrainBlock* tblock, const Point2I &min, const Point2I &max );

   // static protected field set methods
   static bool ptSetBreakAngle( void *object, const char *index, const char *data );
   static bool ptSetTextureLength( void *object, const char *index, const char *data );
  
protected:

   // Field Vars
   F32 mBreakAngle;
   U32 mSegmentsPerBatch;
   F32 mTextureLength;
   String mMaterialName;
   U32 mRenderPriority;

   // Static ConsoleVars for editor
   static bool smEditorOpen;
   static bool smWireframe;
   static bool smShowBatches;
   static bool smDiscardAll;   
   static bool smShowSpline;
   static bool smShowRoad; 
   static S32 smUpdateDelay;
   
   static SimObjectPtr<SimSet> smServerDecalRoadSet;

   // Other Internal Vars

   RoadEdgeVector mEdges;
   RoadNodeVector mNodes;
   RoadBatchVector mBatches;
   
   bool mLoadRenderData;
   
   SimObjectPtr<Material> mMaterial;
   BaseMatInstance *mMatInst;

   GFXVertexBufferHandle<GFXVertexPNTBT> mVB;
   GFXPrimitiveBufferHandle mPB;

   U32 mTriangleCount;
   U32 mVertCount;

   S32 mUpdateEventId;   
   DecalRoadUpdateEvent *mLastEvent;

   Box3F mTerrainUpdateRect;
};


#endif // _DECALROAD_H_
