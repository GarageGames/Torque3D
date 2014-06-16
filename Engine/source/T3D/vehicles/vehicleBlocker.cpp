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
#include "T3D/vehicles/vehicleBlocker.h"

#include "core/stream/bitStream.h"
#include "scene/sceneRenderState.h"
#include "scene/sceneManager.h"
#include "math/mathIO.h"
#include "console/consoleTypes.h"

IMPLEMENT_CO_NETOBJECT_V1(VehicleBlocker);

ConsoleDocClass( VehicleBlocker,
   "@brief Legacy class from Tribes, originally used for blocking Vehicle objects.\n\n"

   "@note This is no longer useful and should be deprecated soon.\n\n"

   "@internal\n"
);

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
VehicleBlocker::VehicleBlocker()
{
   mNetFlags.set(Ghostable | ScopeAlways);

   mTypeMask = VehicleBlockerObjectType;

   mConvexList = new Convex;
}

VehicleBlocker::~VehicleBlocker()
{
   delete mConvexList;
   mConvexList = NULL;
}

//--------------------------------------------------------------------------
void VehicleBlocker::initPersistFields()
{
   addField("dimensions", TypePoint3F, Offset(mDimensions, VehicleBlocker));
   Parent::initPersistFields();
}

//--------------------------------------------------------------------------
bool VehicleBlocker::onAdd()
{
   if(!Parent::onAdd())
      return false;

   mObjBox.minExtents.set(-mDimensions.x, -mDimensions.y, 0);
   mObjBox.maxExtents.set( mDimensions.x,  mDimensions.y, mDimensions.z);
   if( !mObjBox.isValidBox() )
   {
      Con::errorf("VehicleBlocker::onAdd - Fail - No valid object box");
      return false;
   }

   resetWorldBox();
   setRenderTransform(mObjToWorld);

   addToScene();

   return true;
}


void VehicleBlocker::onRemove()
{
   mConvexList->nukeList();
   removeFromScene();

   Parent::onRemove();
}


U32 VehicleBlocker::packUpdate(NetConnection *con, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   mathWrite(*stream, getTransform());
   mathWrite(*stream, getScale());
   mathWrite(*stream, mDimensions);

   return retMask;
}


void VehicleBlocker::unpackUpdate(NetConnection *con, BitStream *stream)
{
   Parent::unpackUpdate(con, stream);

   MatrixF mat;
   Point3F scale;
   Box3F objBox;
   mathRead(*stream, &mat);
   mathRead(*stream, &scale);
   mathRead(*stream, &mDimensions);
   mObjBox.minExtents.set(-mDimensions.x, -mDimensions.y, 0);
   mObjBox.maxExtents.set( mDimensions.x,  mDimensions.y, mDimensions.z);
   setScale(scale);
   setTransform(mat);
}


void VehicleBlocker::buildConvex(const Box3F& box, Convex* convex)
{
   // These should really come out of a pool
   mConvexList->collectGarbage();

   if (box.isOverlapped(getWorldBox()) == false)
      return;

   // Just return a box convex for the entire shape...
   Convex* cc = 0;
   CollisionWorkingList& wl = convex->getWorkingList();
   for (CollisionWorkingList* itr = wl.wLink.mNext; itr != &wl; itr = itr->wLink.mNext) 
   {
      if (itr->mConvex->getType() == BoxConvexType &&
          itr->mConvex->getObject() == this) {
         cc = itr->mConvex;
         break;
      }
   }
   if (cc)
      return;

   // Create a new convex.
   BoxConvex* cp = new BoxConvex;
   mConvexList->registerObject(cp);
   convex->addToWorkingList(cp);
   cp->init(this);

   mObjBox.getCenter(&cp->mCenter);
   cp->mSize.x = mObjBox.len_x() / 2.0f;
   cp->mSize.y = mObjBox.len_y() / 2.0f;
   cp->mSize.z = mObjBox.len_z() / 2.0f;
}

