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
#include "console/console.h"

#include "console/ast.h"
#include "core/tAlgorithm.h"

#include "core/strings/findMatch.h"
#include "core/strings/stringUnit.h"
#include "console/consoleInternal.h"
#include "core/stream/fileStream.h"
#include "console/compiler.h"

#include "console/simBase.h"
#include "console/telnetDebugger.h"
#include "sim/netStringTable.h"
#include "console/ICallMethod.h"
#include "console/stringStack.h"
#include "util/messaging/message.h"
#include "core/frameAllocator.h"

#ifndef TORQUE_TGB_ONLY
#include "materials/materialDefinition.h"
#include "materials/materialManager.h"
#endif

// Uncomment to optimize function calls at the expense of potential invalid package lookups
//#define COMPILER_OPTIMIZE_FUNCTION_CALLS

using namespace Compiler;

enum EvalConstants {
   MaxStackSize = 1024,
   MethodOnComponent = -2
};

namespace Con
{
// Current script file name and root, these are registered as
// console variables.
extern StringTableEntry gCurrentFile;
extern StringTableEntry gCurrentRoot;
extern S32 gObjectCopyFailures;
}

/// Frame data for a foreach/foreach$ loop.
struct IterStackRecord
{
   /// If true, this is a foreach$ loop; if not, it's a foreach loop.
   bool mIsStringIter;
   
   /// The iterator variable.
   Dictionary::Entry* mVariable;
   
   /// Information for an object iterator loop.
   struct ObjectPos
   {
      /// The set being iterated over.
      SimSet* mSet;

      /// Current index in the set.
      U32 mIndex;
   };
   
   /// Information for a string iterator loop.
   struct StringPos
   {
      /// The raw string data on the string stack.
      StringStackPtr mString;
      
      /// Current parsing position.
      U32 mIndex;
   };
   
   union
   {
      ObjectPos mObj;
      StringPos mStr;
   } mData;
};

IterStackRecord iterStack[ MaxStackSize ];

F64 floatStack[MaxStackSize];
S64 intStack[MaxStackSize];




StringStack STR;
ConsoleValueStack CSTK;

U32 _FLT = 0;     ///< Stack pointer for floatStack.
U32 _UINT = 0;    ///< Stack pointer for intStack.
U32 _ITER = 0;    ///< Stack pointer for iterStack.

namespace Con
{
   const char *getNamespaceList(Namespace *ns)
   {
      U32 size = 1;
      Namespace * walk;
      for(walk = ns; walk; walk = walk->mParent)
         size += dStrlen(walk->mName) + 4;
      char *ret = Con::getReturnBuffer(size);
      ret[0] = 0;
      for(walk = ns; walk; walk = walk->mParent)
      {
         dStrcat(ret, walk->mName);
         if(walk->mParent)
            dStrcat(ret, " -> ");
      }
      return ret;
   }
}

//------------------------------------------------------------

F64 consoleStringToNumber(const char *str, StringTableEntry file, U32 line)
{
   F64 val = dAtof(str);
   if(val != 0)
      return val;
   else if(!dStricmp(str, "true"))
      return 1;
   else if(!dStricmp(str, "false"))
      return 0;
   else if(file)
   {
      Con::warnf(ConsoleLogEntry::General, "%s (%d): string always evaluates to 0.", file, line);
      return 0;
   }
   return 0;
}

//------------------------------------------------------------

namespace Con
{

   char *getReturnBuffer(U32 bufferSize)

   {
      return STR.getReturnBuffer(bufferSize);
   }

   char *getReturnBuffer( const char *stringToCopy )
   {
      U32 len = dStrlen( stringToCopy ) + 1;
      char *ret = STR.getReturnBuffer( len);
      dMemcpy( ret, stringToCopy, len );
      return ret;
   }

   char* getReturnBuffer( const String& str )
   {
      const U32 size = str.size();
      char* ret = STR.getReturnBuffer( size );
      dMemcpy( ret, str.c_str(), size );
      return ret;
   }

   char* getReturnBuffer( const StringBuilder& str )
   {
      char* buffer = Con::getReturnBuffer( str.length() + 1 );
      str.copy( buffer );
      buffer[ str.length() ] = '\0';
      
      return buffer;
   }

   char *getArgBuffer(U32 bufferSize)
   {
      return STR.getArgBuffer(bufferSize);
   }

   char *getFloatArg(F64 arg)
   {
      char *ret = STR.getArgBuffer(32);
      dSprintf(ret, 32, "%g", arg);
      return ret;
   }

   char *getIntArg(S32 arg)
   {
      char *ret = STR.getArgBuffer(32);
      dSprintf(ret, 32, "%d", arg);
      return ret;
   }

   char* getBoolArg(bool arg)
   {
      char *ret = STR.getArgBuffer(32);
      dSprintf(ret, 32, "%d", arg);
      return ret;
   }

   char *getStringArg( const char *arg )
   {
      U32 len = dStrlen( arg ) + 1;
      char *ret = STR.getArgBuffer( len );
      dMemcpy( ret, arg, len );
      return ret;
   }

   char* getStringArg( const String& arg )
   {
      const U32 size = arg.size();
      char* ret = STR.getArgBuffer( size );
      dMemcpy( ret, arg.c_str(), size );
      return ret;
   }
}

//------------------------------------------------------------

inline void ExprEvalState::setCurVarName(StringTableEntry name)
{
   if(name[0] == '$')
      currentVariable = globalVars.lookup(name);
   else if( getStackDepth() > 0 )
      currentVariable = getCurrentFrame().lookup(name);
   if(!currentVariable && gWarnUndefinedScriptVariables)
	   Con::warnf(ConsoleLogEntry::Script, "Variable referenced before assignment: %s", name);
}

inline void ExprEvalState::setCurVarNameCreate(StringTableEntry name)
{
   if(name[0] == '$')
      currentVariable = globalVars.add(name);
   else if( getStackDepth() > 0 )
      currentVariable = getCurrentFrame().add(name);
   else
   {
      currentVariable = NULL;
      Con::warnf(ConsoleLogEntry::Script, "Accessing local variable in global scope... failed: %s", name);
   }
}

//------------------------------------------------------------

inline S32 ExprEvalState::getIntVariable()
{
   return currentVariable ? currentVariable->getIntValue() : 0;
}

inline F64 ExprEvalState::getFloatVariable()
{
   return currentVariable ? currentVariable->getFloatValue() : 0;
}

inline const char *ExprEvalState::getStringVariable()
{
   return currentVariable ? currentVariable->getStringValue() : "";
}

//------------------------------------------------------------

inline void ExprEvalState::setIntVariable(S32 val)
{
   AssertFatal(currentVariable != NULL, "Invalid evaluator state - trying to set null variable!");
   currentVariable->setIntValue(val);
}

inline void ExprEvalState::setFloatVariable(F64 val)
{
   AssertFatal(currentVariable != NULL, "Invalid evaluator state - trying to set null variable!");
   currentVariable->setFloatValue(val);
}

inline void ExprEvalState::setStringVariable(const char *val)
{
   AssertFatal(currentVariable != NULL, "Invalid evaluator state - trying to set null variable!");
   currentVariable->setStringValue(val);
}

inline void ExprEvalState::setStringStackPtrVariable(StringStackPtr str)
{
   AssertFatal(currentVariable != NULL, "Invalid evaluator state - trying to set null variable!");
   currentVariable->setStringStackPtrValue(str);
}

inline void ExprEvalState::setCopyVariable()
{
   if (copyVariable)
   {
      switch (copyVariable->value.type)
      {
         case ConsoleValue::TypeInternalInt:
            currentVariable->setIntValue(copyVariable->getIntValue());
         break;
         case ConsoleValue::TypeInternalFloat:
            currentVariable->setFloatValue(copyVariable->getFloatValue());
         break;
         default:
            currentVariable->setStringValue(copyVariable->getStringValue());
         break;
	   }
   }
}

//------------------------------------------------------------

// Gets a component of an object's field value or a variable and returns it
// in val.
static void getFieldComponent( SimObject* object, StringTableEntry field, const char* array, StringTableEntry subField, char val[] )
{
   const char* prevVal = NULL;
   
   // Grab value from object.
   if( object && field )
      prevVal = object->getDataField( field, array );
   
   // Otherwise, grab from the string stack. The value coming in will always
   // be a string because that is how multicomponent variables are handled.
   else
      prevVal = STR.getStringValue();

   // Make sure we got a value.
   if ( prevVal && *prevVal )
   {
      static const StringTableEntry xyzw[] = 
      {
         StringTable->insert( "x" ),
         StringTable->insert( "y" ),
         StringTable->insert( "z" ),
         StringTable->insert( "w" )
      };

      static const StringTableEntry rgba[] = 
      {
         StringTable->insert( "r" ),
         StringTable->insert( "g" ),
         StringTable->insert( "b" ),
         StringTable->insert( "a" )
      };

      // Translate xyzw and rgba into the indexed component 
      // of the variable or field.
      if ( subField == xyzw[0] || subField == rgba[0] )
         dStrcpy( val, StringUnit::getUnit( prevVal, 0, " \t\n") );

      else if ( subField == xyzw[1] || subField == rgba[1] )
         dStrcpy( val, StringUnit::getUnit( prevVal, 1, " \t\n") );

      else if ( subField == xyzw[2] || subField == rgba[2] )
         dStrcpy( val, StringUnit::getUnit( prevVal, 2, " \t\n") );

      else if ( subField == xyzw[3] || subField == rgba[3] )
         dStrcpy( val, StringUnit::getUnit( prevVal, 3, " \t\n") );

      else
         val[0] = 0;
   }
   else
      val[0] = 0;
}

// Sets a component of an object's field value based on the sub field. 'x' will
// set the first field, 'y' the second, and 'z' the third.
static void setFieldComponent( SimObject* object, StringTableEntry field, const char* array, StringTableEntry subField )
{
   // Copy the current string value
   char strValue[1024];
   dStrncpy( strValue, STR.getStringValue(), 1024 );

   char val[1024] = "";
   const char* prevVal = NULL;

   // Set the value on an object field.
   if( object && field )
      prevVal = object->getDataField( field, array );

   // Set the value on a variable.
   else if( gEvalState.currentVariable )
      prevVal = gEvalState.getStringVariable();

   // Ensure that the variable has a value
   if (!prevVal)
	   return;

   static const StringTableEntry xyzw[] = 
   {
      StringTable->insert( "x" ),
      StringTable->insert( "y" ),
      StringTable->insert( "z" ),
      StringTable->insert( "w" )
   };

   static const StringTableEntry rgba[] = 
   {
      StringTable->insert( "r" ),
      StringTable->insert( "g" ),
      StringTable->insert( "b" ),
      StringTable->insert( "a" )
   };

   // Insert the value into the specified 
   // component of the string.
   if ( subField == xyzw[0] || subField == rgba[0] )
	  dStrcpy( val, StringUnit::setUnit( prevVal, 0, strValue, " \t\n") );

   else if ( subField == xyzw[1] || subField == rgba[1] )
      dStrcpy( val, StringUnit::setUnit( prevVal, 1, strValue, " \t\n") );

   else if ( subField == xyzw[2] || subField == rgba[2] )
      dStrcpy( val, StringUnit::setUnit( prevVal, 2, strValue, " \t\n") );

   else if ( subField == xyzw[3] || subField == rgba[3] )
      dStrcpy( val, StringUnit::setUnit( prevVal, 3, strValue, " \t\n") );

   if ( val[0] != 0 )
   {
      // Update the field or variable.
      if( object && field )
         object->setDataField( field, 0, val );
      else if( gEvalState.currentVariable )
         gEvalState.setStringVariable( val );
   }
}

ConsoleValueRef CodeBlock::exec(U32 ip, const char *functionName, Namespace *thisNamespace, U32 argc, ConsoleValueRef *argv, bool noCalls, StringTableEntry packageName, S32 setFrame)
{

#ifdef TORQUE_VALIDATE_STACK
   U32 stackStart = STR.mStartStackSize;
   U32 consoleStackStart = CSTK.mStackPos;
#endif

   //Con::printf("CodeBlock::exec(%s,%u)", functionName ? functionName : "??", ip);

   static char traceBuffer[1024];
   S32 i;
   
   U32 iterDepth = 0;

   incRefCount();
   F64 *curFloatTable;
   char *curStringTable;
   S32 curStringTableLen = 0; //clint to ensure we dont overwrite it
   STR.clearFunctionOffset(); // ensures arg buffer offset is back to 0
   StringTableEntry thisFunctionName = NULL;
   bool popFrame = false;
   if(argv)
   {
      // assume this points into a function decl:
      U32 fnArgc = code[ip + 2 + 6];
      thisFunctionName = CodeToSTE(code, ip);
      S32 wantedArgc = getMin(argc-1, fnArgc); // argv[0] is func name
      if(gEvalState.traceOn)
      {
         traceBuffer[0] = 0;
         dStrcat(traceBuffer, "Entering ");
         if(packageName)
         {
            dStrcat(traceBuffer, "[");
            dStrcat(traceBuffer, packageName);
            dStrcat(traceBuffer, "]");
         }
         if(thisNamespace && thisNamespace->mName)
         {
            dSprintf(traceBuffer + dStrlen(traceBuffer), sizeof(traceBuffer) - dStrlen(traceBuffer),
               "%s::%s(", thisNamespace->mName, thisFunctionName);
         }
         else
         {
            dSprintf(traceBuffer + dStrlen(traceBuffer), sizeof(traceBuffer) - dStrlen(traceBuffer),
               "%s(", thisFunctionName);
         }
         for(i = 0; i < wantedArgc; i++)
         {
            dStrcat(traceBuffer, argv[i+1]);
            if(i != wantedArgc - 1)
               dStrcat(traceBuffer, ", ");
         }
         dStrcat(traceBuffer, ")");
         Con::printf("%s", traceBuffer);
      }
      gEvalState.pushFrame(thisFunctionName, thisNamespace);
      popFrame = true;

      for(i = 0; i < wantedArgc; i++)
      {
         StringTableEntry var = CodeToSTE(code, ip + (2 + 6 + 1) + (i * 2));
         gEvalState.setCurVarNameCreate(var);

         ConsoleValueRef ref = argv[i+1];

         switch(argv[i+1].getType())
         {
         case ConsoleValue::TypeInternalInt:
            gEvalState.setIntVariable(argv[i+1]);
            break;
         case ConsoleValue::TypeInternalFloat:
            gEvalState.setFloatVariable(argv[i+1]);
            break;
         case ConsoleValue::TypeInternalStringStackPtr:
            gEvalState.setStringStackPtrVariable(argv[i+1].getStringStackPtrValue());
            break;
         case ConsoleValue::TypeInternalStackString:
         case ConsoleValue::TypeInternalString:
         default:
            gEvalState.setStringVariable(argv[i+1]);
            break;
         }
      }

      ip = ip + (fnArgc * 2) + (2 + 6 + 1);
      curFloatTable = functionFloats;
      curStringTable = functionStrings;
      curStringTableLen = functionStringsMaxLen;
   }
   else
   {
      curFloatTable = globalFloats;
      curStringTable = globalStrings;
      curStringTableLen = globalStringsMaxLen;

      // If requested stack frame isn't available, request a new one
      // (this prevents assert failures when creating local
      //  variables without a stack frame)
      if (gEvalState.getStackDepth() <= setFrame)
         setFrame = -1;

      // Do we want this code to execute using a new stack frame?
      if (setFrame < 0)
      {
         gEvalState.pushFrame(NULL, NULL);
         popFrame = true;
      }
      else
      {
         // We want to copy a reference to an existing stack frame
         // on to the top of the stack.  Any change that occurs to 
         // the locals during this new frame will also occur in the 
         // original frame.
         S32 stackIndex = gEvalState.getStackDepth() - setFrame - 1;
         gEvalState.pushFrameRef( stackIndex );
         popFrame = true;
      }
   }

   // Grab the state of the telenet debugger here once
   // so that the push and pop frames are always balanced.
   const bool telDebuggerOn = TelDebugger && TelDebugger->isConnected();
   if ( telDebuggerOn && setFrame < 0 )
      TelDebugger->pushStackFrame();

   StringTableEntry var, objParent;
   StringTableEntry fnName;
   StringTableEntry fnNamespace, fnPackage;

   // Add local object creation stack [7/9/2007 Black]
   static const U32 objectCreationStackSize = 32;
   U32 objectCreationStackIndex = 0;
   struct {
      SimObject *newObject;
      U32 failJump;
   } objectCreationStack[ objectCreationStackSize ];
   
   SimObject *currentNewObject = 0;
   U32 failJump = 0;
   StringTableEntry prevField = NULL;
   StringTableEntry curField = NULL;
   SimObject *prevObject = NULL;
   SimObject *curObject = NULL;
   SimObject *saveObject=NULL;
   Namespace::Entry *nsEntry;
   Namespace *ns;
   const char* curFNDocBlock = NULL;
   const char* curNSDocBlock = NULL;
   const S32 nsDocLength = 128;
   char nsDocBlockClass[nsDocLength];

   U32 callArgc;
   ConsoleValueRef *callArgv;

   static char curFieldArray[256];
   static char prevFieldArray[256];

   CodeBlock *saveCodeBlock = smCurrentCodeBlock;
   smCurrentCodeBlock = this;
   if(this->name)
   {
      Con::gCurrentFile = this->name;
      Con::gCurrentRoot = this->modPath;
   }
   const char * val;
   StringStackPtr retValue;

   // note: anything returned is pushed to CSTK and will be invalidated on the next exec()
   ConsoleValueRef returnValue;

   // The frame temp is used by the variable accessor ops (OP_SAVEFIELD_* and
   // OP_LOADFIELD_*) to store temporary values for the fields.
   static S32 VAL_BUFFER_SIZE = 1024;
   FrameTemp<char> valBuffer( VAL_BUFFER_SIZE );

   for(;;)
   {
      U32 instruction = code[ip++];
      nsEntry = NULL;
breakContinue:
      switch(instruction)
      {
         case OP_FUNC_DECL:
            if(!noCalls)
            {
               fnName       = CodeToSTE(code, ip);
               fnNamespace  = CodeToSTE(code, ip+2);
               fnPackage    = CodeToSTE(code, ip+4);
               bool hasBody = ( code[ ip + 6 ] & 0x01 ) != 0;
               U32 lineNumber = code[ ip + 6 ] >> 1;
               
               Namespace::unlinkPackages();
               ns = Namespace::find(fnNamespace, fnPackage);
               ns->addFunction(fnName, this, hasBody ? ip : 0, curFNDocBlock ? dStrdup( curFNDocBlock ) : NULL, lineNumber );// if no body, set the IP to 0
               if( curNSDocBlock )
               {
                  if( fnNamespace == StringTable->lookup( nsDocBlockClass ) )
                  {
                     char *usageStr = dStrdup( curNSDocBlock );
                     usageStr[dStrlen(usageStr)] = '\0';
                     ns->mUsage = usageStr;
                     ns->mCleanUpUsage = true;
                     curNSDocBlock = NULL;
                  }
               }
               Namespace::relinkPackages();

               // If we had a docblock, it's definitely not valid anymore, so clear it out.
               curFNDocBlock = NULL;

               //Con::printf("Adding function %s::%s (%d)", fnNamespace, fnName, ip);
            }
            ip = code[ip + 7];
            break;

         case OP_CREATE_OBJECT:
         {
            // Read some useful info.
            objParent        = CodeToSTE(code, ip);
            bool isDataBlock =          code[ip + 2];
            bool isInternal  =          code[ip + 3];
            bool isSingleton =          code[ip + 4];
            U32  lineNumber  =          code[ip + 5];
            failJump         =          code[ip + 6];
                        
            // If we don't allow calls, we certainly don't allow creating objects!
            // Moved this to after failJump is set. Engine was crashing when
            // noCalls = true and an object was being created at the beginning of
            // a file. ADL.
            if(noCalls)
            {
               ip = failJump;
               break;
            }

            // Push the old info to the stack
            //Assert( objectCreationStackIndex < objectCreationStackSize );
            objectCreationStack[ objectCreationStackIndex ].newObject = currentNewObject;
            objectCreationStack[ objectCreationStackIndex++ ].failJump = failJump;

            // Get the constructor information off the stack.
            CSTK.getArgcArgv(NULL, &callArgc, &callArgv);
            const char *objectName = callArgv[ 2 ];

            // Con::printf("Creating object...");

            // objectName = argv[1]...
            currentNewObject = NULL;

            // Are we creating a datablock? If so, deal with case where we override
            // an old one.
            if(isDataBlock)
            {
               // Con::printf("  - is a datablock");

               // Find the old one if any.
               SimObject *db = Sim::getDataBlockGroup()->findObject( objectName );
               
               // Make sure we're not changing types on ourselves...
               if(db && dStricmp(db->getClassName(), callArgv[1]))
               {
                  Con::errorf(ConsoleLogEntry::General, "%s: Cannot re-declare data block %s with a different class.", getFileLine(ip), objectName);
                  ip = failJump;
                  STR.popFrame();
                  CSTK.popFrame();
                  break;
               }

               // If there was one, set the currentNewObject and move on.
               if(db)
                  currentNewObject = db;
            }
            else if (!isInternal)
            {
               // IF we aren't looking at a local/internal object, then check if 
               // this object already exists in the global space

               AbstractClassRep* rep = AbstractClassRep::findClassRep( objectName );
               if (rep != NULL) {
                  Con::errorf(ConsoleLogEntry::General, "%s: Cannot name object [%s] the same name as a script class.",
                     getFileLine(ip), objectName);
                  ip = failJump;
                  STR.popFrame();
                  break;
               }

               SimObject *obj = Sim::findObject( (const char*)objectName );
               if (obj /*&& !obj->isLocalName()*/)
               {
                  if ( isSingleton )
                  {
                     // Make sure we're not trying to change types
                     if ( dStricmp( obj->getClassName(), (const char*)callArgv[1] ) != 0 )
                     {
                        Con::errorf(ConsoleLogEntry::General, "%s: Cannot re-declare object [%s] with a different class [%s] - was [%s].",
                           getFileLine(ip), objectName, (const char*)callArgv[1], obj->getClassName());
                        ip = failJump;
                        STR.popFrame();
                        CSTK.popFrame();
                        break;
                     }

                     // We're creating a singleton, so use the found object
                     // instead of creating a new object.
                     currentNewObject = obj;
                  }
                  else
                  {
                     const char* redefineBehavior = Con::getVariable( "$Con::redefineBehavior" );
                     
                     if( dStricmp( redefineBehavior, "replaceExisting" ) == 0 )
                     {
                        // Save our constructor args as the argv vector is stored on the
                        // string stack and may get stomped if deleteObject triggers
                        // script execution.
                        
                        ConsoleValueRef savedArgv[ StringStack::MaxArgs ];
                        for (int i=0; i<callArgc; i++) {
                           savedArgv[i] = callArgv[i];
                        }
                        //dMemcpy( savedArgv, callArgv, sizeof( savedArgv[ 0 ] ) * callArgc );
                        
                        // Prevent stack value corruption
                        CSTK.pushFrame();
                        STR.pushFrame();
                        // --

                        obj->deleteObject();
                        obj = NULL;

                        // Prevent stack value corruption
                        CSTK.popFrame();
                        STR.popFrame();
                        // --

                        //dMemcpy( callArgv, savedArgv, sizeof( callArgv[ 0 ] ) * callArgc );
                        for (int i=0; i<callArgc; i++) {
                           callArgv[i] = savedArgv[i];
                        }
                     }
                     else if( dStricmp( redefineBehavior, "renameNew" ) == 0 )
                     {
                        for( U32 i = 1;; ++ i )
                        {
                           String newName = String::ToString( "%s%i", objectName, i );
                           if( !Sim::findObject( newName ) )
                           {
                              objectName = StringTable->insert( newName );
                              break;
                           }
                        }
                     }
                     else if( dStricmp( redefineBehavior, "unnameNew" ) == 0 )
                     {
                        objectName = StringTable->insert( "" );
                     }
                     else if( dStricmp( redefineBehavior, "postfixNew" ) == 0 )
                     {
                        const char* postfix = Con::getVariable( "$Con::redefineBehaviorPostfix" );
                        String newName = String::ToString( "%s%s", objectName, postfix );
                        
                        if( Sim::findObject( newName ) )
                        {
                           Con::errorf( ConsoleLogEntry::General, "%s: Cannot re-declare object with postfix [%s].",
                              getFileLine(ip), newName.c_str() );
                           ip = failJump;
                           STR.popFrame();
                           CSTK.popFrame();
                           break;
                        }
                        else
                           objectName = StringTable->insert( newName );
                     }
                     else
                     {
                        Con::errorf(ConsoleLogEntry::General, "%s: Cannot re-declare object [%s].",
                           getFileLine(ip), objectName);
                        ip = failJump;
                        STR.popFrame();
                        CSTK.popFrame();
                        break;
                     }
                  }
               }
            }

            STR.popFrame();
            CSTK.popFrame();
            
            if(!currentNewObject)
            {
               // Well, looks like we have to create a new object.
               ConsoleObject *object = ConsoleObject::create((const char*)callArgv[1]);

               // Deal with failure!
               if(!object)
               {
                  Con::errorf(ConsoleLogEntry::General, "%s: Unable to instantiate non-conobject class %s.", getFileLine(ip), (const char*)callArgv[1]);
                  ip = failJump;
                  break;
               }

               // Do special datablock init if appropros
               if(isDataBlock)
               {
                  SimDataBlock *dataBlock = dynamic_cast<SimDataBlock *>(object);
                  if(dataBlock)
                  {
                     dataBlock->assignId();
                  }
                  else
                  {
                     // They tried to make a non-datablock with a datablock keyword!
                     Con::errorf(ConsoleLogEntry::General, "%s: Unable to instantiate non-datablock class %s.", getFileLine(ip), (const char*)callArgv[1]);
                     // Clean up...
                     delete object;
                     ip = failJump;
                     break;
                  }
               }

               // Finally, set currentNewObject to point to the new one.
               currentNewObject = dynamic_cast<SimObject *>(object);

               // Deal with the case of a non-SimObject.
               if(!currentNewObject)
               {
                  Con::errorf(ConsoleLogEntry::General, "%s: Unable to instantiate non-SimObject class %s.", getFileLine(ip), (const char*)callArgv[1]);
                  delete object;
                  ip = failJump;
                  break;
               }

               // Set the declaration line
               currentNewObject->setDeclarationLine(lineNumber);

               // Set the file that this object was created in
               currentNewObject->setFilename(name);

               // Does it have a parent object? (ie, the copy constructor : syntax, not inheriance)
               if(*objParent)
               {
                  // Find it!
                  SimObject *parent;
                  if(Sim::findObject(objParent, parent))
                  {
                     // Con::printf(" - Parent object found: %s", parent->getClassName());

                     currentNewObject->setCopySource( parent );
                     currentNewObject->assignFieldsFrom( parent );
                  }
                  else
                  {
                     if ( Con::gObjectCopyFailures == -1 )
                        Con::errorf(ConsoleLogEntry::General, "%s: Unable to find parent object %s for %s.", getFileLine(ip), objParent, (const char*)callArgv[1]);
                     else
                        ++Con::gObjectCopyFailures;

                     // Fail to create the object.
                     delete object;
                     ip = failJump;
                     break;
                  }
               }

               // If a name was passed, assign it.
               if( objectName[ 0 ] )
               {
                  if( !isInternal )
                     currentNewObject->assignName( objectName );
                  else
                     currentNewObject->setInternalName( objectName );

                  // Set the original name
                  currentNewObject->setOriginalName( objectName );
               }

               // Prevent stack value corruption
               CSTK.pushFrame();
               STR.pushFrame();
               // --

               // Do the constructor parameters.
               if(!currentNewObject->processArguments(callArgc-3, callArgv+3))
               {
                  delete currentNewObject;
                  currentNewObject = NULL;
                  ip = failJump;

                  // Prevent stack value corruption
                  CSTK.popFrame();
                  STR.popFrame();
                  // --
                  break;
               }

               // Prevent stack value corruption
               CSTK.popFrame();
               STR.popFrame();
               // --

               // If it's not a datablock, allow people to modify bits of it.
               if(!isDataBlock)
               {
                  currentNewObject->setModStaticFields(true);
                  currentNewObject->setModDynamicFields(true);
               }
            }

            // Advance the IP past the create info...
            ip += 7;
            break;
         }

         case OP_ADD_OBJECT:
         {
            // See OP_SETCURVAR for why we do this.
            curFNDocBlock = NULL;
            curNSDocBlock = NULL;
            
            // Do we place this object at the root?
            bool placeAtRoot = code[ip++];

            // Con::printf("Adding object %s", currentNewObject->getName());

            // Prevent stack value corruption
            CSTK.pushFrame();
            STR.pushFrame();
            // --

            // Make sure it wasn't already added, then add it.
            if(currentNewObject->isProperlyAdded() == false)
            {
               bool ret = false;

               Message *msg = dynamic_cast<Message *>(currentNewObject);
               if(msg)
               {
                  SimObjectId id = Message::getNextMessageID();
                  if(id != 0xffffffff)
                     ret = currentNewObject->registerObject(id);
                  else
                     Con::errorf("%s: No more object IDs available for messages", getFileLine(ip));
               }
               else
                  ret = currentNewObject->registerObject();

               if(! ret)
               {
                  // This error is usually caused by failing to call Parent::initPersistFields in the class' initPersistFields().
                  Con::warnf(ConsoleLogEntry::General, "%s: Register object failed for object %s of class %s.", getFileLine(ip), currentNewObject->getName(), currentNewObject->getClassName());
                  delete currentNewObject;
                  ip = failJump;
                  // Prevent stack value corruption
                  CSTK.popFrame();
                  STR.popFrame();
                  // --
                  break;
               }
            }

            // Are we dealing with a datablock?
            SimDataBlock *dataBlock = dynamic_cast<SimDataBlock *>(currentNewObject);
            static String errorStr;



            // If so, preload it.
            if(dataBlock && !dataBlock->preload(true, errorStr))
            {
               Con::errorf(ConsoleLogEntry::General, "%s: preload failed for %s: %s.", getFileLine(ip),
                           currentNewObject->getName(), errorStr.c_str());
               dataBlock->deleteObject();
               ip = failJump;
			   
               // Prevent stack value corruption
               CSTK.popFrame();
               STR.popFrame();
               // --
               break;
            }

            // What group will we be added to, if any?
            U32 groupAddId = intStack[_UINT];
            SimGroup *grp = NULL;
            SimSet   *set = NULL;
            bool isMessage = dynamic_cast<Message *>(currentNewObject) != NULL;

            if(!placeAtRoot || !currentNewObject->getGroup())
            {
               if(! isMessage)
               {
                  if(! placeAtRoot)
                  {
                     // Otherwise just add to the requested group or set.
                     if(!Sim::findObject(groupAddId, grp))
                        Sim::findObject(groupAddId, set);
                  }
                  
                  if(placeAtRoot)
                  {
                     // Deal with the instantGroup if we're being put at the root or we're adding to a component.
                     if( Con::gInstantGroup.isEmpty()
                        || !Sim::findObject( Con::gInstantGroup, grp ) )
                        grp = Sim::getRootGroup();
                  }
               }

               // If we didn't get a group, then make sure we have a pointer to
               // the rootgroup.
               if(!grp)
                  grp = Sim::getRootGroup();

               // add to the parent group
               grp->addObject(currentNewObject);
               
               // If for some reason the add failed, add the object to the
               // root group so it won't leak.
               if( !currentNewObject->getGroup() )
                  Sim::getRootGroup()->addObject( currentNewObject );

               // add to any set we might be in
               if(set)
                  set->addObject(currentNewObject);
            }

            // store the new object's ID on the stack (overwriting the group/set
            // id, if one was given, otherwise getting pushed)
            if(placeAtRoot) 
               intStack[_UINT] = currentNewObject->getId();
            else
               intStack[++_UINT] = currentNewObject->getId();

            // Prevent stack value corruption
            CSTK.popFrame();
            STR.popFrame();
            // --
            break;
         }

         case OP_END_OBJECT:
         {
            // If we're not to be placed at the root, make sure we clean up
            // our group reference.
            bool placeAtRoot = code[ip++];
            if(!placeAtRoot)
               _UINT--;
            break;
         }

         case OP_FINISH_OBJECT:
         {
            //Assert( objectCreationStackIndex >= 0 );
            // Restore the object info from the stack [7/9/2007 Black]
            currentNewObject = objectCreationStack[ --objectCreationStackIndex ].newObject;
            failJump = objectCreationStack[ objectCreationStackIndex ].failJump;
            break;
         }

         case OP_JMPIFFNOT:
            if(floatStack[_FLT--])
            {
               ip++;
               break;
            }
            ip = code[ip];
            break;
         case OP_JMPIFNOT:
            if(intStack[_UINT--])
            {
               ip++;
               break;
            }
            ip = code[ip];
            break;
         case OP_JMPIFF:
            if(!floatStack[_FLT--])
            {
               ip++;
               break;
            }
            ip = code[ip];
            break;
         case OP_JMPIF:
            if(!intStack[_UINT--])
            {
               ip ++;
               break;
            }
            ip = code[ip];
            break;
         case OP_JMPIFNOT_NP:
            if(intStack[_UINT])
            {
               _UINT--;
               ip++;
               break;
            }
            ip = code[ip];
            break;
         case OP_JMPIF_NP:
            if(!intStack[_UINT])
            {
               _UINT--;
               ip++;
               break;
            }
            ip = code[ip];
            break;
         case OP_JMP:
            ip = code[ip];
            break;
            
         // This fixes a bug when not explicitly returning a value.
         case OP_RETURN_VOID:
      		STR.setStringValue("");
      		// We're falling thru here on purpose.
            
         case OP_RETURN:
            retValue = STR.getStringValuePtr();

            if( iterDepth > 0 )
            {
               // Clear iterator state.
               while( iterDepth > 0 )
               {
                  iterStack[ -- _ITER ].mIsStringIter = false;
                  -- iterDepth;
               }

               STR.rewind();
               STR.setStringValue( StringStackPtrRef(retValue).getPtr(&STR) ); // Not nice but works.
               retValue = STR.getStringValuePtr();
            }

            // Previously the return value was on the stack and would be returned using STR.getStringValue().
            // Now though we need to wrap it in a ConsoleValueRef 
            returnValue.value = CSTK.pushStringStackPtr(retValue);
               
            goto execFinished;

         case OP_RETURN_FLT:
         
            if( iterDepth > 0 )
            {
               // Clear iterator state.
               while( iterDepth > 0 )
               {
                  iterStack[ -- _ITER ].mIsStringIter = false;
                  -- iterDepth;
               }
               
            }

            returnValue.value = CSTK.pushFLT(floatStack[_FLT]);
            _FLT--;
               
            goto execFinished;

         case OP_RETURN_UINT:
         
            if( iterDepth > 0 )
            {
               // Clear iterator state.
               while( iterDepth > 0 )
               {
                  iterStack[ -- _ITER ].mIsStringIter = false;
                  -- iterDepth;
               }
            }

            returnValue.value = CSTK.pushUINT(intStack[_UINT]);
            _UINT--;
               
            goto execFinished;
            
         case OP_CMPEQ:
            intStack[_UINT+1] = bool(floatStack[_FLT] == floatStack[_FLT-1]);
            _UINT++;
            _FLT -= 2;
            break;

         case OP_CMPGR:
            intStack[_UINT+1] = bool(floatStack[_FLT] > floatStack[_FLT-1]);
            _UINT++;
            _FLT -= 2;
            break;

         case OP_CMPGE:
            intStack[_UINT+1] = bool(floatStack[_FLT] >= floatStack[_FLT-1]);
            _UINT++;
            _FLT -= 2;
            break;

         case OP_CMPLT:
            intStack[_UINT+1] = bool(floatStack[_FLT] < floatStack[_FLT-1]);
            _UINT++;
            _FLT -= 2;
            break;

         case OP_CMPLE:
            intStack[_UINT+1] = bool(floatStack[_FLT] <= floatStack[_FLT-1]);
            _UINT++;
            _FLT -= 2;
            break;

         case OP_CMPNE:
            intStack[_UINT+1] = bool(floatStack[_FLT] != floatStack[_FLT-1]);
            _UINT++;
            _FLT -= 2;
            break;

         case OP_XOR:
            intStack[_UINT-1] = intStack[_UINT] ^ intStack[_UINT-1];
            _UINT--;
            break;

         case OP_MOD:
            if(  intStack[_UINT-1] != 0 )
               intStack[_UINT-1] = intStack[_UINT] % intStack[_UINT-1];
            else
               intStack[_UINT-1] = 0;
            _UINT--;
            break;

         case OP_BITAND:
            intStack[_UINT-1] = intStack[_UINT] & intStack[_UINT-1];
            _UINT--;
            break;

         case OP_BITOR:
            intStack[_UINT-1] = intStack[_UINT] | intStack[_UINT-1];
            _UINT--;
            break;

         case OP_NOT:
            intStack[_UINT] = !intStack[_UINT];
            break;

         case OP_NOTF:
            intStack[_UINT+1] = !floatStack[_FLT];
            _FLT--;
            _UINT++;
            break;

         case OP_ONESCOMPLEMENT:
            intStack[_UINT] = ~intStack[_UINT];
            break;

         case OP_SHR:
            intStack[_UINT-1] = intStack[_UINT] >> intStack[_UINT-1];
            _UINT--;
            break;

         case OP_SHL:
            intStack[_UINT-1] = intStack[_UINT] << intStack[_UINT-1];
            _UINT--;
            break;

         case OP_AND:
            intStack[_UINT-1] = intStack[_UINT] && intStack[_UINT-1];
            _UINT--;
            break;

         case OP_OR:
            intStack[_UINT-1] = intStack[_UINT] || intStack[_UINT-1];
            _UINT--;
            break;

         case OP_ADD:
            floatStack[_FLT-1] = floatStack[_FLT] + floatStack[_FLT-1];
            _FLT--;
            break;

         case OP_SUB:
            floatStack[_FLT-1] = floatStack[_FLT] - floatStack[_FLT-1];
            _FLT--;
            break;

         case OP_MUL:
            floatStack[_FLT-1] = floatStack[_FLT] * floatStack[_FLT-1];
            _FLT--;
            break;
         case OP_DIV:
            floatStack[_FLT-1] = floatStack[_FLT] / floatStack[_FLT-1];
            _FLT--;
            break;
         case OP_NEG:
            floatStack[_FLT] = -floatStack[_FLT];
            break;

         case OP_SETCURVAR:
            var = CodeToSTE(code, ip);
            ip += 2;

            // If a variable is set, then these must be NULL. It is necessary
            // to set this here so that the vector parser can appropriately
            // identify whether it's dealing with a vector.
            prevField = NULL;
            prevObject = NULL;
            curObject = NULL;

            gEvalState.setCurVarName(var);

            // In order to let docblocks work properly with variables, we have
            // clear the current docblock when we do an assign. This way it 
            // won't inappropriately carry forward to following function decls.
            curFNDocBlock = NULL;
            curNSDocBlock = NULL;
            break;

         case OP_SETCURVAR_CREATE:
            var = CodeToSTE(code, ip);
            ip += 2;

            // See OP_SETCURVAR
            prevField = NULL;
            prevObject = NULL;
            curObject = NULL;

            gEvalState.setCurVarNameCreate(var);

            // See OP_SETCURVAR for why we do this.
            curFNDocBlock = NULL;
            curNSDocBlock = NULL;
            break;

         case OP_SETCURVAR_ARRAY:
            var = STR.getSTValue();

            // See OP_SETCURVAR
            prevField = NULL;
            prevObject = NULL;
            curObject = NULL;

            gEvalState.setCurVarName(var);

            // See OP_SETCURVAR for why we do this.
            curFNDocBlock = NULL;
            curNSDocBlock = NULL;
            break;

         case OP_SETCURVAR_ARRAY_CREATE:
            var = STR.getSTValue();

            // See OP_SETCURVAR
            prevField = NULL;
            prevObject = NULL;
            curObject = NULL;

            gEvalState.setCurVarNameCreate(var);

            // See OP_SETCURVAR for why we do this.
            curFNDocBlock = NULL;
            curNSDocBlock = NULL;
            break;

         case OP_LOADVAR_UINT:
            intStack[_UINT+1] = gEvalState.getIntVariable();
            _UINT++;
            break;

         case OP_LOADVAR_FLT:
            floatStack[_FLT+1] = gEvalState.getFloatVariable();
            _FLT++;
            break;

         case OP_LOADVAR_STR:
            val = gEvalState.getStringVariable();
            STR.setStringValue(val);
            break;

         case OP_LOADVAR_VAR:
            // Sets current source of OP_SAVEVAR_VAR
            gEvalState.copyVariable = gEvalState.currentVariable;
            break;

         case OP_SAVEVAR_UINT:
            gEvalState.setIntVariable(intStack[_UINT]);
            break;

         case OP_SAVEVAR_FLT:
            gEvalState.setFloatVariable(floatStack[_FLT]);
            break;

         case OP_SAVEVAR_STR:
            gEvalState.setStringVariable(STR.getStringValue());
            break;
		    
         case OP_SAVEVAR_VAR:
            // this basically handles %var1 = %var2
            gEvalState.setCopyVariable();
            break;

         case OP_SETCUROBJECT:
            // Save the previous object for parsing vector fields.
            prevObject = curObject;
            val = STR.getStringValue();

            // Sim::findObject will sometimes find valid objects from
            // multi-component strings. This makes sure that doesn't
            // happen.
            for( const char* check = val; *check; check++ )
            {
               if( *check == ' ' )
               {
                  val = "";
                  break;
               }
            }
            curObject = Sim::findObject(val);
            break;

         case OP_SETCUROBJECT_INTERNAL:
            ++ip; // To skip the recurse flag if the object wasn't found
            if(curObject)
            {
               SimSet *set = dynamic_cast<SimSet *>(curObject);
               if(set)
               {
                  StringTableEntry intName = StringTable->insert(STR.getStringValue());
                  bool recurse = code[ip-1];
                  SimObject *obj = set->findObjectByInternalName(intName, recurse);
                  intStack[_UINT+1] = obj ? obj->getId() : 0;
                  _UINT++;
               }
               else
               {
                  Con::errorf(ConsoleLogEntry::Script, "%s: Attempt to use -> on non-set %s of class %s.", getFileLine(ip-2), curObject->getName(), curObject->getClassName());
                  intStack[_UINT] = 0;
               }
            }
            break;

         case OP_SETCUROBJECT_NEW:
            curObject = currentNewObject;
            break;

         case OP_SETCURFIELD:
            // Save the previous field for parsing vector fields.
            prevField = curField;
            dStrcpy( prevFieldArray, curFieldArray );
            curField = CodeToSTE(code, ip);
            curFieldArray[0] = 0;
            ip += 2;
            break;

         case OP_SETCURFIELD_ARRAY:
            dStrcpy(curFieldArray, STR.getStringValue());
            break;

         case OP_SETCURFIELD_TYPE:
            if(curObject)
               curObject->setDataFieldType(code[ip], curField, curFieldArray);
            ip++;
            break;

         case OP_LOADFIELD_UINT:
            if(curObject)
               intStack[_UINT+1] = U32(dAtoi(curObject->getDataField(curField, curFieldArray)));
            else
            {
               // The field is not being retrieved from an object. Maybe it's
               // a special accessor?
               getFieldComponent( prevObject, prevField, prevFieldArray, curField, valBuffer );
               intStack[_UINT+1] = dAtoi( valBuffer );
            }
            _UINT++;
            break;

         case OP_LOADFIELD_FLT:
            if(curObject)
               floatStack[_FLT+1] = dAtof(curObject->getDataField(curField, curFieldArray));
            else
            {
               // The field is not being retrieved from an object. Maybe it's
               // a special accessor?
               getFieldComponent( prevObject, prevField, prevFieldArray, curField, valBuffer );
               floatStack[_FLT+1] = dAtof( valBuffer );
            }
            _FLT++;
            break;

         case OP_LOADFIELD_STR:
            if(curObject)
            {
               val = curObject->getDataField(curField, curFieldArray);
               STR.setStringValue( val );
            }
            else
            {
               // The field is not being retrieved from an object. Maybe it's
               // a special accessor?
               getFieldComponent( prevObject, prevField, prevFieldArray, curField, valBuffer );
               STR.setStringValue( valBuffer );
            }
            break;

         case OP_SAVEFIELD_UINT:
            STR.setIntValue(intStack[_UINT]);
            if(curObject)
               curObject->setDataField(curField, curFieldArray, STR.getStringValue());
            else
            {
               // The field is not being set on an object. Maybe it's
               // a special accessor?
               setFieldComponent( prevObject, prevField, prevFieldArray, curField );
               prevObject = NULL;
            }
            break;

         case OP_SAVEFIELD_FLT:
            STR.setFloatValue(floatStack[_FLT]);
            if(curObject)
               curObject->setDataField(curField, curFieldArray, STR.getStringValue());
            else
            {
               // The field is not being set on an object. Maybe it's
               // a special accessor?
               setFieldComponent( prevObject, prevField, prevFieldArray, curField );
               prevObject = NULL;
            }
            break;

         case OP_SAVEFIELD_STR:
            if(curObject)
               curObject->setDataField(curField, curFieldArray, STR.getStringValue());
            else
            {
               // The field is not being set on an object. Maybe it's
               // a special accessor?
               setFieldComponent( prevObject, prevField, prevFieldArray, curField );
               prevObject = NULL;
            }
            break;

         case OP_STR_TO_UINT:
            intStack[_UINT+1] = STR.getIntValue();
            _UINT++;
            break;

         case OP_STR_TO_FLT:
            floatStack[_FLT+1] = STR.getFloatValue();
            _FLT++;
            break;

         case OP_STR_TO_NONE:
            // This exists simply to deal with certain typecast situations.
            break;

         case OP_FLT_TO_UINT:
            intStack[_UINT+1] = (S64)floatStack[_FLT];
            _FLT--;
            _UINT++;
            break;

         case OP_FLT_TO_STR:
            STR.setFloatValue(floatStack[_FLT]);
            _FLT--;
            break;

         case OP_FLT_TO_NONE:
            _FLT--;
            break;

         case OP_UINT_TO_FLT:
            floatStack[_FLT+1] = (F32)intStack[_UINT];
            _UINT--;
            _FLT++;
            break;

         case OP_UINT_TO_STR:
            STR.setIntValue(intStack[_UINT]);
            _UINT--;
            break;

         case OP_UINT_TO_NONE:
            _UINT--;
            break;

         case OP_COPYVAR_TO_NONE:
            gEvalState.copyVariable = NULL;
            break;

         case OP_LOADIMMED_UINT:
            intStack[_UINT+1] = code[ip++];
            _UINT++;
            break;

         case OP_LOADIMMED_FLT:
            floatStack[_FLT+1] = curFloatTable[code[ip]];
            ip++;
            _FLT++;
            break;
            
         case OP_TAG_TO_STR:
            code[ip-1] = OP_LOADIMMED_STR;
            // it's possible the string has already been converted
            if(U8(curStringTable[code[ip]]) != StringTagPrefixByte)
            {
               U32 id = GameAddTaggedString(curStringTable + code[ip]);
               dSprintf(curStringTable + code[ip] + 1, 7, "%d", id);
               *(curStringTable + code[ip]) = StringTagPrefixByte;
            }
         case OP_LOADIMMED_STR:
            STR.setStringValue(curStringTable + code[ip++]);
            break;

         case OP_DOCBLOCK_STR:
            {
               // If the first word of the doc is '\class' or '@class', then this
               // is a namespace doc block, otherwise it is a function doc block.
               const char* docblock = curStringTable + code[ip++];

               const char* sansClass = dStrstr( docblock, "@class" );
               if( !sansClass )
                  sansClass = dStrstr( docblock, "\\class" );

               if( sansClass )
               {
                  // Don't save the class declaration. Scan past the 'class'
                  // keyword and up to the first whitespace.
                  sansClass += 7;
                  S32 index = 0;
                  while( ( *sansClass != ' ' ) && ( *sansClass != '\n' ) && *sansClass && ( index < ( nsDocLength - 1 ) ) )
                  {
                     nsDocBlockClass[index++] = *sansClass;
                     sansClass++;
                  }
                  nsDocBlockClass[index] = '\0';

                  curNSDocBlock = sansClass + 1;
               }
               else
                  curFNDocBlock = docblock;
            }

            break;

         case OP_LOADIMMED_IDENT:
            STR.setStringValue(CodeToSTE(code, ip));
            ip += 2;
            break;

         case OP_CALLFUNC_RESOLVE:
            // This deals with a function that is potentially living in a namespace.
            fnNamespace = CodeToSTE(code, ip+2);
            fnName      = CodeToSTE(code, ip);

            // Try to look it up.
            ns = Namespace::find(fnNamespace);
            nsEntry = ns->lookup(fnName);
            if(!nsEntry)
            {
               ip+= 5;
               Con::warnf(ConsoleLogEntry::General,
                  "%s: Unable to find function %s%s%s",
                  getFileLine(ip-7), fnNamespace ? fnNamespace : "",
                  fnNamespace ? "::" : "", fnName);
               STR.popFrame();
               CSTK.popFrame();
               break;
            }
            
#ifdef COMPILER_OPTIMIZE_FUNCTION_CALLS
            // Now fall through to OP_CALLFUNC...
            // Now, rewrite our code a bit (ie, avoid future lookups) and fall
            // through to OP_CALLFUNC
#ifdef TORQUE_CPU_X64
            *((U64*)(code+ip+2)) = ((U64)nsEntry);
#else
            code[ip+2] = ((U32)nsEntry);
#endif
            code[ip-1] = OP_CALLFUNC;
#endif

         case OP_CALLFUNC:
         {
            // This routingId is set when we query the object as to whether
            // it handles this method.  It is set to an enum from the table
            // above indicating whether it handles it on a component it owns
            // or just on the object.
            S32 routingId = 0;

            fnName = CodeToSTE(code, ip);

            //if this is called from inside a function, append the ip and codeptr
            if( gEvalState.getStackDepth() > 0 )
            {
               gEvalState.getCurrentFrame().code = this;
               gEvalState.getCurrentFrame().ip = ip - 1;
            }

            U32 callType = code[ip+4];

            ip += 5;
            CSTK.getArgcArgv(fnName, &callArgc, &callArgv);

            const char *componentReturnValue = "";

            if(callType == FuncCallExprNode::FunctionCall) 
            {
               if( !nsEntry )
               {
#ifdef COMPILER_OPTIMIZE_FUNCTION_CALLS
#ifdef TORQUE_CPU_X64
                  nsEntry = ((Namespace::Entry *) *((U64*)(code+ip-3)));
#else
                  nsEntry = ((Namespace::Entry *) *(code+ip-3));
#endif
#else
                  nsEntry = Namespace::global()->lookup( fnName );
#endif
                  ns = NULL;
               }
               ns = NULL;
            }
            else if(callType == FuncCallExprNode::MethodCall)
            {
               saveObject = gEvalState.thisObject;
               gEvalState.thisObject = Sim::findObject((const char*)callArgv[1]);
               if(!gEvalState.thisObject)
               {
                  // Go back to the previous saved object.
                  gEvalState.thisObject = saveObject;

                  Con::warnf(ConsoleLogEntry::General,"%s: Unable to find object: '%s' attempting to call function '%s'", getFileLine(ip-4), (const char*)callArgv[1], fnName);
                  STR.popFrame();
                  CSTK.popFrame();
                  STR.setStringValue("");
                  break;
               }
               
               bool handlesMethod = gEvalState.thisObject->handlesConsoleMethod(fnName,&routingId);
               if( handlesMethod && routingId == MethodOnComponent )
               {
                  ICallMethod *pComponent = dynamic_cast<ICallMethod *>( gEvalState.thisObject );
                  if( pComponent )
                     componentReturnValue = pComponent->callMethodArgList( callArgc, callArgv, false );
               }
               
               ns = gEvalState.thisObject->getNamespace();
               if(ns)
                  nsEntry = ns->lookup(fnName);
               else
                  nsEntry = NULL;
            }
            else // it's a ParentCall
            {
               if(thisNamespace)
               {
                  ns = thisNamespace->mParent;
                  if(ns)
                     nsEntry = ns->lookup(fnName);
                  else
                     nsEntry = NULL;
               }
               else
               {
                  ns = NULL;
                  nsEntry = NULL;
               }
            }

            Namespace::Entry::CallbackUnion * nsCb = NULL;
            const char * nsUsage = NULL;
            if (nsEntry)
            {
               nsCb = &nsEntry->cb;
               nsUsage = nsEntry->mUsage;
               routingId = 0;
            }
            if(!nsEntry || noCalls)
            {
               if(!noCalls && !( routingId == MethodOnComponent ) )
               {
                  Con::warnf(ConsoleLogEntry::General,"%s: Unknown command %s.", getFileLine(ip-6), fnName);
                  if(callType == FuncCallExprNode::MethodCall)
                  {
                     Con::warnf(ConsoleLogEntry::General, "  Object %s(%d) %s",
                           gEvalState.thisObject->getName() ? gEvalState.thisObject->getName() : "",
                           gEvalState.thisObject->getId(), Con::getNamespaceList(ns) );
                  }
               }
               STR.popFrame();
               CSTK.popFrame();

               if( routingId == MethodOnComponent )
                  STR.setStringValue( componentReturnValue );
               else
                  STR.setStringValue( "" );

               break;
            }
            if(nsEntry->mType == Namespace::Entry::ConsoleFunctionType)
            {
               ConsoleValueRef ret;
               if(nsEntry->mFunctionOffset)
                  ret = nsEntry->mCode->exec(nsEntry->mFunctionOffset, fnName, nsEntry->mNamespace, callArgc, callArgv, false, nsEntry->mPackage);

               STR.popFrame();
               // Functions are assumed to return strings, so look ahead to see if we can skip the conversion
               if(code[ip] == OP_STR_TO_UINT)
               {
                  ip++;
                  intStack[++_UINT] = (U32)((S32)ret);
               }
               else if(code[ip] == OP_STR_TO_FLT)
               {
                  ip++;
                  floatStack[++_FLT] = (F32)ret;
               }
               else if(code[ip] == OP_STR_TO_NONE)
               {
                  STR.setStringValue(ret.getStringValue());
                  ip++;
               }
               else
                  STR.setStringValue((const char*)ret);

               // This will clear everything including returnValue
               CSTK.popFrame();
               //STR.clearFunctionOffset();
            }
            else
            {
               const char* nsName = ns? ns->mName: "";
#ifndef TORQUE_DEBUG
               // [tom, 12/13/2006] This stops tools functions from working in the console,
               // which is useful behavior when debugging so I'm ifdefing this out for debug builds.
               if(nsEntry->mToolOnly && ! Con::isCurrentScriptToolScript())
               {
                  Con::errorf(ConsoleLogEntry::Script, "%s: %s::%s - attempting to call tools only function from outside of tools.", getFileLine(ip-6), nsName, fnName);
               }
               else
#endif
               if((nsEntry->mMinArgs && S32(callArgc) < nsEntry->mMinArgs) || (nsEntry->mMaxArgs && S32(callArgc) > nsEntry->mMaxArgs))
               {
                  Con::warnf(ConsoleLogEntry::Script, "%s: %s::%s - wrong number of arguments (got %i, expected min %i and max %i).",
                     getFileLine(ip-6), nsName, fnName,
                     callArgc, nsEntry->mMinArgs, nsEntry->mMaxArgs);
                  Con::warnf(ConsoleLogEntry::Script, "%s: usage: %s", getFileLine(ip-6), nsEntry->mUsage);
                  STR.popFrame();
                  CSTK.popFrame();
               }
               else
               {
                  switch(nsEntry->mType)
                  {
                     case Namespace::Entry::StringCallbackType:
                     {
                        const char *ret = nsEntry->cb.mStringCallbackFunc(gEvalState.thisObject, callArgc, callArgv);
                        STR.popFrame();
                        CSTK.popFrame();
                        if(ret != STR.getStringValue())
                           STR.setStringValue(ret);
                        //else
                        //   STR.setLen(dStrlen(ret));
                        break;
                     }
                     case Namespace::Entry::IntCallbackType:
                     {
                        S32 result = nsEntry->cb.mIntCallbackFunc(gEvalState.thisObject, callArgc, callArgv);
                        STR.popFrame();
                        CSTK.popFrame();
                        if(code[ip] == OP_STR_TO_UINT)
                        {
                           ip++;
                           intStack[++_UINT] = result;
                           break;
                        }
                        else if(code[ip] == OP_STR_TO_FLT)
                        {
                           ip++;
                           floatStack[++_FLT] = result;
                           break;
                        }
                        else if(code[ip] == OP_STR_TO_NONE)
                           ip++;
                        else
                           STR.setIntValue(result);
                        break;
                     }
                     case Namespace::Entry::FloatCallbackType:
                     {
                        F64 result = nsEntry->cb.mFloatCallbackFunc(gEvalState.thisObject, callArgc, callArgv);
                        STR.popFrame();
                        CSTK.popFrame();
                        if(code[ip] == OP_STR_TO_UINT)
                        {
                           ip++;
                           intStack[++_UINT] = (S64)result;
                           break;
                        }
                        else if(code[ip] == OP_STR_TO_FLT)
                        {
                           ip++;
                           floatStack[++_FLT] = result;
                           break;
                        }
                        else if(code[ip] == OP_STR_TO_NONE)
                           ip++;
                        else
                           STR.setFloatValue(result);
                        break;
                     }
                     case Namespace::Entry::VoidCallbackType:
                        nsEntry->cb.mVoidCallbackFunc(gEvalState.thisObject, callArgc, callArgv);
                        if( code[ ip ] != OP_STR_TO_NONE && Con::getBoolVariable( "$Con::warnVoidAssignment", true ) )
                           Con::warnf(ConsoleLogEntry::General, "%s: Call to %s in %s uses result of void function call.", getFileLine(ip-6), fnName, functionName);
                        
                        STR.popFrame();
                        CSTK.popFrame();
                        STR.setStringValue("");
                        break;
                     case Namespace::Entry::BoolCallbackType:
                     {
                        bool result = nsEntry->cb.mBoolCallbackFunc(gEvalState.thisObject, callArgc, callArgv);
                        STR.popFrame();
                        CSTK.popFrame();
                        if(code[ip] == OP_STR_TO_UINT)
                        {
                           ip++;
                           intStack[++_UINT] = result;
                           break;
                        }
                        else if(code[ip] == OP_STR_TO_FLT)
                        {
                           ip++;
                           floatStack[++_FLT] = result;
                           break;
                        }
                        else if(code[ip] == OP_STR_TO_NONE)
                           ip++;
                        else
                           STR.setIntValue(result);
                        break;
                     }
                  }
               }
            }

            if(callType == FuncCallExprNode::MethodCall)
               gEvalState.thisObject = saveObject;
            break;
         }
         case OP_ADVANCE_STR:
            STR.advance();
            break;
         case OP_ADVANCE_STR_APPENDCHAR:
            STR.advanceChar(code[ip++]);
            break;

         case OP_ADVANCE_STR_COMMA:
            STR.advanceChar('_');
            break;

         case OP_ADVANCE_STR_NUL:
            STR.advanceChar(0);
            break;

         case OP_REWIND_STR:
            STR.rewind();
            break;

         case OP_TERMINATE_REWIND_STR:
            STR.rewindTerminate();
            break;

         case OP_COMPARE_STR:
            intStack[++_UINT] = STR.compare();
            break;
         case OP_PUSH:
            STR.push();
            CSTK.pushStringStackPtr(STR.getPreviousStringValuePtr());
            break;
         case OP_PUSH_UINT:
            CSTK.pushUINT(intStack[_UINT]);
            _UINT--;
            break;
         case OP_PUSH_FLT:
            CSTK.pushFLT(floatStack[_FLT]);
            _FLT--;
            break;
         case OP_PUSH_VAR:
            if (gEvalState.currentVariable)
               CSTK.pushValue(gEvalState.currentVariable->value);
            else
               CSTK.pushString("");
            break;

         case OP_PUSH_FRAME:
            STR.pushFrame();
            CSTK.pushFrame();
            break;

         case OP_ASSERT:
         {
            if( !intStack[_UINT--] )
            {
               const char *message = curStringTable + code[ip];

               U32 breakLine, inst;
               findBreakLine( ip - 1, breakLine, inst );

               if ( PlatformAssert::processAssert( PlatformAssert::Fatal, 
                                                   name ? name : "eval", 
                                                   breakLine,  
                                                   message ) )
               {
                  if ( TelDebugger && TelDebugger->isConnected() && breakLine > 0 )
                  {
                     TelDebugger->breakProcess();
                  }
                  else
                     Platform::debugBreak();
               }
            }

            ip++;
            break;
         }

         case OP_BREAK:
         {
            //append the ip and codeptr before managing the breakpoint!
            AssertFatal( gEvalState.getStackDepth() > 0, "Empty eval stack on break!");
            gEvalState.getCurrentFrame().code = this;
            gEvalState.getCurrentFrame().ip = ip - 1;

            U32 breakLine;
            findBreakLine(ip-1, breakLine, instruction);
            if(!breakLine)
               goto breakContinue;
            TelDebugger->executionStopped(this, breakLine);
            goto breakContinue;
         }
         
         case OP_ITER_BEGIN_STR:
         {
            iterStack[ _ITER ].mIsStringIter = true;
            /* fallthrough */
         }
         
         case OP_ITER_BEGIN:
         {
            StringTableEntry varName = CodeToSTE(code, ip);
            U32 failIp = code[ ip + 2 ];
            
            IterStackRecord& iter = iterStack[ _ITER ];
            
            iter.mVariable = gEvalState.getCurrentFrame().add( varName );
            
            if( iter.mIsStringIter )
            {
               iter.mData.mStr.mString = STR.getStringValuePtr();
               iter.mData.mStr.mIndex = 0;
            }
            else
            {
               // Look up the object.
               
               SimSet* set;
               if( !Sim::findObject( STR.getStringValue(), set ) )
               {
                  Con::errorf( ConsoleLogEntry::General, "No SimSet object '%s'", STR.getStringValue() );
                  Con::errorf( ConsoleLogEntry::General, "Did you mean to use 'foreach$' instead of 'foreach'?" );
                  ip = failIp;
                  continue;
               }
               
               // Set up.
               
               iter.mData.mObj.mSet = set;
               iter.mData.mObj.mIndex = 0;
            }
            
            _ITER ++;
            iterDepth ++;
            
            STR.push();
            
            ip += 3;
            break;
         }
         
         case OP_ITER:
         {
            U32 breakIp = code[ ip ];
            IterStackRecord& iter = iterStack[ _ITER - 1 ];
            
            if( iter.mIsStringIter )
            {
               const char* str = StringStackPtrRef(iter.mData.mStr.mString).getPtr(&STR);
                              
               U32 startIndex = iter.mData.mStr.mIndex;
               U32 endIndex = startIndex;
               
               // Break if at end.

               if( !str[ startIndex ] )
               {
                  ip = breakIp;
                  continue;
               }

               // Find right end of current component.
               
               if( !dIsspace( str[ endIndex ] ) )
                  do ++ endIndex;
                  while( str[ endIndex ] && !dIsspace( str[ endIndex ] ) );
                  
               // Extract component.
                  
               if( endIndex != startIndex )
               {
                  char savedChar = str[ endIndex ];
                  const_cast< char* >( str )[ endIndex ] = '\0'; // We are on the string stack so this is okay.
                  iter.mVariable->setStringValue( &str[ startIndex ] );
                  const_cast< char* >( str )[ endIndex ] = savedChar;                  
               }
               else
                  iter.mVariable->setStringValue( "" );

               // Skip separator.
               if( str[ endIndex ] != '\0' )
                  ++ endIndex;
               
               iter.mData.mStr.mIndex = endIndex;
            }
            else
            {
               U32 index = iter.mData.mObj.mIndex;
               SimSet* set = iter.mData.mObj.mSet;
               
               if( index >= set->size() )
               {
                  ip = breakIp;
                  continue;
               }
               
               iter.mVariable->setIntValue( set->at( index )->getId() );
               iter.mData.mObj.mIndex = index + 1;
            }
            
            ++ ip;
            break;
         }
         
         case OP_ITER_END:
         {
            -- _ITER;
            -- iterDepth;
            
            STR.rewind();
            
            iterStack[ _ITER ].mIsStringIter = false;
            break;
         }
         
         case OP_INVALID:

         default:
            // error!
            goto execFinished;
      }
   }
execFinished:

   if ( telDebuggerOn && setFrame < 0 )
      TelDebugger->popStackFrame();

   if ( popFrame )
      gEvalState.popFrame();

   if(argv)
   {
      if(gEvalState.traceOn)
      {
         traceBuffer[0] = 0;
         dStrcat(traceBuffer, "Leaving ");

         if(packageName)
         {
            dStrcat(traceBuffer, "[");
            dStrcat(traceBuffer, packageName);
            dStrcat(traceBuffer, "]");
         }
         if(thisNamespace && thisNamespace->mName)
         {
            dSprintf(traceBuffer + dStrlen(traceBuffer), sizeof(traceBuffer) - dStrlen(traceBuffer),
               "%s::%s() - return %s", thisNamespace->mName, thisFunctionName, STR.getStringValue());
         }
         else
         {
            dSprintf(traceBuffer + dStrlen(traceBuffer), sizeof(traceBuffer) - dStrlen(traceBuffer),
               "%s() - return %s", thisFunctionName, STR.getStringValue());
         }
         Con::printf("%s", traceBuffer);
      }
   }

   smCurrentCodeBlock = saveCodeBlock;
   if(saveCodeBlock && saveCodeBlock->name)
   {
      Con::gCurrentFile = saveCodeBlock->name;
      Con::gCurrentRoot = saveCodeBlock->modPath;
   }

   decRefCount();

#ifdef TORQUE_VALIDATE_STACK
   AssertFatal(!(STR.mStartStackSize > stackStart), "String stack not popped enough in script exec");
   AssertFatal(!(STR.mStartStackSize < stackStart), "String stack popped too much in script exec");
#endif

   return returnValue;
}

//------------------------------------------------------------
