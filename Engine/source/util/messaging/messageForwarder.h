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

#include "console/simBase.h"
#include "util/messaging/dispatcher.h"
#include "util/messaging/scriptMsgListener.h"

#ifndef _MESSAGEFORWARDER_H_
#define _MESSAGEFORWARDER_H_

/// @addtogroup msgsys Message System
// @{

//-----------------------------------------------------------------------------
/// @brief Forward messages from one queue to another
/// 
/// MessageForwarder is a script class that can be used to forward messages
/// from one queue to another.
///
/// <h2>Example</h2>
///
/// @code
/// %fwd = new MessageForwarder()
/// {
///    toQueue = "QueueToSendTo";
/// };
///
/// registerMessageListener("FromQueue", %fwd);
/// @endcode
///
/// Where "QueueToSendTo" is the queue you want to forward to, and
/// "FromQueue" is the queue you want to forward from.
///
//-----------------------------------------------------------------------------
class MessageForwarder : public ScriptMsgListener
{
   typedef ScriptMsgListener Parent;

protected:
   StringTableEntry mToQueue;

public:
   MessageForwarder();
   virtual ~MessageForwarder();
   DECLARE_CONOBJECT(MessageForwarder);

   static void initPersistFields();

   virtual bool onMessageReceived(StringTableEntry queue, const char *event, const char *data);
   virtual bool onMessageObjectReceived(StringTableEntry queue, Message *msg);
};

// @}

#endif // _MESSAGEFORWARDER_H_
