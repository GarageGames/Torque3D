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

#ifndef _THREADPOOLASYNCIO_H_
#define _THREADPOOLASYNCIO_H_

#ifndef _THREADPOOL_H_
#  include "platform/threads/threadPool.h"
#endif
#ifndef _RAWDATA_H_
#  include "core/util/rawData.h"
#endif
#ifndef _TSTREAM_H_
#  include "core/stream/tStream.h"
#endif


//RDTODO: I/O error handling

/// @file
/// Thread pool work items for asynchronous stream I/O.
/// Through the use of stream filters, this can be basically used for any
/// type of asynchronous stream processing.

//--------------------------------------------------------------------------
//    AsyncIOItem.
//--------------------------------------------------------------------------

/// Abstract superclass of async I/O work items.
///
/// Supports both offset-based stream I/O as well as I/O on streams with
/// implicit positions.  Note that if you use the latter type, make sure
/// that no other thread is messing with the stream at the same time or
/// chaos will ensue.
///
/// @param T Type of elements being streamed.
template< typename T, class Stream >
class AsyncIOItem : public ThreadPool::WorkItem
{
   public:

      typedef WorkItem Parent;
      typedef T ValueType;
      typedef RawDataT< ValueType > BufferType;
      typedef U32 OffsetType;
      typedef Stream StreamType;

   protected:

      /// Buffer keeping/receiving the data elements.
      BufferType mBuffer;
      
      /// The stream to read from/write to.
      StreamType* mStream;

      /// Number of elements to read from/write to the stream.
      U32 mNumElements;

      /// Offset in "mBuffer" from where to read/where to start writing to.
      U32 mOffsetInBuffer;

      /// Offset in stream from where to read/where to write to.
      /// @note This is only meaningful if the stream is an offset I/O
      ///   stream.  For a stream that is can do both types of I/O,
      ///   explicit offsets are preferred and this value is used.
      OffsetType mOffsetInStream;

      ///
      ValueType* getBufferPtr()
      {
         return &getBuffer().data[ getOffsetInBuffer() ]; 
      }

   public:
   
      ///
      /// If the stream uses implicit positioning, then the supplied "offsetInStream"
      /// is meaningless and ignored.
      AsyncIOItem( StreamType* stream, U32 numElements, OffsetType offsetInStream,
                   ThreadContext* context = 0 )
         : Parent( context ),
           mStream( stream ),
           mNumElements( numElements ),
           mOffsetInBuffer( 0 ),
           mOffsetInStream( offsetInStream ) {}

      /// Construct a read item on "stream" that stores data into the given "buffer".
      ///
      AsyncIOItem( StreamType* stream, BufferType& buffer, U32 offsetInBuffer,
                   U32 numElements, OffsetType offsetInStream, bool takeOwnershipOfBuffer = true,
                   ThreadContext* context = 0 )
         : Parent( context ),
           mBuffer( buffer ),
           mStream( stream ),
           mNumElements( numElements ),
           mOffsetInBuffer( offsetInBuffer ),
           mOffsetInStream( offsetInStream )
      {
         if( takeOwnershipOfBuffer )
            mBuffer.ownMemory = true;
      }

      /// Return the stream being written to/read from.
      StreamType* getStream()
      {
         return mStream;
      }

      /// Return the data buffer being written to/read from.
      /// @note This may not yet have been allocated.
      BufferType& getBuffer()
      {
         return mBuffer;

      }

      /// Return the number of elements involved in the transfer.
      U32 getNumElements()
      {
         return mNumElements;
      }

      /// Return the position in the data buffer at which to start the transfer.
      U32 getOffsetInBuffer()
      {
         return mOffsetInBuffer;
      }

      /// Return the position in the stream at which to start the transfer.
      /// @note Only meaningful for streams that support offset I/O.
      OffsetType getOffsetInStream()
      {
         return mOffsetInStream;
      }
};

//--------------------------------------------------------------------------
//    AsyncReadItem.
//--------------------------------------------------------------------------

//RDTODO: error handling
/// Work item to asynchronously read from a stream.
///
/// The given stream type may implement any of the input stream
/// interfaces.  Preference is given to IAsyncInputStream, then to
/// IOffsetInputStream, and only if none of these are implemented
/// IInputStream is used.
///
/// For IAsyncInputStreams, the async read operation is issued immediately
/// on the constructing thread and then picked up on the worker thread.
/// This ensures optimal use of concurrency.

template< typename T, class Stream = IOffsetInputStream< T > >
class AsyncReadItem : public AsyncIOItem< T, Stream >
{
   public:

      typedef AsyncIOItem< T, Stream > Parent;
      typedef typename Parent::StreamType StreamType;
      typedef typename Parent::OffsetType OffsetType;
      typedef typename Parent::BufferType BufferType;
      typedef typename Parent::ValueType ValueType;

      /// Construct a read item that reads "numElements" at "offsetInStream"
      /// from "stream".
      ///
      /// Since with this constructor no data buffer is supplied, it will be
      /// dynamically allocated by the read() method.  Note that this only makes
      /// sense if this class is subclassed and processing is done on the buffer
      /// after it has been read.
      ///
      /// @param stream The stream to read from.
      /// @param numElement The number of elements to read from the stream.
      /// @param offsetInStream The offset at which to read from the stream;
      ///   ignored if the stream uses implicit positioning
      /// @param context The tread pool context to place the item into.
      AsyncReadItem( StreamType* stream, U32 numElements, OffsetType offsetInStream,
                     ThreadContext* context = 0 )
         : Parent( stream, numElements, offsetInStream, context )
      {
         _prep();
      }

      AsyncReadItem( StreamType* stream, U32 numElements, OffsetType offsetInStream,
                     BufferType& buffer, bool takeOwnershipOfBuffer = false,
                     U32 offsetInBuffer = 0, ThreadContext* context = 0 )
         : Parent( stream, buffer, offsetInBuffer, numElements, offsetInStream, takeOwnershipOfBuffer, context )
      {
         _prep();
      }

      /// @return The number of elements actually read from the stream.
      U32 getNumElementsRead()
      {
         return mNumElementsRead;
      }

   protected:

      /// Handle of asynchronous stream read, if we are using an async interface.
      void* mAsyncHandle;

      /// After the read operation has completed, this holds the number of
      /// elements actually read from the stream.
      U32 mNumElementsRead;

      virtual void execute();
      
      void _allocBuffer()
      {
         if( !this->getBuffer().data )
            this->getBuffer().alloc( this->getNumElements() );
      }

      void _prep()
      {
         IAsyncInputStream< T >* s = dynamic_cast< IAsyncInputStream< T >* >( this->getStream() );
         if( s )
         {
            _allocBuffer();
            mAsyncHandle = s->issueReadAt( this->getOffsetInStream(), this->getBufferPtr(), this->getNumElements() );
         }
      }

      // Helper functions to differentiate between stream types.

      void _read( IInputStream< T >* stream )
      {
         mNumElementsRead = stream->read( this->getBufferPtr(), this->getNumElements() );
      }
      void _read( IOffsetInputStream< T >* stream )
      {
         mNumElementsRead = stream->readAt( this->getOffsetInStream(), this->getBufferPtr(), this->getNumElements() );
      }
      void _read( IAsyncInputStream< T >* stream )
      {
         stream->tryCompleteReadAt( mAsyncHandle, mNumElementsRead, true );
      }
};

template< typename T, class Stream >
void AsyncReadItem< T, Stream >::execute()
{
   _allocBuffer();
   
   // Read the data.  Do a dynamic cast for any of the
   // interfaces we prefer.

   if( this->cancellationPoint() ) return;
   StreamType* stream = this->getStream();
   if( dynamic_cast< IAsyncInputStream< T >* >( stream ) )
      _read( ( IAsyncInputStream< T >* ) stream );
   else if( dynamic_cast< IOffsetInputStream< T >* >( stream ) )
      _read( ( IOffsetInputStream< T >* ) stream );
   else
      _read( stream );
}

//--------------------------------------------------------------------------
//    AsyncWriteItem.
//--------------------------------------------------------------------------

/// Work item for writing to an output stream.
///
/// The stream being written to may implement any of the given output stream
/// interfaces.  Preference is given to IAsyncOutputStream, then to
/// IOffsetOutputStream, and only if none of these is implemented IOutputStream
/// is used.
///
/// A useful feature is to yield ownership of the data buffer to the
/// write item.  This way, this can be pretty much used in a fire-and-forget
/// manner where after submission, no further synchronization happens
/// between the client and the work item.
///
/// @note Be aware that if writing to an output stream that has an implicit
///   position property, multiple concurrent writes will interfere with each other.
template< typename T, class Stream = IOffsetOutputStream< T > >
class AsyncWriteItem : public AsyncIOItem< T, Stream >
{
   public:

      typedef AsyncIOItem< T, Stream > Parent;
      typedef typename Parent::StreamType StreamType;
      typedef typename Parent::OffsetType OffsetType;
      typedef typename Parent::BufferType BufferType;
      typedef typename Parent::ValueType ValueType;

      AsyncWriteItem( StreamType* stream, U32 numElements, OffsetType offsetInStream,
                      BufferType& buffer, bool takeOwnershipOfBuffer = true,
                      U32 offsetInBuffer = 0, ThreadContext* context = 0 )
         : Parent( stream, buffer, offsetInBuffer, numElements, offsetInStream, takeOwnershipOfBuffer, context )
      {
         _prep( stream );
      }

   protected:

      /// Handle of asynchronous write operation, if the stream implements IAsyncOutputStream.
      void* mAsyncHandle;
      
      virtual void execute();

      void _prep( StreamType* stream )
      {
         IAsyncOutputStream< T >* s = dynamic_cast< IAsyncOutputStream< T >* >( stream );
         if( s )
            mAsyncHandle = s->issueWriteAt( this->getOffset(), this->getBufferPtr(), this->getNumElements() );
      }

      void _write( IOutputStream< T >* stream )
      {
         stream->write( this->getBufferPtr(), this->getNumElements() );
      }
      void _write( IOffsetOutputStream< T >* stream )
      {
         stream->writeAt( this->getOffsetInStream(), this->getBufferPtr(), this->getNumElements() );
      }
      void _write( IAsyncOutputStream< T >* stream )
      {
         stream->tryCompleteWriteAt( mAsyncHandle, true );
      }
};

template< typename T, class Stream >
void AsyncWriteItem< T, Stream >::execute()
{
   if( this->cancellationPoint() ) return;

   StreamType* stream = this->getStream();
   if( dynamic_cast< IAsyncOutputStream< T >* >( stream ) )
      _write( ( IAsyncOutputStream< T >* ) stream );
   if( dynamic_cast< IOffsetOutputStream< T >* >( stream ) )
      _write( ( IOffsetOutputStream< T >* ) stream );
   else
      _write( stream );
}

#endif // _THREADPOOLASYNCIO_H_
