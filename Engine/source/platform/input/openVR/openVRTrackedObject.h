#ifndef _OPENVR_TRACKED_OBJECT_H_
#define _OPENVR_TRACKED_OBJECT_H_

#ifndef _GAMEBASE_H_
#include "T3D/gameBase/gameBase.h"
#endif
#ifndef _GFXVERTEXBUFFER_H_
#include "gfx/gfxVertexBuffer.h"
#endif
#ifndef _GFXPRIMITIVEBUFFER_H_
#include "gfx/gfxPrimitiveBuffer.h"
#endif
#ifndef _TSSHAPEINSTANCE_H_
#include "ts/tsShapeInstance.h"
#endif
#include "collision/earlyOutPolyList.h"

#include <openvr.h>

class BaseMatInstance;
class OpenVRRenderModel;
class PhysicsBody;

class OpenVRTrackedObjectData : public GameBaseData {
public:
   typedef GameBaseData Parent;

   StringTableEntry mShapeFile;
   Resource<TSShape> mShape; ///< Torque model

   Point3F mCollisionBoxMin;
   Point3F mCollisionBoxMax;

public:

   OpenVRTrackedObjectData();
   ~OpenVRTrackedObjectData();

   DECLARE_CONOBJECT(OpenVRTrackedObjectData);

   bool onAdd();
   bool preload(bool server, String &errorStr);

   static void  initPersistFields();

   virtual void packData(BitStream* stream);
   virtual void unpackData(BitStream* stream);
};

/// Implements a GameObject which tracks an OpenVR controller
class OpenVRTrackedObject : public GameBase
{
   typedef GameBase Parent;

   enum MaskBits
   {
      UpdateMask = Parent::NextFreeMask << 0,
      NextFreeMask = Parent::NextFreeMask << 1
   };

   struct RenderModelSlot
   {
      StringTableEntry componentName; ///< Component name
      S16 mappedNodeIdx; ///< Mapped node idx in mShape
      OpenVRRenderModel *nativeModel; ///< Native model
   };

   OpenVRTrackedObjectData *mDataBlock;

   /// @name Rendering
   /// {
   TSShapeInstance *mShapeInstance; ///< Shape used to render controller (uses native model otherwise)
   StringTableEntry mModelName;
   OpenVRRenderModel *mBasicModel; ///< Basic model
   Vector<RenderModelSlot> mRenderComponents;
   /// }

   S32 mDeviceIndex; ///< Controller idx in openvr (for direct updating)
   S32 mMappedMoveIndex; ///< Movemanager move index for rotation

   vr::VRControllerState_t mCurrentControllerState;
   vr::VRControllerState_t mPreviousControllerState;

   IDevicePose mPose; ///< Current openvr pose data, or reconstructed data from the client

   Convex* mConvexList;
   EarlyOutPolyList     mClippedList;
   PhysicsBody *mPhysicsRep;

   SimObjectPtr<SceneObject> mCollisionObject; ///< Object we're currently colliding with
   SimObjectPtr<SceneObject> mInteractObject;  ///< Object we've designated as important to interact with

   bool mHoldInteractedObject; ///< Performs pickup logic with mInteractObject
   bool mIgnoreParentRotation; ///< Ignores the rotation of the parent object

   static bool smDebugControllerPosition; ///< Shows latest controller position in DebugDrawer
   static bool smDebugControllerMovePosition; ///< Shows move position in DebugDrawer
   static U32 sServerCollisionMask;
   static U32 sClientCollisionMask;

public:
   OpenVRTrackedObject();
   virtual ~OpenVRTrackedObject();

   void updateRenderData();
   void setupRenderDataFromModel(bool loadComponentModels);

   void clearRenderData();

   DECLARE_CONOBJECT(OpenVRTrackedObject);

   static void initPersistFields();

   virtual void inspectPostApply();

   bool onAdd();
   void onRemove();


   void _updatePhysics();
   bool onNewDataBlock(GameBaseData *dptr, bool reload);

   void setInteractObject(SceneObject* object, bool holding);

   void setTransform(const MatrixF &mat);
   void setModelName(String &modelName);

   U32  packUpdate(NetConnection *conn, U32 mask, BitStream *stream);
   void unpackUpdate(NetConnection *conn, BitStream *stream);
   void writePacketData(GameConnection *conn, BitStream *stream);
   void readPacketData(GameConnection *conn, BitStream *stream);

   void prepRenderImage(SceneRenderState *state);

   MatrixF getTrackedTransform();
   MatrixF getLastTrackedTransform();
   MatrixF getBaseTrackingTransform();

   U32 getCollisionMask();
   void updateWorkingCollisionSet();

   // Time management
   void updateMove(const Move *move);
   void processTick(const Move *move);
   void interpolateTick(F32 delta);
   void advanceTime(F32 dt);

   // Collision
   bool castRay(const Point3F &start, const Point3F &end, RayInfo* info);
   void buildConvex(const Box3F& box, Convex* convex);
   bool testObject(SceneObject* enter);

};

#endif // _OPENVR_TRACKED_OBJECT_H_