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

#ifndef _FOLIAGEREPLICATOR_H_
#define _FOLIAGEREPLICATOR_H_

#ifndef _SCENEOBJECT_H_
#include "scene/sceneObject.h"
#endif
#ifndef _GFXTEXTUREHANDLE_H_
#include "gfx/gfxTextureHandle.h"
#endif
#ifndef _GFXPRIMITIVEBUFFER_H_
#include "gfx/gfxPrimitiveBuffer.h"
#endif
#ifndef _GBITMAP_H_
#include "gfx/bitmap/gBitmap.h"
#endif
#ifndef _RENDERPASSMANAGER_H_
#include "renderInstance/renderPassManager.h"
#endif
#ifndef _MATHUTIL_FRUSTUM_H_
#include "math/util/frustum.h"
#endif

#pragma warning( push, 4 )

#define AREA_ANIMATION_ARC         (1.0f / 360.0f)

#define FXFOLIAGEREPLICATOR_COLLISION_MASK   (   TerrainObjectType     |   \
                                                 StaticShapeObjectType |   \
                                                 WaterObjectType      )

#define FXFOLIAGEREPLICATOR_NOWATER_COLLISION_MASK   (   TerrainObjectType      |   \
                                          StaticShapeObjectType   )


#define FXFOLIAGE_ALPHA_EPSILON            1e-4



//------------------------------------------------------------------------------
// Class: fxFoliageItem
//------------------------------------------------------------------------------
class fxFoliageItem
{
public:
   MatrixF     Transform;     
   F32         Width;         
   F32         Height;        
   Box3F       FoliageBox;    
   bool        Flipped;       
   F32         SwayPhase;     
   F32         SwayTimeRatio; 
   F32         LightPhase;    
   F32         LightTimeRatio; 
   U32         LastFrameSerialID; 
};

//------------------------------------------------------------------------------
// Class: fxFoliageCulledList
//------------------------------------------------------------------------------
class fxFoliageCulledList
{
public:
   fxFoliageCulledList() {};
   fxFoliageCulledList(Box3F SearchBox, fxFoliageCulledList* InVec);
   ~fxFoliageCulledList() {};

   void FindCandidates(const Box3F& SearchBox, fxFoliageCulledList* InVec);

   U32 GetListCount(void) { return mCulledObjectSet.size(); };
   fxFoliageItem* GetElement(U32 index) { return mCulledObjectSet[index]; };

   Vector<fxFoliageItem*>   mCulledObjectSet;      // Culled Object Set.
};


//------------------------------------------------------------------------------
// Class: fxFoliageQuadNode
//------------------------------------------------------------------------------
class fxFoliageQuadrantNode {
public:
   U32                  Level;
   Box3F               QuadrantBox;
   fxFoliageQuadrantNode*   QuadrantChildNode[4];
   Vector<fxFoliageItem*>   RenderList;
   // Used in DrawIndexPrimitive call.
   U32                      startIndex;
   U32                      primitiveCount;
};


//------------------------------------------------------------------------------
// Class: fxFoliageRenderList
//------------------------------------------------------------------------------
class fxFoliageRenderList
{
public:

   Box3F                   mBox;               // Clipping Box.
   Frustum                 mFrustum;           // View frustum.

   Vector<fxFoliageItem*>     mVisObjectSet;   // Visible Object Set.
   F32                        mHeightLerp;         // Height Lerp.

public:
   bool IsQuadrantVisible(const Box3F VisBox, const MatrixF& RenderTransform);
   void SetupClipPlanes(SceneRenderState* state, const F32 FarClipPlane);
   void DrawQuadBox(const Box3F& QuadBox, const ColorF Colour);
};


// Define a vertex 
GFXDeclareVertexFormat( GFXVertexFoliage )
{
   Point3F point;
   Point3F normal;
   Point2F texCoord;
   Point2F texCoord2;
};


//------------------------------------------------------------------------------
// Class: fxFoliageReplicator
//------------------------------------------------------------------------------
class fxFoliageReplicator : public SceneObject
{
private:
   typedef SceneObject      Parent;

protected:

   void CreateFoliage(void);
   void DestroyFoliage(void);
   void DestroyFoliageItems();


   void SyncFoliageReplicators(void);

   Box3F FetchQuadrant(const Box3F& Box, U32 Quadrant);
   void ProcessQuadrant(fxFoliageQuadrantNode* pParentNode, fxFoliageCulledList* pCullList, U32 Quadrant);
   void ProcessNodeChildren(fxFoliageQuadrantNode* pParentNode, fxFoliageCulledList* pCullList);

   enum {   FoliageReplicationMask   = (1 << 0) };


   U32   mCreationAreaAngle;
   bool  mClientReplicationStarted;
   U32   mCurrentFoliageCount;

   Vector<fxFoliageQuadrantNode*>   mFoliageQuadTree;
   Vector<fxFoliageItem*>           mReplicatedFoliage;
   fxFoliageRenderList              mFrustumRenderSet;

   GFXVertexBufferHandle<GFXVertexFoliage> mVertexBuffer;
   GFXPrimitiveBufferHandle   mPrimBuffer;
   GFXShaderRef               mShader;
   ShaderData*                mShaderData;
   GBitmap*                   mAlphaLookup;

   MRandomLCG                 RandomGen;
   F32                        mFadeInGradient;
   F32                        mFadeOutGradient;
   S32                        mLastRenderTime;
   F32                        mGlobalSwayPhase;
   F32                        mGlobalSwayTimeRatio;
   F32                        mGlobalLightPhase;
   F32                        mGlobalLightTimeRatio;
   U32                        mFrameSerialID;

   U32                        mQuadTreeLevels;            // Quad-Tree Levels.
   U32                        mPotentialFoliageNodes;     // Potential Foliage Nodes.
   U32                        mNextAllocatedNodeIdx;      // Next Allocated Node Index.
   U32                        mBillboardsAcquired;        // Billboards Acquired.

   // Used for alpha lookup in the pixel shader
   GFXTexHandle               mAlphaTexture;

   GFXStateBlockRef  mPlacementSB;
   GFXStateBlockRef  mRenderSB;
   GFXStateBlockRef  mDebugSB;

   GFXShaderConstBufferRef mFoliageShaderConsts;

   GFXShaderConstHandle* mFoliageShaderProjectionSC;
   GFXShaderConstHandle* mFoliageShaderWorldSC;
   GFXShaderConstHandle* mFoliageShaderGlobalSwayPhaseSC;
   GFXShaderConstHandle* mFoliageShaderSwayMagnitudeSideSC;
   GFXShaderConstHandle* mFoliageShaderSwayMagnitudeFrontSC;
   GFXShaderConstHandle* mFoliageShaderGlobalLightPhaseSC;
   GFXShaderConstHandle* mFoliageShaderLuminanceMagnitudeSC;
   GFXShaderConstHandle* mFoliageShaderLuminanceMidpointSC;
   GFXShaderConstHandle* mFoliageShaderDistanceRangeSC;
   GFXShaderConstHandle* mFoliageShaderCameraPosSC;
   GFXShaderConstHandle* mFoliageShaderTrueBillboardSC;

   //pixel shader
   GFXShaderConstHandle* mFoliageShaderGroundAlphaSC;
   GFXShaderConstHandle* mFoliageShaderAmbientColorSC;
   GFXShaderConstHandle* mDiffuseTextureSC;
   GFXShaderConstHandle* mAlphaMapTextureSC;



   bool              mDirty;
   
   void SetupShader();
   void SetupBuffers();
   void renderObject(ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance*);
   void renderBuffers(SceneRenderState* state);
   void renderArc(const F32 fRadiusX, const F32 fRadiusY);
   void renderPlacementArea(const F32 ElapsedTime);
   void renderQuad(fxFoliageQuadrantNode* quadNode, const MatrixF& RenderTransform, const bool UseDebug);
   void computeAlphaTex();
public:
   fxFoliageReplicator();
   ~fxFoliageReplicator();

   void StartUp(void);
   void ShowReplication(void);
   void HideReplication(void);

   // SceneObject
   virtual void prepRenderImage( SceneRenderState *state );

   // SimObject
   bool onAdd();
   void onRemove();
   void inspectPostApply();

   // NetObject
   U32 packUpdate(NetConnection *conn, U32 mask, BitStream *stream);
   void unpackUpdate(NetConnection *conn, BitStream *stream);

   // Editor
   void onGhostAlwaysDone();

   // ConObject.
   static void initPersistFields();

   // Field Data.
   class tagFieldData
   {
      public:

      bool              mUseDebugInfo;
      F32               mDebugBoxHeight;
      U32               mSeed;
      StringTableEntry  mFoliageFile;
      GFXTexHandle      mFoliageTexture;
      U32               mFoliageCount;
      U32               mFoliageRetries;

      U32               mInnerRadiusX;
      U32               mInnerRadiusY;
      U32               mOuterRadiusX;
      U32               mOuterRadiusY;

      F32               mMinWidth;
      F32               mMaxWidth;
      F32               mMinHeight;
      F32               mMaxHeight;
      bool              mFixAspectRatio;
      bool              mFixSizeToMax;
      F32               mOffsetZ;
      bool              mRandomFlip;
      bool              mUseTrueBillboards;

      bool              mUseCulling;
      U32               mCullResolution;
      F32               mViewDistance;
      F32               mViewClosest;
      F32               mFadeInRegion;
      F32               mFadeOutRegion;
      F32               mAlphaCutoff;
      F32               mGroundAlpha;

      bool              mSwayOn;
      bool              mSwaySync;
      F32               mSwayMagnitudeSide;
      F32               mSwayMagnitudeFront;
      F32               mMinSwayTime;
      F32               mMaxSwayTime;

      bool              mLightOn;
      bool              mLightSync;
      F32               mMinLuminance;
      F32               mMaxLuminance;
      F32               mLightTime;

      bool            mAllowOnTerrain;
      bool            mAllowStatics;
      bool            mAllowOnWater;
      bool            mAllowWaterSurface;
      S32             mAllowedTerrainSlope;

      bool            mHideFoliage;
      bool            mShowPlacementArea;
      U32             mPlacementBandHeight;
      ColorF          mPlaceAreaColour;

      tagFieldData()
      {
         // Set Defaults.
         mUseDebugInfo         = false;
         mDebugBoxHeight       = 1.0f;
         mSeed                 = 1376312589;
         mFoliageFile          = StringTable->EmptyString();
         mFoliageTexture       = GFXTexHandle();
         mFoliageCount         = 10;
         mFoliageRetries       = 100;

         mInnerRadiusX         = 0;
         mInnerRadiusY         = 0;
         mOuterRadiusX         = 128;
         mOuterRadiusY         = 128;

         mMinWidth             = 1;
         mMaxWidth             = 3;
         mMinHeight            = 1;
         mMaxHeight            = 5;
         mFixAspectRatio       = true;
         mFixSizeToMax         = false;
         mOffsetZ              = 0;
         mRandomFlip           = true;
         mUseTrueBillboards    = false;

         mUseCulling           = true;
         mCullResolution       = 64;
         mViewDistance         = 50.0f;
         mViewClosest          = 1.0f;
         mFadeInRegion         = 10.0f;
         mFadeOutRegion        = 1.0f;
         mAlphaCutoff          = 0.2f;
         mGroundAlpha          = 1.0f;

         mSwayOn               = false;
         mSwaySync             = false;
         mSwayMagnitudeSide    = 0.1f;
         mSwayMagnitudeFront   = 0.2f;
         mMinSwayTime          = 3.0f;
         mMaxSwayTime          = 10.0f;

         mLightOn              = false;
         mLightSync            = false;
         mMinLuminance         = 0.7f;
         mMaxLuminance         = 1.0f;
         mLightTime            = 5.0f;

         mAllowOnTerrain       = true;
         mAllowStatics         = true;
         mAllowOnWater         = false;
         mAllowWaterSurface    = false;
         mAllowedTerrainSlope  = 90;

         mHideFoliage          = false;
         mShowPlacementArea    = true;
         mPlacementBandHeight  = 25;
         mPlaceAreaColour      .set(0.4f, 0, 0.8f);
      }

   } mFieldData;

   // Declare Console Object.
   DECLARE_CONOBJECT(fxFoliageReplicator);
};
#pragma warning( pop ) 
#endif // _FOLIAGEREPLICATOR_H_
