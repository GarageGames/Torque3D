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
#include "gfx/gfxTextureManager.h"

#include "gfx/gfxDevice.h"
#include "gfx/gfxCardProfile.h"
#include "gfx/gfxStringEnumTranslate.h"
#include "gfx/bitmap/ddsUtils.h"
#include "core/strings/stringFunctions.h"
#include "core/util/safeDelete.h"
#include "core/resourceManager.h"
#include "core/volume.h"
#include "core/util/dxt5nmSwizzle.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"

using namespace Torque;

//#define DEBUG_SPEW


S32 GFXTextureManager::smTextureReductionLevel = 0;

String GFXTextureManager::smMissingTexturePath("core/art/missingTexture");
String GFXTextureManager::smUnavailableTexturePath("core/art/unavailable");
String GFXTextureManager::smWarningTexturePath("core/art/warnmat");

GFXTextureManager::EventSignal GFXTextureManager::smEventSignal;

static const String  sDDSExt( "dds" );

void GFXTextureManager::init()
{
   Con::addVariable( "$pref::Video::textureReductionLevel", TypeS32, &smTextureReductionLevel,
      "The number of mipmap levels to drop on loaded textures to reduce "
      "video memory usage.  It will skip any textures that have been defined "
      "as not allowing down scaling.\n"
      "@ingroup GFX\n" );

   Con::addVariable( "$pref::Video::missingTexturePath", TypeRealString, &smMissingTexturePath,
      "The file path of the texture to display when the requested texture is missing.\n"
      "@ingroup GFX\n" );

   Con::addVariable( "$pref::Video::unavailableTexturePath", TypeRealString, &smUnavailableTexturePath,
      "@brief The file path of the texture to display when the requested texture is unavailable.\n\n"
      "Often this texture is used by GUI controls to indicate that the request image is unavailable.\n"
      "@ingroup GFX\n" );

   Con::addVariable( "$pref::Video::warningTexturePath", TypeRealString, &smWarningTexturePath,
      "The file path of the texture used to warn the developer.\n"
      "@ingroup GFX\n" );
}

GFXTextureManager::GFXTextureManager()
{
   mListHead = mListTail = NULL;
   mTextureManagerState = GFXTextureManager::Living;

   // Set up the hash table
   mHashCount = 1023;
   mHashTable = new GFXTextureObject *[mHashCount];
   for(U32 i = 0; i < mHashCount; i++)
      mHashTable[i] = NULL;
}

GFXTextureManager::~GFXTextureManager()
{
   if( mHashTable )
      SAFE_DELETE_ARRAY( mHashTable );

   mCubemapTable.clear();
}

U32 GFXTextureManager::getTextureDownscalePower( GFXTextureProfile *profile )
{
   if ( !profile || profile->canDownscale() )
      return smTextureReductionLevel;

   return 0;
}

bool GFXTextureManager::validateTextureQuality( GFXTextureProfile *profile, U32 &width, U32 &height )
{
   U32 scaleFactor = getTextureDownscalePower( profile );
   if ( scaleFactor == 0 )
      return true;

   // Otherwise apply the appropriate scale...
   width  >>= scaleFactor;
   height >>= scaleFactor;

   return true;
}

void GFXTextureManager::kill()
{
   AssertFatal( mTextureManagerState != GFXTextureManager::Dead, "Texture Manager already killed!" );

   // Release everything in the cache we can
   // so we don't leak any textures.
   cleanupCache();

   GFXTextureObject *curr = mListHead;
   GFXTextureObject *temp;

   // Actually delete all the textures we know about.
   while( curr != NULL ) 
   {
      temp = curr->mNext;
      curr->kill();
      curr = temp;
   }

   mCubemapTable.clear();

   mTextureManagerState = GFXTextureManager::Dead;
}

void GFXTextureManager::zombify()
{
   AssertFatal( mTextureManagerState != GFXTextureManager::Zombie, "Texture Manager already a zombie!" );

   // Notify everyone that cares about the zombification!
   smEventSignal.trigger( GFXZombify );

   // Release unused pool textures.
   cleanupPool();

   // Release everything in the cache we can.
   cleanupCache();

   // Free all the device copies of the textures.
   GFXTextureObject *temp = mListHead;
   while( temp != NULL ) 
   {
      freeTexture( temp, true );
      temp = temp->mNext;
   }

   // Finally, note our state.
   mTextureManagerState = GFXTextureManager::Zombie;
}

void GFXTextureManager::resurrect()
{
   // Reupload all the device copies of the textures.
   GFXTextureObject *temp = mListHead;

   while( temp != NULL ) 
   {
      refreshTexture( temp );
      temp = temp->mNext;
   }

   // Notify callback registries.
   smEventSignal.trigger( GFXResurrect );
   
   // Update our state.
   mTextureManagerState = GFXTextureManager::Living;
}

void GFXTextureManager::cleanupPool()
{
   PROFILE_SCOPE( GFXTextureManager_CleanupPool );

   TexturePoolMap::Iterator iter = mTexturePool.begin();
   for ( ; iter != mTexturePool.end(); )
   {
      if ( iter->value->getRefCount() == 1 )
      {
         // This texture is unreferenced, so take the time
         // now to completely remove it from the pool.
         TexturePoolMap::Iterator unref = iter;
         ++iter;
         unref->value = NULL;
         mTexturePool.erase( unref );
         continue;
      }

      ++iter;
   }
}

void GFXTextureManager::requestDeleteTexture( GFXTextureObject *texture )
{
   // If this is a non-cached texture then just really delete it.
   if ( texture->mTextureLookupName.isEmpty() )
   {
      delete texture;
      return;
   }

   // Set the time and store it.
   texture->mDeleteTime = Platform::getTime();
   mToDelete.push_back_unique( texture );
}

void GFXTextureManager::cleanupCache( U32 secondsToLive )
{
   PROFILE_SCOPE( GFXTextureManager_CleanupCache );

   U32 killTime = Platform::getTime() - secondsToLive;

   for ( U32 i=0; i < mToDelete.size(); )
   {
      GFXTextureObject *tex = mToDelete[i];

      // If the texture was picked back up by a user
      // then just remove it from the list.
      if ( tex->getRefCount() != 0 )
      {
         mToDelete.erase_fast( i );
         continue;
      }

      // If its time has expired delete it for real.
      if ( tex->mDeleteTime <= killTime )
      {
         //Con::errorf( "Killed texture: %s", tex->mTextureLookupName.c_str() );
         delete tex;
         mToDelete.erase_fast( i );
         continue;
      }

      i++;
   }
}

GFXTextureObject *GFXTextureManager::_lookupTexture( const char *hashName, const GFXTextureProfile *profile  )
{
   GFXTextureObject *ret = hashFind( hashName );

   // TODO: Profile checking HERE

   return ret;
}

GFXTextureObject *GFXTextureManager::_lookupTexture( const DDSFile *ddsFile, const GFXTextureProfile *profile )
{
   if( ddsFile->getTextureCacheString().isNotEmpty() )
   {
      // Call _lookupTexture()
      return _lookupTexture( ddsFile->getTextureCacheString(), profile );
   }

   return NULL;
}

GFXTextureObject *GFXTextureManager::createTexture( GBitmap *bmp, const String &resourceName, GFXTextureProfile *profile, bool deleteBmp )
{
   AssertFatal(bmp, "GFXTextureManager::createTexture() - Got NULL bitmap!");

   GFXTextureObject *cacheHit = _lookupTexture( resourceName, profile );
   if( cacheHit != NULL)
   {
      // Con::errorf("Cached texture '%s'", (resourceName.isNotEmpty() ? resourceName.c_str() : "unknown"));
      if (deleteBmp)
         delete bmp;
      return cacheHit;
   }

   return _createTexture( bmp, resourceName, profile, deleteBmp, NULL );
}

GFXTextureObject *GFXTextureManager::_createTexture(  GBitmap *bmp, 
                                                      const String &resourceName, 
                                                      GFXTextureProfile *profile, 
                                                      bool deleteBmp,
                                                      GFXTextureObject *inObj )
{
   PROFILE_SCOPE( GFXTextureManager_CreateTexture_Bitmap );
   
   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[GFXTextureManager] _createTexture (GBitmap) '%s'",
      resourceName.c_str()
   );
   #endif

   // Massage the bitmap based on any resize rules.
   U32 scalePower = getTextureDownscalePower( profile );

   GBitmap *realBmp = bmp;
   U32 realWidth = bmp->getWidth();
   U32 realHeight = bmp->getHeight();

   if (  scalePower && 
         isPow2(bmp->getWidth()) && 
         isPow2(bmp->getHeight()) && 
         profile->canDownscale() )
   {
      // We only work with power of 2 textures for now, so we 
      // don't have to worry about padding.

      // We downscale the bitmap on the CPU... this is the reason
      // you should be using DDS which already has good looking mips.
      GBitmap *padBmp = bmp;
      padBmp->extrudeMipLevels();
      scalePower = getMin( scalePower, padBmp->getNumMipLevels() - 1 );

      realWidth  = getMax( (U32)1, padBmp->getWidth() >> scalePower );
      realHeight = getMax( (U32)1, padBmp->getHeight() >> scalePower );
      realBmp = new GBitmap( realWidth, realHeight, false, bmp->getFormat() );

      // Copy to the new bitmap...
      dMemcpy( realBmp->getWritableBits(), 
               padBmp->getBits(scalePower),
               padBmp->getBytesPerPixel() * realWidth * realHeight );

      // This line is commented out because createPaddedBitmap is commented out.
      // If that line is added back in, this line should be added back in.
      // delete padBmp;
   }

   // Call the internal create... (use the real* variables now, as they
   // reflect the reality of the texture we are creating.)
   U32 numMips = 0;
   GFXFormat realFmt = realBmp->getFormat();
   _validateTexParams( realWidth, realHeight, profile, numMips, realFmt );

   GFXTextureObject *ret;
   if ( inObj )
   {
      // If the texture has changed in dimensions 
      // then we need to recreate it.
      if (  inObj->getWidth() != realWidth ||
            inObj->getHeight() != realHeight ||
            inObj->getFormat() != realFmt )
         ret = _createTextureObject( realHeight, realWidth, 0, realFmt, profile, numMips, false, 0, inObj );
      else
         ret = inObj;
   }
   else
      ret = _createTextureObject(realHeight, realWidth, 0, realFmt, profile, numMips );

   if(!ret)
   {
      Con::errorf("GFXTextureManager - failed to create texture (1) for '%s'", (resourceName.isNotEmpty() ? resourceName.c_str() : "unknown"));
      return NULL;
   }

   // Extrude mip levels
   // Don't do this for fonts!
   if( ret->mMipLevels > 1 && ( realBmp->getNumMipLevels() == 1 ) && ( realBmp->getFormat() != GFXFormatA8 ) &&
      isPow2( realBmp->getHeight() ) && isPow2( realBmp->getWidth() ) && !profile->noMip() )
   {
      // NOTE: This should really be done by extruding mips INTO a DDS file instead
      // of modifying the gbitmap
      realBmp->extrudeMipLevels(false); 
   }

   // If _validateTexParams kicked back a different format, than there needs to be
   // a conversion
   DDSFile *bmpDDS = NULL;
   if( realBmp->getFormat() != realFmt )
   {
      const GFXFormat oldFmt = realBmp->getFormat();

      // TODO: Set it up so that ALL format conversions use DDSFile. Rip format
      // switching out of GBitmap entirely.
      if( !realBmp->setFormat( realFmt ) )
      {
         // This is not the ideal implementation...
         bmpDDS = DDSFile::createDDSFileFromGBitmap( realBmp );

         bool convSuccess = false;

         if( bmpDDS != NULL )
         {       
            // This shouldn't live here, I don't think
            switch( realFmt )
            {
               case GFXFormatDXT1:
               case GFXFormatDXT2:
               case GFXFormatDXT3:
               case GFXFormatDXT4:
               case GFXFormatDXT5:
                  // If this is a Normal Map profile, than the data needs to be conditioned
                  // to use the swizzle trick
                  if( ret->mProfile->getType() == GFXTextureProfile::NormalMap )
                  {
                     PROFILE_START(DXT_DXTNMSwizzle);
                     static DXT5nmSwizzle sDXT5nmSwizzle;
                     DDSUtil::swizzleDDS( bmpDDS, sDXT5nmSwizzle );
                     PROFILE_END();
                  }

                  convSuccess = DDSUtil::squishDDS( bmpDDS, realFmt );
                  break;
               default:
                  AssertFatal(false, "Attempting to convert to a non-DXT format");
                  break;
            }
         }

         if( !convSuccess )
         {
            Con::errorf( "[GFXTextureManager]: Failed to change source format from %s to %s. Cannot create texture.", 
               GFXStringTextureFormat[oldFmt], GFXStringTextureFormat[realFmt] );
            delete bmpDDS;

            return NULL;
         }
      }
#ifdef TORQUE_DEBUG
      else
      {
         //Con::warnf( "[GFXTextureManager]: Changed bitmap format from %s to %s.", 
         //   GFXStringTextureFormat[oldFmt], GFXStringTextureFormat[realFmt] );
      }
#endif
   }

   // Call the internal load...
   if( ( bmpDDS == NULL && !_loadTexture( ret, realBmp ) ) || // If we aren't doing a DDS format change, use bitmap load
       ( bmpDDS != NULL && !_loadTexture( ret, bmpDDS ) ) )   // If there is a DDS, than load that instead. A format change took place.
   {
      Con::errorf("GFXTextureManager - failed to load GBitmap for '%s'", (resourceName.isNotEmpty() ? resourceName.c_str() : "unknown"));
      return NULL;
   }

   // Do statistics and book-keeping...
   
   //    - info for the texture...
   ret->mTextureLookupName = resourceName;
   ret->mBitmapSize.set(realWidth, realHeight,0);

#ifdef TORQUE_DEBUG
   if (resourceName.isNotEmpty())
      ret->mDebugDescription = resourceName;
   else
      ret->mDebugDescription = "Anonymous Texture Object";

#endif

   if(profile->doStoreBitmap())
   {
      // NOTE: may store a downscaled copy!
      SAFE_DELETE( ret->mBitmap );
      SAFE_DELETE( ret->mDDS );

      if( bmpDDS == NULL )
         ret->mBitmap = new GBitmap( *realBmp );
      else
         ret->mDDS = bmpDDS;
   }
   else
   {
      // Delete the DDS if we made one
      SAFE_DELETE( bmpDDS );
   }

   if ( !inObj )
      _linkTexture( ret );

   //    - output debug info?
   // Save texture for debug purpose
   //   static int texId = 0;
   //   char buff[256];
   //   dSprintf(buff, sizeof(buff), "tex_%d", texId++);
   //   bmp->writePNGDebug(buff);
   //   texId++;

   // Before we delete the bitmap save our transparency flag
   ret->mHasTransparency = realBmp->getHasTransparency();

   // Some final cleanup...
   if(realBmp != bmp)
      SAFE_DELETE(realBmp);
   if (deleteBmp)
      SAFE_DELETE(bmp);

   // Return the new texture!
   return ret;
}

GFXTextureObject *GFXTextureManager::createTexture( DDSFile *dds, GFXTextureProfile *profile, bool deleteDDS )
{
   AssertFatal(dds, "GFXTextureManager::createTexture() - Got NULL dds!");

   // Check the cache first...
   GFXTextureObject *cacheHit = _lookupTexture( dds, profile );
   if ( cacheHit )
   {
      //      Con::errorf("Cached texture '%s'", (fileName.isNotEmpty() ? fileName.c_str() : "unknown"));
      if( deleteDDS )
         delete dds;

      return cacheHit;
   }

   return _createTexture( dds, profile, deleteDDS, NULL );
}

GFXTextureObject *GFXTextureManager::_createTexture(  DDSFile *dds,
                                                      GFXTextureProfile *profile,
                                                      bool deleteDDS,
                                                      GFXTextureObject *inObj )
{
   PROFILE_SCOPE( GFXTextureManager_CreateTexture_DDS );

   const char *fileName = dds->getTextureCacheString();
   if( !fileName )
      fileName = "unknown";

   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[GFXTextureManager] _createTexture (DDS) '%s'",
      fileName
   );
   #endif

   // Ignore padding from the profile.
   U32 numMips = dds->mMipMapCount;
   GFXFormat fmt = dds->mFormat;
   _validateTexParams( dds->getHeight(), dds->getWidth(), profile, numMips, fmt );

   if( fmt != dds->mFormat )
   {
      Con::errorf( "GFXTextureManager - failed to validate texture parameters for DDS file '%s'", fileName );
      return NULL;
   }

   // Call the internal create... (use the real* variables now, as they
   // reflect the reality of the texture we are creating.)

   GFXTextureObject *ret;
   if ( inObj )
   {
      // If the texture has changed in dimensions 
      // then we need to recreate it.   
      if (  inObj->getWidth() != dds->getWidth() ||
            inObj->getHeight() != dds->getHeight() ||
            inObj->getFormat() != fmt ||
            inObj->getMipLevels() != numMips )
         ret = _createTextureObject(   dds->getHeight(), dds->getWidth(), 0, 
                                       fmt, profile, numMips, 
                                       true, 0, inObj );
      else
         ret = inObj;
   }
   else
      ret =  _createTextureObject(  dds->getHeight(), dds->getWidth(), 0, 
                                    fmt, profile, numMips, true );


   if(!ret)
   {
      Con::errorf("GFXTextureManager - failed to create texture (1) for '%s' DDSFile.", fileName);
      return NULL;
   }

   // Call the internal load...
   if(!_loadTexture(ret, dds))
   {
      Con::errorf("GFXTextureManager - failed to load DDS for '%s'", fileName);
      return NULL;
   }

   // Do statistics and book-keeping...

   //    - info for the texture...
   ret->mTextureLookupName = dds->getTextureCacheString();
   ret->mBitmapSize.set( dds->mWidth, dds->mHeight, 0 );

#ifdef TORQUE_DEBUG
   ret->mDebugDescription = fileName;
#endif

   if(profile->doStoreBitmap())
   {
      // NOTE: may store a downscaled copy!
      SAFE_DELETE( ret->mBitmap );
      SAFE_DELETE( ret->mDDS );

      ret->mDDS = new DDSFile( *dds );
   }

   if ( !inObj )
      _linkTexture( ret );

   //    - output debug info?
   // Save texture for debug purpose
   //   static int texId = 0;
   //   char buff[256];
   //   dSprintf(buff, sizeof(buff), "tex_%d", texId++);
   //   bmp->writePNGDebug(buff);
   //   texId++;

   // Save our transparency flag
   ret->mHasTransparency = dds->getHasTransparency();

   if( deleteDDS )
      delete dds;

   // Return the new texture!
   return ret;
}

GFXTextureObject *GFXTextureManager::createTexture( const Torque::Path &path, GFXTextureProfile *profile )
{
   PROFILE_SCOPE( GFXTextureManager_createTexture );
   
   // Resource handles used for loading.  Hold on to them
   // throughout this function so that change notifications
   // don't get added, then removed, and then re-added.
   
   Resource< DDSFile > dds;
   Resource< GBitmap > bitmap;
   
   // We need to handle path's that have had "incorrect"
   // extensions parsed out of the file name
   Torque::Path correctPath = path;

   bool textureExt = false;

   // Easiest case to handle is when there isn't an extension
   if (path.getExtension().isEmpty())
      textureExt = true;

   // Since "dds" isn't registered with GBitmap currently we
   // have to test it separately
   if (sDDSExt.equal( path.getExtension(), String::NoCase ) )
      textureExt = true;

   // Now loop through the rest of the GBitmap extensions
   // to see if we have any matches
   for ( U32 i = 0; i < GBitmap::sRegistrations.size(); i++ )
   {
      // If we have gotten a match (either in this loop or before)
      // then we can exit
      if (textureExt)
         break;

      const GBitmap::Registration   &reg = GBitmap::sRegistrations[i];
      const Vector<String>          &extensions = reg.extensions;

      for ( U32 j = 0; j < extensions.size(); ++j )
      {    
         if ( extensions[j].equal( path.getExtension(), String::NoCase ) )
         {
            // Found a valid texture extension
            textureExt = true;
            break;
         }
      }
   }

   // If we didn't find a valid texture extension then assume that
   // the parsed out "extension" was actually intended to be part of
   // the texture name so add it back
   if (!textureExt)
   {
      correctPath.setFileName( Torque::Path::Join( path.getFileName(), '.', path.getExtension() ) );
      correctPath.setExtension( String::EmptyString );
   }

   // Check the cache first...
   String pathNoExt = Torque::Path::Join( correctPath.getRoot(), ':', correctPath.getPath() );
   pathNoExt = Torque::Path::Join( pathNoExt, '/', correctPath.getFileName() );

   GFXTextureObject *retTexObj = _lookupTexture( pathNoExt, profile );
   if( retTexObj )
      return retTexObj;

   const U32 scalePower = getTextureDownscalePower( profile );

   // If this is a valid file (has an extension) than load it
   Path realPath;
   if( Torque::FS::IsFile( correctPath ) )
   {
      // Check for DDS
      if( sDDSExt.equal(correctPath.getExtension(), String::NoCase ) )
      {
         dds = DDSFile::load( correctPath, scalePower );
         if( dds != NULL )
         {
            realPath = dds.getPath();
            retTexObj = createTexture( dds, profile, false );
         }
      }
      else // Let GBitmap take care of it
      {
         bitmap = GBitmap::load( correctPath );
         if( bitmap != NULL )
         {
            realPath = bitmap.getPath();
            retTexObj = createTexture( bitmap, pathNoExt, profile, false );
         }
      }      
   }
   else
   {
      // NOTE -- We should probably remove the code from GBitmap that tries different
      // extensions for things GBitmap loads, and move it here. I think it should
      // be a bit more involved than just a list of extensions. Some kind of 
      // extension registration thing, maybe.

      // Check to see if there is a .DDS file with this name (if no extension is provided)
      Torque::Path tryDDSPath = pathNoExt;
      if( tryDDSPath.getExtension().isNotEmpty() )
         tryDDSPath.setFileName( tryDDSPath.getFullFileName() );
      tryDDSPath.setExtension( sDDSExt );

      if( Torque::FS::IsFile( tryDDSPath ) )
      {
         dds = DDSFile::load( tryDDSPath, scalePower );
         if( dds != NULL )
         {
            realPath = dds.getPath();
            retTexObj = createTexture( dds, profile, false );
         }
      }
      
      // Otherwise, retTexObj stays NULL, and fall through to the generic GBitmap
      // load.
   }

   // If we still don't have a texture object yet, feed the correctPath to GBitmap and
   // it will try a bunch of extensions
   if( retTexObj == NULL )
   {
      // Find and load the texture.
      bitmap = GBitmap::load( correctPath );

      if ( bitmap != NULL )
      {
         realPath = bitmap.getPath();
         retTexObj = createTexture( bitmap, pathNoExt, profile, false );
      }
   }

   if ( retTexObj )
   {
      // Store the path for later use.
      retTexObj->mPath = realPath;

      // Register the texture file for change notifications.
      FS::AddChangeNotification( retTexObj->getPath(), this, &GFXTextureManager::_onFileChanged );
   }

   // Could put in a final check for 'retTexObj == NULL' here as an error message.

   return retTexObj;
}

GFXTextureObject *GFXTextureManager::createTexture(  U32 width, U32 height, void *pixels, GFXFormat format, GFXTextureProfile *profile )
{
   // For now, stuff everything into a GBitmap and pass it off... This may need to be revisited -- BJG
   GBitmap *bmp = new GBitmap(width, height, 0, format);
   dMemcpy(bmp->getWritableBits(), pixels, width * height * bmp->getBytesPerPixel());

   return createTexture( bmp, String::EmptyString, profile, true );
}

GFXTextureObject *GFXTextureManager::createTexture( U32 width, U32 height, GFXFormat format, GFXTextureProfile *profile, U32 numMipLevels, S32 antialiasLevel )
{
   // Deal with sizing issues...
   U32 localWidth = width;
   U32 localHeight = height;

   // TODO: Format check HERE! -patw

   validateTextureQuality(profile, localWidth, localHeight);

   U32 numMips = numMipLevels;
   GFXFormat checkFmt = format;
   _validateTexParams( localWidth, localHeight, profile, numMips, checkFmt );

   AssertFatal( checkFmt == format, "Anonymous texture didn't get the format it wanted." );

   GFXTextureObject *outTex = NULL;

   // If this is a pooled profile then look there first.
   if ( profile->isPooled() )
   {
      outTex = _findPooledTexure(   localWidth, localHeight, checkFmt, 
                                    profile, numMips, antialiasLevel );

      // If we got a pooled texture then its
      // already setup... just return it.
      if ( outTex )
         return outTex;
   }
   
   // Create the texture if we didn't get one from the pool.
   if ( !outTex )
   {
      outTex = _createTextureObject( localHeight, localWidth, 0, format, profile, numMips, false, antialiasLevel );

      // Make sure we add it to the pool.
      if ( outTex && profile->isPooled() )
         mTexturePool.insertEqual( profile, outTex );
   }

   if ( !outTex )
   {
      Con::errorf("GFXTextureManager - failed to create anonymous texture.");
      return NULL;
   }

   // And do book-keeping...
   //    - texture info
   outTex->mBitmapSize.set(localWidth, localHeight, 0);
   outTex->mAntialiasLevel = antialiasLevel;

   // PWTODO: Need to assign this a lookup name before _linkTexture() is called
   // otherwise it won't get a hash insert call

   _linkTexture( outTex );

   return outTex;
}

GFXTextureObject *GFXTextureManager::createTexture(   U32 width,
                                                      U32 height,
                                                      U32 depth,
                                                      void *pixels,
                                                      GFXFormat format,
                                                      GFXTextureProfile *profile )
{
   PROFILE_SCOPE( GFXTextureManager_CreateTexture_3D );

   // Create texture...
   GFXTextureObject *ret = _createTextureObject( height, width, depth, format, profile, 1 );

   if(!ret)
   {
      Con::errorf("GFXTextureManager - failed to create anonymous texture.");
      return NULL;
   }

   // Call the internal load...
   if( !_loadTexture( ret, pixels ) )
   {
      Con::errorf("GFXTextureManager - failed to load volume texture" );
      return NULL;
   }


   // And do book-keeping...
   //    - texture info
   ret->mBitmapSize.set( width, height, depth );

   _linkTexture( ret );


   // Return the new texture!
   return ret;
}

GFXTextureObject* GFXTextureManager::_findPooledTexure(  U32 width, 
                                                         U32 height, 
                                                         GFXFormat format, 
                                                         GFXTextureProfile *profile,
                                                         U32 numMipLevels,
                                                         S32 antialiasLevel )
{
   PROFILE_SCOPE( GFXTextureManager_FindPooledTexure );

   GFXTextureObject *outTex;

   // First see if we have a free one in the pool.
   TexturePoolMap::Iterator iter = mTexturePool.find( profile );
   for ( ; iter != mTexturePool.end() && iter->key == profile; iter++ )
   {
      outTex = iter->value;

      // If the reference count is 1 then we're the only
      // ones holding on to this texture and we can hand
      // it out if the size matches... else its in use.
      if ( outTex->getRefCount() != 1 )
         continue;

      // Check for a match... if so return it.  The assignment
      // to a GFXTexHandle will take care of incrementing the
      // reference count and keeping it from being handed out
      // to anyone else.
      if (  outTex->getFormat() == format &&
            outTex->getWidth() == width &&
            outTex->getHeight() == height &&            
            outTex->getMipLevels() == numMipLevels &&
            outTex->mAntialiasLevel == antialiasLevel )
         return outTex;
   }

   return NULL;
}

void GFXTextureManager::hashInsert( GFXTextureObject *object )
{
   if ( object->mTextureLookupName.isEmpty() )
      return;
      
   U32 key = object->mTextureLookupName.getHashCaseInsensitive() % mHashCount;
   object->mHashNext = mHashTable[key];
   mHashTable[key] = object;
}

void GFXTextureManager::hashRemove( GFXTextureObject *object )
{
   if ( object->mTextureLookupName.isEmpty() )
      return;

   U32 key = object->mTextureLookupName.getHashCaseInsensitive() % mHashCount;
   GFXTextureObject **walk = &mHashTable[key];
   while(*walk)
   {
      if(*walk == object)
      {
         *walk = object->mHashNext;
         break;
      }
      walk = &((*walk)->mHashNext);
   }
}

GFXTextureObject* GFXTextureManager::hashFind( const String &name )
{
   if ( name.isEmpty() )
      return NULL;

   U32 key = name.getHashCaseInsensitive() % mHashCount;
   GFXTextureObject *walk = mHashTable[key];
   for(; walk; walk = walk->mHashNext)
   {
      if( walk->mTextureLookupName.equal( name, String::NoCase ) )
         break;
   }

   return walk;
}

void GFXTextureManager::freeTexture(GFXTextureObject *texture, bool zombify)
{
   // Ok, let the backend deal with it.
   _freeTexture(texture, zombify);
}

void GFXTextureManager::refreshTexture(GFXTextureObject *texture)
{
   _refreshTexture(texture);
}

void GFXTextureManager::_linkTexture( GFXTextureObject *obj )
{
   // info for the profile
   GFXTextureProfile::updateStatsForCreation(obj);

   // info for the cache
   hashInsert(obj);

   // info for the master list
   if( mListHead == NULL )
      mListHead = obj;

   if( mListTail != NULL ) 
      mListTail->mNext = obj;

   obj->mPrev = mListTail;
   mListTail = obj;
}

void GFXTextureManager::deleteTexture( GFXTextureObject *texture )
{
   if ( mTextureManagerState == GFXTextureManager::Dead )
      return;

   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[GFXTextureManager] deleteTexture '%s'",
      texture->mTextureLookupName.c_str()
   );
   #endif

   if( mListHead == texture )
      mListHead = texture->mNext;
   if( mListTail == texture )
      mListTail = texture->mPrev;

   hashRemove( texture );

   // If we have a path for the texture then
   // remove change notifications for it.
   Path texPath = texture->getPath();
   if ( !texPath.isEmpty() )
      FS::RemoveChangeNotification( texPath, this, &GFXTextureManager::_onFileChanged );

   GFXTextureProfile::updateStatsForDeletion(texture);

   freeTexture( texture );
}

void GFXTextureManager::_validateTexParams( const U32 width, const U32 height, 
                                          const GFXTextureProfile *profile, 
                                          U32 &inOutNumMips, GFXFormat &inOutFormat  )
{
   // Validate mipmap parameter. If this profile requests no mips, set mips to 1.
   if( profile->noMip() )
   {
      inOutNumMips = 1;
   }
   else if( !isPow2( width ) || !isPow2( height ) )
   {
      // If a texture is not power-of-2 in size for both dimensions, it must
      // have only 1 mip level.
      inOutNumMips = 1;
   }
   
   // Check format, and compatibility with texture profile requirements
   bool autoGenSupp = ( inOutNumMips == 0 );

   // If the format is non-compressed, and the profile requests a compressed format
   // than change the format.
   GFXFormat testingFormat = inOutFormat;
   if( profile->getCompression() != GFXTextureProfile::NONE )
   {
      const S32 offset = profile->getCompression() - GFXTextureProfile::DXT1;
      testingFormat = GFXFormat( GFXFormatDXT1 + offset );

      // No auto-gen mips on compressed textures
      autoGenSupp = false;
   }

   // inOutFormat is not modified by this method
   GFXCardProfiler* cardProfiler = GFX->getCardProfiler();
   bool chekFmt = cardProfiler->checkFormat(testingFormat, profile, autoGenSupp);
   
   if( !chekFmt )
   {
      // It tested for a compressed format, and didn't like it
      if( testingFormat != inOutFormat && profile->getCompression() )
         testingFormat = inOutFormat; // Reset to requested format, and try again

      // Trying again here, so reset autogen mip
      autoGenSupp = ( inOutNumMips == 0 );

      // Wow more weak sauce. There should be a better way to do this.
      switch( inOutFormat )
      {
         case GFXFormatR8G8B8:
            testingFormat = GFXFormatR8G8B8X8;
            chekFmt = cardProfiler->checkFormat(testingFormat, profile, autoGenSupp);
            break;

         case GFXFormatA8:
            testingFormat = GFXFormatR8G8B8A8;
            chekFmt = cardProfiler->checkFormat(testingFormat, profile, autoGenSupp);
            break;
         
         default:
            chekFmt = cardProfiler->checkFormat(testingFormat, profile, autoGenSupp);
            break;
      }
   }

   // Write back num mips that need to be generated by GBitmap
   if( !chekFmt )
      Con::errorf( "Format %s not supported with specified profile.", GFXStringTextureFormat[inOutFormat] );
   else
   {
      inOutFormat = testingFormat;

      // If auto gen mipmaps were requested, and they aren't supported for whatever
      // reason, than write out the number of mips that need to be generated.
      //
      // NOTE: Does this belong here?
      if( inOutNumMips == 0 && !autoGenSupp )
      {
         U32 currWidth  = width;
         U32 currHeight = height;

         inOutNumMips = 1;
         do 
         {
            currWidth  >>= 1;
            currHeight >>= 1;
            if( currWidth == 0 )
               currWidth  = 1;
            if( currHeight == 0 ) 
               currHeight = 1;

            inOutNumMips++;
         } while ( currWidth != 1 && currHeight != 1 );
      }
   }
}

GFXCubemap* GFXTextureManager::createCubemap( const Torque::Path &path )
{
   // Very first thing... check the cache.
   CubemapTable::Iterator iter = mCubemapTable.find( path.getFullPath() );
   if ( iter != mCubemapTable.end() )
      return iter->value;

   // Not in the cache... we have to load it ourselves.

   // First check for a DDS file.
   if ( !sDDSExt.equal( path.getExtension(), String::NoCase ) )
   {
      // At the moment we only support DDS cubemaps.
      return NULL;
   }

   const U32 scalePower = getTextureDownscalePower( NULL );

   // Ok... load the DDS file then.
   Resource<DDSFile> dds = DDSFile::load( path, scalePower );
   if ( !dds || !dds->isCubemap() )
   {
      // This wasn't a cubemap... give up too.
      return NULL;
   }

   // We loaded the cubemap dds, so now we create the GFXCubemap from it.
   GFXCubemap *cubemap = GFX->createCubemap();
   cubemap->initStatic( dds );
   cubemap->_setPath( path.getFullPath() );

   // Store the cubemap into the cache.
   mCubemapTable.insertUnique( path.getFullPath(), cubemap );

   return cubemap;
}

void GFXTextureManager::releaseCubemap( GFXCubemap *cubemap )
{
   if ( mTextureManagerState == GFXTextureManager::Dead )
      return;

   const String &path = cubemap->getPath();

   CubemapTable::Iterator iter = mCubemapTable.find( path );
   if ( iter != mCubemapTable.end() && iter->value == cubemap )
      mCubemapTable.erase( iter );

   // If we have a path for the texture then
   // remove change notifications for it.
   //Path texPath = texture->getPath();
   //if ( !texPath.isEmpty() )
      //FS::RemoveChangeNotification( texPath, this, &GFXTextureManager::_onFileChanged );
}

void GFXTextureManager::_onFileChanged( const Torque::Path &path )
{
   String pathNoExt = Torque::Path::Join( path.getRoot(), ':', path.getPath() );
   pathNoExt = Torque::Path::Join( pathNoExt, '/', path.getFileName() );

   // See if we've got it loaded.
   GFXTextureObject *obj = hashFind( pathNoExt );
   if ( !obj || path != obj->getPath() )
      return;

   Con::errorf( "[GFXTextureManager::_onFileChanged] : File changed [%s]", path.getFullPath().c_str() );

   const U32 scalePower = getTextureDownscalePower( obj->mProfile );

   if ( sDDSExt.equal( path.getExtension(), String::NoCase) )
   {
      Resource<DDSFile> dds = DDSFile::load( path, scalePower );
      if ( dds )
         _createTexture( dds, obj->mProfile, false, obj );
   }
   else
   {
      Resource<GBitmap> bmp = GBitmap::load( path );
      if( bmp )
         _createTexture( bmp, obj->mTextureLookupName, obj->mProfile, false, obj );
   }
}

void GFXTextureManager::reloadTextures()
{
   GFXTextureObject *tex = mListHead;

   while ( tex != NULL ) 
   {
      const Torque::Path path( tex->mPath );
      if ( !path.isEmpty() )
      {
         const U32 scalePower = getTextureDownscalePower( tex->mProfile );

         if ( sDDSExt.equal( path.getExtension(), String::NoCase ) )
         {
            Resource<DDSFile> dds = DDSFile::load( path, scalePower );
            if ( dds )
               _createTexture( dds, tex->mProfile, false, tex );
         }
         else
         {
            Resource<GBitmap> bmp = GBitmap::load( path );
            if( bmp )
               _createTexture( bmp, tex->mTextureLookupName, tex->mProfile, false, tex );
         }
      }

      tex = tex->mNext;
   }
}

DefineEngineFunction( flushTextureCache, void, (),,
   "Releases all textures and resurrects the texture manager.\n"
   "@ingroup GFX\n" )
{
   if ( !GFX || !TEXMGR )
      return;

   TEXMGR->zombify();
   TEXMGR->resurrect();
}

DefineEngineFunction( cleanupTexturePool, void, (),,
   "Release the unused pooled textures in texture manager freeing up video memory.\n"
   "@ingroup GFX\n" )
{
   if ( !GFX || !TEXMGR )
      return;

   TEXMGR->cleanupPool();
}

DefineEngineFunction( reloadTextures, void, (),,
   "Reload all the textures from disk.\n"
   "@ingroup GFX\n" )
{
   if ( !GFX || !TEXMGR )
      return;

   TEXMGR->reloadTextures();
}
