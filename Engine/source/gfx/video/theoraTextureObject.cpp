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

#include "gfx/video/theoraTextureObject.h"
#include "console/engineAPI.h"
#include "platform/platform.h"
#include "sfx/sfxTypes.h"

#ifdef TORQUE_OGGTHEORA

IMPLEMENT_CONOBJECT( TheoraTextureObject );

ConsoleDocClass( TheoraTextureObject,

   "@brief Definition of a named texture target playing a Theora video.\n\n"

   "TheoraTextureObject defines a named texture target that may play back a Theora video.  This texture "
   "target can, for example, be used by materials to texture objects with videos.\n\n"

   "@tsexample\n"
   "// The object that provides the video texture and controls its playback.\n"
   "singleton TheoraTextureObject( TheVideo )\n"
   "{\n"
   "   // Unique name for the texture target for referencing in materials.\n"
   "   texTargetName = \"video\";\n"
   "\n"
   "   // Path to the video file.\n"
   "   theoraFile = \"./MyVideo.ogv\";\n"
   "};\n"
   "\n"
   "// Material that uses the video texture.\n"
   "singleton Material( TheVideoMaterial )\n"
   "{\n"
   "   // This has to reference the named texture target defined by the\n"
   "   // TheoraTextureObject's 'texTargetName' property.  Prefix with '#' to\n"
   "   // identify as texture target reference.\n"
   "   diffuseMap[ 0 ] = \"#video\";\n"
   "};\n"
   "@endtsexample\n"
   "\n"

   "@ingroup Rendering"
);


//-----------------------------------------------------------------------------

TheoraTextureObject::TheoraTextureObject()
   : mSFXDescription( NULL )
{
   mIsPlaying = false;
   mLoop = false;

   mTexTarget.getTextureDelegate().bind( this, &TheoraTextureObject::_texDelegate );
}

//-----------------------------------------------------------------------------

void TheoraTextureObject::initPersistFields()
{
   addGroup( "Theora" );

      addField( "theoraFile", TypeStringFilename,  Offset( mFilename, TheoraTextureObject ),
         "Theora video file to play." );

      addField( "texTargetName", TypeRealString, Offset( mTexTargetName, TheoraTextureObject ),
         "Name of the texture target by which the texture can be referenced in materials." );

      addField( "sfxDescription", TypeSFXDescriptionName, Offset( mSFXDescription, TheoraTextureObject ),
         "Sound description to use for the video's audio channel.\n\n"
         "If not set, will use a default one." );

      addField( "loop", TypeBool,  Offset( mLoop, TheoraTextureObject ),
         "Should the video loop." );

   endGroup( "Theora" );

   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

GFXTextureObject* TheoraTextureObject::_texDelegate( U32 index )
{
   // Refresh the video texture state.
   mTheoraTexture.refresh();

   // No texture if not playing.
   if( !mTheoraTexture.isPlaying() )
   {
      // Was this video playing and should it loop?
      if(mIsPlaying && mLoop)
      {
         play();

         // It won't be ready this frame
         return NULL;
      }
      else
      {
         mIsPlaying = false;
         return NULL;
      }
   }

   // Return the Theora video texture for the current frame.
   return mTheoraTexture.getTexture();
}

//-----------------------------------------------------------------------------

bool TheoraTextureObject::onAdd()
{
   if( !Parent::onAdd() )
      return false;

   if( mFilename.isEmpty() )
   {
      Con::errorf( "TheoraTextureObject::onAdd - 'filename' must be set" );
      return false;
   }

   if( mTexTargetName.isEmpty() )
   {
      Con::errorf( "TheoraTextureObject::onAdd - 'texTargetName' not set" );
      return false;
   }

   if( !mTexTarget.registerWithName( mTexTargetName ) )
   {
      Con::errorf( "TheoraTextureObject::onAdd - Could not register texture target '%s", mTexTargetName.c_str() );
      return false;
   }

   return true;
}

//-----------------------------------------------------------------------------

void TheoraTextureObject::onRemove()
{
   // Stop playback if it's running.
   mTheoraTexture.stop();

   // Unregister the texture target.
   mTexTarget.unregister();

   Parent::onRemove();
}

//-----------------------------------------------------------------------------

void TheoraTextureObject::play()
{
   if( mTheoraTexture.getFilename().isEmpty() || mTheoraTexture.getFilename() != mFilename )
   {
      if( !mTheoraTexture.setFile( mFilename, mSFXDescription ) )
      {
         Con::errorf( "TheoraTextureObject::play - Could not load video '%s'", mFilename.c_str() );
         return;
      }
   }

   mIsPlaying = true;
   mTheoraTexture.play();
}

//=============================================================================
//    Console API.
//=============================================================================
// MARK: ---- Console API ----

//-----------------------------------------------------------------------------

DefineConsoleMethod( TheoraTextureObject, play, void, (),,
   "Start playback of the video." )
{
   object->play();
}

//-----------------------------------------------------------------------------

DefineConsoleMethod( TheoraTextureObject, stop, void, (),,
   "Stop playback of the video." )
{
   object->stop();
}

//-----------------------------------------------------------------------------

DefineConsoleMethod( TheoraTextureObject, pause, void, (),,
   "Pause playback of the video." )
{
   object->pause();
}

#endif // TORQUE_OGGTHEORA
