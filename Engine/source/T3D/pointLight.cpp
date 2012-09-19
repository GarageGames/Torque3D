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
#include "T3D/pointLight.h"

#include "console/consoleTypes.h"
#include "core/stream/bitStream.h"
#include "gfx/gfxDrawUtil.h"


IMPLEMENT_CO_NETOBJECT_V1( PointLight );

ConsoleDocClass( PointLight,
   "@brief Lighting object that radiates light in all directions.\n\n"

   "PointLight is one of the two types of lighting objects that can be added "
   "to a Torque 3D level, the other being SpotLight. Unlike directional or conical light, "
   "the PointLight emits lighting in all directions. The attenuation is controlled "
   "by a single variable: LightObject::radius.\n\n"

   "@tsexample\n"
   "// Declaration of a point light in script, or created by World Editor\n"
   "new PointLight(CrystalLight)\n"
   "{\n"
   "   radius = \"10\";\n"
   "   isEnabled = \"1\";\n"
   "   color = \"1 0.905882 0 1\";\n"
   "   brightness = \"0.5\";\n"
   "   castShadows = \"1\";\n"
   "   priority = \"1\";\n"
   "   animate = \"1\";\n"
   "   animationType = \"SubtlePulseLightAnim\";\n"
   "   animationPeriod = \"3\";\n"
   "   animationPhase = \"3\";\n"
   "   flareScale = \"1\";\n"
   "   attenuationRatio = \"0 1 1\";\n"
   "   shadowType = \"DualParaboloidSinglePass\";\n"
   "   texSize = \"512\";\n"
   "   overDarkFactor = \"2000 1000 500 100\";\n"
   "   shadowDistance = \"400\";\n"
   "   shadowSoftness = \"0.15\";\n"
   "   numSplits = \"1\";\n"
   "   logWeight = \"0.91\";\n"
   "   fadeStartDistance = \"0\";\n"
   "   lastSplitTerrainOnly = \"0\";\n"
   "   splitFadeDistances = \"10 20 30 40\";\n"
   "   representedInLightmap = \"0\";\n"
   "   shadowDarkenColor = \"0 0 0 -1\";\n"
   "   includeLightmappedGeometryInShadow = \"1\";\n"
   "   position = \"-61.3866 1.69186 5.1464\";\n"
   "   rotation = \"1 0 0 0\";\n"
   "};\n"
   "@endtsexample\n\n"

   "@see LightBase\n\n"
   "@see SpotLight\n\n"
   "@ingroup Lighting\n"
);

PointLight::PointLight()
   : mRadius( 5.0f )
{
   // We set the type here to ensure the extended
   // parameter validation works when setting fields.
   mLight->setType( LightInfo::Point );
}

PointLight::~PointLight()
{
}

void PointLight::initPersistFields()
{
   addGroup( "Light" );

      addField( "radius", TypeF32, Offset( mRadius, PointLight ), "Controls the falloff of the light emission" );

   endGroup( "Light" );

   // We do the parent fields at the end so that
   // they show up that way in the inspector.
   Parent::initPersistFields();

   // Remove the scale field... it's already 
   // defined by the light radius.
   removeField( "scale" );
}

void PointLight::_conformLights()
{
   mLight->setTransform( getRenderTransform() );

   mLight->setRange( mRadius );

   mLight->setColor( mColor );

   mLight->setBrightness( mBrightness );
   mLight->setCastShadows( mCastShadows );
   mLight->setPriority( mPriority );

   // Update the bounds and scale to fit our light.
   mObjBox.minExtents.set( -1, -1, -1 );
   mObjBox.maxExtents.set( 1, 1, 1 );
   mObjScale.set( mRadius, mRadius, mRadius );

   // Skip our transform... it just dirties mask bits.
   Parent::setTransform( mObjToWorld );
}

U32 PointLight::packUpdate(NetConnection *conn, U32 mask, BitStream *stream )
{
   if ( stream->writeFlag( mask & UpdateMask ) )
      stream->write( mRadius );

   return Parent::packUpdate( conn, mask, stream );
}

void PointLight::unpackUpdate( NetConnection *conn, BitStream *stream )
{
   if ( stream->readFlag() ) // UpdateMask
      stream->read( &mRadius );

   Parent::unpackUpdate( conn, stream );
}

void PointLight::setScale( const VectorF &scale )
{
   // Use the average of the three coords.
   mRadius = ( scale.x + scale.y + scale.z ) / 3.0f;

   // We changed our settings so notify the client.
   setMaskBits( UpdateMask );

   // Let the parent do the final scale.
   Parent::setScale( VectorF( mRadius, mRadius, mRadius ) );
}

void PointLight::_renderViz( SceneRenderState *state )
{
   GFXDrawUtil *draw = GFX->getDrawUtil();

   GFXStateBlockDesc desc;
   desc.setZReadWrite( true, false );
   desc.setCullMode( GFXCullNone );
   desc.setBlend( true );

   // Base the sphere color on the light color.
   ColorI color( mColor );
   color.alpha = 16;

   draw->drawSphere( desc, mRadius, getPosition(), color );
}
