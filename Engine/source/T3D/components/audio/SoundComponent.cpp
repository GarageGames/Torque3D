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
#include "T3D/components/audio/SoundComponent.h"
#include "core/stream/bitStream.h"
#include "sim/netConnection.h"

#include "sfx/sfxSystem.h"
#include "sfx/sfxSource.h"
#include "sfx/sfxTrack.h"
#include "sfx/sfxDescription.h"
#include "T3D/sfx/sfx3DWorld.h"

#include "sfx/sfxTrack.h"
#include "sfx/sfxTypes.h"

#include "renderInstance/renderPassManager.h"
#include "gfx/gfxDrawUtil.h"

// Timeout for non-looping sounds on a channel
static SimTime sAudioTimeout = 500;

extern bool gEditingMission;

//////////////////////////////////////////////////////////////////////////
// Constructor/Destructor
//////////////////////////////////////////////////////////////////////////
SoundComponent::SoundComponent() : Component()
{
   //These flags inform that, in this particular component, we network down to the client, which enables the pack/unpackData functions to operate
   mNetworked = true;

   mFriendlyName = "Sound(Component)";
   mComponentType = "Sound";
   mDescription = getDescriptionText("Stores up to 4 sounds for playback.");

   for (U32 slotNum = 0; slotNum < MaxSoundThreads; slotNum++) {
      mSoundThread[slotNum].play = false;
      mSoundThread[slotNum].profile = 0;
      mSoundThread[slotNum].sound = 0;

      mSoundFile[slotNum] = NULL;
      mPreviewSound[slotNum] = false;
      mPlay[slotNum] = false;
   }
}

SoundComponent::~SoundComponent()
{
}

IMPLEMENT_CO_NETOBJECT_V1(SoundComponent);

//Standard onAdd function, for when the component is created
bool SoundComponent::onAdd()
{
   if (!Parent::onAdd())
      return false;

   for (U32 slotNum = 0; slotNum < MaxSoundThreads; slotNum++)
      mPreviewSound[slotNum] = false;

   return true;
}

//Standard onRemove function, when the component object is deleted
void SoundComponent::onRemove()
{
   Parent::onRemove();
}

//This is called when the component has been added to an entity
void SoundComponent::onComponentAdd()
{
   Parent::onComponentAdd();

   Con::printf("We were added to an entity! SoundComponent reporting in for owner entity %i", mOwner->getId());
}

//This is called when the component has been removed from an entity
void SoundComponent::onComponentRemove()
{
   Con::printf("We were removed from our entity! SoundComponent signing off for owner entity %i", mOwner->getId());
   Parent::onComponentRemove();
}

//This is called any time a component is added to an entity. Every component currently owned by the entity is informed of the event. 
//This allows you to do dependency behavior, like collisions being aware of a mesh component, etc
void SoundComponent::componentAddedToOwner(Component *comp)
{
   for (S32 slotNum = 0; slotNum < MaxSoundThreads; slotNum++)
   {
      if (mPlay[slotNum])
      {
         playAudio(slotNum, mSoundFile[slotNum]);
      }
   }
   Con::printf("Our owner entity has a new component being added! SoundComponent welcomes component %i of type %s", comp->getId(), comp->getClassRep()->getNameSpace());
}

//This is called any time a component is removed from an entity. Every component current owned by the entity is informed of the event.
//This allows cleanup and dependency management.
void SoundComponent::componentRemovedFromOwner(Component *comp)
{
   Con::printf("Our owner entity has a removed a component! SoundComponent waves farewell to component %i of type %s", comp->getId(), comp->getClassRep()->getNameSpace());
}

//Regular init persist fields function to set up static fields.
void SoundComponent::initPersistFields()
{
   //addArray("Sounds", MaxSoundThreads);
   addField("mSoundFile", TypeSFXTrackName, Offset(mSoundFile, SoundComponent), MaxSoundThreads, "If the text will not fit in the control, the deniedSound is played.");
   addProtectedField("mPreviewSound", TypeBool, Offset(mPreviewSound, SoundComponent),
      &_previewSound, &defaultProtectedGetFn, MaxSoundThreads, "Preview Sound", AbstractClassRep::FieldFlags::FIELD_ComponentInspectors);
   addProtectedField("play", TypeBool, Offset(mPlay, SoundComponent),
      &_autoplay, &defaultProtectedGetFn, MaxSoundThreads, "Whether playback of the emitter's sound should start as soon as the emitter object is added to the level.\n"
      "If this is true, the emitter will immediately start to play when the level is loaded.");
   //endArray("Sounds");
   Parent::initPersistFields();
}

bool SoundComponent::_previewSound(void *object, const char *index, const char *data)
{
   U32 slotNum = (index != NULL) ? dAtoui(index) : 0;
   SoundComponent* component = reinterpret_cast< SoundComponent* >(object);
   if (!component->mPreviewSound[slotNum])
      component->playAudio(slotNum, component->mSoundFile[slotNum]);
   else
      component->stopAudio(slotNum);
   component->mPreviewSound[slotNum] = !component->mPreviewSound[slotNum];

   return false;
}

bool SoundComponent::_autoplay(void *object, const char *index, const char *data)
{
   U32 slotNum = (index != NULL) ? dAtoui(index) : 0;
   SoundComponent* component = reinterpret_cast< SoundComponent* >(object);
   component->mPlay[slotNum] = dAtoui(data);
   if (component->mPlay[slotNum])
      component->playAudio(slotNum, component->mSoundFile[slotNum]);
   else
      component->stopAudio(slotNum);

   return false;
}

U32 SoundComponent::packUpdate(NetConnection *con, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   if (mask & InitialUpdateMask)
   {
      // mask off sounds that aren't playing
      S32 slotNum;
      for (slotNum = 0; slotNum < MaxSoundThreads; slotNum++)
         if (!mSoundThread[slotNum].play)
            mask &= ~(SoundMaskN << slotNum);
   }

   for (S32 slotNum = 0; slotNum < MaxSoundThreads; slotNum++)
      stream->writeFlag(mPreviewSound[slotNum]);

   if (stream->writeFlag(mask & SoundMask)) 
   {
      for (S32 slotNum = 0; slotNum < MaxSoundThreads; slotNum++) 
      {
         Sound& st = mSoundThread[slotNum];

         if (stream->writeFlag(mask & (SoundMaskN << slotNum)))
         {
            if (stream->writeFlag(st.play))
               //stream->writeRangedU32(st.profile->getId(), DataBlockObjectIdFirst,
               //   DataBlockObjectIdLast);
               stream->writeString(st.profile->getName());

         }
      }
   }

   return retMask;
}

void SoundComponent::unpackUpdate(NetConnection *con, BitStream *stream)
{
   Parent::unpackUpdate(con, stream);

   for (S32 slotNum = 0; slotNum < MaxSoundThreads; slotNum++)
      mPreviewSound[slotNum] = stream->readFlag();

   if (stream->readFlag())
   {
      for (S32 slotNum = 0; slotNum < MaxSoundThreads; slotNum++)
      {
         if (stream->readFlag())
         {
            Sound& st = mSoundThread[slotNum];
            st.play = stream->readFlag();
            if (st.play)
            {
               //st.profile = (SFXTrack*)stream->readRangedU32(DataBlockObjectIdFirst,
               //   DataBlockObjectIdLast);
               char profileName[255];
               stream->readString(profileName);

               if (!Sim::findObject(profileName, st.profile))
                  Con::errorf("Could not find SFXTrack");
            }

            //if (isProperlyAdded())
               updateAudioState(st);
         }
      }
   }
}

//This allows custom behavior in the event the owner is being edited
void SoundComponent::onInspect()
{
}

//This allows cleanup of the custom editor behavior if our owner stopped being edited
void SoundComponent::onEndInspect()
{
}

//Process tick update function, natch
void SoundComponent::processTick()
{
   Parent::processTick();
}

//Client-side advance function
void SoundComponent::advanceTime(F32 dt)
{

}

//Client-side interpolation function
void SoundComponent::interpolateTick(F32 delta)
{

}

void SoundComponent::prepRenderImage(SceneRenderState *state)
{
   if (!mEnabled || !mOwner || !gEditingMission)
      return;
   ObjectRenderInst* ri = state->getRenderPass()->allocInst< ObjectRenderInst >();

   ri->renderDelegate.bind(this, &SoundComponent::_renderObject);
   ri->type = RenderPassManager::RIT_Editor;
   ri->defaultKey = 0;
   ri->defaultKey2 = 0;

   state->getRenderPass()->addInst(ri);
}

void SoundComponent::_renderObject(ObjectRenderInst *ri,
   SceneRenderState *state,
   BaseMatInstance *overrideMat)
{
   if (overrideMat)
      return;

   GFXStateBlockDesc desc;
   desc.setBlend(true);

   MatrixF camera = GFX->getWorldMatrix();
   camera.inverse();
   Point3F pos = mOwner->getPosition();

   for (S32 slotNum = 0; slotNum < MaxSoundThreads; slotNum++)
   {
      if (mPreviewSound[slotNum])
      {
         Sound& st = mSoundThread[slotNum];
         if (st.sound && st.sound->getDescription())
         {
            F32 minRad = st.sound->getDescription()->mMinDistance;
            F32 falloffRad = st.sound->getDescription()->mMaxDistance;
            SphereF sphere(pos, falloffRad);
            if (sphere.isContained(camera.getPosition()))
               desc.setCullMode(GFXCullNone);

            GFX->getDrawUtil()->drawSphere(desc, minRad, pos, ColorI(255, 0, 255, 64));
            GFX->getDrawUtil()->drawSphere(desc, falloffRad, pos, ColorI(128, 0, 128, 64));
         }
      }
   }
}

void SoundComponent::playAudio(U32 slotNum, SFXTrack* _profile)
{
   AssertFatal(slotNum < MaxSoundThreads, "ShapeBase::playAudio() bad slot index");
   SFXTrack* profile = (_profile != NULL) ? _profile : mSoundFile[slotNum];
   Sound& st = mSoundThread[slotNum];
   if (profile && (!st.play || st.profile != profile))
   {
      setMaskBits(SoundMaskN << slotNum);
      st.play = true;
      st.profile = profile;
      updateAudioState(st);
   }
}

void SoundComponent::stopAudio(U32 slotNum)
{
   AssertFatal(slotNum < MaxSoundThreads, "ShapeBase::stopAudio() bad slot index");

   Sound& st = mSoundThread[slotNum];
   if (st.play)
   {
      st.play = false;
      setMaskBits(SoundMaskN << slotNum);
      updateAudioState(st);
   }
}

void SoundComponent::updateServerAudio()
{
   // Timeout non-looping sounds
   for (S32 slotNum = 0; slotNum < MaxSoundThreads; slotNum++) 
   {
      Sound& st = mSoundThread[slotNum];
      if (st.play && st.timeout && st.timeout < Sim::getCurrentTime()) 
      {
         //clearMaskBits(SoundMaskN << slotNum);
         st.play = false;
      }
   }
}

void SoundComponent::updateAudioState(Sound& st)
{
   SFX_DELETE(st.sound);

   if (st.play && st.profile)
   {
      if (isClientObject())
      {
         //if (Sim::findObject(SimObjectId((uintptr_t)st.profile), st.profile))
        // {
            MatrixF transform = mOwner->getTransform();
            st.sound = SFX->createSource(st.profile, &transform);
            if (st.sound)
               st.sound->play();
         //}
         else
            st.play = false;
      }
      else
      {
         // Non-looping sounds timeout on the server
         st.timeout = 0;
         if (!st.profile->getDescription()->mIsLooping)
            st.timeout = Sim::getCurrentTime() + sAudioTimeout;
      }
   }
   else
      st.play = false;
}

void SoundComponent::updateAudioPos()
{
   for (S32 slotNum = 0; slotNum < MaxSoundThreads; slotNum++)
   {
      SFXSource* source = mSoundThread[slotNum].sound;
      if (source)
         source->setTransform(mOwner->getTransform());
   }
}

//----------------------------------------------------------------------------
DefineEngineMethod(SoundComponent, playAudio, bool, (S32 slot, SFXTrack* track), (0, nullAsType<SFXTrack*>()),
   "@brief Attach a sound to this shape and start playing it.\n\n"

   "@param slot Audio slot index for the sound (valid range is 0 - 3)\n" // 3 = ShapeBase::MaxSoundThreads-1
   "@param track SFXTrack to play\n"
   "@return true if the sound was attached successfully, false if failed\n\n"

   "@see stopAudio()\n")
{
   if (track && slot >= 0 && slot < SoundComponent::MaxSoundThreads) {
      object->playAudio(slot, track);
      return true;
   }
   return false;
}

DefineEngineMethod(SoundComponent, stopAudio, bool, (S32 slot), ,
   "@brief Stop a sound started with playAudio.\n\n"

   "@param slot audio slot index (started with playAudio)\n"
   "@return true if the sound was stopped successfully, false if failed\n\n"

   "@see playAudio()\n")
{
   if (slot >= 0 && slot < SoundComponent::MaxSoundThreads) {
      object->stopAudio(slot);
      return true;
   }
   return false;
}