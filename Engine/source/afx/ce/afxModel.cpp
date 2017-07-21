
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

#include "T3D/objectTypes.h"
#include "T3D/gameBase/gameProcess.h"
#include "core/resourceManager.h"
#include "sim/netConnection.h"
#include "scene/sceneRenderState.h"
#include "scene/sceneManager.h"
#include "ts/tsShapeInstance.h"
#include "ts/tsMaterialList.h"

#include "afx/ce/afxModel.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxModelData

IMPLEMENT_CO_DATABLOCK_V1(afxModelData);
 
ConsoleDocClass( afxModelData,
   "@brief A datablock that specifies a Model effect.\n\n"

   "A Model effect is a lightweight client-only geometry object useful for effect-driven props."
   "\n\n"

   "@ingroup afxEffects\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxModelData::afxModelData()
{
  shapeName = ST_NULLSTRING;
  sequence = ST_NULLSTRING;
  seq_rate = 1.0f;
  seq_offset = 0.0f;
  alpha_mult = 1.0f;
  use_vertex_alpha = false;
  force_on_material_flags = 0;
  force_off_material_flags = 0;
  texture_filtering = true;
  fog_mult = 1.0f;
  remap_txr_tags = ST_NULLSTRING;
  remap_buffer = 0;

  overrideLightingOptions = false;
  receiveSunLight = true;
  receiveLMLighting = true;
  useAdaptiveSelfIllumination = false;
  useCustomAmbientLighting = false;
  customAmbientForSelfIllumination = false;
  customAmbientLighting = ColorF(0.0f, 0.0f, 0.0f);
  shadowEnable = false;

  shadowSize = 128;
  shadowMaxVisibleDistance = 80.0f;
  shadowProjectionDistance = 10.0f;
  shadowSphereAdjust = 1.0;
}

afxModelData::afxModelData(const afxModelData& other, bool temp_clone) : GameBaseData(other, temp_clone)
{
  shapeName = other.shapeName;
  shape = other.shape; // --
  sequence = other.sequence;
  seq_rate = other.seq_rate;
  seq_offset = other.seq_offset;
  alpha_mult = other.alpha_mult;
  use_vertex_alpha = other.use_vertex_alpha;
  force_on_material_flags = other.force_on_material_flags;
  force_off_material_flags = other.force_off_material_flags;
  texture_filtering = other.texture_filtering;
  fog_mult = other.fog_mult;
  remap_txr_tags = other.remap_txr_tags;
  remap_buffer = other.remap_buffer;
  overrideLightingOptions = other.overrideLightingOptions;
  receiveSunLight = other.receiveSunLight;
  receiveLMLighting = other.receiveLMLighting;
  useAdaptiveSelfIllumination = other.useAdaptiveSelfIllumination;
  useCustomAmbientLighting = other.useCustomAmbientLighting;
  customAmbientForSelfIllumination = other.customAmbientForSelfIllumination;
  customAmbientLighting = other.customAmbientLighting;
  shadowEnable = other.shadowEnable;
}

afxModelData::~afxModelData()
{
   if (remap_buffer)
      dFree(remap_buffer);
}

bool afxModelData::preload(bool server, String &errorStr)
{
  if (Parent::preload(server, errorStr) == false)
    return false;
  
  // don't need to do this stuff on the server
  if (server) 
    return true;
  
  if (shapeName != ST_NULLSTRING && !shape)
  {
    shape = ResourceManager::get().load(shapeName);
    if (!shape)
    {
      errorStr = String::ToString("afxModelData::load: Failed to load shape \"%s\"", shapeName);
      return false;
    }

    // just parse up the string and collect the remappings in txr_tag_remappings.
    if (remap_txr_tags != ST_NULLSTRING)
    {
       txr_tag_remappings.clear();
       if (remap_buffer)
          dFree(remap_buffer);

       remap_buffer = dStrdup(remap_txr_tags);

       char* remap_token = dStrtok(remap_buffer, " \t");
       while (remap_token != NULL)
       {
          char* colon = dStrchr(remap_token, ':');
          if (colon)
          {
             *colon = '\0';
             txr_tag_remappings.increment();
             txr_tag_remappings.last().old_tag = remap_token;
             txr_tag_remappings.last().new_tag = colon+1;
          }
          remap_token = dStrtok(NULL, " \t");
       }
    }

    // this little hack messes things up when remapping texture tags
    if (txr_tag_remappings.size() == 0)
    {
      // this little hack forces the textures to preload
      TSShapeInstance* pDummy = new TSShapeInstance(shape);
      delete pDummy;
    }
  }

  return true;
}

#define myOffset(field) Offset(field, afxModelData)

void afxModelData::initPersistFields()
{
  addField("shapeFile",             TypeFilename, myOffset(shapeName),
    "The name of a .dts format file to use for the model.");
  addField("sequence",              TypeFilename, myOffset(sequence),
    "The name of an animation sequence to play in the model.");
  addField("sequenceRate",          TypeF32,      myOffset(seq_rate),
    "The rate of playback for the sequence.");
  addField("sequenceOffset",        TypeF32,      myOffset(seq_offset),
    "An offset in seconds indicating a starting point for the animation sequence "
    "specified by the sequence field. A rate of 1.0 (rather than sequenceRate) is used "
    "to convert from seconds to the thread offset.");
  addField("alphaMult",             TypeF32,      myOffset(alpha_mult),
    "An alpha multiplier used to set maximum opacity of the model.");

  addField("fogMult",               TypeF32,      myOffset(fog_mult),
    "");
  addField("remapTextureTags",      TypeString,   myOffset(remap_txr_tags),
    "Rename one or more texture tags in the model. Texture tags are what link a "
    "model's textures to materials.\n"
    "Field should be a string containing space-separated remapping tokens. A remapping "
    "token is two names separated by a colon, ':'. The first name should be a texture-tag "
    "that exists in the model, while the second is a new name to replace it. The string "
    "can have any number of remapping tokens as long as the total string length does not "
    "exceed 255.");
  addField("shadowEnable",                  TypeBool,   myOffset(shadowEnable),
    "Sets whether the model casts a shadow.");

  addField("useVertexAlpha",        TypeBool,     myOffset(use_vertex_alpha),
    "deprecated");
  addField("forceOnMaterialFlags",  TypeS32,      myOffset(force_on_material_flags),
    "deprecated");
  addField("forceOffMaterialFlags", TypeS32,      myOffset(force_off_material_flags),
    "deprecated");
  addField("textureFiltering",      TypeBool,     myOffset(texture_filtering),
    "deprecated");
  addField("overrideLightingOptions",       TypeBool,   myOffset(overrideLightingOptions),
    "deprecated");
  addField("receiveSunLight",               TypeBool,   myOffset(receiveSunLight),
    "");
  addField("receiveLMLighting",             TypeBool,   myOffset(receiveLMLighting),
    "deprecated");
  addField("useAdaptiveSelfIllumination",   TypeBool,   myOffset(useAdaptiveSelfIllumination),
    "deprecated");
  addField("useCustomAmbientLighting",      TypeBool,   myOffset(useCustomAmbientLighting),
    "deprecated");
  addField("customAmbientSelfIllumination", TypeBool,   myOffset(customAmbientForSelfIllumination),
    "deprecated");
  addField("customAmbientLighting",         TypeColorF, myOffset(customAmbientLighting),
    "deprecated");
  addField("shadowSize",                    TypeS32,    myOffset(shadowSize),
    "deprecated");
  addField("shadowMaxVisibleDistance",      TypeF32,    myOffset(shadowMaxVisibleDistance),
    "deprecated");
  addField("shadowProjectionDistance",      TypeF32,    myOffset(shadowProjectionDistance),
    "deprecated");
  addField("shadowSphereAdjust",            TypeF32,    myOffset(shadowSphereAdjust),
    "deprecated");

  Parent::initPersistFields();

  // Material Flags
  Con::setIntVariable("$MaterialFlags::S_Wrap",              TSMaterialList::S_Wrap);
  Con::setIntVariable("$MaterialFlags::T_Wrap",              TSMaterialList::T_Wrap);
  Con::setIntVariable("$MaterialFlags::Translucent",         TSMaterialList::Translucent);
  Con::setIntVariable("$MaterialFlags::Additive",            TSMaterialList::Additive);
  Con::setIntVariable("$MaterialFlags::Subtractive",         TSMaterialList::Subtractive);
  Con::setIntVariable("$MaterialFlags::SelfIlluminating",    TSMaterialList::SelfIlluminating);
  Con::setIntVariable("$MaterialFlags::NeverEnvMap",         TSMaterialList::NeverEnvMap);
  Con::setIntVariable("$MaterialFlags::NoMipMap",            TSMaterialList::NoMipMap);
  Con::setIntVariable("$MaterialFlags::MipMap_ZeroBorder",   TSMaterialList::MipMap_ZeroBorder);
  Con::setIntVariable("$MaterialFlags::AuxiliaryMap",        TSMaterialList::AuxiliaryMap);

#if defined(AFX_CAP_AFXMODEL_TYPE)
  Con::setIntVariable("$TypeMasks::afxModelObjectType",      afxModelObjectType);
#endif
}

void afxModelData::packData(BitStream* stream)
{
  Parent::packData(stream);

  stream->writeString(shapeName);
  stream->writeString(sequence);
  stream->write(seq_rate);  
  stream->write(seq_offset);
  stream->write(alpha_mult); 
  stream->write(use_vertex_alpha); 
  stream->write(force_on_material_flags);
  stream->write(force_off_material_flags);
  stream->writeFlag(texture_filtering);
  stream->write(fog_mult);

  stream->writeString(remap_txr_tags);

  stream->writeFlag(overrideLightingOptions);
  stream->writeFlag(receiveSunLight);
  stream->writeFlag(useAdaptiveSelfIllumination);
  stream->writeFlag(useCustomAmbientLighting);
  stream->writeFlag(customAmbientForSelfIllumination);
  stream->write(customAmbientLighting);
  stream->writeFlag(receiveLMLighting);
  stream->writeFlag(shadowEnable);

  stream->write(shadowSize);
  stream->write(shadowMaxVisibleDistance);
  stream->write(shadowProjectionDistance);
  stream->write(shadowSphereAdjust);
}

void afxModelData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);

  shapeName = stream->readSTString();
  sequence = stream->readSTString();
  stream->read(&seq_rate);
  stream->read(&seq_offset);
  stream->read(&alpha_mult);
  stream->read(&use_vertex_alpha);
  stream->read(&force_on_material_flags);
  stream->read(&force_off_material_flags);
  texture_filtering = stream->readFlag();
  stream->read(&fog_mult);

  remap_txr_tags = stream->readSTString();

  overrideLightingOptions = stream->readFlag();
  receiveSunLight = stream->readFlag();
  useAdaptiveSelfIllumination = stream->readFlag();
  useCustomAmbientLighting = stream->readFlag();
  customAmbientForSelfIllumination = stream->readFlag();
  stream->read(&customAmbientLighting);
  receiveLMLighting = stream->readFlag();
  shadowEnable = stream->readFlag();

  stream->read(&shadowSize);
  stream->read(&shadowMaxVisibleDistance);
  stream->read(&shadowProjectionDistance);
  stream->read(&shadowSphereAdjust);
}

void afxModelData::onPerformSubstitutions() 
{ 
  if (shapeName != ST_NULLSTRING)
  {
    shape = ResourceManager::get().load(shapeName);
    if (!shape)
    {
      Con::errorf("afxModelData::onPerformSubstitutions: Failed to load shape \"%s\"", shapeName);
      return;
    }

    // REMAP-TEXTURE-TAGS ISSUES?
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxModel

IMPLEMENT_CO_NETOBJECT_V1(afxModel);

ConsoleDocClass( afxModel,
   "@brief A Model effect as defined by an afxModelData datablock.\n\n"

   "A Model effect is a lightweight client-only geometry object useful for effect-driven "
   "props.\n"

   "@ingroup afxEffects\n"
   "@ingroup AFX\n"
);

afxModel::afxModel()
{
  mTypeMask |= DynamicShapeObjectType;
#if defined(AFX_CAP_AFXMODEL_TYPE)
  mTypeMask |= afxModelObjectType;
#endif

  shape_inst = 0;

  main_seq_thread = 0;
  main_seq_id = -1;
  seq_rate_factor = 1.0f;
  last_anim_tag = 0;

  seq_animates_vis = false;
  fade_amt = 1.0f;
  is_visible = true;
  sort_priority = 0;

  mNetFlags.set( IsGhost );
}

afxModel::~afxModel()
{
  delete shape_inst;
}

void afxModel::setSequenceRateFactor(F32 factor)
{
  seq_rate_factor = factor;
  if (shape_inst != NULL && main_seq_thread != NULL)
    shape_inst->setTimeScale(main_seq_thread, seq_rate_factor*mDataBlock->seq_rate);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

bool afxModel::onNewDataBlock(GameBaseData* dptr, bool reload)
{
  mDataBlock = dynamic_cast<afxModelData*>(dptr);
  if (!mDataBlock || !Parent::onNewDataBlock(dptr, reload))
    return false;

  return true;
}

bool afxModel::onAdd()
{
  // first check if we have a server connection, if we don't then this is on the server
  // and we should exit, then check if the parent fails to add the object
  NetConnection* conn = NetConnection::getConnectionToServer();
  if (!conn || !Parent::onAdd())
    return false;

  // setup our bounding box
  if (mDataBlock->shape)
    mObjBox = mDataBlock->shape->bounds;
  else
    mObjBox = Box3F(Point3F(-1, -1, -1), Point3F(1, 1, 1));

  // setup the shape instance and sequence
  if (mDataBlock->shape)
  {
     if (/*isClientObject() && */mDataBlock->txr_tag_remappings.size() > 0)
     {
        // temporarily substitute material tags with alternates
        TSMaterialList* mat_list = mDataBlock->shape->materialList;
        if (mat_list)
        {
           for (S32 i = 0; i < mDataBlock->txr_tag_remappings.size(); i++)
           {
              afxModelData::TextureTagRemapping* remap = &mDataBlock->txr_tag_remappings[i];
              Vector<String> & mat_names = (Vector<String>&) mat_list->getMaterialNameList();
              for (S32 j = 0; j < mat_names.size(); j++) 
              {
                 if (mat_names[j].compare(remap->old_tag, dStrlen(remap->old_tag), String::NoCase) == 0)
                 {
                    //Con::printf("REMAP TEXTURE TAG [%s] TO [%s]", remap->old_tag, remap->new_tag);
                    mat_names[j] = String(remap->new_tag);
                    mat_names[j].insert(0,'#');
                    break;
                 }
              }
           }
        }
     }

    shape_inst = new TSShapeInstance(mDataBlock->shape);

    if (true) // isClientObject())
    {
       shape_inst->cloneMaterialList();

       // restore the material tags to original form
       if (mDataBlock->txr_tag_remappings.size() > 0)
       {
          TSMaterialList* mat_list = mDataBlock->shape->materialList;
          if (mat_list)
          {
             for (S32 i = 0; i < mDataBlock->txr_tag_remappings.size(); i++)
             {
                afxModelData::TextureTagRemapping* remap = &mDataBlock->txr_tag_remappings[i];
                Vector<String> & mat_names = (Vector<String>&) mat_list->getMaterialNameList();
                for (S32 j = 0; j < mat_names.size(); j++) 
                {
                   if (mat_names[j].compare(remap->new_tag, dStrlen(remap->new_tag)) == 0)
                   {
                      //Con::printf("UNREMAP TEXTURE TAG [%s] TO [%s]", remap->new_tag, remap->old_tag);
                      mat_names[j] = String(remap->old_tag);
                      break;
                   }
                }
             }
          }
       }
    }

    if (mDataBlock->sequence == ST_NULLSTRING)
    {
      main_seq_thread = 0;
      main_seq_id = -1;
    }
    else
    {
      // here we start the default animation sequence
      TSShape* shape = shape_inst->getShape();
      main_seq_id = shape->findSequence(mDataBlock->sequence);
      if (main_seq_id != -1)
      {      
        main_seq_thread = shape_inst->addThread();
      
        F32 seq_pos = 0.0f;
        if (mDataBlock->seq_offset > 0.0f && mDataBlock->seq_offset < shape_inst->getDuration(main_seq_thread))
          seq_pos = mDataBlock->seq_offset / shape_inst->getDuration(main_seq_thread);

        shape_inst->setTimeScale(main_seq_thread, seq_rate_factor*mDataBlock->seq_rate);
        shape_inst->setSequence(main_seq_thread, main_seq_id, seq_pos);
        seq_animates_vis = shape->sequences[main_seq_id].visMatters.testAll();
      }
    }

    // deal with material changes
    if (shape_inst && (mDataBlock->force_on_material_flags | mDataBlock->force_off_material_flags))
    {
      shape_inst->cloneMaterialList();
      TSMaterialList* mats = shape_inst->getMaterialList();
      if (mDataBlock->force_on_material_flags != 0)
      {
        for (U32 i = 0; i < mats->size(); i++)
          mats->setFlags(i, mats->getFlags(i) | mDataBlock->force_on_material_flags);
      }

      if (mDataBlock->force_off_material_flags != 0)
      {
        for (U32 i = 0; i < mats->size(); i++)
          mats->setFlags(i, mats->getFlags(i) & ~mDataBlock->force_off_material_flags);
      }
    }
  }

  resetWorldBox();

  if (mDataBlock->shape)
  {
    // Scan out the collision hulls...
    static const String sCollisionStr( "collision-" );

    for (U32 i = 0; i < mDataBlock->shape->details.size(); i++)
    {
      const String &name = mDataBlock->shape->names[mDataBlock->shape->details[i].nameIndex];

      if (name.compare( sCollisionStr, sCollisionStr.length(), String::NoCase ) == 0)
      {
        mCollisionDetails.push_back(i);

        // The way LOS works is that it will check to see if there is a LOS detail that matches
        // the the collision detail + 1 + MaxCollisionShapes (this variable name should change in
        // the future). If it can't find a matching LOS it will simply use the collision instead.
        // We check for any "unmatched" LOS's further down
        mLOSDetails.increment();

        char buff[128];
        dSprintf(buff, sizeof(buff), "LOS-%d", i + 1 + 8/*MaxCollisionShapes*/);
        U32 los = mDataBlock->shape->findDetail(buff);
        if (los == -1)
          mLOSDetails.last() = i;
        else
          mLOSDetails.last() = los;
      }
    }

    // Snag any "unmatched" LOS details
    static const String sLOSStr( "LOS-" );

    for (U32 i = 0; i < mDataBlock->shape->details.size(); i++)
    {
      const String &name = mDataBlock->shape->names[mDataBlock->shape->details[i].nameIndex];

      if (name.compare( sLOSStr, sLOSStr.length(), String::NoCase ) == 0)
      {
        // See if we already have this LOS
        bool found = false;
        for (U32 j = 0; j < mLOSDetails.size(); j++)
        {
          if (mLOSDetails[j] == i)
          {
            found = true;
            break;
          }
        }

        if (!found)
          mLOSDetails.push_back(i);
      }
    }

    // Compute the hull accelerators (actually, just force the shape to compute them)
    for (U32 i = 0; i < mCollisionDetails.size(); i++)
      shape_inst->getShape()->getAccelerator(mCollisionDetails[i]);
  }

  // tell engine the model exists
  gClientSceneGraph->addObjectToScene(this);
  removeFromProcessList();
  ClientProcessList::get()->addObject(this);
  conn->addObject(this);

  return true;
}

void afxModel::onRemove()
{
  mSceneManager->removeObjectFromScene(this);
  getContainer()->removeObject(this);
  Parent::onRemove();
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

void afxModel::advanceTime(F32 dt)
{
  if (main_seq_thread)
    shape_inst->advanceTime(dt, main_seq_thread);

  for (S32 i = 0; i < blend_clips.size(); i++)
    shape_inst->advanceTime(dt, blend_clips[i].thread);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

void afxModel::prepRenderImage(SceneRenderState* state)
{
  if (!is_visible || !shape_inst)
    return;
  
  // calculate distance to camera
  Point3F cameraOffset;
  getRenderTransform().getColumn(3, &cameraOffset);
  cameraOffset -= state->getCameraPosition();   
  F32 dist = cameraOffset.len();
  if (dist < 0.01f)
    dist = 0.01f;

  F32 invScale = (1.0f/getMax(getMax(mObjScale.x,mObjScale.y),mObjScale.z));
  shape_inst->setDetailFromDistance(state, dist*invScale);
  if ( shape_inst->getCurrentDetail() < 0 )
    return;

  renderObject(state);
}

bool afxModel::castRay(const Point3F &start, const Point3F &end, RayInfo* info)
{
  if (shape_inst)
  {
    RayInfo shortest;
    shortest.t = 1e8;

    info->object = NULL;
    if (mLOSDetails.size() > 0)
    {
      for (U32 i = 0; i < mLOSDetails.size(); i++)
      {
        shape_inst->animate(mLOSDetails[i]);
        if (shape_inst->castRay(start, end, info, mLOSDetails[i]))
        {
          info->object = this;
          if (info->t < shortest.t)
            shortest = *info;
        }
      }
    }
    else
    {
      if (mCollisionDetails.size() > 0)
      {
        for (U32 i = 0; i < mCollisionDetails.size(); i++)
        {
          shape_inst->animate(mCollisionDetails[i]);
          if (shape_inst->castRay(start, end, info, mCollisionDetails[i]))
          {
            info->object = this;
            if (info->t < shortest.t)
              shortest = *info;
          }
        }
      }
    }

    if (info->object == this) 
    {
      // Copy out the shortest time...
      *info = shortest;
      return true;
    }
  }

  return false;
}

U32 afxModel::unique_anim_tag_counter = 1;
#define BAD_ANIM_ID  999999999

U32 afxModel::setAnimClip(const char* clip, F32 pos, F32 rate, F32 trans)
{
  if (!shape_inst)
    return 0;

  TSShape* shape = shape_inst->getShape();

  S32 seq_id = shape->findSequence(clip);
  if (seq_id == -1)
  {
    Con::errorf("afxModel::setAnimClip() -- failed to find a sequence matching the name, \"%s\".", clip);
    return 0;
  }

  // JTF Note: test if this blend implementation is working
  if (shape->sequences[seq_id].isBlend())
  {
    BlendThread blend_clip;
    blend_clip.tag = ((unique_anim_tag_counter++) | 0x80000000);

    blend_clip.thread = shape_inst->addThread();
    shape_inst->setSequence(blend_clip.thread, seq_id, pos);
    shape_inst->setTimeScale(blend_clip.thread, rate);

    blend_clips.push_back(blend_clip);

    return blend_clip.tag;
  }

  if (!main_seq_thread)
  {
    main_seq_thread = shape_inst->addThread();
    shape_inst->setTimeScale(main_seq_thread, seq_rate_factor*rate);
    shape_inst->setSequence(main_seq_thread, seq_id, pos);
    seq_animates_vis = shape->sequences[seq_id].visMatters.testAll();
  }
  else
  {
    shape_inst->setTimeScale(main_seq_thread, seq_rate_factor*rate);

    F32 transTime = (trans < 0) ? 0.25 : trans;
    if (transTime > 0.0f)
      shape_inst->transitionToSequence(main_seq_thread, seq_id, pos, transTime, true);
    else
      shape_inst->setSequence(main_seq_thread, seq_id, pos);

    seq_animates_vis = shape->sequences[seq_id].visMatters.testAll();
  }

  last_anim_tag = unique_anim_tag_counter++;

  return last_anim_tag;
}

void afxModel::resetAnimation(U32 tag)
{
  // check if this is a blended clip
  if ((tag & 0x80000000) != 0)
  {
    for (S32 i = 0; i < blend_clips.size(); i++)
    {
      if (blend_clips[i].tag == tag)
      {
        if (blend_clips[i].thread)
        {
          //Con::printf("DESTROY THREAD %d of %d tag=%d" , i, blend_clips.size(), tag & 0x7fffffff);
          shape_inst->destroyThread(blend_clips[i].thread);
        }
        blend_clips.erase_fast(i);
        break;
      }
    }
    return;  
  }

  if (tag != 0 && tag == last_anim_tag)
  {
    // restore original non-animated state
    if (main_seq_id == -1)
    {
      shape_inst->destroyThread(main_seq_thread);
      main_seq_thread = 0;
    }
    // restore original sequence
    else
    {
      shape_inst->setTimeScale(main_seq_thread, seq_rate_factor*mDataBlock->seq_rate);
      shape_inst->transitionToSequence(main_seq_thread, main_seq_id , 0.0f, 0.25f, true);
    }
    last_anim_tag = 0;
  }
}

F32 afxModel::getAnimClipDuration(const char* clip)
{
  if (!shape_inst)
    return 0.0f;

  TSShape* shape = shape_inst->getShape();
  S32 seq_id = shape->findSequence(clip);
  return (seq_id != -1) ? shape->sequences[seq_id].duration : 0.0f;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
