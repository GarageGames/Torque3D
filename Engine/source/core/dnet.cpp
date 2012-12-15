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

#include "core/stream/bitStream.h"
#include "core/dnet.h"
#include "core/strings/stringFunctions.h"

#include "console/consoleTypes.h"


bool gLogToConsole = false;

S32 gNetBitsReceived = 0;

enum NetPacketType
{
   DataPacket,
   PingPacket,
   AckPacket,
   InvalidPacketType,
};

static const char *packetTypeNames[] =
{
   "DataPacket",
   "PingPacket",
   "AckPacket",
};

//-----------------------------------------------------------------
//-----------------------------------------------------------------
//-----------------------------------------------------------------
ConsoleFunction(DNetSetLogging, void, 2, 2, "(bool enabled)"
   "@brief Enables logging of the connection protocols\n\n"
   "When enabled a lot of network debugging information is sent to the console.\n"
   "@param enabled True to enable, false to disable\n"
   "@ingroup Networking")
{
   TORQUE_UNUSED(argc);
   gLogToConsole = dAtob(argv[1]);
}

ConnectionProtocol::ConnectionProtocol()
{
   mLastSeqRecvd = 0;
   mHighestAckedSeq = 0;
   mLastSendSeq = 0; // start sending at 1
   mAckMask = 0;
   mLastRecvAckAck = 0;
}
void ConnectionProtocol::buildSendPacketHeader(BitStream *stream, S32 packetType)
{
   S32 ackByteCount = ((mLastSeqRecvd - mLastRecvAckAck + 7) >> 3);
   AssertFatal(ackByteCount <= 4, "Too few ack bytes!");

   // S32 headerSize = 3 + ackByteCount;

   if(packetType == DataPacket)
      mLastSendSeq++;

   stream->writeFlag(true);
   stream->writeInt(mConnectSequence & 1, 1);
   stream->writeInt(mLastSendSeq & 0x1FF, 9);
   stream->writeInt(mLastSeqRecvd & 0x1FF, 9);
   stream->writeInt(packetType & 0x3, 2);
   stream->writeInt(ackByteCount & 0x7, 3);
   U32 bitmask = ~(0xFFFFFFFF << (ackByteCount*8));
   if(ackByteCount == 4)
   {
      // Performing a bit shift that is the same size as the variable being shifted
      // is undefined in the C/C++ standard.  Handle that exception here when
      // ackByteCount*8 == 4*8 == 32
      bitmask = 0xFFFFFFFF;
   }
   stream->writeInt(mAckMask & bitmask, ackByteCount * 8);

   // if we're resending this header, we can't advance the
   // sequence recieved (in case this packet drops and the prev one
   // goes through)

   if(gLogToConsole)
      Con::printf("build hdr %d %d", mLastSendSeq, packetType);

   if(packetType == DataPacket)
      mLastSeqRecvdAtSend[mLastSendSeq & 0x1F] = mLastSeqRecvd;
}

void ConnectionProtocol::sendPingPacket()
{
   U8 buffer[16];
   BitStream bs(buffer, 16);
   buildSendPacketHeader(&bs, PingPacket);
   if(gLogToConsole)
      Con::printf("send ping %d", mLastSendSeq);

   sendPacket(&bs);
}

void ConnectionProtocol::sendAckPacket()
{
   U8 buffer[16];
   BitStream bs(buffer, 16);
   buildSendPacketHeader(&bs, AckPacket);
   if(gLogToConsole)
      Con::printf("send ack %d", mLastSendSeq);

   sendPacket(&bs);
}

// packets are read directly into the data portion of
// connection notify packets... makes the events easier to post into
// the system.

void ConnectionProtocol::processRawPacket(BitStream *pstream)
{
   // read in the packet header:

   // Fixed packet header: 3 bytes
   //
   //   1 bit game packet flag
   //   1 bit connect sequence
   //   9 bits packet seq number
   //   9 bits ackstart seq number
   //   2 bits packet type
   //   2 bits ack byte count
   //
   // type is:
   //    00 data packet
   //    01 ping packet
   //    02 ack packet

   // next 1-4 bytes are ack flags
   //
   //   header len is 4-9 bytes
   //   average case 4 byte header

   gNetBitsReceived = pstream->getStreamSize();

   pstream->readFlag(); // get rid of the game info packet bit
   U32 pkConnectSeqBit  = pstream->readInt(1);
   U32 pkSequenceNumber = pstream->readInt(9);
   U32 pkHighestAck     = pstream->readInt(9);
   U32 pkPacketType     = pstream->readInt(2);
   S32 pkAckByteCount   = pstream->readInt(3);

   // check connection sequence bit
   if(pkConnectSeqBit != (mConnectSequence & 1))
      return;

   if(pkAckByteCount > 4 || pkPacketType >= InvalidPacketType)
      return;

   S32 pkAckMask = pstream->readInt(8 * pkAckByteCount);

   // verify packet ordering and acking and stuff
   // check if the 9-bit sequence is within the packet window
   // (within 31 packets of the last received sequence number).

   pkSequenceNumber |= (mLastSeqRecvd & 0xFFFFFE00);
   // account for wrap around
   if(pkSequenceNumber < mLastSeqRecvd)
      pkSequenceNumber += 0x200;

   if(pkSequenceNumber > mLastSeqRecvd + 31)
   {
      // the sequence number is outside the window... must be out of order
      // discard.
      return;
   }

   pkHighestAck |= (mHighestAckedSeq & 0xFFFFFE00);
   // account for wrap around

   if(pkHighestAck < mHighestAckedSeq)
      pkHighestAck += 0x200;

   if(pkHighestAck > mLastSendSeq)
   {
      // the ack number is outside the window... must be an out of order
      // packet, discard.
      return;
   }

   if(gLogToConsole)
   {
      for(U32 i = mLastSeqRecvd+1; i < pkSequenceNumber; i++)
         Con::printf("Not recv %d", i);
      Con::printf("Recv %d %s", pkSequenceNumber, packetTypeNames[pkPacketType]);
   }

   // shift up the ack mask by the packet difference
   // this essentially nacks all the packets dropped

   mAckMask <<= pkSequenceNumber - mLastSeqRecvd;

   // if this packet is a data packet (i.e. not a ping packet or an ack packet), ack it
   if(pkPacketType == DataPacket)
      mAckMask |= 1;

   // do all the notifies...
   for(U32 i = mHighestAckedSeq+1; i <= pkHighestAck; i++)
   {
      bool packetTransmitSuccess = pkAckMask & (1 << (pkHighestAck - i));
      handleNotify(packetTransmitSuccess);
      if(gLogToConsole)
         Con::printf("Ack %d %d", i, packetTransmitSuccess);

      if(packetTransmitSuccess)
      {
         mLastRecvAckAck = mLastSeqRecvdAtSend[i & 0x1F];
         if(!mConnectionEstablished)
         {
            mConnectionEstablished = true;
            handleConnectionEstablished();
         }
      }
   }

   // the other side knows more about its window than we do.
   if(pkSequenceNumber - mLastRecvAckAck > 32)
      mLastRecvAckAck = pkSequenceNumber - 32;

   mHighestAckedSeq = pkHighestAck;

   // first things first...
   // ackback any pings or accept connects

   if(pkPacketType == PingPacket)
   {
      // send an ack to the other side
      // the ack will have the same packet sequence as our last sent packet
      // if the last packet we sent was the connection accepted packet
      // we must resend that packet
      sendAckPacket();
   }
   keepAlive(); // notification that the connection is ok

   // note: handlePacket() may delete the connection if an error occurs.
   if(mLastSeqRecvd != pkSequenceNumber)
   {
      mLastSeqRecvd = pkSequenceNumber;
      if(pkPacketType == DataPacket)
         handlePacket(pstream);
   }
}

bool ConnectionProtocol::windowFull()
{
   return mLastSendSeq - mHighestAckedSeq >= 30;
}

void ConnectionProtocol::writeDemoStartBlock(ResizeBitStream *stream)
{
   for(U32 i = 0; i < 32; i++)
      stream->write(mLastSeqRecvdAtSend[i]);
   stream->write(mLastSeqRecvd);
   stream->write(mHighestAckedSeq);
   stream->write(mLastSendSeq);
   stream->write(mAckMask);
   stream->write(mConnectSequence);
   stream->write(mLastRecvAckAck);
   stream->write(mConnectionEstablished);
}

bool ConnectionProtocol::readDemoStartBlock(BitStream *stream)
{
   for(U32 i = 0; i < 32; i++)
      stream->read(&mLastSeqRecvdAtSend[i]);
   stream->read(&mLastSeqRecvd);
   stream->read(&mHighestAckedSeq);
   stream->read(&mLastSendSeq);
   stream->read(&mAckMask);
   stream->read(&mConnectSequence);
   stream->read(&mLastRecvAckAck);
   stream->read(&mConnectionEstablished);
   return true;
}
