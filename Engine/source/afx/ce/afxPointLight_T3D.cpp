
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

#include "afx/ce/afxPointLight_T3D.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxT3DPointLightData

IMPLEMENT_CO_DATABLOCK_V1(afxT3DPointLightData);

ConsoleDocClass( afxT3DPointLightData,
   "@brief A datablock that specifies a dynamic Point Light effect.\n\n"

   "A Point Light effect that uses the T3D PointLight object. afxT3DPointLightData has the same fields found in PointLight but "
   "in a datablock structure."
   "\n\n"

   "@ingroup afxEffects\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxT3DPointLightData::afxT3DPointLightData()
   : mRadius( 5.0f )
{
}

afxT3DPointLightData::afxT3DPointLightData(const afxT3DPointLightData& other, bool temp_clone) : afxT3DLightBaseData(other, temp_clone)
{
  mRadius = other.mRadius;
}

//
// NOTE: keep this as consistent as possible with PointLight::initPersistFields()
//
void afxT3DPointLightData::initPersistFields()
{
   addGroup( "Light" );
      
      addField( "radius", TypeF32, Offset( mRadius, afxT3DPointLightData ),
        "Controls the falloff of the light emission");

   endGroup( "Light" );

   // We do the parent fields at the end so that
   // they show up that way in the inspector.
   Parent::initPersistFields();

   // Remove the scale field... it's already 
   // defined by the light radius.
   removeField( "scale" );
}

bool afxT3DPointLightData::onAdd()
{
  if (Parent::onAdd() == false)
    return false;

  return true;
}

void afxT3DPointLightData::packData(BitStream* stream)
{
	Parent::packData(stream);

  stream->write( mRadius );
}

void afxT3DPointLightData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);

  stream->read( &mRadius );
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
