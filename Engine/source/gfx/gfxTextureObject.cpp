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
#include "gfx/gfxTextureObject.h"

#include "gfx/gfxDevice.h"
#include "gfx/gfxTextureManager.h"
#include "core/util/safeDelete.h"
#include "core/strings/stringFunctions.h"
#include "core/stream/fileStream.h"
#include "console/console.h"
#include "console/engineAPI.h"


// TODO: Change this to be in non-shipping builds maybe?
#ifdef TORQUE_DEBUG

GFXTextureObject *GFXTextureObject::smHead = NULL;
U32 GFXTextureObject::smActiveTOCount = 0;

U32 GFXTextureObject::dumpActiveTOs()
{
   if(!smActiveTOCount)
   {
      Con::printf( "GFXTextureObject::dumpActiveTOs - no active TOs to dump." );
      return 0;
   }

   Con::printf("GFXTextureObject Usage Report - %d active TOs", smActiveTOCount);
   Con::printf("---------------------------------------------------------------");
   Con::printf(" Addr   Dim. GFXTextureProfile  ProfilerPath DebugDescription");

   for(GFXTextureObject *walk = smHead; walk; walk=walk->mDebugNext)
      Con::printf(" %x  (%4d, %4d)  %s    %s    %s", walk, walk->getWidth(), 
      walk->getHeight(), walk->mProfile->getName().c_str(), walk->mDebugCreationPath.c_str(), walk->mDebugDescription.c_str());

   Con::printf("----- dump complete -------------------------------------------");
   return smActiveTOCount;
}

DefineEngineFunction( dumpTextureObjects, void, (),,
   "Dumps a list of all active texture objects to the console.\n"
   "@note This function is only available in debug builds.\n"
   "@ingroup GFX\n" )
{
   GFXTextureObject::dumpActiveTOs();
}

#endif // TORQUE_DEBUG

//-----------------------------------------------------------------------------
// GFXTextureObject
//-----------------------------------------------------------------------------
GFXTextureObject::GFXTextureObject(GFXDevice *aDevice, GFXTextureProfile *aProfile) 
{
   mHashNext = mNext = mPrev = NULL;

   mDevice = aDevice;
   mProfile = aProfile;

   mBitmap         = NULL;
   mMipLevels      = 1;
   mAntialiasLevel = 0;

   mTextureSize.set( 0, 0, 0 );

   mDead = false;

   mDeleteTime = 0;

   mBitmap = NULL;
   mDDS    = NULL;
   
   mFormat = GFXFormatR8G8B8;

   mHasTransparency = false;

#if defined(TORQUE_DEBUG)
   // Active object tracking.
   smActiveTOCount++;
   mDebugDescription = "Anonymous Texture Object";
#if defined(TORQUE_ENABLE_PROFILE_PATH)   
   mDebugCreationPath = gProfiler->getProfilePath();
#endif
   mDebugNext = smHead;
   mDebugPrev = NULL;

   if(smHead)
   {
      AssertFatal(smHead->mDebugPrev == NULL, "GFXTextureObject::GFXTextureObject - found unexpected previous in current head!");
      smHead->mDebugPrev = this;
   }

   smHead = this;
#endif
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
GFXTextureObject::~GFXTextureObject() 
{
   kill();

#ifdef TORQUE_DEBUG
   if(smHead == this)
      smHead = this->mDebugNext;

   if(mDebugNext)
      mDebugNext->mDebugPrev = mDebugPrev;

   if(mDebugPrev)
      mDebugPrev->mDebugNext = mDebugNext;

   mDebugPrev = mDebugNext = NULL;

   smActiveTOCount--;
#endif
}

void GFXTextureObject::destroySelf()
{
   mDevice->mTextureManager->requestDeleteTexture(this);
}

//-----------------------------------------------------------------------------
// kill - this function clears out the data in texture object.  It's done like
// this because the texture object needs to release its pointers to textures
// before the GFXDevice is shut down.  The texture objects themselves get
// deleted by the refcount structure - which may be after the GFXDevice has
// been destroyed.
//-----------------------------------------------------------------------------
void GFXTextureObject::kill()
{
   if( mDead )
      return;

#ifdef TORQUE_DEBUG
   // This makes sure that nobody is forgetting to call kill from the derived
   // destructor.  If they are, then we should get a pure virtual function
   // call here.
   pureVirtualCrash();
#endif

   // If we're a dummy, don't do anything...
   if( !mDevice || !mDevice->mTextureManager ) 
   {
      mDead = true;
      return;
   }

   // Remove ourselves from the texture list and hash
   mDevice->mTextureManager->deleteTexture(this);

   // Delete the stored bitmap.
   SAFE_DELETE(mBitmap)
   SAFE_DELETE(mDDS);

   // Clean up linked list
   if(mNext)
      mNext->mPrev = mPrev;
   if(mPrev)
      mPrev->mNext = mNext;

   mDead = true;
}

const String GFXTextureObject::describeSelf() const
{
   return String::ToString(" (width: %4d, height: %4d)  profile: %s   creation path: %s", getWidth(), 
#if defined(TORQUE_DEBUG) && defined(TORQUE_ENABLE_PROFILER)  
      getHeight(), mProfile->getName().c_str(), mDebugCreationPath.c_str());
#else                  
      getHeight(), mProfile->getName().c_str(), "");
#endif
}

F32 GFXTextureObject::getMaxUCoord() const
{
   return 1.0f;
}

F32 GFXTextureObject::getMaxVCoord() const
{
   return 1.0f;
}

U32 GFXTextureObject::getEstimatedSizeInBytes() const
{
   // We have to deal with DDS specially.
   if ( mFormat >= GFXFormatDXT1 )
      return DDSFile::getSizeInBytes( mFormat, mTextureSize.x, mTextureSize.y, mMipLevels );

   // Else we need to calculate the size ourselves.
   S32 texSizeX = mTextureSize.x;
   S32 texSizeY = mTextureSize.y;
   S32 volDepth = getMax( 1, mTextureSize.z );
   U32 byteSize = GFXFormat_getByteSize( mFormat );
   U32 totalBytes = texSizeX * texSizeY * volDepth * byteSize;

   // Without mips we're done.
   if ( mProfile->noMip() )
      return totalBytes;

   // NOTE: While we have mMipLevels, at the time of this
   // comment it only stores the accessable mip levels and
   // not the count of the autogen mips.
   //
   // So we figure out the mip count ourselves assuming its
   // a complete mip chain.
   while ( texSizeX > 1 || texSizeY > 1 )
   {
      texSizeX = getMax( texSizeX >> 1, 1 );
      texSizeY = getMax( texSizeY >> 1, 1 );
      volDepth = getMax( volDepth >> 1, 1 );

      totalBytes += texSizeX * texSizeY * volDepth * byteSize;
   }

   return totalBytes;
}

bool GFXTextureObject::dumpToDisk( const String &bmType, const String &path )
{   
   FileStream stream;
   if ( !stream.open( path, Torque::FS::File::Write ) )
      return false;

   if ( mBitmap )
      return mBitmap->writeBitmap( bmType, stream );

   GBitmap bitmap( getWidth(), getHeight(), false, getFormat() );
   copyToBmp( &bitmap );
   return bitmap.writeBitmap( bmType, stream );
}
