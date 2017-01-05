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

#ifndef _POST_EFFECT_H_
#define _POST_EFFECT_H_

#ifndef _SIMSET_H_
#include "console/simSet.h"
#endif
#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif
#ifndef _MPOINT2_H_
#include "math/mPoint2.h"
#endif
#ifndef _GFXSHADER_H_
#include "gfx/gfxShader.h"
#endif
#ifndef _GFXSTATEBLOCK_H_
#include "gfx/gfxStateBlock.h"
#endif
#ifndef _GFXTEXTUREHANDLE_H_
#include "gfx/gfxTextureHandle.h"
#endif
#ifndef _GFXTARGET_H_
#include "gfx/gfxTarget.h"
#endif
#ifndef _GFXVERTEXBUFFER_H_
#include "gfx/gfxVertexBuffer.h"
#endif
#ifndef _POSTEFFECTCOMMON_H_
#include "postFx/postEffectCommon.h"
#endif
#ifndef _TDICTIONARY_H_
#include "core/util/tDictionary.h"
#endif
#ifndef _MATTEXTURETARGET_H_
#include "materials/matTextureTarget.h"
#endif

class GFXStateBlockData;
class Frustum;
class SceneRenderState;
class ConditionerFeature;


///
GFX_DeclareTextureProfile( PostFxTargetProfile );




///
class PostEffect : public SimGroup
{
   typedef SimGroup Parent;

   friend class PostEffectVis;

public:

   enum
   {
      NumTextures = 8,
   };

protected:

   FileName mTexFilename[NumTextures];

   GFXTexHandle mTextures[NumTextures];

   NamedTexTarget mNamedTarget;
   NamedTexTarget mNamedTargetDepthStencil; 

   GFXTextureObject *mActiveTextures[NumTextures];

   NamedTexTarget *mActiveNamedTarget[NumTextures];

   RectI mActiveTextureViewport[NumTextures];

   GFXStateBlockData *mStateBlockData;

   GFXStateBlockRef mStateBlock;

   String mShaderName;

   GFXShaderRef mShader;

   Vector<GFXShaderMacro> mShaderMacros;
   
   GFXShaderConstBufferRef mShaderConsts;

   GFXShaderConstHandle *mRTSizeSC;
   GFXShaderConstHandle *mOneOverRTSizeSC;

   GFXShaderConstHandle *mTexSizeSC[NumTextures];
   GFXShaderConstHandle *mRenderTargetParamsSC[NumTextures];

   GFXShaderConstHandle *mViewportOffsetSC;

   GFXShaderConstHandle *mTargetViewportSC;

   GFXShaderConstHandle *mFogDataSC;
   GFXShaderConstHandle *mFogColorSC;
   GFXShaderConstHandle *mEyePosSC;
   GFXShaderConstHandle *mMatWorldToScreenSC;
   GFXShaderConstHandle *mMatScreenToWorldSC;
   GFXShaderConstHandle *mMatPrevScreenToWorldSC;
   GFXShaderConstHandle *mNearFarSC;
   GFXShaderConstHandle *mInvNearFarSC;   
   GFXShaderConstHandle *mWorldToScreenScaleSC;
   GFXShaderConstHandle *mProjectionOffsetSC;
   GFXShaderConstHandle *mWaterColorSC;
   GFXShaderConstHandle *mWaterFogDataSC;     
   GFXShaderConstHandle *mAmbientColorSC;
   GFXShaderConstHandle *mWaterFogPlaneSC;
   GFXShaderConstHandle *mWaterDepthGradMaxSC;
   GFXShaderConstHandle *mScreenSunPosSC;
   GFXShaderConstHandle *mLightDirectionSC;
   GFXShaderConstHandle *mCameraForwardSC;
   GFXShaderConstHandle *mAccumTimeSC;
   GFXShaderConstHandle *mDeltaTimeSC;
   GFXShaderConstHandle *mInvCameraMatSC;

   bool mAllowReflectPass;

   /// If true update the shader.
   bool mUpdateShader;   

   GFXTextureTargetRef mTarget;

   String mTargetName;
   GFXTexHandle mTargetTex;

   String mTargetDepthStencilName;
   GFXTexHandle mTargetDepthStencil;

   /// If mTargetSize is zero then this scale is
   /// used to make a relative texture size to the
   /// active render target.
   Point2F mTargetScale;

   /// If non-zero this is used as the absolute
   /// texture target size.
   /// @see mTargetScale
   Point2I mTargetSize;

   GFXFormat mTargetFormat;

   /// The color to prefill the named target when
   /// first created by the effect.
   ColorF mTargetClearColor;

   PFXRenderTime mRenderTime;
   PFXTargetClear mTargetClear;
   PFXTargetViewport mTargetViewport;

   String mRenderBin;

   F32 mRenderPriority;

   /// This is true if the effect has been succesfully
   /// initialized and all requirements are met for use.
   bool mIsValid;

   /// True if the effect has been enabled by the manager.
   bool mEnabled;
   
   /// Skip processing of this PostEffect and its children even if its parent is enabled. 
   /// Parent and sibling PostEffects in the chain are still processed.
   /// This is intended for debugging purposes.
   bool mSkip;

   bool mOneFrameOnly;
   bool mOnThisFrame;  

   U32 mShaderReloadKey;

   class EffectConst
   {
   public:

      EffectConst( const String &name, const String &val )
         : mName( name ), 
           mHandle( NULL ),
           mDirty( true )
      {
         set( val );
      }

      void set( const String &newVal );

      void setToBuffer( GFXShaderConstBufferRef buff );

      String mName;

      GFXShaderConstHandle *mHandle;

      String mStringVal;

      bool mDirty;
   };

   typedef HashTable<StringCase,EffectConst*> EffectConstTable;

   EffectConstTable mEffectConsts;

   ///
   virtual void _updateScreenGeometry( const Frustum &frustum,
                                       GFXVertexBufferHandle<PFXVertex> *outVB );

   ///
   virtual void _setupStateBlock( const SceneRenderState *state );

   /// 
   virtual void _setupConstants( const SceneRenderState *state );

   ///
   virtual void _setupTransforms();

   ///
   virtual void _setupTarget( const SceneRenderState *state, bool *outClearTarget );

   ///
   virtual void _setupTexture( U32 slot, GFXTexHandle &inputTex, const RectI *inTexViewport );

   /// Protected set method for toggling the enabled state.
   static bool _setIsEnabled( void *object, const char *index, const char *data );

   /// Called from the light manager activate signal.
   /// @see LightManager::addActivateCallback
   void _onLMActivate( const char*, bool activate )
   {
      if ( activate ) 
         mUpdateShader = true; 
   }

   /// We handle texture events to release named rendered targets.
   /// @see GFXTextureManager::addEventDelegate
   void _onTextureEvent( GFXTexCallbackCode code )
   {
      if ( code == GFXZombify && (mNamedTarget.isRegistered() || mNamedTargetDepthStencil.isRegistered()) )
         _cleanTargets();
   }

   ///
   void _updateConditioners();

   ///
   void _cleanTargets( bool recurse = false );

   /// 
   void _checkRequirements();

   ///
   GFXTextureObject* _getTargetTexture( U32 index );

public:

   /// Constructor.
   PostEffect();

   /// Destructor.
   virtual ~PostEffect();

   DECLARE_CONOBJECT(PostEffect);

   // SimObject
   virtual bool onAdd();
   virtual void onRemove();
   static void initPersistFields();

   /// @name Callbacks
   /// @{

   DECLARE_CALLBACK( void, onAdd, () );
   DECLARE_CALLBACK( void, preProcess, () );
   DECLARE_CALLBACK( void, setShaderConsts, () );
   DECLARE_CALLBACK( bool, onEnabled, () );
   DECLARE_CALLBACK( void, onDisabled, () );

   /// @}

   virtual void process(   const SceneRenderState *state, 
                           GFXTexHandle &inOutTex,
                           const RectI *inTexViewport = NULL );

   /// 
   void reload();

   /// 
   void enable();

   /// 
   void disable();

   /// Dump the shader disassembly to a temporary text file.
   /// Returns true and sets outFilename to the file if successful.
   bool dumpShaderDisassembly( String &outFilename ) const;

   /// Returns the SimSet which contains all PostEffects.
   SimSet* getSet() const;

   ///
   bool isEnabled() const { return mEnabled; }

   /// Is set to skip rendering.
   bool isSkipped() const { return mSkip; }

   /// Set the effect to skip rendering.
   void setSkip( bool skip ) { mSkip = skip; }

   PFXRenderTime getRenderTime() const { return mRenderTime; }

   const String& getRenderBin() const { return mRenderBin; }

   F32 getPriority() const { return mRenderPriority; }

   void setTexture( U32 index, const String &filePath );

   void setShaderMacro( const String &name, const String &value = String::EmptyString );
   bool removeShaderMacro( const String &name );
   void clearShaderMacros();

   ///
   void setShaderConst( const String &name, const String &val );   

   void setOnThisFrame( bool enabled ) { mOnThisFrame = enabled; }
   bool isOnThisFrame() { return mOnThisFrame; }
   void setOneFrameOnly( bool enabled ) { mOneFrameOnly = enabled; }
   bool isOneFrameOnly() { return mOneFrameOnly; }   

   F32 getAspectRatio() const;
   

   enum PostEffectRequirements
   {
      RequiresDepth      = BIT(0),
      RequiresNormals    = BIT(1),
      RequiresLightInfo  = BIT(2),
   };
};

#endif // _POST_EFFECT_H_