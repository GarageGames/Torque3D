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

#ifndef _SUN_H_
#define _SUN_H_

#ifndef _SCENEOBJECT_H_
#include "scene/sceneObject.h"
#endif
#ifndef _COLOR_H_
#include "core/color.h"
#endif
#ifndef _LIGHTINFO_H_
#include "lighting/lightInfo.h"
#endif
#ifndef _LIGHTFLAREDATA_H_
#include "T3D/lightFlareData.h"
#endif

class TimeOfDay;
class MatrixSet;

///
class Sun : public SceneObject, public ISceneLight
{
   typedef SceneObject Parent;

protected:

   F32 mSunAzimuth;
   
   F32 mSunElevation;

   ColorF mLightColor;

   ColorF mLightAmbient;

   F32 mBrightness;

   bool mAnimateSun;
   F32  mTotalTime;
   F32  mCurrTime;
   F32  mStartAzimuth;
   F32  mEndAzimuth;
   F32  mStartElevation;
   F32  mEndElevation;

   bool mCastShadows;

   LightInfo *mLight;

   LightFlareData *mFlareData;
   LightFlareState mFlareState;
   F32 mFlareScale;

   bool mCoronaEnabled;
   String mCoronaMatName;
   BaseMatInstance *mCoronaMatInst;
   MatrixSet *mMatrixSet;   
   F32 mCoronaScale;
   ColorF mCoronaTint;
   bool mCoronaUseLightColor;

   // These are not user specified.
   // These hold data calculated once used across several methods.
   F32 mCoronaWorldRadius;
   Point3F mLightWorldPos;

   void _conformLights();
   void _initCorona();
   void _renderCorona( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat );
   void _updateTimeOfDay( TimeOfDay *timeOfDay, F32 time );

   // SimObject.
   virtual void _onSelected();
   virtual void _onUnselected();

   enum NetMaskBits 
   {
      UpdateMask = BIT(0)
   };

public:

   Sun();
   virtual ~Sun();

   // SimObject
   virtual bool onAdd();
   virtual void onRemove();

   // ConsoleObject
   DECLARE_CONOBJECT(Sun);   
   static void initPersistFields();
   void inspectPostApply();

   // NetObject
   U32 packUpdate( NetConnection *conn, U32 mask, BitStream *stream );
   void unpackUpdate( NetConnection *conn, BitStream *stream ); 

   // ISceneLight
   virtual void submitLights( LightManager *lm, bool staticLighting );
   virtual LightInfo* getLight() { return mLight; }   

   // SceneObject   
   virtual void prepRenderImage( SceneRenderState* state );

   // ProcessObject
   virtual void advanceTime( F32 dt );

   ///
   void setAzimuth( F32 azimuth );

   ///
   void setElevation( F32 elevation );

   ///
   void setColor( const ColorF &color );

   ///
   void animate( F32 duration, F32 startAzimuth, F32 endAzimuth, F32 startElevation, F32 endElevation );
};

#endif // _SUN_H_
