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

#include "app/banList.h"
#include "core/stream/fileStream.h"
#include "core/module.h"
#include "console/engineAPI.h"


IMPLEMENT_STATIC_CLASS( BanList,, "Functions for maintaing a list of banned users." );

ConsoleDoc(
   "@class BanList\n"
   "@ingroup Miscellaneous\n"
   "@brief Used for kicking and banning players from a server.\n"
   "There is only a single instance of BanList. It is very important to note that you do not ever create this object in script "
   "like you would other game play objects. You simply reference it via namespace.\n\n"
   "For this to be used effectively, make sure you are hooking up other functions to BanList. "
   "For example, functions like GameConnection::onConnectRequestRejected( %this, %msg ) and function GameConnection::onConnectRequest are excellent "
   "places to make use of the BanList. Other systems can be used in conjunction for strict control over a server\n\n"
   "@see addBadWord\n"
   "@see containsBadWords\n"
);

BanList* BanList::smInstance;

MODULE_BEGIN( BanList )
   MODULE_INIT
   {
      new BanList;
   }
   MODULE_SHUTDOWN
   {
      delete BanList::instance();
   }
MODULE_END;


//------------------------------------------------------------------------------

BanList::BanList()
{
   AssertFatal( !smInstance, "BanList::BanList - already instantiated" );
   VECTOR_SET_ASSOCIATION( list );
   
   smInstance = this;
}

//------------------------------------------------------------------------------

void BanList::addBan(S32 uniqueId, const char *TA, S32 banTime)
{
   S32 curTime = Platform::getTime();

   if(banTime != 0 && banTime < curTime)
      return;

   // make sure this bastard isn't already banned on this server
   Vector<BanInfo>::iterator i;
   for(i = list.begin();i != list.end();i++)
   {
      if(uniqueId == i->uniqueId)
      {
         i->bannedUntil = banTime;
         return;
      }
   }

   BanInfo b;
   dStrcpy(b.transportAddress, TA);
   b.uniqueId = uniqueId;
   b.bannedUntil = banTime;

   if(!dStrnicmp(b.transportAddress, "ip:", 3))
   {
      char *c = dStrchr(b.transportAddress+3, ':');
      if(c)
      {
         *(c+1) = '*';
         *(c+2) = 0;
      }
   }

   list.push_back(b);
}

//------------------------------------------------------------------------------

void BanList::addBanRelative(S32 uniqueId, const char *TA, S32 numSeconds)
{
   S32 curTime = Platform::getTime();
   S32 banTime = 0;
   if(numSeconds != -1)
      banTime = curTime + numSeconds;

   addBan(uniqueId, TA, banTime);
}

//------------------------------------------------------------------------------

void BanList::removeBan(S32 uniqueId, const char *)
{
   Vector<BanInfo>::iterator i;
   for(i = list.begin();i != list.end();i++)
   {
      if(uniqueId == i->uniqueId)
      {
         list.erase(i);
         return;
      }
   }
}

//------------------------------------------------------------------------------

bool BanList::isBanned(S32 uniqueId, const char *)
{
   S32 curTime = Platform::getTime();

   Vector<BanInfo>::iterator i;
   for(i = list.begin();i != list.end();)
   {
      if(i->bannedUntil != 0 && i->bannedUntil < curTime)
      {
         list.erase(i);
         continue;
      }
      else if(uniqueId == i->uniqueId)
         return true;
      i++;
   }
   return false;
}

//------------------------------------------------------------------------------

bool BanList::isTAEq(const char *bannedTA, const char *TA)
{
   char a, b;
   for(;;)
   {
      a = *bannedTA++;
      b = *TA++;
      if(a == '*' || (!a && b == ':')) // ignore port
         return true;
      if(dTolower(a) != dTolower(b))
         return false;
      if(!a)
         return true;
   }
}

//------------------------------------------------------------------------------

void BanList::exportToFile(const char *name)
{
   FileStream *banlist;

   char filename[1024];
   Con::expandScriptFilename(filename, sizeof(filename), name);
   if((banlist = FileStream::createAndOpen( filename, Torque::FS::File::Write )) == NULL)
      return;

   char buf[1024];
   Vector<BanInfo>::iterator i;
   for(i = list.begin(); i != list.end(); i++)
   {
      dSprintf(buf, sizeof(buf), "BanList::addAbsolute(%d, \"%s\", %d);\r\n", i->uniqueId, i->transportAddress, i->bannedUntil);
      banlist->write(dStrlen(buf), buf);
   }

   delete banlist;
}

//=============================================================================
//    API.
//=============================================================================
// MARK: ---- API ----

//-----------------------------------------------------------------------------

DefineEngineStaticMethod( BanList, addAbsolute, void, ( S32 uniqueId, const char* transportAddress, S32 banTime ),,
              "Ban a user until a given time.\n\n"
              "@param uniqueId Unique ID of the player.\n"
              "@param transportAddress Address from which the player connected.\n"
              "@param banTime Time at which they will be allowed back in."
			  "@tsexample\n"
			  "// Kick someone off the server\n"
			  "// %client - This is the connection to the person we are kicking\n"
			  "function kick(%client)\n"
			  "{\n"
			  "		// Let the server know what happened\n"
			  "		messageAll( 'MsgAdminForce', '\\c2The Admin has kicked %1.', %client.playerName);\n\n"
			  "		// If it is not an AI Player, execute the ban.\n"
			  "		if (!%client.isAIControlled())\n"
			  "			BanList::addAbsolute(%client.guid, %client.getAddress(), $pref::Server::KickBanTime);\n\n"				   
			  "		// Let the player know they messed up\n"
			  "		%client.delete(\"You have been kicked from this server\");\n"
			  "}\n"
			  "@endtsexample\n\n")
{
   BanList::instance()->addBan( uniqueId, transportAddress, banTime );
}

//-----------------------------------------------------------------------------

DefineEngineStaticMethod( BanList, add, void, ( S32 uniqueId, const char* transportAddress, S32 banLength ),,
              "Ban a user for banLength seconds.\n\n"
              "@param uniqueId Unique ID of the player.\n"
              "@param transportAddress Address from which the player connected.\n"
              "@param banLength Time period over which to ban the player."
			  "@tsexample\n"
			  "// Kick someone off the server\n"
			  "// %client - This is the connection to the person we are kicking\n"
			  "function kick(%client)\n"
			  "{\n"
			  "		// Let the server know what happened\n"
			  "		messageAll( 'MsgAdminForce', '\\c2The Admin has kicked %1.', %client.playerName);\n\n"
			  "		// If it is not an AI Player, execute the ban.\n"
			  "		if (!%client.isAIControlled())\n"
			  "			BanList::add(%client.guid, %client.getAddress(), $pref::Server::KickBanTime);\n\n"				   
			  "		// Let the player know they messed up\n"
			  "		%client.delete(\"You have been kicked from this server\");\n"
			  "}\n"
			  "@endtsexample\n\n")
{
   BanList::instance()->addBanRelative( uniqueId, transportAddress, banLength );
}

//-----------------------------------------------------------------------------

DefineEngineStaticMethod( BanList, removeBan, void, ( S32 uniqueId, const char* transportAddress ),,
              "Unban someone.\n\n"
              "@param uniqueId Unique ID of the player.\n"
              "@param transportAddress Address from which the player connected.\n" 
			  "@tsexample\n"
			  "BanList::removeBan(%userID, %ipAddress);\n"
			  "@endtsexample\n\n")
{
   BanList::instance()->removeBan( uniqueId, transportAddress );
}

//-----------------------------------------------------------------------------

DefineEngineStaticMethod( BanList, isBanned, bool, ( S32 uniqueId, const char* transportAddress ),,
              "Is someone banned?\n\n"
              "@param uniqueId Unique ID of the player.\n"
              "@param transportAddress Address from which the player connected.\n\n"
			  "@tsexample\n"
			  "//-----------------------------------------------------------------------------\n"
			  "// This script function is called before a client connection\n"
			  "// is accepted.  Returning "" will accept the connection,\n"
			  "// anything else will be sent back as an error to the client.\n"
			  "// All the connect args are passed also to onConnectRequest\n"
			  "function GameConnection::onConnectRequest( %client, %netAddress, %name )\n"
			  "{\n"
			  "	  // Find out who is trying to connect\n"
			  "	  echo(\"Connect request from: \" @ %netAddress);\n\n"
			  "	  // Are they allowed in?\n"
			  "	  if(BanList::isBanned(%client.guid, %netAddress))\n"
			  "		  return \"CR_YOUAREBANNED\";\n\n"
			  "	  // Is there room for an unbanned player?\n"
			  "	  if($Server::PlayerCount >= $pref::Server::MaxPlayers)\n"
			  "		  return \"CR_SERVERFULL\";\n"
			  "	  return "";\n"
			  "}\n"
			  "@endtsexample\n\n")
{
   return BanList::instance()->isBanned( uniqueId, transportAddress );
}

//-----------------------------------------------------------------------------

DefineEngineStaticMethod( BanList, export, void, ( const char* filename ),,
              "Dump the banlist to a file.\n\n"
              "@param filename Path of the file to write the list to.\n\n"
			  "@tsexample\n"
			  "BanList::Export(\"./server/banlist.cs\");\n"
			  "@endtsexample\n\n")
{
   BanList::instance()->exportToFile( filename );
}
