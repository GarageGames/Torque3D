
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

#include "afx/ce/afxCameraShake.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxCameraShakeData

IMPLEMENT_CO_DATABLOCK_V1(afxCameraShakeData);

ConsoleDocClass( afxCameraShakeData,
   "@brief A datablock that specifies a Camera Shake effect.\n\n"

   "Camera Shake internally utilizes the standard Torque CameraShake class to implement a shaken camera effect."
   "\n\n"

   "@ingroup afxEffects\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxCameraShakeData::afxCameraShakeData()
{
  camShakeFreq.set( 10.0, 10.0, 10.0 );
  camShakeAmp.set( 1.0, 1.0, 1.0 );
  camShakeRadius = 10.0;
  camShakeFalloff = 10.0;
}

afxCameraShakeData::afxCameraShakeData(const afxCameraShakeData& other, bool temp_clone) : GameBaseData(other, temp_clone)
{
  camShakeFreq = other.camShakeFreq;
  camShakeAmp = other.camShakeAmp;
  camShakeRadius = other.camShakeRadius;
  camShakeFalloff = other.camShakeFalloff;
}

#define myOffset(field) Offset(field, afxCameraShakeData)

void afxCameraShakeData::initPersistFields()
{
  addField("frequency", TypePoint3F,   Offset(camShakeFreq,       afxCameraShakeData),
    "The camera shake frequencies for all three axes: X, Y, Z.");
  addField("amplitude", TypePoint3F,   Offset(camShakeAmp,        afxCameraShakeData),
    "The camera shake amplitudes for all three axes: X, Y, Z.");
  addField("radius",    TypeF32,       Offset(camShakeRadius,     afxCameraShakeData),
    "Radius about the effect position in which shaking will be applied.");
  addField("falloff",   TypeF32,       Offset(camShakeFalloff,    afxCameraShakeData),
    "Magnitude by which shaking decreases over distance to radius.");

  Parent::initPersistFields();
}

bool afxCameraShakeData::onAdd()
{
  if (Parent::onAdd() == false)
    return false;

  return true;
}

void afxCameraShakeData::packData(BitStream* stream)
{
	Parent::packData(stream);

  stream->write(camShakeFreq.x);
  stream->write(camShakeFreq.y);
  stream->write(camShakeFreq.z);
  stream->write(camShakeAmp.x);
  stream->write(camShakeAmp.y);
  stream->write(camShakeAmp.z);
  stream->write(camShakeRadius);
  stream->write(camShakeFalloff);
}

void afxCameraShakeData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);

  stream->read(&camShakeFreq.x);
  stream->read(&camShakeFreq.y);
  stream->read(&camShakeFreq.z);
  stream->read(&camShakeAmp.x);
  stream->read(&camShakeAmp.y);
  stream->read(&camShakeAmp.z);
  stream->read(&camShakeRadius);
  stream->read(&camShakeFalloff);
}


//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
