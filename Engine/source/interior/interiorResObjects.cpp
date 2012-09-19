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

#include "interior/interiorResObjects.h"
#include "core/stream/stream.h"
#include "math/mathIO.h"

//--------------------------------------------------------------------------
//--------------------------------------
//

void InteriorDict::read(Stream &stream)
{
   U32 sz;
   stream.read(&sz);
   setSize(sz);
   for(U32 i = 0; i < sz; i++)
   {
      InteriorDictEntry e;
      stream.readString(e.name);
      stream.readString(e.value);
      (*this)[i] = e;
   }
}

void InteriorDict::write(Stream &stream) const
{
   U32 sz = size();
   stream.write(sz);
   for(U32 i = 0; i < sz; i++)
   {
      stream.writeString((*this)[i].name);
      stream.writeString((*this)[i].value);
   }
}

bool InteriorResTrigger::read(Stream& stream)
{
   U32 i, size;
   stream.readString(mName);
   mDataBlock = stream.readSTString();
   mDictionary.read(stream);

   // Read the polyhedron
   stream.read(&size);
   mPolyhedron.pointList.setSize(size);
   for (i = 0; i < mPolyhedron.pointList.size(); i++)
      mathRead(stream, &mPolyhedron.pointList[i]);

   stream.read(&size);
   mPolyhedron.planeList.setSize(size);
   for (i = 0; i < mPolyhedron.planeList.size(); i++)
      mathRead(stream, &mPolyhedron.planeList[i]);

   stream.read(&size);
   mPolyhedron.edgeList.setSize(size);
   for (i = 0; i < mPolyhedron.edgeList.size(); i++) {
      Polyhedron::Edge& rEdge = mPolyhedron.edgeList[i];

      stream.read(&rEdge.face[0]);
      stream.read(&rEdge.face[1]);
      stream.read(&rEdge.vertex[0]);
      stream.read(&rEdge.vertex[1]);
   }

   // And the offset
   mathRead(stream, &mOffset);

   return (stream.getStatus() == Stream::Ok);
}

bool InteriorResTrigger::write(Stream& stream) const
{
   U32 i;

   stream.writeString(mName);
   stream.writeString(mDataBlock);
   mDictionary.write(stream);

   // Write the polyhedron
   stream.write(mPolyhedron.pointList.size());
   for (i = 0; i < mPolyhedron.pointList.size(); i++)
      mathWrite(stream, mPolyhedron.pointList[i]);

   stream.write(mPolyhedron.planeList.size());
   for (i = 0; i < mPolyhedron.planeList.size(); i++)
      mathWrite(stream, mPolyhedron.planeList[i]);

   stream.write(mPolyhedron.edgeList.size());
   for (i = 0; i < mPolyhedron.edgeList.size(); i++) {
      const Polyhedron::Edge& rEdge = mPolyhedron.edgeList[i];

      stream.write(rEdge.face[0]);
      stream.write(rEdge.face[1]);
      stream.write(rEdge.vertex[0]);
      stream.write(rEdge.vertex[1]);
   }

   // And the offset
   mathWrite(stream, mOffset);

   return (stream.getStatus() == Stream::Ok);
}

InteriorPathFollower::InteriorPathFollower()
{
   VECTOR_SET_ASSOCIATION( mTriggerIds );
   VECTOR_SET_ASSOCIATION( mWayPoints );

   mName      = "";
   mPathIndex = 0;
   mOffset.set(0, 0, 0);
}

InteriorPathFollower::~InteriorPathFollower()
{

}

bool InteriorPathFollower::read(Stream& stream)
{
   mName = stream.readSTString();
   mDataBlock = stream.readSTString();
   stream.read(&mInteriorResIndex);
   mathRead(stream, &mOffset);
   mDictionary.read(stream);

   U32 numTriggers;
   stream.read(&numTriggers);
   mTriggerIds.setSize(numTriggers);
   for (U32 i = 0; i < mTriggerIds.size(); i++)
      stream.read(&mTriggerIds[i]);

   U32 numWayPoints;
   stream.read(&numWayPoints);
   mWayPoints.setSize(numWayPoints);
   for(U32 i = 0; i < numWayPoints; i++)
   {
      mathRead(stream, &mWayPoints[i].pos);
      mathRead(stream, &mWayPoints[i].rot);
      stream.read(&mWayPoints[i].msToNext);
      stream.read(&mWayPoints[i].smoothingType);
   }
   stream.read(&mTotalMS);
   return (stream.getStatus() == Stream::Ok);
}

bool InteriorPathFollower::write(Stream& stream) const
{
   stream.writeString(mName);
   stream.writeString(mDataBlock);
   stream.write(mInteriorResIndex);
   mathWrite(stream, mOffset);
   mDictionary.write(stream);

   stream.write(mTriggerIds.size());
   for (U32 i = 0; i < mTriggerIds.size(); i++)
      stream.write(mTriggerIds[i]);

   stream.write(U32(mWayPoints.size()));
   for (U32 i = 0; i < mWayPoints.size(); i++) {
      mathWrite(stream, mWayPoints[i].pos);
      mathWrite(stream, mWayPoints[i].rot);
      stream.write(mWayPoints[i].msToNext);
      stream.write(mWayPoints[i].smoothingType);
   }
   stream.write(mTotalMS);

   return (stream.getStatus() == Stream::Ok);
}

AISpecialNode::AISpecialNode()
{
   mName = "";
   mPos.set(0, 0, 0);
}

AISpecialNode::~AISpecialNode()
{
}

bool AISpecialNode::read(Stream& stream)
{
   mName = stream.readSTString();
   mathRead(stream, &mPos);

   return (stream.getStatus() == Stream::Ok);
}

bool AISpecialNode::write(Stream& stream) const
{
   stream.writeString(mName);
   mathWrite(stream, mPos);

   return (stream.getStatus() == Stream::Ok);
}

ItrGameEntity::ItrGameEntity()
{
   mDataBlock = "";
   mGameClass = "";
   mPos.set(0, 0, 0);
}

ItrGameEntity::~ItrGameEntity()
{
}

bool ItrGameEntity::read(Stream& stream)
{
   mDataBlock = stream.readSTString();
   mGameClass = stream.readSTString();
   mathRead(stream, &mPos);
   mDictionary.read(stream);

   return (stream.getStatus() == Stream::Ok);
}

bool ItrGameEntity::write(Stream& stream) const
{
   stream.writeString(mDataBlock);
   stream.writeString(mGameClass);
   mathWrite(stream, mPos);
   mDictionary.write(stream);

   return (stream.getStatus() == Stream::Ok);
}
