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

#ifndef _SCRIPTMSGLISTENER_H_
#define _SCRIPTMSGLISTENER_H_

#ifndef _DISPATCHER_H_
#include "util/messaging/dispatcher.h"
#endif

/// @addtogroup msgsys Message System
// @{

//-----------------------------------------------------------------------------
/// @brief Script accessible version of Dispatcher::IMessageListener
///
/// The main use of ScriptMsgListener is to allow script to listen for
/// messages. You can subclass ScriptMsgListener in script to receive
/// the Dispatcher::IMessageListener callbacks.
///
/// Alternatively, you can derive from it in C++ instead of SimObject to
/// get an object that implements Dispatcher::IMessageListener with script
/// callbacks. If you need to derive from something other then SimObject,
/// then you will need to implement the Dispatcher::IMessageListener
/// interface yourself.
//-----------------------------------------------------------------------------
class ScriptMsgListener : public SimObject, public virtual Dispatcher::IMessageListener
{
   typedef SimObject Parent;
   typedef Dispatcher::IMessageListener IMLParent;

public:
   ScriptMsgListener();

   DECLARE_CONOBJECT(ScriptMsgListener);
   
   DECLARE_CALLBACK( void, onAdd, () );
   DECLARE_CALLBACK( void, onRemove, () );

   DECLARE_CALLBACK( bool, onMessageReceived, ( const char* queue, const char* event, const char* data ) );
   DECLARE_CALLBACK( bool, onMessageObjectReceived, ( const char* queue, Message *msg ) );
   
   DECLARE_CALLBACK( void, onAddToQueue, ( const char* queue ) );
   DECLARE_CALLBACK( void, onRemoveFromQueue, ( const char* queue ) );

   ///////////////////////////////////////////////////////////////////////

   virtual bool onAdd();
   virtual void onRemove();

   ///////////////////////////////////////////////////////////////////////

   virtual bool onMessageReceived(StringTableEntry queue, const char* event, const char* data);
   virtual bool onMessageObjectReceived(StringTableEntry queue, Message *msg);

   virtual void onAddToQueue(StringTableEntry queue);
   virtual void onRemoveFromQueue(StringTableEntry queue);
};

// @}

#endif // _SCRIPTMSGLISTENER_H_
