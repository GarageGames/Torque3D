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

#ifndef _GFXTEXTUREPROFILE_H_
#define _GFXTEXTUREPROFILE_H_

#ifndef _TORQUE_STRING_H_
#include "core/util/str.h"
#endif

class GFXTextureObject;

/// Helper struct for gathering profile stats.
class GFXTextureProfileStats
{
public:

   /// Constructs and clears the stats.
   GFXTextureProfileStats() { clear(); }

   /// Zeros all the stats.
   void clear()
   {
      dMemset( this, 0, sizeof( GFXTextureProfileStats ) );
   }

   /// Adds stats together.
   GFXTextureProfileStats& operator += ( const GFXTextureProfileStats &stats )
   {
      activeCount += stats.activeCount;
      activeTexels += stats.activeTexels;
      activeBytes += stats.activeBytes;
      allocatedTextures += stats.allocatedTextures;
      allocatedTexels += stats.allocatedTexels;
      allocatedBytes += stats.allocatedBytes;
      return *this;
   }

   U32 activeCount;       ///< Count of textures of this profile type allocated.
   U32 activeTexels;      ///< Amount of texelspace currently allocated under this profile.
   U32 activeBytes;       ///< Amount of storage currently allocated under this profile.
   U32 allocatedTextures; ///< Total number of textures allocated under this profile.
   U32 allocatedTexels;   ///< Total number of texels allocated under this profile.
   U32 allocatedBytes;    ///< Total number of bytes allocated under this profile.
};


class GFXTextureProfile
{
public:
   enum Types
   {
      DiffuseMap,
      NormalMap,
      AlphaMap,
      LuminanceMap
   };

   enum Flags
   {
      PreserveSize   = BIT(0),  ///< Never shrink this bitmap in low VRAM situations.
      NoMipmap       = BIT(1),  ///< Do not generate mipmap chain for this texture.
      SystemMemory   = BIT(2),  ///< System memory texture - isn't uploaded to card - useful as target for copying surface data out of video ram
      RenderTarget   = BIT(3),  ///< This texture will be used as a render target.
      Dynamic        = BIT(4),  ///< This texture may be refreshed. (Precludes Static)
      Static         = BIT(5),  ///< This texture will never be modified once loaded. (Precludes Dynamic)
      NoPadding      = BIT(6),  ///< Do not pad this texture if it's non pow2.
      KeepBitmap     = BIT(7),  ///< Always keep a copy of this texture's bitmap. (Potentially in addition to the API managed copy?)
      ZTarget        = BIT(8),  ///< This texture will be used as a Z target.

      /// Track and pool textures of this type for reuse.
      ///
      /// You should use this profile flag sparingly.  Odd
      /// sized textures and spikes in allocation can cause
      /// the pool to contain unused textures which will remain
      /// in memory until a flush occurs.
      ///
      Pooled = BIT(9), 

      /// A hint that the device is not allowed to discard the content
      /// of a target texture after presentation or deactivated.
      ///
      /// This is mainly a depth buffer optimization.
      NoDiscard = BIT(10)

   };

   enum Compression
   {
      None,
      DXT1,
      DXT2,
      DXT3,
      DXT4,
      DXT5,
   };

   GFXTextureProfile(const String &name, Types type, U32 flags, Compression compression = None);

   // Accessors
   String getName() const { return mName; };
   Types getType() const { return (Types)(mProfile & (BIT(TypeBits) - 1)); }
   const Compression getCompression() const { return (Compression)((mProfile >> (FlagBits + TypeBits)) & (BIT(CompressionBits + 1) - 1)); };

   bool testFlag(Flags flag)  const
   {
      return (mProfile & (flag << TypeBits)) != 0;
   }

   // Mutators
   const U32 getDownscale() const { return mDownscale; }
   void setDownscale(const U32 shift) { mDownscale = shift; }
   void incActiveCopies() { mStats.activeCount++; }
   void decActiveCopies() { AssertFatal( mStats.activeCount != 0, "Ran out of extant copies!"); mStats.activeCount--; }   

   // And static interface...
   static void init();
   static GFXTextureProfile *find(const String &name);
   static void updateStatsForCreation(GFXTextureObject *t);
   static void updateStatsForDeletion(GFXTextureObject *t);

   /// Collects the total stats for all the profiles which
   /// include any of the flag bits.
   static void collectStats( Flags flags, GFXTextureProfileStats *stats );

   /// Returns the total profile count in the list.
   static U32 getProfileCount() { return smProfileCount; }

   /// Returns the head of the profile list.
   static GFXTextureProfile* getHead() { return smHead; }

   /// Returns the next profile in the list.
   GFXTextureProfile* getNext() const { return mNext; }

   /// Returns the allocation stats for this texture profile.
   inline const GFXTextureProfileStats& getStats() const { return mStats; }

   // Helper functions...
   inline bool doStoreBitmap() const { return testFlag(KeepBitmap); }
   inline bool canDownscale() const { return !testFlag(PreserveSize); }
   inline bool isDynamic() const { return testFlag(Dynamic); }
   inline bool isRenderTarget() const { return testFlag(RenderTarget); }
   inline bool isZTarget() const { return testFlag(ZTarget); }
   inline bool isSystemMemory() const { return testFlag(SystemMemory); }
   inline bool noMip() const { return testFlag(NoMipmap); }
   inline bool isPooled() const { return testFlag(Pooled); }
   inline bool canDiscard() const { return !testFlag(NoDiscard); }

private:
   /// These constants control the packing for the profile; if you add flags, types, or
   /// compression info then make sure these are giving enough bits!
   enum Constants
   {
      TypeBits = 2,
      FlagBits = 11,
      CompressionBits = 3,
   };

   String    mName;        ///< Name of this profile...
   U32 mDownscale;         ///< Amount to shift textures of this type down, if any.
   U32 mProfile;           ///< Stores a munged version of the profile data.
   U32 mActiveCount;       ///< Count of textures of this profile type allocated.
   U32 mActiveTexels;      ///< Amount of texelspace currently allocated under this profile.
   U32 mActiveBytes;       ///< Amount of storage currently allocated under this profile.
   U32 mAllocatedTextures; ///< Total number of textures allocated under this profile.
   U32 mAllocatedTexels;   ///< Total number of texels allocated under this profile.
   U32 mAllocatedBytes;    ///< Total number of bytes allocated under this profile.

   /// The texture profile stats.
   GFXTextureProfileStats mStats;
   
   /// The number of profiles in the system.
   static U32 smProfileCount;

   /// Keep a list of all the profiles.
   GFXTextureProfile *mNext;
   static GFXTextureProfile *smHead;
};

#define GFX_DeclareTextureProfile(name)  extern GFXTextureProfile name
#define GFX_ImplementTextureProfile(name, type,  flags, compression) GFXTextureProfile name(#name, type, flags, compression)

// Set up some defaults..

// Texture we can render to.
GFX_DeclareTextureProfile(GFXDefaultRenderTargetProfile);
// Standard diffuse texture that stays in system memory.
GFX_DeclareTextureProfile(GFXDefaultPersistentProfile);
// Generic diffusemap. This works in most cases.
GFX_DeclareTextureProfile(GFXDefaultStaticDiffuseProfile);
// Generic normal map.
GFX_DeclareTextureProfile(GFXDefaultStaticNormalMapProfile);
// DXT5 swizzled normal map
GFX_DeclareTextureProfile(GFXDefaultStaticDXT5nmProfile);
// Texture that resides in system memory - used to copy data to
GFX_DeclareTextureProfile(GFXSystemMemProfile);
// Depth buffer texture
GFX_DeclareTextureProfile(GFXDefaultZTargetProfile);

#endif
