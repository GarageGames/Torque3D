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

#include "console/engineObject.h"
#include "console/engineAPI.h"
#include "platform/tmm_off.h"


IMPLEMENT_NONINSTANTIABLE_CLASS( EngineObject,
   "Abstract base class for all objects exposed through the engine API." )
END_IMPLEMENT_CLASS;
IMPLEMENT_NONINSTANTIABLE_CLASS( StaticEngineObject,
   "Abstract base class for objects that are statically allocated in the engine." )
END_IMPLEMENT_CLASS;


#ifdef TORQUE_DEBUG
U32 EngineObject::smNumEngineObjects;
EngineObject* EngineObject::smFirstEngineObject;
#endif


IEngineObjectPool* IEngineObjectPool::DEFAULT = EngineCRuntimeObjectPool::instance();
EngineCRuntimeObjectPool EngineCRuntimeObjectPool::smInstance;

// Helper to get to engine object's user data.  Not exposed through get/set methods
// as the rest of the engine has no business accessing this data.  It is for the
// exclusive use by the control layer.
void*& _USERDATA( EngineObject* object )
{
   return object->mEngineObjectUserData;
}

//-----------------------------------------------------------------------------

EngineObject::EngineObject()
   : mEngineObjectUserData( NULL )
{
   #ifdef TORQUE_DEBUG
   // Add to instance list.

   mNextEngineObject = smFirstEngineObject;
   mPrevEngineObject = NULL;

   if( smFirstEngineObject )
      smFirstEngineObject->mPrevEngineObject = this;
   smFirstEngineObject = this;

   smNumEngineObjects ++;
   #endif
}

//-----------------------------------------------------------------------------

EngineObject::~EngineObject()
{
   #ifdef TORQUE_DEBUG
   if( mPrevEngineObject )
      mPrevEngineObject->mNextEngineObject = mNextEngineObject;
   else
      smFirstEngineObject = mNextEngineObject;
      
   if( mNextEngineObject )
      mNextEngineObject->mPrevEngineObject = mPrevEngineObject;

   smNumEngineObjects --;
   #endif
}

//-----------------------------------------------------------------------------

void EngineObject::destroySelf()
{
   if( !engineAPI::gUseConsoleInterop )
      AssertFatal( !getRefCount() || TYPEOF( this )->isDisposable(), "EngineObject::destroySelf - object still referenced!" );

   // Call the internal _destroySelf().

   _destroySelf();

   // Destruct the object and release it in the pool.
   
   IEngineObjectPool* pool = this->mEngineObjectPool;
   void* object = this;
   
   destructInPlace( this );
   if( pool )
      pool->freeObject( object );
}

//-----------------------------------------------------------------------------

String EngineObject::describeSelf() const
{
   String desc = String::ToString( "class: %s", TYPEOF( this )->getFullyQualifiedExportName().c_str() );
   
   return desc;
}

//-----------------------------------------------------------------------------

#ifndef TORQUE_DISABLE_MEMORY_MANAGER
void* EngineObject::operator new( size_t size )
{
   AssertFatal( IEngineObjectPool::DEFAULT, "EngineObject::new - No default pool set!" );

   void* ptr = IEngineObjectPool::DEFAULT->allocateObject( size TORQUE_TMM_LOC );
   if( !ptr )
   {
      Platform::AlertOK( "Torque Memory Error", "Out of memory. Shutting down.\n");
      Platform::forceShutdown( -1 );
   }
   
   EngineObject* object = reinterpret_cast< EngineObject* >( ptr );
   object->mEngineObjectPool = IEngineObjectPool::DEFAULT;
   
   return ptr;
}
#endif

//-----------------------------------------------------------------------------

#ifndef TORQUE_DISABLE_MEMORY_MANAGER
void* EngineObject::operator new( size_t size, IEngineObjectPool* pool )
{
   AssertFatal( pool, "EngineObject::new - Got a NULL pool pointer!" );

   void* ptr = pool->allocateObject( size TORQUE_TMM_LOC );
   if( !ptr )
   {
      // Fall back to default pool.
      
      pool = IEngineObjectPool::DEFAULT;
      AssertFatal( pool, "EngineObject::new - No default pool set!" );
      ptr = pool->allocateObject( size TORQUE_TMM_LOC );
      
      if( !ptr )
      {
         Platform::AlertOK( "Torque Memory Error", "Out of memory. Shutting down.\n");
         Platform::forceShutdown( -1 );
      }
   }
   
   EngineObject* object = reinterpret_cast< EngineObject* >( ptr );
   object->mEngineObjectPool = pool;
   
   return ptr;
}
#endif

//-----------------------------------------------------------------------------

void* EngineObject::operator new( size_t size TORQUE_TMM_ARGS_DECL )
{
   AssertFatal( IEngineObjectPool::DEFAULT, "EngineObject::new - No default pool set!" );

   void* ptr = IEngineObjectPool::DEFAULT->allocateObject( size TORQUE_TMM_ARGS );
   if( !ptr )
   {
      Platform::AlertOK( "Torque Memory Error", "Out of memory. Shutting down.\n");
      Platform::forceShutdown( -1 );
   }
   
   EngineObject* object = reinterpret_cast< EngineObject* >( ptr );
   object->mEngineObjectPool = IEngineObjectPool::DEFAULT;
   
   return ptr;
}

//-----------------------------------------------------------------------------

void* EngineObject::operator new( size_t size, IEngineObjectPool* pool TORQUE_TMM_ARGS_DECL )
{
   AssertFatal( pool, "EngineObject::new - Got a NULL pool pointer!" );

   void* ptr = pool->allocateObject( size TORQUE_TMM_ARGS );
   if( !ptr )
   {
      // Fall back to default pool.
      
      pool = IEngineObjectPool::DEFAULT;
      AssertFatal( pool, "EngineObject::new - No default pool set!" );
      ptr = pool->allocateObject( size TORQUE_TMM_ARGS );
      
      if( !ptr )
      {
         Platform::AlertOK( "Torque Memory Error", "Out of memory. Shutting down.\n");
         Platform::forceShutdown( -1 );
      }
   }
   
   EngineObject* object = reinterpret_cast< EngineObject* >( ptr );
   object->mEngineObjectPool = pool;
   
   return ptr;
}

//-----------------------------------------------------------------------------

void EngineObject::operator delete( void* ptr )
{
   if( !ptr )
      return;

//   AssertWarn( false, "EngineObject::delete - Directly deleting an EngineObject is disallowed!" );
   
   EngineObject* object = reinterpret_cast< EngineObject* >( ptr );
   AssertFatal( !object->getRefCount(), "EngineObject::delete - object still referenced!" );
   if( object->mEngineObjectPool )
      object->mEngineObjectPool->freeObject( object );
}

//-----------------------------------------------------------------------------

#ifdef TORQUE_DEBUG

void EngineObject::debugDumpInstances()
{
   Con::printf( "----------- Dumping EngineObjects ----------------" );
   for( EngineObject* object = smFirstEngineObject; object != NULL; object = object->mNextEngineObject )
      Con::printf( object->describeSelf() );
   Con::printf( "%i EngineObjects", smNumEngineObjects );
}

//-----------------------------------------------------------------------------

void EngineObject::debugEnumInstances( const std::type_info& type, DebugEnumInstancesCallback callback )
{
   for( EngineObject* object = smFirstEngineObject; object != NULL; object = object->mNextEngineObject )
      if( typeid( *object ) == type )
      {
         // Set breakpoint here to break for each instance.
         if( callback )
            callback( object );
      }
}

//-----------------------------------------------------------------------------

void EngineObject::debugEnumInstances( const char* className, DebugEnumInstancesCallback callback )
{
   for( EngineObject* object = smFirstEngineObject; object != NULL; object = object->mNextEngineObject )
   {
      for( const EngineTypeInfo* type = TYPEOF( object ); type != NULL; type = type->getSuperType() )
         if( dStricmp( type->getTypeName(), className ) == 0 )
         {
            // Set breakpoint here to break for each instance.
            if( callback )
               callback( object );
               
            break;
         }
   }
}

#endif // TORQUE_DEBUG

//-----------------------------------------------------------------------------

void* EngineCRuntimeObjectPool::allocateObject( U32 size TORQUE_TMM_ARGS_DECL )
{
   #ifdef TORQUE_DISABLE_MEMORY_MANAGER
      return dMalloc( size );
   #else
      return dMalloc_r( size TORQUE_TMM_ARGS );
   #endif
}

//-----------------------------------------------------------------------------

void EngineCRuntimeObjectPool::freeObject( void* ptr )
{
   dFree( ptr );
}

//-----------------------------------------------------------------------------

StaticEngineObject::StaticEngineObject()
{
   mEngineObjectPool = NULL;
   
   // Add an artificial reference to the object.
   incRefCount();
}

//-----------------------------------------------------------------------------

void StaticEngineObject::destroySelf()
{
   AssertFatal( false, "StaticEngineObject::destroySelf - Cannot destroy static object!" );
}

//=============================================================================
//    API.
//=============================================================================
// MARK: ---- API ----

//-----------------------------------------------------------------------------

DefineNewEngineMethod( EngineObject, getType, const EngineTypeInfo*, (),,
   "Return the type descriptor for the type the object is an instance of.\n"
   "@return The type descriptor for the object's dynamic type." )
{
   return TYPEOF( object );
}

//-----------------------------------------------------------------------------

DefineNewEngineMethod( EngineObject, addRef, void, (),,
   "Increase the reference count of the given object.\n\n"
   "@param object An object." )
{
   object->incRefCount();
}

//-----------------------------------------------------------------------------

DefineNewEngineMethod( EngineObject, release, void, (),,
   "Decrease the reference count of the given object.  If the count drops to "
   "zero, the object will be deleted.\n\n"
   "@param object An object." )
{
   object->decRefCount();
}

//-----------------------------------------------------------------------------

DefineNewEngineMethod( EngineObject, getUserData, void*, (),,
   "Get the opaque user data pointer installed on the object.\n"
   "@return The user data pointer previously installed on the object; NULL by default." )
{
   return _USERDATA( object );
}

//-----------------------------------------------------------------------------

DefineNewEngineMethod( EngineObject, setUserData, void, ( void* ptr ),,
   "Install an opaque pointer on the object that the control layer can use to "
   "associate data with the object.\n"
   "@param ptr A pointer.\n" )
{
   _USERDATA( object ) = ptr;
}

//-----------------------------------------------------------------------------

#ifdef TORQUE_DEBUG

DefineEngineFunction( debugDumpAllObjects, void, (),,
   "@brief Dumps all current EngineObject instances to the console.\n\n"
   "@note This function is only available in debug builds.\n\n"
   "@ingroup Debugging" )
{
   EngineObject::debugDumpInstances();
}

#endif // TORQUE_DEBUG
