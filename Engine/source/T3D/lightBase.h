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

#ifndef _LIGHTBASE_H_
#define _LIGHTBASE_H_

#ifndef _SCENEOBJECT_H_
#include "scene/sceneObject.h"
#endif
#ifndef _LIGHTINFO_H_
#include "lighting/lightInfo.h"
#endif
#ifndef _ITICKABLE_H_
#include "core/iTickable.h"
#endif
#ifndef _LIGHTFLAREDATA_H_
#include "T3D/lightFlareData.h"
#endif
#ifndef _LIGHTANIMDATA_H_
#include "T3D/lightAnimData.h"
#endif

class LightAnimData;

class LightBase : public SceneObject, public ISceneLight, public virtual ITickable
{
   typedef SceneObject Parent;
   friend class LightAnimData;
   friend class LightFlareData;

protected:

   bool mIsEnabled;

   ColorF mColor;

   F32 mBrightness;

   bool mCastShadows;
   S32 mStaticRefreshFreq;
   S32 mDynamicRefreshFreq;
   F32 mPriority;

   LightInfo *mLight;

   LightAnimData *mAnimationData; 
   LightAnimState mAnimState;

   LightFlareData *mFlareData;
   LightFlareState mFlareState;   
   F32 mFlareScale;

   static bool smRenderViz;

   virtual void _conformLights() {}

   void _onRenderViz(   ObjectRenderInst *ri, 
                        SceneRenderState *state, 
                        BaseMatInstance *overrideMat );

   virtual void _renderViz( SceneRenderState *state ) {}

   enum LightMasks
   {
      InitialUpdateMask = Parent::NextFreeMask,
      EnabledMask       = Parent::NextFreeMask << 1,
      TransformMask     = Parent::NextFreeMask << 2,
      UpdateMask        = Parent::NextFreeMask << 3,
      DatablockMask     = Parent::NextFreeMask << 4,      
      NextFreeMask      = Parent::NextFreeMask << 5
   };

   // SimObject.
   virtual void _onSelected();
   virtual void _onUnselected();

public:

   LightBase();
   virtual ~LightBase();

   // SimObject
   virtual bool onAdd();
   virtual void onRemove();

   // ConsoleObject
   void inspectPostApply();
   static void initPersistFields();
   DECLARE_CONOBJECT(LightBase);

   // NetObject
   U32 packUpdate( NetConnection *conn, U32 mask, BitStream *stream );
   void unpackUpdate( NetConnection *conn, BitStream *stream );  

   // ISceneLight
   virtual void submitLights( LightManager *lm, bool staticLighting );
   virtual LightInfo* getLight() { return mLight; }

   // SceneObject
   virtual void setTransform( const MatrixF &mat );
   virtual void prepRenderImage( SceneRenderState *state );

   // ITickable
   virtual void interpolateTick( F32 delta );
   virtual void processTick();
   virtual void advanceTime( F32 timeDelta );

   /// Toggles the light on and off.
   void setLightEnabled( bool enabled );
   bool getLightEnabled() { return mIsEnabled; };

   /// Animate the light.
   virtual void pauseAnimation( void );
   virtual void playAnimation( void );
   virtual void playAnimation( LightAnimData *animData );
};

#endif // _LIGHTBASE_H_
