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

#include "core/strings/stringFunctions.h"

#include "console/consoleTypes.h"
#include "app/badWordFilter.h"
#include "core/module.h"
#include "console/engineAPI.h"

MODULE_BEGIN( BadWordFilter )

   MODULE_INIT
   {
      BadWordFilter::create();
   }
   
   MODULE_SHUTDOWN
   {
      BadWordFilter::destroy();
   }

MODULE_END;


BadWordFilter *gBadWordFilter = NULL;
bool BadWordFilter::filteringEnabled = true;

BadWordFilter::BadWordFilter()
{
   VECTOR_SET_ASSOCIATION( filterTables );

   dStrcpy(defaultReplaceStr, "knqwrtlzs");
   filterTables.push_back(new FilterTable);
   curOffset = 0;
}

BadWordFilter::~BadWordFilter()
{
   for(U32 i = 0; i < filterTables.size(); i++)
      delete filterTables[i];
}

void BadWordFilter::create()
{
   Con::addVariable("pref::enableBadWordFilter", TypeBool, &filteringEnabled, 
      "@brief If true, the bad word filter will be enabled.\n\n"
	   "@ingroup Game");
   gBadWordFilter = new BadWordFilter;
   gBadWordFilter->addBadWord("shit");
   gBadWordFilter->addBadWord("fuck");
   gBadWordFilter->addBadWord("cock");
   gBadWordFilter->addBadWord("bitch");
   gBadWordFilter->addBadWord("cunt");
   gBadWordFilter->addBadWord("nigger");
   gBadWordFilter->addBadWord("bastard");
   gBadWordFilter->addBadWord("dick");
   gBadWordFilter->addBadWord("whore");
   gBadWordFilter->addBadWord("goddamn");
   gBadWordFilter->addBadWord("asshole");
}

void BadWordFilter::destroy()
{
   delete gBadWordFilter;
   gBadWordFilter = NULL;
}


U8 BadWordFilter::remapTable[257] = "------------------------------------------------OI---------------ABCDEFGHIJKLMNOPQRSTUVWXYZ------ABCDEFGHIJKLMNOPQRSTUVWXYZ-----C--F--TT--S-C-Z-----------S-C-ZY--CLOY-S-CA---R---UT-UP--IO-----AAAAAAACEEEEIIIIDNOOOOOXOUUUUYDBAAAAAAACEEEEIIIIDNOOOOO-OUUUUYDY";
U8 BadWordFilter::randomJunk[MaxBadwordLength+1] = "REMsg rk34n4ksqow;xnskq;KQoaWnZa";

BadWordFilter::FilterTable::FilterTable()
{
   for(U32 i = 0; i < 26; i++)
      nextState[i] = TerminateNotFound;
}

bool BadWordFilter::addBadWord(const char *cword)
{
   FilterTable *curFilterTable = filterTables[0];
   // prescan the word to see if it has any skip chars
   const U8 *word = (const U8 *) cword;
   const U8 *walk = word;
   if(dStrlen(cword) > MaxBadwordLength)
      return false;
   while(*walk)
   {
      if(remapTable[*walk] == '-')
         return false;
      walk++;
   }
   while(*word)
   {
      U8 remap = remapTable[*word] - 'A';
      U16 state = curFilterTable->nextState[remap];

      if(state < TerminateNotFound)
      {
         // this character is already in the state table...
         curFilterTable = filterTables[state];
      }
      else if(state == TerminateFound)
      {
         // a subset of this word is already in the table...
         // exit out.
         return false;
      }
      else if(state == TerminateNotFound)
      {
         if(word[1])
         {
            curFilterTable->nextState[remap] = filterTables.size();
            filterTables.push_back(new FilterTable);
            curFilterTable = filterTables[filterTables.size() - 1];
         }
         else
            curFilterTable->nextState[remap] = TerminateFound;
      }
      word++;
   }
   return true;
}

bool BadWordFilter::setDefaultReplaceStr(const char *str)
{
   U32 len = dStrlen(str);
   if(len < 2 || len >= sizeof(defaultReplaceStr))
      return false;
   dStrcpy(defaultReplaceStr, str);
   return true;
}

void BadWordFilter::filterString(char *cstring, const char *replaceStr)
{
   if(!replaceStr)
      replaceStr = defaultReplaceStr;
   U8 *string = (U8 *) cstring;
   U8 *starts[MaxBadwordLength];
   U8 *curStart = string;
   U32 replaceLen = dStrlen(replaceStr);
   while(*curStart)
   {
      FilterTable *curFilterTable = filterTables[0];
      S32 index = 0;
      U8 *walk = curStart;
      while(*walk)
      {
         U8 remap = remapTable[*walk];
         if(remap != '-')
         {
            starts[index++] = walk;
            U16 table = curFilterTable->nextState[remap - 'A'];
            if(table < TerminateNotFound)
               curFilterTable = filterTables[table];
            else if(table == TerminateNotFound)
            {
               curStart++;
               break;
            }
            else // terminate found
            {
               for(U32 i = 0; i < index; i++)
               {
                  starts[i][0] = (U8 )replaceStr[curOffset % replaceLen];
                  curOffset += randomJunk[curOffset & (MaxBadwordLength - 1)];
               }
               curStart = walk + 1;
               break;
            }
         }
         walk++;
      }
      if(!*walk)
         curStart++;
   }
}

bool BadWordFilter::containsBadWords(const char *cstring)
{
   U8 *string = (U8 *) cstring;
   U8 *curStart = string;
   while(*curStart)
   {
      FilterTable *curFilterTable = filterTables[0];
      U8 *walk = curStart;
      while(*walk)
      {
         U8 remap = remapTable[*walk];
         if(remap != '-')
         {
            U16 table = curFilterTable->nextState[remap - 'A'];
            if(table < TerminateNotFound)
               curFilterTable = filterTables[table];
            else if(table == TerminateNotFound)
            {
               curStart++;
               break;
            }
            else // terminate found
               return true;
         }
         walk++;
      }
      if(!*walk)
         curStart++;
   }
   return false;
}

DefineEngineFunction(addBadWord, bool, (const char* badWord),,
   "@brief Add a string to the bad word filter\n\n"

   "The bad word filter is a table containing words which will not be "
   "displayed in chat windows. Instead, a designated replacement string will be displayed.  "
   "There are already a number of bad words automatically defined.\n\n"

   "@param badWord Exact text of the word to restrict.\n"
   "@return True if word was successfully added, false if the word or a subset of it already exists in the table\n"

   "@see filterString()\n\n"

   "@tsexample\n"
      "// In this game, \"Foobar\" is banned\n"
      "%badWord = \"Foobar\";\n\n"
      "// Returns true, word was successfully added\n"
      "addBadWord(%badWord);\n\n"
      "// Returns false, word has already been added\n"
      "addBadWord(\"Foobar\");"
   "@endtsexample\n"

   "@ingroup Game")
{
	return gBadWordFilter->addBadWord(badWord);
}

DefineEngineFunction(filterString, const char *, (const char* baseString, const char* replacementChars), (NULL, NULL),
   "@brief Replaces the characters in a string with designated text\n\n"

   "Uses the bad word filter to determine which characters within the string will be replaced.\n\n"

   "@param baseString  The original string to filter.\n"
   "@param replacementChars A string containing letters you wish to swap in the baseString.\n"
   "@return The new scrambled string \n"

   "@see addBadWord()\n"
   "@see containsBadWords()\n"

   "@tsexample\n"
      "// Create the base string, can come from anywhere\n"
      "%baseString = \"Foobar\";\n\n"
      "// Create a string of random letters\n"
      "%replacementChars = \"knqwrtlzs\";\n\n"
      "// Filter the string\n"
      "%newString = filterString(%baseString, %replacementChars);\n\n"
      "// Print the new string to console\n"
      "echo(%newString);"
   "@endtsexample\n"

   "@ingroup Game")
{
	const char *replaceStr = NULL;

	if(replacementChars)
		replaceStr = replacementChars;
	else
		replaceStr = gBadWordFilter->getDefaultReplaceStr();

	char *ret = Con::getReturnBuffer(dStrlen(baseString) + 1);
	dStrcpy(ret, baseString);
	gBadWordFilter->filterString(ret, replaceStr);
	return ret;
}

DefineEngineFunction(containsBadWords, bool, (const char* text),,
   "@brief Checks to see if text is a bad word\n\n"

   "The text is considered to be a bad word if it has been added to the bad word filter.\n\n"

   "@param text Text to scan for bad words\n"
   "@return True if the text has bad word(s), false if it is clean\n"

   "@see addBadWord()\n"
   "@see filterString()\n"

   "@tsexample\n"
      "// In this game, \"Foobar\" is banned\n"
      "%badWord = \"Foobar\";\n\n"
      "// Add a banned word to the bad word filter\n"
      "addBadWord(%badWord);\n\n"
      "// Create the base string, can come from anywhere like user chat\n"
      "%userText = \"Foobar\";\n\n"
      "// Create a string of random letters\n"
      "%replacementChars = \"knqwrtlzs\";\n\n"
      "// If the text contains a bad word, filter it before printing\n"
      "// Otherwise print the original text\n"
      "if(containsBadWords(%userText))\n"
      "{\n"
      "	// Filter the string\n"
      "	%filteredText = filterString(%userText, %replacementChars);\n\n"
      "	// Print filtered text\n"
      "	echo(%filteredText);\n"
      "}\n"
      "else\n"
      "	echo(%userText);\n\n"
   "@endtsexample\n"

   "@ingroup Game")
{
	return gBadWordFilter->containsBadWords(text);
}

