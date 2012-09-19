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

#include "core/stream/stream.h"
#include "core/strings/stringFunctions.h"

#include "core/util/zip/centralDir.h"
#include "core/util/zip/compressor.h"

#include "core/util/safeDelete.h"

namespace Zip
{

//-----------------------------------------------------------------------------
// CentralDir Class
//-----------------------------------------------------------------------------

CentralDir::CentralDir()
{
   mHeaderSig = mCentralDirSignature;

   mDiskNumStart = 0;

   mInternalFileAttr = 0;
   mExternalFileAttr = 0;

   mLocalHeadOffset = 0;
   
   mVersionMadeBy = 0;

   mFileComment = NULL;

   mInternalFlags = 0;
}

CentralDir::CentralDir(FileHeader &fh) : FileHeader(fh)
{
   mHeaderSig = mCentralDirSignature;

   mDiskNumStart = 0;

   mInternalFileAttr = 0;
   mExternalFileAttr = 0;

   mLocalHeadOffset = 0;

   mVersionMadeBy = 0;

   mFileComment = NULL;
}

CentralDir::~CentralDir()
{
   SAFE_DELETE_ARRAY(mFileComment);
}

//-----------------------------------------------------------------------------

bool CentralDir::read(Stream *stream)
{
   stream->read(&mHeaderSig);
   if(mHeaderSig != mCentralDirSignature)
      return false;

   stream->read(&mVersionMadeBy);
   stream->read(&mExtractVer);
   stream->read(&mFlags);
   stream->read(&mCompressMethod);
   stream->read(&mModTime);
   stream->read(&mModDate);
   stream->read(&mCRC32);
   stream->read(&mCompressedSize);
   stream->read(&mUncompressedSize);

   U16 fnLen, efLen, fcLen;
   stream->read(&fnLen);
   stream->read(&efLen);
   stream->read(&fcLen);

   stream->read(&mDiskNumStart);

   stream->read(&mInternalFileAttr);
   stream->read(&mExternalFileAttr);

   stream->read(&mLocalHeadOffset);

   char *fn = new char[fnLen + 1];
   stream->read(fnLen, fn);
   fn[fnLen] = 0;
   mFilename = String(fn);
   SAFE_DELETE_ARRAY(fn);
   

   // [tom, 10/28/2006] We currently only need the extra fields when we want to
   // open the file, so we won't bother reading them here. This avoids keeping
   // them in memory twice.

   //readExtraFields(stream, efLen);
   stream->setPosition(stream->getPosition() + efLen);

   fn = new char[fcLen + 1];
   stream->read(fcLen, fn);
   fn[fcLen] = 0;

   SAFE_DELETE_ARRAY(mFileComment);
   mFileComment = fn;

   // Sanity checks to make life easier elsewhere
   if(mCompressMethod != Stored && mUncompressedSize == 0 && mCompressedSize == 0)
      mCompressMethod = Stored;

   return true;
}

bool CentralDir::write(Stream *stream)
{
   mHeaderSig = mCentralDirSignature;
   stream->write(mHeaderSig);

   stream->write(mVersionMadeBy);
   stream->write(mExtractVer);
   stream->write(mFlags);
   stream->write(mCompressMethod);
   stream->write(mModTime);
   stream->write(mModDate);
   stream->write(mCRC32);
   stream->write(mCompressedSize);
   stream->write(mUncompressedSize);

   U16 fnLen = mFilename.length(),
       efLen = 0,
       fcLen = mFileComment ? (U16)dStrlen(mFileComment) : 0;
   stream->write(fnLen);
   stream->write(efLen);
   stream->write(fcLen);

   stream->write(mDiskNumStart);

   stream->write(mInternalFileAttr);
   stream->write(mExternalFileAttr);

   stream->write(mLocalHeadOffset);

   if(fnLen)
      stream->write(fnLen, mFilename);

   // FIXME [tom, 10/29/2006] Write extra fields here

   if(fcLen)
      stream->write(fcLen, mFileComment);

   return true;
}

//-----------------------------------------------------------------------------

void CentralDir::setFileComment(const char *comment)
{
   SAFE_DELETE_ARRAY(mFileComment);
   mFileComment = new char [dStrlen(comment)+1];
   dStrcpy(mFileComment, comment);
}

//-----------------------------------------------------------------------------
// EndOfCentralDir Class
//-----------------------------------------------------------------------------

EndOfCentralDir::EndOfCentralDir()
{
   mHeaderSig = mEOCDSignature;

   mDiskNum = 0;
   mStartCDDiskNum = 0;
   mNumEntriesInThisCD = 0;
   mTotalEntriesInCD = 0;
   mCDSize = 0;
   mCDOffset = 0;
   mCommentSize = 0;
   mZipComment = NULL;
}

EndOfCentralDir::~EndOfCentralDir()
{
   SAFE_DELETE_ARRAY(mZipComment);
}

//-----------------------------------------------------------------------------

bool EndOfCentralDir::read(Stream *stream)
{
   stream->read(&mHeaderSig);
   if(mHeaderSig != mEOCDSignature)
      return false;

   stream->read(&mDiskNum);
   stream->read(&mStartCDDiskNum);
   stream->read(&mNumEntriesInThisCD);
   stream->read(&mTotalEntriesInCD);
   stream->read(&mCDSize);
   stream->read(&mCDOffset);

   stream->read(&mCommentSize);
   
   char *comment = new char[mCommentSize + 1];
   stream->read(mCommentSize, comment);
   comment[mCommentSize] = 0;

   SAFE_DELETE_ARRAY(mZipComment);
   mZipComment = comment;

   return true;
}

bool EndOfCentralDir::write(Stream *stream)
{
   stream->write(mHeaderSig);

   stream->write(mDiskNum);
   stream->write(mStartCDDiskNum);
   stream->write(mNumEntriesInThisCD);
   stream->write(mTotalEntriesInCD);
   stream->write(mCDSize);
   stream->write(mCDOffset);

   stream->write(mCommentSize);
   if(mZipComment && mCommentSize)
      stream->write(mCommentSize, mZipComment);

   return true;
}

//-----------------------------------------------------------------------------

// [tom, 10/19/2006] I know, i know ... this'll get rewritten.
// [tom, 1/23/2007] Maybe.

bool EndOfCentralDir::findInStream(Stream *stream)
{
   U32 initialPos = stream->getPosition();
   U32 size = stream->getStreamSize();
   U32 pos;
   if(size == 0)
      return false;

   if(! stream->setPosition(size - mRecordSize))
      goto hell;

   U32 sig;
   stream->read(&sig);

   if(sig == mEOCDSignature)
   {
      stream->setPosition(size - mRecordSize);
      return true;
   }

   // OK, so we couldn't find the EOCD where we expected it. The zip file
   // either has comments or isn't a zip file. We need to search the last
   // 64Kb of the file for the EOCD.

   pos = size > mEOCDSearchSize ? size - mEOCDSearchSize : 0;
   if(! stream->setPosition(pos))
      goto hell;

   while(pos < (size - 4))
   {
      stream->read(&sig);

      if(sig == mEOCDSignature)
      {
         stream->setPosition(pos);
         return true;
      }

      pos++;
      if(! stream->setPosition(pos))
         goto hell;
   }

hell:
   stream->setPosition(initialPos);
   return false;
}

//-----------------------------------------------------------------------------

void EndOfCentralDir::setZipComment(U16 commentSize, const char *zipComment)
{
   SAFE_DELETE_ARRAY(mZipComment);
   mZipComment = new char [commentSize];
   dMemcpy((void *)mZipComment, zipComment, commentSize);
   mCommentSize = commentSize;
}

void EndOfCentralDir::setZipComment(const char *zipComment)
{
   setZipComment(dStrlen(zipComment), zipComment);
}

} // end namespace Zip
