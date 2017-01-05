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


#ifndef _COMPILER_H_
#define _COMPILER_H_

//#define DEBUG_CODESTREAM

#ifdef DEBUG_CODESTREAM
#include <stdio.h>
#endif

class Stream;
class DataChunker;

#include "platform/platform.h"
#include "console/ast.h"
#include "console/codeBlock.h"

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif

namespace Compiler
{
   /// The opcodes for the TorqueScript VM.
   enum CompiledInstructions
   {
      OP_FUNC_DECL,
      OP_CREATE_OBJECT,
      OP_ADD_OBJECT,
      OP_END_OBJECT,
      // Added to fix the stack issue [7/9/2007 Black]
      OP_FINISH_OBJECT,

      OP_JMPIFFNOT,
      OP_JMPIFNOT,
      OP_JMPIFF,
      OP_JMPIF,
      OP_JMPIFNOT_NP,
      OP_JMPIF_NP,    // 10
      OP_JMP,
      OP_RETURN,
      // fixes a bug when not explicitly returning a value
      OP_RETURN_VOID,
      OP_RETURN_FLT,
      OP_RETURN_UINT,

      OP_CMPEQ,
      OP_CMPGR,
      OP_CMPGE,
      OP_CMPLT,
      OP_CMPLE,
      OP_CMPNE,
      OP_XOR,         // 20
      OP_MOD,
      OP_BITAND,
      OP_BITOR,
      OP_NOT,
      OP_NOTF,
      OP_ONESCOMPLEMENT,

      OP_SHR,
      OP_SHL,
      OP_AND,
      OP_OR,          // 30

      OP_ADD,
      OP_SUB,
      OP_MUL,
      OP_DIV,
      OP_NEG,

      OP_SETCURVAR,
      OP_SETCURVAR_CREATE,
      OP_SETCURVAR_ARRAY,
      OP_SETCURVAR_ARRAY_CREATE,

      OP_LOADVAR_UINT,// 40
      OP_LOADVAR_FLT,
      OP_LOADVAR_STR,
      OP_LOADVAR_VAR,

      OP_SAVEVAR_UINT,
      OP_SAVEVAR_FLT,
      OP_SAVEVAR_STR,
      OP_SAVEVAR_VAR,

      OP_SETCUROBJECT,
      OP_SETCUROBJECT_NEW,
      OP_SETCUROBJECT_INTERNAL,

      OP_SETCURFIELD,
      OP_SETCURFIELD_ARRAY, // 50
      OP_SETCURFIELD_TYPE,

      OP_LOADFIELD_UINT,
      OP_LOADFIELD_FLT,
      OP_LOADFIELD_STR,

      OP_SAVEFIELD_UINT,
      OP_SAVEFIELD_FLT,
      OP_SAVEFIELD_STR,

      OP_STR_TO_UINT,
      OP_STR_TO_FLT,
      OP_STR_TO_NONE,  // 60
      OP_FLT_TO_UINT,
      OP_FLT_TO_STR,
      OP_FLT_TO_NONE,
      OP_UINT_TO_FLT,
      OP_UINT_TO_STR,
      OP_UINT_TO_NONE,
      OP_COPYVAR_TO_NONE,

      OP_LOADIMMED_UINT,
      OP_LOADIMMED_FLT,
      OP_TAG_TO_STR,
      OP_LOADIMMED_STR, // 70
      OP_DOCBLOCK_STR,
      OP_LOADIMMED_IDENT,

      OP_CALLFUNC_RESOLVE,
      OP_CALLFUNC,

      OP_ADVANCE_STR,
      OP_ADVANCE_STR_APPENDCHAR,
      OP_ADVANCE_STR_COMMA,
      OP_ADVANCE_STR_NUL,
      OP_REWIND_STR,
      OP_TERMINATE_REWIND_STR,  // 80
      OP_COMPARE_STR,

      OP_PUSH,          // String
      OP_PUSH_UINT,     // Integer
      OP_PUSH_FLT,      // Float
      OP_PUSH_VAR,      // Variable
      OP_PUSH_FRAME,    // Frame

      OP_ASSERT,
      OP_BREAK,
      
      OP_ITER_BEGIN,       ///< Prepare foreach iterator.
      OP_ITER_BEGIN_STR,   ///< Prepare foreach$ iterator.
      OP_ITER,             ///< Enter foreach loop.
      OP_ITER_END,         ///< End foreach loop.

      OP_INVALID   // 90
   };

   //------------------------------------------------------------

   F64 consoleStringToNumber(const char *str, StringTableEntry file = 0, U32 line = 0);
   
   U32 compileBlock(StmtNode *block, CodeStream &codeStream, U32 ip);

   //------------------------------------------------------------

   struct CompilerIdentTable
   {
      struct Entry
      {
         U32 offset;
         U32 ip;
         Entry *next;
         Entry *nextIdent;
      };
      Entry *list;
      void add(StringTableEntry ste, U32 ip);
      void reset();
      void write(Stream &st);
   };

   //------------------------------------------------------------

   struct CompilerStringTable
   {
      U32 totalLen;
      struct Entry
      {
         char *string;
         U32 start;
         U32 len;
         bool tag;
         Entry *next;
      };
      Entry *list;

      char buf[256];

      U32 add(const char *str, bool caseSens = true, bool tag = false);
      U32 addIntString(U32 value);
      U32 addFloatString(F64 value);
      void reset();
      char *build();
      void write(Stream &st);
   };

   //------------------------------------------------------------

   struct CompilerFloatTable
   {
      struct Entry
      {
         F64 val;
         Entry *next;
      };
      U32 count;
      Entry *list;

      U32 add(F64 value);
      void reset();
      F64 *build();
      void write(Stream &st);
   };

   //------------------------------------------------------------

   inline StringTableEntry CodeToSTE(U32 *code, U32 ip)
   {
#ifdef TORQUE_CPU_X64
      return (StringTableEntry)(*((U64*)(code+ip)));
#else
      return (StringTableEntry)(*(code+ip));
#endif
   }

   extern void (*STEtoCode)(StringTableEntry ste, U32 ip, U32 *ptr);
   
   void evalSTEtoCode(StringTableEntry ste, U32 ip, U32 *ptr);
   void compileSTEtoCode(StringTableEntry ste, U32 ip, U32 *ptr);

   CompilerStringTable *getCurrentStringTable();
   CompilerStringTable &getGlobalStringTable();
   CompilerStringTable &getFunctionStringTable();

   void setCurrentStringTable (CompilerStringTable* cst);

   CompilerFloatTable *getCurrentFloatTable();
   CompilerFloatTable &getGlobalFloatTable();
   CompilerFloatTable &getFunctionFloatTable();

   void setCurrentFloatTable (CompilerFloatTable* cst);

   CompilerIdentTable &getIdentTable();

   void precompileIdent(StringTableEntry ident);

   /// Helper function to reset the float, string, and ident tables to a base
   /// starting state.
   void resetTables();

   void *consoleAlloc(U32 size);
   void consoleAllocReset();

   extern bool gSyntaxError;
};

/// Utility class to emit and patch bytecode
class CodeStream
{
public:
   
   enum FixType
   {
      // For loops
      FIXTYPE_LOOPBLOCKSTART,
      FIXTYPE_BREAK,
      FIXTYPE_CONTINUE
   };
   
   enum Constants
   {
      BlockSize = 16384,
   };
   
protected:
   
   typedef struct PatchEntry
   {
      U32 addr;  ///< Address to patch
      U32 value; ///< Value to place at addr
      
      PatchEntry() {;}
      PatchEntry(U32 a, U32 v)  : addr(a), value(v) {;}
   } PatchEntry;
   
   typedef struct CodeData
   {
      U8 *data;       ///< Allocated data (size is BlockSize)
      U32 size;       ///< Bytes used in data
      CodeData *next; ///< Next block
   } CodeData;
   
   /// @name Emitted code
   /// {
   CodeData *mCode;
   CodeData *mCodeHead;
   U32 mCodePos;
   /// }
   
   /// @name Code fixing stacks
   /// {
   Vector<U32> mFixList;
   Vector<U32> mFixStack;
   Vector<bool> mFixLoopStack;
   Vector<PatchEntry> mPatchList;
   /// }
   
   Vector<U32> mBreakLines; ///< Line numbers
   
public:

   CodeStream() : mCode(0), mCodeHead(NULL), mCodePos(0)
   {
   }
   
   ~CodeStream()
   {
      reset();
      
      if (mCode)
      {
         dFree(mCode->data);
         delete mCode;
      }
   }
   
   U8 *allocCode(U32 sz);
   
   inline U32 emit(U32 code)
   {
      U32 *ptr = (U32*)allocCode(4);
      *ptr = code;
#ifdef DEBUG_CODESTREAM
      printf("code[%u] = %u\n", mCodePos, code);
#endif
      return mCodePos++;
   }
   
   inline void patch(U32 addr, U32 code)
   {
#ifdef DEBUG_CODESTREAM
      printf("patch[%u] = %u\n", addr, code);
#endif
      mPatchList.push_back(PatchEntry(addr, code));
   }
   
   inline U32 emitSTE(const char *code)
   {
      U64 *ptr = (U64*)allocCode(8);
      *ptr = 0;
      Compiler::STEtoCode(code, mCodePos, (U32*)ptr);
#ifdef DEBUG_CODESTREAM
      printf("code[%u] = %s\n", mCodePos, code);
#endif
      mCodePos += 2;
      return mCodePos-2;
   }
   
   inline U32 tell()
   {
      return mCodePos;
   }
   
   inline bool inLoop()
   {
      for (U32 i=0; i<mFixLoopStack.size(); i++)
      {
         if (mFixLoopStack[i])
            return true;
      }
      return false;
   }
   
   inline U32 emitFix(FixType type)
   {
      U32 *ptr = (U32*)allocCode(4);
      *ptr = (U32)type;
      
#ifdef DEBUG_CODESTREAM
      printf("code[%u] = [FIX:%u]\n", mCodePos, (U32)type);
#endif
      
      mFixList.push_back(mCodePos);
      mFixList.push_back((U32)type);
      return mCodePos++;
   }
   
   inline void pushFixScope(bool isLoop)
   {
      mFixStack.push_back(mFixList.size());
      mFixLoopStack.push_back(isLoop);
   }
   
   inline void popFixScope()
   {
      AssertFatal(mFixStack.size() > 0, "Fix stack mismatch");
      
      U32 newSize = mFixStack[mFixStack.size()-1];
      while (mFixList.size() > newSize)
         mFixList.pop_back();
      mFixStack.pop_back();
      mFixLoopStack.pop_back();
   }
   
   void fixLoop(U32 loopBlockStart, U32 breakPoint, U32 continuePoint);
   
   inline void addBreakLine(U32 lineNumber, U32 ip)
   {
      mBreakLines.push_back(lineNumber);
      mBreakLines.push_back(ip);
   }
   
   inline U32 getNumLineBreaks()
   {
      return mBreakLines.size() / 2;
   }
   
   void emitCodeStream(U32 *size, U32 **stream, U32 **lineBreaks);
   
   void reset();
};

#endif
