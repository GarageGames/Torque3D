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

#ifndef _THREADSAFEFREELIST_H_
#define _THREADSAFEFREELIST_H_

#ifndef _THREADSAFEREFCOUNT_H_
#  include "platform/threads/threadSafeRefCount.h"
#endif
#ifndef _PLATFORMINTRINSICS_H_
#  include "platform/platformIntrinsics.h"
#endif

#include "platform/tmm_off.h"


/// @file
/// Lock-free freelists for concurrent access.


/// Freelist for re-using allocations in a concurrent setting.
///
/// @note Make sure that there are no more allocations in use
///   when the free list is destructed.
/// @note Allocated instances come with a reference already counted
///   on the instance.
///
/// @param T Type of elements to allocate; must be derived from
///   ThreadSafeRefCount and have at least define one additional
///   pointer-sized field.
template< class T >
class ThreadSafeFreeList
{
   protected:

      T* mFreeList;

      #ifdef TORQUE_DEBUG
      S32 mNumNodesTotal;
      S32 mNumNodesFree;
      #endif

      T*& getNext( T* ptr )
      {
         return *( ( T** ) &( ( U8* ) ptr )[ sizeof( T ) - sizeof( T* ) ] );
      }

   public:

      /// Create the freelist.
      ///
      /// @param numPreAlloc Number of instances to pre-allocate.
      ThreadSafeFreeList( U32 numPreAlloc = 0 )
         : mFreeList( 0 )
      {
         #ifdef TORQUE_DEBUG
         mNumNodesTotal = 0;
         mNumNodesFree = 0;
         #endif

         for( U32 i = 0; i < numPreAlloc; ++ i )
            free( alloc() );
      }

      ~ThreadSafeFreeList()
      {
         #ifdef TORQUE_DEBUG
         AssertWarn( mNumNodesTotal == mNumNodesFree,
            "ThreadSafeFreeList::~ThreadSafeFreeList() - still got live instances" );
         #endif

         // Destroy remaining nodes.  Not synchronized.  We assume all
         // concurrent processing to have finished.

         while( mFreeList )
         {
            T* next = getNext( mFreeList );
            dFree( mFreeList );
            mFreeList = next;
         }
      }

      /// Return memory for a new instance.
      void* alloc()
      {
         T* ptr;
         while( 1 )
         {
            ptr = ThreadSafeRef< T >::safeRead( mFreeList );
            if( !ptr )
            {
               ptr = ( T* ) dMalloc( sizeof( T ) );
               dMemset( ptr, 0, sizeof( T ) );

               #ifdef TORQUE_DEBUG
               dFetchAndAdd( mNumNodesTotal, 1 );
               #endif

               ptr->addRef();
               break;
            }
            else if( dCompareAndSwap( mFreeList, ptr, getNext( ptr ) ) )
            {
               #ifdef TORQUE_DEBUG
               dFetchAndAdd( mNumNodesFree, -1 );
               #endif

               ptr->clearLowestBit();
               break;
            }
            else
               ptr->release();
         }

         return ptr;
      }

      /// Return the memory allocated to the given instance to the freelist.
      void free( void* ptr )
      {
         AssertFatal( ptr, "ThreadSafeFreeList::free() - got a NULL pointer" );
         T* node = ( T* ) ptr;

         while( 1 )
         {
            T* list = mFreeList;
            getNext( node ) = list;
            if( dCompareAndSwap( mFreeList, list, node ) )
               break;
         }

         #ifdef TORQUE_DEBUG
         dFetchAndAdd( mNumNodesFree, 1 );
         #endif
      }

      void dumpDebug()
      {
         #ifdef TORQUE_DEBUG
         Platform::outputDebugString( "[ThreadSafeFreeList] total=%i, free=%i",
            mNumNodesTotal, mNumNodesFree );
         #endif
      }
};

/// Baseclass for objects allocated from ThreadSafeFreeLists.
template< class T, class DeletePolicy = DeleteSingle >
class ThreadSafeFreeListNode : public ThreadSafeRefCount< T, DeletePolicy >
{
   public:

      typedef ThreadSafeRefCount< T, DeletePolicy > Parent;

      ThreadSafeFreeListNode()
         : Parent( false ) {}

      static void* operator new( size_t size, ThreadSafeFreeList< T >& freeList )
      {
         AssertFatal( size <= sizeof( T ),
            "ThreadSafeFreeListNode::new() - size exceeds limit of freelist" );
         TORQUE_UNUSED( size );
         return freeList.alloc();
      }
      static void operator delete( void* ptr, ThreadSafeFreeList< T >& freeList )
      {
         freeList.free( ptr );
      }
};


#include "platform/tmm_on.h"

#endif // _THREADSAFEFREELIST_H_
