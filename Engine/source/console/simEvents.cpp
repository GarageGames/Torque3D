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

#include "console/console.h"
#include "console/consoleInternal.h"
#include "platform/threads/semaphore.h"
#include "console/simEvents.h"

// Stupid globals not declared in a header
extern ExprEvalState gEvalState;

SimConsoleEvent::SimConsoleEvent(S32 argc, const char **argv, bool onObject)
{
   mOnObject = onObject;
   mArgc = argc;
   U32 totalSize = 0;
   S32 i;
   for(i = 0; i < argc; i++)
      totalSize += dStrlen(argv[i]) + 1;
   totalSize += sizeof(char *) * argc;

   mArgv = (char **) dMalloc(totalSize);
   char *argBase = (char *) &mArgv[argc];

   for(i = 0; i < argc; i++)
   {
      mArgv[i] = argBase;
      dStrcpy(mArgv[i], argv[i]);
      argBase += dStrlen(argv[i]) + 1;
   }
}

SimConsoleEvent::~SimConsoleEvent()
{
   dFree(mArgv);
}

void SimConsoleEvent::process(SimObject* object)
{
   // #ifdef DEBUG
   //    Con::printf("Executing schedule: %d", sequenceCount);
   // #endif
   if(mOnObject)
      Con::execute(object, mArgc, const_cast<const char**>( mArgv ));
   else
   {
      // Grab the function name. If '::' doesn't exist, then the schedule is
      // on a global function.
      char* func = dStrstr( mArgv[0], (char*)"::" );
      if( func )
      {
         // Set the first colon to NULL, so we can reference the namespace.
         // This is okay because events are deleted immediately after
         // processing. Maybe a bad idea anyway?
         func[0] = '\0';

         // Move the pointer forward to the function name.
         func += 2;

         // Lookup the namespace and function entry.
         Namespace* ns = Namespace::find( StringTable->insert( mArgv[0] ) );
         if( ns )
         {
            Namespace::Entry* nse = ns->lookup( StringTable->insert( func ) );
            if( nse )
               // Execute.
               nse->execute( mArgc, (const char**)mArgv, &gEvalState );
         }
      }

      else
         Con::execute(mArgc, const_cast<const char**>( mArgv ));
   }
}

//-----------------------------------------------------------------------------

SimConsoleThreadExecCallback::SimConsoleThreadExecCallback() : retVal(NULL)
{
   sem = new Semaphore(0);
}

SimConsoleThreadExecCallback::~SimConsoleThreadExecCallback()
{
   delete sem;
}

void SimConsoleThreadExecCallback::handleCallback(const char *ret)
{
   retVal = ret;
   sem->release();
}

const char *SimConsoleThreadExecCallback::waitForResult()
{
   if(sem->acquire(true))
   {
      return retVal;
   }

   return NULL;
}

//-----------------------------------------------------------------------------

SimConsoleThreadExecEvent::SimConsoleThreadExecEvent(S32 argc, const char **argv, bool onObject, SimConsoleThreadExecCallback *callback) :
   SimConsoleEvent(argc, argv, onObject), cb(callback)
{
}

void SimConsoleThreadExecEvent::process(SimObject* object)
{
   const char *retVal;
   if(mOnObject)
      retVal = Con::execute(object, mArgc, const_cast<const char**>( mArgv ));
   else
      retVal = Con::execute(mArgc, const_cast<const char**>( mArgv ));

   if(cb)
      cb->handleCallback(retVal);
}
