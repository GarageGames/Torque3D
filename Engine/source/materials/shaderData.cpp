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
#include "materials/shaderData.h"

#include "console/consoleTypes.h"
#include "gfx/gfxDevice.h"
#include "core/strings/stringUnit.h"
#include "lighting/lightManager.h"
#include "console/engineAPI.h"

using namespace Torque;


Vector<ShaderData*> ShaderData::smAllShaderData;


IMPLEMENT_CONOBJECT( ShaderData );

ConsoleDocClass( ShaderData,
	"@brief Special type of data block that stores information about a handwritten shader.\n\n"

	"To use hand written shaders, a ShaderData datablock must be used. This datablock "
	"refers only to the vertex and pixel shader filenames and a hardware target level. "
	"Shaders are API specific, so DirectX and OpenGL shaders must be explicitly identified.\n\n "

	"@tsexample\n"
	"// Used for the procedural clould system\n"
	"singleton ShaderData( CloudLayerShader )\n"
	"{\n"
	"	DXVertexShaderFile   = \"shaders/common/cloudLayerV.hlsl\";\n"
	"	DXPixelShaderFile    = \"shaders/common/cloudLayerP.hlsl\";\n"
	"	OGLVertexShaderFile = \"shaders/common/gl/cloudLayerV.glsl\";\n"
	"	OGLPixelShaderFile = \"shaders/common/gl/cloudLayerP.glsl\";\n"
	"	pixVersion = 2.0;\n"
	"};\n"
	"@endtsexample\n\n"

	"@ingroup Shaders\n");

ShaderData::ShaderData()
{
   VECTOR_SET_ASSOCIATION( mShaderMacros );

   mUseDevicePixVersion = false;
   mPixVersion = 1.0;

   for( int i = 0; i < NumTextures; ++i)
      mRTParams[i] = false;
}

void ShaderData::initPersistFields()
{
   addField("DXVertexShaderFile",   TypeStringFilename,  Offset(mDXVertexShaderName,   ShaderData),
	   "@brief %Path to the DirectX vertex shader file to use for this ShaderData.\n\n"
	   "It must contain only one program and no pixel shader, just the vertex shader."
	   "It can be either an HLSL or assembly level shader. HLSL's must have a "
	   "filename extension of .hlsl, otherwise its assumed to be an assembly file.");

   addField("DXPixelShaderFile",    TypeStringFilename,  Offset(mDXPixelShaderName,  ShaderData),
	   "@brief %Path to the DirectX pixel shader file to use for this ShaderData.\n\n"
	   "It must contain only one program and no vertex shader, just the pixel "
	   "shader. It can be either an HLSL or assembly level shader. HLSL's "
	   "must have a filename extension of .hlsl, otherwise its assumed to be an assembly file.");

   addField("OGLVertexShaderFile",  TypeStringFilename,  Offset(mOGLVertexShaderName,   ShaderData),
	   "@brief %Path to an OpenGL vertex shader file to use for this ShaderData.\n\n"
	   "It must contain only one program and no pixel shader, just the vertex shader.");

   addField("OGLPixelShaderFile",   TypeStringFilename,  Offset(mOGLPixelShaderName,  ShaderData),
	   "@brief %Path to an OpenGL pixel shader file to use for this ShaderData.\n\n"
	   "It must contain only one program and no vertex shader, just the pixel "
	   "shader.");

   addField("useDevicePixVersion",  TypeBool,            Offset(mUseDevicePixVersion,   ShaderData),
	   "@brief If true, the maximum pixel shader version offered by the graphics card will be used.\n\n"
	   "Otherwise, the script-defined pixel shader version will be used.\n\n");

   addField("pixVersion",           TypeF32,             Offset(mPixVersion,   ShaderData),
	   "@brief Indicates target level the shader should be compiled.\n\n"
	   "Valid numbers at the time of this writing are 1.1, 1.4, 2.0, and 3.0. "
	   "The shader will not run properly if the hardware does not support the "
	   "level of shader compiled.");
   
   addField("defines",              TypeRealString,      Offset(mDefines,   ShaderData), 
	   "@brief String of case-sensitive defines passed to the shader compiler.\n\n"
      "The string should be delimited by a semicolon, tab, or newline character."
      
      "@tsexample\n"
       "singleton ShaderData( FlashShader )\n"
          "{\n"
              "DXVertexShaderFile 	= \"shaders/common/postFx/flashV.hlsl\";\n"
              "DXPixelShaderFile 	= \"shaders/common/postFx/flashP.hlsl\";\n\n"
              " //Define setting the color of WHITE_COLOR.\n"
              "defines = \"WHITE_COLOR=float4(1.0,1.0,1.0,0.0)\";\n\n"
              "pixVersion = 2.0\n"
          "}\n"
      "@endtsexample\n\n"
      );

   addField("samplerNames",              TypeRealString,      Offset(mSamplerNames,   ShaderData), NumTextures, 
      "@brief Indicates names of samplers present in shader. Order is important.\n\n"
	   "Order of sampler names are used to assert correct sampler register/location"
      "Other objects (GFXStateBlockData, PostEffect...) use index number to link samplers."
      );

   addField("rtParams",              TypeBool,      Offset(mRTParams,   ShaderData), NumTextures, "");

   Parent::initPersistFields();

   // Make sure we get activation signals.
   LightManager::smActivateSignal.notify( &ShaderData::_onLMActivate );
}

bool ShaderData::onAdd()
{
   if( !Parent::onAdd() )
      return false;

   mShaderMacros.clear();

   // Keep track of it.
   smAllShaderData.push_back( this );

   // NOTE: We initialize the shader on request.

   for(int i = 0; i < NumTextures; ++i)
   {
      if( mSamplerNames[i].isNotEmpty() && !mSamplerNames[i].startsWith("$") )      
         mSamplerNames[i].insert(0, "$");      
   }

   return true;
}

void ShaderData::onRemove()
{
   // Remove it from the all shaders list.
   smAllShaderData.remove( this );

   Parent::onRemove();
}

const Vector<GFXShaderMacro>& ShaderData::_getMacros()
{
   // If they have already been processed then 
   // return the cached result.
   if ( mShaderMacros.size() != 0 || mDefines.isEmpty() )
      return mShaderMacros;

   mShaderMacros.clear();  
   GFXShaderMacro macro;
   const U32 defineCount = StringUnit::getUnitCount( mDefines, ";\n\t" );
   for ( U32 i=0; i < defineCount; i++ )
   {
      String define = StringUnit::getUnit( mDefines, i, ";\n\t" );

      macro.name   = StringUnit::getUnit( define, 0, "=" );
      macro.value  = StringUnit::getUnit( define, 1, "=" );
      mShaderMacros.push_back( macro );
   }

   return mShaderMacros;
}

GFXShader* ShaderData::getShader( const Vector<GFXShaderMacro> &macros )
{
   PROFILE_SCOPE( ShaderData_GetShader );

   // Combine the dynamic macros with our script defined macros.
   Vector<GFXShaderMacro> finalMacros;
   finalMacros.merge( _getMacros() );
   finalMacros.merge( macros );

   // Convert the final macro list to a string.
   String cacheKey;
   GFXShaderMacro::stringize( macros, &cacheKey );   

   // Lookup the shader for this instance.
   ShaderCache::Iterator iter = mShaders.find( cacheKey );
   if ( iter != mShaders.end() )
      return iter->value;

   // Create the shader instance... if it fails then
   // bail out and return nothing to the caller.
   GFXShader *shader = _createShader( finalMacros );
   if ( !shader )
      return NULL;

   _checkDefinition(shader);

   // Store the shader in the cache and return it.
   mShaders.insertUnique( cacheKey, shader );
   return shader;
}

GFXShader* ShaderData::_createShader( const Vector<GFXShaderMacro> &macros )
{
   F32 pixver = mPixVersion;
   if ( mUseDevicePixVersion )
      pixver = getMax( pixver, GFX->getPixelShaderVersion() );

   // Enable shader error logging.
   GFXShader::setLogging( true, true );

   GFXShader *shader = GFX->createShader();
   bool success = false;

   Vector<String> samplers;
   samplers.setSize(ShaderData::NumTextures);
   for(int i = 0; i < ShaderData::NumTextures; ++i)
      samplers[i] = mSamplerNames[i][0] == '$' ? mSamplerNames[i] : "$"+mSamplerNames[i];

   // Initialize the right shader type.
   switch( GFX->getAdapterType() )
   {
      case Direct3D9_360:
      case Direct3D9:
      case Direct3D11:
      {
         success = shader->init( mDXVertexShaderName, 
                                 mDXPixelShaderName, 
                                 pixver,
                                 macros,
                                 samplers);
         break;
      }

      case OpenGL:
      {
         success = shader->init( mOGLVertexShaderName,
                                 mOGLPixelShaderName,
                                 pixver,
                                 macros,
                                 samplers);
         break;
      }
         
      default:
         // Other device types are assumed to not support shaders.
         success = false;
         break;
   }

#if defined(TORQUE_DEBUG)
   //Assert Sampler registers
   const Vector<GFXShaderConstDesc>& descs = shader->getShaderConstDesc();
   for(int i = 0; i < descs.size(); ++i)
   {
      if(descs[i].constType != GFXSCT_Sampler && descs[i].constType != GFXSCT_SamplerCube)
         continue;
      
      GFXShaderConstHandle *handle = shader->findShaderConstHandle(descs[i].name);
      if(!handle || !handle->isValid())
         continue;

      int reg = handle->getSamplerRegister();
      if( descs[i].name != samplers[reg] )
      {
         const char *err = avar("ShaderData(%s): samplerNames[%d] = \"%s\" are diferent to sampler in shader: %s : register(S%d)"
            ,getName(), reg, samplers[reg].c_str(), handle->getName().c_str(), reg);
         Con::printf(err);
         GFXAssertFatal(0, err);
      }
   }
#endif

   // If we failed to load the shader then
   // cleanup and return NULL.
   if ( !success )
      SAFE_DELETE( shader );

   return shader;
}

void ShaderData::reloadShaders()
{
   ShaderCache::Iterator iter = mShaders.begin();
   for ( ; iter != mShaders.end(); iter++ )
      iter->value->reload();
}

void ShaderData::reloadAllShaders()
{
   Vector<ShaderData*>::iterator iter = smAllShaderData.begin();
   for ( ; iter != smAllShaderData.end(); iter++ )
      (*iter)->reloadShaders();
}

void ShaderData::_onLMActivate( const char *lm, bool activate )
{
   // Only on activations do we do anything.
   if ( !activate )
      return;

   // Since the light manager usually swaps shadergen features
   // and changes system wide shader defines we need to completely
   // flush and rebuild all shaders.

   reloadAllShaders();
}

bool ShaderData::hasSamplerDef(const String &_samplerName, int &pos) const
{
   String samplerName = _samplerName.startsWith("$") ? _samplerName : "$"+_samplerName;   
   for(int i = 0; i < NumTextures; ++i)
   {
      if( mSamplerNames[i].equal(samplerName, String::NoCase ) )
      {
         pos = i;
         return true;
      }
   }

   pos = -1;
   return false;
}

bool ShaderData::_checkDefinition(GFXShader *shader)
{
   bool error = false;
   Vector<String> samplers;
   samplers.reserve(NumTextures);
   bool rtParams[NumTextures];
   for(int i = 0; i < NumTextures; ++i)
      rtParams[i] = false;   

   const Vector<GFXShaderConstDesc> &shaderConstDesc = shader->getShaderConstDesc(); 

   for(int i = 0; i < shaderConstDesc.size(); ++i)
   {
      const GFXShaderConstDesc &desc = shaderConstDesc[i];
      if(desc.constType == GFXSCT_Sampler)
      {
         samplers.push_back(desc.name );
      }      
   }

   for(int i = 0; i < samplers.size(); ++i)
   {
      int pos;
      bool find = hasSamplerDef(samplers[i], pos);

      if(find && pos >= 0 && mRTParams[pos])
      {              
         if( !shader->findShaderConstHandle( String::ToString("$rtParams%d", pos)) )
         {
            String error = String::ToString("ShaderData(%s) sampler[%d] used but rtParams%d not used in shader compilation. Possible error", shader->getPixelShaderFile().c_str(), pos, pos);
            Con::errorf( error );
            error = true;
         }
      }     

      if(!find)
      {
         String error = String::ToString("ShaderData(%s) sampler %s not defined", shader->getPixelShaderFile().c_str(), samplers[i].c_str());
         Con::errorf(error );
         GFXAssertFatal(0, error );
         error = true;
      }
   }  

   return !error;
}

DefineEngineMethod( ShaderData, reload, void, (),,
				   "@brief Rebuilds all the vertex and pixel shader instances created from this ShaderData.\n\n"

				   "@tsexample\n"
				   "// Rebuild the shader instances from ShaderData CloudLayerShader\n"
				   "CloudLayerShader.reload();\n"
				   "@endtsexample\n\n")
{
	object->reloadShaders();
}
