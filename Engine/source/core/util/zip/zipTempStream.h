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

#include "core/util/zip/zipArchive.h"
#include "core/util/str.h"

#ifndef _ZIPTEMPSTREAM_H_
#define _ZIPTEMPSTREAM_H_

namespace Zip
{

/// @addtogroup zipint_group
/// @ingroup zip_group
// @{

class ZipTempStream : public FileStream
{
   typedef FileStream Parent;

protected:
   CentralDir *mCD;
   bool mDeleteOnClose;
   String mFilename;

public:
   ZipTempStream() : mCD(NULL), mDeleteOnClose(false) {}
   ZipTempStream(CentralDir *cd) : mCD(cd), mDeleteOnClose(false) {}
   virtual ~ZipTempStream() { close(); }

   void setCentralDir(CentralDir *cd)     { mCD = cd; }
   CentralDir *getCentralDir()            { return mCD; }

   void setDeleteOnClose(bool del)        { mDeleteOnClose = del; }

   virtual bool open(String filename, Torque::FS::File::AccessMode mode);
   
   /// Open a temporary file in ReadWrite mode. The file will be deleted when the stream is closed.
   virtual bool open()
   {
      return open(String(), Torque::FS::File::ReadWrite);
   }

   virtual void close()
   {
      Parent::close();

      if(mDeleteOnClose)
         Torque::FS::Remove(mFilename);
      
   }

   /// Disallow setPosition() 
   virtual bool setPosition(const U32 i_newPosition)        { return false; }

   /// Seek back to the start of the file.
   /// This is used internally by the zip code and should never be called whilst
   /// filters are attached (e.g. when reading or writing in a zip file)
   bool rewind()
   {
      mStreamCaps |= U32(StreamPosition);
      bool ret = Parent::setPosition(0);
      mStreamCaps &= ~U32(StreamPosition);

      return ret;
   }
};

// @}

} // end namespace Zip

#endif // _ZIPTEMPSTREAM_H_
