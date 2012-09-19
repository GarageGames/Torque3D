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

#include "platform/platformNet.h"
#include "unit/test.h"
#include "core/util/journal/process.h"

using namespace UnitTesting;

CreateUnitTest( TestTCPRequest, "Platform/Net/TCPRequest")
{
   NetSocket mSocket;
   S32 mDataRecved;

   void handleNotify(NetSocket sock, U32 state) 
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

         test(e == Net::NoError, "Got an error sending our HTTP request!");
      }
      else if(state == Net::Disconnected)
      {
         Process::requestShutdown();
         mSocket = NULL;
      }
   }

   void handleReceive(NetSocket sock, RawData incomingData)
   {
      // Only consider our own socket.
      if(mSocket != sock)
         return;

      char buff[4096];
      dMemcpy(buff, incomingData.data, incomingData.size);
      buff[incomingData.size] = 0;

      UnitPrint("Got a message...\n");
      UnitPrint(buff);
      UnitPrint("------\n");

      mDataRecved += incomingData.size;
   }

   void run()
   {
      mSocket = InvalidSocket;
      mDataRecved = 0;

      // Initialize networking - done by initLibraries currently
      //test(Net::init(), "Failed to initialize networking!");

      // Hook into the signals.
      Net::smConnectionNotify. notify(this, &TestTCPRequest::handleNotify);
      Net::smConnectionReceive.notify(this, &TestTCPRequest::handleReceive);

      // Open a TCP connection to garagegames.com
      mSocket = Net::openConnectTo("ip:72.246.107.193:80");

      while(Process::processEvents())
         ;

      // Unhook from the signals.
      Net::smConnectionNotify. remove(this, &TestTCPRequest::handleNotify);
      Net::smConnectionReceive.remove(this, &TestTCPRequest::handleReceive);

      test(mDataRecved > 0, "Didn't get any data back!");
   }
};

CreateUnitTest( TestTCPRequestJournal, "Platform/Net/JournalTCPRequest")
{
   NetSocket mSocket;
   S32 mDataRecved;

   void handleNotify(NetSocket sock, U32 state) 
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

         test(e == Net::NoError, "Got an error sending our HTTP request!");
      }
      else if(state == Net::Disconnected)
      {
         Process::requestShutdown();
         mSocket = NULL;
      }
   }

   void handleReceive(NetSocket sock, RawData incomingData)
   {
      // Only consider our own socket.
      if(mSocket != sock)
         return;

      char buff[4096];
      dMemcpy(buff, incomingData.data, incomingData.size);
      buff[incomingData.size] = 0;

      UnitPrint("Got a message...\n");
      UnitPrint(buff);
      UnitPrint("------\n");

      mDataRecved += incomingData.size;
   }

   void makeRequest()
   {
      mSocket = InvalidSocket;
      mDataRecved = 0;

      // Initialize networking - done by initLibraries currently
      //test(Net::init(), "Failed to initialize networking!");

      // Hook into the signals.
      Net::smConnectionNotify. notify(this, &TestTCPRequestJournal::handleNotify);
      Net::smConnectionReceive.notify(this, &TestTCPRequestJournal::handleReceive);

      // Open a TCP connection to garagegames.com
      mSocket = Net::openConnectTo("ip:72.246.107.193:80");

      // Let the callbacks enable things to process.
      while(Process::processEvents())
         ;

      // Unhook from the signals.
      Net::smConnectionNotify. remove(this, &TestTCPRequestJournal::handleNotify);
      Net::smConnectionReceive.remove(this, &TestTCPRequestJournal::handleReceive);

      test(mDataRecved > 0, "Didn't get any data back!");
   }

   void run()
   {
      Journal::Record("journalTCP.jrn");

      makeRequest();

      S32 bytesRead = mDataRecved;

      Journal::Stop();

      Journal::Play("journalTCP.jrn");

      makeRequest();

      Journal::Stop();

      test(bytesRead == mDataRecved, "Didn't get same data back from journal playback.");

   }
};