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

#ifndef _ASYNCPACKETQUEUE_H_
#define _ASYNCPACKETQUEUE_H_

#ifndef _TFIXEDSIZEQUEUE_H_
#include "core/util/tFixedSizeDeque.h"
#endif

#ifndef _TSTREAM_H_
#include "core/stream/tStream.h"
#endif

#ifndef _TYPETRAITS_H_
#include "platform/typetraits.h"
#endif


//#define DEBUG_SPEW


/// @file
/// Time-based packet streaming.
///
/// The classes contained in this file can be used for any kind
/// of continuous playback that depends on discrete samplings of
/// a source stream (i.e. any kind of digital media streaming).



//--------------------------------------------------------------------------
//    Async packet queue.
//--------------------------------------------------------------------------

/// Time-based packet stream queue.
///
/// AsyncPacketQueue writes data packets to a consumer stream in sync to
/// a tick time source.  Outdated packets may optionally be dropped automatically
/// by the queue.  A fixed maximum number of packets can reside in the queue
/// concurrently at any one time.
///
/// Be aware that using single item queues for synchronizing to a timer
/// will usually result in bad timing behavior when packet uploading takes
/// any non-trivial amount of time.
///
/// @note While the queue associates a variable tick count with each
///   individual packet, the queue fill status is measured in number of
///   packets rather than in total tick time.
///
/// @param Packet Value type of packets passed through this queue.
/// @param TimeSource Value type for time tick source to which the queue
///   is synchronized.
/// @param Consumer Value type of stream to which the packets are written.
///
template< typename Packet, typename TimeSource = IPositionable< U32 >*, typename Consumer = IOutputStream< Packet >*, typename Tick = U32 >
class AsyncPacketQueue
{
   public:

      typedef void Parent;

      /// The type of data packets being streamed through this queue.
      typedef typename TypeTraits< Packet >::BaseType PacketType;

      /// The type of consumer that receives the packets from this queue.
      typedef typename TypeTraits< Consumer >::BaseType ConsumerType;

      ///
      typedef typename TypeTraits< TimeSource >::BaseType TimeSourceType;
      
      /// Type for counting ticks.
      typedef Tick TickType;

   protected:

      /// Information about the time slice covered by an
      /// individual packet currently on the queue.
      struct QueuedPacket
      {
         /// First tick contained in this packet.
         TickType mStartTick;

         /// First tick *not* contained in this packet anymore.
         TickType mEndTick;

         QueuedPacket( TickType start, TickType end )
            : mStartTick( start ), mEndTick( end ) {}

         /// Return the total number of ticks in this packet.
         TickType getNumTicks() const
         {
            return ( mEndTick - mStartTick );
         }
      };

      typedef FixedSizeDeque< QueuedPacket > PacketQueue;

      /// If true, packets that have missed their proper queuing timeframe
      /// will be dropped.  If false, they will be queued nonetheless.
      bool mDropPackets;

      /// Total number of ticks spanned by the total queue playback time.
      /// If this is zero, the total queue time is considered to be infinite.
      TickType mTotalTicks;

      /// Total number of ticks submitted to the queue so far.
      TickType mTotalQueuedTicks;

      /// Queue that holds records for each packet currently in the queue.  New packets
      /// are added to back.
      PacketQueue mPacketQueue;

      /// The time source to which we are sync'ing.
      TimeSource mTimeSource;

      /// The output stream that this queue feeds into.
      Consumer mConsumer;

      /// Total number of packets queued so far.
      U32 mTotalQueuedPackets;
      
   public:

      /// Construct an AsyncPacketQueue of the given length.
      ///
      /// @param maxQueuedPackets The length of the queue in packets.  Only a maximum of
      ///   'maxQueuedPackets' packets can be concurrently in the queue at any one time.
      /// @param timeSource The tick time source to which the queue synchronizes.
      /// @param consumer The output stream that receives the packets in sync to timeSource.
      /// @param totalTicks The total number of ticks that will be played back through the
      ///   queue; if 0, the length is considered indefinite.
      /// @param dropPackets Whether the queue should drop outdated packets; if dropped, a
      ///   packet will not reach the consumer.
      AsyncPacketQueue(    U32 maxQueuedPackets,
                           TimeSource timeSource,
                           Consumer consumer,
                           TickType totalTicks = 0,
                           bool dropPackets = false )
         : mDropPackets( dropPackets ),
           mTotalTicks( totalTicks ),
           mTotalQueuedTicks( 0 ),
           mPacketQueue( maxQueuedPackets ),
           mTimeSource( timeSource ),
           mConsumer( consumer )

      {
         #ifdef TORQUE_DEBUG
         mTotalQueuedPackets = 0;
         #endif
      }

      /// Return true if there are currently
      bool isEmpty() const { return mPacketQueue.isEmpty(); }

      /// Return true if all packets have been streamed.
      bool isAtEnd() const;

      /// Return true if the queue needs one or more new packets to be submitted.
      bool needPacket();

      /// Submit a data packet to the queue.
      ///
      /// @param packet The data packet.
      /// @param packetTicks The duration of the packet in ticks.
      /// @param isLast If true, the packet is the last one in the stream.
      /// @param packetPos The absolute position of the packet in the stream; if this is not supplied
      ///   the packet is assumed to immediately follow the preceding packet.
      ///
      /// @return true if the packet has been queued or false if it has been dropped.
      bool submitPacket(   Packet packet,
                           TickType packetTicks,
                           bool isLast = false,
                           TickType packetPos = TypeTraits< TickType >::MAX );

      /// Return the current playback position according to the time source.
      TickType getCurrentTick() const { return Deref( mTimeSource ).getPosition(); }

      /// Return the total number of ticks that have been queued so far.
      TickType getTotalQueuedTicks() const { return mTotalQueuedTicks; }
      
      /// Return the total number of packets that have been queued so far.
      U32 getTotalQueuedPackets() const { return mTotalQueuedPackets; }
};

template< typename Packet, typename TimeSource, typename Consumer, typename Tick >
inline bool AsyncPacketQueue< Packet, TimeSource, Consumer, Tick >::isAtEnd() const
{
   // Never at end if infinite.

   if( !mTotalTicks )
      return false;

   // Otherwise, we're at end if we're past the total tick count.

   return ( getCurrentTick() >= mTotalTicks
            && ( mDropPackets || mTotalQueuedTicks >= mTotalTicks ) );
}

template< typename Packet, typename TimeSource, typename Consumer, typename Tick >
bool AsyncPacketQueue< Packet, TimeSource, Consumer, Tick >::needPacket()
{
   // Never need more packets once we have reached the
   // end.

   if( isAtEnd() )
      return false;

   // Always needs packets while the queue is not
   // filled up completely.

   if( mPacketQueue.capacity() != 0 )
      return true;

   // Unqueue packets that have expired their playtime.

   TickType currentTick = getCurrentTick();
   while( mPacketQueue.size() && currentTick >= mPacketQueue.front().mEndTick )
   {
      #ifdef DEBUG_SPEW
      Platform::outputDebugString( "[AsyncPacketQueue] expired packet #%i: %i-%i (tick: %i; queue: %i)",
         mTotalQueuedPackets - mPacketQueue.size(),
         U32( mPacketQueue.front().mStartTick ),
         U32( mPacketQueue.front().mEndTick ),
         U32( currentTick ),
         mPacketQueue.size() );
      #endif
      
      mPacketQueue.popFront();
   }

   // Need more packets if the queue isn't full anymore.

   return ( mPacketQueue.capacity() != 0 );
}

template< typename Packet, typename TimeSource, typename Consumer, typename Tick >
bool AsyncPacketQueue< Packet, TimeSource, Consumer, Tick >::submitPacket( Packet packet, TickType packetTicks, bool isLast, TickType packetPos )
{
   AssertFatal( mPacketQueue.capacity() != 0,
      "AsyncPacketQueue::submitPacket() - Queue is full!" );

   TickType packetStartPos;
   TickType packetEndPos;
   
   if( packetPos != TypeTraits< TickType >::MAX )
   {
      packetStartPos = packetPos;
      packetEndPos = packetPos + packetTicks;
   }
   else
   {
      packetStartPos = mTotalQueuedTicks;
      packetEndPos = mTotalQueuedTicks + packetTicks;
   }

   // Check whether the packet is outdated, if enabled.

   bool dropPacket = false;
   if( mDropPackets )
   {
      TickType currentTick = getCurrentTick();
      if( currentTick >= packetEndPos )
         dropPacket = true;
   }

   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[AsyncPacketQueue] new packet #%i: %i-%i (ticks: %i, current: %i, queue: %i)%s",
      mTotalQueuedPackets,
      U32( mTotalQueuedTicks ),
      U32( packetEndPos ),
      U32( packetTicks ),
      U32( getCurrentTick() ),
      mPacketQueue.size(),
      dropPacket ? " !! DROPPED !!" : "" );
   #endif

   // Queue the packet.

   if( !dropPacket )
   {
      mPacketQueue.pushBack( QueuedPacket( packetStartPos, packetEndPos ) );
      Deref( mConsumer ).write( &packet, 1 );
   }

   mTotalQueuedTicks = packetEndPos;
   if( isLast && !mTotalTicks )
      mTotalTicks = mTotalQueuedTicks;
      
   mTotalQueuedPackets ++;
   
   return !dropPacket;
}

#undef DEBUG_SPEW
#endif // _ASYNCPACKETQUEUE_H_
