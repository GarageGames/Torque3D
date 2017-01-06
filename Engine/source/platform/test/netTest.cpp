//-----------------------------------------------------------------------------
// Copyright (c) 2014 GarageGames, LLC
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

#ifdef TORQUE_TESTS_ENABLED
#include "testing/unitTesting.h"
#include "platform/platformNet.h"
#include "core/util/journal/process.h"

struct TcpHandle
{
   NetSocket mSocket;
   S32 mDataReceived;

   void notify(NetSocket sock, U32 state) 
   {
      // Only consider our own socket.
      if(mSocket != sock)
         return;

      // Ok - what's the state? We do some dumb responses to given states
      // in order to fulfill the request.
      if(state == Net::Connected)
      {
         U8 reqBuffer[] = {
            "GET / HTTP/1.0\nUser-Agent: Torque/1.0\n\n"
         };

         Net::Error e = Net::sendtoSocket(mSocket, reqBuffer, sizeof(reqBuffer));

         ASSERT_EQ(Net::NoError, e)
            << "Got an error sending our HTTP request!";
      }
      else
      {
         Process::requestShutdown();
         mSocket = NetSocket::INVALID;
         ASSERT_EQ(Net::Disconnected, state)
            << "Ended with a network error!";
      }
   }

   void receive(NetSocket sock, RawData incomingData)
   {
      // Only consider our own socket.
      if(mSocket != sock)
         return;

      mDataReceived += incomingData.size;
   }
};

TEST(Net, TCPRequest)
{
   TcpHandle handler;

   handler.mSocket = NetSocket::INVALID;
   handler.mDataReceived = 0;

   // Hook into the signals.
   Net::smConnectionNotify .notify(&handler, &TcpHandle::notify);
   Net::smConnectionReceive.notify(&handler, &TcpHandle::receive);

   // Open a TCP connection to garagegames.com
   handler.mSocket = Net::openConnectTo("72.246.107.193:80");
   const U32 limit = Platform::getRealMilliseconds() + (5*1000);
   while(Process::processEvents() && (Platform::getRealMilliseconds() < limit) ) {}

   // Unhook from the signals.
   Net::smConnectionNotify .remove(&handler, &TcpHandle::notify);
   Net::smConnectionReceive.remove(&handler, &TcpHandle::receive);

   EXPECT_GT(handler.mDataReceived, 0)
      << "Didn't get any data back!";
}

struct JournalHandle
{
   NetSocket mSocket;
   S32 mDataReceived;

   void notify(NetSocket sock, U32 state)
   {
      // Only consider our own socket.
      if(mSocket != sock)
         return;

      // Ok - what's the state? We do some dumb responses to given states
      // in order to fulfill the request.
      if(state == Net::Connected)
      {
         U8 reqBuffer[] = {
            "GET / HTTP/1.0\nUser-Agent: Torque/1.0\n\n"
         };

         Net::Error e = Net::sendtoSocket(mSocket, reqBuffer, sizeof(reqBuffer));

         ASSERT_EQ(Net::NoError, e)
            << "Got an error sending our HTTP request!";
      }
      else
      {
         Process::requestShutdown();
         mSocket = NetSocket::INVALID;
         ASSERT_EQ(Net::Disconnected, state)
            << "Ended with a network error!";
      }
   }

   void receive(NetSocket sock, RawData incomingData)
   {
      // Only consider our own socket.
      if(mSocket != sock)
         return;
      mDataReceived += incomingData.size;
   }

   void makeRequest()
   {
      mSocket = NetSocket::INVALID;
      mDataReceived = 0;

      // Hook into the signals.
      Net::smConnectionNotify .notify(this, &JournalHandle::notify);
      Net::smConnectionReceive.notify(this, &JournalHandle::receive);

      // Open a TCP connection to garagegames.com
      mSocket = Net::openConnectTo("72.246.107.193:80");

      // Let the callbacks enable things to process.
      while(Process::processEvents()) {}

      // Unhook from the signals.
      Net::smConnectionNotify .remove(this, &JournalHandle::notify);
      Net::smConnectionReceive.remove(this, &JournalHandle::receive);

      EXPECT_GT(mDataReceived, 0)
         << "Didn't get any data back!";
   }
};

TEST(Net, JournalTCPRequest)
{
   JournalHandle handler;

   Journal::Record("journalTCP.jrn");
   ASSERT_TRUE(Journal::IsRecording());
   handler.makeRequest();
   S32 bytesRead = handler.mDataReceived;
   Journal::Stop();

   Journal::Play("journalTCP.jrn");
   handler.makeRequest();
   Journal::Stop();

   EXPECT_EQ(bytesRead, handler.mDataReceived)
      << "Didn't get same data back from journal playback.";
}

#endif
