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

#include "core/util/zip/fileHeader.h"
#include "core/util/zip/centralDir.h"
#include "core/util/zip/compressor.h"

#include "core/stream/fileStream.h"

#include "core/util/tVector.h"
#include "core/util/tDictionary.h"
#include "core/util/timeClass.h"

#ifndef _ZIPARCHIVE_H_
#define _ZIPARCHIVE_H_

/// @addtogroup zip_group Zip Code
// @{

/// Password to use when opening encrypted zip files. Change this to whatever the password is for your zips.
#define DEFAULT_ZIP_PASSWORD     "changeme"

class ZipTestWrite;
class ZipTestRead;
class ZipTestMisc;

namespace Zip
{

/// @addtogroup zip_group Zip Code
// @{

// Forward Refs
class ZipTempStream;

// [tom, 10/18/2006] This will be split up into a separate interface for allowing
// the resource manager to handle any kind of archive relatively easily.

// [tom, 2/9/2007] At least, it was designed so that it could be. It may not
// actually happen for a very long time, if ever ;-)

//-----------------------------------------------------------------------------
/// @brief Class for accessing Zip files
///
/// ZipArchive provides two interfaces for reading or writing zip files.
/// The first is a stream based interface that should be familiar to anyone
/// who's ever written code, and the other is an archiver interface that
/// should be familiar to anyone who's ever used a standard Zip application.
///
/// The two interfaces are not mutually exclusive so you can use both if
/// you wish. For example, you may want to use the stream interface to add
/// a configuration file to a zip without having to create a temporary file
/// and then add a number of existing files using the addFile() method.
///
/// Both interfaces have their advantages and disadvantages, which will
/// be discussed below.
///
/// <h3>Accessing a Zip file</h3>
///
/// Before you can access any files in the zip, you first need to open the
/// archive. This is the same regardless of which interface you use, and there
/// are two ways to accomplish it.
///
/// <b>Opening from a file on the file system</b>
/// 
/// The simplest method of opening a zip file is to use openArchive(const char *, AccessMode)
/// to open a file that is on the disk. 
/// 
/// When opening a zip file on the file system, the filename is automatically set.
///
/// <b>Opening a file from a stream</b>
/// 
/// A more advanced way to open the zip file is from an arbitrary stream. The
/// only requirements are that the stream supports seeking and was opened with
/// the correct access mode. Use the openArchive(Stream *, AccessMode) method to
/// do this.
/// 
/// Opening zip files from arbitrary streams is a very powerful feature and
/// opens many interesting doors. For example, combined with some small changes
/// to the resource manager and startup code, it was possible to implement a
/// VFS that allows the entire game to run from a single executable with no
/// external files.
/// 
/// Note that the filename is not automatically set when you open the zip file
/// from a stream. The filename is used in error reporting and by the resource
/// manager, so you may wish to set it to something meaningful.
/// 
/// Regardless of which method you use to open the file, the #AccessMode controls
/// what you can do with it. If you open the archive as #ReadWrite, you can both
/// write to and read from files in the zip. However, it is not possible to open
/// files in the zip as #ReadWrite.
/// 
/// <b>Closing the zip file</b>
/// 
/// When you are done with the zip file, call closeArchive() to free any resources
/// and rebuild the zip file if it was open for #Write.
/// 
/// <b>Example</b>
/// 
/// @code
/// Zip::ZipArchive za;
/// if(za.openArchive("filename.zip", ZipArchive::Read))
/// {
///    // ... do stuff ...
///    za.closeArchive();
/// }
/// @endcode
///
/// <h3>Archiver Interface</h3>
/// 
/// The archiver style interface allows you to add, extract and delete files in
/// the zip in a way similar to that of an standard archiver application.
/// 
/// While the archiver interface is simple to use, it is blocking and thus
/// difficult to use asynchronously. If you require zip file support and
/// responsive UI then you should consider using the stream interface instead.
/// 
/// See the following method documentation for more information:
/// 
/// <ul>
///   <li> addFile()
///   <li> extractFile()
///   <li> deleteFile()
/// </ul>
///
/// <b>Example</b>
/// 
/// @code
/// Zip::ZipArchive za;
/// if(za.openArchive("filename.zip", ZipArchive::ReadWrite))
/// {
///    // Extract a file
///    za.extractFile("test.txt", "test.txt");
///    // Add a file
///    za.addFile("test.txt", "test.txt");
///    // Delete a file
///    za.deleteFile("test.txt");
///
///    za.closeArchive();
/// }
/// @endcode
/// 
/// <h3>Stream Interface</h3>
/// 
/// The stream based interface allows you to access files within the zip
/// in a similar way to accessing the file system through the ResourceManager.
/// 
/// There are a few small caveats to the stream interface:
/// <ul>
///   <li> When writing files, the whole file must be written sequentially. You
///        cannot seek in the stream.
///   <li> It may or may not be possible to seek in streams opened for read.
///        Files that were not compressed in the zip file support seeking with
///        no penalty. In all cases where the file is compressed, if seeking is
///        supported by the decompression and/or decryption filter then it
///        carries with it an extreme performance penalty and should be avoided.
///        All currently available decompression filters (Deflate and BZip2) and
///        decryption filters (Zip 2.0 and AES) support seeking, but have to reset
///        their state and re-decompress/decrypt the entire file up to the point
///        you are seeking to. An extreme example would be that if you had a
///        20MB file and were currently at the end of the file, seeking back 1 byte
///        of the file would cause the entire file to be decompressed again. This
///        would be a blocking operation that would lock Torque up for an appreciable
///        chunk of time.
///   <li> Files can only be open as #Read or #Write, but not #ReadWrite
///   <li> Only one file can be open for read at a time, but multiple files can
///        be open for write at a time. - [tom, 2/9/2007] Check this
/// </ul>
/// 
/// See the following method documentation for more information:
/// 
/// <ul>
///   <li> openFile()
///   <li> closeFile()
/// </ul>
///
/// <b>CRC Checking</b>
/// 
/// Unlike the archiver interface, there is no automatic CRC checking when
/// reading from files using the stream interface. If you will only be
/// reading files sequentially, see the documentation for ZipStatFilter
/// for a useful trick to get easy CRC checking.
///
/// <b>Example</b>
/// 
/// @code
/// Zip::ZipArchive za;
/// if(za.openArchive("filename.zip", ZipArchive::Write))
/// {
///    // Write to the file
///    Stream *stream;
///    if(stream = za.openFile("test.txt", ZipArchive::Write))
///    {
///       stream->writeLine((U8 *)"Hello, Zipped World!");
///       za.closeFile(stream);
///    }
///
///    za.closeArchive();
/// }
/// @endcode
/// 
/// <h3>Compressed Files</h3>
/// 
/// The zip code included with stock Torque supports "stored" (uncompressed) files
/// and deflate compressed files. The code is easily extensible to support any
/// compression format that the Zip file format supports.
/// 
/// In addition to the deflate and stored formats, BZip2 is supported but not
/// included with stock Torque. BZip2 support will be released as a resource in
/// the future.
/// 
/// <h3>Encrypted Files</h3>
/// 
/// Preliminary support for Encrypted/Passworded files is included in TGB Pro only.
/// Currently, only Zip 2.0 encryption is supported by the stock code. AES support
/// exists and may be released as a resource in the future.
/// 
/// To set the password used for zips, you need to modify the #DEFAULT_ZIP_PASSWORD
/// define in core/zip/zipArchive.h. This password will be used for all zips that
/// require a password. The default password is changeme. This may be used by
/// TGB Binary users to test encrypted zips with their game. Shipping with the
/// default password is not recommended for obvious reasons.
/// 
/// The intended use of encrypted zips is for preventing casual copying of your
/// game's assets. Zip 2.0 encryption has known weaknesses that allow an attacker
/// to decrypt the contents of the zip. AES encryption is significantly more secure,
/// but as the password must be stored in the executable it will not stop a
/// determined attacker.
/// 
/// A script accessible mechanism for setting the password does not currently exist.
/// To use encrypted mod zips, if the password was in script then the password
/// would be clearly visible to anyone that cared to poke around in your scripts.
/// 
/// Encrypted zip support will be improved in a future version. For now, a more
/// secure method of storing the password is left as an exercise for the reader.
/// 
/// <h3>Accessing Zip files from script</h3>
/// 
/// ZipArchive is a C++ class and thus cannot be used from script. However,
/// a wrapper is provided to allow script access to zips. See the documentation
/// on ZipObject for more information.
///
/// <h3>More Examples</h3>
/// 
/// More in depth example code than that featured here can be found in the
/// unit tests for the zip code (in the core/zip/unitTests directory)
/// and the script code for the packaging utility.
/// 
//-----------------------------------------------------------------------------
class ZipArchive : public StrongRefBase
{
   friend class ::ZipTestWrite;
   friend class ::ZipTestRead;
   friend class ::ZipTestMisc;

public:
   /// Access modes for zip files and files within the zip
   enum AccessMode
   {
      Read = Torque::FS::File::Read,               //!< Open a zip or file in a zip for reading
      Write = Torque::FS::File::Write,             //!< Open a zip or file in a zip for writing
      ReadWrite = Torque::FS::File::ReadWrite      //!< Open a zip file for reading and writing. <b>Note</b>: Not valid for files in zips.
   };

   struct ZipEntry
   {
      ZipEntry *mParent;
      
      String mName;

      bool mIsDirectory;
      CentralDir mCD;
      
      Map<String,ZipEntry*> mChildren;

      ZipEntry()
      {
         mName = "";
         mIsDirectory = false;
         mParent = NULL;
      }
   };
protected:

   Stream *mStream;
   FileStream *mDiskStream;
   AccessMode mMode;

   EndOfCentralDir mEOCD;

   // mRoot forms a tree of entries for fast queries given a file path
   // mEntries allows easy iteration of the entire file list
   ZipEntry *mRoot;
   Vector<ZipEntry *> mEntries;

   const char *mFilename;

   Vector<ZipTempStream *> mTempFiles;

   bool readCentralDirectory();

   void insertEntry(ZipEntry *ze);
   void removeEntry(ZipEntry *ze);
   
   Stream *createNewFile(const char *filename, Compressor *method);
   Stream *createNewFile(const char *filename, const char *method)
   {
      return createNewFile(filename, Compressor::findCompressor(method));
   }
   Stream *createNewFile(const char *filename, S32 method)
   {
      return createNewFile(filename, Compressor::findCompressor(method));
   }

   void updateFile(ZipTempStream *stream);
   bool rebuildZip();
   bool copyFileToNewZip(CentralDir *cdir, Stream *newZipStream);
   bool writeDirtyFileToNewZip(ZipTempStream *fileStream, Stream *zipStream);

public:
   ZipEntry* getRoot() { return mRoot; }
   ZipEntry* findZipEntry(const char *filename);
   static U32 localTimeToDOSTime(const Torque::Time::DateTime &dt);
   static Torque::Time DOSTimeToTime(U16 time, U16 date);
   static Torque::Time DOSTimeToTime(U32 datetime);
   static U32 TimeToDOSTime(const Torque::Time& t);
   static U32 currentTimeToDOSTime();
   void dumpCentralDirectory(ZipEntry* entry = NULL, String* indent = NULL);

public:
   ZipArchive();
   virtual ~ZipArchive();

   /// @name Miscellaneous Methods
   // @{
   
   //-----------------------------------------------------------------------------
   /// @brief Set the filename of the zip file.
   ///
   /// The zip filename is used by the resource manager and for error reporting.
   ///
   /// <b>Note</b>: The filename is set automatically when you open the file.
   ///
   /// @param filename Filename of the zip file
   //-----------------------------------------------------------------------------
   void setFilename(const char *filename);

   /// Set the disk stream pointer.  The ZipArchive is then responsible for 
   /// deleting the stream when appropriate and the caller should not do the same.
   /// This function should only be called after openArchive(Stream*) has been 
   /// successfully executed.
   void setDiskStream(FileStream* stream) { mDiskStream = stream; }

   //-----------------------------------------------------------------------------
   /// @brief Get the filename of the zip file.
   ///
   /// @returns Filename of the zip file, or NULL if none set
   //-----------------------------------------------------------------------------
   const char *getFilename()                          { return mFilename; }

   //-----------------------------------------------------------------------------
   /// @brief Determine if the Zip code is in verbose mode
   ///
   /// Verbose mode causes the Zip code to provide diagnostic error messages
   /// when things go wrong. It can be enabled or disabled through script by
   /// setting the $pref::Zip::Verbose variable to true to enable it or false
   /// to disable it.
   ///
   /// Verbose mode is mostly useful when tracking down issues with opening
   /// a zip file without having to resort to using a debugger.
   ///
   /// @returns The value of $pref::Zip::Verbose
   /// @see ZipArchive::setVerbose()
   //-----------------------------------------------------------------------------
   bool isVerbose();

   //-----------------------------------------------------------------------------
   /// @brief Turn verbose mode on or off.
   ///
   /// This sets the $pref::Zip::Verbose variable.
   ///
   /// See isVerbose() for a discussion on verbose mode.
   ///
   /// @param verbose True to enable verbose mode, false to disable
   /// @see ZipArchive::isVerbose()
   //-----------------------------------------------------------------------------
   void setVerbose(bool verbose);
   // @}

   /// @name Archive Access Methods
   // @{

   //-----------------------------------------------------------------------------
   /// @brief Open a zip archive from a file
   ///
   /// The archive must be closed with closeArchive() when you are done with it.
   ///
   /// @param filename Filename of zip file to open
   /// @param mode Access mode. May be Read, Write or ReadWrite
   /// @return true for success, false for failure
   /// @see ZipArchive::openArchive(Stream *, AccessMode), ZipArchive::closeArchive()
   //-----------------------------------------------------------------------------
   virtual bool openArchive(const char *filename, AccessMode mode = Read);

   //-----------------------------------------------------------------------------
   /// @brief Open a zip archive from a stream
   ///
   /// The stream must support seeking and must support the specified access
   /// mode. For example, if the stream is opened for Read you cannot specify
   /// Write to openArchive(). However, if the stream is open for ReadWrite
   /// then you can specify any one of Read, Write or ReadWrite as the mode
   /// argument to openArchive().
   ///
   /// The archive must be closed with closeArchive() when you are done with it.
   ///
   /// @param stream Pointer to stream to open the zip archive from
   /// @param mode Access mode. May be Read, Write or ReadWrite
   /// @return true for success, false for failure
   /// @see ZipArchive::openArchive(const char *, AccessMode), ZipArchive::closeArchive()
   //-----------------------------------------------------------------------------

   // CodeReview [tom, 2/9/2007] I just thought, if we open a stream directly
   // for write then rebuilding the zip file probably won't work. This needs to
   // be checked and either fixed or worked around.

   virtual bool openArchive(Stream *stream, AccessMode mode = Read);

   //-----------------------------------------------------------------------------
   /// @brief Close the zip archive and free any resources
   ///
   /// @see ZipArchive::openArchive(Stream *, AccessMode), ZipArchive::openArchive(const char *, AccessMode)
   //-----------------------------------------------------------------------------
   virtual void closeArchive();
   // @}

   /// @name Stream Based File Access Methods
   // @{

   //-----------------------------------------------------------------------------
   /// @brief Open a file within the zip file
   ///
   /// The access mode can only be Read or Write. It is not possible to open
   /// a file within the zip as ReadWrite.
   ///
   /// The returned stream must be freed with closeFile(). Do not delete it
   /// directly.
   ///
   /// In verbose mode, openFile() will display additional error information 
   /// in the console when it fails.
   ///
   /// @param filename Filename of the file in the zip
   /// @param mode Access mode. May be Read or Write
   /// @param ze Output zip entry.  May be unspecified.
   /// @return Pointer to stream or NULL for failure
   /// @see ZipArchive::closeFile(), ZipArchive::isVerbose()
   //-----------------------------------------------------------------------------
   virtual Stream *openFile(const char *filename, AccessMode mode = Read);
   virtual Stream *openFile(const char *filename, ZipEntry* ze, AccessMode = Read);

   //-----------------------------------------------------------------------------
   /// @brief Close a file opened through openFile()
   ///
   /// @param stream Stream to close
   /// @see ZipArchive::openFile(const char *, AccessMode)
   //-----------------------------------------------------------------------------
   virtual void closeFile(Stream *stream);

   //-----------------------------------------------------------------------------
   /// @brief Open a file within the zip file for read
   ///
   /// This method exists mainly for the integration with the resource manager.
   /// Unless there is good reason to use this method, it is better to use the
   /// openFile() method instead.
   ///
   /// @param fileCD Pointer to central directory of the file to open
   /// @return Pointer to stream or NULL for failure
   /// @see ZipArchive::openFile(const char *, AccessMode), ZipArchive::closeFile()
   //-----------------------------------------------------------------------------
   Stream *openFileForRead(const CentralDir *fileCD);
   // @}

   /// @name Archiver Style File Access Methods
   // @{
   
   //-----------------------------------------------------------------------------
   /// @brief Add a file to the zip
   ///
   /// If replace is false and the file already exists in the zip, this function
   /// will fail and return false. If replace is true, the existing file will be
   /// overwritten.
   ///
   /// @param filename Filename on the local file system to add
   /// @param pathInZip The path and filename in the zip file to give this file
   /// @param replace true to replace existing files, false otherwise
   /// @return true for success, false for failure
   /// @see ZipArchive::extractFile(), ZipArchive::deleteFile(), ZipArchive::isVerbose()
   //-----------------------------------------------------------------------------
   bool addFile(const char *filename, const char *pathInZip, bool replace = true);

   //-----------------------------------------------------------------------------
   /// @brief Extract a file from the zip
   ///
   /// The file will be created through the resource manager and so must be
   /// in a location that is writable.
   ///
   /// The file will be CRC checked during extraction and extractFile() will
   /// return false if the CRC check failed. The CRC check is just an advisory,
   /// the output file will still exist if the CRC check failed. It is up to
   /// the caller to decide what to do in the event of a CRC failure.
   ///
   /// You can optionally pass a pointer to a bool to determine if a CRC check
   /// failed. If extractFile() returns false and crcFail is false then the failure
   /// was not CRC related. If crcFail is true and extractFile() returns false,
   /// then the CRC check failed but the file otherwise extracted OK. You can
   /// take your chances as to whether the data is valid or not, if you wish.
   ///
   /// In verbose mode, extractFile() will display an error in the console when
   /// a file fails the CRC check.
   ///
   /// @param pathInZip The path and filename in the zip file to extract
   /// @param filename Filename on the local file system to extract to
   /// @param crcFail Pointer to a boolean that receives the result of the CRC check
   /// @return true for success, false for failure
   /// @see ZipArchive::addFile(), ZipArchive::deleteFile(), ZipArchive::isVerbose()
   //-----------------------------------------------------------------------------
   bool extractFile(const char *pathInZip, const char *filename, bool *crcFail = NULL);

   //-----------------------------------------------------------------------------
   /// @brief Delete a file from the zip
   ///
   /// Flags a file as deleted so it is removed when the zip file is rebuilt.
   ///
   /// @param filename Filename in the zip to delete
   /// @return true for success, false for failure
   /// @see ZipArchive::addFile(), ZipArchive::extractFile(), ZipArchive::isVerbose()
   //-----------------------------------------------------------------------------
   bool deleteFile(const char *filename);
   // @}

   /// @name Enumeration Methods
   // @{

   //-----------------------------------------------------------------------------
   /// @brief Get number of entries in the central directory
   ///
   /// @see ZipArchive::findFileInfo(const char *)
   //-----------------------------------------------------------------------------
   U32 numEntries() const                             { return mEntries.size(); }

   //-----------------------------------------------------------------------------
   /// Get a central directory entry
   //-----------------------------------------------------------------------------
   const CentralDir & operator[](const U32 idx) const { return mEntries[idx]->mCD; }

   //-----------------------------------------------------------------------------
   /// @brief Find a file in the zip
   ///
   /// @param filename Path and filename to find
   /// @return Pointer to the central directory entry
   //-----------------------------------------------------------------------------
   CentralDir *findFileInfo(const char *filename);
   // @}
};

// @}

} // end namespace Zip

// @}

#endif // _ZIPARCHIVE_H_
