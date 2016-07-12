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

#ifndef _TAML_WRITE_NODE_H_
#define _TAML_WRITE_NODE_H_

#ifndef _TAML_CUSTOM_H_
#include "persistence/taml/tamlCustom.h"
#endif

#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif

#ifndef _VECTOR_H_
#include "core/util/tVector.h"
#endif

//-----------------------------------------------------------------------------

class TamlCallbacks;

//-----------------------------------------------------------------------------

class TamlWriteNode
{
public:
    class FieldValuePair
    {
    public:        
        FieldValuePair( StringTableEntry name, const char* pValue )
        {
            // Set the field name.
            mName = name;

            // Allocate and copy the value.
            mpValue = new char[ dStrlen(pValue)+1 ];
            dStrcpy( (char *)mpValue, pValue );
        }
        

        StringTableEntry    mName;
        const char*         mpValue;
    };

public:
    TamlWriteNode()
    {
        // NOTE: This MUST be done before the state is reset otherwise we'll be touching uninitialized stuff.
        mRefToNode = NULL;
        mpSimObject = NULL;
        mpTamlCallbacks = NULL;
        mpObjectName = NULL;
        mChildren = NULL;

        resetNode();
    }

    void set( SimObject* pSimObject )
    {
        // Sanity!
        AssertFatal( pSimObject != NULL, "Cannot set a write node with a NULL sim object." );

        // Reset the node.
        resetNode();

        // Set sim object.
        mpSimObject = pSimObject;

        // Fetch name.
        const char* pObjectName = pSimObject->getName();

        // Assign name usage.
        if ( pObjectName != NULL &&
            pObjectName != StringTable->EmptyString() &&
            dStrlen( pObjectName ) > 0 )
        {
            mpObjectName = pObjectName;
        }

        // Find Taml callbacks.
        mpTamlCallbacks = dynamic_cast<TamlCallbacks*>( mpSimObject );
    }

    void resetNode( void );

    U32                         mRefId;
    TamlWriteNode*              mRefToNode;
    SimObject*                  mpSimObject;
    TamlCallbacks*              mpTamlCallbacks;
    const char*                 mpObjectName;
    Vector<TamlWriteNode::FieldValuePair*> mFields;
    Vector<TamlWriteNode*>*     mChildren;
    TamlCustomNodes             mCustomNodes;
};

#endif // _TAML_WRITE_NODE_H_