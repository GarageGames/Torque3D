
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// Arcane-FX for MIT Licensed Open Source version of Torque 3D from GarageGames
// Copyright (C) 2015 Faust Logic, Inc.
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
//
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#include "afx/arcaneFX.h"

#include "console/consoleTypes.h"
#include "core/stream/bitStream.h"

#include "afx/ce/afxSpotLight_T3D.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxT3DSpotLightData

IMPLEMENT_CO_DATABLOCK_V1(afxT3DSpotLightData);

ConsoleDocClass( afxT3DSpotLightData,
   "@brief A datablock that specifies a dynamic Spot Light effect.\n\n"

   "A Spot Light effect that uses the T3D SpotLight object. afxT3DSpotLightData has the same fields found in SpotLight but in "
   "a datablock structure."
   "\n\n"

   "@ingroup afxEffects\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxT3DSpotLightData::afxT3DSpotLightData()
   :  mRange( 10.0f ),
      mInnerConeAngle( 40.0f ),
      mOuterConeAngle( 45.0f )
{
}

afxT3DSpotLightData::afxT3DSpotLightData(const afxT3DSpotLightData& other, bool temp_clone) : afxT3DLightBaseData(other, temp_clone)
{
  mRange = other.mRange;
  mInnerConeAngle = other.mInnerConeAngle;
  mOuterConeAngle = other.mOuterConeAngle;
}

//
// NOTE: keep this as consistent as possible with PointLight::initPersistFields()
//
void afxT3DSpotLightData::initPersistFields()
{
   addGroup( "Light" );
      
      addField( "range", TypeF32, Offset( mRange, afxT3DSpotLightData ),
        "...");
      addField( "innerAngle", TypeF32, Offset( mInnerConeAngle, afxT3DSpotLightData ),
        "...");
      addField( "outerAngle", TypeF32, Offset( mOuterConeAngle, afxT3DSpotLightData ),
        "...");

   endGroup( "Light" );

   // We do the parent fields at the end so that
   // they show up that way in the inspector.
   Parent::initPersistFields();
}

bool afxT3DSpotLightData::onAdd()
{
  if (Parent::onAdd() == false)
    return false;

  return true;
}

void afxT3DSpotLightData::packData(BitStream* stream)
{
	Parent::packData(stream);

  stream->write( mRange );
  stream->write( mInnerConeAngle );
  stream->write( mOuterConeAngle );
}

void afxT3DSpotLightData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);

  stream->read( &mRange );
  stream->read( &mInnerConeAngle );
  stream->read( &mOuterConeAngle );
}


//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
