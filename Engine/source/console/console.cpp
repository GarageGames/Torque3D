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
#include "platform/platformTLS.h"
#include "platform/threads/thread.h"
#include "console/console.h"
#include "console/consoleInternal.h"
#include "console/consoleObject.h"
#include "console/consoleParser.h"
#include "core/stream/fileStream.h"
#include "console/ast.h"
#include "core/tAlgorithm.h"
#include "console/consoleTypes.h"
#include "console/telnetDebugger.h"
#include "console/simBase.h"
#include "console/compiler.h"
#include "console/stringStack.h"
#include "console/ICallMethod.h"
#include "console/engineAPI.h"
#include <stdarg.h>
#include "platform/threads/mutex.h"


extern StringStack STR;
extern ConsoleValueStack CSTK;

ConsoleDocFragment* ConsoleDocFragment::smFirst;
ExprEvalState gEvalState;
StmtNode *gStatementList;
StmtNode *gAnonFunctionList;
U32 gAnonFunctionID = 0;
ConsoleConstructor *ConsoleConstructor::first = NULL;
bool gWarnUndefinedScriptVariables;

static char scratchBuffer[4096];

CON_DECLARE_PARSER(CMD);

static const char * prependDollar ( const char * name )
{
   if(name[0] != '$')
   {
      S32   len = dStrlen(name);
      AssertFatal(len < sizeof(scratchBuffer)-2, "CONSOLE: name too long");
      scratchBuffer[0] = '$';
      dMemcpy(scratchBuffer + 1, name, len + 1);
      name = scratchBuffer;
   }
   return name;
}

static const char * prependPercent ( const char * name )
{
   if(name[0] != '%')
   {
      S32   len = dStrlen(name);
      AssertFatal(len < sizeof(scratchBuffer)-2, "CONSOLE: name too long");
      scratchBuffer[0] = '%';
      dMemcpy(scratchBuffer + 1, name, len + 1);
      name = scratchBuffer;
   }
   return name;
}

//--------------------------------------
void ConsoleConstructor::init( const char *cName, const char *fName, const char *usg, S32 minArgs, S32 maxArgs, bool isToolOnly, ConsoleFunctionHeader* header )
{
   mina = minArgs;
   maxa = maxArgs;
   funcName = fName;
   usage = usg;
   className = cName;
   sc = 0; fc = 0; vc = 0; bc = 0; ic = 0;
   callback = group = false;
   next = first;
   ns = false;
   first = this;
   toolOnly = isToolOnly;
   this->header = header;
}

void ConsoleConstructor::setup()
{
   for(ConsoleConstructor *walk = first; walk; walk = walk->next)
   {
#ifdef TORQUE_DEBUG
      walk->validate();
#endif

      if( walk->sc )
         Con::addCommand( walk->className, walk->funcName, walk->sc, walk->usage, walk->mina, walk->maxa, walk->toolOnly, walk->header );
      else if( walk->ic )
         Con::addCommand( walk->className, walk->funcName, walk->ic, walk->usage, walk->mina, walk->maxa, walk->toolOnly, walk->header );
      else if( walk->fc )
         Con::addCommand( walk->className, walk->funcName, walk->fc, walk->usage, walk->mina, walk->maxa, walk->toolOnly, walk->header );
      else if( walk->vc )
         Con::addCommand( walk->className, walk->funcName, walk->vc, walk->usage, walk->mina, walk->maxa, walk->toolOnly, walk->header );
      else if( walk->bc )
         Con::addCommand( walk->className, walk->funcName, walk->bc, walk->usage, walk->mina, walk->maxa, walk->toolOnly, walk->header );
      else if( walk->group )
         Con::markCommandGroup( walk->className, walk->funcName, walk->usage );
      else if( walk->callback )
         Con::noteScriptCallback( walk->className, walk->funcName, walk->usage, walk->header );
      else if( walk->ns )
      {
         Namespace* ns = Namespace::find( StringTable->insert( walk->className ) );
         if( ns )
            ns->mUsage = walk->usage;
      }
      else
      {
         AssertISV( false, "Found a ConsoleConstructor with an indeterminate type!" );
      }
   }
}

ConsoleConstructor::ConsoleConstructor(const char *className, const char *funcName, StringCallback sfunc, const char *usage, S32 minArgs, S32 maxArgs, bool isToolOnly, ConsoleFunctionHeader* header )
{
   init( className, funcName, usage, minArgs, maxArgs, isToolOnly, header );
   sc = sfunc;
}

ConsoleConstructor::ConsoleConstructor(const char *className, const char *funcName, IntCallback ifunc, const char *usage, S32 minArgs, S32 maxArgs, bool isToolOnly, ConsoleFunctionHeader* header )
{
   init( className, funcName, usage, minArgs, maxArgs, isToolOnly, header );
   ic = ifunc;
}

ConsoleConstructor::ConsoleConstructor(const char *className, const char *funcName, FloatCallback ffunc, const char *usage, S32 minArgs, S32 maxArgs, bool isToolOnly, ConsoleFunctionHeader* header )
{
   init( className, funcName, usage, minArgs, maxArgs, isToolOnly, header );
   fc = ffunc;
}

ConsoleConstructor::ConsoleConstructor(const char *className, const char *funcName, VoidCallback vfunc, const char *usage, S32 minArgs, S32 maxArgs, bool isToolOnly, ConsoleFunctionHeader* header )
{
   init( className, funcName, usage, minArgs, maxArgs, isToolOnly, header );
   vc = vfunc;
}

ConsoleConstructor::ConsoleConstructor(const char *className, const char *funcName, BoolCallback bfunc, const char *usage, S32 minArgs, S32 maxArgs, bool isToolOnly, ConsoleFunctionHeader* header )
{
   init( className, funcName, usage, minArgs, maxArgs, isToolOnly, header );
   bc = bfunc;
}

ConsoleConstructor::ConsoleConstructor(const char* className, const char* groupName, const char* aUsage)
{
   init(className, groupName, usage, -1, -2);

   group = true;

   // Somewhere, the entry list is getting flipped, partially.
   // so we have to do tricks to deal with making sure usage
   // is properly populated.

   // This is probably redundant.
   static char * lastUsage = NULL;
   if(aUsage)
      lastUsage = (char *)aUsage;

   usage = lastUsage;
}

ConsoleConstructor::ConsoleConstructor(const char *className, const char *callbackName, const char *usage, ConsoleFunctionHeader* header )
{
   init( className, callbackName, usage, -2, -3, false, header );
   callback = true;
   ns = true;
}

void ConsoleConstructor::validate()
{
#ifdef TORQUE_DEBUG
   // Don't do the following check if we're not a method/func.
   if(this->group)
      return;

   // In debug, walk the list and make sure this isn't a duplicate.
   for(ConsoleConstructor *walk = first; walk; walk = walk->next)
   {
      // Skip mismatching func/method names.
      if(dStricmp(walk->funcName, this->funcName))
         continue;

      // Don't compare functions with methods or vice versa.
      if(bool(this->className) != bool(walk->className))
         continue;

      // Skip mismatching classnames, if they're present.
      if(this->className && walk->className && dStricmp(walk->className, this->className))
         continue;

      // If we encounter ourselves, stop searching; this prevents duplicate
      // firing of the assert, instead only firing for the latter encountered
      // entry.
      if(this == walk)
         break;

      // Match!
      if(this->className)
      {
         AssertISV(false, avar("ConsoleConstructor::setup - ConsoleMethod '%s::%s' collides with another of the same name.", this->className, this->funcName));
      }
      else
      {
         AssertISV(false, avar("ConsoleConstructor::setup - ConsoleFunction '%s' collides with another of the same name.", this->funcName));
      }
   }
#endif
}

// We comment out the implementation of the Con namespace when doxygenizing because
// otherwise Doxygen decides to ignore our docs in console.h
#ifndef DOXYGENIZING

namespace Con
{

static Vector<ConsumerCallback> gConsumers(__FILE__, __LINE__);
static Vector< String > sInstantGroupStack( __FILE__, __LINE__ );
static DataChunker consoleLogChunker;
static Vector<ConsoleLogEntry> consoleLog(__FILE__, __LINE__);
static bool consoleLogLocked;
static bool logBufferEnabled=true;
static S32 printLevel = 10;
static FileStream consoleLogFile;
static const char *defLogFileName = "console.log";
static S32 consoleLogMode = 0;
static bool active = false;
static bool newLogFile;
static const char *logFileName;

static const S32 MaxCompletionBufferSize = 4096;
static char completionBuffer[MaxCompletionBufferSize];
static char tabBuffer[MaxCompletionBufferSize] = {0};
static SimObjectPtr<SimObject> tabObject;
static U32 completionBaseStart;
static U32 completionBaseLen;

String gInstantGroup;
Con::ConsoleInputEvent smConsoleInput;

/// Current script file name and root, these are registered as
/// console variables.
/// @{

///
StringTableEntry gCurrentFile;
StringTableEntry gCurrentRoot;
/// @}

S32 gObjectCopyFailures = -1;

bool alwaysUseDebugOutput = true;
bool useTimestamp = false;

ConsoleFunctionGroupBegin( Clipboard, "Miscellaneous functions to control the clipboard and clear the console.");

DefineConsoleFunction( cls, void, (), , "()"
				"@brief Clears the console output.\n\n"
				"@ingroup Console")
{
   if(consoleLogLocked)
      return;
   consoleLogChunker.freeBlocks();
   consoleLog.setSize(0);
};

DefineConsoleFunction( getClipboard, const char*, (), , "()"
				"@brief Get text from the clipboard.\n\n"
				"@internal")
{
	return Platform::getClipboard();
};

DefineConsoleFunction( setClipboard, bool, (const char* text), , "(string text)"
               "@brief Set the system clipboard.\n\n"
			   "@internal")
{
	return Platform::setClipboard(text);
};

ConsoleFunctionGroupEnd( Clipboard );


void postConsoleInput( RawData data );

void init()
{
   AssertFatal(active == false, "Con::init should only be called once.");

   // Set up general init values.
   active                        = true;
   logFileName                   = NULL;
   newLogFile                    = true;
   gWarnUndefinedScriptVariables = false;

   // Initialize subsystems.
   Namespace::init();
   ConsoleConstructor::setup();

   // Set up the parser(s)
   CON_ADD_PARSER(CMD,   "cs",   true);   // TorqueScript

   // Setup the console types.
   ConsoleBaseType::initialize();

   // And finally, the ACR...
   AbstractClassRep::initialize();

   // Variables
   setVariable("Con::prompt", "% ");
   addVariable("Con::logBufferEnabled", TypeBool, &logBufferEnabled, "If true, the log buffer will be enabled.\n"
	   "@ingroup Console\n");
   addVariable("Con::printLevel", TypeS32, &printLevel, 
      "@brief This is deprecated.\n\n"
      "It is no longer in use and does nothing.\n"      
	   "@ingroup Console\n");
   addVariable("Con::warnUndefinedVariables", TypeBool, &gWarnUndefinedScriptVariables, "If true, a warning will be displayed in the console whenever a undefined variable is used in script.\n"
	   "@ingroup Console\n");
   addVariable( "instantGroup", TypeRealString, &gInstantGroup, "The group that objects will be added to when they are created.\n"
	   "@ingroup Console\n");

   addVariable("Con::objectCopyFailures", TypeS32, &gObjectCopyFailures, "If greater than zero then it counts the number of object creation "
      "failures based on a missing copy object and does not report an error..\n"
	   "@ingroup Console\n");   

   // Current script file name and root
   addVariable( "Con::File", TypeString, &gCurrentFile, "The currently executing script file.\n"
	   "@ingroup FileSystem\n");
   addVariable( "Con::Root", TypeString, &gCurrentRoot, "The mod folder for the currently executing script file.\n"
	   "@ingroup FileSystem\n" );

   // alwaysUseDebugOutput determines whether to send output to the platform's 
   // "debug" system.  see winConsole for an example.  
   // in ship builds we don't expose this variable to script
   // and we set it to false by default (don't want to provide more information
   // to potential hackers).  platform code should also ifdef out the code that 
   // pays attention to this in ship builds (see winConsole.cpp) 
   // note that enabling this can slow down your game 
   // if you are running from the debugger and printing a lot of console messages.
#ifndef TORQUE_SHIPPING
   addVariable("Con::alwaysUseDebugOutput", TypeBool, &alwaysUseDebugOutput, 
      "@brief Determines whether to send output to the platform's \"debug\" system.\n\n" 
      "@note This is disabled in shipping builds.\n"
	   "@ingroup Console");
#else
   alwaysUseDebugOutput = false;
#endif

   // controls whether a timestamp is prepended to every console message
   addVariable("Con::useTimestamp", TypeBool, &useTimestamp, "If true a timestamp is prepended to every console message.\n"
	   "@ingroup Console\n");

   // Plug us into the journaled console input signal.
   smConsoleInput.notify(postConsoleInput);
}

//--------------------------------------

void shutdown()
{
   AssertFatal(active == true, "Con::shutdown should only be called once.");
   active = false;

   smConsoleInput.remove(postConsoleInput);

   consoleLogFile.close();
   Namespace::shutdown();
   AbstractClassRep::shutdown();
   Compiler::freeConsoleParserList();
}

bool isActive()
{
   return active;
}

bool isMainThread()
{
#ifdef TORQUE_MULTITHREAD
   return ThreadManager::isMainThread();
#else
   // If we're single threaded we're always in the main thread.
   return true;
#endif
}

//--------------------------------------

void getLockLog(ConsoleLogEntry *&log, U32 &size)
{
   consoleLogLocked = true;
   log = consoleLog.address();
   size = consoleLog.size();
}

void unlockLog()
{
   consoleLogLocked = false;
}

U32 tabComplete(char* inputBuffer, U32 cursorPos, U32 maxResultLength, bool forwardTab)
{
   // Check for null input.
   if (!inputBuffer[0]) 
   {
      return cursorPos;
   }

   // Cap the max result length.
   if (maxResultLength > MaxCompletionBufferSize) 
   {
      maxResultLength = MaxCompletionBufferSize;
   }

   // See if this is the same partial text as last checked.
   if (dStrcmp(tabBuffer, inputBuffer)) 
   {
      // If not...
      // Save it for checking next time.
      dStrcpy(tabBuffer, inputBuffer);
      // Scan backward from the cursor position to find the base to complete from.
      S32 p = cursorPos;
      while ((p > 0) && (inputBuffer[p - 1] != ' ') && (inputBuffer[p - 1] != '.') && (inputBuffer[p - 1] != '('))
      {
         p--;
      }
      completionBaseStart = p;
      completionBaseLen = cursorPos - p;
      // Is this function being invoked on an object?
      if (inputBuffer[p - 1] == '.') 
      {
         // If so...
         if (p <= 1) 
         {
            // Bail if no object identifier.
            return cursorPos;
         }

         // Find the object identifier.
         S32 objLast = --p;
         while ((p > 0) && (inputBuffer[p - 1] != ' ') && (inputBuffer[p - 1] != '(')) 
         {
            p--;
         }

         if (objLast == p) 
         {
            // Bail if no object identifier.
            return cursorPos;
         }

         // Look up the object identifier.
         dStrncpy(completionBuffer, inputBuffer + p, objLast - p);
         completionBuffer[objLast - p] = 0;
         tabObject = Sim::findObject(completionBuffer);
         if (tabObject == NULL) 
         {
            // Bail if not found.
            return cursorPos;
         }
      }
      else 
      {
         // Not invoked on an object; we'll use the global namespace.
         tabObject = 0;
      }
   }

   // Chop off the input text at the cursor position.
   inputBuffer[cursorPos] = 0;

   // Try to find a completion in the appropriate namespace.
   const char *newText;

   if (tabObject != 0)
   {
      newText = tabObject->tabComplete(inputBuffer + completionBaseStart, completionBaseLen, forwardTab);
   }
   else 
   {
      // In the global namespace, we can complete on global vars as well as functions.
      if (inputBuffer[completionBaseStart] == '$')
      {
         newText = gEvalState.globalVars.tabComplete(inputBuffer + completionBaseStart, completionBaseLen, forwardTab);
      }
      else 
      {
         newText = Namespace::global()->tabComplete(inputBuffer + completionBaseStart, completionBaseLen, forwardTab);
      }
   }

   if (newText) 
   {
      // If we got something, append it to the input text.
      S32 len = dStrlen(newText);
      if (len + completionBaseStart > maxResultLength)
      {
         len = maxResultLength - completionBaseStart;
      }
      dStrncpy(inputBuffer + completionBaseStart, newText, len);
      inputBuffer[completionBaseStart + len] = 0;
      // And set the cursor after it.
      cursorPos = completionBaseStart + len;
   }

   // Save the modified input buffer for checking next time.
   dStrcpy(tabBuffer, inputBuffer);

   // Return the new (maybe) cursor position.
   return cursorPos;
}

//------------------------------------------------------------------------------
static void log(const char *string)
{
   // Bail if we ain't logging.
   if (!consoleLogMode) 
   {
      return;
   }

   // In mode 1, we open, append, close on each log write.
   if ((consoleLogMode & 0x3) == 1) 
   {
      consoleLogFile.open(defLogFileName, Torque::FS::File::ReadWrite);
   }

   // Write to the log if its status is hunky-dory.
   if ((consoleLogFile.getStatus() == Stream::Ok) || (consoleLogFile.getStatus() == Stream::EOS)) 
   {
      consoleLogFile.setPosition(consoleLogFile.getStreamSize());
      // If this is the first write...
      if (newLogFile) 
      {
         // Make a header.
         Platform::LocalTime lt;
         Platform::getLocalTime(lt);
         char buffer[128];
         dSprintf(buffer, sizeof(buffer), "//-------------------------- %d/%d/%d -- %02d:%02d:%02d -----\r\n",
               lt.month + 1,
               lt.monthday,
               lt.year + 1900,
               lt.hour,
               lt.min,
               lt.sec);
         consoleLogFile.write(dStrlen(buffer), buffer);
         newLogFile = false;
         if (consoleLogMode & 0x4) 
         {
            // Dump anything that has been printed to the console so far.
            consoleLogMode -= 0x4;
            U32 size, line;
            ConsoleLogEntry *log;
            getLockLog(log, size);
            for (line = 0; line < size; line++) 
            {
               consoleLogFile.write(dStrlen(log[line].mString), log[line].mString);
               consoleLogFile.write(2, "\r\n");
            }
            unlockLog();
         }
      }
      // Now write what we came here to write.
      consoleLogFile.write(dStrlen(string), string);
      consoleLogFile.write(2, "\r\n");
   }

   if ((consoleLogMode & 0x3) == 1) 
   {
      consoleLogFile.close();
   }
}

//------------------------------------------------------------------------------

static void _printf(ConsoleLogEntry::Level level, ConsoleLogEntry::Type type, const char* fmt, va_list argptr)
{
   if (!active)
	   return;
   Con::active = false; 

   char buffer[8192];
   U32 offset = 0;
   if( gEvalState.traceOn && gEvalState.getStackDepth() > 0 )
   {
      offset = gEvalState.getStackDepth() * 3;
      for(U32 i = 0; i < offset; i++)
         buffer[i] = ' ';
   }

   if (useTimestamp)
   {
      static U32 startTime = Platform::getRealMilliseconds();
      U32 curTime = Platform::getRealMilliseconds() - startTime;
      offset += dSprintf(buffer + offset, sizeof(buffer) - offset, "[+%4d.%03d]", U32(curTime * 0.001), curTime % 1000);
   }
   dVsprintf(buffer + offset, sizeof(buffer) - offset, fmt, argptr);

   for(S32 i = 0; i < gConsumers.size(); i++)
      gConsumers[i](level, buffer);

   if(logBufferEnabled || consoleLogMode)
   {
      char *pos = buffer;
      while(*pos)
      {
         if(*pos == '\t')
            *pos = '^';
         pos++;
      }
      pos = buffer;

      for(;;)
      {
         char *eofPos = dStrchr(pos, '\n');
         if(eofPos)
            *eofPos = 0;

         log(pos);
         if(logBufferEnabled && !consoleLogLocked)
         {
            ConsoleLogEntry entry;
            entry.mLevel  = level;
            entry.mType   = type;
#ifndef TORQUE_SHIPPING // this is equivalent to a memory leak, turn it off in ship build            
            entry.mString = (const char *)consoleLogChunker.alloc(dStrlen(pos) + 1);
            dStrcpy(const_cast<char*>(entry.mString), pos);
            
            // This prevents infinite recursion if the console itself needs to
            // re-allocate memory to accommodate the new console log entry, and 
            // LOG_PAGE_ALLOCS is defined. It is kind of a dirty hack, but the
            // uses for LOG_PAGE_ALLOCS are limited, and it is not worth writing
            // a lot of special case code to support this situation. -patw
            const bool save = Con::active;
            Con::active = false;
            consoleLog.push_back(entry);
            Con::active = save;
#endif
         }
         if(!eofPos)
            break;
         pos = eofPos + 1;
      }
   }

   Con::active = true;
}

//------------------------------------------------------------------------------
void printf(const char* fmt,...)
{
   va_list argptr;
   va_start(argptr, fmt);
   _printf(ConsoleLogEntry::Normal, ConsoleLogEntry::General, fmt, argptr);
   va_end(argptr);
}

void warnf(ConsoleLogEntry::Type type, const char* fmt,...)
{
   va_list argptr;
   va_start(argptr, fmt);
   _printf(ConsoleLogEntry::Warning, type, fmt, argptr);
   va_end(argptr);
}

void errorf(ConsoleLogEntry::Type type, const char* fmt,...)
{
   va_list argptr;
   va_start(argptr, fmt);
   _printf(ConsoleLogEntry::Error, type, fmt, argptr);
   va_end(argptr);
}

void warnf(const char* fmt,...)
{
   va_list argptr;
   va_start(argptr, fmt);
   _printf(ConsoleLogEntry::Warning, ConsoleLogEntry::General, fmt, argptr);
   va_end(argptr);
}

void errorf(const char* fmt,...)
{
   va_list argptr;
   va_start(argptr, fmt);
   _printf(ConsoleLogEntry::Error, ConsoleLogEntry::General, fmt, argptr);
   va_end(argptr);
}

//---------------------------------------------------------------------------

bool getVariableObjectField(const char *name, SimObject **object, const char **field)
{
   // get the field info from the object..
   const char *dot = dStrchr(name, '.');
   if(name[0] != '$' && dot)
   {
      S32 len = dStrlen(name);
      AssertFatal(len < sizeof(scratchBuffer)-1, "Sim::getVariable - name too long");
      dMemcpy(scratchBuffer, name, len+1);

      char * token = dStrtok(scratchBuffer, ".");
      SimObject * obj = Sim::findObject(token);
      if(!obj)
         return false;

      token = dStrtok(0, ".\0");
      if(!token)
         return false;

      while(token != NULL)
      {
         const char * val = obj->getDataField(StringTable->insert(token), 0);
         if(!val)
            return false;

         char *fieldToken = token;
         token = dStrtok(0, ".\0");
         if(token)
         {
            obj = Sim::findObject(token);
            if(!obj)
               return false;
         }
         else
         {
            *object = obj;
            *field = fieldToken;
            return true;
         }
      }
   }

   return false;
}

Dictionary::Entry *getLocalVariableEntry(const char *name)
{
   name = prependPercent(name);
   return gEvalState.getCurrentFrame().lookup(StringTable->insert(name));
}

Dictionary::Entry *getVariableEntry(const char *name)
{
   name = prependDollar(name);
   return gEvalState.globalVars.lookup(StringTable->insert(name));
}

Dictionary::Entry *addVariableEntry(const char *name)
{
   name = prependDollar(name);
   return gEvalState.globalVars.add(StringTable->insert(name));
}

Dictionary::Entry *getAddVariableEntry(const char *name)
{
   name = prependDollar(name);
   StringTableEntry stName = StringTable->insert(name);
   Dictionary::Entry *entry = gEvalState.globalVars.lookup(stName);
   if (!entry)
	   entry = gEvalState.globalVars.add(stName);
   return entry;
}

Dictionary::Entry *getAddLocalVariableEntry(const char *name)
{
   name = prependPercent(name);
   StringTableEntry stName = StringTable->insert(name);
   Dictionary::Entry *entry = gEvalState.getCurrentFrame().lookup(stName);
   if (!entry)
	   entry = gEvalState.getCurrentFrame().add(stName);
   return entry;
}

void setVariable(const char *name, const char *value)
{
   SimObject *obj = NULL;
   const char *objField = NULL;

   if (getVariableObjectField(name, &obj, &objField))
   {
	   obj->setDataField(StringTable->insert(objField), 0, value);
   }
   else 
   {
      name = prependDollar(name);
      gEvalState.globalVars.setVariable(StringTable->insert(name), value);
   }
}

void setLocalVariable(const char *name, const char *value)
{
   name = prependPercent(name);
   gEvalState.getCurrentFrame().setVariable(StringTable->insert(name), value);
}

void setBoolVariable(const char *varName, bool value)
{
   SimObject *obj = NULL;
   const char *objField = NULL;

   if (getVariableObjectField(varName, &obj, &objField))
   {
	   obj->setDataField(StringTable->insert(objField), 0, value ? "1" : "0");
   }
   else
   {
      varName = prependDollar(varName);
      Dictionary::Entry *entry = getAddVariableEntry(varName);
	  entry->setStringValue(value ? "1" : "0");
   }
}

void setIntVariable(const char *varName, S32 value)
{
   SimObject *obj = NULL;
   const char *objField = NULL;

   if (getVariableObjectField(varName, &obj, &objField))
   {
	   char scratchBuffer[32];
	   dSprintf(scratchBuffer, sizeof(scratchBuffer), "%d", value);
	   obj->setDataField(StringTable->insert(objField), 0, scratchBuffer);
   }
   else
   {
      varName = prependDollar(varName);
      Dictionary::Entry *entry = getAddVariableEntry(varName);
      entry->setIntValue(value);
   }
}

void setFloatVariable(const char *varName, F32 value)
{
   SimObject *obj = NULL;
   const char *objField = NULL;

   if (getVariableObjectField(varName, &obj, &objField))
   {
	   char scratchBuffer[32];
	   dSprintf(scratchBuffer, sizeof(scratchBuffer), "%g", value);
	   obj->setDataField(StringTable->insert(objField), 0, scratchBuffer);
   }
   else
   {
      varName = prependDollar(varName);
      Dictionary::Entry *entry = getAddVariableEntry(varName);
	  entry->setFloatValue(value);
   }
}

//---------------------------------------------------------------------------
void addConsumer(ConsumerCallback consumer)
{
   gConsumers.push_back(consumer);
}

// dhc - found this empty -- trying what I think is a reasonable impl.
void removeConsumer(ConsumerCallback consumer)
{
   for(S32 i = 0; i < gConsumers.size(); i++)
   {
      if (gConsumers[i] == consumer)
      {
         // remove it from the list.
         gConsumers.erase(i);
         break;
      }
   }
}

void stripColorChars(char* line)
{
   char* c = line;
   char cp = *c;
   while (cp) 
   {
      if (cp < 18) 
      {
         // Could be a color control character; let's take a closer look.
         if ((cp != 8) && (cp != 9) && (cp != 10) && (cp != 13)) 
         {
            // Yep... copy following chars forward over this.
            char* cprime = c;
            char cpp;
            do 
            {
               cpp = *++cprime;
               *(cprime - 1) = cpp;
            } 
            while (cpp);
            // Back up 1 so we'll check this position again post-copy.
            c--;
         }
      }
      cp = *++c;
   }
}

// 
const char *getObjectTokenField(const char *name)
{
   const char *dot = dStrchr(name, '.');
   if(name[0] != '$' && dot)
   {
      S32 len = dStrlen(name);
      AssertFatal(len < sizeof(scratchBuffer)-1, "Sim::getVariable - object name too long");
      dMemcpy(scratchBuffer, name, len+1);

      char * token = dStrtok(scratchBuffer, ".");
      SimObject * obj = Sim::findObject(token);
      if(!obj)
         return("");

      token = dStrtok(0, ".\0");
      if(!token)
         return("");

      while(token != NULL)
      {
         const char * val = obj->getDataField(StringTable->insert(token), 0);
         if(!val)
            return("");

         token = dStrtok(0, ".\0");
         if(token)
         {
            obj = Sim::findObject(token);
            if(!obj)
               return("");
         }
         else
            return(val);
      }
   }

   return NULL;
}

const char *getVariable(const char *name)
{
   const char *objField = getObjectTokenField(name);
   if (objField)
   {
      return objField;
   }
   else
   {
      Dictionary::Entry *entry = getVariableEntry(name);
      return entry ? entry->getStringValue() : "";
   }
}

const char *getLocalVariable(const char *name)
{
   name = prependPercent(name);

   return gEvalState.getCurrentFrame().getVariable(StringTable->insert(name));
}

bool getBoolVariable(const char *varName, bool def)
{
   const char *objField = getObjectTokenField(varName);
   if (objField)
   {
      return *objField ? dAtob(objField) : def;
   }
   else
   {
      Dictionary::Entry *entry = getVariableEntry(varName);
      objField = entry ? entry->getStringValue() : "";
      return *objField ? dAtob(objField) : def;
   }
}

S32 getIntVariable(const char *varName, S32 def)
{
   const char *objField = getObjectTokenField(varName);
   if (objField)
   {
      return *objField ? dAtoi(objField) : def;
   }
   else
   {
      Dictionary::Entry *entry = getVariableEntry(varName);
      return entry ? entry->getIntValue() : def;
   }
}

F32 getFloatVariable(const char *varName, F32 def)
{
   const char *objField = getObjectTokenField(varName);
   if (objField)
   {
      return *objField ? dAtof(objField) : def;
   }
   else
   {
      Dictionary::Entry *entry = getVariableEntry(varName);
	   return entry ? entry->getFloatValue() : def;
   }
}

//---------------------------------------------------------------------------

void addVariable(    const char *name, 
                     S32 type, 
                     void *dptr, 
                     const char* usage )
{
   gEvalState.globalVars.addVariable( name, type, dptr, usage );
}

void addConstant(    const char *name, 
                     S32 type, 
                     const void *dptr, 
                     const char* usage )
{
   Dictionary::Entry* entry = gEvalState.globalVars.addVariable( name, type, const_cast< void* >( dptr ), usage );
   entry->mIsConstant = true;
}

bool removeVariable(const char *name)
{
   name = StringTable->lookup(prependDollar(name));
   return name!=0 && gEvalState.globalVars.removeVariable(name);
}

void addVariableNotify( const char *name, const NotifyDelegate &callback )
{
   gEvalState.globalVars.addVariableNotify( name, callback );
}

void removeVariableNotify( const char *name, const NotifyDelegate &callback )
{
   gEvalState.globalVars.removeVariableNotify( name, callback );
}

//---------------------------------------------------------------------------

void addCommand( const char *nsName, const char *name,StringCallback cb, const char *usage, S32 minArgs, S32 maxArgs, bool isToolOnly, ConsoleFunctionHeader* header )
{
   Namespace *ns = lookupNamespace(nsName);
   ns->addCommand( StringTable->insert(name), cb, usage, minArgs, maxArgs, isToolOnly, header );
}

void addCommand( const char *nsName, const char *name,VoidCallback cb, const char *usage, S32 minArgs, S32 maxArgs, bool isToolOnly, ConsoleFunctionHeader* header )
{
   Namespace *ns = lookupNamespace(nsName);
   ns->addCommand( StringTable->insert(name), cb, usage, minArgs, maxArgs, isToolOnly, header );
}

void addCommand( const char *nsName, const char *name,IntCallback cb, const char *usage, S32 minArgs, S32 maxArgs, bool isToolOnly, ConsoleFunctionHeader* header )
{
   Namespace *ns = lookupNamespace(nsName);
   ns->addCommand( StringTable->insert(name), cb, usage, minArgs, maxArgs, isToolOnly, header );
}

void addCommand( const char *nsName, const char *name,FloatCallback cb, const char *usage, S32 minArgs, S32 maxArgs, bool isToolOnly, ConsoleFunctionHeader* header )
{
   Namespace *ns = lookupNamespace(nsName);
   ns->addCommand( StringTable->insert(name), cb, usage, minArgs, maxArgs, isToolOnly, header );
}

void addCommand( const char *nsName, const char *name,BoolCallback cb, const char *usage, S32 minArgs, S32 maxArgs, bool isToolOnly, ConsoleFunctionHeader* header )
{
   Namespace *ns = lookupNamespace(nsName);
   ns->addCommand( StringTable->insert(name), cb, usage, minArgs, maxArgs, isToolOnly, header );
}

void noteScriptCallback( const char *className, const char *funcName, const char *usage, ConsoleFunctionHeader* header )
{
   Namespace *ns = lookupNamespace(className);
   ns->addScriptCallback( StringTable->insert(funcName), usage, header );
}

void markCommandGroup(const char * nsName, const char *name, const char* usage)
{
   Namespace *ns = lookupNamespace(nsName);
   ns->markGroup(name,usage);
}

void beginCommandGroup(const char * nsName, const char *name, const char* usage)
{
   markCommandGroup(nsName, name, usage);
}

void endCommandGroup(const char * nsName, const char *name)
{
   markCommandGroup(nsName, name, NULL);
}

void addCommand( const char *name,StringCallback cb,const char *usage, S32 minArgs, S32 maxArgs, bool isToolOnly, ConsoleFunctionHeader* header )
{
   Namespace::global()->addCommand( StringTable->insert(name), cb, usage, minArgs, maxArgs, isToolOnly, header );
}

void addCommand( const char *name,VoidCallback cb,const char *usage, S32 minArgs, S32 maxArgs, bool isToolOnly, ConsoleFunctionHeader* header )
{
   Namespace::global()->addCommand( StringTable->insert(name), cb, usage, minArgs, maxArgs, isToolOnly, header );
}

void addCommand( const char *name,IntCallback cb,const char *usage, S32 minArgs, S32 maxArgs, bool isToolOnly, ConsoleFunctionHeader* header )
{
   Namespace::global()->addCommand( StringTable->insert(name), cb, usage, minArgs, maxArgs, isToolOnly, header );
}

void addCommand( const char *name,FloatCallback cb,const char *usage, S32 minArgs, S32 maxArgs, bool isToolOnly, ConsoleFunctionHeader* header )
{
   Namespace::global()->addCommand( StringTable->insert(name), cb, usage, minArgs, maxArgs, isToolOnly, header );
}

void addCommand( const char *name,BoolCallback cb,const char *usage, S32 minArgs, S32 maxArgs, bool isToolOnly, ConsoleFunctionHeader* header )
{
   Namespace::global()->addCommand( StringTable->insert(name), cb, usage, minArgs, maxArgs, isToolOnly, header );
}

ConsoleValueRef evaluate(const char* string, bool echo, const char *fileName)
{
   ConsoleStackFrameSaver stackSaver;
   stackSaver.save();

   if (echo)
   {
      if (string[0] == '%')
         Con::printf("%s", string);
      else
         Con::printf("%s%s", getVariable( "$Con::Prompt" ), string);
   }

   if(fileName)
      fileName = StringTable->insert(fileName);

   CodeBlock *newCodeBlock = new CodeBlock();
   return newCodeBlock->compileExec(fileName, string, false, fileName ? -1 : 0);
}

//------------------------------------------------------------------------------
ConsoleValueRef evaluatef(const char* string, ...)
{
   ConsoleStackFrameSaver stackSaver;
   stackSaver.save();

   char buffer[4096];
   va_list args;
   va_start(args, string);
   dVsprintf(buffer, sizeof(buffer), string, args);
   va_end(args);
   CodeBlock *newCodeBlock = new CodeBlock();
   return newCodeBlock->compileExec(NULL, buffer, false, 0);
}

//------------------------------------------------------------------------------

// Internal execute for global function which does not save the stack
ConsoleValueRef _internalExecute(S32 argc, ConsoleValueRef argv[])
{
   Namespace::Entry *ent;
   StringTableEntry funcName = StringTable->insert(argv[0]);
   ent = Namespace::global()->lookup(funcName);

   if(!ent)
   {
      warnf(ConsoleLogEntry::Script, "%s: Unknown command.", (const char*)argv[0]);

      STR.clearFunctionOffset();
      return ConsoleValueRef();
   }
   return ent->execute(argc, argv, &gEvalState);
}

ConsoleValueRef execute(S32 argc, ConsoleValueRef argv[])
{
#ifdef TORQUE_MULTITHREAD
   if(isMainThread())
   {
#endif
      ConsoleStackFrameSaver stackSaver;
      stackSaver.save();
	   return _internalExecute(argc, argv);
#ifdef TORQUE_MULTITHREAD
   }
   else
   {
      SimConsoleThreadExecCallback cb;
      SimConsoleThreadExecEvent *evt = new SimConsoleThreadExecEvent(argc, argv, false, &cb);
      Sim::postEvent(Sim::getRootGroup(), evt, Sim::getCurrentTime());
      
      return cb.waitForResult();
   }
#endif
}

ConsoleValueRef execute(S32 argc, const char *argv[])
{
   ConsoleStackFrameSaver stackSaver;
   stackSaver.save();
   StringStackConsoleWrapper args(argc, argv);
   return execute(args.count(), args);
}

//------------------------------------------------------------------------------

// Internal execute for object method which does not save the stack
ConsoleValueRef _internalExecute(SimObject *object, S32 argc, ConsoleValueRef argv[], bool thisCallOnly)
{
   if(argc < 2)
   {
      STR.clearFunctionOffset();
      return ConsoleValueRef();
   }

   // [neo, 10/05/2007 - #3010]
   // Make sure we don't get recursive calls, respect the flag!   
   // Should we be calling handlesMethod() first?
   if( !thisCallOnly )
   {
      ICallMethod *com = dynamic_cast<ICallMethod *>(object);
      if(com)
      {
         STR.pushFrame();
         CSTK.pushFrame();
         com->callMethodArgList(argc, argv, false);
         STR.popFrame();
         CSTK.popFrame();
      }
   }

   if(object->getNamespace())
   {
      U32 ident = object->getId();
      ConsoleValueRef oldIdent(argv[1]);

      StringTableEntry funcName = StringTable->insert(argv[0]);
      Namespace::Entry *ent = object->getNamespace()->lookup(funcName);

      if(ent == NULL)
      {
         //warnf(ConsoleLogEntry::Script, "%s: undefined for object '%s' - id %d", funcName, object->getName(), object->getId());

         STR.clearFunctionOffset();
         return ConsoleValueRef();
      }

      // Twiddle %this argument
      ConsoleValue func_ident;
      func_ident.setIntValue((S32)ident);
      argv[1] = ConsoleValueRef::fromValue(&func_ident);

      SimObject *save = gEvalState.thisObject;
      gEvalState.thisObject = object;
      ConsoleValueRef ret = ent->execute(argc, argv, &gEvalState);
      gEvalState.thisObject = save;

      // Twiddle it back
      argv[1] = oldIdent;

      return ret;
   }

   warnf(ConsoleLogEntry::Script, "Con::execute - %d has no namespace: %s", object->getId(), (const char*)argv[0]);
   STR.clearFunctionOffset();
   return ConsoleValueRef();
}


ConsoleValueRef execute(SimObject *object, S32 argc, ConsoleValueRef argv[], bool thisCallOnly)
{
   if(argc < 2)
   {
      STR.clearFunctionOffset();
      return ConsoleValueRef();
   }

   ConsoleStackFrameSaver stackSaver;
   stackSaver.save();

   if (object->getNamespace() || !thisCallOnly)
   {
      if (isMainThread())
      {
         return _internalExecute(object, argc, argv, thisCallOnly);
      }
      else
      {
         SimConsoleThreadExecCallback cb;
         SimConsoleThreadExecEvent *evt = new SimConsoleThreadExecEvent(argc, argv, true, &cb);
         Sim::postEvent(object, evt, Sim::getCurrentTime());
      }
   }

   warnf(ConsoleLogEntry::Script, "Con::execute - %d has no namespace: %s", object->getId(), (const char*)argv[0]);
   STR.clearFunctionOffset();
   return ConsoleValueRef();
}

ConsoleValueRef execute(SimObject *object, S32 argc, const char *argv[], bool thisCallOnly)
{
   ConsoleStackFrameSaver stackSaver;
   stackSaver.save();
   StringStackConsoleWrapper args(argc, argv);
   return execute(object, args.count(), args, thisCallOnly);
}

inline ConsoleValueRef _executef(SimObject *obj, S32 checkArgc, S32 argc, ConsoleValueRef *argv)
{
   const U32 maxArg = 12;
   AssertWarn(checkArgc == argc, "Incorrect arg count passed to Con::executef(SimObject*)");
   AssertFatal(argc <= maxArg - 1, "Too many args passed to Con::_executef(SimObject*). Please update the function to handle more.");
   return execute(obj, argc, argv);
}

//------------------------------------------------------------------------------
inline ConsoleValueRef _executef(S32 checkArgc, S32 argc, ConsoleValueRef *argv)
{
   const U32 maxArg = 10;
   AssertFatal(checkArgc == argc, "Incorrect arg count passed to Con::executef()");
   AssertFatal(argc <= maxArg, "Too many args passed to Con::_executef(). Please update the function to handle more.");
   return execute(argc, argv);
}

//------------------------------------------------------------------------------
bool isFunction(const char *fn)
{
   const char *string = StringTable->lookup(fn);
   if(!string)
      return false;
   else
      return Namespace::global()->lookup(string) != NULL;
}

//------------------------------------------------------------------------------

void setLogMode(S32 newMode)
{
   if ((newMode & 0x3) != (consoleLogMode & 0x3)) {
      if (newMode && !consoleLogMode) {
         // Enabling logging when it was previously disabled.
         newLogFile = true;
      }
      if ((consoleLogMode & 0x3) == 2) {
         // Changing away from mode 2, must close logfile.
         consoleLogFile.close();
      }
      else if ((newMode & 0x3) == 2) {
#ifdef _XBOX
         // Xbox is not going to support logging to a file. Use the OutputDebugStr
         // log consumer
         Platform::debugBreak();
#endif
         // Starting mode 2, must open logfile.
         consoleLogFile.open(defLogFileName, Torque::FS::File::Write);
      }
      consoleLogMode = newMode;
   }
}

Namespace *lookupNamespace(const char *ns)
{
   if(!ns)
      return Namespace::global();
   return Namespace::find(StringTable->insert(ns));
}

bool linkNamespaces(const char *parent, const char *child)
{
   Namespace *pns = lookupNamespace(parent);
   Namespace *cns = lookupNamespace(child);
   if(pns && cns)
      return cns->classLinkTo(pns);
   return false;
}

bool unlinkNamespaces(const char *parent, const char *child)
{
   Namespace *pns = lookupNamespace(parent);
   Namespace *cns = lookupNamespace(child);

   if(pns == cns)
   {
      Con::warnf("Con::unlinkNamespaces - trying to unlink '%s' from itself, aborting.", parent);
      return false;
   }

   if(pns && cns)
      return cns->unlinkClass(pns);

   return false;
}

bool classLinkNamespaces(Namespace *parent, Namespace *child)
{
   if(parent && child)
      return child->classLinkTo(parent);
   return false;
}

void setData(S32 type, void *dptr, S32 index, S32 argc, const char **argv, const EnumTable *tbl, BitSet32 flag)
{
   ConsoleBaseType *cbt = ConsoleBaseType::getType(type);
   AssertFatal(cbt, "Con::setData - could not resolve type ID!");
   cbt->setData((void *) (((const char *)dptr) + index * cbt->getTypeSize()),argc, argv, tbl, flag);
}

const char *getData(S32 type, void *dptr, S32 index, const EnumTable *tbl, BitSet32 flag)
{
   ConsoleBaseType *cbt = ConsoleBaseType::getType(type);
   AssertFatal(cbt, "Con::getData - could not resolve type ID!");
   return cbt->getData((void *) (((const char *)dptr) + index * cbt->getTypeSize()), tbl, flag);
}

const char *getFormattedData(S32 type, const char *data, const EnumTable *tbl, BitSet32 flag)
{
   ConsoleBaseType *cbt = ConsoleBaseType::getType( type );
   AssertFatal(cbt, "Con::getData - could not resolve type ID!");

   // Datablock types are just a datablock 
   // name and don't ever need formatting.
   if ( cbt->isDatablock() )
      return data;

   bool currWarn = gWarnUndefinedScriptVariables;
   gWarnUndefinedScriptVariables = false;

   const char* globalValue = Con::getVariable(data);

   gWarnUndefinedScriptVariables = currWarn;

   if (dStrlen(globalValue) > 0)
      return globalValue;

   void* variable = cbt->getNativeVariable();

   if (variable)
   {
      Con::setData(type, variable, 0, 1, &data, tbl, flag);
      const char* formattedVal = Con::getData(type, variable, 0, tbl, flag);

      static const U32 bufSize = 2048;
      char* returnBuffer = Con::getReturnBuffer(bufSize);
      dSprintf(returnBuffer, bufSize, "%s\0", formattedVal );

      cbt->deleteNativeVariable(variable);

      return returnBuffer;
   }
   else
      return data;
}

//------------------------------------------------------------------------------

bool isCurrentScriptToolScript()
{
   // With a player build we ALWAYS return false
#ifndef TORQUE_TOOLS
   return false;
#else
   const StringTableEntry cbFullPath = CodeBlock::getCurrentCodeBlockFullPath();
   if(cbFullPath == NULL)
      return false;
   const StringTableEntry exePath = Platform::getMainDotCsDir();

   return dStrnicmp(exePath, cbFullPath, dStrlen(exePath)) == 0;
#endif
}

//------------------------------------------------------------------------------

StringTableEntry getModNameFromPath(const char *path)
{
   if(path == NULL || *path == 0)
      return NULL;

   char buf[1024];
   buf[0] = 0;

   if(path[0] == '/' || path[1] == ':')
   {
      // It's an absolute path
      const StringTableEntry exePath = Platform::getMainDotCsDir();
      U32 len = dStrlen(exePath);
      if(dStrnicmp(exePath, path, len) == 0)
      {
         const char *ptr = path + len + 1;
         const char *slash = dStrchr(ptr, '/');
         if(slash)
         {
            dStrncpy(buf, ptr, slash - ptr);
            buf[slash - ptr] = 0;
         }
         else
            return NULL;
      }
      else
         return NULL;
   }
   else
   {
      const char *slash = dStrchr(path, '/');
      if(slash)
      {
         dStrncpy(buf, path, slash - path);
         buf[slash - path] = 0;
      }
      else
         return NULL;
   }

   return StringTable->insert(buf);
}

void postConsoleInput( RawData data )
{
   // Schedule this to happen at the next time event.
   ConsoleValue values[2];
   ConsoleValueRef argv[2];

   values[0].init();
   values[0].setStringValue("eval");
   values[1].init();
   values[1].setStringValue((const char*)data.data);
   argv[0].value = &values[0];
   argv[1].value = &values[1];

   Sim::postCurrentEvent(Sim::getRootGroup(), new SimConsoleEvent(2, argv, false));
}

//------------------------------------------------------------------------------

void pushInstantGroup( String name )
{
   sInstantGroupStack.push_back( gInstantGroup );
   gInstantGroup = name;
}

void popInstantGroup()
{
   if( sInstantGroupStack.empty() )
      gInstantGroup = String::EmptyString;
   else
   {
      gInstantGroup = sInstantGroupStack.last();
      sInstantGroupStack.pop_back();
   }
}


typedef HashMap<StringTableEntry, StringTableEntry> typePathExpandoMap;
static typePathExpandoMap PathExpandos;

//-----------------------------------------------------------------------------

void addPathExpando(const char* pExpandoName, const char* pPath)
{
   // Sanity!
   AssertFatal(pExpandoName != NULL, "Expando name cannot be NULL.");
   AssertFatal(pPath != NULL, "Expando path cannot be NULL.");

   // Fetch expando name.
   StringTableEntry expandoName = StringTable->insert(pExpandoName);

   // Fetch the length of the path.
   S32 pathLength = dStrlen(pPath);

   char pathBuffer[1024];

   // Sanity!
   if (pathLength == 0 || pathLength >= sizeof(pathBuffer))
   {
      Con::warnf("Cannot add path expando '%s' with path '%s' as the path is an invalid length.", pExpandoName, pPath);
      return;
   }

   // Strip repeat slashes.
   if (!Con::stripRepeatSlashes(pathBuffer, pPath, sizeof(pathBuffer)))
   {
      Con::warnf("Cannot add path expando '%s' with path '%s' as the path is an invalid length.", pExpandoName, pPath);
      return;
   }

   // Fetch new path length.
   pathLength = dStrlen(pathBuffer);

   // Sanity!
   if (pathLength == 0)
   {
      Con::warnf("Cannot add path expando '%s' with path '%s' as the path is an invalid length.", pExpandoName, pPath);
      return;
   }

   // Remove any terminating slash.
   if (pathBuffer[pathLength - 1] == '/')
      pathBuffer[pathLength - 1] = 0;

   // Fetch expanded path.
   StringTableEntry expandedPath = StringTable->insert(pathBuffer);

   // Info.
#if defined(TORQUE_DEBUG)
   Con::printf("Adding path expando of '%s' as '%s'.", expandoName, expandedPath);
#endif

   // Find any existing path expando.
   typePathExpandoMap::iterator expandoItr = PathExpandos.find(pExpandoName);

   // Does the expando exist?
   if (expandoItr != PathExpandos.end())
   {
      // Yes, so modify the path.
      expandoItr->value = expandedPath;
      return;
   }

   // Insert expando.
   PathExpandos.insert(expandoName, expandedPath);
}

//-----------------------------------------------------------------------------

StringTableEntry getPathExpando(const char* pExpandoName)
{
   // Sanity!
   AssertFatal(pExpandoName != NULL, "Expando name cannot be NULL.");

   // Fetch expando name.
   StringTableEntry expandoName = StringTable->insert(pExpandoName);

   // Find any existing path expando.
   typePathExpandoMap::iterator expandoItr = PathExpandos.find(expandoName);

   // Does the expando exist?
   if (expandoItr != PathExpandos.end())
   {
      // Yes, so return it.
      return expandoItr->value;
   }

   // Not found.
   return NULL;
}

//-----------------------------------------------------------------------------

void removePathExpando(const char* pExpandoName)
{
   // Sanity!
   AssertFatal(pExpandoName != NULL, "Expando name cannot be NULL.");

   // Fetch expando name.
   StringTableEntry expandoName = StringTable->insert(pExpandoName);

   // Find any existing path expando.
   typePathExpandoMap::iterator expandoItr = PathExpandos.find(expandoName);

   // Does the expando exist?
   if (expandoItr == PathExpandos.end())
   {
      // No, so warn.
#if defined(TORQUE_DEBUG)
      Con::warnf("Removing path expando of '%s' but it does not exist.", expandoName);
#endif
      return;
   }

   // Info.
#if defined(TORQUE_DEBUG)
   Con::printf("Removing path expando of '%s' as '%s'.", expandoName, expandoItr->value);
#endif
   // Remove expando.
   PathExpandos.erase(expandoItr);
}

//-----------------------------------------------------------------------------

bool isPathExpando(const char* pExpandoName)
{
   // Sanity!
   AssertFatal(pExpandoName != NULL, "Expando name cannot be NULL.");

   // Fetch expando name.
   StringTableEntry expandoName = StringTable->insert(pExpandoName);

   return PathExpandos.contains(expandoName);
}

//-----------------------------------------------------------------------------

U32 getPathExpandoCount(void)
{
   return PathExpandos.size();
}

//-----------------------------------------------------------------------------

StringTableEntry getPathExpandoKey(U32 expandoIndex)
{
   // Finish if index is out of range.
   if (expandoIndex >= PathExpandos.size())
      return NULL;

   // Find indexed iterator.
   typePathExpandoMap::iterator expandoItr = PathExpandos.begin();
   while (expandoIndex > 0) { ++expandoItr; --expandoIndex; }

   return expandoItr->key;
}

//-----------------------------------------------------------------------------

StringTableEntry getPathExpandoValue(U32 expandoIndex)
{
   // Finish if index is out of range.
   if (expandoIndex >= PathExpandos.size())
      return NULL;

   // Find indexed iterator.
   typePathExpandoMap::iterator expandoItr = PathExpandos.begin();
   while (expandoIndex > 0) { ++expandoItr; --expandoIndex; }

   return expandoItr->value;
}

//-----------------------------------------------------------------------------

bool expandPath(char* pDstPath, U32 size, const char* pSrcPath, const char* pWorkingDirectoryHint, const bool ensureTrailingSlash)
{
   char pathBuffer[2048];
   const char* pSrc = pSrcPath;
   char* pSlash;

   // Fetch leading character.
   const char leadingToken = *pSrc;

   // Fetch following token.
   const char followingToken = leadingToken != 0 ? pSrc[1] : 0;

   // Expando.
   if (leadingToken == '^')
   {
      // Initial prefix search.
      const char* pPrefixSrc = pSrc + 1;
      char* pPrefixDst = pathBuffer;

      // Search for end of expando.
      while (*pPrefixSrc != '/' && *pPrefixSrc != 0)
      {
         // Copy prefix character.
         *pPrefixDst++ = *pPrefixSrc++;
      }

      // Yes, so terminate the expando string.
      *pPrefixDst = 0;

      // Fetch the expando path.
      StringTableEntry expandoPath = getPathExpando(pathBuffer);

      // Does the expando exist?
      if (expandoPath == NULL)
      {
         // No, so error.
         Con::errorf("expandPath() : Could not find path expando '%s' for path '%s'.", pathBuffer, pSrcPath);

         // Are we ensuring the trailing slash?
         if (ensureTrailingSlash)
         {
            // Yes, so ensure it.
            Con::ensureTrailingSlash(pDstPath, pSrcPath);
         }
         else
         {
            // No, so just use the source path.
            dStrcpy(pDstPath, pSrcPath);
         }

         return false;
      }

      // Skip the expando and the following slash.
      pSrc += dStrlen(pathBuffer) + 1;

      // Format the output path.
      dSprintf(pathBuffer, sizeof(pathBuffer), "%s/%s", expandoPath, pSrc);

      // Are we ensuring the trailing slash?
      if (ensureTrailingSlash)
      {
         // Yes, so ensure it.
         Con::ensureTrailingSlash(pathBuffer, pathBuffer);
      }

      // Strip repeat slashes.
      Con::stripRepeatSlashes(pDstPath, pathBuffer, size);

      return true;
   }

   // Script-Relative.
   if (leadingToken == '.')
   {
      // Fetch the code-block file-path.
      const StringTableEntry codeblockFullPath = CodeBlock::getCurrentCodeBlockFullPath();

      // Do we have a code block full path?
      if (codeblockFullPath == NULL)
      {
         // No, so error.
         Con::errorf("expandPath() : Could not find relative path from code-block for path '%s'.", pSrcPath);

         // Are we ensuring the trailing slash?
         if (ensureTrailingSlash)
         {
            // Yes, so ensure it.
            Con::ensureTrailingSlash(pDstPath, pSrcPath);
         }
         else
         {
            // No, so just use the source path.
            dStrcpy(pDstPath, pSrcPath);
         }

         return false;
      }

      // Yes, so use it as the prefix.
      dStrncpy(pathBuffer, codeblockFullPath, sizeof(pathBuffer) - 1);

      // Find the final slash in the code-block.
      pSlash = dStrrchr(pathBuffer, '/');

      // Is this a parent directory token?
      if (followingToken == '.')
      {
         // Yes, so terminate after the slash so we include it.
         pSlash[1] = 0;
      }
      else
      {
         // No, it's a current directory token so terminate at the slash so we don't include it.
         pSlash[0] = 0;

         // Skip the current directory token.
         pSrc++;
      }

      // Format the output path.
      dStrncat(pathBuffer, "/", sizeof(pathBuffer) - 1 - strlen(pathBuffer));
      dStrncat(pathBuffer, pSrc, sizeof(pathBuffer) - 1 - strlen(pathBuffer));

      // Are we ensuring the trailing slash?
      if (ensureTrailingSlash)
      {
         // Yes, so ensure it.
         Con::ensureTrailingSlash(pathBuffer, pathBuffer);
      }

      // Strip repeat slashes.
      Con::stripRepeatSlashes(pDstPath, pathBuffer, size);

      return true;
   }

   // All else.

   //Using a special case here because the code below barfs on trying to build a full path for apk reading
#ifdef TORQUE_OS_ANDROID
   if (leadingToken == '/' || strstr(pSrcPath, "/") == NULL)
      Platform::makeFullPathName(pSrcPath, pathBuffer, sizeof(pathBuffer), pWorkingDirectoryHint);
   else
      dSprintf(pathBuffer, sizeof(pathBuffer), "/%s", pSrcPath);
#else
   Platform::makeFullPathName(pSrcPath, pathBuffer, sizeof(pathBuffer), pWorkingDirectoryHint);
#endif

   // Are we ensuring the trailing slash?
   if (ensureTrailingSlash)
   {
      // Yes, so ensure it.
      Con::ensureTrailingSlash(pathBuffer, pathBuffer);
   }

   // Strip repeat slashes.
   Con::stripRepeatSlashes(pDstPath, pathBuffer, size);

   return true;
}

//-----------------------------------------------------------------------------

bool isBasePath(const char* SrcPath, const char* pBasePath)
{
   char expandBuffer[1024];
   Con::expandPath(expandBuffer, sizeof(expandBuffer), SrcPath);
   return dStrnicmp(pBasePath, expandBuffer, dStrlen(pBasePath)) == 0;
}

//-----------------------------------------------------------------------------

void collapsePath(char* pDstPath, U32 size, const char* pSrcPath, const char* pWorkingDirectoryHint)
{
   // Check path against expandos.  If there are multiple matches, choose the
   // expando that produces the shortest relative path.

   char pathBuffer[2048];

   // Fetch expando count.
   const U32 expandoCount = getPathExpandoCount();

   // Iterate expandos.
   U32 expandoRelativePathLength = U32_MAX;
   for (U32 expandoIndex = 0; expandoIndex < expandoCount; ++expandoIndex)
   {
      // Fetch expando value (path).
      StringTableEntry expandoValue = getPathExpandoValue(expandoIndex);

      // Skip if not the base path.
      if (!isBasePath(pSrcPath, expandoValue))
         continue;

      // Fetch path relative to expando path.
      StringTableEntry relativePath = Platform::makeRelativePathName(pSrcPath, expandoValue);

      // If the relative path is simply a period
      if (relativePath[0] == '.')
         relativePath++;

      if (dStrlen(relativePath) > expandoRelativePathLength)
      {
         // This expando covers less of the path than any previous one found.
         // We will keep the previous one.
         continue;
      }

      // Keep track of the relative path length
      expandoRelativePathLength = dStrlen(relativePath);

      // Fetch expando key (name).
      StringTableEntry expandoName = getPathExpandoKey(expandoIndex);

      // Format against expando.
      dSprintf(pathBuffer, sizeof(pathBuffer), "^%s/%s", expandoName, relativePath);
   }

   // Check if we've found a suitable expando
   if (expandoRelativePathLength != U32_MAX)
   {
      // Strip repeat slashes.
      Con::stripRepeatSlashes(pDstPath, pathBuffer, size);

      return;
   }

   // Fetch the working directory.
   StringTableEntry workingDirectory = pWorkingDirectoryHint != NULL ? pWorkingDirectoryHint : Platform::getCurrentDirectory();

   // Fetch path relative to current directory.
   StringTableEntry relativePath = Platform::makeRelativePathName(pSrcPath, workingDirectory);

   // If the relative path is simply a period
   if (relativePath[0] == '.'  && relativePath[1] != '.')
      relativePath++;

   // Format against expando.
   dSprintf(pathBuffer, sizeof(pathBuffer), "%s/%s", workingDirectory, relativePath);

   // Strip repeat slashes.
   Con::stripRepeatSlashes(pDstPath, pathBuffer, size);
}


void ensureTrailingSlash(char* pDstPath, const char* pSrcPath)
{
   // Copy to target.
   dStrcpy(pDstPath, pSrcPath);

   // Find trailing character index.
   S32 trailIndex = dStrlen(pDstPath);

   // Ignore if empty string.
   if (trailIndex == 0)
      return;

   // Finish if the trailing slash already exists.
   if (pDstPath[trailIndex - 1] == '/')
      return;

   // Add trailing slash.
   pDstPath[trailIndex++] = '/';
   pDstPath[trailIndex] = 0;
}

//-----------------------------------------------------------------------------

bool stripRepeatSlashes(char* pDstPath, const char* pSrcPath, S32 dstSize)
{
   // Note original destination.
   char* pOriginalDst = pDstPath;

   // Reset last source character.
   char lastSrcChar = 0;

   // Search source...
   while (dstSize > 0)
   {
      // Fetch characters.
      const char srcChar = *pSrcPath++;

      // Do we have a repeat slash?
      if (srcChar == '/' && lastSrcChar == '/')
      {
         // Yes, so skip it.
         continue;
      }

      // No, so copy character.
      *pDstPath++ = srcChar;

      // Finish if end of source.
      if (srcChar == 0)
         return true;

      // Reduce room left in destination.
      dstSize--;

      // Set last character.
      lastSrcChar = srcChar;
   }

   // Terminate the destination string as we ran out of room.
   *pOriginalDst = 0;

   // Fail!
   return false;
}

} // end of Console namespace

#endif

//=============================================================================
//    API.
//=============================================================================
// MARK: ---- API ----

//-----------------------------------------------------------------------------

DefineEngineFunction( log, void, ( const char* message ),,
   "@brief Logs a message to the console.\n\n"
   "@param message The message text.\n"
   "@note By default, messages will appear white in the console.\n"
   "@ingroup Logging")
{
   Con::printf( "%s", message );
}

//-----------------------------------------------------------------------------

DefineEngineFunction( logError, void, ( const char* message ),,
   "@brief Logs an error message to the console.\n\n"
   "@param message The message text.\n"
   "@note By default, errors will appear red in the console.\n"
   "@ingroup Logging")
{
   Con::errorf( "%s", message );
}

//-----------------------------------------------------------------------------

DefineEngineFunction( logWarning, void, ( const char* message ),,
   "@brief Logs a warning message to the console.\n\n"
   "@param message The message text.\n\n"
   "@note By default, warnings will appear turquoise in the console.\n"
   "@ingroup Logging")
{
   Con::warnf( "%s", message );
}

//------------------------------------------------------------------------------

extern ConsoleValueStack CSTK;

ConsoleValueRef::ConsoleValueRef(const ConsoleValueRef &ref)
{
   value = ref.value;
}

ConsoleValueRef& ConsoleValueRef::operator=(const ConsoleValueRef &newValue)
{
   value = newValue.value;
   return *this;
}

ConsoleValueRef& ConsoleValueRef::operator=(const char *newValue)
{
   AssertFatal(value, "value should not be NULL");
   value->setStringValue(newValue);
   return *this;
}

ConsoleValueRef& ConsoleValueRef::operator=(S32 newValue)
{
   AssertFatal(value, "value should not be NULL");
   value->setIntValue(newValue);
   return *this;
}

ConsoleValueRef& ConsoleValueRef::operator=(U32 newValue)
{
   AssertFatal(value, "value should not be NULL");
   value->setIntValue(newValue);
   return *this;
}

ConsoleValueRef& ConsoleValueRef::operator=(F32 newValue)
{
   AssertFatal(value, "value should not be NULL");
   value->setFloatValue(newValue);
   return *this;
}

ConsoleValueRef& ConsoleValueRef::operator=(F64 newValue)
{
   AssertFatal(value, "value should not be NULL");
   value->setFloatValue(newValue);
   return *this;
}

//------------------------------------------------------------------------------

StringStackWrapper::StringStackWrapper(int targc, ConsoleValueRef targv[])
{
   argv = new const char*[targc];
   argc = targc;

   for (int i=0; i<targc; i++)
   {
      argv[i] = dStrdup(targv[i]);
   }
}

StringStackWrapper::~StringStackWrapper()
{
   for (int i=0; i<argc; i++)
   {
      dFree(argv[i]);
   }
   delete[] argv;
}


StringStackConsoleWrapper::StringStackConsoleWrapper(int targc, const char** targ)
{
   argv = new ConsoleValueRef[targc];
   argvValue = new ConsoleValue[targc];
   argc = targc;

   for (int i=0; i<targc; i++) {
      argvValue[i].init();
      argv[i].value = &argvValue[i];
      argvValue[i].setStackStringValue(targ[i]);
   }
}

StringStackConsoleWrapper::~StringStackConsoleWrapper()
{
   for (int i=0; i<argc; i++)
   {
      argv[i] = 0;
   }
   delete[] argv;
   delete[] argvValue;
}

//------------------------------------------------------------------------------

S32 ConsoleValue::getSignedIntValue()
{
   if(type <= TypeInternalString)
      return (S32)fval;
   else
      return dAtoi(Con::getData(type, dataPtr, 0, enumTable));
}

U32 ConsoleValue::getIntValue()
{
   if(type <= TypeInternalString)
      return ival;
   else
      return dAtoi(Con::getData(type, dataPtr, 0, enumTable));
}

F32 ConsoleValue::getFloatValue()
{
   if(type <= TypeInternalString)
      return fval;
   else
      return dAtof(Con::getData(type, dataPtr, 0, enumTable));
}

const char *ConsoleValue::getStringValue()
{
   if(type == TypeInternalString || type == TypeInternalStackString)
      return sval;
   else if (type == TypeInternalStringStackPtr)
      return STR.mBuffer + (uintptr_t)sval;
   else
   {
      // We need a string representation, so lets create one
      const char *internalValue = NULL;

      if(type == TypeInternalFloat)
         internalValue = Con::getData(TypeF32, &fval, 0);
      else if(type == TypeInternalInt)
         internalValue = Con::getData(TypeS32, &ival, 0);
      else
         return Con::getData(type, dataPtr, 0, enumTable); // We can't save sval here since it is the same as dataPtr

      if (!internalValue)
         return "";

      U32 stringLen = dStrlen(internalValue);
      U32 newLen = ((stringLen + 1) + 15) & ~15; // pad upto next cache line
	   
      if (bufferLen == 0)
         sval = (char *) dMalloc(newLen);
      else if(newLen > bufferLen)
         sval = (char *) dRealloc(sval, newLen);

      dStrcpy(sval, internalValue);
      bufferLen = newLen;

      return sval;
   }
}

StringStackPtr ConsoleValue::getStringStackPtr()
{
   if (type == TypeInternalStringStackPtr)
      return (uintptr_t)sval;
   else
      return (uintptr_t)-1;
}

bool ConsoleValue::getBoolValue()
{
   if(type == TypeInternalString || type == TypeInternalStackString || type == TypeInternalStringStackPtr)
      return dAtob(getStringValue());
   if(type == TypeInternalFloat)
      return fval > 0;
   else if(type == TypeInternalInt)
      return ival > 0;
   else {
      const char *value = Con::getData(type, dataPtr, 0, enumTable);
      return dAtob(value);
   }
}

void ConsoleValue::setIntValue(S32 val)
{
   setFloatValue(val);
}

void ConsoleValue::setIntValue(U32 val)
{
   if(type <= TypeInternalString)
   {
      fval = (F32)val;
      ival = val;
      if(bufferLen > 0)
      {
         dFree(sval);
         bufferLen = 0;
      }

      sval = typeValueEmpty;
      type = TypeInternalInt;
   }
   else
   {
      const char *dptr = Con::getData(TypeS32, &val, 0);
      Con::setData(type, dataPtr, 0, 1, &dptr, enumTable);
   }
}

void ConsoleValue::setBoolValue(bool val)
{
   return setIntValue(val ? 1 : 0);
}

void ConsoleValue::setFloatValue(F32 val)
{
   if(type <= TypeInternalString)
   {
      fval = val;
      ival = static_cast<U32>(val);
      if(bufferLen > 0)
      {
         dFree(sval);
         bufferLen = 0;
      }
      sval = typeValueEmpty;
      type = TypeInternalFloat;
   }
   else
   {
      const char *dptr = Con::getData(TypeF32, &val, 0);
      Con::setData(type, dataPtr, 0, 1, &dptr, enumTable);
   }
}

//------------------------------------------------------------------------------

ConsoleValueRef _BaseEngineConsoleCallbackHelper::_exec()
{
   ConsoleValueRef returnValue;
   if( mThis )
   {
      // Cannot invoke callback until object has been registered
      if (mThis->isProperlyAdded()) {
         returnValue = Con::_internalExecute( mThis, mArgc, mArgv, false );
      } else {
         STR.clearFunctionOffset();
         returnValue = ConsoleValueRef();
      }
   }
   else
      returnValue = Con::_internalExecute( mArgc, mArgv );

   mArgc = mInitialArgc; // reset args
   return returnValue;
}

ConsoleValueRef _BaseEngineConsoleCallbackHelper::_execLater(SimConsoleThreadExecEvent *evt)
{
   mArgc = mInitialArgc; // reset args
   Sim::postEvent((SimObject*)Sim::getRootGroup(), evt, Sim::getCurrentTime());
   return evt->getCB().waitForResult();
}

//------------------------------------------------------------------------------

void ConsoleStackFrameSaver::save()
{
   CSTK.pushFrame();
   STR.pushFrame();
   mSaved = true;
}

void ConsoleStackFrameSaver::restore()
{
   if (mSaved)
   {
      CSTK.popFrame();
      STR.popFrame();
   }
}
