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
#include "shaderGen/shaderGen.h"

#include "shaderGen/conditionerFeature.h"
#include "core/stream/fileStream.h"
#include "shaderGen/featureMgr.h"
#include "shaderGen/shaderOp.h"
#include "gfx/gfxDevice.h"
#include "core/memVolume.h"
#include "core/module.h"


MODULE_BEGIN( ShaderGen )

   MODULE_INIT_BEFORE( GFX )
   MODULE_SHUTDOWN_AFTER( GFX )

   MODULE_INIT
   {
      ManagedSingleton< ShaderGen >::createSingleton();
   }
   
   MODULE_SHUTDOWN
   {
      ManagedSingleton< ShaderGen >::deleteSingleton();
   }

MODULE_END;


ShaderGen::ShaderGen()
{
   mInit = false;
   GFXDevice::getDeviceEventSignal().notify(this, &ShaderGen::_handleGFXEvent);
   mOutput = NULL;
}

ShaderGen::~ShaderGen()
{
   GFXDevice::getDeviceEventSignal().remove(this, &ShaderGen::_handleGFXEvent);
   _uninit();
}

void ShaderGen::registerInitDelegate(GFXAdapterType adapterType, ShaderGenInitDelegate& initDelegate)
{
   mInitDelegates[(U32)adapterType] = initDelegate;
}

bool ShaderGen::_handleGFXEvent(GFXDevice::GFXDeviceEventType event)
{
   switch (event)
   {
   case GFXDevice::deInit :
      initShaderGen();
      break;
   case GFXDevice::deDestroy :
      {
         flushProceduralShaders();
      }
      break;
   default :
      break;
   }
   return true;
}

void ShaderGen::initShaderGen()
{   
   if (mInit)
      return;

   const GFXAdapterType adapterType = GFX->getAdapterType();
   if (!mInitDelegates[adapterType])
      return;

   mInitDelegates[adapterType](this);
   mFeatureInitSignal.trigger( adapterType );
   mInit = true;

   String shaderPath = Con::getVariable( "$shaderGen::cachePath");
#if defined(TORQUE_SHADERGEN) && ( defined(TORQUE_OS_XENON) || defined(TORQUE_OS_PS3) )
   // If this is a console build, and TORQUE_SHADERGEN is defined 
   // (signifying that new shaders should be generated) then clear the shader
   // path so that the MemFileSystem is used instead.
   shaderPath.clear();
#endif

   if (!shaderPath.equal( "shadergen:" ) && !shaderPath.isEmpty() )
   {
      // this is necessary, especially under Windows with UAC enabled
      if (!Torque::FS::VerifyWriteAccess(shaderPath))
      {
         // we don't have write access so enable the virtualized memory store
         Con::warnf("ShaderGen: Write permission unavailable, switching to virtualized memory storage");
         shaderPath.clear();
      }

   }

   if ( shaderPath.equal( "shadergen:" ) || shaderPath.isEmpty() )
   {
      // If we didn't get a path then we're gonna cache the shaders to
      // a virtualized memory file system.
      mMemFS = new Torque::Mem::MemFileSystem( "shadergen:/" ); 
      Torque::FS::Mount( "shadergen", mMemFS );
   }
   else
      Torque::FS::Mount( "shadergen", shaderPath + "/" );

   // Delete the auto-generated conditioner include file.
   Torque::FS::Remove( "shadergen:/" + ConditionerFeature::ConditionerIncludeFileName );
}

void ShaderGen::generateShader( const MaterialFeatureData &featureData,
                                char *vertFile, 
                                char *pixFile, 
                                F32 *pixVersion,
                                const GFXVertexFormat *vertexFormat,
                                const char* cacheName,
                                Vector<GFXShaderMacro> &macros )
{
   PROFILE_SCOPE( ShaderGen_GenerateShader );

   mFeatureData = featureData;
   mVertexFormat = vertexFormat;

   _uninit();
   _init();

   char vertShaderName[256];
   char pixShaderName[256];

   // Note:  We use a postfix of _V/_P here so that it sorts the matching
   // vert and pixel shaders together when listed alphabetically.   
   dSprintf( vertShaderName, sizeof(vertShaderName), "shadergen:/%s_V.%s", cacheName, mFileEnding.c_str() );
   dSprintf( pixShaderName, sizeof(pixShaderName), "shadergen:/%s_P.%s", cacheName, mFileEnding.c_str() );
   
   dStrcpy( vertFile, vertShaderName );
   dStrcpy( pixFile, pixShaderName );   
   
   // this needs to change - need to optimize down to ps v.1.1
   *pixVersion = GFX->getPixelShaderVersion();
   
   if ( !Con::getBoolVariable( "ShaderGen::GenNewShaders", true ) )
   {
      // If we are not regenerating the shader we will return here.
      // But we must fill in the shader macros first!

      _processVertFeatures( macros, true );
      _processPixFeatures( macros, true );      

      return;
   }

   // create vertex shader
   //------------------------
   FileStream* s = new FileStream();
   if(!s->open(vertShaderName, Torque::FS::File::Write ))
   {
      AssertFatal(false, "Failed to open Shader Stream" );
      return;
   }

   mOutput = new MultiLine;
   mInstancingFormat.clear();
   _processVertFeatures(macros);
   _printVertShader( *s );
   delete s;
   
   ((ShaderConnector*)mComponents[C_CONNECTOR])->reset();
   LangElement::deleteElements();

   // create pixel shader
   //------------------------
   s = new FileStream();
   if(!s->open(pixShaderName, Torque::FS::File::Write ))
   {
      AssertFatal(false, "Failed to open Shader Stream" );
      return;
   }   

   mOutput = new MultiLine;
   _processPixFeatures(macros);
   _printPixShader( *s );

   delete s;
   LangElement::deleteElements();
}

void ShaderGen::_init()
{
   _createComponents();
}

void ShaderGen::_uninit()
{
   for( U32 i=0; i<mComponents.size(); i++ )
   {
      delete mComponents[i];
      mComponents[i] = NULL;
   }
   mComponents.setSize(0);

   LangElement::deleteElements();

   Var::reset();
}

void ShaderGen::_createComponents()
{
   ShaderComponent* vertComp = mComponentFactory->createVertexInputConnector( *mVertexFormat );
   mComponents.push_back(vertComp);

   ShaderComponent* vertPixelCon = mComponentFactory->createVertexPixelConnector();
   mComponents.push_back(vertPixelCon);

   ShaderComponent* vertParamDef = mComponentFactory->createVertexParamsDef();
   mComponents.push_back(vertParamDef);

   ShaderComponent* pixParamDef = mComponentFactory->createPixelParamsDef();
   mComponents.push_back(pixParamDef);
}

//----------------------------------------------------------------------------
// Process features
//----------------------------------------------------------------------------
void ShaderGen::_processVertFeatures( Vector<GFXShaderMacro> &macros, bool macrosOnly )
{
   const FeatureSet &features = mFeatureData.features;

   for( U32 i=0; i < features.getCount(); i++ )
   {
      S32 index;
      const FeatureType &type = features.getAt( i, &index );
      ShaderFeature* feature = FEATUREMGR->getByType( type );
      if ( feature )
      {
         feature->setProcessIndex( index );

         feature->processVertMacros( macros, mFeatureData );

         if ( macrosOnly )
            continue;

         feature->setInstancingFormat( &mInstancingFormat );

         feature->mVertexFormat = mVertexFormat;

         feature->processVert( mComponents, mFeatureData );

         String line;
         if ( index > -1 )
            line = String::ToString( "   // %s %d\r\n", feature->getName().c_str(), index );
         else
            line = String::ToString( "   // %s\r\n", feature->getName().c_str() );
         mOutput->addStatement( new GenOp( line ) );

         if ( feature->getOutput() )
            mOutput->addStatement( feature->getOutput() );

         feature->reset();
         mOutput->addStatement( new GenOp( "   \r\n" ) );         
      }
   }

   ShaderConnector *connect = dynamic_cast<ShaderConnector *>( mComponents[C_CONNECTOR] );
   connect->sortVars();
}

void ShaderGen::_processPixFeatures( Vector<GFXShaderMacro> &macros, bool macrosOnly )
{
   const FeatureSet &features = mFeatureData.features;

   for( U32 i=0; i < features.getCount(); i++ )
   {
      S32 index;
      const FeatureType &type = features.getAt( i, &index );
      ShaderFeature* feature = FEATUREMGR->getByType( type );
      if ( feature )
      {
         feature->setProcessIndex( index );

         feature->processPixMacros( macros, mFeatureData );

         if ( macrosOnly )
            continue;

         feature->setInstancingFormat( &mInstancingFormat );
         feature->processPix( mComponents, mFeatureData );

         String line;
         if ( index > -1 )
            line = String::ToString( "   // %s %d\r\n", feature->getName().c_str(), index );
         else
            line = String::ToString( "   // %s\r\n", feature->getName().c_str() );
         mOutput->addStatement( new GenOp( line ) );

         if ( feature->getOutput() )
            mOutput->addStatement( feature->getOutput() );

         feature->reset();
         mOutput->addStatement( new GenOp( "   \r\n" ) );
      }
   }
   
   ShaderConnector *connect = dynamic_cast<ShaderConnector *>( mComponents[C_CONNECTOR] );
   connect->sortVars();
}

void ShaderGen::_printFeatureList(Stream &stream)
{
   mPrinter->printLine(stream, "// Features:");
      
   const FeatureSet &features = mFeatureData.features;

   for( U32 i=0; i < features.getCount(); i++ )
   {
      S32 index;
      const FeatureType &type = features.getAt( i, &index );
      ShaderFeature* feature = FEATUREMGR->getByType( type );
      if ( feature )
      {
         String line;
         if ( index > -1 )
            line = String::ToString( "// %s %d", feature->getName().c_str(), index );
         else
            line = String::ToString( "// %s", feature->getName().c_str() );

         mPrinter->printLine( stream, line );
      }
   }

   mPrinter->printLine(stream, "");
}

void ShaderGen::_printDependencies(Stream &stream)
{
   Vector<const ShaderDependency *> dependencies;

   for( U32 i=0; i < FEATUREMGR->getFeatureCount(); i++ )
   {
      const FeatureInfo &info = FEATUREMGR->getAt( i );
      if ( mFeatureData.features.hasFeature( *info.type ) )
         dependencies.merge( info.feature->getDependencies() );
   }

   // Do a quick loop removing any duplicate dependancies.
   for( U32 i=0; i < dependencies.size(); )
   {
      bool dup = false;

      for( U32 j=0; j < dependencies.size(); j++ )
      {
         if (  j != i && 
               *dependencies[i] == *dependencies[j] )
         {
            dup = true;
            break;
         }
      }

      if ( dup )
         dependencies.erase( i );
      else        
         i++;
   }

   // Print dependencies
   if( dependencies.size() > 0 )
   {
      mPrinter->printLine(stream, "// Dependencies:");

      for( S32 i = 0; i < dependencies.size(); i++ )
         dependencies[i]->print( stream );

      mPrinter->printLine(stream, "");
   }
}

void ShaderGen::_printFeatures( Stream &stream )
{
   mOutput->print( stream );
}

void ShaderGen::_printVertShader( Stream &stream )
{
   mPrinter->printShaderHeader(stream);

   _printDependencies(stream); // TODO: Split into vert and pix dependencies?
   _printFeatureList(stream);

   // print out structures
   mComponents[C_VERT_STRUCT]->print( stream, true );
   mComponents[C_CONNECTOR]->print( stream, true );

   mPrinter->printMainComment(stream);

   mComponents[C_VERT_MAIN]->print( stream, true );
   mComponents[C_VERT_STRUCT]->printOnMain( stream, true );

   // print out the function
   _printFeatures( stream );

   mPrinter->printVertexShaderCloser(stream);
}

void ShaderGen::_printPixShader( Stream &stream )
{
   mPrinter->printShaderHeader(stream);

   _printDependencies(stream); // TODO: Split into vert and pix dependencies?
   _printFeatureList(stream);

   mComponents[C_CONNECTOR]->print( stream, false );

   mPrinter->printPixelShaderOutputStruct(stream, mFeatureData);
   mPrinter->printMainComment(stream);

   mComponents[C_PIX_MAIN]->print( stream, false );
   mComponents[C_CONNECTOR]->printOnMain( stream, false );

   // print out the function
   _printFeatures( stream );

   mPrinter->printPixelShaderCloser(stream);
}

GFXShader* ShaderGen::getShader( const MaterialFeatureData &featureData, const GFXVertexFormat *vertexFormat, const Vector<GFXShaderMacro> *macros, const Vector<String> &samplers )
{
   PROFILE_SCOPE( ShaderGen_GetShader );

   const FeatureSet &features = featureData.codify();

   // Build a description string from the features
   // and vertex format combination ( and macros ).
   String shaderDescription = vertexFormat->getDescription() + features.getDescription();
   if ( macros && !macros->empty() )
   {
      String macroStr;
      GFXShaderMacro::stringize( *macros, &macroStr );
      shaderDescription += macroStr;
   }

   // Generate a single 64bit hash from the description string.
   //
   // Don't get paranoid!  This has 1 in 18446744073709551616
   // chance for collision... it won't happen in this lifetime.
   //
   U64 hash = Torque::hash64( (const U8*)shaderDescription.c_str(), shaderDescription.length(), 0 );
   hash = convertHostToLEndian(hash);
   U32 high = (U32)( hash >> 32 );
   U32 low = (U32)( hash & 0x00000000FFFFFFFF );
   String cacheKey = String::ToString( "%x%x", high, low );
   // return shader if exists
   GFXShader *match = mProcShaders[cacheKey];
   if ( match )
      return match;

   // if not, then create it
   char vertFile[256];
   char pixFile[256];
   F32  pixVersion;

   Vector<GFXShaderMacro> shaderMacros;
   shaderMacros.push_back( GFXShaderMacro( "TORQUE_SHADERGEN" ) );
   if ( macros )
      shaderMacros.merge( *macros );
   generateShader( featureData, vertFile, pixFile, &pixVersion, vertexFormat, cacheKey, shaderMacros );

   GFXShader *shader = GFX->createShader();
   if (!shader->init(vertFile, pixFile, pixVersion, shaderMacros, samplers, &mInstancingFormat))
   {
      delete shader;
      return NULL;
   }

   mProcShaders[cacheKey] = shader;

   return shader;
}

void ShaderGen::flushProceduralShaders()
{
   // The shaders are reference counted, so we
   // just need to clear the map.
   mProcShaders.clear();  
}
