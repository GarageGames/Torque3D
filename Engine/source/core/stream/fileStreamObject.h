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

#ifndef _FILESTREAMOBJECT_H_
#define _FILESTREAMOBJECT_H_

#ifndef _STREAMOBJECT_H_
#include "core/stream/streamObject.h"
#endif

#ifndef _FILESTREAM_H_
#include "core/stream/fileStream.h"
#endif


class FileStreamObject : public StreamObject
{
   typedef StreamObject Parent;

protected:
   FileStream mFileStream;

public:
   FileStreamObject();
   virtual ~FileStreamObject();
   DECLARE_CONOBJECT(FileStreamObject);

   virtual bool onAdd();

   //-----------------------------------------------------------------------------
   /// @brief Open a file
   /// 
   /// @param filename Name of file to open
   /// @param mode One of #Torque::FS::File::Read, #Torque::FS::File::Write, #Torque::FS::File::ReadWrite or #Torque::FS::File::WriteAppend
   /// @return true for success, false for failure
   /// @see close()
   //-----------------------------------------------------------------------------
   bool open(const char *filename, Torque::FS::File::AccessMode mode);

   //-----------------------------------------------------------------------------
   /// @brief Close the file
   /// 
   /// @see open()
   //-----------------------------------------------------------------------------
   void close();
};

#endif // _FILESTREAMOBJECT_H_
