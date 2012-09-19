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

#ifndef _CODEBLOCK_H_
#define _CODEBLOCK_H_

#include "console/compiler.h"
#include "console/consoleParser.h"

class Stream;

/// Core TorqueScript code management class.
///
/// This class represents a block of code, usually mapped directly to a file.
class CodeBlock
{
private:
   static CodeBlock* smCodeBlockList;
   static CodeBlock* smCurrentCodeBlock;
   
public:
   static U32                       smBreakLineCount;
   static bool                      smInFunction;
   static Compiler::ConsoleParser * smCurrentParser;

   static CodeBlock* getCurrentBlock()
   {
      return smCurrentCodeBlock;
   }

   static CodeBlock *getCodeBlockList()
   {
      return smCodeBlockList;
   }

   static StringTableEntry getCurrentCodeBlockName();
   static StringTableEntry getCurrentCodeBlockFullPath();
   static StringTableEntry getCurrentCodeBlockModName();
   static CodeBlock *find(StringTableEntry);

   CodeBlock();
   ~CodeBlock();

   StringTableEntry name;
   StringTableEntry fullPath;
   StringTableEntry modPath;

   char *globalStrings;
   char *functionStrings;

   U32 functionStringsMaxLen;
   U32 globalStringsMaxLen;

   F64 *globalFloats;
   F64 *functionFloats;

   U32 codeSize;
   U32 *code;

   U32 refCount;
   U32 lineBreakPairCount;
   U32 *lineBreakPairs;
   U32 breakListSize;
   U32 *breakList;
   CodeBlock *nextFile;

   void addToCodeList();
   void removeFromCodeList();
   void calcBreakList();
   void clearAllBreaks();
   void setAllBreaks();
   void dumpInstructions( U32 startIp = 0, bool upToReturn = false );

   /// Returns the first breakable line or 0 if none was found.
   /// @param lineNumber The one based line number.
   U32 findFirstBreakLine(U32 lineNumber);

   void clearBreakpoint(U32 lineNumber);

   /// Set a OP_BREAK instruction on a line. If a break 
   /// is not possible on that line it returns false.
   /// @param lineNumber The one based line number.
   bool setBreakpoint(U32 lineNumber);

   void findBreakLine(U32 ip, U32 &line, U32 &instruction);
   const char *getFileLine(U32 ip);

   /// 
   String getFunctionArgs( U32 offset );

   bool read(StringTableEntry fileName, Stream &st);
   bool compile(const char *dsoName, StringTableEntry fileName, const char *script, bool overrideNoDso = false);

   void incRefCount();
   void decRefCount();

   /// Compiles and executes a block of script storing the compiled code in this
   /// CodeBlock. If there is no filename breakpoints will not be generated and 
   /// the CodeBlock will not be added to the linked list of loaded CodeBlocks. 
   /// Note that if the script contains no executable statements the CodeBlock
   /// will delete itself on return an empty string. The return string is any 
   /// result of the code executed, if any, or an empty string.
   ///
   /// @param fileName The file name, including path and extension, for the 
   /// block of code or an empty string.
   /// @param script The script code to compile and execute.
   /// @param noCalls Skips calling functions from the script.
   /// @param setFrame A zero based index of the stack frame to execute the code 
   /// with, zero being the top of the stack. If the the index is
   /// -1 a new frame is created. If the index is out of range the
   /// top stack frame is used.
   const char *compileExec(StringTableEntry fileName, const char *script, 
      bool noCalls, int setFrame = -1 );

   /// Executes the existing code in the CodeBlock. The return string is any 
   /// result of the code executed, if any, or an empty string.
   ///
   /// @param offset The instruction offset to start executing from.
   /// @param fnName The name of the function to execute or null.
   /// @param ns The namespace of the function to execute or null.
   /// @param argc The number of parameters passed to the function or
   /// zero to execute code outside of a function.
   /// @param argv The function parameter list.
   /// @param noCalls Skips calling functions from the script.
   /// @param setFrame A zero based index of the stack frame to execute the code 
   /// with, zero being the top of the stack. If the the index is
   /// -1 a new frame is created. If the index is out of range the
   /// top stack frame is used.
   /// @param packageName The code package name or null.
   const char *exec(U32 offset, const char *fnName, Namespace *ns, U32 argc, 
      const char **argv, bool noCalls, StringTableEntry packageName, 
      S32 setFrame = -1);
};

#endif