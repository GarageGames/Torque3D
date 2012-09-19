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

#include "core/fileObject.h"
#include "console/engineAPI.h"

IMPLEMENT_CONOBJECT(FileObject);

ConsoleDocClass( FileObject,
   "@brief This class is responsible opening, reading, creating, and saving file contents.\n\n"

   "FileObject acts as the interface with OS level files.  You create a new FileObject and pass into it "
   "a file's path and name.  The FileObject class supports three distinct operations for working with files:\n\n"

   "<table border='1' cellpadding='1'>"
   "<tr><th>Operation</th><th>%FileObject Method</th><th>Description</th></tr>"
   "<tr><td>Read</td><td>openForRead()</td><td>Open the file for reading</td></tr>"
   "<tr><td>Write</td><td>openForWrite()</td><td>Open the file for writing to and replace its contents (if any)</td></tr>"
   "<tr><td>Append</td><td>openForAppend()</td><td>Open the file and start writing at its end</td></tr>"
   "</table>\n\n"

   "Before you may work with a file you need to use one of the three above methods on the FileObject.\n\n"

   "@tsexample\n"
   "// Create a file object for writing\n"
   "%fileWrite = new FileObject();\n\n"
   "// Open a file to write to, if it does not exist it will be created\n"
   "%result = %fileWrite.OpenForWrite(\"./test.txt\");\n\n"
   "if ( %result )\n"
   "{\n"
   "   // Write a line to the text files\n"
   "   %fileWrite.writeLine(\"READ. READ CODE. CODE\");\n"
   "}\n\n"
   "// Close the file when finished\n"
   "%fileWrite.close();\n\n"
   "// Cleanup the file object\n"
   "%fileWrite.delete();\n\n\n"

   "// Create a file object for reading\n"
   "%fileRead = new FileObject();\n\n"
   "// Open a text file, if it exists\n"
   "%result = %fileRead.OpenForRead(\"./test.txt\");\n\n"
   "if ( %result )\n"
   "{\n"
   "   // Read in the first line\n"
   "   %line = %fileRead.readline();\n\n"
   "   // Print the line we just read\n"
   "   echo(%line);\n"
   "}\n\n"
   "// Close the file when finished\n"
   "%fileRead.close();\n\n"
   "// Cleanup the file object\n"
   "%fileRead.delete();\n"
   "@endtsexample\n\n"

   "@ingroup FileSystem\n"
);

bool FileObject::isEOF()
{
   return mCurPos == mBufferSize;
}

FileObject::FileObject()
{
   mFileBuffer = NULL;
   mBufferSize = 0;
   mCurPos = 0;
   stream = NULL;
}

FileObject::~FileObject()
{
   SAFE_DELETE_ARRAY(mFileBuffer);
   SAFE_DELETE(stream);
}

void FileObject::close()
{
   SAFE_DELETE(stream);
   SAFE_DELETE_ARRAY(mFileBuffer);
   mFileBuffer = NULL;
   mBufferSize = mCurPos = 0;
}

bool FileObject::openForWrite(const char *fileName, const bool append)
{
   char buffer[1024];
   Con::expandScriptFilename( buffer, sizeof( buffer ), fileName );

   close();
   
   if( !buffer[ 0 ] )
      return false;

   if((stream = FileStream::createAndOpen( fileName, append ? Torque::FS::File::WriteAppend : Torque::FS::File::Write )) == NULL)
      return false;

   stream->setPosition( stream->getStreamSize() );
   return( true );
}

bool FileObject::openForRead(const char* /*fileName*/)
{
   AssertFatal(false, "Error, not yet implemented!");
   return false;
}

bool FileObject::readMemory(const char *fileName)
{
   char buffer[1024];
   Con::expandScriptFilename( buffer, sizeof( buffer ), fileName );

   close();

   void *data = NULL;
   U32 dataSize = 0;
   Torque::FS::ReadFile(buffer, data, dataSize, true);
   if(data == NULL)
      return false;

   mBufferSize = dataSize;
   mFileBuffer = (U8 *)data;
   mCurPos = 0;

   return true;
}

const U8 *FileObject::readLine()
{
   if(!mFileBuffer)
      return (U8 *) "";

   U32 tokPos = mCurPos;

   for(;;)
   {
      if(mCurPos == mBufferSize)
         break;

      if(mFileBuffer[mCurPos] == '\r')
      {
         mFileBuffer[mCurPos++] = 0;
         if(mFileBuffer[mCurPos] == '\n')
            mCurPos++;
         break;
      }

      if(mFileBuffer[mCurPos] == '\n')
      {
         mFileBuffer[mCurPos++] = 0;
         break;
      }

      mCurPos++;
   }

   return mFileBuffer + tokPos;
}

void FileObject::peekLine( U8* line, S32 length )
{
   if(!mFileBuffer)
   {
      line[0] = '\0';
      return;
   }

   // Copy the line into the buffer. We can't do this like readLine because
   // we can't modify the file buffer.
   S32 i = 0;
   U32 tokPos = mCurPos;
   while( ( tokPos != mBufferSize ) && ( mFileBuffer[tokPos] != '\r' ) && ( mFileBuffer[tokPos] != '\n' ) && ( i < ( length - 1 ) ) )
      line[i++] = mFileBuffer[tokPos++];

   line[i++] = '\0';

   //if( i == length )
      //Con::warnf( "FileObject::peekLine - The line contents could not fit in the buffer (size %d). Truncating.", length );
}

void FileObject::writeLine(const U8 *line)
{
   stream->write(dStrlen((const char *) line), line);
   stream->write(2, "\r\n");
}

void FileObject::writeObject( SimObject* object, const U8* objectPrepend )
{
   if( objectPrepend == NULL )
      stream->write(2, "\r\n");
   else
      stream->write(dStrlen((const char *) objectPrepend), objectPrepend );
   object->write( *stream, 0 );
}

DefineEngineMethod( FileObject, openForRead, bool, ( const char* filename ),,
   "@brief Open a specified file for reading\n\n"
   
   "There is no limit as to what kind of file you can read. Any format and data contained within is accessible, not just text\n\n"

   "@param filename Path, name, and extension of file to be read"
   
   "@tsexample\n"
   "// Create a file object for reading\n"
   "%fileRead = new FileObject();\n\n"
   "// Open a text file, if it exists\n"
   "%result = %fileRead.OpenForRead(\"./test.txt\");\n"
   "@endtsexample\n\n"

   "@return True if file was successfully opened, false otherwise\n")
{
	return object->readMemory(filename);
}

DefineEngineMethod( FileObject, openForWrite, bool, ( const char* filename ),,
   "@brief Open a specified file for writing\n\n"
   
   "There is no limit as to what kind of file you can write. Any format and data is allowable, not just text\n\n"

   "@param filename Path, name, and extension of file to write to"
   
   "@tsexample\n"
   "// Create a file object for writing\n"
   "%fileWrite = new FileObject();\n\n"
   "// Open a file to write to, if it does not exist it will be created\n"
   "%result = %fileWrite.OpenForWrite(\"./test.txt\");\n"
   "@endtsexample\n\n"

   "@return True if file was successfully opened, false otherwise\n")
{
	return object->openForWrite(filename);
}

DefineEngineMethod( FileObject, openForAppend, bool, ( const char* filename ),,
   "@brief Open a specified file for writing, adding data to the end of the file\n\n"
   
   "There is no limit as to what kind of file you can write. Any format and data is allowable, not just text. Unlike openForWrite(), "
   "which will erase an existing file if it is opened, openForAppend() preserves data in an existing file and adds to it.\n\n"

   "@param filename Path, name, and extension of file to append to"
   
   "@tsexample\n"
   "// Create a file object for writing\n"
   "%fileWrite = new FileObject();\n\n"
   "// Open a file to write to, if it does not exist it will be created\n"
   "// If it does exist, whatever we write will be added to the end\n"
   "%result = %fileWrite.OpenForAppend(\"./test.txt\");\n"
   "@endtsexample\n\n"

   "@return True if file was successfully opened, false otherwise\n")
{
	return object->openForWrite(filename, true);
}

DefineEngineMethod( FileObject, isEOF, bool, (),,
   "@brief Determines if the parser for this FileObject has reached the end of the file\n\n"
   
   "@tsexample\n"
   "// Create a file object for reading\n"
   "%fileRead = new FileObject();\n\n"
   "// Open a text file, if it exists\n"
   "%fileRead.OpenForRead(\"./test.txt\");\n\n"
   "// Keep reading until we reach the end of the file\n"
   "while( !%fileRead.isEOF() )\n"
   "{\n"
   "   %line = %fileRead.readline();\n"
   "   echo(%line);\n"
   "}\n\n"
   "// Made it to the end\n"
   "echo(\"Finished reading file\");\n"
   "@endtsexample\n\n"

   "@return True if the parser has reached the end of the file, false otherwise\n")
{
	return object->isEOF();
}

DefineEngineMethod( FileObject, readLine, const char*, (),,
   "@brief Read a line from file.\n\n"
   
   "Emphasis on *line*, as in you cannot parse individual characters or chunks of data.  "
   "There is no limitation as to what kind of data you can read.\n\n"
   
   "@tsexample\n"
   "// Create a file object for reading\n"
   "%fileRead = new FileObject();\n\n"
   "// Open a text file, if it exists\n"
   "%fileRead.OpenForRead(\"./test.txt\");\n\n"
   "// Read in the first line\n"
   "%line = %fileRead.readline();\n\n"
   "// Print the line we just read\n"
   "echo(%line);\n"
   "@endtsexample\n\n"

   "@return String containing the line of data that was just read\n")
{
	return (const char *) object->readLine();
}

DefineEngineMethod( FileObject, peekLine, const char*, (),,
   "@brief Read a line from the file without moving the stream position.\n\n"
   
   "Emphasis on *line*, as in you cannot parse individual characters or chunks of data.  "
   "There is no limitation as to what kind of data you can read. Unlike readLine, the parser does not move forward after reading.\n\n"

   "@param filename Path, name, and extension of file to be read"
   
   "@tsexample\n"
   "// Create a file object for reading\n"
   "%fileRead = new FileObject();\n\n"
   "// Open a text file, if it exists\n"
   "%fileRead.OpenForRead(\"./test.txt\");\n\n"
   "// Peek the first line\n"
   "%line = %fileRead.peekLine();\n\n"
   "// Print the line we just peeked\n"
   "echo(%line);\n"
   "// If we peek again...\n"
   "%line = %fileRead.peekLine();\n\n"
   "// We will get the same output as the first time\n"
   "// since the stream did not move forward\n"
   "echo(%line);\n"
   "@endtsexample\n\n"

   "@return String containing the line of data that was just peeked\n")
{
	char *line = Con::getReturnBuffer( 512 );
	object->peekLine( (U8*)line, 512 );
	return line;
}

DefineEngineMethod( FileObject, writeLine, void, ( const char* text ),,
   "@brief Write a line to the file, if it was opened for writing.\n\n"
   
   "There is no limit as to what kind of text you can write. Any format and data is allowable, not just text. "
   "Be careful of what you write, as whitespace, current values, and literals will be preserved.\n\n"

   "@param text The data we are writing out to file."
   
   "@tsexample\n"
   "// Create a file object for writing\n"
   "%fileWrite = new FileObject();\n\n"
   "// Open a file to write to, if it does not exist it will be created\n"
   "%fileWrite.OpenForWrite(\"./test.txt\");\n\n"
   "// Write a line to the text files\n"
   "%fileWrite.writeLine(\"READ. READ CODE. CODE\");\n\n"
   "@endtsexample\n\n"

   "@return True if file was successfully opened, false otherwise\n")
{
	object->writeLine((const U8 *) text);
}

DefineEngineMethod( FileObject, close, void, (),,
   "@brief Close the file.\n\n"
   
   "It is EXTREMELY important that you call this function when you are finished reading or writing to a file. "
   "Failing to do so is not only a bad programming practice, but could result in bad data or corrupt files. "
   "Remember: Open, Read/Write, Close, Delete...in that order!\n\n"
   
   "@tsexample\n"
   "// Create a file object for reading\n"
   "%fileRead = new FileObject();\n\n"
   "// Open a text file, if it exists\n"
   "%fileRead.OpenForRead(\"./test.txt\");\n\n"
   "// Peek the first line\n"
   "%line = %fileRead.peekLine();\n\n"
   "// Print the line we just peeked\n"
   "echo(%line);\n"
   "// If we peek again...\n"
   "%line = %fileRead.peekLine();\n\n"
   "// We will get the same output as the first time\n"
   "// since the stream did not move forward\n"
   "echo(%line);\n\n"
   "// Close the file when finished\n"
   "%fileWrite.close();\n\n"
   "// Cleanup the file object\n"
   "%fileWrite.delete();\n"
   "@endtsexample\n\n")
{
	object->close();
}

static ConsoleDocFragment _FileObjectwriteObject1(
   "@brief Write an object to a text file.\n\n"
   
   "Unlike a simple writeLine using specified strings, this function writes an entire object "
   "to file, preserving its type, name, and properties. This is similar to the save() functionality of "
   "the SimObject class, but with a bit more control.\n\n"
   
   "@param object The SimObject being written to file, properties, name, and all.\n"
   
   "@tsexample\n"
   "// Let's assume this SpawnSphere was created and currently\n"
   "// exists in the running level\n"
   "new SpawnSphere(TestSphere)\n"
   "{\n"
   "   spawnClass = \"Player\";\n"
   "   spawnDatablock = \"DefaultPlayerData\";\n"
   "   autoSpawn = \"1\";\n"
   "   radius = \"5\";\n"
   "   sphereWeight = \"1\";\n"
   "   indoorWeight = \"1\";\n"
   "   outdoorWeight = \"1\";\n"
   "   dataBlock = \"SpawnSphereMarker\";\n"
   "   position = \"-42.222 1.4845 4.80334\";\n"
   "   rotation = \"0 0 -1 108\";\n"
   "   scale = \"1 1 1\";\n"
   "   canSaveDynamicFields = \"1\";\n"
   "};\n\n"
   "// Create a file object for writing\n"
   "%fileWrite = new FileObject();\n\n"
   "// Open a file to write to, if it does not exist it will be created\n"
   "%fileWrite.OpenForWrite(\"./spawnSphers.txt\");\n\n"
   "// Write out the TestSphere\n"
   "%fileWrite.writeObject(TestSphere);\n\n"
   "// Close the text file\n"
   "%fileWrite.close();\n\n"   
   "// Cleanup\n"
   "%fileWrite.delete();\n"
   "@endtsexample\n\n\n",
   "FileObject",
   "void writeObject( SimObject* object);");

static ConsoleDocFragment _FileObjectwriteObject2(
   "@brief Write an object to a text file, with some data added first.\n\n"
   
   "Unlike a simple writeLine using specified strings, this function writes an entire object "
   "to file, preserving its type, name, and properties. This is similar to the save() functionality of "
   "the SimObject class, but with a bit more control.\n\n"
   
   "@param object The SimObject being written to file, properties, name, and all.\n"
   "@param prepend Data or text that is written out before the SimObject.\n\n"
   
   "@tsexample\n"
   "// Let's assume this SpawnSphere was created and currently\n"
   "// exists in the running level\n"
   "new SpawnSphere(TestSphere)\n"
   "{\n"
   "   spawnClass = \"Player\";\n"
   "   spawnDatablock = \"DefaultPlayerData\";\n"
   "   autoSpawn = \"1\";\n"
   "   radius = \"5\";\n"
   "   sphereWeight = \"1\";\n"
   "   indoorWeight = \"1\";\n"
   "   outdoorWeight = \"1\";\n"
   "   dataBlock = \"SpawnSphereMarker\";\n"
   "   position = \"-42.222 1.4845 4.80334\";\n"
   "   rotation = \"0 0 -1 108\";\n"
   "   scale = \"1 1 1\";\n"
   "   canSaveDynamicFields = \"1\";\n"
   "};\n\n"
   "// Create a file object for writing\n"
   "%fileWrite = new FileObject();\n\n"
   "// Open a file to write to, if it does not exist it will be created\n"
   "%fileWrite.OpenForWrite(\"./spawnSphers.txt\");\n\n"
   "// Write out the TestSphere, with a prefix\n"
   "%fileWrite.writeObject(TestSphere, \"$mySphere = \");\n\n"
   "// Close the text file\n"
   "%fileWrite.close();\n\n"   
   "// Cleanup\n"
   "%fileWrite.delete();\n"
   "@endtsexample\n\n\n",
   "FileObject",
   "void writeObject( SimObject* object, string prepend);");

ConsoleMethod( FileObject, writeObject, void, 3, 4, "FileObject.writeObject(SimObject, object prepend)" 
			  "@hide")
{
   SimObject* obj = Sim::findObject( argv[2] );
   if( !obj )
   {
      Con::printf("FileObject::writeObject - Invalid Object!");
      return;
   }

   char *objName = NULL;
   if( argc == 4 )
      objName = (char*)argv[3];

   object->writeObject( obj, (const U8*)objName );
}

