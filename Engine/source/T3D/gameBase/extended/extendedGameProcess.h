#ifndef _GAMEPROCESS_EXTENDED_H_
#define _GAMEPROCESS_EXTENDED_H_

//#include "T3D/gameBase/processList.h"
#ifndef _GAMEPROCESS_H_
#include "T3D/gameBase/gameProcess.h"
#endif

class GameBase;
class GameConnection;
struct Move;

//----------------------------------------------------------------------------

/// List to keep track of GameBases to process.
class ExtendedClientProcessList : public ClientProcessList
{
   typedef ClientProcessList Parent;

protected:
   
   // ProcessList
   void onTickObject(ProcessObject *);
   void advanceObjects();
   void onAdvanceObjects();
   
public:

   ExtendedClientProcessList();  

   // ProcessList
   bool advanceTime( SimTime timeDelta );
   
   // ClientProcessList
   void clientCatchup( GameConnection *conn );
   
   static void init();
   static void shutdown();
};

class ExtendedServerProcessList : public ServerProcessList
{
   typedef ServerProcessList Parent;

protected:

   // ProcessList
   void onPreTickObject( ProcessObject *pobj );
   void onTickObject( ProcessObject *pobj );
   void advanceObjects();

public:

   ExtendedServerProcessList();  

   static void init();
   static void shutdown();
};

#endif   // _GAMEPROCESS_EXTENDED_H_
