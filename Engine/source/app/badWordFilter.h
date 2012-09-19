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
#ifndef _H_BADWORDFILTER
#define _H_BADWORDFILTER

#include "core/util/tVector.h"

class BadWordFilter
{
private:
   struct FilterTable
   {
      U16 nextState[26]; // only 26 alphabetical chars.
      FilterTable();
   };
   friend struct FilterTable;
   Vector<FilterTable*> filterTables;

   enum {
      TerminateNotFound = 0xFFFE,
      TerminateFound = 0xFFFF,
      MaxBadwordLength = 32,
   };
   char defaultReplaceStr[32];

   BadWordFilter();
   ~BadWordFilter();
   U32 curOffset;
   static U8 remapTable[257];
   static U8 randomJunk[MaxBadwordLength + 1];
   static bool filteringEnabled;

public:
   bool addBadWord(const char *word);
   bool setDefaultReplaceStr(const char *str);
   const char* getDefaultReplaceStr(){ return defaultReplaceStr; }
   void filterString(char *string, const char *replaceStr = NULL);
   bool containsBadWords(const char *string);

   static bool isEnabled() { return filteringEnabled; }
   static void setEnabled(bool enable) { filteringEnabled = enable; }
   static void create();
   static void destroy();
};

extern BadWordFilter *gBadWordFilter;

#endif