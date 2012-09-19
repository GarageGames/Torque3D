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

#include "platform/platform.h"
#include "core/stream/fileStream.h"


//-----------------------------------------------------------------------------
// FileStream methods...
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
FileStream::FileStream()
{
   // initialize the file stream
   init();
}

FileStream *FileStream::createAndOpen(const String &inFileName, Torque::FS::File::AccessMode inMode)
{
   FileStream  *newStream = new FileStream;

   if ( newStream )
   {
      bool success = newStream->open( inFileName, inMode );

      if ( !success )
      {
         delete newStream;
         newStream = NULL;
      }
   }

   return newStream;
}

//-----------------------------------------------------------------------------
FileStream::~FileStream()
{
   // make sure the file stream is closed
   close();
}

//-----------------------------------------------------------------------------
bool FileStream::hasCapability(const Capability i_cap) const
{
   return(0 != (U32(i_cap) & mStreamCaps));
}

//-----------------------------------------------------------------------------
U32 FileStream::getPosition() const
{
   AssertFatal(0 != mStreamCaps, "FileStream::getPosition: the stream isn't open");
   //AssertFatal(true == hasCapability(StreamPosition), "FileStream::getPosition(): lacks positioning capability");

   // return the position inside the buffer if its valid, otherwise return the underlying file position
   return((BUFFER_INVALID != mBuffHead) ? mBuffPos : mFile->getPosition());
}

//-----------------------------------------------------------------------------
bool FileStream::setPosition(const U32 i_newPosition)
{
   AssertFatal(0 != mStreamCaps, "FileStream::setPosition: the stream isn't open");
   AssertFatal(hasCapability(StreamPosition), "FileStream::setPosition: lacks positioning capability");

   // if the buffer is valid, test the new position against the bounds of the buffer
   if ((BUFFER_INVALID != mBuffHead) && (i_newPosition >= mBuffHead) && (i_newPosition <= mBuffTail))
   {
      // set the position and return
      mBuffPos = i_newPosition;
      
      // FIXME [tom, 9/5/2006] This needs to be checked. Basically, when seeking within
      // the buffer, if the stream has an EOS status before the seek then if you try to
      // read immediately after seeking, you'll incorrectly get an EOS.
      //
      // I am not 100% sure if this fix is correct, but it seems to be working for the undo system.
      if(mBuffPos < mBuffTail)
         Stream::setStatus(Ok);
      
      return(true);
   }
   // otherwise the new position lies in some block not in memory
   else
   {
      if (mDirty)
         flush();

      clearBuffer();

      mFile->setPosition(i_newPosition, Torque::FS::File::Begin);

      setStatus();
      
      if (mFile->getStatus() == Torque::FS::FileNode::EndOfFile)
         mEOF = true;

      return(Ok == getStatus() || EOS == getStatus());
   }
}

//-----------------------------------------------------------------------------
U32 FileStream::getStreamSize()
{
   AssertWarn(0 != mStreamCaps, "FileStream::getStreamSize: the stream isn't open");
   AssertFatal((BUFFER_INVALID != mBuffHead && true == mDirty) || false == mDirty, "FileStream::getStreamSize: buffer must be valid if its dirty");

   // the stream size may not match the size on-disk if its been written to...
   if (mDirty)
      return(getMax((U32)(mFile->getSize()), mBuffTail + 1));  ///<@todo U64 vs U32 issue
   // otherwise just get the size on disk...
   else
      return(mFile->getSize());
}

//-----------------------------------------------------------------------------
bool FileStream::open(const String &inFileName, Torque::FS::File::AccessMode inMode)
{
   AssertWarn(0 == mStreamCaps, "FileStream::setPosition: the stream is already open");
   AssertFatal(inFileName.isNotEmpty(), "FileStream::open: empty filename");

   // make sure the file stream's state is clean
   clearBuffer();

   Torque::Path   filePath(inFileName);

   // IF we are writing, make sure the path exists
   if( inMode == Torque::FS::File::Write || inMode == Torque::FS::File::WriteAppend || inMode == Torque::FS::File::ReadWrite )
      Torque::FS::CreatePath(filePath);

   mFile = Torque::FS::OpenFile(filePath, inMode);

   if (mFile != NULL)
   {
      setStatus();
      switch (inMode)
      {
         case Torque::FS::File::Read:
            mStreamCaps = U32(StreamRead) |
                          U32(StreamPosition);
            break;
         case Torque::FS::File::Write:
         case Torque::FS::File::WriteAppend:
            mStreamCaps = U32(StreamWrite) |
                          U32(StreamPosition);
            break;
         case Torque::FS::File::ReadWrite:
            mStreamCaps = U32(StreamRead)  |
                          U32(StreamWrite) |
                          U32(StreamPosition);
            break;
         default:
            AssertFatal(false, String::ToString( "FileStream::open: bad access mode on %s", inFileName.c_str() ));
      }
   }
   else
   {
      Stream::setStatus(IOError);
      return(false);
   }

   return getStatus() == Ok;
}

//-----------------------------------------------------------------------------
void FileStream::close()
{
   if (getStatus() == Closed)
      return;

   if (mFile != NULL )
   {
      // make sure nothing in the buffer differs from what is on disk
      if (mDirty)
         flush();

      // and close the file
      mFile->close();

      AssertFatal(mFile->getStatus() == Torque::FS::FileNode::Closed, "FileStream::close: close failed");

      mFile = NULL;
   }

   // clear the file stream's state
   init();
}

//-----------------------------------------------------------------------------
bool FileStream::flush()
{
   AssertWarn(0 != mStreamCaps, "FileStream::flush: the stream isn't open");
   AssertFatal(false == mDirty || BUFFER_INVALID != mBuffHead, "FileStream::flush: buffer must be valid if its dirty");

   // if the buffer is dirty
   if (mDirty)
   {
      AssertFatal(hasCapability(StreamWrite), "FileStream::flush: a buffer without write-capability should never be dirty");
      
      // align the file pointer to the buffer head
      if (mBuffHead != mFile->getPosition())
      {
         mFile->setPosition(mBuffHead, Torque::FS::File::Begin);
         if (mFile->getStatus() != Torque::FS::FileNode::Open && mFile->getStatus() != Torque::FS::FileNode::EndOfFile)
            return(false);
      }

      // write contents of the buffer to disk
      U32 blockHead;
      calcBlockHead(mBuffHead, &blockHead);
      mFile->write((char *)mBuffer + (mBuffHead - blockHead), mBuffTail - mBuffHead + 1);
      // and update the file stream's state
      setStatus();
      if (EOS == getStatus())
         mEOF = true;

      if (Ok == getStatus() || EOS == getStatus())
         // and update the status of the buffer
         mDirty = false;
      else
         return(false);
   }
   return(true);
}

//-----------------------------------------------------------------------------
bool FileStream::_read(const U32 i_numBytes, void *o_pBuffer)
{
   AssertFatal(0 != mStreamCaps, "FileStream::_read: the stream isn't open");
   AssertFatal(NULL != o_pBuffer || i_numBytes == 0, "FileStream::_read: NULL destination pointer with non-zero read request");

   if (!hasCapability(Stream::StreamRead))
   {
      AssertFatal(false, "FileStream::_read: file stream lacks capability");
      Stream::setStatus(IllegalCall);
      return(false);
   }

   // exit on pre-existing errors
   if (Ok != getStatus())
      return(false);

   // if a request of non-zero length was made
   if (0 != i_numBytes)
   {
      U8 *pDst = (U8 *)o_pBuffer;
      U32 readSize;
      U32 remaining = i_numBytes;
      U32 bytesRead;
      U32 blockHead;
      U32 blockTail;

      // check if the buffer has some data in it
      if (BUFFER_INVALID != mBuffHead)
      {
         // copy as much as possible from the buffer into the destination
         readSize = ((mBuffTail + 1) >= mBuffPos) ? (mBuffTail + 1 - mBuffPos) : 0;
         readSize = getMin(readSize, remaining);
         calcBlockHead(mBuffPos, &blockHead);
         dMemcpy(pDst, mBuffer + (mBuffPos - blockHead), readSize);
         // reduce the remaining amount to read
         remaining -= readSize;
         // advance the buffer pointers
         mBuffPos += readSize;
         pDst += readSize;

         if (mBuffPos > mBuffTail && remaining != 0)
         {
            flush();
            mBuffHead = BUFFER_INVALID;
            if (mEOF == true)
               Stream::setStatus(EOS);
         }
      }

      // if the request wasn't satisfied by the buffer and the file has more data
      if (false == mEOF && 0 < remaining)
      {
         // flush the buffer if its dirty, since we now need to go to disk
         if (true == mDirty)
            flush();

         // make sure we know the current read location in the underlying file
         mBuffPos = mFile->getPosition();
         calcBlockBounds(mBuffPos, &blockHead, &blockTail);

         // check if the data to be read falls within a single block
         if ((mBuffPos + remaining) <= blockTail)
         {
            // fill the buffer from disk
            if (true == fillBuffer(mBuffPos))
            {
               // copy as much as possible from the buffer to the destination
               remaining = getMin(remaining, mBuffTail - mBuffPos + 1);
               dMemcpy(pDst, mBuffer + (mBuffPos - blockHead), remaining);
               // advance the buffer pointer
               mBuffPos += remaining;
            }
            else
               return(false);
         }
         // otherwise the remaining spans multiple blocks
         else
         {
            clearBuffer();
            // read from disk directly into the destination
            bytesRead = mFile->read((char *)pDst, remaining);
            setStatus();
            // check to make sure we read as much as expected
            if (Ok == getStatus() || EOS == getStatus())
            {
               // if not, update the end-of-file status
               if (0 != bytesRead && EOS == getStatus())
               {
                  Stream::setStatus(Ok);
                  mEOF = true;
               }
            }
            else
               return(false);
         }
      }
   }
   return(true);
}

//-----------------------------------------------------------------------------
bool FileStream::_write(const U32 i_numBytes, const void *i_pBuffer)
{
   AssertFatal(0 != mStreamCaps, "FileStream::_write: the stream isn't open");
   AssertFatal(NULL != i_pBuffer || i_numBytes == 0, "FileStream::_write: NULL source buffer pointer on non-zero write request");

   if (!hasCapability(Stream::StreamWrite))
   {
      AssertFatal(false, "FileStream::_write: file stream lacks capability");
      Stream::setStatus(IllegalCall);
      return(false);
   }

   // exit on pre-existing errors
   if (Ok != getStatus() && EOS != getStatus())
      return(false);

   // if a request of non-zero length was made
   if (0 != i_numBytes)
   {
      U8 *pSrc = (U8 *)i_pBuffer;
      U32 writeSize;
      U32 remaining = i_numBytes;
      U32 bytesWrit;
      U32 blockHead;
      U32 blockTail;

      // check if the buffer is valid
      if (BUFFER_INVALID != mBuffHead)
      {
         // copy as much as possible from the source to the buffer
         calcBlockBounds(mBuffHead, &blockHead, &blockTail);
         writeSize = (mBuffPos > blockTail) ? 0 : blockTail - mBuffPos + 1;
         writeSize = getMin(writeSize, remaining);

         AssertFatal(0 == writeSize || (mBuffPos - blockHead) < BUFFER_SIZE, "FileStream::_write: out of bounds buffer position");
         dMemcpy(mBuffer + (mBuffPos - blockHead), pSrc, writeSize);
         // reduce the remaining amount to be written
         remaining -= writeSize;
         // advance the buffer pointers
         mBuffPos += writeSize;
         mBuffTail = getMax(mBuffTail, mBuffPos - 1);
         pSrc += writeSize;
         // mark the buffer dirty
         if (0 < writeSize)
            mDirty = true;
      }

      // if the request wasn't satisfied by the buffer
      if (0 < remaining)
      {
         // flush the buffer if its dirty, since we now need to go to disk
         if (mDirty)
            flush();

         // make sure we know the current write location in the underlying file
         mBuffPos = mFile->getPosition();
         calcBlockBounds(mBuffPos, &blockHead, &blockTail);

         // check if the data to be written falls within a single block
         if ((mBuffPos + remaining) <= blockTail)
         {
            // write the data to the buffer
            dMemcpy(mBuffer + (mBuffPos - blockHead), pSrc, remaining);
            // update the buffer pointers
            mBuffHead = mBuffPos;
            mBuffPos += remaining;
            mBuffTail = mBuffPos - 1;
            // mark the buffer dirty
            mDirty = true;
         }
         // otherwise the remaining spans multiple blocks
         else
         {
            clearBuffer();
            // write to disk directly from the source
            bytesWrit = mFile->write((char *)pSrc, remaining);
            setStatus();
            return(Ok == getStatus() || EOS == getStatus());
         }
      }
   }
   return(true);
}

//-----------------------------------------------------------------------------
void FileStream::init()
{
   mStreamCaps = 0;
   Stream::setStatus(Closed);
   clearBuffer();
}

//-----------------------------------------------------------------------------
bool FileStream::fillBuffer(const U32 i_startPosition)
{
   AssertFatal(0 != mStreamCaps, "FileStream::fillBuffer: the stream isn't open");
   AssertFatal(false == mDirty, "FileStream::fillBuffer: buffer must be clean to fill");

   // make sure start position and file pointer jive
   if (i_startPosition != mFile->getPosition())
   {
      mFile->setPosition(i_startPosition, Torque::FS::File::Begin);
      if (mFile->getStatus() != Torque::FS::FileNode::Open && mFile->getStatus() != Torque::FS::FileNode::EndOfFile)
      {
         setStatus();
         return(false);
      }
      else
         // update buffer pointer
         mBuffPos = i_startPosition;
   }

   // check if file pointer is at end-of-file
   if (EOS == getStatus())
   {
      // invalidate the buffer
      mBuffHead = BUFFER_INVALID;
      // set the status to end-of-stream
      mEOF = true;
   }
   // otherwise
   else
   {
      U32 blockHead;
      // locate bounds of buffer containing current position
      calcBlockHead(mBuffPos, &blockHead);
      // read as much as possible from input file
      U32 bytesRead = mFile->read((char *)mBuffer + (i_startPosition - blockHead), BUFFER_SIZE - (i_startPosition - blockHead));
      setStatus();
      if (Ok == getStatus() || EOS == getStatus())
      {
         // update buffer pointers
         mBuffHead = i_startPosition;
         mBuffPos = i_startPosition;
         mBuffTail = i_startPosition + bytesRead - 1;
         // update end-of-file status
         if (0 != bytesRead && EOS == getStatus())
         {
            Stream::setStatus(Ok);
            mEOF = true;
         }
      }
      else
      {
         mBuffHead = BUFFER_INVALID;
         return(false);
      }
   }
   return(true);
}

//-----------------------------------------------------------------------------
void FileStream::clearBuffer()
{
   mBuffHead = BUFFER_INVALID;
   mBuffPos  = 0;
   mBuffTail = 0;
   mDirty = false;
   mEOF = false;
}

//-----------------------------------------------------------------------------
void FileStream::calcBlockHead(const U32 i_position, U32 *o_blockHead)
{
   AssertFatal(NULL != o_blockHead, "FileStream::calcBlockHead: NULL pointer passed for block head");

   *o_blockHead = i_position/BUFFER_SIZE * BUFFER_SIZE;
}

//-----------------------------------------------------------------------------
void FileStream::calcBlockBounds(const U32 i_position, U32 *o_blockHead, U32 *o_blockTail)
{
   AssertFatal(NULL != o_blockHead, "FileStream::calcBlockBounds: NULL pointer passed for block head");
   AssertFatal(NULL != o_blockTail, "FileStream::calcBlockBounds: NULL pointer passed for block tail");

   *o_blockHead = i_position/BUFFER_SIZE * BUFFER_SIZE;
   *o_blockTail = *o_blockHead + BUFFER_SIZE - 1;
}

//-----------------------------------------------------------------------------
void FileStream::setStatus()
{
   switch (mFile->getStatus())
   {
      case Torque::FS::FileNode::Open:
         Stream::setStatus(Ok);
         break;

      case Torque::FS::FileNode::Closed:
         Stream::setStatus(Closed);
         break;

      case Torque::FS::FileNode::EndOfFile:
         Stream::setStatus(EOS);
         break;

      case Torque::FS::FileNode::FileSystemFull:
      case Torque::FS::FileNode::NoSuchFile:
      case Torque::FS::FileNode::AccessDenied:
      case Torque::FS::FileNode::NoDisk:
      case Torque::FS::FileNode::SharingViolation:
         Stream::setStatus(IOError);
         break;

      case Torque::FS::FileNode::IllegalCall:
         Stream::setStatus(IllegalCall);
         break;

      case Torque::FS::FileNode::UnknownError:
         Stream::setStatus(UnknownError);
         break;

      default:
         AssertFatal(false, "FileStream::setStatus: invalid error mode");
   }
}

FileStream* FileStream::clone() const
{
   Torque::FS::File::AccessMode mode;
   if( hasCapability( StreamWrite ) && hasCapability( StreamRead ) )
      mode = Torque::FS::File::ReadWrite;
   else if( hasCapability( StreamWrite ) )
      mode = Torque::FS::File::Write;
   else
      mode = Torque::FS::File::Read;

   FileStream* copy = createAndOpen( mFile->getName(), mode );
   if( copy && copy->setPosition( getPosition() ) )
      return copy;

   delete copy;
   return NULL;
}
