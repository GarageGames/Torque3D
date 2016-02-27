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

#ifndef _TAML_ASSET_REFERENCED_UPDATE_VISITOR_H_
#define _TAML_ASSET_REFERENCED_UPDATE_VISITOR_H_

#ifndef _TAML_VISITOR_H_
#include "persistence/taml/tamlVisitor.h"
#endif

#ifndef _TAML_PARSER_H_
#include "persistence\/taml/tamlParser.h"
#endif

#ifndef _STRINGUNIT_H_
#include "string/stringUnit.h"
#endif

#ifndef _STRINGTABLE_H_
#include "string/stringTable.h"
#endif

#ifndef _ASSET_FIELD_TYPES_H_
#include "assets/assetFieldTypes.h"
#endif

// Debug Profiling.
#include "platform/profiler.h"

//-----------------------------------------------------------------------------

class TamlAssetReferencedUpdateVisitor : public TamlVisitor
{
private:    
    StringTableEntry mAssetIdFrom;
    StringTableEntry mAssetIdTo;

public:
    TamlAssetReferencedUpdateVisitor() {}
    virtual ~TamlAssetReferencedUpdateVisitor() {}

    void setAssetIdFrom( const char* pAssetIdFrom )
    {
        // Sanity!
        AssertFatal( pAssetIdFrom != NULL, "Asset Id from cannot be NULL." );

        mAssetIdFrom = StringTable->insert( pAssetIdFrom );
    }
    StringTableEntry getAssetIdFrom( void ) const { return mAssetIdFrom; }

    void setAssetIdTo( const char* pAssetIdTo )
    {
        // Sanity!
        AssertFatal( pAssetIdTo != NULL, "Asset Id to cannot be NULL." );

        mAssetIdTo = StringTable->insert( pAssetIdTo );
    }
    const char* getAssetIdTo( void ) const { return mAssetIdTo; }

    virtual bool wantsPropertyChanges( void ) { return true; }
    virtual bool wantsRootOnly( void ) { return false; }

    virtual bool visit( const TamlParser& parser, TamlVisitor::PropertyState& propertyState )
    {
        // Debug Profiling.
        PROFILE_SCOPE(TamlAssetReferencedUpdateVisitor_Visit);

        // Fetch the property value.
        const char* pPropertyValue = propertyState.getPropertyValue();

        // Fetch property value word count.
        const U32 valueWordCount = StringUnit::getUnitCount( pPropertyValue, ASSET_ASSIGNMENT_TOKEN );

        // Finish if not two words.
        if ( valueWordCount != 2 )
            return true;

        // Finish if this is not an asset signature.
        if ( dStricmp( StringUnit::getUnit( pPropertyValue, 0, ASSET_ASSIGNMENT_TOKEN), assetLooseIdSignature ) != 0 )
            return true;

        // Get the asset value.
        const char* pAssetValue = StringUnit::getUnit( pPropertyValue, 1, ASSET_ASSIGNMENT_TOKEN );

        // Finish if not the asset Id we're looking for.
        if ( dStricmp( pAssetValue, mAssetIdFrom ) != 0 )
            return true;

        // Is the target asset empty?
        if ( mAssetIdTo == StringTable->EmptyString() )
        {
            // Yes, so update the property as empty.
           propertyState.updatePropertyValue(StringTable->EmptyString());
            return true;
        }

        // Format asset.
        char assetBuffer[1024];
        dSprintf( assetBuffer, sizeof(assetBuffer), "%s%s%s", assetLooseIdSignature, ASSET_ASSIGNMENT_TOKEN, mAssetIdTo );

        // Assign new value.
        propertyState.updatePropertyValue( assetBuffer );

        return true;
    }
};

#endif // _TAML_ASSET_REFERENCED_UPDATE_VISITOR_H_
