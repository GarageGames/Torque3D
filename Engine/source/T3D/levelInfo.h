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

#ifndef _LEVELINFO_H_
#define _LEVELINFO_H_

#ifndef _NETOBJECT_H_
   #include "sim/netObject.h"
#endif
#ifndef _COLOR_H_
   #include "core/color.h"
#endif
#ifndef _FOGSTRUCTS_H_
   #include "scene/fogStructs.h"
#endif
#ifndef _SFXCOMMON_H_
   #include "sfx/sfxCommon.h"
#endif

#ifndef _GFXTEXTUREHANDLE_H_
#include "gfx/gfxTextureHandle.h"
#endif

class SFXAmbience;
class SFXSoundscape;


class LevelInfo : public NetObject
{
      typedef NetObject Parent;
      
   private:

      F32 mWorldSize;

      FogData mFogData;
      
      F32 mNearClip;

      F32 mVisibleDistance;

      F32 mVisibleGhostDistance;

      F32 mDecalBias;

      ColorI mCanvasClearColor;

      /// @name Lighting Properties
      /// @{

      bool mAdvancedLightmapSupport;

      /// Seconds it takes to go from one global ambient color
      /// to a different one.
      F32 mAmbientLightBlendPhase;

      /// Interpolation for going from one global ambient color
      /// to a different one.
      EaseF mAmbientLightBlendCurve;

      /// @}
      
      /// @name Sound Properties
      /// @{
      
      /// Global ambient sound space properties.
      SFXAmbience* mSoundAmbience;
      
      /// Distance attenuation model to use.
      SFXDistanceModel mSoundDistanceModel;
      
      ///
      SFXSoundscape* mSoundscape;
         
      /// @}
      
      /// Responsible for passing on
      /// the LevelInfo settings to the
      /// client SceneGraph, from which
      /// other systems can get at them.
      void _updateSceneGraph();

      void _onLMActivate(const char *lm, bool enable);
   protected:
      // Name (path) of the accumulation texture.
      String mAccuTextureName;

   public:

      LevelInfo();
      virtual ~LevelInfo();

      DECLARE_CONOBJECT(LevelInfo);

      /// @name SceneObject Inheritance
      /// @{
      
      virtual SFXAmbience* getSoundAmbience() const { return mSoundAmbience; }

      /// @}
      
      /// @name SimObject Inheritance
      /// @{

      virtual bool onAdd();
      virtual void onRemove();
      virtual void inspectPostApply();

      static void initPersistFields();

      /// @}

      /// @name NetObject Inheritance
      /// @{
      
      enum NetMaskBits 
      {
         UpdateMask = BIT(0)
      };

      GFXTexHandle mAccuTexture;

      virtual U32 packUpdate( NetConnection *conn, U32 mask, BitStream *stream );
      virtual void unpackUpdate( NetConnection *conn, BitStream *stream );
      static bool _setLevelAccuTexture(void *object, const char *index, const char *data);
      void setLevelAccuTexture(const String& name);
      /// @}
};

#endif // _LEVELINFO_H_