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

#ifndef _INTERIORRES_H_
#define _INTERIORRES_H_

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif


class Stream;
class Interior;
class GBitmap;
class InteriorResTrigger;
class InteriorPath;
class InteriorPathFollower;
class ForceField;
class AISpecialNode;
class ItrGameEntity;

class InteriorResource
{
   static const U32 smFileVersion;

  protected:
   Vector<Interior*>             mDetailLevels;
   Vector<Interior*>             mSubObjects;
   Vector<InteriorResTrigger*>   mTriggers;
   Vector<InteriorPathFollower*> mInteriorPathFollowers;
   Vector<ForceField*>           mForceFields;
   Vector<AISpecialNode*>        mAISpecialNodes;
   Vector<ItrGameEntity*>           mGameEntities;

   GBitmap* mPreviewBitmap;

  public:
   InteriorResource();
   ~InteriorResource();

   bool            read(Stream& stream);
   bool            write(Stream& stream) const;
   static GBitmap* extractPreview(Stream&);

   S32       getNumDetailLevels() const;
   S32       getNumSubObjects() const;
   S32       getNumTriggers() const;
   S32       getNumInteriorPathFollowers() const;
   S32       getNumForceFields() const;
   S32       getNumSpecialNodes() const;
   S32       getNumGameEntities() const;

   Interior*             getDetailLevel(const U32);
   Interior*             getSubObject(const U32);
   InteriorResTrigger*   getTrigger(const U32);
   InteriorPathFollower* getInteriorPathFollower(const U32);
   ForceField*           getForceField(const U32);
   AISpecialNode*        getSpecialNode(const U32);
   ItrGameEntity*        getGameEntity(const U32);
};

//--------------------------------------------------------------------------
inline S32 InteriorResource::getNumDetailLevels() const
{
   return mDetailLevels.size();
}

inline S32 InteriorResource::getNumSubObjects() const
{
   return mSubObjects.size();
}

inline S32 InteriorResource::getNumTriggers() const
{
   return mTriggers.size();
}

inline S32 InteriorResource::getNumSpecialNodes() const
{
   return mAISpecialNodes.size();
}

inline S32 InteriorResource::getNumGameEntities() const
{
   return mGameEntities.size();
}

inline S32 InteriorResource::getNumInteriorPathFollowers() const
{
   return mInteriorPathFollowers.size();
}

inline S32 InteriorResource::getNumForceFields() const
{
   return mForceFields.size();
}

inline Interior* InteriorResource::getDetailLevel(const U32 idx)
{
   AssertFatal(idx < getNumDetailLevels(), "Error, out of bounds detail level!");

   if (idx < getNumDetailLevels())
      return mDetailLevels[idx];
   else
      return NULL;
}

inline Interior* InteriorResource::getSubObject(const U32 idx)
{
   AssertFatal(idx < getNumSubObjects(), "Error, out of bounds subObject!");

   return mSubObjects[idx];
}

inline InteriorResTrigger* InteriorResource::getTrigger(const U32 idx)
{
   AssertFatal(idx < getNumTriggers(), "Error, out of bounds trigger!");

   return mTriggers[idx];
}

inline InteriorPathFollower* InteriorResource::getInteriorPathFollower(const U32 idx)
{
   AssertFatal(idx < getNumInteriorPathFollowers(), "Error, out of bounds path follower!");

   return mInteriorPathFollowers[idx];
}

inline ForceField* InteriorResource::getForceField(const U32 idx)
{
   AssertFatal(idx < getNumForceFields(), "Error, out of bounds force field!");

   return mForceFields[idx];
}

inline AISpecialNode* InteriorResource::getSpecialNode(const U32 idx)
{
   AssertFatal(idx < getNumSpecialNodes(), "Error, out of bounds Special Nodes!");

   return mAISpecialNodes[idx];
}

inline ItrGameEntity* InteriorResource::getGameEntity(const U32 idx)
{
   AssertFatal(idx < getNumGameEntities(), "Error, out of bounds Game ENts!");

   return mGameEntities[idx];
}

#endif  // _H_INTERIORRES_

