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
#include "math/mMath.h"
#include "math/mathIO.h"
#include "console/simBase.h"
#include "console/console.h"
#include "console/consoleTypes.h"
#include "core/stream/bitStream.h"
#include "core/dnet.h"
#include "scene/pathManager.h"
#include "app/game.h"
#include "T3D/gameBase/gameConnection.h"
#include "T3D/fx/cameraFXMgr.h"
#include "console/engineAPI.h"
#include "math/mTransform.h"

#include "T3D/pathCamera.h"


//----------------------------------------------------------------------------

IMPLEMENT_CO_DATABLOCK_V1(PathCameraData);

ConsoleDocClass( PathCameraData,
   "@brief General interface to control a PathCamera object from the script level.\n"
   "@see PathCamera\n"
	"@tsexample\n"
		"datablock PathCameraData(LoopingCam)\n"
		"	{\n"
		"		mode = \"\";\n"
		"	};\n"
	"@endtsexample\n"
   "@ingroup PathCameras\n"
   "@ingroup Datablocks\n"
);

void PathCameraData::consoleInit()
{
}

void PathCameraData::initPersistFields()
{
   Parent::initPersistFields();
}

void PathCameraData::packData(BitStream* stream)
{
   Parent::packData(stream);
}

void PathCameraData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);
}


//----------------------------------------------------------------------------

IMPLEMENT_CO_NETOBJECT_V1(PathCamera);

ConsoleDocClass( PathCamera,
   "@brief A camera that moves along a path. The camera can then be made to travel along this path forwards or backwards.\n\n"

   "A camera's path is made up of knots, which define a position, rotation and speed for the camera.  Traversal from one knot to "
   "another may be either linear or using a Catmull-Rom spline.  If the knot is part of a spline, then it may be a normal knot "
   "or defined as a kink.  Kinked knots are a hard transition on the spline rather than a smooth one.  A knot may also be defined "
   "as a position only.  In this case the knot is treated as a normal knot but is ignored when determining how to smoothly rotate "
   "the camera while it is travelling along the path (the algorithm moves on to the next knot in the path for determining rotation).\n\n"

   "The datablock field for a PathCamera is a previously created PathCameraData, which acts as the interface between the script and the engine "
   "for this PathCamera object.\n\n"

   "@see PathCameraData\n"

		"@tsexample\n"
		   "%newPathCamera = new PathCamera()\n"
		   "{\n"
		   "  dataBlock = LoopingCam;\n"
		   "  position = \"0 0 300 1 0 0 0\";\n"
		   "};\n"
		"@endtsexample\n"

   "@ingroup PathCameras\n"
);

IMPLEMENT_CALLBACK( PathCamera, onNode, void, (S32 node), (node),
					"A script callback that indicates the path camera has arrived at a specific node in its path.  Server side only.\n"
					"@param Node Unique ID assigned to this node.\n");

PathCamera::PathCamera()
{
   mNetFlags.clear(Ghostable);
   mTypeMask |= CameraObjectType;
   delta.time = 0;
   delta.timeVec = 0;

   mDataBlock = 0;
   mState = Forward;
   mNodeBase = 0;
   mNodeCount = 0;
   mPosition = 0;
   mTarget = 0;
   mTargetSet = false;

   MatrixF mat(1);
   mat.setPosition(Point3F(0,0,700));
   Parent::setTransform(mat);
}

PathCamera::~PathCamera()
{
}


//----------------------------------------------------------------------------

bool PathCamera::onAdd()
{
   if(!Parent::onAdd())
      return false;

   // Initialize from the current transform.
   if (!mNodeCount) {
      QuatF rot(getTransform());
      Point3F pos = getPosition();
      mSpline.removeAll();
      mSpline.push_back(new CameraSpline::Knot(pos,rot,1,
         CameraSpline::Knot::NORMAL, CameraSpline::Knot::SPLINE));
      mNodeCount = 1;
   }

   //
   mObjBox.maxExtents = mObjScale;
   mObjBox.minExtents = mObjScale;
   mObjBox.minExtents.neg();
   resetWorldBox();

   if (mShapeInstance)
   {
      mNetFlags.set(Ghostable);
      setScopeAlways();
   }

   addToScene();

   return true;
}

void PathCamera::onRemove()
{
   removeFromScene();

   Parent::onRemove();
}

bool PathCamera::onNewDataBlock( GameBaseData *dptr, bool reload )
{
   mDataBlock = dynamic_cast< PathCameraData* >( dptr );
   if ( !mDataBlock || !Parent::onNewDataBlock( dptr, reload ) )
      return false;

   scriptOnNewDataBlock();
   return true;
}

//----------------------------------------------------------------------------

void PathCamera::onEditorEnable()
{
   mNetFlags.set(Ghostable);
}

void PathCamera::onEditorDisable()
{
   mNetFlags.clear(Ghostable);
}


//----------------------------------------------------------------------------

void PathCamera::initPersistFields()
{
   Parent::initPersistFields();
}

void PathCamera::consoleInit()
{
}


//----------------------------------------------------------------------------

void PathCamera::processTick(const Move* move)
{
   // client and server
   Parent::processTick(move);

   // Move to new time
   advancePosition(TickMs);

   // Set new position
   MatrixF mat;
   interpolateMat(mPosition,&mat);
   Parent::setTransform(mat);

   updateContainer();
}

void PathCamera::interpolateTick(F32 dt)
{
   Parent::interpolateTick(dt);
   MatrixF mat;
   interpolateMat(delta.time + (delta.timeVec * dt),&mat);
   Parent::setRenderTransform(mat);
}

void PathCamera::interpolateMat(F32 pos,MatrixF* mat)
{
   CameraSpline::Knot knot;
   mSpline.value(pos - mNodeBase,&knot);
   knot.mRotation.setMatrix(mat);
   mat->setPosition(knot.mPosition);
}

void PathCamera::advancePosition(S32 ms)
{
   delta.timeVec = mPosition;

   // Advance according to current speed
   if (mState == Forward) {
      mPosition = mSpline.advanceTime(mPosition - mNodeBase,ms);
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
         mPosition = mSpline.advanceTime(mPosition - mNodeBase,-ms);
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
   delta.timeVec -= mPosition;
}


//----------------------------------------------------------------------------

void PathCamera::getCameraTransform(F32* pos, MatrixF* mat)
{
   // Overide the ShapeBase method to skip all the first/third person support.
   getRenderEyeTransform(mat);

   // Apply Camera FX.
   mat->mul( gCamFXMgr.getTrans() );
}


//----------------------------------------------------------------------------

void PathCamera::setPosition(F32 pos)
{
   mPosition = mClampF(pos, (F32)mNodeBase, (F32)(mNodeBase + mNodeCount - 1));
   MatrixF mat;
   interpolateMat(mPosition,&mat);
   Parent::setTransform(mat);
   setMaskBits(PositionMask);
}

void PathCamera::setTarget(F32 pos)
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

void PathCamera::setState(State s)
{
   mState = s;
   setMaskBits(StateMask);
}


//-----------------------------------------------------------------------------

void PathCamera::reset(F32 speed)
{
   CameraSpline::Knot *knot = new CameraSpline::Knot;
   mSpline.value(mPosition - mNodeBase,knot);
   if (speed)
      knot->mSpeed = speed;
   mSpline.removeAll();
   mSpline.push_back(knot);

   mNodeBase = 0;
   mNodeCount = 1;
   mPosition = 0;
   mTargetSet = false;
   mState = Forward;
   setMaskBits(StateMask | PositionMask | WindowMask | TargetMask);
}

void PathCamera::pushBack(CameraSpline::Knot *knot)
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

void PathCamera::pushFront(CameraSpline::Knot *knot)
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
}

void PathCamera::popFront()
{
   if (mNodeCount < 2)
      return;

   // Remove the first node. Node base and position are unaffected.
   mNodeCount--;
   delete mSpline.remove(mSpline.getKnot(0));

   if( mPosition > 0 )
      mPosition --;
}


//----------------------------------------------------------------------------

void PathCamera::onNode(S32 node)
{
   if (!isGhost())
		onNode_callback(node);
   
}

U32 PathCamera::packUpdate(NetConnection *con, U32 mask, BitStream *stream)
{
   Parent::packUpdate(con,mask,stream);

   if (stream->writeFlag(mask & StateMask))
      stream->writeInt(mState,StateBits);

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
   if(stream->writeFlag(getControllingClient() == con && !(mask & InitialUpdateMask)))
      return 0;

   return 0;
}

void PathCamera::unpackUpdate(NetConnection *con, BitStream *stream)
{
   Parent::unpackUpdate(con,stream);

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

}


//-----------------------------------------------------------------------------
// Console access methods
//-----------------------------------------------------------------------------

DefineEngineMethod(PathCamera, setPosition, void, (F32 position),(0.0f), "Set the current position of the camera along the path.\n"
													"@param position Position along the path, from 0.0 (path start) - 1.0 (path end), to place the camera.\n"
													"@tsexample\n"
                                          "// Set the camera on a position along its path from 0.0 - 1.0.\n"
														"%position = \"0.35\";\n\n"
														"// Force the pathCamera to its new position along the path.\n"
														"%pathCamera.setPosition(%position);\n"
													"@endtsexample\n")
{
   object->setPosition(position);
}

DefineEngineMethod(PathCamera, setTarget, void, (F32 position),(1.0f), "@brief Set the movement target for this camera along its path.\n\n"
                                       "The camera will attempt to move along the path to the given target in the direction provided "
                                       "by setState() (the default is forwards).  Once the camera moves past this target it will come "
                                       "to a stop, and the target state will be cleared.\n"
													"@param position Target position, between 0.0 (path start) and 1.0 (path end), for the camera to move to along its path.\n"
													"@tsexample\n"
                                          "// Set the position target, between 0.0 (path start) and 1.0 (path end), for this camera to move to.\n"
														"%position = \"0.50\";\n\n"
														"// Inform the pathCamera of the new target position it will move to.\n"
														"%pathCamera.setTarget(%position);\n"
													"@endtsexample\n")
{
   object->setTarget(position);
}

DefineEngineMethod(PathCamera, setState, void, (const char* newState),("forward"), "Set the movement state for this path camera.\n"
													"@param newState New movement state type for this camera. Forward, Backward or Stop.\n"
													"@tsexample\n"
														"// Set the state type (forward, backward, stop).\n"
                                          "// In this example, the camera will travel from the first node\n"
                                          "// to the last node (or target if given with setTarget())\n"
														"%state = \"forward\";\n\n"
														"// Inform the pathCamera to change its movement state to the defined value.\n"
														"%pathCamera.setState(%state);\n"
													"@endtsexample\n")
{
   if (!dStricmp(newState,"forward"))
      object->setState(PathCamera::Forward);
   else
      if (!dStricmp(newState,"backward"))
         object->setState(PathCamera::Backward);
      else
         object->setState(PathCamera::Stop);
}

DefineEngineMethod(PathCamera, reset, void, (F32 speed),(1.0f), "@brief Clear the camera's path and set the camera's current transform as the start of the new path.\n\n"
                                       "What specifically occurs is a new knot is created from the camera's current transform.  Then the current path "
                                       "is cleared and the new knot is pushed onto the path.  Any previous target is cleared and the camera's movement "
                                       "state is set to Forward.  The camera is now ready for a new path to be defined.\n"
													"@param speed Speed for the camera to move along its path after being reset.\n"
													"@tsexample\n"
														"//Determine the new movement speed of this camera. If not set, the speed will default to 1.0.\n"
														"%speed = \"0.50\";\n\n"
														"// Inform the path camera to start a new path at"
                                          "// the camera's current position, and set the new "
                                          "// path's speed value.\n"
														"%pathCamera.reset(%speed);\n"
                                       "@endtsexample\n")
{
	object->reset(speed);
}

static CameraSpline::Knot::Type resolveKnotType(const char *arg)
{
   if (dStricmp(arg, "Position Only") == 0) 
      return CameraSpline::Knot::POSITION_ONLY;
   if (dStricmp(arg, "Kink") == 0) 
      return CameraSpline::Knot::KINK;
   return CameraSpline::Knot::NORMAL;
}

static CameraSpline::Knot::Path resolveKnotPath(const char *arg)
{
   if (!dStricmp(arg, "Linear"))
      return CameraSpline::Knot::LINEAR;
   return CameraSpline::Knot::SPLINE;
}

DefineEngineMethod(PathCamera, pushBack, void, (TransformF transform, F32 speed, const char* type, const char* path),
											   (1.0f, "Normal", "Linear"), 
											      "@brief Adds a new knot to the back of a path camera's path.\n"
													"@param transform Transform for the new knot.  In the form of \"x y z ax ay az aa\" such as returned by SceneObject::getTransform()\n"
													"@param speed Speed setting for this knot.\n"
													"@param type Knot type (Normal, Position Only, Kink).\n"
													"@param path %Path type (Linear, Spline).\n"
													"@tsexample\n"
														"// Transform vector for new knot. (Pos_X Pos_Y Pos_Z Rot_X Rot_Y Rot_Z Angle)\n"
														"%transform = \"15.0 5.0 5.0 1.4 1.0 0.2 1.0\"\n\n"
														"// Speed setting for knot.\n"
														"%speed = \"1.0\"\n\n"
														"// Knot type. (Normal, Position Only, Kink)\n"
														"%type = \"Normal\";\n\n"
														"// Path Type. (Linear, Spline)\n"
														"%path = \"Linear\";\n\n"
														"// Inform the path camera to add a new knot to the back of its path\n"
														"%pathCamera.pushBack(%transform,%speed,%type,%path);\n"
													"@endtsexample\n")
{
   QuatF rot(transform.getOrientation());

   object->pushBack( new CameraSpline::Knot(transform.getPosition(), rot, speed, resolveKnotType(type), resolveKnotPath(path)) );
}

DefineEngineMethod(PathCamera, pushFront, void, (TransformF transform, F32 speed, const char* type, const char* path),
											   (1.0f, "Normal", "Linear"), 
											      "@brief Adds a new knot to the front of a path camera's path.\n"
													"@param transform Transform for the new knot. In the form of \"x y z ax ay az aa\" such as returned by SceneObject::getTransform()\n"
													"@param speed Speed setting for this knot.\n"
													"@param type Knot type (Normal, Position Only, Kink).\n"
													"@param path %Path type (Linear, Spline).\n"
													"@tsexample\n"
														"// Transform vector for new knot. (Pos_X,Pos_Y,Pos_Z,Rot_X,Rot_Y,Rot_Z,Angle)\n"
														"%transform = \"15.0 5.0 5.0 1.4 1.0 0.2 1.0\"\n\n"
														"// Speed setting for knot.\n"
														"%speed = \"1.0\";\n\n"
														"// Knot type. (Normal, Position Only, Kink)\n"
														"%type = \"Normal\";\n\n"
														"// Path Type. (Linear, Spline)\n"
														"%path = \"Linear\";\n\n"
														"// Inform the path camera to add a new knot to the front of its path\n"
														"%pathCamera.pushFront(%transform, %speed, %type, %path);\n"
													"@endtsexample\n")
{
   QuatF rot(transform.getOrientation());

   object->pushFront( new CameraSpline::Knot(transform.getPosition(), rot, speed, resolveKnotType(type), resolveKnotPath(path)) );
}

DefineEngineMethod(PathCamera, popFront, void, (),, "Removes the knot at the front of the camera's path.\n"
													"@tsexample\n"
														"// Remove the first knot in the camera's path.\n"
														"%pathCamera.popFront();\n"
													"@endtsexample\n")
{
   object->popFront();
}