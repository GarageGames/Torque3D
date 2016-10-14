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

#ifndef _GFXVERTEXBUFFER_H_
#define _GFXVERTEXBUFFER_H_

#ifndef _GFXSTRUCTS_H_
#include "gfx/gfxStructs.h"
#endif


//*****************************************************************************
// GFXVertexBuffer - base vertex buffer class
//*****************************************************************************
class GFXVertexBuffer : public StrongRefBase, public GFXResource
{
   friend class GFXVertexBufferHandleBase;
   friend class GFXDevice;

public:

   /// Number of vertices in this buffer.
   U32 mNumVerts;

   /// A copy of the vertex format for this buffer.
   GFXVertexFormat mVertexFormat;

   /// Vertex size in bytes.
   U32 mVertexSize;

   /// GFX buffer type (static, dynamic or volatile).
   GFXBufferType mBufferType;

   /// Device this vertex buffer was allocated on.
   GFXDevice *mDevice;

   bool  isLocked;
   U32   lockedVertexStart;
   U32   lockedVertexEnd;
   void* lockedVertexPtr;
   U32   mVolatileStart;

   GFXVertexBuffer(  GFXDevice *device, 
                     U32 numVerts, 
                     const GFXVertexFormat *vertexFormat, 
                     U32 vertexSize, 
                     GFXBufferType bufferType )
      :  mNumVerts( numVerts ),
         mVertexSize( vertexSize ),
         mBufferType( bufferType ),
         mDevice( device ),
         mVolatileStart( 0 )
   {
      if ( vertexFormat )
      {
         vertexFormat->getDecl();
         mVertexFormat.copy( *vertexFormat );
      }
   }
   
   virtual void lock(U32 vertexStart, U32 vertexEnd, void **vertexPtr) = 0;
   virtual void unlock() = 0;
   virtual void prepare() = 0;

   // GFXResource
   virtual const String describeSelf() const;
};


//*****************************************************************************
// GFXVertexBufferHandleBase
//*****************************************************************************
class GFXVertexBufferHandleBase : public StrongRefPtr<GFXVertexBuffer>
{
   friend class GFXDevice;

protected:

   void set(   GFXDevice *theDevice,
               U32 numVerts, 
               const GFXVertexFormat *vertexFormat, 
               U32 vertexSize,
               GFXBufferType type );

   void* lock(U32 vertexStart, U32 vertexEnd)
   {
      if(vertexEnd == 0)
         vertexEnd = getPointer()->mNumVerts;
      AssertFatal(vertexEnd > vertexStart, "Can't get a lock with the end before the start.");
      AssertFatal(vertexEnd <= getPointer()->mNumVerts || getPointer()->mBufferType == GFXBufferTypeVolatile, "Tried to get vertices beyond the end of the buffer!");
      getPointer()->lock(vertexStart, vertexEnd, &getPointer()->lockedVertexPtr);
      return getPointer()->lockedVertexPtr;
   }

   void unlock() ///< unlocks the vertex data, making changes illegal.
   {
      getPointer()->unlock();
   }
};


/// A handle object for allocating, filling, and reading a vertex buffer.
template<class T> 
class GFXVertexBufferHandle : public GFXVertexBufferHandleBase
{
   typedef GFXVertexBufferHandleBase Parent;

   /// Sets this vertex buffer as the current 
   /// vertex buffer for the device it was allocated on
   void prepare() { getPointer()->prepare(); }

public:

   GFXVertexBufferHandle() {}

   GFXVertexBufferHandle(  GFXDevice *theDevice, 
                           U32 numVerts, 
                           GFXBufferType type = GFXBufferTypeVolatile )
   {
      set( theDevice, numVerts, type );
   }

   ~GFXVertexBufferHandle() {}

   void set(   GFXDevice *theDevice, 
               U32 numVerts,
               GFXBufferType type = GFXBufferTypeVolatile )
   {
      Parent::set( theDevice, numVerts, getGFXVertexFormat<T>(), sizeof(T), type );
   }

   T *lock(U32 vertexStart = 0, U32 vertexEnd = 0) ///< locks the vertex buffer range, and returns a pointer to the beginning of the vertex array
                                                   ///< also allows the array operators to work on this vertex buffer.
   {
      return (T*)Parent::lock(vertexStart, vertexEnd);
   }

   void unlock() { Parent::unlock(); }

   T& operator[](U32 index) ///< Array operator allows indexing into a locked vertex buffer.  The debug version of the code
                            ///< will range check the array access as well as validate the locked vertex buffer pointer.
   {
      return ((T*)getPointer()->lockedVertexPtr)[index];
   }

   const T& operator[](U32 index) const ///< Array operator allows indexing into a locked vertex buffer.  The debug version of the code
                                        ///< will range check the array access as well as validate the locked vertex buffer pointer.
   {
      index += getPointer()->mVolatileStart;
      AssertFatal(getPointer()->lockedVertexPtr != NULL, "Cannot access verts from an unlocked vertex buffer!!!");
      AssertFatal(index >= getPointer()->lockedVertexStart && index < getPointer()->lockedVertexEnd, "Out of range vertex access!");
      index -= getPointer()->mVolatileStart;
      return ((T*)getPointer()->lockedVertexPtr)[index];
   }

   T& operator[](S32 index) ///< Array operator allows indexing into a locked vertex buffer.  The debug version of the code
                            ///< will range check the array access as well as validate the locked vertex buffer pointer.
   {
      index += getPointer()->mVolatileStart;
      AssertFatal(getPointer()->lockedVertexPtr != NULL, "Cannot access verts from an unlocked vertex buffer!!!");
      AssertFatal(index >= getPointer()->lockedVertexStart && index < getPointer()->lockedVertexEnd, "Out of range vertex access!");
      index -= getPointer()->mVolatileStart;
      return ((T*)getPointer()->lockedVertexPtr)[index];
   }

   const T& operator[](S32 index) const ///< Array operator allows indexing into a locked vertex buffer.  The debug version of the code
                                        ///< will range check the array access as well as validate the locked vertex buffer pointer.
   {
      index += getPointer()->mVolatileStart;
      AssertFatal(getPointer()->lockedVertexPtr != NULL, "Cannot access verts from an unlocked vertex buffer!!!");
      AssertFatal(index >= getPointer()->lockedVertexStart && index < getPointer()->lockedVertexEnd, "Out of range vertex access!");
      index -= getPointer()->mVolatileStart;
      return ((T*)getPointer()->lockedVertexPtr)[index];
   }

   GFXVertexBufferHandle<T>& operator=(GFXVertexBuffer *ptr)
   {
      StrongObjectRef::set(ptr);
      return *this;
   }

};


/// This is a non-typed vertex buffer handle which can be
/// used when your vertex type is undefined until runtime.
class GFXVertexBufferDataHandle : public GFXVertexBufferHandleBase         
{
   typedef GFXVertexBufferHandleBase Parent;

protected:

   void prepare() { getPointer()->prepare(); }

public:

   GFXVertexBufferDataHandle()
   {
   }

   void set(   GFXDevice *theDevice, 
               U32 vertSize, 
               const GFXVertexFormat *vertexFormat, 
               U32 numVerts, 
               GFXBufferType type )
   {
      Parent::set( theDevice, numVerts, vertexFormat, vertSize, type );
   }

   U8* lock( U32 vertexStart = 0, U32 vertexEnd = 0 )
   {
      return (U8*)Parent::lock( vertexStart, vertexEnd );
   }

   void unlock() { Parent::unlock(); }

   GFXVertexBufferDataHandle& operator=( GFXVertexBuffer *ptr )
   {
      StrongObjectRef::set(ptr);
      return *this;
   }
};


#endif // _GFXVERTEXBUFFER_H_


