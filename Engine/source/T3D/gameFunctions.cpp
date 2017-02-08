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
#include "T3D/gameFunctions.h"
#include "T3D/gameBase/gameConnection.h"
#include "T3D/camera.h"
#include "T3D/sfx/sfx3DWorld.h"
#include "console/consoleTypes.h"
#include "gui/3d/guiTSControl.h"
#include "core/util/journal/process.h"
#include "materials/materialManager.h"
#include "math/mEase.h"
#include "core/module.h"
#include "console/engineAPI.h"
#include "platform/output/IDisplayDevice.h"

static void RegisterGameFunctions();
static void Process3D();

MODULE_BEGIN( 3D )

   MODULE_INIT_AFTER( Process )
   MODULE_INIT_AFTER( Scene )
   
   MODULE_SHUTDOWN_BEFORE( Process )
   MODULE_SHUTDOWN_BEFORE( Sim )
   MODULE_SHUTDOWN_AFTER( Scene )
   
   MODULE_INIT
   {
      Process::notify(Process3D, PROCESS_TIME_ORDER);

      GameConnection::smFovUpdate.notify(GameSetCameraFov);

      RegisterGameFunctions();
   }
   
   MODULE_SHUTDOWN
   {
      GameConnection::smFovUpdate.remove(GameSetCameraFov);

      Process::remove(Process3D);
   }

MODULE_END;


static S32 gEaseInOut = Ease::InOut;
static S32 gEaseIn = Ease::In;
static S32 gEaseOut = Ease::Out;

static S32 gEaseLinear = Ease::Linear;
static S32 gEaseQuadratic= Ease::Quadratic;
static S32 gEaseCubic= Ease::Cubic;
static S32 gEaseQuartic = Ease::Quartic;
static S32 gEaseQuintic = Ease::Quintic;
static S32 gEaseSinusoidal= Ease::Sinusoidal;
static S32 gEaseExponential = Ease::Exponential;
static S32 gEaseCircular = Ease::Circular;
static S32 gEaseElastic = Ease::Elastic;
static S32 gEaseBack = Ease::Back;
static S32 gEaseBounce = Ease::Bounce;	


extern bool gEditingMission;

extern void ShowInit();

//------------------------------------------------------------------------------
/// Camera and FOV info
namespace CameraAndFOV{

   const  U32 MaxZoomSpeed             = 2000;     ///< max number of ms to reach target FOV

   static F32 sConsoleCameraFov        = 90.f;     ///< updated to camera FOV each frame
   static F32 sDefaultFov              = 90.f;     ///< normal FOV
   static F32 sCameraFov               = 90.f;     ///< current camera FOV
   static F32 sTargetFov               = 90.f;     ///< the desired FOV
   static F32 sLastCameraUpdateTime    = 0;        ///< last time camera was updated
   static S32 sZoomSpeed               = 500;      ///< ms per 90deg fov change

   /// A scale to apply to the normal visible distance
   /// typically used for tuning performance.
   static F32 sVisDistanceScale = 1.0f;

} // namespace {}

// query
static SimpleQueryList sgServerQueryList;
static U32 sgServerQueryIndex = 0;

//SERVER FUNCTIONS ONLY
ConsoleFunctionGroupBegin( Containers, "Spatial query functions. <b>Server side only!</b>");

DefineConsoleFunction( containerFindFirst, const char*, (U32 typeMask, Point3F origin, Point3F size), , "(int mask, Point3F point, float x, float y, float z)"
   "@brief Find objects matching the bitmask type within a box centered at point, with extents x, y, z.\n\n"
   "@returns The first object found, or an empty string if nothing was found.  Thereafter, you can get more "
   "results using containerFindNext()."
   "@see containerFindNext\n"
   "@ingroup Game")
{
   //find out what we're looking for

   //build the container volume
   Box3F queryBox;
   queryBox.minExtents = origin;
   queryBox.maxExtents = origin;
   queryBox.minExtents -= size;
   queryBox.maxExtents += size;

   //initialize the list, and do the query
   sgServerQueryList.mList.clear();
   gServerContainer.findObjects(queryBox, typeMask, SimpleQueryList::insertionCallback, &sgServerQueryList);

   //return the first element
   sgServerQueryIndex = 0;
   static const U32 bufSize = 100;
   char *buff = Con::getReturnBuffer(bufSize);
   if (sgServerQueryList.mList.size())
      dSprintf(buff, bufSize, "%d", sgServerQueryList.mList[sgServerQueryIndex++]->getId());
   else
      buff[0] = '\0';

   return buff;
}

DefineConsoleFunction( containerFindNext, const char*, (), , "()"
   "@brief Get more results from a previous call to containerFindFirst().\n\n"
   "@note You must call containerFindFirst() to begin the search.\n"
   "@returns The next object found, or an empty string if nothing else was found.\n"
   "@see containerFindFirst()\n"
	"@ingroup Game")
{
   //return the next element
   static const U32 bufSize = 100;
   char *buff = Con::getReturnBuffer(bufSize);
   if (sgServerQueryIndex < sgServerQueryList.mList.size())
      dSprintf(buff, bufSize, "%d", sgServerQueryList.mList[sgServerQueryIndex++]->getId());
   else
      buff[0] = '\0';

   return buff;
}

ConsoleFunctionGroupEnd( Containers );

//------------------------------------------------------------------------------

bool GameGetCameraTransform(MatrixF *mat, Point3F *velocity)
{
   // Return the position and velocity of the control object
   GameConnection* connection = GameConnection::getConnectionToServer();
   return connection && connection->getControlCameraTransform(0, mat) &&
      connection->getControlCameraVelocity(velocity);
}

//------------------------------------------------------------------------------
DefineEngineFunction( setDefaultFov, void, ( F32 defaultFOV ),,
				"@brief Set the default FOV for a camera.\n"
            "@param defaultFOV The default field of view in degrees\n"
				"@ingroup CameraSystem")
{
   CameraAndFOV::sDefaultFov = mClampF(defaultFOV, MinCameraFov, MaxCameraFov);
   if(CameraAndFOV::sCameraFov == CameraAndFOV::sTargetFov)
      CameraAndFOV::sTargetFov = CameraAndFOV::sDefaultFov;
}

DefineEngineFunction( setZoomSpeed, void, ( S32 speed ),,
				"@brief Set the zoom speed of the camera.\n"
            "This affects how quickly the camera changes from one field of view "
            "to another.\n"
            "@param speed The camera's zoom speed in ms per 90deg FOV change\n"
				"@ingroup CameraSystem")
{
   CameraAndFOV::sZoomSpeed = mClamp(speed, 0, CameraAndFOV::MaxZoomSpeed);
}

DefineEngineFunction( setFov, void, ( F32 FOV ),,
				"@brief Set the FOV of the camera.\n"
            "@param FOV The camera's new FOV in degrees\n"
				"@ingroup CameraSystem")
{
   CameraAndFOV::sTargetFov = mClampF(FOV, MinCameraFov, MaxCameraFov);
}

F32 GameGetCameraFov()
{
   return(CameraAndFOV::sCameraFov);
}

void GameSetCameraFov(F32 fov)
{
   CameraAndFOV::sTargetFov = CameraAndFOV::sCameraFov = fov;
}

void GameSetCameraTargetFov(F32 fov)
{
   CameraAndFOV::sTargetFov = fov;
}

void GameUpdateCameraFov()
{
   F32 time = F32(Platform::getVirtualMilliseconds());

   // need to update fov?
   if(CameraAndFOV::sTargetFov != CameraAndFOV::sCameraFov)
   {
      F32 delta = time - CameraAndFOV::sLastCameraUpdateTime;

      // snap zoom?
      if((CameraAndFOV::sZoomSpeed == 0) || (delta <= 0.0f))
         CameraAndFOV::sCameraFov = CameraAndFOV::sTargetFov;
      else
      {
         // gZoomSpeed is time in ms to zoom 90deg
         F32 step = 90.f * (delta / F32(CameraAndFOV::sZoomSpeed));

         if(CameraAndFOV::sCameraFov > CameraAndFOV::sTargetFov)
         {
            CameraAndFOV::sCameraFov -= step;
            if(CameraAndFOV::sCameraFov < CameraAndFOV::sTargetFov)
               CameraAndFOV::sCameraFov = CameraAndFOV::sTargetFov;
         }
         else
         {
            CameraAndFOV::sCameraFov += step;
            if(CameraAndFOV::sCameraFov > CameraAndFOV::sTargetFov)
               CameraAndFOV::sCameraFov = CameraAndFOV::sTargetFov;
         }
      }
   }

   // the game connection controls the vertical and the horizontal
   GameConnection * connection = GameConnection::getConnectionToServer();
   if(connection)
   {
      // check if fov is valid on control object
      if(connection->isValidControlCameraFov(CameraAndFOV::sCameraFov))
         connection->setControlCameraFov(CameraAndFOV::sCameraFov);
      else
      {
         // will set to the closest fov (fails only on invalid control object)
         if(connection->setControlCameraFov(CameraAndFOV::sCameraFov))
         {
            F32 setFov = CameraAndFOV::sCameraFov;
            connection->getControlCameraFov(&setFov);
            CameraAndFOV::sTargetFov =CameraAndFOV::sCameraFov = setFov;
         }
      }
   }

   // update the console variable
   CameraAndFOV::sConsoleCameraFov = CameraAndFOV::sCameraFov;
   CameraAndFOV::sLastCameraUpdateTime = time;
}
//--------------------------------------------------------------------------

#ifdef TORQUE_DEBUG
// ConsoleFunction(dumpTSShapes, void, 1, 1, "dumpTSShapes();")
// {
//    argc, argv;

//    FindMatch match("*.dts", 4096);
//    gResourceManager->findMatches(&match);

//    for (U32 i = 0; i < match.numMatches(); i++)
//    {
//       U32 j;
//       Resource<TSShape> shape = ResourceManager::get().load(match.matchList[i]);
//       if (bool(shape) == false)
//          Con::errorf(" aaa Couldn't load: %s", match.matchList[i]);

//       U32 numMeshes = 0, numSkins = 0;
//       for (j = 0; j < shape->meshes.size(); j++)
//          if (shape->meshes[j])
//             numMeshes++;
//       for (j = 0; j < shape->skins.size(); j++)
//          if (shape->skins[j])
//             numSkins++;

//      Con::printf(" aaa Shape: %s (%d meshes, %d skins)", match.matchList[i], numMeshes, numSkins);
//       Con::printf(" aaa   Meshes");
//       for (j = 0; j < shape->meshes.size(); j++)
//       {
//          if (shape->meshes[j])
//             Con::printf(" aaa     %d -> nf: %d, nmf: %d, nvpf: %d (%d, %d, %d, %d, %d)",
//                         shape->meshes[j]->meshType & TSMesh::TypeMask,
//                         shape->meshes[j]->numFrames,
//                         shape->meshes[j]->numMatFrames,
//                         shape->meshes[j]->vertsPerFrame,
//                         shape->meshes[j]->verts.size(),
//                         shape->meshes[j]->norms.size(),
//                         shape->meshes[j]->tverts.size(),
//                         shape->meshes[j]->primitives.size(),
//                         shape->meshes[j]->indices.size());
//       }
//       Con::printf(" aaa   Skins");
//       for (j = 0; j < shape->skins.size(); j++)
//       {
//          if (shape->skins[j])
//             Con::printf(" aaa     %d -> nf: %d, nmf: %d, nvpf: %d (%d, %d, %d, %d, %d)",
//                         shape->skins[j]->meshType & TSMesh::TypeMask,
//                         shape->skins[j]->numFrames,
//                         shape->skins[j]->numMatFrames,
//                         shape->skins[j]->vertsPerFrame,
//                         shape->skins[j]->verts.size(),
//                         shape->skins[j]->norms.size(),
//                         shape->skins[j]->tverts.size(),
//                         shape->skins[j]->primitives.size(),
//                         shape->skins[j]->indices.size());
//       }
//    }
// }
#endif

bool GameProcessCameraQuery(CameraQuery *query)
{
   GameConnection* connection = GameConnection::getConnectionToServer();

   if (connection && connection->getControlCameraTransform(0.032f, &query->cameraMatrix))
   {
      query->object = dynamic_cast<GameBase*>(connection->getCameraObject());
      query->nearPlane = gClientSceneGraph->getNearClip();

      // Scale the normal visible distance by the performance 
      // tuning scale which we never let over 1.
      CameraAndFOV::sVisDistanceScale = mClampF( CameraAndFOV::sVisDistanceScale, 0.01f, 1.0f );
      query->farPlane = gClientSceneGraph->getVisibleDistance() * CameraAndFOV::sVisDistanceScale;

      // Provide some default values
      query->stereoTargets[0] = 0;
      query->stereoTargets[1] = 0;
      query->eyeOffset[0] = Point3F::Zero;
      query->eyeOffset[1] = Point3F::Zero;
      query->hasFovPort = false;
      query->hasStereoTargets = false;
      query->displayDevice = NULL;
      
      F32 cameraFov = 0.0f;
      bool fovSet = false;

      // Try to use the connection's display deivce, if any, but only if the editor
      // is not open
      if(!gEditingMission && connection->hasDisplayDevice())
      {
         IDisplayDevice* display = connection->getDisplayDevice();

         query->displayDevice = display;

         // Note: all eye values are invalid until this is called
         display->setDrawCanvas(query->drawCanvas);

         display->setCurrentConnection(connection);

         // Display may activate AFTER so we need to call this again just in case
         display->onStartFrame();

         // The connection's display device may want to set the eye offset
         if(display->providesEyeOffsets())
         {
            display->getEyeOffsets(query->eyeOffset);
         }

         // Grab field of view for both eyes
         if (display->providesFovPorts())
         {
            display->getFovPorts(query->fovPort);
            fovSet = true;
            query->hasFovPort = true;
         }
         
         // Grab the latest overriding render view transforms
         connection->getControlCameraEyeTransforms(display, query->eyeTransforms);
         connection->getControlCameraHeadTransform(display, &query->headMatrix);

         display->getStereoViewports(query->stereoViewports);
         display->getStereoTargets(query->stereoTargets);
         query->hasStereoTargets = true;
      }
      else
      {
         query->eyeTransforms[0] = query->cameraMatrix;
         query->eyeTransforms[1] = query->cameraMatrix;
         query->headMatrix = query->cameraMatrix;
      }

      // Use the connection's FOV settings if requried
      if(!connection->getControlCameraFov(&cameraFov))
      {
         return false;
      }

      query->fov = mDegToRad(cameraFov);
      return true;
   }
   return false;
}

void GameRenderWorld()
{
   PROFILE_START(GameRenderWorld);
   FrameAllocator::setWaterMark(0);

   gClientSceneGraph->renderScene( SPT_Diffuse );

   // renderScene leaves some states dirty, which causes problems if GameTSCtrl is the last Gui object rendered
   GFX->updateStates();

   AssertFatal(FrameAllocator::getWaterMark() == 0,
      "Error, someone didn't reset the water mark on the frame allocator!");
   FrameAllocator::setWaterMark(0);
   PROFILE_END();
}


static void Process3D()
{
   MATMGR->updateTime();
   
   // Update the SFX world, if there is one.
   
   if( gSFX3DWorld )
      gSFX3DWorld->update();
}

static void RegisterGameFunctions()
{
   Con::addVariable( "$pref::Camera::distanceScale", TypeF32, &CameraAndFOV::sVisDistanceScale, 
      "A scale to apply to the normal visible distance, typically used for tuning performance.\n"
	   "@ingroup Game");
   Con::addVariable( "$cameraFov", TypeF32, &CameraAndFOV::sConsoleCameraFov, 
      "The camera's Field of View.\n\n"
	   "@ingroup Game" );

   // Stuff game types into the console
   Con::setIntVariable("$TypeMasks::StaticObjectType",         StaticObjectType);
   Con::setIntVariable("$TypeMasks::EnvironmentObjectType",    EnvironmentObjectType);
   Con::setIntVariable("$TypeMasks::TerrainObjectType",        TerrainObjectType);
   Con::setIntVariable("$TypeMasks::WaterObjectType",          WaterObjectType);
   Con::setIntVariable("$TypeMasks::TriggerObjectType",        TriggerObjectType);
   Con::setIntVariable("$TypeMasks::MarkerObjectType",         MarkerObjectType);
   Con::setIntVariable("$TypeMasks::GameBaseObjectType",       GameBaseObjectType);
   Con::setIntVariable("$TypeMasks::ShapeBaseObjectType",      ShapeBaseObjectType);
   Con::setIntVariable("$TypeMasks::CameraObjectType",         CameraObjectType);
   Con::setIntVariable("$TypeMasks::StaticShapeObjectType",    StaticShapeObjectType);
   Con::setIntVariable("$TypeMasks::DynamicShapeObjectType",   DynamicShapeObjectType);
   Con::setIntVariable("$TypeMasks::PlayerObjectType",         PlayerObjectType);
   Con::setIntVariable("$TypeMasks::ItemObjectType",           ItemObjectType);
   Con::setIntVariable("$TypeMasks::VehicleObjectType",        VehicleObjectType);
   Con::setIntVariable("$TypeMasks::VehicleBlockerObjectType", VehicleBlockerObjectType);
   Con::setIntVariable("$TypeMasks::ProjectileObjectType",     ProjectileObjectType);
   Con::setIntVariable("$TypeMasks::ExplosionObjectType",      ExplosionObjectType);
   Con::setIntVariable("$TypeMasks::CorpseObjectType",         CorpseObjectType);
   Con::setIntVariable("$TypeMasks::DebrisObjectType",         DebrisObjectType);
   Con::setIntVariable("$TypeMasks::PhysicalZoneObjectType",   PhysicalZoneObjectType);
   Con::setIntVariable("$TypeMasks::LightObjectType",          LightObjectType);

   Con::addVariable("Ease::InOut", TypeS32, &gEaseInOut, 
      "InOut ease for curve movement.\n"
	   "@ingroup Game");
   Con::addVariable("Ease::In", TypeS32, &gEaseIn, 
      "In ease for curve movement.\n"
	   "@ingroup Game");
   Con::addVariable("Ease::Out", TypeS32, &gEaseOut, 
      "Out ease for curve movement.\n"
	   "@ingroup Game");

   Con::addVariable("Ease::Linear", TypeS32, &gEaseLinear, 
      "Linear ease for curve movement.\n"
	   "@ingroup Game");
   Con::addVariable("Ease::Quadratic", TypeS32, &gEaseQuadratic, 
      "Quadratic ease for curve movement.\n"
	   "@ingroup Game");
   Con::addVariable("Ease::Cubic", TypeS32, &gEaseCubic, 
      "Cubic ease for curve movement.\n"
	   "@ingroup Game");
   Con::addVariable("Ease::Quartic", TypeS32, &gEaseQuartic, 
      "Quartic ease for curve movement.\n"
	   "@ingroup Game");
   Con::addVariable("Ease::Quintic", TypeS32, &gEaseQuintic, 
      "Quintic ease for curve movement.\n"
	   "@ingroup Game");
   Con::addVariable("Ease::Sinusoidal", TypeS32, &gEaseSinusoidal, 
      "Sinusoidal ease for curve movement.\n"
	   "@ingroup Game");
   Con::addVariable("Ease::Exponential", TypeS32, &gEaseExponential, 
      "Exponential ease for curve movement.\n"
	   "@ingroup Game");
   Con::addVariable("Ease::Circular", TypeS32, &gEaseCircular, 
      "Circular ease for curve movement.\n"
	   "@ingroup Game");
   Con::addVariable("Ease::Elastic", TypeS32, &gEaseElastic, 
      "Elastic ease for curve movement.\n"
	   "@ingroup Game");
   Con::addVariable("Ease::Back", TypeS32, &gEaseBack, 
      "Backwards ease for curve movement.\n"
	   "@ingroup Game");
   Con::addVariable("Ease::Bounce", TypeS32, &gEaseBounce, 
      "Bounce ease for curve movement.\n"
	   "@ingroup Game");
}
