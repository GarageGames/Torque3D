
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

#include <typeinfo>
#include "afx/arcaneFX.h"

#include "T3D/spotLight.h"

#include "afx/ce/afxSpotLight_T3D.h"
#include "afx/afxEffectDefs.h"
#include "afx/afxEffectWrapper.h"
#include "afx/afxChoreographer.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxEA_T3DSpotLight 

class SpotLightProxy;

class afxEA_T3DSpotLight : public afxEffectWrapper
{
  typedef afxEffectWrapper Parent;

  afxT3DSpotLightData* light_data;
  SpotLightProxy*  light;

  void              do_runtime_substitutions();

public:
  /*C*/             afxEA_T3DSpotLight();
  /*D*/             ~afxEA_T3DSpotLight();

  virtual void      ea_set_datablock(SimDataBlock*);
  virtual bool      ea_start();
  virtual bool      ea_update(F32 dt);
  virtual void      ea_finish(bool was_stopped);
  virtual void      ea_set_scope_status(bool flag);
  virtual void      onDeleteNotify(SimObject*);
  virtual void      getBaseColor(ColorF& color);

  virtual bool      ea_is_enabled() { return true; }
};

//~~~~~~~~~~~~~~~~~~~~//

class SpotLightProxy : public SpotLight
{
  F32 fade_amt;

public:
  SpotLightProxy() { fade_amt = 1.0f; }

  void force_ghost() 
  {
    mNetFlags.clear(Ghostable | ScopeAlways);
    mNetFlags.set(IsGhost);
  }

  void setFadeAmount(F32 fade_amt)
  {
    this->fade_amt = fade_amt;
    mLight->setBrightness(mBrightness*fade_amt);
  }

  void updateTransform(const MatrixF& xfm)
  {
    mLight->setTransform(xfm);
    LightBase::setTransform(xfm);
  }

  void initWithDataBlock(const afxT3DSpotLightData* db)
  {
    mRange = getMax(db->mRange, 0.05f);
    mInnerConeAngle = db->mInnerConeAngle;
    mOuterConeAngle = db->mOuterConeAngle;

    mColor = db->mColor;
    mBrightness = db->mBrightness;
    mCastShadows = db->mCastShadows;
    mPriority = db->mPriority;
    mFlareData = db->mFlareData;
    mAnimationData = db->mAnimationData;
    mAnimState.active = (mAnimationData != 0);

    mLocalRenderViz = db->mLocalRenderViz;

    mLight->setType( LightInfo::Spot );
    mLight->setBrightness( db->mBrightness );
    mLight->setRange( db->mRange );
    mLight->setColor( db->mColor );
    mLight->setCastShadows( db->mCastShadows );
    mLight->setPriority( db->mPriority );
    mLight->setInnerConeAngle( db->mInnerConeAngle );
    mLight->setOuterConeAngle( db->mOuterConeAngle );

    // Update the bounds and scale to fit our light.
    F32 radius = mRange * mSin( mDegToRad( mOuterConeAngle ) * 0.5f );
    mObjBox.minExtents.set( -1, -1, -1 );
    mObjBox.maxExtents.set( 1, 1, 1 );
    mObjScale.set( radius, mRange, radius );

    //_conformLights();
  }

  void setLiveColor(const ColorF& live_color)
  {
    mLight->setColor(live_color);
  }

  void submitLights(LightManager* lm, bool staticLighting)
  {
    if (mAnimState.active && mAnimationData && fade_amt < 1.0f)
    {
      F32 mBrightness_save = mBrightness;
      mBrightness *= fade_amt;
      SpotLight::submitLights(lm, staticLighting);
      mBrightness = mBrightness_save;
      return;
    }

    SpotLight::submitLights(lm, staticLighting);
  }
};

//~~~~~~~~~~~~~~~~~~~~//

afxEA_T3DSpotLight::afxEA_T3DSpotLight()
{
  light_data = 0;
  light = 0;
}

afxEA_T3DSpotLight::~afxEA_T3DSpotLight()
{
  if (light)
    light->deleteObject();
  if (light_data && light_data->isTempClone())
    delete light_data;
  light_data = 0;
}

void afxEA_T3DSpotLight::ea_set_datablock(SimDataBlock* db)
{
  light_data = dynamic_cast<afxT3DSpotLightData*>(db);
}

bool afxEA_T3DSpotLight::ea_start()
{
  if (!light_data)
  {
    Con::errorf("afxEA_T3DSpotLight::ea_start() -- missing or incompatible datablock.");
    return false;
  }

  do_runtime_substitutions();

  // create and register effect
  light = new SpotLightProxy();
  light->force_ghost();
  if (!light->registerObject())
  {
    delete light;
    light = 0;
    Con::errorf("afxEA_T3DSpotLight::ea_update() -- effect failed to register.");
    return false;
  }
  deleteNotify(light);

  light->initWithDataBlock(light_data);

  return true;
}

bool afxEA_T3DSpotLight::ea_update(F32 dt)
{
  if (light)
  {
#if 0 // AFX_T3D_DISABLED
    // With sgLightObject lights, the following code block would hook
    // the constraint object up to the light in case the light was 
    // configured to exclude it from flare occusions. The code remains
    // here in case we need to implement the same feature for T3D light.

    afxConstraint* pos_cons = getPosConstraint();
    SceneObject* cons_obj = (pos_cons) ? pos_cons->getSceneObject() : 0;
    light->setConstraintObject(cons_obj);
#endif

    light->setLiveColor(updated_color);

    if (do_fades)
      light->setFadeAmount(fade_value);

    light->updateTransform(updated_xfm);

    // scale should not be updated this way. It messes up the culling.
    //light->setScale(updated_scale);
  }

  return true;
}

void afxEA_T3DSpotLight::ea_finish(bool was_stopped)
{
  if (light)
  {
    light->deleteObject();
    light = 0;
  }
}

void afxEA_T3DSpotLight::ea_set_scope_status(bool in_scope)
{
  if (light)
    light->setLightEnabled(in_scope);
}

void afxEA_T3DSpotLight::onDeleteNotify(SimObject* obj)
{
  if (light == dynamic_cast<SpotLight*>(obj))
    light = 0;

  Parent::onDeleteNotify(obj);
}

void afxEA_T3DSpotLight::getBaseColor(ColorF& color)
{ 
  if (light_data) 
    color = light_data->mColor; 
}

void afxEA_T3DSpotLight::do_runtime_substitutions()
{
  // only clone the datablock if there are substitutions
  if (light_data->getSubstitutionCount() > 0)
  {
    // clone the datablock and perform substitutions
    afxT3DSpotLightData* orig_db = light_data;
    light_data = new afxT3DSpotLightData(*orig_db, true);
    orig_db->performSubstitutions(light_data, choreographer, group_index);
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxEA_T3DSpotLightDesc : public afxEffectAdapterDesc, public afxEffectDefs 
{
  static afxEA_T3DSpotLightDesc desc;

public:
  virtual bool  testEffectType(const SimDataBlock*) const;
  virtual bool  requiresStop(const afxEffectWrapperData*, const afxEffectTimingData&) const;
  virtual bool  runsOnServer(const afxEffectWrapperData*) const { return false; }
  virtual bool  runsOnClient(const afxEffectWrapperData*) const { return true; }

  virtual afxEffectWrapper* create() const { return new afxEA_T3DSpotLight; }
};

afxEA_T3DSpotLightDesc afxEA_T3DSpotLightDesc::desc;

bool afxEA_T3DSpotLightDesc::testEffectType(const SimDataBlock* db) const
{
  return (typeid(afxT3DSpotLightData) == typeid(*db));
}

bool afxEA_T3DSpotLightDesc::requiresStop(const afxEffectWrapperData* ew, const afxEffectTimingData& timing) const
{
  return (timing.lifetime < 0);
}


//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//