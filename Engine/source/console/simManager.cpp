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

#include "platform/platform.h"
#include "platform/threads/mutex.h"
#include "console/simBase.h"
#include "console/simPersistID.h"
#include "core/stringTable.h"
#include "console/console.h"
#include "core/stream/fileStream.h"
#include "core/fileObject.h"
#include "console/consoleInternal.h"
#include "console/engineAPI.h"
#include "core/idGenerator.h"
#include "core/util/safeDelete.h"
#include "platform/platformIntrinsics.h"
#include "platform/profiler.h"
#include "math/mMathFn.h"

extern ExprEvalState gEvalState;

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

// We comment out the implementation of the Con namespace when doxygenizing because
// otherwise Doxygen decides to ignore our docs in console.h
#ifndef DOXYGENIZING

namespace Sim
{


//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// event queue variables:

SimTime gCurrentTime;
SimTime gTargetTime;

void *gEventQueueMutex;
SimEvent *gEventQueue;
U32 gEventSequence;

//---------------------------------------------------------------------------
// event queue init/shutdown

static void initEventQueue()
{
   gCurrentTime = 0;
   gTargetTime = 0;
   gEventSequence = 1;
   gEventQueue = NULL;
   gEventQueueMutex = Mutex::createMutex();
}

static void shutdownEventQueue()
{
   // Delete all pending events
   Mutex::lockMutex(gEventQueueMutex);
   SimEvent *walk = gEventQueue;
   while(walk)
   {
      SimEvent *temp = walk->nextEvent;
      delete walk;
      walk = temp;
   }
   Mutex::unlockMutex(gEventQueueMutex);
   Mutex::destroyMutex(gEventQueueMutex);
}

//---------------------------------------------------------------------------
// event post

U32 postEvent(SimObject *destObject, SimEvent* event,U32 time)
{
   AssertFatal(time == -1 || time >= getCurrentTime(),
      "Sim::postEvent() - Event time must be greater than or equal to the current time." );
   AssertFatal(destObject, "Sim::postEvent() - Destination object for event doesn't exist.");

   Mutex::lockMutex(gEventQueueMutex);

   if( time == -1 )
      time = gCurrentTime;

   event->time = time;
   event->startTime = gCurrentTime;
   event->destObject = destObject;

   if(!destObject)
   {
      delete event;

      Mutex::unlockMutex(gEventQueueMutex);

      return InvalidEventId;
   }
   event->sequenceCount = gEventSequence++;
   SimEvent **walk = &gEventQueue;
   SimEvent *current;
   
   while((current = *walk) != NULL && (current->time < event->time))
      walk = &(current->nextEvent);
   
   // [tom, 6/24/2005] This ensures that SimEvents are dispatched in the same order that they are posted.
   // This is needed to ensure Con::threadSafeExecute() executes script code in the correct order.
   while((current = *walk) != NULL && (current->time == event->time))
      walk = &(current->nextEvent);
   
   event->nextEvent = current;
   *walk = event;

   U32 seqCount = event->sequenceCount;

   Mutex::unlockMutex(gEventQueueMutex);

   return seqCount;
}

//---------------------------------------------------------------------------
// event cancellation

void cancelEvent(U32 eventSequence)
{
   Mutex::lockMutex(gEventQueueMutex);

   SimEvent **walk = &gEventQueue;
   SimEvent *current;
   
   while((current = *walk) != NULL)
   {
      if(current->sequenceCount == eventSequence)
      {
         *walk = current->nextEvent;
         delete current;
         Mutex::unlockMutex(gEventQueueMutex);
         return;
      }
      else
         walk = &(current->nextEvent);
   }

   Mutex::unlockMutex(gEventQueueMutex);
}

void cancelPendingEvents(SimObject *obj)
{
   Mutex::lockMutex(gEventQueueMutex);

   SimEvent **walk = &gEventQueue;
   SimEvent *current;
   
   while((current = *walk) != NULL)
   {
      if(current->destObject == obj)
      {
         *walk = current->nextEvent;
         delete current;
      }
      else
         walk = &(current->nextEvent);
   }
   Mutex::unlockMutex(gEventQueueMutex);
}

//---------------------------------------------------------------------------
// event pending test

bool isEventPending(U32 eventSequence)
{
   Mutex::lockMutex(gEventQueueMutex);

   for(SimEvent *walk = gEventQueue; walk; walk = walk->nextEvent)
      if(walk->sequenceCount == eventSequence)
      {
         Mutex::unlockMutex(gEventQueueMutex);
         return true;
      }
   Mutex::unlockMutex(gEventQueueMutex);
   return false;
}

U32 getEventTimeLeft(U32 eventSequence)
{
   Mutex::lockMutex(gEventQueueMutex);

   for(SimEvent *walk = gEventQueue; walk; walk = walk->nextEvent)
      if(walk->sequenceCount == eventSequence)
      {
         SimTime t = walk->time - getCurrentTime();
         Mutex::unlockMutex(gEventQueueMutex);
         return t;
      }

   Mutex::unlockMutex(gEventQueueMutex);

   return 0;   
}

U32 getScheduleDuration(U32 eventSequence)
{
   for(SimEvent *walk = gEventQueue; walk; walk = walk->nextEvent)
      if(walk->sequenceCount == eventSequence)
         return (walk->time-walk->startTime);
   return 0;
}

U32 getTimeSinceStart(U32 eventSequence)
{
   for(SimEvent *walk = gEventQueue; walk; walk = walk->nextEvent)
      if(walk->sequenceCount == eventSequence)
         return (getCurrentTime()-walk->startTime);
   return 0;
}

//---------------------------------------------------------------------------
// event timing

void advanceToTime(SimTime targetTime)
{
   AssertFatal(targetTime >= getCurrentTime(), 
      "Sim::advanceToTime() - Target time is less than the current time." );

   Mutex::lockMutex(gEventQueueMutex);

   gTargetTime = targetTime;
   while(gEventQueue && gEventQueue->time <= targetTime)
   {
      SimEvent *event = gEventQueue;
      gEventQueue = gEventQueue->nextEvent;
      AssertFatal(event->time >= gCurrentTime,
         "Sim::advanceToTime() - Event time is less than current time.");
      gCurrentTime = event->time;
      SimObject *obj = event->destObject;

      if(!obj->isDeleted())
         event->process(obj);
      delete event;
   }
   gCurrentTime = targetTime;

   Mutex::unlockMutex(gEventQueueMutex);
}

void advanceTime(SimTime delta)
{
   advanceToTime(getCurrentTime() + delta);
}

U32 getCurrentTime()
{
   return dAtomicRead( gCurrentTime);
}

U32 getTargetTime()
{
   return dAtomicRead( gTargetTime );
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

SimGroup *gRootGroup = NULL;
SimManagerNameDictionary *gNameDictionary;
SimIdDictionary *gIdDictionary;
U32 gNextObjectId;

static void initRoot()
{
   gIdDictionary = new SimIdDictionary;
   gNameDictionary = new SimManagerNameDictionary;

   gRootGroup = new SimGroup();
   gRootGroup->incRefCount();
 
   gRootGroup->setId(RootGroupId);
   gRootGroup->assignName("RootGroup");
   gRootGroup->registerObject();

   gNextObjectId = DynamicObjectIdFirst;
}

static void shutdownRoot()
{
   gRootGroup->decRefCount();
   if( engineAPI::gUseConsoleInterop )
      gRootGroup->deleteObject();
   gRootGroup = NULL;

   SAFE_DELETE(gNameDictionary);
   SAFE_DELETE(gIdDictionary);
}

//---------------------------------------------------------------------------

SimObject* findObject(const char* fileName, S32 declarationLine)
{
   PROFILE_SCOPE(SimFindObjectByLine);

   if (!fileName)
      return NULL;

   if (declarationLine < 0)
      return NULL;

   if (!gRootGroup)
      return NULL;

   return gRootGroup->findObjectByLineNumber(fileName, declarationLine, true);
}

SimObject* findObject(ConsoleValueRef &ref)
{
   return findObject((const char*)ref);
}

SimObject* findObject(const char* name)
{
   PROFILE_SCOPE(SimFindObject);

   // Play nice with bad code - JDD
   if( !name )
      return NULL;

   SimObject *obj;
   char c = *name;

   if (c == '%')
   {
      if (gEvalState.getStackDepth())
      {
         Dictionary::Entry* ent = gEvalState.getCurrentFrame().lookup(StringTable->insert(name));

         if (ent)
            return Sim::findObject(ent->getIntValue());
      }
   }
   if(c == '/')
      return gRootGroup->findObject(name + 1 );
   if(c >= '0' && c <= '9')
   {
      // it's an id group
      const char* temp = name + 1;
      for(;;)
      {
         c = *temp++;
         if(!c)
            return findObject(dAtoi(name));
         else if(c == '/')
         {
            obj = findObject(dAtoi(name));
            if(!obj)
               return NULL;
            return obj->findObject(temp);
         }
         else if (c < '0' || c > '9')
            return NULL;
      }
   }
   S32 len;

   for(len = 0; name[len] != 0 && name[len] != '/'; len++)
      ;
   StringTableEntry stName = StringTable->lookupn(name, len);
   if(!stName)
      return NULL;
   obj = gNameDictionary->find(stName);
   if(!name[len])
      return obj;
   if(!obj)
      return NULL;
   return obj->findObject(name + len + 1);
}

SimObject* findObject(SimObjectId id)
{
   return gIdDictionary->find(id);
}

SimObject *spawnObject(String spawnClass, String spawnDataBlock, String spawnName,
                       String spawnProperties, String spawnScript)
{
   if (spawnClass.isEmpty())
   {
      Con::errorf("Unable to spawn an object without a spawnClass");
      return NULL;
   }

   String spawnString;

   spawnString += "$SpawnObject = new " + spawnClass + "(" + spawnName + ") { ";

   if (spawnDataBlock.isNotEmpty() && !spawnDataBlock.equal( "None", String::NoCase ) )
      spawnString += "datablock = " + spawnDataBlock + "; ";

   if (spawnProperties.isNotEmpty())
      spawnString += spawnProperties + " ";

   spawnString += "};";

   // Evaluate our spawn string
   Con::evaluate(spawnString.c_str());

   // Get our spawnObject id
   const char* spawnObjectId = Con::getVariable("$SpawnObject");

   // Get the actual spawnObject
   SimObject* spawnObject = findObject(spawnObjectId);

   // If we have a spawn script go ahead and execute it last
   if (spawnScript.isNotEmpty())
      Con::evaluate(spawnScript.c_str(), true);

   return spawnObject;
}

SimGroup *getRootGroup()
{
   return gRootGroup;
}

String getUniqueName( const char *inName )
{
   String outName( inName );

   if ( outName.isEmpty() )
      return String::EmptyString;

   SimObject *dummy;

   if ( !Sim::findObject( outName, dummy ) )
      return outName;

   S32 suffixNumb = -1;
   String nameStr( String::GetTrailingNumber( outName, suffixNumb ) );
   suffixNumb = mAbs( suffixNumb ) + 1;

   #define MAX_TRIES 100

   for ( U32 i = 0; i < MAX_TRIES; i++ )
   {   
      outName = String::ToString( "%s%d", nameStr.c_str(), suffixNumb );

      if ( !Sim::findObject( outName, dummy ) )
         return outName;         

      suffixNumb++;
   }

   Con::errorf( "Sim::getUniqueName( %s ) - failed after %d attempts", inName, MAX_TRIES );
   return String::EmptyString;
}

String getUniqueInternalName( const char *inName, SimSet *inSet, bool searchChildren )
{
   // Since SimSet::findObjectByInternalName operates with StringTableEntry(s) 
   // we have to muck up the StringTable with our attempts. 
   // But then again, so does everywhere else.

   StringTableEntry outName = StringTable->insert( inName );

   if ( !outName || !outName[0] )   
      return String::EmptyString;

   if ( !inSet->findObjectByInternalName( outName, searchChildren ) )   
      return String(outName);

   S32 suffixNumb = -1;
   String nameStr( String::GetTrailingNumber( outName, suffixNumb ) );
   suffixNumb++;   

   static char tempStr[512];

#define MAX_TRIES 100

   for ( U32 i = 0; i < MAX_TRIES; i++ )
   {   
      dSprintf( tempStr, 512, "%s%d", nameStr.c_str(), suffixNumb );
      outName = StringTable->insert( tempStr );

      if ( !inSet->findObjectByInternalName( outName, searchChildren ) )
         return String(outName);         

      suffixNumb++;
   }

   Con::errorf( "Sim::getUniqueInternalName( %s ) - failed after %d attempts", inName, MAX_TRIES );
   return String::EmptyString;
}

bool isValidObjectName( const char* name )
{
   if( !name || !name[ 0 ] )
      return true; // Anonymous object.
      
   if( !dIsalpha( name[ 0 ] ) && name[ 0 ] != '_' )
      return false;
      
   for( U32 i = 1; name[ i ]; ++ i )
      if( !dIsalnum( name[ i ] ) && name[ i ] != '_' )
         return false;
         
   return true;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#define InstantiateNamedSet(set) g##set = new SimSet; g##set->registerObject(#set); g##set->setNameChangeAllowed(false); gRootGroup->addObject(g##set); SIMSET_SET_ASSOCIATION((*g##set))
#define InstantiateNamedGroup(set) g##set = new SimGroup; g##set->registerObject(#set); g##set->setNameChangeAllowed(false); gRootGroup->addObject(g##set); SIMSET_SET_ASSOCIATION((*g##set))

static bool sgIsShuttingDown;

SimDataBlockGroup *gDataBlockGroup;
SimDataBlockGroup *getDataBlockGroup()
{
   return gDataBlockGroup;
}


void init()
{
   initEventQueue();
   initRoot();

   InstantiateNamedSet(ActiveActionMapSet);
   InstantiateNamedSet(GhostAlwaysSet);
   InstantiateNamedSet(WayPointSet);
   InstantiateNamedSet(fxReplicatorSet);
   InstantiateNamedSet(fxFoliageSet);
   InstantiateNamedSet(MaterialSet);
   InstantiateNamedSet(SFXSourceSet);
   InstantiateNamedSet(SFXDescriptionSet);
   InstantiateNamedSet(SFXTrackSet);
   InstantiateNamedSet(SFXEnvironmentSet);
   InstantiateNamedSet(SFXStateSet);
   InstantiateNamedSet(SFXAmbienceSet);
   InstantiateNamedSet(TerrainMaterialSet);
   InstantiateNamedSet(DataBlockSet);
   InstantiateNamedGroup(ActionMapGroup);
   InstantiateNamedGroup(ClientGroup);
   InstantiateNamedGroup(GuiGroup);
   InstantiateNamedGroup(GuiDataGroup);
   InstantiateNamedGroup(TCPGroup);
   InstantiateNamedGroup(ClientConnectionGroup);
   InstantiateNamedGroup(SFXParameterGroup);
   InstantiateNamedSet(BehaviorSet);
   InstantiateNamedSet(sgMissionLightingFilterSet);

   gDataBlockGroup = new SimDataBlockGroup();
   gDataBlockGroup->registerObject("DataBlockGroup");
   gRootGroup->addObject(gDataBlockGroup);
   
   SimPersistID::init();
}

void shutdown()
{
   sgIsShuttingDown = true;
   
   shutdownRoot();
   shutdownEventQueue();
   
   SimPersistID::shutdown();
}

bool isShuttingDown()
{
   return sgIsShuttingDown;
}

}


#endif // DOXYGENIZING.

SimDataBlockGroup::SimDataBlockGroup()
{
   mLastModifiedKey = 0;
}

S32 QSORT_CALLBACK SimDataBlockGroup::compareModifiedKey(const void* a,const void* b)
{
   const SimDataBlock* dba = *((const SimDataBlock**)a);
   const SimDataBlock* dbb = *((const SimDataBlock**)b);

   return dba->getModifiedKey() - dbb->getModifiedKey();
}


void SimDataBlockGroup::sort()
{
   if(mLastModifiedKey != SimDataBlock::getNextModifiedKey())
   {
      mLastModifiedKey = SimDataBlock::getNextModifiedKey();
      dQsort(objectList.address(),objectList.size(),sizeof(SimObject *),compareModifiedKey);
   }
}
