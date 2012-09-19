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

class Stream;
class DataChunker;

#include "platform/platform.h"
#include "console/ast.h"
#include "console/codeBlock.h"


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
      OP_JMPIF_NP,
      OP_JMP,
      OP_RETURN,
      // fixes a bug when not explicitly returning a value
      OP_RETURN_VOID,
      OP_CMPEQ,
      OP_CMPGR,
      OP_CMPGE,
      OP_CMPLT,
      OP_CMPLE,
      OP_CMPNE,
      OP_XOR,
      OP_MOD,
      OP_BITAND,
      OP_BITOR,
      OP_NOT,
      OP_NOTF,
      OP_ONESCOMPLEMENT,

      OP_SHR,
      OP_SHL,
      OP_AND,
      OP_OR,

      OP_ADD,
      OP_SUB,
      OP_MUL,
      OP_DIV,
      OP_NEG,

      OP_SETCURVAR,
      OP_SETCURVAR_CREATE,
      OP_SETCURVAR_ARRAY,
      OP_SETCURVAR_ARRAY_CREATE,

      OP_LOADVAR_UINT,
      OP_LOADVAR_FLT,
      OP_LOADVAR_STR,

      OP_SAVEVAR_UINT,
      OP_SAVEVAR_FLT,
      OP_SAVEVAR_STR,

      OP_SETCUROBJECT,
      OP_SETCUROBJECT_NEW,
      OP_SETCUROBJECT_INTERNAL,

      OP_SETCURFIELD,
      OP_SETCURFIELD_ARRAY,
      OP_SETCURFIELD_TYPE,

      OP_LOADFIELD_UINT,
      OP_LOADFIELD_FLT,
      OP_LOADFIELD_STR,

      OP_SAVEFIELD_UINT,
      OP_SAVEFIELD_FLT,
      OP_SAVEFIELD_STR,

      OP_STR_TO_UINT,
      OP_STR_TO_FLT,
      OP_STR_TO_NONE,
      OP_FLT_TO_UINT,
      OP_FLT_TO_STR,
      OP_FLT_TO_NONE,
      OP_UINT_TO_FLT,
      OP_UINT_TO_STR,
      OP_UINT_TO_NONE,

      OP_LOADIMMED_UINT,
      OP_LOADIMMED_FLT,
      OP_TAG_TO_STR,
      OP_LOADIMMED_STR,
      OP_DOCBLOCK_STR,
      OP_LOADIMMED_IDENT,

      OP_CALLFUNC_RESOLVE,
      OP_CALLFUNC,

      OP_ADVANCE_STR,
      OP_ADVANCE_STR_APPENDCHAR,
      OP_ADVANCE_STR_COMMA,
      OP_ADVANCE_STR_NUL,
      OP_REWIND_STR,
      OP_TERMINATE_REWIND_STR,
      OP_COMPARE_STR,

      OP_PUSH,
      OP_PUSH_FRAME,

      OP_ASSERT,
      OP_BREAK,
      
      OP_ITER_BEGIN,       ///< Prepare foreach iterator.
      OP_ITER_BEGIN_STR,   ///< Prepare foreach$ iterator.
      OP_ITER,             ///< Enter foreach loop.
      OP_ITER_END,         ///< End foreach loop.

      OP_INVALID
   };

   //------------------------------------------------------------

   F64 consoleStringToNumber(const char *str, StringTableEntry file = 0, U32 line = 0);
   U32 precompileBlock(StmtNode *block, U32 loopCount);
   U32 compileBlock(StmtNode *block, U32 *codeStream, U32 ip, U32 continuePoint, U32 breakPoint);

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

   inline StringTableEntry U32toSTE(U32 u)
   {
      return *((StringTableEntry *) &u);
   }

   extern U32 (*STEtoU32)(StringTableEntry ste, U32 ip);

   U32 evalSTEtoU32(StringTableEntry ste, U32);
   U32 compileSTEtoU32(StringTableEntry ste, U32 ip);

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

   CodeBlock *getBreakCodeBlock();
   void setBreakCodeBlock(CodeBlock *cb);

   /// Helper function to reset the float, string, and ident tables to a base
   /// starting state.
   void resetTables();

   void *consoleAlloc(U32 size);
   void consoleAllocReset();

   extern bool gSyntaxError;
};

#endif
