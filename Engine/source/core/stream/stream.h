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

#ifndef _STREAM_H_
#define _STREAM_H_

#ifndef _TORQUE_TYPES_H_
#include "platform/types.h"
#endif
#ifndef _ENDIAN_H_
#include "core/util/endian.h"
#endif


/// @defgroup stream_overload Primitive Type Stream Operation Overloads
/// These macros declare the read and write functions for all primitive types.
/// @{
#define DECLARE_OVERLOADED_READ(type)  bool read(type* out_read);
#define DECLARE_OVERLOADED_WRITE(type) bool write(type in_write);
/// @}

class ColorI;
class ColorF;
struct NetAddress;
class RawData;
class String;

namespace Torque {
   class ByteBuffer;
}

//------------------------------------------------------------------------------
//-------------------------------------- Base Stream class
//
/// Base stream class for streaming data across a specific media
class Stream
{
   // Public structs and enumerations...
public:
     /// Status constants for the stream
   enum Status {
      Ok = 0,           ///< Ok!
      IOError,          ///< Read or Write error
      EOS,              ///< End of Stream reached (mostly for reads)
      IllegalCall,      ///< An unsupported operation used.  Always w/ accompanied by AssertWarn
      Closed,           ///< Tried to operate on a closed stream (or detached filter)
      UnknownError      ///< Catchall
   };

   enum Capability {
      StreamWrite    = BIT(0), ///< Can this stream write?
      StreamRead     = BIT(1), ///< Can this stream read?
      StreamPosition = BIT(2)  ///< Can this stream position?
   };

   // Accessible only through inline accessors
private:
   Status m_streamStatus;

   // Derived accessible data modifiers...
protected:
   void setStatus(const Status in_newStatus) { m_streamStatus = in_newStatus; }

public:
   Stream();
   virtual ~Stream() {}

   /// Gets the status of the stream
   Stream::Status getStatus() const { return m_streamStatus; }
   /// Gets a printable string form of the status
   static const char* getStatusString(const Status in_status);

   // Derived classes must override these...
protected:
   virtual bool _read(const U32 in_numBytes,  void* out_pBuffer)      = 0;
   virtual bool _write(const U32 in_numBytes, const void* in_pBuffer) = 0;

   virtual void _write(const String & str);
   virtual void _read(String * str);


public:
     /// Checks to see if this stream has the capability of a given function
   virtual bool hasCapability(const Capability caps) const = 0;

   /// Gets the position in the stream
   virtual U32  getPosition() const                      = 0;
   /// Sets the position of the stream.  Returns if the new position is valid or not
   virtual bool setPosition(const U32 in_newPosition) = 0;
   /// Gets the size of the stream
   virtual U32  getStreamSize() = 0;

   /// Reads a line from the stream.
   /// @param buffer buffer to be read into
   /// @param bufferSize max size of the buffer.  Will not read more than the "bufferSize"
   void readLine(U8 *buffer, U32 bufferSize);
   /// writes a line to the stream
   void writeLine(const U8 *buffer);

   /// Reads a string and inserts it into the StringTable
   /// @see StringTable
   const char *readSTString(bool casesens = false);
   /// Reads a string of maximum 255 characters long
   virtual void readString(char stringBuf[256]);
   /// Reads a string that could potentially be more than 255 characters long.
   /// @param maxStringLen Maximum length to read.  If the string is longer than maxStringLen, only maxStringLen bytes will be read.
   /// @param stringBuf buffer where data is read into
   void readLongString(U32 maxStringLen, char *stringBuf);
   /// Writes a string to the stream.  This function is slightly unstable.
   /// Only use this if you have a valid string that is not empty.
   /// writeString is safer.
   void writeLongString(U32 maxStringLen, const char *string);

   /// Write raw text to the stream
   void writeText(const char *text);

   /// Writes a string to the stream.
   virtual void writeString(const char *stringBuf, S32 maxLen=255);

   // read/write real strings
   void write(const String & str) { _write(str); }
   void read(String * str) { _read(str); }

   /// Write an integral color to the stream.
   bool write(const ColorI&);
   /// Write a floating point color to the stream.
   bool write(const ColorF&);
   /// Read an integral color from the stream.
   bool read(ColorI*);
   /// Read a floating point color from the stream.
   bool read(ColorF*);

   /// Write a network address to the stream.
   bool write(const NetAddress &);
   /// Read a network address from the stream.
   bool read(NetAddress*);

   /// Write some raw data onto the stream.
   bool write(const RawData &);
   /// Read some raw data from the stream.
   bool read(RawData *);

   /// Write some raw data onto the stream.
   bool write(const Torque::ByteBuffer &);
   /// Read some raw data from the stream.
   bool read(Torque::ByteBuffer *);

   // Overloaded write and read ops..
  public:
   bool read(const U32 in_numBytes,  void* out_pBuffer) {
      return _read(in_numBytes, out_pBuffer);
   }
   bool write(const U32 in_numBytes, const void* in_pBuffer) {
      return _write(in_numBytes, in_pBuffer);
   }

   DECLARE_OVERLOADED_WRITE(S8)
   DECLARE_OVERLOADED_WRITE(U8)

   DECLARE_OVERLOADED_WRITE(S16)
   DECLARE_OVERLOADED_WRITE(S32)
   DECLARE_OVERLOADED_WRITE(U16)
   DECLARE_OVERLOADED_WRITE(U32)
   DECLARE_OVERLOADED_WRITE(U64)
   DECLARE_OVERLOADED_WRITE(F32)
   DECLARE_OVERLOADED_WRITE(F64)

   DECLARE_OVERLOADED_READ(S8)
   DECLARE_OVERLOADED_READ(U8)

   DECLARE_OVERLOADED_READ(S16)
   DECLARE_OVERLOADED_READ(S32)
   DECLARE_OVERLOADED_READ(U16)
   DECLARE_OVERLOADED_READ(U32)
   DECLARE_OVERLOADED_READ(U64)
   DECLARE_OVERLOADED_READ(F32)
   DECLARE_OVERLOADED_READ(F64)

   // We have to do the bool's by hand, since they are different sizes
   //  on different compilers...
   //
   bool read(bool* out_pRead) {
      U8 translate;
      bool success = read(&translate);
      if (success == false)
         return false;

      *out_pRead = translate != 0;
      return true;
   }
   bool write(const bool& in_rWrite) {
      U8 translate = in_rWrite ? U8(1) : U8(0);
      return write(translate);
   }

   /// Copy the contents of another stream into this one
   bool copyFrom(Stream *other);

   /// Write a number of tabs to this stream
   void writeTabs(U32 count)
   {
      char tab[] = "   ";
      while(count--)
         write(3, (void*)tab);
   }

   /// Create an exact replica of this stream.
   /// Return NULL if the cloning fails.
   virtual Stream* clone() const;
};

// This interface is used to provide the amount of bytes actually written/read when using the stream as a 
// file.  The Stream interface does not provide this information.  This is a lame workaround.  
class IStreamByteCount
{
public:
   virtual ~IStreamByteCount() {}

   virtual U32 getLastBytesRead() = 0;
   virtual U32 getLastBytesWritten() = 0;
};

#endif //_STREAM_H_
