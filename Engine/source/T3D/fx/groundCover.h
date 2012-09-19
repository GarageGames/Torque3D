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

#ifndef _GROUNDCOVER_H_
#define _GROUNDCOVER_H_

#ifndef _SCENEOBJECT_H_
#include "scene/sceneObject.h"
#endif
#ifndef _MATHUTIL_FRUSTUM_H_
#include "math/util/frustum.h"
#endif
#ifndef _GFXTEXTUREHANDLE_H_
#include "gfx/gfxTextureHandle.h"
#endif
#ifndef _GFX_GFXPRIMITIVEBUFFER_H_
#include "gfx/gfxPrimitiveBuffer.h"
#endif
#ifndef _RENDERPASSMANAGER_H_
#include "renderInstance/renderPassManager.h"
#endif
#ifndef _MATTEXTURETARGET_H_
#include "materials/matTextureTarget.h"
#endif
#ifndef _SHADERFEATURE_H_
#include "shaderGen/shaderFeature.h"
#endif

class TerrainBlock;
class GroundCoverCell;
class TSShapeInstance;
class Material;
class MaterialParameters;
class MaterialParameterHandle;


///
#define MAX_COVERTYPES 8


GFXDeclareVertexFormat( GCVertex )
{
   Point3F point;

   Point3F normal;

   // .rgb = ambient
   // .a = corner index
   GFXVertexColor ambient;

   // .x = size x
   // .y = size y
   // .z = type
   // .w = wind amplitude
   Point4F params;
};

struct GroundCoverShaderConstData
{
   Point2F fadeInfo;
   Point3F gustInfo;
   Point2F turbInfo;
   Point3F camRight;
   Point3F camUp;
};

class GroundCover;

class GroundCoverShaderConstHandles : public ShaderFeatureConstHandles
{
public:

   GroundCoverShaderConstHandles();

   virtual void init( GFXShader *shader );

   virtual void setConsts( SceneRenderState *state, 
                           const SceneData &sgData,
                           GFXShaderConstBuffer *buffer );

   GroundCover *mGroundCover;

   GFXShaderConstHandle *mTypeRectsSC;
   GFXShaderConstHandle *mFadeSC;
   GFXShaderConstHandle *mWindDirSC;
   GFXShaderConstHandle *mGustInfoSC;
   GFXShaderConstHandle *mTurbInfoSC;
   GFXShaderConstHandle *mCamRightSC;
   GFXShaderConstHandle *mCamUpSC;
};


class GroundCover : public SceneObject
{
   friend class GroundCoverShaderConstHandles;
   friend class GroundCoverCell;
   typedef SceneObject Parent;

public:

   GroundCover();
   ~GroundCover();

   DECLARE_CONOBJECT(GroundCover);

   static void consoleInit();
   static void initPersistFields();

   bool onAdd();
   void onRemove();
   void inspectPostApply();

   // Network
   U32 packUpdate( NetConnection *, U32 mask, BitStream *stream );
   void unpackUpdate( NetConnection *, BitStream *stream );

   // Rendering
   void prepRenderImage( SceneRenderState *state );
   
   // Editor
   void onTerrainUpdated( U32 flags, TerrainBlock *tblock, const Point2I& min, const Point2I& max );

   // Misc
   const GroundCoverShaderConstData& getShaderConstData() const { return mShaderConstData; }

   /// Sets the global ground cover LOD scalar which controls
   /// the percentage of the maximum designed cover to put down.
   /// It scales both rendering cost and placement CPU performance.
   /// Returns the actual value set.
   static F32 setQualityScale( F32 scale ) { return smDensityScale = mClampF( scale, 0.0f, 1.0f ); }

   /// Returns the current quality scale... see above.
   static F32 getQualityScale() { return smDensityScale; }

protected:      

   enum MaskBits 
   {      
      TerrainBlockMask  = Parent::NextFreeMask << 0,
      NextFreeMask      = Parent::NextFreeMask << 1
   };

   MaterialParameters *mMatParams;
   MaterialParameterHandle *mTypeRectsParam;   
   MaterialParameterHandle *mFadeParams;
   MaterialParameterHandle *mWindDirParam;
   MaterialParameterHandle *mGustInfoParam;
   MaterialParameterHandle *mTurbInfoParam;
   MaterialParameterHandle *mCamRightParam;
   MaterialParameterHandle *mCamUpParam;

   /// This RNG seed is saved and sent to clients
   /// for generating the same cover.
   S32 mRandomSeed;

   /// This is the outer generation radius from
   /// the current camera position.
   F32 mRadius;

   // Offset along the Z axis to render the ground cover.
   F32 mZOffset;

   /// This is less than or equal to mRadius and
   /// defines when fading of cover elements begins.
   F32 mFadeRadius;

   /// This is the distance at which DTS elements are 
   /// completely culled out.
   F32 mShapeCullRadius;

   /// Whether shapes rendered by the GroundCover should cast shadows.
   bool mShapesCastShadows;

   /// This is used to scale the various culling radii 
   /// when rendering a reflection... typically for water.
   F32 mReflectRadiusScale;

   /// This is the number of cells per axis in the grid.
   U32 mGridSize;

   typedef Vector<GroundCoverCell*> CellVector;

   /// This is the allocator for GridCell chunks.
   CellVector mAllocCellList;
   CellVector mFreeCellList;

   /// This is the grid of active cells.
   CellVector mCellGrid;

   /// This is a scratch grid used while updating
   /// the cell grid.
   CellVector mScratchGrid;

   /// This is the index to the first grid cell.
   Point2I mGridIndex;

   /// The maximum amount of cover elements to include in
   /// the grid at any one time.  The actual amount may be
   /// less than this based on randomization.
   S32 mMaxPlacement;

   /// Used to detect changes in cell placement count from 
   /// the global quality scale so we can regen the cells.
   S32 mLastPlacementCount;

   /// Used for culling cells to update and render.
   Frustum mCuller;

   /// Debug parameter for displaying the grid cells.
   bool mDebugRenderCells;

   /// Debug parameter for turning off billboard rendering.
   bool mDebugNoBillboards;

   /// Debug parameter for turning off shape rendering.
   bool mDebugNoShapes;

   /// Debug parameter for locking the culling frustum which
   /// will freeze the cover generation.
   bool mDebugLockFrustum;

   /// Stat for number of rendered cells.
   static U32 smStatRenderedCells;

   /// Stat for number of rendered billboards.
   static U32 smStatRenderedBillboards;

   /// Stat for number of rendered billboard batches.
   static U32 smStatRenderedBatches;

   /// Stat for number of rendered shapes.
   static U32 smStatRenderedShapes;

   /// The global ground cover LOD scalar which controls
   /// the percentage of the maximum amount of cover to put
   /// down.  It scales both rendering cost and placement
   /// CPU performance.
   static F32 smDensityScale;   

   String mMaterialName;
   Material *mMaterial;
   BaseMatInstance *mMatInst;

   GroundCoverShaderConstData mShaderConstData;

   /// This is the maximum amout of degrees the billboard will
   /// tilt down to match the camera.
   F32 mMaxBillboardTiltAngle;

   /// The probability of one cover type verses another.
   F32 mProbability[MAX_COVERTYPES];

   /// The minimum random size for each cover type.
   F32 mSizeMin[MAX_COVERTYPES];

   /// The maximum random size of this cover type.
   F32 mSizeMax[MAX_COVERTYPES];

   /// An exponent used to bias between the minimum
   /// and maximum random sizes.
   F32 mSizeExponent[MAX_COVERTYPES];

   /// The wind effect scale.
   F32 mWindScale[MAX_COVERTYPES];

   /// The maximum slope angle in degrees for placement.
   F32 mMaxSlope[MAX_COVERTYPES];

   /// The minimum world space elevation for placement.
   F32 mMinElevation[MAX_COVERTYPES];

   /// The maximum world space elevation for placement.
   F32 mMaxElevation[MAX_COVERTYPES];

   /// Terrain material name to limit coverage to, or
   /// left empty to cover entire terrain.
   StringTableEntry mLayer[MAX_COVERTYPES];

   /// Inverts the data layer test making the 
   /// layer an exclusion mask.
   bool mInvertLayer[MAX_COVERTYPES];

   /// The minimum amount of elements in a clump.
   S32 mMinClumpCount[MAX_COVERTYPES];

   /// The maximum amount of elements in a clump.
   S32 mMaxClumpCount[MAX_COVERTYPES];

   /// An exponent used to bias between the minimum
   /// and maximum clump counts for a particular clump.
   F32 mClumpCountExponent[MAX_COVERTYPES];

   /// The maximum clump radius.
   F32 mClumpRadius[MAX_COVERTYPES];

   /// This is a cached array of billboard aspect scales
   /// used to avoid some calculations when generating cells.
   F32 mBillboardAspectScales[MAX_COVERTYPES];

   RectF mBillboardRects[MAX_COVERTYPES];

   /// The cover shape filenames.
   StringTableEntry mShapeFilenames[MAX_COVERTYPES];

   /// The cover shape instances.
   TSShapeInstance* mShapeInstances[MAX_COVERTYPES];

   /// This is the same as mProbability, but normalized for use
   /// during the cover placement process. 
   F32 mNormalizedProbability[MAX_COVERTYPES];

   /// A shared primitive buffer setup for drawing the maximum amount
   /// of billboards you could possibly have in a single cell.
   GFXPrimitiveBufferHandle mPrimBuffer;

   /// The length in meters between peaks in the wind gust.
   F32 mWindGustLength;

   /// Controls how often the wind gust peaks per second.
   F32 mWindGustFrequency;

   /// The maximum distance in meters that the peak wind 
   /// gust will displace an element.
   F32 mWindGustStrength;

   /// The direction of the wind.
   Point2F mWindDirection;

   /// Controls the overall rapidity of the wind turbulence.
   F32 mWindTurbulenceFrequency;

   /// The maximum distance in meters that the turbulence can
   /// displace a ground cover element.
   F32 mWindTurbulenceStrength;

   void _initMaterial();

   bool _initShader();

   void _initShapes();

   void _deleteShapes();

   /// Called when GroundCover parameters are changed and
   /// things need to be reinitialized to continue.
   void _initialize( U32 cellCount, U32 cellPlacementCount );

   /// Updates the cover grid by removing cells that
   /// have fallen outside of mRadius and adding new 
   /// ones that have come into view.
   void _updateCoverGrid( const Frustum &culler );

   /// Clears the cell grid, moves all the allocated cells to
   /// the free list, and deletes excess free cells.
   void _freeCells();

   /// Clears the cell grid and deletes all the free cells.
   void _deleteCells();

   /// Returns a cell to the free list.
   void _recycleCell( GroundCoverCell* cell );

   /// Generates a new cell using the recycle list when possible.
   GroundCoverCell* _generateCell(  const Point2I& index,
                                    const Box3F& bounds, 
                                    U32 placementCount,
                                    S32 randSeed );

   void _debugRender( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat );
};

#endif // _GROUNDCOVER_H_
