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

#ifndef _THREADSAFEPRIORITYQUEUE_H_
#define _THREADSAFEPRIORITYQUEUE_H_

#ifndef _PLATFORMINTRINSICS_H_
   #include "platform/platformIntrinsics.h"
#endif
#ifndef _THREADSAFEREFCOUNT_H_
   #include "platform/threads/threadSafeRefCount.h"
#endif
#ifndef _TYPETRAITS_H_
   #include "platform/typetraits.h"
#endif


// Disable TMM's new operator grabbing.
#include "platform/tmm_off.h"


//#define DEBUG_SPEW


/// @file
/// Template code for an efficient thread-safe priority queue
/// implementation.  There are two alternative implementations to
/// choose from: ThreadSafePriorityQueue and ThreadSafePriorityQueueWithUpdate
/// where the latter adds concurrent status updates of queue items to
/// the former implementation.


//--------------------------------------------------------------------------
//    ThreadSafePriorityQueue.
//--------------------------------------------------------------------------

/// Fast, lock-free priority queue implementation for concurrent access.
///
/// Equal priorities are allowed and are placed <em>before</em> existing items of
/// identical priority in the queue.
///
/// Based on (but with significant deviations from) "Fast and Lock-Free Concurrent
/// Priority Queues for Multi-Thread Systems" by Hakan Sundell and Philippas Tsigas.
/// Parts of the skiplist code is based on work by William Pugh.
///
/// @param T The item value type.  Must have a default constructor.
/// @param K The priority key type.  Must be comparable, have a default constructor,
///     and be a valid template parameter to TypeTraits.
/// @param SORT_MIN_TO_MAX If true, the queue sorts from minimum to maximum priority or
///     the reverse if false.
/// @param MAX_LEVEL The number of levels a node can have at most.
/// @param PROBABILISTIC_BIAS The probabilistic level distribution factor for
///     the skiplist.  Multiplied by 100 and turned into int to conform to restrictions
///     on non-type template parameters.
///
/// @see TypeTraits

template< typename T, typename K = F32, bool SORT_MIN_TO_MAX = false, U32 MAX_LEVEL = 4, U32 PROBABILISTIC_BIAS = 50 >
struct ThreadSafePriorityQueue
{
   typedef T ValueType;
   typedef K KeyType;

   enum { MAX_LEVEL_CONST = MAX_LEVEL };

   ThreadSafePriorityQueue();

   bool              isEmpty();
   void              insert( KeyType priority, const T& value );
   bool              takeNext( T& outValue, KeyType upToPriority = ( SORT_MIN_TO_MAX ? TypeTraits< KeyType >::MAX : TypeTraits< KeyType >::MIN ) );

protected:
   struct Node;
   typedef ThreadSafeRef< Node > NodePtr;
   friend class ThreadSafeRefCount< Node >;
   friend struct DeleteSingle;
   
   /// A queue node.
   ///
   /// Nodes are reference-counted to coordinate memory management
   /// between the different threads.  Reclamation happens on the
   /// thread that releases the last reference.
   ///
   /// Reference-counting and deletion requests are kept separate.
   /// A given node is marked for deletion and will then have its references
   /// progressively disappear and eventually be reclaimed once the
   /// reference count drops to zero.  
   ///
   /// Note that 'Next' references are released by the destructor which
   /// is only called when the reference count to the node itself drops to
   /// zero.  This is to avoid threads getting trapped in a node with no
   /// link out.

   struct Node : public ThreadSafeRefCount< Node >
   {
      typedef ThreadSafeRefCount< Node > Parent;

      Node( KeyType priority, const ValueType& value );
      ~Node();

      KeyType        getPriority()                 { return mPriority; }
      ValueType&     getValue()                    { return mValue; }
      U32            getLevel();
      NodePtr&       getNext( U32 level );

      bool           isMarkedForDeletion();
      bool           tryMarkForDeletion();
      
      void           clearValue()                  { mValue = ValueType(); }

      static U32     randomLevel();

      void*          operator new( size_t size, S32 level = -1 );
      void           operator delete( void* ptr );

   private:
      KeyType        mPriority;     ///< Priority key.
      U32            mLevel;        ///< Level count and deletion bit (highest).
      ValueType      mValue;
      Node*          mNext[ 1 ];    ///< Variable-sized array of next pointers.

      struct FreeList
      {
         bool        mDestroyed;
         Node*       mNodes;

         ~FreeList();
      };

      static FreeList smFreeLists[ MAX_LEVEL ];
   };

   NodePtr           mHead;         ///< Artificial head node.
   NodePtr           mTail;         ///< Artificial tail node.

   void              readNext( NodePtr& refPrev, NodePtr& refNext, U32 level );
   void              scan( NodePtr& refPrev, NodePtr& refNext, U32 level, KeyType priority );
   void              scanFromHead( NodePtr& refPrev, NodePtr& refNext, U32 level, KeyType priority );
   void              insert( KeyType priority, const T& value, NodePtr& outResult );
   void              helpDelete();
};

template< typename T, typename K, bool SORT_MIN_TO_MAX, U32 MAX_LEVEL, U32 PROBABILISTIC_BIAS >
typename ThreadSafePriorityQueue< T, K, SORT_MIN_TO_MAX, MAX_LEVEL, PROBABILISTIC_BIAS >::Node::FreeList ThreadSafePriorityQueue< T, K, SORT_MIN_TO_MAX, MAX_LEVEL, PROBABILISTIC_BIAS >::Node::smFreeLists[ MAX_LEVEL ];

/// Construct an empty queue.
///
/// Internally, this creates a head node with maximal priority and a tail node with minimal priority,
/// both at maximum level.

template< typename T, typename K, bool SORT_MIN_TO_MAX, U32 MAX_LEVEL, U32 PROBABILISTIC_BIAS >
ThreadSafePriorityQueue< T, K, SORT_MIN_TO_MAX, MAX_LEVEL, PROBABILISTIC_BIAS >::ThreadSafePriorityQueue()
{
   NodePtr::unsafeWrite( mHead, new ( MAX_LEVEL - 1 )
      Node( SORT_MIN_TO_MAX ? TypeTraits< KeyType >::MIN : TypeTraits< KeyType >::MAX, ValueType() ) );
   NodePtr::unsafeWrite( mTail, new ( MAX_LEVEL - 1 )
      Node( SORT_MIN_TO_MAX ? TypeTraits< KeyType >::MAX : TypeTraits< KeyType >::MIN, ValueType() ) );

   for( U32 level = 0; level < MAX_LEVEL; level ++ )
      mHead->getNext( level ) = mTail;
}

/// Return true if the queue does not currently contain an item.

template< typename T, typename K, bool SORT_MIN_TO_MAX, U32 MAX_LEVEL, U32 PROBABILISTIC_BIAS >
bool ThreadSafePriorityQueue< T, K, SORT_MIN_TO_MAX, MAX_LEVEL, PROBABILISTIC_BIAS >::isEmpty()
{
   return ( mHead->getNext( 0 ) == mTail );
}

/// Insert the given value into the queue at the place determined by the given priority.

template< typename T, typename K, bool SORT_MIN_TO_MAX, U32 MAX_LEVEL, U32 PROBABILISTIC_BIAS >
inline void ThreadSafePriorityQueue< T, K, SORT_MIN_TO_MAX, MAX_LEVEL, PROBABILISTIC_BIAS >::insert( KeyType priority, const ValueType& value )
{
   NodePtr result;
   insert( priority, value, result );
}

template< typename T, typename K, bool SORT_MIN_TO_MAX, U32 MAX_LEVEL, U32 PROBABILISTIC_BIAS >
void ThreadSafePriorityQueue< T, K, SORT_MIN_TO_MAX, MAX_LEVEL, PROBABILISTIC_BIAS >::insert( KeyType priority, const ValueType& value, NodePtr& outResult )
{
   // Create a new node at a random level.

   outResult = NULL;
   NodePtr::unsafeWrite( outResult, new Node( priority, value ) );
   U32 resultNodeLevel = outResult->getLevel();

   // Link up all the levels.  Do this bottom-up instead of
   // top-down (as would be the right way for a skiplist) so
   // that our list state always remains valid.  If going top-down,
   // we'll insert nodes with NULL pointers at their lower levels.

   U32 currentLevel = 0;
   do 
   {
      while( 1 )
      {
         NodePtr nextNode;
         NodePtr prevNode;
         
         scanFromHead( prevNode, nextNode, currentLevel, priority );        

         outResult->getNext( currentLevel ) = nextNode;
         if( prevNode->getNext( currentLevel ).trySetFromTo( nextNode, outResult, NodePtr::TAG_FailIfSet ) )
            break;
         else
            outResult->getNext( currentLevel ) = 0;
      }

      currentLevel ++;
   }
   while(    currentLevel <= resultNodeLevel
          && !outResult->isMarkedForDeletion() ); // No point linking up remaining levels if another thread already took this node.
}

/// Take the item with the highest priority from the queue.
///
/// @param outValue Reference to where the resulting value should be stored.
/// @param upToPriority Priority limit (inclusive) up to which items are taken from the queue.
/// @return true if there was a matching item in the queue.

template< typename T, typename K, bool SORT_MIN_TO_MAX, U32 MAX_LEVEL, U32 PROBABILISTIC_BIAS >
bool ThreadSafePriorityQueue< T, K, SORT_MIN_TO_MAX, MAX_LEVEL, PROBABILISTIC_BIAS >::takeNext( T& outValue, KeyType upToPriority )
{
   // Iterate through to the first unmarked node.

   NodePtr prevNode = mHead;
   while( 1 )
   {
      NodePtr node;
      readNext( prevNode, node, 0 );

      if( node == mTail )
         return false; // End reached.

      bool priorityThresholdReached = SORT_MIN_TO_MAX
         ? ( upToPriority >= node->getPriority() )
         : ( upToPriority <= node->getPriority() );

      if( !priorityThresholdReached )
         return false;
      else
      {
         // Try to mark the node for deletion.  Only if that succeeds, taking the
         // node was a success and we can return.  If it fails, spin and try again.

         if( node->tryMarkForDeletion() )
         {
            helpDelete();

            // Node is now off the list and will disappear as soon as
            // all references held by threads (including this one)
            // go out of scope.

            outValue = node->getValue();
            node->clearValue();

            return true;
         }
      }
   }
}

/// Update the given references to the next non-deleted node at the given level.
/// refPrev will be updated to reference the immediate predecessor of the next
/// node returned.  Note that this can be a node in deleted state.
///
/// @param refPrev Reference to a node of which the successor node should be
///    returned.  Updated to immediate predecessor of refNext on return.
/// @param refNext Reference to update to refer to next non-deleted node on
///    the given level.
/// @param level Skiplist level to operate on.

template< typename T, typename K, bool SORT_MIN_TO_MAX, U32 MAX_LEVEL, U32 PROBABILISTIC_BIAS >
inline void ThreadSafePriorityQueue< T, K, SORT_MIN_TO_MAX, MAX_LEVEL, PROBABILISTIC_BIAS >::readNext( NodePtr& refPrev, NodePtr& refNext, U32 level )
{
   while( 1 )
   {
      refNext = refPrev->getNext( level );
      AssertFatal( refNext != NULL, "ThreadSafePriorityQueue::readNext() - next is NULL" );
      if( !refNext->isMarkedForDeletion() || refNext == mTail )
         break;

      refPrev = refNext;
   }
}

/// Scan for the position at which to insert a node of the given priority.
/// Upon return, the position between refPrev and refNext is the one to insert at.
///
/// @param refPrev position at which to start scanning; updated to match insert position.
/// @param refNext

template< typename T, typename K, bool SORT_MIN_TO_MAX, U32 MAX_LEVEL, U32 PROBABILISTIC_BIAS >
void ThreadSafePriorityQueue< T, K, SORT_MIN_TO_MAX, MAX_LEVEL, PROBABILISTIC_BIAS >::scan( NodePtr& refPrev, NodePtr& refNext, U32 level, KeyType priority )
{
   while( 1 )
   {
      readNext( refPrev, refNext, level );
      if( refNext == mTail
         || ( SORT_MIN_TO_MAX
            ? ( refNext->getPriority() > priority )
            : ( refNext->getPriority() < priority ) ) )
         break;

      refPrev = refNext;
   }
}

///

template< typename T, typename K, bool SORT_MIN_TO_MAX, U32 MAX_LEVEL, U32 PROBABILISTIC_BIAS >
void ThreadSafePriorityQueue< T, K, SORT_MIN_TO_MAX, MAX_LEVEL, PROBABILISTIC_BIAS >::scanFromHead( NodePtr& refPrev, NodePtr& refNext, U32 level, KeyType priority )
{
   // Purge dead nodes at left end of queue so
   // we don't get stuck hitting the same node
   // in deletable state over and over again.
   helpDelete();

   S32 currentLevel = MAX_LEVEL - 1;
   refPrev = mHead;
   do
   {
      scan( refPrev, refNext, currentLevel, priority );
      currentLevel --;
   }
   while( currentLevel >= S32( level ) );
}

template< typename T, typename K, bool SORT_MIN_TO_MAX, U32 MAX_LEVEL, U32 PROBABILISTIC_BIAS >
void ThreadSafePriorityQueue< T, K, SORT_MIN_TO_MAX, MAX_LEVEL, PROBABILISTIC_BIAS >::helpDelete()
{
   // Clean out all the references from head.
   // Spin over a given reference on each level until head
   // clearly refers to a node in non-deletable state.  This
   // makes this code work cooperatively with other threads
   // doing takeNexts on prior or later nodes while also
   // guaranteeing that all next pointers to us will eventually
   // disappear.
   //
   // Note that this is *the only place* where we will be cleaning
   // out our lists.

   S32 level = MAX_LEVEL - 1;
   do
   {
      while( 1 )
      {
         NodePtr ptr = mHead->getNext( level );
         if( !ptr->isMarkedForDeletion() )
            break;
         else
         {
            NodePtr& next = ptr->getNext( level );
            next.setTag();
            mHead->getNext( level ).trySetFromTo( ptr, next, NodePtr::TAG_Unset );
            AssertFatal( next->getRefCount() >= 2, "ThreadSafePriorityQueue::helpDelete() - invalid refcount" );
         }
      }

      level --;
   }
   while( level >= 0 );
}

template< typename T, typename K, bool SORT_MIN_TO_MAX, U32 MAX_LEVEL, U32 PROBABILISTIC_BIAS >
inline ThreadSafePriorityQueue< T, K, SORT_MIN_TO_MAX, MAX_LEVEL, PROBABILISTIC_BIAS >::Node::Node( KeyType priority, const ValueType& value )
   : Parent( false ),
     mPriority( priority ),
     mValue( value )
{
   dMemset( mNext, 0, sizeof( Node* ) * ( getLevel() + 1 ) );

   // Level is already set by the allocation routines.
}

template< typename T, typename K, bool SORT_MIN_TO_MAX, U32 MAX_LEVEL, U32 PROBABILISTIC_BIAS >
ThreadSafePriorityQueue< T, K, SORT_MIN_TO_MAX, MAX_LEVEL, PROBABILISTIC_BIAS >::Node::~Node()
{
   for( U32 level = 0; level < ( getLevel() + 1 ); level ++ )
      getNext( level ) = NULL;
}

/// Return the skip list level the node is at.

template< typename T, typename K, bool SORT_MIN_TO_MAX, U32 MAX_LEVEL, U32 PROBABILISTIC_BIAS >
inline U32 ThreadSafePriorityQueue< T, K, SORT_MIN_TO_MAX, MAX_LEVEL, PROBABILISTIC_BIAS >::Node::getLevel()
{
   // Mask out the deletion request bit.

   return ( mLevel & 0x7FFFFFFF );
}

/// Return the successor node at the given level.
/// @param level The level of the desired successor node; must be within the node's level bounds.

template< typename T, typename K, bool SORT_MIN_TO_MAX, U32 MAX_LEVEL, U32 PROBABILISTIC_BIAS >
inline typename ThreadSafePriorityQueue< T, K, SORT_MIN_TO_MAX, MAX_LEVEL, PROBABILISTIC_BIAS >::NodePtr& ThreadSafePriorityQueue< T, K, SORT_MIN_TO_MAX, MAX_LEVEL, PROBABILISTIC_BIAS >::Node::getNext( U32 level )
{
   return *reinterpret_cast< NodePtr* >( &mNext[ level ] );
}

/// Return true if the node is marked to be deleted.

template< typename T, typename K, bool SORT_MIN_TO_MAX, U32 MAX_LEVEL, U32 PROBABILISTIC_BIAS >
inline bool ThreadSafePriorityQueue< T, K, SORT_MIN_TO_MAX, MAX_LEVEL, PROBABILISTIC_BIAS >::Node::isMarkedForDeletion()
{
   return ( mLevel & 0x80000000 );
}

/// Attempt to mark the node for deletion.  If the mark bit has not yet been set
/// and setting it on the current thread succeeds, returns true.
///
/// @return true, if the marking succeeded.

template< typename T, typename K, bool SORT_MIN_TO_MAX, U32 MAX_LEVEL, U32 PROBABILISTIC_BIAS >
inline bool ThreadSafePriorityQueue< T, K, SORT_MIN_TO_MAX, MAX_LEVEL, PROBABILISTIC_BIAS >::Node::tryMarkForDeletion()
{
	U32 oldVal = mLevel & 0x7FFFFFFF;
	U32 newVal = oldVal | 0x80000000;

	return ( dCompareAndSwap( mLevel, oldVal, newVal ) );
}

/// Choose a random level.
///
/// The chosen level depends on the given PROBABILISTIC_BIAS and MAX_LEVEL,
/// but is not affected by the actual number of nodes in a queue.

template< typename T, typename K, bool SORT_MIN_TO_MAX, U32 MAX_LEVEL, U32 PROBABILISTIC_BIAS >
U32 ThreadSafePriorityQueue< T, K, SORT_MIN_TO_MAX, MAX_LEVEL, PROBABILISTIC_BIAS >::Node::randomLevel()
{
   U32 level = 0;
   while( Platform::getRandom() < ( ( ( F32 ) PROBABILISTIC_BIAS ) / 100 ) && level < ( MAX_LEVEL - 1 ) )
      level ++;
   return level;
}

/// Allocate a new node.
/// The node comes with a reference count of one and its level already set.
///
/// @param level The level to allocate the node at.  If this is -1, a random level is chosen.
/// @return a new node.

template< typename T, typename K, bool SORT_MIN_TO_MAX, U32 MAX_LEVEL, U32 PROBABILISTIC_BIAS >
void* ThreadSafePriorityQueue< T, K, SORT_MIN_TO_MAX, MAX_LEVEL, PROBABILISTIC_BIAS >::Node::operator new( size_t size, S32 level )
{
   if( level == -1 )
      level = randomLevel();

   Node* node = 0;
   while( 1 )
   {
      // Try to take a node from the freelist.  If there's none,
      // allocate a new one.

      if( !smFreeLists[ level ].mDestroyed )
         node = Node::safeRead( smFreeLists[ level ].mNodes );

      if( !node )
      {
         node = ( Node* ) dMalloc( sizeof( Node ) + sizeof( Node* ) * level );
         dMemset( node, 0, sizeof( Node ) );
         node->mLevel = level;
         node->addRef();
         break;
      }
      else if( dCompareAndSwap( smFreeLists[ level ].mNodes, node, node->mNext[ 0 ] ) )
      {
         node->clearLowestBit();
         break;
      }
      else
         node->release(); // Other thread was quicker than us; release.
   }

   AssertFatal( node->getRefCount() != 0, "ThreadSafePriorityQueue::new Node() - invalid refcount" );
   AssertFatal( ( node->getRefCount() % 2 ) == 0, "ThreadSafePriorityQueue::new Node() - invalid refcount" );
   return node;
}

/// Reclaim a node.
///
/// @param node The node to reclaim.  Must refer to a Node instance.

template< typename T, typename K, bool SORT_MIN_TO_MAX, U32 MAX_LEVEL, U32 PROBABILISTIC_BIAS >
void ThreadSafePriorityQueue< T, K, SORT_MIN_TO_MAX, MAX_LEVEL, PROBABILISTIC_BIAS >::Node::operator delete( void* ptr )
{
   //TODO: limit number of nodes kept

   Node* node = ( Node* ) ptr;
   U32 level = node->getLevel();
   node->mLevel = level; // Reset the node's deletion bit.

   while( !smFreeLists[ level ].mDestroyed )
   {
      // Put the node on the freelist.

      Node* freeList = smFreeLists[ level ].mNodes;
      node->mNext[ 0 ] = freeList;
      
      if( dCompareAndSwap( smFreeLists[ level ].mNodes, freeList, node ) )
      {
         node = NULL;
         break;
      }
   }
   
   if( node )
      dFree( node );
}

template< typename T, typename K, bool SORT_MIN_TO_MAX, U32 MAX_LEVEL, U32 PROBABILISTIC_BIAS >
ThreadSafePriorityQueue< T, K, SORT_MIN_TO_MAX, MAX_LEVEL, PROBABILISTIC_BIAS >::Node::FreeList::~FreeList()
{
   mDestroyed = true;
   while( mNodes )
   {
      //FIXME: could leak some bytes under unfortunate circumstances (this in
      //   combination with mDestroyed is a dependent write)

      Node* next = mNodes;
      if( dCompareAndSwap( mNodes, next, next->mNext[ 0 ] ) )
         dFree( next );
   }
}

//--------------------------------------------------------------------------
//    ThreadSafePriorityQueueWithUpdate.
//--------------------------------------------------------------------------

/// Fast, lock-free priority queue implementation for concurrent access that
/// performs dynamic re-prioritization of items.
///
/// Within the bounds of a set update interval UPDATE_INTERVAL, the takeNext
/// method is guaranteed to always return the item that has the highest priority
/// at the time the method is called rather than at the time items were inserted
/// into the queue.
///
/// Values placed on the queue must implement the following interface:
///
/// @code
/// template&lt; typename K >
/// struct IThreadSafePriorityQueueItem
/// {
///    // Default constructor.
///    IThreadSafePriorityQueueItem();
///
///    // Return the current priority.
///    // This must run normally even if the item is already dead.
///    K getPriority();
///
///    // Return true if the item is still meant to be waiting in the queue.
///    bool isAlive();
/// };
/// @endcode

template< typename T, typename K, bool SORT_MIN_TO_MAX = false, U32 MAX_LEVEL = 4, U32 PROBABILISTIC_BIAS = 50 >
struct ThreadSafePriorityQueueWithUpdate : public ThreadSafePriorityQueue< T, K, SORT_MIN_TO_MAX, MAX_LEVEL, PROBABILISTIC_BIAS >
{

   typedef T ValueType;
   typedef K KeyType;

   enum { DEFAULT_UPDATE_INTERVAL = 256 };

   ThreadSafePriorityQueueWithUpdate( U32 updateInterval = DEFAULT_UPDATE_INTERVAL );

   void              insert( KeyType priority, const T& value );
   bool              takeNext( T& outValue, KeyType upToPriority = ( SORT_MIN_TO_MAX ? TypeTraits< KeyType >::MAX : TypeTraits< KeyType >::MIN ) );

   U32               getUpdateInterval() const;
   void              setUpdateInterval( U32 value );

   KeyType           getTimeBasedPriorityBoost() const;
   void              setTimeBasedPriorityBoost( KeyType value );

   void              updatePriorities();

protected:
   typedef ThreadSafePriorityQueue< T, K, SORT_MIN_TO_MAX, MAX_LEVEL, PROBABILISTIC_BIAS > Parent;
   typedef U32 TickType;
   typedef typename Parent::NodePtr NodePtr;

   U32               mUpdateInterval;
   KeyType           mPriorityBoost;      ///< If this is non-zero, priorities will be boosted by this amount each update.  This can be used to prevent constant high-priority inserts to starve low-priority items already in the queue.

   /// Work queue for node updates.
   ThreadSafePriorityQueue< NodePtr, TickType, true, MAX_LEVEL, PROBABILISTIC_BIAS > mUpdateQueue;

   TickType          getTick()            { return Platform::getRealMilliseconds(); }
};

template< typename T, typename K, bool SORT_MIN_TO_MAX, U32 MAX_LEVEL, U32 PROBABILISTIC_BIAS >
ThreadSafePriorityQueueWithUpdate< T, K, SORT_MIN_TO_MAX, MAX_LEVEL, PROBABILISTIC_BIAS >::ThreadSafePriorityQueueWithUpdate( U32 updateInterval )
   : mUpdateInterval( updateInterval ),
     mPriorityBoost( TypeTraits< KeyType >::ZERO )
{
}

/// Return the current update interval in milliseconds.

template< typename T, typename K, bool SORT_MIN_TO_MAX, U32 MAX_LEVEL, U32 PROBABILISTIC_BIAS >
U32 ThreadSafePriorityQueueWithUpdate< T, K, SORT_MIN_TO_MAX, MAX_LEVEL, PROBABILISTIC_BIAS >::getUpdateInterval() const
{
   return mUpdateInterval;
}

/// Set update interval of queue to given value.
///
/// <em>Call this method on the main thread only.</em>
///
/// @param value Time between priority updates in milliseconds.

template< typename T, typename K, bool SORT_MIN_TO_MAX, U32 MAX_LEVEL, U32 PROBABILISTIC_BIAS >
void ThreadSafePriorityQueueWithUpdate< T, K, SORT_MIN_TO_MAX, MAX_LEVEL, PROBABILISTIC_BIAS >::setUpdateInterval( U32 value )
{
   mUpdateInterval = value;
}

/// Return the delta to apply to priorities on each update.
/// Set to zero to deactivate time-based priority adjustments.

template< typename T, typename K, bool SORT_MIN_TO_MAX, U32 MAX_LEVEL, U32 PROBABILISTIC_BIAS >
K ThreadSafePriorityQueueWithUpdate< T, K, SORT_MIN_TO_MAX, MAX_LEVEL, PROBABILISTIC_BIAS >::getTimeBasedPriorityBoost() const
{
   return mPriorityBoost;
}

/// Set the delta for time-based priority adjustments to the given value.
///
/// <em>Call this method on the main thread only.</em>
///
/// @param value The new priority adjustment value.

template< typename T, typename K, bool SORT_MIN_TO_MAX, U32 MAX_LEVEL, U32 PROBABILISTIC_BIAS >
void ThreadSafePriorityQueueWithUpdate< T, K, SORT_MIN_TO_MAX, MAX_LEVEL, PROBABILISTIC_BIAS >::setTimeBasedPriorityBoost( KeyType value )
{
   mPriorityBoost = value;
}

template< typename T, typename K, bool SORT_MIN_TO_MAX, U32 MAX_LEVEL, U32 PROBABILISTIC_BIAS >
void ThreadSafePriorityQueueWithUpdate< T, K, SORT_MIN_TO_MAX, MAX_LEVEL, PROBABILISTIC_BIAS >::insert( KeyType priority, const ValueType& value )
{
   NodePtr node;
   Parent::insert( priority, value, node );
   mUpdateQueue.insert( getTick() + getUpdateInterval(), node );
}

template< typename T, typename K, bool SORT_MIN_TO_MAX, U32 MAX_LEVEL, U32 PROBABILISTIC_BIAS >
bool ThreadSafePriorityQueueWithUpdate< T, K, SORT_MIN_TO_MAX, MAX_LEVEL, PROBABILISTIC_BIAS >::takeNext( T& outValue, KeyType upToPriority )
{
   updatePriorities();

   bool result = false;
   do
   {
      result = Parent::takeNext( outValue, upToPriority );
   }
   while( result && !outValue.isAlive() );
   
   return result;
}

///

template< typename T, typename K, bool SORT_MIN_TO_MAX, U32 MAX_LEVEL, U32 PROBABILISTIC_BIAS >
void ThreadSafePriorityQueueWithUpdate< T, K, SORT_MIN_TO_MAX, MAX_LEVEL, PROBABILISTIC_BIAS >::updatePriorities()
{
   TickType currentTime       = getTick();
   U32      numNodesUpdated   = 0;
   U32      numNodesDead      = 0;
   U32      numNodesChanged   = 0;

   NodePtr node;
   while( mUpdateQueue.takeNext( node, currentTime ) )
   {
      numNodesUpdated ++;

      // Since we're updating nodes on the update queue only periodically,
      // their associated values or main queue nodes may have died in the
      // meantime.  If so, we just discard them here.

      if( node->getValue().isAlive()
          && !node->isMarkedForDeletion() )
      {
         KeyType newPriority = node->getValue().getPriority() + getTimeBasedPriorityBoost();
         if( newPriority != node->getPriority() )
         {
            // Node is outdated.  Reinsert with new priority and mark the
            // old node for deletion.

            insert( newPriority, node->getValue() );
            node->tryMarkForDeletion();
            numNodesChanged ++;
         }
         else
         {
            // Node is still current.  Just move to end.

            mUpdateQueue.insert( currentTime + getUpdateInterval(), node );
         }
      }
      else
         numNodesDead ++;
   }

   #ifdef DEBUG_SPEW
   if( numNodesUpdated )
      Platform::outputDebugString( "[ThreadSafePriorityQueueWithUpdate] updated %i nodes (%i changed, %i dead)",
                                   numNodesUpdated, numNodesChanged, numNodesDead );
   #endif
}

// Re-enable TMM if necessary.
#include "platform/tmm_on.h"

#undef DEBUG_SPEW

#endif // !_THREADSAFEPRIORITYQUEUE_H_
