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
#include "core/dnet.h"
#include "console/simBase.h"
#include "console/engineAPI.h"
#include "sim/netConnection.h"
#include "core/stream/bitStream.h"
#include "core/stream/fileStream.h"
#include "sim/netObject.h"

class FileDownloadRequestEvent : public NetEvent
{
public:
   typedef NetEvent Parent;
   enum 
   {
      MaxFileNames = 31,
   };
   
   U32 nameCount;
   char mFileNames[MaxFileNames][256];

   FileDownloadRequestEvent(Vector<char *> *nameList = NULL)
   {
      nameCount = 0;
      if(nameList)
      {
         nameCount = nameList->size();

         if(nameCount > MaxFileNames)
            nameCount = MaxFileNames;

         for(U32 i = 0; i < nameCount; i++)
         {
            dStrcpy(mFileNames[i], (*nameList)[i]);
            //Con::printf("Sending request for file %s", mFileNames[i]);
         }
      }
   }

   virtual void pack(NetConnection *, BitStream *bstream)
   {
      bstream->writeRangedU32(nameCount, 0, MaxFileNames);
      for(U32 i = 0; i < nameCount; i++)
         bstream->writeString(mFileNames[i]);
   }

   virtual void write(NetConnection *, BitStream *bstream)
   {
      bstream->writeRangedU32(nameCount, 0, MaxFileNames);
      for(U32 i = 0; i < nameCount; i++)
         bstream->writeString(mFileNames[i]);
   }

   virtual void unpack(NetConnection *, BitStream *bstream)
   {
      nameCount = bstream->readRangedU32(0, MaxFileNames);
      for(U32 i = 0; i < nameCount; i++)
         bstream->readString(mFileNames[i]);
   }

   virtual void process(NetConnection *connection)
   {
      U32 i;
      for(i = 0; i < nameCount; i++)
         if(connection->startSendingFile(mFileNames[i]))
            break;
      if(i == nameCount)
         connection->startSendingFile(NULL);  // none of the files were sent
   }

   DECLARE_CONOBJECT(FileDownloadRequestEvent);

};

IMPLEMENT_CO_NETEVENT_V1(FileDownloadRequestEvent);

ConsoleDocClass( FileDownloadRequestEvent,
				"@brief Used by NetConnection for transmitting requests to obtain files from server during loading.\n\n"
				"Not intended for game development, for editors or internal use only.\n\n "
				"@internal");

class FileChunkEvent : public NetEvent
{
public:
   typedef NetEvent Parent;
   enum
   {
      ChunkSize = 63,
   };

   U8 chunkData[ChunkSize];
   U32 chunkLen;
   
   FileChunkEvent(U8 *data = NULL, U32 len = 0)
   {
      if(data)
         dMemcpy(chunkData, data, len);
      chunkLen = len;
   }
   
   virtual void pack(NetConnection *, BitStream *bstream)
   {
      bstream->writeRangedU32(chunkLen, 0, ChunkSize);
      bstream->write(chunkLen, chunkData);
   }
   
   virtual void write(NetConnection *, BitStream *bstream)
   {
      bstream->writeRangedU32(chunkLen, 0, ChunkSize);
      bstream->write(chunkLen, chunkData);
   }
   
   virtual void unpack(NetConnection *, BitStream *bstream)
   {
      chunkLen = bstream->readRangedU32(0, ChunkSize);
      bstream->read(chunkLen, chunkData);
   }
   
   virtual void process(NetConnection *connection)
   {
      connection->chunkReceived(chunkData, chunkLen);
   }
   
   virtual void notifyDelivered(NetConnection *nc, bool madeIt)
   {
      if(!nc->isRemoved())
        nc->sendFileChunk();
   }
   
   DECLARE_CONOBJECT(FileChunkEvent);
};

IMPLEMENT_CO_NETEVENT_V1(FileChunkEvent);

ConsoleDocClass( FileChunkEvent,
				"@brief Used by NetConnection for sending/receiving chunks of data.\n\n"
				"Not intended for game development, for editors or internal use only.\n\n "
				"@internal");

void NetConnection::sendFileChunk()
{
   U8 buffer[FileChunkEvent::ChunkSize];
   U32 len = FileChunkEvent::ChunkSize;
   if(len + mCurrentFileBufferOffset > mCurrentFileBufferSize)
      len = mCurrentFileBufferSize - mCurrentFileBufferOffset;

   if(!len)
   {
      delete mCurrentDownloadingFile;
      mCurrentDownloadingFile = NULL;
      return;
   }

   mCurrentFileBufferOffset += len;
   mCurrentDownloadingFile->read(len, buffer);
   postNetEvent(new FileChunkEvent(buffer, len));
}

bool NetConnection::startSendingFile(const char *fileName)
{
   if(!fileName || Con::getBoolVariable("$NetConnection::neverUploadFiles"))
   {
      sendConnectionMessage(SendNextDownloadRequest);
      return false;
   }

   mCurrentDownloadingFile = FileStream::createAndOpen( fileName, Torque::FS::File::Read );
   if(!mCurrentDownloadingFile)
   {
      // the server didn't have the file, so send a 0 byte chunk:
      Con::printf("No such file '%s'.", fileName);
      postNetEvent(new FileChunkEvent(NULL, 0));
      return false;
   }

   Con::printf("Sending file '%s'.", fileName);
   mCurrentFileBufferSize = mCurrentDownloadingFile->getStreamSize();
   mCurrentFileBufferOffset = 0;

   // always have 32 file chunks (64 bytes each) in transit
   sendConnectionMessage(FileDownloadSizeMessage, mCurrentFileBufferSize);
   for(U32 i = 0; i < 32; i++)
      sendFileChunk();
   return true;
}

void NetConnection::sendNextFileDownloadRequest()
{
   // see if we've already downloaded this file...
   while(mMissingFileList.size() && (Torque::FS::IsFile(mMissingFileList[0]) || Con::getBoolVariable("$NetConnection::neverDownloadFiles")))
   {
      dFree(mMissingFileList[0]);
      mMissingFileList.pop_front();
   }

   if(mMissingFileList.size())
   {
      postNetEvent(new FileDownloadRequestEvent(&mMissingFileList));
   }
   else
   {
      fileDownloadSegmentComplete();
   }
}


void NetConnection::chunkReceived(U8 *chunkData, U32 chunkLen)
{
   if(chunkLen == 0)
   {
      // the server didn't have the file... apparently it's one we don't need...
      dFree(mCurrentFileBuffer);
      mCurrentFileBuffer = NULL;
      dFree(mMissingFileList[0]);
      mMissingFileList.pop_front();
      return;
   }
   if(chunkLen + mCurrentFileBufferOffset > mCurrentFileBufferSize)
   {
      setLastError("Invalid file chunk from server.");
      return;
   }
   dMemcpy(((U8 *) mCurrentFileBuffer) + mCurrentFileBufferOffset, chunkData, chunkLen);
   mCurrentFileBufferOffset += chunkLen;
   if(mCurrentFileBufferOffset == mCurrentFileBufferSize)
   {
      // this file's done...
      // save it to disk:
      FileStream *stream;

      Con::printf("Saving file %s.", mMissingFileList[0]);
      if((stream = FileStream::createAndOpen( mMissingFileList[0], Torque::FS::File::Write )) == NULL)
      {
         setLastError("Couldn't open file downloaded by server.");
         return;
      }

      dFree(mMissingFileList[0]);
      mMissingFileList.pop_front();
      stream->write(mCurrentFileBufferSize, mCurrentFileBuffer);
      delete stream;
      mNumDownloadedFiles++;
      dFree(mCurrentFileBuffer);
      mCurrentFileBuffer = NULL;
      sendNextFileDownloadRequest();
   }
   else
   {
      Con::executef("onFileChunkReceived", mMissingFileList[0], Con::getIntArg(mCurrentFileBufferOffset), Con::getIntArg(mCurrentFileBufferSize));
   }
}

