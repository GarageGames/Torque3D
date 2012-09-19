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

#ifndef _ASYNCBUFFEREDSTREAM_H_
#define _ASYNCBUFFEREDSTREAM_H_

#ifndef _TSTREAM_H_
   #include "core/stream/tStream.h"
#endif
#ifndef _THREADPOOL_H_
   #include "platform/threads/threadPool.h"
#endif
#ifndef _THREADSAFEDEQUE_H_
   #include "platform/threads/threadSafeDeque.h"
#endif


// Disable nonsense warning about unreferenced
// local function on VC.
#ifdef TORQUE_COMPILER_VISUALC
   #pragma warning( disable: 4505 )
#endif


template< typename T, class Stream >
class AsyncBufferedReadItem;



//=============================================================================
//    AsyncBufferedInputStream.
//=============================================================================


///
template< typename T, class Stream = IInputStream< T >* >
class AsyncBufferedInputStream : public IInputStreamFilter< T, Stream >,
                                 public ThreadSafeRefCount< AsyncBufferedInputStream< T, Stream > >
{
   public:

      typedef IInputStreamFilter< T, Stream > Parent;
      
      /// Type of elements read, buffered, and returned by this stream.
      typedef typename Parent::ElementType ElementType;
      
      /// Type of the source stream being read by this stream.
      typedef typename Parent::SourceStreamType SourceStreamType;
      
      /// Type of elements being read from the source stream.
      ///
      /// @note This does not need to correspond to the type of elements buffered
      ///   in this stream.
      typedef typename Parent::SourceElementType SourceElementType;

      enum
      {
         /// The number of elements to buffer in advance by default.
         DEFAULT_STREAM_LOOKAHEAD = 3
      };
      
      friend class AsyncBufferedReadItem< T, Stream >; // _onArrival

   protected:

      /// Stream elements are kept on deques that can be concurrently
      /// accessed by multiple threads.
      typedef ThreadSafeDeque< ElementType > ElementList;

      /// If true, the stream will restart over from the beginning once
      /// it has been read in entirety.
      bool mIsLooping;

      /// If true, no further requests should be issued on this stream.
      /// @note This in itself doesn't say anything about pending requests.
      bool mIsStopped;

      /// Number of source elements remaining in the source stream.
      U32 mNumRemainingSourceElements;

      /// Number of elements currently on buffer list.
      U32 mNumBufferedElements;

      /// Maximum number of elements allowed on buffer list.
      U32 mMaxBufferedElements;

      /// List of buffered elements.
      ElementList mBufferedElements;

      /// The thread pool to which read items are queued.
      ThreadPool* mThreadPool;

      /// The thread context used for prioritizing read items in the pool.
      ThreadContext* mThreadContext;

      /// Request the next element from the underlying stream.
      virtual void _requestNext() = 0;

      /// Called when an element read has been completed on the underlying stream.
      virtual void _onArrival( const ElementType& element );

   public:

      /// Construct a new buffered stream reading from "source".
      ///
      /// @param stream The source stream from which to read the actual data elements.
      /// @param numSourceElementsToRead Total number of elements to read from "stream".
      /// @param numReadAhead Number of packets to read and buffer in advance.
      /// @param isLooping If true, the packet stream will loop infinitely over the source stream.
      /// @param pool The ThreadPool to use for asynchronous packet reads.
      /// @param context The ThreadContext to place asynchronous packet reads in.
      AsyncBufferedInputStream(  const Stream& stream,
                                 U32 numSourceElementsToRead = 0,
                                 U32 numReadAhead = DEFAULT_STREAM_LOOKAHEAD,
                                 bool isLooping = false,
                                 ThreadPool* pool = &ThreadPool::GLOBAL(),
                                 ThreadContext* context = ThreadContext::ROOT_CONTEXT() );

      virtual ~AsyncBufferedInputStream();
      
      /// @return true if the stream is looping infinitely.
      bool isLooping() const { return mIsLooping; }

      /// @return the number of elements that will be read and buffered in advance.
      U32 getReadAhead() const { return mMaxBufferedElements; }

      /// Initiate the request chain of the element stream.
      void start() { _requestNext(); }

      /// Call for the request chain of the element stream to stop at the next
      /// synchronization point.
      void stop() { mIsStopped = true; }

      // IInputStream.
      virtual U32 read( ElementType* buffer, U32 num );
};

//-----------------------------------------------------------------------------

template< typename T, typename Stream >
AsyncBufferedInputStream< T, Stream >::AsyncBufferedInputStream
         (  const Stream& stream,
            U32 numSourceElementsToRead,
            U32 numReadAhead,
            bool isLooping,
            ThreadPool* threadPool,
            ThreadContext* threadContext )
   : Parent( stream ),
     mIsStopped( false ),
     mIsLooping( isLooping ),
     mNumRemainingSourceElements( numSourceElementsToRead ),
     mNumBufferedElements( 0 ),
     mMaxBufferedElements( numReadAhead ),
     mThreadPool( threadPool ),
     mThreadContext( threadContext )
{
   if( mIsLooping )
   {
      // Stream is looping so we don't count down source elements.
      
      mNumRemainingSourceElements = 0;
   }
   else if( !mNumRemainingSourceElements )
   {
      // If not given number of elements to read, see if the source
      // stream is sizeable.  If so, take its size as the number of elements.
      
      if( dynamic_cast< ISizeable<>* >( &Deref( stream ) ) )
         mNumRemainingSourceElements = ( ( ISizeable<>* ) &Deref( stream ) )->getSize();
      else
      {
         // Can't tell how many source elements there are.
         
         mNumRemainingSourceElements = U32_MAX;
      }
   }
}

//-----------------------------------------------------------------------------

template< typename T, typename Stream >
AsyncBufferedInputStream< T, Stream >::~AsyncBufferedInputStream()
{
   ElementType element;
   while( mBufferedElements.tryPopFront( element ) )
      destructSingle( element );
}

//-----------------------------------------------------------------------------

template< typename T, typename Stream >
void AsyncBufferedInputStream< T, Stream >::_onArrival( const ElementType& element )
{
   mBufferedElements.pushBack( element );
   
   // Adjust buffer count.

   while( 1 )
   {
      S32 numBuffered = mNumBufferedElements;
      if( dCompareAndSwap( mNumBufferedElements, numBuffered, numBuffered + 1 ) )
      {
         // If we haven't run against the lookahead limit and haven't reached
         // the end of the stream, immediately trigger a new request.
         
         if( !mIsStopped && ( numBuffered + 1 ) < mMaxBufferedElements )
            _requestNext();
            
         break;
      }
   }
}

//-----------------------------------------------------------------------------

template< typename T, typename Stream >
U32 AsyncBufferedInputStream< T, Stream >::read( ElementType* buffer, U32 num )
{
   if( !num )
      return 0;

   U32 numRead = 0;
   for( U32 i = 0; i < num; ++ i )
   {
      // Try to pop a element off the buffered element list.
   
      ElementType element;
      if( mBufferedElements.tryPopFront( element ) )
      {
         buffer[ i ] = element;
         numRead ++;
      }
      else
         break;
   }

   // Get the request chain going again, if it has stopped.
   
   while( 1 )
   {
      U32 numBuffered = mNumBufferedElements;
      U32 newNumBuffered = numBuffered - numRead;
      
      if( dCompareAndSwap( mNumBufferedElements, numBuffered, newNumBuffered ) )
      {
         if( numBuffered == mMaxBufferedElements )
            _requestNext();
         
         break;
      }
   }

   return numRead;
}


//=============================================================================
//    AsyncSingleBufferedInputStream.
//=============================================================================


/// Asynchronous work item for reading an element from the source stream.
template< typename T, typename Stream = IInputStream< T >* >
class AsyncBufferedReadItem : public ThreadWorkItem
{
   public:

      typedef ThreadWorkItem Parent;
      typedef ThreadSafeRef< AsyncBufferedInputStream< T, Stream > > AsyncStreamRef;
      
   protected:

      /// The issueing async state.
      AsyncStreamRef mAsyncStream;
      
      ///
      Stream mSourceStream;

      /// The element read from the stream.
      T mElement;

      // WorkItem
      virtual void execute()
      {
         if( Deref( mSourceStream ).read( &mElement, 1 ) )
         {                  
            // Buffer the element.

            if( this->cancellationPoint() ) return;
            mAsyncStream->_onArrival( mElement );
         }
      }
      virtual void onCancelled()
      {
         Parent::onCancelled();
         destructSingle( mElement );
         mAsyncStream = NULL;
      }
      
   public:

      ///
      AsyncBufferedReadItem(
         const AsyncStreamRef& asyncStream,
         ThreadPool::Context* context = NULL
      )
         : Parent( context ),
           mAsyncStream( asyncStream ),
           mSourceStream( asyncStream->getSourceStream() )
      {
      }

};


/// A stream filter that performs background read-aheads on its source stream
/// and buffers the results.
///
/// As each element is read in an independent threaded operation, reading an
/// element should invole a certain amount of work for using this class to
/// make sense.
///
/// @note For looping streams, the stream must implement the IResettable interface.
///
template< typename T, typename Stream = IInputStream< T >*, class ReadItem = AsyncBufferedReadItem< T, Stream > >
class AsyncSingleBufferedInputStream : public AsyncBufferedInputStream< T, Stream >
{
   public:

      typedef AsyncBufferedInputStream< T, Stream > Parent;
      typedef typename Parent::ElementType ElementType;
      typedef typename Parent::SourceElementType SourceElementType;
      typedef typename Parent::SourceStreamType SourceStreamType;
      
   protected:
         
      // AsyncBufferedInputStream.
      virtual void _requestNext();

      /// Create a new work item that reads the next element.
      virtual void _newReadItem( ThreadSafeRef< ThreadWorkItem >& outRef )
      {
         outRef = new ReadItem( this, this->mThreadContext );
      }
            
   public:

      /// Construct a new buffered stream reading from "source".
      ///
      /// @param stream The source stream from which to read the actual data elements.
      /// @param numSourceElementsToRead Total number of elements to read from "stream".
      /// @param numReadAhead Number of packets to read and buffer in advance.
      /// @param isLooping If true, the packet stream will loop infinitely over the source stream.
      /// @param pool The ThreadPool to use for asynchronous packet reads.
      /// @param context The ThreadContext to place asynchronous packet reads in.
      AsyncSingleBufferedInputStream(  const Stream& stream,
                                       U32 numSourceElementsToRead = 0,
                                       U32 numReadAhead = Parent::DEFAULT_STREAM_LOOKAHEAD,
                                       bool isLooping = false,
                                       ThreadPool* pool = &ThreadPool::GLOBAL(),
                                       ThreadContext* context = ThreadContext::ROOT_CONTEXT() )
         : Parent(   stream,
                     numSourceElementsToRead,
                     numReadAhead,
                     isLooping,
                     pool,
                     context ) {}
};

//-----------------------------------------------------------------------------

template< typename T, typename Stream, class ReadItem >
void AsyncSingleBufferedInputStream< T, Stream, ReadItem >::_requestNext()
{
   Stream& stream = this->getSourceStream();
   bool isEOS = !this->mNumRemainingSourceElements;
   if( isEOS && this->mIsLooping )
   {
      SourceStreamType* s = &Deref( stream );
      dynamic_cast< IResettable* >( s )->reset();
      isEOS = false;
   }
   else if( isEOS )
      return;

   //TODO: could scale priority depending on feed status

   // Queue a stream packet work item.

   if( !this->mIsLooping && this->mNumRemainingSourceElements != U32_MAX )
      -- this->mNumRemainingSourceElements;
      
   ThreadSafeRef< ThreadWorkItem > workItem;
   _newReadItem( workItem );
   this->mThreadPool->queueWorkItem( workItem );
}

#endif // !_ASYNCBUFFEREDSTREAM_H_
