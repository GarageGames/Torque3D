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

#ifndef _TAML_BINARYREADER_H_
#define _TAML_BINARYREADER_H_

#ifndef _TDICTIONARY_H_
#include "core/util/tDictionary.h"
#endif

#ifndef _TAML_H_
#include "persistence/taml/taml.h"
#endif

//-----------------------------------------------------------------------------

/// @ingroup tamlGroup
/// @see tamlGroup
class TamlBinaryReader
{
public:
    TamlBinaryReader( Taml* pTaml ) :
        mpTaml( pTaml )
    {
    }

    virtual ~TamlBinaryReader() {}

    /// Read.
    SimObject* read( FileStream& stream );

private:
    Taml* mpTaml;

    typedef HashTable<SimObjectId, SimObject*> typeObjectReferenceHash;

    typeObjectReferenceHash mObjectReferenceMap;

private:
    void resetParse( void );

    SimObject* parseElement( Stream& stream, const U32 versionId );
    void parseAttributes( Stream& stream, SimObject* pSimObject, const U32 versionId );
    void parseChildren( Stream& stream, TamlCallbacks* pCallbacks, SimObject* pSimObject, const U32 versionId );
    void parseCustomElements( Stream& stream, TamlCallbacks* pCallbacks, TamlCustomNodes& customNodes, const U32 versionId );
    void parseCustomNode( Stream& stream, TamlCustomNode* pCustomNode, const U32 versionId );
};

#endif // _TAML_BINARYREADER_H_