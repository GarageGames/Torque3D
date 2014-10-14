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

#ifndef _TORQUETHREADSTATIC_H_
#define _TORQUETHREADSTATIC_H_

#include "core/util/tVector.h"

//-----------------------------------------------------------------------------
// TorqueThreadStatic Base Class
class _TorqueThreadStatic
{
   friend class _TorqueThreadStaticReg;

private:

#ifdef TORQUE_ENABLE_THREAD_STATIC_METRICS
   U32 mHitCount;
#endif

protected:
   static U32 mListIndex;
   virtual _TorqueThreadStatic *_createInstance() const = 0;

public:
   _TorqueThreadStatic()
#ifdef TORQUE_ENABLE_THREAD_STATIC_METRICS
    :  mHitCount( 0 ) 
#endif
   { };
   virtual ~_TorqueThreadStatic() { }

   static const U32 getListIndex(){ return mListIndex; }

   virtual void *getMemInstPtr() = 0;
   virtual const void *getConstMemInstPtr() const = 0;
   virtual const dsize_t getMemInstSize() const = 0;

#ifdef TORQUE_ENABLE_THREAD_STATIC_METRICS
   _TorqueThreadStatic *_chainHit() { mHitCount++; return this; }
   const U32 &trackHit() { return ++mHitCount; }
   const U32 &getHitCount() const { return mHitCount; }
#endif
};
// Typedef
typedef VectorPtr<_TorqueThreadStatic *> TorqueThreadStaticList;
typedef TorqueThreadStaticList * TorqueThreadStaticListHandle;

//-----------------------------------------------------------------------------
// Auto-registration class and manager of the instances
class _TorqueThreadStaticReg
{
   // This will manage all of the thread static registrations
   static _TorqueThreadStaticReg *smFirst;
   _TorqueThreadStaticReg *mNext;

   // This is a vector of vectors which will store instances of thread static
   // variables. mThreadStaticInsances[0] will be the list of the initial values
   // of the statics, and then indexing for instanced versions will start at 1
   // 
   // Note that the list of instances in mThreadStaticInstances[0] does not, and
   // must not get 'delete' called on it, because all members of the list are
   // pointers to statically allocated memory. All other lists will be contain
   // pointers to dynamically allocated memory, and will need to be freed upon
   // termination.
   //
   // So this was originally a static data member, however that caused problems because
   // I was relying on static initialization order to make sure the vector got initialized
   // *before* any static instance of this class was created via macro. By wrapping the
   // static in a function, I can be assured that the static memory will get initialized
   // before it is modified.
   static Vector<TorqueThreadStaticList> &getThreadStaticListVector();

public:
   /// Constructor
   _TorqueThreadStaticReg( _TorqueThreadStatic *ttsInitial )
   {
      // Link this entry into the list
      mNext = smFirst;
      smFirst = this;

      // Create list 0 (initial values) if it doesn't exist
      if( getThreadStaticListVector().empty() )
         getThreadStaticListVector().increment();

      // Set the index of the thread static for lookup
      ttsInitial->mListIndex = getThreadStaticListVector()[0].size();

      // Add the static to the initial value list
      getThreadStaticListVector()[0].push_back( ttsInitial );
   }

   virtual ~_TorqueThreadStaticReg();

   // Accessors
   static const TorqueThreadStaticList &getStaticList( const U32 idx = 0 ) 
   { 
      AssertFatal( getThreadStaticListVector().size() > idx, "Out of range static list" ); 
      
      return getThreadStaticListVector()[idx]; 
   }

   static void destroyInstances();
   static void destroyInstance( TorqueThreadStaticList *instanceList );

   static const _TorqueThreadStaticReg *getFirst() { return smFirst; }

   const _TorqueThreadStaticReg *getNext() const { return mNext; }

   /// Spawn another copy of the ThreadStatics and pass back the id
   static TorqueThreadStaticListHandle spawnThreadStaticsInstance();
};

//-----------------------------------------------------------------------------
// Template class that will get used as a base for the thread statics
template<class T>
class TorqueThreadStatic : public _TorqueThreadStatic
{
   // The reg object will want access to mInstance
   friend class _TorqueThreadStaticReg;

private:
   T mInstance;

public:
   TorqueThreadStatic( T instanceVal ) : mInstance( instanceVal ) {}
   virtual void *getMemInstPtr() { return &mInstance; }
   virtual const void *getConstMemInstPtr() const { return &mInstance; }

   // I am not sure these are needed, and I don't want to create confusing-to-debug code
#if 0
   // Operator overloads
   operator T*() { return &mInstance; }
   operator T*() const { return &mInstance; }
   operator const T*() const { return &mInstance; }

   bool operator ==( const T &l ) const { return mInstance == l; }
   bool operator !=( const T &l ) const { return mInstance != l; }

   T &operator =( const T &l ) { mInstance = l; return mInstance; }
#endif // if0
};

//-----------------------------------------------------------------------------
// If ThreadStatic behavior is not enabled, than the macros will resolve
// to regular, static memory
#ifndef TORQUE_ENABLE_THREAD_STATICS

#define DITTS( type, name, initialvalue ) static type name = initialvalue
#define ATTS( name ) name

#else // TORQUE_ENABLE_THREAD_STATICS is defined

//-----------------------------------------------------------------------------
// Declare TorqueThreadStatic, and initialize it's value
//
// This macro would be used in a .cpp file to declare a ThreadStatic
#define DITTS(type, name, initalvalue) \
class _##name##TorqueThreadStatic : public TorqueThreadStatic<type> \
{ \
protected:\
   virtual _TorqueThreadStatic *_createInstance() const { return new _##name##TorqueThreadStatic; } \
public: \
   _##name##TorqueThreadStatic() : TorqueThreadStatic<type>( initalvalue ) {} \
   virtual const dsize_t getMemInstSize() const { return sizeof( type ); } \
   type &_cast() { return *reinterpret_cast<type *>( getMemInstPtr() ); } \
   const type &_const_cast() const { return *reinterpret_cast<const type *>( getConstMemInstPtr() ); } \
}; \
static _##name##TorqueThreadStatic name##TorqueThreadStatic; \
static _TorqueThreadStaticReg _##name##TTSReg( reinterpret_cast<_TorqueThreadStatic *>( & name##TorqueThreadStatic ) )

//-----------------------------------------------------------------------------
// Access TorqueThreadStatic

// NOTE: TEMPDEF is there as a temporary place holder for however we want to get the index of the currently running
// thread or whatever.
#define TEMPDEF 0

#ifdef TORQUE_ENABLE_THREAD_STATIC_METRICS
// Access counting macro
#  define ATTS_(name, idx) \
   (reinterpret_cast< _##name##TorqueThreadStatic *>( _TorqueThreadStaticReg::getStaticList( idx )[ _##name##TorqueThreadStatic::getListIndex() ]->_chainHit() )->_cast() )
// Const access counting macro
#  define CATTS_(name, idx) \
   (reinterpret_cast< _##name##TorqueThreadStatic *>( _TorqueThreadStaticReg::getStaticList( idx )[ _##name##TorqueThreadStatic::getListIndex() ]->_chainHit() )->_const_cast() )
#else
// Regular access macro
#  define ATTS_(name, idx) \
   (reinterpret_cast< _##name##TorqueThreadStatic *>( _TorqueThreadStaticReg::getStaticList( idx )[ _##name##TorqueThreadStatic::getListIndex() ] )->_cast() )
// Const access macro
#  define CATTS_(name, idx) \
   (reinterpret_cast< _##name##TorqueThreadStatic *>( _TorqueThreadStaticReg::getStaticList( idx )[ _##name##TorqueThreadStatic::getListIndex() ] )->_const_cast() )
#endif // TORQUE_ENABLE_THREAD_STATIC_METRICS

#define ATTS(name) ATTS_(name, TEMPDEF)
#define CATTS(name) CATTS_(name, TEMPDEF)

#endif // TORQUE_ENABLE_THREAD_STATICS

#endif