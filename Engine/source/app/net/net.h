#ifndef _NET_H_
#define _NET_H_

#include "platform/platform.h"
#include "core/dnet.h"
#include "core/idGenerator.h"
#include "core/stream/bitStream.h"
#include "console/simBase.h"
#include "console/console.h"
#include "console/consoleTypes.h"
#include "sim/netConnection.h"
#include "sim/netObject.h"
#include "app/net/serverQuery.h"
#include "console/engineAPI.h"

class RemoteCommandEvent : public NetEvent
{
public:
   typedef NetEvent Parent;
   enum {
      MaxRemoteCommandArgs = 20,
      CommandArgsBits = 5
   };

private:
   S32 mArgc;
   char *mArgv[MaxRemoteCommandArgs + 1];
   NetStringHandle mTagv[MaxRemoteCommandArgs + 1];
   static char mBuf[1024];

public:
   RemoteCommandEvent(S32 argc=0, const char **argv=NULL, NetConnection *conn = NULL);

#ifdef TORQUE_DEBUG_NET
   const char *getDebugName();
#endif

   ~RemoteCommandEvent();

   virtual void pack(NetConnection* conn, BitStream *bstream);

   virtual void write(NetConnection* conn, BitStream *bstream);

   virtual void unpack(NetConnection* conn, BitStream *bstream);

   virtual void process(NetConnection *conn);

   static void sendRemoteCommand(NetConnection *conn, S32 argc, const char **argv);
	 
   static void removeTaggedString(S32);

   static const char* addTaggedString(const char* str);

   static const char* getTaggedString(const char* tag);

   DECLARE_CONOBJECT(RemoteCommandEvent);
};

#endif