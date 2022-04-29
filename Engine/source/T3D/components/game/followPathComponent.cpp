#include "T3D/components/game/followPathComponent.h"

IMPLEMENT_CO_DATABLOCK_V1(FollowPathComponent);

IMPLEMENT_CALLBACK(FollowPathComponent, onNode, void, (S32 node), (node),
   "A script callback that indicates the path camera has arrived at a specific node in its path.  Server side only.\n"
   "@param Node Unique ID assigned to this node.\n");

FollowPathComponent::FollowPathComponent() 
{
   delta.time = 0;
   delta.timeVec = 0;

   //mDataBlock = 0;
   mState = Forward;
   mNodeBase = 0;
   mNodeCount = 0;
   mPosition = 0;
   mTarget = 0;
   mTargetSet = false;
}

FollowPathComponent::~FollowPathComponent() 
{

}

bool FollowPathComponent::onAdd()
{
   if (!Parent::onAdd())
      return false;

   /*// Initialize from the current transform.
   if (!mNodeCount) 
   {
      QuatF rot(getTransform());
      Point3F pos = getPosition();
      mSpline.removeAll();
      mSpline.push_back(new CameraSpline::Knot(pos, rot, 1,
         CameraSpline::Knot::NORMAL, CameraSpline::Knot::SPLINE));
      mNodeCount = 1;
   }

   //
   mObjBox.maxExtents = mObjScale;
   mObjBox.minExtents = mObjScale;
   mObjBox.minExtents.neg();
   resetWorldBox();*/

   return true;
}
void FollowPathComponent::onRemove() 
{
   Parent::onRemove();
}

void FollowPathComponent::initPersistFields()
{
   Parent::initPersistFields();
}

void FollowPathComponent::componentAddedToOwner(Component *comp) 
{
   Parent::componentAddedToOwner(comp);
}

void FollowPathComponent::componentRemovedFromOwner(Component *comp)
{
   Parent::componentRemovedFromOwner(comp);
}

void FollowPathComponent::processTick() 
{
   Parent::processTick();

   // Move to new time
   advancePosition(TickMs);

   // Set new position
   MatrixF mat;
   interpolateMat(mPosition, &mat);
   mOwner->setTransform(mat);

   mOwner->updateContainer();
}

void FollowPathComponent::interpolateTick(F32 dt) 
{
   Parent::interpolateTick(dt);

   MatrixF mat;
   interpolateMat(delta.time + (delta.timeVec * dt), &mat);
   mOwner->setRenderTransform(mat);
}

void FollowPathComponent::interpolateMat(F32 pos, MatrixF* mat)
{
   /*CameraSpline::Knot knot;
   mSpline.value(pos - mNodeBase, &knot);
   knot.mRotation.setMatrix(mat);
   mat->setPosition(knot.mPosition);*/
}

void FollowPathComponent::advanceTime(F32 dt) 
{
   Parent::advanceTime(dt);
}

void FollowPathComponent::advancePosition(S32 ms)
{
   /*delta.timeVec = mPosition;

   // Advance according to current speed
   if (mState == Forward) {
      mPosition = mSpline.advanceTime(mPosition - mNodeBase, ms);
      if (mPosition > F32(mNodeCount - 1))
         mPosition = F32(mNodeCount - 1);
      mPosition += (F32)mNodeBase;
      if (mTargetSet && mPosition >= mTarget) {
         mTargetSet = false;
         mPosition = mTarget;
         mState = Stop;
      }
   }
   else
      if (mState == Backward) {
         mPosition = mSpline.advanceTime(mPosition - mNodeBase, -ms);
         if (mPosition < 0)
            mPosition = 0;
         mPosition += mNodeBase;
         if (mTargetSet && mPosition <= mTarget) {
            mTargetSet = false;
            mPosition = mTarget;
            mState = Stop;
         }
      }

   // Script callbacks
   if (int(mPosition) != int(delta.timeVec))
      onNode(int(mPosition));

   // Set frame interpolation
   delta.time = mPosition;
   delta.timeVec -= mPosition;*/
}


//----------------------------------------------------------------------------

/*void FollowPathComponent::getCameraTransform(F32* pos, MatrixF* mat)
{
   // Overide the ShapeBase method to skip all the first/third person support.
   getRenderEyeTransform(mat);

   // Apply Camera FX.
   mat->mul(gCamFXMgr.getTrans());
}*/


//----------------------------------------------------------------------------

void FollowPathComponent::setPosition(F32 pos)
{
   mPosition = mClampF(pos, (F32)mNodeBase, (F32)(mNodeBase + mNodeCount - 1));
   MatrixF mat;
   interpolateMat(mPosition, &mat);
   mOwner->setTransform(mat);
   setMaskBits(PositionMask);
}

void FollowPathComponent::setTarget(F32 pos)
{
   mTarget = pos;
   mTargetSet = true;
   if (mTarget > mPosition)
      mState = Forward;
   else
      if (mTarget < mPosition)
         mState = Backward;
      else {
         mTargetSet = false;
         mState = Stop;
      }
   setMaskBits(TargetMask | StateMask);
}

void FollowPathComponent::setState(State s)
{
   mState = s;
   setMaskBits(StateMask);
}


//-----------------------------------------------------------------------------

void FollowPathComponent::reset(F32 speed)
{
   /*CameraSpline::Knot *knot = new CameraSpline::Knot;
   mSpline.value(mPosition - mNodeBase, knot);
   if (speed)
      knot->mSpeed = speed;
   mSpline.removeAll();
   mSpline.push_back(knot);

   mNodeBase = 0;
   mNodeCount = 1;
   mPosition = 0;
   mTargetSet = false;
   mState = Forward;
   setMaskBits(StateMask | PositionMask | WindowMask | TargetMask);*/
}

/*void FollowPathComponent::pushBack(CameraSpline::Knot *knot)
{
   // Make room at the end
   if (mNodeCount == NodeWindow) {
      delete mSpline.remove(mSpline.getKnot(0));
      mNodeBase++;
   }
   else
      mNodeCount++;

   // Fill in the new node
   mSpline.push_back(knot);
   setMaskBits(WindowMask);

   // Make sure the position doesn't fall off
   if (mPosition < mNodeBase) {
      mPosition = (F32)mNodeBase;
      setMaskBits(PositionMask);
   }
}

void FollowPathComponent::pushFront(CameraSpline::Knot *knot)
{
   // Make room at the front
   if (mNodeCount == NodeWindow)
      delete mSpline.remove(mSpline.getKnot(mNodeCount));
   else
      mNodeCount++;
   mNodeBase--;

   // Fill in the new node
   mSpline.push_front(knot);
   setMaskBits(WindowMask);

   // Make sure the position doesn't fall off
   if (mPosition > F32(mNodeBase + (NodeWindow - 1)))
   {
      mPosition = F32(mNodeBase + (NodeWindow - 1));
      setMaskBits(PositionMask);
   }
}*/

void FollowPathComponent::popFront()
{
   /*if (mNodeCount < 2)
      return;

   // Remove the first node. Node base and position are unaffected.
   mNodeCount--;
   delete mSpline.remove(mSpline.getKnot(0));

   if (mPosition > 0)
      mPosition--;*/
}


//----------------------------------------------------------------------------

void FollowPathComponent::onNode(S32 node)
{
   //if (!isGhost())
   //   onNode_callback(node);

}

/*U32 FollowPathComponent::packUpdate(NetConnection *con, U32 mask, BitStream *stream)
{
   Parent::packUpdate(con, mask, stream);

   if (stream->writeFlag(mask & StateMask))
      stream->writeInt(mState, StateBits);

   if (stream->writeFlag(mask & PositionMask))
      stream->write(mPosition);

   if (stream->writeFlag(mask & TargetMask))
      if (stream->writeFlag(mTargetSet))
         stream->write(mTarget);

   if (stream->writeFlag(mask & WindowMask)) {
      stream->write(mNodeBase);
      stream->write(mNodeCount);
      for (S32 i = 0; i < mNodeCount; i++) {
         CameraSpline::Knot *knot = mSpline.getKnot(i);
         mathWrite(*stream, knot->mPosition);
         mathWrite(*stream, knot->mRotation);
         stream->write(knot->mSpeed);
         stream->writeInt(knot->mType, CameraSpline::Knot::NUM_TYPE_BITS);
         stream->writeInt(knot->mPath, CameraSpline::Knot::NUM_PATH_BITS);
      }
   }

   // The rest of the data is part of the control object packet update.
   // If we're controlled by this client, we don't need to send it.
   if (stream->writeFlag(getControllingClient() == con && !(mask & InitialUpdateMask)))
      return 0;

   return 0;
}

void FollowPathComponent::unpackUpdate(NetConnection *con, BitStream *stream)
{
   Parent::unpackUpdate(con, stream);

   // StateMask
   if (stream->readFlag())
      mState = stream->readInt(StateBits);

   // PositionMask
   if (stream->readFlag())
   {
      stream->read(&mPosition);
      delta.time = mPosition;
      delta.timeVec = 0;
   }

   // TargetMask
   if (stream->readFlag())
   {
      mTargetSet = stream->readFlag();
      if (mTargetSet)
         stream->read(&mTarget);
   }

   // WindowMask
   if (stream->readFlag())
   {
      mSpline.removeAll();
      stream->read(&mNodeBase);
      stream->read(&mNodeCount);
      for (S32 i = 0; i < mNodeCount; i++)
      {
         CameraSpline::Knot *knot = new CameraSpline::Knot();
         mathRead(*stream, &knot->mPosition);
         mathRead(*stream, &knot->mRotation);
         stream->read(&knot->mSpeed);
         knot->mType = (CameraSpline::Knot::Type)stream->readInt(CameraSpline::Knot::NUM_TYPE_BITS);
         knot->mPath = (CameraSpline::Knot::Path)stream->readInt(CameraSpline::Knot::NUM_PATH_BITS);
         mSpline.push_back(knot);
      }
   }

   // Controlled by the client?
   if (stream->readFlag())
      return;

}*/