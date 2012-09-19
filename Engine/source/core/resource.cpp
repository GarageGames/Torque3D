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

#include "core/resourceManager.h"
#include "core/volume.h"

#include "console/console.h"


FreeListChunker<ResourceHolderBase> ResourceHolderBase::smHolderFactory;

ResourceBase::Header ResourceBase::smBlank;


U32   ResourceBase::Header::getChecksum() const
{
   Torque::FS::FileNodeRef fileRef = Torque::FS::GetFileNode( mPath );

   if ( fileRef == NULL )
   {
      Con::errorf("ResourceBase::getChecksum could not access file: [%s]", mPath.getFullPath().c_str() );
      return 0;
   }

   return fileRef->getChecksum();
}

void ResourceBase::Header::destroySelf()
{
   if (this == &smBlank)
      return;
      
   if( mNotifyUnload )
      mNotifyUnload( getPath(), getResource() );

   if ( mResource != NULL )
   {
      mResource->~ResourceHolderBase();
      ResourceHolderBase::smHolderFactory.free( mResource );
   }

   ResourceManager::get().remove( this );
   delete this;
}

void  ResourceBase::assign(const ResourceBase &inResource, void* resource)
{
   mResourceHeader = inResource.mResourceHeader;

   if ( mResourceHeader == NULL || mResourceHeader.getPointer() == &(ResourceBase::smBlank) )
      return;

   if (mResourceHeader->getSignature())
   {
      AssertFatal(inResource.mResourceHeader->getSignature() == getSignature(),"Resource::assign: mis-matching signature");
   }
   else
   {
      mResourceHeader->mSignature = getSignature();

      const Torque::Path   path = mResourceHeader->getPath();

      if (resource == NULL)
      {
         if ( !getStaticLoadSignal().trigger(path, &resource) && (resource != NULL) )
         {
            mResourceHeader->mResource = createHolder(resource);
            mResourceHeader->mNotifyUnload = _getNotifyUnloadFn();
            _triggerPostLoadSignal();
            return;
         }

         resource = create(path);
      }

      if (resource)
      {
         mResourceHeader->mResource = createHolder(resource);
         mResourceHeader->mNotifyUnload = _getNotifyUnloadFn();
         _triggerPostLoadSignal();
      }
      else
      {
         // Failed to create...delete signature so we can attempt to successfully create resource later
         Con::warnf("Failed to create resource: [%s]", path.getFullPath().c_str() );

         mResourceHeader->mSignature = 0;
      }
   }
}

