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
#include "console/consoleParser.h"

#include "core/strings/stringFunctions.h"
#include "console/console.h"


namespace Compiler
{

static ConsoleParser *gParserList = NULL;
static ConsoleParser *gDefaultParser = NULL;

void freeConsoleParserList(void)
{
	while(gParserList)
	{
      ConsoleParser * pParser = gParserList;
		gParserList = pParser->next;
		delete pParser;
	}

	gDefaultParser = NULL;
}

bool addConsoleParser(char *ext, fnGetCurrentFile gcf, fnGetCurrentLine gcl, fnParse p, fnRestart r, fnSetScanBuffer ssb, bool def)
{
	AssertFatal(ext && gcf && gcl && p && r, "AddConsoleParser called with one or more NULL arguments");

	ConsoleParser * pParser = new ConsoleParser;

   pParser->ext = ext;
   pParser->getCurrentFile = gcf;
   pParser->getCurrentLine = gcl;
   pParser->parse = p;
   pParser->restart = r;
   pParser->setScanBuffer = ssb;

   if (def)
      gDefaultParser = pParser;

   pParser->next = gParserList;
   gParserList = pParser;

   return true;
}

ConsoleParser * getParserForFile(const char *filename)
{
	if(filename == NULL)
		return gDefaultParser;

	char *ptr = dStrrchr((char *)filename, '.');
	if(ptr != NULL)
	{
		ptr++;

		ConsoleParser *p;
		for(p = gParserList; p; p = p->next)
		{
			if(dStricmp(ptr, p->ext) == 0)
				return p;
		}
	}

	return gDefaultParser;
}

} // end namespace Con
