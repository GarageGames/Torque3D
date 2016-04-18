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

#include "shaderGen/HLSL/accuFeatureHLSL.h"
#include "shaderGen/shaderFeature.h"
#include "shaderGen/shaderOp.h"
#include "shaderGen/featureMgr.h"
#include "materials/materialFeatureTypes.h"
#include "gfx/gfxDevice.h"
#include "materials/processedMaterial.h"

//****************************************************************************
// Accu Texture
//****************************************************************************
void AccuTexFeatHLSL::processVert(  Vector<ShaderComponent*> &componentList, 
                                    const MaterialFeatureData &fd )
{
   MultiLine *meta = new MultiLine;
   getOutTexCoord(   "texCoord", 
                     "float2", 
                     true, 
                     false, 
                     meta, 
                     componentList );
   
   getOutObjToTangentSpace( componentList, meta, fd );
   addOutAccuVec( componentList, meta );

   output = meta;
}

void AccuTexFeatHLSL::processPix(   Vector<ShaderComponent*> &componentList, 
                                    const MaterialFeatureData &fd )
{
   MultiLine *meta = new MultiLine;

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
   accuMap->setType( "sampler2D" );
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

   Var *inTex = getInTexCoord( "texCoord", "float2", true, componentList );
   Var *accuVec = getInTexCoord( "accuVec", "float3", true, componentList );
   Var *bumpNorm = (Var *)LangElement::find( "bumpSample" );
   if( bumpNorm == NULL )
   {
      bumpNorm = (Var *)LangElement::find( "bumpNormal" );
      if (!bumpNorm)
        return;
   }

   // get the accu pixel color
   if (mIsDirect3D11)
   {
      Var *accuMapTex = new Var;
      accuMapTex->setType("Texture2D");
      accuMapTex->setName("accuMapTex");
      accuMapTex->uniform = true;
      accuMapTex->texture = true;
      accuMapTex->constNum = accuMap->constNum;
      meta->addStatement(new GenOp("   @ = @.Sample(@, @ * @);\r\n", colorAccuDecl, accuMapTex, accuMap, inTex, accuScale));
   }
   else
      meta->addStatement(new GenOp("   @ = tex2D(@, @ * @);\r\n", colorAccuDecl, accuMap, inTex, accuScale));

   // scale up normals
   meta->addStatement( new GenOp( "   @.xyz = @.xyz * 2.0 - 0.5;\r\n", bumpNorm, bumpNorm ) );

   // assign direction
   meta->addStatement( new GenOp( "   @.z *= @*2.0;\r\n", accuVec, accuDirection ) );

   // saturate based on strength
   meta->addStatement( new GenOp( "   @ = saturate( dot( @, @.xyz * pow(@, 5) ) );\r\n", plcAccu, bumpNorm, accuVec, accuStrength ) );

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
   meta->addStatement( new GenOp( "   @.a = 1.0;\r\n", color ) );

}

void AccuTexFeatHLSL::setTexData(   Material::StageData &stageDat,
                                    const MaterialFeatureData &fd,
                                    RenderPassData &passData,
                                    U32 &texIndex )
{
   //GFXTextureObject *tex = stageDat.getTex( MFT_AccuMap );
   //if ( tex )
   //{
   passData.mSamplerNames[ texIndex ] = "AccuMap";
   passData.mTexType[ texIndex++ ] = Material::AccuMap;
   //}
}


void AccuTexFeatHLSL::getAccuVec( MultiLine *meta, LangElement *accuVec )
{
   // Get the transform to world space.
   Var *objTrans = (Var*)LangElement::find( "objTrans" );
   if ( !objTrans )
   {
      objTrans = new Var;
      objTrans->setType( "float4x4" );
      objTrans->setName( "objTrans" );
      objTrans->uniform = true;
      objTrans->constSortPos = cspPrimitive;      
   }

   // accu obj trans
   Var *aobjTrans = new Var;
   aobjTrans->setType( "float4x4" );
   aobjTrans->setName( "accuObjTrans" );
   LangElement *accuObjTransDecl = new DecOp( aobjTrans );

   Var *outObjToTangentSpace = (Var*)LangElement::find( "objToTangentSpace" );

   Var *tav = new Var;
   tav->setType( "float4" );
   tav->setName( "tAccuVec" );
   LangElement *tavDecl = new DecOp( tav );

   meta->addStatement( new GenOp( "   @ = float4(0,0,1,0);\r\n", tavDecl ) );
   meta->addStatement( new GenOp( "   @ = transpose(@);\r\n", accuObjTransDecl, objTrans ) );
   meta->addStatement( new GenOp( "   @ = mul(@, @);\r\n", tav, aobjTrans, tav ) );
   meta->addStatement( new GenOp( "   @.xyz = mul(@, @.xyz);\r\n", tav, outObjToTangentSpace, tav ) );
   meta->addStatement( new GenOp( "   @.y *= -1;\r\n", tav ) );
   meta->addStatement( new GenOp( "   @ = @.xyz;\r\n", accuVec, tav ) );
}

Var* AccuTexFeatHLSL::addOutAccuVec( Vector<ShaderComponent*> &componentList, MultiLine *meta )
{
   Var *outAccuVec = (Var*)LangElement::find( "accuVec" );
   if ( !outAccuVec )
   {
      // Setup the connector.
      ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );
      outAccuVec = connectComp->getElement( RT_TEXCOORD );
      outAccuVec->setName( "accuVec" );
      outAccuVec->setStructName( "OUT" );
      outAccuVec->setType( "float3" );
      outAccuVec->mapsToSampler = false;

      getAccuVec( meta, outAccuVec );
   }

   return outAccuVec;
}
