
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
#include "scene/sceneRenderState.h"
#include "math/mathIO.h"

#include "afx/afxChoreographer.h"
#include "afx/ce/afxCameraPuppet.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxCameraPuppetData

IMPLEMENT_CO_DATABLOCK_V1(afxCameraPuppetData);

ConsoleDocClass( afxCameraPuppetData,
   "@brief A datablock that specifies a Camera Puppet effect.\n\n"

   "A Camera Puppet effect is used to control the position and orientation of the camera using the AFX constraint system. "
   "Camera Puppet effects are useful for creating small cut-scenes and can add a lot of visual drama to a spell or effectron "
   "effect."
   "\n\n"

   "Effective use of Camera Puppet effects require a fairly advanced understanding of how Torque cameras work in a "
   "server-client context. Care must be taken to prevent client cameras from drifting too far out of sync from the server camera. "
   "Otherwise, obvious discontinuities in the motion will result when the Camera Puppet ends and control is restored to the "
   "server camera. Scoping problems can also result if a client camera is moved to a location that is inconsistent with the "
   "scene scoping done by the server camera."
   "\n\n"

   "Often it is useful to manage camera controlling in an isolated effectron rather than directly incorporated into a magic-spell. "
   "This way the camera controlling effectron can target the specific client associated with the spellcaster. The spellcasting "
   "player observes the spell in a dramatic cut-scene-like fashion while other players continue to observe from their own "
   "viewing locations."
   "\n\n"

   "@ingroup afxEffects\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxCameraPuppetData::afxCameraPuppetData()
{
  cam_spec = ST_NULLSTRING;
  networking = SERVER_AND_CLIENT;
}

afxCameraPuppetData::afxCameraPuppetData(const afxCameraPuppetData& other, bool temp_clone) : GameBaseData(other, temp_clone)
{
  cam_spec = other.cam_spec;
  networking = other.networking;
}

#define myOffset(field) Offset(field, afxCameraPuppetData)

void afxCameraPuppetData::initPersistFields()
{
  addField("cameraSpec",          TypeString,   myOffset(cam_spec),
    "This field is like the effect-wrapper fields for specifying constraint sources, "
    "but here it specifies a target for the camera-puppet effect.");
  addField("networking",          TypeS8,       myOffset(networking),
    "Specifies the networking model used for the camerapuppet effect. The effect can "
    "puppet just the server camera, just the client camera, or both.\n"
    "Possible values: $AFX::SERVER_ONLY, $AFX::CLIENT_ONLY, or $AFX::SERVER_AND_CLIENT.");

  // disallow some field substitutions
  disableFieldSubstitutions("cameraSpec");
  disableFieldSubstitutions("networking");

  Parent::initPersistFields();
}

bool afxCameraPuppetData::onAdd()
{
  if (Parent::onAdd() == false)
    return false;

  bool runs_on_s = ((networking & (SERVER_ONLY | SERVER_AND_CLIENT)) != 0);
  bool runs_on_c = ((networking & (CLIENT_ONLY | SERVER_AND_CLIENT)) != 0);
  cam_def.parseSpec(cam_spec, runs_on_s, runs_on_c);

  return true;
}

void afxCameraPuppetData::packData(BitStream* stream)
{
	Parent::packData(stream);

  stream->writeString(cam_spec);
  stream->write(networking);
}

void afxCameraPuppetData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);

  cam_spec = stream->readSTString();
  stream->read(&networking);
}

void afxCameraPuppetData::gather_cons_defs(Vector<afxConstraintDef>& defs)
{ 
  if (cam_def.isDefined())
    defs.push_back(cam_def);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//