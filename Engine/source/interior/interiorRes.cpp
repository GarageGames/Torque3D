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
#include "interior/interiorRes.h"

#include "console/console.h"
#include "core/stream/fileStream.h"
#include "interior/interior.h"
#include "interior/interiorResObjects.h"
#include "gfx/bitmap/gBitmap.h"
#include "interior/forceField.h"


const U32 InteriorResource::smFileVersion = 44;

//--------------------------------------------------------------------------
InteriorResource::InteriorResource()
{
   VECTOR_SET_ASSOCIATION(mDetailLevels);
   VECTOR_SET_ASSOCIATION(mSubObjects);
   VECTOR_SET_ASSOCIATION(mTriggers);
   //VECTOR_SET_ASSOCIATION(mPaths);
   VECTOR_SET_ASSOCIATION(mInteriorPathFollowers);
   VECTOR_SET_ASSOCIATION(mForceFields);
   VECTOR_SET_ASSOCIATION(mAISpecialNodes);

   mPreviewBitmap = NULL;
}

InteriorResource::~InteriorResource()
{
   U32 i;

   for (i = 0; i < mDetailLevels.size(); i++)
      delete mDetailLevels[i];
   for (i = 0; i < mSubObjects.size(); i++)
      delete mSubObjects[i];
   for (i = 0; i < mTriggers.size(); i++)
      delete mTriggers[i];
   for (i = 0; i < mInteriorPathFollowers.size(); i++)
      delete mInteriorPathFollowers[i];
   for (i = 0; i < mForceFields.size(); i++)
      delete mForceFields[i];
   for (i = 0; i < mAISpecialNodes.size(); i++)
      delete mAISpecialNodes[i];
   for (i = 0; i < mGameEntities.size(); i++)
      delete mGameEntities[i];

   delete mPreviewBitmap;
   mPreviewBitmap = NULL;
}

bool InteriorResource::read(Stream& stream)
{
   AssertFatal(stream.hasCapability(Stream::StreamRead), "Interior::read: non-read capable stream passed");
   AssertFatal(stream.getStatus() == Stream::Ok, "Interior::read: Error, stream in inconsistent state");

   U32 i;

   // Version this stream
   U32 fileVersion;
   stream.read(&fileVersion);
   if (fileVersion != smFileVersion) {
      Con::errorf(ConsoleLogEntry::General, "InteriorResource::read: incompatible file version found.");
      return false;
   }

   // Handle preview
   bool previewIncluded = false;
   stream.read(&previewIncluded);
   if (previewIncluded) {
      GBitmap bmp;
      bmp.readBitmap("png",stream);
   }

   // Details
   U32 numDetailLevels;
   stream.read(&numDetailLevels);
   mDetailLevels.setSize(numDetailLevels);
   for (i = 0; i < mDetailLevels.size(); i++)
      mDetailLevels[i] = NULL;

   for (i = 0; i < mDetailLevels.size(); i++) {
      mDetailLevels[i] = new Interior;
      if (mDetailLevels[i]->read(stream) == false) {
         Con::errorf(ConsoleLogEntry::General, "Unable to read detail level %d in interior resource", i);
         return false;
      }
   }

   // Subobjects: mirrors, translucencies
   U32 numSubObjects;
   stream.read(&numSubObjects);
   mSubObjects.setSize(numSubObjects);
   for (i = 0; i < mSubObjects.size(); i++)
      mSubObjects[i] = NULL;

   for (i = 0; i < mSubObjects.size(); i++) {
      mSubObjects[i] = new Interior;
      if (mSubObjects[i]->read(stream) == false) {
         AssertISV(false, avar("Unable to read subobject %d in interior resource", i));
         return false;
      }
   }

   // Triggers
   U32 numTriggers;
   stream.read(&numTriggers);
   mTriggers.setSize(numTriggers);
   for (i = 0; i < mTriggers.size(); i++)
      mTriggers[i] = NULL;

   for (i = 0; i < mTriggers.size(); i++) {
      mTriggers[i] = new InteriorResTrigger;
      if (mTriggers[i]->read(stream) == false) {
         AssertISV(false, avar("Unable to read trigger %d in interior resource", i));
         return false;
      }
   }

   U32 numChildren;
   stream.read(&numChildren);
   mInteriorPathFollowers.setSize(numChildren);
   for (i = 0; i < mInteriorPathFollowers.size(); i++)
      mInteriorPathFollowers[i] = NULL;

   for (i = 0; i < mInteriorPathFollowers.size(); i++) {
      mInteriorPathFollowers[i] = new InteriorPathFollower;
      if (mInteriorPathFollowers[i]->read(stream) == false) {
         AssertISV(false, avar("Unable to read child %d in interior resource", i));
         return false;
      }
   }

   U32 numFields;
   stream.read(&numFields);
   mForceFields.setSize(numFields);
   for (i = 0; i < mForceFields.size(); i++)
      mForceFields[i] = NULL;

   for (i = 0; i < mForceFields.size(); i++) {
      mForceFields[i] = new ForceField;
      if (mForceFields[i]->read(stream) == false) {
         AssertISV(false, avar("Unable to read field %d in interior resource", i));
         return false;
      }
   }

   U32 numSpecNodes;
   stream.read(&numSpecNodes);
   mAISpecialNodes.setSize(numSpecNodes);
   for (i = 0; i < mAISpecialNodes.size(); i++)
      mAISpecialNodes[i] = NULL;

   for (i = 0; i < mAISpecialNodes.size(); i++) {
      mAISpecialNodes[i] = new AISpecialNode;
      if (mAISpecialNodes[i]->read(stream) == false) {
         AssertISV(false, avar("Unable to read SpecNode %d in interior resource", i));
         return false;
      }
   }

   U32 dummyInt;
   stream.read(&dummyInt);
   if (dummyInt == 1)
   {
      if (mDetailLevels.size() != 0)
         getDetailLevel(0)->readVehicleCollision(stream);
   }

   // For expansion purposes
   stream.read(&dummyInt);
   if(dummyInt == 2)
   {
      U32 numGameEnts;
      stream.read(&numGameEnts);
      mGameEntities.setSize(numGameEnts);
      for (i = 0; i < numGameEnts; i++)
         mGameEntities[i] = new ItrGameEntity;

      for (i = 0; i < numGameEnts; i++) {
         if (mGameEntities[i]->read(stream) == false) {
            AssertISV(false, avar("Unable to read SpecNode %d in interior resource", i));
            return false;
         }
      }
      stream.read(&dummyInt);
   }

   return (stream.getStatus() == Stream::Ok);
}

bool InteriorResource::write(Stream& stream) const
{
   AssertFatal(stream.hasCapability(Stream::StreamWrite), "Interior::write: non-write capable stream passed");
   AssertFatal(stream.getStatus() == Stream::Ok, "Interior::write: Error, stream in inconsistent state");

   // Version the stream
   stream.write(smFileVersion);

   // Handle preview
   //
   if (mPreviewBitmap != NULL) {
      stream.write(bool(true));
      mPreviewBitmap->writeBitmap("png",stream);
   } else {
      stream.write(bool(false));
   }

   // Write out the interiors
   stream.write(mDetailLevels.size());
   U32 i;
   for (i = 0; i < mDetailLevels.size(); i++) {
      if (mDetailLevels[i]->write(stream) == false) {
         AssertISV(false, "Unable to write detail level to stream");
         return false;
      }
   }

   stream.write(mSubObjects.size());
   for (i = 0; i < mSubObjects.size(); i++) {
      if (mSubObjects[i]->write(stream) == false) {
         AssertISV(false, "Unable to write subobject to stream");
         return false;
      }
   }

   stream.write(mTriggers.size());
   for (i = 0; i < mTriggers.size(); i++) {
      if (mTriggers[i]->write(stream) == false) {
         AssertISV(false, "Unable to write trigger to stream");
         return false;
      }
   }

   stream.write(mInteriorPathFollowers.size());
   for (i = 0; i < mInteriorPathFollowers.size(); i++) {
      if (mInteriorPathFollowers[i]->write(stream) == false) {
         AssertISV(false, avar("Unable to write child %d in interior resource", i));
         return false;
      }
   }

   stream.write(mForceFields.size());
   for (i = 0; i < mForceFields.size(); i++) {
      if (mForceFields[i]->write(stream) == false) {
         AssertISV(false, avar("Unable to write field %d in interior resource", i));
         return false;
      }
   }

   stream.write(mAISpecialNodes.size());
   for (i = 0; i < mAISpecialNodes.size(); i++) {
      if (mAISpecialNodes[i]->write(stream) == false) {
         AssertISV(false, avar("Unable to write SpecNode %d in interior resource", i));
         return false;
      }
   }

   stream.write(U32(1));
   if (mDetailLevels.size() != 0)
      const_cast<Interior*>(mDetailLevels[0])->writeVehicleCollision(stream);

   // For expansion purposes
   if (mGameEntities.size())
   {
      stream.write(U32(2));
      stream.write(mGameEntities.size());
      for(i = 0; i < mGameEntities.size(); i++)
      {
         if (mGameEntities[i]->write(stream) == false) {
            AssertISV(false, avar("Unable to write GameEnt %d in interior resource", i));
            return false;
         }
      }
   }
   stream.write(U32(0));

   return (stream.getStatus() == Stream::Ok);
}

GBitmap* InteriorResource::extractPreview(Stream& stream)
{
   AssertFatal(stream.hasCapability(Stream::StreamRead), "Interior::read: non-read capable stream passed");
   AssertFatal(stream.getStatus() == Stream::Ok, "Interior::read: Error, stream in inconsistent state");

   // Version this stream
   U32 fileVersion;
   stream.read(&fileVersion);
   if (fileVersion != smFileVersion) {
      Con::errorf(ConsoleLogEntry::General, "InteriorResource::read: incompatible file version found.");
      return NULL;
   }

   // Handle preview
   bool previewIncluded = false;
   stream.read(&previewIncluded);
   if (previewIncluded) {
      GBitmap* pBmp = new GBitmap;
      if (pBmp->readBitmap("png",stream) == true)
         return pBmp;

      delete pBmp;
   }

   return NULL;
}

//------------------------------------------------------------------------------
//-------------------------------------- Interior Resource constructor
template<> void *Resource<InteriorResource>::create(const Torque::Path &path)
{
   FileStream  stream;

   stream.open( path.getFullPath(), Torque::FS::File::Read );

   if ( stream.getStatus() != Stream::Ok )
      return NULL;

   InteriorResource* pResource = new InteriorResource;

   if (pResource->read(stream) == true)
      return pResource;
   else
   {
      delete pResource;
      return NULL;
   }
}

template<> ResourceBase::Signature  Resource<InteriorResource>::signature()
{
   return MakeFourCC('t','d','i','f');
}
