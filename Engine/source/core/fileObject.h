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

#ifndef _FILEOBJECT_H_
#define _FILEOBJECT_H_

#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif
#ifndef _FILESTREAM_H_
#include "core/stream/fileStream.h"
#endif

class FileObject : public SimObject
{
   typedef SimObject Parent;
   U8 *mFileBuffer;
   U32 mBufferSize;
   U32 mCurPos;
   FileStream *stream;
public:
   FileObject();
   ~FileObject();

   bool openForWrite(const char *fileName, const bool append = false);
   bool openForRead(const char *fileName);
   bool readMemory(const char *fileName);
   const U8 *buffer() { return mFileBuffer; }
   const U8 *readLine();
   void peekLine(U8 *line, S32 length);
   bool isEOF();
   void writeLine(const U8 *line);
   void close();
   void writeObject( SimObject* object, const U8* objectPrepend = NULL );

   DECLARE_CONOBJECT(FileObject);
};

#endif
