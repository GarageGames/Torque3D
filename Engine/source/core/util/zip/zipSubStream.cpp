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

#include "zlib.h"
#include "core/util/zip/zipSubStream.h"


const U32 ZipSubRStream::csm_streamCaps      = U32(Stream::StreamRead) | U32(Stream::StreamPosition);
const U32 ZipSubRStream::csm_inputBufferSize = 4096;

const U32 ZipSubWStream::csm_streamCaps      = U32(Stream::StreamWrite);
const U32 ZipSubWStream::csm_bufferSize      = (2048 * 1024);

//--------------------------------------------------------------------------
//--------------------------------------
//
ZipSubRStream::ZipSubRStream()
 : m_pStream(NULL),
   m_uncompressedSize(0),
   m_currentPosition(0),
   m_EOS(false),
   m_pZipStream(NULL),
   m_pInputBuffer(NULL),
   m_originalSlavePosition(0),
   m_lastBytesRead(0)
{
   //
}

//--------------------------------------
ZipSubRStream::~ZipSubRStream()
{
   detachStream();
}

//--------------------------------------
bool ZipSubRStream::attachStream(Stream* io_pSlaveStream)
{
   AssertFatal(io_pSlaveStream != NULL, "NULL Slave stream?");
   AssertFatal(m_pStream == NULL,       "Already attached!");

   m_pStream          = io_pSlaveStream;
   m_originalSlavePosition = io_pSlaveStream->getPosition();
   m_uncompressedSize = 0;
   m_currentPosition  = 0;
   m_EOS = false;

   // Initialize zipStream state...
   m_pZipStream   = new z_stream_s;
   m_pInputBuffer = new U8[csm_inputBufferSize];

   m_pZipStream->zalloc = Z_NULL;
   m_pZipStream->zfree  = Z_NULL;
   m_pZipStream->opaque = Z_NULL;

   U32 buffSize = fillBuffer(csm_inputBufferSize);

   m_pZipStream->next_in  = m_pInputBuffer;
   m_pZipStream->avail_in = buffSize;
   m_pZipStream->total_in = 0;
   inflateInit2(m_pZipStream, -MAX_WBITS);

   setStatus(Ok);
   return true;
}

//--------------------------------------
void ZipSubRStream::detachStream()
{
   if (m_pZipStream != NULL)
   {
      // close out zip stream...
      inflateEnd(m_pZipStream);

      delete [] m_pInputBuffer;
      m_pInputBuffer = NULL;
      delete m_pZipStream;
      m_pZipStream = NULL;
   }

   m_pStream          = NULL;
   m_originalSlavePosition = 0;
   m_uncompressedSize = 0;
   m_currentPosition  = 0;
   m_EOS              = false;
   setStatus(Closed);
}

//--------------------------------------
Stream* ZipSubRStream::getStream()
{
   return m_pStream;
}

//--------------------------------------
void ZipSubRStream::setUncompressedSize(const U32 in_uncSize)
{
   AssertFatal(m_pStream != NULL, "error, no stream to set unc size for");

   m_uncompressedSize = in_uncSize;
}

//--------------------------------------
bool ZipSubRStream::_read(const U32 in_numBytes, void *out_pBuffer)
{
   m_lastBytesRead = 0;
   if (in_numBytes == 0)
      return true;

   AssertFatal(out_pBuffer != NULL, "NULL output buffer");
   if (getStatus() == Closed) {
      AssertFatal(false, "Attempted read from closed stream");
      return false;
   }


   if (Ok != getStatus())
      return false;

   if (m_EOS)
   {
      setStatus(EOS);
      return true;
   };

   // Ok, we need to call inflate() until the output buffer is full.
   //  first, set up the output portion of the z_stream
   //
   m_pZipStream->next_out  = (Bytef*)out_pBuffer;
   m_pZipStream->avail_out = in_numBytes;
   m_pZipStream->total_out = 0;

   while (m_pZipStream->avail_out != 0)
   {
      S32 retVal = Z_OK;

      if(m_pZipStream->avail_in == 0)
      {
         // check if there is more output pending
         inflate(m_pZipStream, Z_SYNC_FLUSH);

         if(m_pZipStream->total_out != in_numBytes)
         {
            // Need to provide more input bytes for the stream to read...
            U32 buffSize = fillBuffer(csm_inputBufferSize);
            //AssertFatal(buffSize != 0, "Must find a more graceful way to handle this");

            m_pZipStream->next_in  = m_pInputBuffer;
            m_pZipStream->avail_in = buffSize;
            m_pZipStream->total_in = 0;
         }
      }

      // need to get more?
      if(m_pZipStream->total_out != in_numBytes)
         retVal = inflate(m_pZipStream, Z_SYNC_FLUSH);

      AssertFatal(retVal != Z_BUF_ERROR, "Should never run into a buffer error");
      AssertFatal(retVal == Z_OK || retVal == Z_STREAM_END, "error in the stream");

      m_lastBytesRead = m_pZipStream->total_out;

      if (retVal == Z_STREAM_END)
      {
         if (m_pZipStream->avail_out != 0)
            m_EOS = true;

         setStatus(EOS);
         m_currentPosition += m_pZipStream->total_out;
         return true;
      }
   }
   AssertFatal(m_pZipStream->total_out == in_numBytes,
               "Error, didn't finish the decompression!");

   // If we're here, everything went peachy...
   setStatus(Ok);
   m_currentPosition += m_pZipStream->total_out;

   return true;
}

//--------------------------------------
bool ZipSubRStream::hasCapability(const Capability in_cap) const
{
   return (csm_streamCaps & U32(in_cap)) != 0;
}

//--------------------------------------
U32 ZipSubRStream::getPosition() const
{
   AssertFatal(m_pStream != NULL, "Error, not attached");

   return m_currentPosition;
}

//--------------------------------------
bool ZipSubRStream::setPosition(const U32 in_newPosition)
{
   AssertFatal(m_pStream != NULL, "Error, not attached");

   if (in_newPosition == 0)
   {
      Stream* pStream = getStream();
      U32 resetPosition = m_originalSlavePosition;
      U32 uncompressedSize = m_uncompressedSize;
      detachStream();
      pStream->setPosition(resetPosition);
      attachStream(pStream);
      setUncompressedSize(uncompressedSize);
      return true;
   }
   else
   {
      if (in_newPosition > m_uncompressedSize)
         return false;

      U32 newPosition = in_newPosition;
      if (newPosition < m_currentPosition)
      {
         Stream* pStream = getStream();
         U32 resetPosition = m_originalSlavePosition;
         U32 uncompressedSize = m_uncompressedSize;
         detachStream();
         pStream->setPosition(resetPosition);
         attachStream(pStream);
         setUncompressedSize(uncompressedSize);
      }
      else
      {
         newPosition -= m_currentPosition;
      }

      bool bRet = true;
      char *buffer = new char[2048];
      while (newPosition >= 2048)
      {
         newPosition -= 2048;
         if (!_read(2048,buffer))
         {
            bRet = false;
            break;
         }
      };
      if (bRet && newPosition > 0)
      {
         if (!_read(newPosition,buffer))
         {
            bRet = false;
         };
      };

      delete [] buffer;

      return bRet;

   }
}

//--------------------------------------
U32 ZipSubRStream::getStreamSize()
{
   AssertFatal(m_pStream != NULL, "No stream to size()");
   AssertFatal(m_uncompressedSize != 0, "No data?  Properties probably not set...");

   return m_uncompressedSize;
}

//--------------------------------------
U32 ZipSubRStream::fillBuffer(const U32 in_attemptSize)
{
   AssertFatal(m_pStream != NULL, "No stream to fill from?");
   AssertFatal(m_pStream->getStatus() != Stream::Closed,
               "Fill from a closed stream?");

   U32 streamSize = m_pStream->getStreamSize();
   U32 currPos    = m_pStream->getPosition();

   U32 actualReadSize;
   if (in_attemptSize + currPos > streamSize) {
      actualReadSize = streamSize - currPos;
   } else {
      actualReadSize = in_attemptSize;
   }

   if (m_pStream->read(actualReadSize, m_pInputBuffer) == true) {
      return actualReadSize;
   } else {
      AssertWarn(false, "Read failed while trying to fill buffer");
      return 0;
   }
}


//--------------------------------------------------------------------------
ZipSubWStream::ZipSubWStream()
 : m_pStream(NULL),
   m_pZipStream(NULL),
   m_currPosition(0),
   m_pOutputBuffer(NULL),
   m_pInputBuffer(NULL),
   m_lastBytesRead(0),
   m_lastBytesWritten(0)
{
   //
}

//--------------------------------------
ZipSubWStream::~ZipSubWStream()
{
   detachStream();
}

//--------------------------------------
bool ZipSubWStream::attachStream(Stream* io_pSlaveStream)
{
   AssertFatal(io_pSlaveStream != NULL, "NULL Slave stream?");
   AssertFatal(m_pStream == NULL,       "Already attached!");

   m_pStream      = io_pSlaveStream;
   m_currPosition = 0;

   m_pOutputBuffer = new U8[csm_bufferSize];
   m_pInputBuffer  = new U8[csm_bufferSize];

   // Initialize zipStream state...
   m_pZipStream = new z_stream_s;

   m_pZipStream->zalloc = Z_NULL;
   m_pZipStream->zfree  = Z_NULL;
   m_pZipStream->opaque = Z_NULL;

   m_pZipStream->next_in   = m_pInputBuffer;
   m_pZipStream->avail_in  = csm_bufferSize;
   m_pZipStream->total_in  = 0;
   m_pZipStream->next_out  = m_pOutputBuffer;
   m_pZipStream->avail_out = csm_bufferSize;
   m_pZipStream->total_out = 0;

   deflateInit2(m_pZipStream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -MAX_WBITS, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY);

   setStatus(Ok);
   return true;
}

//--------------------------------------
void ZipSubWStream::detachStream()
{
   // Must finish...
   if (m_pZipStream != NULL)
   {
      m_pZipStream->avail_in = 0;
      deflate(m_pZipStream, Z_FINISH);

      // write the remainder
      m_pStream->write(csm_bufferSize - m_pZipStream->avail_out, m_pOutputBuffer);

      // close out zip stream...
      deflateEnd(m_pZipStream);

      delete m_pZipStream;
      m_pZipStream = NULL;

      delete [] m_pInputBuffer;
      delete [] m_pOutputBuffer;
      m_pInputBuffer  = NULL;
      m_pOutputBuffer = NULL;
   }

   m_pStream      = NULL;
   m_currPosition = 0;
   setStatus(Closed);
}

//--------------------------------------
Stream* ZipSubWStream::getStream()
{
   return m_pStream;
}

//--------------------------------------
bool ZipSubWStream::_read(const U32, void*)
{
   AssertFatal(false, "Cannot read from a ZipSubWStream");

   setStatus(IllegalCall);
   return false;
}

//--------------------------------------
bool ZipSubWStream::_write(const U32 numBytes, const void *pBuffer)
{
   m_lastBytesWritten = 0;
   if (numBytes == 0)
      return true;

   AssertFatal(pBuffer != NULL, "NULL input buffer");
   if (getStatus() == Closed)
   {
      AssertFatal(false, "Attempted write to a closed stream");
      return false;
   }

   m_pZipStream->next_in = (U8*)pBuffer;
   m_pZipStream->avail_in = numBytes;

   // write as many bufferSize chunks as possible
   while(m_pZipStream->avail_in != 0)
   {
      if(m_pZipStream->avail_out == 0)
      {
         if(!m_pStream->write(csm_bufferSize, m_pOutputBuffer))
            return(false);

         m_pZipStream->next_out = m_pOutputBuffer;
         m_pZipStream->avail_out = csm_bufferSize;
      }

      S32 retVal = deflate(m_pZipStream, Z_NO_FLUSH);
      AssertFatal(retVal !=  Z_BUF_ERROR, "ZipSubWStream::_write: invalid buffer");
   }

   setStatus(Ok);
   m_currPosition += m_pZipStream->total_out;

   m_lastBytesWritten = m_pZipStream->total_out;

   return true;
}

//--------------------------------------
bool ZipSubWStream::hasCapability(const Capability in_cap) const
{
   return (csm_streamCaps & U32(in_cap)) != 0;
}

//--------------------------------------
U32 ZipSubWStream::getPosition() const
{
   AssertFatal(m_pStream != NULL, "Error, not attached");

   return m_currPosition;
}

//--------------------------------------
bool ZipSubWStream::setPosition(const U32 /*in_newPosition*/)
{
   AssertFatal(m_pStream != NULL, "Error, not attached");
   AssertFatal(false, "Not implemented!");

   // Erk.  How do we do this.
   return false;
}

U32 ZipSubWStream::getStreamSize()
{
   AssertFatal(false, "Undecided how to implement this!");
   return 0;
}

