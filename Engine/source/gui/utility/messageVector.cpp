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

#include "gui/utility/messageVector.h"
#include "core/fileObject.h"
#include "console/engineAPI.h"

IMPLEMENT_CONOBJECT(MessageVector);

ConsoleDocClass( MessageVector,
	"@brief Store a list of chat messages.\n\n"

	"This is responsible for managing messages which appear in the chat HUD, not the actual control rendered to the screen\n\n"
	
	"@tsexample\n"
	"// Declare ChatHud, which is what will display the actual chat from a MessageVector\n"
	"new GuiMessageVectorCtrl(ChatHud) {\n"
    "   profile = \"ChatHudMessageProfile\";\n"
    "   horizSizing = \"width\";\n"
    "   vertSizing = \"height\";\n"
    "   position = \"1 1\";\n"
    "   extent = \"252 16\";\n"
    "   minExtent = \"8 8\";\n"
    "   visible = \"1\";\n"
    "   helpTag = \"0\";\n"
    "   lineSpacing = \"0\";\n"
    "   lineContinuedIndex = \"10\";\n"
    "   matchColor = \"0 0 255 255\";\n"
    "   maxColorIndex = \"5\";\n"
    "};\n\n"
	"// All messages are stored in this HudMessageVector, the actual\n"
	"// MainChatHud only displays the contents of this vector.\n"
	"new MessageVector(HudMessageVector);\n\n"
	"// Attach the MessageVector to the chat control\n"
	"chatHud.attach(HudMessageVector);\n"
	"@endtsexample\n\n"

	"@see GuiMessageVectorCtrl for more details on how this is used."

	"@ingroup GuiUtil\n"
);

DefineEngineMethod( MessageVector, clear, void, (),,
   "Clear all messages in the vector\n\n"
   "@tsexample\n"
   "HudMessageVector.clear();\n"
   "@endtsexample\n\n")
{
   object->clear();
}

//ConsoleMethod( MessageVector, clear, void, 2, 2, "Clear the message vector.")
//{
//   object->clear();
//}

DefineEngineMethod( MessageVector, pushBackLine, void, ( const char* msg, S32 tag ),,
   "Push a line onto the back of the list.\n\n"
   "@param msg Text that makes up the message\n"
   "@param tag Numerical value associated with this message, useful for searching.\n\n"
   "@tsexample\n"
   "// Add the message...\n"
   "HudMessageVector.pushBackLine(\"Hello World\", 0);\n"
   "@endtsexample\n\n")
{
   object->pushBackLine(msg, tag);
}

//ConsoleMethod( MessageVector, pushBackLine, void, 3, 4, "(string msg, int tag=0)"
//              "Push a line onto the back of the list.")
//{
//   U32 tag = 0;
//   if (argc == 4)
//      tag = dAtoi(argv[3]);
//
//   object->pushBackLine(argv[2], tag);
//}

DefineEngineMethod( MessageVector, popBackLine, bool, (),,
   "Pop a line from the back of the list; destroys the line.\n\n"
   "@tsexample\n"
   "HudMessageVector.popBackLine();\n"
   "@endtsexample\n\n"
   "@return False if there are no lines to pop (underflow), true otherwise")
{
   if (object->getNumLines() == 0) {
      Con::errorf(ConsoleLogEntry::General, "MessageVector::popBackLine(): underflow");
      return false;
   }

   object->popBackLine();
   return true;
}

//ConsoleMethod( MessageVector, popBackLine, bool, 2, 2, "()"
//              "Pop a line from the back of the list; destroys the line.")
//{
//   if (object->getNumLines() == 0) {
//      Con::errorf(ConsoleLogEntry::General, "MessageVector::popBackLine(): underflow");
//      return false;
//   }
//
//   object->popBackLine();
//   return true;
//}

DefineEngineMethod( MessageVector, pushFrontLine, void, ( const char* msg, S32 tag ),,
   "Push a line onto the front of the vector.\n\n"
   "@param msg Text that makes up the message\n"
   "@param tag Numerical value associated with this message, useful for searching.\n\n"
   "@tsexample\n"
   "// Add the message...\n"
   "HudMessageVector.pushFrontLine(\"Hello World\", 0);\n"
   "@endtsexample\n\n")
{
   object->pushFrontLine(msg, tag);
}

//ConsoleMethod( MessageVector, pushFrontLine, void, 3, 4, "(string msg, int tag=0)"
//              "Push a line onto the front of the vector.")
//{
//   U32 tag = 0;
//   if (argc == 4)
//      tag = dAtoi(argv[3]);
//
//   object->pushFrontLine(argv[2], tag);
//}

DefineEngineMethod( MessageVector, popFrontLine, bool, (),,
   "Pop a line from the front of the vector, destroying the line.\n\n"
   "@tsexample\n"
   "HudMessageVector.popFrontLine();\n"
   "@endtsexample\n\n"
   "@return False if there are no lines to pop (underflow), true otherwise")
{
   if (object->getNumLines() == 0) {
      Con::errorf(ConsoleLogEntry::General, "MessageVector::popFrontLine(): underflow");
      return false;
   }

   object->popFrontLine();
   return true;
}

//ConsoleMethod( MessageVector, popFrontLine, bool, 2, 2,
//              "Pop a line from the front of the vector, destroying the line.")
//{
//   if (object->getNumLines() == 0) {
//      Con::errorf(ConsoleLogEntry::General, "MessageVector::popFrontLine(): underflow");
//      return false;
//   }
//
//   object->popFrontLine();
//   return true;
//}

DefineEngineMethod( MessageVector, insertLine, bool, ( S32 insertPos, const char* msg, S32 tag ),,
   "Push a line onto the back of the list.\n\n"
   "@param msg Text that makes up the message\n"
   "@param tag Numerical value associated with this message, useful for searching.\n\n"
   "@tsexample\n"
   "// Add the message...\n"
   "HudMessageVector.insertLine(1, \"Hello World\", 0);\n"
   "@endtsexample\n\n"
   "@return False if insertPos is greater than the number of lines in the current vector")
{
   if (insertPos > object->getNumLines())
      return false;

   object->insertLine(insertPos, msg, tag);
   return true;
}

//ConsoleMethod( MessageVector, insertLine, bool, 4, 5, "(int insertPos, string msg, int tag=0)"
//              "Insert a new line into the vector at the specified position.")
//{
//   U32 pos = U32(dAtoi(argv[2]));
//   if (pos > object->getNumLines())
//      return false;
//
//   S32 tag = 0;
//   if (argc == 5)
//      tag = dAtoi(argv[4]);
//
//   object->insertLine(pos, argv[3], tag);
//   return true;
//}

DefineEngineMethod( MessageVector, deleteLine, bool, ( S32 deletePos),,
   "Delete the line at the specified position.\n\n"
   "@param deletePos Position in the vector containing the line to be deleted\n"
   "@tsexample\n"
   "// Delete the first line (index 0) in the vector...\n"
   "HudMessageVector.deleteLine(0);\n"
   "@endtsexample\n\n"
   "@return False if deletePos is greater than the number of lines in the current vector")
{
   if (deletePos >= object->getNumLines())
      return false;

   object->deleteLine(deletePos);
   return true;
}

//ConsoleMethod( MessageVector, deleteLine, bool, 3, 3, "(int deletePos)"
//               "Delete the line at the specified position.")
//{
//   U32 pos = U32(dAtoi(argv[2]));
//   if (pos >= object->getNumLines())
//      return false;
//
//   object->deleteLine(pos);
//   return true;
//}
static ConsoleDocFragment _MessageVectordump1(
   "@brief Dump the message vector to a file without a header.\n\n"
   
   "@param filename Name and path of file to dump text to.\n"
   
   "@tsexample\n"
   "// Dump the entire chat log to a text file\n"
   "HudMessageVector.dump(\"./chatLog.txt\");\n"
   "@endtsexample\n\n\n",
   "MessageVector",
   "void dump( string filename);");

static ConsoleDocFragment _MessageVectordump2(
   "@brief Dump the message vector to a file with a header.\n\n"
   
   "@param filename Name and path of file to dump text to.\n"
   "@param header Prefix information for write out\n\n"
   
   "@tsexample\n"
   "// Arbitrary header data\n"
   "%headerInfo = \"Ars Moriendi Chat Log\";\n\n"
   "// Dump the entire chat log to a text file\n"
   "HudMessageVector.dump(\"./chatLog.txt\", %headerInfo);\n"
   "@endtsexample\n\n\n",
   "MessageVector",
   "void dump( string filename, string header);");

DefineConsoleMethod( MessageVector, dump, void, (const char * filename, const char * header), (""), "(string filename, string header=NULL)"
              "Dump the message vector to a file, optionally prefixing a header."
			  "@hide")
{

   object->dump( filename, header );
}

DefineEngineMethod( MessageVector, getNumLines, S32, (),,
   "Get the number of lines in the vector.\n\n"
   "@tsexample\n"
   "// Find out how many lines have been stored in HudMessageVector\n"
   "%chatLines = HudMessageVector.getNumLines();\n"
   "echo(%chatLines);\n"
   "@endtsexample\n\n")
{
   return object->getNumLines();
}

//ConsoleMethod( MessageVector, getNumLines, S32, 2, 2, "Get the number of lines in the vector.")
//{
//   return object->getNumLines();
//}

DefineEngineMethod( MessageVector, getLineTextByTag, const char*, ( S32 tag),,
   "Scan through the lines in the vector, returning the first line that has a matching tag.\n\n"
   "@param tag Numerical value assigned to a message when it was added or inserted\n"
   "@tsexample\n"
   "// Locate text in the vector tagged with the value \"1\", then print it\n"
   "%taggedText = HudMessageVector.getLineTextByTag(1);\n"
   "echo(%taggedText);\n"
   "@endtsexample\n\n"
   "@return Text from a line with matching tag, other wise \"\"")
{
   for(U32 i = 0; i < object->getNumLines(); i++)
      if(object->getLine(i).messageTag == tag)
         return object->getLine(i).message;
   return "";
}

//ConsoleMethod( MessageVector, getLineTextByTag, const char*, 3, 3, "(int tag)"
//              "Scan through the lines in the vector, returning the first line that has a matching tag.")
//{
//   U32 tag = dAtoi(argv[2]);
//
//   for(U32 i = 0; i < object->getNumLines(); i++)
//      if(object->getLine(i).messageTag == tag)
//         return object->getLine(i).message;
//   return "";
//}

DefineEngineMethod( MessageVector, getLineIndexByTag, S32, ( S32 tag),,
   "Scan through the vector, returning the line number of the first line that matches the specified tag; else returns -1 if no match was found.\n\n"
   "@param tag Numerical value assigned to a message when it was added or inserted\n"
   "@tsexample\n"
   "// Locate a line of text tagged with the value \"1\", then delete it.\n"
   "%taggedLine = HudMessageVector.getLineIndexByTag(1);\n"
   "HudMessageVector.deleteLine(%taggedLine);\n"
   "@endtsexample\n\n"
   "@return Line with matching tag, other wise -1")
{
   for(U32 i = 0; i < object->getNumLines(); i++)
      if(object->getLine(i).messageTag == tag)
         return i;
   return -1;
}

//ConsoleMethod( MessageVector, getLineIndexByTag, S32, 3, 3, "(int tag)"
//              "Scan through the vector, returning the line number of the first line that matches the specified tag; else returns -1 if no match was found.")
//{
//   U32 tag = dAtoi(argv[2]);
//
//   for(U32 i = 0; i < object->getNumLines(); i++)
//      if(object->getLine(i).messageTag == tag)
//         return i;
//   return -1;
//}

DefineEngineMethod( MessageVector, getLineText, const char*, ( S32 pos),,
   "Get the text at a specified line.\n\n"
   "@param pos Position in vector to grab text from\n"
   "@tsexample\n"
   "// Print a line of text at position 1.\n"
   "%text = HudMessageVector.getLineText(1);\n"
   "echo(%text);\n"
   "@endtsexample\n\n"
   "@return Text at specified line, if the position is greater than the number of lines return \"\"")
{
   if (pos >= object->getNumLines()) {
      Con::errorf(ConsoleLogEntry::General, "MessageVector::getLineText(con): out of bounds line");
      return "";
   }

   return object->getLine(pos).message;
}

//ConsoleMethod( MessageVector, getLineText, const char*, 3, 3, "(int line)"
//              "Get the text at a specified line.")
//{
//   U32 pos = U32(dAtoi(argv[2]));
//   if (pos >= object->getNumLines()) {
//      Con::errorf(ConsoleLogEntry::General, "MessageVector::getLineText(con): out of bounds line");
//      return "";
//   }
//
//   return object->getLine(pos).message;
//}

DefineEngineMethod( MessageVector, getLineTag, S32, ( S32 pos),,
   "Get the tag of a specified line.\n\n"
   "@param pos Position in vector to grab tag from\n"
   "@tsexample\n"
   "// Remove all lines that do not have a tag value of 1.\n"
   "while( HudMessageVector.getNumLines())\n"
   "{\n"
   "   %tag = HudMessageVector.getLineTag(1);\n"
   "   if(%tag != 1)\n"
   "      %tag.delete();\n"
   "   HudMessageVector.popFrontLine();\n"
   "}\n"
   "@endtsexample\n\n"
   "@return Tag value of a given line, if the position is greater than the number of lines return 0")
{
   if (pos >= object->getNumLines()) {
      Con::errorf(ConsoleLogEntry::General, "MessageVector::getLineTag(con): out of bounds line");
      return 0;
   }

   return object->getLine(pos).messageTag;
}

//ConsoleMethod( MessageVector, getLineTag, S32, 3, 3, "(int line)"
//              "Get the tag of a specified line.")
//{
//   U32 pos = U32(dAtoi(argv[2]));
//   if (pos >= object->getNumLines()) {
//      Con::errorf(ConsoleLogEntry::General, "MessageVector::getLineTag(con): out of bounds line");
//      return 0;
//   }
//
//   return object->getLine(pos).messageTag;
//}

//--------------------------------------------------------------------------
MessageVector::MessageVector()
{
   VECTOR_SET_ASSOCIATION(mMessageLines);
   VECTOR_SET_ASSOCIATION(mSpectators);
}


//--------------------------------------------------------------------------
MessageVector::~MessageVector()
{
   for (U32 i = 0; i < mMessageLines.size(); i++) {
      char* pDelete = const_cast<char*>(mMessageLines[i].message);
      delete [] pDelete;
      mMessageLines[i].message = 0;
      mMessageLines[i].messageTag = 0xFFFFFFFF;
   }
   mMessageLines.clear();
}


//--------------------------------------------------------------------------
void MessageVector::initPersistFields()
{
   Parent::initPersistFields();
}



//--------------------------------------------------------------------------
bool MessageVector::onAdd()
{
   return Parent::onAdd();
}


//--------------------------------------------------------------------------
void MessageVector::onRemove()
{
   // Delete all the lines from the observers, and then forcibly detatch ourselves
   //
   for (S32 i = mMessageLines.size() - 1; i >= 0; i--)
      spectatorMessage(LineDeleted, i);
   spectatorMessage(VectorDeletion, 0);
   mSpectators.clear();

   Parent::onRemove();
}


//--------------------------------------------------------------------------
void MessageVector::pushBackLine(const char* newMessage, const S32 newMessageTag)
{
   insertLine(mMessageLines.size(), newMessage, newMessageTag);
}


void MessageVector::popBackLine()
{
   AssertFatal(mMessageLines.size() != 0, "MessageVector::popBackLine: nothing to pop!");
   if (mMessageLines.size() == 0)
      return;

   deleteLine(mMessageLines.size() - 1);
}

void MessageVector::clear()
{
   while(mMessageLines.size())
      deleteLine(mMessageLines.size() - 1);
}

//--------------------------------------------------------------------------
void MessageVector::pushFrontLine(const char* newMessage, const S32 newMessageTag)
{
   insertLine(0, newMessage, newMessageTag);
}


void MessageVector::popFrontLine()
{
   AssertFatal(mMessageLines.size() != 0, "MessageVector::popBackLine: nothing to pop!");
   if (mMessageLines.size() == 0)
      return;

   deleteLine(0);
}


//--------------------------------------------------------------------------
void MessageVector::insertLine(const U32   position,
                               const char* newMessage,
                               const S32   newMessageTag)
{
   AssertFatal(position >= 0 && position <= mMessageLines.size(), "MessageVector::insertLine: out of range position!");
   AssertFatal(newMessage != NULL, "Error, no message to insert!");

   U32 len = dStrlen(newMessage) + 1;
   char* copy = new char[len];
   dStrcpy(copy, newMessage);

   mMessageLines.insert(position);
   mMessageLines[position].message    = copy;
   mMessageLines[position].messageTag = newMessageTag;

   // Notify of insert
   spectatorMessage(LineInserted, position);
}


//--------------------------------------------------------------------------
void MessageVector::deleteLine(const U32 position)
{
   AssertFatal(position >= 0 && position < mMessageLines.size(), "MessageVector::deleteLine: out of range position!");

   char* pDelete = const_cast<char*>(mMessageLines[position].message);
   delete [] pDelete;
   mMessageLines[position].message    = NULL;
   mMessageLines[position].messageTag = 0xFFFFFFFF;

   mMessageLines.erase(position);

   // Notify of delete
   spectatorMessage(LineDeleted, position);
}


//--------------------------------------------------------------------------
bool MessageVector::dump( const char* filename, const char* header )
{
   Con::printf( "Dumping message vector %s to %s...", getName(), filename );
   FileObject file;
   if ( !file.openForWrite( filename ) )
      return( false );

   // If passed a header line, write it out first:
   if ( header )
      file.writeLine( (const U8*) header );

   // First write out the record count:
   char* lineBuf = (char*) dMalloc( 10 );
   dSprintf( lineBuf, 10, "%d", mMessageLines.size() );
   file.writeLine( (const U8*) lineBuf );

   // Write all of the lines of the message vector:
   U32 len;
   for ( U32 i = 0; i < mMessageLines.size(); i++ )
   {
      len = ( dStrlen( mMessageLines[i].message ) * 2 ) + 10;
      lineBuf = (char*) dRealloc( lineBuf, len );
      dSprintf( lineBuf, len, "%d ", mMessageLines[i].messageTag );
      expandEscape( lineBuf + dStrlen( lineBuf ), mMessageLines[i].message );
      file.writeLine( (const U8*) lineBuf );
   }

   file.close();
   return( true );
}


//--------------------------------------------------------------------------
void MessageVector::registerSpectator(SpectatorCallback callBack, void *spectatorKey)
{
   AssertFatal(callBack != NULL, "Error, must have a callback!");

   // First, make sure that this hasn't been registered already...
   U32 i;
   for (i = 0; i < mSpectators.size(); i++) {
      AssertFatal(mSpectators[i].key != spectatorKey, "Error, spectator key already registered!");
      if (mSpectators[i].key == spectatorKey)
         return;
   }

   mSpectators.increment();
   SpectatorRef& lastSpectatorRef = mSpectators.last();
   lastSpectatorRef.callback = callBack;
   lastSpectatorRef.key = spectatorKey;

   // Need to message this spectator of all the lines currently inserted...
   for (i = 0; i < mMessageLines.size(); i++) {
      (*lastSpectatorRef.callback)(lastSpectatorRef.key,
                                     LineInserted, i);
   }
}

void MessageVector::unregisterSpectator(void * spectatorKey)
{
   for (U32 i = 0; i < mSpectators.size(); i++) {
      if (mSpectators[i].key == spectatorKey) {
         // Need to message this spectator of all the lines currently inserted...
         for (S32 j = mMessageLines.size() - 1; j >= 0 ; j--) {
            (*mSpectators[i].callback)(mSpectators[i].key,
                                       LineDeleted, j);
         }

         mSpectators.erase(i);
         return;
      }
   }

   AssertFatal(false, "MessageVector::unregisterSpectator: tried to unregister a spectator that isn't subscribed!");
   Con::errorf(ConsoleLogEntry::General,
               "MessageVector::unregisterSpectator: tried to unregister a spectator that isn't subscribed!");
}

void MessageVector::spectatorMessage(MessageCode code, const U32 arg)
{
   for (U32 i = 0; i < mSpectators.size(); i++) {
      (*mSpectators[i].callback)(mSpectators[i].key,
                                 code, arg);
   }
}

