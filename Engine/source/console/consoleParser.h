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

#ifndef _CONSOLE_PARSER_H_
#define _CONSOLE_PARSER_H_

#include <stdio.h>

/// @ingroup console_system Console System
/// @{

namespace Compiler
{

//-----------------------------------------------------------------------------
/// \brief Function for GetCurrentFile from the lexer
//-----------------------------------------------------------------------------
typedef const char *(*fnGetCurrentFile)();
//-----------------------------------------------------------------------------
/// \brief Function for GetCurrentLine from the lexer
//-----------------------------------------------------------------------------
typedef S32 (*fnGetCurrentLine)();
//-----------------------------------------------------------------------------
/// \brief Function for Parse from the lexer
//-----------------------------------------------------------------------------
typedef S32 (*fnParse)();
//-----------------------------------------------------------------------------
/// \brief Function for Restart from the lexer
//-----------------------------------------------------------------------------
typedef void (*fnRestart)(FILE *input_file);
//-----------------------------------------------------------------------------
/// \brief Function for SetScanBuffer from the lexer
//-----------------------------------------------------------------------------
typedef void (*fnSetScanBuffer)(const char *sb, const char *fn);

//-----------------------------------------------------------------------------
/// \brief List of parsers for the compiler
//-----------------------------------------------------------------------------
struct ConsoleParser
{
	struct ConsoleParser *next;       //!< Next object in list or NULL

	char *ext;                        //!< Filename extension handled by this parser
	
	fnGetCurrentFile getCurrentFile;  //!< GetCurrentFile lexer function
	fnGetCurrentLine getCurrentLine;  //!< GetCurrentLine lexer function
	fnParse          parse;           //!< Parse lexer function
	fnRestart        restart;         //!< Restart lexer function
	fnSetScanBuffer  setScanBuffer;   //!< SetScanBuffer lexer function
};

// Macros

//-----------------------------------------------------------------------------
/// \brief Declare a parser's function prototypes
//-----------------------------------------------------------------------------
#define CON_DECLARE_PARSER(prefix) \
	const char * prefix##GetCurrentFile(); \
	S32 prefix##GetCurrentLine(); \
	void prefix##SetScanBuffer(const char *sb, const char *fn); \
	S32 prefix##parse(); \
	void prefix##restart(FILE *input_file)

//-----------------------------------------------------------------------------
/// \brief Helper macro to add console parsers
//-----------------------------------------------------------------------------
#define CON_ADD_PARSER(prefix, ext, def) \
	Compiler::addConsoleParser(ext, prefix##GetCurrentFile, prefix##GetCurrentLine, prefix##parse, \
						  prefix##restart, prefix##SetScanBuffer, def)

//-----------------------------------------------------------------------------
/// \brief Free the console parser list
/// 
/// \sa AddConsoleParser()
//-----------------------------------------------------------------------------
void freeConsoleParserList(void);

//-----------------------------------------------------------------------------
/// \brief Add a console parser to the list
/// 
/// \param ext Filename extension
/// \param gcf GetCurrentFile function
/// \param gcl GetCurrentLine function
/// \param p Parse function
/// \param r Restart function
/// \param ssb SetScanBuffer function
/// \param def true if this is the default parser (<b>Note:</b> set this only on the .cs parser!)
/// \return true for success, false for failure (out of memory)
/// \sa FreeConsoleParserList(), ConsoleParser
//-----------------------------------------------------------------------------
bool addConsoleParser(char *ext, fnGetCurrentFile gcf, fnGetCurrentLine gcl, fnParse p, fnRestart r, fnSetScanBuffer ssb, bool def = false);

//-----------------------------------------------------------------------------
/// \brief Get the parser for a particular file based on its extension
/// 
/// \param filename Filename of file to obtain parser for
/// \sa ConsoleParser
//-----------------------------------------------------------------------------
ConsoleParser * getParserForFile(const char *filename);

} // end namespace Con

/// @}

#endif // _CONSOLE_PARSER_H_
