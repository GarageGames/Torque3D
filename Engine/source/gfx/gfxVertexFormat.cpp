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
#include "gfx/gfxVertexFormat.h"

#include "platform/profiler.h"
#include "core/util/hashFunction.h"
#include "gfx/gfxDevice.h"


namespace GFXSemantic
{
   const String POSITION = String( "POSITION" ).intern();
   const String NORMAL = String( "NORMAL" ).intern();
   const String BINORMAL = String( "BINORMAL" ).intern();
   const String TANGENT = String( "TANGENT" ).intern();
   const String TANGENTW = String( "TANGENTW" ).intern();
   const String COLOR = String( "COLOR" ).intern();
   const String TEXCOORD = String( "TEXCOORD" ).intern();
}


U32 GFXVertexElement::getSizeInBytes() const
{
   switch ( mType )
   {
      case GFXDeclType_Float:
         return 4;

      case GFXDeclType_Float2:
         return 8;

      case GFXDeclType_Float3:
         return 12;

      case GFXDeclType_Float4:
         return 16;

      case GFXDeclType_Color:
         return 4;

      default:
         return 0;
   };
}


GFXVertexFormat::GFXVertexFormat()
   :  mDirty( true ),
      mHasColor( false ),
      mHasNormal( false ),
      mHasTangent( false ),
      mHasInstancing( false ),
      mTexCoordCount( 0 ),
      mSizeInBytes( 0 ),
      mDecl( NULL )
{
   VECTOR_SET_ASSOCIATION( mElements );
}

void GFXVertexFormat::copy( const GFXVertexFormat &format )
{
   mDirty = format.mDirty;
   mHasNormal = format.mHasNormal;
   mHasTangent = format.mHasTangent;
   mHasColor = format.mHasColor;
   mHasInstancing = format.mHasInstancing;
   mTexCoordCount = format.mTexCoordCount;
   mSizeInBytes = format.mSizeInBytes;
   mDescription = format.mDescription;
   mElements = format.mElements;
   mDecl = format.mDecl;
}

void GFXVertexFormat::append( const GFXVertexFormat &format, U32 streamIndex )
{
   for ( U32 i=0; i < format.getElementCount(); i++ )
   {
      mElements.increment();
      mElements.last() = format.getElement( i );
      if ( streamIndex != -1 )
         mElements.last().mStreamIndex = streamIndex;
   }

   mDirty = true;
}

void GFXVertexFormat::clear()
{ 
   mDirty = true;
   mElements.clear(); 
   mDecl = NULL;
}

void GFXVertexFormat::addElement( const String& semantic, GFXDeclType type, U32 index, U32 stream ) 
{ 
   mDirty = true;
   mElements.increment();
   GFXVertexElement& lastElement = mElements.last();
   lastElement.mStreamIndex = stream;
   lastElement.mSemantic = semantic.intern();
   lastElement.mSemanticIndex = index;
   lastElement.mType = type;
}

const String& GFXVertexFormat::getDescription() const
{
   if ( mDirty )
      const_cast<GFXVertexFormat*>(this)->_updateDirty();

   return mDescription;
}

GFXVertexDecl* GFXVertexFormat::getDecl() const
{
   if ( !mDecl || mDirty )
      const_cast<GFXVertexFormat*>(this)->_updateDecl();

   return mDecl;
}

bool GFXVertexFormat::hasNormal() const
{
   if ( mDirty )
      const_cast<GFXVertexFormat*>(this)->_updateDirty();

   return mHasNormal;
}

bool GFXVertexFormat::hasTangent() const
{
   if ( mDirty )
      const_cast<GFXVertexFormat*>(this)->_updateDirty();

   return mHasTangent;
}

bool GFXVertexFormat::hasColor() const
{
   if ( mDirty )
      const_cast<GFXVertexFormat*>(this)->_updateDirty();

   return mHasColor;
}

bool GFXVertexFormat::hasInstancing() const
{
   if (mDirty)
      const_cast<GFXVertexFormat*>(this)->_updateDirty();

   return mHasInstancing;
}

U32 GFXVertexFormat::getTexCoordCount() const
{
   if ( mDirty )
      const_cast<GFXVertexFormat*>(this)->_updateDirty();

   return mTexCoordCount;
}

U32 GFXVertexFormat::getSizeInBytes() const
{
   if ( mDirty )
      const_cast<GFXVertexFormat*>(this)->_updateDirty();

   return mSizeInBytes;
}

void GFXVertexFormat::enableInstancing()
{
   mHasInstancing = true;
}

void GFXVertexFormat::_updateDirty()
{
   PROFILE_SCOPE( GFXVertexFormat_updateDirty );

   mTexCoordCount = 0;

   mHasColor = false;
   mHasNormal = false;
   mHasTangent = false;
   mSizeInBytes = 0;

   String desc;

   for ( U32 i=0; i < mElements.size(); i++ )
   {
      const GFXVertexElement &element = mElements[i];

      desc += String::ToString( "%d,%s,%d,%d\n",   element.mStreamIndex,
                                                   element.mSemantic.c_str(), 
                                                   element.mSemanticIndex, 
                                                   element.mType );

      if ( element.isSemantic( GFXSemantic::NORMAL ) )
         mHasNormal = true;
      else if ( element.isSemantic( GFXSemantic::TANGENT ) )
         mHasTangent = true;
      else if ( element.isSemantic( GFXSemantic::COLOR ) )
         mHasColor = true;
      else if ( element.isSemantic( GFXSemantic::TEXCOORD ) )
         ++mTexCoordCount;

      mSizeInBytes += element.getSizeInBytes();
   }

   // Intern the string for fast compares later.
   mDescription = desc.intern();

   mDirty = false;
}

void GFXVertexFormat::_updateDecl()
{
   PROFILE_SCOPE( GFXVertexFormat_updateDecl );

   if ( mDirty )
      _updateDirty();

   mDecl = GFX->allocVertexDecl( this );
}
