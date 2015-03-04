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

#ifndef _COLLADA_EXTENSIONS_H_
#define _COLLADA_EXTENSIONS_H_

#ifndef _TSSHAPE_LOADER_H_
#include "ts/loader/tsShapeLoader.h"
#endif
#ifndef _COLLADA_UTILS_H_
#include "ts/collada/colladaUtils.h"
#endif

//-----------------------------------------------------------------------------
// Collada allows custom data to be included with many elements using the <extra>
// tag, followed by one or more named technique profiles. eg.
// <some_element>
//   <extra>
//     <technique profile="SOME_PROFILE">
//       <custom_element0>value0</custom_element0>
//       <custom_element1>value1</custom_element1>
//       ...
//     <technique profile="ANOTHER_PROFILE">
//       <custom_element0>value0</custom_element0>
//       <custom_element1>value1</custom_element1>
//       ...
//
// This class provides an easy way to read the custom parameters into a strongly
// typed subclass.
class ColladaExtension
{
   // Helper macro to simplify getting named parameters
   #define GET_EXTRA_PARAM(param, defaultVal)   \
      get(#param, param, defaultVal)

protected:
   const domTechnique* pTechnique;

   /// Find the technique with the named profile
   template<class T> const domTechnique* findExtraTechnique(const T* element, const char* name) const
   {
      if (element) {
         for (S32 iExt = 0; iExt < element->getExtra_array().getCount(); iExt++) {
            for (S32 iTech = 0; iTech < element->getExtra_array()[iExt]->getTechnique_array().getCount(); iTech++) {
               if (dStrEqual(element->getExtra_array()[iExt]->getTechnique_array()[iTech]->getProfile(), name))
                  return element->getExtra_array()[iExt]->getTechnique_array()[iTech];
            }
         }         
      }
      return NULL;
   }

   /// The <texture> element does not define an extra_array, so need a specialized
   /// version of the template
   const domTechnique* findExtraTechnique(
      const domCommon_color_or_texture_type_complexType::domTexture* element, const char* name) const
   {
      if (element && element->getExtra()) {
         for (S32 iTech = 0; iTech < element->getExtra()->getTechnique_array().getCount(); iTech++) {
            if (dStrEqual(element->getExtra()->getTechnique_array()[iTech]->getProfile(), name))
               return element->getExtra()->getTechnique_array()[iTech];
         }
      }
      return NULL;
   }

   /// Find the parameter with the given name
   const domAny* findParam(const char* name)
   {
      if (pTechnique) {
         // search the technique contents for the desired parameter
         for (S32 iParam = 0; iParam < pTechnique->getContents().getCount(); iParam++) {
            const domAny* param = daeSafeCast<domAny>(pTechnique->getContents()[iParam]);
            if (param && !dStrcmp(param->getElementName(), name))
               return param;
         }
      }
      return NULL;
   }

   /// Get the value of the named parameter (use defaultVal if parameter not found)
   template<typename T> void get(const char* name, T& value, T defaultVal)
   {
      value = defaultVal;
      if (const domAny* param = findParam(name))
         value = convert<T>(param->getValue());
   }

   /// Get the value of the named animated parameter (use defaultVal if parameter not found)
   template<typename T> void get(const char* name, AnimatedElement<T>& value, T defaultVal)
   {
      value.defaultVal = defaultVal;
      if (const domAny* param = findParam(name))
         value.element = param;
   }

public:
   ColladaExtension() : pTechnique(0) { }
   virtual ~ColladaExtension() { }
};

/// Extensions for the <effect> element (and its children)
class ColladaExtension_effect : public ColladaExtension
{
   // Cached texture transform
   F32            lastAnimTime;
   MatrixF        textureTransform;

public:
   //----------------------------------
   // <effect>
   // MAX3D profile elements
   bool double_sided;

   //----------------------------------
   // <effect>.<profile_COMMON>
   // GOOGLEEARTH profile elements
   //bool double_sided;

   //----------------------------------
   // <effect>.<profile_COMMON>.<technique>.<blinn/phong/lambert>.<diffuse>.<texture>
   // MAYA profile elements
   bool           wrapU, wrapV;
   bool           mirrorU, mirrorV;
   AnimatedFloat  coverageU, coverageV;
   AnimatedFloat  translateFrameU, translateFrameV;
   AnimatedFloat  rotateFrame;
   AnimatedBool   stagger;       // @todo: not supported yet
   AnimatedFloat  repeatU, repeatV;
   AnimatedFloat  offsetU, offsetV;
   AnimatedFloat  rotateUV;
   AnimatedFloat  noiseU, noiseV;

   //----------------------------------
   // <effect>.<profile_COMMON>.<technique>
   // FCOLLADA profile elements
   domFx_sampler2D_common_complexType*   bumpSampler;

public:
   ColladaExtension_effect(const domEffect* effect)
      : lastAnimTime(TSShapeLoader::DefaultTime-1), textureTransform(true), bumpSampler(0)
   {
      //----------------------------------
      // <effect>
      // MAX3D profile
      pTechnique = findExtraTechnique(effect, "MAX3D");
      GET_EXTRA_PARAM(double_sided, false);

      //----------------------------------
      // <effect>.<profile_COMMON>
      const domProfile_COMMON* profileCommon = ColladaUtils::findEffectCommonProfile(effect);

      // GOOGLEEARTH profile (same double_sided element)
      pTechnique = findExtraTechnique(profileCommon, "GOOGLEEARTH");
      GET_EXTRA_PARAM(double_sided, double_sided);

      //----------------------------------
      // <effect>.<profile_COMMON>.<technique>.<blinn/phong/lambert>.<diffuse>.<texture>
      const domCommon_color_or_texture_type_complexType* domDiffuse = ColladaUtils::findEffectDiffuse(effect);
      const domFx_sampler2D_common_complexType* sampler2D = ColladaUtils::getTextureSampler(effect, domDiffuse);

      // Use the sampler2D to set default values for wrap/mirror flags
      wrapU = wrapV = true;
      mirrorU = mirrorV = false;
      if (sampler2D) {
         domFx_sampler2D_common_complexType::domWrap_s* wrap_s = sampler2D->getWrap_s();
         domFx_sampler2D_common_complexType::domWrap_t* wrap_t = sampler2D->getWrap_t();

         mirrorU = (wrap_s && wrap_s->getValue() == FX_SAMPLER_WRAP_COMMON_MIRROR);
         wrapU = (mirrorU || !wrap_s || (wrap_s->getValue() == FX_SAMPLER_WRAP_COMMON_WRAP));
         mirrorV = (wrap_t && wrap_t->getValue() == FX_SAMPLER_WRAP_COMMON_MIRROR);
         wrapV = (mirrorV || !wrap_t || (wrap_t->getValue() == FX_SAMPLER_WRAP_COMMON_WRAP));
      }

      // MAYA profile
      pTechnique = findExtraTechnique(domDiffuse ? domDiffuse->getTexture() : 0, "MAYA");
      GET_EXTRA_PARAM(wrapU, wrapU);            GET_EXTRA_PARAM(wrapV, wrapV);
      GET_EXTRA_PARAM(mirrorU, mirrorU);        GET_EXTRA_PARAM(mirrorV, mirrorV);
      GET_EXTRA_PARAM(coverageU, 1.0);          GET_EXTRA_PARAM(coverageV, 1.0);
      GET_EXTRA_PARAM(translateFrameU, 0.0);    GET_EXTRA_PARAM(translateFrameV, 0.0);
      GET_EXTRA_PARAM(rotateFrame, 0.0);
      GET_EXTRA_PARAM(stagger, false);
      GET_EXTRA_PARAM(repeatU, 1.0);            GET_EXTRA_PARAM(repeatV, 1.0);
      GET_EXTRA_PARAM(offsetU, 0.0);            GET_EXTRA_PARAM(offsetV, 0.0);
      GET_EXTRA_PARAM(rotateUV, 0.0);
      GET_EXTRA_PARAM(noiseU, 0.0);             GET_EXTRA_PARAM(noiseV, 0.0);

      // FCOLLADA profile
      if (profileCommon) {
         pTechnique = findExtraTechnique((const domProfile_COMMON::domTechnique*)profileCommon->getTechnique(), "FCOLLADA");
         if (pTechnique) {
            domAny* bump = daeSafeCast<domAny>(const_cast<domTechnique*>(pTechnique)->getChild("bump"));
            if (bump) {
               domAny* bumpTexture = daeSafeCast<domAny>(bump->getChild("texture"));
               if (bumpTexture) {
                  daeSIDResolver resolver(const_cast<domEffect*>(effect), bumpTexture->getAttribute("texture").c_str());
                  domCommon_newparam_type* param = daeSafeCast<domCommon_newparam_type>(resolver.getElement());
                  if (param)
                     bumpSampler = param->getSampler2D();
               }
            }
         }
      }
   }

   /// Check if any of the MAYA texture transform elements are animated within
   /// the interval
   bool animatesTextureTransform(F32 start, F32 end);

   /// Apply the MAYA texture transform to the given UV coordinates
   void applyTextureTransform(Point2F& uv, F32 time);
};

/// Extensions for the <node> element
class ColladaExtension_node : public ColladaExtension
{
public:
   // FCOLLADA or OpenCOLLADA profile elements
   AnimatedFloat visibility;
   const char* user_properties;

   ColladaExtension_node(const domNode* node)
   {
      // FCOLLADA profile
      pTechnique = findExtraTechnique(node, "FCOLLADA");
      GET_EXTRA_PARAM(visibility, 1.0);
      GET_EXTRA_PARAM(user_properties, "");

      // OpenCOLLADA profile
      pTechnique = findExtraTechnique(node, "OpenCOLLADA");
      if (!visibility.element)
         GET_EXTRA_PARAM(visibility, 1.0);
      GET_EXTRA_PARAM(user_properties, user_properties);
   }
};

/// Extensions for the <geometry> element
class ColladaExtension_geometry : public ColladaExtension
{
public:
   // MAYA profile elements
   bool double_sided;

   ColladaExtension_geometry(const domGeometry* geometry)
   {
      // MAYA profile
      pTechnique = findExtraTechnique(geometry, "MAYA");
      GET_EXTRA_PARAM(double_sided, false);
   }
};

// Extensions for the <animation_clip> element
class ColladaExtension_animation_clip : public ColladaExtension
{
public:
   struct Trigger {
      F32 time;
      S32 state;
   };

   // Torque profile elements (none of these are animatable)
   S32 num_triggers;
   Vector<Trigger> triggers;
   bool cyclic;
   bool blend;
   F32 blendReferenceTime;
   F32 priority;

   ColladaExtension_animation_clip(const domAnimation_clip* clip)
   {
      // Torque profile
      pTechnique = findExtraTechnique(clip, "Torque");
      GET_EXTRA_PARAM(num_triggers, 0);
      for (S32 iTrigger = 0; iTrigger < num_triggers; iTrigger++) {
         triggers.increment();
         get(avar("trigger_time%d", iTrigger), triggers.last().time, 0.0f);
         get(avar("trigger_state%d", iTrigger), triggers.last().state, 0);
      }
      GET_EXTRA_PARAM(cyclic, false);
      GET_EXTRA_PARAM(blend, false);
      GET_EXTRA_PARAM(blendReferenceTime, 0.0f);
      GET_EXTRA_PARAM(priority, 5.0f);
   }
};

#endif // _COLLADA_EXTENSIONS_H_
