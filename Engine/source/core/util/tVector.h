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

#ifndef _TVECTOR_H_
#define _TVECTOR_H_

// TODO: This shouldn't be included in headers... it should
// be included by the source file before all other includes.
#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif
#include <algorithm>

//-----------------------------------------------------------------------------
// Helper definitions for the vector class.

/// Size of memory blocks to allocate at a time for vectors.
const static S32 VectorBlockSize = 16;
#ifdef TORQUE_DEBUG_GUARD
extern bool VectorResize(U32 *aSize, U32 *aCount, void **arrayPtr, U32 newCount, U32 elemSize,
                         const char* fileName,
                         const U32   lineNum);
#else
extern bool VectorResize(U32 *aSize, U32 *aCount, void **arrayPtr, U32 newCount, U32 elemSize);
#endif

/// Use the following macro to bind a vector to a particular line
///  of the owning class for memory tracking purposes
#ifdef TORQUE_DEBUG_GUARD
#define VECTOR_SET_ASSOCIATION(x) x.setFileAssociation(__FILE__, __LINE__)
#else
#define VECTOR_SET_ASSOCIATION(x)
#endif

// =============================================================================
/// A dynamic array template class.
///
/// The vector grows as you insert or append
/// elements.  Insertion is fastest at the end of the array.  Resizing
/// of the array can be avoided by pre-allocating space using the
/// reserve() method.
///
/// @nosubgrouping
template<class T>
class Vector
{
  protected:
   U32 mElementCount; ///< Number of elements currently in the Vector.
   U32 mArraySize;    ///< Number of elements allocated for the Vector.
   T*  mArray;        ///< Pointer to the Vector elements.

#ifdef TORQUE_DEBUG_GUARD
   const char* mFileAssociation;
   U32         mLineAssociation;
#endif

   bool  resize(U32); // resizes, but does no construction/destruction
   void  destroy(U32 start, U32 end);   ///< Destructs elements from <i>start</i> to <i>end-1</i>
   void  construct(U32 start, U32 end); ///< Constructs elements from <i>start</i> to <i>end-1</i>
   void  construct(U32 start, U32 end, const T* array);
  public:
   Vector(const U32 initialSize = 0);
   Vector(const U32 initialSize, const char* fileName, const U32 lineNum);
   Vector(const char* fileName, const U32 lineNum);
   Vector(const Vector&);
   ~Vector();

#ifdef TORQUE_DEBUG_GUARD
   void setFileAssociation(const char* file, const U32 line);
#endif

   /// @name STL interface
   /// @{

   typedef T        value_type;
   typedef T&       reference;
   typedef const T& const_reference;

   typedef T*       iterator;
   typedef const T* const_iterator;
   typedef S32    difference_type;
   typedef U32    size_type;

   typedef difference_type (QSORT_CALLBACK *compare_func)(const T *a, const T *b);

   Vector<T>& operator=(const Vector<T>& p);

   iterator       begin();
   const_iterator begin() const;
   iterator       end();
   const_iterator end() const;

   S32 size() const;
   bool empty() const;
   bool contains(const T&) const;

   void insert(iterator, const T&);
   void erase(iterator);

   T&       front();
   const T& front() const;
   T&       back();
   const T& back() const;

   void push_front(const T&);
   void push_back(const T&);
   U32 push_front_unique(const T&);
   U32 push_back_unique(const T&);
   S32 find_next( const T&, U32 start = 0 ) const;
   void pop_front();
   void pop_back();

   T& operator[](U32);
   const T& operator[](U32) const;

   T& operator[](S32 i)              { return operator[](U32(i)); }
   const T& operator[](S32 i ) const { return operator[](U32(i)); }

   void reserve(U32);
   U32 capacity() const;

   /// @}

   /// @name Extended interface
   /// @{

   U32  memSize() const;
   T*   address() const;
   U32  setSize(U32);
   void increment();
   void decrement();
   void increment(U32);
   void decrement(U32);
   void insert(U32);
   void insert(U32, const T&);
   void erase(U32);
   void erase_fast(U32);
   void erase(U32 index, U32 count);
   void erase_fast(iterator);
   void clear();
   void compact();
   void sort(compare_func f);
   void fill( const T& value );

   /// Finds the first matching element and erases it.   
   /// @return Returns true if a match is found.
   bool remove( const T& );

   T& first();
   T& last();
   const T& first() const;
   const T& last() const;

   void set(void * addr, U32 sz);

   /// Appends the content of the vector to this one.
   void merge( const Vector &p );

   /// Appends the content of the array to the vector.
   ///
   /// @param addr   A pointer to the first item of the array to merge.
   /// @param count  The number of elements in the array to merge.
   ///
   void merge( const T *addr, U32 count );

   // Reverses the order of elements.
   void reverse();

   /// @}
};

template<class T> inline Vector<T>::~Vector()
{
   clear();
   dFree(mArray);
}

template<class T> inline Vector<T>::Vector(const U32 initialSize)
{
#ifdef TORQUE_DEBUG_GUARD
   mFileAssociation = NULL;
   mLineAssociation = 0;
#endif

   mArray        = 0;
   mElementCount = 0;
   mArraySize    = 0;
   if(initialSize)
      reserve(initialSize);
}

template<class T> inline Vector<T>::Vector(const U32 initialSize,
                                           const char* fileName,
                                           const U32   lineNum)
{
#ifdef TORQUE_DEBUG_GUARD
   mFileAssociation = fileName;
   mLineAssociation = lineNum;
#else
//   TORQUE_UNUSED(fileName);
//   TORQUE_UNUSED(lineNum);
#endif

   mArray        = 0;
   mElementCount = 0;
   mArraySize    = 0;
   if(initialSize)
      reserve(initialSize);
}

template<class T> inline Vector<T>::Vector(const char* fileName,
                                           const U32   lineNum)
{
#ifdef TORQUE_DEBUG_GUARD
   mFileAssociation = fileName;
   mLineAssociation = lineNum;
#else
//   TORQUE_UNUSED(fileName);
//   TORQUE_UNUSED(lineNum);
#endif

   mArray        = 0;
   mElementCount = 0;
   mArraySize    = 0;
}

template<class T> inline Vector<T>::Vector(const Vector& p)
{
#ifdef TORQUE_DEBUG_GUARD
   mFileAssociation = p.mFileAssociation;
   mLineAssociation = p.mLineAssociation;
#endif

   mArray = 0;
   resize(p.mElementCount);
   construct(0, p.mElementCount, p.mArray);
}


#ifdef TORQUE_DEBUG_GUARD
template<class T> inline void Vector<T>::setFileAssociation(const char* file,
                                                            const U32   line)
{
   mFileAssociation = file;
   mLineAssociation = line;
}
#endif

template<class T> inline void  Vector<T>::destroy(U32 start, U32 end) // destroys from start to end-1
{
   // This check is a little generous as we can legitimately get (0,0) as
   // our parameters... so it won't detect every invalid case but it does
   // remain simple.
   AssertFatal(start <= mElementCount && end <= mElementCount, "Vector<T>::destroy - out of bounds start/end.");

   // destroys from start to end-1
   while(start < end)
      destructInPlace(&mArray[start++]);
}

template<class T> inline void  Vector<T>::construct(U32 start, U32 end) // destroys from start to end-1
{
   AssertFatal(start <= mElementCount && end <= mElementCount, "Vector<T>::construct - out of bounds start/end.");
   while(start < end)
      constructInPlace(&mArray[start++]);
}

template<class T> inline void  Vector<T>::construct(U32 start, U32 end, const T* array) // destroys from start to end-1
{
   AssertFatal(start <= mElementCount && end <= mElementCount, "Vector<T>::construct - out of bounds start/end.");
   while(start < end)
   {
      constructInPlace(&mArray[start], &array[start]);
      start++;
   }
}

template<class T> inline U32 Vector<T>::memSize() const
{
   return capacity() * sizeof(T);
}

template<class T> inline T* Vector<T>::address() const
{
   return mArray;
}

template<class T> inline U32 Vector<T>::setSize(U32 size)
{
   const U32 oldSize = mElementCount;
   
   if(size > mElementCount)
   {
      if (size > mArraySize)
         resize(size);

      // Set count first so we are in a valid state for construct.
      mElementCount = size;
      construct(oldSize, size);
   }
   else if(size < mElementCount)
   {
      destroy(size, oldSize);
      mElementCount = size;
   }

   return mElementCount;
}

template<class T> inline void Vector<T>::increment()
{
   if(mElementCount == mArraySize)
      resize(mElementCount + 1);
   else
      mElementCount++;
   constructInPlace(&mArray[mElementCount - 1]);
}

template<class T> inline void Vector<T>::decrement()
{
   AssertFatal(mElementCount != 0, "Vector<T>::decrement - cannot decrement zero-length vector.");
   mElementCount--;
   destructInPlace(&mArray[mElementCount]);
}

template<class T> inline void Vector<T>::increment(U32 delta)
{
   U32 count = mElementCount;
   if ((mElementCount += delta) > mArraySize)
      resize(mElementCount);
   construct(count, mElementCount);
}

template<class T> inline void Vector<T>::decrement(U32 delta)
{
   AssertFatal(mElementCount != 0, "Vector<T>::decrement - cannot decrement zero-length vector.");

   const U32 count = mElementCount;

   // Determine new count after decrement...
   U32 newCount = mElementCount;
   if (mElementCount > delta)
      newCount -= delta;
   else
      newCount = 0;

   // Destruct removed items...
   destroy(newCount, count);

   // Note new element count.
   mElementCount = newCount;
}

template<class T> inline void Vector<T>::insert(U32 index)
{
   AssertFatal(index <= mElementCount, "Vector<T>::insert - out of bounds index.");

   if(mElementCount == mArraySize)
      resize(mElementCount + 1);
   else
      mElementCount++;

   dMemmove(&mArray[index + 1],
                    &mArray[index],
                    (mElementCount - index - 1) * sizeof(value_type));
   
   constructInPlace(&mArray[index]);
}

template<class T> inline void Vector<T>::insert(U32 index,const T& x)
{
   insert(index);
   mArray[index] = x;
}

template<class T> inline void Vector<T>::erase(U32 index)
{
   AssertFatal(index < mElementCount, "Vector<T>::erase - out of bounds index!");

   destructInPlace(&mArray[index]);

   if (index < (mElementCount - 1))
   {
      dMemmove(&mArray[index],
         &mArray[index + 1],
         (mElementCount - index - 1) * sizeof(value_type));
   }

   mElementCount--;
}

template<class T> inline bool Vector<T>::remove( const T& x )
{
   iterator i = begin();
   while (i != end())
   {
      if (*i == x)
      {
         erase( i );
         return true;
      }

      i++;
   }

   return false;
}

template<class T> inline void Vector<T>::erase(U32 index, U32 count)
{
   AssertFatal(index < mElementCount, "Vector<T>::erase - out of bounds index!");
   AssertFatal(count > 0, "Vector<T>::erase - count must be greater than zero!");
   AssertFatal(index+count <= mElementCount, "Vector<T>::erase - out of bounds count!");

   destroy( index, index+count );

   dMemmove(   &mArray[index],
               &mArray[index + count],
               (mElementCount - index - count) * sizeof(value_type));

   mElementCount -= count;
}

template<class T> inline void Vector<T>::erase_fast(U32 index)
{
   AssertFatal(index < mElementCount, "Vector<T>::erase_fast - out of bounds index.");

   // CAUTION: this operator does NOT maintain list order
   // Copy the last element into the deleted 'hole' and decrement the
   //   size of the vector.
   destructInPlace(&mArray[index]);
   if (index < (mElementCount - 1))
      dMemmove(&mArray[index], &mArray[mElementCount - 1], sizeof(value_type));
   mElementCount--;
}

template<class T> inline bool Vector<T>::contains(const T& x) const
{
	const_iterator i = begin();
	while (i != end())
	{
		if (*i == x)
			return true;

		i++;
	}

	return false;
}

template< class T > inline void Vector< T >::fill( const T& value )
{
   for( U32 i = 0; i < size(); ++ i )
      mArray[ i ] = value;
}

template<class T> inline T& Vector<T>::first()
{
   AssertFatal(mElementCount != 0, "Vector<T>::first - Error, no first element of a zero sized array!");
   return mArray[0];
}

template<class T> inline const T& Vector<T>::first() const
{
   AssertFatal(mElementCount != 0, "Vector<T>::first - Error, no first element of a zero sized array! (const)");
   return mArray[0];
}

template<class T> inline T& Vector<T>::last()
{
   AssertFatal(mElementCount != 0, "Vector<T>::last - Error, no last element of a zero sized array!");
   return mArray[mElementCount - 1];
}

template<class T> inline const T& Vector<T>::last() const
{
   AssertFatal(mElementCount != 0, "Vector<T>::last - Error, no last element of a zero sized array! (const)");
   return mArray[mElementCount - 1];
}

template<class T> inline void Vector<T>::clear()
{
   destroy(0, mElementCount);
   mElementCount = 0;
}

template<class T> inline void Vector<T>::compact()
{
   resize(mElementCount);
}

typedef S32 (QSORT_CALLBACK *qsort_compare_func)(const void *, const void *);

template<class T> inline void Vector<T>::sort(compare_func f)
{
   qsort(address(), size(), sizeof(T), (qsort_compare_func) f);
}

//-----------------------------------------------------------------------------

template<class T> inline Vector<T>& Vector<T>::operator=(const Vector<T>& p)
{
   if(mElementCount > p.mElementCount)
   {
      destroy(p.mElementCount, mElementCount);
   }
   
   U32 count = getMin( mElementCount, p.mElementCount );
   U32 i;
   for( i=0; i < count; i++ )
   {
      mArray[i] = p.mArray[i];
   }
   
   resize( p.mElementCount );
   
   if( i < p.mElementCount )
   {
      construct(i, p.mElementCount, p.mArray);
   }
   return *this;
}

template<class T> inline typename Vector<T>::iterator Vector<T>::begin()
{
   return mArray;
}

template<class T> inline typename Vector<T>::const_iterator Vector<T>::begin() const
{
   return mArray;
}

template<class T> inline typename Vector<T>::iterator Vector<T>::end()
{
   return mArray + mElementCount;
}

template<class T> inline typename Vector<T>::const_iterator Vector<T>::end() const
{
   return mArray +mElementCount;
}

template<class T> inline S32 Vector<T>::size() const
{
   return (S32)mElementCount;
}

template<class T> inline bool Vector<T>::empty() const
{
   return (mElementCount == 0);
}

template<class T> inline void Vector<T>::insert(iterator p,const T& x)
{
   U32 index = (U32) (p - mArray);
   insert(index);
   mArray[index] = x;
}

template<class T> inline void Vector<T>::erase(iterator q)
{
   erase(U32(q - mArray));
}

template<class T> inline void Vector<T>::erase_fast(iterator q)
{
   erase_fast(U32(q - mArray));
}

template<class T> inline T& Vector<T>::front()
{
   return *begin();
}

template<class T> inline const T& Vector<T>::front() const
{
   return *begin();
}

template<class T> inline T& Vector<T>::back()
{
   AssertFatal(mElementCount != 0, "Vector<T>::back - cannot access last element of zero-length vector.");
   return *(end()-1);
}

template<class T> inline const T& Vector<T>::back() const
{
   AssertFatal(mElementCount != 0, "Vector<T>::back - cannot access last element of zero-length vector.");
   return *(end()-1);
}

template<class T> inline void Vector<T>::push_front(const T& x)
{
   insert(0);
   mArray[0] = x;
}

template<class T> inline void Vector<T>::push_back(const T& x)
{
   increment();
   mArray[mElementCount - 1] = x;
}

template<class T> inline U32 Vector<T>::push_front_unique(const T& x)
{
   S32 index = find_next(x);

   if (index == -1)
   {
      index = 0;

      insert(index);
      mArray[index] = x;
   }

   return index;
}

template<class T> inline U32 Vector<T>::push_back_unique(const T& x)
{
   S32 index = find_next(x);

   if (index == -1)
   {
      increment();

      index = mElementCount - 1;
      mArray[index] = x;
   }

   return index;
}

template<class T> inline S32 Vector<T>::find_next( const T& x, U32 start ) const
{
   S32 index = -1;

   if (start < mElementCount)
   {
      for (U32 i = start; i < mElementCount; i++)
      {
         if (mArray[i] == x)
         {
            index = i;
            break;
         }
      }
   }

   return index;
}

template<class T> inline void Vector<T>::pop_front()
{
   AssertFatal(mElementCount != 0, "Vector<T>::pop_front - cannot pop the front of a zero-length vector.");
   erase(U32(0));
}

template<class T> inline void Vector<T>::pop_back()
{
   AssertFatal(mElementCount != 0, "Vector<T>::pop_back - cannot pop the back of a zero-length vector.");
   decrement();
}

template<class T> inline T& Vector<T>::operator[](U32 index)
{
   AssertFatal(index < mElementCount, avar("Vector<T>::operator[%i/%i] - out of bounds array access!", index, mElementCount));
   return mArray[index];
}

template<class T> inline const T& Vector<T>::operator[](U32 index) const
{
   AssertFatal(index < mElementCount, avar("Vector<T>::operator[%i/%i] - out of bounds array access!", index, mElementCount));
   return mArray[index];
}

template<class T> inline void Vector<T>::reserve(U32 size)
{
   if (size <= mArraySize)
      return;

   const U32 ec = mElementCount;
   if (resize(size))
      mElementCount = ec;
}

template<class T> inline U32 Vector<T>::capacity() const
{
    return mArraySize;
}

template<class T> inline void Vector<T>::set(void * addr, U32 sz)
{
   if ( !addr )
      sz = 0;

   setSize( sz );

   if ( addr && sz > 0 )
      dMemcpy(address(),addr,sz*sizeof(T));
}

//-----------------------------------------------------------------------------

template<class T> inline bool Vector<T>::resize(U32 ecount)
{
#ifdef TORQUE_DEBUG_GUARD
   return VectorResize(&mArraySize, &mElementCount, (void**) &mArray, ecount, sizeof(T),
                       mFileAssociation, mLineAssociation);
#else
   return VectorResize(&mArraySize, &mElementCount, (void**) &mArray, ecount, sizeof(T));
#endif
}

template<class T> inline void Vector<T>::merge( const Vector &p )
{
   if ( !p.size() )
      return;

   const U32 oldSize = mElementCount;
   const U32 newSize = oldSize + p.size();
   if ( newSize > mArraySize )
      resize( newSize );

   T *dest = mArray + oldSize;
   const T *src = p.mArray;
   while ( dest < mArray + newSize )
      constructInPlace( dest++, src++ );

   mElementCount = newSize;
}

template<class T> inline void Vector<T>::merge( const T *addr, U32 count )
{
   const U32 oldSize = mElementCount;
   const U32 newSize = oldSize + count;
   if ( newSize > mArraySize )
      resize( newSize );

   T *dest = mArray + oldSize;
   while ( dest < mArray + newSize )
      constructInPlace( dest++, addr++ );

   mElementCount = newSize;
}

template<class T> inline void Vector<T>::reverse()
{
   for (U32 i = 0, j = size();  (i != j) && (i != --j);  ++i)
      std::swap( mArray[ i ],  mArray[ j ] );
}

//-----------------------------------------------------------------------------
/// Template for vectors of pointers.
template <class T>
class VectorPtr : public Vector<void *>
{
   /// @deprecated Disallowed.
   VectorPtr(const VectorPtr&);  // Disallowed

  public:
   VectorPtr();
   VectorPtr(const char* fileName, const U32 lineNum);

   /// @name STL interface
   /// @{

   typedef T        value_type;
   typedef T&       reference;
   typedef const T& const_reference;

   typedef T*       iterator;
   typedef const T* const_iterator;
   typedef U32      difference_type;
   typedef U32      size_type;

   iterator       begin();
   const_iterator begin() const;
   iterator       end();
   const_iterator end() const;

   void insert(iterator,const T&);
   void insert(S32 idx) { Parent::insert(idx); }
   void erase(iterator);

   T&       front();
   const T& front() const;
   T&       back();
   const T& back() const;
   void     push_front(const T&);
   void     push_back(const T&);

   T&       operator[](U32);
   const T& operator[](U32) const;

   /// @}

   /// @name Extended interface
   /// @{

   typedef Vector<void*> Parent;
   T&       first();
   T&       last();
   const T& first() const;
   const T& last() const;
   void erase_fast(U32);
   void erase_fast(iterator);

   /// @}
};


//-----------------------------------------------------------------------------
template<class T> inline VectorPtr<T>::VectorPtr()
{
   //
}

template<class T> inline VectorPtr<T>::VectorPtr(const char* fileName,
                                                 const U32   lineNum)
   : Vector<void*>(fileName, lineNum)
{
   //
}

template<class T> inline T& VectorPtr<T>::first()
{
   return (T&)Parent::first();
}

template<class T> inline const T& VectorPtr<T>::first() const
{
   return (const T)Parent::first();
}

template<class T> inline T& VectorPtr<T>::last()
{
   return (T&)Parent::last();
}

template<class T> inline const T& VectorPtr<T>::last() const
{
   return (const T&)Parent::last();
}

template<class T> inline typename VectorPtr<T>::iterator VectorPtr<T>::begin()
{
   return (iterator)Parent::begin();
}

template<class T> inline typename VectorPtr<T>::const_iterator VectorPtr<T>::begin() const
{
   return (const_iterator)Parent::begin();
}

template<class T> inline typename VectorPtr<T>::iterator VectorPtr<T>::end()
{
   return (iterator)Parent::end();
}

template<class T> inline typename VectorPtr<T>::const_iterator VectorPtr<T>::end() const
{
   return (const_iterator)Parent::end();
}

template<class T> inline void VectorPtr<T>::insert(iterator i,const T& x)
{
   Parent::insert( (Parent::iterator)i, (Parent::reference)x );
}

template<class T> inline void VectorPtr<T>::erase(iterator i)
{
   Parent::erase( (Parent::iterator)i );
}

template<class T> inline void VectorPtr<T>::erase_fast(U32 index)
{
   AssertFatal(index < mElementCount, "VectorPtr<T>::erase_fast - out of bounds index." );

   // CAUTION: this operator does not maintain list order
   // Copy the last element into the deleted 'hole' and decrement the
   //   size of the vector.
   // Assert: index >= 0 && index < mElementCount
   if (index < (mElementCount - 1))
      mArray[index] = mArray[mElementCount - 1];
   decrement();
}

template<class T> inline void VectorPtr<T>::erase_fast(iterator i)
{
   erase_fast(U32(i - iterator(mArray)));
}

template<class T> inline T& VectorPtr<T>::front()
{
   return *begin();
}

template<class T> inline const T& VectorPtr<T>::front() const
{
   return *begin();
}

template<class T> inline T& VectorPtr<T>::back()
{
   AssertFatal(mElementCount != 0, "Vector<T>::back - cannot access last element of zero-length vector.");
   return *(end()-1);
}

template<class T> inline const T& VectorPtr<T>::back() const
{
   AssertFatal(mElementCount != 0, "Vector<T>::back - cannot access last element of zero-length vector.");
   return *(end()-1);
}

template<class T> inline void VectorPtr<T>::push_front(const T& x)
{
   Parent::push_front((Parent::const_reference)x);
}

template<class T> inline void VectorPtr<T>::push_back(const T& x)
{
   Parent::push_back((Parent::const_reference)x);
}

template<class T> inline T& VectorPtr<T>::operator[](U32 index)
{
   return (T&)Parent::operator[](index);
}

template<class T> inline const T& VectorPtr<T>::operator[](U32 index) const
{
   return (const T&)Parent::operator[](index);
}

//------------------------------------------------------------------------------

template <class T> class VectorSet : public Vector<T>
{
public:
   void insert(T dat)
   {
      if(find(this->begin(), this->end(), dat) == this->end())
         push_back(dat);
   }
};

// Include vector specializations
#ifndef _VECTORSPEC_H_
#include "core/util/tVectorSpecializations.h"
#endif

#endif //_TVECTOR_H_

