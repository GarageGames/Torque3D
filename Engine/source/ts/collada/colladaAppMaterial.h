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

#ifndef _COLLADA_APP_MATERIAL_H_
#define _COLLADA_APP_MATERIAL_H_

#ifndef _APPMATERIAL_H_
#include "ts/loader/appMaterial.h"
#endif
#ifndef _COLLADA_EXTENSIONS_H_
#include "ts/collada/colladaExtensions.h"
#endif

class Material;

class ColladaAppMaterial : public AppMaterial
{
public:
   const domMaterial*         mat;              ///< Collada <material> element
   domEffect*                 effect;           ///< Collada <effect> element
   ColladaExtension_effect*   effectExt;        ///< effect extension
   String                     name;             ///< Name of this material (cleaned)

   // Settings extracted from the Collada file, and optionally saved to materials.cs
   String                     diffuseMap;
   String                     normalMap;
   String                     specularMap;
   ColorF                     diffuseColor;
   ColorF                     specularColor;
   F32                        specularPower;
   bool                       doubleSided;

   ColladaAppMaterial(const char* matName);
   ColladaAppMaterial(const domMaterial* pMat);
   ~ColladaAppMaterial() { delete effectExt; }

   String getName() const { return name; }

   void resolveFloat(const domCommon_float_or_param_type* value, F32* dst);
   void resolveColor(const domCommon_color_or_texture_type* value, ColorF* dst);

   // Determine the material transparency
   template<class T> void resolveTransparency(const T shader, F32* dst)
   {
      // Start out by getting the <transparency> value
      *dst = 1.0f;
      resolveFloat(shader->getTransparency(), dst);

      // Multiply the transparency by the transparent color
      ColorF transColor(1.0f, 1.0f, 1.0f, 1.0f);
      if (shader->getTransparent() && shader->getTransparent()->getColor()) {
         const domCommon_color_or_texture_type::domColor* color = shader->getTransparent()->getColor();
         transColor.set(color->getValue()[0], color->getValue()[1], color->getValue()[2], color->getValue()[3]);
      }

      if (!shader->getTransparent() || (shader->getTransparent()->getOpaque() == FX_OPAQUE_ENUM_A_ONE)) {
         // multiply by alpha value and invert (so 1.0 is fully opaque)
         *dst = 1.0f - (*dst * transColor.alpha);
      }
      else {
         // multiply by average of the RGB values
         F32 avg = (transColor.red + transColor.blue + transColor.green) / 3;
         *dst *= avg;
      }
   }

   Material *createMaterial(const Torque::Path& path) const;
};

#endif // _COLLADA_APP_MATERIAL_H_
