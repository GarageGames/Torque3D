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
#include <vector>
#include "net.h"
//----------------------------------------------------------------
// remote procedure call console functions
//----------------------------------------------------------------


   RemoteCommandEvent::RemoteCommandEvent(S32 argc, const char **argv, NetConnection *conn)
   {
      mArgc = argc;
      for(S32 i = 0; i < argc; i++)
      {
         if(argv[i][0] == StringTagPrefixByte)
         {
            char buffer[256];
            mTagv[i+1] = NetStringHandle(dAtoi(argv[i]+1));
            if(conn)
            {
               dSprintf(buffer + 1, sizeof(buffer) - 1, "%d", conn->getNetSendId(mTagv[i+1]));
               buffer[0] = StringTagPrefixByte;
               mArgv[i+1] = dStrdup(buffer);
            }
         }
         else
            mArgv[i+1] = dStrdup(argv[i]);
      }
   }

#ifdef TORQUE_DEBUG_NET
   const char *RemoteCommandEvent::getDebugName()
   {
      static char buffer[256];
      dSprintf(buffer, sizeof(buffer), "%s [%s]", getClassName(), mTagv[1].isValidString() ? mTagv[1].getString() : "--unknown--" );
      return buffer;
   }
#endif

   RemoteCommandEvent::~RemoteCommandEvent()
   {
      for(S32 i = 0; i < mArgc; i++)
         dFree(mArgv[i+1]);
   }

   void RemoteCommandEvent::pack(NetConnection* conn, BitStream *bstream)
   {
      bstream->writeInt(mArgc, CommandArgsBits);
      // write it out reversed... why?
      // automatic string substitution with later arguments -
      // handled automatically by the system.

      for(S32 i = 0; i < mArgc; i++)
         conn->packString(bstream, mArgv[i+1]);
   }

   void RemoteCommandEvent::write(NetConnection* conn, BitStream *bstream)
   {
      pack(conn, bstream);
   }

   void RemoteCommandEvent::unpack(NetConnection* conn, BitStream *bstream)
   {

      mArgc = bstream->readInt(CommandArgsBits);
      // read it out backwards
      for(S32 i = 0; i < mArgc; i++)
      {
         conn->unpackString(bstream, mBuf);
         mArgv[i+1] = dStrdup(mBuf);
      }
   }

   void RemoteCommandEvent::process(NetConnection *conn)
   {
      static char idBuf[10];

      // de-tag the command name

      for(S32 i = mArgc - 1; i >= 0; i--)
      {
         char *arg = mArgv[i+1];
         if(*arg == StringTagPrefixByte)
         {
            // it's a tag:
            U32 localTag = dAtoi(arg + 1);
            NetStringHandle tag = conn->translateRemoteStringId(localTag);
            NetStringTable::expandString( tag,
                                          mBuf,
                                          sizeof(mBuf),
                                          (mArgc - 1) - i,
                                          (const char**)(mArgv + i + 2) );
            dFree(mArgv[i+1]);
            mArgv[i+1] = dStrdup(mBuf);
         }
      }
      const char *rmtCommandName = dStrchr(mArgv[1], ' ') + 1;
      if(conn->isConnectionToServer())
      {
         dStrcpy(mBuf, "clientCmd");
         dStrcat(mBuf, rmtCommandName);

         char *temp = mArgv[1];
         mArgv[1] = mBuf;

         Con::execute(mArgc, (const char **) mArgv+1);
         mArgv[1] = temp;
      }
      else
      {
         dStrcpy(mBuf, "serverCmd");
         dStrcat(mBuf, rmtCommandName);
         char *temp = mArgv[1];

         dSprintf(idBuf, sizeof(idBuf), "%d", conn->getId());
         mArgv[0] = mBuf;
         mArgv[1] = idBuf;

         Con::execute(mArgc+1, (const char **) mArgv);
         mArgv[1] = temp;
      }
   }

   void RemoteCommandEvent::sendRemoteCommand(NetConnection *conn, S32 argc, const char **argv)
	   {
	   if(U8(argv[0][0]) != StringTagPrefixByte)
		   {
		   Con::errorf(ConsoleLogEntry::Script, "Remote Command Error - command must be a tag.");
		   return;
		   }
	   S32 i;
	   for(i = argc - 1; i >= 0; i--)
		   {
		   if(argv[i][0] != 0)
			   break;
		   argc = i;
		   }
	   for(i = 0; i < argc; i++)
		   conn->validateSendString(argv[i]);
	   RemoteCommandEvent *cevt = new RemoteCommandEvent(argc, argv, conn);
	   conn->postNetEvent(cevt);
	   }

   const char* RemoteCommandEvent::getTaggedString(const char* tag)
	{
	const char *indexPtr = tag;
	if (*indexPtr == StringTagPrefixByte)
		indexPtr++;
	return gNetStringTable->lookupString(dAtoi(indexPtr));
	}

   void RemoteCommandEvent::removeTaggedString(S32 tag)
	{
	if (tag)
		gNetStringTable->removeString(tag, true);
	}

   const char* RemoteCommandEvent::addTaggedString(const char* str)
	   {
	   NetStringHandle s(str);
	   gNetStringTable->incStringRefScript(s.getIndex());

	   char *ret = Con::getReturnBuffer(10);
	   ret[0] = StringTagPrefixByte;
	   dSprintf(ret + 1, 9, "%d", s.getIndex());
	   return ret;
	   }


char RemoteCommandEvent::mBuf[1024];

IMPLEMENT_CO_NETEVENT_V1(RemoteCommandEvent);

ConsoleDocClass( RemoteCommandEvent,
				"@brief Object used for remote procedure calls.\n\n"
				"Not intended for game development, for exposing ConsoleFunctions (such as commandToClient) only.\n\n"
				"@internal");



ConsoleFunctionGroupBegin( Net, "Functions for use with the network; tagged strings and remote commands.");


ConsoleFunction( commandToServer, void, 2, RemoteCommandEvent::MaxRemoteCommandArgs + 1, "(string func, ...)"
	"@brief Send a command to the server.\n\n"

   "@param func Name of the server command being called\n"
   "@param ... Various parameters being passed to server command\n\n"

   "@tsexample\n"
      "// Create a standard function.  Needs to be executed on the client, such \n"
      "// as within scripts/client/default.bind.cs\n"
      "function toggleCamera(%val)\n"
      "{\n"
      "	// If key was down, call a server command named 'ToggleCamera'\n"
      "	if (%val)\n"
      "		commandToServer('ToggleCamera');\n"
      "}\n\n"
      "// Server command being called from above.  Needs to be executed on the \n"
      "// server, such as within scripts/server/commands.cs\n"
      "function serverCmdToggleCamera(%client)\n"
      "{\n"
      "   if (%client.getControlObject() == %client.player)\n"
      "   {\n"
      "	  %client.camera.setVelocity(\"0 0 0\");\n"
      "	  %control = %client.camera;\n"
      "   }\n"
      "   else\n"
      "   {\n"
      "	  %client.player.setVelocity(\"0 0 0\");\n"
      "	  %control = %client.player;\n"
      "  }\n"
      "   %client.setControlObject(%control);\n"
      "   clientCmdSyncEditorGui();\n"
      "}\n"
   "@endtsexample\n\n"

   "@ingroup Networking")
{
   NetConnection *conn = NetConnection::getConnectionToServer();
   if(!conn)
      return;
   StringStackWrapper args(argc - 1, argv + 1);
   RemoteCommandEvent::sendRemoteCommand(conn, args.count(), args);
}

ConsoleFunction( commandToClient, void, 3, RemoteCommandEvent::MaxRemoteCommandArgs + 2, "(NetConnection client, string func, ...)"
   "@brief Send a command from the server to the client\n\n"

   "@param client The numeric ID of a client GameConnection\n"
   "@param func Name of the client function being called\n"
   "@param ... Various parameters being passed to client command\n\n"

   "@tsexample\n"
      "// Set up the client command.  Needs to be executed on the client, such as\n"
      "// within scripts/client/client.cs\n"
      "// Update the Ammo Counter with current ammo, if not any then hide the counter.\n"
      "function clientCmdSetAmmoAmountHud(%amount)\n"
      "{\n"
      "   if (!%amount)\n"
      "	  AmmoAmount.setVisible(false);\n"
      "   else\n"
      "   {\n"
      "	  AmmoAmount.setVisible(true);\n"
      "	  AmmoAmount.setText(\"Ammo: \"@%amount);\n"
      "   }\n"
      "}\n\n"
      "// Call it from a server function.  Needs to be executed on the server, \n"
      "//such as within scripts/server/game.cs\n"
      "function GameConnection::setAmmoAmountHud(%client, %amount)\n"
      "{\n"
      "   commandToClient(%client, 'SetAmmoAmountHud', %amount);\n"
      "}\n"
   "@endtsexample\n\n"

   "@ingroup Networking\n")
{
   NetConnection *conn;
   if(!Sim::findObject(argv[1], conn))
      return;
   StringStackWrapper args(argc - 2, argv + 2);
   RemoteCommandEvent::sendRemoteCommand(conn, args.count(), args);
}





DefineEngineFunction(removeTaggedString, void, (S32 tag), (-1),
   "@brief Remove a tagged string from the Net String Table\n\n"

   "@param tag The tag associated with the string\n\n"

   "@see \\ref syntaxDataTypes under Tagged %Strings\n"
   "@see addTaggedString()\n"
   "@see getTaggedString()\n"
   "@ingroup Networking\n")
   {
   RemoteCommandEvent::removeTaggedString(tag);
   }


DefineEngineFunction(addTaggedString, const char* , (const char* str), (""),
   "@brief Use the addTaggedString function to tag a new string and add it to the NetStringTable\n\n"

   "@param str The string to be tagged and placed in the NetStringTable. Tagging ignores case, "
   "so tagging the same string (excluding case differences) will be ignored as a duplicated tag.\n\n"

   "@return Returns a string( containing a numeric value) equivalent to the string ID for the newly tagged string"

   "@see \\ref syntaxDataTypes under Tagged %Strings\n"
   "@see removeTaggedString()\n"
   "@see getTaggedString()\n"
   "@ingroup Networking\n")
   {
   return RemoteCommandEvent::addTaggedString(str);
   }


DefineEngineFunction(getTaggedString, const char* , (const char *tag), (""),
   "@brief Use the getTaggedString function to convert a tag to a string.\n\n"

   "This is not the same as detag() which can only be used within the context "
   "of a function that receives a tag. This function can be used any time and "
   "anywhere to convert a tag to a string.\n\n"

   "@param tag A numeric tag ID.\n"

   "@returns The string as found in the Net String table.\n"

   "@see \\ref syntaxDataTypes under Tagged %Strings\n"
   "@see addTaggedString()\n"
   "@see removeTaggedString()\n"
   "@ingroup Networking\n")
	{
	return RemoteCommandEvent::getTaggedString(tag);
	}



ConsoleFunction( buildTaggedString, const char*, 2, 11, "(string format, ...)"
   "@brief Build a string using the specified tagged string format.\n\n"

   "This function takes an already tagged string (passed in as a tagged string ID) and one "
   "or more additional strings.  If the tagged string contains argument tags that range from "
   "%%1 through %%9, then each additional string will be substituted into the tagged string.  "
   "The final (non-tagged) combined string will be returned.  The maximum length of the tagged "
   "string plus any inserted additional strings is 511 characters.\n\n"

   "@param format A tagged string ID that contains zero or more argument tags, in the form of "
   "%%1 through %%9.\n"
   "@param ... A variable number of arguments that are insterted into the tagged string "
   "based on the argument tags within the format string.\n"

   "@returns An ordinary string that is a combination of the original tagged string with any additional "
   "strings passed in inserted in place of each argument tag.\n"

   "@tsexample\n"
      "// Create a tagged string with argument tags\n"
      "%taggedStringID = addTaggedString(\"Welcome %1 to the game!\");\n\n"

      "// Some point later, combine the tagged string with some other string\n"
      "%string = buildTaggedString(%taggedStringID, %playerName);\n"
      "echo(%string);\n"
	"@endtsexample\n\n"

   "@see \\ref syntaxDataTypes under Tagged %Strings\n"
   "@see addTaggedString()\n"
   "@see getTaggedString()\n"
   "@ingroup Networking\n")
{
   const char *indexPtr = argv[1];
   if (*indexPtr == StringTagPrefixByte)
      indexPtr++;
   const char *fmtString = gNetStringTable->lookupString(dAtoi(indexPtr));
   static const U32 bufSize = 512;
   char *strBuffer = Con::getReturnBuffer(bufSize);
   const char *fmtStrPtr = fmtString;
   char *strBufPtr = strBuffer;
   S32 strMaxLength = bufSize - 1;
   if (!fmtString)
      goto done;

   //build the string
   while (*fmtStrPtr)
   {
      //look for an argument tag
      if (*fmtStrPtr == '%')
      {
         if (fmtStrPtr[1] >= '1' && fmtStrPtr[1] <= '9')
         {
            S32 argIndex = S32(fmtStrPtr[1] - '0') + 1;
            if (argIndex >= argc)
               goto done;
            const char *argStr = argv[argIndex];
            if (!argStr)
               goto done;
            S32 strLength = dStrlen(argStr);
            if (strLength > strMaxLength)
               goto done;
            dStrcpy(strBufPtr, argStr);
            strBufPtr += strLength;
            strMaxLength -= strLength;
            fmtStrPtr += 2;
            continue;
         }
      }

      //if we don't continue, just copy the character
      if (strMaxLength <= 0)
         goto done;
      *strBufPtr++ = *fmtStrPtr++;
      strMaxLength--;
   }

done:
   *strBufPtr = '\0';
   return strBuffer;
}

ConsoleFunctionGroupEnd( Net );
