#pragma once

#include "T3D/components/component.h"

class FollowPathComponent : public Component
{
   typedef Component Parent;

   enum State {
      Forward,
      Backward,
      Stop,
      StateBits = 3
   };

   enum MaskBits {
      WindowMask = Parent::NextFreeMask,
      PositionMask = Parent::NextFreeMask + 1,
      TargetMask = Parent::NextFreeMask + 2,
      StateMask = Parent::NextFreeMask + 3,
      NextFreeMask = Parent::NextFreeMask << 1
   };

   struct StateDelta {
      F32 time;
      F32 timeVec;
   };
   StateDelta delta;

   enum Constants {
      NodeWindow = 128    // Maximum number of active nodes
   };

   //
   //PathCameraData* mDataBlock;
   //CameraSpline mSpline;
   S32 mNodeBase;
   S32 mNodeCount;
   F32 mPosition;
   S32 mState;
   F32 mTarget;
   bool mTargetSet;

   void interpolateMat(F32 pos, MatrixF* mat);
   void advancePosition(S32 ms);

public:
   DECLARE_CONOBJECT(FollowPathComponent);

   FollowPathComponent();
   ~FollowPathComponent();

   virtual bool onAdd();
   virtual void onRemove();
   static void initPersistFields();

   //This is called when a different component is added to our owner entity
   virtual void componentAddedToOwner(Component *comp);
   //This is called when a different component is removed from our owner entity
   virtual void componentRemovedFromOwner(Component *comp);

   virtual void processTick();
   virtual void interpolateTick(F32 dt);
   virtual void advanceTime(F32 dt);

   //
   //void onEditorEnable();
   //void onEditorDisable();

   void onNode(S32 node);

   void reset(F32 speed = 1);
   //void pushFront(CameraSpline::Knot *knot);
   //void pushBack(CameraSpline::Knot *knot);
   void popFront();

   void setPosition(F32 pos);
   void setTarget(F32 pos);
   void setState(State s);

   DECLARE_CALLBACK(void, onNode, (S32 node));
};