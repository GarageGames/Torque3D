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
#include "materials/customMaterialDefinition.h"

#include "materials/materialManager.h"
#include "console/consoleTypes.h"
#include "materials/shaderData.h"
#include "gfx/sim/cubemapData.h"
#include "gfx/gfxCubemap.h"
#include "gfx/sim/gfxStateBlockData.h"


//****************************************************************************
// Custom Material
//****************************************************************************
IMPLEMENT_CONOBJECT(CustomMaterial);

ConsoleDocClass( CustomMaterial,
   "@brief Material object which provides more control over surface properties.\n\n"

   "CustomMaterials allow the user to specify their own shaders via the ShaderData datablock. "
   "Because CustomMaterials are derived from Materials, they can hold a lot of the same properties. "
   "It is up to the user to code how these properties are used.\n\n"

   "@tsexample\n"
   "singleton CustomMaterial( WaterBasicMat )\n"
   "{\n"
   "   sampler[\"reflectMap\"] = \"$reflectbuff\";\n"
   "   sampler[\"refractBuff\"] = \"$backbuff\";\n\n"
   "   cubemap = NewLevelSkyCubemap;\n"
   "   shader = WaterBasicShader;\n"
   "   stateBlock = WaterBasicStateBlock;\n"
   "   version = 2.0;\n"
   "};\n"
   "@endtsexample\n\n"

   "@see Material, GFXStateBlockData, ShaderData\n\n"

   "@ingroup Materials\n"
);

//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------
CustomMaterial::CustomMaterial()
{  
   mFallback = NULL;
   mMaxTex = 0;
   mVersion = 1.1f;
   mTranslucent = false;
   dMemset( mFlags, 0, sizeof( mFlags ) );   
   mShaderData = NULL;
   mRefract = false;
   mStateBlockData = NULL;
   mForwardLit = false;
}

//--------------------------------------------------------------------------
// Init fields
//--------------------------------------------------------------------------
void CustomMaterial::initPersistFields()
{
   addField("version",     TypeF32,             Offset(mVersion, CustomMaterial), 
      "@brief Specifies pixel shader version for hardware.\n\n"
      "Valid pixel shader versions include 2.0, 3.0, etc. "
      "@note All features aren't compatible with all pixel shader versions.");
   addField("fallback",    TYPEID< Material >(),    Offset(mFallback,  CustomMaterial), 
      "@brief Alternate material for targeting lower end hardware.\n\n"
      "If the CustomMaterial requires a higher pixel shader version than the one "
      "it's using, it's fallback Material will be processed instead. "
      "If the fallback material wasn't defined, Torque 3D will assert and attempt to use a very "
      "basic material in it's place.\n\n");
   addField("shader",      TypeRealString,      Offset(mShaderDataName, CustomMaterial), 
      "@brief Name of the ShaderData to use for this effect.\n\n");
   addField("stateBlock",  TYPEID< GFXStateBlockData >(),    Offset(mStateBlockData,  CustomMaterial), 
      "@brief Name of a GFXStateBlockData for this effect.\n\n");
   addField("target",      TypeRealString,      Offset(mOutputTarget, CustomMaterial), 
      "@brief String identifier of this material's target texture.");
   addField("forwardLit",  TypeBool,      Offset(mForwardLit, CustomMaterial), 
      "@brief Determines if the material should recieve lights in Basic Lighting. "
      "Has no effect in Advanced Lighting.\n\n");

   Parent::initPersistFields();
}

//--------------------------------------------------------------------------
// On add - verify data settings
//--------------------------------------------------------------------------
bool CustomMaterial::onAdd()
{
   if (Parent::onAdd() == false)
      return false;

   mShaderData = dynamic_cast<ShaderData*>(Sim::findObject( mShaderDataName ) );
   if(mShaderDataName.isNotEmpty() && mShaderData == NULL)
   {
      logError("Failed to find ShaderData %s", mShaderDataName.c_str());
      return false;
   }
   
   const char* samplerDecl = "sampler";
   S32 i = 0;
   for (SimFieldDictionaryIterator itr(getFieldDictionary()); *itr; ++itr)
   {
   	SimFieldDictionary::Entry* entry = *itr;
      if (dStrStartsWith(entry->slotName, samplerDecl))
      {
      	if (i >= MAX_TEX_PER_PASS)
         {
            logError("Too many sampler declarations, you may only have %i", MAX_TEX_PER_PASS);
            return false;
         }
         
         if (dStrlen(entry->slotName) == dStrlen(samplerDecl))
         {
         	logError("sampler declarations must have a sampler name, e.g. sampler[\"diffuseMap\"]");
            return false;
         }
         
         // Assert sampler names are defined on ShaderData
         S32 pos = -1;
         String samplerName = entry->slotName + dStrlen(samplerDecl);
         samplerName.insert(0, '$');
         mShaderData->hasSamplerDef(samplerName, pos);
         
         if(pos == -1)
         {
            const char *error = (avar("CustomMaterial(%s) bind sampler[%s] and is not present on ShaderData(%s)", 
               getName(), samplerName.c_str(), mShaderDataName.c_str() ));
            Con::errorf(error);

            pos = i;

#ifdef TORQUE_OPENGL
            GFXAssertFatal(0, error);
            continue;
#endif
         }
         mSamplerNames[pos] = samplerName;
         mTexFilename[pos] = entry->value;
         ++i;
      }
   }

   return true;
}

//--------------------------------------------------------------------------
// On remove
//--------------------------------------------------------------------------
void CustomMaterial::onRemove()
{
   Parent::onRemove();
}

//--------------------------------------------------------------------------
// Map this material to the texture specified in the "mapTo" data variable
//--------------------------------------------------------------------------
void CustomMaterial::_mapMaterial()
{
   if( String(getName()).isEmpty() )
   {
      Con::warnf( "Unnamed Material!  Could not map to: %s", mMapTo.c_str() );
      return;
   }

   if( mMapTo.isEmpty() )
      return;

   MATMGR->mapMaterial(mMapTo, getName());
}

const GFXStateBlockData* CustomMaterial::getStateBlockData() const
{
   return mStateBlockData;
}
