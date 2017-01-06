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
#include "gfx/gfxShader.h"

#include "shaderGen/conditionerFeature.h"
#include "core/volume.h"
#include "console/engineAPI.h"


Vector<GFXShaderMacro> GFXShader::smGlobalMacros;
bool GFXShader::smLogErrors = true;
bool GFXShader::smLogWarnings = true;


GFXShader::GFXShader()
   :  mPixVersion( 0.0f ),
      mReloadKey( 0 ),
      mInstancingFormat( NULL )
{
}

GFXShader::~GFXShader()
{
   Torque::FS::RemoveChangeNotification( mVertexFile, this, &GFXShader::_onFileChanged );
   Torque::FS::RemoveChangeNotification( mPixelFile, this, &GFXShader::_onFileChanged );

   SAFE_DELETE(mInstancingFormat);
}

#ifndef TORQUE_OPENGL
bool GFXShader::init(   const Torque::Path &vertFile, 
                        const Torque::Path &pixFile, 
                        F32 pixVersion, 
                        const Vector<GFXShaderMacro> &macros )
{
   Vector<String> samplerNames;
   return init( vertFile, pixFile, pixVersion, macros, samplerNames );
}
#endif

bool GFXShader::init(   const Torque::Path &vertFile, 
                        const Torque::Path &pixFile, 
                        F32 pixVersion, 
                        const Vector<GFXShaderMacro> &macros,
                        const Vector<String> &samplerNames,
                        GFXVertexFormat *instanceFormat)
{
   // Take care of instancing
   if (instanceFormat)
   {
      mInstancingFormat = new GFXVertexFormat;
      mInstancingFormat->copy(*instanceFormat);
   }

   // Store the inputs for use in reloading.
   mVertexFile = vertFile;
   mPixelFile = pixFile;
   mPixVersion = pixVersion;
   mMacros = macros;
   mSamplerNamesOrdered = samplerNames;

   // Before we compile the shader make sure the
   // conditioner features have been updated.
   ConditionerFeature::updateConditioners();

   // Now do the real initialization.
   if ( !_init() )
      return false;

   _updateDesc();

   // Add file change notifications for reloads.
   Torque::FS::AddChangeNotification( mVertexFile, this, &GFXShader::_onFileChanged );
   Torque::FS::AddChangeNotification( mPixelFile, this, &GFXShader::_onFileChanged );

   return true;
}

bool GFXShader::reload()
{
   // Before we compile the shader make sure the
   // conditioner features have been updated.
   ConditionerFeature::updateConditioners();

   mReloadKey++;

   // Init does the work.
   bool success = _init();
   if ( success )
      _updateDesc();

   // Let anything that cares know that
   // this shader has reloaded
   mReloadSignal.trigger();

   return success;
}

void GFXShader::_updateDesc()
{
   mDescription = String::ToString( "Files: %s, %s Pix Version: %0.2f\nMacros: ", 
      mVertexFile.getFullPath().c_str(), mPixelFile.getFullPath().c_str(), mPixVersion );

   GFXShaderMacro::stringize( smGlobalMacros, &mDescription );
   GFXShaderMacro::stringize( mMacros, &mDescription );   
}

void GFXShader::addGlobalMacro( const String &name, const String &value )
{
   // Check to see if we already have this macro.
   Vector<GFXShaderMacro>::iterator iter = smGlobalMacros.begin();
   for ( ; iter != smGlobalMacros.end(); iter++ )
   {
      if ( iter->name == name )
      {
         if ( iter->value != value )
            iter->value = value;
         return;
      }
   }

   // Add a new macro.
   smGlobalMacros.increment();
   smGlobalMacros.last().name = name;
   smGlobalMacros.last().value = value;
}

bool GFXShader::removeGlobalMacro( const String &name )
{
   Vector<GFXShaderMacro>::iterator iter = smGlobalMacros.begin();
   for ( ; iter != smGlobalMacros.end(); iter++ )
   {
      if ( iter->name == name )
      {
         smGlobalMacros.erase( iter );
         return true;
      }
   }

   return false;
}

void GFXShader::_unlinkBuffer( GFXShaderConstBuffer *buf )
{   
   Vector<GFXShaderConstBuffer*>::iterator iter = mActiveBuffers.begin();
   for ( ; iter != mActiveBuffers.end(); iter++ )
   {
      if ( *iter == buf )
      {
         mActiveBuffers.erase_fast( iter );
         return;
      }
   }

   AssertFatal( false, "GFXShader::_unlinkBuffer - buffer was not found?" );
}


DefineEngineFunction( addGlobalShaderMacro, void, 
   ( const char *name, const char *value ), ( nullAsType<const char*>() ),
   "Adds a global shader macro which will be merged with the script defined "
   "macros on every shader.  The macro will replace the value of an existing "
   "macro of the same name.  For the new macro to take effect all the shaders "
   "in the system need to be reloaded.\n"
   "@see resetLightManager, removeGlobalShaderMacro\n"
   "@ingroup Rendering\n" )
{
   GFXShader::addGlobalMacro( name, value );
}

DefineEngineFunction( removeGlobalShaderMacro, void, ( const char *name ),, 
   "Removes an existing global macro by name.\n"
   "@see addGlobalShaderMacro\n"
   "@ingroup Rendering\n" )
{
   GFXShader::removeGlobalMacro( name );
}
