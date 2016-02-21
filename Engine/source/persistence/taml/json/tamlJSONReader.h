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

#ifndef _TAML_JSONREADER_H_
#define _TAML_JSONREADER_H_

#ifndef _TDICTIONARY_H_
#include "core/util/tDictionary.h"
#endif

#ifndef _TAML_H_
#include "persistence/taml/taml.h"
#endif

/// RapidJson.
#include "persistence/rapidjson/document.h"
#include "persistence/rapidjson/prettywriter.h"

//-----------------------------------------------------------------------------

/// @ingroup tamlGroup
/// @see tamlGroup
class TamlJSONReader
{
public:
    TamlJSONReader( Taml* pTaml ) :
        mpTaml( pTaml )
    {}

    virtual ~TamlJSONReader() {}

    /// Read.
    SimObject* read( FileStream& stream );

private:
    Taml* mpTaml;

    typedef HashTable<SimObjectId, SimObject*> typeObjectReferenceHash;
    typeObjectReferenceHash mObjectReferenceMap;

private:
    void resetParse( void );

    SimObject* parseType( const rapidjson::Value::ConstMemberIterator& memberItr );
    inline void parseField( rapidjson::Value::ConstMemberIterator& memberItr, SimObject* pSimObject );
    inline void parseChild( rapidjson::Value::ConstMemberIterator& memberItr, SimObject* pSimObject );
    inline void parseCustom( rapidjson::Value::ConstMemberIterator& memberItr, SimObject* pSimObject, const char* pCustomNodeName, TamlCustomNodes& customNodes );
    inline void parseCustomNode( rapidjson::Value::ConstMemberIterator& memberItr, TamlCustomNode* pCustomNode );

    inline StringTableEntry getDemangledName( const char* pMangledName );
    inline bool parseStringValue( char* pBuffer, const S32 bufferSize, const rapidjson::Value& value, const char* pName );
    inline U32 getTamlRefId( const rapidjson::Value& value );
    inline U32 getTamlRefToId( const rapidjson::Value& value );
    inline const char* getTamlObjectName( const rapidjson::Value& value );   
};

#endif // _TAML_JSONREADER_H_