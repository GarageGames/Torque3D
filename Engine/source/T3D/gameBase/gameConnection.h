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

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// Arcane-FX for MIT Licensed Open Source version of Torque 3D from GarageGames
// Copyright (C) 2015 Faust Logic, Inc.
//
//    Changes:
//          db-cache -- implementation of datablock caching system.
//        obj-select -- implementation of object selection used for spell targeting.
//          zoned-in -- connection is flagged as "zoned-in" when client is fully
//              connected and user can interact with it.
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#ifndef _GAMECONNECTION_H_
#define _GAMECONNECTION_H_

#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif
#ifndef _GAMEBASE_H_
#include "T3D/gameBase/gameBase.h"
#endif
#ifndef _NETCONNECTION_H_
#include "sim/netConnection.h"
#endif
#ifndef _MOVEMANAGER_H_
#include "T3D/gameBase/moveManager.h"
#endif
#ifndef _BITVECTOR_H_
#include "core/bitVector.h"
#endif

enum GameConnectionConstants
{
   MaxClients = 126,
   DataBlockQueueCount = 16
};

class IDisplayDevice;
class SFXProfile;
class MatrixF;
class MatrixF;
class Point3F;
class MoveManager;
class MoveList;
struct Move;
struct AuthInfo;

// AFX CODE BLOCK (db-cache) <<
//
// To disable datablock caching, remove or comment out the AFX_CAP_DATABLOCK_CACHE define below.
// Also, at a minimum, the following script preferences should be set to false:
//   $pref::Client::EnableDatablockCache = false; (in arcane.fx/client/defaults.cs)
//   $Pref::Server::EnableDatablockCache = false; (in arcane.fx/server/defaults.cs)
// Alternatively, all script code marked with "DATABLOCK CACHE CODE" can be removed or
// commented out.
//
#define AFX_CAP_DATABLOCK_CACHE
// AFX CODE BLOCK (db-cache) >>

const F32 MinCameraFov              = 1.f;      ///< min camera FOV
const F32 MaxCameraFov              = 179.f;    ///< max camera FOV

class GameConnection : public NetConnection
{
private:
   typedef NetConnection Parent;

   SimObjectPtr<GameBase> mControlObject;
   SimObjectPtr<GameBase> mCameraObject;
   U32 mDataBlockSequence;
   char mDisconnectReason[256];

   U32  mMissionCRC;             // crc of the current mission file from the server

   F32 mVisibleGhostDistance;

private:
   U32 mLastControlRequestTime;
   S32 mDataBlockModifiedKey;
   S32 mMaxDataBlockModifiedKey;

   /// @name Client side first/third person
   /// @{

   ///
   bool  mFirstPerson;     ///< Are we currently first person or not.
   bool  mUpdateFirstPerson; ///< Set to notify client or server of first person change.
   bool  mUpdateCameraFov; ///< Set to notify server of camera FOV change.
   F32   mCameraFov;       ///< Current camera fov (in degrees).
   F32   mCameraPos;       ///< Current camera pos (0-1).
   F32   mCameraSpeed;     ///< Camera in/out speed.

   IDisplayDevice* mDisplayDevice;  ///< Optional client display device that imposes rendering properties.
   /// @}

   /// @name Client side control scheme that may be referenced by control objects
   /// @{
   bool  mUpdateControlScheme;   ///< Set to notify client or server of control scheme change
   bool  mAbsoluteRotation;      ///< Use absolute rotation values from client, likely through ExtendedMove
   bool  mAddYawToAbsRot;        ///< Add relative yaw control to the absolute rotation calculation.  Only useful with mAbsoluteRotation.
   bool  mAddPitchToAbsRot;      ///< Add relative pitch control to the absolute rotation calculation.  Only useful with mAbsoluteRotation.
   /// @}

public:

   /// @name Protocol Versions
   ///
   /// Protocol versions are used to indicated changes in network traffic.
   /// These could be changes in how any object transmits or processes
   /// network information. You can specify backwards compatibility by
   /// specifying a MinRequireProtocolVersion.  If the client
   /// protocol is >= this min value, the connection is accepted.
   ///
   /// Torque (V12) SDK 1.0 uses protocol  =  1
   ///
   /// Torque SDK 1.1 uses protocol = 2
   /// Torque SDK 1.4 uses protocol = 12
   /// @{
   static const U32 CurrentProtocolVersion;
   static const U32 MinRequiredProtocolVersion;
   /// @}

   /// Configuration
   enum Constants {
      BlockTypeMove = NetConnectionBlockTypeCount,
      GameConnectionBlockTypeCount,
      MaxConnectArgs = 16,
      DataBlocksDone = NumConnectionMessages,
      DataBlocksDownloadDone,
   };

   /// Set connection arguments; these are passed to the server when we connect.
   void setConnectArgs(U32 argc, const char **argv);

   /// Set the server password to use when we join.
   void setJoinPassword(const char *password);

   /// @name Event Handling
   /// @{

   virtual void onTimedOut();
   virtual void onConnectTimedOut();
   virtual void onDisconnect(const char *reason);
   virtual void onConnectionRejected(const char *reason);
   virtual void onConnectionEstablished(bool isInitiator);
   virtual void handleStartupError(const char *errorString);
   /// @}

   /// @name Packet I/O
   /// @{

   virtual void writeConnectRequest(BitStream *stream);
   virtual bool readConnectRequest(BitStream *stream, const char **errorString);
   virtual void writeConnectAccept(BitStream *stream);
   virtual bool readConnectAccept(BitStream *stream, const char **errorString);
   /// @}

   bool canRemoteCreate();

   void setVisibleGhostDistance(F32 dist);
   F32 getVisibleGhostDistance();

private:
   /// @name Connection State
   /// This data is set with setConnectArgs() and setJoinPassword(), and
   /// sent across the wire when we connect.
   /// @{

   U32      mConnectArgc;
   char *mConnectArgv[MaxConnectArgs];
   char *mJoinPassword;
   /// @}

protected:
   struct GamePacketNotify : public NetConnection::PacketNotify
   {
      S32 cameraFov;
      GamePacketNotify();
   };
   PacketNotify *allocNotify();

   bool mControlForceMismatch;

   Vector<SimDataBlock *> mDataBlockLoadList;

public:

   MoveList *mMoveList;

protected:
   bool        mAIControlled;
   AuthInfo *  mAuthInfo;

   static S32  mLagThresholdMS;
   S32         mLastPacketTime;
   bool        mLagging;

   /// @name Flashing
   ////
   /// Note, these variables are not networked, they are for the local connection only.
   /// @{
   F32 mDamageFlash;
   F32 mWhiteOut;

   F32   mBlackOut;
   S32   mBlackOutTimeMS;
   S32   mBlackOutStartTimeMS;
   bool  mFadeToBlack;

   /// @}

   /// @name Packet I/O
   /// @{

   void readPacket      (BitStream *bstream);
   void writePacket     (BitStream *bstream, PacketNotify *note);
   void packetReceived  (PacketNotify *note);
   void packetDropped   (PacketNotify *note);
   void connectionError (const char *errorString);

   void writeDemoStartBlock   (ResizeBitStream *stream);
   bool readDemoStartBlock    (BitStream *stream);
   void handleRecordedBlock   (U32 type, U32 size, void *data);
   /// @}
   void ghostWriteExtra(NetObject *,BitStream *);
   void ghostReadExtra(NetObject *,BitStream *, bool newGhost);
   void ghostPreRead(NetObject *, bool newGhost);
   
   virtual void onEndGhosting();

public:

   DECLARE_CONOBJECT(GameConnection);
   void handleConnectionMessage(U32 message, U32 sequence, U32 ghostCount);
   void preloadDataBlock(SimDataBlock *block);
   void fileDownloadSegmentComplete();
   void preloadNextDataBlock(bool hadNew);
   
   static void consoleInit();

   void setDisconnectReason(const char *reason);
   GameConnection();
   ~GameConnection();

   bool onAdd();
   void onRemove();

   static GameConnection *getConnectionToServer() 
   { 
      return dynamic_cast<GameConnection*>((NetConnection *) mServerConnection); 
   }
   
   static GameConnection *getLocalClientConnection() 
   { 
      return dynamic_cast<GameConnection*>((NetConnection *) mLocalClientConnection); 
   }

   /// @name Control object
   /// @{

   ///
   void setControlObject(GameBase *);
   GameBase* getControlObject() {  return  mControlObject; }
   const GameBase* getControlObject() const {  return  mControlObject; }
   
   void setCameraObject(GameBase *);
   GameBase* getCameraObject();
   
   bool getControlCameraTransform(F32 dt,MatrixF* mat);
   bool getControlCameraVelocity(Point3F *vel);

   /// Returns the head transform for the control object, using supplemental information
   /// from the provided IDisplayDevice
   bool getControlCameraHeadTransform(IDisplayDevice *display, MatrixF *transform);

   /// Returns the eye transforms for the control object, using supplemental information 
   /// from the provided IDisplayDevice.
   bool getControlCameraEyeTransforms(IDisplayDevice *display, MatrixF *transforms);
   
   bool getControlCameraDefaultFov(F32 *fov);
   bool getControlCameraFov(F32 *fov);
   bool setControlCameraFov(F32 fov);
   bool isValidControlCameraFov(F32 fov);
   
   // Used by editor
   bool isControlObjectRotDampedCamera();

   void setFirstPerson(bool firstPerson);
   
   bool hasDisplayDevice() const { return mDisplayDevice != NULL; }
   IDisplayDevice* getDisplayDevice() const { return mDisplayDevice; }
   void setDisplayDevice(IDisplayDevice* display) { if (mDisplayDevice) mDisplayDevice->setDrawCanvas(NULL); mDisplayDevice = display; }
   void clearDisplayDevice() { mDisplayDevice = NULL; }

   void setControlSchemeParameters(bool absoluteRotation, bool addYawToAbsRot, bool addPitchToAbsRot);
   bool getControlSchemeAbsoluteRotation() {return mAbsoluteRotation;}
   bool getControlSchemeAddYawToAbsRot() {return mAddYawToAbsRot;}
   bool getControlSchemeAddPitchToAbsRot() {return mAddPitchToAbsRot;}

   /// @}

   void detectLag();

   /// @name Datablock management
   /// @{

   S32  getDataBlockModifiedKey     ()  { return mDataBlockModifiedKey; }
   void setDataBlockModifiedKey     (S32 key)  { mDataBlockModifiedKey = key; }
   S32  getMaxDataBlockModifiedKey  ()  { return mMaxDataBlockModifiedKey; }
   void setMaxDataBlockModifiedKey  (S32 key)  { mMaxDataBlockModifiedKey = key; }

   /// Return the datablock sequence number that this game connection is on.
   /// The datablock sequence number is synchronized to the mission sequence number
   /// on each datablock transmission.
   U32 getDataBlockSequence() { return mDataBlockSequence; }
   
   /// Set the datablock sequence number.
   void setDataBlockSequence(U32 seq) { mDataBlockSequence = seq; }

   /// @}

   /// @name Fade control
   /// @{

   F32 getDamageFlash() const { return mDamageFlash; }
   F32 getWhiteOut() const { return mWhiteOut; }

   void setBlackOut(bool fadeToBlack, S32 timeMS);
   F32  getBlackOut();
   /// @}

   /// @name Authentication
   ///
   /// This is remnant code from Tribes 2.
   /// @{

   void            setAuthInfo(const AuthInfo *info);
   const AuthInfo *getAuthInfo();
   /// @}

   /// @name Sound
   /// @{

   void play2D(SFXProfile *profile);
   void play3D(SFXProfile *profile, const MatrixF *transform);
   /// @}

   /// @name Misc.
   /// @{

   bool isFirstPerson() const  { return mCameraPos == 0; }
   bool isAIControlled() { return mAIControlled; }

   void doneScopingScene();
   void demoPlaybackComplete();

   void setMissionCRC(U32 crc)           { mMissionCRC = crc; }
   U32  getMissionCRC()           { return(mMissionCRC); }
   /// @}

   static Signal<void(F32)> smFovUpdate;
   static Signal<void()> smPlayingDemo;

protected:
   DECLARE_CALLBACK( void, onConnectionTimedOut, () );
   DECLARE_CALLBACK( void, onConnectionAccepted, () );
   DECLARE_CALLBACK( void, onConnectRequestTimedOut, () );
   DECLARE_CALLBACK( void, onConnectionDropped, (const char* reason) );
   DECLARE_CALLBACK( void, onConnectRequestRejected, (const char* reason) );
   DECLARE_CALLBACK( void, onConnectionError, (const char* errorString) );
   DECLARE_CALLBACK( void, onDrop, (const char* disconnectReason) );
   DECLARE_CALLBACK( void, initialControlSet, () );
   DECLARE_CALLBACK( void, onControlObjectChange, () );
   DECLARE_CALLBACK( void, setLagIcon, (bool state) );
   DECLARE_CALLBACK( void, onDataBlocksDone, (U32 sequence) );
   DECLARE_CALLBACK( void, onFlash, (bool state) );

   // AFX CODE BLOCK (obj-select)(zoned-in) <<
   // GameConnection is modified to keep track of object selections which are used in
   // spell targeting. This code stores the current object selection as well as the
   // current rollover object beneath the cursor. The rollover object is treated as a
   // pending object selection and actual object selection is usually made by promoting
   // the rollover object to the current object selection.
private:   
   SimObjectPtr<SceneObject> mRolloverObj;  
   SimObjectPtr<SceneObject> mPreSelectedObj;  
   SimObjectPtr<SceneObject> mSelectedObj;  
   bool          mChangedSelectedObj;
   U32           mPreSelectTimestamp;
protected:
   virtual void  onDeleteNotify(SimObject*);
public:   
   void          setRolloverObj(SceneObject*);   
   SceneObject*  getRolloverObj() { return  mRolloverObj; }   
   void          setSelectedObj(SceneObject*, bool propagate_to_client=false);   
   SceneObject*  getSelectedObj() { return  mSelectedObj; }  
   void          setPreSelectedObjFromRollover();
   void          clearPreSelectedObj();
   void          setSelectedObjFromPreSelected();
   // Flag is added to indicate when a client is fully connected or "zoned-in". 
   // This information determines when AFX will startup active effects on a newly
   // added client. 
private:
   bool          zoned_in;
public:
   bool          isZonedIn() const { return zoned_in; }
   void          setZonedIn() { zoned_in = true; }
   // AFX CODE BLOCK (obj-select)(zoned-in) >>
#ifdef AFX_CAP_DATABLOCK_CACHE // AFX CODE BLOCK (db-cache) <<
private:
   static StringTableEntry  server_cache_filename;
   static StringTableEntry  client_cache_filename;
   static bool   server_cache_on;
   static bool   client_cache_on;
   BitStream*    client_db_stream;
   U32           server_cache_CRC;
public:
   void          repackClientDatablock(BitStream*, S32 start_pos);
   void          saveDatablockCache(bool on_server);
   void          loadDatablockCache();
   bool          loadDatablockCache_Begin();
   bool          loadDatablockCache_Continue();
   void          tempDisableStringBuffering(BitStream* bs) const;
   void          restoreStringBuffering(BitStream* bs) const;
   void          setServerCacheCRC(U32 crc) { server_cache_CRC = crc; }

   static void   resetDatablockCache();
   static bool   serverCacheEnabled() { return server_cache_on; }
   static bool   clientCacheEnabled() { return client_cache_on; }
   static const char* serverCacheFilename() { return server_cache_filename; }
   static const char* clientCacheFilename() { return client_cache_filename; }
#endif // AFX CODE BLOCK (db-cache) >>
};

#endif
