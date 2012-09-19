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

#ifndef __RESOURCE_H__
#define __RESOURCE_H__

#ifndef _DATACHUNKER_H_
#include "core/dataChunker.h"
#endif

#ifndef _PATH_H_
#include "core/util/path.h"
#endif

#ifndef _REFBASE_H_
#include "core/util/refBase.h"
#endif

#ifndef _TIMECLASS_H_
#include "core/util/timeClass.h"
#endif

#ifndef _TSIGNAL_H_
#include "core/util/tSignal.h"
#endif

#ifndef _PLATFORMASSERT_H_
#include "platform/platformAssert.h"
#endif

class ResourceManager;

// This is a utility class used by the resource manager.
// The prime responsibility of this class is to delete
// a resource.  The base type is never used, but a template
// version is always derived that knows how to delete the
// particular type of resource.  Normally, one needs not
// worry about this class.  However, if one wants to delete
// a particular resource in a special manner, one may
// override the ResourceHolder<T>::~ResourceHolder method.
class ResourceHolderBase
{
public:
   static FreeListChunker<ResourceHolderBase> smHolderFactory;

   virtual ~ResourceHolderBase() {}
   
   // Return void pointer to resource data.
   void *getResource() const { return mRes; }

protected:
   // Construct a resource holder pointing at 'p'.
   ResourceHolderBase(void *p) : mRes(p) {}

   void *mRes;
};

// All resources are derived from this type.  The base type
// is only instantiated by the resource manager
// (the resource manager will return a base resource which a
// derived resource, Resource<T>, will construct itself
// with).  Base class handles locking and unlocking and
// provides several virtual functions to be defined by
// derived resource.
class ResourceBase
{
   friend class ResourceManager;
 
protected:
   class Header;

public:
   typedef U32 Signature;

public:
   ResourceBase(Header *header) { mResourceHeader = (header ? header : &smBlank); }
   virtual ~ResourceBase() {}

   const Torque::Path &getPath() const
   {
      AssertFatal(mResourceHeader != NULL,"ResourceBase::getPath called on invalid resource");
      return mResourceHeader->getPath();
   }

   U32   getChecksum() const
   {
      AssertFatal(mResourceHeader != NULL,"ResourceBase::getChecksum called on invalid resource");
      return mResourceHeader->getChecksum();
   }

protected:

   typedef void ( *NotifyUnloadFn )( const Torque::Path& path, void* resource );

   class Header : public StrongRefBase
   {
   public:
      Header()
      : mSignature(0),
         mResource(NULL),
         mNotifyUnload( NULL )
      {
      }

      const Torque::Path   &getPath() const { return mPath; }

      Signature   getSignature() const { return mSignature; }
      void *getResource() const { return (mResource ? mResource->getResource() : NULL); }
      U32   getChecksum() const;

      virtual void destroySelf();

   private:
      
      friend class ResourceBase;
      friend class ResourceManager;

      Signature            mSignature;
      ResourceHolderBase*  mResource;
      Torque::Path         mPath;
      NotifyUnloadFn       mNotifyUnload;
   };

protected:
   static Header  smBlank;
   ResourceBase() : mResourceHeader(&smBlank) {}

   StrongRefPtr<Header> mResourceHeader;

   void assign(const ResourceBase &inResource, void* resource = NULL);

   // The following functions are virtual, but cannot be pure-virtual
   // because we need to be able to instantiate this class.

   // To be defined by derived class.  Creates a resource
   // holder of the desired type.  Resource template handles
   // this, so one should never need to override.
   virtual ResourceHolderBase *createHolder(void *)
   {
      AssertFatal(0,"ResourceBase::createHolder: should not be called");
      return NULL;
   }

   // Create a Resource of desired type using passed path.  Derived
   // resource class must define this.
   virtual void *create(const Torque::Path &path)
   {
      AssertFatal(0,"ResourceBase::create: should not be called");
      return NULL;
   }

   // Return signature for desired type.
   virtual Signature getSignature() const
   {
      return mResourceHeader->getSignature();
   }

   virtual Signal<bool(const Torque::Path &, void**)>   &getStaticLoadSignal()
   {
      AssertFatal(0,"ResourceBase::getStaticLoadSignal: should not be called");
      static Signal<bool(const Torque::Path &, void**)>   sLoadSignal;

      return sLoadSignal;
   }
   
   virtual void _triggerPostLoadSignal() {}
   virtual NotifyUnloadFn _getNotifyUnloadFn() { return ( NotifyUnloadFn ) NULL; }
};

// This is a utility class used by resource manager.  Classes derived
// from this template pretty much just know how to delete the template's
// type.
template<class T> class ResourceHolder : public ResourceHolderBase
{
public:
   ResourceHolder(T *t) : ResourceHolderBase(t) {}
   virtual ~ResourceHolder() { delete ((T*)mRes); }
};

// Resource template.  When dealing with resources, this is the
// class that will be used.  One creates resources by opening or
// creating them via the resource manager.  The resource manager
// returns ResourceBases, which can be used to construct any
// type of resource (see the constructors for this template).
// When instantiating classes using this template, it is always
// necessary to define the create and getSignature methods.
// The createMethod will be responsible for loading a resource
// from disk using passed path.
template<class T> class Resource : public ResourceBase
{
public:
   Resource() {}
   Resource(const ResourceBase &base) { assign(base); }

   void operator=(const ResourceBase & base) { assign(base); }
   T* operator->()              { return getResource(); }
   T& operator*()               { return *getResource(); }
   operator T*()                { return getResource(); }
   const T* operator->() const  { return getResource(); }
   const T& operator*() const   { return *getResource(); }
   operator const T*() const    { return getResource(); }

   void setResource(const ResourceBase & base, void* resource) { assign(base, resource); }

   static Signature  signature();

   /// Registering with this signal will give an opportunity to handle resource
   /// creation before calling the create() function.  This may be used to handle
   /// file extensions differently and allow a more plugin-like approach to
   /// adding resources.  Using this mechanism, one could, for example, override
   /// the default methods for loading DTS without touching the main source.
   static Signal<bool(const Torque::Path &, void**)>  &getLoadSignal()
   {
      static Signal<bool(const Torque::Path &, void**)>   sLoadSignal;
      return sLoadSignal;
   }
   
   /// Register with this signal to get notified when resources of this type
   /// have been loaded.
   static Signal< void( Resource< T >& ) >& getPostLoadSignal()
   {
      static Signal< void( Resource< T >& ) > sPostLoadSignal;
      return sPostLoadSignal;
   }
   
   /// Register with this signal to get notified when resources of this type
   /// are about to get unloaded.
   static Signal< void( const Torque::Path&, T* ) >& getUnloadSignal()
   {
      static Signal< void( const Torque::Path&, T* ) > sUnloadSignal;
      return sUnloadSignal;
   }

private:
   T        *getResource() { return (T*)mResourceHeader->getResource(); }
   const T  *getResource() const { return (T*)mResourceHeader->getResource(); }

   Signature   getSignature() const { return Resource<T>::signature(); }

   ResourceHolderBase   *createHolder(void *);

   Signal<bool(const Torque::Path &, void**)>   &getStaticLoadSignal() { return getLoadSignal(); }
   
   static void _notifyUnload( const Torque::Path& path, void* resource ) { getUnloadSignal().trigger( path, ( T* ) resource ); }
   
   virtual void _triggerPostLoadSignal() { getPostLoadSignal().trigger( *this ); }
   virtual NotifyUnloadFn _getNotifyUnloadFn() { return ( NotifyUnloadFn ) &_notifyUnload; }

   // These are to be define by instantiated resources
   // No generic version is provided...however, since
   // base resources are instantiated by resource manager,
   // these are not pure virtuals if undefined (but will assert)...
   void *create(const Torque::Path &path);
};


template<class T> inline ResourceHolderBase *Resource<T>::createHolder(void *ptr)
{
   ResourceHolder<T> *resHolder = (ResourceHolder<T>*)(ResourceHolderBase::smHolderFactory.alloc());

   resHolder = constructInPlace(resHolder,(T*)ptr);

   return resHolder;
}

//-----------------------------------------------------------------------------
//    Load Signal Hooks.
//-----------------------------------------------------------------------------

/// This template may be used to register a load signal as follows:
///   static ResourceRegisterLoadSignal<T> sgAuto( staticLoadFunction );
template <class T>
class ResourceRegisterLoadSignal
{
public:
   ResourceRegisterLoadSignal( Delegate<bool(const Torque::Path &, void **)> func )
   {
      Resource<T>::getLoadSignal().notify( func );
   }
};

template< class T >
class ResourceRegisterPostLoadSignal
{
   public:
   
      ResourceRegisterPostLoadSignal( Delegate< void( Resource< T >& ) > func )
      {
         Resource< T >::getPostLoadSignal().notify( func );
      }
};

template< class T >
class ResourceRegisterUnloadSignal
{
   public:
   
      ResourceRegisterUnloadSignal( Delegate< void( const Torque::Path&, T* ) > func )
      {
         Resource< T >::getUnloadSignal().notify( func );
      }
};

#endif // __RESOURCE_H__
