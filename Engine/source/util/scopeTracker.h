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

#ifndef _SCOPETRACKER_H_
#define _SCOPETRACKER_H_

#ifndef _TYPETRAITS_H_
   #include "platform/typetraits.h"
#endif
#ifndef _BITSET_H_
   #include "core/bitSet.h"
#endif
#ifndef _TVECTOR_H_
   #include "core/util/tVector.h"
#endif
#ifndef _CONSOLE_H_
   #include "console/console.h"
#endif
#ifndef _PROFILER_H_
   #include "platform/profiler.h"
#endif

//#define DEBUG_SPEW


/// @file
/// A mechanism for continuous tracking of point/box intersections.


/// Base class for objects registered with a ScopeTracker.
template< S32 NUM_DIMENSIONS >
class ScopeTrackerObject
{
   public:
   
      typedef void Parent;

      /// TrackingNodes are used to track object bounds along individual world axes.
      class TrackingNode
      {
         public:
         
            typedef void Parent;
            
            enum EFlags
            {
               FLAG_Min          = BIT( 0 ),
               FLAG_Max          = BIT( 1 ),
               FLAG_Reference    = BIT( 2 ),
            };
                     
            ///
            BitSet32 mFlags;
            
            ///
            TrackingNode* mOpposite;
            
            /// Distance along axis.
            F32 mPosition;

            /// The object being tracked by this node or NULL.
            ScopeTrackerObject* mObject;
            
            /// Next node on axis tracking chain.
            TrackingNode* mNext;
            
            /// Previous node on axis tracking chain.
            TrackingNode* mPrev;
            
            ///
            TrackingNode()
               : mOpposite( NULL ), mPosition( 0.0f ), mObject( NULL ), mNext( NULL ), mPrev( NULL ) {}
               
            /// Return the object to which this tracking node belongs.
            ScopeTrackerObject* getObject() const { return mObject; }
            
            ///
            TrackingNode* getOpposite() const { return mOpposite; }
            
            ///
            F32 getPosition() const { return mPosition; }
            
            ///
            void setPosition( F32 value ) { mPosition = value; }
            
            ///
            TrackingNode* getNext() const { return mNext; }
            
            ///
            void setNext( TrackingNode* node ) { mNext = node; }
            
            ///
            TrackingNode* getPrev() const { return mPrev; }
            
            ///
            void setPrev( TrackingNode* node ) { mPrev = node; }

            /// Return true if this is left/lower bound node of an object.
            bool isMin() const { return mFlags.test( FLAG_Min ); }
            
            /// Return true if this is the right/upper bound node of an object.
            bool isMax() const { return mFlags.test( FLAG_Max ); }
            
            /// Return true if this is the reference center tracking node.  There will only
            /// ever be one such node on each tracking list.
            bool isReference() const { return mFlags.test( FLAG_Reference ); }
      };
      
      enum
      {
         AllInScope = ( 0x01010101 >> ( ( 4 - NUM_DIMENSIONS ) * 8 ) )
      };
      
   protected:
         
      ///
      union
      {
         U8 mBytes[ 4 ];
         U32 mDWord;
      } mScopeMask;

      ///
      TrackingNode mTrackingNodes[ NUM_DIMENSIONS ][ 2 ];

   public:
   
      ///
      ScopeTrackerObject( U32 flags = 0 )
      {
         clearScopeMask();
         for( U32 n = 0; n < NUM_DIMENSIONS; ++ n )
         {
            TrackingNode* minNode = getMinTrackingNode( n );
            TrackingNode* maxNode = getMaxTrackingNode( n );
            
            minNode->mFlags    = flags;
            maxNode->mFlags    = flags;
            
            minNode->mObject   = this;
            maxNode->mObject   = this;
            
            minNode->mOpposite = maxNode;
            maxNode->mOpposite = minNode;
            
            minNode->mFlags.set( TrackingNode::FLAG_Min );
            maxNode->mFlags.set( TrackingNode::FLAG_Max );
         }
      }
      
      /// Return true if the object is currently being tracked.
      bool isRegistered() const
      {
         for( U32 n = 0; n < NUM_DIMENSIONS; ++ n )
            if( getMinTrackingNode( n )->getNext() != NULL )
               return true;
         return false;
      }
   
      /// Return true if the reference center lies within the object bound's on all axes.
      bool isInScope() const { return ( mScopeMask.mDWord == AllInScope ); }
      
      ///
      bool isInScope( U32 dimension ) const { return mScopeMask.mBytes[ dimension ]; }
      
      ///
      void setInScope( U32 dimension, bool state ) { mScopeMask.mBytes[ dimension ] = ( state ? 1 : 0 ); }
      
      ///
      void clearScopeMask() { mScopeMask.mDWord = 0; }
            
      ///
      TrackingNode* getMinTrackingNode( U32 dimension ) { return &mTrackingNodes[ dimension ][ 0 ]; }
      const TrackingNode* getMinTrackingNode( U32 dimension ) const { return &mTrackingNodes[ dimension ][ 0 ]; }
      
      ///
      TrackingNode* getMaxTrackingNode( U32 dimension ) { return &mTrackingNodes[ dimension ][ 1 ]; }
      const TrackingNode* getMaxTrackingNode( U32 dimension ) const { return &mTrackingNodes[ dimension ][ 1 ]; }
      
      /// @name Implementor Interface
      ///
      /// The following methods must be implemented by the client.  They are defined here
      /// just for reference.  If you don't override them, you'll get link errors.
      ///
      /// @{
      
      /// Return the position of the object in world-space.
      void getPosition( F32 pos[ NUM_DIMENSIONS ] ) const;
      
      /// If this object is the reference object, this method should return the world-space pivot
      /// point in the object that will be the world reference center.
      void getReferenceCenter( F32 pos[ NUM_DIMENSIONS ] ) const;
      
      /// Return the object's bounding box in world-space.
      void getBounds( F32 minBounds[ NUM_DIMENSIONS ], F32 maxBounds[ NUM_DIMENSIONS ] ) const;
      
      ///
      String describeSelf() const;
   
      /// @}
};


/// Helper class to track the position of a point in N-dimensional space relative to a collection
/// of N-dimensional volumes.
///
/// This class works by breaking the N-dimensional case down into N one-dimensional cases.  By tracking
/// objects independently along each of their axes, intersection testing becomes a trivial matter of
/// doing point-on-line tests.  The line segments can be conveniently represented as ordered linked
/// lists along which the reference point is being moved.
///
/// To determine N-dimensional containment of the reference point, the result of each of the one-dimensional
/// point-on-line tests simply has to be combined.  If the point lies on each of N 1D segments of a
/// given volume, then the point is fully contained in the volume.
///
/// This class may be used in places where otherwise a spatial subdivision scheme would be necessary in
/// order to limit the number of containment tests triggered by each movement of the reference point.
/// Once the tracker has been set up, each position change of an object or the reference center will result
/// in a usually small number of incremental list updates.
///
/// Another advantage is that this class makes it easy to reduce 3D tracking to 2D tracking if tracking on
/// the height axis isn't important.
///
/// The following interface must be implemented by the given "Object" type:
///
/// @code
/// struct Object : public ScopeTrackerObject< NUM_DIMENSIONS >
/// {
///    /// Return the position of the object in world-space.
///    void getPosition( F32 pos[ NUM_DIMENSIONS ] ) const;
///
///    /// If this object is the reference object, this method should return the world-space pivot
///    /// point in the object that will be the world reference center.
///    void getReferenceCenter( F32 pos[ NUM_DIMENSIONS ] ) const;
///
///    /// Return the object's bounding box in world-space.
///    void getBounds( F32 minBounds[ NUM_DIMENSIONS ], F32 maxBounds[ NUM_DIMENSIONS ] ) const;
/// };
/// @endcode
///
/// Terminology:
///
/// - "In Scope": A volume is in scope if it fully contains the reference center.
/// - "Reference Object": Object that is the designated center of the world. 
///
/// @param NUM_DIMENSIONS Number of dimensions to track; must be <=4.
/// @param Object Value type for objects tracked by the ScopeTracker.  Must have pointer behavior.
template< S32 NUM_DIMENSIONS, typename Object >
class ScopeTracker
{
   public:
   
      typedef void Parent;
      typedef typename TypeTraits< Object >::BaseType ObjectType;
      typedef typename ObjectType::TrackingNode NodeType;
   
   protected:
   
      enum
      {
         MIN = 0,
         MAX = 1
      };
            
      /// The reference object.  This is the center relative to which all
      /// tracking occurs.  Any other object is in scope when it contains the
      /// reference object.
      Object mReferenceObject;
      
      ///
      NodeType* mTrackingList[ NUM_DIMENSIONS ][ 2 ];
      
      ///
      NodeType mBoundaryNodes[ NUM_DIMENSIONS ][ 2 ];
      
      ///
      Vector< Object > mPotentialScopeInObjects;
      
      /// @name Scoping
      /// @{
      
      virtual void _onScopeIn( Object object ) {}
      
      virtual void _onScopeOut( Object object ) {}
      
      /// Set the scoping state of the given object.
      void _setScope( Object object );
      
      /// @}
      
      /// @name Tracking
      /// @{

      ///
      void _insertTrackingNode( U32 dimension, NodeType* node );
      
      ///
      void _removeTrackingNode( U32 dimension, NodeType* node );
      
      ///
      void _moveTrackingNode( U32 dimension, NodeType* node, F32 newPos );
      
      ///
      void _initTracking();
      
      ///
      void _uninitTracking();
            
      /// @}
   
   public:
   
      ///
      ScopeTracker();
   
      /// Add a volume object to the world.
      void registerObject( Object object );
      
      /// Remove a volume object from the world.
      void unregisterObject( Object object );
      
      /// Update the position of the object in the world.
      void updateObject( Object object );
      
      ///
      Object getReferenceObject() const { return mReferenceObject; }
      
      ///
      ///
      /// @note Switching reference centers is potentially costly.
      void setReferenceObject( Object object );
      
      ///
      void debugDump();
};


//-----------------------------------------------------------------------------

template< S32 NUM_DIMENSIONS, class Object >
ScopeTracker< NUM_DIMENSIONS, Object >::ScopeTracker()
   : mReferenceObject( NULL )
{
   VECTOR_SET_ASSOCIATION( mPotentialScopeInObjects );
   
   // Initialize the tracking lists.  Put the boundary
   // nodes in place that will always be the heads and tails
   // of each list.
   
   dMemset( mTrackingList, 0, sizeof( mTrackingList ) );
   for( U32 n = 0; n < NUM_DIMENSIONS; ++ n )
   {
      mBoundaryNodes[ n ][ MIN ].setPosition( TypeTraits< F32 >::MIN );
      mBoundaryNodes[ n ][ MAX ].setPosition( TypeTraits< F32 >::MAX );
      
      mBoundaryNodes[ n ][ MIN ].setNext( &mBoundaryNodes[ n ][ MAX ] );
      mBoundaryNodes[ n ][ MAX ].setPrev( &mBoundaryNodes[ n ][ MIN ] );
      
      mBoundaryNodes[ n ][ MIN ].mOpposite = &mBoundaryNodes[ n ][ MAX ];
      mBoundaryNodes[ n ][ MAX ].mOpposite = &mBoundaryNodes[ n ][ MIN ];
            
      mTrackingList[ n ][ MIN ] = &mBoundaryNodes[ n ][ MIN ];
      mTrackingList[ n ][ MAX ] = &mBoundaryNodes[ n ][ MAX ];
   }
}

//-----------------------------------------------------------------------------

template< S32 NUM_DIMENSIONS, class Object >
void ScopeTracker< NUM_DIMENSIONS, Object >::setReferenceObject( Object object )
{
   AssertFatal( !object || !Deref( object ).isRegistered(),
      "ScopeTracker::setReferenceObject - reference object must not be volume object" );
      
   if( mReferenceObject == object )
      return;
      
   // If object is invalid, remove the reference center
   // tracking.

   if( !object )
   {
      // Transition all objects to out-of-scope and
      // deactivate tracking.

      _uninitTracking();
      mReferenceObject = object;
      return;
   }

   if( mReferenceObject )
   {
      //RDFIXME: this is very disruptive
      
      // We have an existing reference object so we need to update
      // the scoping to match it.  Brute-force this for now.

      _uninitTracking();
      mReferenceObject = object;
      _initTracking();      
   }
   else
   {
      // No reference object yet.

      mReferenceObject = object;
      _initTracking();
   }
   
   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[ScopeTracker] Reference object is now 0x%x", object );
   #endif
}

//-----------------------------------------------------------------------------

template< S32 NUM_DIMENSIONS, class Object >
void ScopeTracker< NUM_DIMENSIONS, Object >::registerObject( Object object )
{
   PROFILE_SCOPE( ScopeTracker_registerObject );
   
   // Get the object bounds.
   
   F32 minBounds[ NUM_DIMENSIONS ];
   F32 maxBounds[ NUM_DIMENSIONS ];
   
   Deref( object ).getBounds( minBounds, maxBounds );
   
   // Insert the object's tracking nodes.

   for( U32 n = 0; n < NUM_DIMENSIONS; ++ n )
   {
      NodeType* minNode = Deref( object ).getMinTrackingNode( n );
      NodeType* maxNode = Deref( object ).getMaxTrackingNode( n );

      minNode->setPosition( minBounds[ n ] );
      maxNode->setPosition( maxBounds[ n ] );
      
      // Insert max before min so that max always comes out
      // to the right of min.

      _insertTrackingNode( n, maxNode );
      _insertTrackingNode( n, minNode );
   }
   
   // Set the scoping state of the object.
   
   _setScope( object );
}

//-----------------------------------------------------------------------------

template< S32 NUM_DIMENSIONS, class Object >
void ScopeTracker< NUM_DIMENSIONS, Object >::unregisterObject( Object object )
{
   PROFILE_SCOPE( ScopeTracker_unregisterObject );
   
   if( !Deref( object ).isRegistered() )
      return;
      
   // Clear its scoping state.
   
   if( Deref( object ).isInScope() )
      _onScopeOut( object );
   Deref( object ).clearScopeMask();
   
   // Remove the tracking state.

   for( U32 n = 0; n < NUM_DIMENSIONS; ++ n )
   {
      _removeTrackingNode( n, Deref( object ).getMinTrackingNode( n ) );
      _removeTrackingNode( n, Deref( object ).getMaxTrackingNode( n ) );
   }
}

//-----------------------------------------------------------------------------

template< S32 NUM_DIMENSIONS, class Object >
void ScopeTracker< NUM_DIMENSIONS, Object >::updateObject( Object object )
{
   PROFILE_SCOPE( ScopeTracker_updateObject );
   
   if( object == mReferenceObject )
   {
      // Get the reference center position.
      
      F32 position[ NUM_DIMENSIONS ];
      Deref( mReferenceObject ).getReferenceCenter( position );
      
      // Move the reference tracking node.
      
      for( U32 n = 0; n < NUM_DIMENSIONS; ++ n )
         _moveTrackingNode( n, Deref( mReferenceObject ).getMinTrackingNode( n ), position[ n ] );
         
      // Flush the potential-scope-in list.
      
      while( !mPotentialScopeInObjects.empty() )
      {
         Object object = mPotentialScopeInObjects.last();
         mPotentialScopeInObjects.decrement();
         
         if( Deref( object ).isInScope() )
            _onScopeIn( object );
      }
   }
   else
   {
      // Get the object bounds.
      
      F32 minBounds[ NUM_DIMENSIONS ];
      F32 maxBounds[ NUM_DIMENSIONS ];
      
      Deref( object ).getBounds( minBounds, maxBounds );
      
      // Move the object's tracking nodes.

      bool wasInScope = Deref( object ).isInScope();
      for( U32 n = 0; n < NUM_DIMENSIONS; ++ n )
      {
         NodeType* minNode = Deref( object ).getMinTrackingNode( n );
         NodeType* maxNode = Deref( object ).getMaxTrackingNode( n );

         _moveTrackingNode( n, minNode, minBounds[ n ] );
         _moveTrackingNode( n, maxNode, maxBounds[ n ] );
      }
      
      // Rescope the object, if necessary.
      
      if( wasInScope && !Deref( object ).isInScope() )
         _onScopeOut( object );
      else if( !wasInScope && Deref( object ).isInScope() )
         _onScopeIn( object );
   }
}

//-----------------------------------------------------------------------------

template< S32 NUM_DIMENSIONS, class Object >
void ScopeTracker< NUM_DIMENSIONS, Object >::_insertTrackingNode( U32 dimension, NodeType* node )
{
   //RDTODO: substitute brute-force search with some smarter insertion algorithm
   // (at least dynamically decide on direction for search)

   F32        pos      = node->getPosition();
   NodeType*  current  = mTrackingList[ dimension ][ MIN ]->getNext();
   NodeType*  prev     = mTrackingList[ dimension ][ MIN ];

   while( current->getPosition() < pos )
   {
        prev      = current;
        current   = current->getNext();
   }
   
   prev->setNext( node );
   current->setPrev( node );

   node->setPrev( prev );
   node->setNext( current );
}

//-----------------------------------------------------------------------------

template< S32 NUM_DIMENSIONS, class Object >
void ScopeTracker< NUM_DIMENSIONS, Object >::_removeTrackingNode( U32 dimension, NodeType* node )
{
   NodeType* next = node->getNext();
   NodeType* prev = node->getPrev();
   
   AssertFatal( next != NULL, "ScopeTracker::_insertTrackingNode - invalid list state (no next node)!" );
   AssertFatal( prev != NULL, "ScopeTracker::_insertTrackingNode - invalid list state (no prev node)!" );

   next->setPrev( prev );
   prev->setNext( next );
   
   node->setNext( NULL );
   node->setPrev( NULL );
}

//-----------------------------------------------------------------------------

template< S32 NUM_DIMENSIONS, class Object >
void ScopeTracker< NUM_DIMENSIONS, Object >::_moveTrackingNode( U32 dimension, NodeType* node, F32 newPosition )
{
   PROFILE_SCOPE( ScopeTracker_moveTrackingNode );
   
   AssertFatal( TypeTraits< F32 >::MIN <= newPosition && newPosition <= TypeTraits< F32 >::MAX, "Invalid float in object coordinate!" );

   enum EDirection
   {
      DIRECTION_Up,
      DIRECTION_Down
   };

   // Determine in which direction we are sliding the node.

   EDirection direction;
   if( newPosition < node->getPosition() )
   {
      direction = DIRECTION_Down;
      if( node->getPrev()->getPosition() <= newPosition )
      {
         node->setPosition( newPosition );
         return; // Nothing to do.
      }
   }
   else if( newPosition > node->getPosition() )
   {
      direction = DIRECTION_Up;
      if( node->getNext()->getPosition() >= newPosition )
      {
         node->setPosition( newPosition );
         return; // Nothing to do.
      }
   }
   else
      return; // Nothing to to.

   const bool isReferenceNode = node->isReference();
   
   // Unlink the node.

   NodeType* next = node->getNext();
   NodeType* prev = node->getPrev();
   
   next->setPrev( prev );
   prev->setNext( next );

   // Iterate through to the node's new position.
   
   while(    ( direction == DIRECTION_Up && next->getPosition() < newPosition )
          || ( direction == DIRECTION_Down && prev->getPosition() > newPosition ) )
   {
      NodeType* current = 0;
      switch( direction )
      {
         case DIRECTION_Up:   current = next; break;
         case DIRECTION_Down: current = prev; break;
      }

      if( isReferenceNode )
      {
         Object object = ( Object ) current->getObject();
         if(    ( direction == DIRECTION_Up && current->isMin() )
             || ( direction == DIRECTION_Down && current->isMax() ) )
         {
            Deref( object ).setInScope( dimension, true );
            mPotentialScopeInObjects.push_back( object );
         }
         else
         {
            const bool wasInScope = Deref( object ).isInScope();
            Deref( object ).setInScope( dimension, false );
            if( wasInScope )
               _onScopeOut( object );
         }
      }
      else
      {
         if( current->isReference() )
         {
            Object object = ( Object ) node->getObject();
            if(    ( direction == DIRECTION_Up && node->isMin() )
                || ( direction == DIRECTION_Down && node->isMax() ) )
               Deref( object ).setInScope( dimension, false );
            else
               Deref( object ).setInScope( dimension, true );
         }
      }
      
      switch( direction )
      {
         case DIRECTION_Down:
            next = current;
            prev = current->getPrev();
            break;
            
         case DIRECTION_Up:
            prev = current;
            next = current->getNext();
            break;
      }
   }
   
   // Relink the node.

   prev->setNext( node );
   next->setPrev( node );
   
   node->setPrev( prev );
   node->setNext( next );

   node->setPosition( newPosition );
}

//-----------------------------------------------------------------------------

template< S32 NUM_DIMENSIONS, class Object >
void ScopeTracker< NUM_DIMENSIONS, Object >::_setScope( Object object )
{
   // If there's no reference object, all objects are out of scope.
   
   if( !mReferenceObject || object == mReferenceObject )
   {
      Deref( object ).clearScopeMask();
      return;
   }

   const bool wasInScope = Deref( object ).isInScope();

   // Set the scoping state on each axis.
   
   for( U32 n = 0; n < NUM_DIMENSIONS; ++ n )
   {
      const F32 referencePos  = Deref( mReferenceObject ).getMinTrackingNode( n )->getPosition();
      const F32 objectMin     = Deref( object ).getMinTrackingNode( n )->getPosition();
      const F32 objectMax     = Deref( object ).getMaxTrackingNode( n )->getPosition();

      bool isInScope =  referencePos >= objectMin
                     && referencePos <= objectMax;
      
      Deref( object ).setInScope( n, isInScope );
   }
   
   // Scope in/out if the scoping state has changed.
   
   if( Deref( object ).isInScope() )
   {
      if( !wasInScope )
         _onScopeIn( object );
   }
   else
   {
      if( wasInScope )
         _onScopeOut( object );
   }
}

//-----------------------------------------------------------------------------

template< S32 NUM_DIMENSIONS, class Object >
void ScopeTracker< NUM_DIMENSIONS, Object >::_initTracking()
{
   PROFILE_SCOPE( ScopeTracker_initTracking );
   
   AssertFatal( bool( getReferenceObject() ),
      "ScopeTracker::_initTracking - can only be called with a valid reference object" );
      
   // Put a single tracking node onto each of the lists for
   // the reference object center.
      
   F32 position[ NUM_DIMENSIONS ];
   Deref( mReferenceObject ).getReferenceCenter( position );
   
   for( U32 n = 0; n < NUM_DIMENSIONS; ++ n )
   {
      AssertFatal( TypeTraits< F32 >::MIN <= position[ n ] && position[ n ] <= TypeTraits< F32 >::MAX, "Invalid float in object coordinate!" );

      NodeType* node = Deref( mReferenceObject ).getMinTrackingNode( n );
      node->mFlags.set( NodeType::FLAG_Reference );
      
      node->setPosition( position[ n ] );
      
      _insertTrackingNode( n, node );
   }

   // Update the surroundings of the reference object
   // in the tracking lists for each dimension.
   
   for( U32 n = 0; n < NUM_DIMENSIONS; ++ n )
   {
      //TODO: this could be optimized by dynamically determining whether to walk upwards
      // or downwards depending on which span has fewer nodes; finding that out is not immediately
      // obvious, though
      
      // Walk from the left bound node upwards until we reach the
      // reference object's marker.  Everything that has its max node
      // past the reference object is in scope.
      
      F32 referencePos = Deref( mReferenceObject ).getMinTrackingNode( n )->getPosition();
      for( NodeType* node = mTrackingList[ n ][ 0 ]->getNext(); node->getPosition() < referencePos; node = node->getNext() )
         if( !node->isMax() && node->getOpposite()->getPosition() > referencePos )
         {
            node->getObject()->setInScope( n, true );
            
            // If this is the last dimension we're working on and
            // the current object is in-scope on all dimension,
            // promote to in-scope status.
            
            if( n == ( NUM_DIMENSIONS - 1 ) && node->getObject()->isInScope() )
               _onScopeIn( ( Object ) node->getObject() );
         }
   }
}

//-----------------------------------------------------------------------------

template< S32 NUM_DIMENSIONS, class Object >
void ScopeTracker< NUM_DIMENSIONS, Object >::_uninitTracking()
{
   PROFILE_SCOPE( ScopeTracker_uninitTracking );
   
   AssertFatal( bool( getReferenceObject() ),
      "ScopeTracker::_uninitTracking - can only be called with a valid reference object" );
      
   // Put all objects currently in scope, out of scope.
      
   for( U32 n = 0; n < NUM_DIMENSIONS; ++ n )
   {
      U32 referencePos = Deref( mReferenceObject ).getMinTrackingNode( n )->getPosition();
      for( NodeType* node = mTrackingList[ n ][ 0 ]->getNext(); node->getPosition() < referencePos; node = node->getNext() )
      {
         if( node->getObject()->isInScope() )
            _onScopeOut( ( Object ) node->getObject() );
         node->getObject()->clearScopeMask();
      }
   }
   
   // Remove the reference object's tracking nodes.
   
   for( U32 n = 0; n < NUM_DIMENSIONS; ++ n )
      _removeTrackingNode( n, Deref( mReferenceObject ).getMinTrackingNode( n ) );
}

//-----------------------------------------------------------------------------

template< S32 NUM_DIMENSIONS, class Object >
void ScopeTracker< NUM_DIMENSIONS, Object >::debugDump()
{
   for( U32 n = 0; n < NUM_DIMENSIONS; ++ n )
   {
      Con::printf( "Dimension %i", n );
      Con::printf( "----------------" );
      
      for( NodeType* node = mTrackingList[ n ][ 0 ]; node != NULL; node = node->getNext() )
      {
         String desc;
         if( node->getObject() )
         {
            Object object = ( Object ) node->getObject();
            desc = Deref( object ).describeSelf();
         }

         Con::printf( "pos=%f, type=%s, scope=%s, object=%s",
            node->getPosition(),
            node->isReference() ? "reference" : node->isMin() ? "min" : "max",
            node->getObject() ? node->getObject()->isInScope( n ) ? "1" : "0" : "0",
            desc.c_str() );
      }
      
      Con::printf( "" );
   }
}

#endif // !_SCOPETRACKER_H_
