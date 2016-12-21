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
#include "environment/waterObject.h"

#include "console/consoleTypes.h"
#include "materials/materialParameters.h"
#include "materials/baseMatInstance.h"
#include "materials/materialManager.h"
#include "materials/customMaterialDefinition.h"
#include "materials/sceneData.h"
#include "core/stream/bitStream.h"
#include "scene/reflectionManager.h"
#include "scene/sceneRenderState.h"
#include "lighting/lightInfo.h"
#include "math/mathIO.h"
#include "postFx/postEffect.h"
#include "T3D/gameBase/gameConnection.h"
#include "T3D/shapeBase.h"
#include "gfx/gfxOcclusionQuery.h"
#include "gfx/gfxTextureManager.h"
#include "gfx/sim/cubemapData.h"
#include "math/util/matrixSet.h"
#include "sfx/sfxAmbience.h"
#include "T3D/sfx/sfx3DWorld.h"
#include "sfx/sfxTypes.h"


GFXImplementVertexFormat( GFXWaterVertex )
{
   addElement( "POSITION", GFXDeclType_Float3 );
   addElement( "NORMAL", GFXDeclType_Float3 );
   addElement( "TEXCOORD", GFXDeclType_Float2, 0 );
   addElement( "TEXCOORD", GFXDeclType_Float4, 1 );   
}

void WaterMatParams::clear()
{
   mRippleMatSC = NULL;
   mRippleDirSC = NULL;
   mRippleTexScaleSC = NULL;
   mRippleSpeedSC = NULL;
   mRippleMagnitudeSC = NULL;
   mFoamDirSC = NULL;
   mFoamTexScaleSC = NULL;
   mFoamSpeedSC = NULL;
   mFoamOpacitySC = NULL;
   mWaveDirSC = NULL;
   mWaveDataSC = NULL;   
   mReflectTexSizeSC = NULL;
   mBaseColorSC = NULL;
   mMiscParamsSC = NULL;
   mReflectParamsSC = NULL;
   mReflectNormalSC = NULL;
   mHorizonPositionSC = NULL;
   mFogParamsSC = NULL;   
   mMoreFogParamsSC = NULL;
   mFarPlaneDistSC = NULL;
   mWetnessParamsSC = NULL;
   mDistortionParamsSC = NULL;
   mUndulateMaxDistSC = NULL;
   mAmbientColorSC = NULL;
   mLightDirSC = NULL;
   mFoamParamsSC = NULL;   
   mGridElementSizeSC = NULL;
   mElapsedTimeSC = NULL;
   mFoamSamplerSC = NULL;
   mRippleSamplerSC = NULL;
   mCubemapSamplerSC = NULL;
   mSpecularParamsSC = NULL;
   mDepthGradMaxSC = NULL;
   mReflectivitySC = NULL;
   mDepthGradSamplerSC = NULL;
}

void WaterMatParams::init( BaseMatInstance* matInst )
{
   clear();

   mRippleMatSC = matInst->getMaterialParameterHandle( "$rippleMat" );
   mRippleDirSC = matInst->getMaterialParameterHandle( "$rippleDir" );
   mRippleTexScaleSC = matInst->getMaterialParameterHandle( "$rippleTexScale" );
   mRippleSpeedSC = matInst->getMaterialParameterHandle( "$rippleSpeed" );
   mRippleMagnitudeSC = matInst->getMaterialParameterHandle( "$rippleMagnitude" );
   mFoamDirSC = matInst->getMaterialParameterHandle( "$foamDir" );
   mFoamTexScaleSC = matInst->getMaterialParameterHandle( "$foamTexScale" );
   mFoamSpeedSC = matInst->getMaterialParameterHandle( "$foamSpeed" );
   mFoamOpacitySC = matInst->getMaterialParameterHandle( "$foamOpacity" );
   mWaveDirSC = matInst->getMaterialParameterHandle( "$waveDir" );
   mWaveDataSC = matInst->getMaterialParameterHandle( "$waveData" );
   mReflectTexSizeSC = matInst->getMaterialParameterHandle( "$reflectTexSize" );
   mBaseColorSC = matInst->getMaterialParameterHandle( "$baseColor" );
   mMiscParamsSC = matInst->getMaterialParameterHandle( "$miscParams" );
   mReflectParamsSC = matInst->getMaterialParameterHandle( "$reflectParams" );   
   mReflectNormalSC = matInst->getMaterialParameterHandle( "$reflectNormal" );
   mHorizonPositionSC = matInst->getMaterialParameterHandle( "$horizonPos" );
   mFogParamsSC = matInst->getMaterialParameterHandle( "$fogParams" ); 
   mMoreFogParamsSC = matInst->getMaterialParameterHandle( "$moreFogParams" );
   mFarPlaneDistSC = matInst->getMaterialParameterHandle( "$farPlaneDist" );
   mWetnessParamsSC = matInst->getMaterialParameterHandle( "$wetnessParams" );
   mDistortionParamsSC = matInst->getMaterialParameterHandle( "$distortionParams" );
   mUndulateMaxDistSC = matInst->getMaterialParameterHandle( "$undulateMaxDist" );   
   mAmbientColorSC = matInst->getMaterialParameterHandle( "$ambientColor" );
   mLightDirSC = matInst->getMaterialParameterHandle( "$inLightVec" );
   mFoamParamsSC = matInst->getMaterialParameterHandle( "$foamParams" );   
   mGridElementSizeSC = matInst->getMaterialParameterHandle( "$gridElementSize" );
   mElapsedTimeSC = matInst->getMaterialParameterHandle( "$elapsedTime" );
   mModelMatSC = matInst->getMaterialParameterHandle( "$modelMat" );
   mFoamSamplerSC = matInst->getMaterialParameterHandle( "$foamMap" );
   mRippleSamplerSC = matInst->getMaterialParameterHandle( "$bumpMap" );
   mCubemapSamplerSC = matInst->getMaterialParameterHandle( "$skyMap" );
   mSpecularParamsSC = matInst->getMaterialParameterHandle( "$specularParams" );   
   mDepthGradMaxSC = matInst->getMaterialParameterHandle( "$depthGradMax" );
   mReflectivitySC = matInst->getMaterialParameterHandle( "$reflectivity" );
   mDepthGradSamplerSC = matInst->getMaterialParameterHandle( "$depthGradMap" );
}


bool WaterObject::smWireframe = false;
bool WaterObject::smDisableTrueReflections = false;

//-------------------------------------------------------------------------
// WaterObject Class
//-------------------------------------------------------------------------

IMPLEMENT_CONOBJECT( WaterObject );

ConsoleDocClass( WaterObject,
   "@brief Abstract base class for representing a body of water.\n\n"
   
   "%WaterObject is abstract and may not be created. It defines functionality "
   "shared by its derived classes.\n\n"
   
   "%WaterObject exposes many fields for controlling it visual quality.\n\n"

   "%WaterObject surface rendering has the following general features:\n"
   "\t- Waves represented by vertex undulation and user paramaters.\n"
   "\t- Ripples represented by a normal map and user parameters.\n"
   "\t- Refraction of underwater objects.\n"
   "\t- Dynamic planar reflection or static cubemap reflection.\n"
   "\t- Paramable water fog and color shift.\n\n"

   "It will, however, look significantly different depending on the LightingManager "
   "that is active. With Basic Lighting, we do not have a prepass texture to "
   "lookup per-pixel depth and therefore cannot use our rendering techniques that depend on it.\n\n"   

   "In particular, the following field groups are not used under Basic Lighting:\n"
   "\t- Underwater Fogging \n"
   "\t- Misc \n"
   "\t- Distortion \n"
   "\t- And foam related fields under the %WaterObject group.\n\n"

   "%WaterObject also defines several fields for gameplay use and objects "
   "that support buoyancy.\n\n"   
   
   "@ingroup Water"
);

WaterObject::WaterObject()
 : mViscosity( 1.0f ),
   mDensity( 1.0f ),
   mLiquidType( "Water" ),
   mFresnelBias( 0.3f ),
   mFresnelPower( 6.0f ),
   mReflectNormalUp( true ),
   mReflectivity( 0.5f ),   
   mDistortStartDist( 0.1f ),
   mDistortEndDist( 20.0f ),
   mDistortFullDepth( 3.5f ),
   mOverallFoamOpacity( 1.0f ),
   mFoamMaxDepth( 2.0f ),
   mFoamAmbientLerp( 0.5f ),
   mFoamRippleInfluence( 0.05f ),
   mClarity( 0.5f ),
   mUnderwaterColor(9, 6, 5, 240),
   mUndulateMaxDist(50.0f),
   mMiscParamW( 0.0f ),
   mUnderwaterPostFx( NULL ),
   mBasicLighting( false ),
   mOverallWaveMagnitude( 1.0f ),
   mOverallRippleMagnitude( 0.1f ),
   mCubemap( NULL ),
   mSoundAmbience( NULL ),
   mSpecularPower( 48.0f ),
   mSpecularColor( 1.0f, 1.0f, 1.0f, 1.0f ),
   mDepthGradientMax( 50.0f ),
   mEmissive( false )
{
   mTypeMask = WaterObjectType;

   for( U32 i=0; i < MAX_WAVES; i++ )
   {
      mRippleDir[i].set( 0.0f, 0.0f );
      mRippleSpeed[i] = 0.0f;
      mRippleTexScale[i].set( 0.0f, 0.0f );

      mWaveDir[i].set( 0.0f, 0.0f );
      mWaveSpeed[i] = 0.0f;      
      mWaveMagnitude[i] = 0.0f;
   }

   for ( U32 i = 0; i < MAX_FOAM; i++ )
   {
      mFoamDir[i].set( 0.0f, 0.0f );
      mFoamSpeed[i] = 0.0f;
      mFoamTexScale[i].set( 0.0f, 0.0f );
      mFoamOpacity[i] = 0.0f;
   }   

   mFoamDir[0].set( 1, 0 );
   mFoamDir[1].set( 0, 1 );
   mFoamTexScale[0].set( 1, 1 );
   mFoamTexScale[1].set( 3, 3 );

   mRippleMagnitude[0] = 1.0f;
   mRippleMagnitude[1] = 1.0f;
   mRippleMagnitude[2] = 0.3f;

   mWaterFogData.density = 0.1f;
   mWaterFogData.densityOffset = 1.0f;     
   mWaterFogData.wetDepth = 1.5f;
   mWaterFogData.wetDarkening = 0.2f;
   mWaterFogData.color = ColorI::BLUE;

   mSurfMatName[WaterMat] = "WaterMat";
   mSurfMatName[UnderWaterMat] = "UnderWaterMat";
   mSurfMatName[BasicWaterMat] = "WaterBasicMat";   
   mSurfMatName[BasicUnderWaterMat] = "UnderWaterBasicMat";

   dMemset( mMatInstances, 0, sizeof(mMatInstances) );

   mWaterPos.set( 0,0,0 );
   mWaterPlane.set( mWaterPos, Point3F(0,0,1) );

   mGenerateVB = true;

   mMatrixSet = reinterpret_cast<MatrixSet *>(dMalloc_aligned(sizeof(MatrixSet), 16));
   constructInPlace(mMatrixSet);
}

WaterObject::~WaterObject()
{
   dFree_aligned(mMatrixSet);
}


void WaterObject::initPersistFields()
{
   addGroup( "WaterObject" );

      addProtectedField( "density", TypeF32, Offset( mDensity, WaterObject ), &WaterObject::_checkDensity, &defaultProtectedGetFn, "Affects buoyancy of an object, thus affecting the Z velocity of a player (jumping, falling, etc.");
      addField( "viscosity", TypeF32, Offset( mViscosity, WaterObject ), "Affects drag force applied to an object submerged in this container." );
      addField( "liquidType", TypeRealString, Offset( mLiquidType, WaterObject ), "Liquid type of WaterBlock, such as water, ocean, lava"
		  " Currently only Water is defined and used.");
      addField( "baseColor", TypeColorI,  Offset( mWaterFogData.color, WaterObject ), "Changes color of water fog." );
      addField( "fresnelBias",  TypeF32,  Offset( mFresnelBias, WaterObject ), "Extent of fresnel affecting reflection fogging." );
      addField( "fresnelPower",  TypeF32,  Offset( mFresnelPower, WaterObject ), "Measures intensity of affect on reflection based on fogging." );
      addField( "specularPower", TypeF32, Offset( mSpecularPower, WaterObject ), "Power used for specularity on the water surface ( sun only )." );
      addField( "specularColor", TypeColorF, Offset( mSpecularColor, WaterObject ), "Color used for specularity on the water surface ( sun only )." );
      addField( "emissive", TypeBool, Offset( mEmissive, WaterObject ), "When true the water colors don't react to changes to environment lighting." );

      addArray( "Waves (vertex undulation)", MAX_WAVES );

         addField( "waveDir",       TypePoint2F,  Offset( mWaveDir, WaterObject ), MAX_WAVES, "Direction waves flow toward shores." );
         addField( "waveSpeed",     TypeF32,  Offset( mWaveSpeed, WaterObject ), MAX_WAVES, "Speed of water undulation." );
         addField( "waveMagnitude", TypeF32, Offset( mWaveMagnitude, WaterObject ), MAX_WAVES, "Height of water undulation." );

      endArray( "Waves (vertex undulation)" );

      addField( "overallWaveMagnitude", TypeF32, Offset( mOverallWaveMagnitude, WaterObject ), "Master variable affecting entire body" 
		  " of water's undulation" );  
      
      addField( "rippleTex", TypeImageFilename, Offset( mRippleTexName, WaterObject ), "Normal map used to simulate small surface ripples" );

      addArray( "Ripples (texture animation)", MAX_WAVES );

         addField( "rippleDir",       TypePoint2F, Offset( mRippleDir, WaterObject ), MAX_WAVES, "Modifies the direction of ripples on the surface." );
         addField( "rippleSpeed",     TypeF32, Offset( mRippleSpeed, WaterObject ), MAX_WAVES, "Modifies speed of surface ripples.");
         addField( "rippleTexScale",  TypePoint2F, Offset( mRippleTexScale, WaterObject ), MAX_WAVES, "Intensifies the affect of the normal map "
			 "applied to the surface.");
         addField( "rippleMagnitude", TypeF32, Offset( mRippleMagnitude, WaterObject ), MAX_WAVES, "Intensifies the vertext modification of the surface." );

      endArray( "Ripples (texture animation)" );

      addField( "overallRippleMagnitude", TypeF32, Offset( mOverallRippleMagnitude, WaterObject ), "Master variable affecting entire surface");

      addField( "foamTex", TypeImageFilename, Offset( mFoamTexName, WaterObject ), "Diffuse texture for foam in shallow water (advanced lighting only)" );

      addArray( "Foam", MAX_FOAM );

         addField( "foamDir",       TypePoint2F, Offset( mFoamDir, WaterObject ), MAX_FOAM, "" );
         addField( "foamSpeed",     TypeF32, Offset( mFoamSpeed, WaterObject ), MAX_FOAM, "");
         addField( "foamTexScale",  TypePoint2F, Offset( mFoamTexScale, WaterObject ), MAX_FOAM, ""
			 "applied to the surface.");
         addField( "foamOpacity", TypeF32, Offset( mFoamOpacity, WaterObject ), MAX_FOAM, "" );

      endArray( "Foam" );
      
      addField( "overallFoamOpacity", TypeF32, Offset( mOverallFoamOpacity, WaterObject ), "" );
      addField( "foamMaxDepth", TypeF32, Offset( mFoamMaxDepth, WaterObject ), "" );
      addField( "foamAmbientLerp", TypeF32, Offset( mFoamAmbientLerp, WaterObject ), "" );     
      addField( "foamRippleInfluence", TypeF32, Offset( mFoamRippleInfluence, WaterObject ), "" );

   endGroup( "WaterObject" );

   addGroup( "Reflect" );

      addField( "cubemap", TypeCubemapName, Offset( mCubemapName, WaterObject ), "Cubemap used instead of reflection texture if fullReflect is off." );
      
      addProtectedField( "fullReflect", TypeBool, Offset( mFullReflect, WaterObject ), 
         &WaterObject::_setFullReflect, 
         &defaultProtectedGetFn, 
         "Enables dynamic reflection rendering." );

      addField( "reflectivity", TypeF32, Offset( mReflectivity, WaterObject ), "Overall scalar to the reflectivity of the water surface." );
      addField( "reflectPriority", TypeF32, Offset( mReflectorDesc.priority, WaterObject ), "Affects the sort order of reflected objects." );
      addField( "reflectMaxRateMs", TypeS32, Offset( mReflectorDesc.maxRateMs, WaterObject ), "Affects the sort time of reflected objects." );
      //addField( "reflectMaxDist", TypeF32, Offset( mReflectMaxDist, WaterObject ), "vert distance at which only cubemap color is used" );
      //addField( "reflectMinDist", TypeF32, Offset( mReflectMinDist, WaterObject ), "vert distance at which only reflection color is used" );
      addField( "reflectDetailAdjust", TypeF32, Offset( mReflectorDesc.detailAdjust, WaterObject ), "scale up or down the detail level for objects rendered in a reflection" );
      addField( "reflectNormalUp", TypeBool, Offset( mReflectNormalUp, WaterObject ), "always use z up as the reflection normal" );
      addField( "useOcclusionQuery", TypeBool, Offset( mReflectorDesc.useOcclusionQuery, WaterObject ), "turn off reflection rendering when occluded (delayed)." );
      addField( "reflectTexSize", TypeS32, Offset( mReflectorDesc.texSize, WaterObject ), "The texture size used for reflections (square)" );

   endGroup( "Reflect" );   

   addGroup( "Underwater Fogging" );

      addField( "waterFogDensity", TypeF32, Offset( mWaterFogData.density, WaterObject ), "Intensity of underwater fogging." );
      addField( "waterFogDensityOffset", TypeF32, Offset( mWaterFogData.densityOffset, WaterObject ), "Delta, or limit, applied to waterFogDensity." );
      addField( "wetDepth", TypeF32, Offset( mWaterFogData.wetDepth, WaterObject ), "The depth in world units at which full darkening will be received,"
		  " giving a wet look to objects underwater." );
      addField( "wetDarkening", TypeF32, Offset( mWaterFogData.wetDarkening, WaterObject ), "The refract color intensity scaled at wetDepth." );

   endGroup( "Underwater Fogging" );

   addGroup( "Misc" );
      
      addField( "depthGradientTex", TypeImageFilename, Offset( mDepthGradientTexName, WaterObject ), "1D texture defining the base water color by depth" );
      addField( "depthGradientMax", TypeF32, Offset( mDepthGradientMax, WaterObject ), "Depth in world units, the max range of the color gradient texture." );      

   endGroup( "Misc" );

   addGroup( "Distortion" );

      addField( "distortStartDist", TypeF32, Offset( mDistortStartDist, WaterObject ), "Determines start of distortion effect where water"
		  " surface intersects the camera near plane.");
      addField( "distortEndDist", TypeF32, Offset( mDistortEndDist, WaterObject ), "Max distance that distortion algorithm is performed. "
		  "The lower, the more distorted the effect.");
      addField( "distortFullDepth", TypeF32, Offset( mDistortFullDepth, WaterObject ), "Determines the scaling down of distortion "
		  "in shallow water.");

   endGroup( "Distortion" ); 

   addGroup( "Basic Lighting" );

      addField( "clarity", TypeF32, Offset( mClarity, WaterObject ), "Relative opacity or transparency of the water surface." );
      addField( "underwaterColor", TypeColorI, Offset( mUnderwaterColor, WaterObject ), "Changes the color shading of objects beneath"
		  " the water surface.");

   endGroup( "Basic Lighting" );

   addGroup( "Sound" );
   
      addField( "soundAmbience", TypeSFXAmbienceName, Offset( mSoundAmbience, WaterObject ), "Ambient sound environment when listener is submerged." );
         
   endGroup( "Sound" );

   Parent::initPersistFields();

   Con::addVariable( "$WaterObject::wireframe", TypeBool, &smWireframe, "If true, will render the wireframe of the WaterObject.\n"
	   "@ingroup Water\n");
}

void WaterObject::consoleInit()
{
   Parent::consoleInit();

   Con::addVariable( "$pref::Water::disableTrueReflections", TypeBool, &WaterObject::smDisableTrueReflections, 
      "Force all water objects to use static cubemap reflections.\n"
	  "@ingroup Water");     
}

void WaterObject::inspectPostApply()
{
   Parent::inspectPostApply();

   setMaskBits( UpdateMask | WaveMask | TextureMask | SoundMask );
}

bool WaterObject::processArguments( S32 argc, ConsoleValueRef *argv )
{
   if( typeid( *this ) == typeid( WaterObject ) )
   {
      Con::errorf( ConsoleLogEntry::Script, "WaterObject is an abstract class, only its child classes may be allocated." );
      return false;
   }
   else
      return Parent::processArguments( argc, argv );
}

bool WaterObject::_setFullReflect( void *object, const char *index, const char *data )
{
   WaterObject *water = static_cast<WaterObject*>( object );
   water->mFullReflect = dAtob( data );
   
   if ( water->isProperlyAdded() && water->isClientObject() )
   {
      bool isEnabled = water->mPlaneReflector.isEnabled();

      bool enable = water->mFullReflect && !smDisableTrueReflections;

      if ( enable && !isEnabled )
         water->mPlaneReflector.registerReflector( water, &water->mReflectorDesc );
      else if ( !enable && isEnabled )
         water->mPlaneReflector.unregisterReflector();
   }

   return false;
}

bool WaterObject::_checkDensity( void *object, const char *index, const char *data )
{
   //Water densities above 1000 shoot the player high and fast into the air.
   //value clamped to prevent errors.
   WaterObject *water = static_cast<WaterObject*>( object );
   water->mDensity = mClampF(dAtof( data ), 0.0f, 1000.0f);

   return false;
}

U32 WaterObject::packUpdate( NetConnection * conn, U32 mask, BitStream *stream )
{
   U32 retMask = Parent::packUpdate( conn, mask, stream );

   if ( stream->writeFlag( mask & UpdateMask ) )
   {
      stream->write( mDensity );
      stream->write( mViscosity );
      stream->write( mLiquidType );

      if ( stream->writeFlag( mFullReflect ) )
      {
         stream->write( mReflectorDesc.priority );
         stream->writeInt( mReflectorDesc.maxRateMs, 32 );
         //stream->write( mReflectMaxDist );
         //stream->write( mReflectMinDist );
         stream->write( mReflectorDesc.detailAdjust );         
         stream->writeFlag( mReflectNormalUp );
         stream->writeFlag( mReflectorDesc.useOcclusionQuery );
         stream->writeInt( mReflectorDesc.texSize, 32 );
      }

      stream->write( mReflectivity );

      stream->write( mWaterFogData.density );
      stream->write( mWaterFogData.densityOffset );      
      stream->write( mWaterFogData.wetDepth );
      stream->write( mWaterFogData.wetDarkening );

      stream->write( mDistortStartDist );
      stream->write( mDistortEndDist );
      stream->write( mDistortFullDepth );

      stream->write( mDepthGradientMax );
      stream->writeFlag( mEmissive );
      
      stream->write( mFoamMaxDepth );
      stream->write( mFoamAmbientLerp );     
      stream->write( mFoamRippleInfluence );

      stream->write( mWaterFogData.color );

      stream->write( mFresnelBias );
      stream->write( mFresnelPower );
      
      Point4F specularData( mSpecularColor.red, mSpecularColor.green, mSpecularColor.blue, mSpecularPower );      
      mathWrite( *stream, specularData );

      stream->write( mClarity );
      stream->write( mUnderwaterColor );

      stream->write( mOverallRippleMagnitude );
      stream->write( mOverallWaveMagnitude );
      stream->write( mOverallFoamOpacity );
   }

   if ( stream->writeFlag( mask & WaveMask ) )
   {
      for( U32 i=0; i<MAX_WAVES; i++ )
      {
         stream->write( mRippleSpeed[i] );
         mathWrite( *stream, mRippleDir[i] );
         mathWrite( *stream, mRippleTexScale[i] );
         stream->write( mRippleMagnitude[i] );

         stream->write( mWaveSpeed[i] );
         mathWrite( *stream, mWaveDir[i] );
         stream->write( mWaveMagnitude[i] );  
      }

      for ( U32 i = 0; i < MAX_FOAM; i++ )
      {
         stream->write( mFoamSpeed[i] );
         mathWrite( *stream, mFoamDir[i] );
         mathWrite( *stream, mFoamTexScale[i] );
         stream->write( mFoamOpacity[i] );
      }
   }

   if ( stream->writeFlag( mask & MaterialMask ) )
   {
      for ( U32 i = 0; i < NumMatTypes; i++ )      
         stream->write( mSurfMatName[i] );
   }

   if ( stream->writeFlag( mask & TextureMask ) )
   {
      stream->write( mRippleTexName );
      stream->write( mDepthGradientTexName );
      stream->write( mFoamTexName );
      stream->write( mCubemapName );      
   }

   if( stream->writeFlag( mask & SoundMask ) )
      sfxWrite( stream, mSoundAmbience );

   return retMask;
}

void WaterObject::unpackUpdate( NetConnection * conn, BitStream *stream )
{
   Parent::unpackUpdate( conn, stream );

   // UpdateMask
   if ( stream->readFlag() )
   {
      stream->read( &mDensity );
      stream->read( &mViscosity );
      stream->read( &mLiquidType );
      
      if ( stream->readFlag() )
      {
         mFullReflect = true;
         stream->read( &mReflectorDesc.priority );
         mReflectorDesc.maxRateMs = stream->readInt( 32 );
         //stream->read( &mReflectMaxDist );    
         //stream->read( &mReflectMinDist );
         stream->read( &mReflectorDesc.detailAdjust );         
         mReflectNormalUp = stream->readFlag();
         mReflectorDesc.useOcclusionQuery = stream->readFlag();
         mReflectorDesc.texSize = stream->readInt( 32 );

         if ( isProperlyAdded() && !mPlaneReflector.isEnabled() && !smDisableTrueReflections )
            mPlaneReflector.registerReflector( this, &mReflectorDesc );
      }
      else
      {
         mFullReflect = false;
         if ( isProperlyAdded() && mPlaneReflector.isEnabled() )
            mPlaneReflector.unregisterReflector();
      }

      stream->read( &mReflectivity );

      stream->read( &mWaterFogData.density );
      stream->read( &mWaterFogData.densityOffset );      
      stream->read( &mWaterFogData.wetDepth );
      stream->read( &mWaterFogData.wetDarkening );

      stream->read( &mDistortStartDist );
      stream->read( &mDistortEndDist );
      stream->read( &mDistortFullDepth );

      stream->read( &mDepthGradientMax );
      mEmissive = stream->readFlag();

      stream->read( &mFoamMaxDepth );
      stream->read( &mFoamAmbientLerp );      
      stream->read( &mFoamRippleInfluence );

      stream->read( &mWaterFogData.color );

      stream->read( &mFresnelBias );
      stream->read( &mFresnelPower );

      Point4F specularData;
      mathRead( *stream, &specularData );
      mSpecularColor.set( specularData.x, specularData.y, specularData.z, 1.0f );
      mSpecularPower = specularData.w;

      stream->read( &mClarity );
      stream->read( &mUnderwaterColor );

      stream->read( &mOverallRippleMagnitude );
      stream->read( &mOverallWaveMagnitude );
      stream->read( &mOverallFoamOpacity );
   }

   // WaveMask
   if ( stream->readFlag() )
   {
      for( U32 i=0; i<MAX_WAVES; i++ )
      {
         stream->read( &mRippleSpeed[i] );
         mathRead( *stream, &mRippleDir[i] );
         mathRead( *stream, &mRippleTexScale[i] );         
         stream->read( &mRippleMagnitude[i] );

         stream->read( &mWaveSpeed[i] );
         mathRead( *stream, &mWaveDir[i] );         
         stream->read( &mWaveMagnitude[i] );
      }

      for ( U32 i = 0; i < MAX_FOAM; i++ )
      {
         stream->read( &mFoamSpeed[i] );
         mathRead( *stream, &mFoamDir[i] );
         mathRead( *stream, &mFoamTexScale[i] );
         stream->read( &mFoamOpacity[i] );
      }
   }

   // MaterialMask
   if ( stream->readFlag() ) 
   {
      for ( U32 i = 0; i < NumMatTypes; i++ )      
         stream->read( &mSurfMatName[i] );

      if ( isProperlyAdded() )    
      {
         // So they will be reloaded on next use.
         cleanupMaterials();         
      }
   }  

   // TextureMask
   if ( stream->readFlag() )
   {
      stream->read( &mRippleTexName );
      stream->read( &mDepthGradientTexName );
      stream->read( &mFoamTexName );
      stream->read( &mCubemapName );

      if ( isProperlyAdded() )
         initTextures();
   }
   
   // Sound environment.
   if( stream->readFlag() )
   {
      String errorStr;
      if( !sfxReadAndResolve( stream, &mSoundAmbience, errorStr ) )
         Con::errorf( "WaterObject::unpackUpdate - pad packet: %s", errorStr.c_str() );
         
      if( isProperlyAdded() && gSFX3DWorld )
         gSFX3DWorld->notifyChanged( this );
   }
}

void WaterObject::prepRenderImage( SceneRenderState *state )
{
   PROFILE_SCOPE(WaterObject_prepRenderImage);

   // Are we in Basic Lighting?
   mBasicLighting = dStricmp( LIGHTMGR->getId(), "BLM" ) == 0;
   mUnderwater = isUnderwater( state->getCameraPosition() );

   // We only render during the normal diffuse render pass.
   if( !state->isDiffusePass() )
      return;

   // Setup scene transforms
   mMatrixSet->setSceneView(GFX->getWorldMatrix());
   mMatrixSet->setSceneProjection(GFX->getProjectionMatrix());

   _getWaterPlane( state->getCameraPosition(), mWaterPlane, mWaterPos );
   mWaterFogData.plane = mWaterPlane;
   mPlaneReflector.refplane = mWaterPlane;

   updateUnderwaterEffect( state );

   ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
   ri->renderDelegate.bind( this, &WaterObject::renderObject );
   ri->type = RenderPassManager::RIT_Water;
   ri->defaultKey = 1;
   state->getRenderPass()->addInst( ri );

   //mRenderUpdateCount++;
}

void WaterObject::renderObject( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat )
{
   if ( overrideMat )
      return;   

   // TODO: Revive projection z-bias at some point.
   // The current issue with this method of fixing z-fighting
   // in the WaterBlock is that a constant bias does not alleviate
   // the issue at the extreme end of the view range.
   //GFXTransformSaver saver;

   //MatrixF projMat( true );
   //const Frustum &frustum = ri->state->getFrustum();
   //
   //F32 bias = Con::getFloatVariable( "$waterBlockBias", 0.0002418f );

   //MathUtils::getZBiasProjectionMatrix( bias, frustum, &projMat );
   //GFX->setProjectionMatrix( projMat );
 
   
   GFXOcclusionQuery *query = mPlaneReflector.getOcclusionQuery();

   bool doQuery = ( !mPlaneReflector.mQueryPending && query && mReflectorDesc.useOcclusionQuery );

   // We need to call this for avoid a DX9 or Nvidia bug.
   // At some resollutions read from render target,
   // break current occlusion query.
   REFLECTMGR->getRefractTex();

   if ( doQuery )
      query->begin();

   // Real render call, done by derived class.
   innerRender( state );

   if ( doQuery )
      query->end();   

   if ( mUnderwater && mBasicLighting )
      drawUnderwaterFilter( state );
}

void WaterObject::setCustomTextures( S32 matIdx, U32 pass, const WaterMatParams &paramHandles )
{
   // Always use the ripple texture.
   GFX->setTexture( paramHandles.mRippleSamplerSC->getSamplerRegister(pass), mRippleTex );

   // Only above-water in advanced-lighting uses the foam texture.
   if ( matIdx == WaterMat )
   {
      GFX->setTexture( paramHandles.mFoamSamplerSC->getSamplerRegister(pass), mFoamTex );
      GFX->setTexture( paramHandles.mDepthGradSamplerSC->getSamplerRegister(pass), mDepthGradientTex );
   }

   if ( ( matIdx == WaterMat || matIdx == BasicWaterMat ) && mCubemap )   
      GFX->setCubeTexture( paramHandles.mCubemapSamplerSC->getSamplerRegister(pass), mCubemap->mCubemap );
   else if(paramHandles.mCubemapSamplerSC->getSamplerRegister(pass) != -1 )
      GFX->setCubeTexture( paramHandles.mCubemapSamplerSC->getSamplerRegister(pass), NULL );
}

void WaterObject::drawUnderwaterFilter( SceneRenderState *state )
{
   // set up camera transforms
   MatrixF proj = GFX->getProjectionMatrix();
   MatrixF newMat(true);
   GFX->setProjectionMatrix( newMat );
   GFX->pushWorldMatrix();
   GFX->setWorldMatrix( newMat );   

   // set up render states
   GFX->setupGenericShaders();
   GFX->setStateBlock( mUnderwaterSB );

   /*
   const Frustum &frustum = state->getFrustum();
   const MatrixF &camXfm = state->getCameraTransform();
   F32 nearDist = frustum.getNearDist();
   F32 nearLeft = frustum.getNearLeft();
   F32 nearRight = frustum.getNearRight();
   F32 nearTop = frustum.getNearTop();
   F32 nearBottom = frustum.getNearBottom();
   Point3F centerPnt;
   frustum.getCenterPoint( &centerPnt );

   MatrixF.mul
   Point3F linePnt, lineDir;
   if ( mIntersect( nearPlane, mWaterPlane, &linePnt, &lineDir ) )
   {
      Point3F leftPnt( centerPnt );
      leftPnt.x = near
   }   
   */

   Point2I resolution = GFX->getActiveRenderTarget()->getSize();
   F32 copyOffsetX = 1.0 / resolution.x;
   F32 copyOffsetY = 1.0 / resolution.y;

   /*
   ClippedPolyList polylist;
   polylist.addPoint( Point3F( -1.0f - copyOffsetX, -1.0f + copyOffsetY, 0.0f ) );
   polylist.addPoint( Point3F( -1.0f - copyOffsetX, 1.0f + copyOffsetY, 0.0f ) );
   polylist.addPoint( Point3F( 1.0f - copyOffsetX, 1.0f + copyOffsetY, 0.0f ) );
   polylist.addPoint( Point3F( 1.0f - copyOffsetX, -1.0f + copyOffsetY, 0.0f ) );
   polylist.addPlane( clipPlane );

   polylist.begin( NULL, 0 );
   polylist.vertex( 0 );
   polylist.vertex( 1 );
   polylist.vertex( 2 );
   polylist.vertex( 0 );
   polylist.vertex( 2 );
   polylist.vertex( 3 );
   */

   // draw quad
   

   GFXVertexBufferHandle<GFXVertexPCT> verts( GFX, 4, GFXBufferTypeVolatile );
   verts.lock();

   verts[0].point.set(1.0 - copyOffsetX, -1.0 + copyOffsetY, 0.0);
   verts[0].color = mUnderwaterColor;

   verts[1].point.set(1.0 - copyOffsetX, 1.0 + copyOffsetY, 0.0);
   verts[1].color = mUnderwaterColor;

   verts[2].point.set(-1.0 - copyOffsetX, -1.0 + copyOffsetY, 0.0);
   verts[2].color = mUnderwaterColor;

   verts[3].point.set(-1.0 - copyOffsetX, 1.0 + copyOffsetY, 0.0);
   verts[3].color = mUnderwaterColor;

   verts.unlock();

   GFX->setVertexBuffer( verts );
   GFX->drawPrimitive( GFXTriangleStrip, 0, 2 );

   // reset states / transforms
   GFX->setProjectionMatrix( proj );
   GFX->popWorldMatrix();
}

bool WaterObject::onAdd()
{
   if ( !Parent::onAdd() )
      return false;

   Con::NotifyDelegate clbk( this, &WaterObject::_onDisableTrueRelfections );   
   Con::addVariableNotify( "$pref::Water::disableTrueReflections", clbk );

   if ( isClientObject() )
   {
      GFXStateBlockDesc desc;
      desc.blendDefined = true;
      desc.blendEnable = true;
      desc.blendSrc = GFXBlendSrcAlpha;
      desc.blendDest = GFXBlendInvSrcAlpha;
      desc.zDefined = true;
      desc.zEnable = false;
      desc.cullDefined = true;
      desc.cullMode = GFXCullNone;
      mUnderwaterSB = GFX->createStateBlock( desc );

      initTextures();
      
      if ( mFullReflect && !smDisableTrueReflections )
         mPlaneReflector.registerReflector( this, &mReflectorDesc );
   }

   return true;
}

void WaterObject::onRemove()
{
   Con::NotifyDelegate clbk( this, &WaterObject::_onDisableTrueRelfections ); 
   Con::removeVariableNotify( "$pref::Water::disableTrueReflections", clbk );

   if ( isClientObject() )
   {
      mPlaneReflector.unregisterReflector();
      cleanupMaterials();

      PostEffect *underWaterEffect = getUnderwaterEffect( );
      if( underWaterEffect )
         underWaterEffect->disable( );
   }

   Parent::onRemove();
}

void WaterObject::_onDisableTrueRelfections()
{
   // Same code as _setFullReflect
   if ( isProperlyAdded() && isClientObject() )
   {
      bool isEnabled = mPlaneReflector.isEnabled();

      bool enable = mFullReflect && !smDisableTrueReflections;

      if ( enable && !isEnabled )
         mPlaneReflector.registerReflector( this, &mReflectorDesc );
      else if ( !enable && isEnabled )
         mPlaneReflector.unregisterReflector();
   }
}

void WaterObject::setShaderParams( SceneRenderState *state, BaseMatInstance *mat, const WaterMatParams &paramHandles )
{
   MaterialParameters* matParams = mat->getMaterialParameters();

   matParams->setSafe( paramHandles.mElapsedTimeSC, (F32)Sim::getCurrentTime() / 1000.0f );
   
   // set vertex shader constants
   //-----------------------------------   
   
   Point2F reflectTexSize( mPlaneReflector.reflectTex.getWidth(), mPlaneReflector.reflectTex.getHeight() );
   matParams->setSafe( paramHandles.mReflectTexSizeSC, reflectTexSize );

   static AlignedArray<Point2F> mConstArray( MAX_WAVES, sizeof( Point4F ) );   

   // Ripples...

   for ( U32 i = 0; i < MAX_WAVES; i++ )
      mConstArray[i].set( -mRippleDir[i].x, -mRippleDir[i].y );
   matParams->setSafe( paramHandles.mRippleDirSC, mConstArray );

   Point3F rippleSpeed( mRippleSpeed[0], mRippleSpeed[1], mRippleSpeed[2] );        
   matParams->setSafe( paramHandles.mRippleSpeedSC, rippleSpeed );

   Point4F rippleMagnitude( mRippleMagnitude[0], 
                            mRippleMagnitude[1], 
                            mRippleMagnitude[2],
                            mOverallRippleMagnitude );
   matParams->setSafe( paramHandles.mRippleMagnitudeSC, rippleMagnitude );

   for ( U32 i = 0; i < MAX_WAVES; i++ )
   {
      Point2F texScale = mRippleTexScale[i];
      if ( texScale.x > 0.0 )
         texScale.x = 1.0 / texScale.x;
      if ( texScale.y > 0.0 )
         texScale.y = 1.0 / texScale.y;

      mConstArray[i].set( texScale.x, texScale.y );
   }
   matParams->setSafe(paramHandles.mRippleTexScaleSC, mConstArray);

   static AlignedArray<Point4F> mConstArray4F( 3, sizeof( Point4F ) );

   F32 angle, cosine, sine;

   for ( U32 i = 0; i < MAX_WAVES; i++ )
   {
      angle = mAtan2( mRippleDir[i].x, -mRippleDir[i].y );
      cosine = mCos( angle );
      sine = mSin( angle );

      mConstArray4F[i].set( cosine, sine, -sine, cosine );
      matParams->setSafe( paramHandles.mRippleMatSC, mConstArray4F );
   }

   // Waves...

   for ( U32 i = 0; i < MAX_WAVES; i++ )
      mConstArray[i].set( -mWaveDir[i].x, -mWaveDir[i].y );
   matParams->setSafe( paramHandles.mWaveDirSC, mConstArray );

   for ( U32 i = 0; i < MAX_WAVES; i++ )
      mConstArray[i].set( mWaveSpeed[i], mWaveMagnitude[i] * mOverallWaveMagnitude );   
   matParams->setSafe( paramHandles.mWaveDataSC, mConstArray );   

   // Foam...

   Point4F foamDir( mFoamDir[0].x, mFoamDir[0].y, mFoamDir[1].x, mFoamDir[1].y );
   matParams->setSafe( paramHandles.mFoamDirSC, foamDir );

   Point2F foamSpeed( mFoamSpeed[0], mFoamSpeed[1] );        
   matParams->setSafe( paramHandles.mFoamSpeedSC, foamSpeed );

   //Point3F rippleMagnitude( mRippleMagnitude[0] * mOverallRippleMagnitude, 
   //                         mRippleMagnitude[1] * mOverallRippleMagnitude, 
   //                         mRippleMagnitude[2] * mOverallRippleMagnitude );
   //matParams->setSafe( paramHandles.mRippleMagnitudeSC, rippleMagnitude );

   Point4F foamTexScale( mFoamTexScale[0].x, mFoamTexScale[0].y, mFoamTexScale[1].x, mFoamTexScale[1].y );

   for ( U32 i = 0; i < 4; i++ )
   {
      if ( foamTexScale[i] > 0.0f )
         foamTexScale[i] = 1.0 / foamTexScale[i];      
   }

   matParams->setSafe(paramHandles.mFoamTexScaleSC, foamTexScale);

   // Other vert params...

   matParams->setSafe( paramHandles.mUndulateMaxDistSC, mUndulateMaxDist );

   // set pixel shader constants
   //-----------------------------------

   Point2F fogParams( mWaterFogData.density, mWaterFogData.densityOffset );
   matParams->setSafe(paramHandles.mFogParamsSC, fogParams );

   matParams->setSafe(paramHandles.mFarPlaneDistSC, (F32)state->getFarPlane() );

   Point2F wetnessParams( mWaterFogData.wetDepth, mWaterFogData.wetDarkening );
   matParams->setSafe(paramHandles.mWetnessParamsSC, wetnessParams );

   Point3F distortionParams( mDistortStartDist, mDistortEndDist, mDistortFullDepth );
   matParams->setSafe(paramHandles.mDistortionParamsSC, distortionParams );

   LightInfo *sun = LIGHTMGR->getSpecialLight(LightManager::slSunLightType);
   const ColorF &sunlight = state->getAmbientLightColor();
   Point3F ambientColor = mEmissive ? Point3F::One : sunlight;
   matParams->setSafe(paramHandles.mAmbientColorSC, ambientColor );
   matParams->setSafe(paramHandles.mLightDirSC, sun->getDirection() );

   Point4F foamParams( mOverallFoamOpacity, mFoamMaxDepth, mFoamAmbientLerp, mFoamRippleInfluence );
   matParams->setSafe(paramHandles.mFoamParamsSC, foamParams );   

   Point4F miscParams( mFresnelBias, mFresnelPower, mClarity, mMiscParamW );
   matParams->setSafe( paramHandles.mMiscParamsSC, miscParams );
   
   Point4F specularParams( mSpecularColor.red, mSpecularColor.green, mSpecularColor.blue, mSpecularPower );   
   if ( !mEmissive )
   {
      const ColorF &sunColor = sun->getColor();
      F32 brightness = sun->getBrightness();
      specularParams.x *= sunColor.red * brightness;
      specularParams.y *= sunColor.green * brightness;
      specularParams.z *= sunColor.blue * brightness;
   }
   matParams->setSafe( paramHandles.mSpecularParamsSC, specularParams );

   matParams->setSafe( paramHandles.mDepthGradMaxSC, mDepthGradientMax );

   matParams->setSafe( paramHandles.mReflectivitySC, mReflectivity );
}

PostEffect* WaterObject::getUnderwaterEffect()
{
   if ( mUnderwaterPostFx.isValid() )
      return mUnderwaterPostFx;
   
   PostEffect *effect;
   if ( Sim::findObject( "UnderwaterFogPostFx", effect ) )   
      mUnderwaterPostFx = effect;

   return mUnderwaterPostFx;   
}

void WaterObject::updateUnderwaterEffect( SceneRenderState *state )
{
   AssertFatal( isClientObject(), "uWaterObject::updateUnderwaterEffect() called on the server" );

   PostEffect *effect = getUnderwaterEffect();
   if ( !effect )
      return;

   // Never use underwater postFx with Basic Lighting, we don't have depth.
   if ( mBasicLighting )
   {
      effect->disable();
      return;
   }

   GameConnection *conn = GameConnection::getConnectionToServer();
   if ( !conn )
      return;

   GameBase *control = conn->getControlObject();
   if ( !control )
      return;

   WaterObject *water = control->getCurrentWaterObject();
   if ( water == NULL )
      effect->disable();

   else if ( water == this )
   {
      MatrixF mat;      
      conn->getControlCameraTransform( 0, &mat );
      
      if ( mUnderwater )
      {
         effect->enable();
         effect->setOnThisFrame( true );

         mWaterFogData.depthGradMax = mDepthGradientMax;
         state->getSceneManager()->setWaterFogData( mWaterFogData );

         // Register our depthGradient texture with a name so it can
         // be fetched by the effect when it renders.
         if ( !mNamedDepthGradTex.isRegistered() )
            mNamedDepthGradTex.registerWithName( "waterDepthGradMap" );
         mNamedDepthGradTex.setTexture( mDepthGradientTex );         
      }
      else
         effect->disable();
   }
}

bool WaterObject::initMaterial( S32 idx )
{
   // We must return false for any case which it is NOT safe for the caller
   // to use the indexed material.
   
   if ( idx < 0 || idx > NumMatTypes )
      return false;

   BaseMatInstance *mat = mMatInstances[idx];
   WaterMatParams &matParams = mMatParamHandles[idx];
   
   // Is it already initialized?

   if ( mat && mat->isValid() )
      return true;

   // Do we need to allocate anything?

   if ( mSurfMatName[idx].isNotEmpty() )
   {      
      if ( mat )
         SAFE_DELETE( mat );

      CustomMaterial *custMat;
      if ( Sim::findObject( mSurfMatName[idx], custMat ) && custMat->mShaderData )
         mat = custMat->createMatInstance();
      else
         mat = MATMGR->createMatInstance( mSurfMatName[idx] );

      const GFXVertexFormat *flags = getGFXVertexFormat<GFXVertexPCT>();

      if ( mat && mat->init( MATMGR->getDefaultFeatures(), flags ) )
      {      
         mMatInstances[idx] = mat;
         matParams.init( mat );         
         return true;
      }
            
      SAFE_DELETE( mat );      
   }

   return false;
}

void WaterObject::initTextures()
{
   if ( mRippleTexName.isNotEmpty() )
      mRippleTex.set( mRippleTexName, &GFXDefaultStaticDiffuseProfile, "WaterObject::mRippleTex" );
   if ( mRippleTex.isNull() )
      mRippleTex.set( GFXTextureManager::getWarningTexturePath(), &GFXDefaultStaticDiffuseProfile, "WaterObject::mRippleTex" );

   if ( mDepthGradientTexName.isNotEmpty() )
      mDepthGradientTex.set( mDepthGradientTexName, &GFXDefaultStaticDiffuseProfile, "WaterObject::mDepthGradientTex" );
   if ( mDepthGradientTex.isNull() )
      mDepthGradientTex.set( GFXTextureManager::getWarningTexturePath(), &GFXDefaultStaticDiffuseProfile, "WaterObject::mDepthGradientTex" );
   
   if ( mNamedDepthGradTex.isRegistered() )
      mNamedDepthGradTex.setTexture( mDepthGradientTex );

   if ( mFoamTexName.isNotEmpty() )
      mFoamTex.set( mFoamTexName, &GFXDefaultStaticDiffuseProfile, "WaterObject::mFoamTex" );
   if ( mFoamTex.isNull() )
      mFoamTex.set( GFXTextureManager::getWarningTexturePath(), &GFXDefaultStaticDiffuseProfile, "WaterObject::mFoamTex" );

   if ( mCubemapName.isNotEmpty() )
      Sim::findObject( mCubemapName, mCubemap );   
   if ( mCubemap )
      mCubemap->createMap();
}

void WaterObject::cleanupMaterials()
{
   for (U32 i = 0; i < NumMatTypes; i++)
      SAFE_DELETE(mMatInstances[i]);
}

S32 WaterObject::getMaterialIndex( const Point3F &camPos )
{
   bool underwater = isUnderwater( camPos );
   bool basicLighting = dStricmp( LIGHTMGR->getId(), "BLM" ) == 0;

   // set the material
   S32 matIdx = -1;
   if ( underwater )
   {
      if ( basicLighting )
         matIdx = BasicUnderWaterMat;
      else
         matIdx = UnderWaterMat;
   }
   else
   {
      if ( basicLighting )
         matIdx = BasicWaterMat;
      else
         matIdx = WaterMat;
   }

   return matIdx;
}