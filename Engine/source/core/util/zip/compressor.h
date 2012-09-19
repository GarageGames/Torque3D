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

#include "core/util/zip/centralDir.h"

#ifndef _COMPRESSOR_H_
#define _COMPRESSOR_H_

// Forward refs
class Stream;

namespace Zip
{

/// @addtogroup zipint_group
/// @ingroup zip_group
// @{

// [tom, 10/26/2006] Standard Zip compression methods
enum CompressionMethod
{
   Stored            = 0,
   Shrunk            = 1,
   ReducedL1         = 2,
   ReducedL2         = 3,
   ReducedL3         = 4,
   ReducedL4         = 5,
   Imploded          = 6,
   ReservedTokenized = 7,
   Deflated          = 8,
   EnhDefalted       = 9,
   DateCompression   = 10,
   ReservedPKWARE    = 11,
   BZip2             = 12,
   PPMd              = 98,
   AESEncrypted      = 99,    // WinZip's AES Encrypted zips use compression method 99
                              // to indicate AES encryption. The actual compression
                              // method is specified in the AES extra field.
};

class Compressor
{
   Compressor *mNext;

protected:
   const char *mName;         //!< The name of the compression method
   S32 mMethod;               //!< The compression method as in the Zip header

public:
   Compressor(S32 method, const char *name);
   virtual ~Compressor() {}

   inline const char * getName()                { return mName; }
   inline S32 getMethod()                       { return mMethod; }

   virtual Stream *createReadStream(const CentralDir *cdir, Stream *zipStream) = 0;
   virtual Stream *createWriteStream(const CentralDir *cdir, Stream *zipStream) = 0;

   // Run time lookup methods
   static Compressor *findCompressor(const char *name);
   static Compressor *findCompressor(S32 method);
};

#define ImplementCompressor(name, method)       \
   class Compressor##name : public Compressor   \
   {                                            \
   public:                                      \
      Compressor##name(S32 m, const char *n) : Compressor(m, n) {} \
      virtual Stream *createReadStream(const CentralDir *cdir, Stream *zipStream); \
      virtual Stream *createWriteStream(const CentralDir *cdir, Stream *zipStream); \
   };                                           \
   Compressor##name gCompressor##name##instance(method, #name);

#define  CompressorCreateReadStream(name) \
   Stream * Compressor##name::createReadStream(const CentralDir *cdir, Stream *zipStream)

#define  CompressorCreateWriteStream(name) \
   Stream * Compressor##name::createWriteStream(const CentralDir *cdir, Stream *zipStream)

// @}

} // end namespace Zip

#endif // _COMPRESSOR_H_
