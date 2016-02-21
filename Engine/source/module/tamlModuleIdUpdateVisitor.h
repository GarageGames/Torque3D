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

#ifndef _TAML_MODULE_ID_UPDATE_VISITOR_H_
#define _TAML_MODULE_ID_UPDATE_VISITOR_H_

#ifndef _TAML_VISITOR_H_
#include "persistence/taml/tamlVisitor.h"
#endif

#ifndef _TAML_PARSER_H_
#include "persistence/taml/tamlParser.h"
#endif

#ifndef _ASSET_FIELD_TYPES_H_
#include "assets/assetFieldTypes.h"
#endif

#include "platform/profiler.h"

//-----------------------------------------------------------------------------

class TamlModuleIdUpdateVisitor : public TamlVisitor
{
private:    
    StringTableEntry mModuleIdFrom;
    StringTableEntry mModuleIdTo;
    U32 mModuleIdFromLength;
    U32 mModuleIdToLength;

public:
    TamlModuleIdUpdateVisitor() :
        mModuleIdFrom( StringTable->EmptyString() ),
        mModuleIdTo(StringTable->EmptyString()),
        mModuleIdFromLength( 0 ),
        mModuleIdToLength( 0 )      
        {}
    virtual ~TamlModuleIdUpdateVisitor() {}

    virtual bool wantsPropertyChanges( void ) { return true; }
    virtual bool wantsRootOnly( void ) { return true; }

    virtual bool visit( const TamlParser& parser, TamlVisitor::PropertyState& propertyState )
    {
        // Debug Profiling.
        PROFILE_SCOPE(TamlModuleIdUpdateVisitor_Visit);

        // Fetch property value.
        const char* pPropertyValue = propertyState.getPropertyValue();

        // Fetch value length.
        const U32 valueLenth = dStrlen(pPropertyValue);

        char newAttributeValueBuffer[1024];

        // Is this an expando?
        if ( *pPropertyValue == '^' )
        {
            // Yes, so finish if it's not the correct length.
            if ( valueLenth < mModuleIdFromLength+1 )
                return true;

            // Is this the module Id?
            if ( dStrnicmp( pPropertyValue+1, mModuleIdFrom, mModuleIdFromLength ) == 0 )
            {
                // Yes, so format a new value.
                dSprintf( newAttributeValueBuffer, sizeof(newAttributeValueBuffer), "^%s%s",
                    mModuleIdTo, pPropertyValue+1+mModuleIdFromLength );

                // Assign new value.
                propertyState.updatePropertyValue( newAttributeValueBuffer );
            }

            return true;
        }

        // Does the field start with the module Id?
        if ( dStrnicmp( pPropertyValue, mModuleIdFrom, mModuleIdFromLength ) == 0 )
        {
            // Yes, so format a new value.
            dSprintf( newAttributeValueBuffer, sizeof(newAttributeValueBuffer), "%s%s",
                mModuleIdTo, pPropertyValue+mModuleIdFromLength );

            // Assign new value.
            propertyState.updatePropertyValue( newAttributeValueBuffer );
        }

        return true;
    }

    void setModuleIdFrom( const char* pModuleIdFrom )
    {
        // Sanity!
        AssertFatal( pModuleIdFrom != NULL, "Module Id from cannot be NULL." );

        // Set module Id.
        mModuleIdFrom = StringTable->insert( pModuleIdFrom );
        mModuleIdFromLength = dStrlen(mModuleIdFrom);
    }
    StringTableEntry getModuleIdFrom( void ) const { return mModuleIdFrom; }

    void setModuleIdTo( const char* pModuleIdTo )
    {
        // Sanity!
        AssertFatal( pModuleIdTo != NULL, "Module Id to cannot be NULL." );

        // Set module Id.
        mModuleIdTo = StringTable->insert( pModuleIdTo );
        mModuleIdToLength = dStrlen(mModuleIdTo);
    }
    const char* getModuleIdTo( void ) const { return mModuleIdTo; }
};

#endif // _TAML_MODULE_ID_UPDATE_VISITOR_H_
