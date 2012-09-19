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

#ifndef _ALIGNEDARRAY_H_
#define _ALIGNEDARRAY_H_

/// This is a fixed size class that will align its elements on configurable boundaries.    
template<typename T>
class AlignedArray
{
public:
   AlignedArray();
   /// Create an AlignedArray
   /// @param arraySize How many items
   /// @param elementSize Size of each element (including padding)
   AlignedArray(const U32 arraySize, const U32 elementSize);
   /// Create an AlignedArray
   /// @param arraySize How many items
   /// @param elementSize Size of each element (including padding)
   /// @param buffer Preallocated buffer (with data and aligned on elementSize boundaries)
   /// @param takeOwn If true, this class will take ownership of the buffer and free on destruct
   AlignedArray(const U32 arraySize, const U32 elementSize, U8* buffer, bool takeOwn);
   ~AlignedArray();

   void setCapacity(const U32 arraySize, const U32 elementSize);
   void setCapacity(const U32 arraySize, const U32 elementSize, U8* buffer, bool takeOwn);

   /// Size of the array
   U32 size() const;      

   /// Set a new array size (up to initial size created returned by capacity)
   void setSize(U32 newsize);

   /// Capacity of the array (you can setCapacity the size this high)
   U32 capacity() const;
   
   /// Returns the size of an element (useful for asserting, etc)
   U32 getElementSize() const;

   /// Returns a pointer to the raw buffer data.
   const void* getBuffer() const;
   void* getBuffer();

   // Returns the buffer size in bytes.
   U32 getBufferSize() const { return mElementSize * mElementCount; }

   // Operators
   T& operator[](const U32);
   const T& operator[](const U32) const;
protected:
   // How big an element is, this includes padding need to align
   U32 mElementSize;
   // How many elements do we have
   U32 mElementCount;
   // How many elements can we have
   U32 mCapacity;
   // Storage, we use placement new and reinterpret casts to deal with 
   // alignment
   U8* mBuffer;
   // Do we own this buffer? Or are we just wrapping it?
   bool mOwnBuffer;
};

template<typename T>
inline AlignedArray<T>::AlignedArray()
{
   mElementSize = 0;
   mElementCount = 0;
   mCapacity = 0;
   mBuffer = NULL;
   mOwnBuffer = true;
}

template<typename T>
inline AlignedArray<T>::AlignedArray(const U32 arraySize, const U32 elementSize)
{
   mElementCount = 0; // avoid debug assert
   setCapacity(arraySize, elementSize);
}

template<typename T>
inline AlignedArray<T>::AlignedArray(const U32 arraySize, const U32 elementSize, U8* buffer, bool takeOwn)
{
   mElementCount = 0; // avoid debug assert
   setCapacity(arraySize, elementSize, buffer, takeOwn);
}

template<typename T>
inline AlignedArray<T>::~AlignedArray()
{
   if (mOwnBuffer)
      delete[] mBuffer;
}
template<typename T>
inline void AlignedArray<T>::setCapacity(const U32 arraySize, const U32 elementSize)
{
   AssertFatal(mElementCount == 0, "Unable to set array properties after they are init'ed");
   AssertFatal(elementSize >= sizeof(T), "Element size is too small!");
   AssertFatal(arraySize > 0, "0 length AlignedArrays are not allowed!");

   mElementSize = elementSize;
   mElementCount = arraySize;
   mCapacity = arraySize;
   mBuffer = new U8[mElementSize * mElementCount];
   dMemset(mBuffer, 0xFF, mElementSize * mElementCount);
   U32 bufIndex = 0;
   for (U32 i = 0; i < mElementCount; i++)
   {
      T* item = reinterpret_cast<T*>(&mBuffer[bufIndex]);
      constructInPlace(item);
      bufIndex += mElementSize;
   }
   mOwnBuffer = true;
}

template<typename T>
inline void AlignedArray<T>::setCapacity(const U32 arraySize, const U32 elementSize, U8* buffer, bool takeOwn)
{
   AssertFatal(mElementCount == 0, "Unable to set array properties after they are init'ed");
   AssertFatal(elementSize >= sizeof(T), "Element size is too small!");
   AssertFatal(arraySize > 0, "0 length AlignedArrays are not allowed!");
   AssertFatal(buffer, "NULL buffer!");

   mElementSize = elementSize;
   mElementCount = arraySize;
   mCapacity = arraySize;
   mBuffer = buffer;
   mOwnBuffer = takeOwn;
}

/// Set a new array size (up to initial size created returned by capacity)
template<typename T>
inline void AlignedArray<T>::setSize(U32 newsize)
{
   AssertFatal(newsize <= mCapacity, "Unable to grow this type of array!");
   mElementCount = newsize;
}

template<typename T>
inline U32 AlignedArray<T>::size() const
{
   return mElementCount;
}

template<typename T>
inline U32 AlignedArray<T>::capacity() const
{
   return mCapacity;
}

/// Returns the size of an element (useful for asserting, etc)
template<typename T>
U32 AlignedArray<T>::getElementSize() const
{
   return mElementSize;
}

template<typename T>
inline T& AlignedArray<T>::operator[](const U32 index)
{
   AssertFatal(index < mElementCount, "AlignedArray<T>::operator[] - out of bounds array access!");
   return reinterpret_cast<T&>(mBuffer[index*mElementSize]);
}

template<typename T>
inline const T& AlignedArray<T>::operator[](const U32 index) const
{
   AssertFatal(index < mElementCount, "AlignedArray<T>::operator[] - out of bounds array access!");
   return reinterpret_cast<const T&>(mBuffer[index*mElementSize]);
}

template<typename T>
const void* AlignedArray<T>::getBuffer() const
{
   return reinterpret_cast<const void*>(mBuffer);
}

template<typename T>
void* AlignedArray<T>::getBuffer()
{
   return reinterpret_cast<void*>(mBuffer);
}

#endif // _ALIGNEDARRAY_H_
