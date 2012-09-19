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

#include "platform/platform.h"
#include "core/resourceManager.h"

#include "core/volume.h"
#include "console/console.h"
#include "core/util/autoPtr.h"

#include "console/engineAPI.h"

static AutoPtr< ResourceManager > smInstance;

ResourceManager::ResourceManager()
:  mIterSigFilter( U32_MAX )
{
}

ResourceManager::~ResourceManager()
{
   // TODO: Dump resources that have not been released?
}

ResourceManager &ResourceManager::get()
{
   if ( smInstance.isNull() )
      smInstance = new ResourceManager;
   return *smInstance;
}

ResourceBase ResourceManager::load(const Torque::Path &path)
{
#ifdef TORQUE_DEBUG_RES_MANAGER
   Con::printf( "ResourceManager::load : [%s]", path.getFullPath().c_str() );
#endif

   ResourceHeaderMap::Iterator iter = mResourceHeaderMap.findOrInsert( path.getFullPath() );

   ResourceHeaderMap::Pair &pair = *iter;

   if ( pair.value == NULL )
   {
      pair.value = new ResourceBase::Header;

      // TODO: This can fail if the file doesn't exist 
      // at all which is possible.
      //
      // The problem is the templated design in ResourceManager
      // keeps me from checking to see if the resource load failed
      // before adding a notification.
      //
      // IMO the resource manager is overly templateized and
      // we should refactor it so that its not so.
      //
      FS::AddChangeNotification( path, this, &ResourceManager::notifiedFileChanged );
   }

   ResourceBase::Header *header = pair.value;

   if (header->getSignature() == 0)
     header->mPath = path;

   return ResourceBase( header );
}

ResourceBase ResourceManager::find(const Torque::Path &path)
{
#ifdef TORQUE_DEBUG_RES_MANAGER
   Con::printf( "ResourceManager::find : [%s]", path.getFullPath().c_str() );
#endif

   ResourceHeaderMap::Iterator iter = mResourceHeaderMap.find( path.getFullPath() );

   if ( iter == mResourceHeaderMap.end() )
      return ResourceBase();

   ResourceHeaderMap::Pair &pair = *iter;

   ResourceBase::Header	*header = pair.value;

   return ResourceBase(header);
}

#ifdef TORQUE_DEBUG
void ResourceManager::dumpToConsole()
{
   const U32   numResources = mResourceHeaderMap.size();

   if ( numResources == 0 )
   {
      Con::printf( "ResourceManager is not managing any resources" );
      return;
   }

   Con::printf( "ResourceManager is managing %d resources:", numResources );
   Con::printf( " [ref count/signature/path]" );

   ResourceHeaderMap::Iterator iter;

   for( iter = mResourceHeaderMap.begin(); iter != mResourceHeaderMap.end(); ++iter )
   {
      ResourceBase::Header	*header = (*iter).value;
            
      char fourCC[ 5 ];
      *( ( U32* ) fourCC ) = header->getSignature();
      fourCC[ 4 ] = 0;

      Con::printf( " %3d %s [%s] ", header->getRefCount(), fourCC, (*iter).key.c_str() );
   }
}
#endif

bool ResourceManager::remove( ResourceBase::Header* header )
{
   const Path &path = header->getPath();

#ifdef TORQUE_DEBUG_RES_MANAGER
   Con::printf( "ResourceManager::remove : [%s]", path.getFullPath().c_str() );
#endif

   ResourceHeaderMap::Iterator iter = mResourceHeaderMap.find( path.getFullPath() );
   if ( iter != mResourceHeaderMap.end() && iter->value == header )
   {
      AssertISV( header && (header->getRefCount() == 0), "ResourceManager error: trying to remove resource which is still in use." );
      mResourceHeaderMap.erase( iter );
   }
   else
   {
      iter = mPrevResourceHeaderMap.find( path.getFullPath() );
      if ( iter == mPrevResourceHeaderMap.end() || iter->value != header )
      {
         Con::errorf( "ResourceManager::remove : Trying to remove non-existent resource [%s]", path.getFullPath().c_str() );
         return false;
      }

      AssertISV( header && (header->getRefCount() == 0), "ResourceManager error: trying to remove resource which is still in use." );
      mPrevResourceHeaderMap.erase( iter );
   }

   FS::RemoveChangeNotification( path, this, &ResourceManager::notifiedFileChanged );

   return true;
}

void ResourceManager::notifiedFileChanged( const Torque::Path &path )
{
   reloadResource( path, true );
}

void ResourceManager::reloadResource( const Torque::Path &path, bool showMessage )
{
   if ( showMessage )
      Con::warnf( "[ResourceManager::notifiedFileChanged] : File changed [%s]", path.getFullPath().c_str() );

   ResourceHeaderMap::Iterator iter = mResourceHeaderMap.find( path.getFullPath() );
   if ( iter != mResourceHeaderMap.end() )
   {
      ResourceBase::Header	*header = (*iter).value;
      mResourceHeaderMap.erase( iter );

      // Move the resource into the previous resource map.
      iter = mPrevResourceHeaderMap.findOrInsert( path );
      iter->value = header;
   }
	
   // Now notify users of the resource change so they 
   // can release and reload.
   mChangeSignal.trigger( path );
}

ResourceBase ResourceManager::startResourceList( ResourceBase::Signature inSignature )
{
   mIter = mResourceHeaderMap.begin();

   mIterSigFilter = inSignature;

   return nextResource();
}

ResourceBase ResourceManager::nextResource()
{
   ResourceBase::Header	*header = NULL;

   while( mIter != mResourceHeaderMap.end() )
   {
      header = (*mIter).value;

      ++mIter;

      if ( mIterSigFilter == U32_MAX )
         return ResourceBase(header);

      if ( header->getSignature() == mIterSigFilter )
         return ResourceBase(header);
   }
   
   return ResourceBase();
}

ConsoleFunctionGroupBegin(ResourceManagerFunctions, "Resource management functions.");

#ifdef TORQUE_DEBUG
ConsoleFunction(resourceDump, void, 1, 1, "()"
				"@brief List the currently managed resources\n\n"
				"Currently used by editors only, internal\n"
				"@ingroup Editors\n"
				"@internal")
{
   ResourceManager::get().dumpToConsole();
}
#endif

DefineEngineFunction( reloadResource, void, ( const char* path ),,
   "Force the resource at specified input path to be reloaded\n"
   "@param path Path to the resource to be reloaded\n\n"
   "@tsexample\n"
   "reloadResource( \"art/shapes/box.dts\" );\n"
   "@endtsexample\n\n"
   "@note Currently used by editors only\n"
   "@ingroup Editors\n"
   "@internal")
{
   ResourceManager::get().reloadResource( path );
}

ConsoleFunctionGroupEnd( ResourceManagerFunctions );
