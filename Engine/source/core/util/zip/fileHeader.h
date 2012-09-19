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

#include "core/util/zip/extraField.h"
#include "core/util/tVector.h"

#ifndef _FILEHEADER_H_
#define _FILEHEADER_H_

// Forward Refs
class Stream;

namespace Zip
{

/// @addtogroup zipint_group
/// @ingroup zip_group
// @{

enum FileFlags
{
   Encrypted = BIT(0),

   // For implode compression
   Implode8KDictionary = BIT(1),
   Implode3ShannonFanoTrees = BIT(2),

   // For deflate compression
   DeflateTypeMask = BIT(1) | BIT(2),

   FileInfoInDirectory = BIT(3),

   // Note that much of the following flag bits are unsupported for various reasons
   ReservedEnhDeflate = BIT(4),
   PatchData = BIT(5),
   StrongEncryption = BIT(6),

   UnusedReserved1 = BIT(7),
   UnusedReserved2 = BIT(8),
   UnusedReserved3 = BIT(9),
   UnusedReserved4 = BIT(10),
   UnusedReserved5 = BIT(11),

   ReservedPKWARE1 = BIT(12),

   EncryptedDirectory = BIT(13),

   ReservedPKWARE2 = BIT(14),
   ReservedPKWARE3 = BIT(15),
};

class FileHeader
{
   static const U32 mFileHeaderSignature = 0x04034b50;

protected:
   bool readExtraFields(Stream *stream, U16 efLen);

public:
   U32 mHeaderSig;

   U16 mExtractVer;
   U16 mFlags;
   U16 mCompressMethod;

   U16 mModTime;
   U16 mModDate;

   U32 mCRC32;

   U32 mCompressedSize;
   U32 mUncompressedSize;

   String mFilename;

   Vector<ExtraField *> mExtraFields;

   FileHeader();
   virtual ~FileHeader();

   virtual bool read(Stream *stream);
   virtual bool write(Stream *stream);

   ExtraField *findExtraField(U16 id);

   void setFilename(String filename);
};

// @}

} // end namespace Zip

#endif
