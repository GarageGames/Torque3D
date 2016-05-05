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
#include "gfx/gfxTextureProfile.h"

#include "gfx/gfxTextureObject.h"
#include "gfx/bitmap/gBitmap.h"
#include "core/strings/stringFunctions.h"
#include "console/console.h"
#include "console/engineAPI.h"


// Set up defaults...
GFX_ImplementTextureProfile(GFXDefaultRenderTargetProfile, 
                            GFXTextureProfile::DiffuseMap, 
                            GFXTextureProfile::PreserveSize | GFXTextureProfile::NoMipmap | GFXTextureProfile::RenderTarget, 
                            GFXTextureProfile::NONE);
GFX_ImplementTextureProfile(GFXDefaultStaticDiffuseProfile, 
                            GFXTextureProfile::DiffuseMap, 
                            GFXTextureProfile::Static, 
                            GFXTextureProfile::NONE);
GFX_ImplementTextureProfile(GFXDefaultStaticNormalMapProfile, 
                            GFXTextureProfile::NormalMap, 
                            GFXTextureProfile::Static, 
                            GFXTextureProfile::NONE);
GFX_ImplementTextureProfile(GFXDefaultStaticDXT5nmProfile, 
                            GFXTextureProfile::NormalMap, 
                            GFXTextureProfile::Static, 
                            GFXTextureProfile::DXT5);
GFX_ImplementTextureProfile(GFXDefaultPersistentProfile,
                            GFXTextureProfile::DiffuseMap, 
                            GFXTextureProfile::PreserveSize | GFXTextureProfile::Static | GFXTextureProfile::KeepBitmap, 
                            GFXTextureProfile::NONE);
GFX_ImplementTextureProfile(GFXSystemMemProfile, 
                            GFXTextureProfile::DiffuseMap, 
                            GFXTextureProfile::PreserveSize | GFXTextureProfile::NoMipmap | GFXTextureProfile::SystemMemory,
                            GFXTextureProfile::NONE);
GFX_ImplementTextureProfile(GFXDefaultZTargetProfile,
                            GFXTextureProfile::DiffuseMap, 
                            GFXTextureProfile::PreserveSize | GFXTextureProfile::NoMipmap | GFXTextureProfile::ZTarget | GFXTextureProfile::NoDiscard, 
                            GFXTextureProfile::NONE);
GFX_ImplementTextureProfile(GFXDynamicTextureProfile,
                            GFXTextureProfile::DiffuseMap,
                            GFXTextureProfile::Dynamic,
                            GFXTextureProfile::NONE);

//-----------------------------------------------------------------------------

GFXTextureProfile *GFXTextureProfile::smHead = NULL;
U32 GFXTextureProfile::smProfileCount = 0;

GFXTextureProfile::GFXTextureProfile(const String &name, Types type, U32 flag, Compression compression)
:  mName( name )
{
   // Take type, flag, and compression and produce a munged profile word.
   mProfile = (type & (BIT(TypeBits + 1) - 1)) |
             ((flag & (BIT(FlagBits + 1) - 1)) << TypeBits) | 
             ((compression & (BIT(CompressionBits + 1) - 1)) << (FlagBits + TypeBits));   

   // Stick us on the linked list.
   mNext = smHead;
   smHead = this;
   ++smProfileCount;

   // Now do some sanity checking. (Ben is not proud of this code.)
   AssertFatal( (testFlag(Dynamic) && !testFlag(Static)) 
                  || (!testFlag(Dynamic) && !testFlag(Static)) 
                  || (!testFlag(Dynamic) &&  testFlag(Static)), 
                  "GFXTextureProfile::GFXTextureProfile - Cannot have a texture profile be both static and dynamic!");
   mDownscale = 0;
}

void GFXTextureProfile::init()
{
   // Do something, anything?
}

GFXTextureProfile * GFXTextureProfile::find(const String &name)
{
   // Not really necessary at this time.
   return NULL;
}

void GFXTextureProfile::collectStats( Flags flags, GFXTextureProfileStats *stats )
{
   // Walk the profile list.
   GFXTextureProfile *curr = smHead;
   while ( curr )
   {
      if ( curr->testFlag( flags ) )
         (*stats) += curr->getStats();

      curr = curr->mNext;
   }
}

void GFXTextureProfile::updateStatsForCreation(GFXTextureObject *t)
{
   if(t->mProfile)
   {
      t->mProfile->incActiveCopies();
      t->mProfile->mStats.allocatedTextures++;
      
      U32 texSize = t->getHeight() * t->getWidth();
      U32 byteSize = t->getEstimatedSizeInBytes();

      t->mProfile->mStats.allocatedTexels += texSize;
      t->mProfile->mStats.allocatedBytes  += byteSize;

      t->mProfile->mStats.activeTexels += texSize;
      t->mProfile->mStats.activeBytes += byteSize; 
   }
}

void GFXTextureProfile::updateStatsForDeletion(GFXTextureObject *t)
{
   if(t->mProfile)
   {
      t->mProfile->decActiveCopies();
      
      U32 texSize = t->getHeight() * t->getWidth();
      U32 byteSize = t->getEstimatedSizeInBytes();

      t->mProfile->mStats.activeTexels -= texSize;
      t->mProfile->mStats.activeBytes -= byteSize; 
   }
}

DefineEngineFunction( getTextureProfileStats, String, (),,
   "Returns a list of texture profiles in the format: ProfileName TextureCount TextureMB\n"
   "@ingroup GFX\n" )
{
   StringBuilder result;

   GFXTextureProfile *profile = GFXTextureProfile::getHead();
   while ( profile )
   {
      const GFXTextureProfileStats &stats = profile->getStats();

      F32 mb = ( stats.activeBytes / 1024.0f ) / 1024.0f;

      result.format( "%s %d %0.2f\n",
         profile->getName().c_str(),
         stats.activeCount,
         mb );

      profile = profile->getNext();
   }

   return result.end();
}

