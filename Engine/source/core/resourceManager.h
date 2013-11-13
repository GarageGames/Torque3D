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

#ifndef _RESOURCEMANAGER_H_
#define _RESOURCEMANAGER_H_

#ifndef __RESOURCE_H__
#include "core/resource.h"
#endif

#ifndef _TDICTIONARY_H_
#include "core/util/tDictionary.h"
#endif

class ResourceManager
{
public:

   static ResourceManager &get();

   ResourceBase load(const Torque::Path &path);
   ResourceBase find(const Torque::Path &path);

   ResourceBase startResourceList( ResourceBase::Signature inSignature = U32_MAX );
   ResourceBase nextResource();

   void reloadResource( const Torque::Path &path, bool showMessage = false );

   typedef Signal<void(const Torque::Path &path)> ChangedSignal;

   /// Registering with this signal will give an opportunity to handle a change to the
   /// resource on disk.  For example, if a PNG file is edited by the artist and saved
   /// the ResourceManager will signal that the file has changed so the TextureManager
   /// may act appropriately - which probably means to re-init the materials using that PNG.
   /// The signal passes the Resource's signature so the callee may filter these.
   ChangedSignal &getChangedSignal() { return mChangeSignal; }

#ifdef TORQUE_DEBUG
   void  dumpToConsole();
#endif

   ~ResourceManager();

protected:

   friend class ResourceBase::Header;

   ResourceManager();

   bool remove( ResourceBase::Header* header );

   void  notifiedFileChanged( const Torque::Path &path );

   typedef HashTable<String,ResourceBase::Header*> ResourceHeaderMap;

   /// The map of resources.
   ResourceHeaderMap mResourceHeaderMap;

   /// The map of old resources which have been replaced by
   /// new resources from a file change notification.
   ResourceHeaderMap mPrevResourceHeaderMap;

   ResourceHeaderMap::Iterator mIter;

   U32 mIterSigFilter;

   ChangedSignal mChangeSignal;
};

#endif
