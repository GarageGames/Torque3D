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

#include "platform/platform.h"
#include "postFx/postEffect.h"

#include "console/engineAPI.h"
#include "core/stream/fileStream.h"
#include "core/strings/stringUnit.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "math/util/frustum.h"
#include "math/mathUtils.h"
#include "gfx/gfxTransformSaver.h"
#include "gfx/gfxStringEnumTranslate.h"
#include "gfx/gfxTextureManager.h"
#include "gfx/gfxDebugEvent.h"
#include "gfx/util/screenspace.h"
#include "gfx/sim/gfxStateBlockData.h"
#include "scene/sceneRenderState.h"
#include "shaderGen/shaderGenVars.h"
#include "lighting/lightInfo.h"
#include "lighting/lightManager.h"
#include "materials/materialManager.h"
#include "materials/shaderData.h"
#include "postFx/postEffectManager.h"
#include "postFx/postEffectVis.h"

using namespace Torque;

ConsoleDocClass( PostEffect, 
   "@brief A fullscreen shader effect.\n\n"

   "@section PFXTextureIdentifiers\n\n"   

   "@ingroup Rendering\n"
);

IMPLEMENT_CALLBACK( PostEffect, onAdd, void, (), (),
   "Called when this object is first created and registered."
);

IMPLEMENT_CALLBACK( PostEffect, preProcess, void, (), (),
   "Called when an effect is processed but before textures are bound. This "
   "allows the user to change texture related paramaters or macros at runtime.\n"
   "@tsexample\n"   
   "function SSAOPostFx::preProcess( %this )\n"
   "{\n"   
   "   if ( $SSAOPostFx::quality !$= %this.quality )\n"
   "   {\n"
   "      %this.quality = mClamp( mRound( $SSAOPostFx::quality ), 0, 2 );\n"
   "      \n"         
   "      %this.setShaderMacro( \"QUALITY\", %this.quality );\n"
   "   }\n"     
   "   %this.targetScale = $SSAOPostFx::targetScale;\n"
   "}\n"
   "@endtsexample\n"
   "@see setShaderConst\n"
   "@see setShaderMacro"
);

IMPLEMENT_CALLBACK( PostEffect, setShaderConsts, void, (), (),
   "Called immediate before processing this effect. This is the user's chance "
   "to set the value of shader uniforms (constants).\n"
   "@see setShaderConst"
);

IMPLEMENT_CALLBACK( PostEffect, onEnabled, bool, (), (),
   "Called when this effect becomes enabled. If the user returns false from "
   "this callback the effect will not be enabled.\n"
   "@return True to allow this effect to be enabled."
);

IMPLEMENT_CALLBACK( PostEffect, onDisabled, void, (), (),
   "Called when this effect becomes disabled."
);

ImplementEnumType( PFXRenderTime,
   "When to process this effect during the frame.\n"
   "@ingroup Rendering\n\n")
   { PFXBeforeBin, "PFXBeforeBin", "Before a RenderInstManager bin.\n" },
   { PFXAfterBin, "PFXAfterBin", "After a RenderInstManager bin.\n" },
   { PFXAfterDiffuse, "PFXAfterDiffuse", "After the diffuse rendering pass.\n" },
   { PFXEndOfFrame, "PFXEndOfFrame", "When the end of the frame is reached.\n" },
   { PFXTexGenOnDemand, "PFXTexGenOnDemand", "This PostEffect is not processed by the manager. It will generate its texture when it is requested.\n" }
EndImplementEnumType;

ImplementEnumType( PFXTargetClear,
   "Describes when the target texture should be cleared\n"
   "@ingroup Rendering\n\n")
   { PFXTargetClear_None, "PFXTargetClear_None", "Never clear the PostEffect target.\n" },
   { PFXTargetClear_OnCreate, "PFXTargetClear_OnCreate", "Clear once on create.\n" },
   { PFXTargetClear_OnDraw, "PFXTargetClear_OnDraw", "Clear before every draw.\n" },
EndImplementEnumType;

ImplementEnumType( PFXTargetViewport,
   "Specifies how the viewport should be set up for a PostEffect's target.\n"
   "@note Applies to both the diffuse target and the depth target (if defined).\n"
   "@ingroup Rendering\n\n")
   { PFXTargetViewport_TargetSize, "PFXTargetViewport_TargetSize", "Set viewport to match target size (default).\n" },
   { PFXTargetViewport_GFXViewport, "PFXTargetViewport_GFXViewport", "Use the current GFX viewport (scaled to match target size).\n" },
   { PFXTargetViewport_NamedInTexture0, "PFXTargetViewport_NamedInTexture0", "Use the input texture 0 if it is named (scaled to match target size), otherwise revert to PFXTargetViewport_TargetSize if there is none.\n" },
EndImplementEnumType;


GFXImplementVertexFormat( PFXVertex )
{
   addElement( "POSITION", GFXDeclType_Float3 );
   addElement( "TEXCOORD", GFXDeclType_Float2, 0 );
   addElement( "TEXCOORD", GFXDeclType_Float3, 1 );
};

GFX_ImplementTextureProfile(  PostFxTargetProfile,
                              GFXTextureProfile::DiffuseMap,
                              GFXTextureProfile::PreserveSize |
                              GFXTextureProfile::RenderTarget |
                              GFXTextureProfile::Pooled,
                              GFXTextureProfile::NONE );

IMPLEMENT_CONOBJECT(PostEffect);


GFX_ImplementTextureProfile( PostFxTextureProfile,
                            GFXTextureProfile::DiffuseMap,
                            GFXTextureProfile::Static | GFXTextureProfile::PreserveSize | GFXTextureProfile::NoMipmap,
                            GFXTextureProfile::NONE );

GFX_ImplementTextureProfile( VRTextureProfile,
                            GFXTextureProfile::DiffuseMap,
                            GFXTextureProfile::PreserveSize |
                            GFXTextureProfile::RenderTarget |
                            GFXTextureProfile::NoMipmap,
                            GFXTextureProfile::NONE );

GFX_ImplementTextureProfile( VRDepthProfile,
                            GFXTextureProfile::DiffuseMap,
                            GFXTextureProfile::PreserveSize |
                            GFXTextureProfile::NoMipmap |
                            GFXTextureProfile::ZTarget,
                            GFXTextureProfile::NONE );

void PostEffect::EffectConst::set( const String &newVal )
{
   if ( mStringVal == newVal )
      return;

   mStringVal = newVal;
   mDirty = true;
}

void PostEffect::EffectConst::setToBuffer( GFXShaderConstBufferRef buff )
{
   // Nothing to do if the value hasn't changed.
   if ( !mDirty )
      return;
   mDirty = false;

   // If we don't have a handle... get it now.
   if ( !mHandle )
      mHandle = buff->getShader()->getShaderConstHandle( mName );

   // If the handle isn't valid then we're done.
   if ( !mHandle->isValid() )
      return;

   const GFXShaderConstType type = mHandle->getType();

   // For now, we're only going
   // to support float4 arrays.
   // Expand to other types as necessary.
   U32 arraySize = mHandle->getArraySize();

   const char *strVal = mStringVal.c_str();

   if ( type == GFXSCT_Int )
   {
      S32 val;
      Con::setData( TypeS32, &val, 0, 1, &strVal );
      buff->set( mHandle, val );
   }
   else if ( type == GFXSCT_Float )
   {
      F32 val;
      Con::setData( TypeF32, &val, 0, 1, &strVal );
      buff->set( mHandle, val );
   }
   else if ( type == GFXSCT_Float2 )
   {
      Point2F val;
      Con::setData( TypePoint2F, &val, 0, 1, &strVal );
      buff->set( mHandle, val );
   }
   else if ( type == GFXSCT_Float3 )
   {
      Point3F val;
      Con::setData( TypePoint3F, &val, 0, 1, &strVal );
      buff->set( mHandle, val );
   }
   else if ( type == GFXSCT_Float4 )
   {
      Point4F val;

      if ( arraySize > 1 )
      {
         // Do array setup!
         //U32 unitCount = StringUnit::getUnitCount( strVal, "\t" );
         //AssertFatal( unitCount == arraySize, "" );

         String tmpString;
         Vector<Point4F> valArray;

         for ( U32 i = 0; i < arraySize; i++ )
         {
            tmpString = StringUnit::getUnit( strVal, i, "\t" );
            valArray.increment();
            const char *tmpCStr = tmpString.c_str();

            Con::setData( TypePoint4F, &valArray.last(), 0, 1, &tmpCStr );
         }

         AlignedArray<Point4F> rectData( valArray.size(), sizeof( Point4F ), (U8*)valArray.address(), false );
         buff->set( mHandle, rectData );
      }
      else
      {
         // Do regular setup.
         Con::setData( TypePoint4F, &val, 0, 1, &strVal );
         buff->set( mHandle, val );
      }
   }
   else
   {
#if TORQUE_DEBUG
      const char* err = avar("PostEffect::EffectConst::setToBuffer $s type is not implemented", mName.c_str());
      Con::errorf(err);
      GFXAssertFatal(0,err);
#endif
   }
}


//-------------------------------------------------------------------------
// PostEffect
//-------------------------------------------------------------------------

PostEffect::PostEffect()
   :  mRenderTime( PFXAfterDiffuse ),
      mRenderPriority( 1.0 ),
      mEnabled( false ),
      mSkip( false ),
      mUpdateShader( true ),
      mStateBlockData( NULL ),
      mAllowReflectPass( false ),
      mTargetClear( PFXTargetClear_None ),
      mTargetViewport( PFXTargetViewport_TargetSize ),
      mTargetScale( Point2F::One ),
      mTargetSize( Point2I::Zero ),
      mTargetFormat( GFXFormatR8G8B8A8 ),
      mTargetClearColor( ColorF::BLACK ),
      mOneFrameOnly( false ),
      mOnThisFrame( true ),
      mShaderReloadKey( 0 ),
      mIsValid( false ),
      mRTSizeSC( NULL ),
      mOneOverRTSizeSC( NULL ),
      mViewportOffsetSC( NULL ),
      mTargetViewportSC( NULL ),
      mFogDataSC( NULL ),
      mFogColorSC( NULL ),
      mEyePosSC( NULL ),
      mMatWorldToScreenSC( NULL ),
      mMatScreenToWorldSC( NULL ),
      mMatPrevScreenToWorldSC( NULL ),
      mNearFarSC( NULL ),
      mInvNearFarSC( NULL ),
      mWorldToScreenScaleSC( NULL ),
      mProjectionOffsetSC( NULL ),
      mWaterColorSC( NULL ),
      mWaterFogDataSC( NULL ),
      mAmbientColorSC( NULL ),
      mWaterFogPlaneSC( NULL ),
      mWaterDepthGradMaxSC( NULL ),
      mScreenSunPosSC( NULL ),
      mLightDirectionSC( NULL ),
      mCameraForwardSC( NULL ),
      mAccumTimeSC( NULL ),
      mDeltaTimeSC( NULL ),
      mInvCameraMatSC( NULL )
{
   dMemset( mActiveTextures, 0, sizeof( GFXTextureObject* ) * NumTextures );
   dMemset( mActiveNamedTarget, 0, sizeof( NamedTexTarget* ) * NumTextures );
   dMemset( mActiveTextureViewport, 0, sizeof( RectI ) * NumTextures );
   dMemset( mTexSizeSC, 0, sizeof( GFXShaderConstHandle* ) * NumTextures );
   dMemset( mRenderTargetParamsSC, 0, sizeof( GFXShaderConstHandle* ) * NumTextures );
}

PostEffect::~PostEffect()
{
   EffectConstTable::Iterator iter = mEffectConsts.begin();
   for ( ; iter != mEffectConsts.end(); iter++ )
      delete iter->value;
}

void PostEffect::initPersistFields()
{
   addField( "shader", TypeRealString, Offset( mShaderName, PostEffect ),
      "Name of a GFXShaderData for this effect." );

   addField( "stateBlock", TYPEID<GFXStateBlockData>(), Offset( mStateBlockData,  PostEffect ),
      "Name of a GFXStateBlockData for this effect." );

   addField( "target", TypeRealString, Offset( mTargetName, PostEffect ),
      "String identifier of this effect's target texture.\n"
      "@see PFXTextureIdentifiers" );
   
   addField( "targetDepthStencil", TypeRealString, Offset( mTargetDepthStencilName, PostEffect ),
      "Optional string identifier for this effect's target depth/stencil texture.\n"
      "@see PFXTextureIdentifiers" );

   addField( "targetScale", TypePoint2F, Offset( mTargetScale, PostEffect ),
       "If targetSize is zero this is used to set a relative size from the current target." );
       
   addField( "targetSize", TypePoint2I, Offset( mTargetSize, PostEffect ), 
      "If non-zero this is used as the absolute target size." );   
      
   addField( "targetFormat", TypeGFXFormat, Offset( mTargetFormat, PostEffect ),
      "Format of the target texture, not applicable if writing to the backbuffer." );

   addField( "targetClearColor", TypeColorF, Offset( mTargetClearColor, PostEffect ),
      "Color to which the target texture is cleared before rendering." );

   addField( "targetClear", TYPEID< PFXTargetClear >(), Offset( mTargetClear, PostEffect ),
      "Describes when the target texture should be cleared." );

   addField( "targetViewport", TYPEID< PFXTargetViewport >(), Offset( mTargetViewport, PostEffect ),
      "Specifies how the viewport should be set up for a target texture." );

   addField( "texture", TypeImageFilename, Offset( mTexFilename, PostEffect ), NumTextures,
      "Input textures to this effect ( samplers ).\n"
      "@see PFXTextureIdentifiers" );

   addField( "renderTime", TYPEID< PFXRenderTime >(), Offset( mRenderTime, PostEffect ),
      "When to process this effect during the frame." );

   addField( "renderBin", TypeRealString, Offset( mRenderBin, PostEffect ),
      "Name of a renderBin, used if renderTime is PFXBeforeBin or PFXAfterBin." );

   addField( "renderPriority", TypeF32, Offset( mRenderPriority, PostEffect ), 
      "PostEffects are processed in DESCENDING order of renderPriority if more than one has the same renderBin/Time." );

   addField( "allowReflectPass", TypeBool, Offset( mAllowReflectPass, PostEffect ), 
      "Is this effect processed during reflection render passes." );

   addProtectedField( "isEnabled", TypeBool, Offset( mEnabled, PostEffect),
      &PostEffect::_setIsEnabled, &defaultProtectedGetFn,
      "Is the effect on." );

   addField( "onThisFrame", TypeBool, Offset( mOnThisFrame, PostEffect ), 
      "Allows you to turn on a PostEffect for only a single frame." );

   addField( "oneFrameOnly", TypeBool, Offset( mOneFrameOnly, PostEffect ), 
      "Allows you to turn on a PostEffect for only a single frame." );

   addField( "skip", TypeBool, Offset( mSkip, PostEffect ), 
      "Skip processing of this PostEffect and its children even if its parent "
      "is enabled. Parent and sibling PostEffects in the chain are still processed." );

   Parent::initPersistFields();
}

bool PostEffect::onAdd()
{
   if( !Parent::onAdd() )
      return false;

   LightManager::smActivateSignal.notify( this, &PostEffect::_onLMActivate );
   mUpdateShader = true;

   // Grab the script path.
   Torque::Path scriptPath( Con::getVariable( "$Con::File" ) );
   scriptPath.setFileName( String::EmptyString );
   scriptPath.setExtension( String::EmptyString );

   // Find additional textures
   for( S32 i = 0; i < NumTextures; i++ )
   {
      String texFilename = mTexFilename[i];

      // Skip empty stages or ones with variable or target names.
      if (  texFilename.isEmpty() ||
            texFilename[0] == '$' ||
            texFilename[0] == '#' )
         continue;

      // If '/', then path is specified, open normally
      if ( texFilename[0] != '/' )
         texFilename = scriptPath.getFullPath() + '/' + texFilename;

      // Try to load the texture.
      bool success = mTextures[i].set( texFilename, &PostFxTextureProfile, avar( "%s() - (line %d)", __FUNCTION__, __LINE__ ) );
      if (!success)
         Con::errorf("Invalid Texture for PostEffect (%s), The Texture '%s' does not exist!", this->getName(), texFilename.c_str());
   }

   // Is the target a named target?
   if ( mTargetName.isNotEmpty() && mTargetName[0] == '#' )
   {
      mNamedTarget.registerWithName( mTargetName.substr( 1 ) );
      mNamedTarget.getTextureDelegate().bind( this, &PostEffect::_getTargetTexture );
   }
   if ( mTargetDepthStencilName.isNotEmpty() && mTargetDepthStencilName[0] == '#' )
      mNamedTargetDepthStencil.registerWithName( mTargetDepthStencilName.substr( 1 ) );

   if (mNamedTarget.isRegistered() || mNamedTargetDepthStencil.isRegistered())
      GFXTextureManager::addEventDelegate( this, &PostEffect::_onTextureEvent );

   // Call onAdd in script
   onAdd_callback();

   // Should we start enabled?
   if ( mEnabled )
   {
      mEnabled = false;
      enable();
   }

   getSet()->addObject( this );

   return true;
}

void PostEffect::onRemove()
{
   Parent::onRemove();

   PFXMGR->_removeEffect( this );

   LightManager::smActivateSignal.remove( this, &PostEffect::_onLMActivate );

   mShader = NULL;
   _cleanTargets();

   if ( mNamedTarget.isRegistered() || mNamedTargetDepthStencil.isRegistered() )
      GFXTextureManager::removeEventDelegate( this, &PostEffect::_onTextureEvent );

   if ( mNamedTarget.isRegistered() )
   {
      mNamedTarget.unregister();
      mNamedTarget.getTextureDelegate().clear();
   }

   if ( mNamedTargetDepthStencil.isRegistered() )
      mNamedTargetDepthStencil.unregister();
}

void PostEffect::_updateScreenGeometry(   const Frustum &frustum,
                                          GFXVertexBufferHandle<PFXVertex> *outVB )
{
   outVB->set( GFX, 4, GFXBufferTypeVolatile );

   const Point3F *frustumPoints = frustum.getPoints();
   const Point3F& cameraPos = frustum.getPosition();

   // Perform a camera offset.  We need to manually perform this offset on the postFx's
   // polygon, which is at the far plane.
   const Point2F& projOffset = frustum.getProjectionOffset();
   Point3F cameraOffsetPos = cameraPos;
   if(!projOffset.isZero())
   {
      // First we need to calculate the offset at the near plane.  The projOffset
      // given above can be thought of a percent as it ranges from 0..1 (or 0..-1).
      F32 nearOffset = frustum.getNearRight() * projOffset.x;

      // Now given the near plane distance from the camera we can solve the right
      // triangle and calcuate the SIN theta for the offset at the near plane.
      // SIN theta = x/y
      F32 sinTheta = nearOffset / frustum.getNearDist();

      // Finally, we can calcuate the offset at the far plane, which is where our sun (or vector)
      // light's polygon is drawn.
      F32 farOffset = frustum.getFarDist() * sinTheta;

      // We can now apply this far plane offset to the far plane itself, which then compensates
      // for the project offset.
      MatrixF camTrans = frustum.getTransform();
      VectorF offset = camTrans.getRightVector();
      offset *= farOffset;
      cameraOffsetPos += offset;
   }

   PFXVertex *vert = outVB->lock();

   vert->point.set(-1.0, 1.0, 0.0);
   vert->texCoord.set(0.0f, 0.0f);
   vert->wsEyeRay = frustumPoints[Frustum::FarTopLeft] - cameraOffsetPos;
   vert++;

   vert->point.set(1.0, 1.0, 0.0);
   vert->texCoord.set(1.0f, 0.0f);
   vert->wsEyeRay = frustumPoints[Frustum::FarTopRight] - cameraOffsetPos;
   vert++;

   vert->point.set(-1.0, -1.0, 0.0);
   vert->texCoord.set(0.0f, 1.0f);
   vert->wsEyeRay = frustumPoints[Frustum::FarBottomLeft] - cameraOffsetPos;
   vert++;

   vert->point.set(1.0, -1.0, 0.0);
   vert->texCoord.set(1.0f, 1.0f);
   vert->wsEyeRay = frustumPoints[Frustum::FarBottomRight] - cameraOffsetPos;
   vert++;

   outVB->unlock();
}

void PostEffect::_setupStateBlock( const SceneRenderState *state )
{
   if ( mStateBlock.isNull() )
   {
      GFXStateBlockDesc desc;
      if ( mStateBlockData )
         desc = mStateBlockData->getState();

      mStateBlock = GFX->createStateBlock( desc );
   }

   GFX->setStateBlock( mStateBlock );
}

void PostEffect::_setupConstants( const SceneRenderState *state )
{
   // Alloc the const buffer.
   if ( mShaderConsts.isNull() )
   {
      mShaderConsts = mShader->allocConstBuffer();

      mRTSizeSC = mShader->getShaderConstHandle( "$targetSize" );
      mOneOverRTSizeSC = mShader->getShaderConstHandle( "$oneOverTargetSize" );

      mTexSizeSC[0] = mShader->getShaderConstHandle( "$texSize0" );
      mTexSizeSC[1] = mShader->getShaderConstHandle( "$texSize1" );
      mTexSizeSC[2] = mShader->getShaderConstHandle( "$texSize2" );
      mTexSizeSC[3] = mShader->getShaderConstHandle( "$texSize3" );
      mTexSizeSC[4] = mShader->getShaderConstHandle( "$texSize4" );
      mTexSizeSC[5] = mShader->getShaderConstHandle( "$texSize5" );
      mTexSizeSC[6] = mShader->getShaderConstHandle( "$texSize6" );
      mTexSizeSC[7] = mShader->getShaderConstHandle( "$texSize7" );

      mRenderTargetParamsSC[0] = mShader->getShaderConstHandle( "$rtParams0" );
      mRenderTargetParamsSC[1] = mShader->getShaderConstHandle( "$rtParams1" );
      mRenderTargetParamsSC[2] = mShader->getShaderConstHandle( "$rtParams2" );
      mRenderTargetParamsSC[3] = mShader->getShaderConstHandle( "$rtParams3" );
      mRenderTargetParamsSC[4] = mShader->getShaderConstHandle( "$rtParams4" );
      mRenderTargetParamsSC[5] = mShader->getShaderConstHandle( "$rtParams5" );
      mRenderTargetParamsSC[6] = mShader->getShaderConstHandle( "$rtParams6" );
      mRenderTargetParamsSC[7] = mShader->getShaderConstHandle( "$rtParams7" );

      //mViewportSC = shader->getShaderConstHandle( "$viewport" );

      mTargetViewportSC = mShader->getShaderConstHandle( "$targetViewport" );

      mFogDataSC = mShader->getShaderConstHandle( ShaderGenVars::fogData );
      mFogColorSC = mShader->getShaderConstHandle( ShaderGenVars::fogColor );

      mEyePosSC = mShader->getShaderConstHandle( ShaderGenVars::eyePosWorld );

      mNearFarSC = mShader->getShaderConstHandle( "$nearFar" );
      mInvNearFarSC = mShader->getShaderConstHandle( "$invNearFar" );
      mWorldToScreenScaleSC = mShader->getShaderConstHandle( "$worldToScreenScale" );

      mMatWorldToScreenSC = mShader->getShaderConstHandle( "$matWorldToScreen" );
      mMatScreenToWorldSC = mShader->getShaderConstHandle( "$matScreenToWorld" );
      mMatPrevScreenToWorldSC = mShader->getShaderConstHandle( "$matPrevScreenToWorld" );

      mProjectionOffsetSC = mShader->getShaderConstHandle( "$projectionOffset" );

      mWaterColorSC = mShader->getShaderConstHandle( "$waterColor" );
      mAmbientColorSC = mShader->getShaderConstHandle( "$ambientColor" );
      mWaterFogDataSC = mShader->getShaderConstHandle( "$waterFogData" );
      mWaterFogPlaneSC = mShader->getShaderConstHandle( "$waterFogPlane" );      
      mWaterDepthGradMaxSC = mShader->getShaderConstHandle( "$waterDepthGradMax" );
      mScreenSunPosSC = mShader->getShaderConstHandle( "$screenSunPos" );
      mLightDirectionSC = mShader->getShaderConstHandle( "$lightDirection" );
      mCameraForwardSC = mShader->getShaderConstHandle( "$camForward" );

      mAccumTimeSC = mShader->getShaderConstHandle( "$accumTime" );
      mDeltaTimeSC = mShader->getShaderConstHandle( "$deltaTime" );

      mInvCameraMatSC = mShader->getShaderConstHandle( "$invCameraMat" );
   }

   // Set up shader constants for source image size
   if ( mRTSizeSC->isValid() )
   {
      const Point2I &resolution = GFX->getActiveRenderTarget()->getSize();
      Point2F pixelShaderConstantData;

      pixelShaderConstantData.x = resolution.x;
      pixelShaderConstantData.y = resolution.y;

      mShaderConsts->set( mRTSizeSC, pixelShaderConstantData );
   }

   if ( mOneOverRTSizeSC->isValid() )
   {
      const Point2I &resolution = GFX->getActiveRenderTarget()->getSize();
      Point2F oneOverTargetSize( 1.0f / (F32)resolution.x, 1.0f / (F32)resolution.y );

      mShaderConsts->set( mOneOverRTSizeSC, oneOverTargetSize );
   }

   // Set up additional textures
   Point2F texSizeConst;
   for( U32 i = 0; i < NumTextures; i++ )
   {
      if( !mActiveTextures[i] )
         continue;

      if ( mTexSizeSC[i]->isValid() )
      {
         texSizeConst.x = (F32)mActiveTextures[i]->getWidth();
         texSizeConst.y = (F32)mActiveTextures[i]->getHeight();
         mShaderConsts->set( mTexSizeSC[i], texSizeConst );
      }
   }

   for ( U32 i = 0; i < NumTextures; i++ )
   {
      if ( !mRenderTargetParamsSC[i]->isValid() )
         continue;

      Point4F rtParams( Point4F::One );

      if ( mActiveTextures[i] )
      {
         const Point3I &targetSz = mActiveTextures[i]->getSize();
         RectI targetVp = mActiveTextureViewport[i];
         ScreenSpace::RenderTargetParameters(targetSz, targetVp, rtParams);
      }

      mShaderConsts->set( mRenderTargetParamsSC[i], rtParams );
   }

   // Target viewport (in target space)
   if ( mTargetViewportSC->isValid() )
   {
      const Point2I& targetSize = GFX->getActiveRenderTarget()->getSize();
      Point3I size(targetSize.x, targetSize.y, 0);
      const RectI& viewport = GFX->getViewport();

      Point2F offset((F32)viewport.point.x / (F32)targetSize.x, (F32)viewport.point.y / (F32)targetSize.y );
      Point2F scale((F32)viewport.extent.x / (F32)targetSize.x, (F32)viewport.extent.y / (F32)targetSize.y );

      const bool hasTexelPixelOffset = GFX->getAdapterType() == Direct3D9;
      const Point2F halfPixel(  hasTexelPixelOffset ? (0.5f / targetSize.x) : 0.0f, 
                                hasTexelPixelOffset ? (0.5f / targetSize.y) : 0.0f );

      Point4F targetParams;
      targetParams.x = offset.x + halfPixel.x;
      targetParams.y = offset.y + halfPixel.y;
      targetParams.z = offset.x + scale.x - halfPixel.x;
      targetParams.w = offset.y + scale.y - halfPixel.y;

      mShaderConsts->set( mTargetViewportSC, targetParams );
   }

   // Set the fog data.
   if ( mFogDataSC->isValid() )
   {
      const FogData &data = state->getSceneManager()->getFogData();

      Point3F params;
      params.x = data.density;
      params.y = data.densityOffset;

      if ( !mIsZero( data.atmosphereHeight ) )
         params.z = 1.0f / data.atmosphereHeight;
      else
         params.z = 0.0f;

      mShaderConsts->set( mFogDataSC, params );
   }

   const PFXFrameState &thisFrame = PFXMGR->getFrameState();

   if ( mMatWorldToScreenSC->isValid() )
   {
      // Screen space->world space
      MatrixF tempMat = thisFrame.cameraToScreen;
      tempMat.mul( thisFrame.worldToCamera );
      tempMat.fullInverse();
      tempMat.transpose();

      // Support using these matrices as float3x3 or float4x4...
      mShaderConsts->set( mMatWorldToScreenSC, tempMat, mMatWorldToScreenSC->getType() );
   }

   if ( mMatScreenToWorldSC->isValid() )
   {
      // World space->screen space
      MatrixF tempMat = thisFrame.cameraToScreen;
      tempMat.mul( thisFrame.worldToCamera );
      tempMat.transpose();

      // Support using these matrices as float3x3 or float4x4...
      mShaderConsts->set( mMatScreenToWorldSC, tempMat, mMatScreenToWorldSC->getType() );
   }

   if ( mMatPrevScreenToWorldSC->isValid() )
   {
      const PFXFrameState &lastFrame = PFXMGR->getLastFrameState();

      // Previous frame world space->screen space
      MatrixF tempMat = lastFrame.cameraToScreen;
      tempMat.mul( lastFrame.worldToCamera );
      tempMat.transpose();
      mShaderConsts->set( mMatPrevScreenToWorldSC, tempMat );
   }

   if (mAmbientColorSC->isValid() && state)
   {
      const ColorF &sunlight = state->getAmbientLightColor();
      Point3F ambientColor( sunlight.red, sunlight.green, sunlight.blue );

      mShaderConsts->set( mAmbientColorSC, ambientColor );
   }

   mShaderConsts->setSafe( mAccumTimeSC, MATMGR->getTotalTime() );
   mShaderConsts->setSafe( mDeltaTimeSC, MATMGR->getDeltaTime() );

   // Now set all the constants that are dependent on the scene state.
   if ( state )
   {
      mShaderConsts->setSafe( mEyePosSC, state->getDiffuseCameraPosition() );
      mShaderConsts->setSafe( mNearFarSC, Point2F( state->getNearPlane(), state->getFarPlane() ) );
      mShaderConsts->setSafe( mInvNearFarSC, Point2F( 1.0f / state->getNearPlane(), 1.0f / state->getFarPlane() ) );
      mShaderConsts->setSafe( mWorldToScreenScaleSC, state->getWorldToScreenScale() );
      mShaderConsts->setSafe( mProjectionOffsetSC, state->getCameraFrustum().getProjectionOffset() );
      mShaderConsts->setSafe( mFogColorSC, state->getSceneManager()->getFogData().color );

      if ( mWaterColorSC->isValid() )
      {
         ColorF color( state->getSceneManager()->getWaterFogData().color );
         mShaderConsts->set( mWaterColorSC, color );
      }

      if ( mWaterFogDataSC->isValid() )
      {
         const WaterFogData &data = state->getSceneManager()->getWaterFogData();
         Point4F params( data.density, data.densityOffset, data.wetDepth, data.wetDarkening );
         mShaderConsts->set( mWaterFogDataSC, params );
      }

      if ( mWaterFogPlaneSC->isValid() )
      {
         const PlaneF &plane = state->getSceneManager()->getWaterFogData().plane;
         mShaderConsts->set( mWaterFogPlaneSC, plane );
      }

      if ( mWaterDepthGradMaxSC->isValid() )
      {
         mShaderConsts->set( mWaterDepthGradMaxSC, state->getSceneManager()->getWaterFogData().depthGradMax );
      }

      if ( mScreenSunPosSC->isValid() )
      {
         // Grab our projection matrix
         // from the frustum.
         Frustum frust = state->getCameraFrustum();
         MatrixF proj( true );
         frust.getProjectionMatrix( &proj );

         // Grab the ScatterSky world matrix.
         MatrixF camMat = state->getCameraTransform();
         camMat.inverse();
         MatrixF tmp( true );
         tmp = camMat;
         tmp.setPosition( Point3F( 0, 0, 0 ) );

         Point3F sunPos( 0, 0, 0 );

         // Get the light manager and sun light object.
         LightInfo *sunLight = LIGHTMGR->getSpecialLight( LightManager::slSunLightType );

         // Grab the light direction and scale
         // by the ScatterSky radius to get the world
         // space sun position.
         const VectorF &lightDir = sunLight->getDirection();

         Point3F lightPos( lightDir.x * (6378.0f * 1000.0f),
                           lightDir.y * (6378.0f * 1000.0f),
                           lightDir.z * (6378.0f * 1000.0f) );

         RectI viewPort = GFX->getViewport();

         // Get the screen space sun position.
         MathUtils::mProjectWorldToScreen(lightPos, &sunPos, viewPort, tmp, proj);

         // And normalize it to the 0 to 1 range.
         sunPos.x -= (F32)viewPort.point.x;
         sunPos.y -= (F32)viewPort.point.y;
         sunPos.x /= (F32)viewPort.extent.x;
         sunPos.y /= (F32)viewPort.extent.y;

         mShaderConsts->set( mScreenSunPosSC, Point2F( sunPos.x, sunPos.y ) );
      }

      if ( mLightDirectionSC->isValid() )
      {
         LightInfo *sunLight = LIGHTMGR->getSpecialLight( LightManager::slSunLightType );

         const VectorF &lightDir = sunLight->getDirection();
         mShaderConsts->set( mLightDirectionSC, lightDir );
      }

      if ( mCameraForwardSC->isValid() )
      {
         const MatrixF &camMat = state->getCameraTransform();
         VectorF camFwd( 0, 0, 0 );

         camMat.getColumn( 1, &camFwd );

         mShaderConsts->set( mCameraForwardSC, camFwd );
      }

      if ( mInvCameraMatSC->isValid() )
      {
         MatrixF mat = state->getCameraTransform();
         mat.inverse();
         mShaderConsts->set( mInvCameraMatSC, mat, mInvCameraMatSC->getType() );
      }

   } // if ( state )

   // Set EffectConsts - specified from script

   // If our shader has reloaded since last frame we must mark all
   // EffectConsts dirty so they will be reset.
   if ( mShader->getReloadKey() != mShaderReloadKey )
   {
      mShaderReloadKey = mShader->getReloadKey();

      EffectConstTable::Iterator iter = mEffectConsts.begin();
      for ( ; iter != mEffectConsts.end(); iter++ )
      {
         iter->value->mDirty = true;
         iter->value->mHandle = NULL;
      }
   }

   // Doesn't look like anyone is using this anymore.
   // But if we do want to pass this info to script,
   // we should do so in the same way as I am doing below.
   /*
   Point2F texSizeScriptConst( 0, 0 );
   String buffer;
   if ( mActiveTextures[0] )
   {
      texSizeScriptConst.x = (F32)mActiveTextures[0]->getWidth();
      texSizeScriptConst.y = (F32)mActiveTextures[0]->getHeight();

      dSscanf( buffer.c_str(), "%g %g", texSizeScriptConst.x, texSizeScriptConst.y );
   }
   */
      
   {
      PROFILE_SCOPE( PostEffect_SetShaderConsts );

      // Pass some data about the current render state to script.
      // 
      // TODO: This is pretty messy... it should go away.  This info
      // should be available from some other script accessible method
      // or field which isn't PostEffect specific.
      //
      if ( state )
      {
         Con::setFloatVariable( "$Param::NearDist", state->getNearPlane() );
         Con::setFloatVariable( "$Param::FarDist", state->getFarPlane() );   
      }

      setShaderConsts_callback();
   }   

   EffectConstTable::Iterator iter = mEffectConsts.begin();
   for ( ; iter != mEffectConsts.end(); iter++ )
      iter->value->setToBuffer( mShaderConsts );
}

void PostEffect::_setupTexture( U32 stage, GFXTexHandle &inputTex, const RectI *inTexViewport )
{
   const String &texFilename = mTexFilename[ stage ];

   GFXTexHandle theTex;
   NamedTexTarget *namedTarget = NULL;

   RectI viewport = GFX->getViewport();

   if ( texFilename.compare( "$inTex", 0, String::NoCase ) == 0 )
   {
      theTex = inputTex;

      if ( inTexViewport )
      {
         viewport = *inTexViewport;
      }
      else if ( theTex )
      {
         viewport.set( 0, 0, theTex->getWidth(), theTex->getHeight() );
      }
   }
   else if ( texFilename.compare( "$backBuffer", 0, String::NoCase ) == 0 )
   {
      theTex = PFXMGR->getBackBufferTex();

      // Always use the GFX viewport when reading from the backbuffer
   }
   else if ( texFilename.isNotEmpty() && texFilename[0] == '#' )
   {
      namedTarget = NamedTexTarget::find( texFilename.c_str() + 1 );
      if ( namedTarget )
      {
         theTex = namedTarget->getTexture( 0 );
         viewport = namedTarget->getViewport();
      }
   }
   else
   {
      theTex = mTextures[ stage ];
      if ( theTex )
         viewport.set( 0, 0, theTex->getWidth(), theTex->getHeight() );
   }

   mActiveTextures[ stage ] = theTex;
   mActiveNamedTarget[ stage ] = namedTarget;
   mActiveTextureViewport[ stage ] = viewport;

   if ( theTex.isValid() )
      GFX->setTexture( stage, theTex );
}

void PostEffect::_setupTransforms()
{
   // Set everything to identity.
   GFX->setWorldMatrix( MatrixF::Identity );
   GFX->setProjectionMatrix( MatrixF::Identity );
}

void PostEffect::_setupTarget( const SceneRenderState *state, bool *outClearTarget )
{
   if (  mNamedTarget.isRegistered() || 
         mTargetName.compare( "$outTex", 0, String::NoCase ) == 0 )
   {
      // Size it relative to the texture of the first stage or
      // if NULL then use the current target.

      Point2I targetSize;

      // If we have an absolute target size then use that.
      if ( !mTargetSize.isZero() )
         targetSize = mTargetSize;

      // Else generate a relative size using the target scale.
      else if ( mActiveTextures[ 0 ] )
      {
         const Point3I &texSize = mActiveTextures[ 0 ]->getSize();

         targetSize.set(   texSize.x * mTargetScale.x,
                           texSize.y * mTargetScale.y );
      }
      else
      {
         GFXTarget *oldTarget = GFX->getActiveRenderTarget();
         const Point2I &oldTargetSize = oldTarget->getSize();

         targetSize.set(   oldTargetSize.x * mTargetScale.x,
                           oldTargetSize.y * mTargetScale.y );
      }

      // Make sure its at least 1x1.
      targetSize.setMax( Point2I::One );

      if (  mNamedTarget.isRegistered() ||
            !mTargetTex ||
            mTargetTex.getWidthHeight() != targetSize )
      {
         mTargetTex.set( targetSize.x, targetSize.y, mTargetFormat,
            &PostFxTargetProfile, "PostEffect::_setupTarget" );

         if ( mTargetClear == PFXTargetClear_OnCreate )
            *outClearTarget = true;

         if(mTargetViewport == PFXTargetViewport_GFXViewport)
         {
            // We may need to scale the GFX viewport to fit within
            // our target texture size
            GFXTarget *oldTarget = GFX->getActiveRenderTarget();
            const Point2I &oldTargetSize = oldTarget->getSize();
            Point2F scale(targetSize.x / F32(oldTargetSize.x), targetSize.y / F32(oldTargetSize.y));

            const RectI &viewport = GFX->getViewport();

            mNamedTarget.setViewport( RectI( viewport.point.x*scale.x, viewport.point.y*scale.y, viewport.extent.x*scale.x, viewport.extent.y*scale.y ) );
         }
         else if(mTargetViewport == PFXTargetViewport_NamedInTexture0 && mActiveNamedTarget[0] && mActiveNamedTarget[0]->getTexture())
         {
            // Scale the named input texture's viewport to match our target
            const Point3I &namedTargetSize = mActiveNamedTarget[0]->getTexture()->getSize();
            Point2F scale(targetSize.x / F32(namedTargetSize.x), targetSize.y / F32(namedTargetSize.y));

            const RectI &viewport = mActiveNamedTarget[0]->getViewport();

            mNamedTarget.setViewport( RectI( viewport.point.x*scale.x, viewport.point.y*scale.y, viewport.extent.x*scale.x, viewport.extent.y*scale.y ) );
         }
         else
         {
            // PFXTargetViewport_TargetSize
            mNamedTarget.setViewport( RectI( 0, 0, targetSize.x, targetSize.y ) );
         }
      }
   }
   else
      mTargetTex = NULL;

   // Do we have a named depthStencil target?
   if ( mNamedTargetDepthStencil.isRegistered() )
   {
      // Size it relative to the texture of the first stage or
      // if NULL then use the current target.
      Point2I targetSize;

      // If we have an absolute target size then use that.
      if ( !mTargetSize.isZero() )
         targetSize = mTargetSize;

      // Else generate a relative size using the target scale.
      else if ( mActiveTextures[ 0 ] )
      {
         const Point3I &texSize = mActiveTextures[ 0 ]->getSize();

         targetSize.set(   texSize.x * mTargetScale.x,
                           texSize.y * mTargetScale.y );
      }
      else
      {
         GFXTarget *oldTarget = GFX->getActiveRenderTarget();
         const Point2I &oldTargetSize = oldTarget->getSize();

         targetSize.set(   oldTargetSize.x * mTargetScale.x,
                           oldTargetSize.y * mTargetScale.y );
      }

      // Make sure its at least 1x1.
      targetSize.setMax( Point2I::One );
      
      if (  mNamedTargetDepthStencil.isRegistered() &&
            mTargetDepthStencil.getWidthHeight() != targetSize )
      {         
         mTargetDepthStencil.set( targetSize.x, targetSize.y, GFXFormatD24S8,
                     &GFXDefaultZTargetProfile, "PostEffect::_setupTarget" );

         if ( mTargetClear == PFXTargetClear_OnCreate )
            *outClearTarget = true;

         if(mTargetViewport == PFXTargetViewport_GFXViewport)
         {
            // We may need to scale the GFX viewport to fit within
            // our target texture size
            GFXTarget *oldTarget = GFX->getActiveRenderTarget();
            const Point2I &oldTargetSize = oldTarget->getSize();
            Point2F scale(targetSize.x / F32(oldTargetSize.x), targetSize.y / F32(oldTargetSize.y));

            const RectI &viewport = GFX->getViewport();

            mNamedTargetDepthStencil.setViewport( RectI( viewport.point.x*scale.x, viewport.point.y*scale.y, viewport.extent.x*scale.x, viewport.extent.y*scale.y ) );
         }
         else if(mTargetViewport == PFXTargetViewport_NamedInTexture0 && mActiveNamedTarget[0] && mActiveNamedTarget[0]->getTexture())
         {
            // Scale the named input texture's viewport to match our target
            const Point3I &namedTargetSize = mActiveNamedTarget[0]->getTexture()->getSize();
            Point2F scale(targetSize.x / F32(namedTargetSize.x), targetSize.y / F32(namedTargetSize.y));

            const RectI &viewport = mActiveNamedTarget[0]->getViewport();

            mNamedTargetDepthStencil.setViewport( RectI( viewport.point.x*scale.x, viewport.point.y*scale.y, viewport.extent.x*scale.x, viewport.extent.y*scale.y ) );
         }
         else
         {
            // PFXTargetViewport_TargetSize
            mNamedTargetDepthStencil.setViewport( RectI( 0, 0, targetSize.x, targetSize.y ) );
         }
      }
   }
   else
      mTargetDepthStencil = NULL;

   if ( mTargetClear == PFXTargetClear_OnDraw )
      *outClearTarget = true;

   if ( !mTarget && (mTargetTex || mTargetDepthStencil) )
      mTarget = GFX->allocRenderToTextureTarget();
}

void PostEffect::_cleanTargets( bool recurse )
{
   mTargetTex = NULL;
   mTargetDepthStencil = NULL;
   mTarget = NULL;

   if ( !recurse )
      return;

   // Clear the children too!
   for ( U32 i = 0; i < size(); i++ )
   {
      PostEffect *effect = (PostEffect*)(*this)[i];
      effect->_cleanTargets( true );
   }
}

void PostEffect::process(  const SceneRenderState *state,
                           GFXTexHandle &inOutTex,
                           const RectI *inTexViewport )
{
   // If the shader is forced to be skipped... then skip.
   if ( mSkip )
      return;

   // Skip out if we don't support reflection passes.
   if ( state && state->isReflectPass() && !mAllowReflectPass )
      return;

   if ( mOneFrameOnly && !mOnThisFrame )
      return;

   // Check requirements if the shader needs updating.
   if ( mUpdateShader )
   {
      _checkRequirements();

      // Clear the targets if we failed passing
      // the requirements at this time.
      if ( !mIsValid )
         _cleanTargets( true );
   }

   // If we're not valid then we cannot render.
   if ( !mIsValid )
      return;

   GFXDEBUGEVENT_SCOPE_EX( PostEffect_Process, ColorI::GREEN, avar("PostEffect: %s", getName()) );

   preProcess_callback();   

   GFXTransformSaver saver;

   // Set the textures.
   for ( U32 i = 0; i < NumTextures; i++ )
      _setupTexture( i, inOutTex, inTexViewport );

   _setupStateBlock( state ) ;
   _setupTransforms();

   bool clearTarget = false;
   _setupTarget( state, &clearTarget );

   if ( mTargetTex || mTargetDepthStencil )
   {

#ifdef TORQUE_OS_XENON
      // You may want to disable this functionality for speed reasons as it does
      // add some overhead. The upside is it makes things "just work". If you
      // re-work your post-effects properly, this is not needed.
      //
      // If this post effect doesn't alpha blend to the back-buffer, than preserve
      // the active render target contents so they are still around the next time
      // that render target activates
      if(!mStateBlockData->getState().blendEnable)
         GFX->getActiveRenderTarget()->preserve();
#endif

      const RectI &oldViewport = GFX->getViewport();
      GFXTarget *oldTarget = GFX->getActiveRenderTarget();

      GFX->pushActiveRenderTarget();
      mTarget->attachTexture( GFXTextureTarget::Color0, mTargetTex );

      // Set the right depth stencil target.
      if ( !mTargetDepthStencil && mTargetTex.getWidthHeight() == GFX->getActiveRenderTarget()->getSize() )
         mTarget->attachTexture( GFXTextureTarget::DepthStencil, GFXTextureTarget::sDefaultDepthStencil );
      else
         mTarget->attachTexture( GFXTextureTarget::DepthStencil, mTargetDepthStencil );

      // Set the render target but not its viewport.  We'll do that below.
      GFX->setActiveRenderTarget( mTarget, false );

      if(mNamedTarget.isRegistered())
      {
         // Always use the name target's viewport, if available.  It was set up in _setupTarget().
         GFX->setViewport(mNamedTarget.getViewport());
      }
      else if(mTargetViewport == PFXTargetViewport_GFXViewport)
      {
         // Go with the current viewport as scaled against our render target.
         const Point2I &oldTargetSize = oldTarget->getSize();
         const Point2I &targetSize = mTarget->getSize();
         Point2F scale(targetSize.x / F32(oldTargetSize.x), targetSize.y / F32(oldTargetSize.y));
         GFX->setViewport( RectI( oldViewport.point.x*scale.x, oldViewport.point.y*scale.y, oldViewport.extent.x*scale.x, oldViewport.extent.y*scale.y ) );
      }
      else if(mTargetViewport == PFXTargetViewport_NamedInTexture0 && mActiveNamedTarget[0] && mActiveNamedTarget[0]->getTexture())
      {
         // Go with the first input texture, if it is named.  Scale the named input texture's viewport to match our target
         const Point3I &namedTargetSize = mActiveNamedTarget[0]->getTexture()->getSize();
         const Point2I &targetSize = mTarget->getSize();
         Point2F scale(targetSize.x / F32(namedTargetSize.x), targetSize.y / F32(namedTargetSize.y));

         const RectI &viewport = mActiveNamedTarget[0]->getViewport();

         GFX->setViewport( RectI( viewport.point.x*scale.x, viewport.point.y*scale.y, viewport.extent.x*scale.x, viewport.extent.y*scale.y ) );
      }
      else
      {
         // Default to using the whole target as the viewport
         GFX->setViewport( RectI( Point2I::Zero, mTarget->getSize() ) );
      }
   }

   if ( clearTarget )
      GFX->clear( GFXClearTarget, mTargetClearColor, 1.f, 0 );

   // Setup the shader and constants.
   if ( mShader )
   {
      GFX->setShader( mShader );
      _setupConstants( state );

      GFX->setShaderConstBuffer( mShaderConsts );
   }
   else
      GFX->setupGenericShaders();

   Frustum frustum;
   if ( state )
      frustum = state->getCameraFrustum();
   else
   {
      // If we don't have a scene state then setup
      // a dummy frustum... you better not be depending
      // on this being related to the camera in any way.
      
      frustum = Frustum();
   }

   GFXVertexBufferHandle<PFXVertex> vb;
   _updateScreenGeometry( frustum, &vb );

   // Draw it.
   GFX->setVertexBuffer( vb );
   GFX->drawPrimitive( GFXTriangleStrip, 0, 2 );

   // Allow PostEffecVis to hook in.
   PFXVIS->onPFXProcessed( this );

   if ( mTargetTex || mTargetDepthStencil )
   {
      mTarget->resolve();
      GFX->popActiveRenderTarget();
   }
   else
   {
      // We wrote to the active back buffer, so release
      // the current texture copy held by the manager.
      //
      // This ensures a new copy is made.
      PFXMGR->releaseBackBufferTex();
   }

   // Return and release our target texture.
   inOutTex = mTargetTex;
   if ( !mNamedTarget.isRegistered() )
      mTargetTex = NULL;

   // Restore the transforms before the children
   // are processed as it screws up the viewport.
   saver.restore();

   // Now process my children.
   iterator i = begin();
   for ( ; i != end(); i++ )
   {
      PostEffect *effect = static_cast<PostEffect*>(*i);
      effect->process( state, inOutTex );
   }

   if ( mOneFrameOnly )
      mOnThisFrame = false;
}

bool PostEffect::_setIsEnabled( void *object, const char *index, const char *data )
{
   bool enabled = dAtob( data );
   if ( enabled )
      static_cast<PostEffect*>( object )->enable();
   else
      static_cast<PostEffect*>( object )->disable();

   // Always return false from a protected field.
   return false;
}

void PostEffect::enable()
{
   // Don't add TexGen PostEffects to the PostEffectManager!
   if ( mRenderTime == PFXTexGenOnDemand )
      return;

   // Ignore it if its already enabled.
   if ( mEnabled )
      return;

   mEnabled = true;

   // We cannot really enable the effect 
   // until its been registed.
   if ( !isProperlyAdded() )
      return;

   // If the enable callback returns 'false' then 
   // leave the effect disabled.
   if ( !onEnabled_callback() )   
   {
      mEnabled = false;
      return;
   }

   PFXMGR->_addEffect( this );
}

void PostEffect::disable()
{
   if ( !mEnabled )
      return;

   mEnabled = false;
   _cleanTargets( true );

   if ( isProperlyAdded() )
   {
      PFXMGR->_removeEffect( this );
      onDisabled_callback();      
   }
}

void PostEffect::reload()
{
   // Reload the shader if we have one or mark it
   // for updating when its processed next.
   if ( mShader )
      mShader->reload();
   else
      mUpdateShader = true;

   // Null stateblock so it is reloaded.
   mStateBlock = NULL;

   // Call reload on any children
   // this PostEffect may have.
   for ( U32 i = 0; i < size(); i++ )
   {
      PostEffect *effect = (PostEffect*)(*this)[i];
      effect->reload();
   }
}

void PostEffect::setTexture( U32 index, const String &texFilePath )
{
	// Set the new texture name.
	mTexFilename[index] = texFilePath;
	mTextures[index].free();

    // Skip empty stages or ones with variable or target names.
    if (	texFilePath.isEmpty() ||
			texFilePath[0] == '$' ||
			texFilePath[0] == '#' )
		return;

    // Try to load the texture.
    mTextures[index].set( texFilePath, &PostFxTextureProfile, avar( "%s() - (line %d)", __FUNCTION__, __LINE__ ) );
}

void PostEffect::setShaderConst( const String &name, const String &val )
{
   PROFILE_SCOPE( PostEffect_SetShaderConst );

   EffectConstTable::Iterator iter = mEffectConsts.find( name );
   if ( iter == mEffectConsts.end() )
   {
      EffectConst *newConst = new EffectConst( name, val );
      iter = mEffectConsts.insertUnique( name, newConst );
   }

   iter->value->set( val );
}

F32 PostEffect::getAspectRatio() const
{
   const Point2I &rtSize = GFX->getActiveRenderTarget()->getSize();
   return (F32)rtSize.x / (F32)rtSize.y;
}

void PostEffect::_checkRequirements()
{
   // This meets requirements if its shader loads
   // properly, we can find all the input textures,
   // and its formats are supported.

   mIsValid = false;
   mUpdateShader = false;
   mShader = NULL;
   mShaderConsts = NULL;
   EffectConstTable::Iterator iter = mEffectConsts.begin();
   for ( ; iter != mEffectConsts.end(); iter++ )
   {
      iter->value->mDirty = true;
      iter->value->mHandle = NULL;
   }

   // First make sure the target format is supported.
   if ( mNamedTarget.isRegistered() )
   {
      Vector<GFXFormat> formats;
      formats.push_back( mTargetFormat );
      GFXFormat format = GFX->selectSupportedFormat(  &PostFxTargetProfile,
                                                      formats, 
                                                      true,
                                                      false,
                                                      false );
      
      // If we didn't get our format out then its unsupported!
      if ( format != mTargetFormat )
         return;
   }

   // Gather macros specified on this PostEffect.
   Vector<GFXShaderMacro> macros( mShaderMacros );

   // Now check the input named targets and make sure
   // they exist... else we're invalid.
   for ( U32 i=0; i < NumTextures; i++ )
   {
      const String &texFilename = mTexFilename[i];

      if ( texFilename.isNotEmpty() && texFilename[0] == '#' )
      {
         NamedTexTarget *namedTarget = NamedTexTarget::find( texFilename.c_str() + 1 );
         if ( !namedTarget )
            return;

         // Grab the macros for shader initialization.
         namedTarget->getShaderMacros( &macros );
      }
   }

   // Finally find and load the shader.
   ShaderData *shaderData;
   if ( Sim::findObject( mShaderName, shaderData ) )   
      if ( shaderData->getPixVersion() <= GFX->getPixelShaderVersion() )
         mShader = shaderData->getShader( macros );

   // If we didn't get a shader... we're done.
   if ( !mShader )
      return;

   // If we got here then we're valid.
   mIsValid = true;
}

bool PostEffect::dumpShaderDisassembly( String &outFilename ) const
{
   String data;

   if ( !mShader || !mShader->getDisassembly( data ) )
      return false;
         
   outFilename = FS::MakeUniquePath( "", "ShaderDisassembly", "txt" );

   FileStream *fstream = FileStream::createAndOpen( outFilename, Torque::FS::File::Write );
   if ( !fstream )
      return false;

   fstream->write( data );
   fstream->close();
   delete fstream;   

   return true;
}

SimSet* PostEffect::getSet() const
{
   SimSet *set;
   if ( !Sim::findObject( "PFXSet", set ) )
   {
      set = new SimSet();
      set->registerObject( "PFXSet" );
      Sim::getRootGroup()->addObject( set );
   }

   return set;
}

void PostEffect::setShaderMacro( const String &name, const String &value )
{
   // Check to see if we already have this macro.
   Vector<GFXShaderMacro>::iterator iter = mShaderMacros.begin();
   for ( ; iter != mShaderMacros.end(); iter++ )
   {
      if ( iter->name == name )
      {
         if ( iter->value != value )
         {
            iter->value = value;
            mUpdateShader = true;
         }
         return;
      }
   }

   // Add a new macro.
   mShaderMacros.increment();
   mShaderMacros.last().name = name;
   mShaderMacros.last().value = value;
   mUpdateShader = true;
}

bool PostEffect::removeShaderMacro( const String &name )
{
   Vector<GFXShaderMacro>::iterator iter = mShaderMacros.begin();
   for ( ; iter != mShaderMacros.end(); iter++ )
   {
      if ( iter->name == name )
      {
         mShaderMacros.erase( iter );
         mUpdateShader = true;
         return true;
      }
   }

   return false;
}

void PostEffect::clearShaderMacros()
{
   if ( mShaderMacros.empty() )
      return;

   mShaderMacros.clear();
   mUpdateShader = true;
}

GFXTextureObject* PostEffect::_getTargetTexture( U32 )
{
   // A TexGen PostEffect will generate its texture now if it
   // has not already.
   if (  mRenderTime == PFXTexGenOnDemand &&
         ( !mTargetTex || mUpdateShader ) )
   {
      GFXTexHandle chainTex;
      process( NULL, chainTex );
      
      // TODO: We should add a conditional copy
      // to a non-RT texture here to reduce the
      // amount of non-swappable RTs in use.      
   }

   return mTargetTex.getPointer();
}

DefineEngineMethod( PostEffect, reload, void, (),,
   "Reloads the effect shader and textures." )
{
   return object->reload();
}

DefineEngineMethod( PostEffect, enable, void, (),,
   "Enables the effect." )
{
   object->enable();
}

DefineEngineMethod( PostEffect, disable, void, (),,
   "Disables the effect." )
{
   object->disable();
}

DefineEngineMethod( PostEffect, toggle, bool, (),,
   "Toggles the effect between enabled / disabled.\n"
   "@return True if effect is enabled." )
{
   if ( object->isEnabled() )
      object->disable();
   else
      object->enable();

   return object->isEnabled();
}

DefineEngineMethod( PostEffect, isEnabled, bool, (),,
   "@return True if the effect is enabled." )
{
   return object->isEnabled();
}

DefineEngineMethod( PostEffect, setTexture, void, ( S32 index, const char *filePath ),,
   "This is used to set the texture file and load the texture on a running effect. "
   "If the texture file is not different from the current file nothing is changed.  If "
   "the texture cannot be found a null texture is assigned.\n"    
   "@param index The texture stage index.\n" 
   "@param filePath The file name of the texture to set.\n" )
{   
	if ( index > -1 && index < PostEffect::NumTextures )
		object->setTexture( index, filePath );
}

DefineEngineMethod( PostEffect, setShaderConst, void, ( const char* name, const char* value ),,
   "Sets the value of a uniform defined in the shader. This will usually "
   "be called within the setShaderConsts callback. Array type constants are "
   "not supported.\n"    
   "@param name Name of the constanst, prefixed with '$'.\n" 
   "@param value Value to set, space seperate values with more than one element.\n"
   "@tsexample\n"
   "function MyPfx::setShaderConsts( %this )\n"
   "{\n"
   "   // example float4 uniform\n"
   "   %this.setShaderConst( \"$colorMod\", \"1.0 0.9 1.0 1.0\" );\n"
   "   // example float1 uniform\n"
   "   %this.setShaderConst( \"$strength\", \"3.0\" );\n"
   "   // example integer uniform\n"
   "   %this.setShaderConst( \"$loops\", \"5\" );"
   "}\n"
   "@endtsexample" )   
{
   object->setShaderConst( name, value );
}

DefineEngineMethod( PostEffect, getAspectRatio, F32, (),,
   "@return Width over height of the backbuffer." )
{
   return object->getAspectRatio();
}

DefineEngineMethod( PostEffect, dumpShaderDisassembly, String, (),,
   "Dumps this PostEffect shader's disassembly to a temporary text file.\n"
   "@return Full path to the dumped file or an empty string if failed." )
{
   String fileName;
   object->dumpShaderDisassembly( fileName );

   return fileName;   
}

DefineEngineMethod( PostEffect, setShaderMacro, void, ( const char* key, const char* value ), ( "" ),
   "Adds a macro to the effect's shader or sets an existing one's value. "
   "This will usually be called within the onAdd or preProcess callback.\n"
   "@param key lval of the macro."
   "@param value rval of the macro, or may be empty."
   "@tsexample\n"
   "function MyPfx::onAdd( %this )\n"
   "{\n"
   "   %this.setShaderMacro( \"NUM_SAMPLES\", \"10\" );\n"
   "   %this.setShaderMacro( \"HIGH_QUALITY_MODE\" );\n"
   "   \n"
   "   // In the shader looks like... \n"
   "   // #define NUM_SAMPLES 10\n"
   "   // #define HIGH_QUALITY_MODE\n"
   "}\n"
   "@endtsexample" )
{   
   object->setShaderMacro( key, value );
}

DefineEngineMethod( PostEffect, removeShaderMacro, void, ( const char* key ),,
   "Remove a shader macro. This will usually be called within the preProcess callback.\n"
   "@param key Macro to remove." )
{
   object->removeShaderMacro( key );
}

DefineEngineMethod( PostEffect, clearShaderMacros, void, (),,
   "Remove all shader macros." )
{
   object->clearShaderMacros();
}

DefineEngineFunction( dumpRandomNormalMap, void, (),,
   "Creates a 64x64 normal map texture filled with noise. The texture is saved "
   "to randNormTex.png in the location of the game executable.\n\n"
   "@ingroup GFX")
{
   GFXTexHandle tex;

   tex.set( 64, 64, GFXFormatR8G8B8A8, &GFXDefaultPersistentProfile, "" );

   GFXLockedRect *rect = tex.lock();
   U8 *f = rect->bits;

   for ( U32 i = 0; i < 64*64; i++, f += 4 )
   {               
      VectorF vec;

      vec.x = mRandF( -1.0f, 1.0f );
      vec.y = mRandF( -1.0f, 1.0f );
      vec.z = mRandF( -1.0f, 1.0f );

      vec.normalizeSafe();

      f[0] = U8_MAX * ( ( 1.0f + vec.x ) * 0.5f );
      f[1] = U8_MAX * ( ( 1.0f + vec.y ) * 0.5f );
      f[2] = U8_MAX * ( ( 1.0f + vec.z ) * 0.5f );
      f[3] = U8_MAX;
   }

   tex.unlock();

   String path = Torque::FS::MakeUniquePath( "", "randNormTex", "png" );
   tex->dumpToDisk( "png", path );   
}