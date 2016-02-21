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

#ifndef _LIGHTDESCRIPTION_H_
#define _LIGHTDESCRIPTION_H_

#ifndef _SIMDATABLOCK_H_
#include "console/simDatablock.h"
#endif
#ifndef _CONSOLETYPES_H_
#include "console/consoleTypes.h"
#endif
#ifndef _COLOR_H_
#include "core/color.h"
#endif
#ifndef _LIGHTANIMDATA_H_
#include "T3D/lightAnimData.h"
#endif
#ifndef _LIGHTFLAREDATA_H_
#include "T3D/lightFlareData.h"
#endif

struct LightState
{
   LightInfo *lightInfo;
   F32 fullBrightness;   

   LightAnimState animState;   
   LightFlareState flareState;      

   void clear() 
   {
      lightInfo = NULL;
      fullBrightness = 1.0f;
      flareState.clear();     
   }

   void setLightInfo( LightInfo *li )
   {
      flareState.lightInfo = lightInfo = li;
   }
};

/// LightDescription is a helper datablock used by classes (such as shapebase)
/// that submit lights to the scene but do not use actual "LightBase" objects.
/// This datablock stores the properties of that light as fields that can be
/// initialized from script.

class LightAnimData;
class LightFlareData;
class LightManager;
class ISceneLight;

class LightDescription : public SimDataBlock
{
   typedef SimDataBlock Parent;

public:

   LightDescription();
   virtual ~LightDescription();

   DECLARE_CONOBJECT( LightDescription );

   static void initPersistFields();
   virtual void inspectPostApply();
   
   bool onAdd();

   // SimDataBlock
   virtual bool preload( bool server, String &errorStr );
   virtual void packData( BitStream *stream );
   virtual void unpackData( BitStream *stream );
   
   //void animateLight( LightState *state );
   void submitLight( LightState *state, const MatrixF &xfm, LightManager *lm, SimObject *object );
   void prepRender( SceneRenderState *sceneState, LightState *lightState, const MatrixF &xfm );

   bool _preload( bool server, String &errorStr );
   
   ColorF color;
   F32 brightness;
   F32 range;
   bool castShadows;
   S32 mStaticRefreshFreq;
   S32 mDynamicRefreshFreq;

   LightAnimData *animationData;   
   S32 animationDataId;
   F32 animationPeriod;
   F32 animationPhase;

   LightFlareData *flareData;
   S32 flareDataId;
   F32 flareScale;
};

#endif // _LIGHTDESCRIPTION_H_