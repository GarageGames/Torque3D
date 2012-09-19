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

#ifndef _GAMEPROCESS_H_
#define _GAMEPROCESS_H_

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif
#ifndef _PROCESSLIST_H_
#include "T3D/gameBase/processList.h"
#endif


class GameBase;
class GameConnection;
struct Move;


class ClientProcessList : public ProcessList
{
   typedef ProcessList Parent;
   
public:

   ClientProcessList();
   
   // ProcessList
   void addObject( ProcessObject *pobj );  
   
   /// Called after a correction packet is received from the server.
   /// If the control object was corrected it will now play back any moves
   /// which were rolled back.
   virtual void clientCatchup( GameConnection *conn ) {}

   static ClientProcessList* get() { return smClientProcessList; }

protected:   
   
   // ProcessList
   void onPreTickObject( ProcessObject *pobj );

   /// Returns true if backlogged.
   bool doBacklogged( SimTime timeDelta );

protected:

   static ClientProcessList* smClientProcessList;
};


class ServerProcessList : public ProcessList
{
   typedef ProcessList Parent;

public:

   ServerProcessList();
   
   // ProcessList
   void addObject( ProcessObject *pobj );

   static ServerProcessList* get() { return smServerProcessList; }

protected:

   // ProcessList
   void onPreTickObject( ProcessObject *pobj );
   void advanceObjects();

protected:

   static ServerProcessList* smServerProcessList;
};


#endif // _GAMEPROCESS_H_