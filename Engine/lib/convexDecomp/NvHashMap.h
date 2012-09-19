/*

NvHashMap.h : A simple hash map and array template class to avoid introducing dependencies on the STL for containers.

*/


// This code contains NVIDIA Confidential Information and is disclosed
// under the Mutual Non-Disclosure Agreement.
//
// Notice
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright � 2009 NVIDIA Corporation. All rights reserved.
// Copyright � 2002-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright � 2001-2006 NovodeX. All rights reserved.

#ifndef NV_HASH_MAP_H
#define NV_HASH_MAP_H

#include "NvUserMemAlloc.h"

#if (defined(NX_WINDOWS) | defined(NX_X360))
#include <typeinfo.h>
#endif

#include <new>
#include <typeinfo>
#include <stdlib.h>
#include <string.h>
//******************************************************
//******************************************************
//******************************************************


#ifndef NV_FOUNDATION_BASIC_TEMPLATES_H
#define NV_FOUNDATION_BASIC_TEMPLATES_H

#pragma warning(push)
#pragma warning(disable:4512) // suppress the 'assignment operator could not be generated' warning message.

namespace CONVEX_DECOMPOSITION
{
	template<typename A>
	struct Equal
	{
		bool operator()(const A& a, const A& b)	const { return a==b; }
	};

	template<typename A>
	struct Less
	{
		bool operator()(const A& a, const A& b)	const { return a<b; }
	};

	template<typename A>
	struct Greater
	{
		bool operator()(const A& a, const A& b)	const { return a>b; }
	};


	template <class F, class S> 
	class Pair
	{
	public:
		F	first;
		S	second;
		Pair(): first(F()), second(S()) {}
		Pair(const F &f, const S &s): first(f), second(s) {}
		Pair(const Pair &p): first(p.first), second(p.second) {}
	};

	template<unsigned int A>	struct LogTwo	{	static const unsigned int value  = LogTwo<(A>>1)>::value + 1; };
	template<>					struct LogTwo<1>{	static const unsigned int value  = 0;	};

	template<typename T> struct UnConst	{ typedef T Type; };
	template<typename T> struct UnConst<const T> { typedef T Type; };
}

#pragma warning(pop)

#endif

#ifndef NV_FOUNDATION_ALLOCATOR
#define NV_FOUNDATION_ALLOCATOR

#pragma warning(push)
#pragma warning(disable:4100)

namespace CONVEX_DECOMPOSITION
{


/**
\brief The return value is the greater of the two specified values.
*/
template<class N>
NX_INLINE N NxMax(N a, N b)							{	return a<b ? b : a;						}


/**
\brief The return value is the greater of the two specified values.
*/
template <>
NX_INLINE NxF32 NxMax(NxF32 a, NxF32 b)				{	return  a > b ? a : b;	}

/**
\brief The return value is the lesser of the two specified values.
*/
template<class N>
NX_INLINE N NxMin(N a, N b)							{	return a<b ? a : b;						}

/**
\brief The return value is the lesser of the two specified values.
*/
template <>
NX_INLINE NxF32 NxMin(NxF32 a, NxF32 b)				{	return a < b ? a : b;	}



	/**
	Allocator used to access the global NxUserAllocator instance without providing additional information.
	*/
	class Allocator
	{
	public:
		Allocator(const char* dummy = 0) 
		{
		}
		void* allocate(size_t size, const char* file, int line)
		{
      return MEMALLOC_MALLOC(size);
		}
		void deallocate(void* ptr)
		{
      MEMALLOC_FREE(ptr);
		}
	};

	/**
	Allocator used to access the global NxUserAllocator instance using a dynamic name.
	*/
	class NamedAllocator
	{
	public:
		NamedAllocator(const char* name = 0) 
			
		{

    }
		void* allocate(size_t size, const char* filename, int line)
		{
      return MEMALLOC_MALLOC(size);
		}
		void deallocate(void* ptr)
		{
      MEMALLOC_FREE(ptr);
		}
	private:
	};

	/**
	Allocator used to access the global NxUserAllocator instance using a static name derived from T.
	*/
	template <typename T>
	class ReflectionAllocator
	{
		static const char* getName()
		{
#if defined NX_GNUC
			return __PRETTY_FUNCTION__;
#else
			return typeid(T).name();
#endif
		}
	public:
		ReflectionAllocator(const char* dummy=0) 
		{
		}
		void* allocate(size_t size, const char* filename, int line)
		{
      return MEMALLOC_MALLOC(size);
		}
		void deallocate(void* ptr)
		{
      MEMALLOC_FREE(ptr);
		}
	};

	// if you get a build error here, you are trying to NX_NEW a class 
	// that is neither plain-old-type nor derived from CONVEX_DECOMPOSITION::UserAllocated
	template <typename T, typename X>
	union EnableIfPod
	{
		int i; T t;
		typedef X Type;
	};

}

// Global placement new for ReflectionAllocator templated by plain-old-type. Allows using NX_NEW for pointers and built-in-types.
// ATTENTION: You need to use NX_DELETE_POD or NX_FREE to deallocate memory, not NX_DELETE. NX_DELETE_POD redirects to NX_FREE.
// Rationale: NX_DELETE uses global operator delete(void*), which we dont' want to overload. 
// Any other definition of NX_DELETE couldn't support array syntax 'NX_DELETE([]a);'. 
// NX_DELETE_POD was preferred over NX_DELETE_ARRAY because it is used less often and applies to both single instances and arrays.
template <typename T>
NX_INLINE void* operator new(size_t size, CONVEX_DECOMPOSITION::ReflectionAllocator<T> alloc, const char* fileName, typename CONVEX_DECOMPOSITION::EnableIfPod<T, int>::Type line)
{
	return alloc.allocate(size, fileName, line);
}

template <typename T>
NX_INLINE void* operator new[](size_t size, CONVEX_DECOMPOSITION::ReflectionAllocator<T> alloc, const char* fileName, typename CONVEX_DECOMPOSITION::EnableIfPod<T, int>::Type line)
{
	return alloc.allocate(size, fileName, line);
}

// If construction after placement new throws, this placement delete is being called.
template <typename T>
NX_INLINE void  operator delete(void* ptr, CONVEX_DECOMPOSITION::ReflectionAllocator<T> alloc, const char* fileName, typename CONVEX_DECOMPOSITION::EnableIfPod<T, int>::Type line)
{
	alloc.deallocate(ptr);
}

// If construction after placement new throws, this placement delete is being called.
template <typename T>
NX_INLINE void  operator delete[](void* ptr, CONVEX_DECOMPOSITION::ReflectionAllocator<T> alloc, const char* fileName, typename CONVEX_DECOMPOSITION::EnableIfPod<T, int>::Type line)
{
	alloc.deallocate(ptr);
}

#pragma warning(pop)

#endif


#ifndef NV_FOUNDATION_USERALLOCATED
#define NV_FOUNDATION_USERALLOCATED

// an expression that should expand to nothing in _DEBUG builds.  We currently
// use this only for tagging the purpose of containers for memory use tracking.
#if defined(_DEBUG)
#define NV_DEBUG_EXP(x) (x)
#define NV_DEBUG_EXP_C(x) x,
#else
#define NV_DEBUG_EXP(x)
#define NV_DEBUG_EXP_C(x)
#endif

#if defined (NX_X360) | defined (NX_WINDOWS) | defined (NX_CELL) | defined (NXLINUX) | defined(NX_WII) 
// Stack allocation with alloc fallback for large allocations (>50% of default stack size for platform)
#	define NX_ALLOCA(var, type, number)											\
		bool alloced_##var = false;												\
    if (sizeof(type)*number*2 > (CONVEX_DECOMPOSITION::gSystemServices ? gSystemServices->getAllocaThreshold() : 8192)  )	\
		{																		\
			var = (type *)MEMALLOC_MALLOC(sizeof(type)*number);					\
			alloced_##var = true;												\
		} else {																\
			var = (type *)MEMALLOC_ALLOCA(sizeof(type)*number);						\
		}
#	define NX_FREEA(var) if (alloced_##var) MEMALLOC_FREE(var);
#else
#	define NX_ALLOCA(var, type, number)		var = (type *)NxAlloca(sizeof(type)*number);
#	define NX_FREEA(var)					0;
#endif

namespace CONVEX_DECOMPOSITION
{
	/**
	Provides new and delete using a UserAllocator.
	Guarantees that 'delete x;' uses the UserAllocator too.
	*/
	class UserAllocated
	{
	public:

		template <typename Alloc>
		NX_INLINE void* operator new(size_t size, Alloc alloc, const char* fileName, int line)
		{
      return MEMALLOC_MALLOC(size);
		}
		template <typename Alloc>
		NX_INLINE void* operator new[](size_t size, Alloc alloc, const char* fileName, int line)
		{
      return MEMALLOC_MALLOC(size);
		}

		NX_INLINE void  operator delete(void* ptr)
		{
      MEMALLOC_FREE(ptr);
		}
		NX_INLINE void  operator delete[](void* ptr)
		{
      MEMALLOC_FREE(ptr);
		}
	};
};

#endif


#ifndef NV_FOUNDATION_ALIGNEDMALLOC_H
#define NV_FOUNDATION_ALIGNEDMALLOC_H

/*!
Allocate aligned memory.
Alignment must be a power of 2!
-- should be templated by a base allocator
*/

namespace CONVEX_DECOMPOSITION
{
	/**
	Allocator, which is used to access the global NxUserAllocator instance
	(used for dynamic data types template instantiation), which can align memory
	*/

	// SCS: AlignedMalloc with 3 params not found, seems not used on PC either
	// disabled for now to avoid GCC error

	template<NxU32 N, typename BaseAllocator = Allocator >
	class AlignedAllocator : public BaseAllocator
	{
	public:
		AlignedAllocator(const BaseAllocator& base = BaseAllocator()) 
		: BaseAllocator(base) {}

		void* allocate(size_t size, const char* file, int line)
		{
			size_t pad = N - 1 + sizeof(size_t); // store offset for delete.
			NxU8* base = (NxU8*)BaseAllocator::allocate(size+pad, file, line);

			NxU8* ptr = (NxU8*)(size_t(base + pad) & ~(N - 1)); // aligned pointer
			((size_t*)ptr)[-1] = ptr - base; // store offset

			return ptr;
		}
		void deallocate(void* ptr)
		{
			if(ptr == NULL)
				return;

			NxU8* base = ((NxU8*)ptr) - ((size_t*)ptr)[-1];
			BaseAllocator::deallocate(base);
		}
	};
}

#endif


#ifndef NV_FOUNDATION_INLINE_ALLOCATOR_H
#define NV_FOUNDATION_INLINE_ALLOCATOR_H

namespace CONVEX_DECOMPOSITION
{
	// this is used by the array class to allocate some space for a small number
	// of objects along with the metadata
	template<NxU32 N, typename BaseAllocator>
	class InlineAllocator : private BaseAllocator
	{
	public:

		InlineAllocator(const BaseAllocator& alloc = BaseAllocator())
			: BaseAllocator(alloc)
		{}

		void* allocate(size_t size, const char* filename, int line)
		{
			return size <= N ? mBuffer : BaseAllocator::allocate(size, filename, line);
		}

		void deallocate(void* ptr)
		{
			if(ptr != mBuffer)
				BaseAllocator::deallocate(ptr);
		}

	private:
		NxU8 mBuffer[N];
	};
}

#endif


#ifndef NV_FOUNDATION_NXSTRIDEDDATA
#define NV_FOUNDATION_NXSTRIDEDDATA
/** \addtogroup foundation
  @{
*/

template<typename T>
class NvStrideIterator
{
	template <typename X>
	struct StripConst
	{
		typedef X Type;
	};

	template <typename X>
	struct StripConst<const X>
	{
		typedef X Type;
	};

public:
	explicit NX_INLINE NvStrideIterator(T* ptr = NULL, NxU32 stride = sizeof(T)) :
		mPtr(ptr), mStride(stride)
	{
		NX_ASSERT(mStride == 0 || sizeof(T) <= mStride);
	}

	NX_INLINE NvStrideIterator(const NvStrideIterator<typename StripConst<T>::Type>& strideIterator) :
		mPtr(strideIterator.ptr()), mStride(strideIterator.stride())
	{
		NX_ASSERT(mStride == 0 || sizeof(T) <= mStride);
	}

	NX_INLINE T* ptr() const
	{
		return mPtr;
	}

	NX_INLINE NxU32 stride() const
	{
		return mStride;
	}

	NX_INLINE T& operator*() const
	{
		return *mPtr;
	}

	NX_INLINE T* operator->() const
	{
		return mPtr;
	}

	NX_INLINE T& operator[](int i) const
	{
		return *byteAdd(mPtr, i * stride());
	}

	// preincrement
	NX_INLINE NvStrideIterator& operator++()
	{
		mPtr = byteAdd(mPtr, stride());
		return *this;
	}

	// postincrement
	NX_INLINE NvStrideIterator operator++(int)
	{
		NvStrideIterator tmp = *this;
		mPtr = byteAdd(mPtr, stride());
		return tmp;
	}

	// predecrement
	NX_INLINE NvStrideIterator& operator--()
	{
		mPtr = byteSub(mPtr, stride());
		return *this;
	}

	// postdecrement
	NX_INLINE NvStrideIterator operator--(int)
	{
		NvStrideIterator tmp = *this;
		mPtr = byteSub(mPtr, stride());
		return tmp;
	}

	NX_INLINE NvStrideIterator& operator+=(int i)
	{
		mPtr = byteAdd(mPtr, i * stride());
		return *this;
	}

	NX_INLINE NvStrideIterator operator+(int i) const
	{	
		return NvStrideIterator(byteAdd(mPtr, i * stride()), stride());
	}

	NX_INLINE NvStrideIterator& operator-=(int i)
	{
		mPtr = byteSub(mPtr, i * stride());
		return *this;
	}

	NX_INLINE NvStrideIterator operator-(int i) const
	{
		return NvStrideIterator(byteSub(mPtr, i * stride()), stride());
	}

	// iterator difference
	NX_INLINE int operator-(const NvStrideIterator& other) const
	{
		NX_ASSERT(isCompatible(other));
		int byteDiff = static_cast<int>(reinterpret_cast<const NxU8*>(mPtr) - reinterpret_cast<const NxU8*>(other.mPtr));
		return byteDiff / static_cast<int>(stride());
	}

	NX_INLINE bool operator==(const NvStrideIterator& other) const
	{
		NX_ASSERT(isCompatible(other));
		return mPtr == other.mPtr;
	}

	NX_INLINE bool operator!=(const NvStrideIterator& other) const
	{
		NX_ASSERT(isCompatible(other));
		return mPtr != other.mPtr;
	}

	NX_INLINE bool operator<(const NvStrideIterator& other) const
	{
		NX_ASSERT(isCompatible(other));
		return mPtr < other.mPtr;
	}

	NX_INLINE bool operator>(const NvStrideIterator& other) const
	{
		NX_ASSERT(isCompatible(other));
		return mPtr > other.mPtr;
	}

	NX_INLINE bool operator<=(const NvStrideIterator& other) const
	{
		NX_ASSERT(isCompatible(other));
		return mPtr <= other.mPtr;
	}

	NX_INLINE bool operator>=(const NvStrideIterator& other) const
	{
		NX_ASSERT(isCompatible(other));
		return mPtr >= other.mPtr;
	}

private:
	NX_INLINE static T* byteAdd(T* ptr, NxU32 bytes) 
	{ 
		return const_cast<T*>(reinterpret_cast<const T*>(reinterpret_cast<const NxU8*>(ptr) + bytes));
	}

	NX_INLINE static T* byteSub(T* ptr, NxU32 bytes)
	{ 
		return const_cast<T*>(reinterpret_cast<const T*>(reinterpret_cast<const NxU8*>(ptr) - bytes));
	}

	NX_INLINE bool isCompatible(const NvStrideIterator& other) const
	{
		int byteDiff = static_cast<int>(reinterpret_cast<const NxU8*>(mPtr) - reinterpret_cast<const NxU8*>(other.mPtr));
		return (stride() == other.stride()) && (abs(byteDiff) % stride() == 0);
	}

	T* mPtr;
	NxU32 mStride;
};


template<typename T>
NX_INLINE NvStrideIterator<T> operator+(int i, NvStrideIterator<T> it)
{
	it += i;
	return it;
}

 /** @} */
#endif

#ifndef NV_FOUNDATION_ARRAY
#define NV_FOUNDATION_ARRAY

namespace CONVEX_DECOMPOSITION
{
	namespace Internal
	{
		template <typename T>
		struct ArrayMetaData
		{
			T*					mData;
			NxU32				mCapacity;
			NxU32				mSize;
			ArrayMetaData(): mSize(0), mCapacity(0), mData(0) {}
		};

		template <typename T>
		struct AllocatorTraits
		{
#if defined _DEBUG
			typedef NamedAllocator Type;
#else
			typedef ReflectionAllocator<T> Type;
#endif
		};
	}

	/*!
	An array is a sequential container.

	Implementation note
	* entries between 0 and size are valid objects
	* we use inheritance to build this because the array is included inline in a lot
	  of objects and we want the allocator to take no space if it's not stateful, which
	  aggregation doesn't allow. Also, we want the metadata at the front for the inline
	  case where the allocator contains some inline storage space
	*/
	template<class T, class Alloc = typename Internal::AllocatorTraits<T>::Type >
	class Array : private Internal::ArrayMetaData<T>, private Alloc
	{
		typedef Internal::ArrayMetaData<T> MetaData;

		using MetaData::mCapacity;
		using MetaData::mData;
		using MetaData::mSize;

	public:

		typedef T*			Iterator;
		typedef const T*	ConstIterator;

		/*!
		Default array constructor. Initialize an empty array
		*/
		NX_INLINE Array(const Alloc& alloc = Alloc()) : Alloc(alloc) {}

		/*!
		Initialize array with given length
		*/
		NX_INLINE  explicit Array(NxU32 capacity, const Alloc& alloc = Alloc())
		: Alloc(alloc)
		{
			if(mCapacity>0)
				allocate(mCapacity);
		}

		/*!
		Copy-constructor. Copy all entries from other array
		*/
		template <class A> 
		NX_INLINE Array(const Array<T,A>& other, const Alloc& alloc = Alloc()) 
		{
			if(other.mSize > 0)
			{
				mData = allocate(mSize = mCapacity = other.mSize);
				copy(mData, other.mData, mSize);
			}
		}

		/*!
		Default destructor
		*/
		NX_INLINE ~Array()
		{
			destroy(0, mSize);
			if(mCapacity)
				deallocate(mData);
		}

		/*!
		Assignment operator. Copy content (deep-copy)
		*/
		template <class A> 
		NX_INLINE const Array& operator= (const Array<T,A>& t)
		{
			if(&t == this)
				return *this;

			if(mCapacity < t.mSize)
			{
				destroy(0,mSize);
				deallocate(mData);

				mData = allocate(t.mCapacity);
				mCapacity = t.mCapacity;

				copy(mData,t.mData,t.mSize);
				mSize = t.mSize;

				return;
			}
			else
			{
				NxU32 m = NxMin(t.mSize,mSize);
				copy(mData,t.mData,m);
				for(NxU32 i = m; i < mSize;i++)
					mData[i].~T();
				for(NxU32 i = m; i < t.mSize; i++)
					new(mData+i)T(t.mData[i]);
			}

			mSize = t.mSize;
			return *this;
		}

		/*!
		Array indexing operator.
		\param i
		The index of the element that will be returned.
		\return
		The element i in the array.
		*/
		NX_INLINE const T& operator[] (NxU32 i) const 
		{
			return mData[i];
		}

		/*!
		Array indexing operator.
		\param i
		The index of the element that will be returned.
		\return
		The element i in the array.
		*/
		NX_INLINE T& operator[] (NxU32 i) 
		{
			return mData[i];
		}

		/*!
		Returns a pointer to the initial element of the array.
		\return
		a pointer to the initial element of the array.
		*/
		NX_INLINE ConstIterator begin() const 
		{
			return mData;
		}

		NX_INLINE Iterator begin()
		{
			return mData;
		}

		/*!
		Returns an iterator beyond the last element of the array. Do not dereference.
		\return
		a pointer to the element beyond the last element of the array.
		*/

		NX_INLINE ConstIterator end() const 
		{
			return mData+mSize;
		}

		NX_INLINE Iterator end()
		{
			return mData+mSize;
		}

		/*!
		Returns a reference to the first element of the array. Undefined if the array is empty.
		\return a reference to the first element of the array
		*/

		NX_INLINE const T& front() const 
		{
			NX_ASSERT(mSize);
			return mData[0];
		}

		NX_INLINE T& front()
		{
			NX_ASSERT(mSize);
			return mData[0];
		}

		/*!
		Returns a reference to the last element of the array. Undefined if the array is empty
		\return a reference to the last element of the array
		*/

		NX_INLINE const T& back() const 
		{
			NX_ASSERT(mSize);
			return mData[mSize-1];
		}

		NX_INLINE T& back()
		{
			NX_ASSERT(mSize);
			return mData[mSize-1];
		}


		/*!
		Returns the number of entries in the array. This can, and probably will,
		differ from the array capacity.
		\return
		The number of of entries in the array.
		*/
		NX_INLINE NxU32 size() const 
		{
			return mSize;
		}

		/*!
		Clears the array.
		*/
		NX_INLINE void clear()
		{
			destroy(0,mSize);
			mSize = 0;
		}

		/*!
		Returns whether the array is empty (i.e. whether its size is 0).
		\return
		true if the array is empty
		*/
		NX_INLINE bool empty() const
		{
			return mSize==0;
		}

		/*!
		Finds the first occurrence of an element in the array.
		\param a
		The element that will be removed. 
		*/


		NX_INLINE Iterator find(const T&a)
		{
			NxU32 index;
			for(index=0;index<mSize && mData[index]!=a;index++)
				;
			return mData+index;
		}

		NX_INLINE ConstIterator find(const T&a) const
		{
			NxU32 index;
			for(index=0;index<mSize && mData[index]!=a;index++)
				;
			return mData+index;
		}


		/////////////////////////////////////////////////////////////////////////
		/*!
		Adds one element to the end of the array. Operation is O(1).
		\param a
		The element that will be added to this array.
		*/
		/////////////////////////////////////////////////////////////////////////

		NX_INLINE T& pushBack(const T& a)
		{
			if(mCapacity<=mSize) 
				grow(capacityIncrement());

			new((void*)(mData + mSize)) T(a);

			return mData[mSize++];
		}

		/////////////////////////////////////////////////////////////////////////
		/*!
		Returns the element at the end of the array. Only legal if the array is non-empty.
		*/
		/////////////////////////////////////////////////////////////////////////
		NX_INLINE T popBack() 
		{
			NX_ASSERT(mSize);
			T t = mData[mSize-1];
			mData[--mSize].~T();
			return t;
		}


		/////////////////////////////////////////////////////////////////////////
		/*!
		Construct one element at the end of the array. Operation is O(1).
		*/
		/////////////////////////////////////////////////////////////////////////
		NX_INLINE T& insert()
		{
			if(mCapacity<=mSize) 
				grow(capacityIncrement());

			return *(new (mData+mSize++)T);
		}

		/////////////////////////////////////////////////////////////////////////
		/*!
		Subtracts the element on position i from the array and replace it with
		the last element.
		Operation is O(1)
		\param i
		The position of the element that will be subtracted from this array.
		\return
		The element that was removed.
		*/
		/////////////////////////////////////////////////////////////////////////
		NX_INLINE void replaceWithLast(NxU32 i)
		{
			NX_ASSERT(i<mSize);
			mData[i] = mData[--mSize];
			mData[mSize].~T();
		}

		NX_INLINE void replaceWithLast(Iterator i) 
		{
			replaceWithLast(static_cast<NxU32>(i-mData));
		}

		/////////////////////////////////////////////////////////////////////////
		/*!
		Replaces the first occurrence of the element a with the last element
		Operation is O(n)
		\param i
		The position of the element that will be subtracted from this array.
		\return Returns true if the element has been removed.
		*/
		/////////////////////////////////////////////////////////////////////////

		NX_INLINE bool findAndReplaceWithLast(const T& a)
		{
			NxU32 index;
			for(index=0;index<mSize && mData[index]!=a;index++)
				;
			if(index >= mSize)
				return false;
			replaceWithLast(index);
			return true;
		}

		/////////////////////////////////////////////////////////////////////////
		/*!
		Subtracts the element on position i from the array. Shift the entire
		array one step.
		Operation is O(n)
		\param i
		The position of the element that will be subtracted from this array.
		\return
		The element that was removed.
		*/
		/////////////////////////////////////////////////////////////////////////
		NX_INLINE void remove(NxU32 i) 
		{
			NX_ASSERT(i<mSize);
			while(i+1<mSize)
			{
				mData[i] = mData[i+1];
				i++;
			}

			mData[--mSize].~T();
		}


		//////////////////////////////////////////////////////////////////////////
		/*!
		Resize array
		\param compaction
		If set to true and the specified size is smaller than the capacity, a new
		memory block which fits the size is allocated and the old one gets freed.
		*/
		//////////////////////////////////////////////////////////////////////////
		NX_INLINE void resize(const NxU32 size, const bool compaction = false, const T& a = T())
		{
			if(size > mCapacity)
			{
				grow(size);
			}
			else if (compaction && (size != mCapacity))
			{
				recreate(size, NxMin(mSize, size));
			}

			for(NxU32 i = mSize; i < size; i++)
				::new(mData+i)T(a);

			if (!compaction)  // With compaction, these elements have been deleted already
			{
				for(NxU32 i = size; i < mSize; i++)
					mData[i].~T();
			}

			mSize = size;
		}


		//////////////////////////////////////////////////////////////////////////
		/*!
		Resize array such that only as much memory is allocated to hold the 
		existing elements
		*/
		//////////////////////////////////////////////////////////////////////////
		NX_INLINE void shrink()
		{
			resize(mSize, true);
		}


		//////////////////////////////////////////////////////////////////////////
		/*!
		Deletes all array elements and frees memory.
		*/
		//////////////////////////////////////////////////////////////////////////
		NX_INLINE void reset()
		{
			resize(0, true);
		}


		//////////////////////////////////////////////////////////////////////////
		/*!
		Ensure that the array has at least size capacity.
		*/
		//////////////////////////////////////////////////////////////////////////
		NX_INLINE void reserve(const NxU32 size)
		{
			if(size > mCapacity)
				grow(size);
		}

		//////////////////////////////////////////////////////////////////////////
		/*!
		Query the capacity(allocated mem) for the array.
		*/
		//////////////////////////////////////////////////////////////////////////
		NX_INLINE NxU32 capacity()	const
		{
			return mCapacity;
		}


	private:

		NX_INLINE T* allocate(size_t capacity)
		{
			return (T*)Alloc::allocate(sizeof(T) * capacity, __FILE__, __LINE__);
		}

		NX_INLINE void deallocate(void *mem)
		{
			Alloc::deallocate(mem);
		}

		NX_INLINE void copy(T* dst, const T* src, size_t count)
		{
			for(size_t i=0;i<count;i++)
				::new (dst+i)T(src[i]);
		}

		NX_INLINE void destroy(size_t start, size_t end)
		{
			for(size_t i = start; i<end; i++)
				mData[i].~T();
		}

		// The idea here is to prevent accidental brain-damage with pushBack or insert. Unfortunately
		// it interacts badly with InlineArrays with smaller inline allocations.
		// TODO(dsequeira): policy template arg, this is exactly what they're for.
		NX_INLINE NxU32 capacityIncrement()	const
		{
			return mCapacity == 0 ? 1 : mCapacity * 2;
		}

		/*!
		Creates a new memory block, copies all entries to the new block and destroys old entries.

		\param capacity
		The number of entries that the set should be able to hold.
		\param copyCount
		The number of entries to copy.
		*/
		NX_INLINE void recreate(NxU32 capacity, NxU32 copyCount)
		{
			NX_ASSERT(capacity >= copyCount);
			NX_ASSERT(mSize >= copyCount);
			T* newData = allocate(capacity);
			NX_ASSERT(	((newData != NULL) && (capacity > 0)) ||
						((newData == NULL) && (capacity == 0)) );

			if(mCapacity)
			{
				copy(newData,mData,copyCount);
				destroy(0,mSize);
				deallocate(mData);
			}

			mData = newData;
			mCapacity = capacity;
		}

		/*!
		Resizes the available memory for the array.

		\param capacity
		The number of entries that the set should be able to hold.
		*/	
		NX_INLINE void grow(NxU32 capacity) 
		{
			NX_ASSERT(mCapacity < capacity);
			recreate(capacity, mSize);
		}
	};

	// array that pre-allocates for N elements
	template <typename T, NxU32 N, typename Alloc = typename Internal::AllocatorTraits<T>::Type>
	class InlineArray : public Array<T, InlineAllocator<N * sizeof(T), Alloc> >
	{
		typedef InlineAllocator<N * sizeof(T), Alloc> Allocator;
	public:
		NX_INLINE InlineArray(const Alloc& alloc = Alloc()) 
			: Array<T, Allocator>(alloc) 
		{}
	};
}

template <typename T>
NX_INLINE NvStrideIterator<T> getStrideIterator(CONVEX_DECOMPOSITION::Array<T>& array)
{
	return NvStrideIterator<T>(array.begin(), sizeof(T));
}

template <typename T>
NX_INLINE NvStrideIterator<const T> getConstStrideIterator(CONVEX_DECOMPOSITION::Array<T>& array)
{
	return NvStrideIterator<const T>(array.begin(), sizeof(T));
}


#endif

#ifndef NV_FOUNDATION_BITUTILS_H
#define NV_FOUNDATION_BITUTILS_H

namespace CONVEX_DECOMPOSITION
{
	NX_INLINE NxU32 bitCount32(NxU32 v)
	{
		// from http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
		NxU32 const w = v - ((v >> 1) & 0x55555555);
		NxU32 const x = (w & 0x33333333) + ((w >> 2) & 0x33333333);
		return ((x + (x >> 4) & 0xF0F0F0F) * 0x1010101) >> 24;
	}

	/*!
	Return the index of the highest set bit. Or 0 if no bits are set.
	*/
	NX_INLINE NxU32 highestSetBit32(NxU32 v)
	{
		for(NxU32 j = 32; j-- > 0;)
		{
			if(v&(1<<j))
				return j;
		}
		return 0;
	}

	NX_INLINE bool isPowerOfTwo(NxU32 x)
	{
		return x!=0 && (x & x-1) == 0;
	}

	// "Next Largest Power of 2
	// Given a binary integer value x, the next largest power of 2 can be computed by a SWAR algorithm
	// that recursively "folds" the upper bits into the lower bits. This process yields a bit vector with
	// the same most significant 1 as x, but all 1's below it. Adding 1 to that value yields the next
	// largest power of 2. For a 32-bit value:"
	NX_INLINE NxU32 nextPowerOfTwo(NxU32 x)
	{
		x |= (x >> 1);
		x |= (x >> 2);
		x |= (x >> 4);
		x |= (x >> 8);
		x |= (x >> 16);
		return x+1;
	}

	// Helper function to approximate log2 of an integer value (assumes that the input is actually power of two)
	NX_INLINE NxU32 ilog2(NxU32 num)
	{
		for (NxU32 i=0; i<32; i++)
		{
			num >>= 1;
			if (num == 0) return i;
		}

		NX_ASSERT(0);
		return (NxU32)-1;
	}

	NX_INLINE int intChop(const NxF32& f)
	{
		NxI32 a			= *reinterpret_cast<const NxI32*>(&f);			// take bit pattern of float into a register
		NxI32 sign		= (a>>31);										// sign = 0xFFFFFFFF if original value is negative, 0 if positive
		NxI32 mantissa	= (a&((1<<23)-1))|(1<<23);						// extract mantissa and add the hidden bit
		NxI32 exponent	= ((a&0x7fffffff)>>23)-127;						// extract the exponent
		NxI32 r			= ((NxU32)(mantissa)<<8)>>(31-exponent);		// ((1<<exponent)*mantissa)>>24 -- (we know that mantissa > (1<<24))
		return ((r ^ (sign)) - sign ) &~ (exponent>>31);				// add original sign. If exponent was negative, make return value 0.
	}

	NX_INLINE int intFloor(const NxF32& f)
	{
		NxI32 a			= *reinterpret_cast<const NxI32*>(&f);									// take bit pattern of float into a register
		NxI32 sign		= (a>>31);																// sign = 0xFFFFFFFF if original value is negative, 0 if positive
		a&=0x7fffffff;																			// we don't need the sign any more
		NxI32 exponent	= (a>>23)-127;															// extract the exponent
		NxI32 expsign   = ~(exponent>>31);														// 0xFFFFFFFF if exponent is positive, 0 otherwise
		NxI32 imask		= ( (1<<(31-(exponent))))-1;											// mask for true integer values
		NxI32 mantissa	= (a&((1<<23)-1));														// extract mantissa (without the hidden bit)
		NxI32 r			= ((NxU32)(mantissa|(1<<23))<<8)>>(31-exponent);						// ((1<<exponent)*(mantissa|hidden bit))>>24 -- (we know that mantissa > (1<<24))
		r = ((r & expsign) ^ (sign)) + ((!((mantissa<<8)&imask)&(expsign^((a-1)>>31)))&sign);	// if (fabs(value)<1.0) value = 0; copy sign; if (value < 0 && value==(int)(value)) value++;
		return r;
	}

	NX_INLINE int intCeil(const NxF32& f)
	{
		NxI32 a			= *reinterpret_cast<const NxI32*>(&f) ^ 0x80000000;						// take bit pattern of float into a register
		NxI32 sign		= (a>>31);																// sign = 0xFFFFFFFF if original value is negative, 0 if positive
		a&=0x7fffffff;																			// we don't need the sign any more
		NxI32 exponent	= (a>>23)-127;															// extract the exponent
		NxI32 expsign   = ~(exponent>>31);														// 0xFFFFFFFF if exponent is positive, 0 otherwise
		NxI32 imask		= ( (1<<(31-(exponent))))-1;											// mask for true integer values
		NxI32 mantissa	= (a&((1<<23)-1));														// extract mantissa (without the hidden bit)
		NxI32 r			= ((NxU32)(mantissa|(1<<23))<<8)>>(31-exponent);						// ((1<<exponent)*(mantissa|hidden bit))>>24 -- (we know that mantissa > (1<<24))
		r = ((r & expsign) ^ (sign)) + ((!((mantissa<<8)&imask)&(expsign^((a-1)>>31)))&sign);	// if (fabs(value)<1.0) value = 0; copy sign; if (value < 0 && value==(int)(value)) value++;
		return -r;
	}

}

#endif

#ifndef NV_FOUNDATION_HASHFUNCTION_H
#define NV_FOUNDATION_HASHFUNCTION_H

/*!
Central definition of hash functions
*/

namespace CONVEX_DECOMPOSITION
{
	// Hash functions
	template<class T>
	NxU32 hash(const T& key)
	{
		return (NxU32)key;
	}

	// Thomas Wang's 32 bit mix
	// http://www.cris.com/~Ttwang/tech/inthash.htm
	template<>
	NX_INLINE NxU32 hash<NxU32>(const NxU32& key)
	{
		NxU32 k = key;
		k += ~(k << 15);
		k ^= (k >> 10);
		k += (k << 3);
		k ^= (k >> 6);
		k += ~(k << 11);
		k ^= (k >> 16);
		return (NxU32)k;
	}

	template<>
	NX_INLINE NxU32 hash<NxI32>(const NxI32& key)
	{
		return hash((NxU32)key);
	}

	// Thomas Wang's 64 bit mix
	// http://www.cris.com/~Ttwang/tech/inthash.htm
	template<>
	NX_INLINE NxU32 hash<NxU64>(const NxU64& key)
	{
		NxU64 k = key;
		k += ~(k << 32);
		k ^= (k >> 22);
		k += ~(k << 13);
		k ^= (k >> 8);
		k += (k << 3);
		k ^= (k >> 15);
		k += ~(k << 27);
		k ^= (k >> 31);
		return (NxU32)k;
	}

	// Helper for pointer hashing
	template<int size>
	NxU32 PointerHash(const void* ptr);

	template<>
	NX_INLINE NxU32 PointerHash<4>(const void* ptr)
	{
		return hash<NxU32>(static_cast<NxU32>(reinterpret_cast<size_t>(ptr)));
	}


	template<>
	NX_INLINE NxU32 PointerHash<8>(const void* ptr)
	{
		return hash<NxU64>(reinterpret_cast<size_t>(ptr));
	}

	// Hash function for pointers
	template<class T>
	NX_INLINE NxU32 hash(T* key)
	{
		return PointerHash<sizeof(const void*)>(key);
	}

	// Hash function object for pointers
	template <class T>
	struct PointerHashFunctor
	{
		NxU32 operator()(const T* t) const
		{
			return PointerHash<sizeof(T*)>(t);
		}
		bool operator()(const T* t0, const T* t1) const
		{
			return t0 == t1;
		}
	};

	/*
	--------------------------------------------------------------------
	lookup2.c, by Bob Jenkins, December 1996, Public Domain.
	--------------------------------------------------------------------
	--------------------------------------------------------------------
	mix -- mix 3 32-bit values reversibly.
	For every delta with one or two bit set, and the deltas of all three
	high bits or all three low bits, whether the original value of a,b,c
	is almost all zero or is uniformly distributed,
	* If mix() is run forward or backward, at least 32 bits in a,b,c
	have at least 1/4 probability of changing.
	* If mix() is run forward, every bit of c will change between 1/3 and
	2/3 of the time.  (Well, 22/100 and 78/100 for some 2-bit deltas.)
	mix() was built out of 36 single-cycle latency instructions in a 
	structure that could supported 2x parallelism, like so:
	a -= b; 
	a -= c; x = (c>>13);
	b -= c; a ^= x;
	b -= a; x = (a<<8);
	c -= a; b ^= x;
	c -= b; x = (b>>13);
	...
	Unfortunately, superscalar Pentiums and Sparcs can't take advantage 
	of that parallelism.  They've also turned some of those single-cycle
	latency instructions into multi-cycle latency instructions.  Still,
	this is the fastest good hash I could find.  There were about 2^^68
	to choose from.  I only looked at a billion or so.
	--------------------------------------------------------------------
	*/
	NX_INLINE NxU32 hashMix(NxU32 &a, NxU32 &b, NxU32 &c)
	{
		a -= b; a -= c; a ^= (c>>13);
		b -= c; b -= a; b ^= (a<<8);
		c -= a; c -= b; c ^= (b>>13);
		a -= b; a -= c; a ^= (c>>12);
		b -= c; b -= a; b ^= (a<<16);
		c -= a; c -= b; c ^= (b>>5);
		a -= b; a -= c; a ^= (c>>3);
		b -= c; b -= a; b ^= (a<<10);
		c -= a; c -= b; c ^= (b>>15);
	}

	NX_INLINE NxU32 hash(const NxU32 *k, NxU32 length)
	{
		NxU32 a,b,c,len;

		/* Set up the internal state */
		len = length;
		a = b = 0x9e3779b9;  /* the golden ratio; an arbitrary value */
		c = 0;           /* the previous hash value */

		/*---------------------------------------- handle most of the key */
		while (len >= 3)
		{
			a += k[0];
			b += k[1];
			c += k[2];
			hashMix(a,b,c);
			k += 3; 
			len -= 3;
		}

		/*-------------------------------------- handle the last 2 ub4's */
		c += length;
		switch(len)              /* all the case statements fall through */
		{
			/* c is reserved for the length */
		case 2 : b+=k[1];
		case 1 : a+=k[0];
			/* case 0: nothing left to add */
		}
		hashMix(a,b,c);
		/*-------------------------------------------- report the result */
		return c;
	}

	template <class Key>
	class Hash
	{
	public:
		NxU32 operator()(const Key &k) const { return hash<Key>(k); }
		bool operator()(const Key &k0, const Key &k1) const { return k0 == k1; }
	};

	class NvStringHash
	{
	public:
		NxU32 operator()(const char *string) const
		{
			// "DJB" string hash 
			NxU32 h = 5381;
			for(const char *ptr = string; *ptr; ptr++)
				h = ((h<<5)+h)^*ptr;
			return h;
		}
		bool operator()(const char* string0, const char* string1) const
		{
			return !strcmp(string0, string1);
		}
	};
}

#endif


#ifndef NV_FOUNDATION_HASHINTERNALS
#define NV_FOUNDATION_HASHINTERNALS


#pragma warning(push)
#pragma warning(disable:4127 4512) // disable the 'conditoinal expression is constant' warning message

namespace CONVEX_DECOMPOSITION
{
	namespace Internal
	{
		template <class Entry,
				  class Key,
				  class HashFn,
				  class GetKey,
				  class Allocator,
				  bool compacting>
		class HashBase
		{
		public:
			typedef Entry EntryType;

			HashBase(NxU32 initialTableSize = 64, float loadFactor = 0.75f):
			mLoadFactor(loadFactor),
				mFreeList((NxU32)EOL),
				mTimestamp(0),
				mSize(0),
				mEntries(Allocator(NV_DEBUG_EXP("hashBaseEntries"))),
				mNext(Allocator(NV_DEBUG_EXP("hashBaseNext"))),
				mHash(Allocator(NV_DEBUG_EXP("hashBaseHash")))
			{
				if(initialTableSize)
					reserveInternal(initialTableSize);
			}

			~HashBase()
			{
				for(NxU32 i = 0;i<mHash.size();i++)
				{				
					for(NxU32 j = mHash[i]; j != EOL; j = mNext[j])
						mEntries[j].~Entry();
				}
			}

			static const int EOL = 0xffffffff;

			NX_INLINE Entry *create(const Key &k, bool &exists)
			{
				NxU32 h=0;
				if(mHash.size())
				{
					h = hash(k);
					NxU32 index = mHash[h];
					while(index!=EOL && !HashFn()(GetKey()(mEntries[index]), k))
						index = mNext[index];
					exists = index!=EOL;
					if(exists)
						return &mEntries[index];
				}

				if(freeListEmpty())
				{
					grow();
					h = hash(k);
				}

				NxU32 entryIndex = freeListGetNext();

				mNext[entryIndex] = mHash[h];
				mHash[h] = entryIndex;

				mSize++;
				mTimestamp++;

				return &mEntries[entryIndex];
			}

			NX_INLINE const Entry *find(const Key &k) const
			{
				if(!mHash.size())
					return false;

				NxU32 h = hash(k);
				NxU32 index = mHash[h];
				while(index!=EOL && !HashFn()(GetKey()(mEntries[index]), k))
					index = mNext[index];
				return index != EOL ? &mEntries[index]:0;
			}

			NX_INLINE bool erase(const Key &k)
			{
				if(!mHash.size())
					return false;

				NxU32 h = hash(k);
				NxU32 *ptr = &mHash[h];
				while(*ptr!=EOL && !HashFn()(GetKey()(mEntries[*ptr]), k))
					ptr = &mNext[*ptr];

				if(*ptr == EOL)
					return false;

				NxU32 index = *ptr;
				*ptr = mNext[index];

				mEntries[index].~Entry();

				mSize--;
				mTimestamp++;

				if(compacting && index!=mSize)
					replaceWithLast(index);

				freeListAdd(index);

				return true;
			}

			NX_INLINE NxU32 size() const
			{ 
				return mSize; 
			}

			void clear()
			{
				if(!mHash.size())
					return;

				for(NxU32 i = 0;i<mHash.size();i++)
					mHash[i] = (NxU32)EOL;
				for(NxU32 i = 0;i<mEntries.size()-1;i++)
					mNext[i] = i+1;
				mNext[mEntries.size()-1] = (NxU32)EOL;
				mFreeList = 0;
				mSize = 0;
			}

			void reserve(NxU32 size)
			{
				if(size>mHash.size())
					reserveInternal(size);
			}

			NX_INLINE const Entry *getEntries() const
			{
				return &mEntries[0];
			}

		private:

			// free list management - if we're coalescing, then we use mFreeList to hold
			// the top of the free list and it should always be equal to size(). Otherwise,
			// we build a free list in the next() pointers.

			NX_INLINE void freeListAdd(NxU32 index)
			{
				if(compacting)
				{
					mFreeList--;
					NX_ASSERT(mFreeList == mSize);
				}
				else
				{
					mNext[index] = mFreeList;
					mFreeList = index;
				}
			}

			NX_INLINE void freeListAdd(NxU32 start, NxU32 end)
			{
				if(!compacting)
				{
					for(NxU32 i = start; i<end-1; i++)	// add the new entries to the free list
						mNext[i] = i+1;
					mNext[end-1] = (NxU32)EOL;
				}
				mFreeList = start;
			}

			NX_INLINE NxU32 freeListGetNext()
			{
				NX_ASSERT(!freeListEmpty());
				if(compacting)
				{
					NX_ASSERT(mFreeList == mSize);
					return mFreeList++;
				}
				else
				{
					NxU32 entryIndex = mFreeList;
					mFreeList = mNext[mFreeList];
					return entryIndex;
				}
			}

			NX_INLINE bool freeListEmpty()
			{
				if(compacting)
					return mSize == mEntries.size();
				else
					return mFreeList == EOL;
			}

			NX_INLINE void replaceWithLast(NxU32 index)
			{
				new(&mEntries[index])Entry(mEntries[mSize]);
				mEntries[mSize].~Entry();
				mNext[index] = mNext[mSize];

				NxU32 h = hash(GetKey()(mEntries[index]));
				NxU32 *ptr;
				for(ptr = &mHash[h]; *ptr!=mSize; ptr = &mNext[*ptr])
					NX_ASSERT(*ptr!=EOL);
				*ptr = index;
			}


			NX_INLINE NxU32 hash(const Key &k) const
			{
				return HashFn()(k)&(mHash.size()-1);
			}

			void reserveInternal(NxU32 size)
			{
				size = nextPowerOfTwo(size);
				// resize the hash and reset
				mHash.resize(size);
				for(NxU32 i=0;i<mHash.size();i++)
					mHash[i] = (NxU32)EOL;

				NX_ASSERT(!(mHash.size()&(mHash.size()-1)));

				NxU32 oldSize = mEntries.size();
				NxU32 newSize = NxU32(float(mHash.size())*mLoadFactor);

				mEntries.resize(newSize);
				mNext.resize(newSize);

				freeListAdd(oldSize,newSize);

				// rehash all the existing entries
				for(NxU32 i=0;i<oldSize;i++)
				{
					NxU32 h = hash(GetKey()(mEntries[i]));
					mNext[i] = mHash[h];
					mHash[h] = i;
				}
			}

			void grow()
			{
				NX_ASSERT(mFreeList == EOL || compacting && mSize == mEntries.size());

				NxU32 size = mHash.size()==0 ? 16 : mHash.size()*2;
				reserve(size);
			}


			Array<Entry, Allocator>	mEntries;
			Array<NxU32, Allocator>	mNext;
			Array<NxU32, Allocator>	mHash;
			float					mLoadFactor;
			NxU32					mFreeList;
			NxU32					mTimestamp;
			NxU32					mSize;

			friend class Iter;

		public:
			class Iter
			{
			public:
				NX_INLINE Iter(HashBase &b): mBase(b), mTimestamp(b.mTimestamp), mBucket(0), mEntry((NxU32)b.EOL)
				{
					if(mBase.mEntries.size()>0)
					{
						mEntry = mBase.mHash[0];
						skip();
					}
				}

				NX_INLINE void check()				{ NX_ASSERT(mTimestamp == mBase.mTimestamp);	}
				NX_INLINE Entry operator*()			{ check(); return mBase.mEntries[mEntry];		}
				NX_INLINE Entry *operator->()		{ check(); return &mBase.mEntries[mEntry];		}
				NX_INLINE Iter operator++()			{ check(); advance(); return *this;				}
				NX_INLINE Iter operator++(int)		{ check(); Iter i = *this; advance(); return i;	}
				NX_INLINE bool done()				{ check(); return mEntry == mBase.EOL;			}

			private:
				NX_INLINE void advance()			{	mEntry = mBase.mNext[mEntry]; skip();		}
				NX_INLINE void skip()
				{
					while(mEntry==mBase.EOL) 
					{ 
						if(++mBucket == mBase.mHash.size())
							break;
						mEntry = mBase.mHash[mBucket];
					}
				}

				NxU32 mBucket;
				NxU32 mEntry;
				NxU32 mTimestamp;
				HashBase &mBase;
			};
		};

		template <class Key, 
				  class HashFn, 
				  class Allocator = Allocator,
				  bool Coalesced = false>
		class HashSetBase
		{ 
		public:
			struct GetKey { NX_INLINE const Key &operator()(const Key &e) {	return e; }	};

			typedef HashBase<Key, Key, HashFn, GetKey, Allocator, Coalesced> BaseMap;
			typedef typename BaseMap::Iter Iterator;

			HashSetBase(NxU32 initialTableSize = 64, 
						float loadFactor = 0.75f):	mBase(initialTableSize,loadFactor)	{}

			bool insert(const Key &k)
			{
				bool exists;
				Key *e = mBase.create(k,exists);
				if(!exists)
					new(e)Key(k);
				return !exists;
			}

			NX_INLINE bool		contains(const Key &k)	const	{	return mBase.find(k)!=0;		}
			NX_INLINE bool		erase(const Key &k)				{	return mBase.erase(k);			}
			NX_INLINE NxU32		size()					const	{	return mBase.size();			}
			NX_INLINE void		reserve(NxU32 size)				{	mBase.reserve(size);			}
			NX_INLINE void		clear()							{	mBase.clear();					}
		protected:
			BaseMap mBase;

		};

		template <class Key, 
			  class Value,
			  class HashFn, 
			  class Allocator = Allocator >

		class HashMapBase
		{ 
		public:
			typedef Pair<const Key,Value> Entry;
			struct GetKey { NX_INLINE const Key &operator()(const Entry &e) {	return e.first; }	};
			typedef HashBase<Pair<const Key,Value>, Key, HashFn, GetKey, Allocator, true> BaseMap;
			typedef typename BaseMap::Iter Iterator;

			HashMapBase(NxU32 initialTableSize = 64, float loadFactor = 0.75f):	mBase(initialTableSize,loadFactor)	{}

			bool insert(const Key &k, const Value &v)
			{
				bool exists;
				Entry *e = mBase.create(k,exists);
				if(!exists)
					new(e)Entry(k,v);
				return !exists;
			}

			Value &operator [](const Key &k)
			{
				bool exists;
				Entry *e = mBase.create(k, exists);
				if(!exists)
					new(e)Entry(k,Value());
		
				return e->second;
			}

			NX_INLINE const Entry *	find(const Key &k)		const	{	return mBase.find(k);			}
			NX_INLINE bool			erase(const Key &k)				{	return mBase.erase(k);			}
			NX_INLINE NxU32			size()					const	{	return mBase.size();			}
			NX_INLINE Iterator		getIterator()					{	return Iterator(mBase);			}
			NX_INLINE void			reserve(NxU32 size)				{	mBase.reserve(size);			}
			NX_INLINE void			clear()							{	mBase.clear();					}

		protected:
			BaseMap mBase;
		};

	}
}

#pragma warning(pop)

#endif

#ifndef NV_FOUNDATION_HASHMAP
#define NV_FOUNDATION_HASHMAP


// TODO: make this doxy-format
//
// This header defines two hash maps. Hash maps
// * support custom initial table sizes (rounded up internally to power-of-2)
// * support custom static allocator objects
// * auto-resize, based on a load factor (i.e. a 64-entry .75 load factor hash will resize 
//                                        when the 49th element is inserted)
// * are based on open hashing
// * have O(1) contains, erase
//
// Maps have STL-like copying semantics, and properly initialize and destruct copies of objects
// 
// There are two forms of map: coalesced and uncoalesced. Coalesced maps keep the entries in the
// initial segment of an array, so are fast to iterate over; however deletion is approximately
// twice as expensive.
//
// HashMap<T>:
//		bool			insert(const Key &k, const Value &v)	O(1) amortized (exponential resize policy)
//		Value &			operator[](const Key &k)				O(1) for existing objects, else O(1) amortized
//		const Entry *	find(const Key &k);						O(1)
//		bool			erase(const T &k);						O(1)
//		NxU32			size();									constant
//		void			reserve(NxU32 size);					O(MAX(currentOccupancy,size))
//		void			clear();								O(currentOccupancy) (with zero constant for objects without destructors) 
//      Iterator		getIterator();
//
// operator[] creates an entry if one does not exist, initializing with the default constructor.
// CoalescedHashMap<T> does not support getInterator, but instead supports
// 		const Key *getEntries();
//
// Use of iterators:
// 
// for(HashMap::Iterator iter = test.getIterator(); !iter.done(); ++iter)
//			myFunction(iter->first, iter->second);

namespace CONVEX_DECOMPOSITION
{
	template <class Key,
			  class Value,
			  class HashFn = Hash<Key>,
			  class Allocator = Allocator >
	class HashMap: public Internal::HashMapBase<Key, Value, HashFn, Allocator>
	{
	public:

		typedef Internal::HashMapBase<Key, Value, HashFn, Allocator> HashMapBase;
		typedef typename HashMapBase::Iterator Iterator;

		HashMap(NxU32 initialTableSize = 64, float loadFactor = 0.75f):	HashMapBase(initialTableSize,loadFactor) {}
		Iterator getIterator() { return Iterator(HashMapBase::mBase); }
	};

	template <class Key, 
			  class Value,
			  class HashFn = Hash<Key>, 
			  class Allocator = Allocator >
	class CoalescedHashMap: public Internal::HashMapBase<Key, Value, HashFn, Allocator>
	{
		typedef Internal::HashMapBase<Key, Value, HashFn, Allocator> HashMapBase;

		CoalescedHashMap(NxU32 initialTableSize = 64, float loadFactor = 0.75f): HashMapBase(initialTableSize,loadFactor){}
		const Key *getEntries() const { return HashMapBase::mBase.getEntries(); }
	};

}
#endif

#endif
