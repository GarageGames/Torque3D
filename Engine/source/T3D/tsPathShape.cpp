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
//
// Much of this code was taken directly from tsStatic.cpp and the PathShape
// resource: http://www.garagegames.com/community/resource/view/20385/1
// With additional improvements by Azaezel:
// http://www.garagegames.com/community/forums/viewthread/137195
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "tsPathShape.h"

#include "core/stream/bitStream.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "sim/netConnection.h"
#include "math/mathIO.h"
#include "math/mTransform.h"

using namespace Torque;

extern bool gEditingMission;

IMPLEMENT_CO_NETOBJECT_V1(TSPathShape);

ConsoleDocClass( TSPathShape,
   "@brief A shape that will follow a predefined path.\n\n"

   "TSPathShape is a simple path following shape.\n\n"

   "@tsexample\n"
         "new TSPathShape(Platform1) {\n"
         "   Path = \"mPathA\";\n"
         "   movementState = \"Stop\";\n"
         "   shapeName = \"art/shapes/desertStructures/station01.dts\";\n"
         "   playAmbient = \"1\";\n"
         "   collisionType = \"Visible Mesh\";\n"
         "   decalType = \"Collision Mesh\";\n"
         "   allowPlayerStep = \"1\";\n"
         "   renderNormals = \"0\";\n"
         "   forceDetail = \"-1\";\n"
         "   position = \"315.18 -180.418 244.313\";\n"
         "   rotation = \"0 0 1 195.952\";\n"
         "   scale = \"1 1 1\";\n"
         "   isRenderEnabled = \"true\";\n"
         "   canSaveDynamicFields = \"1\";\n"
         "};\n"
   "@endtsexample\n"

   "@ingroup gameObjects\n"
);

IMPLEMENT_CALLBACK( TSPathShape, onAdd, void, (), (),
   "Called when this TSPathShape is added to the system.\n");

IMPLEMENT_CALLBACK( TSPathShape, onPathChange, void, (), (),
   "Called when the path variable for this this TSPathShape has been changed from script or editor.\n");

IMPLEMENT_CALLBACK( TSPathShape, onNode, void, (S32 node), (node),
               "A script callback that indicates the path shape has arrived at a specific node in its path.  Server side only.\n"
               "@param Node The index number for the node reached.\n");

IMPLEMENT_CALLBACK( TSPathShape, onTargetReached, void, (F32 value), (value),
               "A script callback that indicates the path shape has arrived at it's target value.  Server side only.\n"
               "@param value The current position value.\n");

U32 TSPathShape::mUpdateTics = 100;

TSPathShape::TSPathShape()
{
   mNetFlags.set(Ghostable | ScopeAlways);

   mTypeMask |= StaticObjectType | StaticShapeObjectType | DynamicShapeObjectType;

   mState = Stop;
   mNodeBase = 0;
   mNodeCount = 0;
   mPosition = 0;
   mTarget = 0;
   mTargetSet = false;
   mLooping = false;

   delta.state = Stop;
   delta.time = 0;
   delta.timeVec = 0;

   mUpdateTickCount = 0;
}

TSPathShape::~TSPathShape()
{
}

ImplementEnumType( PathShapeState,
   "Movement type for a TSPathShape.\n"
   "@ingroup gameObjects" )
   { TSPathShape::Forward,    "Forward",  "Move forward along the path." },
   { TSPathShape::Backward,   "Backward", "Move backward along the path" },
   { TSPathShape::Stop,       "Stop",     "Stop shape movement." },
EndImplementEnumType;



void TSPathShape::initPersistFields()
{
   addGroup("Movement");

      addField( "Path", TYPEID< SimObjectRef<SimPath::Path> >(), Offset( mPath, TSPathShape ),
         "Name of a Path to follow." );
      addField( "movementState", TypePathShapeState, Offset( mState,   TSPathShape ),
         "The shape movement direction/state." );

   endGroup("Movement");

   Parent::initPersistFields();
}

bool TSPathShape::onAdd()
{
   PROFILE_SCOPE(TSPathShape_onAdd);

   if ( !Parent::onAdd() )
      return false;

   if (!isGhost())
      onAdd_callback();

   _updateShouldTick();

   return true;
}

void TSPathShape::onRemove()
{
   Parent::onRemove();
}

void TSPathShape::onStaticModified( const char* slotName, const char*newValue )
{
   if (dStricmp( slotName, "Path") == 0 )
   {  // If the path has been changed in the editor, make a script callback so it gets installed
      if ( !isGhost() && isProperlyAdded() )
         onPathChange_callback();
   }
   else if ( dStricmp(slotName, "movementState") == 0 )
   {
      if ( isServerObject() )
      {
         setMaskBits( StateMask );
         _updateShouldTick();
      }
   }
   else
      Parent::onStaticModified( slotName, newValue );
}

void TSPathShape::interpolateMat(F32 pos, MatrixF* mat)
{
   CameraSpline::Knot knot;
   F32 timeVal;
   if ( mLooping && (pos > (mNodeCount-1)) )
      timeVal = pos - (mNodeCount - 1);
   else if ( mLooping && (pos < mNodeBase) )
      timeVal = pos + (mNodeCount - 1);
   else
      timeVal = pos - mNodeBase;
   mSpline.value(timeVal, &knot, false, mLooping);
   knot.mRotation.setMatrix(mat);
   mat->setPosition(knot.mPosition);
}

void TSPathShape::processTick( const Move *move )
{
   Parent::processTick(move);
   if (((mState == Stop) && (delta.state == Stop)) || (mNodeCount < 1))
      return;

   mUpdateTickCount++;

   // Move to new time
   advancePosition(TickMs);

   // Set new position
   MatrixF mat;
   interpolateMat(mPosition,&mat);
   setTransform(mat);
}

void TSPathShape::interpolateTick( F32 dt )
{
   Parent::interpolateTick(dt);
   if ((delta.state == Stop) || (mNodeCount < 1))
      return;

   MatrixF mat;
   interpolateMat(delta.time + (delta.timeVec * dt),&mat);
   Parent::setRenderTransform(mat);
}

void TSPathShape::advancePosition(S32 ms)
{
   bool useLoopInterpolation = false;
   delta.timeVec = mPosition;
   delta.state = mState;

   // Advance according to current speed
   if (mState == Forward)
   {
      mPosition = mSpline.advanceTime(mPosition - mNodeBase,ms);
      if (mTargetSet && mPosition >= mTarget)
      {
         mTargetSet = false;
         mPosition = mTarget;
         mState = Stop;
         onTargetReached_callback(mPosition);
      }
      else
      {
         if (mPosition > mNodeCount - 1)
         {  // We've passed the last node...
            if ( mLooping )
            {  // Loop around to beginning and set up interpolation
               useLoopInterpolation = true;
               delta.time = mPosition + mNodeBase;
               delta.timeVec -= (mPosition + mNodeBase);
               mPosition = mPosition - (mNodeCount - 1);
            }
            else
            {  // Stop right on the last node
               mPosition = mNodeCount - 1;
               mState = Stop;
            }
         }
      }
      mPosition += mNodeBase;
   }
   else if (mState == Backward)
   {
      mPosition = mSpline.advanceTime(mPosition - mNodeBase,-ms);
      if (mTargetSet && mPosition <= mTarget) {
         mTargetSet = false;
         mPosition = mTarget;
         mState = Stop;
         onTargetReached_callback(mPosition);
      }
      else
      {
         if (mPosition < 0)
         {  // We've gone backwards past the first node
            if ( mLooping )
            {  // Loop around to the end and set up interpolation
               useLoopInterpolation = true;
               delta.time = mPosition + mNodeBase;
               delta.timeVec -= (mPosition + mNodeBase);
               mPosition = mPosition + (mNodeCount - 1);
            }
            else
            {  // Stop right on the start node
               mPosition = 0;
               mState = Stop;

               // Fire the callback here because this case won't get caught below
               if ( isServerObject() )
                  onNode_callback(mNodeBase);
            }
         }
      }
      mPosition += mNodeBase;
   }
   else
      _updateShouldTick();

   if ( isServerObject() )
   {
      // Script callbacks
      if ( useLoopInterpolation )
         onNode_callback(mNodeBase);
      else if ( int(mPosition) != int(delta.timeVec) )
      {
         if ( mPosition > delta.timeVec )
            onNode_callback(int(mPosition));
         else if ( int(delta.timeVec) != (mNodeBase + mNodeCount - 1) )
            onNode_callback(int(delta.timeVec));
      }

      if ( mUpdateTickCount > mUpdateTics )
      {
         setMaskBits(PositionMask); // Keep clients in synch
         mUpdateTickCount = 0;
      }
   }

   // Set frame interpolation
   if ( !useLoopInterpolation )
   {
      delta.time = mPosition;
      delta.timeVec -= mPosition;
   }
}

bool TSPathShape::_getShouldTick()
{
   return (mState != Stop) || Parent::_getShouldTick();
}

//----------------------------------------------------------------------------
U32 TSPathShape::packUpdate(NetConnection *con, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   if (stream->writeFlag(mask & WindowMask)) {
      stream->write(mNodeBase);
      stream->write(mNodeCount);
      for (int i = 0; i < mNodeCount; i++) {
         CameraSpline::Knot *knot = mSpline.getKnot(i);
         mathWrite(*stream, knot->mPosition);
         mathWrite(*stream, knot->mRotation);
         stream->write(knot->mSpeed);
         stream->writeInt(knot->mType, CameraSpline::Knot::NUM_TYPE_BITS);
         stream->writeInt(knot->mPath, CameraSpline::Knot::NUM_PATH_BITS);
      }
   }

   if (stream->writeFlag( mask & (PositionMask | StateMask) ))
   {
      stream->write(mPosition);
      stream->writeInt(mState, MoveStateBits);
      stream->writeFlag(mLooping);
   }

   if (stream->writeFlag(mask & TargetMask))
      if (stream->writeFlag(mTargetSet))
         stream->write(mTarget);

   return retMask;
}

void TSPathShape::unpackUpdate(NetConnection *con, BitStream *stream)
{
   Parent::unpackUpdate(con, stream);

   // WindowMask
   if (stream->readFlag()) {
      mSpline.removeAll();
      stream->read(&mNodeBase);
      stream->read(&mNodeCount);
      for (int i = 0; i < mNodeCount; i++) {
         CameraSpline::Knot *knot = new CameraSpline::Knot();
         mathRead(*stream, &knot->mPosition);
         mathRead(*stream, &knot->mRotation);
         stream->read(&knot->mSpeed);
         knot->mType = (CameraSpline::Knot::Type)stream->readInt(CameraSpline::Knot::NUM_TYPE_BITS);
         knot->mPath = (CameraSpline::Knot::Path)stream->readInt(CameraSpline::Knot::NUM_PATH_BITS);
         mSpline.push_back(knot);
      }
   }

   // PositionMask | StateMask
   if (stream->readFlag()) {
      stream->read(&mPosition);
      F32 oldPos = delta.time + delta.timeVec;
      setPathPosition(mPosition);
      delta.time = mPosition;
      delta.timeVec = oldPos - mPosition;
      mState = (MoveState) stream->readInt(MoveStateBits);
      mLooping = stream->readFlag();
      if ( isProperlyAdded() )
         _updateShouldTick();
   }


   // TargetMask
   if (stream->readFlag()) { 
      mTargetSet = stream->readFlag();
      if (mTargetSet) {
         stream->read(&mTarget);
      }
   }
}

//------------------------------------------------------------------------
// Movement control methods
//-----------------------------------------------------------------------------

void TSPathShape::setPathPosition(F32 pos)
{
   if ( mNodeCount == 0 )
      return;

   mPosition = mClampF(pos, (F32)mNodeBase, (F32)(mNodeBase + mNodeCount - 1));
   setMaskBits(PositionMask);
}

void TSPathShape::setTarget(F32 pos)
{
   mTarget = pos;
   mTargetSet = true;
   if (mTarget > mPosition)
      mState = Forward;
   else if (mTarget < mPosition)
      mState = Backward;
   else
   {
      mTargetSet = false;
      mState = Stop;
   }
   setMaskBits(TargetMask | StateMask);
}

void TSPathShape::setMoveState(MoveState s)
{
   if ( mState != s )
   {
      mState = s;
      setMaskBits(StateMask);
      _updateShouldTick();
   }
}

void TSPathShape::setLooping(bool isLooping)
{
   mLooping = isLooping;
   setMaskBits(StateMask);
}

//-----------------------------------------------------------------------------
DefineEngineMethod(TSPathShape, setPathPosition, void, (F32 position),(0.0f), "Set the current position of the shape along the path.\n"
                                       "@param position Position along the path, from 0.0 (path start) - 1.0 (path end), to place the shape.\n")
{
   object->setPathPosition(position);
}

DefineEngineMethod(TSPathShape, getPathPosition, F32, (), , "Get the current position of the shape along the path (0.0 - lastNode - 1).\n")
{
   return object->getPathPosition();
}

DefineEngineMethod(TSPathShape, setTarget, void, (F32 position),(1.0f), "@brief Set the movement target for this shape along its path.\n\n"
                                       "The shape will attempt to move along the path to the given target without going past the loop node. "
                                       "Once the shape arrives at the target,the onTargetReached() callback will be triggered and the target "
                                       "state will be cleared.\n"
                                       "@param position Target position, between 0.0 (path start) and nodeCount - 1 (path end), for the "
                                       "shape to move to along its path.\n")
{
   object->setTarget(position);
}

DefineEngineMethod(TSPathShape, setMoveState, void, (PathShapeState newState),(TSPathShape::Forward), "Set the movement state for this shape.\n"
                                       "@param newState New movement state type for this shape. Forward, Backward or Stop.\n")
{
   object->setMoveState(newState);
}

//-----------------------------------------------------------------------------
// Path creation methods
//-----------------------------------------------------------------------------

bool TSPathShape::reset(F32 speed, bool makeKnot)
{
   bool knotMade = false;

   if ( makeKnot && mNodeCount && mSpline.size())
   {
      CameraSpline::Knot *knot = new CameraSpline::Knot;
      mSpline.value(mPosition - mNodeBase,knot);
      if (speed)
         knot->mSpeed = speed;
      mSpline.removeAll();
      mSpline.push_back(knot);
      mNodeCount = 1;
      knotMade = true;
   }
   else
   {
      mSpline.removeAll();
      mNodeCount = 0;
   }

   mNodeBase = 0;
   mPosition = 0;
   mTargetSet = false;
   mLooping = false;
   mState = Stop;
   setMaskBits(StateMask | PositionMask | WindowMask | TargetMask);

   return knotMade;
}

void TSPathShape::pushBack(CameraSpline::Knot *knot)
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

void TSPathShape::pushFront(CameraSpline::Knot *knot)
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

void TSPathShape::popFront()
{
   if (mNodeCount < 2)
      return;

   // Remove the first node. Node base and position are unaffected.
   mNodeCount--;
   delete mSpline.remove(mSpline.getKnot(0));

   if( mPosition > 0 )
      mPosition--;
}

//-----------------------------------------------------------------------------
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


//-----------------------------------------------------------------------------
DefineEngineMethod(TSPathShape, reset, void, (F32 speed, bool makeFirstKnot, bool initFromPath),(1.0f, true, true),
         "@brief Clear the shapes's path and optionally initializes the first node with the shapes current transform and speed.\n\n"
         "The shapes movement is stopped and any current path is cleared. The target and position values are both reset to 0. "
         "When makeFirstKnot is true a new knot is created and pushed onto the path.\n"
         "@param speed Speed for the first knot if created.\n"
         "@param makeFirstKnot Initialize a new path with the current shape transform.\n"
         "@param initFromPath Initialize the knot type and smoothing values from the current path.\n")
{
   if ( !object->reset(speed, makeFirstKnot && initFromPath) && makeFirstKnot )
   {  // No knot was created because no path has been set yet.
      MatrixF mat = object->getTransform();
      AngAxisF mOrientation;
      mOrientation.set(mat);
      QuatF rot(mOrientation);

      object->pushBack( new CameraSpline::Knot(mat.getPosition(), rot, speed, CameraSpline::Knot::NORMAL, CameraSpline::Knot::SPLINE) );
   }
}

DefineEngineMethod(TSPathShape, pushBack, void, (TransformF transform, F32 speed, const char* type, const char* path),
                                    (1.0f, "Normal", "Linear"), 
                                       "@brief Adds a new knot to the back of a shape's path.\n"
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
                                          "// Inform the shape to add a new knot to the back of its path\n"
                                          "%pathShape.pushBack(%transform,%speed,%type,%path);\n"
                                       "@endtsexample\n")
{
   QuatF rot(transform.getOrientation());

   object->pushBack( new CameraSpline::Knot(transform.getPosition(), rot, speed, resolveKnotType(type), resolveKnotPath(path)) );
}

DefineEngineMethod(TSPathShape, pushFront, void, (TransformF transform, F32 speed, const char* type, const char* path),
                                    (1.0f, "Normal", "Linear"), 
                                       "@brief Adds a new knot to the front of a path shape's path.\n"
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
                                          "// Inform the shape to add a new knot to the front of its path\n"
                                          "%pathShape.pushFront(%transform, %speed, %type, %path);\n"
                                       "@endtsexample\n")
{
   QuatF rot(transform.getOrientation());

   object->pushFront( new CameraSpline::Knot(transform.getPosition(), rot, speed, resolveKnotType(type), resolveKnotPath(path)) );
}

DefineEngineMethod(TSPathShape, popFront, void, (),, "Removes the knot at the front of the shape's path.\n"
                                       "@tsexample\n"
                                          "// Remove the first knot in the shape's path.\n"
                                          "%pathShape.popFront();\n"
                                       "@endtsexample\n")
{
   object->popFront();
}

DefineEngineMethod(TSPathShape, setLooping, void, (bool isLooping),(true), "Sets whether the path should loop or stop at the last node.\n"
                                       "@param isLooping New loop flag true/false.\n")
{
   object->setLooping(isLooping);
}

DefineEngineMethod(TSPathShape, getLooping, bool, (), , "Returns the looping state for the shape.\n")
{
   return object->getLooping();
}

DefineEngineMethod(TSPathShape, getNodeCount, S32, (), , "Returns the number of nodes on the shape's path.\n")
{
   return object->getNodeCount();
}
