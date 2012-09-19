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

#ifndef _TSINGLETON_H_
#define _TSINGLETON_H_

#ifndef _PLATFORMASSERT_H_
#include "platform/platformAssert.h"
#endif

/// This is a simple thread safe singleton class based on the 
/// design of boost::singleton_default (see http://www.boost.org/).
///
/// This singleton is guaranteed to be constructed before main() is called
/// and destroyed after main() exits.  It will also be created on demand
/// if Singleton<T>::instance() is called before main() begins.
/// 
/// This thread safety only works within the execution context of main().
/// Access to the singleton from multiple threads outside of main() is
/// is not guaranteed to be thread safe.
///
/// To create a singleton you only need to access it once in your code:
///
///   Singleton<MySingletonClass>::instance()->myFunction();
///
/// You do not need to derive from this class.
///
/// @note Generally stay away from this class (!!) except if your class T
///   has no meaningful constructor.  Otherwise, it is very easy to make
///   execution of global ctors ordering dependent.
template <typename T>
class Singleton
{
private:

   // This is the creator object which ensures that the
   // singleton is created before main() begins.
   struct SingletonCreator
   {
      SingletonCreator() { Singleton<T>::instance(); } 

      // This dummy function is used below to force 
      // singleton creation at startup.
      inline void forceSafeCreate() const {}
   };

   // The creator object instance.
   static SingletonCreator smSingletonCreator;
   
   /// This is private on purpose.
   Singleton();

public:

   /// Returns the singleton instance.
   static T* instance()
   {
      // The singleton.
      static T theSingleton;

      // This is here to force the compiler to create
      // the singleton before main() is called.
      smSingletonCreator.forceSafeCreate();

      return &theSingleton;
   }

};

template <typename T> 
typename Singleton<T>::SingletonCreator Singleton<T>::smSingletonCreator;


/// This is a managed singleton class with explict creation
/// and destruction functions which must be called at startup
/// and shutdown of the engine.
///
/// Your class to be managed must implement the following
/// function:
///
/// static const char* getSingletonName() { return "YourClassName"; }
///
/// This allow us to provide better asserts.
///
template <typename T>
class ManagedSingleton
{
private:

   static T *smSingleton;

public:

   /// Create the singleton instance.
   /// @note Asserts when the singleton instance has already been constructed.
   static void createSingleton() 
   {
      AssertFatal( smSingleton == NULL, String::ToString( "%s::createSingleton() - The singleton is already created!", T::getSingletonName() ) );
      smSingleton = new T(); 
   }

   /// Destroy the singleton instance.
   /// @note Asserts when no singleton has been constructed.
   static void deleteSingleton()
   {
      AssertFatal( smSingleton, String::ToString( "%s::deleteSingleton() - The singleton doest not exist!", T::getSingletonName() ) );
      delete smSingleton;
      smSingleton = NULL;
   }

   /// Return the singleton instance.
   /// @note Asserts when called before createSingleton().
   static T* instance() 
   { 
      AssertFatal( smSingleton, String::ToString( "%s::instance() - The singleton has not been created!", T::getSingletonName() ) );
      return smSingleton; 
   }

   /// Return the singleton instance or NULL if it has been deleted or not yet constructed.
   static T* instanceOrNull()
   {
      return smSingleton;
   }
};

///
template <typename T> 
T* ManagedSingleton<T>::smSingleton = NULL;


#endif //_TSINGLETON_H_

