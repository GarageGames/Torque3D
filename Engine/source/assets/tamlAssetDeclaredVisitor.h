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

#ifndef _TAML_ASSET_DECLARED_VISITOR_H_
#define _TAML_ASSET_DECLARED_VISITOR_H_

#ifndef _TAML_VISITOR_H_
#include "persistence/taml/tamlVisitor.h"
#endif

#ifndef _TAML_PARSER_H_
#include "persistence\/taml/tamlParser.h"
#endif

#ifndef _ASSET_FIELD_TYPES_H_
#include "assets/assetFieldTypes.h"
#endif

#ifndef _ASSET_DEFINITION_H_
#include "assetDefinition.h"
#endif

#ifndef _ASSET_BASE_H_
#include "assetBase.h"
#endif

// Debug Profiling.
#include "platform/profiler.h"

//-----------------------------------------------------------------------------

class TamlAssetDeclaredVisitor : public TamlVisitor
{
public:
    typedef StringTableEntry typeAssetId;
    typedef Vector<typeAssetId> typeAssetIdVector;
    typedef Vector<StringTableEntry> typeLooseFileVector;

private:
    AssetDefinition         mAssetDefinition;
    typeAssetIdVector       mAssetDependencies;
    typeLooseFileVector     mAssetLooseFiles;

public:
    TamlAssetDeclaredVisitor() { mAssetDefinition.reset(); }
    virtual ~TamlAssetDeclaredVisitor() {}


    inline AssetDefinition& getAssetDefinition( void ) { return mAssetDefinition; }
    inline typeAssetIdVector& getAssetDependencies( void ) { return mAssetDependencies; }
    inline typeLooseFileVector& getAssetLooseFiles( void ) { return mAssetLooseFiles; }

    void clear( void ) { mAssetDefinition.reset(); mAssetDependencies.clear(); mAssetLooseFiles.clear(); }

    virtual bool wantsPropertyChanges( void ) { return false; }
    virtual bool wantsRootOnly( void ) { return false; }

    virtual bool visit( const TamlParser& parser, TamlVisitor::PropertyState& propertyState )
    {    
        // Debug Profiling.
        PROFILE_SCOPE(TamlAssetDeclaredVisitor_Visit);

        // Fetch property name and value.
        StringTableEntry propertyName = propertyState.getPropertyName();
        const char* pPropertyValue = propertyState.getPropertyValue();

        // Is this the root object?
        if ( propertyState.isRootObject() )
        {
            // Yes, so is the asset type set yet?
            if ( mAssetDefinition.mAssetType == StringTable->EmptyString() )
            {
                // No, set set asset type and base file-path.
                mAssetDefinition.mAssetType = propertyState.getObjectName();
                mAssetDefinition.mAssetBaseFilePath = parser.getParsingFilename();
            }

            // Asset name?
            if ( propertyName == assetNameField )
            {
                // Yes, so assign it.
                mAssetDefinition.mAssetName = StringTable->insert( pPropertyValue );
                return true;
            }
            // Asset description?
            else if ( propertyName == assetDescriptionField )
            {
                // Yes, so assign it.
                mAssetDefinition.mAssetDescription = StringTable->insert( pPropertyValue );
                return true;
            }
            // Asset description?
            else if ( propertyName == assetCategoryField )
            {
                // Yes, so assign it.
                mAssetDefinition.mAssetCategory = StringTable->insert( pPropertyValue );
                return true;
            }
            // Asset auto-unload?
            else if ( propertyName == assetAutoUnloadField )
            {
                // Yes, so assign it.
                mAssetDefinition.mAssetAutoUnload = dAtob( pPropertyValue );
                return true;
            }
            // Asset internal?
            else if ( propertyName == assetInternalField )
            {
                // Yes, so assign it.
                mAssetDefinition.mAssetInternal = dAtob( pPropertyValue );
                return true;
            }
        }

        // Fetch property word count.
        const U32 propertyWordCount = StringUnit::getUnitCount( pPropertyValue, ASSET_ASSIGNMENT_TOKEN );

        // Finish if there's not two words.
        if ( propertyWordCount != 2 )
            return true;

        // Fetch the asset signature.
        StringTableEntry assetSignature = StringTable->insert( StringUnit::getUnit( pPropertyValue, 0, ASSET_ASSIGNMENT_TOKEN ) );

        // Is this an asset Id signature?
        if ( assetSignature == assetLooseIdSignature )
        {
            // Yes, so get asset Id.
            typeAssetId assetId = StringTable->insert( StringUnit::getUnit( pPropertyValue, 1, ASSET_ASSIGNMENT_TOKEN ) );

            // Finish if the dependency is itself!
            if ( mAssetDefinition.mAssetId == assetId )
                return true;

            // Iterate existing dependencies.
            for( typeAssetIdVector::iterator dependencyItr = mAssetDependencies.begin(); dependencyItr != mAssetDependencies.end(); ++dependencyItr )
            {
                // Finish if asset Id is already a dependency.
                if ( *dependencyItr == assetId )
                    return true;
            }

            // Insert asset reference.
            mAssetDependencies.push_back( assetId );
        }
        // Is this a loose-file signature?
        else if ( assetSignature == assetLooseFileSignature )
        {
            // Yes, so get loose-file reference.
            const char* pAssetLooseFile = StringUnit::getUnit( pPropertyValue, 1, ASSET_ASSIGNMENT_TOKEN );

            // Fetch asset path only.
            char assetBasePathBuffer[1024];
            dSprintf( assetBasePathBuffer, sizeof(assetBasePathBuffer), "%s", mAssetDefinition.mAssetBaseFilePath );
            char* pFinalSlash = dStrrchr( assetBasePathBuffer, '/' );
            if ( pFinalSlash != NULL ) *pFinalSlash = 0;

            // Expand the path in the usual way.
            char assetFilePathBuffer[1024];
            Con::expandPath( assetFilePathBuffer, sizeof(assetFilePathBuffer), pAssetLooseFile, assetBasePathBuffer );

            // Insert asset loose-file.
            mAssetLooseFiles.push_back( StringTable->insert( assetFilePathBuffer ) );
        }

        return true;
    }
};

#endif // _TAML_ASSET_DECLARED_VISITOR_H_
