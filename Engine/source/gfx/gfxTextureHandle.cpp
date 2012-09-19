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
#include "gfx/gfxTextureHandle.h"

#include "gfx/gfxDevice.h"
#include "gfx/gfxTextureManager.h"


GFXTexHandle GFXTexHandle::ZERO;
GFXTexHandle GFXTexHandle::ONE;
GFXTexHandle GFXTexHandle::ZUP;


GFXTexHandle::GFXTexHandle( GFXTextureObject *obj )
{
   StrongObjectRef::set( obj );
}

GFXTexHandle::GFXTexHandle( const GFXTexHandle &handle, const String &desc )
{
   StrongObjectRef::set( handle.getPointer() );
   
   #ifdef TORQUE_DEBUG
      if ( getPointer() )
         getPointer()->mDebugDescription = desc;
   #endif   
}

GFXTexHandle::GFXTexHandle( const String &texName, GFXTextureProfile *profile, const String &desc )
{
   set( texName, profile, desc );
}

bool GFXTexHandle::set( const String &texName, GFXTextureProfile *profile, const String &desc )
{
   // Clear the existing texture first, so that
   // its memory is free for the new allocation.
   free();
   
   // Create and set the new texture.
   AssertFatal( texName.isNotEmpty(), "Texture name is empty" );
   StrongObjectRef::set( TEXMGR->createTexture( texName, profile ) );
   
   #ifdef TORQUE_DEBUG
      if ( getPointer() )
         getPointer()->mDebugDescription = desc;
   #endif
   
   return isValid();
}

GFXTexHandle::GFXTexHandle( GBitmap *bmp, GFXTextureProfile *profile, bool deleteBmp, const String &desc )
{
   set( bmp, profile, deleteBmp, desc );
}

bool GFXTexHandle::set( GBitmap *bmp, GFXTextureProfile *profile, bool deleteBmp, const String &desc  )
{
   // Clear the existing texture first, so that
   // its memory is free for the new allocation.
   free();
   
   // Create and set the new texture.
   AssertFatal( bmp, "Bitmap is NULL" );
   StrongObjectRef::set( TEXMGR->createTexture( bmp, String(), profile, deleteBmp ) );

   #ifdef TORQUE_DEBUG
      if ( getPointer() )
         getPointer()->mDebugDescription = desc;
   #endif

   return isValid();
}

GFXTexHandle::GFXTexHandle( DDSFile *dds, GFXTextureProfile *profile, bool deleteDDS, const String &desc )
{
   set( dds, profile, deleteDDS, desc );
}

bool GFXTexHandle::set( DDSFile *dds, GFXTextureProfile *profile, bool deleteDDS, const String &desc )
{
   // Clear the existing texture first, so that
   // its memory is free for the new allocation.
   free();

   // Create and set the new texture.
   AssertFatal( dds, "Bitmap is NULL" );
   StrongObjectRef::set( TEXMGR->createTexture( dds, profile, deleteDDS ) );

   #ifdef TORQUE_DEBUG
      if ( getPointer() )
         getPointer()->mDebugDescription = desc;
   #endif

   return isValid();
}

GFXTexHandle::GFXTexHandle( U32 width, U32 height, GFXFormat format, GFXTextureProfile *profile, const String &desc, U32 numMipLevels, S32 antialiasLevel)
{
   set( width, height, format, profile, desc, numMipLevels, antialiasLevel );
}

bool GFXTexHandle::set( U32 width, U32 height, GFXFormat format, GFXTextureProfile *profile, const String &desc, U32 numMipLevels, S32 antialiasLevel)
{
   // Clear the existing texture first, so that
   // its memory is free for the new allocation.
   free();

   // Create and set the new texture.
   StrongObjectRef::set( TEXMGR->createTexture( width, height, format, profile, numMipLevels, antialiasLevel ) );

   #ifdef TORQUE_DEBUG
      if ( getPointer() )
         getPointer()->mDebugDescription = desc;
   #endif

   return isValid();
}

bool GFXTexHandle::set( U32 width, U32 height, U32 depth, void *pixels, GFXFormat format, GFXTextureProfile *profile, const String &desc, U32 numMipLevels )
{
   // Clear the existing texture first, so that
   // its memory is free for the new allocation.
   free();

   // Create and set the new texture.
   StrongObjectRef::set( TEXMGR->createTexture( width, height, depth, pixels, format, profile ) );

   #ifdef TORQUE_DEBUG
      if ( getPointer() )
         getPointer()->mDebugDescription = desc;
   #endif

   return isValid();
}

void GFXTexHandle::refresh()
{
   TEXMGR->reloadTexture( getPointer() );
}
