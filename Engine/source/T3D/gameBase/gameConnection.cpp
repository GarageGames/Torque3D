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
#include "T3D/gameBase/gameConnection.h"

#include "platform/profiler.h"
#include "core/dnet.h"
#include "core/util/safeDelete.h"
#include "core/stream/bitStream.h"
#include "console/consoleTypes.h"
#include "console/simBase.h"
#include "sfx/sfxProfile.h"
#include "sfx/sfxDescription.h"
#include "app/game.h"
#include "app/auth.h"
#include "T3D/camera.h"
#include "T3D/gameBase/gameProcess.h"
#include "T3D/gameBase/gameConnectionEvents.h"
#include "console/engineAPI.h"
#include "math/mTransform.h"

#ifdef TORQUE_EXPERIMENTAL_EC
#include "T3D/entity.h"
#include "T3D/components/coreInterfaces.h"
#endif

#ifdef TORQUE_HIFI_NET
   #include "T3D/gameBase/hifi/hifiMoveList.h"
#elif defined TORQUE_EXTENDED_MOVE
   #include "T3D/gameBase/extended/extendedMoveList.h"
#else
   #include "T3D/gameBase/std/stdMoveList.h"
#endif

//----------------------------------------------------------------------------
#define MAX_MOVE_PACKET_SENDS 4

#define ControlRequestTime 5000

const U32 GameConnection::CurrentProtocolVersion = 12;
const U32 GameConnection::MinRequiredProtocolVersion = 12;

//----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(GameConnection);
S32 GameConnection::mLagThresholdMS = 0;
Signal<void(F32)> GameConnection::smFovUpdate;
Signal<void()>    GameConnection::smPlayingDemo;

ConsoleDocClass( GameConnection,
   "@brief The game-specific subclass of NetConnection.\n\n"
   
   "The GameConnection introduces the concept of the control object.  The control object "
   "is simply the object that the client is associated with that network connection controls.  By "
   "default the control object is an instance of the Player class, but can also be an instance "
   "of Camera (when editing the mission, for example), or any other ShapeBase derived class as "
   "appropriate for the game.\n\n"

   "Torque uses a model in which the server is the authoritative master of the simulation.  To "
   "prevent clients from cheating, the server simulates all player moves and then tells the "
   "client where his player is in the world.  This model, while secure, can have problems.  If "
   "the network latency is high, this round-trip time can give the player a very noticeable sense "
   "of movement lag.  To correct this problem, the game uses a form of prediction - it simulates "
   "the movement of the control object on the client and on the server both.  This way the client "
   "doesn't need to wait for round-trip verification of his moves.  Only in the case of a force "
   "acting on the control object on the server that doesn't exist on the client does the client's "
   "position need to be forcefully changed.\n\n"

   "To support this, all control objects (derivative of ShapeBase) must supply a writePacketData() "
   "and readPacketData() function that send enough data to accurately simulate the object on the "
   "client.  These functions are only called for the current control object, and only when the "
   "server can determine that the client's simulation is somehow out of sync with the server.  This "
   "occurs usually if the client is affected by a force not present on the server (like an "
   "interpolating object) or if the server object is affected by a server only force (such as the "
   "impulse from an explosion).\n\n"

   "The Move structure is a 32 millisecond snapshot of player input, containing x, y, and z "
   "positional and rotational changes as well as trigger state changes. When time passes in the "
   "simulation moves are collected (depending on how much time passes), and applied to the current "
   "control object on the client. The same moves are then packed over to the server in "
   "GameConnection::writePacket(), for processing on the server's version of the control object.\n\n"

   "@see @ref Networking, NetConnection, ShapeBase\n\n"

   "@ingroup Networking\n");

//----------------------------------------------------------------------------
IMPLEMENT_CALLBACK( GameConnection, onConnectionTimedOut, void, (), (),
   "@brief Called on the client when the connection to the server times out.\n\n");

IMPLEMENT_CALLBACK( GameConnection, onConnectionAccepted, void, (), (),
   "@brief Called on the client when the connection to the server has been established.\n\n");

IMPLEMENT_CALLBACK( GameConnection, onConnectRequestTimedOut, void, (), (),
   "@brief Called when connection attempts have timed out.\n\n");

IMPLEMENT_CALLBACK( GameConnection, onConnectionDropped, void, (const char* reason), (reason),
   "@brief Called on the client when the connection to the server has been dropped.\n\n"
   "@param reason The reason why the connection was dropped.\n\n");

IMPLEMENT_CALLBACK( GameConnection, onConnectRequestRejected, void, (const char* reason), (reason),
   "@brief Called on the client when the connection to the server has been rejected.\n\n"
   "@param reason The reason why the connection request was rejected.\n\n");

IMPLEMENT_CALLBACK( GameConnection, onConnectionError, void, (const char* errorString), (errorString),
   "@brief Called on the client when there is an error with the connection to the server.\n\n"
   "@param errorString The connection error text.\n\n");

IMPLEMENT_CALLBACK( GameConnection, onDrop, void, (const char* disconnectReason), (disconnectReason),
   "@brief Called on the server when the client's connection has been dropped.\n\n"
   "@param disconnectReason The reason why the connection was dropped.\n\n");

IMPLEMENT_CALLBACK( GameConnection, initialControlSet, void, (), (),
   "@brief Called on the client when the first control object has been set by the "
   "server and we are now ready to go.\n\n"
   "A common action to perform when this callback is called is to switch the GUI "
   "canvas from the loading screen and over to the 3D game GUI.");

IMPLEMENT_CALLBACK( GameConnection, onControlObjectChange, void, (), (),
   "@brief Called on the client when the control object has been changed by the "
   "server.\n\n");

IMPLEMENT_CALLBACK( GameConnection, setLagIcon, void, (bool state), (state),
   "@brief Called on the client to display the lag icon.\n\n"
   "When the connection with the server is lagging, this callback is called to "
   "allow the game GUI to display some indicator to the player.\n\n"
   "@param state Set to true if the lag icon should be displayed.\n\n");

IMPLEMENT_CALLBACK( GameConnection, onDataBlocksDone, void, (U32 sequence), (sequence),
   "@brief Called on the server when all datablocks has been sent to the client.\n\n"
   "During phase 1 of the mission download, all datablocks are sent from the server "
   "to the client.  Once all datablocks have been sent, this callback is called and "
   "the mission download procedure may move on to the next phase.\n\n"
   "@param sequence The sequence is common between the server and client and ensures "
   "that the client is acting on the most recent mission start process.  If an errant "
   "network packet (one that was lost but has now been found) is received by the client "
   "with an incorrect sequence, it is just ignored.  This sequence number is updated on "
   "the server every time a mission is loaded.\n\n"
   "@see GameConnection::transmitDataBlocks()\n\n");

IMPLEMENT_GLOBAL_CALLBACK( onDataBlockObjectReceived, void, (U32 index, U32 total), (index, total),
   "@brief Called on the client each time a datablock has been received.\n\n"
   "This callback is typically used to notify the player of how far along "
   "in the datablock download process they are.\n\n"
   "@param index The index of the datablock just received.\n"
   "@param total The total number of datablocks to be received.\n\n"
   "@see GameConnection, GameConnection::transmitDataBlocks(), GameConnection::onDataBlocksDone()\n\n"
   "@ingroup Networking\n");

IMPLEMENT_CALLBACK( GameConnection, onFlash, void, (bool state), (state),
   "@brief Called on the client when the damage flash or white out states change.\n\n"
   "When the server changes the damage flash or white out values, this callback is called "
   "either is on or both are off.  Typically this is used to enable the flash postFx.\n\n"
   "@param state Set to true if either the damage flash or white out conditions are active.\n\n");

//----------------------------------------------------------------------------
GameConnection::GameConnection()
{
   mLagging = false;
   mControlObject = NULL;
   mCameraObject = NULL;

#ifdef TORQUE_HIFI_NET
   mMoveList = new HifiMoveList();
#elif defined TORQUE_EXTENDED_MOVE
   mMoveList = new ExtendedMoveList();
#else
   mMoveList = new StdMoveList();
#endif

   mMoveList->setConnection( this );

   mDataBlockModifiedKey = 0;
   mMaxDataBlockModifiedKey = 0;
   mAuthInfo = NULL;
   mControlForceMismatch = false;
   mConnectArgc = 0;

   for(U32 i = 0; i < MaxConnectArgs; i++)
      mConnectArgv[i] = 0;

   mJoinPassword = NULL;

   mMissionCRC = 0xffffffff;

   mDamageFlash = mWhiteOut = 0;

   mCameraPos = 0;
   mCameraSpeed = 10;

   mCameraFov = 90.f;
   mUpdateCameraFov = false;

   mAIControlled = false;

   mLastPacketTime = 0;

   mDisconnectReason[0] = 0;

   //blackout vars
   mBlackOut = 0.0f;
   mBlackOutTimeMS = 0;
   mBlackOutStartTimeMS = 0;
   mFadeToBlack = false;

   // first person
   mFirstPerson = true;
   mUpdateFirstPerson = false;

   // Control scheme
   mUpdateControlScheme = false;
   mAbsoluteRotation = false;
   mAddYawToAbsRot = false;
   mAddPitchToAbsRot = false;

   mVisibleGhostDistance = 0.0f;

   clearDisplayDevice();
}

GameConnection::~GameConnection()
{
   setDisplayDevice(NULL);
   delete mAuthInfo;
   for(U32 i = 0; i < mConnectArgc; i++)
      dFree(mConnectArgv[i]);
   dFree(mJoinPassword);
   delete mMoveList;
}

//----------------------------------------------------------------------------

void GameConnection::setVisibleGhostDistance(F32 dist)
{
   mVisibleGhostDistance = dist;
}

F32 GameConnection::getVisibleGhostDistance()
{
   return mVisibleGhostDistance;
}

bool GameConnection::canRemoteCreate()
{
   return true;
}

void GameConnection::setConnectArgs(U32 argc, const char **argv)
{
   if(argc > MaxConnectArgs)
      argc = MaxConnectArgs;
   mConnectArgc = argc;
   for(U32 i = 0; i < mConnectArgc; i++)
      mConnectArgv[i] = dStrdup(argv[i]);
}

void GameConnection::setJoinPassword(const char *password)
{
   mJoinPassword = dStrdup(password);
}

DefineEngineMethod( GameConnection, setJoinPassword, void, (const char* password),,
              "@brief On the client, set the password that will be passed to the server.\n\n"
              
              "On the server, this password is compared with what is stored in $pref::Server::Password.  "
              "If $pref::Server::Password is empty then the client's sent password is ignored.  Otherwise, "
              "if the passed in client password and the server password do not match, the CHR_PASSWORD "
              "error string is sent back to the client and the connection is immediately terminated.\n\n"
              
              "This password checking is performed quite early on in the connection request process so as "
              "to minimize the impact of multiple failed attempts -- also known as hacking.")
{
   object->setJoinPassword(password);
}

ConsoleMethod(GameConnection, setConnectArgs, void, 3, 17,
   "(const char* args) @brief On the client, pass along a variable set of parameters to the server.\n\n"
   
   "Once the connection is established with the server, the server calls its onConnect() method "
   "with the client's passed in parameters as aruments.\n\n"
   
   "@see GameConnection::onConnect()\n\n")
{
   StringStackWrapper args(argc - 2, argv + 2);
   object->setConnectArgs(args.count(), args);
}

void GameConnection::onTimedOut()
{
   if(isConnectionToServer())
   {
      Con::printf("Connection to server timed out");
      onConnectionTimedOut_callback();
   }
   else
   {
      Con::printf("Client %d timed out.", getId());
      setDisconnectReason("TimedOut");
   }

}

void GameConnection::onConnectionEstablished(bool isInitiator)
{
   if(isInitiator)
   {
      setGhostFrom(false);
      setGhostTo(true);
      setSendingEvents(true);
      setTranslatesStrings(true);
      setIsConnectionToServer();
      mServerConnection = this;
      Con::printf("Connection established %d", getId());
      onConnectionAccepted_callback();
   }
   else
   {
      setGhostFrom(true);
      setGhostTo(false);
      setSendingEvents(true);
      setTranslatesStrings(true);
      Sim::getClientGroup()->addObject(this);
      mMoveList->init();

      const char *argv[MaxConnectArgs + 2];
      argv[0] = "onConnect";
      argv[1] = NULL; // Filled in later
      for(U32 i = 0; i < mConnectArgc; i++)
         argv[i + 2] = mConnectArgv[i];
      // NOTE: Need to fallback to Con::execute() as IMPLEMENT_CALLBACK does not 
      // support variadic functions.
      Con::execute(this, mConnectArgc + 2, argv);
   }
}

void GameConnection::onConnectTimedOut()
{
   onConnectRequestTimedOut_callback();
}

void GameConnection::onDisconnect(const char *reason)
{
   if(isConnectionToServer())
   {
      Con::printf("Connection with server lost.");
      onConnectionDropped_callback(reason);
      mMoveList->init();
   }
   else
   {
      Con::printf("Client %d disconnected.", getId());
      setDisconnectReason(reason);
   }
}

void GameConnection::onConnectionRejected(const char *reason)
{
   onConnectRequestRejected_callback(reason);
}

void GameConnection::handleStartupError(const char *errorString)
{
   onConnectRequestRejected_callback(errorString);
}

void GameConnection::writeConnectAccept(BitStream *stream)
{
   Parent::writeConnectAccept(stream);
   stream->write(getProtocolVersion());
}

bool GameConnection::readConnectAccept(BitStream *stream, const char **errorString)
{
   if(!Parent::readConnectAccept(stream, errorString))
      return false;

   U32 protocolVersion;
   stream->read(&protocolVersion);
   if(protocolVersion < MinRequiredProtocolVersion || protocolVersion > CurrentProtocolVersion)
   {
      *errorString = "CHR_PROTOCOL"; // this should never happen unless someone is faking us out.
      return false;
   }
   return true;
}

void GameConnection::writeConnectRequest(BitStream *stream)
{
   Parent::writeConnectRequest(stream);
   stream->writeString(TORQUE_APP_NAME);
   stream->write(CurrentProtocolVersion);
   stream->write(MinRequiredProtocolVersion);
   stream->writeString(mJoinPassword);

   stream->write(mConnectArgc);
   for(U32 i = 0; i < mConnectArgc; i++)
      stream->writeString(mConnectArgv[i]);
}

bool GameConnection::readConnectRequest(BitStream *stream, const char **errorString)
{
   if(!Parent::readConnectRequest(stream, errorString))
      return false;
   U32 currentProtocol, minProtocol;
   char gameString[256];
   stream->readString(gameString);
   if(dStrcmp(gameString, TORQUE_APP_NAME))
   {
      *errorString = "CHR_GAME";
      return false;
   }

   stream->read(&currentProtocol);
   stream->read(&minProtocol);

   char joinPassword[256];
   stream->readString(joinPassword);

   if(currentProtocol < MinRequiredProtocolVersion)
   {
      *errorString = "CHR_PROTOCOL_LESS";
      return false;
   }
   if(minProtocol > CurrentProtocolVersion)
   {
      *errorString = "CHR_PROTOCOL_GREATER";
      return false;
   }
   setProtocolVersion(currentProtocol < CurrentProtocolVersion ? currentProtocol : CurrentProtocolVersion);

   const char *serverPassword = Con::getVariable("pref::Server::Password");
   if(serverPassword[0])
   {
      if(dStrcmp(joinPassword, serverPassword))
      {
         *errorString = "CHR_PASSWORD";
         return false;
      }
   }

   stream->read(&mConnectArgc);
   if(mConnectArgc > MaxConnectArgs)
   {
      *errorString = "CR_INVALID_ARGS";
      return false;
   }
   ConsoleValueRef connectArgv[MaxConnectArgs + 3];
   ConsoleValue connectArgvValue[MaxConnectArgs + 3];

   for(U32 i = 0; i < mConnectArgc+3; i++)
   {
	   connectArgv[i].value = &connectArgvValue[i];
	   connectArgvValue[i].init();
   }

   for(U32 i = 0; i < mConnectArgc; i++)
   {
      char argString[256];
      stream->readString(argString);
      mConnectArgv[i] = dStrdup(argString);
      connectArgv[i + 3] = mConnectArgv[i];
   }
   connectArgvValue[0].setStackStringValue("onConnectRequest");
   connectArgvValue[1].setIntValue(0);
   char buffer[256];
   Net::addressToString(getNetAddress(), buffer);
   connectArgvValue[2].setStackStringValue(buffer);

   // NOTE: Cannot convert over to IMPLEMENT_CALLBACK as it has variable args.
   const char *ret = Con::execute(this, mConnectArgc + 3, connectArgv);
   if(ret[0])
   {
      *errorString = ret;
      return false;
   }
   return true;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void GameConnection::connectionError(const char *errorString)
{
   if(isConnectionToServer())
   {
      Con::printf("Connection error: %s.", errorString);
      onConnectionError_callback(errorString);
   }
   else
   {
      Con::printf("Client %d packet error: %s.", getId(), errorString);
      setDisconnectReason("Packet Error.");
   }
   deleteObject();
}


void GameConnection::setAuthInfo(const AuthInfo *info)
{
   mAuthInfo = new AuthInfo;
   *mAuthInfo = *info;
}

const AuthInfo *GameConnection::getAuthInfo()
{
   return mAuthInfo;
}


//----------------------------------------------------------------------------

void GameConnection::setControlObject(GameBase *obj)
{
   if(mControlObject == obj)
      return;

   if(mControlObject && mControlObject != mCameraObject)
      mControlObject->setControllingClient(0);

   if(obj)
   {
      // Nothing else is permitted to control this object.
      if (GameBase* coo = obj->getControllingObject())
         coo->setControlObject(0);
      if (GameConnection *con = obj->getControllingClient())
      {
         if(this != con)
         {
            // was it controlled via camera or control
            if(con->getControlObject() == obj)
               con->setControlObject(0);
            else
               con->setCameraObject(0);
         }
      }

      // We are now the controlling client of this object.
      obj->setControllingClient(this);

      // Update the camera's FOV to match the new control object
      //but only if we don't have a specific camera object
      if (!mCameraObject)
         setControlCameraFov(obj->getCameraFov());
   }

   // Okay, set our control object.
   mControlObject = obj;
   mControlForceMismatch = true;

   if(mCameraObject.isNull())
      setScopeObject(mControlObject);
}

void GameConnection::setCameraObject(GameBase *obj)
{
   if(mCameraObject == obj)
      return;

   if(mCameraObject && mCameraObject != mControlObject)
      mCameraObject->setControllingClient(0);

   if(obj)
   {
      // nothing else is permitted to control this object
      if(GameBase *coo = obj->getControllingObject())
         coo->setControlObject(0);

      if(GameConnection *con = obj->getControllingClient())
      {
         if(this != con)
         {
            // was it controlled via camera or control
            if(con->getControlObject() == obj)
               con->setControlObject(0);
            else
               con->setCameraObject(0);
         }
      }

      // we are now the controlling client of this object
      obj->setControllingClient(this);
   }

   // Okay, set our camera object.
   mCameraObject = obj;

   if(mCameraObject.isNull())
      setScopeObject(mControlObject);
   else
   {
      setScopeObject(mCameraObject);

      // if this is a client then set the fov and active image
      if(isConnectionToServer())
      {
         F32 fov = mCameraObject->getDefaultCameraFov();
         //GameSetCameraFov(fov);
         smFovUpdate.trigger(fov);
      }
   }
}

GameBase* GameConnection::getCameraObject()
{
   // If there is no camera object, or if we're first person, return
   // the control object.
   if( !mControlObject.isNull() && (mCameraObject.isNull() || mFirstPerson))
      return mControlObject;
   return mCameraObject;
}

static S32 sChaseQueueSize = 0;
static MatrixF* sChaseQueue = 0;
static S32 sChaseQueueHead = 0;
static S32 sChaseQueueTail = 0;

bool GameConnection::getControlCameraTransform(F32 dt, MatrixF* mat)
{
   GameBase* obj = getCameraObject();
   if(!obj)
      return false;

   GameBase* cObj = obj;
   while((cObj = cObj->getControlObject()) != 0)
   {
      if(cObj->useObjsEyePoint())
         obj = cObj;
   }

   if (dt) 
   {
      if (mFirstPerson || obj->onlyFirstPerson()) 
      {
         if (mCameraPos > 0)
            if ((mCameraPos -= mCameraSpeed * dt) <= 0)
               mCameraPos = 0;
      }
      else 
      {
         if (mCameraPos < 1)
            if ((mCameraPos += mCameraSpeed * dt) > 1)
               mCameraPos = 1;
      }
   }

   if (!sChaseQueueSize || mFirstPerson || obj->onlyFirstPerson())
      obj->getCameraTransform(&mCameraPos,mat);
   else 
   {
      MatrixF& hm = sChaseQueue[sChaseQueueHead];
      MatrixF& tm = sChaseQueue[sChaseQueueTail];
      obj->getCameraTransform(&mCameraPos,&hm);
      *mat = tm;
      if (dt) 
      {
         if ((sChaseQueueHead += 1) >= sChaseQueueSize)
            sChaseQueueHead = 0;
         if (sChaseQueueHead == sChaseQueueTail)
            if ((sChaseQueueTail += 1) >= sChaseQueueSize)
               sChaseQueueTail = 0;
      }
   }
   return true;
}

bool GameConnection::getControlCameraEyeTransforms(IDisplayDevice *display, MatrixF *transforms)
{
   GameBase* obj = getCameraObject();
   if(!obj)
      return false;

   GameBase* cObj = obj;
   while((cObj = cObj->getControlObject()) != 0)
   {
      if(cObj->useObjsEyePoint())
         obj = cObj;
   }

   // Perform operation on left & right eyes. For each we need to calculate the world space 
   // of the rotated eye offset and add that onto the camera world space.
   for (U32 i=0; i<2; i++)
   {
      obj->getEyeCameraTransform(display, i, &transforms[i]);
   }

   return true;
}


bool GameConnection::getControlCameraDefaultFov(F32 * fov)
{
   //find the last control object in the chain (client->player->turret->whatever...)
   GameBase *obj = getCameraObject();
   GameBase *cObj = NULL;
   while (obj)
   {
      cObj = obj;
      obj = obj->getControlObject();
   }
   if (cObj)
   {
      *fov = cObj->getDefaultCameraFov();
      return(true);
   }

   return(false);
}

bool GameConnection::getControlCameraFov(F32 * fov)
{
   //find the last control object in the chain (client->player->turret->whatever...)
   GameBase *obj = getCameraObject();
   GameBase *cObj = NULL;
   while (obj)
   {
      cObj = obj;
      obj = obj->getControlObject();
   }
   if (cObj)
   {
#ifdef TORQUE_EXPERIMENTAL_EC
      if (Entity* ent = dynamic_cast<Entity*>(cObj))
      {
         if (CameraInterface* camInterface = ent->getComponent<CameraInterface>())
         {
            *fov = camInterface->getCameraFov();
         }
      }
      else
      {
      *fov = cObj->getCameraFov();
      }
#else
      *fov = cObj->getCameraFov();
#endif
      return(true);
   }

   return(false);
}

bool GameConnection::isValidControlCameraFov(F32 fov)
{
   //find the last control object in the chain (client->player->turret->whatever...)
   GameBase *obj = getCameraObject();
   GameBase *cObj = NULL;
   while (obj)
   {
      cObj = obj;
      obj = obj->getControlObject();
   }

   if (cObj)
   {
#ifdef TORQUE_EXPERIMENTAL_EC
      if (Entity* ent = dynamic_cast<Entity*>(cObj))
      {
         if (CameraInterface* camInterface = ent->getComponent<CameraInterface>())
         {
            return camInterface->isValidCameraFov(fov);
         }
      }
      else
      {
         return cObj->isValidCameraFov(fov);
      }
#else
      return cObj->isValidCameraFov(fov);
#endif
   }

   return NULL;
}

bool GameConnection::setControlCameraFov(F32 fov)
{
   //find the last control object in the chain (client->player->turret->whatever...)
   GameBase *obj = getCameraObject();
   GameBase *cObj = NULL;
   while (obj)
   {
      cObj = obj;
      obj = obj->getControlObject();
   }
   if (cObj)
   {

#ifdef TORQUE_EXPERIMENTAL_EC
      F32 newFov = 90.f;
      if (Entity* ent = dynamic_cast<Entity*>(cObj))
      {
         if (CameraInterface* camInterface = ent->getComponent<CameraInterface>())
         {
            camInterface->setCameraFov(mClampF(fov, MinCameraFov, MaxCameraFov));
            newFov = camInterface->getCameraFov();
         }
         else
         {
            Con::errorf("Attempted to setControlCameraFov, but we don't have a camera!");
         }
      }
      else
      {
         // allow shapebase to clamp fov to its datablock values
         cObj->setCameraFov(mClampF(fov, MinCameraFov, MaxCameraFov));
         newFov = cObj->getCameraFov();
      }
#else
      // allow shapebase to clamp fov to its datablock values
      cObj->setCameraFov(mClampF(fov, MinCameraFov, MaxCameraFov));
      F32 newFov = cObj->getCameraFov();
#endif

      // server fov of client has 1degree resolution
      if( S32(newFov) != S32(mCameraFov) || newFov != fov )
         mUpdateCameraFov = true;

      mCameraFov = newFov;
      return(true);
   }
   return(false);
}

bool GameConnection::getControlCameraVelocity(Point3F *vel)
{
   if (GameBase* obj = getCameraObject()) {
      *vel = obj->getVelocity();
      return true;
   }
   return false;
}

bool GameConnection::isControlObjectRotDampedCamera()
{
   if (Camera* cam = dynamic_cast<Camera*>(getCameraObject())) {
      if(cam->isRotationDamped())
         return true;
   }
   return false;
}

void GameConnection::setFirstPerson(bool firstPerson)
{
   mFirstPerson = firstPerson;
   mUpdateFirstPerson = true;
}

//----------------------------------------------------------------------------

void GameConnection::setControlSchemeParameters(bool absoluteRotation, bool addYawToAbsRot, bool addPitchToAbsRot)
{
   mAbsoluteRotation = absoluteRotation;
   mAddYawToAbsRot = addYawToAbsRot;
   mAddPitchToAbsRot = addPitchToAbsRot;
   mUpdateControlScheme = true;
}

//----------------------------------------------------------------------------

bool GameConnection::onAdd()
{
   if (!Parent::onAdd())
      return false;

   return true;
}

void GameConnection::onRemove()
{
   if(isNetworkConnection())
   {
      sendDisconnectPacket(mDisconnectReason);
   }
   else if (isLocalConnection() && isConnectionToServer())
   {
      // We're a client-side but local connection
      // delete the server side of the connection on our local server so that it updates 
      // clientgroup and what not (this is so that we can disconnect from a local server
      // without needing to destroy and recreate the server before we can connect to it 
      // again).
	   // Safe-delete as we don't know whether the server connection is currently being
	   // worked on.
      getRemoteConnection()->safeDeleteObject();
      setRemoteConnectionObject(NULL);
   }
   if(!isConnectionToServer())
   {
      onDrop_callback(mDisconnectReason);
   }

   if (mControlObject)
      mControlObject->setControllingClient(0);
   Parent::onRemove();
}

void GameConnection::setDisconnectReason(const char *str)
{
   dStrncpy(mDisconnectReason, str, sizeof(mDisconnectReason) - 1);
   mDisconnectReason[sizeof(mDisconnectReason) - 1] = 0;
}

//----------------------------------------------------------------------------

void GameConnection::handleRecordedBlock(U32 type, U32 size, void *data)
{
   switch(type)
   {
      case BlockTypeMove:
         mMoveList->pushMove(*((Move *) data));
         if(isRecording()) // put it back into the stream
            recordBlock(type, size, data);
         break;
      default:
         Parent::handleRecordedBlock(type, size, data);
         break;
   }
}

void GameConnection::writeDemoStartBlock(ResizeBitStream *stream)
{
   // write all the data blocks to the stream:

   for(SimObjectId i = DataBlockObjectIdFirst; i <= DataBlockObjectIdLast; i++)
   {
      SimDataBlock *data;
      if(Sim::findObject(i, data))
      {
         stream->writeFlag(true);
         SimDataBlockEvent evt(data);
         evt.pack(this, stream);
         stream->validate();
      }
   }
   stream->writeFlag(false);
   stream->write(mFirstPerson);
   stream->write(mCameraPos);
   stream->write(mCameraSpeed);

   // Control scheme
   stream->write(mAbsoluteRotation);
   stream->write(mAddYawToAbsRot);
   stream->write(mAddPitchToAbsRot);

   stream->writeString(Con::getVariable("$Client::MissionFile"));

   mMoveList->writeDemoStartBlock(stream);

   // dump all the "demo" vars associated with this connection:
   SimFieldDictionaryIterator itr(getFieldDictionary());

   SimFieldDictionary::Entry *entry;
   while((entry = *itr) != NULL)
   {
      if(!dStrnicmp(entry->slotName, "demo", 4))
      {
         stream->writeFlag(true);
         stream->writeString(entry->slotName + 4);
         stream->writeString(entry->value);
         stream->validate();
      }
      ++itr;
   }
   stream->writeFlag(false);
   Parent::writeDemoStartBlock(stream);

   stream->validate();

   // dump out the control object ghost id
   S32 idx = mControlObject ? getGhostIndex(mControlObject) : -1;
   stream->write(idx);
   if(mControlObject)
   {
#ifdef TORQUE_NET_STATS
      U32 beginPos = stream->getBitPosition();
#endif
      mControlObject->writePacketData(this, stream);
#ifdef TORQUE_NET_STATS
//      TYPEOF( mControlObject )->getNetInfo().updateNetStatWriteData( stream->getBitPosition() - beginPos );
      
      mControlObject->getClassRep()->updateNetStatWriteData( stream->getBitPosition() - beginPos);
#endif
   }
   idx = mCameraObject ? getGhostIndex(mCameraObject) : -1;
   stream->write(idx);
   if(mCameraObject && mCameraObject != mControlObject)
   {
#ifdef TORQUE_NET_STATS
      U32 beginPos = stream->getBitPosition();
#endif

      mCameraObject->writePacketData(this, stream);

#ifdef TORQUE_NET_STATS
      mCameraObject->getClassRep()->updateNetStatWriteData( stream->getBitPosition() - beginPos);
#endif
   }
   mLastControlRequestTime = Platform::getVirtualMilliseconds();
}

bool GameConnection::readDemoStartBlock(BitStream *stream)
{
   while(stream->readFlag())
   {
      SimDataBlockEvent evt;
      evt.unpack(this, stream);
      evt.process(this);
   }

   while(mDataBlockLoadList.size())
   {
      preloadNextDataBlock(false);
      if(mErrorBuffer.isNotEmpty())
         return false;
   }

   stream->read(&mFirstPerson);
   stream->read(&mCameraPos);
   stream->read(&mCameraSpeed);

   // Control scheme
   stream->read(&mAbsoluteRotation);
   stream->read(&mAddYawToAbsRot);
   stream->read(&mAddPitchToAbsRot);

   char buf[256];
   stream->readString(buf);
   Con::setVariable("$Client::MissionFile",buf);

   mMoveList->readDemoStartBlock(stream);

   // read in all the demo vars associated with this recording
   // they are all tagged on to the object and start with the
   // string "demo"

   while(stream->readFlag())
   {
      StringTableEntry slotName = StringTable->insert("demo");
      char array[256];
      char value[256];
      stream->readString(array);
      stream->readString(value);
      setDataField(slotName, array, value);
   }
   bool ret = Parent::readDemoStartBlock(stream);
   // grab the control object
   S32 idx;
   stream->read(&idx);

   GameBase * obj = 0;
   if(idx != -1)
   {
      obj = dynamic_cast<GameBase*>(resolveGhost(idx));
      setControlObject(obj);
      obj->readPacketData(this, stream);
   }

   // Get the camera object, and read it in if it's different
   S32 idx2;
   stream->read(&idx2);
   obj = 0;
   if(idx2 != -1 && idx2 != idx)
   {
      obj = dynamic_cast<GameBase*>(resolveGhost(idx2));
      setCameraObject(obj);
      obj->readPacketData(this, stream);
   }
   return ret;
}

void GameConnection::demoPlaybackComplete()
{
   static const char* demoPlaybackArgv[1] = { "demoPlaybackComplete" };
   static StringStackConsoleWrapper demoPlaybackCmd(1, demoPlaybackArgv);

   Sim::postCurrentEvent(Sim::getRootGroup(), new SimConsoleEvent(demoPlaybackCmd.argc, demoPlaybackCmd.argv, false));
   Parent::demoPlaybackComplete();
}

void GameConnection::ghostPreRead(NetObject * nobj, bool newGhost)
{
   Parent::ghostPreRead( nobj, newGhost );

   mMoveList->ghostPreRead(nobj,newGhost);
}

void GameConnection::ghostReadExtra(NetObject * nobj, BitStream * bstream, bool newGhost)
{
   Parent::ghostReadExtra( nobj, bstream, newGhost );

   mMoveList->ghostReadExtra(nobj, bstream, newGhost);
   }

void GameConnection::ghostWriteExtra(NetObject * nobj, BitStream * bstream)
{
   Parent::ghostWriteExtra( nobj, bstream);

   mMoveList->ghostWriteExtra(nobj, bstream);
}

//----------------------------------------------------------------------------

void GameConnection::readPacket(BitStream *bstream)
{
   bstream->clearStringBuffer();
   bstream->clearCompressionPoint();

   if (isConnectionToServer())
   {
      mMoveList->clientReadMovePacket(bstream);

      bool hadFlash = mDamageFlash > 0 || mWhiteOut > 0;
      mDamageFlash = 0;
      mWhiteOut = 0;
      if(bstream->readFlag())
      {
         if(bstream->readFlag())
            mDamageFlash = bstream->readFloat(7);
         if(bstream->readFlag())
            mWhiteOut = bstream->readFloat(7) * 1.5;

         if(!hadFlash)
         {
            // Started a damage flash or white out
            onFlash_callback(true);
         }
         else
         {
            if(!(mDamageFlash > 0 || mWhiteOut > 0))
            {
               // Received a zero damage flash and white out.
               onFlash_callback(false);
            }
         }
      }
      else if(hadFlash)
      {
         // Catch those cases where both the damage flash and white out are at zero
         // on the server but we did not receive an exact zero packet due to
         // precision over the network issues.  No problem as the flag we just
         // read (which is false if we're here) has also told us about the change.
         onFlash_callback(false);
      }

      if ( bstream->readFlag() ) // gIndex != -1
      {         
         if ( bstream->readFlag() ) // mMoveList->isMismatch() || mControlForceMismatch
         {
            // the control object is dirty...so we get an update:
            bool callScript = false;
            bool controlObjChange = false;
            if(mControlObject.isNull())
               callScript = true;

            S32 gIndex = bstream->readInt(NetConnection::GhostIdBitSize);
            GameBase* obj = dynamic_cast<GameBase*>(resolveGhost(gIndex));
            if (mControlObject != obj)
            {
               setControlObject(obj);
               controlObjChange = true;
            }
#ifdef TORQUE_NET_STATS
            U32 beginSize = bstream->getBitPosition();
#endif
            obj->readPacketData(this, bstream);
#ifdef TORQUE_NET_STATS
            obj->getClassRep()->updateNetStatReadData(bstream->getBitPosition() - beginSize);
#endif

            // let move list know that control object is dirty
            mMoveList->markControlDirty();

            if(callScript)
            {
               initialControlSet_callback();
            }

            if(controlObjChange)
            {
               onControlObjectChange_callback();
            }
         }
         else
         {
            // read out the compression point
            Point3F pos;
            bstream->read(&pos.x);
            bstream->read(&pos.y);
            bstream->read(&pos.z);
            bstream->setCompressionPoint(pos);
         }
      }

      if (bstream->readFlag())
      {
         bool callScript = false;
         if (mCameraObject.isNull())
            callScript = true;

         S32 gIndex = bstream->readInt(NetConnection::GhostIdBitSize);
         GameBase* obj = dynamic_cast<GameBase*>(resolveGhost(gIndex));
         setCameraObject(obj);
         obj->readPacketData(this, bstream);

         if (callScript)
            initialControlSet_callback();
      }
      else
         setCameraObject(0);

      // server changed control scheme
      if(bstream->readFlag())
      {
         bool absoluteRotation = bstream->readFlag();
         bool addYawToAbsRot = bstream->readFlag();
         bool addPitchToAbsRot = bstream->readFlag();
         setControlSchemeParameters(absoluteRotation, addYawToAbsRot, addPitchToAbsRot);
         mUpdateControlScheme = false;
      }

      // server changed first person
      if(bstream->readFlag())
      {
         setFirstPerson(bstream->readFlag());
         mUpdateFirstPerson = false;
      }

      // server forcing a fov change?
      if(bstream->readFlag())
      {
         S32 fov = bstream->readInt(8);
         setControlCameraFov((F32)fov);

         // don't bother telling the server if we were able to set the fov
         F32 setFov;
         if(getControlCameraFov(&setFov) && (S32(setFov) == fov))
            mUpdateCameraFov = false;

         // update the games fov info
         smFovUpdate.trigger((F32)fov);
      }
   }
   else
   {
      mMoveList->serverReadMovePacket(bstream);

      mCameraPos = bstream->readFlag() ? 1.0f : 0.0f;
      if (bstream->readFlag())
         mControlForceMismatch = true;

      // client changed control scheme
      if(bstream->readFlag())
      {
         bool absoluteRotation = bstream->readFlag();
         bool addYawToAbsRot = bstream->readFlag();
         bool addPitchToAbsRot = bstream->readFlag();
         setControlSchemeParameters(absoluteRotation, addYawToAbsRot, addPitchToAbsRot);
         mUpdateControlScheme = false;
      }

      // client changed first person
      if(bstream->readFlag())
      {
         setFirstPerson(bstream->readFlag());
         mUpdateFirstPerson = false;
      }

      // check fov change.. 1degree granularity on server
      if(bstream->readFlag())
      {
         S32 fov = mClamp(bstream->readInt(8), S32(MinCameraFov), S32(MaxCameraFov));
         setControlCameraFov((F32)fov);

         // may need to force client back to a valid fov
         F32 setFov;
         if(getControlCameraFov(&setFov) && (S32(setFov) == fov))
            mUpdateCameraFov = false;
      }
   }

   Parent::readPacket(bstream);
   bstream->clearCompressionPoint();
   bstream->clearStringBuffer();

   if (isConnectionToServer())
   {
      PROFILE_START(ClientCatchup);
      ClientProcessList::get()->clientCatchup(this);
      PROFILE_END();
   }
}

void GameConnection::writePacket(BitStream *bstream, PacketNotify *note)
{
   bstream->clearCompressionPoint();
   bstream->clearStringBuffer();

   GamePacketNotify *gnote = (GamePacketNotify *) note;

   U32 startPos = bstream->getBitPosition();
   if (isConnectionToServer())
   {
      mMoveList->clientWriteMovePacket(bstream);

      bstream->writeFlag(mCameraPos == 1);

      // if we're recording, we want to make sure that we get periodic updates of the
      // control object "just in case" - ie if the math copro is different between the
      // recording machine (SIMD vs FPU), we get periodic corrections

      bool forceUpdate = false;
      if(isRecording())
      {
         U32 currentTime = Platform::getVirtualMilliseconds();
         if(currentTime - mLastControlRequestTime > ControlRequestTime)
         {
            mLastControlRequestTime = currentTime;
            forceUpdate=true;;
         }
      }
      bstream->writeFlag(forceUpdate);

      // Control scheme changed?
      if(bstream->writeFlag(mUpdateControlScheme))
      {
         bstream->writeFlag(mAbsoluteRotation);
         bstream->writeFlag(mAddYawToAbsRot);
         bstream->writeFlag(mAddPitchToAbsRot);
         mUpdateControlScheme = false;
      }

      // first person changed?
      if(bstream->writeFlag(mUpdateFirstPerson)) 
      {
         bstream->writeFlag(mFirstPerson);
         mUpdateFirstPerson = false;
      }

      // camera fov changed? (server fov resolution is 1 degree)
      if(bstream->writeFlag(mUpdateCameraFov))
      {
         bstream->writeInt(mClamp(S32(mCameraFov), S32(MinCameraFov), S32(MaxCameraFov)), 8);
         mUpdateCameraFov = false;
      }
      DEBUG_LOG(("PKLOG %d CLIENTMOVES: %d", getId(), bstream->getCurPos() - startPos));
   }
   else
   {
      mMoveList->serverWriteMovePacket(bstream);

      // get the ghost index of the control object, and write out
      // all the damage flash & white out

      S32 gIndex = -1;
      if (!mControlObject.isNull())
      {
         gIndex = getGhostIndex(mControlObject);

         F32 flash = mControlObject->getDamageFlash();
         F32 whiteOut = mControlObject->getWhiteOut();
         if(bstream->writeFlag(flash != 0 || whiteOut != 0))
         {
            if(bstream->writeFlag(flash != 0))
               bstream->writeFloat(flash, 7);
            if(bstream->writeFlag(whiteOut != 0))
               bstream->writeFloat(whiteOut/1.5, 7);
         }
      }
      else
         bstream->writeFlag(false);

      if (bstream->writeFlag(gIndex != -1))
      {
         // assume that the control object will write in a compression point
         if(bstream->writeFlag(mMoveList->isMismatch() || mControlForceMismatch))
         {
#ifdef TORQUE_DEBUG_NET
            if (mMoveList->isMismatch())
               Con::printf("packetDataChecksum disagree!");
            else
               Con::printf("packetDataChecksum disagree! (force)");
#endif

            bstream->writeInt(gIndex, NetConnection::GhostIdBitSize);
#ifdef TORQUE_NET_STATS
            U32 beginSize = bstream->getBitPosition();
#endif
            mControlObject->writePacketData(this, bstream);
#ifdef TORQUE_NET_STATS
            mControlObject->getClassRep()->updateNetStatWriteData(bstream->getBitPosition() - beginSize);
#endif
            mControlForceMismatch = false;
         }
         else
         {
            // we'll have to use the control object's position as the compression point
            // should make this lower res for better space usage:
            Point3F coPos = mControlObject->getPosition();
            bstream->write(coPos.x);
            bstream->write(coPos.y);
            bstream->write(coPos.z);
            bstream->setCompressionPoint(coPos);
         }
      }
      DEBUG_LOG(("PKLOG %d CONTROLOBJECTSTATE: %d", getId(), bstream->getCurPos() - startPos));
      startPos = bstream->getBitPosition();

      if (!mCameraObject.isNull() && mCameraObject != mControlObject)
      {
         gIndex = getGhostIndex(mCameraObject);
         if (bstream->writeFlag(gIndex != -1))
         {
            bstream->writeInt(gIndex, NetConnection::GhostIdBitSize);
            mCameraObject->writePacketData(this, bstream);
         }
      }
      else
         bstream->writeFlag( false );

      // Control scheme changed?
      if(bstream->writeFlag(mUpdateControlScheme))
      {
         bstream->writeFlag(mAbsoluteRotation);
         bstream->writeFlag(mAddYawToAbsRot);
         bstream->writeFlag(mAddPitchToAbsRot);
         mUpdateControlScheme = false;
      }

      // first person changed?
      if(bstream->writeFlag(mUpdateFirstPerson)) 
      {
         bstream->writeFlag(mFirstPerson);
         mUpdateFirstPerson = false;
      }

      // server forcing client fov?
      gnote->cameraFov = -1;
      if(bstream->writeFlag(mUpdateCameraFov))
      {
         gnote->cameraFov = mClamp(S32(mCameraFov), S32(MinCameraFov), S32(MaxCameraFov));
         bstream->writeInt(gnote->cameraFov, 8);
         mUpdateCameraFov = false;
      }
      DEBUG_LOG(("PKLOG %d PINGCAMSTATE: %d", getId(), bstream->getCurPos() - startPos));
   }

   Parent::writePacket(bstream, note);
   bstream->clearCompressionPoint();
   bstream->clearStringBuffer();
}


void GameConnection::detectLag()
{
   //see if we're lagging...
   S32 curTime = Sim::getCurrentTime();
   if (curTime - mLastPacketTime > mLagThresholdMS)
   {
      if (!mLagging)
      {
         mLagging = true;
         setLagIcon_callback(true);
      }
   }
   else if (mLagging)
   {
      mLagging = false;
      setLagIcon_callback(false);
   }
}

GameConnection::GamePacketNotify::GamePacketNotify()
{
   // need to fill in empty notifes for demo start block
   cameraFov = 0;
}

NetConnection::PacketNotify *GameConnection::allocNotify()
{
   return new GamePacketNotify;
}

void GameConnection::packetReceived(PacketNotify *note)
{
   //record the time so we can tell if we're lagging...
   mLastPacketTime = Sim::getCurrentTime();

   // If we wanted to do something special, we grab our note like this:
   //GamePacketNotify *gnote = (GamePacketNotify *) note;

   Parent::packetReceived(note);
}

void GameConnection::packetDropped(PacketNotify *note)
{
   Parent::packetDropped(note);
   GamePacketNotify *gnote = (GamePacketNotify *) note;
   if(gnote->cameraFov != -1)
      mUpdateCameraFov = true;
}

//----------------------------------------------------------------------------

void GameConnection::play2D(SFXProfile* profile)
{
   postNetEvent(new Sim2DAudioEvent(profile));
}

void GameConnection::play3D(SFXProfile* profile, const MatrixF *transform)
{
   if ( !transform )
      play2D(profile);

   else if ( !mControlObject )
      postNetEvent(new Sim3DAudioEvent(profile,transform));

   else
   {
      // TODO: Maybe improve this to account for the duration
      // of the sound effect and if the control object can get
      // into hearing range within time?

      // Only post the event if it's within audible range
      // of the control object.
      Point3F ear,pos;
      transform->getColumn(3,&pos);
      mControlObject->getTransform().getColumn(3,&ear);
      if ((ear - pos).len() < profile->getDescription()->mMaxDistance)
         postNetEvent(new Sim3DAudioEvent(profile,transform));
   } 
}

void GameConnection::doneScopingScene()
{
   // Could add special post-scene scoping here, such as scoping
   // objects not visible to the camera, but visible to sensors.
}

void GameConnection::preloadDataBlock(SimDataBlock *db)
{
   mDataBlockLoadList.push_back(db);
   if(mDataBlockLoadList.size() == 1)
      preloadNextDataBlock(false);
}

void GameConnection::fileDownloadSegmentComplete()
{
   // this is called when a the file list has finished processing...
   // at this point we can try again to add the object
   // subclasses can override this to do, for example, datablock redos.
   if(mDataBlockLoadList.size())
      preloadNextDataBlock(mNumDownloadedFiles != 0);
   Parent::fileDownloadSegmentComplete();
}

void GameConnection::preloadNextDataBlock(bool hadNewFiles)
{
   if(!mDataBlockLoadList.size())
      return;
   while(mDataBlockLoadList.size())
   {
      // only check for new files if this is the first load, or if new
      // files were downloaded from the server.
//       if(hadNewFiles)
//          gResourceManager->setMissingFileLogging(true);
//       gResourceManager->clearMissingFileList();
      SimDataBlock *object = mDataBlockLoadList[0];
      if(!object)
      {
         // a null object is used to signify that the last ghost in the list is down
         mDataBlockLoadList.pop_front();
         AssertFatal(mDataBlockLoadList.size() == 0, "Error! Datablock save list should be empty!");
         sendConnectionMessage(DataBlocksDownloadDone, mDataBlockSequence);

//          gResourceManager->setMissingFileLogging(false);
         return;
      }
      mFilesWereDownloaded = hadNewFiles;
      if(!object->preload(false, mErrorBuffer))
      {
         mFilesWereDownloaded = false;
         // make sure there's an error message if necessary
         if(mErrorBuffer.isEmpty())
            setLastError("Invalid packet. (object preload)");

         // if there were new files, make sure the error message
         // is the one from the last time we tried to add this object
         if(hadNewFiles)
         {
            mErrorBuffer = mLastFileErrorBuffer;
//             gResourceManager->setMissingFileLogging(false);
            return;
         }

         // object failed to load, let's see if it had any missing files
         if(isLocalConnection() /*|| !gResourceManager->getMissingFileList(mMissingFileList)*/)
         {
            // no missing files, must be an error
            // connection will automagically delete the ghost always list
            // when this error is reported.
//             gResourceManager->setMissingFileLogging(false);
            return;
         }

         // ok, copy the error buffer out to a scratch pad for now
         mLastFileErrorBuffer = mErrorBuffer;
         //mErrorBuffer = String();

         // request the missing files...
         mNumDownloadedFiles = 0;
         sendNextFileDownloadRequest();
         break;
      }
      mFilesWereDownloaded = false;
//       gResourceManager->setMissingFileLogging(false);
      mDataBlockLoadList.pop_front();
      hadNewFiles = true;
   }
}

void GameConnection::onEndGhosting()
{
   Parent::onEndGhosting();
   
   // Reset move list.  All the moves are obsolete now and furthermore,
   // if we don't clear out the list now we might run in danger of
   // getting backlogged later on what is a list full of obsolete moves.
   
   mMoveList->reset();
}

//----------------------------------------------------------------------------
//localconnection only blackout functions
void GameConnection::setBlackOut(bool fadeToBlack, S32 timeMS)
{
   mFadeToBlack = fadeToBlack;
   mBlackOutStartTimeMS = Sim::getCurrentTime();
   mBlackOutTimeMS = timeMS;

   //if timeMS <= 0 set the value instantly
   if (mBlackOutTimeMS <= 0)
      mBlackOut = (mFadeToBlack ? 1.0f : 0.0f);
}

F32 GameConnection::getBlackOut()
{
   S32 curTime = Sim::getCurrentTime();

   //see if we're in the middle of a black out
   if (curTime < mBlackOutStartTimeMS + mBlackOutTimeMS)
   {
      S32 elapsedTime = curTime - mBlackOutStartTimeMS;
      F32 timePercent = F32(elapsedTime) / F32(mBlackOutTimeMS);
      mBlackOut = (mFadeToBlack ? timePercent : 1.0f - timePercent);
   }
   else
      mBlackOut = (mFadeToBlack ? 1.0f : 0.0f);

   //return the blackout time
   return mBlackOut;
}

void GameConnection::handleConnectionMessage(U32 message, U32 sequence, U32 ghostCount)
{
   if(isConnectionToServer())
   {
      if(message == DataBlocksDone)
      {
         mDataBlockLoadList.push_back(NULL);
         mDataBlockSequence = sequence;
         if(mDataBlockLoadList.size() == 1)
            preloadNextDataBlock(true);
      }
   }
   else
   {
      if(message == DataBlocksDownloadDone)
      {
         if(getDataBlockSequence() == sequence)
         {
            onDataBlocksDone_callback( getDataBlockSequence() );
         }
      }
   }
   Parent::handleConnectionMessage(message, sequence, ghostCount);
}

//----------------------------------------------------------------------------

DefineEngineMethod( GameConnection, transmitDataBlocks, void, (S32 sequence),,
   "@brief Sent by the server during phase 1 of the mission download to send the datablocks to the client.\n\n"
   
   "SimDataBlocks, also known as just datablocks, need to be transmitted to the client "
   "prior to the client entering the game world.  These represent the static data that "
   "most objects in the world reference.  This is typically done during the standard "
   "mission start phase 1 when following Torque's example mission startup sequence.\n\n"

   "When the datablocks have all been transmitted, onDataBlocksDone() is called to move "
   "the mission start process to the next phase."

   "@param sequence The sequence is common between the server and client and ensures "
   "that the client is acting on the most recent mission start process.  If an errant "
   "network packet (one that was lost but has now been found) is received by the client "
   "with an incorrect sequence, it is just ignored.  This sequence number is updated on "
   "the server every time a mission is loaded.\n\n"

   "@tsexample\n"
   "function serverCmdMissionStartPhase1Ack(%client, %seq)\n"
   "{\n"
   "   // Make sure to ignore calls from a previous mission load\n"
   "   if (%seq != $missionSequence || !$MissionRunning)\n"
   "      return;\n"
   "   if (%client.currentPhase != 0)\n"
   "      return;\n"
   "   %client.currentPhase = 1;\n"
   "\n"
   "   // Start with the CRC\n"
   "   %client.setMissionCRC( $missionCRC );\n"
   "\n"
   "   // Send over the datablocks...\n"
   "   // OnDataBlocksDone will get called when have confirmation\n"
   "   // that they've all been received.\n"
   "   %client.transmitDataBlocks($missionSequence);\n"
   "}\n"
   "@endtsexample\n\n"
   
   "@see GameConnection::onDataBlocksDone()\n\n")
{
    // Set the datablock sequence.
    object->setDataBlockSequence(sequence);

    // Store a pointer to the datablock group.
    SimDataBlockGroup* pGroup = Sim::getDataBlockGroup();

    // Determine the size of the datablock group.
    const U32 iCount = pGroup->size();

    // If this is the local client...
    if (GameConnection::getLocalClientConnection() == object)
    {
        // Set up a pointer to the datablock.
        SimDataBlock* pDataBlock = 0;

        // Set up a buffer for the datablock send.
        U8 iBuffer[16384];
        BitStream mStream(iBuffer, 16384);

        // Iterate through all the datablocks...
        for (U32 i = 0; i < iCount; i++)
        {
            // Get a pointer to the datablock in question...
            pDataBlock = (SimDataBlock*)(*pGroup)[i];

            // Set the client's new modified key.
            object->setMaxDataBlockModifiedKey(pDataBlock->getModifiedKey());

            // Pack the datablock stream.
            mStream.setPosition(0);
            mStream.clearCompressionPoint();
            pDataBlock->packData(&mStream);

            // Unpack the datablock stream.
            mStream.setPosition(0);
            mStream.clearCompressionPoint();
            pDataBlock->unpackData(&mStream);

            // Call the console function to set the number of blocks to be sent.
            onDataBlockObjectReceived_callback(i, iCount);

            // Preload the datablock on the dummy client.
            pDataBlock->preload(false, NetConnection::getErrorBuffer());
        }

        // Get the last datablock (if any)...
        if (pDataBlock)
        {
            // Ensure the datablock modified key is set.
            object->setDataBlockModifiedKey(object->getMaxDataBlockModifiedKey());

            // Ensure that the client knows that the datablock send is done...
            object->sendConnectionMessage(GameConnection::DataBlocksDone, object->getDataBlockSequence());
        }

        if (iCount == 0)
        {
           //if we have no datablocks to send, we still need to be able to complete the level load process
           //so fire off our callback anyways
           object->sendConnectionMessage(GameConnection::DataBlocksDone, object->getDataBlockSequence());
        }
    } 
    else
    {
        // Otherwise, store the current datablock modified key.
        const S32 iKey = object->getDataBlockModifiedKey();

        // Iterate through the datablock group...
        U32 i = 0;
        for (; i < iCount; i++)
        {
            // If the datablock's modified key has already been set, break out of the loop...
            if (((SimDataBlock*)(*pGroup)[i])->getModifiedKey() > iKey)
            {
                break;
            }
        }

        // If this is the last datablock in the group...
        if (i == iCount)
        {
            // Ensure that the client knows that the datablock send is done...
            object->sendConnectionMessage(GameConnection::DataBlocksDone, object->getDataBlockSequence());

            // Then exit out since nothing else needs to be done.
            return;
        }

        // Set the maximum datablock modified key value.
        object->setMaxDataBlockModifiedKey(iKey);

        // Get the minimum number of datablocks...
        const U32 iMax = getMin(i + DataBlockQueueCount, iCount);

        // Iterate through the remaining datablocks...
        for (;i < iMax; i++)
        {
            // Get a pointer to the datablock in question...
            SimDataBlock* pDataBlock = (SimDataBlock*)(*pGroup)[i];

            // Post the datablock event to the client.
            object->postNetEvent(new SimDataBlockEvent(pDataBlock, i, iCount, object->getDataBlockSequence()));
        }
    }
}

DefineEngineMethod( GameConnection, activateGhosting, void, (),,
   "@brief Called by the server during phase 2 of the mission download to start sending ghosts to the client.\n\n"
   
   "Ghosts represent objects on the server that are in scope for the client.  These need "
   "to be synchronized with the client in order for the client to see and interact with them.  "
   "This is typically done during the standard mission start phase 2 when following Torque's "
   "example mission startup sequence.\n\n"

   "@tsexample\n"
   "function serverCmdMissionStartPhase2Ack(%client, %seq, %playerDB)\n"
   "{\n"
   "   // Make sure to ignore calls from a previous mission load\n"
   "   if (%seq != $missionSequence || !$MissionRunning)\n"
   "      return;\n"
   "   if (%client.currentPhase != 1.5)\n"
   "      return;\n"
   "   %client.currentPhase = 2;\n"
   "\n"
   "   // Set the player datablock choice\n"
   "   %client.playerDB = %playerDB;\n"
   "\n"
   "   // Update mod paths, this needs to get there before the objects.\n"
   "   %client.transmitPaths();\n"
   "\n"
   "   // Start ghosting objects to the client\n"
   "   %client.activateGhosting();\n"
   "}\n"
   "@endtsexample\n\n"

   "@see @ref ghosting_scoping for a description of the ghosting system.\n\n")
{
   object->activateGhosting();
}

DefineEngineMethod( GameConnection, resetGhosting, void, (),,
   "@brief On the server, resets the connection to indicate that ghosting has been disabled.\n\n"

   "Typically when a mission has ended on the server, all connected clients are informed of this change "
   "and their connections are reset back to a starting state.  This method resets a connection on the "
   "server to indicate that ghosts are no longer being transmitted.  On the client end, all ghost "
   "information will be deleted.\n\n"

   "@tsexample\n"
   "   // Inform the clients\n"
   "   for (%clientIndex = 0; %clientIndex < ClientGroup.getCount(); %clientIndex++)\n"
   "   {\n"
   "      // clear ghosts and paths from all clients\n"
   "      %cl = ClientGroup.getObject(%clientIndex);\n"
   "      %cl.endMission();\n"
   "      %cl.resetGhosting();\n"
   "      %cl.clearPaths();\n"
   "   }\n"
   "@endtsexample\n\n"

   "@see @ref ghosting_scoping for a description of the ghosting system.\n\n")
{
   object->resetGhosting();
}

DefineEngineMethod( GameConnection, setControlObject, bool, (GameBase* ctrlObj),,
   "@brief On the server, sets the object that the client will control.\n\n"
   "By default the control object is an instance of the Player class, but can also be an instance "
   "of Camera (when editing the mission, for example), or any other ShapeBase derived class as "
   "appropriate for the game.\n\n"
   "@param ctrlObj The GameBase object on the server to control.")
{
   if(!ctrlObj)
      return false;

   object->setControlObject(ctrlObj);
   return true;
}

DefineEngineMethod( GameConnection, clearDisplayDevice, void, (),,
   "@brief Clear any display device.\n\n"
   "A display device may define a number of properties that are used during rendering.\n\n")
{
   object->clearDisplayDevice();
}

DefineEngineMethod( GameConnection, getControlObject, GameBase*, (),,
   "@brief On the server, returns the object that the client is controlling."
   "By default the control object is an instance of the Player class, but can also be an instance "
   "of Camera (when editing the mission, for example), or any other ShapeBase derived class as "
   "appropriate for the game.\n\n"
   "@see GameConnection::setControlObject()\n\n")
{
   return object->getControlObject();
}

DefineEngineMethod( GameConnection, isAIControlled, bool, (),,
   "@brief Returns true if this connection is AI controlled.\n\n"
   "@see AIConnection")
{
   return object->isAIControlled();
}

DefineEngineMethod( GameConnection, isControlObjectRotDampedCamera, bool, (),,
   "@brief Returns true if the object being controlled by the client is making use "
   "of a rotation damped camera.\n\n"
   "@see Camera")
{
   return object->isControlObjectRotDampedCamera();
}

DefineEngineMethod( GameConnection, play2D, bool, (SFXProfile* profile),,
   "@brief Used on the server to play a 2D sound that is not attached to any object.\n\n"

   "@param profile The SFXProfile that defines the sound to play.\n\n"

   "@tsexample\n"
   "function ServerPlay2D(%profile)\n"
   "{\n"
   "   // Play the given sound profile on every client.\n"
   "   // The sounds will be transmitted as an event, not attached to any object.\n"
   "   for(%idx = 0; %idx < ClientGroup.getCount(); %idx++)\n"
   "      ClientGroup.getObject(%idx).play2D(%profile);\n"
   "}\n"
   "@endtsexample\n\n")
{
   if(!profile)
      return false;

   object->play2D(profile);
   return true;
}

DefineEngineMethod( GameConnection, play3D, bool, (SFXProfile* profile, TransformF location),,
   "@brief Used on the server to play a 3D sound that is not attached to any object.\n\n"
   
   "@param profile The SFXProfile that defines the sound to play.\n"
   "@param location The position and orientation of the 3D sound given in the form of \"x y z ax ay az aa\".\n\n"

   "@tsexample\n"
   "function ServerPlay3D(%profile,%transform)\n"
   "{\n"
   "   // Play the given sound profile at the given position on every client\n"
   "   // The sound will be transmitted as an event, not attached to any object.\n"
   "   for(%idx = 0; %idx < ClientGroup.getCount(); %idx++)\n"
   "      ClientGroup.getObject(%idx).play3D(%profile,%transform);\n"
   "}\n"
   "@endtsexample\n\n")
{
   if(!profile)
      return false;

   MatrixF mat = location.getMatrix();
   object->play3D(profile,&mat);
   return true;
}

DefineEngineMethod( GameConnection, chaseCam, bool, (S32 size),,
   "@brief Sets the size of the chase camera's matrix queue.\n\n"
   "@note This sets the queue size across all GameConnections.\n\n"
   "@note This is not currently hooked up.\n\n")
{
   if (size != sChaseQueueSize) 
   {
      SAFE_DELETE_ARRAY(sChaseQueue);

      sChaseQueueSize = size;
      sChaseQueueHead = sChaseQueueTail = 0;

      if (size) 
      {
         sChaseQueue = new MatrixF[size];
         return true;
      }
   }
   return false;
}

DefineEngineMethod( GameConnection, getControlCameraDefaultFov, F32, (),,
   "@brief Returns the default field of view as used by the control object's camera.\n\n")
{
   F32 fov = 0.0f;
   if(!object->getControlCameraDefaultFov(&fov))
      return(0.0f);
   return(fov);
}

DefineEngineMethod( GameConnection, setControlCameraFov, void, (F32 newFOV),,
   "@brief On the server, sets the control object's camera's field of view.\n\n"
   "@param newFOV New field of view (in degrees) to force the control object's camera to use.  This value "
   "is clamped to be within the range of 1 to 179 degrees.\n\n"
   "@note When transmitted over the network to the client, the resolution is limited to "
   "one degree.  Any fraction is dropped.")
{
   object->setControlCameraFov(newFOV);
}

DefineEngineMethod( GameConnection, getControlCameraFov, F32, (),,
   "@brief Returns the field of view as used by the control object's camera.\n\n")
{
   F32 fov = 0.0f;
   if(!object->getControlCameraFov(&fov))
      return(0.0f);
   return(fov);
}

DefineEngineMethod( GameConnection, getDamageFlash, F32, (),,
   "@brief On the client, get the control object's damage flash level.\n\n"
   "@return flash level\n")
{
   return object->getDamageFlash();
}

DefineEngineMethod( GameConnection, getWhiteOut, F32, (),,
   "@brief On the client, get the control object's white-out level.\n\n"
   "@return white-out level\n")
{
   return object->getWhiteOut();
}

DefineEngineMethod( GameConnection, setBlackOut, void, (bool doFade, S32 timeMS),,
   "@brief On the server, sets the client's 3D display to fade to black.\n\n"
   "@param doFade Set to true to fade to black, and false to fade from black.\n"
   "@param timeMS Time it takes to perform the fade as measured in ms.\n\n"
   "@note Not currently hooked up, and is not synchronized over the network.")
{
   object->setBlackOut(doFade, timeMS);
}

DefineEngineMethod( GameConnection, setMissionCRC, void, (S32 CRC),,
   "@brief On the server, transmits the mission file's CRC value to the client.\n\n"

   "Typically, during the standard mission start phase 1, the mission file's CRC value "
   "on the server is send to the client.  This allows the client to determine if the mission "
   "has changed since the last time it downloaded this mission and act appropriately, such as "
   "rebuilt cached lightmaps.\n\n"

   "@param CRC The mission file's CRC value on the server.\n\n"

   "@tsexample\n"
   "function serverCmdMissionStartPhase1Ack(%client, %seq)\n"
   "{\n"
   "   // Make sure to ignore calls from a previous mission load\n"
   "   if (%seq != $missionSequence || !$MissionRunning)\n"
   "      return;\n"
   "   if (%client.currentPhase != 0)\n"
   "      return;\n"
   "   %client.currentPhase = 1;\n"
   "\n"
   "   // Start with the CRC\n"
   "   %client.setMissionCRC( $missionCRC );\n"
   "\n"
   "   // Send over the datablocks...\n"
   "   // OnDataBlocksDone will get called when have confirmation\n"
   "   // that they've all been received.\n"
   "   %client.transmitDataBlocks($missionSequence);\n"
   "}\n"
   "@endtsexample\n\n")
{
   if(object->isConnectionToServer())
      return;

   object->postNetEvent(new SetMissionCRCEvent(CRC));
}

DefineEngineMethod( GameConnection, delete, void, (const char* reason), (""),
   "@brief On the server, disconnect a client and pass along an optional reason why.\n\n"

   "This method performs two operations: it disconnects a client connection from the server, "
   "and it deletes the connection object.  The optional reason is sent in the disconnect packet "
   "and is often displayed to the user so they know why they've been disconnected.\n\n"
   
   "@param reason [optional] The reason why the user has been disconnected from the server.\n\n"
   
   "@tsexample\n"
   "function kick(%client)\n"
   "{\n"
   "   messageAll( 'MsgAdminForce', '\\c2The Admin has kicked %1.', %client.playerName);\n"
   "\n"
   "   if (!%client.isAIControlled())\n"
   "      BanList::add(%client.guid, %client.getAddress(), $Pref::Server::KickBanTime);\n"
   "   %client.delete(\"You have been kicked from this server\");\n"
   "}\n"
   "@endtsexample\n\n")
{
   object->setDisconnectReason(reason);
   object->deleteObject();
}


//--------------------------------------------------------------------------
void GameConnection::consoleInit()
{
   Con::addVariable("$pref::Net::LagThreshold", TypeS32, &mLagThresholdMS,
      "@brief How long between received packets before the client is considered as lagging (in ms).\n\n"

      "This is used by GameConnection to determine if the client is lagging.  "
      "If the client is indeed lagging, setLagIcon() is called to inform the user in some way.  i.e. "
      "display an icon on screen.\n\n"

      "@see GameConnection, GameConnection::setLagIcon()\n\n"

      "@ingroup Networking\n");

   // Con::addVariable("specialFog", TypeBool, &SceneGraph::useSpecial);
}

DefineEngineMethod( GameConnection, startRecording, void, (const char* fileName),,
   "@brief On the client, starts recording the network connection's traffic to a demo file.\n\n"
   
   "It is often useful to play back a game session.  This could be for producing a "
   "demo of the game that will be shown at a later time, or for debugging a game.  "
   "By recording the entire network stream it is possible to later play game the game "
   "exactly as it unfolded during the actual play session.  This is because all user "
   "control and server results pass through the connection.\n\n"
   
   "@param fileName The file name to use for the demo recording.\n\n"
   
   "@see GameConnection::stopRecording(), GameConnection::playDemo()")
{
   char expFileName[1024];
   Con::expandScriptFilename(expFileName, sizeof(expFileName), fileName);
   object->startDemoRecord(expFileName);
}

DefineEngineMethod( GameConnection, stopRecording, void, (),,
   "@brief On the client, stops the recording of a connection's network traffic to a file.\n\n"
   
   "@see GameConnection::startRecording(), GameConnection::playDemo()")
{
   object->stopRecording();
}

DefineEngineMethod( GameConnection, playDemo, bool, (const char* demoFileName),,
   "@brief On the client, play back a previously recorded game session.\n\n"
   
   "It is often useful to play back a game session.  This could be for producing a "
   "demo of the game that will be shown at a later time, or for debugging a game.  "
   "By recording the entire network stream it is possible to later play game the game "
   "exactly as it unfolded during the actual play session.  This is because all user "
   "control and server results pass through the connection.\n\n"

   "@returns True if the playback was successful.  False if there was an issue, such as "
   "not being able to open the demo file for playback.\n\n"
   
   "@see GameConnection::startRecording(), GameConnection::stopRecording()")
{
   char filename[1024];
   Con::expandScriptFilename(filename, sizeof(filename), demoFileName);

   // Note that calling onConnectionEstablished will change the values in argv!
   object->onConnectionEstablished(true);
   object->setEstablished();

   if(!object->replayDemoRecord(filename))
   {
      Con::printf("Unable to open demo file %s.", filename);
      object->deleteObject();
      return false;
   }

   // After demo has loaded, execute the scene re-light the scene
   //SceneLighting::lightScene(0, 0);
   GameConnection::smPlayingDemo.trigger();

   return true;
}

DefineEngineMethod( GameConnection, isDemoPlaying, bool, (),,
   "@brief Returns true if a previously recorded demo file is now playing.\n\n"
   
   "@see GameConnection::playDemo()")
{
   return object->isPlayingBack();
}

DefineEngineMethod( GameConnection, isDemoRecording, bool, (),,
   "@brief Returns true if a demo file is now being recorded.\n\n"
   
   "@see GameConnection::startRecording(), GameConnection::stopRecording()")
{
   return object->isRecording();
}

DefineEngineMethod( GameConnection, listClassIDs, void, (),,
   "@brief List all of the classes that this connection knows about, and what their IDs are. Useful for debugging network problems.\n\n"
   "@note The list is sent to the console.\n\n")
{
   Con::printf("--------------- Class ID Listing ----------------");
   Con::printf(" id    |   name");

   for(AbstractClassRep *rep = AbstractClassRep::getClassList();
      rep;
      rep = rep->getNextClass())
   {
      ConsoleObject *obj = rep->create();
      if(obj && rep->getClassId(object->getNetClassGroup()) >= 0)
         Con::printf("%7.7d| %s", rep->getClassId(object->getNetClassGroup()), rep->getClassName());
      delete obj;
   }
}

DefineEngineStaticMethod( GameConnection, getServerConnection, S32, (),,
   "@brief On the client, this static mehtod will return the connection to the server, if any.\n\n"

   "@returns The SimObject ID of the server connection, or -1 if none is found.\n\n")
{
   if(GameConnection::getConnectionToServer())
      return GameConnection::getConnectionToServer()->getId();
   else
   {
      Con::errorf("GameConnection::getServerConnection - no connection available.");
      return -1;
   }
}

DefineEngineMethod( GameConnection, setCameraObject, bool, (GameBase* camera),,
   "@brief On the server, set the connection's camera object used when not viewing "
   "through the control object.\n\n"
   
   "@see GameConnection::getCameraObject() and GameConnection::clearCameraObject()\n\n")
{
   if(!camera)
      return false;

   object->setCameraObject(camera);
   return true;
}

DefineEngineMethod( GameConnection, getCameraObject, SimObject*, (),,
   "@brief Returns the connection's camera object used when not viewing through the control object.\n\n"
   
   "@see GameConnection::setCameraObject() and GameConnection::clearCameraObject()\n\n")
{
   SimObject *obj = dynamic_cast<SimObject*>(object->getCameraObject());
   return obj;
}

DefineEngineMethod( GameConnection, clearCameraObject, void, (),,
   "@brief Clear the connection's camera object reference.\n\n"
   
   "@see GameConnection::setCameraObject() and GameConnection::getCameraObject()\n\n")
{
   object->setCameraObject(NULL);
}

DefineEngineMethod( GameConnection, isFirstPerson, bool, (),,
   "@brief Returns true if this connection is in first person mode.\n\n"

   "@note Transition to first person occurs over time via mCameraPos, so this "
   "won't immediately return true after a set.\n\n")
{
   // Note: Transition to first person occurs over time via mCameraPos, so this
   // won't immediately return true after a set.
   return object->isFirstPerson();
}

DefineEngineMethod( GameConnection, setFirstPerson, void, (bool firstPerson),,
   "@brief On the server, sets this connection into or out of first person mode.\n\n"
   
   "@param firstPerson Set to true to put the connection into first person mode.\n\n")
{
   object->setFirstPerson(firstPerson);
}

DefineEngineMethod( GameConnection, setControlSchemeParameters, void, (bool absoluteRotation, bool addYawToAbsRot, bool addPitchToAbsRot),,
   "@brief Set the control scheme that may be used by a connection's control object.\n\n"
   
   "@param absoluteRotation Use absolute rotation values from client, likely through ExtendedMove.\n"
   "@param addYawToAbsRot Add relative yaw control to the absolute rotation calculation.  Only useful when absoluteRotation is true.\n\n" )
{
   object->setControlSchemeParameters(absoluteRotation, addYawToAbsRot, addPitchToAbsRot);
}

DefineEngineMethod( GameConnection, getControlSchemeAbsoluteRotation, bool, (),,
   "@brief Get the connection's control scheme absolute rotation property.\n\n"
   
   "@return True if the connection's control object should use an absolute rotation control scheme.\n\n"
   "@see GameConnection::setControlSchemeParameters()\n\n")
{
   return object->getControlSchemeAbsoluteRotation();
}

DefineEngineMethod( GameConnection, setVisibleGhostDistance, void, (F32 dist),,
   "@brief Sets the distance that objects around it will be ghosted. If set to 0, "
   "it may be defined by the LevelInfo.\n\n"
   "@dist - is the max distance\n\n"
   )
{
   object->setVisibleGhostDistance(dist);
}

DefineEngineMethod( GameConnection, getVisibleGhostDistance, F32, (),,
   "@brief Gets the distance that objects around the connection will be ghosted.\n\n"
   
   "@return S32 of distance.\n\n"
   )
{
   return object->getVisibleGhostDistance();
}