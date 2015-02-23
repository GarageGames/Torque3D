//-----------------------------------------------------------------------------
// Copyright (c) 2013 GarageGames, LLC
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

#ifndef _TAML_PARSER_H_
#define _TAML_PARSER_H_

#include "core/stringTable.h"

//-----------------------------------------------------------------------------

class TamlVisitor;

//-----------------------------------------------------------------------------

/// @ingroup tamlGroup
/// @see tamlGroup
class TamlParser
{
public:
    TamlParser() :
        mParsingFilename(StringTable->EmptyString())
    {}
    virtual ~TamlParser() {}

    /// Whether the parser can change a property or not.
    virtual bool canChangeProperty( void ) = 0;

    /// Accept visitor.
    virtual bool accept( const char* pFilename, TamlVisitor& visitor ) = 0;

    /// Filename.
    inline void setParsingFilename( const char* pFilename ) { mParsingFilename = StringTable->insert(pFilename); }
    inline StringTableEntry getParsingFilename( void ) const { return mParsingFilename; }

private:
    StringTableEntry mParsingFilename;
};

#endif // _TAML_PARSER_H_