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

#ifndef _THREADSAFEDEQUE_H_
#define _THREADSAFEDEQUE_H_

#ifndef _THREADSAFEFREELIST_H_
#  include "platform/threads/threadSafeFreeList.h"
#endif

#include "platform/tmm_off.h"


/// Fast, lock-free double-ended queue for concurrent access.
///
/// @param T Type of list elements; must have default contructor.
template< typename T >
class ThreadSafeDeque
{
   // Lock-free deques using just single-word atomic writes are
   // very tricky as each pointer update must immediately result
   // in a fully valid list state.  The idea here is to maintain the
   // deque's head and tail pointers unreliably but otherwise keep a
   // regular double-linked list (since we never insert nodes in the
   // middle, single-word writes are all we need).
   //
   // Deletions are a bit less straightforward and require the threads
   // to work cooperatively.  Since failure of a pointer update depends
   // on the deletion state, the deletion flag has to be encoded into
   // the link fields.  However, as there are two link fields this creates
   // two independent deletion flags for each single node, one on the
   // next link and one on the prev link.
   //
   // This will not lead to a problem, though, as it only becomes relevant
   // when there is only a single value in the list which, even if the
   // respective node gets both deleted and appended/prepended a new node,
   // will result in a valid list state.


   public:

      typedef T ValueType;

   protected:

      class Node;
      class DeleteNode;
      typedef ThreadSafeRef< Node > NodeRef;

      /// List node.
      class Node : public ThreadSafeFreeListNode< Node, DeleteNode >
      {
         public:

            friend class DeleteNode; // mFreeList;
            typedef typename ThreadSafeDeque< T >::ValueType ValueType;

            /// Thread claim flag.  This is to prevent two threads who concurrently
            /// do a tryPopFront() and tryPopBack() respectively on a deque with just
            /// a single node to both claim and return the same value (which would happen
            /// without the flag as otherwise both threads would use two different
            /// deletion bits for claiming the node).
            U32 mIsClaimed;

            /// Link to the freelist that the node has been
            /// allocated from.
            ThreadSafeFreeList< Node >& mFreeList;

            /// Value contained in the node.
            ValueType mValue;

            /// Reference to next node and deletion bit.
            NodeRef mNext;

            /// Reference to previous node and deletion bit.
            NodeRef mPrev;

            /// Construct an unlinked node allocated from "freeList".
            Node( ThreadSafeFreeList< Node >& freeList, const ValueType& value )
               : mIsClaimed( 0 ), mFreeList( freeList ), mValue( value ) {}
      };

      class DeleteNode
      {
         public:
            template< typename N >
            static void destroy( N* ptr )
            {
               AssertFatal( ptr->mIsClaimed,
                  "ThreadSafeDeque::DeleteNode::destroy() - deleting unclaimed node" );
               destructInPlace( ptr );
               ptr->mFreeList.free( ptr );
            }
      };

      #ifdef TORQUE_DEBUG
      S32 mNumValues;
      #endif

      /// Reference to the head node.
      NodeRef mHead;

      ///
      NodeRef mTail;

      /// Free list for list nodes.
      ThreadSafeFreeList< Node > mFreeList;

      /// @return the leftmost node in the list.
      /// @note Updates the list state and may purge deleted nodes.
      NodeRef getHead();

      /// @return the rightmost node in the list.
      /// @note Updates the list state and may purge deleted nodes.
      NodeRef getTail();

   public:

      /// Construct an empty deque.
      ThreadSafeDeque()
      {
         #ifdef TORQUE_DEBUG
         mNumValues = 0;
         #endif
      }

      ~ThreadSafeDeque()
      {
         ValueType value;
         while( tryPopFront( value ) );
         AssertFatal( isEmpty(), "ThreadSafeDeque::~ThreadSafeDeque() - not empty" );
      }

      /// @return true if the queue is empty.
      bool isEmpty()
      {
         return ( !getHead() && !getTail() );
      }

      /// Prepend the given value to the list.
      void pushFront( const ValueType& value );

      /// Append the given value to the list.
      void pushBack( const ValueType& value );

      /// Try to take the leftmost value from the deque.
      /// Fails if the deque is empty at the time the method tries to
      /// take a node from the list.
      bool tryPopFront( ValueType& outValue );

      /// Try to take the rightmost value from the deque.
      /// Fails if the deque is empty at the time the method tries to
      /// take a node from the list.
      bool tryPopBack( ValueType& outValue );

      void dumpDebug()
      {
         #ifdef TORQUE_DEBUG
         Platform::outputDebugString( "[ThreadSafeDeque] numValues=%i", mNumValues );
         mFreeList.dumpDebug();
         #endif
      }
};

// The getHead() and getTail() code here is pretty much brute-force in order
// to keep the complexities of synchronizing it bounded.  We just let each
// thread work as if it is the only thread but require each one to start from
// scratch on each iteration.

template< typename T >
typename ThreadSafeDeque< T >::NodeRef ThreadSafeDeque< T >::getHead()
{
   // Find leftmost node.

   NodeRef result;
   while( 1 )
   {
      // Iterate through to leftmost node.
      
      {
         NodeRef head = mHead;
         while( head != NULL )
         {
            NodeRef prev = head->mPrev;
            if( prev != NULL )
               mHead.trySetFromTo( head, prev, NodeRef::TAG_Unset );
            else
               break;
               
            head = mHead;
         }
      }
      
      // Clear out dead nodes at front of list.
      
      {
         NodeRef head = mHead;
         if( head && head->mPrev.isTagged() )
         {
            NodeRef next = head->mNext;
            
            mHead.trySetFromTo( head, next, NodeRef::TAG_Unset );
            mTail.trySetFromTo( head, next, NodeRef::TAG_Unset );

            if( next != NULL )
               next->mPrev.trySetFromTo( head, NULL );

            head->mNext.trySetFromTo( next, NULL, NodeRef::TAG_Set );
            
            continue; // Restart.
         }
      }
      
      // Try head.
      
      NodeRef head = mHead;
      if( head != NULL && !head->mPrev.isTagged() )
      {
         result = head;
         break;
      }
         
      // Try tail.
      
      if( !head )
      {
         head = mTail;
         if( !head )
            break;
      }

      // Update head.
      
      NodeRef prev = head->mPrev;
      if( head->mPrev != NULL )
      {
         if( !mHead.trySetFromTo( head, prev, NodeRef::TAG_Unset ) )
            mHead.trySetFromTo( NULL, prev );
      }
      else
         mHead.trySetFromTo( NULL, head );
   }

   AssertFatal( !result.isTagged(), "ThreadSafeDeque::getHead() - head got tagged" );
   return result;
}

template< typename T >
typename ThreadSafeDeque< T >::NodeRef ThreadSafeDeque< T >::getTail()
{
   // Find rightmost node.

   NodeRef result;
   while( 1 )
   {
      // Iterate through to rightmost node.
      
      {
         NodeRef tail = mTail;
         while( tail != NULL )
         {
            NodeRef next = tail->mNext;
            if( next != NULL )
               mTail.trySetFromTo( tail, next, NodeRef::TAG_Unset );
            else
               break;
               
            tail = mTail;
         }
      }
      
      // Clear out dead nodes at tail of list.
      
      {
         NodeRef tail = mTail;
         if( tail != NULL && tail->mNext.isTagged() )
         {
            NodeRef prev = tail->mPrev;
            
            mHead.trySetFromTo( tail, prev, NodeRef::TAG_Unset );
            mTail.trySetFromTo( tail, prev, NodeRef::TAG_Unset );

            if( prev != NULL )
               prev->mNext.trySetFromTo( tail, NULL );

            tail->mPrev.trySetFromTo( prev, NULL, NodeRef::TAG_Set );
            
            continue; // Restart.
         }
      }
      
      // Try tail.
      
      NodeRef tail = mTail;
      if( tail != NULL && !tail->mNext.isTagged() )
      {
         result = tail;
         break;
      }
         
      // Try head.
      
      if( !tail )
      {
         tail = mHead;
         if( !tail )
            break;
      }

      // Update tail.
      
      NodeRef next = tail->mNext;
      if( next != NULL )
      {
         if( !mTail.trySetFromTo( tail, next, NodeRef::TAG_Unset ) )
            mTail.trySetFromTo( NULL, next );
      }
      else
         mTail.trySetFromTo( NULL, tail );
   }

   AssertFatal( !result.isTagged(), "ThreadSafeDeque::getTail() - tail got tagged" );
   return result;
}

template< typename T >
void ThreadSafeDeque< T >::pushFront( const ValueType& value )
{
   NodeRef nextNode;
   NodeRef newNode;

   NodeRef::unsafeWrite( newNode, new ( mFreeList ) Node( mFreeList, value ) );

   while( 1 )
   {
      nextNode = getHead();
      if( !nextNode )
      {
         newNode->mNext = NULL;
         if( mHead.trySetFromTo( NULL, newNode ) )
            break;
      }
      else
      {
         newNode->mNext = nextNode;
         if( nextNode->mPrev.trySetFromTo( NULL, newNode, NodeRef::TAG_FailIfSet ) )
            break;
      }
   }

#ifdef TORQUE_DEBUG
   dFetchAndAdd( mNumValues, 1 );
#endif
}

template< typename T >
void ThreadSafeDeque< T >::pushBack( const ValueType& value )
{
   NodeRef prevNode;
   NodeRef newNode;

   NodeRef::unsafeWrite( newNode, new ( mFreeList ) Node( mFreeList, value ) );

   while( 1 )
   {
      prevNode = getTail();
      if( !prevNode )
      {
         newNode->mPrev = NULL;
         if( mHead.trySetFromTo( NULL, newNode ) ) // use head so we synchronize with pushFront
            break;
      }
      else
      {
         newNode->mPrev = prevNode;
         if( prevNode->mNext.trySetFromTo( NULL, newNode, NodeRef::TAG_FailIfSet ) )
            break;
      }
   }

#ifdef TORQUE_DEBUG
   dFetchAndAdd( mNumValues, 1 );
#endif
}

template< typename T >
bool ThreadSafeDeque< T >::tryPopFront( ValueType& outValue )
{
   NodeRef oldHead;

   while( 1 )
   {
      oldHead = getHead();
      if( !oldHead )
         return false;

      // Try to claim the node.

      if( oldHead->mPrev.trySetFromTo( NULL, NULL, NodeRef::TAG_SetOrFail ) )
      {
         if( dCompareAndSwap( oldHead->mIsClaimed, 0, 1 ) )
            break;
         else
            continue;
      }
   }

   outValue = oldHead->mValue;
   oldHead = NULL;

   // Cleanup.
   getHead();

#ifdef TORQUE_DEBUG
   dFetchAndAdd( mNumValues, -1 );
#endif
   return true;
}

template< typename T >
bool ThreadSafeDeque< T >::tryPopBack( ValueType& outValue )
{
   NodeRef oldTail;

   while( 1 )
   {
      oldTail = getTail();
      if( !oldTail )
         return false;

      // Try to claim the node.

      if( oldTail->mNext.trySetFromTo( NULL, NULL, NodeRef::TAG_SetOrFail ) )
      {
         if( dCompareAndSwap( oldTail->mIsClaimed, 0, 1 ) )
            break;
      }
   }

   outValue = oldTail->mValue;
   oldTail = NULL;

   // Cleanup.
   getTail();

#ifdef TORQUE_DEBUG
   dFetchAndAdd( mNumValues, -1 );
#endif
   return true;
}


#include "platform/tmm_on.h"

#endif // _THREADSAFEDEQUE_H_
