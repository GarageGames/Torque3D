
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

#ifndef _AFX_ZODIAC_GROUNDPLANE_RENDERER_H_
#define _AFX_ZODIAC_GROUNDPLANE_RENDERER_H_

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#include "renderInstance/renderBinManager.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class ConcretePolyList;
class GroundPlane;

class afxZodiacGroundPlaneRenderer : public RenderBinManager
{
   typedef RenderBinManager Parent;

   struct GroundPlaneZodiacElem
   {
     const GroundPlane* gp;
     U32                zode_idx;
     F32                ang;
     F32                camDist;
   };

   Vector<GroundPlaneZodiacElem> groundPlane_zodiacs;
   static afxZodiacGroundPlaneRenderer* master;

   GFXStateBlockRef norm_norefl_zb_SB, norm_refl_zb_SB;
   GFXStateBlockRef add_norefl_zb_SB, add_refl_zb_SB;
   GFXStateBlockRef sub_norefl_zb_SB, sub_refl_zb_SB;

   ShaderData*    zodiac_shader;
   GFXShaderConstBufferRef shader_consts;
   GFXShaderConstHandle* projection_sc;
   GFXShaderConstHandle* color_sc;

   bool           shader_initialized;

   GFXStateBlock* chooseStateBlock(U32 blend, bool isReflectPass);

public:
   static const RenderInstType RIT_GroundPlaneZodiac;

   /*C*/          afxZodiacGroundPlaneRenderer();
   /*C*/          afxZodiacGroundPlaneRenderer(F32 renderOrder, F32 processAddOrder);
   /*D*/          ~afxZodiacGroundPlaneRenderer();

   // RenderBinManager
   virtual void   sort(){}  // don't sort them
   virtual void   clear();

   void           initShader();
   void           addZodiac(U32 zode_idx, const Point3F& pos, F32 ang, const GroundPlane*, F32 camDist);

   virtual void   render(SceneRenderState* state);

   static afxZodiacGroundPlaneRenderer* getMaster();

   // ConsoleObject
   DECLARE_CONOBJECT(afxZodiacGroundPlaneRenderer);
   DECLARE_CATEGORY("AFX");
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#endif // _AFX_ZODIAC_GROUNDPLANE_RENDERER_H_
