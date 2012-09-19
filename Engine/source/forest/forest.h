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

#ifndef _H_FOREST_
#define _H_FOREST_

#ifndef _FORESTITEM_H_
   #include "forest/forestItem.h"
#endif
#ifndef _FORESTDATAFILE_H_
   #include "forest/forestDataFile.h"
#endif
#ifndef _MATHUTIL_FRUSTUM_H_  
   #include "math/util/frustum.h"
#endif
#ifndef _GFXTEXTUREHANDLE_H_
   #include "gfx/gfxTextureHandle.h"
#endif
#ifndef _COLLISION_H_
   #include "collision/collision.h"
#endif
#ifndef _SCENEOBJECT_H_
   #include "scene/sceneObject.h"
#endif
#ifndef _SIGNAL_H_
   #include "core/util/tSignal.h"
#endif
#ifndef __RESOURCE_H__
   #include "core/resource.h"
#endif
#ifndef _CONVEX_H_
   #include "collision/convex.h"
#endif


class TSShapeInstance;
class Forest;
class ForestItemData;
class ForestData;
class GBitmap;
class IForestCollision;
class IForestMask;
class PhysicsBody;
struct ObjectRenderInst;
struct TreePlacementInfo;
class ForestRayInfo;
class SceneZoneSpaceManager;


struct TreeInfo
{
   U32                     treeId;   
   SimObjectId             treeTypeId;
   F32                     distance;
   SimObjectId             forestId;
   Point3F                 position;
};

typedef Signal<void( Forest *forest )> ForestCreatedSignal;


/// 
class Forest : public SceneObject
{
   friend class CreateForestEvent;
   friend class ForestConvex;

protected:

   typedef SceneObject Parent;
   
   /// Collision and Physics
   /// @{

   Convex* mConvexList;   
   
   /// @}

   /// The name of the planting data file.
   StringTableEntry mDataFileName;

   /// The forest data file which defines planting.
   Resource<ForestData> mData;

   /// Used to scale the tree LODs when rendering into 
   /// reflections.  It should be greater or equal to 1.
   F32 mReflectionLodScalar;

   /// Set when rezoning of forest cells is required.
   bool mZoningDirty;

   /// Debug helpers.
   static bool smForceImposters;
   static bool smDisableImposters;
   static bool smDrawCells;
   static bool smDrawBounds;

   ///
   bool mRegen;

   enum MaskBits
   {
      MediaMask         = Parent::NextFreeMask << 1,
      LodMask           = Parent::NextFreeMask << 2,
      NextFreeMask      = Parent::NextFreeMask << 3
   };


   static U32  smTotalCells;
   static U32  smCellsRendered;
   static U32  smCellItemsRendered;
   static U32  smCellsBatched;
   static U32  smCellItemsBatched;
   static F32  smAverageItemsPerCell;

   static void _clearStats(bool);

   void _renderCellBounds( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat );

   void _onZoningChanged( SceneZoneSpaceManager *zoneManager );

   static ForestCreatedSignal smCreatedSignal;
   static ForestCreatedSignal smDestroyedSignal;

public:
   
   static ForestCreatedSignal& getCreatedSignal() { return smCreatedSignal; }
   static ForestCreatedSignal& getDestroyedSignal() { return smDestroyedSignal; }

   Forest();
   virtual ~Forest();

   DECLARE_CONOBJECT(Forest);
   static void consoleInit();
   static void initPersistFields();   

   // SimObject
   bool onAdd();
   void onRemove();

   /// Overloaded from SceneObject to properly update
   /// the client side forest when changes occur within
   /// the mission editor.
   void inspectPostApply();

   /// Overloaded from SceneObject for updating the 
   /// client side position of the forest.
   void setTransform( const MatrixF &mat );

   void prepRenderImage( SceneRenderState *state );

   bool isTreeInRange( const Point2F& point, F32 radius ) const;

   // Network
   U32 packUpdate( NetConnection *conn, U32 mask, BitStream *stream );
   void unpackUpdate( NetConnection *conn, BitStream *stream );

   //IForestCollision *getCollision() const { return mCollision; }

   // SceneObject - Collision
   virtual void buildConvex( const Box3F& box, Convex* convex );
   virtual bool buildPolyList( PolyListContext context, AbstractPolyList* polyList, const Box3F &box, const SphereF &sphere );
   virtual bool castRay( const Point3F &start, const Point3F &end, RayInfo *outInfo );
   virtual bool castRayRendered( const Point3F &start, const Point3F &end, RayInfo *outInfo );
   virtual bool collideBox( const Point3F &start, const Point3F &end, RayInfo *outInfo );

   // SceneObject - Other
   virtual void applyRadialImpulse( const Point3F &origin, F32 radius, F32 magnitude );

   bool castRayBase( const Point3F &start, const Point3F &end, RayInfo *outInfo, bool rendered );
     
   const Resource<ForestData>& getData() const { return mData; }

   Resource<ForestData>& getData() { return mData; }

   //bool  buildPolyList(AbstractPolyList* polyList, const Box3F &box, const SphereF& sphere);
   //void  buildConvex(const Box3F& box, Convex* convex);

   void getLocalWindTrees( const Point3F &camPos, F32 radius, Vector<TreePlacementInfo> *placementInfo );

   /// Called to create a new empty planting data file and
   /// assign it to this forest.
   void createNewFile();

   ///
   void saveDataFile( const char *path = NULL );

   ///
   void clear() { mData->clear(); }

   /// Called to rebuild the collision state.
   void updateCollision();
};

#endif // _H_FOREST_