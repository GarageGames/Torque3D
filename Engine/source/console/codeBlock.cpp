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
#include "console/compiler.h"
#include "console/codeBlock.h"
#include "console/telnetDebugger.h"
#include "console/ast.h"
#include "core/strings/unicode.h"
#include "core/strings/stringFunctions.h"
#include "core/stringTable.h"
#include "core/stream/fileStream.h"

using namespace Compiler;

bool           CodeBlock::smInFunction = false;
U32            CodeBlock::smBreakLineCount = 0;
CodeBlock *    CodeBlock::smCodeBlockList = NULL;
CodeBlock *    CodeBlock::smCurrentCodeBlock = NULL;
ConsoleParser *CodeBlock::smCurrentParser = NULL;

//-------------------------------------------------------------------------

CodeBlock::CodeBlock()
{
   mGlobalStrings = NULL;
   mFunctionStrings = NULL;
   mFunctionStringsMaxLen = 0;
   mGlobalStringsMaxLen = 0;
   mGlobalFloats = NULL;
   mFunctionFloats = NULL;
   mLineBreakPairs = NULL;
   mBreakList = NULL;
   mBreakListSize = 0;

   mRefCount = 0;
   mCode = NULL;
   mName = NULL;
   mFullPath = NULL;
   mModPath = NULL;
}

CodeBlock::~CodeBlock()
{
   // Make sure we aren't lingering in the current code block...
   AssertFatal(smCurrentCodeBlock != this, "CodeBlock::~CodeBlock - Caught lingering in smCurrentCodeBlock!")

   if(mName)
      removeFromCodeList();
   delete[] const_cast<char*>(mGlobalStrings);
   delete[] const_cast<char*>(mFunctionStrings);
   
   mFunctionStringsMaxLen = 0;
   mGlobalStringsMaxLen = 0;

   delete[] mGlobalFloats;
   delete[] mFunctionFloats;
   delete[] mCode;
   delete[] mBreakList;
}

//-------------------------------------------------------------------------

StringTableEntry CodeBlock::getCurrentCodeBlockName()
{
   if (CodeBlock::getCurrentBlock())
      return CodeBlock::getCurrentBlock()->mName;
   else
      return NULL;
}   

StringTableEntry CodeBlock::getCurrentCodeBlockFullPath()
{
   if (CodeBlock::getCurrentBlock())
      return CodeBlock::getCurrentBlock()->mFullPath;
   else
      return NULL;
}

StringTableEntry CodeBlock::getCurrentCodeBlockModName()
{
   if (CodeBlock::getCurrentBlock())
      return CodeBlock::getCurrentBlock()->mModPath;
   else
      return NULL;
}

CodeBlock *CodeBlock::find(StringTableEntry name)
{
   for(CodeBlock *walk = CodeBlock::getCodeBlockList(); walk; walk = walk->mNextFile)
      if(walk->mName == name)
         return walk;
   return NULL;
}

//-------------------------------------------------------------------------

void CodeBlock::addToCodeList()
{
   // remove any code blocks with my name
   for(CodeBlock **walk = &smCodeBlockList; *walk;walk = &((*walk)->mNextFile))
   {
      if((*walk)->mName == mName)
      {
         *walk = (*walk)->mNextFile;
         break;
      }
   }
   mNextFile = smCodeBlockList;
   smCodeBlockList = this;
}

void CodeBlock::clearAllBreaks()
{
   if(!mLineBreakPairs)
      return;
   for(U32 i = 0; i < mLineBreakPairCount; i++)
   {
      U32 *p = mLineBreakPairs + i * 2;
      mCode[p[1]] = p[0] & 0xFF;
   }
}

void CodeBlock::clearBreakpoint(U32 lineNumber)
{
   if(!mLineBreakPairs)
      return;
   for(U32 i = 0; i < mLineBreakPairCount; i++)
   {
      U32 *p = mLineBreakPairs + i * 2;
      if((p[0] >> 8) == lineNumber)
      {
         mCode[p[1]] = p[0] & 0xFF;
         return;
      }
   }
}

void CodeBlock::setAllBreaks()
{
   if(!mLineBreakPairs)
      return;
   for(U32 i = 0; i < mLineBreakPairCount; i++)
   {
      U32 *p = mLineBreakPairs + i * 2;
      mCode[p[1]] = OP_BREAK;
   }
}

bool CodeBlock::setBreakpoint(U32 lineNumber)
{
   if(!mLineBreakPairs)
      return false;

   for(U32 i = 0; i < mLineBreakPairCount; i++)
   {
      U32 *p = mLineBreakPairs + i * 2;
      if((p[0] >> 8) == lineNumber)
      {
         mCode[p[1]] = OP_BREAK;
         return true;
      }
   }

   return false;
}

U32 CodeBlock::findFirstBreakLine(U32 lineNumber)
{
   if(!mLineBreakPairs)
      return 0;

   for(U32 i = 0; i < mLineBreakPairCount; i++)
   {
      U32 *p = mLineBreakPairs + i * 2;
      U32 line = (p[0] >> 8);

      if( lineNumber <= line )
         return line;
   }

   return 0;
}

struct LinePair
{
   U32 instLine;
   U32 ip;
};

void CodeBlock::findBreakLine(U32 ip, U32 &line, U32 &instruction)
{
   U32 min = 0;
   U32 max = mLineBreakPairCount - 1;
   LinePair *p = (LinePair *) mLineBreakPairs;

   U32 found;
   if(!mLineBreakPairCount || p[min].ip > ip || p[max].ip < ip)
   {
      line = 0;
      instruction = OP_INVALID;
      return;
   }
   else if(p[min].ip == ip)
      found = min;
   else if(p[max].ip == ip)
      found = max;
   else
   {
      for(;;)
      {
         if(min == max - 1)
         {
            found = min;
            break;
         }
         U32 mid = (min + max) >> 1;
         if(p[mid].ip == ip)
         {
            found = mid;
            break;
         }
         else if(p[mid].ip > ip)
            max = mid;
         else
            min = mid;
      }
   }
   instruction = p[found].instLine & 0xFF;
   line = p[found].instLine >> 8;
}

const char *CodeBlock::getFileLine(U32 ip)
{
   static char nameBuffer[256];
   U32 line, inst;
   findBreakLine(ip, line, inst);

   dSprintf(nameBuffer, sizeof(nameBuffer), "%s (%d)", mName ? mName : "<input>", line);
   return nameBuffer;
}

void CodeBlock::removeFromCodeList()
{
   for(CodeBlock **walk = &smCodeBlockList; *walk; walk = &((*walk)->mNextFile))
   {
      if(*walk == this)
      {
         *walk = mNextFile;

         // clear out all breakpoints
         clearAllBreaks();
         break;
      }
   }

   // Let the telnet debugger know that this code
   // block has been unloaded and that it needs to
   // remove references to it.
   if ( TelDebugger )
      TelDebugger->clearCodeBlockPointers( this );
}

void CodeBlock::calcBreakList()
{
   U32 size = 0;
   S32 line = -1;
   U32 seqCount = 0;
   U32 i;
   for(i = 0; i < mLineBreakPairCount; i++)
   {
      U32 lineNumber = mLineBreakPairs[i * 2];
      if(lineNumber == U32(line + 1))
         seqCount++;
      else
      {
         if(seqCount)
            size++;
         size++;
         seqCount = 1;
      }
      line = lineNumber;
   }
   if(seqCount)
      size++;

   mBreakList = new U32[size];
   mBreakListSize = size;
   line = -1;
   seqCount = 0;
   size = 0;

   for(i = 0; i < mLineBreakPairCount; i++)
   {
      U32 lineNumber = mLineBreakPairs[i * 2];

      if(lineNumber == U32(line + 1))
         seqCount++;
      else
      {
         if(seqCount)
            mBreakList[size++] = seqCount;
         mBreakList[size++] = lineNumber - getMax(0, line) - 1;
         seqCount = 1;
      }

      line = lineNumber;
   }

   if(seqCount)
      mBreakList[size++] = seqCount;

   for(i = 0; i < mLineBreakPairCount; i++)
   {
      U32 *p = mLineBreakPairs + i * 2;
      p[0] = (p[0] << 8) | mCode[p[1]];
   }

   // Let the telnet debugger know that this code
   // block has been loaded and that it can add break
   // points it has for it.
   if ( TelDebugger )
      TelDebugger->addAllBreakpoints( this );
}

bool CodeBlock::read(StringTableEntry fileName, Stream &st)
{
   const StringTableEntry exePath = Platform::getMainDotCsDir();
   const StringTableEntry cwd = Platform::getCurrentDirectory();

   mName = fileName;

   if(fileName)
   {
      mFullPath = NULL;

      if(Platform::isFullPath(fileName))
         mFullPath = fileName;

      if(dStrnicmp(exePath, fileName, dStrlen(exePath)) == 0)
         mName = StringTable->insert(fileName + dStrlen(exePath) + 1, true);
      else if(dStrnicmp(cwd, fileName, dStrlen(cwd)) == 0)
         mName = StringTable->insert(fileName + dStrlen(cwd) + 1, true);

      if(mFullPath == NULL)
      {
         char buf[1024];
         mFullPath = StringTable->insert(Platform::makeFullPathName(fileName, buf, sizeof(buf)), true);
      }

      mModPath = Con::getModNameFromPath(fileName);
   }
   
   //
   addToCodeList();

   U32 globalSize,size,i;
   st.read(&size);
   if(size)
   {
      mGlobalStrings = new char[size];
      mGlobalStringsMaxLen = size;
      st.read(size, mGlobalStrings);
   }
   globalSize = size;
   st.read(&size);
   if(size)
   {
      mFunctionStrings = new char[size];
      mFunctionStringsMaxLen = size;
      st.read(size, mFunctionStrings);
   }
   st.read(&size);
   if(size)
   {
      mGlobalFloats = new F64[size];
      for(U32 i = 0; i < size; i++)
         st.read(&mGlobalFloats[i]);
   }
   st.read(&size);
   if(size)
   {
      mFunctionFloats = new F64[size];
      for(U32 i = 0; i < size; i++)
         st.read(&mFunctionFloats[i]);
   }
   U32 codeSize;
   st.read(&codeSize);
   st.read(&mLineBreakPairCount);

   U32 totSize = codeSize + mLineBreakPairCount * 2;
   mCode = new U32[totSize];

   for(i = 0; i < codeSize; i++)
   {
      U8 b;
      st.read(&b);
      if(b == 0xFF)
         st.read(&mCode[i]);
      else
         mCode[i] = b;
   }

   for(i = codeSize; i < totSize; i++)
      st.read(&mCode[i]);

   mLineBreakPairs = mCode + codeSize;

   // StringTable-ize our identifiers.
   U32 identCount;
   st.read(&identCount);
   while(identCount--)
   {
      U32 offset;
      st.read(&offset);
      StringTableEntry ste;
      if(offset < globalSize)
         ste = StringTable->insert(mGlobalStrings + offset);
      else
         ste = StringTable->insert("");
      U32 count;
      st.read(&count);
      while(count--)
      {
         U32 ip;
         st.read(&ip);
         mCode[ip] = *((U32 *) &ste);
      }
   }

   if(mLineBreakPairCount)
      calcBreakList();

   return true;
}


bool CodeBlock::compile(const char *codeFileName, StringTableEntry fileName, const char *inScript, bool overrideNoDso)
{
   // This will return true, but return value is ignored
   char *script;
   chompUTF8BOM( inScript, &script );
   
   gSyntaxError = false;

   consoleAllocReset();

   STEtoU32 = compileSTEtoU32;

   gStatementList = NULL;

   // Set up the parser.
   smCurrentParser = getParserForFile(fileName);
   AssertISV(smCurrentParser, avar("CodeBlock::compile - no parser available for '%s'!", fileName));

   // Now do some parsing.
   smCurrentParser->setScanBuffer(script, fileName);
   smCurrentParser->restart(NULL);
   smCurrentParser->parse();

   if(gSyntaxError)
   {
      consoleAllocReset();
      return false;
   }   

#ifdef TORQUE_NO_DSO_GENERATION
   if(!overrideNoDso)
      return false;
#endif // !TORQUE_NO_DSO_GENERATION

   FileStream st;
   if(!st.open(codeFileName, Torque::FS::File::Write)) 
      return false;
   st.write(U32(Con::DSOVersion));

   // Reset all our value tables...
   resetTables();

   smInFunction = false;
   smBreakLineCount = 0;
   setBreakCodeBlock(this);

   if(gStatementList)
      mCodeSize = precompileBlock(gStatementList, 0) + 1;
   else
      mCodeSize = 1;

   mLineBreakPairCount = smBreakLineCount;
   mCode = new U32[mCodeSize + smBreakLineCount * 2];
   mLineBreakPairs = mCode + mCodeSize;

   // Write string table data...
   getGlobalStringTable().write(st);
   getFunctionStringTable().write(st);

   // Write float table data...
   getGlobalFloatTable().write(st);
   getFunctionFloatTable().write(st);

   smBreakLineCount = 0;
   U32 lastIp;
   if(gStatementList)
      lastIp = compileBlock(gStatementList, mCode, 0, 0, 0);
   else
      lastIp = 0;

   if(lastIp != mCodeSize - 1)
      Con::errorf(ConsoleLogEntry::General, "CodeBlock::compile - precompile size mismatch, a precompile/compile function pair is probably mismatched.");

   mCode[lastIp++] = OP_RETURN;
   U32 totSize = mCodeSize + smBreakLineCount * 2;
   st.write(mCodeSize);
   st.write(mLineBreakPairCount);

   // Write out our bytecode, doing a bit of compression for low numbers.
   U32 i;   
   for(i = 0; i < mCodeSize; i++)
   {
      if(mCode[i] < 0xFF)
         st.write(U8(mCode[i]));
      else
      {
         st.write(U8(0xFF));
         st.write(mCode[i]);
      }
   }

   // Write the break info...
   for(i = mCodeSize; i < totSize; i++)
      st.write(mCode[i]);

   getIdentTable().write(st);

   consoleAllocReset();
   st.close();

   return true;


}

const char *CodeBlock::compileExec(StringTableEntry fileName, const char *inString, bool noCalls, S32 setFrame)
{
   // Check for a UTF8 script file
   char *string;
   chompUTF8BOM( inString, &string );

   STEtoU32 = evalSTEtoU32;
   consoleAllocReset();

   mName = fileName;

   if(fileName)
   {
      const StringTableEntry exePath = Platform::getMainDotCsDir();
      const StringTableEntry cwd = Platform::getCurrentDirectory();

      mFullPath = NULL;
      
      if(Platform::isFullPath(fileName))
         mFullPath = fileName;

      if(dStrnicmp(exePath, fileName, dStrlen(exePath)) == 0)
         mName = StringTable->insert(fileName + dStrlen(exePath) + 1, true);
      else if(dStrnicmp(cwd, fileName, dStrlen(cwd)) == 0)
         mName = StringTable->insert(fileName + dStrlen(cwd) + 1, true);

      if(mFullPath == NULL)
      {
         char buf[1024];
         mFullPath = StringTable->insert(Platform::makeFullPathName(fileName, buf, sizeof(buf)), true);
      }

      mModPath = Con::getModNameFromPath(fileName);
   }

   if(mName)
      addToCodeList();
   
   gStatementList = NULL;

   // Set up the parser.
   smCurrentParser = getParserForFile(fileName);
   AssertISV(smCurrentParser, avar("CodeBlock::compile - no parser available for '%s'!", fileName));

   // Now do some parsing.
   smCurrentParser->setScanBuffer(string, fileName);
   smCurrentParser->restart(NULL);
   smCurrentParser->parse();

   if(!gStatementList)
   {
      delete this;
      return "";
   }

   resetTables();

   smInFunction = false;
   smBreakLineCount = 0;
   setBreakCodeBlock(this);

   mCodeSize = precompileBlock(gStatementList, 0) + 1;

   mLineBreakPairCount = smBreakLineCount;

   mGlobalStrings   = getGlobalStringTable().build();
   mGlobalStringsMaxLen = getGlobalStringTable().totalLen;

   mFunctionStrings = getFunctionStringTable().build();
   mFunctionStringsMaxLen = getFunctionStringTable().totalLen;

   mGlobalFloats    = getGlobalFloatTable().build();
   mFunctionFloats  = getFunctionFloatTable().build();

   mCode = new U32[mCodeSize + mLineBreakPairCount * 2];
   mLineBreakPairs = mCode + mCodeSize;

   smBreakLineCount = 0;
   U32 lastIp = compileBlock(gStatementList, mCode, 0, 0, 0);
   mCode[lastIp++] = OP_RETURN;
   
   consoleAllocReset();

   if(mLineBreakPairCount && fileName)
      calcBreakList();

   if(lastIp != mCodeSize)
      Con::warnf(ConsoleLogEntry::General, "precompile size mismatch, precompile: %d compile: %d", mCodeSize, lastIp);

   return exec(0, fileName, NULL, 0, 0, noCalls, NULL, setFrame);
}

//-------------------------------------------------------------------------

void CodeBlock::incRefCount()
{
   mRefCount++;
}

void CodeBlock::decRefCount()
{
   mRefCount--;
   if(!mRefCount)
      delete this;
}

//-------------------------------------------------------------------------

String CodeBlock::getFunctionArgs( U32 ip )
{
   StringBuilder str;
   
   U32 fnArgc = mCode[ ip + 5 ];
   for( U32 i = 0; i < fnArgc; ++ i )
   {
      StringTableEntry var = U32toSTE( mCode[ ip + i + 6 ] );
      
      if( i != 0 )
         str.append( ", " );

      str.append( "string " );

      // Try to capture junked parameters
      if( var[ 0 ] )
         str.append( var + 1 );
      else
         str.append( "JUNK" );
   }
   
   return str.end();
}

//-------------------------------------------------------------------------

void CodeBlock::dumpInstructions( U32 startIp, bool upToReturn )
{
   U32 ip = startIp;
   while( ip < mCodeSize )
   {
      switch( mCode[ ip ++ ] )
      {
         case OP_FUNC_DECL:
         {
            StringTableEntry fnName       = U32toSTE(mCode[ip]);
            StringTableEntry fnNamespace  = U32toSTE(mCode[ip+1]);
            StringTableEntry fnPackage    = U32toSTE(mCode[ip+2]);
            bool hasBody = bool(mCode[ip+3]);
            U32 newIp = mCode[ ip + 4 ];
            U32 argc = mCode[ ip + 5 ];
            
            Con::printf( "%i: OP_FUNC_DECL name=%s nspace=%s package=%s hasbody=%i newip=%i argc=%i",
               ip - 1, fnName, fnNamespace, fnPackage, hasBody, newIp, argc );
               
            // Skip args.
                           
            ip += 6 + argc;
            break;
         }
            
         case OP_CREATE_OBJECT:
         {
            StringTableEntry objParent = U32toSTE(mCode[ip    ]);
            bool isDataBlock =          mCode[ip + 1];
            bool isInternal  =          mCode[ip + 2];
            bool isSingleton =          mCode[ip + 3];
            U32  lineNumber  =          mCode[ip + 4];
            U32 failJump     =          mCode[ip + 5];
            
            Con::printf( "%i: OP_CREATE_OBJECT objParent=%s isDataBlock=%i isInternal=%i isSingleton=%i lineNumber=%i failJump=%i",
               ip - 1, objParent, isDataBlock, isInternal, isSingleton, lineNumber, failJump );

            ip += 6;
            break;
         }

         case OP_ADD_OBJECT:
         {
            bool placeAtRoot = mCode[ip++];
            Con::printf( "%i: OP_ADD_OBJECT placeAtRoot=%i", ip - 1, placeAtRoot );
            break;
         }
         
         case OP_END_OBJECT:
         {
            bool placeAtRoot = mCode[ip++];
            Con::printf( "%i: OP_END_OBJECT placeAtRoot=%i", ip - 1, placeAtRoot );
            break;
         }
         
         case OP_FINISH_OBJECT:
         {
            Con::printf( "%i: OP_FINISH_OBJECT", ip - 1 );
            break;
         }
         
         case OP_JMPIFFNOT:
         {
            Con::printf( "%i: OP_JMPIFFNOT ip=%i", ip - 1, mCode[ ip ] );
            ++ ip;
            break;
         }
         
         case OP_JMPIFNOT:
         {
            Con::printf( "%i: OP_JMPIFNOT ip=%i", ip - 1, mCode[ ip ] );
            ++ ip;
            break;
         }
         
         case OP_JMPIFF:
         {
            Con::printf( "%i: OP_JMPIFF ip=%i", ip - 1, mCode[ ip ] );
            ++ ip;
            break;
         }

         case OP_JMPIF:
         {
            Con::printf( "%i: OP_JMPIF ip=%i", ip - 1, mCode[ ip ] );
            ++ ip;
            break;
         }

         case OP_JMPIFNOT_NP:
         {
            Con::printf( "%i: OP_JMPIFNOT_NP ip=%i", ip - 1, mCode[ ip ] );
            ++ ip;
            break;
         }

         case OP_JMPIF_NP:
         {
            Con::printf( "%i: OP_JMPIF_NP ip=%i", ip - 1, mCode[ ip ] );
            ++ ip;
            break;
         }

         case OP_JMP:
         {
            Con::printf( "%i: OP_JMP ip=%i", ip - 1, mCode[ ip ] );
            ++ ip;
            break;
         }

         case OP_RETURN:
         {
            Con::printf( "%i: OP_RETURN", ip - 1 );
            
            if( upToReturn )
               return;
               
            break;
         }

         case OP_RETURN_VOID:
         {
            Con::printf( "%i: OP_RETURNVOID", ip - 1 );

            if( upToReturn )
               return;

            break;
         }

         case OP_CMPEQ:
         {
            Con::printf( "%i: OP_CMPEQ", ip - 1 );
            break;
         }

         case OP_CMPGR:
         {
            Con::printf( "%i: OP_CMPGR", ip - 1 );
            break;
         }

         case OP_CMPGE:
         {
            Con::printf( "%i: OP_CMPGE", ip - 1 );
            break;
         }

         case OP_CMPLT:
         {
            Con::printf( "%i: OP_CMPLT", ip - 1 );
            break;
         }

         case OP_CMPLE:
         {
            Con::printf( "%i: OP_CMPLE", ip - 1 );
            break;
         }

         case OP_CMPNE:
         {
            Con::printf( "%i: OP_CMPNE", ip - 1 );
            break;
         }

         case OP_XOR:
         {
            Con::printf( "%i: OP_XOR", ip - 1 );
            break;
         }

         case OP_MOD:
         {
            Con::printf( "%i: OP_MOD", ip - 1 );
            break;
         }

         case OP_BITAND:
         {
            Con::printf( "%i: OP_BITAND", ip - 1 );
            break;
         }

         case OP_BITOR:
         {
            Con::printf( "%i: OP_BITOR", ip - 1 );
            break;
         }

         case OP_NOT:
         {
            Con::printf( "%i: OP_NOT", ip - 1 );
            break;
         }

         case OP_NOTF:
         {
            Con::printf( "%i: OP_NOTF", ip - 1 );
            break;
         }

         case OP_ONESCOMPLEMENT:
         {
            Con::printf( "%i: OP_ONESCOMPLEMENT", ip - 1 );
            break;
         }

         case OP_SHR:
         {
            Con::printf( "%i: OP_SHR", ip - 1 );
            break;
         }

         case OP_SHL:
         {
            Con::printf( "%i: OP_SHL", ip - 1 );
            break;
         }

         case OP_AND:
         {
            Con::printf( "%i: OP_AND", ip - 1 );
            break;
         }

         case OP_OR:
         {
            Con::printf( "%i: OP_OR", ip - 1 );
            break;
         }

         case OP_ADD:
         {
            Con::printf( "%i: OP_ADD", ip - 1 );
            break;
         }

         case OP_SUB:
         {
            Con::printf( "%i: OP_SUB", ip - 1 );
            break;
         }

         case OP_MUL:
         {
            Con::printf( "%i: OP_MUL", ip - 1 );
            break;
         }

         case OP_DIV:
         {
            Con::printf( "%i: OP_DIV", ip - 1 );
            break;
         }

         case OP_NEG:
         {
            Con::printf( "%i: OP_NEG", ip - 1 );
            break;
         }

         case OP_SETCURVAR:
         {
            StringTableEntry var = U32toSTE(mCode[ip]);
            
            Con::printf( "%i: OP_SETCURVAR var=%s", ip - 1, var );
            ip++;
            break;
         }
         
         case OP_SETCURVAR_CREATE:
         {
            StringTableEntry var = U32toSTE(mCode[ip]);
            
            Con::printf( "%i: OP_SETCURVAR_CREATE var=%s", ip - 1, var );
            ip++;
            break;
         }
         
         case OP_SETCURVAR_ARRAY:
         {
            Con::printf( "%i: OP_SETCURVAR_ARRAY", ip - 1 );
            break;
         }
         
         case OP_SETCURVAR_ARRAY_CREATE:
         {
            Con::printf( "%i: OP_SETCURVAR_ARRAY_CREATE", ip - 1 );
            break;
         }
         
         case OP_LOADVAR_UINT:
         {
            Con::printf( "%i: OP_LOADVAR_UINT", ip - 1 );
            break;
         }
         
         case OP_LOADVAR_FLT:
         {
            Con::printf( "%i: OP_LOADVAR_FLT", ip - 1 );
            break;
         }

         case OP_LOADVAR_STR:
         {
            Con::printf( "%i: OP_LOADVAR_STR", ip - 1 );
            break;
         }

         case OP_SAVEVAR_UINT:
         {
            Con::printf( "%i: OP_SAVEVAR_UINT", ip - 1 );
            break;
         }

         case OP_SAVEVAR_FLT:
         {
            Con::printf( "%i: OP_SAVEVAR_FLT", ip - 1 );
            break;
         }

         case OP_SAVEVAR_STR:
         {
            Con::printf( "%i: OP_SAVEVAR_STR", ip - 1 );
            break;
         }

         case OP_SETCUROBJECT:
         {
            Con::printf( "%i: OP_SETCUROBJECT", ip - 1 );
            break;
         }

         case OP_SETCUROBJECT_NEW:
         {
            Con::printf( "%i: OP_SETCUROBJECT_NEW", ip - 1 );
            break;
         }
         
         case OP_SETCUROBJECT_INTERNAL:
         {
            Con::printf( "%i: OP_SETCUROBJECT_INTERNAL", ip - 1 );
            ++ ip;
            break;
         }
         
         case OP_SETCURFIELD:
         {
            StringTableEntry curField = U32toSTE(mCode[ip]);
            Con::printf( "%i: OP_SETCURFIELD field=%s", ip - 1, curField );
            ++ ip;
         }
         
         case OP_SETCURFIELD_ARRAY:
         {
            Con::printf( "%i: OP_SETCURFIELD_ARRAY", ip - 1 );
            break;
         }

         case OP_SETCURFIELD_TYPE:
         {
            U32 type = mCode[ ip ];
            Con::printf( "%i: OP_SETCURFIELD_TYPE type=%i", ip - 1, type );
            ++ ip;
            break;
         }

         case OP_LOADFIELD_UINT:
         {
            Con::printf( "%i: OP_LOADFIELD_UINT", ip - 1 );
            break;
         }

         case OP_LOADFIELD_FLT:
         {
            Con::printf( "%i: OP_LOADFIELD_FLT", ip - 1 );
            break;
         }

         case OP_LOADFIELD_STR:
         {
            Con::printf( "%i: OP_LOADFIELD_STR", ip - 1 );
            break;
         }

         case OP_SAVEFIELD_UINT:
         {
            Con::printf( "%i: OP_SAVEFIELD_UINT", ip - 1 );
            break;
         }

         case OP_SAVEFIELD_FLT:
         {
            Con::printf( "%i: OP_SAVEFIELD_FLT", ip - 1 );
            break;
         }

         case OP_SAVEFIELD_STR:
         {
            Con::printf( "%i: OP_SAVEFIELD_STR", ip - 1 );
            break;
         }

         case OP_STR_TO_UINT:
         {
            Con::printf( "%i: OP_STR_TO_UINT", ip - 1 );
            break;
         }

         case OP_STR_TO_FLT:
         {
            Con::printf( "%i: OP_STR_TO_FLT", ip - 1 );
            break;
         }

         case OP_STR_TO_NONE:
         {
            Con::printf( "%i: OP_STR_TO_NONE", ip - 1 );
            break;
         }

         case OP_FLT_TO_UINT:
         {
            Con::printf( "%i: OP_FLT_TO_UINT", ip - 1 );
            break;
         }

         case OP_FLT_TO_STR:
         {
            Con::printf( "%i: OP_FLT_TO_STR", ip - 1 );
            break;
         }

         case OP_FLT_TO_NONE:
         {
            Con::printf( "%i: OP_FLT_TO_NONE", ip - 1 );
            break;
         }

         case OP_UINT_TO_FLT:
         {
            Con::printf( "%i: OP_SAVEFIELD_STR", ip - 1 );
            break;
         }

         case OP_UINT_TO_STR:
         {
            Con::printf( "%i: OP_UINT_TO_STR", ip - 1 );
            break;
         }

         case OP_UINT_TO_NONE:
         {
            Con::printf( "%i: OP_UINT_TO_NONE", ip - 1 );
            break;
         }

         case OP_LOADIMMED_UINT:
         {
            U32 val = mCode[ ip ];
            Con::printf( "%i: OP_LOADIMMED_UINT val=%i", ip - 1, val );
            ++ ip;
            break;
         }

         case OP_LOADIMMED_FLT:
         {
            F64 val = mFunctionFloats[ mCode[ ip ] ];
            Con::printf( "%i: OP_LOADIMMED_FLT val=%f", ip - 1, val );
            ++ ip;
            break;
         }

         case OP_TAG_TO_STR:
         {
            const char* str = mFunctionStrings + mCode[ ip ];
            Con::printf( "%i: OP_TAG_TO_STR str=%s", ip - 1, str );
            ++ ip;
            break;
         }
         
         case OP_LOADIMMED_STR:
         {
            const char* str = mFunctionStrings + mCode[ ip ];
            Con::printf( "%i: OP_LOADIMMED_STR str=%s", ip - 1, str );
            ++ ip;
            break;
         }

         case OP_DOCBLOCK_STR:
         {
            const char* str = mFunctionStrings + mCode[ ip ];
            Con::printf( "%i: OP_DOCBLOCK_STR str=%s", ip - 1, str );
            ++ ip;
            break;
         }
         
         case OP_LOADIMMED_IDENT:
         {
            StringTableEntry str = U32toSTE( mCode[ ip ] );
            Con::printf( "%i: OP_LOADIMMED_IDENT str=%s", ip - 1, str );
            ++ ip;
            break;
         }

         case OP_CALLFUNC_RESOLVE:
         {
            StringTableEntry fnNamespace = U32toSTE(mCode[ip+1]);
            StringTableEntry fnName      = U32toSTE(mCode[ip]);
            U32 callType = mCode[ip+2];

            Con::printf( "%i: OP_CALLFUNC_RESOLVE name=%s nspace=%s callType=%s", ip - 1, fnName, fnNamespace,
               callType == FuncCallExprNode::FunctionCall ? "FunctionCall"
                  : callType == FuncCallExprNode::MethodCall ? "MethodCall" : "ParentCall" );
            
            ip += 3;
            break;
         }
         
         case OP_CALLFUNC:
         {
            StringTableEntry fnNamespace = U32toSTE(mCode[ip+1]);
            StringTableEntry fnName      = U32toSTE(mCode[ip]);
            U32 callType = mCode[ip+2];

            Con::printf( "%i: OP_CALLFUNC name=%s nspace=%s callType=%s", ip - 1, fnName, fnNamespace,
               callType == FuncCallExprNode::FunctionCall ? "FunctionCall"
                  : callType == FuncCallExprNode::MethodCall ? "MethodCall" : "ParentCall" );
            
            ip += 3;
            break;
         }

         case OP_ADVANCE_STR:
         {
            Con::printf( "%i: OP_ADVANCE_STR", ip - 1 );
            break;
         }

         case OP_ADVANCE_STR_APPENDCHAR:
         {
            char ch = mCode[ ip ];
            Con::printf( "%i: OP_ADVANCE_STR_APPENDCHAR char=%c", ip - 1, ch );
            ++ ip;
            break;
         }

         case OP_ADVANCE_STR_COMMA:
         {
            Con::printf( "%i: OP_ADVANCE_STR_COMMA", ip - 1 );
            break;
         }

         case OP_ADVANCE_STR_NUL:
         {
            Con::printf( "%i: OP_ADVANCE_STR_NUL", ip - 1 );
            break;
         }

         case OP_REWIND_STR:
         {
            Con::printf( "%i: OP_REWIND_STR", ip - 1 );
            break;
         }

         case OP_TERMINATE_REWIND_STR:
         {
            Con::printf( "%i: OP_TERMINATE_REWIND_STR", ip - 1 );
            break;
         }

         case OP_COMPARE_STR:
         {
            Con::printf( "%i: OP_COMPARE_STR", ip - 1 );
            break;
         }

         case OP_PUSH:
         {
            Con::printf( "%i: OP_PUSH", ip - 1 );
            break;
         }

         case OP_PUSH_FRAME:
         {
            Con::printf( "%i: OP_PUSH_FRAME", ip - 1 );
            break;
         }

         case OP_ASSERT:
         {
            const char* message = mFunctionStrings + mCode[ ip ];
            Con::printf( "%i: OP_ASSERT message=%s", ip - 1, message );
            ++ ip;
            break;
         }

         case OP_BREAK:
         {
            Con::printf( "%i: OP_BREAK", ip - 1 );
            break;
         }
         
         case OP_ITER_BEGIN:
         {
            StringTableEntry varName = U32toSTE( mCode[ ip ] );
            U32 failIp = mCode[ ip + 1 ];
            
            Con::printf( "%i: OP_ITER_BEGIN varName=%s failIp=%i", varName, failIp );

            ++ ip;
         }

         case OP_ITER_BEGIN_STR:
         {
            StringTableEntry varName = U32toSTE( mCode[ ip ] );
            U32 failIp = mCode[ ip + 1 ];
            
            Con::printf( "%i: OP_ITER_BEGIN varName=%s failIp=%i", varName, failIp );

            ip += 2;
         }
         
         case OP_ITER:
         {
            U32 breakIp = mCode[ ip ];
            
            Con::printf( "%i: OP_ITER breakIp=%i", breakIp );

            ++ ip;
         }
         
         case OP_ITER_END:
         {
            Con::printf( "%i: OP_ITER_END", ip - 1 );
            break;
         }

         default:
            Con::printf( "%i: !!INVALID!!", ip - 1 );
            break;
      }
   }
}
