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

#ifndef _GENERICCONSTBUFFER_H_
#define _GENERICCONSTBUFFER_H_

#ifndef _TORQUE_STRING_H_
#include "core/util/str.h"
#endif
#ifndef _TDICTIONARY_H_
#include "core/util/tDictionary.h"
#endif
#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif
#ifndef _ALIGNEDARRAY_H_
#include "core/util/tAlignedArray.h"
#endif
#ifndef _COLOR_H_
#include "core/color.h"
#endif
#ifndef _MMATRIX_H_
#include "math/mMatrix.h"
#endif
#ifndef _MPOINT2_H_
#include "math/mPoint2.h"
#endif
#ifndef _GFXENUMS_H_
#include "gfx/gfxEnums.h"
#endif

class Stream;


/// This class defines the memory layout for a GenericConstBuffer.
class GenericConstBufferLayout 
{   
public:
   /// Describes the parameters we contain
   struct ParamDesc
   {
      ParamDesc()
         :  name(),
            offset( 0 ),
            size( 0 ),
            constType( GFXSCT_Float ),
            arraySize( 0 ),
            alignValue( 0 ),
            index( 0 )
      {
      }

      void clear()
      {
         name = String::EmptyString;
         offset = 0;
         size = 0;
         constType = GFXSCT_Float;
         arraySize = 0;
         alignValue = 0;
         index = 0;
      }

      /// Parameter name
      String name;

      /// Offset into the memory block
      U32 offset;

      /// Size of the block
      U32 size;

      /// Type of data
      GFXShaderConstType constType;

      // For arrays, how many elements
      U32 arraySize;

      // Array element alignment value
      U32 alignValue;

      /// 0 based index of this param, in order of addParameter calls.
      U32 index;
   };

   GenericConstBufferLayout();
   virtual ~GenericConstBufferLayout() {}

   /// Add a parameter to the buffer
   virtual void addParameter(const String& name, const GFXShaderConstType constType, const U32 offset, const U32 size, const U32 arraySize, const U32 alignValue);

   /// Get the size of the buffer
   inline U32 getBufferSize() const { return mBufferSize; }

   /// Get the number of parameters
   inline U32 getParameterCount() const { return mParams.size(); }

   /// Returns the ParamDesc of a parameter 
   bool getDesc(const String& name, ParamDesc& param) const;

   /// Returns the ParamDesc of a parameter 
   bool getDesc(const U32 index, ParamDesc& param) const;

   /// Set a parameter, given a base pointer
   virtual bool set(const ParamDesc& pd, const GFXShaderConstType constType, const U32 size, const void* data, U8* basePointer);

   /// Save this layout to a stream
   bool write(Stream* s);

   /// Load this layout from a stream
   bool read(Stream* s);

   /// Restore to initial state.
   void clear();

protected:

   /// Set a matrix, given a base pointer.
   virtual bool setMatrix(const ParamDesc& pd, const GFXShaderConstType constType, const U32 size, const void* data, U8* basePointer);

   /// Vector of parameter descriptions.
   typedef Vector<ParamDesc> Params;
   
   /// Vector of parameter descriptions.
   Params mParams;
   U32 mBufferSize;
   U32 mCurrentIndex;

   // This if for debugging shader reloading and can be removed later.
   U32 mTimesCleared;
};


/// This class manages shader constant data in a system memory buffer.  It is
/// used by device specific classes for batching together many constant changes 
/// which are then copied to the device thru a single API call.
///
/// @see GenericConstBufferLayout
///
class GenericConstBuffer
{
public:
   GenericConstBuffer(GenericConstBufferLayout* layout);
   ~GenericConstBuffer();

   /// @name Set shader constant values
   /// @{
   /// Actually set shader constant values
   /// @param name Name of the constant, this should be a name contained in the array returned in getShaderConstDesc,
   /// if an invalid name is used, its ignored, but it's not an error.
   inline void set(const GenericConstBufferLayout::ParamDesc& pd, const F32 f) { internalSet(pd, GFXSCT_Float, sizeof(F32), &f); }
   inline void set(const GenericConstBufferLayout::ParamDesc& pd, const Point2F& fv) { internalSet(pd, GFXSCT_Float2, sizeof(Point2F), &fv); }
   inline void set(const GenericConstBufferLayout::ParamDesc& pd, const Point3F& fv) { internalSet(pd, GFXSCT_Float3, sizeof(Point3F), &fv); }
   inline void set(const GenericConstBufferLayout::ParamDesc& pd, const Point4F& fv) { internalSet(pd, GFXSCT_Float4, sizeof(Point4F), &fv); }
   inline void set(const GenericConstBufferLayout::ParamDesc& pd, const PlaneF& fv) { internalSet(pd, GFXSCT_Float4, sizeof(PlaneF), &fv); }
   inline void set(const GenericConstBufferLayout::ParamDesc& pd, const ColorF& fv) { internalSet(pd, GFXSCT_Float4, sizeof(Point4F), &fv); }
   inline void set(const GenericConstBufferLayout::ParamDesc& pd, const S32 f) { internalSet(pd, GFXSCT_Int, sizeof(S32), &f); }
   inline void set(const GenericConstBufferLayout::ParamDesc& pd, const Point2I& fv) { internalSet(pd, GFXSCT_Int2, sizeof(Point2I), &fv); }
   inline void set(const GenericConstBufferLayout::ParamDesc& pd, const Point3I& fv) { internalSet(pd, GFXSCT_Int3, sizeof(Point3I), &fv); }
   inline void set(const GenericConstBufferLayout::ParamDesc& pd, const Point4I& fv) { internalSet(pd, GFXSCT_Int4, sizeof(Point4I), &fv); }
   inline void set(const GenericConstBufferLayout::ParamDesc& pd, const AlignedArray<F32>& fv) { internalSet(pd, GFXSCT_Float, fv.getElementSize() * fv.size(), fv.getBuffer()); }
   inline void set(const GenericConstBufferLayout::ParamDesc& pd, const AlignedArray<Point2F>& fv) { internalSet(pd, GFXSCT_Float2, fv.getElementSize() * fv.size(), fv.getBuffer()); }
   inline void set(const GenericConstBufferLayout::ParamDesc& pd, const AlignedArray<Point3F>& fv) { internalSet(pd, GFXSCT_Float3, fv.getElementSize() * fv.size(), fv.getBuffer()); }
   inline void set(const GenericConstBufferLayout::ParamDesc& pd, const AlignedArray<Point4F>& fv) { internalSet(pd, GFXSCT_Float4, fv.getElementSize() * fv.size(), fv.getBuffer()); }
   inline void set(const GenericConstBufferLayout::ParamDesc& pd, const AlignedArray<S32>& fv) { internalSet(pd, GFXSCT_Int, fv.getElementSize() * fv.size(), fv.getBuffer()); }
   inline void set(const GenericConstBufferLayout::ParamDesc& pd, const AlignedArray<Point2I>& fv) { internalSet(pd, GFXSCT_Int2, fv.getElementSize() * fv.size(), fv.getBuffer()); }
   inline void set(const GenericConstBufferLayout::ParamDesc& pd, const AlignedArray<Point3I>& fv) { internalSet(pd, GFXSCT_Int3, fv.getElementSize() * fv.size(), fv.getBuffer()); }
   inline void set(const GenericConstBufferLayout::ParamDesc& pd, const AlignedArray<Point4I>& fv) { internalSet(pd, GFXSCT_Int4, fv.getElementSize() * fv.size(), fv.getBuffer()); }

   inline void set( const GenericConstBufferLayout::ParamDesc& pd, const MatrixF& mat, const GFXShaderConstType matrixType )
   {
      AssertFatal(   matrixType == GFXSCT_Float2x2 || 
                     matrixType == GFXSCT_Float3x3 || 
                     matrixType == GFXSCT_Float4x4, 
         "GenericConstBuffer::set() - Invalid matrix type!" );

      internalSet( pd, matrixType, sizeof(MatrixF), &mat );
   }

   inline void set( const GenericConstBufferLayout::ParamDesc& pd, const MatrixF* mat, const U32 arraySize, const GFXShaderConstType matrixType )
   {
      AssertFatal(   matrixType == GFXSCT_Float2x2 || 
                     matrixType == GFXSCT_Float3x3 || 
                     matrixType == GFXSCT_Float4x4, 
         "GenericConstBuffer::set() - Invalid matrix type!" );

      internalSet( pd, matrixType, sizeof(MatrixF)*arraySize, mat );
   }

   /// Gets the dirty buffer range and clears the dirty
   /// state at the same time.
   inline const U8* getDirtyBuffer( U32 *start, U32 *size );

   /// Gets the entire buffer ignoring dirty range
   inline const U8* getEntireBuffer();

   /// Sets the entire buffer as dirty or clears the dirty state.
   inline void setDirty( bool dirty );

   /// Returns true if the buffer has been modified since the 
   /// last call to getDirtyBuffer or setDirty.  The buffer is
   /// not dirty on initial creation.
   ///
   /// @see getDirtyBuffer
   /// @see setDirty
   inline bool isDirty() const { return mDirtyEnd != 0; }

   /// Returns true if have the same layout and hold the same 
   /// data as the input buffer.
   inline bool isEqual( const GenericConstBuffer *buffer ) const;

   /// Returns our layout object.
   inline GenericConstBufferLayout* getLayout() const { return mLayout; }

   #ifdef TORQUE_DEBUG
   
      /// Helper function used to assert on unset constants.
      void assertUnassignedConstants( const char *shaderName );

   #endif

protected:

   /// Returns a pointer to the raw buffer
   inline const U8* getBuffer() const { return mBuffer; }

   /// Called by the inlined set functions above to do the
   /// real dirty work of copying the data to the right location
   /// within the buffer.
   inline void internalSet(   const GenericConstBufferLayout::ParamDesc &pd, 
                              const GFXShaderConstType constType, 
                              const U32 size, 
                              const void *data );

   /// The buffer layout.
   GenericConstBufferLayout *mLayout;

   /// The pointer to the contant store or
   /// NULL if the layout is empty.
   U8 *mBuffer;

   /// The byte offset to the start of the dirty
   /// range within the buffer or U32_MAX if the
   /// buffer is not dirty.
   U32 mDirtyStart;

   /// The btye offset to the end of the dirty
   /// range within the buffer or 0 if the buffer
   /// is not dirty.
   U32 mDirtyEnd;


   #ifdef TORQUE_DEBUG
   
      /// A vector used to keep track if a constant 
      /// has beed assigned a value or not.
      ///
      /// @see assertUnassignedConstants
      Vector<bool> mWasAssigned;

   #endif
};


// NOTE: These inlines below are here to get the very best possible
// performance when setting the device shader constants and can be
// called 4000-8000 times per frame or more.
//
// You need a very good reason to consider changing them.

inline void GenericConstBuffer::internalSet( const GenericConstBufferLayout::ParamDesc &pd, 
                                             const GFXShaderConstType constType, 
                                             const U32 size, 
                                             const void *data )
{   
   // NOTE: We should have never gotten here if the buffer 
   // was null as no valid shader constant could have been 
   // assigned.
   //
   // If this happens its a bug in another part of the code.
   //
   AssertFatal( mBuffer, "GenericConstBuffer::internalSet - The buffer is NULL!" );

   if ( mLayout->set( pd, constType, size, data, mBuffer ) )
   {
      #ifdef TORQUE_DEBUG
         
         // Update the debug assignment tracking.
         mWasAssigned[ pd.index ] = true;

      #endif

      // Keep track of the dirty range so it can be queried
      // later in GenericConstBuffer::getDirtyBuffer.
      mDirtyStart = getMin( pd.offset, mDirtyStart );
      mDirtyEnd = getMax( pd.offset + pd.size, mDirtyEnd );
   }
}

inline void GenericConstBuffer::setDirty( bool dirty )
{ 
   if ( !mBuffer )
      return;

   if ( dirty )
   {
      mDirtyStart = 0;
      mDirtyEnd = mLayout->getBufferSize();
   }
   else if ( !dirty )
   {
      mDirtyStart = U32_MAX;
      mDirtyEnd = 0;
   }
}

inline const U8* GenericConstBuffer::getDirtyBuffer( U32 *start, U32 *size )
{
   AssertFatal( isDirty(), "GenericConstBuffer::getDirtyBuffer() - Buffer is not dirty!" );
   AssertFatal( mDirtyEnd > mDirtyStart, "GenericConstBuffer::getDirtyBuffer() - Dirty range is invalid!" );
   AssertFatal( mBuffer, "GenericConstBuffer::getDirtyBuffer() - Buffer is empty!" );

   // Use the area we calculated during internalSet.
   *size = mDirtyEnd - mDirtyStart;
   *start = mDirtyStart;
   const U8 *buffer = mBuffer + mDirtyStart;

   // Clear the dirty state while we're here.
   mDirtyStart = U32_MAX;
   mDirtyEnd = 0;

   return buffer;
}

inline const U8* GenericConstBuffer::getEntireBuffer()
{
   AssertFatal(mBuffer, "GenericConstBuffer::getDirtyBuffer() - Buffer is empty!");

   return mBuffer;
}

inline bool GenericConstBuffer::isEqual( const GenericConstBuffer *buffer ) const
{      
   U32 bsize = mLayout->getBufferSize();
   if ( bsize != buffer->mLayout->getBufferSize() )
      return false;

   return dMemcmp( mBuffer, buffer->getBuffer(), bsize ) == 0;
}

#endif // _GENERICCONSTBUFFER_H_
