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

#ifndef _ACCUFEATUREHLSL_H_
#define _ACCUFEATUREHLSL_H_

#ifndef _SHADERGEN_HLSL_SHADERFEATUREHLSL_H_
#include "shaderGen/HLSL/shaderFeatureHLSL.h"
#endif
#ifndef _LANG_ELEMENT_H_
#include "shaderGen/langElement.h"
#endif
#ifndef _GFXDEVICE_H_
#include "gfx/gfxDevice.h"
#endif
#ifndef _FEATUREMGR_H_
#include "shaderGen/featureMgr.h"
#endif
#ifndef _MATERIALFEATURETYPES_H_
#include "materials/materialFeatureTypes.h"
#endif
#ifndef _MATERIALFEATUREDATA_H_
#include "materials/materialFeatureData.h"
#endif

/// Accu texture
class AccuTexFeatHLSL : public ShaderFeatureHLSL
{
public:

   //****************************************************************************
   // Accu Texture
   //****************************************************************************
   virtual void processVert(  Vector<ShaderComponent*> &componentList, 
                              const MaterialFeatureData &fd );

   virtual void processPix(   Vector<ShaderComponent*> &componentList, 
                              const MaterialFeatureData &fd );

   void getAccuVec( MultiLine *meta, LangElement *accuVec );

   Var* addOutAccuVec( Vector<ShaderComponent*> &componentList, MultiLine *meta );

   virtual Material::BlendOp getBlendOp(){ return Material::LerpAlpha; }

   virtual Resources getResources( const MaterialFeatureData &fd )
   {
      Resources res; 
      res.numTex = 1;
      res.numTexReg = 1;
      return res;
   }

   virtual void setTexData(   Material::StageData &stageDat,
                              const MaterialFeatureData &fd,
                              RenderPassData &passData,
                              U32 &texIndex );

   virtual String getName()
   {
      return "Accu Texture";
   }
};

class AccuScaleFeature : public ShaderFeatureHLSL
{
public:
   virtual void processPix( Vector<ShaderComponent*> &componentList, const MaterialFeatureData &fd )
   {
      // Find the constant value
      Var *accuScale = (Var *)( LangElement::find("accuScale") );
      if( accuScale == NULL )
      {
         accuScale = new Var;
         accuScale->setType( "float" );
         accuScale->setName( "accuScale" );
         accuScale->constSortPos = cspPotentialPrimitive;
         accuScale->uniform = true;
      }
   }

   virtual String getName() { return "Accu Scale"; }
};

class AccuDirectionFeature : public ShaderFeatureHLSL
{
public:
   virtual void processPix( Vector<ShaderComponent*> &componentList, const MaterialFeatureData &fd )
   {
      // Find the constant value
      Var *accuDirection = (Var *)( LangElement::find("accuDirection") );
      if( accuDirection == NULL )
      {
         accuDirection = new Var;
         accuDirection->setType( "float" );
         accuDirection->setName( "accuDirection" );
         accuDirection->constSortPos = cspPotentialPrimitive;
         accuDirection->uniform = true;
      }
   }

   virtual String getName() { return "Accu Direction"; }
};

class AccuStrengthFeature : public ShaderFeatureHLSL
{
public:
   virtual void processPix( Vector<ShaderComponent*> &componentList, const MaterialFeatureData &fd )
   {
      // Find the constant value
      Var *accuStrength = (Var *)( LangElement::find("accuStrength") );
      if( accuStrength == NULL )
      {
         accuStrength = new Var;
         accuStrength->setType( "float" );
         accuStrength->setName( "accuStrength" );
         accuStrength->constSortPos = cspPotentialPrimitive;
         accuStrength->uniform = true;
      }
   }

   virtual String getName() { return "Accu Strength"; }
};

class AccuCoverageFeature : public ShaderFeatureHLSL
{
public:
   virtual void processPix( Vector<ShaderComponent*> &componentList, const MaterialFeatureData &fd )
   {
      // Find the constant value
      Var *accuCoverage = (Var *)( LangElement::find("accuCoverage") );
      if( accuCoverage == NULL )
      {
         accuCoverage = new Var;
         accuCoverage->setType( "float" );
         accuCoverage->setName( "accuCoverage" );
         accuCoverage->constSortPos = cspPotentialPrimitive;
         accuCoverage->uniform = true;
      }
   }

   virtual String getName() { return "Accu Coverage"; }
};


class AccuSpecularFeature : public ShaderFeatureHLSL
{
public:
   virtual void processPix( Vector<ShaderComponent*> &componentList, const MaterialFeatureData &fd )
   {
      // Find the constant value
      Var *accuSpecular = (Var *)( LangElement::find("accuSpecular") );
      if( accuSpecular == NULL )
      {
         accuSpecular = new Var;
         accuSpecular->setType( "float" );
         accuSpecular->setName( "accuSpecular" );
         accuSpecular->constSortPos = cspPotentialPrimitive;
         accuSpecular->uniform = true;
      }
   }

   virtual String getName() { return "Accu Specular"; }
};

#endif