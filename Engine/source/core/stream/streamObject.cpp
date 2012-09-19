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

#include "core/stream/streamObject.h"
#include "console/engineAPI.h"

//-----------------------------------------------------------------------------
// Constructor/Destructor
//-----------------------------------------------------------------------------

// This entire class has some major issues. Lots of bad characters when reading and
// writing. Stream doesn't seem to jump correctly. All in all this either needs
// to be deprecated, fixed, or someone explain to me what I'm doing wrong.
// Hard to document when it's not working. -Mich 7/16/2010

StreamObject::StreamObject()
{
   mStream = NULL;
}

StreamObject::StreamObject(Stream *stream)
{
   mStream = stream;
}

StreamObject::~StreamObject()
{
}

IMPLEMENT_CONOBJECT(StreamObject);

ConsoleDocClass( StreamObject,
   "@brief Base class for working with streams.\n\n"

   "You do not instantiate a StreamObject directly.  Instead, it is used as part of a FileStreamObject and ZipObject to "
   "support working with uncompressed and compressed files respectively."

   "@tsexample\n"
   "// You cannot actually declare a StreamObject\n"
   "// Instead, use the derived class \"FileStreamObject\"\n"
   "%fsObject = FileStreamObject();\n"
   "@endtsexample\n\n"

   "@see FileStreamObject for the derived class usable in script.\n"
   "@see ZipObject where StreamObject is used to read and write to files within a zip archive.\n"

   "@ingroup FileSystem\n\n"
);
//-----------------------------------------------------------------------------

bool StreamObject::onAdd()
{
   if(mStream == NULL)
   {
      Con::errorf("StreamObject::onAdd - StreamObject can not be instantiated from script.");
      return false;
   }
   return Parent::onAdd();
}

//-----------------------------------------------------------------------------
// Public Methods
//-----------------------------------------------------------------------------

const char * StreamObject::getStatus()
{
   if(mStream == NULL)
      return "";

   switch(mStream->getStatus())
   {
      case Stream::Ok:
         return "Ok";
      case Stream::IOError:
         return "IOError";
      case Stream::EOS:
         return "EOS";
      case Stream::IllegalCall:
         return "IllegalCall";
      case Stream::Closed:
         return "Closed";
      case Stream::UnknownError:
         return "UnknownError";

      default:
         return "Invalid";
   }
}

//-----------------------------------------------------------------------------

const char * StreamObject::readLine()
{
   if(mStream == NULL)
      return NULL;

   char *buffer = Con::getReturnBuffer(256);
   mStream->readLine((U8 *)buffer, 256);
   return buffer;
}

const char * StreamObject::readString()
{
   if(mStream == NULL)
      return NULL;

   char *buffer = Con::getReturnBuffer(256);
   mStream->readString(buffer);
   return buffer;
}

const char *StreamObject::readLongString(U32 maxStringLen)
{
   if(mStream == NULL)
      return NULL;

   char *buffer = Con::getReturnBuffer(maxStringLen + 1);
   mStream->readLongString(maxStringLen, buffer);
   return buffer;
}

//-----------------------------------------------------------------------------
// Console Methods
//-----------------------------------------------------------------------------

DefineEngineMethod( StreamObject, getStatus, const char*, (),,
   "@brief Gets a printable string form of the stream's status\n\n"
   
   "@tsexample\n"
   "// Create a file stream object for reading\n"
   "%fsObject = new FileStreamObject();\n\n"
   "// Open a file for reading\n"
   "%fsObject.open(\"./test.txt\", \"read\");\n\n"
   "// Get the status and print it\n"
   "%status = %fsObject.getStatus();\n"
   "echo(%status);\n\n"
   "// Always remember to close a file stream when finished\n"
   "%fsObject.close();\n"
   "@endtsexample\n\n"

   "@return String containing status constant, one of the following:\n\n"

   "	OK - Stream is active and no file errors\n\n"

   "	IOError - Something went wrong during read or writing the stream\n\n"

   "	EOS - End of Stream reached (mostly for reads)\n\n"

   "	IllegalCall - An unsupported operation used.  Always w/ accompanied by AssertWarn\n\n"

   "  Closed - Tried to operate on a closed stream (or detached filter)\n\n"

   "	UnknownError - Catch all for an error of some kind\n\n"

   "	Invalid - Entire stream is invalid\n\n")
{
	return object->getStatus();
}

DefineEngineMethod( StreamObject, isEOS, bool, (),,
   "@brief Tests if the stream has reached the end of the file\n\n"
   
   "This is an alternative name for isEOF. Both functions are interchangeable. This simply exists "
   "for those familiar with some C++ file I/O standards.\n\n"

   "@tsexample\n"
   "// Create a file stream object for reading\n"
   "%fsObject = new FileStreamObject();\n\n"
   "// Open a file for reading\n"
   "%fsObject.open(\"./test.txt\", \"read\");\n\n"
   "// Keep reading until we reach the end of the file\n"
   "while( !%fsObject.isEOS() )\n"
   "{\n"
   "   %line = %fsObject.readLine();\n"
   "   echo(%line);\n"
   "}\n"
   "// Made it to the end\n"
   "echo(\"Finished reading file\");\n\n"
   "// Always remember to close a file stream when finished\n"
   "%fsObject.close();\n"
   "@endtsexample\n\n"

   "@return True if the parser has reached the end of the file, false otherwise\n"

   "@see isEOF()")
{
	return object->isEOS();
}

DefineEngineMethod( StreamObject, isEOF, bool, (),,
   "@brief Tests if the stream has reached the end of the file\n\n"
   
   "This is an alternative name for isEOS. Both functions are interchangeable. This simply exists "
   "for those familiar with some C++ file I/O standards.\n\n"
   
   "@tsexample\n"
   "// Create a file stream object for reading\n"
   "%fsObject = new FileStreamObject();\n\n"
   "// Open a file for reading\n"
   "%fsObject.open(\"./test.txt\", \"read\");\n\n"
   "// Keep reading until we reach the end of the file\n"
   "while( !%fsObject.isEOF() )\n"
   "{\n"
   "   %line = %fsObject.readLine();\n"
   "   echo(%line);\n"
   "}\n"
   "// Made it to the end\n"
   "echo(\"Finished reading file\");\n\n"
   "// Always remember to close a file stream when finished\n"
   "%fsObject.close();\n"
   "@endtsexample\n\n"

   "@return True if the parser has reached the end of the file, false otherwise\n"
   
   "@see isEOS()")
{
	return object->isEOS();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( StreamObject, getPosition, S32, (),,
   "@brief Gets the position in the stream\n\n"
   
   "The easiest way to visualize this is to think of a cursor in a text file. If you have moved the cursor by "
   "five characters, the current position is 5. If you move ahead 10 more characters, the position is now 15. "
   "For StreamObject, when you read in the line the position is increased by the number of characters parsed, "
   "the null terminator, and a newline.\n\n"
   
   "@tsexample\n"
   "// Create a file stream object for reading\n"
   "%fsObject = new FileStreamObject();\n\n"
   "// Open a file for reading\n"
   "// This file contains two lines of text repeated:\n"
   "// Hello World\n"
   "// Hello World\n"
   "%fsObject.open(\"./test.txt\", \"read\");\n\n"
   "// Read in the first line\n"
   "%line = %fsObject.readLine();\n\n"
   "// Get the position of the stream\n"
   "%position = %fsObject.getPosition();\n\n"
   "// Print the current position\n"
   "// Should be 13, 10 for the words, 1 for the space, 1 for the null terminator, and 1 for the newline\n"
   "echo(%position);\n\n"
   "// Always remember to close a file stream when finished\n"
   "%fsObject.close();\n"
   "@endtsexample\n\n"

   "@return Number of bytes which stream has parsed so far, null terminators and newlines are included\n"
   
   "@see setPosition()")
{
	return object->getPosition();
}

DefineEngineMethod( StreamObject, setPosition, bool, (S32 newPosition),,
   "@brief Gets the position in the stream\n\n"
   
   "The easiest way to visualize this is to think of a cursor in a text file. If you have moved the cursor by "
   "five characters, the current position is 5. If you move ahead 10 more characters, the position is now 15. "
   "For StreamObject, when you read in the line the position is increased by the number of characters parsed, "
   "the null terminator, and a newline. Using setPosition allows you to skip to specific points of the file.\n\n"
   
   "@tsexample\n"
   "// Create a file stream object for reading\n"
   "%fsObject = new FileStreamObject();\n\n"
   "// Open a file for reading\n"
   "// This file contains the following two lines:\n"
   "// 11111111111\n"
   "// Hello World\n"
   "%fsObject.open(\"./test.txt\", \"read\");\n\n"
   "// Skip ahead by 12, which will bypass the first line entirely\n"
   "%fsObject.setPosition(12);\n\n"
   "// Read in the next line\n"
   "%line = %fsObject.readLine();\n\n"
   "// Print the line just read in, should be \"Hello World\"\n"
   "echo(%line);\n\n"
   "// Always remember to close a file stream when finished\n"
   "%fsObject.close();\n"
   "@endtsexample\n\n"

   "@return Number of bytes which stream has parsed so far, null terminators and newlines are included\n"
   
   "@see getPosition()")
{
	return object->setPosition(newPosition);
}

DefineEngineMethod( StreamObject, getStreamSize, S32, (),,
   "@brief Gets the size of the stream\n\n"
   
   "The size is dependent on the type of stream being used. If it is a file stream, returned value will "
   "be the size of the file. If it is a memory stream, it will be the size of the allocated buffer.\n\n"
      
   "@tsexample\n"
   "// Create a file stream object for reading\n"
   "%fsObject = new FileStreamObject();\n\n"
   "// Open a file for reading\n"
   "// This file contains the following two lines:\n"
   "// HelloWorld\n"
   "// HelloWorld\n"
   "%fsObject.open(\"./test.txt\", \"read\");\n\n"
   "// Found out how large the file stream is\n"
   "// Then print it to the console\n"
   "// Should be 22\n"
   "%streamSize = %fsObject.getStreamSize();\n"
   "echo(%streamSize);\n\n"
   "// Always remember to close a file stream when finished\n"
   "%fsObject.close();\n"
   "@endtsexample\n\n"

   "@return Size of stream, in bytes\n")
{
	return object->getStreamSize();
}

//-----------------------------------------------------------------------------
DefineEngineMethod( StreamObject, readLine, const char*, (),,
   "@brief Read a line from the stream.\n\n"
   
   "Emphasis on *line*, as in you cannot parse individual characters or chunks of data. "
   "There is no limitation as to what kind of data you can read.\n\n"
   
   "@tsexample\n"
   "// Create a file stream object for reading\n"
   "// This file contains the following two lines:\n"
   "// HelloWorld\n"
   "// HelloWorld\n"
   "%fsObject = new FileStreamObject();\n\n"
   "%fsObject.open(\"./test.txt\", \"read\");\n\n"
   "// Read in the first line\n"
   "%line = %fsObject.readLine();\n\n"
   "// Print the line we just read\n"
   "echo(%line);\n\n"
   "// Always remember to close a file stream when finished\n"
   "%fsObject.close();\n"
   "@endtsexample\n\n"

   "@return String containing the line of data that was just read\n"
   
   "@see writeLine()")
{
	const char *str = object->readLine();
	return str ? str : "";
}

DefineEngineMethod( StreamObject, writeLine, void, ( const char* line ),,
   "@brief Write a line to the stream, if it was opened for writing.\n\n"
   
   "There is no limit as to what kind of data you can write. Any format and data is allowable, not just text. "
   "Be careful of what you write, as whitespace, current values, and literals will be preserved.\n\n"

   "@param line The data we are writing out to file."
   
   "@tsexample\n"
   "// Create a file stream\n"
   "%fsObject = new FileStreamObject();\n\n"
   "// Open the file for writing\n"
   "// If it does not exist, it is created. If it does exist, the file is cleared\n"
   "%fsObject.open(\"./test.txt\", \"write\");\n\n"
   "// Write a line to the file\n"
   "%fsObject.writeLine(\"Hello World\");\n\n"
   "// Write another line to the file\n"
   "%fsObject.writeLine(\"Documentation Rocks!\");\n\n"
   "// Always remember to close a file stream when finished\n"
   "%fsObject.close();\n"
   "@endtsexample\n\n"
   
   "@see readLine()")
{
	object->writeLine((U8 *)line);
}

//-----------------------------------------------------------------------------
/* readSTString, readString, and readLongString need to be fixed or deprecated */
DefineEngineMethod(StreamObject, readSTString, String, ( bool caseSensitive ), ( false ),
   "@brief Read in a string and place it on the string table.\n\n"
   "@param caseSensitive If false then case will not be taken into account when attempting "
   "to match the read in string with what is already in the string table.\n"
   "@return The string that was read from the stream.\n"
   "@see writeString()"
   
   "@note When working with these particular string reading and writing methods, the stream "
   "begins with the length of the string followed by the string itself, and does not include "
   "a NULL terminator.")
{
   const char *str = object->readSTString(caseSensitive);
   return str ? str : "";
}

DefineEngineMethod(StreamObject, readString, String, (),,
   "@brief Read a string up to a maximum of 256 characters"
   "@return The string that was read from the stream.\n"
   "@see writeString()"
   
   "@note When working with these particular string reading and writing methods, the stream "
   "begins with the length of the string followed by the string itself, and does not include "
   "a NULL terminator.")
{
   const char *str = object->readString();
   return str ? str : "";
}

DefineEngineMethod(StreamObject, readLongString, String, ( S32 maxLength ),,
   "@brief Read in a string up to the given maximum number of characters.\n\n"
   "@param maxLength The maximum number of characters to read in.\n"
   "@return The string that was read from the stream.\n"
   "@see writeLongString()"
   
   "@note When working with these particular string reading and writing methods, the stream "
   "begins with the length of the string followed by the string itself, and does not include "
   "a NULL terminator.")
{
   const char *str = object->readLongString(maxLength);
   return str ? str : "";
}

DefineEngineMethod(StreamObject, writeLongString, void, ( S32 maxLength, const char* string ),,
   "@brief Write out a string up to the maximum number of characters.\n\n"
   "@param maxLength The maximum number of characters that will be written.\n"
   "@param string The string to write out to the stream.\n"
   "@see readLongString()"
   
   "@note When working with these particular string reading and writing methods, the stream "
   "begins with the length of the string followed by the string itself, and does not include "
   "a NULL terminator.")
{
   object->writeLongString(maxLength, string);
}

DefineEngineMethod(StreamObject, writeString, void, ( const char* string, S32 maxLength ), ( 256 ),
   "@brief Write out a string with a default maximum length of 256 characters.\n\n"
   "@param string The string to write out to the stream\n"
   "@param maxLength The maximum string length to write out with a default of 256 characters.  This "
   "value should not be larger than 256 as it is written to the stream as a single byte.\n"
   "@see readString()"
   
   "@note When working with these particular string reading and writing methods, the stream "
   "begins with the length of the string followed by the string itself, and does not include "
   "a NULL terminator.")
{
   object->writeString(string, maxLength);
}

//-----------------------------------------------------------------------------

DefineEngineMethod(StreamObject, copyFrom, bool, (SimObject* other),,
   "@brief Copy from another StreamObject into this StreamObject\n\n"
   "@param other The StreamObject to copy from.\n"
   "@return True if the copy was successful.\n")
{
   StreamObject *so = dynamic_cast<StreamObject *>(other);
   if(so == NULL)
      return false;

   return object->copyFrom(so);
}
