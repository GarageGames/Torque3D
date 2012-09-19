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
#include "lighting/advanced/glsl/advancedLightingFeaturesGLSL.h"

#include "lighting/advanced/advancedLightBinManager.h"
#include "shaderGen/langElement.h"
#include "shaderGen/shaderOp.h"
#include "shaderGen/conditionerFeature.h"
#include "renderInstance/renderPrePassMgr.h"
#include "materials/processedMaterial.h"
#include "materials/materialFeatureTypes.h"


void DeferredRTLightingFeatGLSL::processPixMacros( Vector<GFXShaderMacro> &macros, 
                                                   const MaterialFeatureData &fd  )
{
   /// TODO: This needs to be done via some sort of material
   /// feature and not just allow all translucent elements to
   /// read from the light prepass.
   /*
   if ( fd.features[MFT_IsTranslucent] )
   {
      Parent::processPixMacros( macros, fd );
      return;
   }
   */

   // Pull in the uncondition method for the light info buffer
   NamedTexTarget *texTarget = NamedTexTarget::find( AdvancedLightBinManager::smBufferName );
   if ( texTarget && texTarget->getConditioner() )
   {
      ConditionerMethodDependency *unconditionMethod = texTarget->getConditioner()->getConditionerMethodDependency(ConditionerFeature::UnconditionMethod);
      unconditionMethod->createMethodMacro( String::ToLower( AdvancedLightBinManager::smBufferName ) + "Uncondition", macros );
      addDependency(unconditionMethod);
   }
}

void DeferredRTLightingFeatGLSL::processVert(   Vector<ShaderComponent*> &componentList, 
                                                const MaterialFeatureData &fd )
{
   /// TODO: This needs to be done via some sort of material
   /// feature and not just allow all translucent elements to
   /// read from the light prepass.
   /*
   if ( fd.features[MFT_IsTranslucent] )
   {
      Parent::processVert( componentList, fd );
      return;
   }
   */

   // Pass screen space position to pixel shader to compute a full screen buffer uv
   ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );
   Var *ssPos = connectComp->getElement( RT_TEXCOORD );
   ssPos->setName( "screenspacePos" );
   ssPos->setType( "vec4" );

//   Var *outPosition = (Var*) LangElement::find( "hpos" );
//   AssertFatal( outPosition, "No hpos, ohnoes." );

   output = new GenOp( "   @ = gl_Position;\r\n", ssPos );
}

void DeferredRTLightingFeatGLSL::processPix( Vector<ShaderComponent*> &componentList,
                                             const MaterialFeatureData &fd )
{
   /// TODO: This needs to be done via some sort of material
   /// feature and not just allow all translucent elements to
   /// read from the light prepass.
   /*
   if ( fd.features[MFT_IsTranslucent] )
   {
      Parent::processPix( componentList, fd );
      return;
   }
   */

   MultiLine *meta = new MultiLine;

   ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );
   Var *ssPos = connectComp->getElement( RT_TEXCOORD );
   ssPos->setName( "screenspacePos" );
   ssPos->setType( "vec4" );

   Var *uvScene = new Var;
   uvScene->setType( "vec2" );
   uvScene->setName( "uvScene" );
   LangElement *uvSceneDecl = new DecOp( uvScene );

   Var *rtParams = (Var*) LangElement::find( "renderTargetParams" );
   if( !rtParams )
   {
      rtParams = new Var;
      rtParams->setType( "vec4" );
      rtParams->setName( "renderTargetParams" );
      rtParams->uniform = true;
      rtParams->constSortPos = cspPass;
   }

   meta->addStatement( new GenOp( "   @ = @.xy / @.w;\r\n", uvSceneDecl, ssPos, ssPos ) ); // get the screen coord... its -1 to +1
   meta->addStatement( new GenOp( "   @ = ( @ + 1.0 ) / 2.0;\r\n", uvScene, uvScene ) ); // get the screen coord to 0 to 1
   meta->addStatement( new GenOp( "   @ = ( @ * @.zw ) + @.xy;\r\n", uvScene, uvScene, rtParams, rtParams) ); // scale it down and offset it to the rt size

   Var *lightInfoSamp = new Var;
   lightInfoSamp->setType( "vec4" );
   lightInfoSamp->setName( "lightInfoSample" );

   // create texture var
   Var *lightInfoBuffer = new Var;
   lightInfoBuffer->setType( "sampler2D" );
   lightInfoBuffer->setName( "lightInfoBuffer" );
   lightInfoBuffer->uniform = true;
   lightInfoBuffer->sampler = true;
   lightInfoBuffer->constNum = Var::getTexUnitNum();     // used as texture unit num here

   String unconditionLightInfo = String::ToLower( AdvancedLightBinManager::smBufferName ) + "Uncondition";
   
   meta->addStatement( new GenOp( "   vec3 d_lightcolor;\r\n" ) );
   meta->addStatement( new GenOp( "   float d_NL_Att;\r\n" ) );
   meta->addStatement( new GenOp( "   float d_specular;\r\n" ) );
   meta->addStatement( new GenOp( avar( "   %s(texture2D(@, @), d_lightcolor, d_NL_Att, d_specular);\r\n", unconditionLightInfo.c_str() ), 
      lightInfoBuffer, uvScene ) );

   Var *rtShading = new Var;
   rtShading->setType( "vec4" );
   rtShading->setName( "rtShading" );
   LangElement *rtShadingDecl = new DecOp( rtShading );
   meta->addStatement( new GenOp( "   @ = vec4( d_lightcolor, 1.0 );\r\n", rtShadingDecl ) );

   // This is kind of weak sauce
   if( !fd.features[MFT_SubSurface] && !fd.features[MFT_ToneMap] && !fd.features[MFT_LightMap] )
      meta->addStatement( new GenOp( "   @;\r\n", assignColor( rtShading, Material::Mul ) ) );

   output = meta;
}

ShaderFeature::Resources DeferredRTLightingFeatGLSL::getResources( const MaterialFeatureData &fd )
{
   /// TODO: This needs to be done via some sort of material
   /// feature and not just allow all translucent elements to
   /// read from the light prepass.
   /*
   if( fd.features[MFT_IsTranslucent] )
      return Parent::getResources( fd );
   */

   Resources res; 
   res.numTex = 1;
   res.numTexReg = 1;
   return res;
}

void DeferredRTLightingFeatGLSL::setTexData( Material::StageData &stageDat,
                                             const MaterialFeatureData &fd, 
                                             RenderPassData &passData, 
                                             U32 &texIndex )
{
   /// TODO: This needs to be done via some sort of material
   /// feature and not just allow all translucent elements to
   /// read from the light prepass.
   /*
   if( fd.features[MFT_IsTranslucent] )
   {
      Parent::setTexData( stageDat, fd, passData, texIndex );
      return;
   }
   */

   NamedTexTarget *texTarget = NamedTexTarget::find( AdvancedLightBinManager::smBufferName );
   if( texTarget )
   {
      passData.mTexType[ texIndex ] = Material::TexTarget;
      passData.mTexSlot[ texIndex++ ].texTarget = texTarget;
   }
}


void DeferredBumpFeatGLSL::processVert(   Vector<ShaderComponent*> &componentList, 
                                          const MaterialFeatureData &fd )
{
   if( fd.features[MFT_PrePassConditioner] )
   {
      // There is an output conditioner active, so we need to supply a transform
      // to the pixel shader. 
      MultiLine *meta = new MultiLine;

      // setup texture space matrix
      Var *texSpaceMat = (Var*) LangElement::find( "objToTangentSpace" );
      if( !texSpaceMat )
      {
         LangElement * texSpaceSetup = setupTexSpaceMat( componentList, &texSpaceMat );
         meta->addStatement( texSpaceSetup );
         texSpaceMat = (Var*) LangElement::find( "objToTangentSpace" );
      }

      // turn obj->tangent into world->tangent
      Var *worldToTangent = new Var;
      worldToTangent->setType( "mat3" );
      worldToTangent->setName( "worldToTangent" );
      LangElement *worldToTangentDecl = new DecOp( worldToTangent );

      // Get the world->obj transform
      Var *worldToObj = new Var;
      worldToObj->setType( "mat4" );
      worldToObj->setName( "worldToObj" );
      worldToObj->uniform = true;
      worldToObj->constSortPos = cspPrimitive;
      
      Var *mat3Conversion = new Var;
      mat3Conversion->setType( "mat3" );
      mat3Conversion->setName( "worldToObjMat3" );
      LangElement* mat3Lang = new DecOp(mat3Conversion);
      meta->addStatement( new GenOp( "   @ = mat3(@[0].xyz, @[1].xyz, @[2].xyz);\r\n ", mat3Lang, worldToObj, worldToObj, worldToObj) );

      // assign world->tangent transform
      meta->addStatement( new GenOp( "   @ = @ * @;\r\n", worldToTangentDecl, texSpaceMat, mat3Conversion ) );

      // send transform to pixel shader
      ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );

      Var *worldToTangentR1 = connectComp->getElement( RT_TEXCOORD );
      worldToTangentR1->setName( "worldToTangentR1" );
      worldToTangentR1->setType( "vec3" );
      meta->addStatement( new GenOp( "   @ = @[0];\r\n", worldToTangentR1, worldToTangent ) );

      Var *worldToTangentR2 = connectComp->getElement( RT_TEXCOORD );
      worldToTangentR2->setName( "worldToTangentR2" );
      worldToTangentR2->setType( "vec3" );
      meta->addStatement( new GenOp( "   @ = @[1];\r\n", worldToTangentR2, worldToTangent ) );

      Var *worldToTangentR3 = connectComp->getElement( RT_TEXCOORD );
      worldToTangentR3->setName( "worldToTangentR3" );
      worldToTangentR3->setType( "vec3" );
      meta->addStatement( new GenOp( "   @ = @[2];\r\n", worldToTangentR3, worldToTangent ) );

      // Make sure there are texcoords
      if( !fd.features[MFT_DiffuseMap] )
      {
         // find incoming texture var
         Var *inTex = getVertTexCoord( "texCoord" );

         // grab connector texcoord register
         ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );
         Var *outTex = connectComp->getElement( RT_TEXCOORD );
         outTex->setName( "outTexCoord" );
         outTex->setType( "vec2" );
         outTex->mapsToSampler = true;

         if( fd.features[MFT_TexAnim] )
         {
            inTex->setType( "vec4" );

            // create texture mat var
            Var *texMat = new Var;
            texMat->setType( "mat4" );
            texMat->setName( "texMat" );
            texMat->uniform = true;
            texMat->constSortPos = cspPotentialPrimitive;

            meta->addStatement( new GenOp( "   @ = @ * @;\r\n", outTex, texMat, inTex ) );
         }
         else
         {
            // setup language elements to output incoming tex coords to output
            meta->addStatement( new GenOp( "   @ = @;\r\n", outTex, inTex ) );
         }
      }

      output = meta;
   }
   else if (   fd.materialFeatures[MFT_NormalsOut] || 
               fd.features[MFT_IsTranslucent] || 
               !fd.features[MFT_RTLighting] )
   {
      Parent::processVert( componentList, fd );
      return;
   }
   else
   {
      output = NULL;
   }
}

void DeferredBumpFeatGLSL::processPix( Vector<ShaderComponent*> &componentList, 
                                       const MaterialFeatureData &fd )
{
   // NULL output in case nothing gets handled
   output = NULL;

   if( fd.features[MFT_PrePassConditioner] )
   {
      MultiLine *meta = new MultiLine;

      // Pull the world->tangent transform from the vertex shader
      ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );

      Var *worldToTangentR1 = connectComp->getElement( RT_TEXCOORD );
      worldToTangentR1->setName( "worldToTangentR1" );
      worldToTangentR1->setType( "vec3" );

      Var *worldToTangentR2 = connectComp->getElement( RT_TEXCOORD );
      worldToTangentR2->setName( "worldToTangentR2" );
      worldToTangentR2->setType( "vec3" );

      Var *worldToTangentR3 = connectComp->getElement( RT_TEXCOORD );
      worldToTangentR3->setName( "worldToTangentR3" );
      worldToTangentR3->setType( "vec3" );

      Var *worldToTangent = new Var;
      worldToTangent->setType( "mat3" );
      worldToTangent->setName( "worldToTangent" );
      LangElement *worldToTangentDecl = new DecOp( worldToTangent );

      // Build world->tangent matrix
      meta->addStatement( new GenOp( "   @;\r\n", worldToTangentDecl ) );
      meta->addStatement( new GenOp( "   @[0] = @;\r\n", worldToTangent, worldToTangentR1 ) );
      meta->addStatement( new GenOp( "   @[1] = @;\r\n", worldToTangent, worldToTangentR2 ) );
      meta->addStatement( new GenOp( "   @[2] = @;\r\n", worldToTangent, worldToTangentR3 ) );

      // create texture var
      Var *bumpMap = new Var;
      bumpMap->setType( "sampler2D" );
      bumpMap->setName( "bumpMap" );
      bumpMap->uniform = true;
      bumpMap->sampler = true;
      bumpMap->constNum = Var::getTexUnitNum();     // used as texture unit num here

      Var *texCoord = (Var*) LangElement::find( "outTexCoord" );
      if( !texCoord )
      {
         // grab connector texcoord register
         ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );
         texCoord = connectComp->getElement( RT_TEXCOORD );
         texCoord->setName( "outTexCoord" );
         texCoord->setType( "vec2" );
         texCoord->mapsToSampler = true;
      }

      LangElement * texOp = new GenOp( "texture2D(@, @)", bumpMap, texCoord );

      // create bump normal
      Var *bumpNorm = new Var;
      bumpNorm->setName( "bumpNormal" );
      bumpNorm->setType( "vec4" );

      LangElement *bumpNormDecl = new DecOp( bumpNorm );
      meta->addStatement( expandNormalMap( texOp, bumpNormDecl, bumpNorm, fd ) );

      // This var is read from GBufferConditionerHLSL and 
      // used in the prepass output.
      Var *gbNormal = new Var;
      gbNormal->setName( "gbNormal" );
      gbNormal->setType( "vec3" );
      LangElement *gbNormalDecl = new DecOp( gbNormal );

      // Normalize is done later... 
      // Note: The reverse mul order is intentional. Affine matrix.
      meta->addStatement( new GenOp( "   @ = @.xyz * @;\r\n", gbNormalDecl, bumpNorm, worldToTangent ) );

      output = meta;
      return;
   }
   else if (   fd.materialFeatures[MFT_NormalsOut] || 
               fd.features[MFT_IsTranslucent] || 
               !fd.features[MFT_RTLighting] )
   {
      Parent::processPix( componentList, fd );
      return;
   }
   else if ( fd.features[MFT_PixSpecular] )
   {
      Var *bumpSample = (Var *)LangElement::find( "bumpSample" );
      if( bumpSample == NULL )
      {
         Var *texCoord = (Var*) LangElement::find( "outTexCoord" );
         if( !texCoord )
         {
            // grab connector texcoord register
            ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );
            texCoord = connectComp->getElement( RT_TEXCOORD );
            texCoord->setName( "outTexCoord" );
            texCoord->setType( "vec2" );
            texCoord->mapsToSampler = true;
         }

         Var *bumpMap = new Var;
         bumpMap->setType( "sampler2D" );
         bumpMap->setName( "bumpMap" );
         bumpMap->uniform = true;
         bumpMap->sampler = true;
         bumpMap->constNum = Var::getTexUnitNum();     // used as texture unit num here

         bumpSample = new Var;
         bumpSample->setType( "vec4" );
         bumpSample->setName( "bumpSample" );
         LangElement *bumpSampleDecl = new DecOp( bumpSample );

         output = new GenOp( "   @ = texture2D(@, @);\r\n", bumpSampleDecl, bumpMap, texCoord );
         return;
      }
   }

   output = NULL;
}

ShaderFeature::Resources DeferredBumpFeatGLSL::getResources( const MaterialFeatureData &fd )
{
   if (  fd.materialFeatures[MFT_NormalsOut] || 
         fd.features[MFT_IsTranslucent] || 
         fd.features[MFT_Parallax] ||
         !fd.features[MFT_RTLighting] )
      return Parent::getResources( fd );

   Resources res; 
   if(!fd.features[MFT_SpecularMap])
   {
      res.numTex = 1;
      res.numTexReg = 1;
   }
   return res;
}

void DeferredBumpFeatGLSL::setTexData( Material::StageData &stageDat,
                                       const MaterialFeatureData &fd, 
                                       RenderPassData &passData, 
                                       U32 &texIndex )
{
   if (  fd.materialFeatures[MFT_NormalsOut] || 
         fd.features[MFT_IsTranslucent] || 
         !fd.features[MFT_RTLighting] )
   {
      Parent::setTexData( stageDat, fd, passData, texIndex );
      return;
   }

   GFXTextureObject *normalMap = stageDat.getTex( MFT_NormalMap );
   if (  !fd.features[MFT_Parallax] && !fd.features[MFT_SpecularMap] &&
         ( fd.features[MFT_PrePassConditioner] ||
           fd.features[MFT_PixSpecular] ) &&         
         normalMap )
   {
      passData.mTexType[ texIndex ] = Material::Bump;
      passData.mTexSlot[ texIndex++ ].texObject = normalMap;
   }
}


void DeferredPixelSpecularGLSL::processVert( Vector<ShaderComponent*> &componentList, 
                                             const MaterialFeatureData &fd )
{
   if( fd.features[MFT_IsTranslucent] || !fd.features[MFT_RTLighting] )
   {
      Parent::processVert( componentList, fd );
      return;
   }
   output = NULL;
}

void DeferredPixelSpecularGLSL::processPix(  Vector<ShaderComponent*> &componentList, 
                                             const MaterialFeatureData &fd )
{
   if( fd.features[MFT_IsTranslucent] || !fd.features[MFT_RTLighting] )
   {
      Parent::processPix( componentList, fd );
      return;
   }

   MultiLine *meta = new MultiLine;

   Var *specular = new Var;
   specular->setType( "float" );
   specular->setName( "specular" );
   LangElement * specDecl = new DecOp( specular );

   Var *specCol = (Var*)LangElement::find( "specularColor" );
   if(specCol == NULL)
   {
      specCol = new Var;
      specCol->setType( "vec4" );
      specCol->setName( "specularColor" );
      specCol->uniform = true;
      specCol->constSortPos = cspPotentialPrimitive;
   }

   Var *specPow = new Var;
   specPow->setType( "float" );
   specPow->setName( "specularPower" );

   // If the gloss map flag is set, than the specular power is in the alpha
   // channel of the specular map
   if( fd.features[ MFT_GlossMap ] )
      meta->addStatement( new GenOp( "   @ = @.a * 255;\r\n", new DecOp( specPow ), specCol ) );
   else
   {
      specPow->uniform = true;
      specPow->constSortPos = cspPotentialPrimitive;
   }

   Var *constSpecPow = new Var;
   constSpecPow->setType( "float" );
   constSpecPow->setName( "constantSpecularPower" );
   constSpecPow->uniform = true;
   constSpecPow->constSortPos = cspPass;

   Var *lightInfoSamp = (Var *)LangElement::find( "lightInfoSample" );
   AssertFatal( lightInfoSamp, "Something hosed the deferred features! Can't find lightInfoSample" );

   // (a^m)^n = a^(m*n)
   meta->addStatement( new GenOp( "   @ = pow(d_specular, ceil(@ / @)) * d_NL_Att;\r\n", specDecl, specPow, constSpecPow ) );

   LangElement *specMul = new GenOp( "@ * @", specCol, specular );
   LangElement *final = specMul;

   // We we have a normal map then mask the specular 
   if( !fd.features[MFT_SpecularMap] && fd.features[MFT_NormalMap] )
   {
      Var *bumpSample = (Var*)LangElement::find( "bumpSample" );
      final = new GenOp( "@ * @.a", final, bumpSample );
   }

   // add to color
   meta->addStatement( new GenOp( "   @;\r\n", assignColor( final, Material::Add ) ) );

   output = meta;
}

ShaderFeature::Resources DeferredPixelSpecularGLSL::getResources( const MaterialFeatureData &fd )
{
   if( fd.features[MFT_IsTranslucent] || !fd.features[MFT_RTLighting] )
      return Parent::getResources( fd );

   Resources res; 
   return res;
}


ShaderFeature::Resources DeferredMinnaertGLSL::getResources( const MaterialFeatureData &fd )
{
   Resources res;
   if( !fd.features[MFT_IsTranslucent] && fd.features[MFT_RTLighting] )
   {
      res.numTex = 1;
      res.numTexReg = 1;
   }
   return res;
}

void DeferredMinnaertGLSL::setTexData( Material::StageData &stageDat,
                                       const MaterialFeatureData &fd, 
                                       RenderPassData &passData, 
                                       U32 &texIndex )
{
   if( !fd.features[MFT_IsTranslucent] && fd.features[MFT_RTLighting] )
   {
      NamedTexTarget *texTarget = NamedTexTarget::find(RenderPrePassMgr::BufferName);
      if ( texTarget )
      {
         passData.mTexType[ texIndex ] = Material::TexTarget;
         passData.mTexSlot[ texIndex++ ].texTarget = texTarget;
      }
   }
}

void DeferredMinnaertGLSL::processPixMacros( Vector<GFXShaderMacro> &macros, 
                                             const MaterialFeatureData &fd  )
{
   if( !fd.features[MFT_IsTranslucent] && fd.features[MFT_RTLighting] )
   {
      // Pull in the uncondition method for the g buffer
      NamedTexTarget *texTarget = NamedTexTarget::find( RenderPrePassMgr::BufferName );
      if ( texTarget && texTarget->getConditioner() )
      {
         ConditionerMethodDependency *unconditionMethod = texTarget->getConditioner()->getConditionerMethodDependency(ConditionerFeature::UnconditionMethod);
         unconditionMethod->createMethodMacro( String::ToLower(RenderPrePassMgr::BufferName) + "Uncondition", macros );
         addDependency(unconditionMethod);
      }
   }
}

void DeferredMinnaertGLSL::processVert(   Vector<ShaderComponent*> &componentList,
                                          const MaterialFeatureData &fd )
{
   // If there is no deferred information, bail on this feature
   if( fd.features[MFT_IsTranslucent] || !fd.features[MFT_RTLighting] )
   {
      output = NULL;
      return;
   }

   // grab incoming vert position
   Var *inVertPos = (Var*) LangElement::find( "position" );
   AssertFatal( inVertPos, "Something went bad with ShaderGen. The vertex position should be already defined." );

   // grab output for gbuffer normal
   ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );
   Var *outWSEyeVec= connectComp->getElement( RT_TEXCOORD );
   outWSEyeVec->setName( "outWSViewVec" );
   outWSEyeVec->setType( "vec4" );

   // create objToWorld variable
   Var *objToWorld = (Var*) LangElement::find( "objTrans" );
   if( !objToWorld )
   {
      objToWorld = new Var;
      objToWorld->setType( "mat4x4" );
      objToWorld->setName( "objTrans" );
      objToWorld->uniform = true;
      objToWorld->constSortPos = cspPrimitive;
   }

   // Eye Pos world
   Var *eyePosWorld = (Var*) LangElement::find( "eyePosWorld" );
   if( !eyePosWorld )
   {
      eyePosWorld = new Var;
      eyePosWorld->setType( "vec3" );
      eyePosWorld->setName( "eyePosWorld" );
      eyePosWorld->uniform = true;
      eyePosWorld->constSortPos = cspPass;
   }

   // Kick out the world-space normal
   LangElement *statement = new GenOp( "   @ = vec4(@, @) - vec4(@, 0.0);\r\n", 
      outWSEyeVec, objToWorld, inVertPos, eyePosWorld );
   output = statement;
}

void DeferredMinnaertGLSL::processPix( Vector<ShaderComponent*> &componentList, 
                                       const MaterialFeatureData &fd )
{
   // If there is no deferred information, bail on this feature
   if( fd.features[MFT_IsTranslucent] || !fd.features[MFT_RTLighting] )
   {
      output = NULL;
      return;
   }

   Var *minnaertConstant = new Var;
   minnaertConstant->setType( "float" );
   minnaertConstant->setName( "minnaertConstant" );
   minnaertConstant->uniform = true;
   minnaertConstant->constSortPos = cspPotentialPrimitive;

   // create texture var
   Var *prepassBuffer = new Var;
   prepassBuffer->setType( "sampler2D" );
   prepassBuffer->setName( "prepassBuffer" );
   prepassBuffer->uniform = true;
   prepassBuffer->sampler = true;
   prepassBuffer->constNum = Var::getTexUnitNum();     // used as texture unit num here

   // Texture coord
   Var *uvScene = (Var*) LangElement::find( "uvScene" );
   AssertFatal(uvScene != NULL, "Unable to find UVScene, no RTLighting feature?");

   ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );
   Var *wsViewVec = (Var*) LangElement::find( "wsPos" );
   if( !wsViewVec )
   {
      wsViewVec = connectComp->getElement( RT_TEXCOORD );
      wsViewVec->setName( "outWSViewVec" );
      wsViewVec->setType( "vec4" );
      wsViewVec->mapsToSampler = false;
      wsViewVec->uniform = false;
   }

   String unconditionPrePassMethod = String::ToLower(RenderPrePassMgr::BufferName) + "Uncondition";

   MultiLine *meta = new MultiLine;
   meta->addStatement( new GenOp( avar( "   vec4 normalDepth = %s(texture2D(@, @));\r\n", unconditionPrePassMethod.c_str() ), prepassBuffer, uvScene ) );
   meta->addStatement( new GenOp( "   vec3 worldViewVec = normalize(@.xyz / @.w);\r\n", wsViewVec, wsViewVec ) );
   meta->addStatement( new GenOp( "   float vDotN = dot(normalDepth.xyz, worldViewVec);\r\n" ) );
   meta->addStatement( new GenOp( "   float Minnaert = pow(d_NL_Att, @) * pow(vDotN, 1.0 - @);\r\n", minnaertConstant, minnaertConstant ) );
   meta->addStatement( new GenOp( "   @;\r\n", assignColor( new GenOp( "vec4(Minnaert, Minnaert, Minnaert, 1.0)" ), Material::Mul ) ) );

   output = meta;
}


void DeferredSubSurfaceGLSL::processPix(  Vector<ShaderComponent*> &componentList, 
                                          const MaterialFeatureData &fd )
{
   // If there is no deferred information, bail on this feature
   if( fd.features[MFT_IsTranslucent] || !fd.features[MFT_RTLighting] )
   {
      output = NULL;
      return;
   }

   Var *subSurfaceParams = new Var;
   subSurfaceParams->setType( "vec4" );
   subSurfaceParams->setName( "subSurfaceParams" );
   subSurfaceParams->uniform = true;
   subSurfaceParams->constSortPos = cspPotentialPrimitive;

   Var *inColor = (Var*) LangElement::find( "rtShading" );

   MultiLine *meta = new MultiLine;
   meta->addStatement( new GenOp( "   float subLamb = smoothstep(-@.a, 1.0, d_NL_Att) - smoothstep(0.0, 1.0, d_NL_Att);\r\n", subSurfaceParams ) );
   meta->addStatement( new GenOp( "   subLamb = max(0.0, subLamb);\r\n" ) );
   meta->addStatement( new GenOp( "   @;\r\n", assignColor( new GenOp( "vec4(@.rgb + (subLamb * @.rgb), 1.0)", inColor, subSurfaceParams ), Material::Mul ) ) );

   output = meta;
}
