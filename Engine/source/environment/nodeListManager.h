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

#ifndef _NODELISTMANAGER_H_
#define _NODELISTMANAGER_H_

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif
#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif
#ifndef _MQUAT_H_
#include "math/mQuat.h"
#endif
#ifndef _NETCONNECTION_H_
#include "sim/netConnection.h"
#endif

//-----------------------------------------------------------------------------

class NodeListNotify;

class NodeListManager
{
public:
   
   struct NodeList
   {
      U32   mId;
      U32   mTotalValidNodes;
      bool  mListComplete;

      NodeList() { mTotalValidNodes=0; mListComplete=false; }
      virtual ~NodeList() { }
   };

   static U32 smMaximumNodesPerEvent;

protected:
   bool  mIsServer;
   U32   mNextListId;

   Vector<NodeList*>       mNodeLists;
   Vector<NodeListNotify*> mNotifyList;

public:
   NodeListManager( const bool isServer );
   ~NodeListManager();

   void clearNodeLists();

   U32 nextListId();

   void addNodeList( NodeList* list );

   bool findListById( U32 id, NodeList** list, bool completeOnly=true );

   void clearNotification( U32 listId );
   void clearAllNotifications();
   void registerNotification( NodeListNotify* notify );
   bool dispatchNotification( U32 listId );
   bool dispatchNotification( NodeList* list );
};

extern NodeListManager* gClientNodeListManager;
extern NodeListManager* gServerNodeListManager;

//-----------------------------------------------------------------------------

class NodeListNotify
{
protected:
   U32  mListId;

public:
   NodeListNotify() { }
   virtual ~NodeListNotify() { }

   U32 getListId() { return mListId; }

   virtual void sendNotification( NodeListManager::NodeList* list ) { }
};

//-----------------------------------------------------------------------------

class NodeListEvent : public NetEvent
{
   typedef NetEvent Parent;

public:
   U32         mId;
   U32         mTotalNodes;
   U32         mLocalListStart;

   NodeListManager::NodeList* mNodeList;

public:
   NodeListEvent() { mNodeList=NULL; mTotalNodes = mLocalListStart = 0; }
   virtual ~NodeListEvent();

   virtual void pack(NetConnection*, BitStream*);
   virtual void write(NetConnection*, BitStream*);
   virtual void unpack(NetConnection*, BitStream*);
   virtual void process(NetConnection*);
   virtual void mergeLists(NodeListManager::NodeList* oldList);

   virtual void copyIntoList(NodeListManager::NodeList* copyInto) { }
   virtual void padListToSize() { }

   DECLARE_CONOBJECT(NodeListEvent);
};

#endif   // _NODELISTMANAGER_H_
