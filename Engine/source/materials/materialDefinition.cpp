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
#include "materials/materialDefinition.h"

#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "math/mathTypes.h"
#include "materials/materialManager.h"
#include "sceneData.h"
#include "gfx/sim/cubemapData.h"
#include "gfx/gfxCubemap.h"
#include "math/mathIO.h"
#include "materials/matInstance.h"
#include "sfx/sfxTrack.h"
#include "sfx/sfxTypes.h"
#include "core/util/safeDelete.h"
#include "T3D/accumulationVolume.h"


IMPLEMENT_CONOBJECT( Material );

ConsoleDocClass( Material,
	"@brief A material in Torque 3D is a data structure that describes a surface.\n\n"

	"It contains many different types of information for rendering properties. "
	"Torque 3D generates shaders from Material definitions. The shaders are compiled "
	"at runtime and output into the example/shaders directory. Any errors or warnings "
	"generated from compiling the procedurally generated shaders are output to the console "
	"as well as the output window in the Visual C IDE.\n\n"

	"@tsexample\n"
	"singleton Material(DECAL_scorch)\n"
	"{\n"
	"	baseTex[0] = \"./scorch_decal.png\";\n"
	"	vertColor[ 0 ] = true;\n\n"
	"	translucent = true;\n"
	"	translucentBlendOp = None;\n"
	"	translucentZWrite = true;\n"
	"	alphaTest = true;\n"
	"	alphaRef = 84;\n"
	"};\n"
	"@endtsexample\n\n"

	"@see Rendering\n"
	"@see ShaderData\n"

	"@ingroup GFX\n");

ImplementBitfieldType( MaterialAnimType,
   "The type of animation effect to apply to this material.\n"
   "@ingroup GFX\n\n")
   { Material::Scroll, "Scroll", "Scroll the material along the X/Y axis.\n" },
   { Material::Rotate, "Rotate" , "Rotate the material around a point.\n"},
   { Material::Wave, "Wave" , "Warps the material with an animation using Sin, Triangle or Square mathematics.\n"},
   { Material::Scale, "Scale", "Scales the material larger and smaller with a pulsing effect.\n" },
   { Material::Sequence, "Sequence", "Enables the material to have multiple frames of animation in its imagemap.\n" }
EndImplementBitfieldType;

ImplementEnumType( MaterialBlendOp,
   "The type of graphical blending operation to apply to this material\n"
   "@ingroup GFX\n\n")
   { Material::None,         "None", "Disable blending for this material." },
   { Material::Mul,          "Mul", "Multiplicative blending." },
   { Material::Add,          "Add", "Adds the color of the material to the frame buffer with full alpha for each pixel." },
   { Material::AddAlpha,     "AddAlpha", "The color is modulated by the alpha channel before being added to the frame buffer." },
   { Material::Sub,          "Sub", "Subtractive Blending. Reverses the color model, causing dark colors to have a stronger visual effect." },
   { Material::LerpAlpha,    "LerpAlpha", "Linearly interpolates between Material color and frame buffer color based on alpha." }
EndImplementEnumType;

ImplementEnumType( MaterialWaveType,
   "When using the Wave material animation, one of these Wave Types will be used to determine the type of wave to display.\n" 
   "@ingroup GFX\n")
   { Material::Sin,          "Sin", "Warps the material along a curved Sin Wave." },
   { Material::Triangle,     "Triangle", "Warps the material along a sharp Triangle Wave." },
   { Material::Square,       "Square", "Warps the material along a wave which transitions between two oppposite states. As a Square Wave, the transition is quick and sudden." },
EndImplementEnumType;


bool Material::sAllowTextureTargetAssignment = false;

GFXCubemap * Material::GetNormalizeCube()
{
   if(smNormalizeCube)
      return smNormalizeCube;
   smNormalizeCube = GFX->createCubemap();
   smNormalizeCube->initNormalize(64);
   return smNormalizeCube;
}

GFXCubemapHandle Material::smNormalizeCube;

Material::Material()
{
   for( U32 i=0; i<MAX_STAGES; i++ )
   {
      mDiffuse[i].set( 1.0f, 1.0f, 1.0f, 1.0f );
      mSpecular[i].set( 1.0f, 1.0f, 1.0f, 1.0f );

      mSpecularPower[i] = 8.0f;
      mSpecularStrength[i] = 1.0f;
      mPixelSpecular[i] = false;

      mAccuEnabled[i]   = false;
      mAccuScale[i]     = 1.0f;
      mAccuDirection[i] = 1.0f;
      mAccuStrength[i]  = 0.6f;
      mAccuCoverage[i]  = 0.9f;
      mAccuSpecular[i]  = 16.0f;
	  
      mParallaxScale[i] = 0.0f;

      mVertLit[i] = false;
      mVertColor[ i ] = false;

      mGlow[i] = false;
      mEmissive[i] = false;

      mDetailScale[i].set( 2.0f, 2.0f );
      
      mDetailNormalMapStrength[i] = 1.0f;

      mMinnaertConstant[i] = -1.0f;
      mSubSurface[i] = false;
      mSubSurfaceColor[i].set( 1.0f, 0.2f, 0.2f, 1.0f );
      mSubSurfaceRolloff[i] = 0.2f;
      
      mAnimFlags[i] = 0;

      mScrollDir[i].set( 0.0f, 0.0f );
      mScrollSpeed[i] = 0.0f;
      mScrollOffset[i].set( 0.0f, 0.0f );
      
      mRotSpeed[i] = 0.0f;
      mRotPivotOffset[i].set( 0.0f, 0.0f );
      mRotPos[i] = 0.0f;

      mWavePos[i] = 0.0f;
      mWaveFreq[i] = 0.0f;
      mWaveAmp[i] = 0.0f;
      mWaveType[i] = 0;

      mSeqFramePerSec[i] = 0.0f;
      mSeqSegSize[i] = 0.0f;

      // Deferred Shading
      mMatInfoFlags[i] = 0.0f;
   }

   dMemset(mCellIndex, 0, sizeof(mCellIndex));
   dMemset(mCellLayout, 0, sizeof(mCellLayout));
   dMemset(mCellSize, 0, sizeof(mCellSize));
   dMemset(mNormalMapAtlas, 0, sizeof(mNormalMapAtlas));
   dMemset(mUseAnisotropic, 0, sizeof(mUseAnisotropic));

   // Deferred Shading : Metalness
   dMemset(mUseMetalness, 0, sizeof(mUseMetalness));

   mImposterLimits = Point4F::Zero;

   mDoubleSided = false;

   mTranslucent = false;
   mTranslucentBlendOp = LerpAlpha;
   mTranslucentZWrite = false;

   mAlphaTest = false;
   mAlphaRef = 1;

   mCastShadows = true;
   mCastDynamicShadows = false;

   mPlanarReflection = false;

   mCubemapData = NULL;
   mDynamicCubemap = NULL;

   mLastUpdateTime = 0;

   mAutoGenerated = false;

   mShowDust = false;
   mShowFootprints = true;

   dMemset( mEffectColor,     0, sizeof( mEffectColor ) );

   mFootstepSoundId = -1;     mImpactSoundId = -1;
   mFootstepSoundCustom = 0;  mImpactSoundCustom = 0;
   mFriction = 0.0;
   
   mDirectSoundOcclusion = 1.f;
   mReverbSoundOcclusion = 1.0;
}

void Material::initPersistFields()
{
   addField("mapTo", TypeRealString, Offset(mMapTo, Material),
      "Used to map this material to the material name used by TSShape." );

   addArray( "Stages", MAX_STAGES );

      addField("diffuseColor", TypeColorF, Offset(mDiffuse, Material), MAX_STAGES,
         "This color is multiplied against the diffuse texture color.  If no diffuse texture "
         "is present this is the material color." );

      addField("diffuseMap", TypeImageFilename, Offset(mDiffuseMapFilename, Material), MAX_STAGES,
         "The diffuse color texture map." );

      addField("overlayMap", TypeImageFilename, Offset(mOverlayMapFilename, Material), MAX_STAGES,
         "A secondary diffuse color texture map which will use the second texcoord of a mesh." );

      addField("lightMap", TypeImageFilename, Offset(mLightMapFilename, Material), MAX_STAGES,
         "The lightmap texture used with pureLight." );

      addField("toneMap", TypeImageFilename, Offset(mToneMapFilename, Material), MAX_STAGES,
         "The tonemap texture used with pureLight.");

      addField("detailMap", TypeImageFilename, Offset(mDetailMapFilename, Material), MAX_STAGES,
         "A typically greyscale detail texture additively blended into the material." );

      addField("detailScale", TypePoint2F, Offset(mDetailScale, Material), MAX_STAGES,
         "The scale factor for the detail map." );

      addField( "normalMap", TypeImageFilename, Offset(mNormalMapFilename, Material), MAX_STAGES,
         "The normal map texture.  You can use the DXTnm format only when per-pixel "
         "specular highlights are disabled, or a specular map is in use." );

      addField( "detailNormalMap", TypeImageFilename, Offset(mDetailNormalMapFilename, Material), MAX_STAGES,
         "A second normal map texture applied at the detail scale.  You can use the DXTnm "
         "format only when per-pixel specular highlights are disabled." );

      addField( "detailNormalMapStrength", TypeF32, Offset(mDetailNormalMapStrength, Material), MAX_STAGES,
         "Used to scale the strength of the detail normal map when blended with the base normal map." );

      addField("specular", TypeColorF, Offset(mSpecular, Material), MAX_STAGES,
         "The color of the specular highlight when not using a specularMap." );

      addField("specularPower", TypeF32, Offset(mSpecularPower, Material), MAX_STAGES,
         "The hardness of the specular highlight when not using a specularMap." );

		addField("specularStrength", TypeF32, Offset(mSpecularStrength, Material), MAX_STAGES,
         "The strength of the specular highlight when not using a specularMap." );

      addField("pixelSpecular", TypeBool, Offset(mPixelSpecular, Material), MAX_STAGES, 
         "This enables per-pixel specular highlights controlled by the alpha channel of the "
         "normal map texture.  Note that if pixel specular is enabled the DXTnm format will not "
         "work with your normal map, unless you are also using a specular map." );

      addProtectedField( "accuEnabled", TYPEID< bool >(), Offset( mAccuEnabled, Material ),
            &_setAccuEnabled, &defaultProtectedGetFn, MAX_STAGES, "Accumulation texture." );

      addField("accuScale",      TypeF32, Offset(mAccuScale, Material), MAX_STAGES,
         "The scale that is applied to the accu map texture. You can use this to fit the texture to smaller or larger objects.");
		 
      addField("accuDirection",  TypeF32, Offset(mAccuDirection, Material), MAX_STAGES,
         "The direction of the accumulation. Chose whether you want the accu map to go from top to bottom (ie. snow) or upwards (ie. mold).");
		 
      addField("accuStrength",   TypeF32, Offset(mAccuStrength, Material), MAX_STAGES,
         "The strength of the accu map. This changes the transparency of the accu map texture. Make it subtle or add more contrast.");
		 
      addField("accuCoverage",   TypeF32, Offset(mAccuCoverage, Material), MAX_STAGES,
         "The coverage ratio of the accu map texture. Use this to make the entire shape pick up some of the accu map texture or none at all.");
		 
      addField("accuSpecular",   TypeF32, Offset(mAccuSpecular, Material), MAX_STAGES,
         "Changes specularity to this value where the accumulated material is present.");

      addField( "specularMap", TypeImageFilename, Offset(mSpecularMapFilename, Material), MAX_STAGES,
         "The specular map texture. The RGB channels of this texture provide a per-pixel replacement for the 'specular' parameter on the material. "
         "If this texture contains alpha information, the alpha channel of the texture will be used as the gloss map. "
         "This provides a per-pixel replacement for the 'specularPower' on the material" );

      addField( "parallaxScale", TypeF32, Offset(mParallaxScale, Material), MAX_STAGES,
         "Enables parallax mapping and defines the scale factor for the parallax effect.  Typically "
         "this value is less than 0.4 else the effect breaks down." );
      
      addField( "useAnisotropic", TypeBool, Offset(mUseAnisotropic, Material), MAX_STAGES,
         "Use anisotropic filtering for the textures of this stage." );
     
      addField("vertLit", TypeBool, Offset(mVertLit, Material), MAX_STAGES,
         "If true the vertex color is used for lighting." );

      addField( "vertColor", TypeBool, Offset( mVertColor, Material ), MAX_STAGES,
         "If enabled, vertex colors are premultiplied with diffuse colors." );

      addField("minnaertConstant", TypeF32, Offset(mMinnaertConstant, Material), MAX_STAGES,
         "The Minnaert shading constant value.  Must be greater than 0 to enable the effect." );

      addField("subSurface", TypeBool, Offset(mSubSurface, Material), MAX_STAGES,
         "Enables the subsurface scattering approximation." );

      addField("subSurfaceColor", TypeColorF, Offset(mSubSurfaceColor, Material), MAX_STAGES,
         "The color used for the subsurface scattering approximation." );

      addField("subSurfaceRolloff", TypeF32, Offset(mSubSurfaceRolloff, Material), MAX_STAGES,
         "The 0 to 1 rolloff factor used in the subsurface scattering approximation." );

      addField("glow", TypeBool, Offset(mGlow, Material), MAX_STAGES,
         "Enables rendering this material to the glow buffer." );

      addField("emissive", TypeBool, Offset(mEmissive, Material), MAX_STAGES,
         "Enables emissive lighting for the material." );

      addField("doubleSided", TypeBool, Offset(mDoubleSided, Material),
         "Disables backface culling casing surfaces to be double sided. "
         "Note that the lighting on the backside will be a mirror of the front "
         "side of the surface."  );

      addField("animFlags", TYPEID< AnimType >(), Offset(mAnimFlags, Material), MAX_STAGES,
         "The types of animation to play on this material." );

      addField("scrollDir", TypePoint2F, Offset(mScrollDir, Material), MAX_STAGES,
         "The scroll direction in UV space when scroll animation is enabled." );

      addField("scrollSpeed", TypeF32, Offset(mScrollSpeed, Material), MAX_STAGES,
         "The speed to scroll the texture in UVs per second when scroll animation is enabled." );

      addField("rotSpeed", TypeF32, Offset(mRotSpeed, Material), MAX_STAGES,
         "The speed to rotate the texture in degrees per second when rotation animation is enabled." );

      addField("rotPivotOffset", TypePoint2F, Offset(mRotPivotOffset, Material), MAX_STAGES,
         "The piviot position in UV coordinates to center the rotation animation." );

      addField("waveType", TYPEID< WaveType >(), Offset(mWaveType, Material), MAX_STAGES,
         "The type of wave animation to perform when wave animation is enabled." );

      addField("waveFreq", TypeF32, Offset(mWaveFreq, Material), MAX_STAGES,
         "The wave frequency when wave animation is enabled." );

      addField("waveAmp", TypeF32, Offset(mWaveAmp, Material), MAX_STAGES,
         "The wave amplitude when wave animation is enabled." );

      addField("sequenceFramePerSec", TypeF32, Offset(mSeqFramePerSec, Material), MAX_STAGES,
         "The number of frames per second for frame based sequence animations if greater than zero." );

      addField("sequenceSegmentSize", TypeF32, Offset(mSeqSegSize, Material), MAX_STAGES,
         "The size of each frame in UV units for sequence animations." );

      // Texture atlasing
      addField("cellIndex", TypePoint2I, Offset(mCellIndex, Material), MAX_STAGES,
         "@internal" );
      addField("cellLayout", TypePoint2I, Offset(mCellLayout, Material), MAX_STAGES,
         "@internal");
      addField("cellSize", TypeS32, Offset(mCellSize, Material), MAX_STAGES,
         "@internal");
      addField("bumpAtlas", TypeBool, Offset(mNormalMapAtlas, Material), MAX_STAGES,
         "@internal");

      // For backwards compatibility.  
      //
      // They point at the new 'map' fields, but reads always return
      // an empty string and writes only apply if the value is not empty.
      //
      addProtectedField("baseTex",        TypeImageFilename,   Offset(mDiffuseMapFilename, Material), 
         defaultProtectedSetNotEmptyFn, emptyStringProtectedGetFn, MAX_STAGES, 
         "For backwards compatibility.\n@see diffuseMap\n" ); 
      addProtectedField("detailTex",      TypeImageFilename,   Offset(mDetailMapFilename, Material), 
         defaultProtectedSetNotEmptyFn, emptyStringProtectedGetFn, MAX_STAGES, 
         "For backwards compatibility.\n@see detailMap\n"); 
      addProtectedField("overlayTex",     TypeImageFilename,   Offset(mOverlayMapFilename, Material),
         defaultProtectedSetNotEmptyFn, emptyStringProtectedGetFn, MAX_STAGES, 
         "For backwards compatibility.\n@see overlayMap\n"); 
      addProtectedField("bumpTex",        TypeImageFilename,   Offset(mNormalMapFilename, Material),
         defaultProtectedSetNotEmptyFn, emptyStringProtectedGetFn, MAX_STAGES, 
         "For backwards compatibility.\n@see normalMap\n"); 
      addProtectedField("colorMultiply",  TypeColorF,          Offset(mDiffuse, Material),
         defaultProtectedSetNotEmptyFn, emptyStringProtectedGetFn, MAX_STAGES,
         "For backwards compatibility.\n@see diffuseColor\n"); 

   endArray( "Stages" );

   addField( "castShadows", TypeBool, Offset(mCastShadows, Material),
      "If set to false the lighting system will not cast shadows from this material." );

   addField( "castDynamicShadows", TypeBool, Offset(mCastDynamicShadows, Material),
      "If set to false the lighting system will not cast dynamic shadows from this material." );

   addField("planarReflection", TypeBool, Offset(mPlanarReflection, Material), "@internal" );

   addField("translucent", TypeBool, Offset(mTranslucent, Material),
      "If true this material is translucent blended." );

   addField("translucentBlendOp", TYPEID< BlendOp >(), Offset(mTranslucentBlendOp, Material),
      "The type of blend operation to use when the material is translucent." );

   addField("translucentZWrite", TypeBool, Offset(mTranslucentZWrite, Material),
      "If enabled and the material is translucent it will write into the depth buffer." );

   addField("alphaTest", TypeBool, Offset(mAlphaTest, Material),
      "Enables alpha test when rendering the material.\n@see alphaRef\n" );

   addField("alphaRef", TypeS32, Offset(mAlphaRef, Material),
      "The alpha reference value for alpha testing.  Must be between 0 to 255.\n@see alphaTest\n" );

   addField("cubemap", TypeRealString, Offset(mCubemapName, Material),
      "The name of a CubemapData for environment mapping." );

   addField("dynamicCubemap", TypeBool, Offset(mDynamicCubemap, Material),
      "Enables the material to use the dynamic cubemap from the ShapeBase object its applied to." );

   addGroup( "Behavioral" );

      addField( "showFootprints", TypeBool, Offset( mShowFootprints, Material ),
         "Whether to show player footprint decals on this material.\n\n"
         "@see PlayerData::decalData" );
         
      addField( "showDust", TypeBool, Offset( mShowDust, Material ),
         "Whether to emit dust particles from a shape moving over the material.  This is, for example, used by "
         "vehicles or players to decide whether to show dust trails." );
         
      addField( "effectColor", TypeColorF, Offset( mEffectColor, Material ), NUM_EFFECT_COLOR_STAGES,
         "If #showDust is true, this is the set of colors to use for the ParticleData of the dust "
         "emitter.\n\n"
         "@see ParticleData::colors" );
         
      addField( "footstepSoundId", TypeS32, Offset( mFootstepSoundId, Material ),
         "What sound to play from the PlayerData sound list when the player walks over the material.  -1 (default) to not play any sound.\n"
         "\n"
         "The IDs are:\n\n"
         "- 0: PlayerData::FootSoftSound\n"
         "- 1: PlayerData::FootHardSound\n"
         "- 2: PlayerData::FootMetalSound\n"
         "- 3: PlayerData::FootSnowSound\n"
         "- 4: PlayerData::FootShallowSound\n"
         "- 5: PlayerData::FootWadingSound\n"
         "- 6: PlayerData::FootUnderwaterSound\n"
         "- 7: PlayerData::FootBubblesSound\n"
         "- 8: PlayerData::movingBubblesSound\n"
         "- 9: PlayerData::waterBreathSound\n"
         "- 10: PlayerData::impactSoftSound\n"
         "- 11: PlayerData::impactHardSound\n"
         "- 12: PlayerData::impactMetalSound\n"
         "- 13: PlayerData::impactSnowSound\n"
         "- 14: PlayerData::impactWaterEasy\n"
         "- 15: PlayerData::impactWaterMedium\n"
         "- 16: PlayerData::impactWaterHard\n"
         "- 17: PlayerData::exitingWater\n" );
         
      addField( "customFootstepSound", TypeSFXTrackName, Offset( mFootstepSoundCustom, Material ),
         "The sound to play when the player walks over the material.  If this is set, it overrides #footstepSoundId.  This field is "
         "useful for directly assigning custom footstep sounds to materials without having to rely on the PlayerData sound assignment.\n\n"
         "@warn Be aware that materials are client-side objects.  This means that the SFXTracks assigned to materials must be client-side, too." );
      addField( "impactSoundId", TypeS32, Offset( mImpactSoundId, Material ),
         "What sound to play from the PlayerData sound list when the player impacts on the surface with a velocity equal or greater "
         "than PlayerData::groundImpactMinSpeed.\n\n"
         "For a list of IDs, see #footstepSoundId" );
      addField( "customImpactSound", TypeSFXTrackName,    Offset( mImpactSoundCustom, Material ),
         "The sound to play when the player impacts on the surface with a velocity equal or greater than PlayerData::groundImpactMinSpeed.  "
         "If this is set, it overrides #impactSoundId.  This field is useful for directly assigning custom impact sounds to materials "
         "without having to rely on the PlayerData sound assignment.\n\n"
         "@warn Be aware that materials are client-side objects.  This means that the SFXTracks assigned to materials must be client-side, too." );
      
      //Deactivate these for the moment as they are not used.
      
      #if 0
      addField( "friction", TypeF32, Offset( mFriction, Material ) );
      addField( "directSoundOcclusion", TypeF32, Offset( mDirectSoundOcclusion, Material ) );
      addField( "reverbSoundOcclusion", TypeF32, Offset( mReverbSoundOcclusion, Material ) );
      #endif

   endGroup( "Behavioral" );

   Parent::initPersistFields();
}

bool Material::writeField( StringTableEntry fieldname, const char *value )
{   
   // Never allow the old field names to be written.
   if (  fieldname == StringTable->insert("baseTex") ||
         fieldname == StringTable->insert("detailTex") ||
         fieldname == StringTable->insert("overlayTex") ||
         fieldname == StringTable->insert("bumpTex") || 
         fieldname == StringTable->insert("envTex") ||
         fieldname == StringTable->insert("colorMultiply") )
      return false;

   return Parent::writeField( fieldname, value );
}

bool Material::onAdd()
{
   if (Parent::onAdd() == false)
      return false;

   mCubemapData = dynamic_cast<CubemapData*>(Sim::findObject( mCubemapName ) );

   if( mTranslucentBlendOp >= NumBlendTypes || mTranslucentBlendOp < 0 )
   {
      Con::errorf( "Invalid blend op in material: %s", getName() );
      mTranslucentBlendOp = LerpAlpha;
   }

   SimSet *matSet = MATMGR->getMaterialSet();
   if( matSet )
      matSet->addObject( (SimObject*)this );

   // save the current script path for texture lookup later
   const String  scriptFile = Con::getVariable("$Con::File");  // current script file - local materials.cs

   String::SizeType  slash = scriptFile.find( '/', scriptFile.length(), String::Right );
   if ( slash != String::NPos )
      mPath = scriptFile.substr( 0, slash + 1 );

   _mapMaterial();

   return true;
}

void Material::onRemove()
{
   smNormalizeCube = NULL;
   Parent::onRemove();
}

void Material::inspectPostApply()
{
   Parent::inspectPostApply();

   // Reload the material instances which 
   // use this material.
   if ( isProperlyAdded() )
      reload();
}


bool Material::isLightmapped() const
{
   bool ret = false;
   for( U32 i=0; i<MAX_STAGES; i++ )
      ret |= mLightMapFilename[i].isNotEmpty() || mToneMapFilename[i].isNotEmpty() || mVertLit[i];
   return ret;
}

void Material::updateTimeBasedParams()
{
   U32 lastTime = MATMGR->getLastUpdateTime();
   F32 dt = MATMGR->getDeltaTime();
   if (mLastUpdateTime != lastTime)
   {
      for (U32 i = 0; i < MAX_STAGES; i++)
      {
         mScrollOffset[i] += mScrollDir[i] * mScrollSpeed[i] * dt;
         mRotPos[i] += mRotSpeed[i] * dt;
         mWavePos[i] += mWaveFreq[i] * dt;
      }
      mLastUpdateTime = lastTime;
   }
}

void Material::_mapMaterial()
{
   if( String(getName()).isEmpty() )
   {
      Con::warnf( "[Material::mapMaterial] - Cannot map unnamed Material" );
      return;
   }

   // If mapTo not defined in script, try to use the base texture name instead
   if( mMapTo.isEmpty() )
   {
      if ( mDiffuseMapFilename[0].isEmpty() )
         return;

      else
      {
         // extract filename from base texture
         if ( mDiffuseMapFilename[0].isNotEmpty() )
         {
            U32 slashPos = mDiffuseMapFilename[0].find('/',0,String::Right);
            if (slashPos == String::NPos)
               // no '/' character, must be no path, just the filename
               mMapTo = mDiffuseMapFilename[0];
            else
               // use everything after the last slash
               mMapTo = mDiffuseMapFilename[0].substr(slashPos+1, mDiffuseMapFilename[0].length() - slashPos - 1);
         }
      }
   }

   // add mapping
   MATMGR->mapMaterial(mMapTo,getName());
}

BaseMatInstance* Material::createMatInstance()
{
   return new MatInstance(*this);
}

void Material::flush()
{
   MATMGR->flushInstance( this );
}

void Material::reload()
{
   MATMGR->reInitInstance( this );
}

void Material::StageData::getFeatureSet( FeatureSet *outFeatures ) const
{
   TextureTable::ConstIterator iter = mTextures.begin();
   for ( ; iter != mTextures.end(); iter++ )
   {
      if ( iter->value.isValid() )
         outFeatures->addFeature( *iter->key );
   }
}

DefineConsoleMethod( Material, flush, void, (),, 
   "Flushes all material instances that use this material." )
{
   object->flush();
}

DefineConsoleMethod( Material, reload, void, (),, 
   "Reloads all material instances that use this material." )
{
   object->reload();
}

DefineConsoleMethod( Material, dumpInstances, void, (),, 
   "Dumps a formatted list of the currently allocated material instances for this material to the console." )
{
   MATMGR->dumpMaterialInstances( object );
}

DefineConsoleMethod( Material, getAnimFlags, const char*, (U32 id), , "" )
{
   char * animFlags = Con::getReturnBuffer(512);

   if(object->mAnimFlags[ id ] & Material::Scroll)
   {
	   if(dStrcmp( animFlags, "" ) == 0)
	      dStrcpy( animFlags, "$Scroll" );
   }
   if(object->mAnimFlags[ id ] & Material::Rotate)
   {
	   if(dStrcmp( animFlags, "" ) == 0)
	      dStrcpy( animFlags, "$Rotate" );
	   else
			dStrcat( animFlags, " | $Rotate");
   }
   if(object->mAnimFlags[ id ] & Material::Wave)
   {
	   if(dStrcmp( animFlags, "" ) == 0)
	      dStrcpy( animFlags, "$Wave" );
	   else
			dStrcat( animFlags, " | $Wave");
   }
   if(object->mAnimFlags[ id ] & Material::Scale)
   {
	   if(dStrcmp( animFlags, "" ) == 0)
	      dStrcpy( animFlags, "$Scale" );
	   else
			dStrcat( animFlags, " | $Scale");
   }
   if(object->mAnimFlags[ id ] & Material::Sequence)
   {
	   if(dStrcmp( animFlags, "" ) == 0)
	      dStrcpy( animFlags, "$Sequence" );
	   else
			dStrcat( animFlags, " | $Sequence");
   }

	return animFlags;
}

DefineConsoleMethod(Material, getFilename, const char*, (),, "Get filename of material")
{
	SimObject *material = static_cast<SimObject *>(object);
   return material->getFilename();
}

DefineConsoleMethod( Material, isAutoGenerated, bool, (),, 
              "Returns true if this Material was automatically generated by MaterialList::mapMaterials()" )
{
   return object->isAutoGenerated();
}

DefineConsoleMethod( Material, setAutoGenerated, void, (bool isAutoGenerated), , 
              "setAutoGenerated(bool isAutoGenerated): Set whether or not the Material is autogenerated." )
{
   object->setAutoGenerated(isAutoGenerated);
}

// Accumulation
bool Material::_setAccuEnabled( void *object, const char *index, const char *data )
{
   Material* mat = reinterpret_cast< Material* >( object );

   if ( index )
   {
      U32 i = dAtoui(index);
      mat->mAccuEnabled[i] = dAtob(data);
      AccumulationVolume::refreshVolumes();
   }
   return true;
}