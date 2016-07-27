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

#ifndef _TAML_XMLREADER_H_
#define _TAML_XMLREADER_H_

#ifndef _TDICTIONARY_H_
#include "core/util/tDictionary.h"
#endif

#ifndef _TAML_H_
#include "persistence/taml/taml.h"
#endif

#ifndef TINYXML_INCLUDED
#include "tinyXML/tinyxml.h"
#endif

//-----------------------------------------------------------------------------

/// @ingroup tamlGroup
/// @see tamlGroup
class TamlXmlReader
{
public:
    TamlXmlReader( Taml* pTaml ) :
        mpTaml( pTaml )
    {}

    virtual ~TamlXmlReader() {}

    /// Read.
    SimObject* read( FileStream& stream );

private:
    Taml* mpTaml;

    typedef HashTable<SimObjectId, SimObject*> typeObjectReferenceHash;
    typeObjectReferenceHash mObjectReferenceMap;

private:
    void resetParse( void );

    SimObject* parseElement( TiXmlElement* pXmlElement );
    void parseAttributes( TiXmlElement* pXmlElement, SimObject* pSimObject );
    void parseCustomElement( TiXmlElement* pXmlElement, TamlCustomNodes& pCustomNode );
    void parseCustomNode( TiXmlElement* pXmlElement, TamlCustomNode* pCustomNode );

    U32 getTamlRefId( TiXmlElement* pXmlElement );
    U32 getTamlRefToId( TiXmlElement* pXmlElement );
    const char* getTamlObjectName( TiXmlElement* pXmlElement );   
};

#endif // _TAML_XMLREADER_H_