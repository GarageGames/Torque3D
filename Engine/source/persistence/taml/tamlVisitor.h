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

#ifndef _TAML_VISITOR_H_
#define _TAML_VISITOR_H_

#include "core/stringTable.h"
#include "core/strings/stringFunctions.h"

//-----------------------------------------------------------------------------

class TamlParser;

//-----------------------------------------------------------------------------

/// @ingroup tamlGroup
/// @see tamlGroup
class TamlVisitor
{
public:
    /// Visitor property state.
    struct PropertyState
    {
    private:
        StringTableEntry    mObjectName;
        StringTableEntry    mPropertyName;
        char                mPropertyValue[4096];
        bool                mPropertyValueDirty;
        bool                mIsRootObject;

    public:
        PropertyState()
        {
            mObjectName = StringTable->EmptyString();
            mPropertyName = StringTable->EmptyString();
            *mPropertyValue = 0;
            mPropertyValueDirty = false;
            mIsRootObject = false;
        }

        inline void setObjectName( const char* pObjectName, const bool rootObject )
        {
            mObjectName = StringTable->insert( pObjectName );
            mIsRootObject = rootObject;
        }
        inline StringTableEntry getObjectName( void ) const { return mObjectName; }
        inline bool isRootObject( void ) const { return mIsRootObject; }

        inline void setProperty( const char* pPropertyName, const char* pPropertyValue )
        {
            // Set property name.
            mPropertyName = StringTable->insert( pPropertyName );

            // Format property value.
            dSprintf( mPropertyValue, sizeof(mPropertyValue), "%s", pPropertyValue );

            // Flag as not dirty.
            mPropertyValueDirty = false;
        }

        inline void updatePropertyValue( const char* pPropertyValue )
        {
            // Update property value.
            dSprintf( mPropertyValue, sizeof(mPropertyValue), "%s", pPropertyValue );

            // Flag as dirty.
            mPropertyValueDirty = true;
        }

        inline StringTableEntry getPropertyName( void ) const { return mPropertyName; }
        inline const char* getPropertyValue( void ) const { return mPropertyValue; }

        inline void resetPropertyValueDirty( void ) { mPropertyValueDirty = false; }
        inline bool getPropertyValueDirty( void ) const { return mPropertyValueDirty; }
    };

public:
    TamlVisitor() {}
    virtual ~TamlVisitor() {}

    /// Whether the visitor wants to perform property changes.
    /// This allows a check against the parser which may not allow changes.
    virtual bool wantsPropertyChanges( void ) = 0;

    /// Whether the visitor wants to visit the root node only.
    virtual bool wantsRootOnly( void ) = 0;

    /// The state of the visited property.
    virtual bool visit( const TamlParser& parser, PropertyState& propertyState ) = 0;
};

#endif // _TAML_VISITOR_H_
