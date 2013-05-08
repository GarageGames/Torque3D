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

// Written by Melvyn May, Started on 4th August 2002.
//
// "My code is written for the Torque community, so do your worst with it,
//	just don't rip-it-off and call it your own without even thanking me".
//
//	- Melv.
//
//
// Conversion to TSE By Brian "bzztbomb" Richardson 9/2005
//   This was a neat piece of code!  Thanks Melv!
//   I've switched this to use one large indexed primitive buffer.  All animation
//   is then done in the vertex shader.  This means we have a static vertex/primitive
//   buffer that never changes!  How spiff!  Because of this, the culling code was
//   changed to render out full quadtree nodes, we don't try to cull each individual
//   node ourselves anymore.  This means to get good performance, you probably need to do the 
//   following:
//     1.  If it's a small area to cover, turn off culling completely.
//     2.  You want to tune the parameters to make sure there are a lot of billboards within
//         each quadrant.
// 
// POTENTIAL TODO LIST:
//   TODO: Clamp item alpha to fog alpha

#include "platform/platform.h"
#include "T3D/fx/fxFoliageReplicator.h"

#include "gfx/gfxDevice.h"
#include "gfx/primBuilder.h"	// Used for debug / mission edit rendering
#include "console/consoleTypes.h"
#include "core/stream/bitStream.h"
#include "math/mRandom.h"
#include "math/mathIO.h"
#include "console/simBase.h"
#include "scene/sceneManager.h"
#include "renderInstance/renderPassManager.h"
#include "scene/sceneRenderState.h"
#include "sim/netConnection.h"
#include "materials/shaderData.h"
#include "console/engineAPI.h"

const U32 AlphaTexLen = 1024;

GFXImplementVertexFormat( GFXVertexFoliage )
{
   addElement( "POSITION", GFXDeclType_Float3 );
   addElement( "NORMAL", GFXDeclType_Float3 );
   addElement( "TEXCOORD", GFXDeclType_Float2, 0 );
   addElement( "TEXCOORD", GFXDeclType_Float2, 1 );
}

//------------------------------------------------------------------------------
//
//	Put the function in /example/common/editor/ObjectBuilderGui.gui [around line 458] ...
//
//	function ObjectBuilderGui::buildfxFoliageReplicator(%this)
//	{
//		%this.className = "fxFoliageReplicator";
//		%this.process();
//	}
//
//------------------------------------------------------------------------------
//
//	Put this in /example/common/editor/EditorGui.cs in [function Creator::init( %this )]
//	
//   %Environment_Item[8] = "fxFoliageReplicator";  <-- ADD THIS.
//
//------------------------------------------------------------------------------
//
//	Put this in /example/common/client/missionDownload.cs in [function clientCmdMissionStartPhase3(%seq,%missionName)] (line 65)
//	after codeline 'onPhase2Complete();'.
//
//	StartFoliageReplication();
//
//------------------------------------------------------------------------------
//
//	Put this in /engine/console/simBase.h (around line 509) in
//
//	namespace Sim
//  {
//	   DeclareNamedSet(fxFoliageSet)  <-- ADD THIS (Note no semi-colon).
//
//------------------------------------------------------------------------------
//
//	Put this in /engine/console/simBase.cc (around line 19) in
//
//  ImplementNamedSet(fxFoliageSet)  <-- ADD THIS (Note no semi-colon).
//
//------------------------------------------------------------------------------
//
//	Put this in /engine/console/simManager.cc [function void init()] (around line 269).
//
//	namespace Sim
//  {
//		InstantiateNamedSet(fxFoliageSet);  <-- ADD THIS (Including Semi-colon).
//
//------------------------------------------------------------------------------
extern bool gEditingMission;

//------------------------------------------------------------------------------

IMPLEMENT_CO_NETOBJECT_V1(fxFoliageReplicator);

ConsoleDocClass( fxFoliageReplicator,
   "@brief An emitter to replicate fxFoliageItem objects across an area.\n"
   "@ingroup Foliage\n"
);

//------------------------------------------------------------------------------
//
// Trig Table Lookups.
//
//------------------------------------------------------------------------------
const F32 PeriodLen = (F32) 2.0f * (F32) M_PI;
const F32 PeriodLenMinus = (F32) (2.0f * M_PI) - 0.01f;

//------------------------------------------------------------------------------
//
// Class: fxFoliageRenderList
//
//------------------------------------------------------------------------------

void fxFoliageRenderList::SetupClipPlanes( SceneRenderState* state, const F32 farClipPlane )
{
   const F32 nearPlane = state->getNearPlane();
   const F32 farPlane = farClipPlane;

   const Frustum& frustum = state->getFrustum();

   // [rene, 23-Feb-11] Why isn't this preserving the ortho state of the original frustum?

   mFrustum.set(  false,//zoneState.frustum.isOrtho(),
                  frustum.getNearLeft(),
                  frustum.getNearRight(),
                  frustum.getNearTop(),
                  frustum.getNearBottom(),
                  nearPlane,
                  farPlane,
                  frustum.getTransform()
   );


   mBox = mFrustum.getBounds();
}

//------------------------------------------------------------------------------


inline void fxFoliageRenderList::DrawQuadBox(const Box3F& QuadBox, const ColorF Colour)
{
   // Define our debug box.
   static Point3F BoxPnts[] = {
      Point3F(0,0,0),
      Point3F(0,0,1),
      Point3F(0,1,0),
      Point3F(0,1,1),
      Point3F(1,0,0),
      Point3F(1,0,1),
      Point3F(1,1,0),
      Point3F(1,1,1)
   };

   static U32 BoxVerts[][4] = {
      {0,2,3,1},     // -x
      {7,6,4,5},     // +x
      {0,1,5,4},     // -y
      {3,2,6,7},     // +y
      {0,4,6,2},     // -z
      {3,7,5,1}      // +z
   };

   // Project our Box Points.
   Point3F ProjectionPoints[8];

   for( U32 i=0; i<8; i++ )
   {
      ProjectionPoints[i].set(BoxPnts[i].x ? QuadBox.maxExtents.x : QuadBox.minExtents.x,
         BoxPnts[i].y ? QuadBox.maxExtents.y : QuadBox.minExtents.y,
         BoxPnts[i].z ? (mHeightLerp * QuadBox.maxExtents.z) + (1-mHeightLerp) * QuadBox.minExtents.z : QuadBox.minExtents.z);

   }

   PrimBuild::color(Colour);

   // Draw the Box.
   for(U32 x = 0; x < 6; x++)
   {
      // Draw a line-loop.
      PrimBuild::begin(GFXLineStrip, 5);

      for(U32 y = 0; y < 4; y++)
      {
         PrimBuild::vertex3f(ProjectionPoints[BoxVerts[x][y]].x,
            ProjectionPoints[BoxVerts[x][y]].y,
            ProjectionPoints[BoxVerts[x][y]].z);
      }
      PrimBuild::vertex3f(ProjectionPoints[BoxVerts[x][0]].x,
         ProjectionPoints[BoxVerts[x][0]].y,
         ProjectionPoints[BoxVerts[x][0]].z);
      PrimBuild::end();
   } 
}

//------------------------------------------------------------------------------
bool fxFoliageRenderList::IsQuadrantVisible(const Box3F VisBox, const MatrixF& RenderTransform)
{
   // Can we trivially accept the visible box?
   if ( !mFrustum.isCulled( VisBox ) )
      return true;

   // Not visible.
   return false;
}



//------------------------------------------------------------------------------
//
// Class: fxFoliageCulledList
//
//------------------------------------------------------------------------------
fxFoliageCulledList::fxFoliageCulledList(Box3F SearchBox, fxFoliageCulledList* InVec)
{
   // Find the Candidates.
   FindCandidates(SearchBox, InVec);
}

//------------------------------------------------------------------------------

void fxFoliageCulledList::FindCandidates(Box3F SearchBox, fxFoliageCulledList* InVec)
{
   // Search the Culled List.
   for (U32 i = 0; i < InVec->GetListCount(); i++)
   {
      // Is this Box overlapping our search box?
      if (SearchBox.isOverlapped(InVec->GetElement(i)->FoliageBox))
      {
         // Yes, so add it to our culled list.
         mCulledObjectSet.push_back(InVec->GetElement(i));
      }
   }
}



//------------------------------------------------------------------------------
//
// Class: fxFoliageReplicator
//
//------------------------------------------------------------------------------

fxFoliageReplicator::fxFoliageReplicator()
{
   // Setup NetObject.
   mTypeMask |= StaticObjectType;
   mNetFlags.set(Ghostable | ScopeAlways);

   // Reset Client Replication Started.
   mClientReplicationStarted = false;

   // Reset Foliage Count.
   mCurrentFoliageCount = 0;

   // Reset Creation Area Angle Animation.
   mCreationAreaAngle = 0;

   // Reset Last Render Time.
   mLastRenderTime = 0;

   // Reset Foliage Nodes.
   mPotentialFoliageNodes = 0;
   // Reset Billboards Acquired.
   mBillboardsAcquired = 0;

   // Reset Frame Serial ID.
   mFrameSerialID = 0;

   mAlphaLookup = NULL;

   mDirty = true;

   mFoliageShaderProjectionSC = NULL;
   mFoliageShaderWorldSC = NULL;
   mFoliageShaderGlobalSwayPhaseSC = NULL;
   mFoliageShaderSwayMagnitudeSideSC = NULL;
   mFoliageShaderSwayMagnitudeFrontSC = NULL;
   mFoliageShaderGlobalLightPhaseSC = NULL;
   mFoliageShaderLuminanceMagnitudeSC = NULL;
   mFoliageShaderLuminanceMidpointSC = NULL;
   mFoliageShaderDistanceRangeSC = NULL;
   mFoliageShaderCameraPosSC = NULL;
   mFoliageShaderTrueBillboardSC = NULL;
   mFoliageShaderGroundAlphaSC = NULL;
   mFoliageShaderAmbientColorSC = NULL;

   mShaderData = NULL;
}

//------------------------------------------------------------------------------

fxFoliageReplicator::~fxFoliageReplicator()
{
   if (mAlphaLookup)
      delete mAlphaLookup;

   mPlacementSB = NULL;
}

//------------------------------------------------------------------------------

void fxFoliageReplicator::initPersistFields()
{
   // Add out own persistent fields.
   addGroup( "Debugging" );	// MM: Added Group Header.
      addField( "UseDebugInfo",        TypeBool,      Offset( mFieldData.mUseDebugInfo,         fxFoliageReplicator ), "Culling bins are drawn when set to true." );
      addField( "DebugBoxHeight",      TypeF32,       Offset( mFieldData.mDebugBoxHeight,       fxFoliageReplicator ), "Height multiplier for drawn culling bins.");
      addField( "HideFoliage",         TypeBool,      Offset( mFieldData.mHideFoliage,          fxFoliageReplicator ), "Foliage is hidden when set to true." );
      addField( "ShowPlacementArea",   TypeBool,      Offset( mFieldData.mShowPlacementArea,    fxFoliageReplicator ), "Draw placement rings when set to true." );
      addField( "PlacementAreaHeight", TypeS32,       Offset( mFieldData.mPlacementBandHeight,  fxFoliageReplicator ), "Height of the placement ring in world units." );
      addField( "PlacementColour",     TypeColorF,    Offset( mFieldData.mPlaceAreaColour,      fxFoliageReplicator ), "Color of the placement ring." );
   endGroup( "Debugging" );	// MM: Added Group Footer.

   addGroup( "Media" );	// MM: Added Group Header.
      addField( "Seed",                TypeS32,       Offset( mFieldData.mSeed,                 fxFoliageReplicator ), "Random seed for foliage placement." );
      addField( "FoliageFile",         TypeFilename,  Offset( mFieldData.mFoliageFile,          fxFoliageReplicator ), "Image file for the foliage texture." );
      addField( "FoliageCount",        TypeS32,       Offset( mFieldData.mFoliageCount,         fxFoliageReplicator ), "Maximum foliage instance count." );
      addField( "FoliageRetries",      TypeS32,       Offset( mFieldData.mFoliageRetries,       fxFoliageReplicator ), "Number of times to try placing a foliage instance before giving up." );
   endGroup( "Media" );	// MM: Added Group Footer.

   addGroup( "Area" );	// MM: Added Group Header.
      addField( "InnerRadiusX",        TypeS32,       Offset( mFieldData.mInnerRadiusX,         fxFoliageReplicator ), "Placement area inner radius on the X axis" );
      addField( "InnerRadiusY",        TypeS32,       Offset( mFieldData.mInnerRadiusY,         fxFoliageReplicator ), "Placement area inner radius on the Y axis" );
      addField( "OuterRadiusX",        TypeS32,       Offset( mFieldData.mOuterRadiusX,         fxFoliageReplicator ), "Placement area outer radius on the X axis" );
      addField( "OuterRadiusY",        TypeS32,       Offset( mFieldData.mOuterRadiusY,         fxFoliageReplicator ), "Placement area outer radius on the Y axis" );
   endGroup( "Area" );	// MM: Added Group Footer.

   addGroup( "Dimensions" );	// MM: Added Group Header.
      addField( "MinWidth",            TypeF32,       Offset( mFieldData.mMinWidth,             fxFoliageReplicator ), "Minimum width of foliage billboards" );
      addField( "MaxWidth",            TypeF32,       Offset( mFieldData.mMaxWidth,             fxFoliageReplicator ), "Maximum width of foliage billboards" );
      addField( "MinHeight",           TypeF32,       Offset( mFieldData.mMinHeight,            fxFoliageReplicator ), "Minimum height of foliage billboards" );
      addField( "MaxHeight",           TypeF32,       Offset( mFieldData.mMaxHeight,            fxFoliageReplicator ), "Maximum height of foliage billboards" );
      addField( "FixAspectRatio",      TypeBool,      Offset( mFieldData.mFixAspectRatio,       fxFoliageReplicator ), "Maintain aspect ratio of image if true. This option ignores MaxWidth." );
      addField( "FixSizeToMax",        TypeBool,      Offset( mFieldData.mFixSizeToMax,         fxFoliageReplicator ), "Use only MaxWidth and MaxHeight for billboard size. Ignores MinWidth and MinHeight." );
      addField( "OffsetZ",             TypeF32,       Offset( mFieldData.mOffsetZ,              fxFoliageReplicator ), "Offset billboards by this amount vertically." );
      addField( "RandomFlip",          TypeBool,      Offset( mFieldData.mRandomFlip,           fxFoliageReplicator ), "Randomly flip billboards left-to-right." );
      addField( "UseTrueBillboards",   TypeBool,      Offset( mFieldData.mUseTrueBillboards,    fxFoliageReplicator ), "Use camera facing billboards ( including the z axis )." );
   endGroup( "Dimensions" );	// MM: Added Group Footer.

   addGroup( "Culling" );	// MM: Added Group Header.
      addField( "UseCulling",          TypeBool,      Offset( mFieldData.mUseCulling,           fxFoliageReplicator ), "Use culling bins when enabled." );
      addField( "CullResolution",      TypeS32,       Offset( mFieldData.mCullResolution,       fxFoliageReplicator ), "Minimum size of culling bins.  Must be >= 8 and <= OuterRadius." );
      addField( "ViewDistance",        TypeF32,       Offset( mFieldData.mViewDistance,         fxFoliageReplicator ), "Maximum distance from camera where foliage appears." );
      addField( "ViewClosest",         TypeF32,       Offset( mFieldData.mViewClosest,          fxFoliageReplicator ), "Minimum distance from camera where foliage appears." );
      addField( "FadeInRegion",        TypeF32,       Offset( mFieldData.mFadeInRegion,         fxFoliageReplicator ), "Region beyond ViewDistance where foliage fades in/out." );
      addField( "FadeOutRegion",       TypeF32,       Offset( mFieldData.mFadeOutRegion,        fxFoliageReplicator ), "Region before ViewClosest where foliage fades in/out." );
      addField( "AlphaCutoff",         TypeF32,       Offset( mFieldData.mAlphaCutoff,          fxFoliageReplicator ), "Minimum alpha value allowed on foliage instances." );
      addField( "GroundAlpha",         TypeF32,       Offset( mFieldData.mGroundAlpha,          fxFoliageReplicator ), "Alpha of the foliage at ground level. 0 = transparent, 1 = opaque." );
   endGroup( "Culling" );	// MM: Added Group Footer.

   addGroup( "Animation" );	// MM: Added Group Header.
      addField( "SwayOn",              TypeBool,      Offset( mFieldData.mSwayOn,               fxFoliageReplicator ), "Foliage should sway randomly when true." );
      addField( "SwaySync",            TypeBool,      Offset( mFieldData.mSwaySync,             fxFoliageReplicator ), "Foliage instances should sway together when true and SwayOn is enabled." );
      addField( "SwayMagSide",         TypeF32,       Offset( mFieldData.mSwayMagnitudeSide,    fxFoliageReplicator ), "Left-to-right sway magnitude." );
      addField( "SwayMagFront",        TypeF32,       Offset( mFieldData.mSwayMagnitudeFront,   fxFoliageReplicator ), "Front-to-back sway magnitude." );
      addField( "MinSwayTime",         TypeF32,       Offset( mFieldData.mMinSwayTime,          fxFoliageReplicator ), "Minumum sway cycle time in seconds." );
      addField( "MaxSwayTime",         TypeF32,       Offset( mFieldData.mMaxSwayTime,          fxFoliageReplicator ), "Maximum sway cycle time in seconds." );
   endGroup( "Animation" );	// MM: Added Group Footer.

   addGroup( "Lighting" );	// MM: Added Group Header.
      addField( "LightOn",             TypeBool,      Offset( mFieldData.mLightOn,              fxFoliageReplicator ), "Foliage should be illuminated with changing lights when true." );
      addField( "LightSync",           TypeBool,      Offset( mFieldData.mLightSync,            fxFoliageReplicator ), "Foliage instances have the same lighting when set and LightOn is set." );
      addField( "MinLuminance",        TypeF32,       Offset( mFieldData.mMinLuminance,         fxFoliageReplicator ), "Minimum luminance for foliage instances." );
      addField( "MaxLuminance",        TypeF32,       Offset( mFieldData.mMaxLuminance,         fxFoliageReplicator ), "Maximum luminance for foliage instances." );
      addField( "LightTime",           TypeF32,       Offset( mFieldData.mLightTime,            fxFoliageReplicator ), "Time before foliage illumination cycle repeats." );
   endGroup( "Lighting" );	// MM: Added Group Footer.

   addGroup( "Restrictions" );	// MM: Added Group Header.
      addField( "AllowOnTerrain",      TypeBool,      Offset( mFieldData.mAllowOnTerrain,       fxFoliageReplicator ), "Foliage will be placed on terrain when set." );
      addField( "AllowOnStatics",      TypeBool,      Offset( mFieldData.mAllowStatics,         fxFoliageReplicator ), "Foliage will be placed on Static shapes when set." );
      addField( "AllowOnWater",        TypeBool,      Offset( mFieldData.mAllowOnWater,         fxFoliageReplicator ), "Foliage will be placed on/under water when set." );
      addField( "AllowWaterSurface",   TypeBool,      Offset( mFieldData.mAllowWaterSurface,    fxFoliageReplicator ), "Foliage will be placed on water when set. Requires AllowOnWater." );
      addField( "AllowedTerrainSlope", TypeS32,       Offset( mFieldData.mAllowedTerrainSlope,  fxFoliageReplicator ), "Maximum surface angle allowed for foliage instances." );
   endGroup( "Restrictions" );	// MM: Added Group Footer.

   // Initialise parents' persistent fields.
   Parent::initPersistFields();
}

//------------------------------------------------------------------------------

void fxFoliageReplicator::CreateFoliage(void)
{
   F32				HypX, HypY;
   F32				Angle;
   U32				RelocationRetry;
   Point3F			FoliagePosition;
   Point3F			FoliageStart;
   Point3F			FoliageEnd;
   Point3F			FoliageScale;
   bool			CollisionResult;
   RayInfo			RayEvent;

   // Let's get a minimum bounding volume.
   Point3F	MinPoint( -0.5, -0.5, -0.5 );
   Point3F	MaxPoint(  0.5,  0.5,  0.5 );

   // Check Host.
   AssertFatal(isClientObject(), "Trying to create Foliage on Server, this is bad!")

      // Cannot continue without Foliage Texture!
      if (dStrlen(mFieldData.mFoliageFile) == 0) 
         return;

   // Check that we can position somewhere!
   if (!(	mFieldData.mAllowOnTerrain ||
      mFieldData.mAllowStatics ||
      mFieldData.mAllowOnWater))
   {
      // Problem ...
      Con::warnf(ConsoleLogEntry::General, "fxFoliageReplicator - Could not place Foliage, All alloweds are off!");

      // Return here.
      return;
   }

   // Destroy Foliage if we've already got some.
   if (mCurrentFoliageCount != 0) DestroyFoliage();

   // Inform the user if culling has been disabled!
   if (!mFieldData.mUseCulling)
   {
      // Console Output.
      Con::printf("fxFoliageReplicator - Culling has been disabled!");
   }

   // ----------------------------------------------------------------------------------------------------------------------
   // > Calculate the Potential Foliage Nodes Required to achieve the selected culling resolution.
   // > Populate Quad-tree structure to depth determined by culling resolution.
   //
   // A little explanation is called for here ...
   //
   //			The approach to this problem has been choosen to make it *much* easier for
   //			the user to control the quad-tree culling resolution.  The user enters a single
   //			world-space value 'mCullResolution' which controls the highest resolution at
   //			which the replicator will check visibility culling.
   //
   //			example:	If 'mCullResolution' is 32 and the size of the replicated area is 128 radius
   //						(256 diameter) then this results in the replicator creating a quad-tree where
   //						there are 256/32 = 8x8 blocks.  Each of these can be checked to see if they
   //						reside within the viewing frustum and if not then they get culled therefore
   //						removing the need to parse all the billboards that occcupy that region.
   //						Most of the time you will get better than this as the culling algorithm will
   //						check the culling pyramid from the top to bottom e.g. the follow 'blocks'
   //						will be checked:-
   //
   //						 1 x 256 x 256 (All of replicated area)
   //						 4 x 128 x 128 (4 corners of above)
   //						16 x  64 x  64 (16 x 4 corners of above)
   //						etc.
   //
   //
   //	1.		First-up, the replicator needs to create a fixed-list of quad-tree nodes to work with.
   //
   //			To calculate this we take the largest outer-radius value set in the replicator and
   //			calculate how many quad-tree levels are required to achieve the selected 'mCullResolution'.
   //			One of the initial problems is that the replicator has seperate radii values for X & Y.
   //			This can lead to a culling resolution smaller in one axis than the other if there is a
   //			difference between the Outer-Radii.  Unfortunately, we just live with this as there is
   //			not much we can do here if we still want to allow the user to have this kind of
   //			elliptical placement control.
   //
   //			To calculate the number of nodes needed we using the following equation:-
   //
   //			Note:- We are changing the Logarithmic bases from 10 -> 2 ... grrrr!
   //
   //			Cr = mCullResolution
   //			Rs = Maximum Radii Diameter
   //
   //
   //				( Log10( Rs / Cr )       )
   //			int ( ---------------- + 0.5 )
   //				( Log10( 2 )             )
   //
   //					---------|
   //					 |
   //					  |			 n
   //					  /			4
   //					 /
   //					---------|
   //					   n = 0
   //
   //
   //			So basically we calculate the number of blocks in 1D at the highest resolution, then
   //			calculate the inverse exponential (base 2 - 1D) to achieve that quantity of blocks.
   //			We round that upto the next highest integer = e.  We then sum 4 to the power 0->e
   //			which gives us the correct number of nodes required.  e is also stored as the starting
   //			level value for populating the quad-tree (see 3. below).
   //
   //	2.		We then proceed to calculate the billboard positions as normal and calculate and assign
   //			each billboard a basic volume (rather than treat each as a point).  We need to take into
   //			account possible front/back swaying as well as the basic plane dimensions here.
   //			When all the billboards have been choosen we then proceed to populate the quad-tree.
   //
   //	3.		To populate the quad-tree we start with a box which completely encapsulates the volume
   //			occupied by all the billboards and enter into a recursive procedure to process that node.
   //			Processing this node involves splitting it into quadrants in X/Y untouched (for now).
   //			We then find candidate billboards with each of these quadrants searching using the
   //			current subset of shapes from the parent (this reduces the searching to a minimum and
   //			is very efficient).
   //
   //			If a quadrant does not enclose any billboards then the node is dropped otherwise it
   //			is processed again using the same procedure.
   //
   //			This happens until we have recursed through the maximum number of levels as calculated
   //			using the summation max (see equation above).  When level 0 is reached, the current list
   //			of enclosed objects is stored within the node (for the rendering algorithm).
   //
   //	4.		When this is complete we have finished here.  The next stage is when rendering takes place.
   //			An algorithm steps through the quad-tree from the top and does visibility culling on
   //			each box (with respect to the viewing frustum) and culls as appropriate.  If the box is
   //			visible then the next level is checked until we reach level 0 where the node contains
   //			a complete subset of billboards enclosed by the visible box.
   //
   //
   //	Using the above algorithm we can now generate *massive* quantities of billboards and (using the
   //	appropriate 'mCullResolution') only visible blocks of billboards will be processed.
   //
   //	- Melv.
   //
   // ----------------------------------------------------------------------------------------------------------------------



   // ----------------------------------------------------------------------------------------------------------------------
   // Step 1.
   // ----------------------------------------------------------------------------------------------------------------------

   // Calculate the maximum dimension.
   F32 MaxDimension = 2.0f * ( (mFieldData.mOuterRadiusX > mFieldData.mOuterRadiusY) ? mFieldData.mOuterRadiusX : mFieldData.mOuterRadiusY );

   // Let's check that our cull resolution is not greater than half our maximum dimension (and less than 1).
   if (mFieldData.mCullResolution > (MaxDimension/2) || mFieldData.mCullResolution < 8)
   {
      // Problem ...
      Con::warnf(ConsoleLogEntry::General, "fxFoliageReplicator - Could create Foliage, invalid Culling Resolution!");
      Con::warnf(ConsoleLogEntry::General, "fxFoliageReplicator - Culling Resolution *must* be >=8 or <= %0.2f!", (MaxDimension/2));

      // Return here.
      return;
   }

   // Take first Timestamp.
   F32 mStartCreationTime = (F32) Platform::getRealMilliseconds();

   // Calculate the quad-tree levels needed for selected 'mCullResolution'.
   mQuadTreeLevels = (U32)(mCeil(mLog( MaxDimension / mFieldData.mCullResolution ) / mLog( 2.0f )));

   // Calculate the number of potential nodes required.
   mPotentialFoliageNodes = 0;
   for (U32 n = 0; n <= mQuadTreeLevels; n++)
      mPotentialFoliageNodes += (U32)(mCeil(mPow(4.0f, (F32) n)));	// Ceil to be safe!

   // ----------------------------------------------------------------------------------------------------------------------
   // Step 2.
   // ----------------------------------------------------------------------------------------------------------------------

   // Set Seed.
   RandomGen.setSeed(mFieldData.mSeed);

   // Add Foliage.
   for (U32 idx = 0; idx < mFieldData.mFoliageCount; idx++)
   {
      fxFoliageItem*	pFoliageItem;
      Point3F			FoliageOffsetPos;

      // Reset Relocation Retry.
      RelocationRetry = mFieldData.mFoliageRetries;

      // Find it a home ...
      do
      {
         // Get the fxFoliageReplicator Position.
         FoliagePosition = getPosition();

         // Calculate a random offset
         HypX	= RandomGen.randF((F32) mFieldData.mInnerRadiusX < mFieldData.mOuterRadiusX ? mFieldData.mInnerRadiusX : mFieldData.mOuterRadiusX, (F32) mFieldData.mOuterRadiusX);
         HypY	= RandomGen.randF((F32) mFieldData.mInnerRadiusY < mFieldData.mOuterRadiusY ? mFieldData.mInnerRadiusY : mFieldData.mOuterRadiusY, (F32) mFieldData.mOuterRadiusY);
         Angle	= RandomGen.randF(0, (F32) M_2PI);

         // Calcualte the new position.
         FoliagePosition.x += HypX * mCos(Angle);
         FoliagePosition.y += HypY * mSin(Angle);

         // Initialise RayCast Search Start/End Positions.
         FoliageStart = FoliageEnd = FoliagePosition;
         FoliageStart.z = 2000.f;
         FoliageEnd.z= -2000.f;

         // Perform Ray Cast Collision on Client.
         CollisionResult = gClientContainer.castRay(	FoliageStart, FoliageEnd, FXFOLIAGEREPLICATOR_COLLISION_MASK, &RayEvent);

         // Did we hit anything?
         if (CollisionResult)
         {
            // For now, let's pretend we didn't get a collision.
            CollisionResult = false;

            // Yes, so get it's type.
            U32 CollisionType = RayEvent.object->getTypeMask();

            // Check Illegal Placements, fail if we hit a disallowed type.
            if (((CollisionType & TerrainObjectType) && !mFieldData.mAllowOnTerrain)	||
               ((CollisionType & StaticShapeObjectType ) && !mFieldData.mAllowStatics)	||
               ((CollisionType & WaterObjectType) && !mFieldData.mAllowOnWater) ) continue;

            // If we collided with water and are not allowing on the water surface then let's find the
            // terrain underneath and pass this on as the original collision else fail.
            if ((CollisionType & WaterObjectType) && !mFieldData.mAllowWaterSurface &&
               !gClientContainer.castRay( FoliageStart, FoliageEnd, FXFOLIAGEREPLICATOR_NOWATER_COLLISION_MASK, &RayEvent)) continue;

            // We passed with flying colour so carry on.
            CollisionResult = true;
         }

         // Invalidate if we are below Allowed Terrain Angle.
         if (RayEvent.normal.z < mSin(mDegToRad(90.0f-mFieldData.mAllowedTerrainSlope))) CollisionResult = false;

         // Wait until we get a collision.
      } while(!CollisionResult && --RelocationRetry);

      // Check for Relocation Problem.
      if (RelocationRetry > 0)
      {
         // Adjust Impact point.
         RayEvent.point.z += mFieldData.mOffsetZ;

         // Set New Position.
         FoliagePosition = RayEvent.point;
      }
      else
      {
         // Warning.
         Con::warnf(ConsoleLogEntry::General, "fxFoliageReplicator - Could not find satisfactory position for Foliage!");

         // Skip to next.
         continue;
      }

      // Monitor the total volume.
      FoliageOffsetPos = FoliagePosition - getPosition();
      MinPoint.setMin(FoliageOffsetPos);
      MaxPoint.setMax(FoliageOffsetPos);

      // Create our Foliage Item.
      pFoliageItem = new fxFoliageItem;

      // Reset Frame Serial.
      pFoliageItem->LastFrameSerialID = 0;

      // Reset Transform.
      pFoliageItem->Transform.identity();

      // Set Position.
      pFoliageItem->Transform.setColumn(3, FoliagePosition);

      // Are we fixing size @ max?
      if (mFieldData.mFixSizeToMax)
      {
         // Yes, so set height maximum height.
         pFoliageItem->Height = mFieldData.mMaxHeight;
         // Is the Aspect Ratio Fixed?
         if (mFieldData.mFixAspectRatio)
            // Yes, so lock to height.
            pFoliageItem->Width = pFoliageItem->Height;
         else
            // No, so set width to maximum width.
            pFoliageItem->Width = mFieldData.mMaxWidth;
      }
      else
      {
         // No, so choose a new Scale.
         pFoliageItem->Height = RandomGen.randF(mFieldData.mMinHeight, mFieldData.mMaxHeight);
         // Is the Aspect Ratio Fixed?
         if (mFieldData.mFixAspectRatio)
            // Yes, so lock to height.
            pFoliageItem->Width = pFoliageItem->Height;
         else
            // No, so choose a random width.
            pFoliageItem->Width = RandomGen.randF(mFieldData.mMinWidth, mFieldData.mMaxWidth);
      }

      // Are we randomly flipping horizontally?
      if (mFieldData.mRandomFlip)
         // Yes, so choose a random flip for this object.
         pFoliageItem->Flipped = (RandomGen.randF(0, 1000) < 500.0f) ? false : true;
      else
         // No, so turn-off flipping.
         pFoliageItem->Flipped = false;		

      // Calculate Foliage Item World Box.
      // NOTE:-	We generate a psuedo-volume here.  It's basically the volume to which the
      //			plane can move and this includes swaying!
      //
      // Is Sway On?
      if (mFieldData.mSwayOn)
      {
         // Yes, so take swaying into account...
         pFoliageItem->FoliageBox.minExtents =	FoliagePosition +
            Point3F(-pFoliageItem->Width / 2.0f - mFieldData.mSwayMagnitudeSide,
            -0.5f - mFieldData.mSwayMagnitudeFront,
            pFoliageItem->Height );

         pFoliageItem->FoliageBox.maxExtents =	FoliagePosition +
            Point3F(+pFoliageItem->Width / 2.0f + mFieldData.mSwayMagnitudeSide,
            +0.5f + mFieldData.mSwayMagnitudeFront,
            pFoliageItem->Height );
      }
      else
      {
         // No, so give it a minimum volume...
         pFoliageItem->FoliageBox.minExtents =	FoliagePosition +
            Point3F(-pFoliageItem->Width / 2.0f,
            -0.5f,
            pFoliageItem->Height );

         pFoliageItem->FoliageBox.maxExtents =	FoliagePosition +
            Point3F(+pFoliageItem->Width / 2.0f,
            +0.5f,
            pFoliageItem->Height );
      }

      // Store Shape in Replicated Shapes Vector.
      mReplicatedFoliage.push_back(pFoliageItem);

      // Increase Foliage Count.
      mCurrentFoliageCount++;
   }

   // Is Lighting On?
   if (mFieldData.mLightOn)
   {
      // Yes, so reset Global Light phase.
      mGlobalLightPhase = 0.0f;
      // Set Global Light Time Ratio.
      mGlobalLightTimeRatio = PeriodLenMinus / mFieldData.mLightTime;

      // Yes, so step through Foliage.
      for (U32 idx = 0; idx < mCurrentFoliageCount; idx++)
      {
         fxFoliageItem*	pFoliageItem;

         // Fetch the Foliage Item.
         pFoliageItem = mReplicatedFoliage[idx];

         // Do we have an item?
         if (pFoliageItem)
         {
            // Yes, so are lights syncronised?
            if (mFieldData.mLightSync)
            {
               pFoliageItem->LightTimeRatio = 1.0f;
               pFoliageItem->LightPhase = 0.0f;
            }
            else
            {
               // No, so choose a random Light phase.
               pFoliageItem->LightPhase = RandomGen.randF(0, PeriodLenMinus);
               // Set Light Time Ratio.
               pFoliageItem->LightTimeRatio = PeriodLenMinus / mFieldData.mLightTime;
            }
         }
      }

   }

   // Is Swaying Enabled?
   if (mFieldData.mSwayOn)
   {
      // Yes, so reset Global Sway phase.
      mGlobalSwayPhase = 0.0f;
      // Always set Global Sway Time Ratio.
      mGlobalSwayTimeRatio = PeriodLenMinus / RandomGen.randF(mFieldData.mMinSwayTime, mFieldData.mMaxSwayTime);

      // Yes, so step through Foliage.
      for (U32 idx = 0; idx < mCurrentFoliageCount; idx++)
      {			
         fxFoliageItem*	pFoliageItem;

         // Fetch the Foliage Item.
         pFoliageItem = mReplicatedFoliage[idx];
         // Do we have an item?
         if (pFoliageItem)
         {
            // Are we using Sway Sync?
            if (mFieldData.mSwaySync)
            {
               pFoliageItem->SwayPhase = 0;
               pFoliageItem->SwayTimeRatio = mGlobalSwayTimeRatio;
            }
            else
            {
               // No, so choose a random Sway phase.
               pFoliageItem->SwayPhase = RandomGen.randF(0, PeriodLenMinus);
               // Set to random Sway Time.
               pFoliageItem->SwayTimeRatio = PeriodLenMinus / RandomGen.randF(mFieldData.mMinSwayTime, mFieldData.mMaxSwayTime);
            }
         }
      }
   }

   // Update our Object Volume.
   mObjBox.minExtents.set(MinPoint);
   mObjBox.maxExtents.set(MaxPoint);
   setTransform(mObjToWorld);

   // ----------------------------------------------------------------------------------------------------------------------
   // Step 3.
   // ----------------------------------------------------------------------------------------------------------------------

   // Reset Next Allocated Node to Stack base.
   mNextAllocatedNodeIdx = 0;

   // Allocate a new Node.
   fxFoliageQuadrantNode* pNewNode = new fxFoliageQuadrantNode;

   // Store it in the Quad-tree.
   mFoliageQuadTree.push_back(pNewNode);

   // Populate Initial Node.
   //
   // Set Start Level.
   pNewNode->Level = mQuadTreeLevels;
   // Calculate Total Foliage Area.
   pNewNode->QuadrantBox = getWorldBox();
   // Reset Quadrant child nodes.
   pNewNode->QuadrantChildNode[0] =
      pNewNode->QuadrantChildNode[1] =
      pNewNode->QuadrantChildNode[2] =
      pNewNode->QuadrantChildNode[3] = NULL;

   // Create our initial cull list with *all* billboards into.
   fxFoliageCulledList CullList;
   CullList.mCulledObjectSet = mReplicatedFoliage;

   // Move to next node Index.
   mNextAllocatedNodeIdx++;

   // Let's start this thing going by recursing it's children.
   ProcessNodeChildren(pNewNode, &CullList);

   // Calculate Elapsed Time and take new Timestamp.
   F32 ElapsedTime = (Platform::getRealMilliseconds() - mStartCreationTime) * 0.001f;

   // Console Output.
   Con::printf("fxFoliageReplicator - Lev: %d  PotNodes: %d  Used: %d  Objs: %d  Time: %0.4fs.",
      mQuadTreeLevels,
      mPotentialFoliageNodes,
      mNextAllocatedNodeIdx-1,
      mBillboardsAcquired,
      ElapsedTime);

   // Dump (*very*) approximate allocated memory.
   F32 MemoryAllocated = (F32) ((mNextAllocatedNodeIdx-1) * sizeof(fxFoliageQuadrantNode));
   MemoryAllocated		+=	mCurrentFoliageCount * sizeof(fxFoliageItem);
   MemoryAllocated		+=	mCurrentFoliageCount * sizeof(fxFoliageItem*);
   Con::printf("fxFoliageReplicator - Approx. %0.2fMb allocated.", MemoryAllocated / 1048576.0f);

   // ----------------------------------------------------------------------------------------------------------------------

   SetupBuffers();

   // Take first Timestamp.
   mLastRenderTime = Platform::getVirtualMilliseconds();
}

void fxFoliageReplicator::SetupShader()
{
   if ( !mShaderData )
   {
      if ( !Sim::findObject( "fxFoliageReplicatorShader", mShaderData ) )
      {
         Con::errorf( "fxFoliageReplicator::SetupShader - could not find ShaderData named fxFoliageReplicatorShader" );
         return;
      }
   }

   Vector<GFXShaderMacro> macros;
   if ( mFieldData.mUseTrueBillboards )   
      macros.push_back( GFXShaderMacro( "TRUE_BILLBOARD" ) );
   
   mShader = mShaderData->getShader( macros );

   if ( !mShader )
      return;

   
   mFoliageShaderConsts = mShader->allocConstBuffer();

   mFoliageShaderProjectionSC          = mShader->getShaderConstHandle( "$projection" );
   mFoliageShaderWorldSC               = mShader->getShaderConstHandle( "$world" );
   mFoliageShaderGlobalSwayPhaseSC     = mShader->getShaderConstHandle( "$GlobalSwayPhase" );
   mFoliageShaderSwayMagnitudeSideSC   = mShader->getShaderConstHandle( "$SwayMagnitudeSide" );
   mFoliageShaderSwayMagnitudeFrontSC  = mShader->getShaderConstHandle( "$SwayMagnitudeFront" );
   mFoliageShaderGlobalLightPhaseSC    = mShader->getShaderConstHandle( "$GlobalLightPhase" );
   mFoliageShaderLuminanceMagnitudeSC  = mShader->getShaderConstHandle( "$LuminanceMagnitude" );
   mFoliageShaderLuminanceMidpointSC   = mShader->getShaderConstHandle( "$LuminanceMidpoint" );
   mFoliageShaderDistanceRangeSC       = mShader->getShaderConstHandle( "$DistanceRange" );
   mFoliageShaderCameraPosSC           = mShader->getShaderConstHandle( "$CameraPos" );
   mFoliageShaderTrueBillboardSC       = mShader->getShaderConstHandle( "$TrueBillboard" );
   mFoliageShaderGroundAlphaSC         = mShader->getShaderConstHandle( "$groundAlpha" );
   mFoliageShaderAmbientColorSC        = mShader->getShaderConstHandle( "$ambient" );     
   mDiffuseTextureSC							= mShader->getShaderConstHandle( "$diffuseMap" );
   mAlphaMapTextureSC						= mShader->getShaderConstHandle( "$alphaMap" ); 
}

// Ok, what we do is let the older code setup the FoliageItem list and the QuadTree.
// Then we build the Vertex and Primitive buffers here.  It would probably be
// slightly more memory efficient to build the buffers directly, but we
// want to sort the items within the buffer by the quadtreenodes
void fxFoliageReplicator::SetupBuffers()
{
   // Following two arrays are used to build the vertex and primitive buffers.	
   Point3F basePoints[8];
   basePoints[0] = Point3F(-0.5f, 0.0f, 1.0f);
   basePoints[1] = Point3F(-0.5f, 0.0f, 0.0f);
   basePoints[2] = Point3F(0.5f, 0.0f, 0.0f);
   basePoints[3] = Point3F(0.5f, 0.0f, 1.0f);

   Point2F texCoords[4];
   texCoords[0] = Point2F(0.0, 0.0);
   texCoords[1] = Point2F(0.0, 1.0);
   texCoords[2] = Point2F(1.0, 1.0);
   texCoords[3] = Point2F(1.0, 0.0);	

   // Init our Primitive Buffer
   U32 indexSize = mFieldData.mFoliageCount * 6;
   U16* indices = new U16[indexSize];
   // Two triangles per particle
   for (U16 i = 0; i < mFieldData.mFoliageCount; i++) {
      U16* idx = &indices[i*6];		// hey, no offset math below, neat
      U16 vertOffset = i*4;
      idx[0] = vertOffset + 0;
      idx[1] = vertOffset + 1;
      idx[2] = vertOffset + 2;
      idx[3] = vertOffset + 2;
      idx[4] = vertOffset + 3;
      idx[5] = vertOffset + 0;
   }
   // Init the prim buffer and copy our indexes over
   U16 *ibIndices;
   mPrimBuffer.set(GFX, indexSize, 0, GFXBufferTypeStatic);
   mPrimBuffer.lock(&ibIndices);
   dMemcpy(ibIndices, indices, indexSize * sizeof(U16));
   mPrimBuffer.unlock();
   delete[] indices;

   // Now, let's init the vertex buffer
   U32 currPrimitiveStartIndex = 0;
   mVertexBuffer.set(GFX, mFieldData.mFoliageCount * 4, GFXBufferTypeStatic);
   mVertexBuffer.lock();
   U32 idx = 0;	
   for (S32 qtIdx = 0; qtIdx < mFoliageQuadTree.size(); qtIdx++) {
      fxFoliageQuadrantNode* quadNode = mFoliageQuadTree[qtIdx];
      if (quadNode->Level == 0) {
         quadNode->startIndex = currPrimitiveStartIndex;
         quadNode->primitiveCount = 0;
         // Ok, there should be data in here!
         for (S32 i = 0; i < quadNode->RenderList.size(); i++) {
            fxFoliageItem* pFoliageItem = quadNode->RenderList[i];
            if (pFoliageItem->LastFrameSerialID == 0) {
               pFoliageItem->LastFrameSerialID++;
               // Dump it into the vertex buffer
               for (U32 vertIndex = 0; vertIndex < 4; vertIndex++) {
                  GFXVertexFoliage *vert = &mVertexBuffer[(idx*4) + vertIndex];
                  // This is the position of the billboard.
                  vert->point = pFoliageItem->Transform.getPosition();			
                  // Normal contains the point of the billboard (except for the y component, see below)
                  vert->normal = basePoints[vertIndex];

                  vert->normal.x *= pFoliageItem->Width;
                  vert->normal.z *= pFoliageItem->Height;
                  // Handle texture coordinates
                  vert->texCoord = texCoords[vertIndex];				
                  if (pFoliageItem->Flipped)
                     vert->texCoord.x = 1.0f - vert->texCoord.x;
                  // Handle sway. Sway is stored in a texture coord. The x coordinate is the sway phase multiplier, 
                  // the y coordinate determines if this vertex actually sways or not.
                  if ((vertIndex == 0) || (vertIndex == 3)) {
                     vert->texCoord2.set(pFoliageItem->SwayTimeRatio / mGlobalSwayTimeRatio, 1.0f);
                  } else {
                     vert->texCoord2.set(0.0f, 0.0f);
                  }
                  // Handle lighting, lighting happens at the same time as global so this is just an offset.
                  vert->normal.y = pFoliageItem->LightPhase;
               }
               idx++;
               quadNode->primitiveCount += 2;
               currPrimitiveStartIndex += 6; 
            }
         }
      }
   }
   mVertexBuffer.unlock();	

   DestroyFoliageItems();
}

//------------------------------------------------------------------------------

Box3F fxFoliageReplicator::FetchQuadrant(Box3F Box, U32 Quadrant)
{
   Box3F QuadrantBox;

   // Select Quadrant.
   switch(Quadrant)
   {
      // UL.
   case 0:
      QuadrantBox.minExtents = Box.minExtents + Point3F(0, Box.len_y()/2, 0);
      QuadrantBox.maxExtents = QuadrantBox.minExtents + Point3F(Box.len_x()/2, Box.len_y()/2, Box.len_z());
      break;

      // UR.
   case 1:
      QuadrantBox.minExtents = Box.minExtents + Point3F(Box.len_x()/2, Box.len_y()/2, 0);
      QuadrantBox.maxExtents = QuadrantBox.minExtents + Point3F(Box.len_x()/2, Box.len_y()/2, Box.len_z());
      break;

      // LL.
   case 2:
      QuadrantBox.minExtents = Box.minExtents;
      QuadrantBox.maxExtents = QuadrantBox.minExtents + Point3F(Box.len_x()/2, Box.len_y()/2, Box.len_z());
      break;

      // LR.
   case 3:
      QuadrantBox.minExtents = Box.minExtents + Point3F(Box.len_x()/2, 0, 0);
      QuadrantBox.maxExtents = QuadrantBox.minExtents + Point3F(Box.len_x()/2, Box.len_y()/2, Box.len_z());
      break;

   default:
      return Box;
   }

   return QuadrantBox;
}

//------------------------------------------------------------------------------

void fxFoliageReplicator::ProcessNodeChildren(fxFoliageQuadrantNode* pParentNode, fxFoliageCulledList* pCullList)
{
   // ---------------------------------------------------------------
   // Split Node into Quadrants and Process each.
   // ---------------------------------------------------------------

   // Process All Quadrants (UL/UR/LL/LR).
   for (U32 q = 0; q < 4; q++)
      ProcessQuadrant(pParentNode, pCullList, q);
}

//------------------------------------------------------------------------------

void fxFoliageReplicator::ProcessQuadrant(fxFoliageQuadrantNode* pParentNode, fxFoliageCulledList* pCullList, U32 Quadrant)
{
   // Fetch Quadrant Box.
   const Box3F QuadrantBox = FetchQuadrant(pParentNode->QuadrantBox, Quadrant);

   // Create our new Cull List.
   fxFoliageCulledList CullList(QuadrantBox, pCullList);

   // Did we get any objects?
   if (CullList.GetListCount() > 0)
   {
      // Yes, so allocate a new Node.
      fxFoliageQuadrantNode* pNewNode = new fxFoliageQuadrantNode;

      // Store it in the Quad-tree.
      mFoliageQuadTree.push_back(pNewNode);

      // Move to next node Index.
      mNextAllocatedNodeIdx++;

      // Populate Quadrant Node.
      //
      // Next Sub-level.
      pNewNode->Level = pParentNode->Level - 1;
      // Calculate Quadrant Box.
      pNewNode->QuadrantBox = QuadrantBox;
      // Reset Child Nodes.
      pNewNode->QuadrantChildNode[0] =
         pNewNode->QuadrantChildNode[1] =
         pNewNode->QuadrantChildNode[2] =
         pNewNode->QuadrantChildNode[3] = NULL;

      // Put a reference in parent.
      pParentNode->QuadrantChildNode[Quadrant] = pNewNode;

      // If we're not at sub-level 0 then process this nodes children.
      if (pNewNode->Level != 0) ProcessNodeChildren(pNewNode, &CullList);
      // If we've reached sub-level 0 then store Cull List (for rendering).
      if (pNewNode->Level == 0)
      {
         // Store the render list from our culled object set.
         pNewNode->RenderList = CullList.mCulledObjectSet;
         // Keep track of the total billboard acquired.
         mBillboardsAcquired += CullList.GetListCount();
      }
   }
}

//------------------------------------------------------------------------------

void fxFoliageReplicator::SyncFoliageReplicators(void)
{
   // Check Host.
   AssertFatal(isServerObject(), "We *MUST* be on server when Synchronising Foliage!")

      // Find the Replicator Set.
   SimSet *fxFoliageSet = dynamic_cast<SimSet*>(Sim::findObject("fxFoliageSet"));

   // Return if Error.
   if (!fxFoliageSet)
   {
      // Console Warning.
      Con::warnf("fxFoliageReplicator - Cannot locate the 'fxFoliageSet', this is bad!");
      // Return here.
      return;
   }

   // Parse Replication Object(s).
   for (SimSetIterator itr(fxFoliageSet); *itr; ++itr)
   {
      // Fetch the Replicator Object.
      fxFoliageReplicator* Replicator = static_cast<fxFoliageReplicator*>(*itr);
      // Set Foliage Replication Mask.
      if (Replicator->isServerObject())
      {
         Con::printf("fxFoliageReplicator - Restarting fxFoliageReplicator Object...");
         Replicator->setMaskBits(FoliageReplicationMask);
      }
   }

   // Info ...
   Con::printf("fxFoliageReplicator - Client Foliage Sync has completed.");
}


//------------------------------------------------------------------------------
// Lets chill our memory requirements out a little
void fxFoliageReplicator::DestroyFoliageItems()
{
   // Remove shapes.
   for (S32 idx = 0; idx < mReplicatedFoliage.size(); idx++)
   {
      fxFoliageItem*	pFoliageItem;

      // Fetch the Foliage Item.
      pFoliageItem = mReplicatedFoliage[idx];

      // Delete Shape.
      if (pFoliageItem) delete pFoliageItem;
   }
   // Clear the Replicated Foliage Vector.
   mReplicatedFoliage.clear();

   // Clear out old references also
   for (S32 qtIdx = 0; qtIdx < mFoliageQuadTree.size(); qtIdx++) {
      fxFoliageQuadrantNode* quadNode = mFoliageQuadTree[qtIdx];
      if (quadNode->Level == 0) {
         quadNode->RenderList.clear();
      }
   }
}

void fxFoliageReplicator::DestroyFoliage(void)
{
   // Check Host.
   AssertFatal(isClientObject(), "Trying to destroy Foliage on Server, this is bad!")

      // Destroy Quad-tree.
      mPotentialFoliageNodes = 0;
   // Reset Billboards Acquired.
   mBillboardsAcquired = 0;

   // Finish if we didn't create any shapes.
   if (mCurrentFoliageCount == 0) return;

   DestroyFoliageItems();

   // Let's remove the Quad-Tree allocations.
   for (	Vector<fxFoliageQuadrantNode*>::iterator QuadNodeItr = mFoliageQuadTree.begin();
      QuadNodeItr != mFoliageQuadTree.end();
      QuadNodeItr++ )
   {
      // Remove the node.
      delete *QuadNodeItr;
   }

   // Clear the Foliage Quad-Tree Vector.
   mFoliageQuadTree.clear();

   // Clear the Frustum Render Set Vector.
   mFrustumRenderSet.mVisObjectSet.clear();

   // Reset Foliage Count.
   mCurrentFoliageCount = 0;
}

//------------------------------------------------------------------------------

void fxFoliageReplicator::StartUp(void)
{
   // Flag, Client Replication Started.
   mClientReplicationStarted = true;

   // Create foliage on Client.
   if (isClientObject()) CreateFoliage();
}

//------------------------------------------------------------------------------

bool fxFoliageReplicator::onAdd()
{
   if(!Parent::onAdd()) return(false);

   // Add the Replicator to the Replicator Set.
   dynamic_cast<SimSet*>(Sim::findObject("fxFoliageSet"))->addObject(this);

   // Set Default Object Box.
   mObjBox.minExtents.set( -0.5, -0.5, -0.5 );
   mObjBox.maxExtents.set(  0.5,  0.5,  0.5 );
   resetWorldBox();
   setRenderTransform(mObjToWorld);

   // Add to Scene.
   addToScene();

   // Are we on the client?
   if ( isClientObject() )
   {
      // Yes, so load foliage texture.
      if( mFieldData.mFoliageFile != NULL && dStrlen(mFieldData.mFoliageFile) > 0 )
         mFieldData.mFoliageTexture = GFXTexHandle( mFieldData.mFoliageFile, &GFXDefaultStaticDiffuseProfile, avar("%s() - mFieldData.mFoliageTexture (line %d)", __FUNCTION__, __LINE__) );

      if ((GFXTextureObject*) mFieldData.mFoliageTexture == NULL)
         Con::printf("fxFoliageReplicator:  %s is an invalid or missing foliage texture file.", mFieldData.mFoliageFile);

      mAlphaLookup = new GBitmap(AlphaTexLen, 1);
      computeAlphaTex();

      // Register for notification when GhostAlways objects are done loading
      NetConnection::smGhostAlwaysDone.notify( this, &fxFoliageReplicator::onGhostAlwaysDone );

      SetupShader();
   }

   // Return OK.
   return(true);
}

//------------------------------------------------------------------------------

void fxFoliageReplicator::onRemove()
{
   // Remove the Replicator from the Replicator Set.
   dynamic_cast<SimSet*>(Sim::findObject("fxFoliageSet"))->removeObject(this);

   NetConnection::smGhostAlwaysDone.remove( this, &fxFoliageReplicator::onGhostAlwaysDone );

   // Remove from Scene.
   removeFromScene();

   // Are we on the Client?
   if (isClientObject())
   {
      // Yes, so destroy Foliage.
      DestroyFoliage();

      // Remove Texture.
      mFieldData.mFoliageTexture = NULL;

      mShader = NULL;
   }

   // Do Parent.
   Parent::onRemove();
}

//------------------------------------------------------------------------------

void fxFoliageReplicator::onGhostAlwaysDone()
{
   if ( isClientObject() )
      CreateFoliage();
}

//------------------------------------------------------------------------------

void fxFoliageReplicator::inspectPostApply()
{
   // Set Parent.
   Parent::inspectPostApply();

   // Set Foliage Replication Mask (this object only).
   setMaskBits(FoliageReplicationMask);

   mDirty = true;
}

//------------------------------------------------------------------------------

DefineEngineFunction(StartFoliageReplication, void,(),, "Activates the foliage replicator.\n"
													"@tsexample\n"
														"// Call the function\n"
														"StartFoliageReplication();\n"
													"@endtsexample\n"
													"@ingroup Foliage")
{
   // Find the Replicator Set.
   SimSet *fxFoliageSet = dynamic_cast<SimSet*>(Sim::findObject("fxFoliageSet"));

   // Return if Error.
   if (!fxFoliageSet)
   {
      // Console Warning.
      Con::warnf("fxFoliageReplicator - Cannot locate the 'fxFoliageSet', this is bad!");
      // Return here.
      return;
   }

   // Parse Replication Object(s).
   U32 startupCount = 0;
   for (SimSetIterator itr(fxFoliageSet); *itr; ++itr)
   {
      // Fetch the Replicator Object.
      fxFoliageReplicator* Replicator = static_cast<fxFoliageReplicator*>(*itr);

      // Start Client Objects Only.
      if (Replicator->isClientObject())
      {
         Replicator->StartUp();
         startupCount++;
      }
   }

   // Info ...
   Con::printf("fxFoliageReplicator - replicated client foliage for %d objects", startupCount);
}

//------------------------------------------------------------------------------

void fxFoliageReplicator::prepRenderImage( SceneRenderState* state )
{
   ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
   ri->renderDelegate.bind(this, &fxFoliageReplicator::renderObject);
   ri->type = RenderPassManager::RIT_Foliage;
   state->getRenderPass()->addInst( ri );
}

//
// RENDERING
//
void fxFoliageReplicator::computeAlphaTex()
{
   // Distances used in alpha
   const F32	ClippedViewDistance		= mFieldData.mViewDistance;
   const F32	MaximumViewDistance		= ClippedViewDistance + mFieldData.mFadeInRegion;

   // This is used for the alpha computation in the shader.
   for (U32 i = 0; i < AlphaTexLen; i++) {
      F32 Distance = ((float) i / (float) AlphaTexLen) * MaximumViewDistance;
      F32 ItemAlpha = 1.0f;
      // Are we fading out?
      if (Distance < mFieldData.mViewClosest)
      {
         // Yes, so set fade-out.
         ItemAlpha = 1.0f - ((mFieldData.mViewClosest - Distance) * mFadeOutGradient);
      }
      // No, so are we fading in?
      else if (Distance > ClippedViewDistance)
      {
         // Yes, so set fade-in
         ItemAlpha = 1.0f - ((Distance - ClippedViewDistance) * mFadeInGradient);
      }

      // Set texture info
      ColorI c((U8) (255.0f * ItemAlpha), 0, 0);
      mAlphaLookup->setColor(i, 0, c);
   }
   mAlphaTexture.set(mAlphaLookup, &GFXDefaultStaticDiffuseProfile, false, String("fxFoliage Replicator Alpha Texture") );
}

// Renders a triangle stripped oval
void fxFoliageReplicator::renderArc(const F32 fRadiusX, const F32 fRadiusY)
{
   PrimBuild::begin(GFXTriangleStrip, 720);
   for (U32 Angle = mCreationAreaAngle; Angle < (mCreationAreaAngle+360); Angle++)
   {
      F32		XPos, YPos;

      // Calculate Position.
      XPos = fRadiusX * mCos(mDegToRad(-(F32)Angle));
      YPos = fRadiusY * mSin(mDegToRad(-(F32)Angle));

      // Set Colour.
      PrimBuild::color4f(mFieldData.mPlaceAreaColour.red,
         mFieldData.mPlaceAreaColour.green,
         mFieldData.mPlaceAreaColour.blue,
         AREA_ANIMATION_ARC * (Angle-mCreationAreaAngle));

      PrimBuild::vertex3f(XPos, YPos, -(F32)mFieldData.mPlacementBandHeight/2.0f);
      PrimBuild::vertex3f(XPos, YPos, +(F32)mFieldData.mPlacementBandHeight/2.0f);
   }			
   PrimBuild::end();
}

// This currently uses the primbuilder, could convert out, but why allocate the buffer if we
// never edit the misison?
void fxFoliageReplicator::renderPlacementArea(const F32 ElapsedTime)
{
   if (gEditingMission && mFieldData.mShowPlacementArea)
   {
      GFX->pushWorldMatrix();
      GFX->multWorld(getTransform());

      if (!mPlacementSB)
      {
         GFXStateBlockDesc transparent;
         transparent.setCullMode(GFXCullNone);
         transparent.alphaTestEnable = true;
         transparent.setZReadWrite(true);
         transparent.zWriteEnable = false;
         transparent.setBlend(true, GFXBlendSrcAlpha, GFXBlendInvSrcAlpha);
         mPlacementSB = GFX->createStateBlock( transparent );
      }

      GFX->setStateBlock(mPlacementSB);

      // Do we need to draw the Outer Radius?
      if (mFieldData.mOuterRadiusX || mFieldData.mOuterRadiusY)
         renderArc((F32) mFieldData.mOuterRadiusX, (F32) mFieldData.mOuterRadiusY);
      // Inner radius?
      if (mFieldData.mInnerRadiusX || mFieldData.mInnerRadiusY)
         renderArc((F32) mFieldData.mInnerRadiusX, (F32) mFieldData.mInnerRadiusY);

      GFX->popWorldMatrix();
      mCreationAreaAngle = (U32)(mCreationAreaAngle + (1000 * ElapsedTime));
      mCreationAreaAngle = mCreationAreaAngle % 360;
   }
}

void fxFoliageReplicator::renderObject(ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance* overrideMat)
{
   if (overrideMat)
      return;

   if ( !mShader )
      return;

   // If we're rendering and we haven't placed any foliage yet - do it.
   if(!mClientReplicationStarted)
   {
      Con::warnf("fxFoliageReplicator::renderObject - tried to render a non replicated fxFoliageReplicator; replicating it now...");

      StartUp();
   }

   // Calculate Elapsed Time and take new Timestamp.
   S32 Time = Platform::getVirtualMilliseconds();
   F32 ElapsedTime = (Time - mLastRenderTime) * 0.001f;
   mLastRenderTime = Time;	

   renderPlacementArea(ElapsedTime);

   if (mCurrentFoliageCount > 0) {

      if ( mRenderSB.isNull() || mDirty)
      {
         mDirty = false;

         GFXStateBlockDesc desc;

         // Debug SB
         desc.samplersDefined = true;
         desc.samplers[0].textureColorOp = GFXTOPDisable;
         desc.samplers[1].textureColorOp = GFXTOPDisable;

         mDebugSB = GFX->createStateBlock(desc);

         // Render SB
         desc.samplers[0].textureColorOp = GFXTOPModulate;
         desc.samplers[1].textureColorOp = GFXTOPModulate;
         desc.samplers[1].addressModeU = GFXAddressClamp;
         desc.samplers[1].addressModeV = GFXAddressClamp;

         desc.setBlend(true, GFXBlendSrcAlpha, GFXBlendInvSrcAlpha);
         desc.setAlphaTest(true, GFXCmpGreater, (U8) (255.0f * mFieldData.mAlphaCutoff));
         desc.setCullMode(GFXCullNone);

         mRenderSB = GFX->createStateBlock(desc);
      }

      if (!mFieldData.mHideFoliage) {
         // Animate Global Sway Phase (Modulus).  
         mGlobalSwayPhase = mGlobalSwayPhase + (mGlobalSwayTimeRatio * ElapsedTime);

         // Animate Global Light Phase (Modulus).
         mGlobalLightPhase = mGlobalLightPhase + (mGlobalLightTimeRatio * ElapsedTime);

         // Compute other light parameters
         const F32	LuminanceMidPoint		= (mFieldData.mMinLuminance + mFieldData.mMaxLuminance) / 2.0f;
         const F32	LuminanceMagnitude		= mFieldData.mMaxLuminance - LuminanceMidPoint;

         // Distances used in alpha
         const F32	ClippedViewDistance		= mFieldData.mViewDistance;
         const F32	MaximumViewDistance		= ClippedViewDistance + mFieldData.mFadeInRegion;

         if (mFoliageShaderConsts.isValid())
         {
            mFoliageShaderConsts->setSafe(mFoliageShaderGlobalSwayPhaseSC, mGlobalSwayPhase);
            mFoliageShaderConsts->setSafe(mFoliageShaderSwayMagnitudeSideSC, mFieldData.mSwayMagnitudeSide);
            mFoliageShaderConsts->setSafe(mFoliageShaderSwayMagnitudeFrontSC, mFieldData.mSwayMagnitudeFront);
            mFoliageShaderConsts->setSafe(mFoliageShaderGlobalLightPhaseSC, mGlobalLightPhase);
            mFoliageShaderConsts->setSafe(mFoliageShaderLuminanceMagnitudeSC, LuminanceMagnitude);
            mFoliageShaderConsts->setSafe(mFoliageShaderLuminanceMidpointSC, LuminanceMidPoint);

            // Set up our shader constants	
            // Projection matrix
            MatrixF proj = GFX->getProjectionMatrix();
            //proj.transpose();
            mFoliageShaderConsts->setSafe(mFoliageShaderProjectionSC, proj);

            // World transform matrix
            MatrixF world = GFX->getWorldMatrix();
            //world.transpose();

            mFoliageShaderConsts->setSafe(mFoliageShaderWorldSC, world);

            Point3F camPos = state->getCameraPosition();

            mFoliageShaderConsts->setSafe(mFoliageShaderDistanceRangeSC, MaximumViewDistance);
            mFoliageShaderConsts->setSafe(mFoliageShaderCameraPosSC, camPos);
            mFoliageShaderConsts->setSafe(mFoliageShaderTrueBillboardSC, mFieldData.mUseTrueBillboards ? 1.0f : 0.0f );
            mFoliageShaderConsts->setSafe(mFoliageShaderGroundAlphaSC, Point4F(mFieldData.mGroundAlpha, mFieldData.mGroundAlpha, mFieldData.mGroundAlpha, mFieldData.mGroundAlpha));

            if (mFoliageShaderAmbientColorSC->isValid())
               mFoliageShaderConsts->set(mFoliageShaderAmbientColorSC, state->getAmbientLightColor());

            GFX->setShaderConstBuffer(mFoliageShaderConsts);

         }

         // Blend ops
         // Set up our texture and color ops.

         GFX->setStateBlock(mRenderSB);
         GFX->setShader( mShader );

         GFX->setTexture(mDiffuseTextureSC->getSamplerRegister(), mFieldData.mFoliageTexture);
         // computeAlphaTex();		// Uncomment if we figure out how to clamp to fogAndHaze
         GFX->setTexture(mAlphaMapTextureSC->getSamplerRegister(), mAlphaTexture);

         // Setup our buffers
         GFX->setVertexBuffer(mVertexBuffer);
         GFX->setPrimitiveBuffer(mPrimBuffer);

         // If we use culling, we're going to send chunks of our buffers to the card
         if (mFieldData.mUseCulling)
         {
            // Setup the Clip-Planes.
            F32 FarClipPlane = getMin((F32)state->getFarPlane(), 
               mFieldData.mViewDistance + mFieldData.mFadeInRegion);
            mFrustumRenderSet.SetupClipPlanes(state, FarClipPlane);

            renderQuad(mFoliageQuadTree[0], getRenderTransform(), false);

            // Multipass, don't want to interrupt the vb state 
            if (mFieldData.mUseDebugInfo) 
            {
               // hey man, we're done, so it doesn't matter if we kill it to render the next part
               GFX->setStateBlock(mDebugSB);
               renderQuad(mFoliageQuadTree[0], getRenderTransform(), true);
            }
         }
         else 
         {	
            // Draw the whole shebang!
            GFX->drawIndexedPrimitive(GFXTriangleList, 0, 0, mVertexBuffer->mNumVerts, 
               0, mPrimBuffer->mIndexCount / 3);
         }
      }
   }
}

void fxFoliageReplicator::renderQuad(fxFoliageQuadrantNode* quadNode, const MatrixF& RenderTransform, const bool UseDebug)
{
   if (quadNode != NULL) {
      if (mFrustumRenderSet.IsQuadrantVisible(quadNode->QuadrantBox, RenderTransform))
      {
         // Draw the Quad Box (Debug Only).
         if (UseDebug) 
            mFrustumRenderSet.DrawQuadBox(quadNode->QuadrantBox, ColorF(0.0f, 1.0f, 0.1f, 1.0f));
         if (quadNode->Level != 0) {
            for (U32 i = 0; i < 4; i++)
               renderQuad(quadNode->QuadrantChildNode[i], RenderTransform, UseDebug);
         } else {
            if (!UseDebug)
               if(quadNode->primitiveCount)
                  GFX->drawIndexedPrimitive(GFXTriangleList, 0, 0, mVertexBuffer->mNumVerts, 
                  quadNode->startIndex, quadNode->primitiveCount);
         }
      } else {
         // Use a different color to say "I think I'm not visible!"
         if (UseDebug) 
            mFrustumRenderSet.DrawQuadBox(quadNode->QuadrantBox, ColorF(1.0f, 0.8f, 0.1f, 1.0f));
      }
   }
}

//------------------------------------------------------------------------------
// NETWORK
//------------------------------------------------------------------------------

U32 fxFoliageReplicator::packUpdate(NetConnection * con, U32 mask, BitStream * stream)
{
   // Pack Parent.
   U32 retMask = Parent::packUpdate(con, mask, stream);

   // Write Foliage Replication Flag.
   if (stream->writeFlag(mask & FoliageReplicationMask))
   {
      stream->writeAffineTransform(mObjToWorld);						// Foliage Master-Object Position.

      stream->writeFlag(mFieldData.mUseDebugInfo);					// Foliage Debug Information Flag.
      stream->write(mFieldData.mDebugBoxHeight);						// Foliage Debug Height.
      stream->write(mFieldData.mSeed);								// Foliage Seed.
      stream->write(mFieldData.mFoliageCount);						// Foliage Count.
      stream->write(mFieldData.mFoliageRetries);						// Foliage Retries.
      stream->writeString(mFieldData.mFoliageFile);					// Foliage File.

      stream->write(mFieldData.mInnerRadiusX);						// Foliage Inner Radius X.
      stream->write(mFieldData.mInnerRadiusY);						// Foliage Inner Radius Y.
      stream->write(mFieldData.mOuterRadiusX);						// Foliage Outer Radius X.
      stream->write(mFieldData.mOuterRadiusY);						// Foliage Outer Radius Y.

      stream->write(mFieldData.mMinWidth);							// Foliage Minimum Width.
      stream->write(mFieldData.mMaxWidth);							// Foliage Maximum Width.
      stream->write(mFieldData.mMinHeight);							// Foliage Minimum Height.
      stream->write(mFieldData.mMaxHeight);							// Foliage Maximum Height.
      stream->write(mFieldData.mFixAspectRatio);						// Foliage Fix Aspect Ratio.
      stream->write(mFieldData.mFixSizeToMax);						// Foliage Fix Size to Max.
      stream->write(mFieldData.mOffsetZ);								// Foliage Offset Z.
      stream->writeFlag(mFieldData.mRandomFlip);					// Foliage Random Flip.
      stream->writeFlag(mFieldData.mUseTrueBillboards);        // Foliage faces the camera (including z axis)

      stream->write(mFieldData.mUseCulling);							// Foliage Use Culling.
      stream->write(mFieldData.mCullResolution);						// Foliage Cull Resolution.
      stream->write(mFieldData.mViewDistance);						// Foliage View Distance.
      stream->write(mFieldData.mViewClosest);							// Foliage View Closest.
      stream->write(mFieldData.mFadeInRegion);						// Foliage Fade-In Region.
      stream->write(mFieldData.mFadeOutRegion);						// Foliage Fade-Out Region.
      stream->write(mFieldData.mAlphaCutoff);							// Foliage Alpha Cutoff.
      stream->write(mFieldData.mGroundAlpha);							// Foliage Ground Alpha.

      stream->writeFlag(mFieldData.mSwayOn);							// Foliage Sway On Flag.
      stream->writeFlag(mFieldData.mSwaySync);						// Foliage Sway Sync Flag.
      stream->write(mFieldData.mSwayMagnitudeSide);					// Foliage Sway Magnitude Side2Side.
      stream->write(mFieldData.mSwayMagnitudeFront);					// Foliage Sway Magnitude Front2Back.
      stream->write(mFieldData.mMinSwayTime);							// Foliage Minimum Sway Time.
      stream->write(mFieldData.mMaxSwayTime);							// Foliage Maximum way Time.

      stream->writeFlag(mFieldData.mLightOn);							// Foliage Light On Flag.
      stream->writeFlag(mFieldData.mLightSync);						// Foliage Light Sync
      stream->write(mFieldData.mMinLuminance);						// Foliage Minimum Luminance.
      stream->write(mFieldData.mMaxLuminance);						// Foliage Maximum Luminance.
      stream->write(mFieldData.mLightTime);							// Foliage Light Time.

      stream->writeFlag(mFieldData.mAllowOnTerrain);					// Allow on Terrain.
      stream->writeFlag(mFieldData.mAllowStatics);					// Allow on Statics.
      stream->writeFlag(mFieldData.mAllowOnWater);					// Allow on Water.
      stream->writeFlag(mFieldData.mAllowWaterSurface);				// Allow on Water Surface.
      stream->write(mFieldData.mAllowedTerrainSlope);					// Foliage Offset Z.

      stream->writeFlag(mFieldData.mHideFoliage);						// Hide Foliage.
      stream->writeFlag(mFieldData.mShowPlacementArea);				// Show Placement Area Flag.
      stream->write(mFieldData.mPlacementBandHeight);					// Placement Area Height.
      stream->write(mFieldData.mPlaceAreaColour);						// Placement Area Colour.
   }

   // Were done ...
   return(retMask);
}

//------------------------------------------------------------------------------

void fxFoliageReplicator::unpackUpdate(NetConnection * con, BitStream * stream)
{
   // Unpack Parent.
   Parent::unpackUpdate(con, stream);

   // Read Replication Details.
   if(stream->readFlag())
   {
      MatrixF		ReplicatorObjectMatrix;

      stream->readAffineTransform(&ReplicatorObjectMatrix);			// Foliage Master Object Position.

      mFieldData.mUseDebugInfo = stream->readFlag();					// Foliage Debug Information Flag.
      stream->read(&mFieldData.mDebugBoxHeight);						// Foliage Debug Height.
      stream->read(&mFieldData.mSeed);								// Foliage Seed.
      stream->read(&mFieldData.mFoliageCount);						// Foliage Count.
      stream->read(&mFieldData.mFoliageRetries);						// Foliage Retries.
      mFieldData.mFoliageFile = stream->readSTString();				// Foliage File.

      stream->read(&mFieldData.mInnerRadiusX);						// Foliage Inner Radius X.
      stream->read(&mFieldData.mInnerRadiusY);						// Foliage Inner Radius Y.
      stream->read(&mFieldData.mOuterRadiusX);						// Foliage Outer Radius X.
      stream->read(&mFieldData.mOuterRadiusY);						// Foliage Outer Radius Y.

      stream->read(&mFieldData.mMinWidth);							// Foliage Minimum Width.
      stream->read(&mFieldData.mMaxWidth);							// Foliage Maximum Width.
      stream->read(&mFieldData.mMinHeight);							// Foliage Minimum Height.
      stream->read(&mFieldData.mMaxHeight);							// Foliage Maximum Height.
      stream->read(&mFieldData.mFixAspectRatio);						// Foliage Fix Aspect Ratio.
      stream->read(&mFieldData.mFixSizeToMax);						// Foliage Fix Size to Max.
      stream->read(&mFieldData.mOffsetZ);								// Foliage Offset Z.
      mFieldData.mRandomFlip = stream->readFlag();					// Foliage Random Flip.
      
      bool wasTrueBB = mFieldData.mUseTrueBillboards;
      mFieldData.mUseTrueBillboards = stream->readFlag();      // Foliage is camera facing (including z axis).

      stream->read(&mFieldData.mUseCulling);							// Foliage Use Culling.
      stream->read(&mFieldData.mCullResolution);						// Foliage Cull Resolution.
      stream->read(&mFieldData.mViewDistance);						// Foliage View Distance.
      stream->read(&mFieldData.mViewClosest);							// Foliage View Closest.
      stream->read(&mFieldData.mFadeInRegion);						// Foliage Fade-In Region.
      stream->read(&mFieldData.mFadeOutRegion);						// Foliage Fade-Out Region.
      stream->read(&mFieldData.mAlphaCutoff);							// Foliage Alpha Cutoff.
      stream->read(&mFieldData.mGroundAlpha);							// Foliage Ground Alpha.

      mFieldData.mSwayOn = stream->readFlag();						// Foliage Sway On Flag.
      mFieldData.mSwaySync = stream->readFlag();						// Foliage Sway Sync Flag.
      stream->read(&mFieldData.mSwayMagnitudeSide);					// Foliage Sway Magnitude Side2Side.
      stream->read(&mFieldData.mSwayMagnitudeFront);					// Foliage Sway Magnitude Front2Back.
      stream->read(&mFieldData.mMinSwayTime);							// Foliage Minimum Sway Time.
      stream->read(&mFieldData.mMaxSwayTime);							// Foliage Maximum way Time.

      mFieldData.mLightOn = stream->readFlag();						// Foliage Light On Flag.
      mFieldData.mLightSync = stream->readFlag();						// Foliage Light Sync
      stream->read(&mFieldData.mMinLuminance);						// Foliage Minimum Luminance.
      stream->read(&mFieldData.mMaxLuminance);						// Foliage Maximum Luminance.
      stream->read(&mFieldData.mLightTime);							// Foliage Light Time.

      mFieldData.mAllowOnTerrain = stream->readFlag();				// Allow on Terrain.
      mFieldData.mAllowStatics = stream->readFlag();					// Allow on Statics.
      mFieldData.mAllowOnWater = stream->readFlag();					// Allow on Water.
      mFieldData.mAllowWaterSurface = stream->readFlag();				// Allow on Water Surface.
      stream->read(&mFieldData.mAllowedTerrainSlope);					// Allowed Terrain Slope.

      mFieldData.mHideFoliage = stream->readFlag();					// Hide Foliage.
      mFieldData.mShowPlacementArea = stream->readFlag();				// Show Placement Area Flag.
      stream->read(&mFieldData.mPlacementBandHeight);					// Placement Area Height.
      stream->read(&mFieldData.mPlaceAreaColour);

      // Calculate Fade-In/Out Gradients.
      mFadeInGradient		= 1.0f / mFieldData.mFadeInRegion;
      mFadeOutGradient	= 1.0f / mFieldData.mFadeOutRegion;

      // Set Transform.
      setTransform(ReplicatorObjectMatrix);

      // Load Foliage Texture on the client.
      if( mFieldData.mFoliageFile != NULL && dStrlen(mFieldData.mFoliageFile) > 0 )
         mFieldData.mFoliageTexture = GFXTexHandle( mFieldData.mFoliageFile, &GFXDefaultStaticDiffuseProfile, avar("%s() - mFieldData.mFoliageTexture (line %d)", __FUNCTION__, __LINE__) );

      if ((GFXTextureObject*) mFieldData.mFoliageTexture == NULL)
         Con::printf("fxFoliageReplicator:  %s is an invalid or missing foliage texture file.", mFieldData.mFoliageFile);

      // Set Quad-Tree Box Height Lerp.
      mFrustumRenderSet.mHeightLerp = mFieldData.mDebugBoxHeight;

      // Create Foliage (if Replication has begun).
      if (mClientReplicationStarted)
      {
         CreateFoliage();
         mDirty = true;
      }

      if ( isProperlyAdded() && mFieldData.mUseTrueBillboards != wasTrueBB )
         SetupShader();
   }
}
