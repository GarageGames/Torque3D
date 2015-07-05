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

#ifndef _ASYNCPACKETSTREAM_H_
#define _ASYNCPACKETSTREAM_H_

#ifndef _ASYNCBUFFEREDSTREAM_H_
   #include "platform/async/asyncBufferedStream.h"
#endif
#ifndef _RAWDATA_H_
   #include "core/util/rawData.h"
#endif
#ifndef _THREADPOOLASYNCIO_H_
   #include "platform/threads/threadPoolAsyncIO.h"
#endif


//#define DEBUG_SPEW


/// @file
/// Input stream filter definitions for turning linear streams into
/// streams that yield data in discrete packets using background
/// reads.


//--------------------------------------------------------------------------
//    Async stream packets.
//--------------------------------------------------------------------------

/// Stream packet read by an asynchronous packet stream.
template< typename T >
class AsyncPacket : public RawDataT< T >
{
   public:

      typedef RawDataT< T > Parent;

      AsyncPacket()
         : mIndex( 0 ), mSizeActual( 0 ), mIsLast( false ) {}
      AsyncPacket( T* data, U32 size, bool ownMemory = false )
         : Parent( data, size, ownMemory ),
           mIndex( 0 ), mSizeActual( 0 ), mIsLast( false ) {}

      /// Running number in stream.
      U32 mIndex;

      /// Number of items that have actually been read into the packet.
      /// This may be less than "size" for end-of-stream packets in non-looping
      /// streams.
      ///
      /// @note Extraneous space at the end of the packet will be cleared using
      ///   constructArray() calls.
      U32 mSizeActual;

      /// If true this is the last packet in the stream.
      bool mIsLast;
};

//--------------------------------------------------------------------------
//    Async packet streams.
//--------------------------------------------------------------------------

/// A packet stream turns a continuous stream of elements into a
/// stream of discrete packets of elements.
///
/// All packets are of the exact same size even if, for end-of-stream
/// packets, they actually contain less data than their actual size.
/// Extraneous space is cleared.
///
/// @note For looping streams, the stream must implement the
///   IResettable interface.
template< typename Stream, class Packet = AsyncPacket< typename TypeTraits< Stream >::BaseType::ElementType > >
class AsyncPacketBufferedInputStream : public AsyncBufferedInputStream< Packet*, Stream >
{
   public:

      typedef AsyncBufferedInputStream< Packet*, Stream > Parent;
      typedef Packet PacketType;
      typedef typename TypeTraits< Stream >::BaseType StreamType;

   protected:

      class PacketReadItem;
      friend class PacketReadItem; // _onArrival

      /// Asynchronous work item for reading a packet from the source stream.
      class PacketReadItem : public AsyncReadItem< typename Parent::SourceElementType, StreamType >
      {
         public:

            typedef AsyncReadItem< typename AsyncPacketBufferedInputStream< Stream, Packet >::SourceElementType, StreamType > Parent;

            PacketReadItem( const ThreadSafeRef< AsyncPacketBufferedInputStream< Stream, Packet > >& asyncStream,
                            PacketType* packet,
                            U32 numElements,
                            ThreadPool::Context* context = NULL )
               : Parent( asyncStream->getSourceStream(), numElements, 0, *packet, false, 0, context ),
                 mAsyncStream( asyncStream ),
                 mPacket( packet ) {}

         protected:

            typedef ThreadSafeRef< AsyncPacketBufferedInputStream< Stream, Packet > > AsyncPacketStreamPtr;

            /// The issueing async state.
            AsyncPacketStreamPtr mAsyncStream;

            /// The packet that receives the data.
            PacketType* mPacket;

            // WorkItem
            virtual void execute()
            {
               Parent::execute();
               mPacket->mSizeActual += this->mNumElementsRead;
               
               #ifdef DEBUG_SPEW
               Platform::outputDebugString( "[AsyncPacketStream] read %i elements into packet #%i with size %i",
                  this->mNumElementsRead, mPacket->mIndex, mPacket->size );
               #endif

               // Handle extraneous space at end of packet.

               if( this->cancellationPoint() ) return;
               U32 numExtraElements = mPacket->size - this->mNumElementsRead;
               if( numExtraElements )
               {                  
                  if( mAsyncStream->mIsLooping
                      && dynamic_cast< IResettable* >( &Deref( this->getStream() ) ) )
                  {
                     #ifdef DEBUG_SPEW
                     Platform::outputDebugString( "[AsyncPacketStream] resetting stream and reading %i more elements", numExtraElements );
                     #endif

                     // Wrap around and start re-reading from beginning of stream.

                     dynamic_cast< IResettable* >( &Deref( this->getStream() ) )->reset();
                     
                     this->mOffsetInBuffer += this->mNumElementsRead;
                     this->mOffsetInStream = 0;
                     this->mNumElements = numExtraElements;

                     this->_prep();
                     Parent::execute();
                     
                     mPacket->mSizeActual += this->mNumElementsRead;
                  }
                  else
                     constructArray( &mPacket->data[ this->mNumElementsRead ], numExtraElements );
               }

               // Buffer the packet.

               if( this->cancellationPoint() ) return;
               mAsyncStream->_onArrival( mPacket );
            }
            virtual void onCancelled()
            {
               Parent::onCancelled();
               destructSingle< PacketType* >( mPacket );
               mAsyncStream = NULL;
            }
      };

      typedef ThreadSafeRef< PacketReadItem > PacketReadItemRef;

      /// Number of elements to read per packet.
      U32 mPacketSize;

      /// Running number of next stream packet.
      U32 mNextPacketIndex;
            
      /// Total number of elements in the source stream.
      U32 mNumTotalSourceElements;

      /// Create a new stream packet of the given size.
      virtual PacketType* _newPacket( U32 packetSize ) { return constructSingle< PacketType* >( packetSize ); }

      /// Request the next packet from the underlying stream.
      virtual void _requestNext();

      /// Create a new work item that reads "numElements" into "packet".
      virtual void _newReadItem( PacketReadItemRef& outRef,
                                 PacketType* packet,
                                 U32 numElements )
      {
         outRef = new PacketReadItem( this, packet, numElements, this->mThreadContext );
      }

   public:

      /// Construct a new packet stream reading from "stream".
      ///
      /// @note If looping is used and "stream" is not read from the beginning, "stream" should
      ///   implement IPositionable<U32> or ISizeable<U32> so the async stream can tell how many elements
      ///   there actually are in the stream after resetting.
      ///
      /// @param stream The source stream from which to read the actual data elements.
      /// @param packetSize Size of stream packets returned by the stream in number of elements.
      /// @param numSourceElementsToRead Number of elements to read from "stream".
      /// @param numReadAhead Number of packets to read and buffer in advance.
      /// @param isLooping If true, the packet stream will loop infinitely over the source stream.
      /// @param pool The ThreadPool to use for asynchronous packet reads.
      /// @param context The ThreadContext to place asynchronous packet reads in.
      AsyncPacketBufferedInputStream( const Stream& stream,
                              U32 packetSize,
                              U32 numSourceElementsToRead = 0,
                              U32 numReadAhead = Parent::DEFAULT_STREAM_LOOKAHEAD,
                              bool isLooping = false,
                              ThreadPool* pool = &ThreadPool::GLOBAL(),
                              ThreadContext* context = ThreadContext::ROOT_CONTEXT() );
      
      /// @return the size of stream packets returned by this stream in number of elements.
      U32 getPacketSize() const { return mPacketSize; }
};

template< typename Stream, class Packet >
AsyncPacketBufferedInputStream< Stream, Packet >::AsyncPacketBufferedInputStream
         (  const Stream& stream,
            U32 packetSize,
            U32 numSourceElementsToRead,
            U32 numReadAhead,
            bool isLooping,
            ThreadPool* threadPool,
            ThreadContext* threadContext )
   : Parent( stream, numSourceElementsToRead, numReadAhead, isLooping, threadPool, threadContext ),
     mPacketSize( packetSize ),
     mNumTotalSourceElements( numSourceElementsToRead ),
     mNextPacketIndex( 0 )
{
   AssertFatal( mPacketSize > 0,
      "AsyncPacketStream::AsyncPacketStream() - packet size cannot be zero" );
      
   // Determine total number of elements in stream, if possible.
      
   IPositionable< U32 >* positionable = dynamic_cast< IPositionable< U32 >* >( &Deref( stream ) );
   if( positionable )
      mNumTotalSourceElements += positionable->getPosition();
   else
   {
      ISizeable< U32 >* sizeable = dynamic_cast< ISizeable< U32 >* >( &Deref( stream ) );
      if( sizeable )
         mNumTotalSourceElements = sizeable->getSize();
   }
   
   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[AsyncPacketStream] %i remaining, %i total (%i packets)",
      this->mNumRemainingSourceElements, mNumTotalSourceElements,
      ( this->mNumRemainingSourceElements / mPacketSize ) + ( this->mNumRemainingSourceElements % mPacketSize ? 1 : 0 ) );
   #endif
}

template< typename Stream, class Packet >
void AsyncPacketBufferedInputStream< Stream, Packet >::_requestNext()
{
   Stream& stream = this->getSourceStream();
   bool isEOS = !this->mNumRemainingSourceElements;
   if( isEOS && this->mIsLooping )
   {
      StreamType* s = &Deref( stream );
      IResettable* resettable = dynamic_cast< IResettable* >( s );
      if( resettable )
      {
         resettable->reset();
         isEOS = false;
         this->mNumRemainingSourceElements = mNumTotalSourceElements;
      }
   }
   else if( isEOS )
      return;

   //TODO: scale priority depending on feed status

   // Allocate a packet.

   U32 numElements = mPacketSize;
   PacketType* packet = _newPacket( numElements );
   packet->mIndex = mNextPacketIndex;
   mNextPacketIndex ++;

   // Queue a stream packet work item.

   if( numElements >= this->mNumRemainingSourceElements )
   {
      if( !this->mIsLooping )
      {
         this->mNumRemainingSourceElements = 0;
         packet->mIsLast = true;
      }
      else
         this->mNumRemainingSourceElements = ( this->mNumTotalSourceElements - numElements + this->mNumRemainingSourceElements );
   }
   else
      this->mNumRemainingSourceElements -= numElements;
      
   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[AsyncPacketStream] packet %i, %i remaining, %i total",
      packet->mIndex, this->mNumRemainingSourceElements, mNumTotalSourceElements );
   #endif

   ThreadSafeRef< PacketReadItem > workItem;
   _newReadItem( workItem, packet, numElements );
   this->mThreadPool->queueWorkItem( workItem );
}

#undef DEBUG_SPEW
#endif // !_ASYNCPACKETSTREAM_H_
