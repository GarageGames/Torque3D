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

#ifndef _TAGDICTIONARY_H_
#define _TAGDICTIONARY_H_

#ifndef _STRINGTABLE_H_
#include "core/stringTable.h"
#endif
#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif

class Stream;

class TagDictionary
{
   struct TagEntry
   {
      S32 id;
      StringTableEntry define;
      StringTableEntry string;
      TagEntry *chain; // for linear traversal
      TagEntry *defineHashLink;
      TagEntry *idHashLink;
   };

   TagEntry **defineHashBuckets;
   TagEntry **idHashBuckets;

   TagEntry *entryChain;
   DataChunker mempool;
   S32 numBuckets;
   S32 numEntries;

   bool match(const char* pattern, const char* str);
   void sortIdVector(Vector<S32>& out_v);
public:
   TagDictionary();
   ~TagDictionary();
   
	//IO functions
	//
   bool writeHeader(Stream &);

   // String/Define retrieval and search functions...
   //

   bool addEntry(S32 value, StringTableEntry define, StringTableEntry string);
   
   StringTableEntry defineToString(StringTableEntry tag);
   StringTableEntry idToString(S32 tag);
   StringTableEntry idToDefine(S32 tag);
   S32 defineToId(StringTableEntry tag);

   // get IDs such that minID < IDs < maxID 
   void findIDs( Vector<S32> &v, const S32 minID, const S32 maxID );
	void findStrings( Vector<S32> &v, const char *pattern);
	void findDefines( Vector<S32> &v, const char *pattern);
};

extern TagDictionary tagDictionary;

#endif //_TAGDICTIONARY_H_
