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

#ifndef _INTERIOR_H_
#define _INTERIOR_H_

#ifndef _COLOR_H_
#include "core/color.h"
#endif
#ifndef _COLLISION_H_
#include "collision/collision.h"
#endif
#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif
#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif
#ifndef _MPLANE_H_
#include "math/mPlane.h"
#endif
#ifndef _MBOX_H_
#include "math/mBox.h"
#endif
#ifndef _MSPHERE_H_
#include "math/mSphere.h"
#endif
#ifndef _CONVEX_H_
#include "collision/convex.h"
#endif
#ifndef _INTERIORLMMANAGER_H_
#include "interior/interiorLMManager.h"
#endif
#ifndef _INTERIORSIMPLEMESH_H_
#include "interior/interiorSimpleMesh.h"
#endif
#ifndef _OPTIMIZEDPOLYLIST_H_
#include "collision/optimizedPolyList.h"
#endif
#ifndef _SCENEDATA_H_
#include "materials/sceneData.h"
#endif

//-------------------------------------- Forward declarations
class MatInstance;
class Stream;
class EditGeometry;
class InteriorInstance;
class GBitmap;
class RectD;
class SphereF;
class MatrixF;
class SceneRenderState;
class MaterialList;
class AbstractPolyList;
class InteriorSubObject;
class TranslucentSubObject;
class BitVector;
struct RayInfo;
struct EdgeList;
class SurfaceHash;
class InteriorPolytope;
class LightInfo;
class PlaneRange;
class EditInteriorResource;
class GFXVertexBuffer;
class GFXPrimitiveBuffer;
struct RenderInst;
struct GFXVertexPNTTB;
struct GFXPrimitiveInfo;
struct MaterialFeatureData;


//--------------------------------------------------------------------------
class InteriorConvex : public Convex
{
   typedef Convex Parent;
   friend class Interior;
   friend class InteriorInstance;

  protected:
   Interior* pInterior;
  public:
   S32       hullId;
   Box3F     box;

  public:
   InteriorConvex() { mType = InteriorConvexType; }
   InteriorConvex(const InteriorConvex& cv) 
   {
      mObject   = cv.mObject;
      pInterior = cv.pInterior;
      hullId    = cv.hullId;
      box       = box;
   }

   Box3F getBoundingBox() const;
   Box3F getBoundingBox(const MatrixF& mat, const Point3F& scale) const;
   Point3F      support(const VectorF& v) const;
   void         getFeatures(const MatrixF& mat,const VectorF& n, ConvexFeature* cf);
   void         getPolyList(AbstractPolyList* list);
};

class ZoneVisDeterminer
{
   enum Mode 
   {
      FromState,
      FromRects
   };

   Mode mMode;

   SceneRenderState* mState;
   U32         mZoneRangeOffset;
   U32         mParentZone;

  public:
   ZoneVisDeterminer() : mMode(FromRects), mState(NULL) { }

   void runFromState(SceneRenderState*, U32, U32);
   void runFromRects(SceneRenderState*, U32, U32);

   bool isZoneVisible(const U32) const;
};


struct ItrPaddedPoint
{
   Point3F point;
   union 
   {
      F32   fogCoord;
      U8    fogColor[4];
   };
};


//------------------------------------------------------------------------------
//-------------------------------------- CLASS NOTES
// Interior: Base for all interior geometries.  Contains all lighting, poly,
//             portal zone, bsp info, etc. to render an interior.
//
// Internal Structure Notes:
//    IBSPNode:
//     planeIndex: Obv.
//     frontIndex/backIndex: Top bit indicates if children are leaves.
//                            Next bit indicates if leaf children are solid.
//
//    IBSPLeafSolid:
//     planeIndex: obv.
//     surfaceIndex/surfaceCount: Polys that are on the faces of this leaf.  Only
//                                 used for collision/surface info detection.
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
class Interior
{
   friend class EditGeometry;
   friend class InteriorInstance;   
   friend class InteriorProxy;
   friend class blInteriorProxy;
   friend class TranslucentSubObject;
   friend class MirrorSubObject;
   friend class InteriorConvex;
   friend class EditInteriorResource;
   friend class InteriorLMManager;
   friend class TSShape;
   friend class SceneLighting;

   //-------------------------------------- Public interfaces
  public:
   Interior();
   ~Interior();


private:   
   U32 mLightMapBorderSize;

public:
   Vector<S32> surfaceZones;
   U32 getLightMapBorderSize() const {return mLightMapBorderSize;}
   void setLightMapBorderSize(U32 value) {mLightMapBorderSize = value;}
   void buildSurfaceZones();
   //void sgSetupLighting(InteriorInstance *intInst, SceneData &sgData);
   //bool sgRenderLights(InteriorInstance *intInst, SceneData &sgData);

   // Interior Render StateBlock
   GFXStateBlockRef mInteriorSB;
	
	MaterialList*           mMaterialList;
   
   // Misc
   U32          getDetailLevel() const;
   U32          getMinPixels() const;
   const Box3F& getBoundingBox() const;
   S32          getNumZones() const;

   // Rendering
   bool prepForRendering(const char* path);

   bool traverseZones(  SceneCullingState*    state,
                        const Frustum& frustum,
                        S32            containingZone,
                        S32            baseZone,
                        U32            zoneOffset,
                        const MatrixF& OSToWS,
                        const Point3F& objScale,
                        const bool     dontRestrictOutside,
                        const bool     flipClipPlanes,
                        Frustum&       outFrustum );

   void prepBatchRender(  InteriorInstance *intInst, SceneRenderState *state );

   void setupRenderStates();


   bool scopeZones(const S32            baseZone,
                   const Point3F&       interiorRoot,
                   bool*                interiorScopingState);

   ZoneVisDeterminer setupZoneVis( InteriorInstance *intInst, SceneRenderState *state );

   SceneData setupSceneGraphInfo( InteriorInstance *intInst,
                                       SceneRenderState *state );

   void setupRender( InteriorInstance *intInst,
                     SceneRenderState *state,
                     MeshRenderInst *coreRi,
                     const MatrixF* worldToCamera);
	

   //-------------------------------------- Collision Interface and zone scans
  public:
   bool scanZones(const Box3F&, const MatrixF&, U16* zones, U32* numZones);
   bool castRay(const Point3F&, const Point3F&, RayInfo*);
   bool buildPolyList(AbstractPolyList*, const Box3F&, const MatrixF&, const Point3F&);

   bool getIntersectingHulls(const Box3F&, U16* hulls, U32* numHulls);
   bool getIntersectingVehicleHulls(const Box3F&, U16* hulls, U32* numHulls);

protected:
   bool castRay_r(const U32, const U16, const Point3F&, const Point3F&, RayInfo*);
   void buildPolyList_r(InteriorPolytope& polytope,
                        SurfaceHash& hash);
   void scanZone_r(const U32      node,
                   const Point3F& center,
                   const Point3F& axisx,
                   const Point3F& axisy,
                   const Point3F& axisz,
                   U16*           zones,
                   U32*           numZones);
   void scanZoneNew(InteriorPolytope& polytope,
                    U16*           zones,
                    U32*           numZones);


   /// @name Lightmap Support
   /// @{
   static void _enableLightMapFeature( ProcessedMaterial *mat,
                                       U32 stageNum,
                                       MaterialFeatureData &fd, 
                                       const FeatureSet &features );
   /// @}

   //-------------------------------------- Global rendering control
public:
   enum RenderModes 
   {
      NormalRender            = 0,
      NormalRenderLines       = 1,
      ShowDetail              = 2,
      ShowAmbiguous           = 3,
      ShowOrphan              = 4,
      ShowLightmaps           = 5,
      ShowTexturesOnly        = 6,
      ShowPortalZones         = 7,
      ShowOutsideVisible      = 8,
      ShowCollisionFans       = 9,
      ShowStrips              = 10,
      ShowNullSurfaces        = 11,
      ShowLargeTextures       = 12,
      ShowHullSurfaces        = 13,
      ShowVehicleHullSurfaces = 14,
      ShowVertexColors        = 15,
      ShowDetailLevel         = 16
   };

   enum Constants 
   {
      NumCoordBins   = 16,

      BinsXY         = 0,
      BinsXZ         = 1,
      BinsYZ         = 2
   };

   static U32  smRenderMode;
   static bool smFocusedDebug;
   static bool smUseVertexLighting;
   static U32  smFileVersion;
   static bool smLightingCastRays;
   static bool smLightingBuildPolyList;

   //-------------------------------------- Persistence interface
  public:
   bool read(Stream& stream);
   bool write(Stream& stream) const;

   bool readVehicleCollision(Stream& stream);
   bool writeVehicleCollision(Stream& stream) const;

  private:
   bool writePlaneVector(Stream&) const;
   bool readPlaneVector(Stream&);
   bool readLMapTexGen(Stream&, PlaneF&, PlaneF&);
   bool writeLMapTexGen(Stream&, const PlaneF&, const PlaneF&) const;
   void setupTexCoords();
   void setupZonePlanes();

   //-------------------------------------- For morian only...
  public:
   void processHullPolyLists();
   void processVehicleHullPolyLists();

   //-------------------------------------- BSP Structures
  private:
   struct IBSPNode 
   {
      U16 planeIndex;
      U32 frontIndex;
      U32 backIndex;

      U16 terminalZone;   // if high bit set, then the lower 15 bits are the zone
                          //  of any of the subsidiary nodes.  Note that this is
                          //  going to overestimate some, since an object could be
                          //  completely contained in solid, but it's probably
                          //  going to turn out alright.
   };
   struct IBSPLeafSolid 
   {
      U32 surfaceIndex;
      U16 surfaceCount;
   };

   bool isBSPLeafIndex(U32 index) const;
   bool isBSPSolidLeaf(U32 index) const;
   bool isBSPEmptyLeaf(U32 index) const;
   U16  getBSPSolidLeafIndex(U32 index) const;
   U16  getBSPEmptyLeafZone(U32 index) const;

   void setupAveTexGenLength();

   void truncateZoneTree();
   void truncateZoneNode(const U32);
   bool getUnifiedZone(const U32, S32*);

  public:
   static U16  getPlaneIndex(U16 index);
   static bool planeIsFlipped(U16 index);
   const PlaneF& getPlane(U16 index) const;
   PlaneF getFlippedPlane(const U16 index) const;

   const Point3F getPointNormal(const U32 surfaceIndex, const U32 pointOffset) const;

  private:
   bool areEqualPlanes(U16, U16) const;

   bool isNullSurfaceIndex(const U32 index) const;
   bool isVehicleNullSurfaceIndex(const U32 index) const;
   U32  getNullSurfaceIndex(const U32 index) const;
   U32  getVehicleNullSurfaceIndex(const U32 index) const;

   //-------------------------------------- Portals and Zone structures
  private:
   struct Zone 
   {
      U16 portalStart;
      U16 portalCount;

      U32 surfaceStart;
      U32 planeStart;

      U16 surfaceCount;
      U16 planeCount;

      U32 staticMeshStart;
      U32 staticMeshCount;

      U16 flags;
      U16 zoneId;       // This is ephemeral, not persisted out.
   };

   struct Portal 
   {
      U16 planeIndex;

      U16 triFanCount;
      U32 triFanStart;    // portals can have multiple windings

      U16 zoneFront;
      U16 zoneBack;
   };

   //-------------------------------------- Poly/Surface structures
public:
   enum SurfaceFlags 
   {
      SurfaceDetail         = BIT(0),
      SurfaceAmbiguous      = BIT(1),
      SurfaceOrphan         = BIT(2),
      SurfaceSharedLMaps    = BIT(3),     // Indicates that the alarm and normal states share a lightmap (for mission lighter)
      SurfaceOutsideVisible = BIT(4),
      SurfaceStaticMesh     = BIT(5),     // This surface belongs to a static mesh collision hull
      SurfaceFlagMask       = (SurfaceDetail      |
                               SurfaceAmbiguous   |
                               SurfaceOrphan      |
                               SurfaceSharedLMaps |
                               SurfaceOutsideVisible |
                               SurfaceStaticMesh)
   };
   enum ZoneFlags 
   {
      ZoneInside = BIT(0)
   };

   const bool isSurfaceOutsideVisible(U32 surface) const;

  public:
   struct TexMatrix
   {
      S32 T;
      S32 N;
      S32 B;

      TexMatrix()
      :  T( -1 ),
         N( -1 ),
         B( -1 )
      {};
   };
      
   struct Edge
   {
      S32 vertexes[2];
      S32 faces[2];
   };
      
   struct TexGenPlanes 
   {
      PlaneF planeX;
      PlaneF planeY;
   };

   struct TriFan 
   {
      U32 windingStart;
      U32 windingCount;
   };

   struct Surface 
   {
      U32 windingStart;          // 1

      U16 planeIndex;            // 2
      U16 textureIndex;

      U32 texGenIndex;           // 3

      U16 lightCount;            // 4
      U8  surfaceFlags;
      U32 windingCount;

      U32 fanMask;               // 5

      U32 lightStateInfoStart;   // 6

      U32 mapOffsetX;            // 7
      U32 mapOffsetY;
      U32 mapSizeX;
      U32 mapSizeY;

      Point3F T,B,N;
      Point3F normal;

	  //U32 VBIndexStart;
	  //U32 primIndex;
	  GFXPrimitive surfaceInfo;
	  
      bool unused;		
   };

   struct NullSurface 
   {
      U32 windingStart;

      U16 planeIndex;
      U8  surfaceFlags;
      U32 windingCount;
   };

   //-------------------------------------- Animated lighting structures
   enum LightFlags 
   {
      AnimationAmbient  = BIT(0),
      AnimationLoop     = BIT(1),
      AnimationFlicker  = BIT(2),
      AnimationTypeMask = BIT(3) - 1,

      AlarmLight        = BIT(3)
   };

   enum LightType 
   {
      AmbientLooping     = AnimationAmbient | AnimationLoop,
      AmbientFlicker     = AnimationAmbient | AnimationFlicker,

      TriggerableLoop    = AnimationLoop,
      TriggerableFlicker = AnimationFlicker,
      TriggerableRamp    = 0
   };

private:
   bool readSurface(Stream&, Surface&, TexGenPlanes&, const bool);

  public:
   // this is public because tools/Morian needs this defination
   struct AnimatedLight {
      U32 nameIndex;   // Light's name
      U32 stateIndex;  // start point in the state list

      U16 stateCount;  // number of states in this light
      U16 flags;       // flags (Apply AnimationTypeMask to get type)

      U32 duration;    // total duration of animation (ms)
   };
  private:
   struct LightState {
      U8  red;                // state's color
      U8  green;
      U8  blue;
      U8  _color_padding_;

      U32 activeTime;         // Time (ms) at which this state becomes active

      U32 dataIndex;          // StateData count and index for this state
      U16 dataCount;

      U16 __32bit_padding__;
   };
   struct LightStateData {
      U32   surfaceIndex;     // Surface affected by this data
      U32   mapIndex;         // Index into StateDataBuffer (0xFFFFFFFF indicates none)
      U16   lightStateIndex;  // Entry to modify in InteriorInstance
      U16   __32bit_padding__;
   };

   // convex hull collision structures...
  protected:
   struct ConvexHull 
   {
      F32   minX;
      F32   maxX;
      F32   minY;
      F32   maxY;

      F32   minZ;
      F32   maxZ;
      U32   hullStart;
      U32   surfaceStart;

      U32   planeStart;
      U16   hullCount;
      U16   surfaceCount;
      U32   polyListPlaneStart;

      U32   polyListPointStart;
      U32   polyListStringStart;
      U16   searchTag;
      bool  staticMesh;
   };

   struct CoordBin 
   {
      U32   binStart;
      U32   binCount;
   };

   struct RenderNode
   {
      bool  exterior;
      U16   baseTexIndex;
      U8    lightMapIndex;
      S32   primInfoIndex;
      BaseMatInstance *matInst;

      RenderNode()
      {
         exterior = true;
         baseTexIndex = 0;
         lightMapIndex = U8(-1);
         primInfoIndex = -1;
         matInst = NULL;
      }
   };

   
  // public because InteriorInstance needs access
  public:   
   Vector< PlaneF > mReflectPlanes;

  protected:
   struct ReflectRenderNode
   {
      bool  exterior;
      S32   reflectPlaneIndex;
      S32   primInfoIndex;
      U8    lightMapIndex;

      BaseMatInstance *matInst;


      ReflectRenderNode()
      {
         exterior = true;
         reflectPlaneIndex = -1;
         lightMapIndex = U8(-1);
         primInfoIndex = -1;
         matInst = NULL;
      }
   };


   struct ZoneRNList
   {
      Vector<RenderNode> renderNodeList;
   };

   struct ZoneReflectRNList
   {
      Vector<ReflectRenderNode> reflectList;
   };

  // needs to be exposed in order for on the fly changes
  public:
	  Vector<ZoneRNList>                     mZoneRNList;

  private:

   // reflective plane data
   GFXVertexBufferHandle<GFXVertexPNTTB> mReflectVertBuff;
   GFXPrimitiveBufferHandle               mReflectPrimBuff;
   Vector<ZoneReflectRNList>              mZoneReflectRNList;

   // standard interior data
   GFXVertexBufferHandle<GFXVertexPNTTB> mVertBuff;
   GFXPrimitiveBufferHandle               mPrimBuff;

  private:
   LM_HANDLE               mLMHandle;
  public:
   LM_HANDLE getLMHandle() {return(mLMHandle);}

  public:

   // SceneLighting::InteriorProxy interface
   const Surface & getSurface(const U32 surface) const;
   const U32 getSurfaceCount() const;
   const U32 getNormalLMapIndex(const U32 surface) const;
   const U32 getAlarmLMapIndex(const U32 surface) const;
   const U32 getStaticMeshCount() const;
   const InteriorSimpleMesh *getStaticMesh(const U32 index) const;
   const U32 getWinding(const U32 index) const;
   const Point3F & getPoint(const U32 index) const;
   const TexGenPlanes & getLMTexGenEQ(const U32 index) const;
   const TexGenPlanes & getTexGenEQ(const U32 index) const;
   bool hasAlarmState() const;
   const U32 getWindingCount() const;
	S32 getTargetCount() const;
	const String& getTargetName( S32 mapToNameIndex ) const;

   //-------------------------------------- Instance Data Members
  private:
   U32                     mFileVersion;
   U32                     mDetailLevel;
   U32                     mMinPixels;
   F32                     mAveTexGenLength;     // Set in Interior::read after loading the texgen planes.
   Box3F                   mBoundingBox;
   SphereF                 mBoundingSphere;

   Vector<PlaneF>          mPlanes;
   Vector<ItrPaddedPoint>  mPoints;
   Vector<U8>              mPointVisibility;

   Vector<Point3F>         mNormals;
   Vector<TexMatrix>       mTexMatrices;
   Vector<U32>             mTexMatIndices;
   
   ColorF                  mBaseAmbient;
   ColorF                  mAlarmAmbient;

   Vector<IBSPNode>        mBSPNodes;
   Vector<IBSPLeafSolid>   mBSPSolidLeaves;

   bool                    mPreppedForRender;

   bool                    mHasTranslucentMaterials;

   Vector<U32>             mWindings;

   Vector<TexGenPlanes>    mTexGenEQs;
   Vector<TexGenPlanes>    mLMTexGenEQs;

   Vector<TriFan>          mWindingIndices;
   Vector<Surface>         mSurfaces;
   Vector<NullSurface>     mNullSurfaces;
   Vector<U32>             mSolidLeafSurfaces;

   Vector<Edge>            mEdges;

   // Portals and zones
   Vector<Zone>            mZones;
   Vector<U16>             mZonePlanes;
   Vector<U16>             mZoneSurfaces;
   Vector<U16>             mZonePortalList;
   Vector<Portal>          mPortals;
   Vector<U32>             mZoneStaticMeshes;

   // Subobjects: Doors, translucencies, mirrors, etc.
   Vector<InteriorSubObject*> mSubObjects;

   // Lighting info
   bool                    mHasAlarmState;
   U32                     mNumLightStateEntries;

   Vector<GBitmap*>        mLightmaps;
   Vector<bool>            mLightmapKeep;
   Vector<U32>             mNormalLMapIndices;
   Vector<U32>             mAlarmLMapIndices;

   U32                     mNumTriggerableLights;        // Note: not persisted

   // Persistent animated light structures
   Vector<AnimatedLight>   mAnimatedLights;
   Vector<LightState>      mLightStates;
   Vector<LightStateData>  mStateData;
   Vector<U8>              mStateDataBuffer;

   Vector<char>            mNameBuffer;

   Vector<ConvexHull>      mConvexHulls;
   Vector<U8>              mConvexHullEmitStrings;
   Vector<U32>             mHullIndices;
   Vector<U32>             mHullEmitStringIndices;
   Vector<U32>             mHullSurfaceIndices;
   Vector<U16>             mHullPlaneIndices;
   Vector<U16>             mPolyListPlanes;
   Vector<U32>             mPolyListPoints;
   Vector<U8>              mPolyListStrings;
   CoordBin                mCoordBins[NumCoordBins * NumCoordBins];
   Vector<U16>             mCoordBinIndices;
   U32                     mCoordBinMode;

   Vector<ConvexHull>      mVehicleConvexHulls;
   Vector<U8>              mVehicleConvexHullEmitStrings;
   Vector<U32>             mVehicleHullIndices;
   Vector<U32>             mVehicleHullEmitStringIndices;
   Vector<U32>             mVehicleHullSurfaceIndices;
   Vector<U16>             mVehicleHullPlaneIndices;
   Vector<U16>             mVehiclePolyListPlanes;
   Vector<U32>             mVehiclePolyListPoints;
   Vector<U8>              mVehiclePolyListStrings;
   Vector<ItrPaddedPoint>  mVehiclePoints;
   Vector<NullSurface>     mVehicleNullSurfaces;
   Vector<PlaneF>          mVehiclePlanes;
   Vector<U32>             mVehicleWindings;
   Vector<TriFan>          mVehicleWindingIndices;

   VectorPtr<InteriorSimpleMesh*> mStaticMeshes;
   
   U16                     mSearchTag;
   Vector<BaseMatInstance*>   mMatInstCleanupList;

   //-------------------------------------- Private interface
  private:

#ifndef TORQUE_SHIPPING
	GFXShaderRef	mDebugShader;
	GFXTexHandle	mDebugTexture;

   GFXStateBlockRef mInteriorDebugNoneSB;
   GFXStateBlockRef mInteriorDebugPortalSB;
   GFXStateBlockRef mInteriorDebugTextureSB;
   GFXStateBlockRef mInteriorDebugTwoTextureSB;

   // Debug Render ConstantBuffers
   GFXShaderConstBufferRef mDebugShaderConsts;

   GFXShaderConstHandle* mDebugShaderModelViewSC;
   GFXShaderConstHandle* mDebugShaderShadeColorSC;
#endif

   const char* getName(const U32 nameIndex) const;
   static const char* getLightTypeString(const LightType);
   S32  getZoneForPoint(const Point3F&) const;

#ifndef TORQUE_SHIPPING
	void debugRender(const ZoneVisDeterminer& zoneVis, SceneData &sgData, InteriorInstance *intInst, MatrixF& modelview);
	// render debug utility functions
	void preDebugRender();
	void debugDefaultRender(const ZoneVisDeterminer& zoneVis, SceneData &sgData, InteriorInstance *intInst);
	// show all surfaces with flag set as color c
	void debugShowSurfaceFlag(const ZoneVisDeterminer& zoneVis, const U32 flag, const ColorF& c);
	// render brushes
	void debugNormalRenderLines(const ZoneVisDeterminer& zoneVis);	
	// next 4 use debugShowSurfaceFlag to show surface info
   void debugShowDetail(const ZoneVisDeterminer& zoneVis);
   void debugShowAmbiguous(const ZoneVisDeterminer& zoneVis);
   void debugShowOrphan(const ZoneVisDeterminer& zoneVis);
   void debugShowOutsideVisible(const ZoneVisDeterminer& zoneVis);
	void debugShowPortalZones(const ZoneVisDeterminer& zoneVis);
	void debugRenderPortals();
	void debugShowCollisionFans(const ZoneVisDeterminer& zoneVis);
	void debugShowStrips(const ZoneVisDeterminer& zoneVis);
	void debugShowDetailLevel(const ZoneVisDeterminer& zoneVis);
	void debugShowHullSurfaces();
   void debugShowNullSurfaces(const ZoneVisDeterminer& zoneVis, SceneData &sgData, InteriorInstance *intInst);
   void debugShowVehicleHullSurfaces(const ZoneVisDeterminer& zoneVis, SceneData &sgData, InteriorInstance *intInst);
	void debugShowTexturesOnly(const ZoneVisDeterminer& zoneVis, SceneData &sgData, InteriorInstance *intInst);
   void debugShowLightmaps(const ZoneVisDeterminer& zoneVis, SceneData &sgData, InteriorInstance *intInst);
   void debugShowLargeTextures(const ZoneVisDeterminer& zoneVis, SceneData &sgData, InteriorInstance *intInst);
//   void debugShowVertexColors(MaterialList* pMaterials);      
#endif

  public:
   void collisionFanFromSurface(const Surface&, U32* fan, U32* numIndices) const;
	void fillSurfaceTexMats();
	void createZoneVBs();
	void cloneMatInstances();
   void initMatInstances();
   void createReflectPlanes();

  private:
   void fullWindingFromSurface(const Surface&, U32* fan, U32* numIndices) const;
   bool projectClipAndBoundFan(U32 fanIndex, F64* pResult);
   void zoneTraversal(S32 baseZone, const bool flipClipPlanes);
   void createZoneRectVectors();
   void destroyZoneRectVectors();
   void traverseZone(const RectD* inRects, const U32 numInputRects, U32 currZone, Vector<U32>& zoneStack);

   void fillVertex( GFXVertexPNTTB &vert, Surface &surface, U32 surfaceIndex );
   void getTexMat(U32 surfaceIndex, U32 pointOffset, Point3F& T, Point3F& N, Point3F& B);
   

   void storeRenderNode(   RenderNode &node,
                           ZoneRNList &RNList,
                           Vector<GFXPrimitive> &primInfoList,
                           Vector<U16> &indexList,
                           Vector<GFXVertexPNTTB> &verts,
                           U32 &startIndex,
                           U32 &startVert );

   void renderZoneNode( SceneRenderState *state,
                        RenderNode &node,
                        InteriorInstance *intInst,
                        SceneData &sgData,
                        MeshRenderInst *coreRi );

   void renderReflectNode( SceneRenderState *state,
                           ReflectRenderNode &node,
                           InteriorInstance *intInst,
                           SceneData &sgData,
                           MeshRenderInst *coreRi );

   void renderLights(SceneRenderState* state, InteriorInstance *intInst, SceneData &sgData, MeshRenderInst *coreRi, const ZoneVisDeterminer &zonevis);


   /// Used to maintain point and normal relationship (which use
   /// dissimilar index lookups) for verts when building vertex buffer.
   struct VertexBufferTempIndex
   {
      U16 index;
      Point3F normal;

      VertexBufferTempIndex()
      {
         index = 0;
         normal.set(0.0f, 0.0f, 0.0f);
      }
      VertexBufferTempIndex(const U16 ind, const Point3F &norm)
      {
         index = ind;
         normal = norm;
      }
   };

   void storeSurfVerts( Vector<U16> &masterIndexList,
                        Vector<VertexBufferTempIndex> &tempIndexList,
                        Vector<GFXVertexPNTTB> &verts,
                        U32 numIndices,
                        Surface &surface,
                        U32 surfaceIndex );

  public:
   void purgeLODData();

   void buildExportPolyList(OptimizedPolyList& polys, MatrixF* mat = NULL, Point3F* scale = NULL);
};

//------------------------------------------------------------------------------
inline bool Interior::isBSPLeafIndex(U32 index) const
{
   if (mFileVersion >= 14)
      return (index & 0x80000) != 0;
   else
      return (index & 0x8000) != 0;
}

inline bool Interior::isBSPSolidLeaf(U32 index) const
{
   AssertFatal(isBSPLeafIndex(index) == true, "Error, only call for leaves!");

   if (mFileVersion >= 14)
      return (index & 0x40000) != 0;
   else
      return (index & 0x4000) != 0;
}

inline bool Interior::isBSPEmptyLeaf(U32 index) const
{
   AssertFatal(isBSPLeafIndex(index) == true, "Error, only call for leaves!");

   if (mFileVersion >= 14)
      return (index & 0x40000) == 0;
   else
      return (index & 0x4000) == 0;
}

inline U16 Interior::getBSPSolidLeafIndex(U32 index) const
{
   AssertFatal(isBSPSolidLeaf(index) == true, "Error, only call for leaves!");

   if (mFileVersion >= 14)
      return U16(index & ~0xC0000);
   else
      return U16(index & ~0xC000);
}

inline U16 Interior::getBSPEmptyLeafZone(U32 index) const
{
   AssertFatal(isBSPEmptyLeaf(index) == true, "Error, only call for leaves!");

   if (mFileVersion >= 14)
      return U16(index & ~0xC0000);
   else
      return U16(index & ~0xC000);
}

inline const PlaneF& Interior::getPlane(U16 index) const
{
   AssertFatal(U32(index & ~0x8000) < mPlanes.size(),
               "Interior::getPlane: planeIndex out of range");

   return mPlanes[index & ~0x8000];
}

inline PlaneF Interior::getFlippedPlane(const U16 index) const
{
   PlaneF plane = getPlane(index);
   if(Interior::planeIsFlipped(index))
      plane.neg();

   return plane;
}

inline U16 Interior::getPlaneIndex(U16 index)
{
   return U16(index & ~0x8000);
}

inline bool Interior::planeIsFlipped(U16 index)
{
   return (index >> 15)!=0;
}

inline bool Interior::areEqualPlanes(U16 o, U16 t) const
{
   return (o & ~0x8000) == (t & ~0x8000);
}

inline const Point3F Interior::getPointNormal(const U32 surfaceIndex, const U32 pointOffset) const
{
   Surface rSurface = mSurfaces[surfaceIndex];

   Point3F normal(0.0f, 0.0f, 0.0f);

   if (mFileVersion >= 11)
   {
      U32 texMatIndex = mTexMatIndices[rSurface.windingStart + pointOffset];
      TexMatrix texMat = mTexMatrices[texMatIndex];

      if (texMat.N > -1)
         normal = mNormals[texMat.N];
   }
   else
      normal = getFlippedPlane(rSurface.planeIndex);

   return normal;
}

inline U32 Interior::getDetailLevel() const
{
   return mDetailLevel;
}

inline U32 Interior::getMinPixels() const
{
   return mMinPixels;
}

inline const Box3F& Interior::getBoundingBox() const
{
   return mBoundingBox;
}

inline S32 Interior::getNumZones() const
{
   return mZones.size();
}

inline bool Interior::isNullSurfaceIndex(const U32 index) const
{
   return (index & 0x80000000) != 0;
}

inline bool Interior::isVehicleNullSurfaceIndex(const U32 index) const
{
   return (index & 0x40000000) != 0;
}

inline U32 Interior::getNullSurfaceIndex(const U32 index) const
{
   AssertFatal(isNullSurfaceIndex(index), "Not a proper index!");
   AssertFatal(!isVehicleNullSurfaceIndex(index), "Not a proper index");
   return (index & ~0x80000000);
}

inline U32 Interior::getVehicleNullSurfaceIndex(const U32 index) const
{
   AssertFatal(isVehicleNullSurfaceIndex(index), "Not a proper index!");
   return (index & ~(0x80000000 | 0x40000000));
}

inline const char* Interior::getLightTypeString(const LightType type)
{
   switch (type) {
     case AmbientLooping:
      return "AmbientLooping";
     case AmbientFlicker:
      return "AmbientFlicker";
     case TriggerableLoop:
      return "TriggerableLoop";
     case TriggerableFlicker:
      return "TriggerableFlicker";
     case TriggerableRamp:
      return "TriggerableRamp";

     default:
      return "<UNKNOWN>";
   }
}

inline const char* Interior::getName(const U32 nameIndex) const
{
   return &mNameBuffer[nameIndex];
}

inline const U32 Interior::getSurfaceCount() const
{
   return(mSurfaces.size());
}

inline const Interior::Surface & Interior::getSurface(const U32 surface) const
{
   AssertFatal(surface < mSurfaces.size(), "invalid index");
   return(mSurfaces[surface]);
}

inline const U32 Interior::getStaticMeshCount() const
{
   return mStaticMeshes.size();
}

inline const InteriorSimpleMesh *Interior::getStaticMesh(const U32 index) const
{
   AssertFatal(index < mStaticMeshes.size(), "invalid index");
   return mStaticMeshes[index];
}

inline const U32 Interior::getNormalLMapIndex(const U32 surface) const
{
   AssertFatal(surface < mNormalLMapIndices.size(), "invalid index");
   return(mNormalLMapIndices[surface]);
}

inline const U32 Interior::getAlarmLMapIndex(const U32 surface) const
{
   AssertFatal(surface < mAlarmLMapIndices.size(), "invalid index");
   return(mAlarmLMapIndices[surface]);
}

inline const U32 Interior::getWinding(const U32 index) const
{
   AssertFatal(index < mWindings.size(), "invalid index");
   return(mWindings[index]);
}

inline const Point3F & Interior::getPoint(const U32 index) const
{
   AssertFatal(index < mPoints.size(), "invalid index");
   return(mPoints[index].point);
}

inline const Interior::TexGenPlanes & Interior::getLMTexGenEQ(const U32 index) const
{
   AssertFatal(index < mLMTexGenEQs.size(), "invalid index");
   return(mLMTexGenEQs[index]);
}

inline const Interior::TexGenPlanes & Interior::getTexGenEQ(const U32 index) const
{
   AssertFatal(index < mTexGenEQs.size(), "invalid index");
   return(mTexGenEQs[index]);
}

inline bool Interior::hasAlarmState() const
{
   return(mHasAlarmState);
}

inline const bool Interior::isSurfaceOutsideVisible(U32 surface) const
{
   AssertFatal(surface < mSurfaces.size(), "Interior::isSurfaceOutsideVisible: Invalid surface index");
   return ((mSurfaces[surface].surfaceFlags & SurfaceOutsideVisible)!=0);
}

inline const U32 Interior::getWindingCount() const
{
   return(mWindings.size());
}

#endif //_INTERIOR_H_
