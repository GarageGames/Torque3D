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

#include "ts/loader/tsShapeLoader.h"
#include "ts/collada/colladaAppMaterial.h"
#include "ts/collada/colladaUtils.h"
#include "ts/tsMaterialList.h"
#include "materials/materialManager.h"

using namespace ColladaUtils;

String cleanString(const String& str)
{
   String cleanStr(str);

   // Replace invalid characters with underscores
   const String badChars(" -,.+=*/");
   for (String::SizeType i = 0; i < badChars.length(); i++)
      cleanStr.replace(badChars[i], '_');

   // Prefix with an underscore if string starts with a number
   if ((cleanStr[0] >= '0') && (cleanStr[0] <= '9'))
      cleanStr.insert(0, '_');

   return cleanStr;
}

//------------------------------------------------------------------------------

ColladaAppMaterial::ColladaAppMaterial(const char* matName)
:  mat(0),
   effect(0),
   effectExt(0)
{
   name = matName;

   // Set some defaults
   flags |= TSMaterialList::S_Wrap;
   flags |= TSMaterialList::T_Wrap;

   diffuseColor = ColorF::ONE;
   specularColor = ColorF::ONE;
   specularPower = 8.0f;
   doubleSided = false;
}

ColladaAppMaterial::ColladaAppMaterial(const domMaterial *pMat)
:  mat(pMat),
   diffuseColor(ColorF::ONE),
   specularColor(ColorF::ONE),
   specularPower(8.0f),
   doubleSided(false)
{
   // Get the effect element for this material
   effect = daeSafeCast<domEffect>(mat->getInstance_effect()->getUrl().getElement());
   effectExt = new ColladaExtension_effect(effect);

   // Get the <profile_COMMON>, <diffuse> and <specular> elements
   const domProfile_COMMON* commonProfile = ColladaUtils::findEffectCommonProfile(effect);
   const domCommon_color_or_texture_type_complexType* domDiffuse = findEffectDiffuse(effect);
   const domCommon_color_or_texture_type_complexType* domSpecular = findEffectSpecular(effect);

   // Wrap flags
   if (effectExt->wrapU)
      flags |= TSMaterialList::S_Wrap;
   if (effectExt->wrapV)
      flags |= TSMaterialList::T_Wrap;

   // Set material attributes
   if (commonProfile) {

      F32 transparency = 0.0f;
      if (commonProfile->getTechnique()->getConstant()) {
         const domProfile_COMMON::domTechnique::domConstant* constant = commonProfile->getTechnique()->getConstant();
         diffuseColor.set(1.0f, 1.0f, 1.0f, 1.0f);
         resolveColor(constant->getReflective(), &specularColor);
         resolveFloat(constant->getReflectivity(), &specularPower);
         resolveTransparency(constant, &transparency);
      }
      else if (commonProfile->getTechnique()->getLambert()) {
         const domProfile_COMMON::domTechnique::domLambert* lambert = commonProfile->getTechnique()->getLambert();
         resolveColor(lambert->getDiffuse(), &diffuseColor);
         resolveColor(lambert->getReflective(), &specularColor);
         resolveFloat(lambert->getReflectivity(), &specularPower);
         resolveTransparency(lambert, &transparency);
      }
      else if (commonProfile->getTechnique()->getPhong()) {
         const domProfile_COMMON::domTechnique::domPhong* phong = commonProfile->getTechnique()->getPhong();
         resolveColor(phong->getDiffuse(), &diffuseColor);
         resolveColor(phong->getSpecular(), &specularColor);
         resolveFloat(phong->getShininess(), &specularPower);
         resolveTransparency(phong, &transparency);
      }
      else if (commonProfile->getTechnique()->getBlinn()) {
         const domProfile_COMMON::domTechnique::domBlinn* blinn = commonProfile->getTechnique()->getBlinn();
         resolveColor(blinn->getDiffuse(), &diffuseColor);
         resolveColor(blinn->getSpecular(), &specularColor);
         resolveFloat(blinn->getShininess(), &specularPower);
         resolveTransparency(blinn, &transparency);
      }

      // Normalize specularPower (1-128). Values > 1 are assumed to be
      // already normalized.
      if (specularPower <= 1.0f)
         specularPower *= 128;
      specularPower = mClampF(specularPower, 1.0f, 128.0f);

      // Set translucency
      if (transparency != 0.0f) {
         flags |= TSMaterialList::Translucent;
         if (transparency > 1.0f) {
            flags |= TSMaterialList::Additive;
            diffuseColor.alpha = transparency - 1.0f;
         }
         else if (transparency < 0.0f) {
            flags |= TSMaterialList::Subtractive;
            diffuseColor.alpha = -transparency;
         }
         else {
            diffuseColor.alpha = transparency;
         }
      }
      else
         diffuseColor.alpha = 1.0f;
   }

   // Double-sided flag
   doubleSided = effectExt->double_sided;

   // Get the paths for the various textures => Collada indirection at its finest!
   // <texture>.<newparam>.<sampler2D>.<source>.<newparam>.<surface>.<init_from>.<image>.<init_from>
   diffuseMap = getSamplerImagePath(effect, getTextureSampler(effect, domDiffuse));
   specularMap = getSamplerImagePath(effect, getTextureSampler(effect, domSpecular));
   normalMap = getSamplerImagePath(effect, effectExt->bumpSampler);

   // Set the material name
   name = ColladaUtils::getOptions().matNamePrefix;
   if ( ColladaUtils::getOptions().useDiffuseNames )
   {
      Torque::Path diffusePath( diffuseMap );
      name += diffusePath.getFileName();
   }
   else
   {
      name += _GetNameOrId(mat);
   }
}

void ColladaAppMaterial::resolveFloat(const domCommon_float_or_param_type* value, F32* dst)
{
   if (value && value->getFloat()) {
      *dst = value->getFloat()->getValue();
   }
}

void ColladaAppMaterial::resolveColor(const domCommon_color_or_texture_type* value, ColorF* dst)
{
   if (value && value->getColor()) {
      dst->red = value->getColor()->getValue()[0];
      dst->green = value->getColor()->getValue()[1];
      dst->blue = value->getColor()->getValue()[2];
      dst->alpha = value->getColor()->getValue()[3];
   }
}

// Generate a new Material object
Material *ColladaAppMaterial::createMaterial(const Torque::Path& path) const
{
   // The filename and material name are used as TorqueScript identifiers, so
   // clean them up first
   String cleanFile = cleanString(TSShapeLoader::getShapePath().getFileName());
   String cleanName = cleanString(getName());

   // Prefix the material name with the filename (if not done already by TSShapeConstructor prefix)
   if (!cleanName.startsWith(cleanFile))
      cleanName = cleanFile + "_" + cleanName;

   // Determine the blend operation for this material
   Material::BlendOp blendOp = (flags & TSMaterialList::Translucent) ? Material::LerpAlpha : Material::None;
   if (flags & TSMaterialList::Additive)
      blendOp = Material::Add;
   else if (flags & TSMaterialList::Subtractive)
      blendOp = Material::Sub;

   // Create the Material definition
   const String oldScriptFile = Con::getVariable("$Con::File");
   Con::setVariable("$Con::File", path.getFullPath());   // modify current script path so texture lookups are correct
   Material *newMat = MATMGR->allocateAndRegister( cleanName, getName() );
   Con::setVariable("$Con::File", oldScriptFile);        // restore script path

   newMat->mDiffuseMapFilename[0] = diffuseMap;
   newMat->mNormalMapFilename[0] = normalMap;
   newMat->mSpecularMapFilename[0] = specularMap;

   newMat->mDiffuse[0] = diffuseColor;
   newMat->mSpecular[0] = specularColor;
   newMat->mSpecularPower[0] = specularPower;

   newMat->mDoubleSided = doubleSided;
   newMat->mTranslucent = (bool)(flags & TSMaterialList::Translucent);
   newMat->mTranslucentBlendOp = blendOp;

   return newMat;
}
