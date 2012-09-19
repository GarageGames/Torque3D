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

#ifndef PLATFORM_NET_ASYNC_H
#define PLATFORM_NET_ASYNC_H

#include "platform/platform.h"
#include "platform/platformNet.h"
#include "core/util/tVector.h"

// class for doing asynchronous network operations on unix (linux and 
// hopefully osx) platforms.  right now it only implements dns lookups
class NetAsync
{
   private:
      struct NameLookupRequest;
      struct NameLookupWorkItem;

      typedef Vector< NameLookupRequest > RequestVector;
      typedef RequestVector::iterator RequestIterator;

      RequestVector mLookupRequests;

   public:
      NetAsync();

      // queue a DNS lookup.  only one dns lookup can be queued per socket at
      // a time.  subsequent queue request for the socket are ignored.  use
      // checkLookup() to check the status of a request.
      void queueLookup(const char* remoteAddr, NetSocket socket);

      // check on the status of a dns lookup for a socket.  if the lookup is 
      // not yet complete, the function will return false.  if it is 
      // complete, the function will return true, and out_h_addr and 
      // out_h_length will be set appropriately.  if out_h_length is -1, then
      // name could not be resolved.  otherwise, it provides the number of
      // address bytes copied into out_h_addr.
      bool checkLookup(NetSocket socket, char* out_h_addr, int* out_h_length, int out_h_addr_size);
};

// the global net async object
extern NetAsync gNetAsync;

#endif
