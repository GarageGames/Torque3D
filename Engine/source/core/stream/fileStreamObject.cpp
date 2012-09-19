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

#include "core/stream/fileStreamObject.h"
#include "console/engineAPI.h"

//-----------------------------------------------------------------------------
// Local Globals
//-----------------------------------------------------------------------------

static const struct
{
   const char *strMode;
   Torque::FS::File::AccessMode mode;
} gModeMap[]=
{
   { "read", Torque::FS::File::Read },
   { "write", Torque::FS::File::Write },
   { "readwrite", Torque::FS::File::ReadWrite },
   { "writeappend", Torque::FS::File::WriteAppend },
   { NULL, (Torque::FS::File::AccessMode)0 }
};

//-----------------------------------------------------------------------------
// Constructor/Destructor
//-----------------------------------------------------------------------------

FileStreamObject::FileStreamObject()
{
}

FileStreamObject::~FileStreamObject()
{
   close();
}

IMPLEMENT_CONOBJECT(FileStreamObject);

ConsoleDocClass( FileStreamObject,
   "@brief A wrapper around StreamObject for parsing text and data from files.\n\n"

   "FileStreamObject inherits from StreamObject and provides some unique methods for working "
   "with strings.  If you're looking for general file handling, you may want to use FileObject.\n\n"

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

   "@see StreamObject for the list of inherited functions variables\n"
   "@see FileObject for general file handling.\n"

   "@ingroup FileSystem\n"
);

//-----------------------------------------------------------------------------

bool FileStreamObject::onAdd()
{
   // [tom, 2/12/2007] Skip over StreamObject's onAdd() so that we can
   // be instantiated from script.
   return SimObject::onAdd();
}

//-----------------------------------------------------------------------------
// Public Methods
//-----------------------------------------------------------------------------

bool FileStreamObject::open(const char *filename, Torque::FS::File::AccessMode mode)
{
   close();

   if(! mFileStream.open(filename, mode))
      return false;

   mStream = &mFileStream;
   return true;
}

void FileStreamObject::close()
{
   mFileStream.close();
   mStream = NULL;
}

//-----------------------------------------------------------------------------
// Console Methods
//-----------------------------------------------------------------------------

DefineEngineMethod( FileStreamObject, open, bool, (const char* filename, const char* openMode),,
   "@brief Open a file for reading, writing, reading and writing, or appending\n\n"
   
   "Using \"Read\" for the open mode allows you to parse the contents of file, but not making modifications. \"Write\" will create a new "
   "file if it does not exist, or erase the contents of an existing file when opened. Write also allows you to modify the contents of the file.\n\n"

   "\"ReadWrite\" will provide the ability to parse data (read it in) and manipulate data (write it out) interchangeably. Keep in mind the stream can "
   "move during each operation. Finally, \"WriteAppend\" will open a file if it exists, but will not clear the contents. You can write new data starting "
   " at the end of the files existing contents.\n\n"

   "@param filename Name of file to open\n"
   "@param openMode One of \"Read\", \"Write\", \"ReadWrite\" or \"WriteAppend\"\n\n"

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

   "@return True if the file was successfully opened, false if something went wrong"
   
   "@see close()")
{
	for(S32 i = 0;gModeMap[i].strMode;++i)
   {
      if(dStricmp(gModeMap[i].strMode, openMode) == 0)
      {
         Torque::FS::File::AccessMode mode = gModeMap[i].mode;
         char buffer[1024];
         Con::expandScriptFilename(buffer, sizeof(buffer), filename);
         return object->open(buffer, mode);
      }
   }

   Con::errorf("FileStreamObject::open - Mode must be one of Read, Write, ReadWrite or WriteAppend.");
   return false;
}

DefineEngineMethod( FileStreamObject, close, void, (),,
   "@brief Close the file. You can no longer read or write to it unless you open it again.\n\n"
   
   "@tsexample\n"
   "// Create a file stream object for reading\n"
   "%fsObject = new FileStreamObject();\n\n"
   "// Open a file for reading\n"
   "%fsObject.open(\"./test.txt\", \"read\");\n\n"
   "// Always remember to close a file stream when finished\n"
   "%fsObject.close();\n"
   "@endtsexample\n\n"
   
   "@see open()")
{
	object->close();
}
