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

#ifndef _TAML_JSONWRITER_H_
#define _TAML_JSONWRITER_H_

#ifndef _TAML_H_
#include "persistence/taml/taml.h"
#endif

/// RapidJson.
#include "persistence/rapidjson/document.h"
#include "persistence/rapidjson/prettywriter.h"

//-----------------------------------------------------------------------------

extern StringTableEntry JSON_RFC4627_NAME_MANGLING_CHARACTERS;
extern StringTableEntry JSON_RFC4627_NAME_MANGLING_FORMAT;

//-----------------------------------------------------------------------------

/// @ingroup tamlGroup
/// @see tamlGroup
class TamlJSONWriter
{
public:
    TamlJSONWriter( Taml* pTaml ) :
        mpTaml( pTaml )
    {}
    virtual ~TamlJSONWriter() {}

    /// Write.
    bool write( FileStream& stream, const TamlWriteNode* pTamlWriteNode );

private:
    Taml* mpTaml;

private:
    void compileType( rapidjson::Document& document, rapidjson::Value* pTypeValue, rapidjson::Value* pParentValue, const TamlWriteNode* pTamlWriteNode, const S32 memberIndex );
    void compileFields( rapidjson::Document& document, rapidjson::Value* pTypeValue, const TamlWriteNode* pTamlWriteNode );
    void compileCustom( rapidjson::Document& document, rapidjson::Value* pTypeValue, const TamlWriteNode* pTamlWriteNode );
    void compileCustomNode( rapidjson::Document& document, rapidjson::Value* pParentValue, const TamlCustomNode* pCustomNode, const S32 memberIndex );

    inline StringTableEntry getManagedName(const char* pName, const S32 memberIndex );
};

#endif // _TAML_JSONWRITER_H_