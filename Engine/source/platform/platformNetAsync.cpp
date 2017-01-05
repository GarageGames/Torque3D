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

#include "platform/platformNetAsync.h"
#include "core/strings/stringFunctions.h"
#include "platform/threads/threadPool.h"
#include "console/console.h"

#if defined(TORQUE_OS_WIN)
#  include <winsock.h>
#elif defined(TORQUE_OS_XENON)
#  include <Xtl.h>
#else
#  include <netdb.h>
#  include <unistd.h>
#endif

#include <errno.h>

NetAsync gNetAsync;

//--------------------------------------------------------------------------
//    NetAsync::NameLookupRequest.
//--------------------------------------------------------------------------

// internal structure for storing information about a name lookup request
struct NetAsync::NameLookupRequest
{
      NetSocket sock;
      char remoteAddr[4096];
      char out_h_addr[4096];
      S32 out_h_length;
      bool complete;

      NameLookupRequest()
      {
         sock = NetSocket::INVALID;
         remoteAddr[0] = 0;
         out_h_addr[0] = 0;
         out_h_length = -1;
         complete = false;
      }
};

//--------------------------------------------------------------------------
//    NetAsync::NameLookupWorkItem.
//--------------------------------------------------------------------------

/// Work item issued to the thread pool for each lookup request.

struct NetAsync::NameLookupWorkItem : public ThreadPool::WorkItem
{
   typedef ThreadPool::WorkItem Parent;

   NameLookupWorkItem( NameLookupRequest& request, ThreadPool::Context* context = 0 )
      : Parent( context ),
        mRequest( request )
   {
   }

protected:
   virtual void execute()
   {
#ifndef TORQUE_OS_XENON

	  NetAddress address;
	  Net::Error error = Net::stringToAddress(mRequest.remoteAddr, &address, true);

      // do it
      if (error != Net::NoError)
      {
         // oh well!  leave the lookup data unmodified (h_length) should
         // still be -1 from initialization
         mRequest.complete = true;
      }
      else
      {
         // copy the stuff we need from the hostent 
         dMemset(mRequest.out_h_addr, 0, 
            sizeof(mRequest.out_h_addr));
         dMemcpy(mRequest.out_h_addr, &address, sizeof(address));

		 mRequest.out_h_length = sizeof(address);
         mRequest.complete = true;
      }
#else
      XNDNS *pxndns = NULL;
      HANDLE hEvent = CreateEvent(NULL, false, false, NULL);
      XNetDnsLookup(mRequest.remoteAddr, hEvent, &pxndns);

      while(pxndns->iStatus == WSAEINPROGRESS) 
         WaitForSingleObject(hEvent, INFINITE);

      if(pxndns->iStatus == 0 && pxndns->cina > 0)
      {
         dMemset(mRequest.out_h_addr, 0, sizeof(mRequest.out_h_addr));

         // This is a suspect section. I need to revisit. [2/22/2010 Pat]
         dMemcpy(mRequest.out_h_addr, pxndns->aina, sizeof(IN_ADDR)); 
         mRequest.out_h_length = sizeof(IN_ADDR);
      }

      mRequest.complete = true;

      XNetDnsRelease(pxndns);
      CloseHandle(hEvent);
#endif
   }

private:
   NameLookupRequest&   mRequest;
};

//--------------------------------------------------------------------------
//    NetAsync.
//--------------------------------------------------------------------------

NetAsync::NetAsync()
{
   VECTOR_SET_ASSOCIATION( mLookupRequests );
}

void NetAsync::queueLookup(const char* remoteAddr, NetSocket socket)
{
   // do we have it already?
   
   U32 i = 0;
   for (i = 0; i < mLookupRequests.size(); ++i)
   {
      if (mLookupRequests[i].sock == socket)
         // found it.  ignore more than one lookup at a time for a socket.
         return;
   }

   // not found, so add it

   mLookupRequests.increment();
   NameLookupRequest& lookupRequest = mLookupRequests.last();
   lookupRequest.sock = socket;
   dStrncpy(lookupRequest.remoteAddr, remoteAddr, sizeof(lookupRequest.remoteAddr));

   ThreadSafeRef< NameLookupWorkItem > workItem( new NameLookupWorkItem( lookupRequest ) );
   ThreadPool::GLOBAL().queueWorkItem( workItem );
}

bool NetAsync::checkLookup(NetSocket socket, void* out_h_addr, 
                           S32* out_h_length, S32 out_h_addr_size)
{
   bool found = false;

   // search for the socket
   RequestIterator iter;
   for (iter = mLookupRequests.begin(); 
        iter != mLookupRequests.end(); 
        ++iter)
      // if we found it and it is complete...
      if (socket == iter->sock && iter->complete)
      {
         // copy the lookup data to the callers parameters
         dMemcpy(out_h_addr, iter->out_h_addr, out_h_addr_size);
         *out_h_length = iter->out_h_length;
         found = true;
         break;
      }

   // we found the socket, so we are done with it.  erase.
   if (found)
      mLookupRequests.erase(iter);

   return found;
}
