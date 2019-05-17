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

#include "customFeatureHLSL.h"
#include "shaderGen/shaderFeature.h"
#include "shaderGen/shaderOp.h"
#include "shaderGen/featureMgr.h"
//#include "materials/materialFeatureTypes.h"
//#include "gfx/gfxDevice.h"
//#include "materials/processedMaterial.h"

//****************************************************************************
// Accu Texture
//****************************************************************************
void CustomFeatureHLSL::processVert(Vector<ShaderComponent*>& componentList,
   const MaterialFeatureData& fd)
{
   /*MultiLine *meta = new MultiLine;
   getOutTexCoord(   "texCoord",
   "float2",
   false,
   meta,
   componentList );

   getOutObjToTangentSpace( componentList, meta, fd );

   output = meta;*/

   meta = new MultiLine;

   mFeatureData = fd;
   mComponentList = componentList;

   mOutputState = VertexOutput;

   if (mOwner->isMethod("processVertHLSL"))
      Con::executef(mOwner, "processVertHLSL");

   output = meta;
}

void CustomFeatureHLSL::processPix(Vector<ShaderComponent*>& componentList,
   const MaterialFeatureData& fd)
{
   meta = new MultiLine;

   mFeatureData = fd;
   mComponentList = componentList;

   mOutputState = PixelOutput;

   /*MultiLine *meta = new MultiLine;

   output = meta;

   // OUT.col
   Var *color = (Var*) LangElement::find( "col1" );
   if (!color)
   {
   output = new GenOp("   //NULL COLOR!");
   return;
   }

   // accu map
   Var *accuMap = new Var;
   accuMap->setType("SamplerState");

   accuMap->setName( "accuMap" );
   accuMap->uniform = true;
   accuMap->sampler = true;
   accuMap->constNum = Var::getTexUnitNum();     // used as texture unit num here

   // accuColor var
   Var *accuColor = new Var;
   accuColor->setType( "float4" );
   accuColor->setName( "accuColor" );
   LangElement *colorAccuDecl = new DecOp( accuColor );

   // plc (placement)
   Var *accuPlc = new Var;
   accuPlc->setType( "float4" );
   accuPlc->setName( "plc" );
   LangElement *plcAccu = new DecOp( accuPlc );

   // accu constants
   Var *accuScale = (Var*)LangElement::find( "accuScale" );
   if ( !accuScale )
   {
   accuScale = new Var;
   accuScale->setType( "float" );
   accuScale->setName( "accuScale" );
   accuScale->uniform = true;
   accuScale->sampler = false;
   accuScale->constSortPos = cspPotentialPrimitive;
   }
   Var *accuDirection = (Var*)LangElement::find( "accuDirection" );
   if ( !accuDirection )
   {
   accuDirection = new Var;
   accuDirection->setType( "float" );
   accuDirection->setName( "accuDirection" );
   accuDirection->uniform = true;
   accuDirection->sampler = false;
   accuDirection->constSortPos = cspPotentialPrimitive;
   }
   Var *accuStrength = (Var*)LangElement::find( "accuStrength" );
   if ( !accuStrength )
   {
   accuStrength = new Var;
   accuStrength->setType( "float" );
   accuStrength->setName( "accuStrength" );
   accuStrength->uniform = true;
   accuStrength->sampler = false;
   accuStrength->constSortPos = cspPotentialPrimitive;
   }
   Var *accuCoverage = (Var*)LangElement::find( "accuCoverage" );
   if ( !accuCoverage )
   {
   accuCoverage = new Var;
   accuCoverage->setType( "float" );
   accuCoverage->setName( "accuCoverage" );
   accuCoverage->uniform = true;
   accuCoverage->sampler = false;
   accuCoverage->constSortPos = cspPotentialPrimitive;
   }
   Var *accuSpecular = (Var*)LangElement::find( "accuSpecular" );
   if ( !accuSpecular )
   {
   accuSpecular = new Var;
   accuSpecular->setType( "float" );
   accuSpecular->setName( "accuSpecular" );
   accuSpecular->uniform = true;
   accuSpecular->sampler = false;
   accuSpecular->constSortPos = cspPotentialPrimitive;
   }

   Var *inTex = getInTexCoord( "texCoord", "float2", componentList );
   Var *accuVec = getInTexCoord( "accuVec", "float3", componentList );
   Var *bumpNorm = (Var *)LangElement::find( "bumpSample" );
   if( bumpNorm == NULL )
   {
   bumpNorm = (Var *)LangElement::find( "bumpNormal" );
   if (!bumpNorm)
   return;
   }

   // get the accu pixel color

   Var *accuMapTex = new Var;
   accuMapTex->setType("Texture2D");
   accuMapTex->setName("accuMapTex");
   accuMapTex->uniform = true;
   accuMapTex->texture = true;
   accuMapTex->constNum = accuMap->constNum;
   meta->addStatement(new GenOp("   @ = @.Sample(@, @ * @);\r\n", colorAccuDecl, accuMapTex, accuMap, inTex, accuScale));

   // scale up normals
   meta->addStatement( new GenOp( "   @.xyz = @.xyz * 2.0 - 0.5;\r\n", bumpNorm, bumpNorm ) );

   // assign direction
   meta->addStatement( new GenOp( "   @.z *= @*2.0;\r\n", accuVec, accuDirection ) );

   // saturate based on strength
   meta->addStatement( new GenOp( "   @ = saturate( dot( @.xyz, @.xyz * pow(@, 5) ) );\r\n", plcAccu, bumpNorm, accuVec, accuStrength ) );

   // add coverage
   meta->addStatement( new GenOp( "   @.a += (2 * pow(@/2, 5)) - 0.5;\r\n", accuPlc, accuCoverage ) );

   // clamp to a sensible value
   meta->addStatement( new GenOp( "   @.a = clamp(@.a, 0, 1);\r\n", accuPlc, accuPlc ) );

   // light
   Var *lightColor = (Var*) LangElement::find( "d_lightcolor" );
   if(lightColor != NULL)
   meta->addStatement( new GenOp( "   @ *= float4(@, 1.0);\r\n\r\n", accuColor, lightColor ) );

   // lerp with current pixel - use the accu alpha as well
   meta->addStatement( new GenOp( "   @ = lerp( @, @, @.a * @.a);\r\n", color, color, accuColor, accuPlc, accuColor ) );

   // the result should always be opaque
   meta->addStatement( new GenOp( "   @.a = 1.0;\r\n", color ) );*/
   if (mOwner->isMethod("processPixelHLSL"))
      Con::executef(mOwner, "processPixelHLSL");

   output = meta;
}

void CustomFeatureHLSL::setTexData(Material::StageData& stageDat,
   const MaterialFeatureData& fd,
   RenderPassData& passData,
   U32& texIndex)
{
   //GFXTextureObject *tex = stageDat.getTex( MFT_AccuMap );
   //if ( tex )
   //{
   //passData.mSamplerNames[ texIndex ] = "AccuMap";
   //passData.mTexType[ texIndex++ ] = Material::AccuMap;
   //}

   if (mOwner->isMethod("setTextureData"))
      Con::executef(mOwner, "setTextureData");
}

void CustomFeatureHLSL::addUniform(String name, String type, String defaultValue, U32 arraySize)
{
   //do the var/arg fetching here
   Var* newVar = (Var*)LangElement::find(name.c_str());
   if (!newVar)
   {
      VarHolder newVarHolder(name, type, "");
      newVarHolder.arraySize = arraySize;
      newVarHolder.sampler = false;
      newVarHolder.uniform = true;
      newVarHolder.constSortPos = cspPotentialPrimitive;

      mVars.push_back(newVarHolder);

      mOwner->mAddedShaderConstants.push_back(StringTable->insert(name.c_str()));
   }
}

void CustomFeatureHLSL::addSampler(String name, String type, U32 arraySize)
{
   //do the var/arg fetching here
   Var* newVar = (Var*)LangElement::find(name.c_str());
   if (!newVar)
   {
      //As far as I know, it's always SamplerState regardless of the texture's type
      VarHolder newVarHolder(name, "SamplerState", "");
      newVarHolder.arraySize = arraySize;
      newVarHolder.sampler = true;
      newVarHolder.uniform = true;
      newVarHolder.constNum = Var::getTexUnitNum();     // used as texture unit num here

      mVars.push_back(newVarHolder);

      mOwner->mAddedShaderConstants.push_back(StringTable->insert(name.c_str()));
   }
}

void CustomFeatureHLSL::addTexture(String name, String type, String samplerState, U32 arraySize)
{
   //do the var/arg fetching here
   Var* newVar = (Var*)LangElement::find(name.c_str());
   if (!newVar)
   {
      //go find our sampler state var
      U32 constNum = 0;

      Var* samplerStateVar = (Var*)LangElement::find(samplerState.c_str());
      if (!samplerStateVar)
      {
         //check our holder vars
         bool foundHolder = false;
         for (U32 v = 0; v < mVars.size(); v++)
         {
            if (mVars[v].varName == samplerState)
            {
               constNum = mVars[v].constNum;
               foundHolder = true;
               break;
            }
         }

         if (!foundHolder)
         {
            Con::errorf("CustomShaderFeature::addTexture: Unable to find texture's sampler state!");
            return;
         }
      }
      else
      {
         constNum = samplerStateVar->constNum;
      }

      VarHolder newVarHolder(name, type, "");
      newVarHolder.arraySize = arraySize;
      newVarHolder.texture = true;
      newVarHolder.uniform = true;
      newVarHolder.constNum = constNum;     // used as texture unit num here

      mVars.push_back(newVarHolder);

      mOwner->mAddedShaderConstants.push_back(StringTable->insert(name.c_str()));
   }
}

void CustomFeatureHLSL::addVariable(String name, String type, String defaultValue)
{
   //do the var/arg fetching here
   Var* newVar = (Var*)LangElement::find(name.c_str());
   if (!newVar)
   {
      if (!defaultValue.isEmpty())
      {
         char declareStatement[128];
         dSprintf(declareStatement, 128, "   @ = %s;\n", defaultValue.c_str());

         newVar = new Var(name, type);
         LangElement* newVarDecl = new DecOp(newVar);
         meta->addStatement(new GenOp(declareStatement, newVarDecl));
      }
      else
      {
         VarHolder newVarHolder(name, type, defaultValue);

         mVars.push_back(newVarHolder);
      }
   }
}

void CustomFeatureHLSL::addConnector(String name, String type, String elementName)
{
   // grab connector texcoord register
   ShaderConnector* connectComp = dynamic_cast<ShaderConnector*>(mComponentList[C_CONNECTOR]);

   //Get element
   S32 element = -1;

   if (elementName == String("RT_POSITION"))
      element = RT_POSITION;
   else if (elementName == String("RT_NORMAL"))
      element = RT_NORMAL;
   else if (elementName == String("RT_BINORMAL"))
      element = RT_BINORMAL;
   else if (elementName == String("RT_TANGENT"))
      element = RT_TANGENT;
   else if (elementName == String("RT_TANGENTW"))
      element = RT_TANGENTW;
   else if (elementName == String("RT_COLOR"))
      element = RT_COLOR;
   else if (elementName == String("RT_TEXCOORD"))
      element = RT_TEXCOORD;
   else if (elementName == String("RT_VPOS"))
      element = RT_VPOS;
   else if (elementName == String("RT_SVPOSITION"))
      element = RT_SVPOSITION;
   else if (elementName == String("RT_BLENDINDICES"))
      element = RT_BLENDINDICES;
   else if (elementName == String("RT_BLENDWEIGHT"))
      element = RT_BLENDWEIGHT;

   if (element == -1)
   {
      Con::errorf("CustomShaderFeatureHLSL::addConnector - Invalid element type %s", elementName.c_str());
      return;
   }

   VarHolder newVarHolder(name, type, "");

   newVarHolder.elementId = element;

   if (mOutputState == VertexOutput)
      newVarHolder.structName = "OUT";
   else if (mOutputState == PixelOutput)
      newVarHolder.structName = "IN";

   mVars.push_back(newVarHolder);
}

void CustomFeatureHLSL::addVertTexCoord(String name)
{
   VarHolder newVarHolder(name, "", "");
   newVarHolder.texCoord = true;

   mVars.push_back(newVarHolder);
}

void CustomFeatureHLSL::writeLine(String format, S32 argc, ConsoleValueRef * argv)
{
   //do the var/arg fetching here
   Vector<Var*> varList;
   bool declarationStatement = false;

   for (U32 i = 0; i < argc; i++)
   {
      String varName = argv[i].getStringValue();
      Var* newVar = (Var*)LangElement::find(varName.c_str());
      if (!newVar)
      {
         //ok, check our existing var holders, see if we just haven't utilized it yet
         for (U32 v = 0; v < mVars.size(); v++)
         {
            if (mVars[v].varName == varName)
            {
               if (!mVars[v].texCoord)
               {
                  if (mVars[v].elementId != -1)
                  {
                     ShaderConnector* connectComp = dynamic_cast<ShaderConnector*>(mComponentList[C_CONNECTOR]);
                     Var* newDeclVar = connectComp->getElement((RegisterType)mVars[v].elementId);
                     newDeclVar->setName(mVars[v].varName);
                     newDeclVar->setStructName(mVars[v].structName);
                     newDeclVar->setType(mVars[v].type);

                     newVar = newDeclVar;
                  }
                  else
                  {
                     Var* newDeclVar = new Var(mVars[v].varName, mVars[v].type);

                     newDeclVar->arraySize = mVars[v].arraySize;
                     newDeclVar->uniform = mVars[v].uniform;
                     newDeclVar->sampler = mVars[v].sampler;
                     newDeclVar->texture = mVars[v].texture;
                     newDeclVar->constNum = mVars[v].constNum;
                     newDeclVar->constSortPos = mVars[v].constSortPos;

                     if (!newDeclVar->uniform)
                     {
                        LangElement* newVarDecl = new DecOp(newDeclVar);
                        newVar = (Var*)newVarDecl;

                        declarationStatement = true;
                     }
                     else
                     {
                        newVar = newDeclVar;
                     }
                  }
               }
               else
               {
                  newVar = getVertTexCoord(mVars[v].varName);
               }

               mVars.erase(v);
               break;
            }
         }

         if (!newVar)
         {
            //couldn't find that variable, bail out
            Con::errorf("CustomShaderFeature::writeLine: unable to find variable %s, meaning it was not declared before being used!", argv[i].getStringValue());
            return;
         }
      }

      varList.push_back(newVar);
   }

   //not happy about it, but do a trampoline here to pass along the args

   switch (varList.size())
   {
      case 0:
         meta->addStatement(new GenOp(format + "\n"));
         break;
      case 1:
         meta->addStatement(new GenOp(format + "\n", varList[0]));
         break;
      case 2:
         meta->addStatement(new GenOp(format + "\n", varList[0], varList[1]));
         break;
      case 3:
         meta->addStatement(new GenOp(format + "\n", varList[0], varList[1], varList[2]));
         break;
      case 4:
         meta->addStatement(new GenOp(format + "\n", varList[0], varList[1], varList[2], varList[3]));
         break;
      case 5:
         meta->addStatement(new GenOp(format + "\n", varList[0], varList[1], varList[2], varList[3], varList[4]));
         break;
   }
}

bool CustomFeatureHLSL::hasFeature(String name)
{
   for (U32 i = 0; i < mFeatureData.materialFeatures.getCount(); i++)
   {
      String featureName = mFeatureData.materialFeatures.getAt(i).getName();
      if (name == featureName)
         return true;
   }

   return false;
}
