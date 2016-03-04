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
#include "console/telnetDebugger.h"

#include "console/ast.h"
#include "core/tAlgorithm.h"

#include "core/strings/findMatch.h"
#include "console/consoleInternal.h"
#include "core/stream/fileStream.h"
#include "console/compiler.h"

#include "console/simBase.h"

namespace Compiler
{

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

   CompilerStringTable *gCurrentStringTable, gGlobalStringTable, gFunctionStringTable;
   CompilerFloatTable  *gCurrentFloatTable,  gGlobalFloatTable,  gFunctionFloatTable;
   DataChunker          gConsoleAllocator;
   CompilerIdentTable   gIdentTable;

   //------------------------------------------------------------

   void evalSTEtoCode(StringTableEntry ste, U32 ip, U32 *ptr)
   {
#ifdef TORQUE_CPU_X64
      *(U64*)(ptr) = (U64)ste;
#else
      *ptr = (U32)ste;
#endif
   }
   
   void compileSTEtoCode(StringTableEntry ste, U32 ip, U32 *ptr)
   {
      if(ste)
         getIdentTable().add(ste, ip);
      *ptr = 0;
      *(ptr+1) = 0;
   }
   
   void (*STEtoCode)(StringTableEntry ste, U32 ip, U32 *ptr) = evalSTEtoCode;

   //------------------------------------------------------------

   bool gSyntaxError = false;

   //------------------------------------------------------------

   CompilerStringTable *getCurrentStringTable()  { return gCurrentStringTable;  }
   CompilerStringTable &getGlobalStringTable()   { return gGlobalStringTable;   }
   CompilerStringTable &getFunctionStringTable() { return gFunctionStringTable; }

   void setCurrentStringTable (CompilerStringTable* cst) { gCurrentStringTable  = cst; }

   CompilerFloatTable *getCurrentFloatTable()    { return gCurrentFloatTable;   }
   CompilerFloatTable &getGlobalFloatTable()     { return gGlobalFloatTable;    }
   CompilerFloatTable &getFunctionFloatTable()   { return gFunctionFloatTable; }

   void setCurrentFloatTable (CompilerFloatTable* cst) { gCurrentFloatTable  = cst; }

   CompilerIdentTable &getIdentTable() { return gIdentTable; }

   void precompileIdent(StringTableEntry ident)
   {
      if(ident)
         gGlobalStringTable.add(ident);
   }

   void resetTables()
   {
      setCurrentStringTable(&gGlobalStringTable);
      setCurrentFloatTable(&gGlobalFloatTable);
      getGlobalFloatTable().reset();
      getGlobalStringTable().reset();
      getFunctionFloatTable().reset();
      getFunctionStringTable().reset();
      getIdentTable().reset();
   }

   void *consoleAlloc(U32 size) { return gConsoleAllocator.alloc(size);  }
   void consoleAllocReset()     { gConsoleAllocator.freeBlocks(); }

}

//-------------------------------------------------------------------------

using namespace Compiler;

//-------------------------------------------------------------------------


U32 CompilerStringTable::add(const char *str, bool caseSens, bool tag)
{
   // Is it already in?
   Entry **walk;
   for(walk = &list; *walk; walk = &((*walk)->next))
   {
      if((*walk)->tag != tag)
         continue;

      if(caseSens)
      {
         if(!dStrcmp((*walk)->string, str))
            return (*walk)->start;
      }
      else
      {
         if(!dStricmp((*walk)->string, str))
            return (*walk)->start;
      }
   }

   // Write it out.
   Entry *newStr = (Entry *) consoleAlloc(sizeof(Entry));
   *walk = newStr;
   newStr->next = NULL;
   newStr->start = totalLen;
   U32 len = dStrlen(str) + 1;
   if(tag && len < 7) // alloc space for the numeric tag 1 for tag, 5 for # and 1 for nul
      len = 7;
   totalLen += len;
   newStr->string = (char *) consoleAlloc(len);
   newStr->len = len;
   newStr->tag = tag;
   dStrcpy(newStr->string, str);
   return newStr->start;
}

U32 CompilerStringTable::addIntString(U32 value)
{
   dSprintf(buf, sizeof(buf), "%d", value);
   return add(buf);
}

U32 CompilerStringTable::addFloatString(F64 value)
{
   dSprintf(buf, sizeof(buf), "%g", value);
   return add(buf);
}

void CompilerStringTable::reset()
{
   list = NULL;
   totalLen = 0;
}

char *CompilerStringTable::build()
{
   char *ret = new char[totalLen];
   for(Entry *walk = list; walk; walk = walk->next)
      dStrcpy(ret + walk->start, walk->string);
   return ret;
}

void CompilerStringTable::write(Stream &st)
{
   st.write(totalLen);
   for(Entry *walk = list; walk; walk = walk->next)
      st.write(walk->len, walk->string);
}

//------------------------------------------------------------

U32 CompilerFloatTable::add(F64 value)
{
   Entry **walk;
   U32 i = 0;
   for(walk = &list; *walk; walk = &((*walk)->next), i++)
      if(value == (*walk)->val)
         return i;
   Entry *newFloat = (Entry *) consoleAlloc(sizeof(Entry));
   newFloat->val = value;
   newFloat->next = NULL;
   count++;
   *walk = newFloat;
   return count-1;
}
void CompilerFloatTable::reset()
{
   list = NULL;
   count = 0;
}
F64 *CompilerFloatTable::build()
{
   F64 *ret = new F64[count];
   U32 i = 0;
   for(Entry *walk = list; walk; walk = walk->next, i++)
      ret[i] = walk->val;
   return ret;
}

void CompilerFloatTable::write(Stream &st)
{
   st.write(count);
   for(Entry *walk = list; walk; walk = walk->next)
      st.write(walk->val);
}

//------------------------------------------------------------

void CompilerIdentTable::reset()
{
   list = NULL;
}

void CompilerIdentTable::add(StringTableEntry ste, U32 ip)
{
   U32 index = gGlobalStringTable.add(ste, false);
   Entry *newEntry = (Entry *) consoleAlloc(sizeof(Entry));
   newEntry->offset = index;
   newEntry->ip = ip;
   for(Entry *walk = list; walk; walk = walk->next)
   {
      if(walk->offset == index)
      {
         newEntry->nextIdent = walk->nextIdent;
         walk->nextIdent = newEntry;
         return;
      }
   }
   newEntry->next = list;
   list = newEntry;
   newEntry->nextIdent = NULL;
}

void CompilerIdentTable::write(Stream &st)
{
   U32 count = 0;
   Entry * walk;
   for(walk = list; walk; walk = walk->next)
      count++;
   st.write(count);
   for(walk = list; walk; walk = walk->next)
   {
      U32 ec = 0;
      Entry * el;
      for(el = walk; el; el = el->nextIdent)
         ec++;
      st.write(walk->offset);
      st.write(ec);
      for(el = walk; el; el = el->nextIdent)
         st.write(el->ip);
   }
}

//-------------------------------------------------------------------------
  
U8 *CodeStream::allocCode(U32 sz)
{
   U8 *ptr = NULL;
   if (mCodeHead)
   {
      const U32 bytesLeft = BlockSize - mCodeHead->size;
      if (bytesLeft > sz)
      {
         ptr = mCodeHead->data + mCodeHead->size;
         mCodeHead->size += sz;
         return ptr;
      }
   }
   
   CodeData *data = new CodeData;
   data->data = (U8*)dMalloc(BlockSize);
   data->size = sz;
   data->next = NULL;
   
   if (mCodeHead)
      mCodeHead->next = data;
   mCodeHead = data;
   if (mCode == NULL)
      mCode = data;
   return data->data;
}
  
//-------------------------------------------------------------------------
  
void CodeStream::fixLoop(U32 loopBlockStart, U32 breakPoint, U32 continuePoint)
{
   AssertFatal(mFixStack.size() > 0, "Fix stack mismatch");
   
   U32 fixStart = mFixStack[mFixStack.size()-1];
   for (U32 i=fixStart; i<mFixList.size(); i += 2)
   {
      FixType type = (FixType)mFixList[i+1];
      
      U32 fixedIp = 0;
      bool valid = true;
      
      switch (type)
      {
         case FIXTYPE_LOOPBLOCKSTART:
            fixedIp = loopBlockStart;
            break;
         case FIXTYPE_BREAK:
            fixedIp = breakPoint;
            break;
         case FIXTYPE_CONTINUE:
            fixedIp = continuePoint;
            break;
         default:
            //Con::warnf("Address %u fixed as %u", mFixList[i], mFixList[i+1]);
            valid = false;
            break;
      }
      
      if (valid)
      {
         patch(mFixList[i], fixedIp);
      }
   }
}

//-------------------------------------------------------------------------
  
void CodeStream::emitCodeStream(U32 *size, U32 **stream, U32 **lineBreaks)
{
   // Alloc stream
   U32 numLineBreaks = getNumLineBreaks();
   *stream = new U32[mCodePos + (numLineBreaks * 2)];
   dMemset(*stream, '\0', mCodePos + (numLineBreaks * 2));
   *size = mCodePos;
   
   // Dump chunks & line breaks
   U32 outBytes = mCodePos * sizeof(U32);
   U8 *outPtr = *((U8**)stream);
   for (CodeData *itr = mCode; itr != NULL; itr = itr->next)
   {
      U32 bytesToCopy = itr->size > outBytes ? outBytes : itr->size;
      dMemcpy(outPtr, itr->data, bytesToCopy);
      outPtr += bytesToCopy;
      outBytes -= bytesToCopy;
   }
   
   *lineBreaks = *stream + mCodePos;
   dMemcpy(*lineBreaks, mBreakLines.address(), sizeof(U32) * mBreakLines.size());
   
   // Apply patches on top
   for (U32 i=0; i<mPatchList.size(); i++)
   {
      PatchEntry &e = mPatchList[i];
      (*stream)[e.addr] = e.value;
   }
}
  
//-------------------------------------------------------------------------
  
void CodeStream::reset()
{
   mCodePos = 0;
   mFixStack.clear();
   mFixLoopStack.clear();
   mFixList.clear();
   mBreakLines.clear();
   
   // Pop down to one code block
   CodeData *itr = mCode ? mCode->next : NULL;
   while (itr != NULL)
   {
      CodeData *next = itr->next;
      dFree(itr->data);
      delete(itr);
      itr = next;
   }
   
   if (mCode)
   {
      mCode->size = 0;
      mCode->next = NULL;
      mCodeHead = mCode;
   }
}

