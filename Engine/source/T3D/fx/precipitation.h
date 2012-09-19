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

#ifndef _PRECIPITATION_H_
#define _PRECIPITATION_H_

#include "gfx/gfxDevice.h"
#include "T3D/gameBase/gameBase.h"

#ifndef _GFXPRIMITIVEBUFFER_H_
#include "gfx/gfxPrimitiveBuffer.h"
#endif
#ifndef _RENDERPASSMANAGER_H_
#include "renderInstance/renderPassManager.h"
#endif

class SFXTrack;
class SFXSource;

//--------------------------------------------------------------------------
/// Precipitation datablock.
class PrecipitationData : public GameBaseData 
{
   typedef GameBaseData Parent;

  public:
   SFXTrack*     soundProfile;

   StringTableEntry mDropName;         ///< Texture filename for drop particles
   StringTableEntry mDropShaderName;   ///< The name of the shader used for raindrops
   StringTableEntry mSplashName;       ///< Texture filename for splash particles
   StringTableEntry mSplashShaderName; ///< The name of the shader used for raindrops

   S32  mDropsPerSide;     ///< How many drops are on a side of the raindrop texture.
   S32  mSplashesPerSide;  ///< How many splash are on a side of the splash texture.
   
   PrecipitationData();
   DECLARE_CONOBJECT(PrecipitationData);
   bool preload( bool server, String& errorStr );
   static void  initPersistFields();
   virtual void packData(BitStream* stream);
   virtual void unpackData(BitStream* stream);
};

struct Raindrop
{
   F32 velocity;           ///< How fast the drop is falling downwards
   Point3F position;       ///< Position of the drop
   Point3F renderPosition; ///< Interpolated render-position of the drop
   F32 time;               ///< Time into the turbulence function
   F32 mass;               ///< Mass of drop used for how much turbulence/wind effects the drop

   U32 texCoordIndex;      ///< Which piece of the material will be used

   bool toRender;          ///< Don't want to render all drops, just the ones that pass a few tests
   bool valid;             ///< Drop becomes invalid after hitting something.  Just keep updating
                           ///<   the position of it, but don't render until it hits the bottom
                           ///<   of the renderbox and respawns

   Point3F  hitPos;        ///< Point at which the drop will collide with something
   U32      hitType;       ///< What kind of object the drop will hit

   Raindrop *nextSplashDrop;  ///< Linked list cruft for easily adding/removing stuff from the splash list
   Raindrop *prevSplashDrop;  ///< Same as next but previous!

   SimTime animStartTime;     ///< Animation time tracker

   Raindrop* next;         ///< linked list cruft

   Raindrop()
   {
      velocity = 0;
      time = 0;
      mass = 1;
      texCoordIndex = 0;
      next = NULL;
      toRender = false;
      valid = true;
      nextSplashDrop = NULL;
      prevSplashDrop = NULL;
      animStartTime = 0;
      hitType = 0;
      hitPos = Point3F(0,0,0);
   }
};

//--------------------------------------------------------------------------
class Precipitation : public GameBase
{
  protected:

   typedef GameBase Parent;
   PrecipitationData*   mDataBlock;

   Raindrop *mDropHead;    ///< Drop linked list head
   Raindrop *mSplashHead;  ///< Splash linked list head

   Point2F* mTexCoords;     ///< texture coords for rain texture
   Point2F* mSplashCoords;  ///< texture coordinates for splash texture

   SFXSource* mAmbientSound;        ///< Ambient sound

   GFXShaderRef mDropShader;     ///< The shader used for raindrops
   GFXTexHandle mDropHandle;     ///< Texture handle for raindrop
   GFXShaderRef mSplashShader;   ///< The shader used for splashes
   GFXTexHandle mSplashHandle;   ///< Texture handle for splash

   U32 mLastRenderFrame;         ///< Used to skip processTick when we haven't been visible.

   U32 mDropHitMask;             ///< Stores the current drop hit mask.

   //console exposed variables
   bool mFollowCam;                 ///< Does the system follow the camera or stay where it's placed.

   F32  mDropSize;                  ///< Droplet billboard size
   F32  mSplashSize;                ///< Splash billboard size
   bool mUseTrueBillboards;         ///< True to use true billboards, false for axis-aligned billboards
   S32  mSplashMS;                  ///< How long in milliseconds a splash will last
   bool mAnimateSplashes;           ///< Animate the splashes using the frames in the texture.

   S32 mDropAnimateMS;           ///< If greater than zero, will animate the drops from
                                 ///< the frames in the texture

   S32 mNumDrops;                ///< Number of drops in the scene
   F32 mPercentage;              ///< Server-side set var (NOT exposed to console)
                                 ///< which controls how many drops are present [0,1]

   F32 mMinSpeed;                ///< Minimum downward speed of drops
   F32 mMaxSpeed;                ///< Maximum downward speed of drops

   F32 mMinMass;                 ///< Minimum mass of drops
   F32 mMaxMass;                 ///< Maximum mass of drops

   F32 mBoxWidth;                ///< How far away in the x and y directions drops will render
   F32 mBoxHeight;               ///< How high drops will render

   F32 mMaxTurbulence;           ///< Coefficient to sin/cos for adding turbulence
   F32 mTurbulenceSpeed;         ///< How fast the turbulence wraps in a circle
   bool mUseTurbulence;          ///< Whether to use turbulence or not (MAY EFFECT PERFORMANCE)

   bool mUseLighting;            ///< This enables shading of the drops and splashes
                                 ///< by the sun color.

   ColorF mGlowIntensity;        ///< Set it to 0 to disable the glow or use it to control 
                                 ///< the intensity of each channel.

   bool mReflect;                ///< This enables the precipitation to be rendered
                                 ///< during reflection passes.  This is expensive.

   bool mUseWind;                ///< This enables the wind from the sky SceneObject
                                 ///< to effect the velocitiy of the drops.

   bool mRotateWithCamVel;       ///< Rotate the drops relative to the camera velocity
                                 ///< This is useful for "streak" type drops

   bool mDoCollision;            ///< Whether or not to do collision
   bool mDropHitPlayers;         ///< Should drops collide with players
   bool mDropHitVehicles;        ///< Should drops collide with vehicles

   F32 mFadeDistance;            ///< The distance at which fading of the particles begins.
   F32 mFadeDistanceEnd;         ///< The distance at which fading of the particles ends.

   U32 mMaxVBDrops;              ///< The maximum drops allowed in one render batch.

   GFXStateBlockRef mDefaultSB;
   GFXStateBlockRef mDistantSB;

   GFXShaderConstBufferRef mDropShaderConsts;
   
   GFXShaderConstHandle* mDropShaderModelViewSC;
   GFXShaderConstHandle* mDropShaderFadeStartEndSC;
   GFXShaderConstHandle* mDropShaderCameraPosSC;
   GFXShaderConstHandle* mDropShaderAmbientSC;

   GFXShaderConstBufferRef mSplashShaderConsts;
   
   GFXShaderConstHandle* mSplashShaderModelViewSC;
   GFXShaderConstHandle* mSplashShaderFadeStartEndSC;
   GFXShaderConstHandle* mSplashShaderCameraPosSC;
   GFXShaderConstHandle* mSplashShaderAmbientSC;

   struct
   {
      bool valid;
      U32 startTime;
      U32 totalTime;
      F32 startPct;
      F32 endPct;

   } mStormData;
   
   struct
   {
      bool valid;
      U32 startTime;
      U32 totalTime;
      F32 startMax;
      F32 startSpeed;
      F32 endMax;
      F32 endSpeed;

   } mTurbulenceData;

   //other functions...
   void processTick(const Move*);
   void interpolateTick(F32 delta);

   VectorF getWindVelocity();
   void fillDropList();                       ///< Adds/removes drops from the list to have the right # of drops
   void killDropList();                       ///< Deletes the entire drop list
   void initRenderObjects();                  ///< Re-inits the texture coord lookup tables
   void initMaterials();                      ///< Re-inits the textures and shaders
   void spawnDrop(Raindrop *drop);            ///< Fills drop info with random velocity, x/y positions, and mass
   void spawnNewDrop(Raindrop *drop);         ///< Same as spawnDrop except also does z position
   
   void findDropCutoff(Raindrop *drop, const Box3F &box, const VectorF &windVel);   ///< Casts a ray to see if/when a drop will collide
   void wrapDrop(Raindrop *drop, const Box3F &box, const U32 currTime, const VectorF &windVel);         ///< Wraps a drop within the specified box
   
   void createSplash(Raindrop *drop);        ///< Adds a drop to the splash list
   void destroySplash(Raindrop *drop);       ///< Removes a drop from the splash list

   GFXPrimitiveBufferHandle mRainIB;
   GFXVertexBufferHandle<GFXVertexPT> mRainVB;

   bool onAdd();
   void onRemove();

   // Rendering
   void prepRenderImage( SceneRenderState* state );
   void renderObject(ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance* );

   void setTransform(const MatrixF &mat);

  public:

   Precipitation();
   ~Precipitation();
   void inspectPostApply();

   enum
   {
      DataMask       = Parent::NextFreeMask << 0,
      PercentageMask = Parent::NextFreeMask << 1,
      StormMask      = Parent::NextFreeMask << 2,
      TransformMask  = Parent::NextFreeMask << 3,
      TurbulenceMask = Parent::NextFreeMask << 4,
      NextFreeMask   = Parent::NextFreeMask << 5
   };

   bool onNewDataBlock( GameBaseData *dptr, bool reload );
   DECLARE_CONOBJECT(Precipitation);
   static void initPersistFields();
   
   U32  packUpdate(NetConnection*, U32 mask, BitStream* stream);
   void unpackUpdate(NetConnection*, BitStream* stream);

   void setPercentage(F32 pct);
   void modifyStorm(F32 pct, U32 ms);

   /// This is used to smoothly change the turbulence
   /// over a desired time period.  Setting ms to zero
   /// will cause the change to be instantaneous.  Setting
   /// max zero will disable turbulence.
   void setTurbulence(F32 max, F32 speed, U32 ms);
};

#endif // PRECIPITATION_H_

