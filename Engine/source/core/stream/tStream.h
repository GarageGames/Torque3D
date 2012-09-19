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

#ifndef _TSTREAM_H_
#define _TSTREAM_H_

#ifndef _PLATFORM_H_
   #include "platform/platform.h"
#endif
#ifndef _TYPETRAITS_H_
   #include "platform/typetraits.h"
#endif


//TODO: error handling


/// @file
/// Definitions for lightweight componentized streaming.
///
/// This file is an assembly of lightweight classes/interfaces that
/// describe various aspects of streaming classes.  The advantage
/// over using Torque's Stream class is that very little requirements
/// are placed on implementations, that specific abilities can be
/// mixed and matched very selectively, and that complex stream processing
/// chains can be hidden behind very simple stream interfaces.



/// Status of an asynchronous I/O operation.
enum EAsyncIOStatus
{
   ASYNC_IO_Pending,       ///< I/O is still in queue or being processed.
   ASYNC_IO_Complete,      ///< I/O has completed successfully.
   ASYNC_IO_Error          ///< I/O has aborted with an error.
};

//-----------------------------------------------------------------------------
//    Several component interfaces.
//-----------------------------------------------------------------------------

/// Interface for streams with an explicit position property.
template< typename P = U32 >
class IPositionable
{
   public:
   
      typedef void Parent;

      /// The type used to indicate positions.
      typedef P PositionType;

      /// @return the current position.
      virtual PositionType getPosition() const = 0;
      
      /// Set the current position to be "pos".
      /// @param pos The new position.
      virtual void setPosition( PositionType pos ) = 0;
};

/// Interface for structures that allow their state to be reset.
class IResettable
{
   public:
   
      typedef void Parent;
   
      /// Reset to the initial state.
      virtual void reset() = 0;
};

/// Interface for structures of finite size.
template< typename T = U32 >
class ISizeable
{
   public:
   
      typedef void Parent;

      /// The type used to indicate the structure's size.
      typedef T SizeType;

      /// @return the size of the structure in number of elements.
      virtual SizeType getSize() const = 0;
};

/// Interface for structures that represent processes.
class IProcess
{
   public:
   
      typedef void Parent;

      /// Start the process.
      virtual void start() = 0;

      /// Stop the process.
      virtual void stop() = 0;

      /// Pause the process.
      virtual void pause() = 0;
};

/// Interface for objects that need continuous explicit updates from
/// an outside source.
class IPolled
{
   public:

      typedef void Parent;
   
      /// Update the object state.
      virtual bool update() = 0;
};


//-----------------------------------------------------------------------------
//    IInputStream.
//-----------------------------------------------------------------------------

/// An input stream delivers a sequence of elements of type "T".
///
/// @note This stream has an inherent position property and is thus not
///   safe for concurrent access.
template< typename T >
class IInputStream
{
   public:

      typedef void Parent;

      /// The element type of this input stream.
      typedef T ElementType;
      
      /// Read the next "num" elements into "buffer".
      ///
      /// @param buffer The buffer into which the elements are stored.
      /// @param num Number of elements to read.
      /// @return the number of elements actually read; this may be less than
      ///   "num" or even zero if no elements are available or reading failed.
      virtual U32 read( ElementType* buffer, U32 num ) = 0;
};

/// An input stream over elements of type "T" that reads from
/// user-specified explicit offsets.
template< typename T, typename Offset = U32 >
class IOffsetInputStream
{
   public:

      typedef void Parent;
      typedef Offset OffsetType;
      typedef T ElementType;

      /// Read the next "num" elements at "offset" into "buffer".
      ///
      /// @param offset The offset in the stream from which to read.
      /// @param buffer The buffer into which the elements are stored.
      /// @param num Number of elements to read.
      /// @return the number of elements actually read; this may be less than
      ///   "num" or even zero if no elements are available or reading failed.
      virtual U32 readAt( OffsetType offset, T* buffer, U32 num ) = 0;
};

/// An input stream over elements of type "T" that works in
/// the background.
template< typename T, typename Offset = U32 >
class IAsyncInputStream
{
   public:

      typedef void Parent;
      typedef Offset OffsetType;
      typedef T ElementType;

      /// Queue a read of "num" elements at "offset" into "buffer".
      ///
      /// @param offset The offset in the stream from which to read.
      /// @param buffer The buffer into which the elements are stored.
      /// @param num Number of elements to read.
      /// @return a handle for the asynchronous read operation or NULL if the
      ///   operation could not be queued.
      virtual void* issueReadAt( OffsetType offset, T* buffer, U32 num ) = 0;

      /// Try moving the given asynchronous read operation to ASYNC_IO_Complete.
      ///
      /// @note This method invalidates the given handle.
      ///
      /// @param handle Handle returned by "issueReadAt".
      /// @param outNumRead Reference that receives the number of bytes actually read in the
      ///   operation.
      /// @param wait If true, the method waits until the given operation either fails or
      ///   completes successfully before returning.
      /// @return the final operation status.
      virtual EAsyncIOStatus tryCompleteReadAt( void* handle, U32& outNumRead, bool wait = false ) = 0;

      /// Cancel the given asynchronous read operation.
      ///
      /// @note This method invalidates the given handle.
      ///
      /// @param handle Handle returned by "issueReadAt".
      virtual void cancelReadAt( void* handle ) = 0;
};

//-----------------------------------------------------------------------------
//    IOutputStream.
//-----------------------------------------------------------------------------

/// An output stream that writes elements of type "T".
///
/// @note This stream has an inherent position property and is thus not
///   safe for concurrent access.
template< typename T >
class IOutputStream
{
   public:

      typedef void Parent;      

      /// The element type of this input stream.
      typedef T ElementType;

      /// Write "num" elements from "buffer" to the stream at its
      /// current position.
      ///
      /// @param buffer The buffer from which to read elements.
      /// @param num Number of elements to write.
      virtual void write( const ElementType* buffer, U32 num ) = 0;
};

/// An output stream that writes elements of type "T" to a
/// user-specified explicit offset.
template< typename T, typename Offset = U32 >
class IOffsetOutputStream
{
   public:

      typedef void Parent;
      typedef Offset OffsetType;
      typedef T ElementType;

      /// Write "num" elements from "buffer" to the stream at "offset".
      ///
      /// @param offset The offset in the stream at which to write the elements.
      /// @param buffer The buffer from which to read elements.
      /// @param num Number of elements to write.
      virtual void writeAt( OffsetType offset, const ElementType* buffer, U32 num ) = 0;
};

/// An output stream that writes elements of type "T" in the background.
template< typename T, typename Offset = U32 >
class IAsyncOutputStream
{
   public:

      typedef void Parent;
      typedef Offset OffsetType;
      typedef T ElementType;

      /// Queue a write operation of "num" elements from "buffer" to stream position
      /// "offset".
      ///
      /// @param offset The offset in the stream at which to write the elements.
      /// @param buffer The buffer from which to read elements.
      /// @param num The number of elements to write.
      /// @return a handle to the asynchronous write operatior or NULL if the operation
      ///   could not be queued.
      virtual void* issueWriteAt( OffsetType offset, const ElementType* buffer, U32 num ) = 0;

      /// Try moving the given asynchronous write operation to ASYNC_IO_Complete.
      ///
      /// @note This method invalidates the given handle.
      ///
      /// @param handle Handle returned by "issueWriteAt".
      /// @param wait If true, the method waits until the given operation either fails or
      ///   completes successfully before returning.
      /// @return the final operation status.
      virtual EAsyncIOStatus tryCompleteWriteAt( void* handle, bool wait = false ) = 0;

      /// Cancel the given asynchronous write operation.
      ///
      /// @note This method invalidates the given handle.
      ///
      /// @param handle Handle return by "issueWriteAt".
      virtual void cancelWriteAt( void* handle ) = 0;
};

//-----------------------------------------------------------------------------
//    IInputStreamFilter.
//-----------------------------------------------------------------------------

/// An input stream filter takes an input stream "Stream" and processes it
/// into an input stream over type "To".
template< typename To, typename Stream >
class IInputStreamFilter : public IInputStream< To >
{
   public:

      typedef IInputStream< To > Parent;

      ///
      typedef typename TypeTraits< Stream >::BaseType SourceStreamType;

      /// The element type of the source stream.
      typedef typename SourceStreamType::ElementType SourceElementType;

      /// Construct a filter reading elements from "stream".
      IInputStreamFilter( const Stream& stream )
         : mSourceStream( stream ) {}

      /// Return the stream from which this filter is reading its
      /// source elements.
      const Stream& getSourceStream() const { return mSourceStream; }
      Stream& getSourceStream() { return mSourceStream; }

   private:

      Stream mSourceStream;
};

//-----------------------------------------------------------------------------
//    IOutputStreamFilter.
//-----------------------------------------------------------------------------

/// An output stream filter takes an output stream "Stream" and processes it
/// into an output stream over type "To".
template< typename To, class Stream >
class IOutputStreamFilter : public IOutputStream< To >
{
   public:

      typedef IOutputStream< To > Parent;

      ///
      typedef typename TypeTraits< Stream >::BaseType TargetStreamType;

      /// The element type of the target stream.
      typedef typename TargetStreamType::ElementType TargetElementType;

      /// Construct a filter writing elements to "stream".
      IOutputStreamFilter( const Stream& stream )
         : mTargetStream( stream ) {}

      /// Return the stream to which this filter is writing its
      /// elements.
      const Stream& getTargetStream() const { return mTargetStream; }
      Stream& getTargetStream() { return mTargetStream; }

   private:

      Stream mTargetStream;
};

#endif // _TSTREAM_H_
