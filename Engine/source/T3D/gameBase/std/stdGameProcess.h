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

#ifndef _GAMEPROCESS_STD_H_
#define _GAMEPROCESS_STD_H_

//#include "T3D/gameBase/processList.h"
#ifndef _GAMEPROCESS_H_
#include "T3D/gameBase/gameProcess.h"
#endif

class GameBase;
class GameConnection;
struct Move;

//----------------------------------------------------------------------------

/// List to keep track of GameBases to process.
class StdClientProcessList : public ClientProcessList
{
   typedef ClientProcessList Parent;

protected:

   // ProcessList
   void onTickObject(ProcessObject *);
   void advanceObjects();
   void onAdvanceObjects();

public:

   StdClientProcessList();

   // ProcessList
   bool advanceTime( SimTime timeDelta );

   // ClientProcessList
   void clientCatchup( GameConnection *conn );

   static void init();
   static void shutdown();
};

class StdServerProcessList : public ServerProcessList
{
   typedef ServerProcessList Parent;

protected:

   // ProcessList
   void onPreTickObject( ProcessObject *pobj );
   void onTickObject( ProcessObject *pobj );
   void advanceObjects();

public:

   StdServerProcessList();

   static void init();
   static void shutdown();
};

#endif // _GAMEPROCESS_STD_H_
