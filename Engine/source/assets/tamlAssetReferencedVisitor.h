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

#ifndef _TAML_ASSET_REFERENCED_VISITOR_H_
#define _TAML_ASSET_REFERENCED_VISITOR_H_

#ifndef _TAML_VISITOR_H_
#include "persistence/taml/tamlVisitor.h"
#endif

#ifndef _TAML_PARSER_H_
#include "persistence/taml/tamlParser.h"
#endif

#ifndef _ASSET_FIELD_TYPES_H_
#include "assets/assetFieldTypes.h"
#endif

// Debug Profiling.
#include "platform/profiler.h"

//-----------------------------------------------------------------------------

class TamlAssetReferencedVisitor : public TamlVisitor
{
public:
    typedef StringTableEntry typeAssetId;
    typedef HashMap<typeAssetId, StringTableEntry> typeAssetReferencedHash;

private:
    typeAssetReferencedHash mAssetReferenced;

public:
    TamlAssetReferencedVisitor() {}
    virtual ~TamlAssetReferencedVisitor() {}

    const typeAssetReferencedHash& getAssetReferencedMap( void ) const { return mAssetReferenced; }

    void clear( void ) { mAssetReferenced.clear(); }

    virtual bool wantsPropertyChanges( void ) { return false; }
    virtual bool wantsRootOnly( void ) { return false; }

    virtual bool visit( const TamlParser& parser, TamlVisitor::PropertyState& propertyState )
    {    
        // Debug Profiling.
        PROFILE_SCOPE(TamlAssetReferencedVisitor_Visit);

        // Fetch property value.
        const char* pPropertyValue = propertyState.getPropertyValue();

        // Fetch property word count.
        const U32 propertyWordCount = StringUnit::getUnitCount( pPropertyValue, ASSET_ASSIGNMENT_TOKEN );

        // Finish if there's not two words.
        if ( propertyWordCount != 2 )
            return true;

        // Finish if the first word is not an asset signature.
        if ( StringTable->insert( StringUnit::getUnit( pPropertyValue, 0, ASSET_ASSIGNMENT_TOKEN ) ) != assetLooseIdSignature )
            return true;

        // Get asset Id.
        typeAssetId assetId = StringTable->insert( StringUnit::getUnit( pPropertyValue, 1, ASSET_ASSIGNMENT_TOKEN ) );

        // Finish if we already have this asset Id.
        if ( mAssetReferenced.contains( assetId ) )
            return true;

        // Insert asset reference.
        mAssetReferenced.insert( assetId, StringTable->EmptyString() );

        return true;
    }
};

#endif // _TAML_ASSET_REFERENCED_VISITOR_H_
