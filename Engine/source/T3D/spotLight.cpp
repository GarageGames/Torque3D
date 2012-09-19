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
#include "T3D/spotLight.h"

#include "console/consoleTypes.h"
#include "core/stream/bitStream.h"
#include "gfx/gfxDrawUtil.h"


IMPLEMENT_CO_NETOBJECT_V1( SpotLight );

ConsoleDocClass( SpotLight,
   "@brief Lighting object which emits conical light in a direction.\n\n"

   "SpotLight is one of the two types of lighting objects that can be added "
   "to a Torque 3D level, the other being PointLight. Unlike directional or "
   "point lights, the SpotLights emits lighting in a specific direction "
   "within a cone. The distance of the cone is controlled by the SpotLight::range "
   "variable.\n\n"

   "@tsexample\n"
   "// Declaration of a point light in script, or created by World Editor\n"
   "new SpotLight(SampleSpotLight)\n"
   "{\n"
   "   range = \"10\";\n"
   "   innerAngle = \"40\";\n"
   "   outerAngle = \"45\";\n"
   "   isEnabled = \"1\";\n"
   "   color = \"1 1 1 1\";\n"
   "   brightness = \"1\";\n"
   "   castShadows = \"0\";\n"
   "   priority = \"1\";\n"
   "   animate = \"1\";\n"
   "   animationPeriod = \"1\";\n"
   "   animationPhase = \"1\";\n"
   "   flareType = \"LightFlareExample0\";\n"
   "   flareScale = \"1\";\n"
   "   attenuationRatio = \"0 1 1\";\n"
   "   shadowType = \"Spot\";\n"
   "   texSize = \"512\";\n"
   "   overDarkFactor = \"2000 1000 500 100\";\n"
   "   shadowDistance = \"400\";\n"
   "   shadowSoftness = \"0.15\";\n"
   "   numSplits = \"1\";\n"
   "   logWeight = \"0.91\";\n"
   "   fadeStartDistance = \"0\";\n"
   "   lastSplitTerrainOnly = \"0\";\n"
   "   representedInLightmap = \"0\";\n"
   "   shadowDarkenColor = \"0 0 0 -1\";\n"
   "   includeLightmappedGeometryInShadow = \"0\";\n"
   "   position = \"-29.4362 -5.86289 5.58602\";\n"
   "   rotation = \"1 0 0 0\";\n"
   "};\n"
   "@endtsexample\n\n"

   "@see LightBase\n\n"
   "@see PointLight\n\n"
   "@ingroup Lighting\n"
);

SpotLight::SpotLight()
   :  mRange( 10.0f ),
      mInnerConeAngle( 40.0f ),
      mOuterConeAngle( 45.0f )
{
   // We set the type here to ensure the extended
   // parameter validation works when setting fields.
   mLight->setType( LightInfo::Spot );
}

SpotLight::~SpotLight()
{
}

void SpotLight::initPersistFields()
{
   addGroup( "Light" );
      
      addField( "range", TypeF32, Offset( mRange, SpotLight ) );
      addField( "innerAngle", TypeF32, Offset( mInnerConeAngle, SpotLight ) );
      addField( "outerAngle", TypeF32, Offset( mOuterConeAngle, SpotLight ) );

   endGroup( "Light" );

   // We do the parent fields at the end so that
   // they show up that way in the inspector.
   Parent::initPersistFields();

   // Remove the scale field... it's already 
   // defined by the range and angle.
   removeField( "scale" );
}

void SpotLight::_conformLights()
{
   mLight->setTransform( getTransform() );

   mRange = getMax( mRange, 0.05f );
   mLight->setRange( mRange );

   mLight->setColor( mColor );
   mLight->setBrightness( mBrightness );
   mLight->setCastShadows( mCastShadows );
   mLight->setPriority( mPriority );

   mOuterConeAngle = getMax( 0.01f, mOuterConeAngle );
   mInnerConeAngle = getMin( mInnerConeAngle, mOuterConeAngle );

   mLight->setInnerConeAngle( mInnerConeAngle );
   mLight->setOuterConeAngle( mOuterConeAngle );

   // Update the bounds and scale to fit our spotlight.
   F32 radius = mRange * mSin( mDegToRad( mOuterConeAngle ) * 0.5f );
   mObjBox.minExtents.set( -1, 0, -1 );
   mObjBox.maxExtents.set( 1, 1, 1 );
   mObjScale.set( radius, mRange, radius );

   // Skip our transform... it just dirties mask bits.
   Parent::setTransform( mObjToWorld );
}

U32 SpotLight::packUpdate(NetConnection *conn, U32 mask, BitStream *stream )
{
   if ( stream->writeFlag( mask & UpdateMask ) )
   {
      stream->write( mRange );
      stream->write( mInnerConeAngle );
      stream->write( mOuterConeAngle );
   }
   
   return Parent::packUpdate( conn, mask, stream );
}

void SpotLight::unpackUpdate( NetConnection *conn, BitStream *stream )
{
   if ( stream->readFlag() ) // UpdateMask
   {   
      stream->read( &mRange );
      stream->read( &mInnerConeAngle );
      stream->read( &mOuterConeAngle );
   }
   
   Parent::unpackUpdate( conn, stream );
}

void SpotLight::setScale( const VectorF &scale )
{
   // The y coord is the spotlight range.
   mRange = getMax( scale.y, 0.05f );

   // Use the average of the x and z to get a radius.  This
   // is the best method i've found to make the manipulation
   // from the WorldEditor gizmo to feel right.
   F32 radius = mClampF( ( scale.x + scale.z ) * 0.5f, 0.05f, mRange );
   mOuterConeAngle = mRadToDeg( mAsin( radius / mRange ) ) * 2.0f;

   // Make sure the inner angle is less than the outer.
   //
   // TODO: Maybe we should make the inner angle a scale
   // and not an absolute angle?
   //
   mInnerConeAngle = getMin( mInnerConeAngle, mOuterConeAngle );

   // We changed a bunch of our settings 
   // so notify the client.
   setMaskBits( UpdateMask );

   // Let the parent do the final scale.
   Parent::setScale( VectorF( radius, mRange, radius ) );
}

void SpotLight::_renderViz( SceneRenderState *state )
{
   GFXDrawUtil *draw = GFX->getDrawUtil();

   GFXStateBlockDesc desc;
   desc.setZReadWrite( true, false );
   desc.setCullMode( GFXCullNone );
   desc.setBlend( true );

   // Base the color on the light color.
   ColorI color( mColor );
   color.alpha = 16;

   F32 radius = mRange * mSin( mDegToRad( mOuterConeAngle * 0.5f ) );
   draw->drawCone(   desc, 
                     getPosition() + ( getTransform().getForwardVector() * mRange ),
                     getPosition(),
                     radius,
                     color );
}
