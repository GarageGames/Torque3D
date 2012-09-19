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

#ifndef _THREADSAFEREFCOUNT_H_
#define _THREADSAFEREFCOUNT_H_

#ifndef _PLATFORMINTRINSICS_H_
#  include "platform/platformIntrinsics.h"
#endif
#ifndef _TYPETRAITS_H_
#  include "platform/typetraits.h"
#endif


/// @file
/// Templated code for concurrent reference-counting.
///
/// Part of this code is based on work by J.D. Valois, Michael M. Maged,
/// and Scott L. Michael.


//--------------------------------------------------------------------------
//    ThreadSafeRefCount.
//--------------------------------------------------------------------------

/// Baseclass for concurrently reference-counted objects.
///
/// @note NOTE that freshly instantiated objects start out with a reference
///    count of ZERO!  Depending on how this class is used, this may not
///    be desirable, so override this behavior in constructors if necessary.
///
/// @param T the class being reference counted; this is passed to this class,
///    so it can call the correct destructor without having to force users
///    to have virtual methods

template< class T, class DeletePolicy = DeleteSingle >
class ThreadSafeRefCount
{
   public:

      typedef void Parent;

      ThreadSafeRefCount()
         : mRefCount( 0 ) {}
      ThreadSafeRefCount( bool noSet ) {}

      bool           isShared() const;
      U32            getRefCount() const;
      void           addRef();
      void           release();
      void           clearLowestBit();
      static T*      safeRead( T* const volatile& refPtr );

   protected:

      U32            mRefCount;  ///< Reference count and claim bit.  Note that this increments in steps of two.

      static U32     decrementAndTestAndSet( U32& refCount );
};

/// @return true if the object is referenced by more than a single
///   reference.

template< class T, class DeletePolicy >
inline bool ThreadSafeRefCount< T, DeletePolicy >::isShared() const
{
   return ( mRefCount > 3 );
}

/// Get the current reference count.  This method is mostly meant for
/// debugging and should not normally be used.

template< class T, class DeletePolicy >
inline U32 ThreadSafeRefCount< T, DeletePolicy >::getRefCount() const
{
   return mRefCount;
}

/// Increase the reference count on the object.

template< class T, class DeletePolicy >
inline void ThreadSafeRefCount< T, DeletePolicy >::addRef()
{
   dFetchAndAdd( mRefCount, 2 );
}

/// Decrease the object's reference count and delete the object, if the count
/// drops to zero and claiming the object by the current thread succeeds.

template< class T, class DeletePolicy >
inline void ThreadSafeRefCount< T, DeletePolicy >::release()
{
   AssertFatal( mRefCount != 0, "ThreadSafeRefCount::release() - refcount of zero" );
   if( decrementAndTestAndSet( mRefCount ) != 0 )
      DeletePolicy::destroy( ( T* ) this );
}

/// Dereference a reference-counted pointer in a multi-thread safe way.

template< class T, class DeletePolicy >
T* ThreadSafeRefCount< T, DeletePolicy >::safeRead( T* const volatile& refPtr )
{
   while( 1 )
   {
      // Support tagged pointers here.

      T* ptr = TypeTraits< T* >::getUntaggedPtr( refPtr );
      if( !ptr )
         return 0;

      ptr->addRef();
      if( ptr == TypeTraits< T* >::getUntaggedPtr( refPtr ) )
         return ptr;
      else
         ptr->release();
   }
}

/// Decrement the given reference count.  Return 1 if the count dropped to zero
/// and the claim bit has been successfully set; return 0 otherwise.

template< class T, class DeletePolicy >
U32 ThreadSafeRefCount< T, DeletePolicy >::decrementAndTestAndSet( U32& refCount )
{
   U32 oldVal;
   U32 newVal;

   do
   {
      oldVal = refCount;
      newVal = oldVal - 2;

      AssertFatal( oldVal >= 2,
         "ThreadSafeRefCount::decrementAndTestAndSet() - invalid refcount" );

      if( newVal == 0 )
         newVal = 1;
   }
   while( !dCompareAndSwap( refCount, oldVal, newVal ) );

   return ( ( oldVal - newVal ) & 1 );
}

///

template< class T, class DeletePolicy >
inline void ThreadSafeRefCount< T, DeletePolicy >::clearLowestBit()
{
   AssertFatal( mRefCount % 2 != 0, "ThreadSafeRefCount::clearLowestBit() - invalid refcount" );

   U32 oldVal;
   U32 newVal;

   do
   {
      oldVal = mRefCount;
      newVal = oldVal - 1;
   }
   while( !dCompareAndSwap( mRefCount, oldVal, newVal ) );
}

//--------------------------------------------------------------------------
//    ThreadSafeRef.
//--------------------------------------------------------------------------

/// Reference to a concurrently reference-counted object.
///
/// This class takes care of the reference-counting as well as protecting
/// the reference itself from concurrent operations.
///
/// Tagging allows the pointer contained in the reference to be flagged.
/// Tag state is preserved through updates to the reference.
///
/// @note If you directly assign a freshly created object with a reference
///   count of zero to a ThreadSafeRef, make absolutely sure the ThreadSafeRef
///   is accessed only by a single thread.  Otherwise there's a risk of the
///   object being released and freed in midst of trying to set the reference.
template< class T >
class ThreadSafeRef
{
   public:

      enum ETag
      {
         TAG_PreserveOld,  ///< Preserve existing tagging state when changing pointer.
         TAG_PreserveNew,  ///< Preserve tagging state of new pointer when changing pointer.
         TAG_Set,          ///< Set tag when changing pointer; okay if already set.
         TAG_Unset,        ///< Unset tag when changing pointer; okay if already unset.
         TAG_SetOrFail,    ///< Set tag when changing pointer; fail if already set.
         TAG_UnsetOrFail,  ///< Unset tag when changing pointer; fail if already unset.
         TAG_FailIfSet,    ///< Fail changing pointer when currently tagged.
         TAG_FailIfUnset   ///< Fail changing pointer when currently untagged.
      };

      typedef ThreadSafeRef< T > ThisType;

      ThreadSafeRef()                        : mPtr( 0 ) {}
      ThreadSafeRef( T* ptr )                : mPtr( ThreadSafeRefCount< T >::safeRead( ptr ) ) {}
      ThreadSafeRef( const ThisType& ref )   : mPtr( ThreadSafeRefCount< T >::safeRead( ref.mPtr ) ) {}
      ~ThreadSafeRef()
      {
         T* ptr = NULL;
         while( !trySetFromTo( mPtr, ptr ) );
      }

      T*             ptr() const { return getUntaggedPtr( mPtr ) ; }
      void           setTag() { while( !trySetFromTo( mPtr, mPtr, TAG_Set ) ); }
      bool           isTagged() const { return isTaggedPtr( mPtr ); }
      bool           trySetFromTo( T* oldVal, T* const volatile& newVal, ETag tag = TAG_PreserveOld );
      bool           trySetFromTo( T* oldVal, const ThisType& newVal, ETag tag = TAG_PreserveOld );
      bool           trySetFromTo( const ThisType& oldVal, const ThisType& newVal, ETag tag = TAG_PreserveOld );
      static void    unsafeWrite( ThisType& ref, T* ptr );
      static T*      safeRead( T* const volatile& refPtr ) { return ThreadSafeRefCount< T >::safeRead( refPtr ); }

      bool           operator ==( T* ptr ) const;
      bool           operator ==( const ThisType& ref ) const;
      bool           operator !=( T* ptr ) const { return !( *this == ptr ); }
      bool           operator !=( const ThisType& ref ) const { return !( *this == ref ); }
      ThisType&      operator =( T* ptr );
      ThisType&      operator =( const ThisType& ref );

      bool           operator !() const   { return ( ptr() == 0 ); }
      T&             operator *() const   { return *ptr(); }
      T*             operator ->() const  { return ptr(); }
                     operator T*() const  { return ptr(); }

   protected:

      T* volatile    mPtr;

      static bool isTaggedPtr( T* ptr ) { return TypeTraits< T* >::isTaggedPtr( ptr ); }
      static T* getTaggedPtr( T* ptr ) { return TypeTraits< T* >::getTaggedPtr( ptr ); }
      static T* getUntaggedPtr( T* ptr ) { return TypeTraits< T* >::getUntaggedPtr( ptr ); }
};

/// Update the reference from pointing to oldVal to point to newVal.
/// Do so in a thread-safe way.
///
/// This operation will only succeed, if, when doing the pointer-swapping,
/// the reference still points to oldVal.  If, however, the reference
/// has been changed in the meantime by another thread, the operation will
/// fail.
///
/// @param oldVal The pointer assumed to currently be contained in this ThreadSafeRef.
/// @param newVal The pointer to store in this ThreadSafeRef.
/// @param tag Operation to perform on the reference's tag field.
///
/// @return true, if the reference now points to newVal.

template< class T >
bool ThreadSafeRef< T >::trySetFromTo( T* oldVal, T* const volatile& newVal, ETag tag )
{
   bool setTag = false;
   bool getTag = false;
   bool isTagged = isTaggedPtr( oldVal );

   switch( tag )
   {
   case TAG_PreserveOld:   setTag = isTaggedPtr( oldVal ); break;
   case TAG_PreserveNew:   setTag = isTaggedPtr( newVal ); break;
   case TAG_Set:           setTag = true; break;
   case TAG_Unset:         setTag = false; break;
   case TAG_SetOrFail:     setTag = true; getTag = true; break;
   case TAG_UnsetOrFail:   setTag = false; getTag = true; break;
   case TAG_FailIfSet:     if( isTagged ) return false; break;
   case TAG_FailIfUnset:   if( !isTagged ) return false; break;
   }

   T* newValPtr = ( setTag
                    ? getTaggedPtr( ThreadSafeRefCount< T >::safeRead( newVal ) )
                    : getUntaggedPtr( ThreadSafeRefCount< T >::safeRead( newVal ) ) );

   if( dCompareAndSwap( mPtr,
                        ( getTag
                          ? ( setTag
                              ? getUntaggedPtr( oldVal )
                              : getTaggedPtr( oldVal ) )
                          : oldVal ),
                        newValPtr ) )
   {
      if( getUntaggedPtr( oldVal ) )
         getUntaggedPtr( oldVal )->release();
      return true;
   }
   else
   {
      if( getUntaggedPtr( newValPtr ) )
         getUntaggedPtr( newValPtr )->release();
      return false;
   }
}

template< class T >
inline bool ThreadSafeRef< T >::trySetFromTo( T* oldVal, const ThisType& newVal, ETag tag )
{
   return trySetFromTo( oldVal, newVal.mPtr, tag );
}

template< class T >
inline bool ThreadSafeRef< T >::trySetFromTo( const ThisType& oldVal, const ThisType& newVal, ETag tag )
{
   return trySetFromTo( oldVal.mPtr, newVal.mPtr, tag );
}

/// Update ref to point to ptr but <em>do not</em> release an existing
/// reference held by ref nor do the operation in a thread-safe way.
///
/// This method is <em>only</em> for when you absolutely know that your
/// thread is the only thread operating on a reference <em>and</em> you
/// are keeping track of reference counts yourself.
///
/// @param ref The reference to update.
/// @param ptr The new pointer to store in ref.

template< class T >
inline void ThreadSafeRef< T >::unsafeWrite( ThisType& ref, T* ptr )
{
   ref.mPtr = ptr;
}

template< class T >
inline bool ThreadSafeRef< T >::operator ==( T* p ) const
{
   return ( ptr() == p );
}

template< class T >
inline bool ThreadSafeRef< T >::operator ==( const ThisType& ref ) const
{
   return ( ptr() == ref.ptr() );
}

template< class T >
inline ThreadSafeRef< T >& ThreadSafeRef< T >::operator =( T* ptr )
{
   while( !trySetFromTo( mPtr, ptr, TAG_PreserveNew ) );
   return *this;
}

template< class T >
inline ThreadSafeRef< T >& ThreadSafeRef< T >::operator =( const ThisType& ref )
{
   while( !trySetFromTo( mPtr, ref, TAG_PreserveNew ) );
   return *this;
}


template< typename T >
struct TypeTraits< ThreadSafeRef< T > > : public TypeTraits< T* > {};
template< typename T >
inline T& Deref( ThreadSafeRef< T >& ref )
{
   return *ref;
}
template< typename T >
inline T& Deref( const ThreadSafeRef< T >& ref )
{
   return *ref;
}

#endif // _THREADSAFEREFCOUNT_H_
