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

#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif

#ifndef _NETCONNECTION_H_
#include "sim/netConnection.h"
#endif

// Forward Refs
class MessageQueue;

/// @addtogroup msgsys Message System
///
/// Most of the message system docs are currently just stubs and will
/// be fleshed out soon.
///
// @{

//-----------------------------------------------------------------------------
// Message Base Class
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// @brief Base class for messages
/// 
/// Message is the base class for C++ defined messages, and may also be used
/// in script for script defined messages if no C++ subclass is appropriate.
///
/// Messages are reference counted and will be automatically deleted when
/// their reference count reaches zero. When you dispatch a message, a
/// reference will be added before the dispatch and freed after the dispatch.
/// This allows for temporary messages with no additional code. If you want
/// to keep the message around, for example to dispatch it to multiple
/// queues, call addReference() before dispatching it and freeReference()
/// when you are done with it. Never delete a Message object directly
/// unless addReference() has not been called or the message has not been
/// dispatched.
///
/// Message IDs are pooled similarly to datablocks, with the exception that
/// IDs are reused. If you keep a message for longer than a single dispatch,
/// then you should ensure that you clear any script variables that refer
/// to it after the last freeReference(). If you don't, then it is probable
/// that the object ID will become valid again in the future and could cause
/// hard to track down bugs.
///
/// Messages have a unique type to simplify message handling code. For object
/// messages, the type is defined as either the script defined class name
/// or the C++ class name if no script class was defined. The message type
/// may be obtained through the getType() method.
///
/// By convention, any data for the message is held in script accessible
/// fields. Messages that need to be handled in C++ as well as script
/// provide the relevant data through persistent fields in a subclass of
/// Message to provide best performance on the C++ side. Script defined
/// messages usually their through dynamic fields, and may be accessed in 
/// C++ using the SimObject::getDataField() method.
//-----------------------------------------------------------------------------
class Message : public SimObject
{
   typedef SimObject Parent;

public:
   Message();
   DECLARE_CONOBJECT(Message);
   DECLARE_CALLBACK( void, onAdd, () );
   DECLARE_CALLBACK( void, onRemove, () );

   //-----------------------------------------------------------------------------
   /// @brief Obtain next available #SimObjectId for messages
   ///
   /// This is used in combination with the newmsg script operator to provide
   /// ID pooling for messages and works similarly to datablock IDs.
   ///
   /// By default, the 64 IDs following the datablock IDs are used for messages.
   /// As message objects generally have a short life time this prevents them
   /// from eating object IDs as if they haven't eaten for a year.
   ///
   /// Note that unlike SimObjects and datablocks, Messages IDs are re-used.
   /// If you store a message object in script and do not clear the variable
   /// containing the object ID after freeing the message, it is probable that
   /// the object ID will become valid again.
   /// 
   /// @return Next available SimObjectId
   //-----------------------------------------------------------------------------
   static SimObjectId getNextMessageID();

   virtual bool onAdd();
   virtual void onRemove();

   //-----------------------------------------------------------------------------
   /// @brief Get the type of the message
   ///
   /// The message type is either the script class name or the C++ class name
   /// if it has not been overridden in script. This allows easy identification
   /// of message types with minimum effort.
   /// 
   /// @return Type of message
   //-----------------------------------------------------------------------------
   const char *getType();

   //-----------------------------------------------------------------------------
   /// @brief Add a reference to the reference count of this message
   ///
   /// Use freeReference() to free the reference when you are done with it.
   /// 
   /// @see freeReference()
   //-----------------------------------------------------------------------------
   void addReference() { incRefCount(); }

   //-----------------------------------------------------------------------------
   /// @brief Free a reference to this message
   /// 
   /// @see addReference()
   //-----------------------------------------------------------------------------
   void freeReference() { decRefCount(); }
};

// @}

#endif // _MESSAGE_H_
