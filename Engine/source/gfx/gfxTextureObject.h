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

#ifndef _GFXTEXTUREOBJECT_H_
#define _GFXTEXTUREOBJECT_H_

#ifndef _REFBASE_H_
#include "core/util/refBase.h"
#endif
#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif
#ifndef _GFXENUMS_H_
#include "gfx/gfxEnums.h"
#endif
#ifndef _GFXTEXTUREPROFILE_H_
#include "gfx/gfxTextureProfile.h"
#endif
#ifndef _GFXRESOURCE_H_
#include "gfx/gfxResource.h"
#endif

class Point2I;
class GFXDevice;
class GFXTextureProfile;
class GBitmap;
struct DDSFile;
class RectI;

/// Contains information on a locked region of a texture.
///
/// In general, to access a given pixel in a locked rectangle, use this
/// equation:
///
/// @code
///     U8 *pixelAtXY = bits + x * pitch + y * pixelSizeInBytes;
/// @endcode
///
/// @note D3DLOCKED_RECT and this structure match up. If you change this
///       assumption, be sure to revisit the D3D GFX implementation.
///
/// @see GFXTextureObject::lock() 
struct GFXLockedRect
{
   /// Pitch of the lock. This is the spacing in bytes of the start
   /// of each row of the locked region.
   int pitch;

   /// Pointer to the start of locked rectangle.
   U8* bits;
};


class GFXTextureObject : public StrongRefBase, public GFXResource
{
public:

   #ifdef TORQUE_DEBUG
      // In debug builds we provide a TOC leak tracking system.
      static U32 smActiveTOCount;
      static GFXTextureObject *smHead;
      static U32 dumpActiveTOs();

      String            mDebugCreationPath;
      String            mDebugDescription;   
      GFXTextureObject *mDebugNext;
      GFXTextureObject *mDebugPrev;
   #endif

   /// The path to the texture file if the
   /// content was loaded from a resource.
   String mPath;

   bool mDead;

   /// The device this texture belongs to.
   GFXDevice *mDevice;   

   /// The next texture in the linked list.
   /// @see GFXTextureManager::mListHead
   GFXTextureObject *mNext;

   /// The previous texture in the linked list.
   /// @see GFXTextureManager::mListHead
   GFXTextureObject *mPrev;

   /// The siblings in the cache hash table.
   /// @see GFXTextureManager::mHashTable
   GFXTextureObject *mHashNext;

   /// This is the file name or other unique string used 
   /// to hash this texture object.
   String mTextureLookupName;

   /// The time at which all references to this
   /// texture were removed.
   U32 mDeleteTime;

   Point3I  mBitmapSize;
   Point3I  mTextureSize;
   U32      mMipLevels;

   // TODO: This looks unused in the engine... not even sure
   // what it means.  We should investigate and remove it.
   S32      mAntialiasLevel;

   bool     mHasTransparency;

   // These two should be removed, and replaced by a reference to a resource
   // object, or data buffer. Something more generic. -patw
   GBitmap           *mBitmap;   ///< GBitmap we are backed by.
   DDSFile           *mDDS;      ///< DDSFile we're backed by.
      
   U32 getFormatByteSize() const { return GFXFormat_getByteSize( mFormat ); }

   GFXTextureProfile *mProfile;
   GFXFormat          mFormat;


   GFXTextureObject(GFXDevice * aDevice, GFXTextureProfile *profile);
   virtual ~GFXTextureObject();

   GBitmap *getBitmap();
   DDSFile *getDDS();
   U32 getWidth() const { return mTextureSize.x; }
   U32 getHeight() const { return mTextureSize.y; }
   const Point3I& getSize() const { return mTextureSize; }
   U32 getDepth() const { return mTextureSize.z; }
   U32 getMipLevels() const { return mMipLevels; }
   U32 getBitmapWidth() const { return mBitmapSize.x; }
   U32 getBitmapHeight() const { return mBitmapSize.y; }
   U32 getBitmapDepth() const { return mBitmapSize.z; }
   GFXFormat getFormat() const { return mFormat; }

   /// Returns true if this texture is a render target.
   bool isRenderTarget() const { return mProfile->isRenderTarget(); }

   /// Returns the file path to the texture if
   /// it was loaded from disk.
   const String& getPath() const { return mPath; }

   virtual F32 getMaxUCoord() const;
   virtual F32 getMaxVCoord() const;

   /// Returns the estimated video memory usage 
   /// in bytes including mipmaps.
   U32 getEstimatedSizeInBytes() const;

   /// Acquire a lock on part of the texture. The GFXLockedRect returned
   /// is managed by the GFXTextureObject and does not need to be freed.
   virtual GFXLockedRect * lock( U32 mipLevel = 0, RectI *inRect = NULL ) = 0;

   /// Releases a lock previously acquired. Note that the mipLevel parameter
   /// must match the corresponding lock!
   virtual void unlock( U32 mipLevel = 0) = 0;

   // copy the texture data into the specified bitmap.  
   //   - this texture object must be a render target.  the function will assert if this is not the case.
   //   - you must have called allocateBitmap() on the input bitmap first.  the bitmap should have the 
   //   same dimensions as this texture.  the bitmap format can be RGB or RGBA (in the latter case
   //   the alpha values from the texture are copied too)
   //   - returns true if successful, false otherwise
   //   - this process is not fast.
   virtual bool copyToBmp(GBitmap* bmp) = 0;

   #ifdef TORQUE_DEBUG

   // It is important for any derived objects to define this method
   // and also call 'kill' from their destructors.  If you fail to
   // do either, you will get a pure virtual function call crash
   // in debug mode.  This is a precaution to make sure you don't
   // forget to add 'kill' to your destructor.
   virtual void pureVirtualCrash() = 0;

   #endif

   virtual void kill();

   /// Debug helper function for writing the texture to disk.
   bool dumpToDisk( const String &bmType, const String &path );

   // GFXResource interface
   /// The resource should put a description of itself (number of vertices, size/width of texture, etc.) in buffer
   virtual const String describeSelf() const;

   // StrongRefBase
   virtual void destroySelf();
};

//-----------------------------------------------------------------------------

inline GBitmap *GFXTextureObject::getBitmap()
{
   AssertFatal( mProfile->doStoreBitmap(), avar("GFXTextureObject::getBitmap - Cannot access bitmap for a '%s' texture.", mProfile->getName().c_str()) );

   return mBitmap;
}

inline DDSFile *GFXTextureObject::getDDS()
{
   AssertFatal( mProfile->doStoreBitmap(), avar("GFXTextureObject::getDDS - Cannot access bitmap for a '%s' texture.", mProfile->getName().c_str()) );

   return mDDS;
}

#endif // _GFXTEXTUREOBJECT_H_
