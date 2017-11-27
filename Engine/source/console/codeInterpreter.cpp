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

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// Arcane-FX for MIT Licensed Open Source version of Torque 3D from GarageGames
// Copyright (C) 2015 Faust Logic, Inc.
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#include "console/codeInterpreter.h"
#include "console/compiler.h"
#include "console/simBase.h"
#include "console/telnetDebugger.h"
#include "sim/netStringTable.h"
#include "console/ICallMethod.h"
#include "console/stringStack.h"
#include "util/messaging/message.h"
#include "core/strings/findMatch.h"
#include "core/strings/stringUnit.h"
#include "console/console.h"
#include "console/consoleInternal.h"

//#define TORQUE_VALIDATE_STACK

using namespace Compiler;

enum EvalConstants
{
   MaxStackSize = 1024,
   FieldBufferSizeString = 2048,
   FieldBufferSizeNumeric = 128,
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

// Gets a component of an object's field value or a variable and returns it
// in val.
static void getFieldComponent(SimObject* object, StringTableEntry field, const char* array, StringTableEntry subField, char val[])
{
   const char* prevVal = NULL;

   // Grab value from object.
   if (object && field)
      prevVal = object->getDataField(field, array);

   // Otherwise, grab from the string stack. The value coming in will always
   // be a string because that is how multicomponent variables are handled.
   else
      prevVal = STR.getStringValue();

   // Make sure we got a value.
   if (prevVal && *prevVal)
   {
      static const StringTableEntry xyzw[] =
      {
         StringTable->insert("x"),
         StringTable->insert("y"),
         StringTable->insert("z"),
         StringTable->insert("w")
      };

      static const StringTableEntry rgba[] =
      {
         StringTable->insert("r"),
         StringTable->insert("g"),
         StringTable->insert("b"),
         StringTable->insert("a")
      };

      // Translate xyzw and rgba into the indexed component 
      // of the variable or field.
      //
      // Review: Should we use strncpy to prevent a buffer overflow?
      if (subField == xyzw[0] || subField == rgba[0])
         dStrcpy(val, StringUnit::getUnit(prevVal, 0, " \t\n"));

      else if (subField == xyzw[1] || subField == rgba[1])
         dStrcpy(val, StringUnit::getUnit(prevVal, 1, " \t\n"));

      else if (subField == xyzw[2] || subField == rgba[2])
         dStrcpy(val, StringUnit::getUnit(prevVal, 2, " \t\n"));

      else if (subField == xyzw[3] || subField == rgba[3])
         dStrcpy(val, StringUnit::getUnit(prevVal, 3, " \t\n"));

      else
         val[0] = 0;
   }
   else
      val[0] = 0;
}

// Sets a component of an object's field value based on the sub field. 'x' will
// set the first field, 'y' the second, and 'z' the third.
static void setFieldComponent(SimObject* object, StringTableEntry field, const char* array, StringTableEntry subField)
{
   // Copy the current string value
   char strValue[1024];
   dStrncpy(strValue, STR.getStringValue(), 1024);

   char val[1024] = "";
   const char* prevVal = NULL;

   // Set the value on an object field.
   if (object && field)
      prevVal = object->getDataField(field, array);

   // Set the value on a variable.
   else if (gEvalState.currentVariable)
      prevVal = gEvalState.getStringVariable();

   // Ensure that the variable has a value
   if (!prevVal)
      return;

   static const StringTableEntry xyzw[] =
   {
      StringTable->insert("x"),
      StringTable->insert("y"),
      StringTable->insert("z"),
      StringTable->insert("w")
   };

   static const StringTableEntry rgba[] =
   {
      StringTable->insert("r"),
      StringTable->insert("g"),
      StringTable->insert("b"),
      StringTable->insert("a")
   };

   // Insert the value into the specified 
   // component of the string.
   //
   // Review: Should we use strncpy to prevent a buffer overflow?
   if (subField == xyzw[0] || subField == rgba[0])
      dStrcpy(val, StringUnit::setUnit(prevVal, 0, strValue, " \t\n"));

   else if (subField == xyzw[1] || subField == rgba[1])
      dStrcpy(val, StringUnit::setUnit(prevVal, 1, strValue, " \t\n"));

   else if (subField == xyzw[2] || subField == rgba[2])
      dStrcpy(val, StringUnit::setUnit(prevVal, 2, strValue, " \t\n"));

   else if (subField == xyzw[3] || subField == rgba[3])
      dStrcpy(val, StringUnit::setUnit(prevVal, 3, strValue, " \t\n"));

   if (val[0] != 0)
   {
      // Update the field or variable.
      if (object && field)
         object->setDataField(field, 0, val);
      else if (gEvalState.currentVariable)
         gEvalState.setStringVariable(val);
   }
}
extern ExprEvalState gEvalState;

char sTraceBuffer[1024];

StringStack STR;
ConsoleValueStack CSTK;

U32 _FLT = 0;     ///< Stack pointer for floatStack.
U32 _UINT = 0;    ///< Stack pointer for intStack.
U32 _ITER = 0;    ///< Stack pointer for iterStack.

IterStackRecord iterStack[MaxStackSize];

F64 floatStack[MaxStackSize];
S64 intStack[MaxStackSize];

char curFieldArray[256];
char prevFieldArray[256];

typedef OPCodeReturn(CodeInterpreter::*OpFn)(U32&);

static OpFn gOpCodeArray[MAX_OP_CODELEN];

CodeInterpreter::CodeInterpreter(CodeBlock *cb) :
   mCodeBlock(cb),
   mIterDepth(0),
   mCurFloatTable(nullptr),
   mCurStringTable(nullptr),
   mThisFunctionName(nullptr),
   mPopFrame(false),
   mObjectCreationStackIndex(0),
   mCurrentNewObject(nullptr),
   mFailJump(0),
   mPrevField(nullptr),
   mCurField(nullptr),
   mPrevObject(nullptr),
   mCurObject(nullptr),
   mSaveObject(nullptr),
   mThisObject(nullptr),
   mNSEntry(nullptr),
   mCurFNDocBlock(nullptr),
   mCurNSDocBlock(nullptr),
   mCallArgc(0),
   mCallArgv(nullptr),
   mSaveCodeBlock(nullptr),
   mCurrentInstruction(0)
{
}

CodeInterpreter::~CodeInterpreter()
{
}

void CodeInterpreter::init()
{
   gOpCodeArray[OP_FUNC_DECL] = &CodeInterpreter::op_func_decl;
   gOpCodeArray[OP_CREATE_OBJECT] = &CodeInterpreter::op_create_object;
   gOpCodeArray[OP_ADD_OBJECT] = &CodeInterpreter::op_add_object;
   gOpCodeArray[OP_END_OBJECT] = &CodeInterpreter::op_end_object;
   gOpCodeArray[OP_FINISH_OBJECT] = &CodeInterpreter::op_finish_object;
   gOpCodeArray[OP_JMPIFFNOT] = &CodeInterpreter::op_jmpiffnot;
   gOpCodeArray[OP_JMPIFNOT] = &CodeInterpreter::op_jmpifnot;
   gOpCodeArray[OP_JMPIFF] = &CodeInterpreter::op_jmpiff;
   gOpCodeArray[OP_JMPIF] = &CodeInterpreter::op_jmpif;
   gOpCodeArray[OP_JMPIFNOT_NP] = &CodeInterpreter::op_jmpifnot_np;
   gOpCodeArray[OP_JMPIF_NP] = &CodeInterpreter::op_jmpif_np;
   gOpCodeArray[OP_JMP] = &CodeInterpreter::op_jmp;
   gOpCodeArray[OP_RETURN] = &CodeInterpreter::op_return;
   gOpCodeArray[OP_RETURN_VOID] = &CodeInterpreter::op_return_void;
   gOpCodeArray[OP_RETURN_FLT] = &CodeInterpreter::op_return_flt;
   gOpCodeArray[OP_RETURN_UINT] = &CodeInterpreter::op_return_uint;
   gOpCodeArray[OP_CMPEQ] = &CodeInterpreter::op_cmpeq;
   gOpCodeArray[OP_CMPGR] = &CodeInterpreter::op_cmpgr;
   gOpCodeArray[OP_CMPGE] = &CodeInterpreter::op_cmpge;
   gOpCodeArray[OP_CMPLT] = &CodeInterpreter::op_cmplt;
   gOpCodeArray[OP_CMPLE] = &CodeInterpreter::op_cmple;
   gOpCodeArray[OP_CMPNE] = &CodeInterpreter::op_cmpne;
   gOpCodeArray[OP_XOR] = &CodeInterpreter::op_xor;
   gOpCodeArray[OP_MOD] = &CodeInterpreter::op_mod;
   gOpCodeArray[OP_BITAND] = &CodeInterpreter::op_bitand;
   gOpCodeArray[OP_BITOR] = &CodeInterpreter::op_bitor;
   gOpCodeArray[OP_NOT] = &CodeInterpreter::op_not;
   gOpCodeArray[OP_NOTF] = &CodeInterpreter::op_notf;
   gOpCodeArray[OP_ONESCOMPLEMENT] = &CodeInterpreter::op_onescomplement;
   gOpCodeArray[OP_SHR] = &CodeInterpreter::op_shr;
   gOpCodeArray[OP_SHL] = &CodeInterpreter::op_shl;
   gOpCodeArray[OP_AND] = &CodeInterpreter::op_and;
   gOpCodeArray[OP_OR] = &CodeInterpreter::op_or;
   gOpCodeArray[OP_ADD] = &CodeInterpreter::op_add;
   gOpCodeArray[OP_SUB] = &CodeInterpreter::op_sub;
   gOpCodeArray[OP_MUL] = &CodeInterpreter::op_mul;
   gOpCodeArray[OP_DIV] = &CodeInterpreter::op_div;
   gOpCodeArray[OP_NEG] = &CodeInterpreter::op_neg;
   gOpCodeArray[OP_INC] = &CodeInterpreter::op_inc;
   gOpCodeArray[OP_DEC] = &CodeInterpreter::op_dec;
   gOpCodeArray[OP_SETCURVAR] = &CodeInterpreter::op_setcurvar;
   gOpCodeArray[OP_SETCURVAR_CREATE] = &CodeInterpreter::op_setcurvar_create;
   gOpCodeArray[OP_SETCURVAR_ARRAY] = &CodeInterpreter::op_setcurvar_array;
   gOpCodeArray[OP_SETCURVAR_ARRAY_VARLOOKUP] = &CodeInterpreter::op_setcurvar_array_varlookup;
   gOpCodeArray[OP_SETCURVAR_ARRAY_CREATE] = &CodeInterpreter::op_setcurvar_array_create;
   gOpCodeArray[OP_SETCURVAR_ARRAY_CREATE_VARLOOKUP] = &CodeInterpreter::op_setcurvar_array_create_varlookup;
   gOpCodeArray[OP_LOADVAR_UINT] = &CodeInterpreter::op_loadvar_uint;
   gOpCodeArray[OP_LOADVAR_FLT] = &CodeInterpreter::op_loadvar_flt;
   gOpCodeArray[OP_LOADVAR_STR] = &CodeInterpreter::op_loadvar_str;
   gOpCodeArray[OP_LOADVAR_VAR] = &CodeInterpreter::op_loadvar_var;
   gOpCodeArray[OP_SAVEVAR_UINT] = &CodeInterpreter::op_savevar_uint;
   gOpCodeArray[OP_SAVEVAR_FLT] = &CodeInterpreter::op_savevar_flt;
   gOpCodeArray[OP_SAVEVAR_STR] = &CodeInterpreter::op_savevar_str;
   gOpCodeArray[OP_SAVEVAR_VAR] = &CodeInterpreter::op_savevar_var;
   gOpCodeArray[OP_SETCUROBJECT] = &CodeInterpreter::op_setcurobject;
   gOpCodeArray[OP_SETCUROBJECT_INTERNAL] = &CodeInterpreter::op_setcurobject_internal;
   gOpCodeArray[OP_SETCUROBJECT_NEW] = &CodeInterpreter::op_setcurobject_new;
   gOpCodeArray[OP_SETCURFIELD] = &CodeInterpreter::op_setcurfield;
   gOpCodeArray[OP_SETCURFIELD_ARRAY] = &CodeInterpreter::op_setcurfield_array;
   gOpCodeArray[OP_SETCURFIELD_TYPE] = &CodeInterpreter::op_setcurfield_type;
   gOpCodeArray[OP_SETCURFIELD_ARRAY_VAR] = &CodeInterpreter::op_setcurfield_array_var;
   gOpCodeArray[OP_SETCURFIELD_THIS] = &CodeInterpreter::op_setcurfield_this;
   gOpCodeArray[OP_LOADFIELD_UINT] = &CodeInterpreter::op_loadfield_uint;
   gOpCodeArray[OP_LOADFIELD_FLT] = &CodeInterpreter::op_loadfield_flt;
   gOpCodeArray[OP_LOADFIELD_STR] = &CodeInterpreter::op_loadfield_str;
   gOpCodeArray[OP_SAVEFIELD_UINT] = &CodeInterpreter::op_savefield_uint;
   gOpCodeArray[OP_SAVEFIELD_FLT] = &CodeInterpreter::op_savefield_flt;
   gOpCodeArray[OP_SAVEFIELD_STR] = &CodeInterpreter::op_savefield_str;
   gOpCodeArray[OP_STR_TO_UINT] = &CodeInterpreter::op_str_to_uint;
   gOpCodeArray[OP_STR_TO_FLT] = &CodeInterpreter::op_str_to_flt;
   gOpCodeArray[OP_STR_TO_NONE] = &CodeInterpreter::op_str_to_none;
   gOpCodeArray[OP_FLT_TO_UINT] = &CodeInterpreter::op_flt_to_uint;
   gOpCodeArray[OP_FLT_TO_STR] = &CodeInterpreter::op_flt_to_str;
   gOpCodeArray[OP_FLT_TO_NONE] = &CodeInterpreter::op_flt_to_none;
   gOpCodeArray[OP_UINT_TO_FLT] = &CodeInterpreter::op_uint_to_flt;
   gOpCodeArray[OP_UINT_TO_STR] = &CodeInterpreter::op_uint_to_str;
   gOpCodeArray[OP_UINT_TO_NONE] = &CodeInterpreter::op_uint_to_none;
   gOpCodeArray[OP_COPYVAR_TO_NONE] = &CodeInterpreter::op_copyvar_to_none;
   gOpCodeArray[OP_LOADIMMED_UINT] = &CodeInterpreter::op_loadimmed_uint;
   gOpCodeArray[OP_LOADIMMED_FLT] = &CodeInterpreter::op_loadimmed_flt;
   gOpCodeArray[OP_TAG_TO_STR] = &CodeInterpreter::op_tag_to_str;
   gOpCodeArray[OP_LOADIMMED_STR] = &CodeInterpreter::op_loadimmed_str;
   gOpCodeArray[OP_DOCBLOCK_STR] = &CodeInterpreter::op_docblock_str;
   gOpCodeArray[OP_LOADIMMED_IDENT] = &CodeInterpreter::op_loadimmed_ident;
   gOpCodeArray[OP_CALLFUNC_RESOLVE] = &CodeInterpreter::op_callfunc_resolve;
   gOpCodeArray[OP_CALLFUNC] = &CodeInterpreter::op_callfunc;
   gOpCodeArray[OP_CALLFUNC_POINTER] = &CodeInterpreter::op_callfunc_pointer;
   gOpCodeArray[OP_CALLFUNC_THIS] = &CodeInterpreter::op_callfunc_this;
   gOpCodeArray[OP_ADVANCE_STR] = &CodeInterpreter::op_advance_str;
   gOpCodeArray[OP_ADVANCE_STR_APPENDCHAR] = &CodeInterpreter::op_advance_str_appendchar;
   gOpCodeArray[OP_ADVANCE_STR_COMMA] = &CodeInterpreter::op_advance_str_comma;
   gOpCodeArray[OP_ADVANCE_STR_NUL] = &CodeInterpreter::op_advance_str_nul;
   gOpCodeArray[OP_REWIND_STR] = &CodeInterpreter::op_rewind_str;
   gOpCodeArray[OP_TERMINATE_REWIND_STR] = &CodeInterpreter::op_terminate_rewind_str;
   gOpCodeArray[OP_COMPARE_STR] = &CodeInterpreter::op_compare_str;
   gOpCodeArray[OP_PUSH] = &CodeInterpreter::op_push;
   gOpCodeArray[OP_PUSH_UINT] = &CodeInterpreter::op_push_uint;
   gOpCodeArray[OP_PUSH_FLT] = &CodeInterpreter::op_push_flt;
   gOpCodeArray[OP_PUSH_VAR] = &CodeInterpreter::op_push_var;
   gOpCodeArray[OP_PUSH_THIS] = &CodeInterpreter::op_push_this;
   gOpCodeArray[OP_PUSH_FRAME] = &CodeInterpreter::op_push_frame;
   gOpCodeArray[OP_ASSERT] = &CodeInterpreter::op_assert;
   gOpCodeArray[OP_BREAK] = &CodeInterpreter::op_break;
   gOpCodeArray[OP_ITER_BEGIN_STR] = &CodeInterpreter::op_iter_begin_str;
   gOpCodeArray[OP_ITER_BEGIN] = &CodeInterpreter::op_iter_begin;
   gOpCodeArray[OP_ITER] = &CodeInterpreter::op_iter;
   gOpCodeArray[OP_ITER_END] = &CodeInterpreter::op_iter_end;
   gOpCodeArray[OP_INVALID] = &CodeInterpreter::op_invalid;
}

ConsoleValueRef CodeInterpreter::exec(U32 ip,
                                      StringTableEntry functionName,
                                      Namespace *thisNamespace,
                                      U32 argc, 
                                      ConsoleValueRef *argv,
                                      bool noCalls,
                                      StringTableEntry packageName,
                                      S32 setFrame) 
{
   mExec.functionName = functionName;
   mExec.thisNamespace = thisNamespace;
   mExec.argc = argc;
   mExec.argv = argv;
   mExec.noCalls = noCalls;
   mExec.packageName = packageName;
   mExec.setFrame = setFrame;

   mCodeBlock->incRefCount();

   mPopFrame = false;

#ifdef TORQUE_VALIDATE_STACK
   U32 stackStart = STR.mStartStackSize;
#endif

   STR.clearFunctionOffset(); // ensures arg buffer offset is back to 0

   // Lets load up our function arguments.
   parseArgs(ip);

   // Grab the state of the telenet debugger here once
   // so that the push and pop frames are always balanced.
   const bool telDebuggerOn = TelDebugger && TelDebugger->isConnected();
   if (telDebuggerOn && setFrame < 0)
      TelDebugger->pushStackFrame();

   mSaveCodeBlock = CodeBlock::smCurrentCodeBlock;
   CodeBlock::smCurrentCodeBlock = mCodeBlock;
   if (mCodeBlock->name)
   {
      Con::gCurrentFile = mCodeBlock->name;
      Con::gCurrentRoot = mCodeBlock->modPath;
   }

   U32 *code = mCodeBlock->code;

   while (true)
   {
      mCurrentInstruction = code[ip++];
      mNSEntry = nullptr;

#ifdef TORQUE_VALIDATE_STACK
      // OP Code check.
      AssertFatal(mCurrentInstruction < MAX_OP_CODELEN, "Invalid OP code in script interpreter");
#endif

   breakContinueLabel:
      OPCodeReturn ret = (this->*gOpCodeArray[mCurrentInstruction])(ip);
      if (ret == OPCodeReturn::exitCode)
         goto exitLabel;
      else if (ret == OPCodeReturn::breakContinue)
         goto breakContinueLabel;
   }
exitLabel:
   if (telDebuggerOn && setFrame < 0)
      TelDebugger->popStackFrame();

   if (mPopFrame)
      gEvalState.popFrame();

   if (argv)
   {
      if (gEvalState.traceOn)
      {
         sTraceBuffer[0] = 0;
         dStrcat(sTraceBuffer, "Leaving ");

         if (packageName)
         {
            dStrcat(sTraceBuffer, "[");
            dStrcat(sTraceBuffer, packageName);
            dStrcat(sTraceBuffer, "]");
         }
         if (thisNamespace && thisNamespace->mName)
         {
            dSprintf(sTraceBuffer + dStrlen(sTraceBuffer), sizeof(sTraceBuffer) - dStrlen(sTraceBuffer),
               "%s::%s() - return %s", thisNamespace->mName, mThisFunctionName, STR.getStringValue());
         }
         else
         {
            dSprintf(sTraceBuffer + dStrlen(sTraceBuffer), sizeof(sTraceBuffer) - dStrlen(sTraceBuffer),
               "%s() - return %s", mThisFunctionName, STR.getStringValue());
         }
         Con::printf("%s", sTraceBuffer);
      }
   }

   CodeBlock::smCurrentCodeBlock = mSaveCodeBlock;
   if (mSaveCodeBlock && mSaveCodeBlock->name)
   {
      Con::gCurrentFile = mSaveCodeBlock->name;
      Con::gCurrentRoot = mSaveCodeBlock->modPath;
   }

   mCodeBlock->decRefCount();

#ifdef TORQUE_VALIDATE_STACK
   AssertFatal(!(STR.mStartStackSize > stackStart), "String stack not popped enough in script exec");
   AssertFatal(!(STR.mStartStackSize < stackStart), "String stack popped too much in script exec");
#endif

   return mReturnValue;
}

void CodeInterpreter::parseArgs(U32 &ip)
{
   U32 *code = mCodeBlock->code;

   if (mExec.argv)
   {
      U32 fnArgc = code[ip + 2 + 6];
      mThisFunctionName = Compiler::CodeToSTE(code, ip);
      S32 wantedArgc = getMin(mExec.argc - 1, fnArgc); // argv[0] is func name
      if (gEvalState.traceOn)
      {
         sTraceBuffer[0] = 0;
         dStrcat(sTraceBuffer, "Entering ");

         if (mExec.packageName)
         {
            dStrcat(sTraceBuffer, "[");
            dStrcat(sTraceBuffer, mExec.packageName);
            dStrcat(sTraceBuffer, "]");
         }
         if (mExec.thisNamespace && mExec.thisNamespace->mName)
         {
            dSprintf(sTraceBuffer + dStrlen(sTraceBuffer), sizeof(sTraceBuffer) - dStrlen(sTraceBuffer),
               "%s::%s(", mExec.thisNamespace->mName, mThisFunctionName);
         }
         else
         {
            dSprintf(sTraceBuffer + dStrlen(sTraceBuffer), sizeof(sTraceBuffer) - dStrlen(sTraceBuffer),
               "%s(", mThisFunctionName);
         }
         for (S32 i = 0; i < wantedArgc; i++)
         {
            dStrcat(sTraceBuffer, mExec.argv[i + 1]);
            if (i != wantedArgc - 1)
               dStrcat(sTraceBuffer, ", ");
         }
         dStrcat(sTraceBuffer, ")");
         Con::printf("%s", sTraceBuffer);
      }

      gEvalState.pushFrame(mThisFunctionName, mExec.thisNamespace);
      mPopFrame = true;

      StringTableEntry thisPointer = StringTable->insert("%this");

      for (S32 i = 0; i < wantedArgc; i++)
      {
         StringTableEntry var = Compiler::CodeToSTE(code, ip + (2 + 6 + 1) + (i * 2));
         gEvalState.setCurVarNameCreate(var);

         ConsoleValueRef ref = mExec.argv[i + 1];

         switch (ref.getType())
         {
         case ConsoleValue::TypeInternalInt:
            gEvalState.setIntVariable(ref);
            break;
         case ConsoleValue::TypeInternalFloat:
            gEvalState.setFloatVariable(ref);
            break;
         case ConsoleValue::TypeInternalStringStackPtr:
            gEvalState.setStringStackPtrVariable(ref.getStringStackPtrValue());
            break;
         case ConsoleValue::TypeInternalStackString:
         case ConsoleValue::TypeInternalString:
         default:
            gEvalState.setStringVariable(ref);
            break;
         }

         if (var == thisPointer)
         {
            // %this gets optimized as it is flagged as a constant.
            // Since it is guarenteed to be constant, we can then perform optimizations.
            gEvalState.currentVariable->mIsConstant = true;

            // Store a reference to the this pointer object.
            mThisObject = Sim::findObject(gEvalState.getStringVariable());
         }
      }

      ip = ip + (fnArgc * 2) + (2 + 6 + 1);
      mCurFloatTable = mCodeBlock->functionFloats;
      mCurStringTable = mCodeBlock->functionStrings;
   }
   else
   {
      mCurFloatTable = mCodeBlock->globalFloats;
      mCurStringTable = mCodeBlock->globalStrings;

      // If requested stack frame isn't available, request a new one
      // (this prevents assert failures when creating local
      //  variables without a stack frame)
      if (gEvalState.getStackDepth() <= mExec.setFrame)
         mExec.setFrame = -1;

      // Do we want this code to execute using a new stack frame?
      if (mExec.setFrame < 0)
      {
         gEvalState.pushFrame(NULL, NULL);
         mPopFrame = true;
      }
      else
      {
         // We want to copy a reference to an existing stack frame
         // on to the top of the stack.  Any change that occurs to 
         // the locals during this new frame will also occur in the 
         // original frame.
         S32 stackIndex = gEvalState.getStackDepth() - mExec.setFrame - 1;
         gEvalState.pushFrameRef(stackIndex);
         mPopFrame = true;
      }
   }
}

OPCodeReturn CodeInterpreter::op_func_decl(U32 &ip)
{
   U32 *code = mCodeBlock->code;

   if (!mExec.noCalls)
   {
      StringTableEntry fnName = CodeToSTE(code, ip);
      StringTableEntry fnNamespace = CodeToSTE(code, ip + 2);
      StringTableEntry fnPackage = CodeToSTE(code, ip + 4);
      bool hasBody = (code[ip + 6] & 0x01) != 0;
      U32 lineNumber = code[ip + 6] >> 1;

      Namespace::unlinkPackages();
      Namespace *ns = Namespace::find(fnNamespace, fnPackage);
      ns->addFunction(fnName, mCodeBlock, hasBody ? ip : 0, mCurFNDocBlock ? dStrdup(mCurFNDocBlock) : NULL, lineNumber);// if no body, set the IP to 0
      if (mCurNSDocBlock)
      {
         // If we have a docblock before we declare the function in the script file,
         // this will attempt to set the doc block to the function.
         // See OP_DOCBLOCK_STR
         if (fnNamespace == StringTable->lookup(mNSDocBlockClass))
         {
            char *usageStr = dStrdup(mCurNSDocBlock);
            usageStr[dStrlen(usageStr)] = '\0';
            ns->mUsage = usageStr;
            ns->mCleanUpUsage = true;
            mCurNSDocBlock = NULL;
         }
      }
      Namespace::relinkPackages();

      // If we had a docblock, it's definitely not valid anymore, so clear it out.
      mCurFNDocBlock = NULL;

      //Con::printf("Adding function %s::%s (%d)", fnNamespace, fnName, ip);
   }
   ip = code[ip + 7];
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_create_object(U32 &ip)
{
   U32 *code = mCodeBlock->code;

   // Read some useful info.
   StringTableEntry objParent = CodeToSTE(code, ip);
   bool isDataBlock = code[ip + 2];
   bool isInternal = code[ip + 3];
   bool isSingleton = code[ip + 4];
   U32  lineNumber = code[ip + 5];
   mFailJump = code[ip + 6];

   // If we don't allow calls, we certainly don't allow creating objects!
   // Moved this to after failJump is set. Engine was crashing when
   // noCalls = true and an object was being created at the beginning of
   // a file. ADL.
   if (mExec.noCalls)
   {
      ip = mFailJump;
      return OPCodeReturn::success;
   }

   // Push the old info to the stack
   //Assert( objectCreationStackIndex < objectCreationStackSize );
   mObjectCreationStack[mObjectCreationStackIndex].newObject = mCurrentNewObject;
   mObjectCreationStack[mObjectCreationStackIndex++].failJump = mFailJump;

   // Get the constructor information off the stack.
   CSTK.getArgcArgv(NULL, &mCallArgc, &mCallArgv);
   const char *objectName = mCallArgv[2];

   // Con::printf("Creating object...");

   // objectName = argv[1]...
   mCurrentNewObject = NULL;

   // Are we creating a datablock? If so, deal with case where we override
   // an old one.
   if (isDataBlock)
   {
      // Con::printf("  - is a datablock");

      // Find the old one if any.
      SimObject *db = Sim::getDataBlockGroup()->findObject(objectName);

      // Make sure we're not changing types on ourselves...
      if (db && dStricmp(db->getClassName(), mCallArgv[1]))
      {
         Con::errorf(ConsoleLogEntry::General, "%s: Cannot re-declare data block %s with a different class.", mCodeBlock->getFileLine(ip), objectName);
         ip = mFailJump;
         STR.popFrame();
         CSTK.popFrame();
         return OPCodeReturn::success;
      }

      // If there was one, set the currentNewObject and move on.
      if (db)
         mCurrentNewObject = db;
   }
   else if (!isInternal)
   {
      // IF we aren't looking at a local/internal object, then check if 
      // this object already exists in the global space

      AbstractClassRep* rep = AbstractClassRep::findClassRep(objectName);
      if (rep != NULL) {
         Con::errorf(ConsoleLogEntry::General, "%s: Cannot name object [%s] the same name as a script class.",
            mCodeBlock->getFileLine(ip), objectName);
         ip = mFailJump;
         STR.popFrame();
         CSTK.popFrame();
         return OPCodeReturn::success;
      }

      SimObject *obj = Sim::findObject((const char*)objectName);
      if (obj /*&& !obj->isLocalName()*/)
      {
         if (isSingleton)
         {
            // Make sure we're not trying to change types
            if (dStricmp(obj->getClassName(), (const char*)mCallArgv[1]) != 0)
            {
               Con::errorf(ConsoleLogEntry::General, "%s: Cannot re-declare object [%s] with a different class [%s] - was [%s].",
                  mCodeBlock->getFileLine(ip), objectName, (const char*)mCallArgv[1], obj->getClassName());
               ip = mFailJump;
               STR.popFrame();
               CSTK.popFrame();
               return OPCodeReturn::success;
            }

            // We're creating a singleton, so use the found object
            // instead of creating a new object.
            mCurrentNewObject = obj;
         }
         else
         {
            const char* redefineBehavior = Con::getVariable("$Con::redefineBehavior");

            if (dStricmp(redefineBehavior, "replaceExisting") == 0)
            {
               // Save our constructor args as the argv vector is stored on the
               // string stack and may get stomped if deleteObject triggers
               // script execution.

               ConsoleValueRef savedArgv[StringStack::MaxArgs];
               for (int i = 0; i< mCallArgc; i++) {
                  savedArgv[i] = mCallArgv[i];
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
               for (int i = 0; i<mCallArgc; i++) {
                  mCallArgv[i] = savedArgv[i];
               }
            }
            else if (dStricmp(redefineBehavior, "renameNew") == 0)
            {
               for (U32 i = 1;; ++i)
               {
                  String newName = String::ToString("%s%i", objectName, i);
                  if (!Sim::findObject(newName))
                  {
                     objectName = StringTable->insert(newName);
                     break;
                  }
               }
            }
            else if (dStricmp(redefineBehavior, "unnameNew") == 0)
            {
               objectName = StringTable->insert("");
            }
            else if (dStricmp(redefineBehavior, "postfixNew") == 0)
            {
               const char* postfix = Con::getVariable("$Con::redefineBehaviorPostfix");
               String newName = String::ToString("%s%s", objectName, postfix);

               if (Sim::findObject(newName))
               {
                  Con::errorf(ConsoleLogEntry::General, "%s: Cannot re-declare object with postfix [%s].",
                     mCodeBlock->getFileLine(ip), newName.c_str());
                  ip = mFailJump;
                  STR.popFrame();
                  CSTK.popFrame();
                  return OPCodeReturn::success;
               }
               else
                  objectName = StringTable->insert(newName);
            }
            else
            {
               Con::errorf(ConsoleLogEntry::General, "%s: Cannot re-declare object [%s].",
                  mCodeBlock->getFileLine(ip), objectName);
               ip = mFailJump;
               STR.popFrame();
               CSTK.popFrame();
               return OPCodeReturn::success;
            }
         }
      }
   }

   STR.popFrame();
   CSTK.popFrame();

   if (!mCurrentNewObject)
   {
      // Well, looks like we have to create a new object.
      ConsoleObject *object = ConsoleObject::create((const char*)mCallArgv[1]);

      // Deal with failure!
      if (!object)
      {
         Con::errorf(ConsoleLogEntry::General, "%s: Unable to instantiate non-conobject class %s.", mCodeBlock->getFileLine(ip), (const char*)mCallArgv[1]);
         ip = mFailJump;
         return OPCodeReturn::success;
      }

      // Do special datablock init if appropros
      if (isDataBlock)
      {
         SimDataBlock *dataBlock = dynamic_cast<SimDataBlock *>(object);
         if (dataBlock)
         {
            dataBlock->assignId();
         }
         else
         {
            // They tried to make a non-datablock with a datablock keyword!
            Con::errorf(ConsoleLogEntry::General, "%s: Unable to instantiate non-datablock class %s.", mCodeBlock->getFileLine(ip), (const char*)mCallArgv[1]);
            // Clean up...
            delete object;
            mCurrentNewObject = NULL;
            ip = mFailJump;
            return OPCodeReturn::success;
         }
      }

      // Finally, set currentNewObject to point to the new one.
      mCurrentNewObject = dynamic_cast<SimObject *>(object);

      // Deal with the case of a non-SimObject.
      if (!mCurrentNewObject)
      {
         Con::errorf(ConsoleLogEntry::General, "%s: Unable to instantiate non-SimObject class %s.", mCodeBlock->getFileLine(ip), (const char*)mCallArgv[1]);
         delete object;
         mCurrentNewObject = NULL;
         ip = mFailJump;
         return OPCodeReturn::success;
      }

      // Set the declaration line
      mCurrentNewObject->setDeclarationLine(lineNumber);

      // Set the file that this object was created in
      mCurrentNewObject->setFilename(mCodeBlock->name);

      // Does it have a parent object? (ie, the copy constructor : syntax, not inheriance)
      if (*objParent)
      {
         // Find it!
         SimObject *parent;
         if (Sim::findObject(objParent, parent))
         {
            // Con::printf(" - Parent object found: %s", parent->getClassName());

            mCurrentNewObject->setCopySource(parent);
            mCurrentNewObject->assignFieldsFrom(parent);

            // copy any substitution statements
            SimDataBlock* parent_db = dynamic_cast<SimDataBlock*>(parent);
            if (parent_db)
            {
               SimDataBlock* currentNewObject_db = dynamic_cast<SimDataBlock*>(mCurrentNewObject);
               if (currentNewObject_db)
                  currentNewObject_db->copySubstitutionsFrom(parent_db);
            }
         }
         else
         {
            if (Con::gObjectCopyFailures == -1)
               Con::errorf(ConsoleLogEntry::General, "%s: Unable to find parent object %s for %s.", mCodeBlock->getFileLine(ip), objParent, (const char*)mCallArgv[1]);
            else
               ++Con::gObjectCopyFailures;

            // Fail to create the object.
            delete object;
            mCurrentNewObject = NULL;
            ip = mFailJump;
            return OPCodeReturn::success;
         }
      }

      // If a name was passed, assign it.
      if (objectName[0])
      {
         if (!isInternal)
            mCurrentNewObject->assignName(objectName);
         else
            mCurrentNewObject->setInternalName(objectName);

         // Set the original name
         mCurrentNewObject->setOriginalName(objectName);
      }

      // Prevent stack value corruption
      CSTK.pushFrame();
      STR.pushFrame();
      // --

      // Do the constructor parameters.
      if (!mCurrentNewObject->processArguments(mCallArgc - 3, mCallArgv + 3))
      {
         delete mCurrentNewObject;
         mCurrentNewObject = NULL;
         ip = mFailJump;

         // Prevent stack value corruption
         CSTK.popFrame();
         STR.popFrame();
         // --
         return OPCodeReturn::success;
      }

      // Prevent stack value corruption
      CSTK.popFrame();
      STR.popFrame();
      // --

      // If it's not a datablock, allow people to modify bits of it.
      if (!isDataBlock)
      {
         mCurrentNewObject->setModStaticFields(true);
         mCurrentNewObject->setModDynamicFields(true);
      }
   }
   else
   {
      mCurrentNewObject->reloadReset(); // AFX (reload-reset)
                                       // Does it have a parent object? (ie, the copy constructor : syntax, not inheriance)
      if (*objParent)
      {
         // Find it!
         SimObject *parent;
         if (Sim::findObject(objParent, parent))
         {
            // Con::printf(" - Parent object found: %s", parent->getClassName());

            // temporarily block name change
            SimObject::preventNameChanging = true;
            mCurrentNewObject->setCopySource(parent);
            mCurrentNewObject->assignFieldsFrom(parent);
            // restore name changing
            SimObject::preventNameChanging = false;

            // copy any substitution statements
            SimDataBlock* parent_db = dynamic_cast<SimDataBlock*>(parent);
            if (parent_db)
            {
               SimDataBlock* currentNewObject_db = dynamic_cast<SimDataBlock*>(mCurrentNewObject);
               if (currentNewObject_db)
                  currentNewObject_db->copySubstitutionsFrom(parent_db);
            }
         }
         else
            Con::errorf(ConsoleLogEntry::General, "%d: Unable to find parent object %s for %s.", lineNumber, objParent, (const char*)mCallArgv[1]);
      }
   }

   // Advance the IP past the create info...
   ip += 7;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_add_object(U32 &ip)
{
   // See OP_SETCURVAR for why we do this.
   mCurFNDocBlock = NULL;
   mCurNSDocBlock = NULL;

   // Do we place this object at the root?
   bool placeAtRoot = mCodeBlock->code[ip++];

   // Con::printf("Adding object %s", currentNewObject->getName());

   // Prevent stack value corruption
   CSTK.pushFrame();
   STR.pushFrame();
   // --

   // Make sure it wasn't already added, then add it.
   if (mCurrentNewObject->isProperlyAdded() == false)
   {
      bool ret = false;

      Message *msg = dynamic_cast<Message *>(mCurrentNewObject);
      if (msg)
      {
         SimObjectId id = Message::getNextMessageID();
         if (id != 0xffffffff)
            ret = mCurrentNewObject->registerObject(id);
         else
            Con::errorf("%s: No more object IDs available for messages", mCodeBlock->getFileLine(ip));
      }
      else
         ret = mCurrentNewObject->registerObject();

      if (!ret)
      {
         // This error is usually caused by failing to call Parent::initPersistFields in the class' initPersistFields().
         Con::warnf(ConsoleLogEntry::General, "%s: Register object failed for object %s of class %s.", mCodeBlock->getFileLine(ip), mCurrentNewObject->getName(), mCurrentNewObject->getClassName());
         delete mCurrentNewObject;
         mCurrentNewObject = NULL;
         ip = mFailJump;
         // Prevent stack value corruption
         CSTK.popFrame();
         STR.popFrame();
         // --
         return OPCodeReturn::success;
      }
   }

   // Are we dealing with a datablock?
   SimDataBlock *dataBlock = dynamic_cast<SimDataBlock *>(mCurrentNewObject);
   static String errorStr;

   // If so, preload it.
   if (dataBlock && !dataBlock->preload(true, errorStr))
   {
      Con::errorf(ConsoleLogEntry::General, "%s: preload failed for %s: %s.", mCodeBlock->getFileLine(ip),
         mCurrentNewObject->getName(), errorStr.c_str());
      dataBlock->deleteObject();
      mCurrentNewObject = NULL;
      ip = mFailJump;

      // Prevent stack value corruption
      CSTK.popFrame();
      STR.popFrame();
      // --
      return OPCodeReturn::success;
   }

   // What group will we be added to, if any?
   U32 groupAddId = intStack[_UINT];
   SimGroup *grp = NULL;
   SimSet   *set = NULL;
   bool isMessage = dynamic_cast<Message *>(mCurrentNewObject) != NULL;

   if (!placeAtRoot || !mCurrentNewObject->getGroup())
   {
      if (!isMessage)
      {
         if (!placeAtRoot)
         {
            // Otherwise just add to the requested group or set.
            if (!Sim::findObject(groupAddId, grp))
               Sim::findObject(groupAddId, set);
         }

         if (placeAtRoot)
         {
            // Deal with the instantGroup if we're being put at the root or we're adding to a component.
            if (Con::gInstantGroup.isEmpty()
               || !Sim::findObject(Con::gInstantGroup, grp))
               grp = Sim::getRootGroup();
         }
      }

      // If we didn't get a group, then make sure we have a pointer to
      // the rootgroup.
      if (!grp)
         grp = Sim::getRootGroup();

      // add to the parent group
      grp->addObject(mCurrentNewObject);

      // If for some reason the add failed, add the object to the
      // root group so it won't leak.
      if (!mCurrentNewObject->getGroup())
         Sim::getRootGroup()->addObject(mCurrentNewObject);

      // add to any set we might be in
      if (set)
         set->addObject(mCurrentNewObject);
   }

   // store the new object's ID on the stack (overwriting the group/set
   // id, if one was given, otherwise getting pushed)
   if (placeAtRoot)
      intStack[_UINT] = mCurrentNewObject->getId();
   else
      intStack[++_UINT] = mCurrentNewObject->getId();

   // Prevent stack value corruption
   CSTK.popFrame();
   STR.popFrame();
   // --
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_end_object(U32 &ip)
{
   // If we're not to be placed at the root, make sure we clean up
   // our group reference.
   bool placeAtRoot = mCodeBlock->code[ip++];
   if (!placeAtRoot)
      _UINT--;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_finish_object(U32 &ip)
{
   if (mCurrentNewObject)
      mCurrentNewObject->onPostAdd();

   //Assert( objectCreationStackIndex >= 0 );
   // Restore the object info from the stack [7/9/2007 Black]
   mCurrentNewObject = mObjectCreationStack[--mObjectCreationStackIndex].newObject;
   mFailJump = mObjectCreationStack[mObjectCreationStackIndex].failJump;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_jmpiffnot(U32 &ip)
{
   if (floatStack[_FLT--])
   {
      ip++;
      return OPCodeReturn::success;
   }
   ip = mCodeBlock->code[ip];
   return OPCodeReturn::success;
}


OPCodeReturn CodeInterpreter::op_jmpifnot(U32 &ip)
{
   if (intStack[_UINT--])
   {
      ip++;
      return OPCodeReturn::success;
   }
   ip = mCodeBlock->code[ip];
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_jmpiff(U32 &ip)
{
   if (!floatStack[_FLT--])
   {
      ip++;
      return OPCodeReturn::success;
   }
   ip = mCodeBlock->code[ip];
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_jmpif(U32 &ip)
{
   if (!intStack[_UINT--])
   {
      ip++;
      return OPCodeReturn::success;
   }
   ip = mCodeBlock->code[ip];
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_jmpifnot_np(U32 &ip)
{
   if (intStack[_UINT])
   {
      _UINT--;
      ip++;
      return OPCodeReturn::success;
   }
   ip = mCodeBlock->code[ip];
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_jmpif_np(U32 &ip)
{
   if (!intStack[_UINT])
   {
      _UINT--;
      ip++;
      return OPCodeReturn::success;
   }
   ip = mCodeBlock->code[ip];
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_jmp(U32 &ip)
{
   ip = mCodeBlock->code[ip];
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_return_void(U32 &ip)
{
   STR.setStringValue("");
   // We're falling thru here on purpose.

   OPCodeReturn ret = op_return(ip);

   return ret;
}

OPCodeReturn CodeInterpreter::op_return(U32 &ip)
{
   StringStackPtr retValue = STR.getStringValuePtr();

   if (mIterDepth > 0)
   {
      // Clear iterator state.
      while (mIterDepth > 0)
      {
         iterStack[--_ITER].mIsStringIter = false;
         --mIterDepth;
      }

      STR.rewind();
      STR.setStringValue(StringStackPtrRef(retValue).getPtr(&STR)); // Not nice but works.
      retValue = STR.getStringValuePtr();
   }

   // Previously the return value was on the stack and would be returned using STR.getStringValue().
   // Now though we need to wrap it in a ConsoleValueRef 
   mReturnValue.value = CSTK.pushStringStackPtr(retValue);

   return OPCodeReturn::exitCode;
}

OPCodeReturn CodeInterpreter::op_return_flt(U32 &ip)
{
   if (mIterDepth > 0)
   {
      // Clear iterator state.
      while (mIterDepth > 0)
      {
         iterStack[--_ITER].mIsStringIter = false;
         --mIterDepth;
      }

   }

   mReturnValue.value = CSTK.pushFLT(floatStack[_FLT]);
   _FLT--;

   return OPCodeReturn::exitCode;
}

OPCodeReturn CodeInterpreter::op_return_uint(U32 &ip)
{
   if (mIterDepth > 0)
   {
      // Clear iterator state.
      while (mIterDepth > 0)
      {
         iterStack[--_ITER].mIsStringIter = false;
         --mIterDepth;
      }
   }

   mReturnValue.value = CSTK.pushUINT(intStack[_UINT]);
   _UINT--;

   return OPCodeReturn::exitCode;
}

OPCodeReturn CodeInterpreter::op_cmpeq(U32 &ip)
{
   intStack[_UINT + 1] = bool(floatStack[_FLT] == floatStack[_FLT - 1]);
   _UINT++;
   _FLT -= 2;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_cmpgr(U32 &ip)
{
   intStack[_UINT + 1] = bool(floatStack[_FLT] > floatStack[_FLT - 1]);
   _UINT++;
   _FLT -= 2;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_cmpge(U32 &ip)
{
   intStack[_UINT + 1] = bool(floatStack[_FLT] >= floatStack[_FLT - 1]);
   _UINT++;
   _FLT -= 2;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_cmplt(U32 &ip)
{
   intStack[_UINT + 1] = bool(floatStack[_FLT] < floatStack[_FLT - 1]);
   _UINT++;
   _FLT -= 2;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_cmple(U32 &ip)
{
   intStack[_UINT + 1] = bool(floatStack[_FLT] <= floatStack[_FLT - 1]);
   _UINT++;
   _FLT -= 2;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_cmpne(U32 &ip)
{
   intStack[_UINT + 1] = bool(floatStack[_FLT] != floatStack[_FLT - 1]);
   _UINT++;
   _FLT -= 2;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_xor(U32 &ip)
{
   intStack[_UINT - 1] = intStack[_UINT] ^ intStack[_UINT - 1];
   _UINT--;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_mod(U32 &ip)
{
   if (intStack[_UINT - 1] != 0)
      intStack[_UINT - 1] = intStack[_UINT] % intStack[_UINT - 1];
   else
      intStack[_UINT - 1] = 0;
   _UINT--;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_bitand(U32 &ip)
{
   intStack[_UINT - 1] = intStack[_UINT] & intStack[_UINT - 1];
   _UINT--;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_bitor(U32 &ip)
{
   intStack[_UINT - 1] = intStack[_UINT] | intStack[_UINT - 1];
   _UINT--;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_not(U32 &ip)
{
   intStack[_UINT] = !intStack[_UINT];
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_notf(U32 &ip)
{
   intStack[_UINT + 1] = !floatStack[_FLT];
   _FLT--;
   _UINT++;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_onescomplement(U32 &ip)
{
   intStack[_UINT] = ~intStack[_UINT];
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_shr(U32 &ip)
{
   intStack[_UINT - 1] = intStack[_UINT] >> intStack[_UINT - 1];
   _UINT--;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_shl(U32 &ip)
{
   intStack[_UINT - 1] = intStack[_UINT] << intStack[_UINT - 1];
   _UINT--;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_and(U32 &ip)
{
   intStack[_UINT - 1] = intStack[_UINT] && intStack[_UINT - 1];
   _UINT--;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_or(U32 &ip)
{
   intStack[_UINT - 1] = intStack[_UINT] || intStack[_UINT - 1];
   _UINT--;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_add(U32 &ip)
{
   floatStack[_FLT - 1] = floatStack[_FLT] + floatStack[_FLT - 1];
   _FLT--;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_sub(U32 &ip)
{
   floatStack[_FLT - 1] = floatStack[_FLT] - floatStack[_FLT - 1];
   _FLT--;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_mul(U32 &ip)
{
   floatStack[_FLT - 1] = floatStack[_FLT] * floatStack[_FLT - 1];
   _FLT--;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_div(U32 &ip)
{
   floatStack[_FLT - 1] = floatStack[_FLT] / floatStack[_FLT - 1];
   _FLT--;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_neg(U32 &ip)
{
   floatStack[_FLT] = -floatStack[_FLT];
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_inc(U32 &ip)
{
   StringTableEntry var = CodeToSTE(mCodeBlock->code, ip);
   ip += 2;

   // If a variable is set, then these must be NULL. It is necessary
   // to set this here so that the vector parser can appropriately
   // identify whether it's dealing with a vector.
   mPrevField = NULL;
   mPrevObject = NULL;
   mCurObject = NULL;

   gEvalState.setCurVarNameCreate(var);

   // In order to let docblocks work properly with variables, we have
   // clear the current docblock when we do an assign. This way it 
   // won't inappropriately carry forward to following function decls.
   mCurFNDocBlock = NULL;
   mCurNSDocBlock = NULL;

   F64 val = gEvalState.getFloatVariable() + 1.0;
   gEvalState.setFloatVariable(val);

   // We gotta push val onto the stack. What if we have
   // more expressions that have to use this.
   // If we don't, we send out an op code to pop it.
   floatStack[_FLT + 1] = val;
   _FLT++;

   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_dec(U32 &ip)
{
   StringTableEntry var = CodeToSTE(mCodeBlock->code, ip);
   ip += 2;

   // If a variable is set, then these must be NULL. It is necessary
   // to set this here so that the vector parser can appropriately
   // identify whether it's dealing with a vector.
   mPrevField = NULL;
   mPrevObject = NULL;
   mCurObject = NULL;

   gEvalState.setCurVarNameCreate(var);

   // In order to let docblocks work properly with variables, we have
   // clear the current docblock when we do an assign. This way it 
   // won't inappropriately carry forward to following function decls.
   mCurFNDocBlock = NULL;
   mCurNSDocBlock = NULL;

   F64 val = gEvalState.getFloatVariable() - 1.0;
   gEvalState.setFloatVariable(val);

   // We gotta push val onto the stack. What if we have
   // more expressions that have to use this.
   // If we don't, we send out an op code to pop it.
   floatStack[_FLT + 1] = val;
   _FLT++;

   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_setcurvar(U32 &ip)
{
   StringTableEntry var = CodeToSTE(mCodeBlock->code, ip);
   ip += 2;

   // If a variable is set, then these must be NULL. It is necessary
   // to set this here so that the vector parser can appropriately
   // identify whether it's dealing with a vector.
   mPrevField = NULL;
   mPrevObject = NULL;
   mCurObject = NULL;

   gEvalState.setCurVarName(var);

   // In order to let docblocks work properly with variables, we have
   // clear the current docblock when we do an assign. This way it 
   // won't inappropriately carry forward to following function decls.
   mCurFNDocBlock = NULL;
   mCurNSDocBlock = NULL;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_setcurvar_create(U32 &ip)
{
   StringTableEntry var = CodeToSTE(mCodeBlock->code, ip);
   ip += 2;

   // See OP_SETCURVAR
   mPrevField = NULL;
   mPrevObject = NULL;
   mCurObject = NULL;

   gEvalState.setCurVarNameCreate(var);

   // See OP_SETCURVAR for why we do this.
   mCurFNDocBlock = NULL;
   mCurNSDocBlock = NULL;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_setcurvar_array(U32 &ip)
{
   StringTableEntry var = STR.getSTValue();

   // See OP_SETCURVAR
   mPrevField = NULL;
   mPrevObject = NULL;
   mCurObject = NULL;

   gEvalState.setCurVarName(var);

   // See OP_SETCURVAR for why we do this.
   mCurFNDocBlock = NULL;
   mCurNSDocBlock = NULL;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_setcurvar_array_varlookup(U32 &ip)
{
   StringTableEntry arrayName = CodeToSTE(mCodeBlock->code, ip);
   StringTableEntry arrayLookup = CodeToSTE(mCodeBlock->code, ip + 2);
   ip += 4;

   STR.setStringValue(arrayName);
   STR.advance();

   // See OP_SETCURVAR
   mPrevField = NULL;
   mPrevObject = NULL;
   mCurObject = NULL;

   // resolve arrayLookup to get the 'value'
   // Note: we have to setCurVarNameCreate in case the var doesn't exist.
   // this won't cause much of a performance hit since vars are hashed.
   gEvalState.setCurVarNameCreate(arrayLookup);
   StringTableEntry hash = gEvalState.getStringVariable();

   STR.setStringValue(hash);
   STR.rewind();

   // Generate new array name.
   StringTableEntry var = STR.getSTValue();
   gEvalState.setCurVarName(var);

   // See OP_SETCURVAR for why we do this.
   mCurFNDocBlock = NULL;
   mCurNSDocBlock = NULL;

   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_setcurvar_array_create(U32 &ip)
{
   StringTableEntry var = STR.getSTValue();

   // See OP_SETCURVAR
   mPrevField = NULL;
   mPrevObject = NULL;
   mCurObject = NULL;

   gEvalState.setCurVarNameCreate(var);

   // See OP_SETCURVAR for why we do this.
   mCurFNDocBlock = NULL;
   mCurNSDocBlock = NULL;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_setcurvar_array_create_varlookup(U32 &ip)
{
   StringTableEntry arrayName = CodeToSTE(mCodeBlock->code, ip);
   StringTableEntry arrayLookup = CodeToSTE(mCodeBlock->code, ip + 2);
   ip += 4;

   // See OP_SETCURVAR
   mPrevField = NULL;
   mPrevObject = NULL;
   mCurObject = NULL;

   STR.setStringValue(arrayName);
   STR.advance();

   // resolve arrayLookup to get the 'value'
   // Note: we have to setCurVarNameCreate in case the var doesn't exist.
   // this won't cause much of a performance hit since vars are hashed.
   gEvalState.setCurVarNameCreate(arrayLookup);
   StringTableEntry hash = gEvalState.getStringVariable();

   STR.setStringValue(hash);
   STR.rewind();

   // Generate new array name.
   StringTableEntry var = STR.getSTValue();
   gEvalState.setCurVarNameCreate(var);

   // See OP_SETCURVAR for why we do this.
   mCurFNDocBlock = NULL;
   mCurNSDocBlock = NULL;

   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_loadvar_uint(U32 &ip)
{
   intStack[_UINT + 1] = gEvalState.getIntVariable();
   _UINT++;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_loadvar_flt(U32 &ip)
{
   floatStack[_FLT + 1] = gEvalState.getFloatVariable();
   _FLT++;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_loadvar_str(U32 &ip)
{
   StringTableEntry val = gEvalState.getStringVariable();
   STR.setStringValue(val);
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_loadvar_var(U32 &ip)
{
   // Sets current source of OP_SAVEVAR_VAR
   gEvalState.copyVariable = gEvalState.currentVariable;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_savevar_uint(U32 &ip)
{
   gEvalState.setIntVariable(intStack[_UINT]);
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_savevar_flt(U32 &ip)
{
   gEvalState.setFloatVariable(floatStack[_FLT]);
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_savevar_str(U32 &ip)
{
   gEvalState.setStringVariable(STR.getStringValue());
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_savevar_var(U32 &ip)
{
   // this basically handles %var1 = %var2
   gEvalState.setCopyVariable();
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_setcurobject(U32 &ip)
{
   // Save the previous object for parsing vector fields.
   mPrevObject = mCurObject;
   StringTableEntry val = STR.getStringValue();

   // Sim::findObject will sometimes find valid objects from
   // multi-component strings. This makes sure that doesn't
   // happen.
   for (const char* check = val; *check; check++)
   {
      if (*check == ' ')
      {
         val = "";
         break;
      }
   }
   mCurObject = Sim::findObject(val);
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_setcurobject_internal(U32 &ip)
{
   ++ip; // To skip the recurse flag if the object wasn't found
   if (mCurObject)
   {
      SimSet *set = dynamic_cast<SimSet *>(mCurObject);
      if (set)
      {
         StringTableEntry intName = StringTable->insert(STR.getStringValue());
         bool recurse = mCodeBlock->code[ip - 1];
         SimObject *obj = set->findObjectByInternalName(intName, recurse);
         intStack[_UINT + 1] = obj ? obj->getId() : 0;
         _UINT++;
      }
      else
      {
         Con::errorf(ConsoleLogEntry::Script, "%s: Attempt to use -> on non-set %s of class %s.", mCodeBlock->getFileLine(ip - 2), mCurObject->getName(), mCurObject->getClassName());
         intStack[_UINT] = 0;
      }
   }
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_setcurobject_new(U32 &ip)
{
   mCurObject = mCurrentNewObject;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_setcurfield(U32 &ip)
{
   // Save the previous field for parsing vector fields.
   mPrevField = mCurField;
   dStrcpy(prevFieldArray, curFieldArray);
   mCurField = CodeToSTE(mCodeBlock->code, ip);
   curFieldArray[0] = 0;
   ip += 2;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_setcurfield_array(U32 &ip)
{
   dStrcpy(curFieldArray, STR.getStringValue());
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_setcurfield_type(U32 &ip)
{
   if (mCurObject)
      mCurObject->setDataFieldType(mCodeBlock->code[ip], mCurField, curFieldArray);
   ip++;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_setcurfield_array_var(U32 &ip)
{
   StringTableEntry var = CodeToSTE(mCodeBlock->code, ip);
   ip += 2;

   // We set the current var name (create it as well in case if it doesn't exist,
   // otherwise we will crash).
   gEvalState.setCurVarNameCreate(var);

   // Then load the var and copy the contents to the current field array
   dStrncpy(curFieldArray, gEvalState.currentVariable->getStringValue(), sizeof(curFieldArray));

   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_setcurfield_this(U32 &ip)
{
   // set the 'this pointer' as the current object.
   mCurObject = mThisObject;

   mPrevField = mCurField;
   dStrcpy(prevFieldArray, curFieldArray);
   mCurField = CodeToSTE(mCodeBlock->code, ip);
   curFieldArray[0] = 0;
   ip += 2;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_loadfield_uint(U32 &ip)
{
   if (mCurObject)
      intStack[_UINT + 1] = U32(dAtoi(mCurObject->getDataField(mCurField, curFieldArray)));
   else
   {
      // The field is not being retrieved from an object. Maybe it's
      // a special accessor?
      char buff[FieldBufferSizeNumeric];
      memset(buff, 0, sizeof(buff));
      getFieldComponent(mPrevObject, mPrevField, prevFieldArray, mCurField, buff);
      intStack[_UINT + 1] = dAtoi(buff);
   }
   _UINT++;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_loadfield_flt(U32 &ip)
{
   if (mCurObject)
      floatStack[_FLT + 1] = dAtof(mCurObject->getDataField(mCurField, curFieldArray));
   else
   {
      // The field is not being retrieved from an object. Maybe it's
      // a special accessor?
      char buff[FieldBufferSizeNumeric];
      memset(buff, 0, sizeof(buff));
      getFieldComponent(mPrevObject, mPrevField, prevFieldArray, mCurField, buff);
      floatStack[_FLT + 1] = dAtof(buff);
   }
   _FLT++;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_loadfield_str(U32 &ip)
{
   if (mCurObject)
   {
      StringTableEntry val = mCurObject->getDataField(mCurField, curFieldArray);
      STR.setStringValue(val);
   }
   else
   {
      // The field is not being retrieved from an object. Maybe it's
      // a special accessor?
      char buff[FieldBufferSizeString];
      memset(buff, 0, sizeof(buff));
      getFieldComponent(mPrevObject, mPrevField, prevFieldArray, mCurField, buff);
      STR.setStringValue(buff);
   }
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_savefield_uint(U32 &ip)
{
   STR.setIntValue(intStack[_UINT]);
   if (mCurObject)
      mCurObject->setDataField(mCurField, curFieldArray, STR.getStringValue());
   else
   {
      // The field is not being set on an object. Maybe it's
      // a special accessor?
      setFieldComponent(mPrevObject, mPrevField, prevFieldArray, mCurField);
      mPrevObject = NULL;
   }
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_savefield_flt(U32 &ip)
{
   STR.setFloatValue(floatStack[_FLT]);
   if (mCurObject)
      mCurObject->setDataField(mCurField, curFieldArray, STR.getStringValue());
   else
   {
      // The field is not being set on an object. Maybe it's
      // a special accessor?
      setFieldComponent(mPrevObject, mPrevField, prevFieldArray, mCurField);
      mPrevObject = NULL;
   }
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_savefield_str(U32 &ip)
{
   if (mCurObject)
      mCurObject->setDataField(mCurField, curFieldArray, STR.getStringValue());
   else
   {
      // The field is not being set on an object. Maybe it's
      // a special accessor?
      setFieldComponent(mPrevObject, mPrevField, prevFieldArray, mCurField);
      mPrevObject = NULL;
   }
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_str_to_uint(U32 &ip)
{
   intStack[_UINT + 1] = STR.getIntValue();
   _UINT++;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_str_to_flt(U32 &ip)
{
   floatStack[_FLT + 1] = STR.getFloatValue();
   _FLT++;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_str_to_none(U32 &ip)
{
   // This exists simply to deal with certain typecast situations.
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_flt_to_uint(U32 &ip)
{
   intStack[_UINT + 1] = (S64)floatStack[_FLT];
   _FLT--;
   _UINT++;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_flt_to_str(U32 &ip)
{
   STR.setFloatValue(floatStack[_FLT]);
   _FLT--;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_flt_to_none(U32 &ip)
{
   _FLT--;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_uint_to_flt(U32 &ip)
{
   floatStack[_FLT + 1] = (F32)intStack[_UINT];
   _UINT--;
   _FLT++;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_uint_to_str(U32 &ip)
{
   STR.setIntValue(intStack[_UINT]);
   _UINT--;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_uint_to_none(U32 &ip)
{
   _UINT--;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_copyvar_to_none(U32 &ip)
{
   gEvalState.copyVariable = NULL;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_loadimmed_uint(U32 &ip)
{
   intStack[_UINT + 1] = mCodeBlock->code[ip++];
   _UINT++;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_loadimmed_flt(U32 &ip)
{
   floatStack[_FLT + 1] = mCurFloatTable[mCodeBlock->code[ip]];
   ip++;
   _FLT++;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_tag_to_str(U32 &ip)
{
   mCodeBlock->code[ip - 1] = OP_LOADIMMED_STR;
   // it's possible the string has already been converted
   if (U8(mCurStringTable[mCodeBlock->code[ip]]) != StringTagPrefixByte)
   {
      U32 id = GameAddTaggedString(mCurStringTable + mCodeBlock->code[ip]);
      dSprintf(mCurStringTable + mCodeBlock->code[ip] + 1, 7, "%d", id);
      *(mCurStringTable + mCodeBlock->code[ip]) = StringTagPrefixByte;
   }

   // Fallthrough
   OPCodeReturn ret = op_loadimmed_str(ip);

   return ret;
}

OPCodeReturn CodeInterpreter::op_loadimmed_str(U32 &ip)
{
   STR.setStringValue(mCurStringTable + mCodeBlock->code[ip++]);
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_docblock_str(U32 &ip)
{
   // If the first word of the doc is '\class' or '@class', then this
   // is a namespace doc block, otherwise it is a function doc block.
   const char* docblock = mCurStringTable + mCodeBlock->code[ip++];

   const char* sansClass = dStrstr(docblock, "@class");
   if (!sansClass)
      sansClass = dStrstr(docblock, "\\class");

   if (sansClass)
   {
      // Don't save the class declaration. Scan past the 'class'
      // keyword and up to the first whitespace.
      sansClass += 7;
      S32 index = 0;
      while ((*sansClass != ' ') && (*sansClass != '\n') && *sansClass && (index < (nsDocLength - 1)))
      {
         mNSDocBlockClass[index++] = *sansClass;
         sansClass++;
      }
      mNSDocBlockClass[index] = '\0';

      mCurNSDocBlock = sansClass + 1;
   }
   else
      mCurFNDocBlock = docblock;

   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_loadimmed_ident(U32 &ip)
{
   STR.setStringValue(CodeToSTE(mCodeBlock->code, ip));
   ip += 2;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_callfunc_resolve(U32 &ip)
{
   // This deals with a function that is potentially living in a namespace.
   StringTableEntry fnNamespace = CodeToSTE(mCodeBlock->code, ip + 2);
   StringTableEntry fnName = CodeToSTE(mCodeBlock->code, ip);

   // Try to look it up.
   mNSEntry = Namespace::find(fnNamespace)->lookup(fnName);
   if (!mNSEntry)
   {
      ip += 5;
      Con::warnf(ConsoleLogEntry::General,
         "%s: Unable to find function %s%s%s",
         mCodeBlock->getFileLine(ip - 7), fnNamespace ? fnNamespace : "",
         fnNamespace ? "::" : "", fnName);
      STR.popFrame();
      CSTK.popFrame();
      return OPCodeReturn::success;
   }

   // Fallthrough to op_callfunc_resolve
   OPCodeReturn ret = op_callfunc(ip);

   return ret;
}

OPCodeReturn CodeInterpreter::op_callfunc(U32 &ip)
{
   // This routingId is set when we query the object as to whether
   // it handles this method.  It is set to an enum from the table
   // above indicating whether it handles it on a component it owns
   // or just on the object.
   S32 routingId = 0;

   U32 *code = mCodeBlock->code;

   StringTableEntry fnName = CodeToSTE(code, ip);

   //if this is called from inside a function, append the ip and codeptr
   if (gEvalState.getStackDepth() > 0)
   {
      gEvalState.getCurrentFrame().code = mCodeBlock;
      gEvalState.getCurrentFrame().ip = ip - 1;
   }

   U32 callType = code[ip + 4];

   ip += 5;
   CSTK.getArgcArgv(fnName, &mCallArgc, &mCallArgv);

   const char *componentReturnValue = "";
   Namespace *ns = NULL;

   if (callType == FuncCallExprNode::FunctionCall)
   {
      if (!mNSEntry)
         mNSEntry = Namespace::global()->lookup(fnName);
   }
   else if (callType == FuncCallExprNode::MethodCall)
   {
      mSaveObject = gEvalState.thisObject;
      gEvalState.thisObject = Sim::findObject((const char*)mCallArgv[1]);
      if (!gEvalState.thisObject)
      {
         // Go back to the previous saved object.
         gEvalState.thisObject = mSaveObject;

         Con::warnf(ConsoleLogEntry::General, "%s: Unable to find object: '%s' attempting to call function '%s'", mCodeBlock->getFileLine(ip - 4), (const char*)mCallArgv[1], fnName);
         STR.popFrame();
         CSTK.popFrame();
         STR.setStringValue("");
         return OPCodeReturn::success;
      }

      bool handlesMethod = gEvalState.thisObject->handlesConsoleMethod(fnName, &routingId);
      if (handlesMethod && routingId == MethodOnComponent)
      {
         ICallMethod *pComponent = dynamic_cast<ICallMethod *>(gEvalState.thisObject);
         if (pComponent)
            componentReturnValue = pComponent->callMethodArgList(mCallArgc, mCallArgv, false);
      }

      ns = gEvalState.thisObject->getNamespace();
      if (ns)
         mNSEntry = ns->lookup(fnName);
      else
         mNSEntry = NULL;
   }
   else // it's a ParentCall
   {
      if (mExec.thisNamespace)
      {
         ns = mExec.thisNamespace->mParent;
         if (ns)
            mNSEntry = ns->lookup(fnName);
         else
            mNSEntry = NULL;
      }
      else
      {
         ns = NULL;
         mNSEntry = NULL;
      }
   }

   Namespace::Entry::CallbackUnion * nsCb = NULL;
   const char * nsUsage = NULL;
   if (mNSEntry)
   {
      nsCb = &mNSEntry->cb;
      nsUsage = mNSEntry->mUsage;
      routingId = 0;
   }
   if (!mNSEntry || mExec.noCalls)
   {
      if (!mExec.noCalls && !(routingId == MethodOnComponent))
      {
         Con::warnf(ConsoleLogEntry::General, "%s: Unknown command %s.", mCodeBlock->getFileLine(ip - 6), fnName);
         if (callType == FuncCallExprNode::MethodCall)
         {
            Con::warnf(ConsoleLogEntry::General, "  Object %s(%d) %s",
               gEvalState.thisObject->getName() ? gEvalState.thisObject->getName() : "",
               gEvalState.thisObject->getId(), Con::getNamespaceList(ns));
         }
      }
      STR.popFrame();
      CSTK.popFrame();

      if (routingId == MethodOnComponent)
         STR.setStringValue(componentReturnValue);
      else
         STR.setStringValue("");
      return OPCodeReturn::success;
   }

   // ConsoleFunctionType is for any function defined by script.
   // Any 'callback' type is an engine function that is exposed to script.
   if (mNSEntry->mType == Namespace::Entry::ConsoleFunctionType)
   {
      ConsoleValueRef ret;
      if (mNSEntry->mFunctionOffset)
         ret = mNSEntry->mCode->exec(mNSEntry->mFunctionOffset, fnName, mNSEntry->mNamespace, mCallArgc, mCallArgv, false, mNSEntry->mPackage);

      STR.popFrame();
      // Functions are assumed to return strings, so look ahead to see if we can skip the conversion
      if (code[ip] == OP_STR_TO_UINT)
      {
         ip++;
         intStack[++_UINT] = (U32)((S32)ret);
      }
      else if (code[ip] == OP_STR_TO_FLT)
      {
         ip++;
         floatStack[++_FLT] = (F32)ret;
      }
      else if (code[ip] == OP_STR_TO_NONE)
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
      const char* nsName = ns ? ns->mName : "";
#ifndef TORQUE_DEBUG
      // [tom, 12/13/2006] This stops tools functions from working in the console,
      // which is useful behavior when debugging so I'm ifdefing this out for debug builds.
      if (mNSEntry->mToolOnly && !Con::isCurrentScriptToolScript())
      {
         Con::errorf(ConsoleLogEntry::Script, "%s: %s::%s - attempting to call tools only function from outside of tools.", mCodeBlock->getFileLine(ip - 6), nsName, fnName);
      }
      else
#endif
      if ((mNSEntry->mMinArgs && S32(mCallArgc) < mNSEntry->mMinArgs) || (mNSEntry->mMaxArgs && S32(mCallArgc) > mNSEntry->mMaxArgs))
      {
         Con::warnf(ConsoleLogEntry::Script, "%s: %s::%s - wrong number of arguments (got %i, expected min %i and max %i).",
            mCodeBlock->getFileLine(ip - 6), nsName, fnName,
            mCallArgc, mNSEntry->mMinArgs, mNSEntry->mMaxArgs);
         Con::warnf(ConsoleLogEntry::Script, "%s: usage: %s", mCodeBlock->getFileLine(ip - 6), mNSEntry->mUsage);
         STR.popFrame();
         CSTK.popFrame();
      }
      else
      {
         switch (mNSEntry->mType)
         {
         case Namespace::Entry::StringCallbackType:
         {
            const char *ret = mNSEntry->cb.mStringCallbackFunc(gEvalState.thisObject, mCallArgc, mCallArgv);
            STR.popFrame();
            CSTK.popFrame();
            if (ret != STR.getStringValue())
               STR.setStringValue(ret);
            //else
            //   sSTR.setLen(dStrlen(ret));
            break;
         }
         case Namespace::Entry::IntCallbackType:
         {
            S32 result = mNSEntry->cb.mIntCallbackFunc(gEvalState.thisObject, mCallArgc, mCallArgv);
            STR.popFrame();
            CSTK.popFrame();
            if (code[ip] == OP_STR_TO_UINT)
            {
               ip++;
               intStack[++_UINT] = result;
               break;
            }
            else if (code[ip] == OP_STR_TO_FLT)
            {
               ip++;
               floatStack[++_FLT] = result;
               break;
            }
            else if (code[ip] == OP_STR_TO_NONE)
               ip++;
            else
               STR.setIntValue(result);
            break;
         }
         case Namespace::Entry::FloatCallbackType:
         {
            F64 result = mNSEntry->cb.mFloatCallbackFunc(gEvalState.thisObject, mCallArgc, mCallArgv);
            STR.popFrame();
            CSTK.popFrame();
            if (code[ip] == OP_STR_TO_UINT)
            {
               ip++;
               intStack[++_UINT] = (S64)result;
               break;
            }
            else if (code[ip] == OP_STR_TO_FLT)
            {
               ip++;
               floatStack[++_FLT] = result;
               break;
            }
            else if (code[ip] == OP_STR_TO_NONE)
               ip++;
            else
               STR.setFloatValue(result);
            break;
         }
         case Namespace::Entry::VoidCallbackType:
            mNSEntry->cb.mVoidCallbackFunc(gEvalState.thisObject, mCallArgc, mCallArgv);
            if (code[ip] != OP_STR_TO_NONE && Con::getBoolVariable("$Con::warnVoidAssignment", true))
               Con::warnf(ConsoleLogEntry::General, "%s: Call to %s in %s uses result of void function call.", mCodeBlock->getFileLine(ip - 6), fnName, mExec.functionName);

            STR.popFrame();
            CSTK.popFrame();
            STR.setStringValue("");
            break;
         case Namespace::Entry::BoolCallbackType:
         {
            bool result = mNSEntry->cb.mBoolCallbackFunc(gEvalState.thisObject, mCallArgc, mCallArgv);
            STR.popFrame();
            CSTK.popFrame();
            if (code[ip] == OP_STR_TO_UINT)
            {
               ip++;
               intStack[++_UINT] = result;
               break;
            }
            else if (code[ip] == OP_STR_TO_FLT)
            {
               ip++;
               floatStack[++_FLT] = result;
               break;
            }
            else if (code[ip] == OP_STR_TO_NONE)
               ip++;
            else
               STR.setIntValue(result);
            break;
         }
         }
      }
   }

   if (callType == FuncCallExprNode::MethodCall)
      gEvalState.thisObject = mSaveObject;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_callfunc_pointer(U32 &ip)
{
   // get function name. This is the 'function pointer'.
   StringTableEntry fnName = StringTable->insert(STR.getStringValue());

   U32 *code = mCodeBlock->code;

   mNSEntry = Namespace::global()->lookup(fnName);

   //if this is called from inside a function, append the ip and codeptr
   if (gEvalState.getStackDepth() > 0)
   {
      gEvalState.getCurrentFrame().code = mCodeBlock;
      gEvalState.getCurrentFrame().ip = ip - 1;
   }

   CSTK.getArgcArgv(fnName, &mCallArgc, &mCallArgv);


   if (!mNSEntry || mExec.noCalls)
   {
      if (!mExec.noCalls)
      {
         Con::warnf(ConsoleLogEntry::General, "%s: Unknown command %s.", mCodeBlock->getFileLine(ip - 6), fnName);
      }
      STR.popFrame();
      CSTK.popFrame();

      STR.setStringValue("");
      return OPCodeReturn::success;
   }

   // ConsoleFunctionType is for any function defined by script.
   // Any 'callback' type is an engine function that is exposed to script.
   if (mNSEntry->mType == Namespace::Entry::ConsoleFunctionType)
   {
      ConsoleValueRef ret;
      if (mNSEntry->mFunctionOffset)
         ret = mNSEntry->mCode->exec(mNSEntry->mFunctionOffset, fnName, mNSEntry->mNamespace, mCallArgc, mCallArgv, false, mNSEntry->mPackage);

      STR.popFrame();
      // Functions are assumed to return strings, so look ahead to see if we can skip the conversion
      if (code[ip] == OP_STR_TO_UINT)
      {
         ip++;
         intStack[++_UINT] = (U32)((S32)ret);
      }
      else if (code[ip] == OP_STR_TO_FLT)
      {
         ip++;
         floatStack[++_FLT] = (F32)ret;
      }
      else if (code[ip] == OP_STR_TO_NONE)
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
      const char* nsName = "";

      Namespace::Entry::CallbackUnion * nsCb = &mNSEntry->cb;
      const char * nsUsage = mNSEntry->mUsage;

#ifndef TORQUE_DEBUG
      // [tom, 12/13/2006] This stops tools functions from working in the console,
      // which is useful behavior when debugging so I'm ifdefing this out for debug builds.
      if (mNSEntry->mToolOnly && !Con::isCurrentScriptToolScript())
      {
         Con::errorf(ConsoleLogEntry::Script, "%s: %s::%s - attempting to call tools only function from outside of tools.", mCodeBlock->getFileLine(ip - 6), nsName, fnName);
      }
      else
#endif
         if ((mNSEntry->mMinArgs && S32(mCallArgc) < mNSEntry->mMinArgs) || (mNSEntry->mMaxArgs && S32(mCallArgc) > mNSEntry->mMaxArgs))
         {
            Con::warnf(ConsoleLogEntry::Script, "%s: %s::%s - wrong number of arguments (got %i, expected min %i and max %i).",
               mCodeBlock->getFileLine(ip - 6), nsName, fnName,
               mCallArgc, mNSEntry->mMinArgs, mNSEntry->mMaxArgs);
            Con::warnf(ConsoleLogEntry::Script, "%s: usage: %s", mCodeBlock->getFileLine(ip - 6), mNSEntry->mUsage);
            STR.popFrame();
            CSTK.popFrame();
         }
         else
         {
            switch (mNSEntry->mType)
            {
            case Namespace::Entry::StringCallbackType:
            {
               const char *ret = mNSEntry->cb.mStringCallbackFunc(gEvalState.thisObject, mCallArgc, mCallArgv);
               STR.popFrame();
               CSTK.popFrame();
               if (ret != STR.getStringValue())
                  STR.setStringValue(ret);
               //else
               //   sSTR.setLen(dStrlen(ret));
               break;
            }
            case Namespace::Entry::IntCallbackType:
            {
               S32 result = mNSEntry->cb.mIntCallbackFunc(gEvalState.thisObject, mCallArgc, mCallArgv);
               STR.popFrame();
               CSTK.popFrame();
               if (code[ip] == OP_STR_TO_UINT)
               {
                  ip++;
                  intStack[++_UINT] = result;
                  break;
               }
               else if (code[ip] == OP_STR_TO_FLT)
               {
                  ip++;
                  floatStack[++_FLT] = result;
                  break;
               }
               else if (code[ip] == OP_STR_TO_NONE)
                  ip++;
               else
                  STR.setIntValue(result);
               break;
            }
            case Namespace::Entry::FloatCallbackType:
            {
               F64 result = mNSEntry->cb.mFloatCallbackFunc(gEvalState.thisObject, mCallArgc, mCallArgv);
               STR.popFrame();
               CSTK.popFrame();
               if (code[ip] == OP_STR_TO_UINT)
               {
                  ip++;
                  intStack[++_UINT] = (S64)result;
                  break;
               }
               else if (code[ip] == OP_STR_TO_FLT)
               {
                  ip++;
                  floatStack[++_FLT] = result;
                  break;
               }
               else if (code[ip] == OP_STR_TO_NONE)
                  ip++;
               else
                  STR.setFloatValue(result);
               break;
            }
            case Namespace::Entry::VoidCallbackType:
               mNSEntry->cb.mVoidCallbackFunc(gEvalState.thisObject, mCallArgc, mCallArgv);
               if (code[ip] != OP_STR_TO_NONE && Con::getBoolVariable("$Con::warnVoidAssignment", true))
                  Con::warnf(ConsoleLogEntry::General, "%s: Call to %s in %s uses result of void function call.", mCodeBlock->getFileLine(ip - 6), fnName, mExec.functionName);

               STR.popFrame();
               CSTK.popFrame();
               STR.setStringValue("");
               break;
            case Namespace::Entry::BoolCallbackType:
            {
               bool result = mNSEntry->cb.mBoolCallbackFunc(gEvalState.thisObject, mCallArgc, mCallArgv);
               STR.popFrame();
               CSTK.popFrame();
               if (code[ip] == OP_STR_TO_UINT)
               {
                  ip++;
                  intStack[++_UINT] = result;
                  break;
               }
               else if (code[ip] == OP_STR_TO_FLT)
               {
                  ip++;
                  floatStack[++_FLT] = result;
                  break;
               }
               else if (code[ip] == OP_STR_TO_NONE)
                  ip++;
               else
                  STR.setIntValue(result);
               break;
            }
            }
         }
   }


   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_callfunc_this(U32 &ip)
{
   U32 *code = mCodeBlock->code;

   StringTableEntry fnName = CodeToSTE(code, ip);

   //if this is called from inside a function, append the ip and codeptr
   if (gEvalState.getStackDepth() > 0)
   {
      gEvalState.getCurrentFrame().code = mCodeBlock;
      gEvalState.getCurrentFrame().ip = ip - 1;
   }

   ip += 2;
   CSTK.getArgcArgv(fnName, &mCallArgc, &mCallArgv);

   Namespace *ns = mThisObject->getNamespace();
   if (ns)
      mNSEntry = ns->lookup(fnName);
   else
      mNSEntry = NULL;

   if (!mNSEntry || mExec.noCalls)
   {
      if (!mExec.noCalls)
      {
         Con::warnf(ConsoleLogEntry::General, "%s: Unknown command %s.", mCodeBlock->getFileLine(ip - 6), fnName);
         Con::warnf(ConsoleLogEntry::General, "  Object %s(%d) %s",
            mThisObject->getName() ? mThisObject->getName() : "",
            mThisObject->getId(), Con::getNamespaceList(ns));
      }
      STR.popFrame();
      CSTK.popFrame();

      STR.setStringValue("");
      return OPCodeReturn::success;
   }

   // ConsoleFunctionType is for any function defined by script.
   // Any 'callback' type is an engine function that is exposed to script.
   if (mNSEntry->mType == Namespace::Entry::ConsoleFunctionType)
   {
      ConsoleValueRef ret;
      if (mNSEntry->mFunctionOffset)
         ret = mNSEntry->mCode->exec(mNSEntry->mFunctionOffset, fnName, mNSEntry->mNamespace, mCallArgc, mCallArgv, false, mNSEntry->mPackage);

      STR.popFrame();
      // Functions are assumed to return strings, so look ahead to see if we can skip the conversion
      if (code[ip] == OP_STR_TO_UINT)
      {
         ip++;
         intStack[++_UINT] = (U32)((S32)ret);
      }
      else if (code[ip] == OP_STR_TO_FLT)
      {
         ip++;
         floatStack[++_FLT] = (F32)ret;
      }
      else if (code[ip] == OP_STR_TO_NONE)
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
      Namespace::Entry::CallbackUnion * nsCb = &mNSEntry->cb;
      const char * nsUsage = mNSEntry->mUsage;
      const char* nsName = ns ? ns->mName : "";
#ifndef TORQUE_DEBUG
      // [tom, 12/13/2006] This stops tools functions from working in the console,
      // which is useful behavior when debugging so I'm ifdefing this out for debug builds.
      if (mNSEntry->mToolOnly && !Con::isCurrentScriptToolScript())
      {
         Con::errorf(ConsoleLogEntry::Script, "%s: %s::%s - attempting to call tools only function from outside of tools.", mCodeBlock->getFileLine(ip - 6), nsName, fnName);
      }
      else
#endif
         if ((mNSEntry->mMinArgs && S32(mCallArgc) < mNSEntry->mMinArgs) || (mNSEntry->mMaxArgs && S32(mCallArgc) > mNSEntry->mMaxArgs))
         {
            Con::warnf(ConsoleLogEntry::Script, "%s: %s::%s - wrong number of arguments (got %i, expected min %i and max %i).",
               mCodeBlock->getFileLine(ip - 6), nsName, fnName,
               mCallArgc, mNSEntry->mMinArgs, mNSEntry->mMaxArgs);
            Con::warnf(ConsoleLogEntry::Script, "%s: usage: %s", mCodeBlock->getFileLine(ip - 6), mNSEntry->mUsage);
            STR.popFrame();
            CSTK.popFrame();
         }
         else
         {
            switch (mNSEntry->mType)
            {
            case Namespace::Entry::StringCallbackType:
            {
               const char *ret = mNSEntry->cb.mStringCallbackFunc(mThisObject, mCallArgc, mCallArgv);
               STR.popFrame();
               CSTK.popFrame();
               if (ret != STR.getStringValue())
                  STR.setStringValue(ret);
               //else
               //   sSTR.setLen(dStrlen(ret));
               break;
            }
            case Namespace::Entry::IntCallbackType:
            {
               S32 result = mNSEntry->cb.mIntCallbackFunc(mThisObject, mCallArgc, mCallArgv);
               STR.popFrame();
               CSTK.popFrame();
               if (code[ip] == OP_STR_TO_UINT)
               {
                  ip++;
                  intStack[++_UINT] = result;
                  break;
               }
               else if (code[ip] == OP_STR_TO_FLT)
               {
                  ip++;
                  floatStack[++_FLT] = result;
                  break;
               }
               else if (code[ip] == OP_STR_TO_NONE)
                  ip++;
               else
                  STR.setIntValue(result);
               break;
            }
            case Namespace::Entry::FloatCallbackType:
            {
               F64 result = mNSEntry->cb.mFloatCallbackFunc(mThisObject, mCallArgc, mCallArgv);
               STR.popFrame();
               CSTK.popFrame();
               if (code[ip] == OP_STR_TO_UINT)
               {
                  ip++;
                  intStack[++_UINT] = (S64)result;
                  break;
               }
               else if (code[ip] == OP_STR_TO_FLT)
               {
                  ip++;
                  floatStack[++_FLT] = result;
                  break;
               }
               else if (code[ip] == OP_STR_TO_NONE)
                  ip++;
               else
                  STR.setFloatValue(result);
               break;
            }
            case Namespace::Entry::VoidCallbackType:
               mNSEntry->cb.mVoidCallbackFunc(mThisObject, mCallArgc, mCallArgv);
               if (code[ip] != OP_STR_TO_NONE && Con::getBoolVariable("$Con::warnVoidAssignment", true))
                  Con::warnf(ConsoleLogEntry::General, "%s: Call to %s in %s uses result of void function call.", mCodeBlock->getFileLine(ip - 6), fnName, mExec.functionName);

               STR.popFrame();
               CSTK.popFrame();
               STR.setStringValue("");
               break;
            case Namespace::Entry::BoolCallbackType:
            {
               bool result = mNSEntry->cb.mBoolCallbackFunc(mThisObject, mCallArgc, mCallArgv);
               STR.popFrame();
               CSTK.popFrame();
               if (code[ip] == OP_STR_TO_UINT)
               {
                  ip++;
                  intStack[++_UINT] = result;
                  break;
               }
               else if (code[ip] == OP_STR_TO_FLT)
               {
                  ip++;
                  floatStack[++_FLT] = result;
                  break;
               }
               else if (code[ip] == OP_STR_TO_NONE)
                  ip++;
               else
                  STR.setIntValue(result);
               break;
            }
            }
         }
   }

   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_advance_str(U32 &ip)
{
   STR.advance();
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_advance_str_appendchar(U32 &ip)
{
   STR.advanceChar(mCodeBlock->code[ip++]);
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_advance_str_comma(U32 &ip)
{
   STR.advanceChar('_');
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_advance_str_nul(U32 &ip)
{
   STR.advanceChar(0);
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_rewind_str(U32 &ip)
{
   STR.rewind();
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_terminate_rewind_str(U32 &ip)
{
   STR.rewindTerminate();
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_compare_str(U32 &ip)
{
   intStack[++_UINT] = STR.compare();
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_push(U32 &ip)
{
   STR.push();
   CSTK.pushStringStackPtr(STR.getPreviousStringValuePtr());
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_push_uint(U32 &ip)
{
   CSTK.pushUINT(intStack[_UINT]);
   _UINT--;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_push_flt(U32 &ip)
{
   CSTK.pushFLT(floatStack[_FLT]);
   _FLT--;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_push_var(U32 &ip)
{
   if (gEvalState.currentVariable)
      CSTK.pushValue(gEvalState.currentVariable->value);
   else
      CSTK.pushString("");
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_push_this(U32 &ip)
{
   StringTableEntry varName = CodeToSTE(mCodeBlock->code, ip);
   ip += 2;

   // shorthand OP_SETCURVAR

   // If a variable is set, then these must be NULL. It is necessary
   // to set this here so that the vector parser can appropriately
   // identify whether it's dealing with a vector.
   mPrevField = NULL;
   mPrevObject = NULL;
   mCurObject = NULL;

   gEvalState.setCurVarName(varName);

   // In order to let docblocks work properly with variables, we have
   // clear the current docblock when we do an assign. This way it 
   // won't inappropriately carry forward to following function decls.
   mCurFNDocBlock = NULL;
   mCurNSDocBlock = NULL;

   // shorthand OP_LOADVAR_STR (since objs can be by name we can't assume uint)
   STR.setStringValue(gEvalState.getStringVariable());

   // shorthand OP_PUSH
   STR.push();
   CSTK.pushStringStackPtr(STR.getPreviousStringValuePtr());

   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_push_frame(U32 &ip)
{
   STR.pushFrame();
   CSTK.pushFrame();
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_assert(U32 &ip)
{
   if (!intStack[_UINT--])
   {
      const char *message = mCurStringTable + mCodeBlock->code[ip];

      U32 breakLine, inst;
      mCodeBlock->findBreakLine(ip - 1, breakLine, inst);

      if (PlatformAssert::processAssert(PlatformAssert::Fatal,
         mCodeBlock->name ? mCodeBlock->name : "eval",
         breakLine,
         message))
      {
         if (TelDebugger && TelDebugger->isConnected() && breakLine > 0)
         {
            TelDebugger->breakProcess();
         }
         else
            Platform::debugBreak();
      }
   }

   ip++;

   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_break(U32 &ip)
{
   //append the ip and codeptr before managing the breakpoint!
   AssertFatal(gEvalState.getStackDepth() > 0, "Empty eval stack on break!");
   gEvalState.getCurrentFrame().code = mCodeBlock;
   gEvalState.getCurrentFrame().ip = ip - 1;

   U32 breakLine;
   mCodeBlock->findBreakLine(ip - 1, breakLine, mCurrentInstruction);
   if (!breakLine)
      return OPCodeReturn::breakContinue;
   TelDebugger->executionStopped(mCodeBlock, breakLine);
   return OPCodeReturn::breakContinue;
}

OPCodeReturn CodeInterpreter::op_iter_begin_str(U32 &ip)
{
   iterStack[_ITER].mIsStringIter = true;

   // Emulate fallthrough:
   OPCodeReturn fallthrough = op_iter_begin(ip);

   return fallthrough;
}

OPCodeReturn CodeInterpreter::op_iter_begin(U32 &ip)
{
   StringTableEntry varName = CodeToSTE(mCodeBlock->code, ip);
   U32 failIp = mCodeBlock->code[ip + 2];

   IterStackRecord& iter = iterStack[_ITER];

   iter.mVariable = gEvalState.getCurrentFrame().add(varName);

   if (iter.mIsStringIter)
   {
      iter.mData.mStr.mString = STR.getStringValuePtr();
      iter.mData.mStr.mIndex = 0;
   }
   else
   {
      // Look up the object.

      SimSet* set;
      if (!Sim::findObject(STR.getStringValue(), set))
      {
         Con::errorf(ConsoleLogEntry::General, "No SimSet object '%s'", STR.getStringValue());
         Con::errorf(ConsoleLogEntry::General, "Did you mean to use 'foreach$' instead of 'foreach'?");
         ip = failIp;
         return OPCodeReturn::success;
      }

      // Set up.

      iter.mData.mObj.mSet = set;
      iter.mData.mObj.mIndex = 0;
   }

   _ITER++;
   mIterDepth++;

   STR.push();

   ip += 3;

   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_iter(U32 &ip)
{
   U32 breakIp = mCodeBlock->code[ip];
   IterStackRecord& iter = iterStack[_ITER - 1];

   if (iter.mIsStringIter)
   {
      const char* str = StringStackPtrRef(iter.mData.mStr.mString).getPtr(&STR);

      U32 startIndex = iter.mData.mStr.mIndex;
      U32 endIndex = startIndex;

      // Break if at end.

      if (!str[startIndex])
      {
         ip = breakIp;
         return OPCodeReturn::success; // continue in old interpreter
      }

      // Find right end of current component.

      if (!dIsspace(str[endIndex]))
         do ++endIndex;
      while (str[endIndex] && !dIsspace(str[endIndex]));

      // Extract component.

      if (endIndex != startIndex)
      {
         char savedChar = str[endIndex];
         const_cast< char* >(str)[endIndex] = '\0'; // We are on the string stack so this is okay.
         iter.mVariable->setStringValue(&str[startIndex]);
         const_cast< char* >(str)[endIndex] = savedChar;
      }
      else
         iter.mVariable->setStringValue("");

      // Skip separator.
      if (str[endIndex] != '\0')
         ++endIndex;

      iter.mData.mStr.mIndex = endIndex;
   }
   else
   {
      U32 index = iter.mData.mObj.mIndex;
      SimSet* set = iter.mData.mObj.mSet;

      if (index >= set->size())
      {
         ip = breakIp;
         return OPCodeReturn::success; // continue in old interpreter
      }

      iter.mVariable->setIntValue(set->at(index)->getId());
      iter.mData.mObj.mIndex = index + 1;
   }

   ++ip;

   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_iter_end(U32 &ip)
{
   --_ITER;
   --mIterDepth;
   STR.rewind();
   iterStack[_ITER].mIsStringIter = false;
   return OPCodeReturn::success;
}

OPCodeReturn CodeInterpreter::op_invalid(U32 &ip)
{
   // Invalid does nothing.
   return OPCodeReturn::exitCode;
}
