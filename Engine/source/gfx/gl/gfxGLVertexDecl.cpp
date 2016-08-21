#include "gfx/gl/gfxGLDevice.h"
#include "gfx/gl/gfxGLStateCache.h"
#include "gfx/gl/gfxGLVertexAttribLocation.h"
#include "gfx/gl/gfxGLVertexDecl.h"

void GFXGLVertexDecl::init(const GFXVertexFormat *format)
{
   AssertFatal(!mFormat, "");
   mFormat = format;
  
   for(int i = 0; i < GFXGL->getNumVertexStreams(); ++i)
      _initVerticesFormat(i);   
}

void GFXGLVertexDecl::prepareVertexFormat() const
{
   AssertFatal(mFormat, "GFXGLVertexDecl - Not inited");
   if( GFXGL->mCapabilities.vertexAttributeBinding )
   {
      for ( U32 i=0; i < glVerticesFormat.size(); i++ )
      {
         const glVertexAttribData &glElement = glVerticesFormat[i];
      
         glVertexAttribFormat( glElement.attrIndex, glElement.elementCount, glElement.type, glElement.normalized, (uintptr_t)glElement.pointerFirst );
         glVertexAttribBinding( glElement.attrIndex, glElement.stream );
      }

      updateActiveVertexAttrib( GFXGL->getOpenglCache()->getCacheVertexAttribActive() );

      return;
   }
}

void GFXGLVertexDecl::prepareBuffer_old(U32 stream, GLint mBuffer, GLint mDivisor) const
{
   PROFILE_SCOPE(GFXGLVertexDecl_prepare);
   AssertFatal(mFormat, "GFXGLVertexDecl - Not inited");

   if( GFXGL->mCapabilities.vertexAttributeBinding )
      return;   

	// Bind the buffer...
   glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
   GFXGL->getOpenglCache()->setCacheBinded(GL_ARRAY_BUFFER, mBuffer);

   // Loop thru the vertex format elements adding the array state...   
   for ( U32 i=0; i < glVerticesFormat.size(); i++ )
   {
      // glEnableVertexAttribArray are called and cache in GFXGLDevice::preDrawPrimitive

      const glVertexAttribData &e = glVerticesFormat[i];
      if(e.stream != stream)
         continue;
      
      glVertexAttribPointer(
         e.attrIndex,      // attribute
         e.elementCount,   // number of elements per vertex, here (r,g,b)
         e.type,           // the type of each element
         e.normalized,     // take our values as-is
         e.stride,         // stride between each position
         e.pointerFirst    // offset of first element
      );
      glVertexAttribDivisor( e.attrIndex, mDivisor );
   }
}

void GFXGLVertexDecl::updateActiveVertexAttrib(U32 lastActiveMask) const
{
   AssertFatal(mVertexAttribActiveMask, "GFXGLVertexDecl::updateActiveVertexAttrib - No vertex attribute are active");

   U32 lastActiveVerxtexAttrib = GFXGL->getOpenglCache()->getCacheVertexAttribActive();
   if(mVertexAttribActiveMask == lastActiveVerxtexAttrib)
      return;

   U32 forActiveMask = mVertexAttribActiveMask & ~lastActiveVerxtexAttrib;
   U32 forDeactiveMask = ~mVertexAttribActiveMask & lastActiveVerxtexAttrib;
   for(int i = 0; i < Torque::GL_VertexAttrib_COUNT; ++i)
   {         
      if( BIT(i) & forActiveMask ) //if is active but not in last mask
         glEnableVertexAttribArray(i);
      else if( BIT(i) & forDeactiveMask ) // if not active but in last mask
         glDisableVertexAttribArray(i);
   }

   GFXGL->getOpenglCache()->setCacheVertexAttribActive(mVertexAttribActiveMask);
}

void GFXGLVertexDecl::_initVerticesFormat2()
{
   for( U32 i=0; i < GFXGL->getNumVertexStreams(); ++i )
   {
      _initVerticesFormat(i);
   }
}

void GFXGLVertexDecl::_initVerticesFormat(U32 stream)
{   
   U32 buffer = 0;
   U32 vertexSize = 0;

   for ( U32 i=0; i < mFormat->getElementCount(); i++ )
   {
      const GFXVertexElement &element = mFormat->getElement( i );

      if(element.getStreamIndex() != stream)
         continue;

      AssertFatal(!mFormat->hasBlendIndices() || !element.isSemantic(GFXSemantic::TEXCOORD) || (mFormat->hasBlendIndices() && element.isSemantic(GFXSemantic::TEXCOORD) && element.getSemanticIndex() < 2), "skinning with more than 2 used texcoords!");

      vertexSize += element.getSizeInBytes();
   }

   // Loop thru the vertex format elements adding the array state...
   U32 texCoordIndex = 0;
   for ( U32 i=0; i < mFormat->getElementCount(); i++ )
   {
      const GFXVertexElement &element = mFormat->getElement( i );

      if(element.getStreamIndex() != stream)
         continue;

      glVerticesFormat.increment();
      glVertexAttribData &glElement = glVerticesFormat.last();
      glElement.stream = element.getStreamIndex();

      if ( element.isSemantic( GFXSemantic::POSITION ) )
      {           
         glElement.attrIndex = Torque::GL_VertexAttrib_Position;
         glElement.elementCount = element.getSizeInBytes() / 4;
         glElement.normalized = false;
         glElement.type = GL_FLOAT;
         glElement.stride = vertexSize;
         glElement.pointerFirst = (void*)buffer;

         buffer += element.getSizeInBytes();
      }
      else if ( element.isSemantic( GFXSemantic::NORMAL ) )
      {
         glElement.attrIndex = Torque::GL_VertexAttrib_Normal;
         glElement.elementCount = 3;
         glElement.normalized = false;
         glElement.type = GL_FLOAT;
         glElement.stride = vertexSize;
         glElement.pointerFirst = (void*)buffer;

         buffer += element.getSizeInBytes();
      }
      else if ( element.isSemantic( GFXSemantic::TANGENT ) )
      {
         glElement.attrIndex = Torque::GL_VertexAttrib_Tangent;
         glElement.elementCount = 3;
         glElement.normalized = false;
         glElement.type = GL_FLOAT;
         glElement.stride = vertexSize;
         glElement.pointerFirst = (void*)buffer;

         buffer += element.getSizeInBytes();
      }
      else if ( element.isSemantic( GFXSemantic::TANGENTW ) )
      {
         glElement.attrIndex = Torque::GL_VertexAttrib_TangentW;
         glElement.elementCount = element.getSizeInBytes()/4;
         glElement.normalized = false;
         glElement.type = GL_FLOAT;
         glElement.stride = vertexSize;
         glElement.pointerFirst = (void*)buffer;

         buffer += element.getSizeInBytes();
      }
      else if ( element.isSemantic( GFXSemantic::BINORMAL ) )
      {
         glElement.attrIndex = Torque::GL_VertexAttrib_Binormal;
         glElement.elementCount = 3;
         glElement.normalized = false;
         glElement.type = GL_FLOAT;
         glElement.stride = vertexSize;
         glElement.pointerFirst = (void*)buffer;

         buffer += element.getSizeInBytes();
      }
      else if ( element.isSemantic( GFXSemantic::COLOR ) )
      {
         glElement.attrIndex = Torque::GL_VertexAttrib_Color;
         glElement.elementCount = element.getSizeInBytes();
         glElement.normalized = true;
         glElement.type = GL_UNSIGNED_BYTE;
         glElement.stride = vertexSize;
         glElement.pointerFirst = (void*)buffer;

         buffer += element.getSizeInBytes();
      }
      else if ( element.isSemantic( GFXSemantic::BLENDWEIGHT ) )
      {
         glElement.attrIndex = Torque::GL_VertexAttrib_BlendWeight0 + element.getSemanticIndex();
         glElement.elementCount = 4;
         glElement.normalized = false;
         glElement.type = GL_FLOAT;
         glElement.stride = vertexSize;
         glElement.pointerFirst = (void*)buffer;

         buffer += element.getSizeInBytes();
      }
      else if ( element.isSemantic( GFXSemantic::BLENDINDICES ) )
      {
         glElement.attrIndex = Torque::GL_VertexAttrib_BlendIndex0 + element.getSemanticIndex();
         glElement.elementCount = 4;
         glElement.normalized = false;
         glElement.type = GL_UNSIGNED_BYTE;
         glElement.stride = vertexSize;
         glElement.pointerFirst = (void*)buffer;

         buffer += element.getSizeInBytes();
      }
      else // Everything else is a texture coordinate.
      {
         String name = element.getSemantic();
         glElement.elementCount = element.getSizeInBytes() / 4;
         texCoordIndex = getMax(texCoordIndex, element.getSemanticIndex());
         glElement.attrIndex = Torque::GL_VertexAttrib_TexCoord0 + texCoordIndex;
            
         glElement.normalized = false;
         glElement.type = GL_FLOAT;
         glElement.stride = vertexSize;
         glElement.pointerFirst = (void*)buffer;

         buffer += element.getSizeInBytes();
         ++texCoordIndex;
      }

      AssertFatal(!( mVertexAttribActiveMask & BIT(glElement.attrIndex) ), "GFXGLVertexBuffer::_initVerticesFormat - Duplicate vertex attrib index");
      mVertexAttribActiveMask |= BIT(glElement.attrIndex);
   }

   mVertexSize[stream] = vertexSize;
   AssertFatal(vertexSize == buffer, "");
}