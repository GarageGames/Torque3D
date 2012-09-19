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

#include "core/util/zip/zipCryptStream.h"
#include "core/util/zip/crctab.h"

#include "console/console.h"

//-----------------------------------------------------------------------------
// Constructor/Destructor
//-----------------------------------------------------------------------------

ZipCryptRStream::ZipCryptRStream() : mStream(NULL), mFileEndPos(0), mPassword(NULL)
{
}

ZipCryptRStream::~ZipCryptRStream()
{
}

//-----------------------------------------------------------------------------
// Private Methods
//-----------------------------------------------------------------------------

U32 ZipCryptRStream::fillBuffer(const U32 in_attemptSize, void *pBuffer)
{
   AssertFatal(mStream != NULL, "No stream to fill from?");
   AssertFatal(mStream->getStatus() != Stream::Closed,
      "Fill from a closed stream?");

   U32 currPos    = mStream->getPosition();

   U32 actualReadSize;
   if (in_attemptSize + currPos > mFileEndPos) {
      actualReadSize = mFileEndPos - currPos;
   } else {
      actualReadSize = in_attemptSize;
   }

   if (mStream->read(actualReadSize, pBuffer) == true) {
      return actualReadSize;
   } else {
      AssertWarn(false, "Read failed while trying to fill buffer");
      return 0;
   }
}

//-----------------------------------------------------------------------------
// Public Methods
//-----------------------------------------------------------------------------

void ZipCryptRStream::setPassword(const char *password)
{
   mKeys[0] = 305419896;
   mKeys[1] = 591751049;
   mKeys[2] = 878082192;
   
   mPassword = password;
   const char *pPtr = password;
   while(*pPtr)
   {
      updateKeys(*pPtr);
      pPtr++;
   }
}

bool ZipCryptRStream::attachStream(Stream* io_pSlaveStream)
{
   mStream = io_pSlaveStream;
   mStreamStartPos = mStream->getPosition();

   // [tom, 12/20/2005] Encrypted zip files have an extra 12 bytes
   // of entropy before the file data.

   U8 buffer[12];
   if(mStream->read(sizeof(buffer), &buffer))
   {
      // Initialize keys
      for(S32 i = 0;i < sizeof(buffer);i++)
      {
         updateKeys(buffer[i] ^= decryptByte());
      }
      
      // if(buffer[11] !)
      mFileStartPos = mStream->getPosition();

      setStatus(Ok);
      return true;
   }
   return false;
}

void ZipCryptRStream::detachStream()
{
   mStream = NULL;

   // Clear keys, just in case
   dMemset(&mKeys, 0, sizeof(mKeys));

   setStatus(Closed);
}

U32 ZipCryptRStream::getPosition() const
{
   return mStream->getPosition();
}

bool ZipCryptRStream::setPosition(const U32 in_newPosition)
{
   if(in_newPosition > mFileEndPos)
      return false;

   U32 curPos = getPosition();
   U32 readSize = in_newPosition - mFileStartPos;
   bool ret = true;

   if(in_newPosition < curPos)
   {
      // Reposition to start of stream
      Stream *stream = getStream();
      U32 startPos = mStreamStartPos;
      const char *password = mPassword;
      detachStream();
      setPassword(password);
      stream->setPosition(startPos);
      ret = attachStream(stream);

      if(in_newPosition == mFileStartPos)
         return ret;
   }

   // Read until we reach the new position
   U8 *buffer = new U8 [1024];
   while(readSize >= 1024)
   {
      readSize -= 1024;
      ret = _read(1024, buffer);
      if(! ret)
         break;
   }

   if(readSize > 0 && ret)
   {
      ret = _read(readSize, buffer);
   }
   delete [] buffer;

   return ret;
}

//-----------------------------------------------------------------------------
// Protected Methods
//-----------------------------------------------------------------------------

void ZipCryptRStream::updateKeys(const U8 c)
{
   mKeys[0] = ZC_CRC32(mKeys[0], c);
   mKeys[1] += mKeys[0] & 0x000000ff;
   mKeys[1] = mKeys[1] * 134775813 + 1;
   U32 k = mKeys[1] >> 24;
   mKeys[2] = ZC_CRC32(mKeys[2], k);
}

U8 ZipCryptRStream::decryptByte()
{
   U16 temp;
   temp = (mKeys[2] & 0xffff) | 2;
   return (temp * (temp ^ 1)) >> 8;
}

bool ZipCryptRStream::_read(const U32 in_numBytes, void* out_pBuffer)
{
   U32 numRead = fillBuffer(in_numBytes, out_pBuffer);
   if(numRead > 0)
   {
      // Decrypt
      U8 *pBytes = (U8 *)out_pBuffer;
      for(S32 i = 0;i < numRead;i++)
      {
         updateKeys(pBytes[i] ^= decryptByte());
      }
      return true;
   }
   return false;
}
