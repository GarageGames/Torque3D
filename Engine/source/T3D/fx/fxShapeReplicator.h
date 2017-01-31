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

#ifndef _SHAPEREPLICATOR_H_
#define _SHAPEREPLICATOR_H_

#ifndef _TSSTATIC_H_
#include "T3D/tsStatic.h"
#endif
#ifndef _TSSHAPEINSTANCE_H_
#include "ts/tsShapeInstance.h"
#endif
#ifndef _RENDERPASSMANAGER_H_
#include "renderInstance/renderPassManager.h"
#endif

#define AREA_ANIMATION_ARC         (1.0f / 360.0f)

#define FXREPLICATOR_COLLISION_MASK   (   TerrainObjectType      |   \
                              StaticShapeObjectType      |   \
                              WaterObjectType      )

#define FXREPLICATOR_NOWATER_COLLISION_MASK   (   TerrainObjectType      |   \
                                    StaticShapeObjectType      )


//------------------------------------------------------------------------------
// Class: fxShapeReplicatedStatic
//------------------------------------------------------------------------------
class fxShapeReplicatedStatic : public TSStatic
{
private:
   typedef SceneObject         Parent;

public:
   fxShapeReplicatedStatic() {};
   ~fxShapeReplicatedStatic() {};
   void touchNetFlags(const U32 m, bool setflag = true) { if (setflag) mNetFlags.set(m); else mNetFlags.clear(m); };
   TSShape* getShape(void) { return mShapeInstance->getShape(); };
   void setTransform(const MatrixF & mat) { Parent::setTransform(mat); setRenderTransform(mat); };

   DECLARE_CONOBJECT(fxShapeReplicatedStatic);
};


//------------------------------------------------------------------------------
// Class: fxShapeReplicator
//------------------------------------------------------------------------------
class fxShapeReplicator : public SceneObject
{
private:
   typedef SceneObject      Parent;

protected:

   void CreateShapes(void);
   void DestroyShapes(void);
   void RenewShapes(void);

   enum {   ReplicationMask   = (1 << 0) };

   U32                              mCreationAreaAngle;
   U32                              mCurrentShapeCount;
   Vector<fxShapeReplicatedStatic*> mReplicatedShapes;
   MRandomLCG                       RandomGen;
   S32                              mLastRenderTime;


public:
   fxShapeReplicator();
   ~fxShapeReplicator();


   void StartUp(void);
   void ShowReplication(void);
   void HideReplication(void);

   GFXStateBlockRef mPlacementSB;

   void renderObject(ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance*);
   void renderArc(const F32 fRadiusX, const F32 fRadiusY);
   void renderPlacementArea(const F32 ElapsedTime);

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

      U32                mSeed;
      StringTableEntry   mShapeFile;
      U32                mShapeCount;
      U32                mShapeRetries;
      Point3F            mShapeScaleMin;
      Point3F            mShapeScaleMax;
      Point3F            mShapeRotateMin;
      Point3F            mShapeRotateMax;
      U32                mInnerRadiusX;
      U32                mInnerRadiusY;
      U32                mOuterRadiusX;
      U32                mOuterRadiusY;
      S32                mOffsetZ;
      bool              mAllowOnTerrain;
      bool              mAllowStatics;
      bool              mAllowOnWater;
      S32               mAllowedTerrainSlope;
      bool              mAlignToTerrain;
      bool              mAllowWaterSurface;
      Point3F           mTerrainAlignment;
      bool              mInteractions;
      bool              mHideReplications;
      bool              mShowPlacementArea;
      U32               mPlacementBandHeight;
      ColorF            mPlaceAreaColour;

      tagFieldData()
      {
         // Set Defaults.
         mSeed               = 1376312589;
         mShapeFile          = StringTable->EmptyString();
         mShapeCount         = 10;
         mShapeRetries       = 100;
         mInnerRadiusX       = 0;
         mInnerRadiusY       = 0;
         mOuterRadiusX       = 100;
         mOuterRadiusY       = 100;
         mOffsetZ            = 0;

         mAllowOnTerrain     = true;
         mAllowStatics       = true;
         mAllowOnWater       = false;
         mAllowWaterSurface  = false;
         mAllowedTerrainSlope= 90;
         mAlignToTerrain     = false;
         mInteractions       = true;

         mHideReplications   = false;

         mShowPlacementArea    = true;
         mPlacementBandHeight  = 25;
         mPlaceAreaColour      .set(0.4f, 0, 0.8f);

         mShapeScaleMin         .set(1, 1, 1);
         mShapeScaleMax         .set(1, 1, 1);
         mShapeRotateMin        .set(0, 0, 0);
         mShapeRotateMax        .set(0, 0, 0);
         mTerrainAlignment      .set(1, 1, 1);
      }

   } mFieldData;

   // Declare Console Object.
   DECLARE_CONOBJECT(fxShapeReplicator);
};

#endif // _SHAPEREPLICATOR_H_
