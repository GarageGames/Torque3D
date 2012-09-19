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

#ifndef _COMPONENTINTERFACE_H_
#define _COMPONENTINTERFACE_H_

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif

#ifndef _SIMOBJECT_H_
#include "console/simObject.h"
#endif

#include "core/util/safeDelete.h"


class SimComponent;


// CodeReview [patw, 2, 13, 2007] The issue I have not addressed in this class is
// interface locking. I think that we want to do this, for sure, but I also want
// to keep it as light-weight as possible. For the most part, there should only
// ever be one thing doing something with a component at one time, but I can see
// many situations where this wouldn't be the case. When we decide to address
// the issues of locking, I believe it should be done here, at the ComponentInterface
// level. I would like lock functionality to be as centralized as possible, and 
// so this is the place for it. The functionality is critical for safe useage of
// the ComponentProperty class, so implementation here would also be ideal.

// CodeReview [patw, 2, 14, 2007] This really should be a ref-counted object
class ComponentInterface
{
   friend class SimComponent;
private:
   SimObjectPtr<SimComponent> mOwner; ///< SimComponent will directly modify this value

public:
   /// Default constructor
   ComponentInterface() : mOwner(NULL) {};

   /// Destructor
   virtual ~ComponentInterface() 
   { 
      mOwner = NULL;
   }

   /// This will return true if the interface is valid
   virtual bool isValid() const 
   { 
      return mOwner != NULL; 
   }

   /// Get the owner of this interface
   SimComponent *getOwner() { return mOwner; }
   const SimComponent *getOwner() const { return mOwner; }
};

typedef VectorPtr<ComponentInterface *> ComponentInterfaceList;
typedef VectorPtr<ComponentInterface *>::iterator ComponentInterfaceListIterator;

// These two asserts I found myself wanting a lot when doing interface methods
#ifdef TORQUE_ENABLE_ASSERTS
#  define VALID_INTERFACE_ASSERT(OwningClassType) \
   AssertFatal( isValid(), "Interface validity check failed." ); \
   AssertFatal( dynamic_cast<const OwningClassType *>( getOwner() ) != NULL, avar( "Owner is not an instance of %s", #OwningClassType ) )
#else
#  define VALID_INTERFACE_ASSERT(OwningClassType)
#endif

/// This class is designed to wrap an existing class or type easily to allow
/// a SimComponent to expose a property with custom processing code in an efficient
/// and safe way. Specialized templates could be written which include validation
/// on sets, and processing on gets. 
///
/// This class has a lot of "blow your leg off" potential, if you have bad aim.
/// I think that a lot of very intuitive functionality can be gained from using
/// this properly, however when implementing a specialized template, be mindful
/// of what you are doing, and 

// CodeReview [patw, 2, 13, 2007] I am very interested in making this as thin as
// possible. I really like the possibilities that it exposes as far as exposing
// "properties" to other components. I want it to be performant, however, so
// if anyone has notes on this, mark up the source, e-mail me, whatever.
template<class T>
class ComponentProperty : public ComponentInterface
{
   typedef ComponentInterface Parent;

protected:
   T *mValuePtr;

   // ComponentInterface Overrides
public:

   // Override this to add a check for valid memory.
   virtual bool isValid() const 
   {
      return ( mValuePtr != NULL ) && Parent::isValid(); 
   }

   // Operator overloads
public:
   /// Dereferencing a value interface will allow get to do any processing and
   /// return the reference to that 
   const T &operator*()
   {
      return get();
   }

   /// Assignment operator will invoke set.
   const T &operator=( const T &lval )
   {
      return set( lval );
   }

   // Constructors/Destructors, specialize these if needed
public:
   /// Default Constructor.
   ComponentProperty() : mValuePtr( NULL )
   {
      mValuePtr = new T;
   }

   /// Copy constructor
   ComponentProperty( const T &copy )
   {
      ComponentProperty();

      // CodeReview [patw, 2, 13, 2007] So, the reasoning here is that I want to
      // use the functionality that a specialized template implements in the set
      // method. See the notes on the set method implementation.
      set( copy );
   }

   /// Destructor, destroy memory
   virtual ~ComponentProperty()
   {
      SAFE_DELETE( mValuePtr );
   }

   // This is the ComponentProperty interface that specializations of the class 
   // will be interested in.
public:

   /// Get the value associated with this interface. Processing code can be done
   /// here for specialized implementations. 
   virtual const T &get() // 'const' is intentionally not used as a modifier here 
   { 
      return *mValuePtr; 
   }

   /// Set the value associated with this interface. Validation/processing code
   /// can be done here. The copy-constructor uses the set method to do it's copy
   /// so be mindful of that, or specialize the copy-constructor.
   virtual const T &set( const T &t )
   { 
      // CodeReview [patw, 2, 13, 2007] So I am using the = operator here. Do you
      // guys think that this should be the default behavior? I am trying to keep
      // everything as object friendly as possible, so I figured I'd use this.
      *mValuePtr = t; 
      return *mValuePtr; 
   }
};

/// This class is just designed to isolate the functionality of querying for, and
/// managing interfaces. 
class ComponentInterfaceCache
{
   // CodeReview [patw, 2, 14, 2007] When we move this whole system to Juggernaught
   // we may want to consider making safe pointers for ComponentInterfaces. Not 
   // sure why I put this note here.
private:
   struct _InterfaceEntry
   {
      ComponentInterface *iface;
      StringTableEntry type;
      StringTableEntry name;
      const SimComponent *owner;
   };

   Vector<_InterfaceEntry> mInterfaceList;
   typedef Vector<_InterfaceEntry>::const_iterator _InterfaceEntryItr;

public:
   /// Add an interface to the cache. This function will return true if the interface
   /// is added successfully. An interface will not be added successfully if an entry
   /// in the list with the same values for 'type' and 'name' is present in the list.
   /// 
   /// @param type Type of the interface being added. If NULL is passed, it will match any type string queried.
   /// @param name Name of interface being added. If NULL is passed, it will match any name string queried.
   /// @param owner The owner of the ComponentInterface being cached
   /// @param cinterface The ComponentInterface being cached
   virtual bool add( const char *type, const char *name, const SimComponent *owner, ComponentInterface *cinterface );

   /// Clear the interface cache. This does not perform any operations on the contents
   /// of the list. 
   virtual void clear();

   /// Query the list for all of the interfaces it stores references to that match
   /// the 'type' and 'name' parameters. The results of the query will be appended 
   /// to the list specified. Pattern matching is done using core/findMatch.h; for
   /// more information on matching, see that code/header pair. Passing NULL for
   /// one of these fields will match all values for that field. The return value
   /// for the method will be the number of interfaces which match the query.
   ///
   /// @param list The list that this method will append search results on to. It is possible to pass NULL here and just receive the return value.
   /// @param type An expression which the 'type' field on an added object must match to be included in results
   /// @param name An expression which the 'name' field on an added object must match to be included in results
   /// @param owner Limit results to components owned/not-owned by this SimComponent (see next param)
   /// @param notOwner If set to true, this will enumerate only interfaces NOT owned by 'owner'
   virtual U32 enumerate( ComponentInterfaceList *list, const char *type = NULL, const char *name = NULL, const SimComponent *owner = NULL, bool notOwner = false ) const;
};

#endif