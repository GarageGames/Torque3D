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

#ifndef _SFXWORLD_H_
#define _SFXWORLD_H_

#ifndef _SCOPETRACKER_H_
   #include "util/scopeTracker.h"
#endif
#ifndef _TVECTOR_H_
   #include "core/util/tVector.h"
#endif
#ifndef _SFXSYSTEM_H_
   #include "sfx/sfxSystem.h"
#endif
#ifndef _SFXSOUNDSCAPE_H_
   #include "sfx/sfxSoundscape.h"
#endif
#ifndef _SFXAMBIENCE_H_
   #include "sfx/sfxAmbience.h"
#endif

// Disable warning about unreferenced
// local function on VC.
#ifdef TORQUE_COMPILER_VISUALC
   #pragma warning( disable: 4505 )
#endif


//#define DEBUG_SPEW


/// @file
/// System for representing and tracking changes to the SFX world of
/// sounds, sound spaces, occluders, and receivers.
///
/// This is system is abstracted over the number of dimensions it will
/// work with, so it is usable for both 2D and 3D.
///
/// To put it to use, it has to be connected to the engine's scene
/// graph structure by deriving suitable types from SFXObject.


class SFXAmbience;


/// Property flags for SFXObjects.
enum SFXObjectFlags
{
   /// Object occludes sound.
   SFXObjectOccluder          = BIT( 0 ),
   
   /// Object connects two ambient zones.  Results in a continuous blend
   /// between the two ambient zones as the listener travels through the
   /// portal.
   SFXObjectPortal            = BIT( 1 ),
   
   ///
   SFXObjectZone              = BIT( 2 ),
   
   /// Object is currently used as listener.
   ///
   /// @note An object used as a listener will automatically have all
   ///   its other sound-related functionality (occlusion, zoning) ignored.
   SFXObjectListener          = BIT( 3 ),
   
   ///
   SFXObjectEmitter           = BIT( 4 ),
};


/// Information about an object in the SFX world.
///
/// SFXObjects are:
///
/// - Occluders: blocking sound
/// - Zones: ambient sound spaces
/// - Portals: connectors between zones
/// - Listeners: receiving sound
///
/// Note that emitters are not managed by the SFX world.  Instead, the set of
/// 3D voices active on the device at any one point is defined as the set of
/// current sound sources.
///
template< int NUM_DIMENSIONS >
class SFXObject : public ScopeTrackerObject< NUM_DIMENSIONS >
{
   public:
   
      typedef ScopeTrackerObject< NUM_DIMENSIONS > Parent;
            
   protected:
   
      ///
      BitSet32 mFlags;
                  
   public:
   
      ///
      SFXObject()
         : Parent() {}
              
      /// Return true if this object is only a sound occluder without any more ambient
      /// sound properties.  These kind of objects are not put into the tracking system.
      bool isOccluderOnly() const { return ( isOccluder() && !isZone() ); }
      
      /// Return true if this object is occluding sound.
      bool isOccluder() const { return mFlags.test( SFXObjectOccluder ); }
      
      /// Return true if this object connects two ambient spaces.
      bool isPortal() const { return mFlags.test( SFXObjectPortal ); }
      
      ///
      bool isZone() const { return mFlags.test( SFXObjectZone ); }
      
      /// Return true if this object is currently being used as the listener.
      /// @note All other sound-related properties (occlusion, ambience, etc.) will be ignored
      ///   on the listener object.
      bool isListener() const { return mFlags.test( SFXObjectListener ); }
      
      ///
      bool isEmitter() const { return mFlags.test( SFXObjectEmitter ); }

      ///
      void setFlags( U32 bits ) { mFlags.set( bits ); }
      
      ///
      void clearFlags( U32 bits ) { mFlags.clear( bits ); }
      
      /// @name Implementor Interface
      ///
      /// The following methods must be implemented by the client.  They are defined here
      /// just for reference.  If you don't override them, you'll get link errors.
      ///
      /// @{
            
      /// Return the world-space position of the ears on this object.
      /// This will only be called on objects that are made listeners.
      void getReferenceCenter( F32 pos[ NUM_DIMENSIONS ] ) const;
   
      /// Return the object's bounding box in world-space including its surround zone.
      void getBounds( F32 minBounds[ NUM_DIMENSIONS ], F32 maxBounds[ NUM_DIMENSIONS ] ) const;

      /// Return the object's bounding box in world-space excluding its surround zone.
      void getRealBounds( F32 minBounds[ NUM_DIMENSIONS ], F32 maxBounds[ NUM_DIMENSIONS ] ) const;
         
      /// Return true if the given point is contained in the object's volume.
      bool containsPoint( const F32 point[ NUM_DIMENSIONS ] ) const;
            
      /// Return the ambient space active within this object.
      /// @note Portals cannot have their own ambient spaces.
      SFXAmbience* getAmbience() const;
         
      ///
      String describeSelf() const;

      /// @}
};


/// SFXWorld tracks an N-dimensional world of SFXObjects.
///
///
/// This class uses two systems as back-ends: occlusion handling is passed on to the
/// occlusion manager installed on the system and tracking the listener traveling through
/// the ambient spaces is 
///
template< int NUM_DIMENSIONS, typename Object >
class SFXWorld : public ScopeTracker< NUM_DIMENSIONS, Object >
{
   public:
   
      typedef ScopeTracker< NUM_DIMENSIONS, Object > Parent;
   
   protected:
   
      /// Record for objects that are currently in scope.
      struct Scope
      {
         /// Sort key on scope stack.  This is used to establish an ordering between
         /// the ambient spaces that the listener is in concurrently.
         F32 mSortValue;
         
         /// The soundscape instance.  Only objects for which the listener actually
         /// is in one of the sound zones, will have an associated soundscape.
         SFXSoundscape* mSoundscape;
         
         /// The object defining this scope.  If this is a portal, we transition
         /// between this space and the space above us in the stack.
         Object mObject;
         
         Scope() {}
         Scope( F32 sortValue, Object object )
            : mSortValue( sortValue ),
              mSoundscape( NULL ),
              mObject( object ) {}
      };
            
      /// A top-down ordering of all objects that are currently in scope.
      Vector< Scope > mScopeStack;
      
      /// Return the index on the scope stack that is tied to the given object or -1 if
      /// the object is not on the stack.
      S32 _findScope( Object object );
      
      ///
      void _sortScopes();
      
      ///
      F32 _getSortValue( Object object );

      // ScopeTracker.
      virtual void _onScopeIn( Object object );
      virtual void _onScopeOut( Object object );
   
   public:
   
      ///
      SFXWorld();
   
      /// Update the state of the SFX world.
      ///
      /// @note This method may potentially be costly; don't update every frame.
      void update();
            
      // ScopeTracker.
      void notifyChanged( Object object );
};



//-----------------------------------------------------------------------------

template< int NUM_DIMENSIONS, class Object >
SFXWorld< NUM_DIMENSIONS, Object >::SFXWorld()
{
   VECTOR_SET_ASSOCIATION( mScopeStack );
}

//-----------------------------------------------------------------------------

template< int NUM_DIMENSIONS, class Object >
void SFXWorld< NUM_DIMENSIONS, Object >::update()
{
   if( !this->mReferenceObject )
      return;

   // Get the reference center (listener) pos.
   
   F32 listenerPos[ NUM_DIMENSIONS ];
   for( U32 n = 0; n < NUM_DIMENSIONS; n ++ )
      listenerPos[ n ] = Deref( this->mReferenceObject ).getMinTrackingNode( n )->getPosition();

   // Check all current scopes.
      
   SFXSoundscapeManager* soundscapeMgr = SFX->getSoundscapeManager();
   for( U32 i = 0; i < mScopeStack.size(); ++ i )
   {
      Scope& scope = mScopeStack[ i ];
      
      if( Deref( scope.mObject ).containsPoint( listenerPos ) )
      {
         // Listener is in the object.
         
         if( !scope.mSoundscape )
         {
            // Add the object's ambient space to the soundscape mix.
            
            SFXAmbience* ambience = Deref( scope.mObject ).getAmbience();
            AssertFatal( ambience != NULL, "SFXWorld::update() - object on stack that does not have an ambient space!" );
            
            // Find the index at which to insert the ambient space.  0 is always
            // the global space and each active soundscape lower down our stack
            // increments by one.
            
            U32 index = 1;
            for( U32 j = 0; j < i; ++ j )
               if( mScopeStack[ j ].mSoundscape )
                  ++ index;
            
            scope.mSoundscape = soundscapeMgr->insertSoundscape( index, ambience );
         }
      }
      else
      {
         SFXAmbience* ambience = Deref( scope.mObject ).getAmbience();
         AssertFatal( ambience != NULL, "SFXWorld::update() - object on stack that does not have an ambient space!" );

         // Listener is neither inside object nor in its
         // proximity zone.  Kill its ambient soundscape if it
         // has one.
         
         if( scope.mSoundscape )
         {
            soundscapeMgr->removeSoundscape( scope.mSoundscape );
            scope.mSoundscape = NULL;
         }
      }
   }
}

//-----------------------------------------------------------------------------

template< int NUM_DIMENSIONS, class Object >
void SFXWorld< NUM_DIMENSIONS, Object >::notifyChanged( Object object )
{
   SFXAmbience* ambience = Deref( object ).getAmbience();
   if( !ambience )
      return;
   
   for( U32 i = 0; i < mScopeStack.size(); ++ i )
   {
      Scope& scope = mScopeStack[ i ];
      if( scope.mObject == object )
      {
         if( scope.mSoundscape )
            scope.mSoundscape->setAmbience( ambience );
         break;
      }
   }
}

//-----------------------------------------------------------------------------

template< int NUM_DIMENSIONS, class Object >
void SFXWorld< NUM_DIMENSIONS, Object >::_onScopeIn( Object object )
{
   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[SFXWorld] Object 0x%x in scope now", object );
   #endif

   // Get the object's ambience properties.  If it has
   // none, ignore the object.
   
   SFXAmbience* ambience = Deref( object ).getAmbience();
   if( !ambience )
      return;

   // Find where to insert the object into the stack.
   
   typename Vector< Scope >::iterator iter = mScopeStack.begin();
   F32 sortValue = _getSortValue( object );
   while( iter != mScopeStack.end() && iter->mSortValue >= sortValue )
      ++ iter;
      
   // Insert the object into the stack.
   
   mScopeStack.insert( iter, Scope( sortValue, object ) );
}

//-----------------------------------------------------------------------------

template< int NUM_DIMENSIONS, class Object >
void SFXWorld< NUM_DIMENSIONS, Object >::_onScopeOut( Object object )
{
   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[SFXWorld] Object 0x%x out of scope now", object );
   #endif
   
   // Find the object on the stack.
   
   S32 index = _findScope( object );
   if( index == -1 )
      return;
      
   // Remove its soundscape.
   
   Scope& scope = mScopeStack[ index ];
   if( scope.mSoundscape )
      SFX->getSoundscapeManager()->removeSoundscape( scope.mSoundscape );
      
   mScopeStack.erase( index );
}

//-----------------------------------------------------------------------------

template< int NUM_DIMENSIONS, class Object >
F32 SFXWorld< NUM_DIMENSIONS, Object >::_getSortValue( Object object )
{
   //RDTODO: probably need to work with the overlap here instead of the full volumes
   
   // Get the real object bounds (without the surround zone).
   
   F32 minBounds[ NUM_DIMENSIONS ], maxBounds[ NUM_DIMENSIONS ];
   Deref( object ).getRealBounds( minBounds, maxBounds );
   
   // Get the minimum extent.
   
   F32 minExtent = maxBounds[ 0 ] - minBounds[ 0 ];
   for( U32 n = 1; n < NUM_DIMENSIONS; ++ n )
      minExtent = getMin( minExtent, maxBounds[ n ] - minBounds[ n ] );
      
   return minExtent;
}

//-----------------------------------------------------------------------------

template< int NUM_DIMENSIONS, class Object >
S32 SFXWorld< NUM_DIMENSIONS, Object >::_findScope( Object object )
{
   for( U32 i = 0; i < mScopeStack.size(); ++ i )
      if( mScopeStack[ i ].mObject == object )
         return i;
         
   return -1;
}

#endif // !_SFXWORLD_H_
