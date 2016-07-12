#include "platform/platform.h"
#include "platform/input/openVR/openVRTrackedObject.h"
#include "platform/input/openVR/openVRProvider.h"

#include "math/mathIO.h"
#include "scene/sceneRenderState.h"
#include "console/consoleTypes.h"
#include "core/stream/bitStream.h"
#include "core/resourceManager.h"
#include "materials/materialManager.h"
#include "materials/baseMatInstance.h"
#include "renderInstance/renderPassManager.h"
#include "lighting/lightQuery.h"
#include "console/engineAPI.h"
#include "gfx/gfxTextureManager.h"
#include "gfx/sim/debugDraw.h"
#include "gfx/gfxTransformSaver.h"
#include "environment/skyBox.h"
#include "collision/boxConvex.h"
#include "collision/concretePolyList.h"
#include "T3D/physics/physicsPlugin.h"
#include "T3D/physics/physicsCollision.h"
#include "T3D/physics/physicsBody.h"

#ifdef TORQUE_EXTENDED_MOVE
#include "T3D/gameBase/extended/extendedMove.h"
#endif


bool OpenVRTrackedObject::smDebugControllerMovePosition = true;
bool OpenVRTrackedObject::smDebugControllerPosition = false;

static const U32 sCollisionMoveMask = (PlayerObjectType |
   StaticShapeObjectType | VehicleObjectType);

U32 OpenVRTrackedObject::sServerCollisionMask = sCollisionMoveMask; // ItemObjectType
U32 OpenVRTrackedObject::sClientCollisionMask = sCollisionMoveMask;

//-----------------------------------------------------------------------------

IMPLEMENT_CO_DATABLOCK_V1(OpenVRTrackedObjectData);

OpenVRTrackedObjectData::OpenVRTrackedObjectData() :
   mShapeFile(NULL)
{
   mCollisionBoxMin = Point3F(-0.02, -0.20, -0.02);
   mCollisionBoxMax = Point3F(0.02, 0.05, 0.02);
}

OpenVRTrackedObjectData::~OpenVRTrackedObjectData()
{
}

bool OpenVRTrackedObjectData::onAdd()
{
   if (Parent::onAdd())
   {
      return true;
   }

   return false;
}

bool OpenVRTrackedObjectData::preload(bool server, String &errorStr)
{
   if (!Parent::preload(server, errorStr))
      return false;

   bool error = false;
   if (!server)
   {
      mShape = mShapeFile ? ResourceManager::get().load(mShapeFile) : NULL;
   }
}

void OpenVRTrackedObjectData::initPersistFields()
{
   addGroup("Render Components");
   addField("shape", TypeShapeFilename, Offset(mShapeFile, OpenVRTrackedObjectData), "Shape file to use for controller model.");
   addField("collisionMin", TypePoint3F, Offset(mCollisionBoxMin, OpenVRTrackedObjectData), "Box min");
   addField("collisionMax", TypePoint3F, Offset(mCollisionBoxMax, OpenVRTrackedObjectData), "Box min");
   endGroup("Render Components");

   Parent::initPersistFields();
}

void OpenVRTrackedObjectData::packData(BitStream* stream)
{
   Parent::packData(stream);

   stream->writeString(mShapeFile);
}

void OpenVRTrackedObjectData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);

   mShapeFile = stream->readSTString();
}

//-----------------------------------------------------------------------------


IMPLEMENT_CO_NETOBJECT_V1(OpenVRTrackedObject);

ConsoleDocClass(OpenVRTrackedObject,
   "@brief Renders and handles interactions OpenVR controllers and tracked objects.\n\n"
   "This class implements basic rendering and interactions with OpenVR controllers.\n\n"
   "The object should be controlled by a player object. Controllers will be rendered at\n"
   "the correct position regardless of the current transform of the object.\n"
   "@ingroup OpenVR\n");


//-----------------------------------------------------------------------------
// Object setup and teardown
//-----------------------------------------------------------------------------
OpenVRTrackedObject::OpenVRTrackedObject() :
   mDataBlock(NULL),
   mShapeInstance(NULL),
   mBasicModel(NULL),
   mDeviceIndex(-1),
   mMappedMoveIndex(-1),
   mIgnoreParentRotation(true),
   mConvexList(new Convex()),
   mPhysicsRep(NULL)
{
   // Flag this object so that it will always
   // be sent across the network to clients
   mNetFlags.set(Ghostable | ScopeAlways);

   // Set it as a "static" object that casts shadows
   mTypeMask |= StaticObjectType | StaticShapeObjectType;

   mPose.connected = false;
}

OpenVRTrackedObject::~OpenVRTrackedObject()
{
   clearRenderData();
   delete mConvexList;
}

void OpenVRTrackedObject::updateRenderData()
{
   clearRenderData();

   if (!mDataBlock)
      return;

   // Are we using a model?
   if (mDataBlock->mShape)
   {
      if (mShapeInstance && mShapeInstance->getShape() != mDataBlock->mShape)
      {
         delete mShapeInstance;
         mShapeInstance = NULL;
      }

      if (!mShapeInstance)
      {
         mShapeInstance = new TSShapeInstance(mDataBlock->mShape, isClientObject());
      }
   }
   else
   {
      setupRenderDataFromModel(isClientObject());
   }
}

void OpenVRTrackedObject::setupRenderDataFromModel(bool loadComponentModels)
{
   clearRenderData();
   
   if (!OPENVR || !OPENVR->isEnabled())
      return;

   vr::IVRRenderModels *models = OPENVR->getRenderModels();
   if (!models)
      return;

   if (!mShapeInstance && mModelName && mModelName[0] != '\0')
   {
      bool failed = false;
      S32 idx = OPENVR->preloadRenderModel(mModelName);
      while (!OPENVR->getRenderModel(idx, &mBasicModel, failed))
      {
         if (failed)
            break;
      }
   }

   if (loadComponentModels)
   {
      mRenderComponents.setSize(models->GetComponentCount(mModelName));

      for (U32 i = 0, sz = mRenderComponents.size(); i < sz; i++)
      {
         RenderModelSlot &slot = mRenderComponents[i];
         char buffer[1024];

         slot.mappedNodeIdx = -1;
         slot.componentName = NULL;
         slot.nativeModel = NULL;

         U32 result = models->GetComponentName(mModelName, i, buffer, sizeof(buffer));
         if (result == 0)
            continue;

#ifdef DEBUG_CONTROLLER_MODELS
         Con::printf("Controller[%s] component %i NAME == %s", mModelName, i, buffer);
#endif

         slot.componentName = StringTable->insert(buffer, true);

         result = models->GetComponentRenderModelName(mModelName, slot.componentName, buffer, sizeof(buffer));
         if (result == 0)
         {
#ifdef DEBUG_CONTROLLER_MODELS
            Con::printf("Controller[%s] component %i NO MODEL", mModelName, i);
#endif
            continue;
         }

#ifdef DEBUG_CONTROLLER_MODELS
         Con::printf("Controller[%s] component %i == %s", mModelName, i, slot.componentName);
#endif

         bool failed = false;
         S32 idx = OPENVR->preloadRenderModel(StringTable->insert(buffer, true));
         while (!OPENVR->getRenderModel(idx, &slot.nativeModel, failed))
         {
            if (failed)
               break;
         }
      }
   }
}

void OpenVRTrackedObject::clearRenderData()
{
   mBasicModel = NULL;
   mRenderComponents.clear();
}

//-----------------------------------------------------------------------------
// Object Editing
//-----------------------------------------------------------------------------
void OpenVRTrackedObject::initPersistFields()
{
   // SceneObject already handles exposing the transform
   Parent::initPersistFields();

   addField("deviceIndex", TypeS32, Offset(mDeviceIndex, OpenVRTrackedObject), "Index of device to track");
   addField("mappedMoveIndex", TypeS32, Offset(mMappedMoveIndex, OpenVRTrackedObject), "Index of movemanager state to track"); addField("deviceIndex", TypeS32, Offset(mDeviceIndex, OpenVRTrackedObject), "Index of device to track");
   addField("ignoreParentRotation", TypeBool, Offset(mIgnoreParentRotation, OpenVRTrackedObject), "Index of movemanager state to track"); addField("deviceIndex", TypeS32, Offset(mDeviceIndex, OpenVRTrackedObject), "Index of device to track");

   static bool conInit = false;
   if (!conInit)
   {
      Con::addVariable("$OpenVRTrackedObject::debugControllerPosition", TypeBool, &smDebugControllerPosition);
      Con::addVariable("$OpenVRTrackedObject::debugControllerMovePosition", TypeBool, &smDebugControllerMovePosition);
      conInit = true;
   }
}

void OpenVRTrackedObject::inspectPostApply()
{
   Parent::inspectPostApply();

   // Flag the network mask to send the updates
   // to the client object
   setMaskBits(UpdateMask);
}

bool OpenVRTrackedObject::onAdd()
{
   if (!Parent::onAdd())
      return false;

   // Set up a 1x1x1 bounding box
   mObjBox.set(Point3F(-0.5f, -0.5f, -0.5f),
      Point3F(0.5f, 0.5f, 0.5f));

   resetWorldBox();

   // Add this object to the scene
   addToScene();

   if (mDataBlock)
   {
      mObjBox.minExtents = mDataBlock->mCollisionBoxMin;
      mObjBox.maxExtents = mDataBlock->mCollisionBoxMax;
      resetWorldBox();
   }
   else
   {
      setGlobalBounds();
   }

   return true;
}

void OpenVRTrackedObject::onRemove()
{
   // Remove this object from the scene
   removeFromScene();

   clearRenderData();

   SAFE_DELETE(mPhysicsRep);

   Parent::onRemove();
}

void OpenVRTrackedObject::_updatePhysics()
{
   SAFE_DELETE(mPhysicsRep);

   if (!PHYSICSMGR)
      return;

   PhysicsCollision *colShape = NULL;
   MatrixF offset(true);
   colShape = PHYSICSMGR->createCollision();
   colShape->addBox(getObjBox().getExtents() * 0.5f * mObjScale, offset);

   if (colShape)
   {
      PhysicsWorld *world = PHYSICSMGR->getWorld(isServerObject() ? "server" : "client");
      mPhysicsRep = PHYSICSMGR->createBody();
      mPhysicsRep->init(colShape, 0, PhysicsBody::BF_TRIGGER | PhysicsBody::BF_KINEMATIC, this, world);
      mPhysicsRep->setTransform(getTransform());
   }
}

bool OpenVRTrackedObject::onNewDataBlock(GameBaseData *dptr, bool reload)
{
   mDataBlock = dynamic_cast<OpenVRTrackedObjectData*>(dptr);
   if (!mDataBlock || !Parent::onNewDataBlock(dptr, reload))
      return false;

   // Setup the models
   clearRenderData();

   mObjBox.minExtents = mDataBlock->mCollisionBoxMin;
   mObjBox.maxExtents = mDataBlock->mCollisionBoxMax;

   mGlobalBounds = false;

   resetWorldBox();

   _updatePhysics();

   scriptOnNewDataBlock();

   return true;
}

void OpenVRTrackedObject::setInteractObject(SceneObject* object, bool holding)
{
   mInteractObject = object;
   mHoldInteractedObject = holding;
}

void OpenVRTrackedObject::setTransform(const MatrixF & mat)
{
   // Let SceneObject handle all of the matrix manipulation
   Parent::setTransform(mat);

   // Dirty our network mask so that the new transform gets
   // transmitted to the client object
   setMaskBits(UpdateMask);
}

void OpenVRTrackedObject::setModelName(String &modelName)
{
   if (!isServerObject())
      return;

   mModelName = StringTable->insert(modelName.c_str(), true);
   setMaskBits(UpdateMask);
}

U32 OpenVRTrackedObject::packUpdate(NetConnection *conn, U32 mask, BitStream *stream)
{
   // Allow the Parent to get a crack at writing its info
   U32 retMask = Parent::packUpdate(conn, mask, stream);

   // Write our transform information
   if (stream->writeFlag(mask & UpdateMask))
   {
      mathWrite(*stream, getTransform());
      mathWrite(*stream, getScale());

      stream->write((S16)mDeviceIndex);
      stream->write((S16)mMappedMoveIndex);
      stream->writeString(mModelName);
   }

   return retMask;
}

void OpenVRTrackedObject::unpackUpdate(NetConnection *conn, BitStream *stream)
{
   // Let the Parent read any info it sent
   Parent::unpackUpdate(conn, stream);

   if (stream->readFlag())  // UpdateMask
   {
      mathRead(*stream, &mObjToWorld);
      mathRead(*stream, &mObjScale);

      setTransform(mObjToWorld);
      
      S16 readDeviceIndex;
      S16 readMoveIndex;
      stream->read(&readDeviceIndex);
      stream->read(&readMoveIndex);

      mDeviceIndex = readDeviceIndex;
      mMappedMoveIndex = readMoveIndex;
      mModelName = stream->readSTString();

      updateRenderData();
   }

}

void OpenVRTrackedObject::writePacketData(GameConnection *conn, BitStream *stream)
{
   Parent::writePacketData(conn, stream);
}

void OpenVRTrackedObject::readPacketData(GameConnection *conn, BitStream *stream)
{
   Parent::readPacketData(conn, stream);
}

MatrixF OpenVRTrackedObject::getTrackedTransform()
{
   IDevicePose pose = OPENVR->getTrackedDevicePose(mDeviceIndex);
   MatrixF trackedMat(1);

   pose.orientation.setMatrix(&trackedMat);
   trackedMat.setPosition(pose.position);

   return trackedMat;
}

MatrixF OpenVRTrackedObject::getLastTrackedTransform()
{
   MatrixF trackedMat(1);

   mPose.orientation.setMatrix(&trackedMat);
   trackedMat.setPosition(mPose.position);

   return trackedMat;
}

MatrixF OpenVRTrackedObject::getBaseTrackingTransform()
{
   if (isMounted())
   {
      MatrixF mat;

      mMount.object->getMountTransform(mMount.node, mMount.xfm, &mat);
      if (mIgnoreParentRotation)
      {
         Point3F pos = mat.getPosition();
         mat = MatrixF(1);
         mat.setPosition(pos);
      }
      //mat.inverse();
      return mat;
   }

   return MatrixF(1);
}

void OpenVRTrackedObject::prepRenderImage(SceneRenderState *state)
{
   RenderPassManager *renderPass = state->getRenderPass();

   // debug rendering for now

   if (mDeviceIndex < 0)
      return;

   // Current pose
   IDevicePose pose = OPENVR->getTrackedDevicePose(mDeviceIndex);
   IDevicePose hmdPose = OPENVR->getTrackedDevicePose(0);

   if (!pose.connected && !mPose.connected)
      return;

   MatrixF offsetMat = getBaseTrackingTransform();
   //offsetMat.inverse();

   Point3F pos = offsetMat.getPosition();
   //Con::printf("Base offs == %f,%f,%f", pos.x, pos.y, pos.z);

   const F32 CONTROLLER_SCALE = 0.1;

   if (smDebugControllerPosition)
   {
      ColorI drawColor = ColorI::GREEN;
      if (!pose.valid)
      {
         drawColor = ColorI::RED;
      }

      // Draw Camera
      /*
      DisplayPose cameraPose;
      OPENVR->getFrameEyePose(&cameraPose, -1);
      Point3F cameraCenter(0);
      MatrixF cameraMat(1);
      cameraPose.orientation.setMatrix(&cameraMat);
      cameraMat.setPosition(cameraPose.position);
      cameraMat.mulP(cameraCenter);
      //DebugDrawer::get()->drawBox(cameraCenter - Point3F(0.1), cameraCenter + Point3F(0.1), ColorI::GREEN);
      
      DebugDrawer::get()->drawTransformedBoxOutline(Point3F(-0.5, -0.1, -0.5), Point3F(0.5, 0.1, 0.5), ColorI::WHITE, cameraMat); // general box 
      */

      // Draw Tracked HMD Pos
      Point3F hmdCenter(0, 0, 0);
      MatrixF hmdMat(1);
      hmdPose.orientation.setMatrix(&hmdMat);
      hmdMat.setPosition(hmdPose.position);
      hmdMat.inverse(); // -> world mat (as opposed to world -> tracked pos)
      hmdMat = offsetMat * hmdMat;
      hmdMat.mulP(hmdCenter);
      DebugDrawer::get()->drawBox(hmdCenter - Point3F(0.1), hmdCenter + Point3F(0.1), ColorI::RED);
      DebugDrawer::get()->drawTransformedBoxOutline(Point3F(-0.5, -0.1, -0.5), Point3F(0.5, 0.1, 0.5), ColorI::GREEN, hmdMat); // general box 


      // Draw Controller
      MatrixF mat(1);
      pose.orientation.setMatrix(&mat);
      mat.setPosition(pose.position);
      mat.inverse(); // same as HMD
      mat = offsetMat * mat;

      Point3F middleStart(0, -1 * CONTROLLER_SCALE, 0);
      Point3F middleEnd(0, 1 * CONTROLLER_SCALE, 0);
      Point3F middle(0, 0, 0);

      Point3F center(0, 0, 0);
      mat.mulP(center);

      //DebugDrawer::get()->drawBox(center - Point3F(0.1), center + Point3F(0.1), ColorI::BLUE);

      mat.mulP(middleStart);
      mat.mulP(middle);
      mat.mulP(middleEnd);

      char buffer[256];
      dSprintf(buffer, 256, "%f %f %f", center.x, center.y, center.z);
      DebugDrawer::get()->drawText(middle, buffer);
      DebugDrawer::get()->drawLine(middleStart, middle, ColorI(0, 255, 0)); // axis back
      DebugDrawer::get()->drawLine(middleEnd, middle, ColorI(255, 0, 0)); // axis forward
      DebugDrawer::get()->drawTransformedBoxOutline(Point3F(-0.5, -1, -0.5) * CONTROLLER_SCALE, Point3F(0.5, 1, 0.5) * CONTROLLER_SCALE, drawColor, mat); // general box 
      DebugDrawer::get()->drawBoxOutline(Point3F(-1), Point3F(1), ColorI::WHITE);
   }

   if (isClientObject() && smDebugControllerMovePosition)
   {
      MatrixF transform = getRenderTransform();
      transform.scale(mObjScale);
      DebugDrawer::get()->drawTransformedBoxOutline(mObjBox.minExtents, mObjBox.maxExtents, ColorI::RED, transform);
      
      // jamesu - grab server object pose for debugging
      OpenVRTrackedObject* tracked = static_cast<OpenVRTrackedObject*>(getServerObject());
      if (tracked)
      {
         mPose = tracked->mPose;
      }

      ColorI drawColor = ColorI::GREEN;
      if (!pose.valid)
      {
         drawColor = ColorI::RED;
      }
                                                                                                 // Draw Controller
      MatrixF mat(1);
      mPose.orientation.setMatrix(&mat);
      mat.setPosition(mPose.position);
      mat.inverse(); // same as HMD
      mat = offsetMat * mat;

      Point3F middleStart(0, -1 * CONTROLLER_SCALE, 0);
      Point3F middleEnd(0, 1 * CONTROLLER_SCALE, 0);
      Point3F middle(0, 0, 0);

      Point3F center(0, 0, 0);
      mat.mulP(center);

      //DebugDrawer::get()->drawBox(center - Point3F(0.1), center + Point3F(0.1), ColorI::BLUE);

      mat.mulP(middleStart);
      mat.mulP(middle);
      mat.mulP(middleEnd);

      char buffer[256];
      dSprintf(buffer, 256, "%f %f %f", center.x, center.y, center.z);
      DebugDrawer::get()->drawText(middle, buffer);
      DebugDrawer::get()->drawLine(middleStart, middle, ColorI(0, 255, 0)); // axis back
      DebugDrawer::get()->drawLine(middleEnd, middle, ColorI(255, 0, 0)); // axis forward
      DebugDrawer::get()->drawTransformedBoxOutline(Point3F(-0.5, -1, -0.5) * CONTROLLER_SCALE, Point3F(0.5, 1, 0.5) * CONTROLLER_SCALE, drawColor, mat); // general box 
      DebugDrawer::get()->drawBoxOutline(Point3F(-1), Point3F(1), ColorI::WHITE);
   }

   // Controller matrix base
   MatrixF trackedMat = getTrackedTransform();
   MatrixF invTrackedMat(1);

   invTrackedMat = trackedMat;
   invTrackedMat.inverse(); // -> world mat (as opposed to world -> tracked pos)

   invTrackedMat = getBaseTrackingTransform() * invTrackedMat;
   trackedMat = invTrackedMat;
   trackedMat.inverse();

   // Render the controllers, using either the render model or the shape
   if (mShapeInstance)
   {
      // Calculate the distance of this object from the camera
      Point3F cameraOffset = invTrackedMat.getPosition();
      cameraOffset -= state->getDiffuseCameraPosition();
      F32 dist = cameraOffset.len();
      if (dist < 0.01f)
      dist = 0.01f;

      // Set up the LOD for the shape
      F32 invScale = (1.0f / getMax(getMax(mObjScale.x, mObjScale.y), mObjScale.z));

      mShapeInstance->setDetailFromDistance(state, dist * invScale);

      // Make sure we have a valid level of detail
      if (mShapeInstance->getCurrentDetail() < 0)
         return;

      // GFXTransformSaver is a handy helper class that restores
      // the current GFX matrices to their original values when
      // it goes out of scope at the end of the function
      GFXTransformSaver saver;

      // Set up our TS render state
      TSRenderState rdata;
      rdata.setSceneState(state);
      rdata.setFadeOverride(1.0f);

      // We might have some forward lit materials
      // so pass down a query to gather lights.
      LightQuery query;
      query.init(getWorldSphere());
      rdata.setLightQuery(&query);

      // Set the world matrix to the objects render transform
      MatrixF mat = trackedMat;

      mat.scale(mObjScale);
      GFX->setWorldMatrix(mat);

      // TODO: move the nodes about for components

      mShapeInstance->animate();
      mShapeInstance->render(rdata);
   }
   else if (mRenderComponents.size() > 0)
   {
      vr::IVRRenderModels *models = OPENVR->getRenderModels();
      if (!models)
         return;

      vr::IVRSystem* vrs = vr::VRSystem();

      if (!vrs->GetControllerState(mDeviceIndex, &mCurrentControllerState))
      {
         return;
      }

      for (U32 i = 0, sz = mRenderComponents.size(); i < sz; i++)
      {
         RenderModelSlot slot = mRenderComponents[i];
         vr::RenderModel_ControllerMode_State_t modeState;
         vr::RenderModel_ComponentState_t componentState;

         modeState.bScrollWheelVisible = false;

         if (models->GetComponentState(mModelName, slot.componentName, &mCurrentControllerState, &modeState, &componentState))
         {
            MeshRenderInst *ri = renderPass->allocInst<MeshRenderInst>();

            // Set our RenderInst as a standard mesh render
            ri->type = RenderPassManager::RIT_Mesh;

            // Calculate our sorting point
            if (state && slot.nativeModel)
            {
               // Calculate our sort point manually.
               const Box3F rBox = slot.nativeModel->getWorldBox(invTrackedMat);
               ri->sortDistSq = rBox.getSqDistanceToPoint(state->getCameraPosition());
            }
            else
            {
               ri->sortDistSq = 0.0f;
            }

            MatrixF newTransform = trackedMat;
            MatrixF controllerOffsMat = OpenVRUtil::convertSteamVRAffineMatrixToMatrixFPlain(componentState.mTrackingToComponentRenderModel);
            MatrixF offComponentMat(1);
            OpenVRUtil::convertTransformFromOVR(controllerOffsMat, offComponentMat);

            newTransform = offComponentMat * newTransform;

            newTransform.inverse();

            //DebugDrawer::get()->drawBox(newTransform.getPosition() - Point3F(0.001), newTransform.getPosition() + Point3F(0.001), ColorI::BLUE);

            if (!slot.nativeModel)
               continue;
            if (i < 1)
               continue;

            // Set up our transforms
            ri->objectToWorld = renderPass->allocUniqueXform(newTransform);
            ri->worldToCamera = renderPass->allocSharedXform(RenderPassManager::View);
            ri->projection = renderPass->allocSharedXform(RenderPassManager::Projection);

            // If our material needs lights then fill the RIs
            // light vector with the best lights.
            if (true)
            {
               LightQuery query;
               Point3F center(0, 0, 0);
               invTrackedMat.mulP(center);
               query.init(SphereF(center, 10.0f));
               query.getLights(ri->lights, 8);
            }

            // Draw model
            slot.nativeModel->draw(state, ri);
            state->getRenderPass()->addInst(ri);
         }
      }
   }
   else if (mBasicModel)
   {
      MeshRenderInst *ri = renderPass->allocInst<MeshRenderInst>();

      // Set our RenderInst as a standard mesh render
      ri->type = RenderPassManager::RIT_Mesh;

      // Calculate our sorting point
      if (state)
      {
         // Calculate our sort point manually.
         const Box3F rBox = mBasicModel->getWorldBox(invTrackedMat);
         ri->sortDistSq = rBox.getSqDistanceToPoint(state->getCameraPosition());
      }
      else
      {
         ri->sortDistSq = 0.0f;
      }

      MatrixF newTransform = invTrackedMat;
      // Set up our transforms
      ri->objectToWorld = renderPass->allocUniqueXform(newTransform);
      ri->worldToCamera = renderPass->allocSharedXform(RenderPassManager::View);
      ri->projection = renderPass->allocSharedXform(RenderPassManager::Projection);

      // If our material needs lights then fill the RIs
      // light vector with the best lights.
      if (true)
      {
         LightQuery query;
         Point3F center(0, 0, 0);
         invTrackedMat.mulP(center);
         query.init(SphereF(center, 10.0f));
         query.getLights(ri->lights, 8);
      }

      // Draw model
      mBasicModel->draw(state, ri);
      state->getRenderPass()->addInst(ri);
   }
}

U32 OpenVRTrackedObject::getCollisionMask()
{
   if (isServerObject())
      return sServerCollisionMask;
   else
      return sClientCollisionMask;
}

void OpenVRTrackedObject::updateWorkingCollisionSet()
{
   const U32 mask = getCollisionMask();
   Box3F convexBox = mConvexList->getBoundingBox(getTransform(), getScale());
   F32 len = (50) * TickSec;
   F32 l = (len * 1.1) + 0.1;  // fudge factor
   convexBox.minExtents -= Point3F(l, l, l);
   convexBox.maxExtents += Point3F(l, l, l);

   disableCollision();
   mConvexList->updateWorkingList(convexBox, mask);
   enableCollision();
}

void OpenVRTrackedObject::updateMove(const Move *move)
{
   // Set transform based on move

#ifdef TORQUE_EXTENDED_MOVE

   const ExtendedMove* emove = dynamic_cast<const ExtendedMove*>(move);
   if (!emove)
      return;

   U32 emoveIndex = mMappedMoveIndex;
   if (emoveIndex >= ExtendedMove::MaxPositionsRotations)
      emoveIndex = 0;

   //IDevicePose pose = OPENVR->getTrackedDevicePose(mDeviceIndex);
   //Con::printf("OpenVRTrackedObject::processTick move %i", emoveIndex);

   if (!emove->EulerBasedRotation[emoveIndex])
   {
      AngAxisF inRot = AngAxisF(Point3F(emove->rotX[emoveIndex], emove->rotY[emoveIndex], emove->rotZ[emoveIndex]), emove->rotW[emoveIndex]);
      // Update our pose based on the move info
      mPose.orientation = inRot;
      mPose.position = Point3F(emove->posX[emoveIndex], emove->posY[emoveIndex], emove->posZ[emoveIndex]);
      mPose.valid = true;
      mPose.connected = true;
   }

   // Set transform based on move pose
   MatrixF trackedMat(1);
   MatrixF invTrackedMat(1);

   mPose.orientation.setMatrix(&trackedMat);
   trackedMat.setPosition(mPose.position);

   invTrackedMat = trackedMat;
   invTrackedMat.inverse(); // -> world mat (as opposed to world -> tracked pos)

   invTrackedMat = getBaseTrackingTransform() * invTrackedMat;
   trackedMat = invTrackedMat;
   trackedMat.inverse();

   SceneObject::setTransform(invTrackedMat);

   if (mPhysicsRep)
      mPhysicsRep->setTransform(invTrackedMat);
#endif
}

void OpenVRTrackedObject::processTick(const Move *move)
{
   // Perform collision checks
   if (isServerObject())
   {
      updateMove(move);

      if (!mPhysicsRep)
      {
         updateWorkingCollisionSet();
      }
   }

   Parent::processTick(move);
}

void OpenVRTrackedObject::interpolateTick(F32 delta)
{
   // Set latest transform

   Parent::interpolateTick(delta);
}

void OpenVRTrackedObject::advanceTime(F32 dt)
{
   Parent::advanceTime(dt);
}

bool OpenVRTrackedObject::castRay(const Point3F &start, const Point3F &end, RayInfo* info)
{
   if (!mPose.connected || !mPose.valid)
      return false;

   // Collide against bounding box.
   F32 st, et, fst = 0.0f, fet = 1.0f;
   F32 *bmin = &mObjBox.minExtents.x;
   F32 *bmax = &mObjBox.maxExtents.x;
   F32 const *si = &start.x;
   F32 const *ei = &end.x;

   for (S32 i = 0; i < 3; i++) {
      if (*si < *ei) {
         if (*si > *bmax || *ei < *bmin)
            return false;
         F32 di = *ei - *si;
         st = (*si < *bmin) ? (*bmin - *si) / di : 0.0f;
         et = (*ei > *bmax) ? (*bmax - *si) / di : 1.0f;
      }
      else {
         if (*ei > *bmax || *si < *bmin)
            return false;
         F32 di = *ei - *si;
         st = (*si > *bmax) ? (*bmax - *si) / di : 0.0f;
         et = (*ei < *bmin) ? (*bmin - *si) / di : 1.0f;
      }
      if (st > fst) fst = st;
      if (et < fet) fet = et;
      if (fet < fst)
         return false;
      bmin++; bmax++;
      si++; ei++;
   }

   info->normal = start - end;
   info->normal.normalizeSafe();
   getTransform().mulV(info->normal);

   info->t = fst;
   info->object = this;
   info->point.interpolate(start, end, fst);
   info->material = 0;
   return true;
}

void OpenVRTrackedObject::buildConvex(const Box3F& box, Convex* convex)
{
   // These should really come out of a pool
   mConvexList->collectGarbage();

   Box3F realBox = box;
   mWorldToObj.mul(realBox);
   realBox.minExtents.convolveInverse(mObjScale);
   realBox.maxExtents.convolveInverse(mObjScale);

   if (realBox.isOverlapped(getObjBox()) == false)
      return;

   // Just return a box convex for the entire shape...
   Convex* cc = 0;
   CollisionWorkingList& wl = convex->getWorkingList();
   for (CollisionWorkingList* itr = wl.wLink.mNext; itr != &wl; itr = itr->wLink.mNext) {
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

bool OpenVRTrackedObject::testObject(SceneObject* enter)
{
   return false; // TODO
}

DefineEngineMethod(OpenVRTrackedObject, setModelName, void, (String modelName),, "Set model name. Typically you should do this from the client to update the server representation.")
{
   object->setModelName(modelName);
}
