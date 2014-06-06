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

#include "core/util/zip/zipObject.h"
#include "core/util/safeDelete.h"
#include "console/engineAPI.h"

//-----------------------------------------------------------------------------
// Constructor/Destructor
//-----------------------------------------------------------------------------

ZipObject::ZipObject()
{
   mZipArchive = NULL;
}

ZipObject::~ZipObject()
{
   closeArchive();
}

IMPLEMENT_CONOBJECT(ZipObject);

ConsoleDocClass( ZipObject,
   "@brief Provides access to a zip file.\n\n"

   "A ZipObject add, delete and extract files that are within a zip archive.  You may also "
   "read and write directly to the files within the archive by obtaining a StreamObject "
   "for the file."

   "@tsexample\n"
   "// Open a zip archive, creating it if it doesn't exist\n"
   "%archive = new ZipObject();\n"
   "%archive.openArchive(\"testArchive.zip\", Write);\n\n"
   "// Add a file to the archive with the given name\n"
   "%archive.addFile(\"./water.png\", \"water.png\");\n\n"
   "// Close the archive to save the changes\n"
   "%archive.closeArchive();\n"
   "@endtsexample\n\n"

   "@note Behind the scenes all of the work is being done with the ZipArchive and StreamObject classes.\n"

   "@see StreamObject when using methods such as openFileForRead() and openFileForWrite()\n\n"

   "@ingroup FileSystem\n"
);


//-----------------------------------------------------------------------------
// Protected Methods
//-----------------------------------------------------------------------------

StreamObject *ZipObject::createStreamObject(Stream *stream)
{
   for(S32 i = 0;i < mStreamPool.size();++i)
   {
      StreamObject *so = mStreamPool[i];

      if(so == NULL)
      {
         // Reuse any free locations in the pool
         so = new StreamObject(stream);
         so->registerObject();
         mStreamPool[i] = so;
         return so;
      }
      
      if(so->getStream() == NULL)
      {
         // Existing unused stream, update it and return it
         so->setStream(stream);
         return so;
      }
   }

   // No free object found, create a new one
   StreamObject *so = new StreamObject(stream);
   so->registerObject();
   mStreamPool.push_back(so);

   return so;
}

//-----------------------------------------------------------------------------
// Public Methods
//-----------------------------------------------------------------------------

bool ZipObject::openArchive(const char *filename, Zip::ZipArchive::AccessMode mode /* = Read */)
{
   closeArchive();

   mZipArchive = new Zip::ZipArchive;
   if(mZipArchive->openArchive(filename, mode))
      return true;

   SAFE_DELETE(mZipArchive);
   return false;
}

void ZipObject::closeArchive()
{
   if(mZipArchive == NULL)
      return;

   for(S32 i = 0;i < mStreamPool.size();++i)
   {
      StreamObject *so = mStreamPool[i];
      if(so && so->getStream() != NULL)
         closeFile(so);
      
      SAFE_DELETE_OBJECT(mStreamPool[i]);
   }
   mStreamPool.clear();

   mZipArchive->closeArchive();
   SAFE_DELETE(mZipArchive);
}

//-----------------------------------------------------------------------------

StreamObject * ZipObject::openFileForRead(const char *filename)
{
   if(mZipArchive == NULL)
      return NULL;

   Stream * stream = mZipArchive->openFile(filename, Zip::ZipArchive::Read);
   if(stream != NULL)
      return createStreamObject(stream);

   return NULL;
}

StreamObject * ZipObject::openFileForWrite(const char *filename)
{
   if(mZipArchive == NULL)
      return NULL;

   Stream * stream = mZipArchive->openFile(filename, Zip::ZipArchive::Write);
   if(stream != NULL)
      return createStreamObject(stream);

   return NULL;
}

void ZipObject::closeFile(StreamObject *stream)
{
   if(mZipArchive == NULL)
      return;

#ifdef TORQUE_DEBUG
   bool found = false;
   for(S32 i = 0;i < mStreamPool.size();++i)
   {
      StreamObject *so = mStreamPool[i];
      if(so && so == stream)
      {
         found = true;
         break;
      }
   }

   AssertFatal(found, "ZipObject::closeFile() - Attempting to close stream not opened by this ZipObject");
#endif

   mZipArchive->closeFile(stream->getStream());
   stream->setStream(NULL);
}

//-----------------------------------------------------------------------------

bool ZipObject::addFile(const char *filename, const char *pathInZip, bool replace /* = true */)
{
   return mZipArchive->addFile(filename, pathInZip, replace);
}

bool ZipObject::extractFile(const char *pathInZip, const char *filename)
{
   return mZipArchive->extractFile(pathInZip, filename);
}

bool ZipObject::deleteFile(const char *filename)
{
   return mZipArchive->deleteFile(filename);
}

//-----------------------------------------------------------------------------

S32 ZipObject::getFileEntryCount()
{
   return mZipArchive ? mZipArchive->numEntries() : 0;
}

String ZipObject::getFileEntry(S32 idx)
{
   if(mZipArchive == NULL)
      return "";

   const Zip::CentralDir &dir = (*mZipArchive)[idx];
   char buffer[1024];
   S32 chars = dSprintf(buffer, sizeof(buffer), "%s\t%d\t%d\t%d\t%08x",
            dir.mFilename.c_str(), dir.mUncompressedSize, dir.mCompressedSize,
            dir.mCompressMethod, dir.mCRC32);
   if (chars < sizeof(buffer))
      buffer[chars] = 0;

   return String(buffer);
}

//-----------------------------------------------------------------------------
// Console Methods
//-----------------------------------------------------------------------------

static const struct
{
   const char *strMode;
   Zip::ZipArchive::AccessMode mode;
} gModeMap[]=
{
   { "read", Zip::ZipArchive::Read },
   { "write", Zip::ZipArchive::Write },
   { "readwrite", Zip::ZipArchive::ReadWrite },
   { NULL, (Zip::ZipArchive::AccessMode)0 }
};

//-----------------------------------------------------------------------------

DefineEngineMethod(ZipObject, openArchive, bool, ( const char* filename, const char* accessMode ), ( "read" ),
   "@brief Open a zip archive for manipulation.\n\n"

   "Once a zip archive is opened use the various ZipObject methods for "
   "working with the files within the archive.  Be sure to close the archive when "
   "you are done with it.\n\n"

   "@param filename The path and file name of the zip archive to open.\n"
   "@param accessMode One of read, write or readwrite\n"

   "@return True is the archive was successfully opened.\n"
   
   "@note If you wish to make any changes to the archive, be sure to open it "
   "with a write or readwrite access mode.\n"

   "@see closeArchive()")
{
   Zip::ZipArchive::AccessMode mode = Zip::ZipArchive::Read;

   for(S32 i = 0;gModeMap[i].strMode;++i)
   {
      if(dStricmp(gModeMap[i].strMode, accessMode) == 0)
      {
         mode = gModeMap[i].mode;
         break;
      }
   }

   char buf[512];
   Con::expandScriptFilename(buf, sizeof(buf), filename);

   return object->openArchive(buf, mode);
}

DefineEngineMethod(ZipObject, closeArchive, void, (),,
   "@brief Close an already opened zip archive.\n\n"
   "@see openArchive()")
{
   object->closeArchive();
}

//-----------------------------------------------------------------------------

DefineEngineMethod(ZipObject, openFileForRead, SimObject*, ( const char* filename ),,
   "@brief Open a file within the zip archive for reading.\n\n"

   "Be sure to close the file when you are done with it.\n"

   "@param filename The path and name of the file to open within the zip archive.\n"

   "@return A standard StreamObject is returned for working with the file.\n"
   "@note You must first open the zip archive before working with files within it.\n"

   "@see closeFile()\n"
   "@see openArchive()")
{
   StreamObject *stream = object->openFileForRead(filename);
   return stream;
}

DefineEngineMethod(ZipObject, openFileForWrite, SimObject*, ( const char* filename ),,
   "@brief Open a file within the zip archive for writing to.\n\n"
   
   "Be sure to close the file when you are done with it.\n"

   "@param filename The path and name of the file to open within the zip archive.\n"

   "@return A standard StreamObject is returned for working with the file.\n"
   "@note You must first open the zip archive before working with files within it.\n"

   "@see closeFile()\n"
   "@see openArchive()")
{
   StreamObject *stream = object->openFileForWrite(filename);
   return stream;
}

DefineEngineMethod(ZipObject, closeFile, void, ( SimObject* stream ),,
   "@brief Close a previously opened file within the zip archive.\n\n"
   "@param stream The StreamObject of a previously opened file within the zip archive.\n"
   "@see openFileForRead()\n"
   "@see openFileForWrite()")
{
   StreamObject *so = dynamic_cast<StreamObject *>(stream);
   if(so == NULL)
   {
      Con::errorf("ZipObject::closeFile - Invalid stream specified");
      return;
   }

   object->closeFile(so);
}

//-----------------------------------------------------------------------------

DefineEngineMethod(ZipObject, addFile, bool, ( const char* filename, const char* pathInZip, bool replace ), ( true ),
   "@brief Add a file to the zip archive\n\n"
   
   "@param filename The path and name of the file to add to the zip archive.\n"
   "@param pathInZip The path and name to be given to the file within the zip archive.\n"
   "@param replace If a file already exists within the zip archive at the same location as this "
   "new file, this parameter indicates if it should be replaced.  By default, it will be replaced.\n"
   "@return True if the file was successfully added to the zip archive.")
{
   // Left this line commented out as it was useful when i had a problem
   //  with the zip's i was creating. [2/21/2007 justind]
   // [tom, 2/21/2007] To is now a warnf() for better visual separation in the console
  // Con::errorf("zipAdd: %s", argv[2]);
   //Con::warnf("    To: %s", argv[3]);

   return object->addFile(filename, pathInZip, replace);
}

DefineEngineMethod(ZipObject, extractFile, bool, ( const char* pathInZip, const char* filename ),,
   "@brief Extact a file from the zip archive and save it to the requested location.\n\n"
   "@param pathInZip The path and name of the file to be extracted within the zip archive.\n"
   "@param filename The path and name to give the extracted file.\n\n"
   "@return True if the file was successfully extracted.")
{
   return object->extractFile(pathInZip, filename);
}

DefineEngineMethod(ZipObject, deleteFile, bool, ( const char* pathInZip ),,
   "@brief Deleted the given file from the zip archive\n\n"
   "@param pathInZip The path and name of the file to be deleted from the zip archive.\n"
   "@return True of the file was successfully deleted.\n"

   "@note Files that have been deleted from the archive will still show up with a "
   "getFileEntryCount() until you close the archive.  If you need to have the file "
   "count up to date with only valid files within the archive, you could close and then "
   "open the archive again.\n"

   "@see getFileEntryCount()\n"
   "@see closeArchive()\n"
   "@see openArchive()")
{
   return object->deleteFile(pathInZip);
}

//-----------------------------------------------------------------------------

DefineEngineMethod(ZipObject, getFileEntryCount, S32, (),,
   "@brief Get the number of files within the zip archive.\n\n"

   "Use getFileEntry() to retrive information on each file within the archive.\n\n"

   "@return The number of files within the zip archive.\n"

   "@note The returned count will include any files that have been deleted from "
   "the archive using deleteFile().  To clear out all deleted files, you could "
   "close and then open the archive again.\n"

   "@see getFileEntry()\n"
   "@see closeArchive()\n"
   "@see openArchive()")
{
   return object->getFileEntryCount();
}

DefineEngineMethod(ZipObject, getFileEntry, String, ( S32 index ),,
   "@brief Get information on the requested file within the zip archive.\n\n"

   "This methods provides five different pieces of information for the requested file:\n"
   "<ul><li>filename - The path and name of the file within the zip archive</li>"
   "<li>uncompressed size</li>"
   "<li>compressed size</li>"
   "<li>compression method</li>"
   "<li>CRC32</li></ul>\n"

   "Use getFileEntryCount() to obtain the total number of files within the archive.\n"

   "@param index The index of the file within the zip archive.  Use getFileEntryCount() to determine the number of files.\n"
   "@return A tab delimited list of information on the requested file, or an empty string if the file could not be found.\n"

   "@see getFileEntryCount()")
{
   return object->getFileEntry(index);
}
