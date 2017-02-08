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

#include "T3D/shapeBase.h"
#include "core/resourceManager.h"
#include "core/stream/bitStream.h"
#include "ts/tsShapeInstance.h"
#include "console/consoleInternal.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "lighting/lightInfo.h"
#include "lighting/lightManager.h"
#include "T3D/fx/particleEmitter.h"
#include "T3D/projectile.h"
#include "T3D/gameBase/gameConnection.h"
#include "math/mathIO.h"
#include "T3D/debris.h"
#include "math/mathUtils.h"
#include "sim/netObject.h"
#include "sfx/sfxTrack.h"
#include "sfx/sfxSource.h"
#include "sfx/sfxSystem.h"
#include "sfx/sfxTypes.h"
#include "scene/sceneManager.h"
#include "core/stream/fileStream.h"
#include "T3D/fx/cameraFXMgr.h"

//----------------------------------------------------------------------------

ShapeBaseImageData* InvalidImagePtr = (ShapeBaseImageData*) 1;

ImplementEnumType( ShapeBaseImageLoadedState,
   "@brief The loaded state of this ShapeBaseImage.\n\n"
   "@ingroup gameObjects\n\n")
   { ShapeBaseImageData::StateData::IgnoreLoaded, "Ignore", "Ignore the loaded state.\n" },
   { ShapeBaseImageData::StateData::Loaded,       "Loaded", "ShapeBaseImage is loaded.\n" },
   { ShapeBaseImageData::StateData::NotLoaded,    "Empty", "ShapeBaseImage is not loaded.\n" },
EndImplementEnumType;

ImplementEnumType( ShapeBaseImageSpinState,
   "@brief How the spin animation should be played.\n\n"
   "@ingroup gameObjects\n\n")
   { ShapeBaseImageData::StateData::IgnoreSpin,"Ignore", "No changes to the spin sequence.\n" },
   { ShapeBaseImageData::StateData::NoSpin,    "Stop", "Stops the spin sequence at its current position\n" },
   { ShapeBaseImageData::StateData::SpinUp,    "SpinUp", "Increase spin sequence timeScale from 0 (on state entry) to 1 (after stateTimeoutValue seconds).\n" },
   { ShapeBaseImageData::StateData::SpinDown,  "SpinDown", "Decrease spin sequence timeScale from 1 (on state entry) to 0 (after stateTimeoutValue seconds).\n" },
   { ShapeBaseImageData::StateData::FullSpin,  "FullSpeed", "Resume the spin sequence playback at its current position with timeScale = 1.\n"},
EndImplementEnumType;

ImplementEnumType( ShapeBaseImageRecoilState,
   "@brief What kind of recoil this ShapeBaseImage should emit when fired.\n\n"
   "@ingroup gameObjects\n\n")
   { ShapeBaseImageData::StateData::NoRecoil,     "NoRecoil", "No recoil occurs.\n" },
   { ShapeBaseImageData::StateData::LightRecoil,  "LightRecoil", "A light recoil occurs.\n" },
   { ShapeBaseImageData::StateData::MediumRecoil, "MediumRecoil", "A medium recoil occurs.\n" },
   { ShapeBaseImageData::StateData::HeavyRecoil,  "HeavyRecoil", "A heavy recoil occurs.\n" },
EndImplementEnumType;

ImplementEnumType( ShapeBaseImageLightType,
   "@brief The type of light to attach to this ShapeBaseImage.\n\n"
   "@ingroup gameObjects\n\n")
	{ ShapeBaseImageData::NoLight,           "NoLight", "No light is attached.\n" },
   { ShapeBaseImageData::ConstantLight,     "ConstantLight", "A constant emitting light is attached.\n" },
   { ShapeBaseImageData::SpotLight,         "SpotLight", "A spotlight is attached.\n" },
   { ShapeBaseImageData::PulsingLight,      "PulsingLight", "A pusling light is attached.\n" },
   { ShapeBaseImageData::WeaponFireLight,   "WeaponFireLight", "Light emits when the weapon is fired, then dissipates.\n" }
EndImplementEnumType;

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

IMPLEMENT_CO_DATABLOCK_V1(ShapeBaseImageData);

ConsoleDocClass( ShapeBaseImageData,
   "@brief Represents geometry to be mounted to a ShapeBase object.\n\n"
   "@ingroup gameObjects\n"
);

IMPLEMENT_CALLBACK( ShapeBaseImageData, onMount, void, ( SceneObject* obj, S32 slot, F32 dt ), ( obj, slot, dt ),
   "@brief Called when the Image is first mounted to the object.\n\n"

   "@param obj object that this Image has been mounted to\n"
   "@param slot Image mount slot on the object\n"
   "@param dt time remaining in this Image update\n" );

IMPLEMENT_CALLBACK( ShapeBaseImageData, onUnmount, void, ( SceneObject* obj, S32 slot, F32 dt ), ( obj, slot, dt ),
   "@brief Called when the Image is unmounted from the object.\n\n"

   "@param obj object that this Image has been unmounted from\n"
   "@param slot Image mount slot on the object\n"
   "@param dt time remaining in this Image update\n" );

ShapeBaseImageData::StateData::StateData()
{
   name = 0;
   transition.loaded[0] = transition.loaded[1] = -1;
   transition.ammo[0] = transition.ammo[1] = -1;
   transition.target[0] = transition.target[1] = -1;
   transition.trigger[0] = transition.trigger[1] = -1;
   transition.altTrigger[0] = transition.altTrigger[1] = -1;
   transition.wet[0] = transition.wet[1] = -1;
   transition.motion[0] = transition.motion[1] = -1;
   transition.timeout = -1;
   waitForTimeout = true;
   timeoutValue = 0;
   fire = false;
   altFire = false;
   reload = false;
   energyDrain = 0;
   allowImageChange = true;
   loaded = IgnoreLoaded;
   spin = IgnoreSpin;
   recoil = NoRecoil;
   sound = 0;
   emitter = NULL;
   script = 0;
   ignoreLoadedForReady = false;
   
   ejectShell = false;
   scaleAnimation = false;
   scaleAnimationFP = false;
   sequenceTransitionIn = false;
   sequenceTransitionOut = false;
   sequenceNeverTransition = false;
   sequenceTransitionTime = 0;
   direction = false;
   emitterTime = 0.0f;

   for( U32 i=0; i<MaxShapes; ++i)
   {
      sequence[i] = -1;
      sequenceVis[i] = -1;
      flashSequence[i] = false;
      emitterNode[i] = -1;
   }
}

static ShapeBaseImageData::StateData gDefaultStateData;

//----------------------------------------------------------------------------

ShapeBaseImageData::ShapeBaseImageData()
{
   emap = false;

   mountPoint = 0;
   mountOffset.identity();
   eyeOffset.identity();
   correctMuzzleVector = true;
   correctMuzzleVectorTP = true;
   firstPerson = true;
   useFirstPersonShape = false;
   useEyeOffset = false;
   useEyeNode = false;
   mass = 0;

   usesEnergy = false;
   minEnergy = 2;
   accuFire = false;

   projectile = NULL;

   cloakable = true;

   lightType = ShapeBaseImageData::NoLight;
   lightColor.set(1.f,1.f,1.f,1.f);
   lightDuration = 1000;
   lightRadius = 10.f;
   lightBrightness = 1.0f;

   shapeName = "core/art/shapes/noshape.dts";
   shapeNameFP = "";
   imageAnimPrefix = "";
   imageAnimPrefixFP = "";
   fireState = -1;
   altFireState = -1;
   reloadState = -1;
   computeCRC = false;

   animateAllShapes = true;
   animateOnServer = false;

   scriptAnimTransitionTime = 0.25f;

   //
   for (S32 i = 0; i < MaxStates; i++) {
      stateName[i] = 0;

      stateTransitionLoaded[i] = 0;
      stateTransitionNotLoaded[i] = 0;
      stateTransitionAmmo[i] = 0;
      stateTransitionNoAmmo[i] = 0;
      stateTransitionTarget[i] = 0;
      stateTransitionNoTarget[i] = 0;
      stateTransitionWet[i] = 0;
      stateTransitionNotWet[i] = 0;
      stateTransitionMotion[i] = 0;
      stateTransitionNoMotion[i] = 0;
      stateTransitionTriggerUp[i] = 0;
      stateTransitionTriggerDown[i] = 0;
      stateTransitionAltTriggerUp[i] = 0;
      stateTransitionAltTriggerDown[i] = 0;
      stateTransitionTimeout[i] = 0;

      stateTransitionGeneric0In[i] = 0;
      stateTransitionGeneric0Out[i] = 0;
      stateTransitionGeneric1In[i] = 0;
      stateTransitionGeneric1Out[i] = 0;
      stateTransitionGeneric2In[i] = 0;
      stateTransitionGeneric2Out[i] = 0;
      stateTransitionGeneric3In[i] = 0;
      stateTransitionGeneric3Out[i] = 0;

      stateWaitForTimeout[i] = true;
      stateTimeoutValue[i] = 0;
      stateFire[i] = false;
      stateAlternateFire[i] = false;
      stateReload[i] = false;
      stateEjectShell[i] = false;
      stateEnergyDrain[i] = 0;
      stateAllowImageChange[i] = true;
      stateScaleAnimation[i] = true;
      stateScaleAnimationFP[i] = true;
      stateSequenceTransitionIn[i] = false;
      stateSequenceTransitionOut[i] = false;
      stateSequenceNeverTransition[i] = false;
      stateSequenceTransitionTime[i] = 0.25f;
      stateDirection[i] = true;
      stateLoaded[i] = StateData::IgnoreLoaded;
      stateSpin[i] = StateData::IgnoreSpin;
      stateRecoil[i] = StateData::NoRecoil;
      stateSequence[i] = 0;
      stateSequenceRandomFlash[i] = false;

      stateShapeSequence[i] = 0;
      stateScaleShapeSequence[i] = false;

      stateSound[i] = 0;
      stateScript[i] = 0;
      stateEmitter[i] = 0;
      stateEmitterTime[i] = 0;
      stateEmitterNode[i] = 0;
      stateIgnoreLoadedForReady[i] = false;
   }
   statesLoaded = false;

   maxConcurrentSounds = 0;

   useRemainderDT = false;

   casing = NULL;
   casingID = 0;
   shellExitDir.set( 1.0, 0.0, 1.0 );
   shellExitDir.normalize();
   shellExitVariance = 20.0;
   shellVelocity = 1.0;
   
   fireStateName = NULL;

   for(U32 i=0; i<MaxShapes; ++i)
   {
      mCRC[i] = U32_MAX;
      mountTransform[i].identity();
      retractNode[i] = -1;
      muzzleNode[i] = -1;
      ejectNode[i] = -1;
      emitterNode[i] = -1;
      eyeMountNode[i] = -1;
      eyeNode[i] = -1;
      spinSequence[i] = -1;
      ambientSequence[i] = -1;
      isAnimated[i] = false;
      hasFlash[i] = false;
      shapeIsValid[i] = false;
   }

   shakeCamera = false;
   camShakeFreq = Point3F::Zero;
   camShakeAmp = Point3F::Zero;
   camShakeDuration = 1.5f;
   camShakeRadius = 3.0f;
   camShakeFalloff = 10.0f;
}

ShapeBaseImageData::~ShapeBaseImageData()
{
}

bool ShapeBaseImageData::onAdd()
{
   if (!Parent::onAdd())
      return false;

   // Copy state data from the scripting arrays into the
   // state structure array. If we have state data already,
   // we are on the client and need to leave it alone.
   for (U32 i = 0; i < MaxStates; i++) {
      StateData& s = state[i];
      if (statesLoaded == false) {
         s.name = stateName[i];
         s.transition.loaded[0] = lookupState(stateTransitionNotLoaded[i]);
         s.transition.loaded[1] = lookupState(stateTransitionLoaded[i]);
         s.transition.ammo[0] = lookupState(stateTransitionNoAmmo[i]);
         s.transition.ammo[1] = lookupState(stateTransitionAmmo[i]);
         s.transition.target[0] = lookupState(stateTransitionNoTarget[i]);
         s.transition.target[1] = lookupState(stateTransitionTarget[i]);
         s.transition.wet[0] = lookupState(stateTransitionNotWet[i]);
         s.transition.wet[1] = lookupState(stateTransitionWet[i]);
         s.transition.motion[0] = lookupState(stateTransitionNoMotion[i]);
         s.transition.motion[1] = lookupState(stateTransitionMotion[i]);
         s.transition.trigger[0] = lookupState(stateTransitionTriggerUp[i]);
         s.transition.trigger[1] = lookupState(stateTransitionTriggerDown[i]);
         s.transition.altTrigger[0] = lookupState(stateTransitionAltTriggerUp[i]);
         s.transition.altTrigger[1] = lookupState(stateTransitionAltTriggerDown[i]);
         s.transition.timeout = lookupState(stateTransitionTimeout[i]);

         s.transition.genericTrigger[0][0] = lookupState(stateTransitionGeneric0Out[i]);
         s.transition.genericTrigger[0][1] = lookupState(stateTransitionGeneric0In[i]);
         s.transition.genericTrigger[1][0] = lookupState(stateTransitionGeneric1Out[i]);
         s.transition.genericTrigger[1][1] = lookupState(stateTransitionGeneric1In[i]);
         s.transition.genericTrigger[2][0] = lookupState(stateTransitionGeneric2Out[i]);
         s.transition.genericTrigger[2][1] = lookupState(stateTransitionGeneric2In[i]);
         s.transition.genericTrigger[3][0] = lookupState(stateTransitionGeneric3Out[i]);
         s.transition.genericTrigger[3][1] = lookupState(stateTransitionGeneric3In[i]);

         s.waitForTimeout = stateWaitForTimeout[i];
         s.timeoutValue = stateTimeoutValue[i];
         s.fire = stateFire[i];
         s.altFire = stateAlternateFire[i];
         s.reload = stateReload[i];
         s.ejectShell = stateEjectShell[i];
         s.energyDrain = stateEnergyDrain[i];
         s.allowImageChange = stateAllowImageChange[i];
         s.scaleAnimation = stateScaleAnimation[i];
         s.scaleAnimationFP = stateScaleAnimationFP[i];
         s.sequenceTransitionIn = stateSequenceTransitionIn[i];
         s.sequenceTransitionOut = stateSequenceTransitionOut[i];
         s.sequenceNeverTransition = stateSequenceNeverTransition[i];
         s.sequenceTransitionTime = stateSequenceTransitionTime[i];
         s.direction = stateDirection[i];
         s.loaded = stateLoaded[i];
         s.spin = stateSpin[i];
         s.recoil = stateRecoil[i];

         s.shapeSequence = stateShapeSequence[i];
         s.shapeSequenceScale = stateScaleShapeSequence[i];

         s.sound = stateSound[i];
         s.script = stateScript[i];
         s.emitter = stateEmitter[i];
         s.emitterTime = stateEmitterTime[i];

         // Resolved at load time
         for( U32 j=0; j<MaxShapes; ++j)
         {
            s.sequence[j]     = -1;    // Sequence is resolved in load
            s.sequenceVis[j]  = -1;    // Vis Sequence is resolved in load
            s.emitterNode[j]  = -1;    // Sequnce is resolved in load
         }
      }

      // The first state marked as "fire" is the state entered on the
      // client when it recieves a fire event.
      if (s.fire && fireState == -1)
         fireState = i;

      // The first state marked as "alternateFire" is the state entered on the
      // client when it recieves an alternate fire event.
      if (s.altFire && altFireState == -1)
         altFireState = i;

      // The first state marked as "reload" is the state entered on the
      // client when it recieves a reload event.
      if (s.reload && reloadState == -1)
         reloadState = i;
   }

   // Always preload images, this is needed to avoid problems with
   // resolving sequences before transmission to a client.
   return true;
}

bool ShapeBaseImageData::preload(bool server, String &errorStr)
{
   if (!Parent::preload(server, errorStr))
      return false;

   // Resolve objects transmitted from server
   if (!server) {
      if (projectile)
         if (Sim::findObject(SimObjectId((uintptr_t)projectile), projectile) == false)
            Con::errorf(ConsoleLogEntry::General, "Error, unable to load projectile for shapebaseimagedata");

      for (U32 i = 0; i < MaxStates; i++) {
         if (state[i].emitter)
            if (!Sim::findObject(SimObjectId((uintptr_t)state[i].emitter), state[i].emitter))
               Con::errorf(ConsoleLogEntry::General, "Error, unable to load emitter for image datablock");
               
         String str;
         if( !sfxResolve( &state[ i ].sound, str ) )
            Con::errorf( ConsoleLogEntry::General, str.c_str() );
      }
   }

   // Use the first person eye offset if it's set.
   useEyeOffset = !eyeOffset.isIdentity();

   // Go through each of the shapes
   for (U32 i=0; i<MaxShapes; ++i)
   {
      // Shape 0: Standard image shape
      // Shape 1: Optional first person image shape

      StringTableEntry name;
      if (i == FirstPersonImageShape)
      {
         if ((useEyeOffset || useEyeNode) && shapeNameFP && shapeNameFP[0])
         {
            // Make use of the first person shape
            useFirstPersonShape = true;
            name = shapeNameFP;
         }
         else
         {
            // Skip the first person shape
            continue;
         }
      }
      else
      {
         name = shapeName;
      }

      if (name && name[0]) {
         // Resolve shapename
         shape[i] = ResourceManager::get().load(name);
         if (!bool(shape[i])) {
            errorStr = String::ToString("Unable to load shape: %s", name);
            return false;
         }
         if(computeCRC)
         {
            Con::printf("Validation required for shape: %s", name);

            Torque::FS::FileNodeRef    fileRef = Torque::FS::GetFileNode(shape[i].getPath());

            if (!fileRef)
            {
               errorStr = String::ToString("ShapeBaseImageData: Couldn't load shape \"%s\"",name);
               return false;
            }

            if(server)
            {
               mCRC[i] = fileRef->getChecksum();
            }
            else if(mCRC[i] != fileRef->getChecksum())
            {
               errorStr = String::ToString("Shape \"%s\" does not match version on server.",name);
               return false;
            }
         }

         // Resolve nodes & build mount transform
         eyeMountNode[i] = shape[i]->findNode("eyeMount");
         eyeNode[i] = shape[i]->findNode("eye");
         if (eyeNode[i] == -1)
            eyeNode[i] = eyeMountNode[i];
         ejectNode[i] = shape[i]->findNode("ejectPoint");
         muzzleNode[i] = shape[i]->findNode("muzzlePoint");
         retractNode[i] = shape[i]->findNode("retractionPoint");
         mountTransform[i] = mountOffset;
         S32 node = shape[i]->findNode("mountPoint");
         if (node != -1) {
            MatrixF total(1);
            do {
               MatrixF nmat;
               QuatF q;
               TSTransform::setMatrix(shape[i]->defaultRotations[node].getQuatF(&q),shape[i]->defaultTranslations[node],&nmat);
               total.mul(nmat);
               node = shape[i]->nodes[node].parentIndex;
            }
            while(node != -1);
            total.inverse();
            mountTransform[i].mul(total);
         }

         // Resolve state sequence names & emitter nodes
         isAnimated[i] = false;
         hasFlash[i] = false;
         for (U32 j = 0; j < MaxStates; j++) {
            StateData& s = state[j];
            if (stateSequence[j] && stateSequence[j][0])
               s.sequence[i] = shape[i]->findSequence(stateSequence[j]);
            if (s.sequence[i] != -1)
            {
               // This state has an animation sequence
               isAnimated[i] = true;
            }

            if (stateSequence[j] && stateSequence[j][0] && stateSequenceRandomFlash[j]) {
               char bufferVis[128];
               dStrncpy(bufferVis, stateSequence[j], 100);
               dStrcat(bufferVis, "_vis");
               s.sequenceVis[i] = shape[i]->findSequence(bufferVis);
            }
            if (s.sequenceVis[i] != -1)
            {
               // This state has a flash animation sequence
               s.flashSequence[i] = true;
               hasFlash[i] = true;
            }

            s.ignoreLoadedForReady = stateIgnoreLoadedForReady[j];

            if (stateEmitterNode[j] && stateEmitterNode[j][0])
               s.emitterNode[i] = shape[i]->findNode(stateEmitterNode[j]);
            if (s.emitterNode[i] == -1)
               s.emitterNode[i] = muzzleNode[i];
         }

         ambientSequence[i] = shape[i]->findSequence("ambient");
         spinSequence[i] = shape[i]->findSequence("spin");

         shapeIsValid[i] = true;
      }
      else {
         errorStr = "Bad Datablock from server";
         return false;
      }
   }

   if( !casing && casingID != 0 )
   {
      if( !Sim::findObject( SimObjectId( casingID ), casing ) )
      {
         Con::errorf( ConsoleLogEntry::General, "ShapeBaseImageData::preload: Invalid packet, bad datablockId(casing): 0x%x", casingID );
      }
   }


   // Preload the shapes
   for( U32 i=0; i<MaxShapes; ++i)
   {
      if( shapeIsValid[i] )
      {
         TSShapeInstance* pDummy = new TSShapeInstance(shape[i], !server);
         delete pDummy;
      }
   }
   return true;
}

S32 ShapeBaseImageData::lookupState(const char* name)
{
   if (!name || !name[0])
      return -1;
   for (U32 i = 0; i < MaxStates; i++)
      if (stateName[i] && !dStricmp(name,stateName[i]))
         return i;
   Con::errorf(ConsoleLogEntry::General,"ShapeBaseImageData:: Could not resolve state \"%s\" for image \"%s\"",name,getName());
   return 0;
}

void ShapeBaseImageData::initPersistFields()
{
   addField( "emap", TypeBool, Offset(emap, ShapeBaseImageData),
      "@brief Whether to enable environment mapping on this Image.\n\n" );

   addField( "shapeFile", TypeShapeFilename, Offset(shapeName, ShapeBaseImageData),
      "@brief The DTS or DAE model to use for this Image.\n\n" );

   addField( "shapeFileFP", TypeShapeFilename, Offset(shapeNameFP, ShapeBaseImageData),
      "@brief The DTS or DAE model to use for this Image when in first person.\n\n"
      "This is an optional parameter that also requires either eyeOffset or useEyeNode "
      "to be set.  If none of these conditions is met then shapeFile will be used "
      "for all cases.\n\n"
      "Typically you set a first person image for a weapon that "
      "includes the player's arms attached to it for animating while firing, "
      "reloading, etc.  This is typical of many FPS games."
      "@see eyeOffset\n"
      "@see useEyeNode\n");

   addField( "imageAnimPrefix", TypeCaseString, Offset(imageAnimPrefix, ShapeBaseImageData),
      "@brief Passed along to the mounting shape to modify animation sequences played in third person. [optional]\n\n" );
   addField( "imageAnimPrefixFP", TypeCaseString, Offset(imageAnimPrefixFP, ShapeBaseImageData),
      "@brief Passed along to the mounting shape to modify animation sequences played in first person. [optional]\n\n" );

   addField( "animateAllShapes", TypeBool, Offset(animateAllShapes, ShapeBaseImageData),
      "@brief Indicates that all shapes should be animated in sync.\n\n"
      "When multiple shapes are defined for this image datablock, each of them are automatically "
      "animated in step with each other.  This allows for easy switching between between shapes "
      "when some other condition changes, such as going from first person to third person, and "
      "keeping their look consistent.  If you know that you'll never switch between shapes on the "
      "fly, such as players only being allowed in a first person view, then you could set this to "
      "false to save some calculations.\n\n"
      "There are other circumstances internal to the engine that determine that only the current shape "
      "should be animated rather than all defined shapes.  In those cases, this property is ignored.\n\n"
      "@note This property is only important if you have more than one shape defined, such as shapeFileFP.\n\n"
      "@see shapeFileFP\n");

   addField( "animateOnServer", TypeBool, Offset(animateOnServer, ShapeBaseImageData),
      "@brief Indicates that the image should be animated on the server.\n\n"
      "In most cases you'll want this set if you're using useEyeNode.  You may also want to "
      "set this if the muzzlePoint is animated while it shoots.  You can set this "
      "to false even if these previous cases are true if the image's shape is set "
      "up in the correct position and orientation in the 'root' pose and none of "
      "the nodes are animated at key times, such as the muzzlePoint essentially "
      "remaining at the same position at the start of the fire state (it could "
      "animate just fine after the projectile is away as the muzzle vector is only "
      "calculated at the start of the state).\n\n"
      "You'll also want to set this to true if you're animating the camera using the "
      "image's 'eye' node -- unless the movement is very subtle and doesn't need to "
      "be reflected on the server.\n\n"
      "@note Setting this to true causes up to four animation threads to be advanced on the server "
      "for each instance in use, although for most images only one or two are actually defined.\n\n"
      "@see useEyeNode\n");

   addField( "scriptAnimTransitionTime", TypeF32, Offset(scriptAnimTransitionTime, ShapeBaseImageData),
      "@brief The amount of time to transition between the previous sequence and new sequence when the script prefix has changed.\n\n"
      "When setImageScriptAnimPrefix() is used on a ShapeBase that has this image mounted, the image "
      "will attempt to switch to the new animation sequence based on the given script prefix.  This is "
      "the amount of time it takes to transition from the previously playing animation sequence to"
      "the new script prefix-based animation sequence.\n"
      "@see ShapeBase::setImageScriptAnimPrefix()");

   addField( "projectile", TYPEID< ProjectileData >(), Offset(projectile, ShapeBaseImageData),
      "@brief The projectile fired by this Image\n\n" );

   addField( "cloakable", TypeBool, Offset(cloakable, ShapeBaseImageData),
      "@brief Whether this Image can be cloaked.\n\n"
      "Currently unused." );

   addField( "mountPoint", TypeS32, Offset(mountPoint, ShapeBaseImageData),
      "@brief Mount node # to mount this Image to.\n\n"
      "This should correspond to a mount# node on the ShapeBase derived object we are mounting to." );

   addField( "offset", TypeMatrixPosition, Offset(mountOffset, ShapeBaseImageData),
      "@brief \"X Y Z\" translation offset from this Image's <i>mountPoint</i> node to "
      "attach to.\n\n"
      "Defaults to \"0 0 0\". ie. attach this Image's "
      "<i>mountPoint</i> node to the ShapeBase model's mount# node without any offset.\n"
      "@see rotation");

   addField( "rotation", TypeMatrixRotation, Offset(mountOffset, ShapeBaseImageData),
      "@brief \"X Y Z ANGLE\" rotation offset from this Image's <i>mountPoint</i> node "
      "to attach to.\n\n"
      "Defaults to \"0 0 0\". ie. attach this Image's "
      "<i>mountPoint</i> node to the ShapeBase model's mount# node without any additional rotation.\n"
      "@see offset");

   addField( "eyeOffset", TypeMatrixPosition, Offset(eyeOffset, ShapeBaseImageData),
      "@brief \"X Y Z\" translation offset from the ShapeBase model's eye node.\n\n"
      "When in first person view, this is the offset from the eye node to place the gun.  This "
      "gives the gun a fixed point in space, typical of a lot of FPS games.\n"
      "@see eyeRotation");

   addField( "eyeRotation", TypeMatrixRotation, Offset(eyeOffset, ShapeBaseImageData),
      "@brief \"X Y Z ANGLE\" rotation offset from the ShapeBase model's eye node.\n\n"
      "When in first person view, this is the rotation from the eye node to place the gun.\n"
      "@see eyeOffset");

   addField( "useEyeNode", TypeBool, Offset(useEyeNode, ShapeBaseImageData),
      "@brief Mount image using image's eyeMount node and place the camera at the image's eye node (or "
      "at the eyeMount node if the eye node is missing).\n\n"
      "When in first person view, if an 'eyeMount' node is present in the image's shape, this indicates "
      "that the image should mount eyeMount node to Player eye node for image placement.  The "
      "Player's camera should also mount to the image's eye node to inherit any animation (or the eyeMount "
      "node if the image doesn't have an eye node).\n\n"
      "@note Used instead of eyeOffset.\n\n"
      "@note Read about the animateOnServer field as you may want to set it to true if you're using useEyeNode.\n\n"
      "@see eyeOffset\n\n"
      "@see animateOnServer\n\n");

   addField( "correctMuzzleVector", TypeBool,  Offset(correctMuzzleVector, ShapeBaseImageData),
      "@brief Flag to adjust the aiming vector to the eye's LOS point when in 1st person view.\n\n"
      "@see ShapeBase::getMuzzleVector()" );

   addField( "correctMuzzleVectorTP", TypeBool,  Offset(correctMuzzleVectorTP, ShapeBaseImageData),
      "@brief Flag to adjust the aiming vector to the camera's LOS point when in 3rd person view.\n\n"
      "@see ShapeBase::getMuzzleVector()" );

   addField( "firstPerson", TypeBool, Offset(firstPerson, ShapeBaseImageData),
      "@brief Set to true to render the image in first person." );

   addField( "mass", TypeF32, Offset(mass, ShapeBaseImageData),
      "@brief Mass of this Image.\n\n"
      "This is added to the total mass of the ShapeBase object." );

   addField( "usesEnergy", TypeBool, Offset(usesEnergy,ShapeBaseImageData),
      "@brief Flag indicating whether this Image uses energy instead of ammo.  The energy level comes from the ShapeBase object we're mounted to.\n\n"
      "@see ShapeBase::setEnergyLevel()");

   addField( "minEnergy", TypeF32, Offset(minEnergy, ShapeBaseImageData),
      "@brief Minimum Image energy for it to be operable.\n\n"
      "@see usesEnergy");

   addField( "accuFire", TypeBool, Offset(accuFire, ShapeBaseImageData),
      "@brief Flag to control whether the Image's aim is automatically converged with "
      "the crosshair.\n\n"
      "Currently unused." );

   addField( "lightType", TYPEID< ShapeBaseImageData::LightType >(), Offset(lightType, ShapeBaseImageData),
      "@brief The type of light this Image emits.\n\n"
      "@see ShapeBaseImageLightType");

   addField( "lightColor", TypeColorF, Offset(lightColor, ShapeBaseImageData),
      "@brief The color of light this Image emits.\n\n"
      "@see lightType");

   addField( "lightDuration", TypeS32, Offset(lightDuration, ShapeBaseImageData),
      "@brief Duration in SimTime of Pulsing and WeaponFire type lights.\n\n"
      "@see lightType");

   addField( "lightRadius", TypeF32, Offset(lightRadius, ShapeBaseImageData),
      "@brief Radius of the light this Image emits.\n\n"
      "@see lightType");

   addField( "lightBrightness", TypeF32, Offset(lightBrightness, ShapeBaseImageData),
      "@brief Brightness of the light this Image emits.\n\n"
      "Only valid for WeaponFireLight."
      "@see lightType");

   addField( "shakeCamera", TypeBool, Offset(shakeCamera, ShapeBaseImageData),
      "@brief Flag indicating whether the camera should shake when this Image fires.\n\n" );

   addField( "camShakeFreq", TypePoint3F, Offset(camShakeFreq, ShapeBaseImageData),
      "@brief Frequency of the camera shaking effect.\n\n"
      "@see shakeCamera" );

   addField( "camShakeAmp", TypePoint3F, Offset(camShakeAmp, ShapeBaseImageData),
      "@brief Amplitude of the camera shaking effect.\n\n"
      "@see shakeCamera" );

   addField( "camShakeDuration", TypeF32, Offset(camShakeDuration, ShapeBaseImageData),
      "Duration (in seconds) to shake the camera." );

   addField( "camShakeRadius", TypeF32, Offset(camShakeRadius, ShapeBaseImageData),
      "Radial distance that a camera's position must be within relative to the "
      "center of the explosion to be shaken." );

   addField( "camShakeFalloff", TypeF32, Offset(camShakeFalloff, ShapeBaseImageData),
      "Falloff value for the camera shake." );

   addField( "casing", TYPEID< DebrisData >(), Offset(casing, ShapeBaseImageData),
      "@brief DebrisData datablock to use for ejected casings.\n\n"
      "@see stateEjectShell" );

   addField( "shellExitDir", TypePoint3F, Offset(shellExitDir, ShapeBaseImageData),
      "@brief Vector direction to eject shell casings.\n\n"
      "@see casing");

   addField( "shellExitVariance", TypeF32, Offset(shellExitVariance, ShapeBaseImageData),
      "@brief Variance (in degrees) from the shellExitDir vector to eject casings.\n\n"
      "@see shellExitDir");

   addField( "shellVelocity", TypeF32, Offset(shellVelocity, ShapeBaseImageData),
      "@brief Speed at which to eject casings.\n\n"
      "@see casing");

   // State arrays
   addArray( "States", MaxStates );

      addField( "stateName", TypeCaseString, Offset(stateName, ShapeBaseImageData), MaxStates,
         "Name of this state." );

      addField( "stateTransitionOnLoaded", TypeString, Offset(stateTransitionLoaded, ShapeBaseImageData), MaxStates,
         "Name of the state to transition to when the loaded state of the Image "
         "changes to 'Loaded'." );
      addField( "stateTransitionOnNotLoaded", TypeString, Offset(stateTransitionNotLoaded, ShapeBaseImageData), MaxStates,
         "Name of the state to transition to when the loaded state of the Image "
         "changes to 'Empty'." );
      addField( "stateTransitionOnAmmo", TypeString, Offset(stateTransitionAmmo, ShapeBaseImageData), MaxStates,
         "Name of the state to transition to when the ammo state of the Image "
         "changes to true." );
      addField( "stateTransitionOnNoAmmo", TypeString, Offset(stateTransitionNoAmmo, ShapeBaseImageData), MaxStates,
         "Name of the state to transition to when the ammo state of the Image "
         "changes to false." );
      addField( "stateTransitionOnTarget", TypeString, Offset(stateTransitionTarget, ShapeBaseImageData), MaxStates,
         "Name of the state to transition to when the Image gains a target." );
      addField( "stateTransitionOnNoTarget", TypeString, Offset(stateTransitionNoTarget, ShapeBaseImageData), MaxStates,
         "Name of the state to transition to when the Image loses a target." );
      addField( "stateTransitionOnWet", TypeString, Offset(stateTransitionWet, ShapeBaseImageData), MaxStates,
         "Name of the state to transition to when the Image enters the water." );
      addField( "stateTransitionOnNotWet", TypeString, Offset(stateTransitionNotWet, ShapeBaseImageData), MaxStates,
         "Name of the state to transition to when the Image exits the water." );
      addField( "stateTransitionOnMotion", TypeString, Offset(stateTransitionMotion, ShapeBaseImageData), MaxStates,
         "Name of the state to transition to when the Player moves." );
      addField( "stateTransitionOnNoMotion", TypeString, Offset(stateTransitionNoMotion, ShapeBaseImageData), MaxStates,
         "Name of the state to transition to when the Player stops moving." );
      addField( "stateTransitionOnTriggerUp", TypeString, Offset(stateTransitionTriggerUp, ShapeBaseImageData), MaxStates,
         "Name of the state to transition to when the trigger state of the Image "
         "changes to true (fire button down)." );
      addField( "stateTransitionOnTriggerDown", TypeString, Offset(stateTransitionTriggerDown, ShapeBaseImageData), MaxStates,
         "Name of the state to transition to when the trigger state of the Image "
         "changes to false (fire button released)." );
      addField( "stateTransitionOnAltTriggerUp", TypeString, Offset(stateTransitionAltTriggerUp, ShapeBaseImageData), MaxStates,
         "Name of the state to transition to when the alt trigger state of the "
         "Image changes to true (alt fire button down)." );
      addField( "stateTransitionOnAltTriggerDown", TypeString, Offset(stateTransitionAltTriggerDown, ShapeBaseImageData), MaxStates,
         "Name of the state to transition to when the alt trigger state of the "
         "Image changes to false (alt fire button up)." );
      addField( "stateTransitionOnTimeout", TypeString, Offset(stateTransitionTimeout, ShapeBaseImageData), MaxStates,
         "Name of the state to transition to when we have been in this state "
         "for stateTimeoutValue seconds." );

      addField( "stateTransitionGeneric0In", TypeString, Offset(stateTransitionGeneric0In, ShapeBaseImageData), MaxStates,
         "Name of the state to transition to when the generic trigger 0 state "
         "changes to true." );
      addField( "stateTransitionGeneric0Out", TypeString, Offset(stateTransitionGeneric0Out, ShapeBaseImageData), MaxStates,
         "Name of the state to transition to when the generic trigger 0 state "
         "changes to false." );
      addField( "stateTransitionGeneric1In", TypeString, Offset(stateTransitionGeneric1In, ShapeBaseImageData), MaxStates,
         "Name of the state to transition to when the generic trigger 1 state "
         "changes to true." );
      addField( "stateTransitionGeneric1Out", TypeString, Offset(stateTransitionGeneric1Out, ShapeBaseImageData), MaxStates,
         "Name of the state to transition to when the generic trigger 1 state "
         "changes to false." );
      addField( "stateTransitionGeneric2In", TypeString, Offset(stateTransitionGeneric2In, ShapeBaseImageData), MaxStates,
         "Name of the state to transition to when the generic trigger 2 state "
         "changes to true." );
      addField( "stateTransitionGeneric2Out", TypeString, Offset(stateTransitionGeneric2Out, ShapeBaseImageData), MaxStates,
         "Name of the state to transition to when the generic trigger 2 state "
         "changes to false." );
      addField( "stateTransitionGeneric3In", TypeString, Offset(stateTransitionGeneric3In, ShapeBaseImageData), MaxStates,
         "Name of the state to transition to when the generic trigger 3 state "
         "changes to true." );
      addField( "stateTransitionGeneric3Out", TypeString, Offset(stateTransitionGeneric3Out, ShapeBaseImageData), MaxStates,
         "Name of the state to transition to when the generic trigger 3 state "
         "changes to false." );

      addField( "stateTimeoutValue", TypeF32, Offset(stateTimeoutValue, ShapeBaseImageData), MaxStates,
         "Time in seconds to wait before transitioning to stateTransitionOnTimeout." );
      addField( "stateWaitForTimeout", TypeBool, Offset(stateWaitForTimeout, ShapeBaseImageData), MaxStates,
         "If false, this state ignores stateTimeoutValue and transitions "
         "immediately if other transition conditions are met." );
      addField( "stateFire", TypeBool, Offset(stateFire, ShapeBaseImageData), MaxStates,
         "The first state with this set to true is the state entered by the "
         "client when it receives the 'fire' event." );
      addField( "stateAlternateFire", TypeBool, Offset(stateAlternateFire, ShapeBaseImageData), MaxStates,
         "The first state with this set to true is the state entered by the "
         "client when it receives the 'altFire' event." );
      addField( "stateReload", TypeBool, Offset(stateReload, ShapeBaseImageData), MaxStates,
         "The first state with this set to true is the state entered by the "
         "client when it receives the 'reload' event." );
      addField( "stateEjectShell", TypeBool, Offset(stateEjectShell, ShapeBaseImageData), MaxStates,
         "If true, a shell casing will be ejected in this state." );
      addField( "stateEnergyDrain", TypeF32, Offset(stateEnergyDrain, ShapeBaseImageData), MaxStates,
         "@brief Amount of energy to subtract from the Image in this state.\n\n"
         "Energy is drained at stateEnergyDrain units/tick as long as we are in "
         "this state.\n"
         "@see usesEnergy");
      addField( "stateAllowImageChange", TypeBool, Offset(stateAllowImageChange, ShapeBaseImageData), MaxStates,
         "@brief If false, other Images will temporarily be blocked from mounting "
         "while the state machine is executing the tasks in this state.\n\n"
         "For instance, if we have a rocket launcher, the player shouldn't "
         "be able to switch out <i>while</i> firing. So, you'd set "
         "stateAllowImageChange to false in firing states, and true the rest "
         "of the time." );
      addField( "stateDirection", TypeBool, Offset(stateDirection, ShapeBaseImageData), MaxStates,
         "@brief Direction of the animation to play in this state.\n\n"
         "True is forward, false is backward." );
      addField( "stateLoadedFlag", TYPEID< ShapeBaseImageData::StateData::LoadedState >(), Offset(stateLoaded, ShapeBaseImageData), MaxStates,
         "@brief Set the loaded state of the Image.\n\n"
         "<ul><li>IgnoreLoaded: Don't change Image loaded state.</li>"
         "<li>Loaded: Set Image loaded state to true.</li>"
         "<li>NotLoaded: Set Image loaded state to false.</li></ul>\n"
         "@see ShapeBaseImageLoadedState");
      addField( "stateSpinThread", TYPEID< ShapeBaseImageData::StateData::SpinState >(), Offset(stateSpin, ShapeBaseImageData), MaxStates,
         "@brief Controls how fast the 'spin' animation sequence will be played in "
         "this state.\n\n"
         "<ul><li>Ignore: No change to the spin sequence.</li>"
         "<li>Stop: Stops the spin sequence at its current position.</li>"
         "<li>SpinUp: Increase spin sequence timeScale from 0 (on state entry) "
         "to 1 (after stateTimeoutValue seconds).</li>"
         "<li>SpinDown: Decrease spin sequence timeScale from 1 (on state entry) "
         "to 0 (after stateTimeoutValue seconds).</li>"
         "<li>FullSpeed: Resume the spin sequence playback at its current "
         "position with timeScale=1.</li></ul>\n"
         "@see ShapeBaseImageSpinState");
      addField( "stateRecoil", TYPEID< ShapeBaseImageData::StateData::RecoilState >(), Offset(stateRecoil, ShapeBaseImageData), MaxStates,
         "@brief Type of recoil sequence to play on the ShapeBase object on entry to "
         "this state.\n\n"
         "<ul><li>NoRecoil: Do not play a recoil sequence.</li>"
         "<li>LightRecoil: Play the light_recoil sequence.</li>"
         "<li>MediumRecoil: Play the medium_recoil sequence.</li>"
         "<li>HeavyRecoil: Play the heavy_recoil sequence.</li></ul>\n"
         "@see ShapeBaseImageRecoilState");
      addField( "stateSequence", TypeString, Offset(stateSequence, ShapeBaseImageData), MaxStates,
         "Name of the sequence to play on entry to this state." );
      addField( "stateSequenceRandomFlash", TypeBool, Offset(stateSequenceRandomFlash, ShapeBaseImageData), MaxStates,
         "@brief If true, the muzzle flash sequence will be played while in this state.\n\n"
         "The name of the muzzle flash sequence is the same as stateSequence, "
         "with \"_vis\" at the end." );
      addField( "stateScaleAnimation", TypeBool, Offset(stateScaleAnimation, ShapeBaseImageData), MaxStates,
         "If true, the timeScale of the stateSequence animation will be adjusted "
         "such that the sequence plays for stateTimeoutValue seconds. " );
      addField( "stateScaleAnimationFP", TypeBool, Offset(stateScaleAnimationFP, ShapeBaseImageData), MaxStates,
         "If true, the timeScale of the first person stateSequence animation will be adjusted "
         "such that the sequence plays for stateTimeoutValue seconds. " );
      addField( "stateSequenceTransitionIn", TypeBool, Offset(stateSequenceTransitionIn, ShapeBaseImageData), MaxStates,
         "Do we transition to the state's sequence when we enter the state?" );
      addField( "stateSequenceTransitionOut", TypeBool, Offset(stateSequenceTransitionOut, ShapeBaseImageData), MaxStates,
         "Do we transition to the new state's sequence when we leave the state?" );
      addField( "stateSequenceNeverTransition", TypeBool, Offset(stateSequenceNeverTransition, ShapeBaseImageData), MaxStates,
         "Never allow a transition to this sequence.  Often used for a fire sequence." );
      addField( "stateSequenceTransitionTime", TypeF32, Offset(stateSequenceTransitionTime, ShapeBaseImageData), MaxStates,
         "The time to transition in or out of a sequence." );

      addField( "stateShapeSequence", TypeString, Offset(stateShapeSequence, ShapeBaseImageData), MaxStates,
         "Name of the sequence that is played on the mounting shape." );
      addField( "stateScaleShapeSequence", TypeBool, Offset(stateScaleShapeSequence, ShapeBaseImageData), MaxStates,
         "Indicates if the sequence to be played on the mounting shape should be scaled to the length of the state." );

      addField( "stateSound", TypeSFXTrackName, Offset(stateSound, ShapeBaseImageData), MaxStates,
         "Sound to play on entry to this state." );
      addField( "stateScript", TypeCaseString, Offset(stateScript, ShapeBaseImageData), MaxStates,
         "@brief Method to execute on entering this state.\n\n"
         "Scoped to this image class name, then ShapeBaseImageData. The script "
         "callback function takes the same arguments as the onMount callback.\n"
         "@see onMount() for the same arguments as this callback.");

      addField( "stateEmitter", TYPEID< ParticleEmitterData >(), Offset(stateEmitter, ShapeBaseImageData), MaxStates,
         "@brief Emitter to generate particles in this state (from muzzle point or "
         "specified node).\n\n"
         "@see stateEmitterNode" );
      addField( "stateEmitterTime", TypeF32, Offset(stateEmitterTime, ShapeBaseImageData), MaxStates,
         "How long (in seconds) to emit particles on entry to this state." );
      addField( "stateEmitterNode", TypeString, Offset(stateEmitterNode, ShapeBaseImageData), MaxStates,
         "@brief Name of the node to emit particles from.\n\n"
         "@see stateEmitter" );
      addField( "stateIgnoreLoadedForReady", TypeBool, Offset(stateIgnoreLoadedForReady, ShapeBaseImageData), MaxStates,
         "@brief If set to true, and both ready and loaded transitions are true, the "
         "ready transition will be taken instead of the loaded transition.\n\n"
         "A state is 'ready' if pressing the fire trigger in that state would "
         "transition to the fire state." );

   endArray( "States" );

   addField( "computeCRC", TypeBool, Offset(computeCRC, ShapeBaseImageData),
      "If true, verify that the CRC of the client's Image matches the server's "
      "CRC for the Image when loaded by the client." );

   addField( "maxConcurrentSounds", TypeS32, Offset(maxConcurrentSounds, ShapeBaseImageData),
      "@brief Maximum number of sounds this Image can play at a time.\n\n"
      "Any value <= 0 indicates that it can play an infinite number of sounds." );

   addField( "useRemainderDT", TypeBool, Offset(useRemainderDT, ShapeBaseImageData), 
      "@brief If true, allow multiple timeout transitions to occur within a single "
      "tick (useful if states have a very small timeout).\n\n" );

   Parent::initPersistFields();
}

void ShapeBaseImageData::packData(BitStream* stream)
{
   Parent::packData(stream);

   if(stream->writeFlag(computeCRC))
   {
      for( U32 j=0; j<MaxShapes; ++j )
      {
         stream->write(mCRC[j]);
      }
   }

   stream->writeString(shapeName);        // shape 0 for normal use
   stream->writeString(shapeNameFP);      // shape 1 for first person use (optional)

   stream->writeString(imageAnimPrefix);
   stream->writeString(imageAnimPrefixFP);

   stream->write(mountPoint);
   if (!stream->writeFlag(mountOffset.isIdentity()))
      stream->writeAffineTransform(mountOffset);
   if (!stream->writeFlag(eyeOffset.isIdentity()))
      stream->writeAffineTransform(eyeOffset);

   stream->writeFlag(animateOnServer);

   stream->write(scriptAnimTransitionTime);

   stream->writeFlag(useEyeNode);

   stream->writeFlag(correctMuzzleVector);
   stream->writeFlag(correctMuzzleVectorTP);
   stream->writeFlag(firstPerson);
   stream->write(mass);
   stream->writeFlag(usesEnergy);
   stream->write(minEnergy);

   for( U32 j=0; j<MaxShapes; ++j)
   {
      stream->writeFlag(hasFlash[j]);
   }

   // Client doesn't need accuFire

   // Write the projectile datablock
   if (stream->writeFlag(projectile))
      stream->writeRangedU32(packed? SimObjectId((uintptr_t)projectile):
                             projectile->getId(),DataBlockObjectIdFirst,DataBlockObjectIdLast);

   stream->writeFlag(cloakable);
   stream->writeRangedU32(lightType, 0, NumLightTypes-1);
   if(lightType != NoLight)
   {
      stream->write(lightRadius);
      stream->write(lightDuration);
      stream->writeFloat(lightColor.red, 7);
      stream->writeFloat(lightColor.green, 7);
      stream->writeFloat(lightColor.blue, 7);
      stream->writeFloat(lightColor.alpha, 7);
      stream->write(lightBrightness);
   }

   if ( stream->writeFlag( shakeCamera ) )
   {      
      mathWrite( *stream, camShakeFreq );
      mathWrite( *stream, camShakeAmp );
      stream->write( camShakeDuration );
      stream->write( camShakeRadius );
      stream->write( camShakeFalloff );
   }

   mathWrite( *stream, shellExitDir );
   stream->write(shellExitVariance);
   stream->write(shellVelocity);

   if( stream->writeFlag( casing ) )
   {
      stream->writeRangedU32(packed? SimObjectId((uintptr_t)casing):
         casing->getId(),DataBlockObjectIdFirst,DataBlockObjectIdLast);
   }

   for (U32 i = 0; i < MaxStates; i++)
      if (stream->writeFlag(state[i].name && state[i].name[0])) {
         StateData& s = state[i];
         // States info not needed on the client:
         //    s.allowImageChange
         //    s.scriptNames
         // Transitions are inc. one to account for -1 values
         stream->writeString(state[i].name);

         stream->writeInt(s.transition.loaded[0]+1,NumStateBits);
         stream->writeInt(s.transition.loaded[1]+1,NumStateBits);
         stream->writeInt(s.transition.ammo[0]+1,NumStateBits);
         stream->writeInt(s.transition.ammo[1]+1,NumStateBits);
         stream->writeInt(s.transition.target[0]+1,NumStateBits);
         stream->writeInt(s.transition.target[1]+1,NumStateBits);
         stream->writeInt(s.transition.wet[0]+1,NumStateBits);
         stream->writeInt(s.transition.wet[1]+1,NumStateBits);
         stream->writeInt(s.transition.trigger[0]+1,NumStateBits);
         stream->writeInt(s.transition.trigger[1]+1,NumStateBits);
         stream->writeInt(s.transition.altTrigger[0]+1,NumStateBits);
         stream->writeInt(s.transition.altTrigger[1]+1,NumStateBits);
         stream->writeInt(s.transition.timeout+1,NumStateBits);

         // Most states don't make use of the motion transition.
         if (stream->writeFlag(s.transition.motion[0] != -1 || s.transition.motion[1] != -1))
         {
            // This state does
            stream->writeInt(s.transition.motion[0]+1,NumStateBits);
            stream->writeInt(s.transition.motion[1]+1,NumStateBits);
         }

         // Most states don't make use of the generic trigger transitions.  Don't transmit
         // if that is the case here.
         for (U32 j=0; j<MaxGenericTriggers; ++j)
         {
            if (stream->writeFlag(s.transition.genericTrigger[j][0] != -1 || s.transition.genericTrigger[j][1] != -1))
            {
               stream->writeInt(s.transition.genericTrigger[j][0]+1,NumStateBits);
               stream->writeInt(s.transition.genericTrigger[j][1]+1,NumStateBits);
            }
         }

         if(stream->writeFlag(s.timeoutValue != gDefaultStateData.timeoutValue))
            stream->write(s.timeoutValue);

         stream->writeFlag(s.waitForTimeout);
         stream->writeFlag(s.fire);
         stream->writeFlag(s.altFire);
         stream->writeFlag(s.reload);
         stream->writeFlag(s.ejectShell);
         stream->writeFlag(s.scaleAnimation);
         stream->writeFlag(s.scaleAnimationFP);
         stream->writeFlag(s.direction);

         stream->writeFlag(s.sequenceTransitionIn);
         stream->writeFlag(s.sequenceTransitionOut);
         stream->writeFlag(s.sequenceNeverTransition);
         if(stream->writeFlag(s.sequenceTransitionTime != gDefaultStateData.sequenceTransitionTime))
            stream->write(s.sequenceTransitionTime);

         stream->writeString(s.shapeSequence);
         stream->writeFlag(s.shapeSequenceScale);

         if(stream->writeFlag(s.energyDrain != gDefaultStateData.energyDrain))
            stream->write(s.energyDrain);

         stream->writeInt(s.loaded,StateData::NumLoadedBits);
         stream->writeInt(s.spin,StateData::NumSpinBits);
         stream->writeInt(s.recoil,StateData::NumRecoilBits);

         for( U32 j=0; j<MaxShapes; ++j )
         {
            if(stream->writeFlag(s.sequence[j] != gDefaultStateData.sequence[j]))
               stream->writeSignedInt(s.sequence[j], 16);

            if(stream->writeFlag(s.sequenceVis[j] != gDefaultStateData.sequenceVis[j]))
               stream->writeSignedInt(s.sequenceVis[j],16);

            stream->writeFlag(s.flashSequence[j]);
         }

         stream->writeFlag(s.ignoreLoadedForReady);

         if (stream->writeFlag(s.emitter))
         {
            stream->writeRangedU32(packed? SimObjectId((uintptr_t)s.emitter):
                                   s.emitter->getId(),DataBlockObjectIdFirst,DataBlockObjectIdLast);
            stream->write(s.emitterTime);

            for( U32 j=0; j<MaxShapes; ++j )
            {
               stream->write(s.emitterNode[j]);
            }
         }

         sfxWrite( stream, s.sound );
      }
   stream->write(maxConcurrentSounds);
   stream->writeFlag(useRemainderDT);
}

void ShapeBaseImageData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);
   computeCRC = stream->readFlag();
   if(computeCRC)
   {
      for( U32 j=0; j<MaxShapes; ++j )
      {
         stream->read(&mCRC[j]);
      }
   }

   shapeName = stream->readSTString();       // shape 0 for normal use
   shapeNameFP = stream->readSTString();     // shape 1 for first person use (optional)

   imageAnimPrefix = stream->readSTString();
   imageAnimPrefixFP = stream->readSTString();

   stream->read(&mountPoint);
   if (stream->readFlag())
      mountOffset.identity();
   else
      stream->readAffineTransform(&mountOffset);
   if (stream->readFlag())
      eyeOffset.identity();
   else
      stream->readAffineTransform(&eyeOffset);

   animateOnServer = stream->readFlag();

   stream->read(&scriptAnimTransitionTime);

   useEyeNode = stream->readFlag();

   correctMuzzleVector = stream->readFlag();
   correctMuzzleVectorTP = stream->readFlag();
   firstPerson = stream->readFlag();
   stream->read(&mass);
   usesEnergy = stream->readFlag();
   stream->read(&minEnergy);

   for( U32 j=0; j<MaxShapes; ++j )
   {
      hasFlash[j] = stream->readFlag();
   }

   projectile = (stream->readFlag() ?
                 (ProjectileData*)stream->readRangedU32(DataBlockObjectIdFirst,
                                                        DataBlockObjectIdLast) : 0);

   cloakable = stream->readFlag();
   lightType = stream->readRangedU32(0, NumLightTypes-1);
   if(lightType != NoLight)
   {
      stream->read(&lightRadius);
      stream->read(&lightDuration);
      lightColor.red = stream->readFloat(7);
      lightColor.green = stream->readFloat(7);
      lightColor.blue = stream->readFloat(7);
      lightColor.alpha = stream->readFloat(7);
      stream->read( &lightBrightness );
   }

   shakeCamera = stream->readFlag();
   if ( shakeCamera )
   {
      mathRead( *stream, &camShakeFreq );
      mathRead( *stream, &camShakeAmp );
      stream->read( &camShakeDuration );
      stream->read( &camShakeRadius );
      stream->read( &camShakeFalloff );
   }

   mathRead( *stream, &shellExitDir );
   stream->read(&shellExitVariance);
   stream->read(&shellVelocity);

   if(stream->readFlag())
   {
      casingID = stream->readRangedU32(DataBlockObjectIdFirst, DataBlockObjectIdLast);
   }

   for (U32 i = 0; i < MaxStates; i++) {
      if (stream->readFlag()) {
         StateData& s = state[i];
         // States info not needed on the client:
         //    s.allowImageChange
         //    s.scriptNames
         // Transitions are dec. one to restore -1 values
         s.name = stream->readSTString();

         s.transition.loaded[0] = stream->readInt(NumStateBits) - 1;
         s.transition.loaded[1] = stream->readInt(NumStateBits) - 1;
         s.transition.ammo[0] = stream->readInt(NumStateBits) - 1;
         s.transition.ammo[1] = stream->readInt(NumStateBits) - 1;
         s.transition.target[0] = stream->readInt(NumStateBits) - 1;
         s.transition.target[1] = stream->readInt(NumStateBits) - 1;
         s.transition.wet[0] = stream->readInt(NumStateBits) - 1;
         s.transition.wet[1] = stream->readInt(NumStateBits) - 1;
         s.transition.trigger[0] = stream->readInt(NumStateBits) - 1;
         s.transition.trigger[1] = stream->readInt(NumStateBits) - 1;
         s.transition.altTrigger[0] = stream->readInt(NumStateBits) - 1;
         s.transition.altTrigger[1] = stream->readInt(NumStateBits) - 1;
         s.transition.timeout = stream->readInt(NumStateBits) - 1;

         // Motion trigger
         if (stream->readFlag())
         {
            s.transition.motion[0] = stream->readInt(NumStateBits) - 1;
            s.transition.motion[1] = stream->readInt(NumStateBits) - 1;
         }
         else
         {
            s.transition.motion[0] = -1;
            s.transition.motion[1] = -1;
         }

         // Generic triggers
         for (U32 j=0; j<MaxGenericTriggers; ++j)
         {
            if (stream->readFlag())
            {
               s.transition.genericTrigger[j][0] = stream->readInt(NumStateBits) - 1;
               s.transition.genericTrigger[j][1] = stream->readInt(NumStateBits) - 1;
            }
            else
            {
               s.transition.genericTrigger[j][0] = -1;
               s.transition.genericTrigger[j][1] = -1;
            }
         }

         if(stream->readFlag())
            stream->read(&s.timeoutValue);
         else
            s.timeoutValue = gDefaultStateData.timeoutValue;

         s.waitForTimeout = stream->readFlag();
         s.fire = stream->readFlag();
         s.altFire = stream->readFlag();
         s.reload = stream->readFlag();
         s.ejectShell = stream->readFlag();
         s.scaleAnimation = stream->readFlag();
         s.scaleAnimationFP = stream->readFlag();
         s.direction = stream->readFlag();

         s.sequenceTransitionIn = stream->readFlag();
         s.sequenceTransitionOut = stream->readFlag();
         s.sequenceNeverTransition = stream->readFlag();
         if (stream->readFlag())
            stream->read(&s.sequenceTransitionTime);
         else
            s.sequenceTransitionTime = gDefaultStateData.sequenceTransitionTime;

         s.shapeSequence = stream->readSTString();
         s.shapeSequenceScale = stream->readFlag();

         if(stream->readFlag())
            stream->read(&s.energyDrain);
         else
            s.energyDrain = gDefaultStateData.energyDrain;

         s.loaded = (StateData::LoadedState)stream->readInt(StateData::NumLoadedBits);
         s.spin = (StateData::SpinState)stream->readInt(StateData::NumSpinBits);
         s.recoil = (StateData::RecoilState)stream->readInt(StateData::NumRecoilBits);

         for( U32 j=0; j<MaxShapes; ++j )
         {
            if(stream->readFlag())
               s.sequence[j] = stream->readSignedInt(16);
            else
               s.sequence[j] = gDefaultStateData.sequence[j];

            if(stream->readFlag())
               s.sequenceVis[j] = stream->readSignedInt(16);
            else
               s.sequenceVis[j] = gDefaultStateData.sequenceVis[j];

            s.flashSequence[j] = stream->readFlag();
         }

         s.ignoreLoadedForReady = stream->readFlag();

         if (stream->readFlag())
         {
            s.emitter = (ParticleEmitterData*) stream->readRangedU32(DataBlockObjectIdFirst,
                                                                     DataBlockObjectIdLast);
            stream->read(&s.emitterTime);

            for( U32 j=0; j<MaxShapes; ++j )
            {
               stream->read(&(s.emitterNode[j]));
            }
         }
         else
            s.emitter = 0;
            
         sfxRead( stream, &s.sound );
      }
   }
   
   stream->read(&maxConcurrentSounds);
   useRemainderDT = stream->readFlag();

   statesLoaded = true;
}

void ShapeBaseImageData::inspectPostApply()
{
   Parent::inspectPostApply();

   // This does not do a very good job of applying changes to states
   // which may have occured in the editor, but at least we can do this...
   useEyeOffset = !eyeOffset.isIdentity();   
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

ShapeBase::MountedImage::MountedImage()
{
   for (U32 i=0; i<ShapeBaseImageData::MaxShapes; ++i)
   {
      shapeInstance[i] = 0;
      ambientThread[i] = NULL;
      visThread[i] = NULL;
      animThread[i] = NULL;
      flashThread[i] = NULL;
      spinThread[i] = NULL;
   }

   doAnimateAllShapes = false;
   forceAnimateAllShapes = false;
   lastShapeIndex = 0;

   state = 0;
   dataBlock = 0;
   nextImage = InvalidImagePtr;
   delayTime = 0;
   ammo = false;
   target = false;
   triggerDown = false;
   altTriggerDown = false;
   loaded = false;
   fireCount = 0;
   altFireCount = 0;
   reloadCount = 0;
   wet = false;
   motion = false;
   lightStart = 0;
   lightInfo = NULL;

   for (U32 i=0; i<ShapeBaseImageData::MaxGenericTriggers; ++i)
   {
      genericTrigger[i] = false;
   }

   nextLoaded = false;
}

ShapeBase::MountedImage::~MountedImage()
{
   for (U32 i=0; i<ShapeBaseImageData::MaxShapes; ++i)
   {
      delete shapeInstance[i];
      shapeInstance[i] = 0;
   }

   // stop sound
   for(Vector<SFXSource*>::iterator i = mSoundSources.begin(); i != mSoundSources.end(); i++)  
   {  
      SFX_DELETE((*i));  
   }  
   mSoundSources.clear(); 

   for (S32 i = 0; i < MaxImageEmitters; i++)
      if (bool(emitter[i].emitter))
         emitter[i].emitter->deleteWhenEmpty();

   if ( lightInfo != NULL )
      delete lightInfo;
}

void ShapeBase::MountedImage::addSoundSource(SFXSource* source)
{
   if(source != NULL)
   {
      if(dataBlock->maxConcurrentSounds > 0 && mSoundSources.size() > dataBlock->maxConcurrentSounds)
      {
         SFX_DELETE(mSoundSources.first());
         mSoundSources.pop_front();
      }
      source->play();
      mSoundSources.push_back(source);
   }
}

void ShapeBase::MountedImage::updateSoundSources( const MatrixF &renderTransform )
{
   // Update all the sounds removing any ones that have stopped.
   for ( U32 i=0; i < mSoundSources.size(); )
   {
      SFXSource *source = mSoundSources[i];

      if ( source->isStopped() )
      {
         SFX_DELETE( source );
         mSoundSources.erase_fast( i );
         continue;
      }

      source->setTransform(renderTransform);
      i++;
   }
}

void ShapeBase::MountedImage::updateDoAnimateAllShapes(const ShapeBase* owner)
{
   doAnimateAllShapes = false;
   if (!dataBlock)
      return;

   // According to ShapeBase::isFirstPerson() the server is always in first person mode.
   // Therefore we don't need to animate any other shapes but the one that will be
   // used for first person.

   // Sometimes this is forced externally, so honour it.
   if (forceAnimateAllShapes)
   {
      doAnimateAllShapes = true;
      return;
   }

   if (owner->isClientObject())
   {
      // If this client object doesn't have a controlling client, then according to 
      // ShapeBase::isFirstPerson() it cannot ever be in first person mode.  So no need 
      // to animate any shapes beyond the current one. 
      if (!owner->getControllingClient()) 
      { 
         return; 
      }

      doAnimateAllShapes = dataBlock->animateAllShapes;
   }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Any item with an item image is selectable

bool ShapeBase::mountImage(ShapeBaseImageData* imageData,U32 imageSlot,bool loaded,NetStringHandle &skinNameHandle)
{
   AssertFatal(imageSlot<MaxMountedImages,"Out of range image slot");

   MountedImage& image = mMountedImageList[imageSlot];
   if (image.dataBlock) {
      if ((image.dataBlock == imageData) && (image.skinNameHandle == skinNameHandle)) {
         // Image already loaded
         image.nextImage = InvalidImagePtr;
         return true;
      }
   }
   //
   setImage(imageSlot,imageData,skinNameHandle,loaded);

   return true;
}

bool ShapeBase::unmountImage(U32 imageSlot)
{
   AssertFatal(imageSlot<MaxMountedImages,"Out of range image slot");

	bool returnValue = false;
   MountedImage& image = mMountedImageList[imageSlot];
   if (image.dataBlock)
   {
      NetStringHandle temp;
      setImage(imageSlot,0, temp);
      returnValue = true;
   }

   return returnValue;
}


//----------------------------------------------------------------------------

ShapeBaseImageData* ShapeBase::getMountedImage(U32 imageSlot)
{
   AssertFatal(imageSlot<MaxMountedImages,"Out of range image slot");

   return mMountedImageList[imageSlot].dataBlock;
}


ShapeBase::MountedImage* ShapeBase::getImageStruct(U32 imageSlot)
{
   return &mMountedImageList[imageSlot];
}


ShapeBaseImageData* ShapeBase::getPendingImage(U32 imageSlot)
{
   ShapeBaseImageData* data = mMountedImageList[imageSlot].nextImage;
   return (data == InvalidImagePtr)? 0: data;
}

bool ShapeBase::isImageFiring(U32 imageSlot)
{
   MountedImage& image = mMountedImageList[imageSlot];
   return image.dataBlock && image.state->fire;
}

bool ShapeBase::isImageAltFiring(U32 imageSlot)
{
   MountedImage& image = mMountedImageList[imageSlot];
   return image.dataBlock && image.state->altFire;
}

bool ShapeBase::isImageReloading(U32 imageSlot)
{
   MountedImage& image = mMountedImageList[imageSlot];
   return image.dataBlock && image.state->reload;
}

bool ShapeBase::isImageReady(U32 imageSlot,U32 ns,U32 depth)
{
   // Will pressing the trigger lead to a fire state?
   MountedImage& image = mMountedImageList[imageSlot];
   if (depth++ > 5 || !image.dataBlock)
      return false;
   ShapeBaseImageData::StateData& stateData = (ns == -1) ?
      *image.state : image.dataBlock->state[ns];
   if (stateData.fire)
      return true;

   // Try the transitions...
   if (stateData.ignoreLoadedForReady == true) {
      if ((ns = stateData.transition.loaded[true]) != -1)
         if (isImageReady(imageSlot,ns,depth))
            return true;
   } else {
      if ((ns = stateData.transition.loaded[image.loaded]) != -1)
         if (isImageReady(imageSlot,ns,depth))
            return true;
   }
   for (U32 i=0; i<ShapeBaseImageData::MaxGenericTriggers; ++i)
   {
      if ((ns = stateData.transition.genericTrigger[i][image.genericTrigger[i]]) != -1)
         if (isImageReady(imageSlot,ns,depth))
            return true;
   }
   if ((ns = stateData.transition.ammo[image.ammo]) != -1)
      if (isImageReady(imageSlot,ns,depth))
         return true;
   if ((ns = stateData.transition.target[image.target]) != -1)
      if (isImageReady(imageSlot,ns,depth))
         return true;
   if ((ns = stateData.transition.wet[image.wet]) != -1)
      if (isImageReady(imageSlot,ns,depth))
         return true;
   if ((ns = stateData.transition.motion[image.motion]) != -1)
      if (isImageReady(imageSlot,ns,depth))
         return true;
   if ((ns = stateData.transition.trigger[1]) != -1)
      if (isImageReady(imageSlot,ns,depth))
         return true;
   if ((ns = stateData.transition.altTrigger[1]) != -1)
      if (isImageReady(imageSlot,ns,depth))
         return true;
   if ((ns = stateData.transition.timeout) != -1)
      if (isImageReady(imageSlot,ns,depth))
         return true;
   return false;
}

bool ShapeBase::isImageMounted(ShapeBaseImageData* imageData)
{
   for (U32 i = 0; i < MaxMountedImages; i++)
      if (imageData == mMountedImageList[i].dataBlock)
         return true;
   return false;
}

S32 ShapeBase::getMountSlot(ShapeBaseImageData* imageData)
{
   for (U32 i = 0; i < MaxMountedImages; i++)
      if (imageData == mMountedImageList[i].dataBlock)
         return i;
   return -1;
}

NetStringHandle ShapeBase::getImageSkinTag(U32 imageSlot)
{
   MountedImage& image = mMountedImageList[imageSlot];
   return image.dataBlock? image.skinNameHandle : NetStringHandle();
}

const char* ShapeBase::getImageState(U32 imageSlot)
{
   MountedImage& image = mMountedImageList[imageSlot];
   return image.dataBlock? image.state->name: 0;
}

void ShapeBase::setImageGenericTriggerState(U32 imageSlot, U32 trigger, bool state)
{
   MountedImage& image = mMountedImageList[imageSlot];
   if (image.dataBlock && image.genericTrigger[trigger] != state) {
      setMaskBits(ImageMaskN << imageSlot);
      image.genericTrigger[trigger] = state;
   }
}

bool ShapeBase::getImageGenericTriggerState(U32 imageSlot, U32 trigger)
{
   MountedImage& image = mMountedImageList[imageSlot];
   if (!image.dataBlock)
      return false;
   return image.genericTrigger[trigger];
}

void ShapeBase::setImageAmmoState(U32 imageSlot,bool ammo)
{
   MountedImage& image = mMountedImageList[imageSlot];
   if (image.dataBlock && !image.dataBlock->usesEnergy && image.ammo != ammo) {
      setMaskBits(ImageMaskN << imageSlot);
      image.ammo = ammo;
   }
}

bool ShapeBase::getImageAmmoState(U32 imageSlot)
{
   MountedImage& image = mMountedImageList[imageSlot];
   if (!image.dataBlock)
      return false;
   return image.ammo;
}

void ShapeBase::setImageWetState(U32 imageSlot,bool wet)
{
   MountedImage& image = mMountedImageList[imageSlot];
   if (image.dataBlock && image.wet != wet) {
      setMaskBits(ImageMaskN << imageSlot);
      image.wet = wet;
   }
}

bool ShapeBase::getImageWetState(U32 imageSlot)
{
   MountedImage& image = mMountedImageList[imageSlot];
   if (!image.dataBlock)
      return false;
   return image.wet;
}

void ShapeBase::setImageMotionState(U32 imageSlot,bool motion)
{
   MountedImage& image = mMountedImageList[imageSlot];
   if (image.dataBlock && image.motion != motion) {
      setMaskBits(ImageMaskN << imageSlot);
      image.motion = motion;
   }
}

bool ShapeBase::getImageMotionState(U32 imageSlot)
{
   MountedImage& image = mMountedImageList[imageSlot];
   if (!image.dataBlock)
      return false;
   return image.motion;
}

void ShapeBase::setImageTargetState(U32 imageSlot,bool target)
{
   MountedImage& image = mMountedImageList[imageSlot];
   if (image.dataBlock && image.target != target) {
      setMaskBits(ImageMaskN << imageSlot);
      image.target = target;
   }
}

bool ShapeBase::getImageTargetState(U32 imageSlot)
{
   MountedImage& image = mMountedImageList[imageSlot];
   if (!image.dataBlock)
      return false;
   return image.target;
}

void ShapeBase::setImageLoadedState(U32 imageSlot,bool loaded)
{
   MountedImage& image = mMountedImageList[imageSlot];
   if (image.dataBlock && image.loaded != loaded) {
      setMaskBits(ImageMaskN << imageSlot);
      image.loaded = loaded;
   }
}

bool ShapeBase::getImageLoadedState(U32 imageSlot)
{
   MountedImage& image = mMountedImageList[imageSlot];
   if (!image.dataBlock)
      return false;
   return image.loaded;
}

void ShapeBase::getMuzzleVector(U32 imageSlot,VectorF* vec)
{
   MatrixF mat;
   getMuzzleTransform(imageSlot,&mat);

   GameConnection * gc = getControllingClient();
   if (gc && !gc->isAIControlled())
   {
      MountedImage& image = mMountedImageList[imageSlot];

      bool fp = gc->isFirstPerson();
      if ((fp && image.dataBlock->correctMuzzleVector) ||
         (!fp && image.dataBlock->correctMuzzleVectorTP))
         if (getCorrectedAim(mat, vec))
            return;
   }

   mat.getColumn(1,vec);
}

void ShapeBase::getMuzzlePoint(U32 imageSlot,Point3F* pos)
{
   MatrixF mat;
   getMuzzleTransform(imageSlot,&mat);
   mat.getColumn(3,pos);
}


void ShapeBase::getRenderMuzzleVector(U32 imageSlot,VectorF* vec)
{
   MatrixF mat;
   getRenderMuzzleTransform(imageSlot,&mat);

   GameConnection * gc = getControllingClient();
   if (gc && !gc->isAIControlled())
   {
      MountedImage& image = mMountedImageList[imageSlot];

      bool fp = gc->isFirstPerson();
      if ((fp && image.dataBlock->correctMuzzleVector) ||
         (!fp && image.dataBlock->correctMuzzleVectorTP))
         if (getCorrectedAim(mat, vec))
            return;
   }

   mat.getColumn(1,vec);
}

void ShapeBase::getRenderMuzzlePoint(U32 imageSlot,Point3F* pos)
{
   MatrixF mat;
   getRenderMuzzleTransform(imageSlot,&mat);
   mat.getColumn(3,pos);
}

//----------------------------------------------------------------------------

void ShapeBase::scriptCallback(U32 imageSlot,const char* function)
{
   MountedImage &image = mMountedImageList[imageSlot];

   char buff1[32];
   dSprintf( buff1, 32, "%d", imageSlot );

   char buff2[32];
   dSprintf( buff2, 32, "%f", image.dataBlock->useRemainderDT ? image.rDT : 0.0f );

   Con::executef( image.dataBlock, function, getIdString(), buff1, buff2 );
}


//----------------------------------------------------------------------------

void ShapeBase::getMountTransform( S32 index, const MatrixF &xfm, MatrixF *outMat )
{
   // Returns mount point to world space transform
   if ( index >= 0 && index < SceneObject::NumMountPoints) {
      S32 ni = mDataBlock->mountPointNode[index];
      if (ni != -1) {
         MatrixF mountTransform = mShapeInstance->mNodeTransforms[ni];
         mountTransform.mul( xfm );
         const Point3F& scale = getScale();

         // The position of the mount point needs to be scaled.
         Point3F position = mountTransform.getPosition();
         position.convolve( scale );
         mountTransform.setPosition( position );

         // Also we would like the object to be scaled to the model.
         outMat->mul(mObjToWorld, mountTransform);
         return;
      }
   }

   // Then let SceneObject handle it.
   Parent::getMountTransform( index, xfm, outMat );      
}

void ShapeBase::getImageTransform(U32 imageSlot,MatrixF* mat)
{
   // Image transform in world space
   MountedImage& image = mMountedImageList[imageSlot];
   if (image.dataBlock) {
      ShapeBaseImageData& data = *image.dataBlock;
      U32 shapeIndex = getImageShapeIndex(image);

      MatrixF nmat;
      if (data.useEyeNode && isFirstPerson() && data.eyeMountNode[shapeIndex] != -1) {
         // We need to animate, even on the server, to make sure the nodes are in the correct location.
         image.shapeInstance[shapeIndex]->animate();

         getEyeBaseTransform(&nmat, mDataBlock->mountedImagesBank);

         MatrixF mountTransform = image.shapeInstance[shapeIndex]->mNodeTransforms[data.eyeMountNode[shapeIndex]];

         mat->mul(nmat, mountTransform);
      }
      else if (data.useEyeOffset && isFirstPerson()) {
         getEyeTransform(&nmat);
         mat->mul(nmat,data.eyeOffset);
      }
      else {
         getMountTransform( image.dataBlock->mountPoint, MatrixF::Identity, &nmat );
         mat->mul(nmat,data.mountTransform[shapeIndex]);
      }
   }
   else
      *mat = mObjToWorld;
}

void ShapeBase::getImageTransform(U32 imageSlot,S32 node,MatrixF* mat)
{
   // Image transform in world space
   MountedImage& image = mMountedImageList[imageSlot];
   if (image.dataBlock)
   {
      if (node != -1)
      {
         ShapeBaseImageData& data = *image.dataBlock;
         U32 shapeIndex = getImageShapeIndex(image);

         MatrixF nmat = image.shapeInstance[shapeIndex]->mNodeTransforms[node];
         MatrixF mmat;

         if (data.useEyeNode && isFirstPerson() && data.eyeMountNode[shapeIndex] != -1)
         {
            // We need to animate, even on the server, to make sure the nodes are in the correct location.
            image.shapeInstance[shapeIndex]->animate();

            MatrixF emat;
            getEyeBaseTransform(&emat, mDataBlock->mountedImagesBank);

            MatrixF mountTransform = image.shapeInstance[shapeIndex]->mNodeTransforms[data.eyeMountNode[shapeIndex]];
            mountTransform.affineInverse();

            mmat.mul(emat, mountTransform);
         }
         else if (data.useEyeOffset && isFirstPerson())
         {
            MatrixF emat;
            getEyeTransform(&emat);
            mmat.mul(emat,data.eyeOffset);
         }
         else
         {
            MatrixF emat;
            getMountTransform( image.dataBlock->mountPoint, MatrixF::Identity, &emat );
            mmat.mul(emat,data.mountTransform[shapeIndex]);
         }

         mat->mul(mmat, nmat);
      }
      else
         getImageTransform(imageSlot,mat);
   }
   else
      *mat = mObjToWorld;
}

void ShapeBase::getImageTransform(U32 imageSlot,StringTableEntry nodeName,MatrixF* mat)
{
   getImageTransform( imageSlot, getNodeIndex( imageSlot, nodeName ), mat );
}

void ShapeBase::getMuzzleTransform(U32 imageSlot,MatrixF* mat)
{
   // Muzzle transform in world space
   MountedImage& image = mMountedImageList[imageSlot];
   if (image.dataBlock)
      getImageTransform(imageSlot,image.dataBlock->muzzleNode[getImageShapeIndex(image)],mat);
   else
      *mat = mObjToWorld;
}


//----------------------------------------------------------------------------

void ShapeBase::getRenderMountTransform( F32 delta, S32 mountPoint, const MatrixF &xfm, MatrixF *outMat )
{
   // Returns mount point to world space transform
   if ( mountPoint >= 0 && mountPoint < SceneObject::NumMountPoints) {
      S32 ni = mDataBlock->mountPointNode[mountPoint];
      if (ni != -1) {
         MatrixF mountTransform = mShapeInstance->mNodeTransforms[ni];
         mountTransform.mul( xfm );
         const Point3F& scale = getScale();

         // The position of the mount point needs to be scaled.
         Point3F position = mountTransform.getPosition();
         position.convolve( scale );
         mountTransform.setPosition( position );

         // Also we would like the object to be scaled to the model.
         mountTransform.scale( scale );
         outMat->mul(getRenderTransform(), mountTransform);
         return;
      }
   }

   // Then let SceneObject handle it.
   Parent::getRenderMountTransform( delta, mountPoint, xfm, outMat );   
}


void ShapeBase::getRenderImageTransform( U32 imageSlot, MatrixF* mat, bool noEyeOffset )
{
   // Image transform in world space
   MountedImage& image = mMountedImageList[imageSlot];
   if (image.dataBlock) 
   {
      ShapeBaseImageData& data = *image.dataBlock;
      U32 shapeIndex = getImageShapeIndex(image);

      MatrixF nmat;
      if ( data.useEyeNode && isFirstPerson() && data.eyeMountNode[shapeIndex] != -1 ) {
         getRenderEyeBaseTransform(&nmat, mDataBlock->mountedImagesBank);

         MatrixF mountTransform = image.shapeInstance[shapeIndex]->mNodeTransforms[data.eyeMountNode[shapeIndex]];

         mat->mul(nmat, mountTransform);
      }
      else if ( !noEyeOffset && data.useEyeOffset && isFirstPerson() ) 
      {
         getRenderEyeTransform(&nmat);
         mat->mul(nmat,data.eyeOffset);
      }
      else 
      {
         getRenderMountTransform( 0.0f, data.mountPoint, MatrixF::Identity, &nmat );
         mat->mul(nmat,data.mountTransform[shapeIndex]);
      }
   }
   else
      *mat = getRenderTransform();
}

void ShapeBase::getRenderImageTransform(U32 imageSlot,S32 node,MatrixF* mat)
{
   // Image transform in world space
   MountedImage& image = mMountedImageList[imageSlot];
   if (image.dataBlock)
   {
      if (node != -1)
      {
         ShapeBaseImageData& data = *image.dataBlock;
         U32 shapeIndex = getImageShapeIndex(image);

         MatrixF nmat = image.shapeInstance[shapeIndex]->mNodeTransforms[node];
         MatrixF mmat;

         if ( data.useEyeNode && isFirstPerson() && data.eyeMountNode[shapeIndex] != -1 )
         {
            MatrixF emat;
            getRenderEyeBaseTransform(&emat, mDataBlock->mountedImagesBank);

            MatrixF mountTransform = image.shapeInstance[shapeIndex]->mNodeTransforms[data.eyeMountNode[shapeIndex]];
            mountTransform.affineInverse();

            mmat.mul(emat, mountTransform);
         }
         else if ( data.useEyeOffset && isFirstPerson() ) 
         {
            MatrixF emat;
            getRenderEyeTransform(&emat);
            mmat.mul(emat,data.eyeOffset);
         }
         else 
         {
            MatrixF emat;
            getRenderMountTransform( 0.0f, data.mountPoint, MatrixF::Identity, &emat );
            mmat.mul(emat,data.mountTransform[shapeIndex]);
         }

         mat->mul(mmat, nmat);
      }
      else
         getRenderImageTransform(imageSlot,mat);
   }
   else
      *mat = getRenderTransform();
}

void ShapeBase::getRenderImageTransform(U32 imageSlot,StringTableEntry nodeName,MatrixF* mat)
{
   getRenderImageTransform( imageSlot, getNodeIndex( imageSlot, nodeName ), mat );
}

void ShapeBase::getRenderMuzzleTransform(U32 imageSlot,MatrixF* mat)
{
   // Muzzle transform in world space
   MountedImage& image = mMountedImageList[imageSlot];
   if (image.dataBlock)
      getRenderImageTransform(imageSlot,image.dataBlock->muzzleNode[getImageShapeIndex(image)],mat);
   else
      *mat = getRenderTransform();
}


void ShapeBase::getRetractionTransform(U32 imageSlot,MatrixF* mat)
{
   // Muzzle transform in world space
   MountedImage& image = mMountedImageList[imageSlot];
   if (image.dataBlock) {
      ShapeBaseImageData& data = *image.dataBlock;
      U32 imageShapeIndex = getImageShapeIndex(image);
      if (data.retractNode[imageShapeIndex] != -1)
         getImageTransform(imageSlot,data.retractNode[imageShapeIndex],mat);
      else
         getImageTransform(imageSlot,data.muzzleNode[imageShapeIndex],mat);
   } else {
      *mat = getTransform();
   }
}


void ShapeBase::getRenderRetractionTransform(U32 imageSlot,MatrixF* mat)
{
   // Muzzle transform in world space
   MountedImage& image = mMountedImageList[imageSlot];
   if (image.dataBlock) {
      ShapeBaseImageData& data = *image.dataBlock;
      U32 imageShapeIndex = getImageShapeIndex(image);
      if (data.retractNode[imageShapeIndex] != -1)
         getRenderImageTransform(imageSlot,data.retractNode[imageShapeIndex],mat);
      else
         getRenderImageTransform(imageSlot,data.muzzleNode[imageShapeIndex],mat);
   } else {
      *mat = getRenderTransform();
   }
}


//----------------------------------------------------------------------------

S32 ShapeBase::getNodeIndex(U32 imageSlot,StringTableEntry nodeName)
{
   MountedImage& image = mMountedImageList[imageSlot];
   if (image.dataBlock)
      return image.dataBlock->shape[getImageShapeIndex(image)]->findNode(nodeName);
   else
      return -1;
}

// Modify muzzle if needed to aim at whatever is straight in front of the camera.  Let the
// caller know if we actually modified the result.
bool ShapeBase::getCorrectedAim(const MatrixF& muzzleMat, VectorF* result)
{
   F32 pullInD = sFullCorrectionDistance;
   const F32 maxAdjD = 500;

   VectorF  aheadVec(0, maxAdjD, 0);

   MatrixF  camMat;
   Point3F  camPos;

   F32 pos = 0;
   GameConnection * gc = getControllingClient();
   if (gc && !gc->isFirstPerson())
      pos = 1.0f;

   getCameraTransform(&pos, &camMat);

   camMat.getColumn(3, &camPos);
   camMat.mulV(aheadVec);
   Point3F  aheadPoint = (camPos + aheadVec);

   // Should we check if muzzle point is really close to camera?  Does that happen?
   Point3F  muzzlePos;
   muzzleMat.getColumn(3, &muzzlePos);

   Point3F  collidePoint;
   VectorF  collideVector;
   disableCollision();
      RayInfo rinfo;
      if (getContainer()->castRay(camPos, aheadPoint, STATIC_COLLISION_TYPEMASK|DAMAGEABLE_TYPEMASK, &rinfo) &&
         (mDot(rinfo.point - mObjToWorld.getPosition(), mObjToWorld.getForwardVector()) > 0)) // Check if point is behind us (could happen in 3rd person view)
         collideVector = ((collidePoint = rinfo.point) - camPos);
      else
         collideVector = ((collidePoint = aheadPoint) - camPos);
   enableCollision();

   // For close collision we want to NOT aim at ground since we're bending
   // the ray here as it is.  But we don't want to pop, so adjust continuously.
   F32   lenSq = collideVector.lenSquared();
   if (lenSq < (pullInD * pullInD) && lenSq > 0.04)
   {
      F32   len = mSqrt(lenSq);
      F32   mid = pullInD;    // (pullInD + len) / 2.0;
      // This gives us point beyond to focus on-
      collideVector *= (mid / len);
      collidePoint = (camPos + collideVector);
   }

   VectorF  muzzleToCollide = (collidePoint - muzzlePos);
   lenSq = muzzleToCollide.lenSquared();
   if (lenSq > 0.04)
   {
      muzzleToCollide *= (1 / mSqrt(lenSq));
      * result = muzzleToCollide;
      return true;
   }
   return false;
}

//----------------------------------------------------------------------------

void ShapeBase::updateMass()
{
   if (mDataBlock) {
      F32 imass = 0;
      for (U32 i = 0; i < MaxMountedImages; i++) {
         MountedImage& image = mMountedImageList[i];
         if (image.dataBlock)
            imass += image.dataBlock->mass;
      }
      //
      mMass = mDataBlock->mass + imass;
      mOneOverMass = 1 / mMass;
   }
}

void ShapeBase::onImage(U32 imageSlot, bool unmount)
{
}

void ShapeBase::onImageRecoil(U32,ShapeBaseImageData::StateData::RecoilState)
{
}

void ShapeBase::onImageStateAnimation(U32 imageSlot, const char* seqName, bool direction, bool scaleToState, F32 stateTimeOutValue)
{
}

void ShapeBase::onImageAnimThreadChange(U32 imageSlot, S32 imageShapeIndex, ShapeBaseImageData::StateData* lastState, const char* anim, F32 pos, F32 timeScale, bool reset)
{
}

void ShapeBase::onImageAnimThreadUpdate(U32 imageSlot, S32 imageShapeIndex, F32 dt)
{
}


//----------------------------------------------------------------------------

void ShapeBase::setImage(  U32 imageSlot, 
                           ShapeBaseImageData* imageData, 
                           NetStringHandle& skinNameHandle, 
                           bool loaded, 
                           bool ammo, 
                           bool triggerDown,
                           bool altTriggerDown,
                           bool motion,
                           bool genericTrigger0,
                           bool genericTrigger1,
                           bool genericTrigger2,
                           bool genericTrigger3,
                           bool target)
{
   AssertFatal(imageSlot<MaxMountedImages,"Out of range image slot");

   MountedImage& image = mMountedImageList[imageSlot];

   // If we already have this datablock...
   if (image.dataBlock == imageData) {
      // Mark that there is not a datablock change pending.
      image.nextImage = InvalidImagePtr;
      // Change the skin handle if necessary.
      if (image.skinNameHandle != skinNameHandle) {
         if (!isGhost()) {
            // Serverside, note the skin handle and tell the client.
            image.skinNameHandle = skinNameHandle;
            setMaskBits(ImageMaskN << imageSlot);
         }
         else {
            // Clientside, do the reskin.
            image.skinNameHandle = skinNameHandle;
            for( U32 i=0; i<ShapeBaseImageData::MaxShapes; ++i)
            {
               if (image.shapeInstance[i])
               {
                  String newSkin = skinNameHandle.getString();
                  image.shapeInstance[i]->reSkin(newSkin, image.appliedSkinName);
                  image.appliedSkinName = newSkin;
               }
            }
         }
      }
      return;
   }

   // Check to see if we need to delay image changes until state change.
   if (!isGhost()) {
      if (imageData && image.dataBlock && !image.state->allowImageChange) {
         image.nextImage = imageData;
         image.nextSkinNameHandle = skinNameHandle;
         image.nextLoaded = loaded;
         return;
      }
   }

   // Mark that updates are happenin'.
   setMaskBits(ImageMaskN << imageSlot);

   // Notify script unmount since we're swapping datablocks.
   if (image.dataBlock && !isGhost()) {
      F32 dt = image.dataBlock->useRemainderDT ? image.rDT : 0.0f;
      image.dataBlock->onUnmount_callback( this, imageSlot, dt );
   }

   // Stop anything currently going on with the image.
   resetImageSlot(imageSlot);

   // If we're just unselecting the current shape without swapping
   // in a new one, then bail.
   if (!imageData) {
      onImage( imageSlot, true);
      return;
   }

   // Otherwise, init the new shape.
   image.dataBlock = imageData;
   image.state = &image.dataBlock->state[0];
   image.skinNameHandle = skinNameHandle;
   image.updateDoAnimateAllShapes(this);

   for (U32 i=0; i<ShapeBaseImageData::MaxShapes; ++i)
   {
      if (image.dataBlock->shapeIsValid[i])
         image.shapeInstance[i] = new TSShapeInstance(image.dataBlock->shape[i], isClientObject());
   }

   if (isClientObject())
   {
      for (U32 i=0; i<ShapeBaseImageData::MaxShapes; ++i)
      {
         if (image.shapeInstance[i])
         {
            image.shapeInstance[i]->cloneMaterialList();
            String newSkin = skinNameHandle.getString();
            image.shapeInstance[i]->reSkin(newSkin, image.appliedSkinName);
            image.appliedSkinName = newSkin;
         }
      }
   }
   image.loaded = loaded;
   image.ammo = ammo;
   image.triggerDown = triggerDown;
   image.altTriggerDown = altTriggerDown;
   image.target = target;
   image.motion = motion;
   image.genericTrigger[0] = genericTrigger0;
   image.genericTrigger[1] = genericTrigger1;
   image.genericTrigger[2] = genericTrigger2;
   image.genericTrigger[3] = genericTrigger3;

   // The server needs the shape loaded for muzzle mount nodes
   // but it doesn't need to run any of the animations, unless the image
   // has animateOnServer set.  Then the server needs to animate as well.
   // This is often set when using useEyeNode.
   for (U32 i=0; i<ShapeBaseImageData::MaxShapes; ++i)
   {
      image.ambientThread[i] = 0;
      image.animThread[i] = 0;
      image.flashThread[i] = 0;
      image.spinThread[i] = 0;
   }

   if (imageData->animateOnServer || isGhost())
   {
      for (U32 i=0; i<ShapeBaseImageData::MaxShapes; ++i)
      {
         if (!image.shapeInstance[i])
            continue;

         if (image.dataBlock->isAnimated[i]) {
            image.animThread[i] = image.shapeInstance[i]->addThread();
            image.shapeInstance[i]->setTimeScale(image.animThread[i],0);
         }
         if (image.dataBlock->hasFlash[i]) {
            image.flashThread[i] = image.shapeInstance[i]->addThread();
            image.shapeInstance[i]->setTimeScale(image.flashThread[i],0);
         }
         if (image.dataBlock->ambientSequence[i] != -1) {
            image.ambientThread[i] = image.shapeInstance[i]->addThread();
            image.shapeInstance[i]->setTimeScale(image.ambientThread[i],1);
            image.shapeInstance[i]->setSequence(image.ambientThread[i],
                                             image.dataBlock->ambientSequence[i],0);
         }
         if (image.dataBlock->spinSequence[i] != -1) {
            image.spinThread[i] = image.shapeInstance[i]->addThread();
            image.shapeInstance[i]->setTimeScale(image.spinThread[i],1);
            image.shapeInstance[i]->setSequence(image.spinThread[i],
                                             image.dataBlock->spinSequence[i],0);
         }
      }
   }

   // Set the image to its starting state.
   setImageState(imageSlot, (U32)0, true);

   // Update the mass for the mount object.
   updateMass();

   // Notify script mount.
   if ( !isGhost() )
   {
      F32 dt = image.dataBlock->useRemainderDT ? image.rDT : 0.0f;
      image.dataBlock->onMount_callback( this, imageSlot, dt );
   }
   else
   {
      if ( imageData->lightType == ShapeBaseImageData::PulsingLight )
         image.lightStart = Sim::getCurrentTime();
   }

   onImage(imageSlot, false);

   // Done.
}


//----------------------------------------------------------------------------

void ShapeBase::resetImageSlot(U32 imageSlot)
{
   AssertFatal(imageSlot<MaxMountedImages,"Out of range image slot");

   // Clear out current image
   MountedImage& image = mMountedImageList[imageSlot];
   for (U32 i=0; i<ShapeBaseImageData::MaxShapes; ++i)
   {
      delete image.shapeInstance[i];
      image.shapeInstance[i] = 0;
   }

   // stop sound
   for(Vector<SFXSource*>::iterator i = image.mSoundSources.begin(); i != image.mSoundSources.end(); i++)  
   {  
      SFX_DELETE((*i));  
   }  
   image.mSoundSources.clear(); 

   for (S32 i = 0; i < MaxImageEmitters; i++) {
      MountedImage::ImageEmitter& em = image.emitter[i];
      if (bool(em.emitter)) {
         em.emitter->deleteWhenEmpty();
         em.emitter = 0;
      }
   }

   image.dataBlock = 0;
   image.nextImage = InvalidImagePtr;
   image.skinNameHandle = NetStringHandle();
   image.nextSkinNameHandle  = NetStringHandle();
   image.state = 0;
   image.delayTime = 0;
   image.rDT = 0;
   image.ammo = false;
   image.triggerDown = false;
   image.altTriggerDown = false;
   image.loaded = false;
   image.motion = false;

   for (U32 i=0; i<ShapeBaseImageData::MaxGenericTriggers; ++i)
   {
      image.genericTrigger[i] = false;
   }

   image.lightStart = 0;
   if ( image.lightInfo != NULL )
      SAFE_DELETE( image.lightInfo );

   updateMass();
}


//----------------------------------------------------------------------------

bool ShapeBase::getImageTriggerState(U32 imageSlot)
{
   if (isGhost() || !mMountedImageList[imageSlot].dataBlock)
      return false;
   return mMountedImageList[imageSlot].triggerDown;
}

void ShapeBase::setImageTriggerState(U32 imageSlot,bool trigger)
{
   if (isGhost() || !mMountedImageList[imageSlot].dataBlock)
      return;
   MountedImage& image = mMountedImageList[imageSlot];

   if (trigger) {
      if (!image.triggerDown && image.dataBlock) {
         image.triggerDown = true;
         if (!isGhost()) {
            setMaskBits(ImageMaskN << imageSlot);
            updateImageState(imageSlot,0);
         }
      }
   }
   else
      if (image.triggerDown) {
         image.triggerDown = false;
         if (!isGhost()) {
            setMaskBits(ImageMaskN << imageSlot);
            updateImageState(imageSlot,0);
         }
      }
}

bool ShapeBase::getImageAltTriggerState(U32 imageSlot)
{
   if (isGhost() || !mMountedImageList[imageSlot].dataBlock)
      return false;
   return mMountedImageList[imageSlot].altTriggerDown;
}

void ShapeBase::setImageAltTriggerState(U32 imageSlot,bool trigger)
{
   if (isGhost() || !mMountedImageList[imageSlot].dataBlock)
      return;
   MountedImage& image = mMountedImageList[imageSlot];

   if (trigger) {
      if (!image.altTriggerDown && image.dataBlock) {
         image.altTriggerDown = true;
         if (!isGhost()) {
            setMaskBits(ImageMaskN << imageSlot);
            updateImageState(imageSlot,0);
         }
      }
   }
   else
      if (image.altTriggerDown) {
         image.altTriggerDown = false;
         if (!isGhost()) {
            setMaskBits(ImageMaskN << imageSlot);
            updateImageState(imageSlot,0);
         }
      }
}

//----------------------------------------------------------------------------

U32 ShapeBase::getImageFireState(U32 imageSlot)
{
   MountedImage& image = mMountedImageList[imageSlot];
   // If there is no fire state, then try state 0
   if (image.dataBlock && image.dataBlock->fireState != -1)
      return image.dataBlock->fireState;
   return 0;
}

U32 ShapeBase::getImageAltFireState(U32 imageSlot)
{
   MountedImage& image = mMountedImageList[imageSlot];
   // If there is no alternate fire state, then try state 0
   if (image.dataBlock && image.dataBlock->altFireState != -1)
      return image.dataBlock->altFireState;
   return 0;
}

U32  ShapeBase::getImageReloadState(U32 imageSlot)
{
   MountedImage& image = mMountedImageList[imageSlot];
   // If there is no reload state, then try state 0
   if (image.dataBlock && image.dataBlock->reloadState != -1)
      return image.dataBlock->reloadState;
   return 0;
}


//----------------------------------------------------------------------------

bool ShapeBase::hasImageState(U32 imageSlot, const char* state)
{
   if (!state || !state[0])
      return false;

   MountedImage& image = mMountedImageList[imageSlot];
   if (image.dataBlock)
   {
      for (U32 i = 0; i < ShapeBaseImageData::MaxStates; i++)
      {
         ShapeBaseImageData::StateData& sd = image.dataBlock->state[i];
         if (sd.name && !dStricmp(state, sd.name))
            return true;
      }
   }

   return false;
}

void ShapeBase::setImageState(U32 imageSlot, U32 newState,bool force)
{
   if (!mMountedImageList[imageSlot].dataBlock)
      return;
   MountedImage& image = mMountedImageList[imageSlot];


   // The client never enters the initial fire state on its own, but it
   //  will continue to set that state...
   if (isGhost() && !force && newState == image.dataBlock->fireState) {
      if (image.state != &image.dataBlock->state[newState])
         return;
   }

   // The client never enters the initial alternate fire state on its own, but it
   //  will continue to set that state...
   if (isGhost() && !force && newState == image.dataBlock->altFireState) {
      if (image.state != &image.dataBlock->state[newState])
         return;
   }

   // The client never enters the initial reload state on its own, but it
   //  will continue to set that state...
   if (isGhost() && !force && newState == image.dataBlock->reloadState) {
      if (image.state != &image.dataBlock->state[newState])
         return;
   }

   // Eject shell casing on every state change (client side only)
   ShapeBaseImageData::StateData& nextStateData = image.dataBlock->state[newState];
   if (isGhost() && nextStateData.ejectShell) {
      ejectShellCasing( imageSlot );
   }

   // Shake camera on client.
   if (isGhost() && nextStateData.fire && image.dataBlock->shakeCamera) {
      shakeCamera( imageSlot );
   }

   // Server must animate the shape if it is a firestate...
   if (isServerObject() && (image.dataBlock->state[newState].fire || image.dataBlock->state[newState].altFire))
      mShapeInstance->animate();

   // Obtain the image's shape index for future use.
   U32 imageShapeIndex = getImageShapeIndex(image);
   image.lastShapeIndex = imageShapeIndex;

   // If going back into the same state, just reset the timer
   // and invoke the script callback
   if (!force && image.state == &image.dataBlock->state[newState]) {
      image.delayTime = image.state->timeoutValue;
      if (image.state->script && !isGhost())
         scriptCallback(imageSlot,image.state->script);

      // If this is a flash sequence, we need to select a new position for the
      //  animation if we're returning to that state...
      F32 randomPos = Platform::getRandom();
      for (U32 i=0; i<ShapeBaseImageData::MaxShapes; ++i)
      {
         if (!image.dataBlock->shapeIsValid[i] || (i != imageShapeIndex && !image.doAnimateAllShapes))
            continue;

         if (image.animThread[i] && image.state->sequence[i] != -1 && image.state->flashSequence[i]) {
            image.shapeInstance[i]->setPos(image.animThread[i], randomPos);
            image.shapeInstance[i]->setTimeScale(image.animThread[i], 0);
            if (image.flashThread[i])
               image.shapeInstance[i]->setPos(image.flashThread[i], 0);
         }
      }

      return;
   }

   F32 lastDelay = image.delayTime;
   ShapeBaseImageData::StateData* lastState = image.state;
   image.state = &image.dataBlock->state[newState];

   //
   // Do state cleanup first...
   //
   ShapeBaseImageData::StateData& stateData = *image.state;
   image.delayTime = stateData.timeoutValue;

   // Mount pending images
   if (image.nextImage != InvalidImagePtr && stateData.allowImageChange) {
      setImage(imageSlot,image.nextImage,image.nextSkinNameHandle,image.nextLoaded);
      return;
   }

   // Reset cyclic sequences back to the first frame to turn it off
   // (the first key frame should be it's off state).
   // We need to do this across all image shapes to make sure we have no hold overs when switching
   // rendering shapes while in the middle of a state change.
   for (U32 i=0; i<ShapeBaseImageData::MaxShapes; ++i)
   {
      // If we are to do a sequence transition then we need to keep the previous animThread active
      if (image.animThread[i] && image.animThread[i]->getSequence()->isCyclic() && (stateData.sequenceNeverTransition || !(stateData.sequenceTransitionIn || lastState->sequenceTransitionOut))) {
         image.shapeInstance[i]->setPos(image.animThread[i],0);
         image.shapeInstance[i]->setTimeScale(image.animThread[i],0);
      }
      if (image.flashThread[i]) {
         image.shapeInstance[i]->setPos(image.flashThread[i],0);
         image.shapeInstance[i]->setTimeScale(image.flashThread[i],0);
      }
   }

   // Broadcast the reset
   onImageAnimThreadChange(imageSlot, imageShapeIndex, lastState, NULL, 0, 0, true);

   // Check for immediate transitions, but only if we don't need to wait for
   // a time out.  Only perform this wait if we're not forced to change.
   S32 ns;
   if (image.delayTime <= 0 || !stateData.waitForTimeout) 
   {
      if ((ns = stateData.transition.loaded[image.loaded]) != -1) {
         setImageState(imageSlot,ns);
         return;
      }
      for (U32 i=0; i<ShapeBaseImageData::MaxGenericTriggers; ++i)
      {
         if ((ns = stateData.transition.genericTrigger[i][image.genericTrigger[i]]) != -1) {
            setImageState(imageSlot, ns);
            return;
         }
      }
      //if (!imageData.usesEnergy)
         if ((ns = stateData.transition.ammo[image.ammo]) != -1) {
            setImageState(imageSlot,ns);
            return;
         }
      if ((ns = stateData.transition.target[image.target]) != -1) {
         setImageState(imageSlot, ns);
         return;
      }
      if ((ns = stateData.transition.wet[image.wet]) != -1) {
         setImageState(imageSlot, ns);
         return;
      }
      if ((ns = stateData.transition.motion[image.motion]) != -1) {
         setImageState(imageSlot, ns);
         return;
      }
      if ((ns = stateData.transition.trigger[image.triggerDown]) != -1) {
         setImageState(imageSlot,ns);
         return;
      }
      if ((ns = stateData.transition.altTrigger[image.altTriggerDown]) != -1) {
         setImageState(imageSlot,ns);
         return;
      }
   }

   //
   // Initialize the new state...
   //
   if (stateData.loaded != ShapeBaseImageData::StateData::IgnoreLoaded)
      image.loaded = stateData.loaded == ShapeBaseImageData::StateData::Loaded;
   if (!isGhost() && image.dataBlock->state[newState].fire) {
      setMaskBits(ImageMaskN << imageSlot);
      image.fireCount = (image.fireCount + 1) & 0x7;
   }
   if (!isGhost() && image.dataBlock->state[newState].altFire) {
      setMaskBits(ImageMaskN << imageSlot);
      image.altFireCount = (image.altFireCount + 1) & 0x7;
   }
   if (!isGhost() && image.dataBlock->state[newState].reload) {
      setMaskBits(ImageMaskN << imageSlot);
      image.reloadCount = (image.reloadCount + 1) & 0x7;
   }

   // Apply recoil
   if (stateData.recoil != ShapeBaseImageData::StateData::NoRecoil)
      onImageRecoil(imageSlot,stateData.recoil);

   // Apply image state animation on mounting shape
   if (stateData.shapeSequence && stateData.shapeSequence[0])
   {
      onImageStateAnimation(imageSlot, stateData.shapeSequence, stateData.direction, stateData.shapeSequenceScale, stateData.timeoutValue);
   }

   // Delete any loooping sounds that were in the previous state.
   if (lastState->sound && lastState->sound->getDescription()->mIsLooping)  
   {  
      for(Vector<SFXSource*>::iterator i = image.mSoundSources.begin(); i != image.mSoundSources.end(); i++)      
         SFX_DELETE((*i));    

      image.mSoundSources.clear();  
   }  

   // Play sound
   if( stateData.sound && isGhost() )
   {
      const Point3F& velocity         = getVelocity();
	   image.addSoundSource(SFX->createSource( stateData.sound, &getRenderTransform(), &velocity )); 
   }

   // Play animation
   updateAnimThread(imageSlot, imageShapeIndex, lastState);
   for (U32 i=0; i<ShapeBaseImageData::MaxShapes; ++i)
   {
      if (!image.dataBlock->shapeIsValid[i] || (i != imageShapeIndex && !image.doAnimateAllShapes))
         continue;

      // Start spin thread
      if (image.spinThread[i]) {
         switch (stateData.spin) {
          case ShapeBaseImageData::StateData::IgnoreSpin:
            image.shapeInstance[i]->setTimeScale(image.spinThread[i], image.shapeInstance[i]->getTimeScale(image.spinThread[i]));
            break;
          case ShapeBaseImageData::StateData::NoSpin:
            image.shapeInstance[i]->setTimeScale(image.spinThread[i],0);
            break;
          case ShapeBaseImageData::StateData::SpinUp:
            if (lastState->spin == ShapeBaseImageData::StateData::SpinDown)
               image.delayTime *= 1.0f - (lastDelay / stateData.timeoutValue);
            break;
          case ShapeBaseImageData::StateData::SpinDown:
            if (lastState->spin == ShapeBaseImageData::StateData::SpinUp)
               image.delayTime *= 1.0f - (lastDelay / stateData.timeoutValue);
            break;
          case ShapeBaseImageData::StateData::FullSpin:
            image.shapeInstance[i]->setTimeScale(image.spinThread[i],1);
            break;
         }
      }
   }

   // Start particle emitter on the client (client side only)
   if (isGhost() && stateData.emitter)
      startImageEmitter(image,stateData);

   // Script callback on server
   if (stateData.script && stateData.script[0] && !isGhost())
      scriptCallback(imageSlot,stateData.script);

   // If there is a zero timeout, and a timeout transition, then
   // go ahead and transition imediately.
   if (!image.delayTime)
   {
      if ((ns = stateData.transition.timeout) != -1)
      {
         setImageState(imageSlot,ns);
         return;
      }
   }
}

void ShapeBase::updateAnimThread(U32 imageSlot, S32 imageShapeIndex, ShapeBaseImageData::StateData* lastState)
{
   MountedImage& image = mMountedImageList[imageSlot];
   ShapeBaseImageData::StateData& stateData = *image.state;

   F32 randomPos = Platform::getRandom();
   for (U32 i=0; i<ShapeBaseImageData::MaxShapes; ++i)
   {
      if (!image.dataBlock->shapeIsValid[i] || (i != imageShapeIndex && !image.doAnimateAllShapes))
         continue;

      if (image.animThread[i] && stateData.sequence[i] != -1) 
      {
         S32 seqIndex = stateData.sequence[i];  // Standard index without any prefix
         bool scaleAnim = stateData.scaleAnimation;
         if (i == ShapeBaseImageData::FirstPersonImageShape)
            scaleAnim = stateData.scaleAnimationFP;

         // We're going to apply various prefixes to determine the final sequence to use.
         // Here is the order:
         // shapeBasePrefix_scriptPrefix_baseAnimName
         // shapeBasePrefix_baseAnimName
         // scriptPrefix_baseAnimName
         // baseAnimName

         // Collect the prefixes
         const char* shapeBasePrefix = getImageAnimPrefix(imageSlot, i);
         bool hasShapeBasePrefix = shapeBasePrefix && shapeBasePrefix[0];
         const char* scriptPrefix = getImageScriptAnimPrefix(imageSlot).getString();
         bool hasScriptPrefix = scriptPrefix && scriptPrefix[0];

         // Find the final sequence based on the prefix combinations
         if (hasShapeBasePrefix || hasScriptPrefix)
         {
            bool found = false;
            String baseSeqName(image.shapeInstance[i]->getShape()->getSequenceName(stateData.sequence[i]));

            if (!found && hasShapeBasePrefix && hasScriptPrefix)
            {
               String seqName = String(shapeBasePrefix) + String("_") + String(scriptPrefix) + String("_") + baseSeqName;
               S32 index = image.shapeInstance[i]->getShape()->findSequence(seqName);
               if (index != -1)
               {
                  seqIndex = index;
                  found = true;
               }
            }

            if (!found && hasShapeBasePrefix)
            {
               String seqName = String(shapeBasePrefix) + String("_") + baseSeqName;
               S32 index = image.shapeInstance[i]->getShape()->findSequence(seqName);
               if (index != -1)
               {
                  seqIndex = index;
                  found = true;
               }
            }

            if (!found && hasScriptPrefix)
            {
               String seqName = String(scriptPrefix) + String("_") + baseSeqName;
               S32 index = image.shapeInstance[i]->getShape()->findSequence(seqName);
               if (index != -1)
               {
                  seqIndex = index;
                  found = true;
               }
            }
         }

         if (seqIndex != -1)
         {
            if (!lastState)
            {
               // No lastState indicates that we are just switching animation sequences, not states.  Transition into this new sequence, but only
               // if it is different than what we're currently playing.
               S32 prevSeq = -1;
               if (image.animThread[i]->hasSequence())
               {
                  prevSeq = image.shapeInstance[i]->getSequence(image.animThread[i]);
               }
               if (seqIndex != prevSeq)
               {
                  image.shapeInstance[i]->transitionToSequence(image.animThread[i], seqIndex, stateData.direction ? 0.0f : 1.0f, image.dataBlock->scriptAnimTransitionTime, true);
               }
            }
            else if (!stateData.sequenceNeverTransition && stateData.sequenceTransitionTime && (stateData.sequenceTransitionIn || lastState->sequenceTransitionOut))
            {
               image.shapeInstance[i]->transitionToSequence(image.animThread[i], seqIndex, stateData.direction ? 0.0f : 1.0f, stateData.sequenceTransitionTime, true);
            }
            else
            {
               image.shapeInstance[i]->setSequence(image.animThread[i], seqIndex, stateData.direction ? 0.0f : 1.0f);
            }

            if (stateData.flashSequence[i] == false) 
            {
               F32 timeScale = (scaleAnim && stateData.timeoutValue) ?
                  image.shapeInstance[i]->getDuration(image.animThread[i]) / stateData.timeoutValue : 1.0f;
               image.shapeInstance[i]->setTimeScale(image.animThread[i], stateData.direction ? timeScale : -timeScale);

               // Broadcast the sequence change
               String seqName = image.shapeInstance[i]->getShape()->getSequenceName(stateData.sequence[i]);
               onImageAnimThreadChange(imageSlot, imageShapeIndex, lastState, seqName, stateData.direction ? 0.0f : 1.0f, stateData.direction ? timeScale : -timeScale);
            }
            else
            {
               image.shapeInstance[i]->setPos(image.animThread[i], randomPos);
               image.shapeInstance[i]->setTimeScale(image.animThread[i], 0);

               S32 seqVisIndex = stateData.sequenceVis[i];

               // Go through the same process as the animThread sequence to find the flashThread sequence
               if (hasShapeBasePrefix || hasScriptPrefix)
               {
                  bool found = false;
                  String baseVisSeqName(image.shapeInstance[i]->getShape()->getSequenceName(stateData.sequenceVis[i]));

                  if (!found && hasShapeBasePrefix && hasScriptPrefix)
                  {
                     String seqName = String(shapeBasePrefix) + String("_") + String(scriptPrefix) + String("_") + baseVisSeqName;
                     S32 index = image.shapeInstance[i]->getShape()->findSequence(seqName);
                     if (index != -1)
                     {
                        seqVisIndex = index;
                        found = true;
                     }
                  }

                  if (!found && hasShapeBasePrefix)
                  {
                     String seqName = String(shapeBasePrefix) + String("_") + baseVisSeqName;
                     S32 index = image.shapeInstance[i]->getShape()->findSequence(seqName);
                     if (index != -1)
                     {
                        seqVisIndex = index;
                        found = true;
                     }
                  }

                  if (!found && hasScriptPrefix)
                  {
                     String seqName = String(scriptPrefix) + String("_") + baseVisSeqName;
                     S32 index = image.shapeInstance[i]->getShape()->findSequence(seqName);
                     if (index != -1)
                     {
                        seqVisIndex = index;
                        found = true;
                     }
                  }
               }

               image.shapeInstance[i]->setSequence(image.flashThread[i], seqVisIndex, 0);
               image.shapeInstance[i]->setPos(image.flashThread[i], 0);
               F32 timeScale = (scaleAnim && stateData.timeoutValue) ?
                  image.shapeInstance[i]->getDuration(image.flashThread[i]) / stateData.timeoutValue : 1.0f;
               image.shapeInstance[i]->setTimeScale(image.flashThread[i], timeScale);

               // Broadcast the sequence change
               String seqName = image.shapeInstance[i]->getShape()->getSequenceName(stateData.sequenceVis[i]);
               onImageAnimThreadChange(imageSlot, imageShapeIndex, lastState, seqName, stateData.direction ? 0.0f : 1.0f, stateData.direction ? timeScale : -timeScale);
            }
         }
      }
   }
}


//----------------------------------------------------------------------------

void ShapeBase::updateImageState(U32 imageSlot,F32 dt)
{
   if (!mMountedImageList[imageSlot].dataBlock)
      return;
   MountedImage& image = mMountedImageList[imageSlot];
   ShapeBaseImageData& imageData = *image.dataBlock;

   image.rDT = dt;
   F32 elapsed;

TICKAGAIN:

   ShapeBaseImageData::StateData& stateData = *image.state;

   if ( image.delayTime > dt )
      elapsed = dt;
   else
      elapsed = image.delayTime;

   dt = elapsed;
   image.rDT -= elapsed;

   image.delayTime -= dt;

   // Energy management
   if (imageData.usesEnergy) 
   {
      F32 newEnergy = getEnergyLevel() - stateData.energyDrain * dt;
      if (newEnergy < 0)
         newEnergy = 0;
      setEnergyLevel(newEnergy);

      if (!isGhost()) 
      {
         bool ammo = newEnergy > imageData.minEnergy;
         if (ammo != image.ammo) 
         {
            setMaskBits(ImageMaskN << imageSlot);
            image.ammo = ammo;
         }
      }
   }

   // Check for transitions. On some states we must wait for the
   // full timeout value before moving on.
   if (image.delayTime <= 0 || !stateData.waitForTimeout) 
   {
      S32 ns;

      if ((ns = stateData.transition.loaded[image.loaded]) != -1) 
         setImageState(imageSlot,ns);
      else if ((ns = stateData.transition.genericTrigger[0][image.genericTrigger[0]]) != -1)
         setImageState(imageSlot,ns);
      else if ((ns = stateData.transition.genericTrigger[1][image.genericTrigger[1]]) != -1)
         setImageState(imageSlot,ns);
      else if ((ns = stateData.transition.genericTrigger[2][image.genericTrigger[2]]) != -1)
         setImageState(imageSlot,ns);
      else if ((ns = stateData.transition.genericTrigger[3][image.genericTrigger[3]]) != -1)
         setImageState(imageSlot,ns);
      else if ((ns = stateData.transition.ammo[image.ammo]) != -1) 
         setImageState(imageSlot,ns);
      else if ((ns = stateData.transition.target[image.target]) != -1) 
         setImageState(imageSlot,ns);
      else if ((ns = stateData.transition.wet[image.wet]) != -1)
         setImageState(imageSlot,ns);
      else if ((ns = stateData.transition.motion[image.motion]) != -1)
         setImageState(imageSlot,ns);
      else if ((ns = stateData.transition.trigger[image.triggerDown]) != -1)
         setImageState(imageSlot,ns);
      else if ((ns = stateData.transition.altTrigger[image.altTriggerDown]) != -1) 
         setImageState(imageSlot,ns);
      else if (image.delayTime <= 0 && (ns = stateData.transition.timeout) != -1) 
         setImageState(imageSlot,ns);
   }

   // Update the spinning thread timeScale
   U32 imageShapeIndex = getImageShapeIndex(image);
   for (U32 i=0; i<ShapeBaseImageData::MaxShapes; ++i)
   {
      if (!image.dataBlock->shapeIsValid[i] || (i != imageShapeIndex && !image.doAnimateAllShapes))
         continue;

      if (image.spinThread[i])
      {
         F32 timeScale;

         switch (stateData.spin) 
         {
            case ShapeBaseImageData::StateData::IgnoreSpin:
            case ShapeBaseImageData::StateData::NoSpin:
            case ShapeBaseImageData::StateData::FullSpin: 
            {
               timeScale = 0;
               image.shapeInstance[i]->setTimeScale(image.spinThread[i], image.shapeInstance[i]->getTimeScale(image.spinThread[i]));
               break;
            }

            case ShapeBaseImageData::StateData::SpinUp: 
            {
               timeScale = 1.0f - image.delayTime / stateData.timeoutValue;
               image.shapeInstance[i]->setTimeScale(image.spinThread[i],timeScale);
               break;
            }

            case ShapeBaseImageData::StateData::SpinDown: 
            {
               timeScale = image.delayTime / stateData.timeoutValue;
               image.shapeInstance[i]->setTimeScale(image.spinThread[i],timeScale);
               break;
            }
         }
      }
   }

   if ( image.rDT > 0.0f && image.delayTime > 0.0f && imageData.useRemainderDT && dt != 0.0f )
   {
      dt = image.rDT;
      goto TICKAGAIN;
   }
}


//----------------------------------------------------------------------------

void ShapeBase::updateImageAnimation(U32 imageSlot, F32 dt)
{
   if (!mMountedImageList[imageSlot].dataBlock)
      return;
   MountedImage& image = mMountedImageList[imageSlot];
   U32 imageShapeIndex = getImageShapeIndex(image);

   // Advance animation threads
   for (U32 i=0; i<ShapeBaseImageData::MaxShapes; ++i)
   {
      if (!image.dataBlock->shapeIsValid[i] || (i != imageShapeIndex && !image.doAnimateAllShapes))
         continue;

      if (image.ambientThread[i])
         image.shapeInstance[i]->advanceTime(dt,image.ambientThread[i]);
      if (image.animThread[i])
         image.shapeInstance[i]->advanceTime(dt,image.animThread[i]);
      if (image.spinThread[i])
         image.shapeInstance[i]->advanceTime(dt,image.spinThread[i]);
      if (image.flashThread[i])
         image.shapeInstance[i]->advanceTime(dt,image.flashThread[i]);
   }

   // Broadcast the update
   onImageAnimThreadUpdate(imageSlot, imageShapeIndex, dt);

   image.updateSoundSources(getRenderTransform());

   // Particle emission
   for (S32 i = 0; i < MaxImageEmitters; i++) {
      MountedImage::ImageEmitter& em = image.emitter[i];
      if (bool(em.emitter)) {
         if (em.time > 0) {
            em.time -= dt;

            // Do we need to update the emitter's node due to the current shape changing?
            if (imageShapeIndex != image.lastShapeIndex)
            {
               em.node = image.state->emitterNode[imageShapeIndex];
            }

            MatrixF mat;
            getRenderImageTransform(imageSlot,em.node,&mat);
            Point3F pos,axis;
            mat.getColumn(3,&pos);
            mat.getColumn(1,&axis);
            em.emitter->emitParticles(pos,true,axis,getVelocity(),(U32) (dt * 1000));
         }
         else {
            em.emitter->deleteWhenEmpty();
            em.emitter = 0;
         }
      }
   }

   image.lastShapeIndex = imageShapeIndex;
}


//----------------------------------------------------------------------------

void ShapeBase::setImageScriptAnimPrefix(U32 imageSlot, NetStringHandle prefix)
{
   MountedImage& image = mMountedImageList[imageSlot];
   if (image.dataBlock) {
      setMaskBits(ImageMaskN << imageSlot);
      image.scriptAnimPrefix = prefix;
   }
}

NetStringHandle ShapeBase::getImageScriptAnimPrefix(U32 imageSlot)
{
   MountedImage& image = mMountedImageList[imageSlot];
   return image.dataBlock? image.scriptAnimPrefix : NetStringHandle();
}


//----------------------------------------------------------------------------

U32 ShapeBase::getImageShapeIndex(const MountedImage& image) const
{
   U32 shapeIndex = ShapeBaseImageData::StandardImageShape;

   const ShapeBaseImageData* data = image.dataBlock;
   if (data && data->useFirstPersonShape && isFirstPerson())
      shapeIndex = ShapeBaseImageData::FirstPersonImageShape;

   return shapeIndex;
}


//----------------------------------------------------------------------------

void ShapeBase::startImageEmitter(MountedImage& image,ShapeBaseImageData::StateData& state)
{
   MountedImage::ImageEmitter* bem = 0;
   MountedImage::ImageEmitter* em = image.emitter;
   MountedImage::ImageEmitter* ee = &image.emitter[MaxImageEmitters];

   U32 imageShapeIndex = getImageShapeIndex(image);

   // If we are already emitting the same particles from the same
   // node, then simply extend the time.  Otherwise, find an empty
   // emitter slot, or grab the one with the least amount of time left.
   for (; em != ee; em++) {
      if (bool(em->emitter)) {
         if (state.emitter == em->emitter->getDataBlock() && state.emitterNode[imageShapeIndex] == em->node) {
            if (state.emitterTime > em->time)
               em->time = state.emitterTime;
            return;
         }
         if (!bem || (bool(bem->emitter) && bem->time > em->time))
            bem = em;
      }
      else
         bem = em;
   }

   bem->time = state.emitterTime;
   bem->node = state.emitterNode[imageShapeIndex];
   bem->emitter = new ParticleEmitter;
   bem->emitter->onNewDataBlock(state.emitter,false);
   if( !bem->emitter->registerObject() )
   {
      bem->emitter.getPointer()->destroySelf();
      bem->emitter = NULL;
   }
}

void ShapeBase::submitLights( LightManager *lm, bool staticLighting )
{
   if ( staticLighting )
      return;

   // Submit lights for MountedImage(s)
   for ( S32 i = 0; i < MaxMountedImages; i++ )
   {
      ShapeBaseImageData *imageData = getMountedImage( i );

      if ( imageData != NULL && imageData->lightType != ShapeBaseImageData::NoLight )
      {                  
         MountedImage &image = mMountedImageList[i];         
         
         F32 intensity;

         switch ( imageData->lightType )
         {
         case ShapeBaseImageData::ConstantLight:
         case ShapeBaseImageData::SpotLight:
            intensity = 1.0f;
            break;

         case ShapeBaseImageData::PulsingLight:
            intensity = 0.5f + 0.5f * mSin( M_PI_F * (F32)Sim::getCurrentTime() / (F32)imageData->lightDuration + image.lightStart );
            intensity = 0.15f + intensity * 0.85f;
            break;

         case ShapeBaseImageData::WeaponFireLight:
            {
            S32 elapsed = Sim::getCurrentTime() - image.lightStart;
            if ( elapsed > imageData->lightDuration )
               continue;
            intensity = ( 1.0 - (F32)elapsed / (F32)imageData->lightDuration ) * imageData->lightBrightness;
            break;
            }
         default:
            intensity = 1.0f;
            return;
         }

         if ( !image.lightInfo )
            image.lightInfo = LightManager::createLightInfo();

         image.lightInfo->setColor( imageData->lightColor );
         image.lightInfo->setBrightness( intensity );   
         image.lightInfo->setRange( imageData->lightRadius );  

         if ( imageData->lightType == ShapeBaseImageData::SpotLight )
         {
            image.lightInfo->setType( LightInfo::Spot );
            // Do we want to expose these or not?
            image.lightInfo->setInnerConeAngle( 15 );
            image.lightInfo->setOuterConeAngle( 40 );      
         }
         else
            image.lightInfo->setType( LightInfo::Point );

         MatrixF imageMat;
         getRenderImageTransform( i, &imageMat );

         image.lightInfo->setTransform( imageMat );

         lm->registerGlobalLight( image.lightInfo, NULL );         
      }
   }
}


//----------------------------------------------------------------------------

void ShapeBase::ejectShellCasing( U32 imageSlot )
{
   MountedImage& image = mMountedImageList[imageSlot];
   ShapeBaseImageData* imageData = image.dataBlock;

   if (!imageData->casing)
      return;

   // Shell casings are client-side only, so use the render transform.
   MatrixF ejectTrans;
   getRenderImageTransform( imageSlot, imageData->ejectNode[getImageShapeIndex(image)], &ejectTrans );

   Point3F ejectDir = imageData->shellExitDir;
   ejectDir.normalize();

   F32 ejectSpread = mDegToRad( imageData->shellExitVariance );
   MatrixF ejectOrient = MathUtils::createOrientFromDir( ejectDir );

   Point3F randomDir;
   randomDir.x = mSin( gRandGen.randF( -ejectSpread, ejectSpread ) );
   randomDir.y = 1.0;
   randomDir.z = mSin( gRandGen.randF( -ejectSpread, ejectSpread ) );
   randomDir.normalizeSafe();

   ejectOrient.mulV( randomDir );

   MatrixF imageTrans = getRenderTransform();
   imageTrans.mulV( randomDir );

   Point3F shellVel = randomDir * imageData->shellVelocity;
   Point3F shellPos = ejectTrans.getPosition();


   Debris *casing = new Debris;
   casing->onNewDataBlock( imageData->casing, false );
   casing->setTransform( imageTrans );

   if (!casing->registerObject())
      delete casing;
   else
      casing->init( shellPos, shellVel );
}

void ShapeBase::shakeCamera( U32 imageSlot )
{
   MountedImage& image = mMountedImageList[imageSlot];
   ShapeBaseImageData* imageData = image.dataBlock;

   if (!imageData->shakeCamera)
      return;

   // Warning: this logic was duplicated from Explosion.

   // first check if explosion is near camera
   GameConnection* connection = GameConnection::getConnectionToServer();
   ShapeBase *obj = dynamic_cast<ShapeBase*>(connection->getControlObject());

   bool applyShake = true;

   if (obj)
   {
      ShapeBase* cObj = obj;
      while ((cObj = cObj->getControlObject()) != 0)
      {
         if (cObj->useObjsEyePoint())
         {
            applyShake = false;
            break;
         }
      }
   }

   if (applyShake && obj)
   {
      VectorF diff;
      getMuzzlePoint(imageSlot, &diff);
      diff = obj->getPosition() - diff;
      F32 dist = diff.len();
      if (dist < imageData->camShakeRadius)
      {
         CameraShake *camShake = new CameraShake;
         camShake->setDuration(imageData->camShakeDuration);
         camShake->setFrequency(imageData->camShakeFreq);

         F32 falloff =  dist / imageData->camShakeRadius;
         falloff = 1.0f + falloff * 10.0f;
         falloff = 1.0f / (falloff * falloff);

         VectorF shakeAmp = imageData->camShakeAmp * falloff;
         camShake->setAmplitude(shakeAmp);
         camShake->setFalloff(imageData->camShakeFalloff);
         camShake->init();
         gCamFXMgr.addFX(camShake);
      }
   }
}
