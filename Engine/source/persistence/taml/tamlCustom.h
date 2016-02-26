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

#ifndef _TAML_CUSTOM_H_
#define _TAML_CUSTOM_H_

#ifndef _FACTORY_CACHE_H_
#include "core/factoryCache.h"
#endif

#ifndef _STRINGTABLE_H_
#include "core/stringTable.h"
#endif

#ifndef _CONSOLE_H_
#include "console/console.h"
#endif

#ifndef _CONSOLETYPES_H_
#include "console/consoleTypes.h"
#endif

#ifndef B2_MATH_H
//TODO: Look at this
//#include "box2d/Common/b2Math.h"
#endif

#ifndef _COLOR_H_
#include "core/color.h"
#endif

#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif

#include "core/util/safeDelete.h"

#include "math/mMath.h"

//-----------------------------------------------------------------------------

#define MAX_TAML_NODE_FIELDVALUE_LENGTH 2048

//-----------------------------------------------------------------------------

class TamlWriteNode;
class TamlCustomNode;
class TamlCustomField;
extern FactoryCache<TamlCustomNode> TamlCustomNodeFactory;
extern FactoryCache<TamlCustomField> TamlCustomFieldFactory;
typedef Vector<TamlCustomNode*> TamlCustomNodeVector;
typedef Vector<TamlCustomField*> TamlCustomFieldVector;

//-----------------------------------------------------------------------------

class TamlCustomField : public IFactoryObjectReset
{
public:
    TamlCustomField()
    {
        resetState();
    }

    virtual ~TamlCustomField()
    {
        // Everything should already be cleared in a state reset.
        // Touching any memory here is dangerous as this type is typically
        // held in a static factory cache until shutdown at which point
        // pretty much anything or everything could be invalid!
    }

    virtual void resetState( void )
    {
        mFieldName = StringTable->EmptyString();
        *mFieldValue = 0;
    }

    void set( const char* pFieldName, const char* pFieldValue );

    inline void setFieldValue( const char* pFieldName, const ColorI& fieldValue )
    {
        // Fetch the field value.
        const char* pFieldValue = Con::getData( TypeColorI, &const_cast<ColorI&>(fieldValue), 0 );

        // Did we get a field value?
        if ( pFieldValue == NULL )
        {
            // No, so warn.
            Con::warnf( "Taml: Failed to add node field name '%s' with ColorI value.", pFieldName );
            pFieldValue = StringTable->EmptyString();
        }

        set( pFieldName, pFieldValue );
    }

    inline void setFieldValue( const char* pFieldName, const ColorF& fieldValue )
    {
        // Fetch the field value.
        const char* pFieldValue = Con::getData( TypeColorF, &const_cast<ColorF&>(fieldValue), 0 );

        // Did we get a field value?
        if ( pFieldValue == NULL )
        {
            // No, so warn.
            Con::warnf( "Taml: Failed to add node field name '%s' with ColorF value.", pFieldName );
            pFieldValue = StringTable->EmptyString();
        }

        set( pFieldName, pFieldValue );
    }

    inline void setFieldValue( const char* pFieldName, const Point2I& fieldValue )
    {
        char fieldValueBuffer[32];
        dSprintf( fieldValueBuffer, sizeof(fieldValueBuffer), "%d %d", fieldValue.x, fieldValue.y );
        set( pFieldName, fieldValueBuffer );
    }

    inline void setFieldValue( const char* pFieldName, const Point2F& fieldValue )
    {
        char fieldValueBuffer[32];
        dSprintf( fieldValueBuffer, sizeof(fieldValueBuffer), "%.5g %0.5g", fieldValue.x, fieldValue.y );
        set( pFieldName, fieldValueBuffer );
    }

    inline void setFieldValue( const char* pFieldName, const Point3I& fieldValue )
    {
        char fieldValueBuffer[32];
        dSprintf( fieldValueBuffer, sizeof(fieldValueBuffer), "%d %d %d", fieldValue.x, fieldValue.y, fieldValue.z );
        set( pFieldName, fieldValueBuffer );
    }
    
    inline void setFieldValue( const char* pFieldName, const Point3F& fieldValue )
    {
        char fieldValueBuffer[32];
        dSprintf( fieldValueBuffer, sizeof(fieldValueBuffer), "%.5g %0.5g %.5g", fieldValue.x, fieldValue.y, fieldValue.z );
        set( pFieldName, fieldValueBuffer );
    }
    
    inline void setFieldValue( const char* pFieldName, const RectF& fieldValue )
    {
        char fieldValueBuffer[32];
        dSprintf( fieldValueBuffer, sizeof(fieldValueBuffer), "%.5g %0.5g %.5g %.5g", 
           fieldValue.point.x, fieldValue.point.y, fieldValue.extent.x, fieldValue.extent.y);
        set( pFieldName, fieldValueBuffer );
    }
    
    inline void setFieldValue( const char* pFieldName, const QuatF& fieldValue )
    {
        char fieldValueBuffer[32];
        dSprintf( fieldValueBuffer, sizeof(fieldValueBuffer), "%.5g %0.5g %.5g %.5g", fieldValue.x, fieldValue.y, fieldValue.z, fieldValue.w );
        set( pFieldName, fieldValueBuffer );
    }
    
    inline void setFieldValue( const char* pFieldName, const AngAxisF& fieldValue )
    {
        char fieldValueBuffer[32];
        dSprintf( fieldValueBuffer, sizeof(fieldValueBuffer), "%.5g %0.5g %.5g %.5g", fieldValue.axis.x, fieldValue.axis.y, fieldValue.axis.z, fieldValue.angle );
        set( pFieldName, fieldValueBuffer );
    }

    inline void setFieldValue( const char* pFieldName, const U32 fieldValue )
    {
        char fieldValueBuffer[16];
        dSprintf( fieldValueBuffer, sizeof(fieldValueBuffer), "%d", fieldValue );
        set( pFieldName, fieldValueBuffer );
    }

    inline void setFieldValue( const char* pFieldName, const bool fieldValue )
    {
        char fieldValueBuffer[16];
        dSprintf( fieldValueBuffer, sizeof(fieldValueBuffer), "%d", fieldValue );
        set( pFieldName, fieldValueBuffer );
    }

    inline void setFieldValue( const char* pFieldName, const S32 fieldValue )
    {
        char fieldValueBuffer[16];
        dSprintf( fieldValueBuffer, sizeof(fieldValueBuffer), "%d", fieldValue );
        set( pFieldName, fieldValueBuffer );
    }

    inline void setFieldValue( const char* pFieldName, const float fieldValue )
    {
        char fieldValueBuffer[16];
        dSprintf( fieldValueBuffer, sizeof(fieldValueBuffer), "%.5g", fieldValue );
        set( pFieldName, fieldValueBuffer );
    }

    inline void setFieldValue( const char* pFieldName, const char* fieldValue )
    {
        set( pFieldName, fieldValue );
    }

    inline void getFieldValue( ColorF& fieldValue ) const
    {
        fieldValue.set( 1.0f, 1.0f, 1.0f, 1.0f );

        // Set color.
        const char* argv = (char*)mFieldValue;
        Con::setData( TypeColorF, &fieldValue, 0, 1, &argv );
    }

    inline void getFieldValue( ColorI& fieldValue ) const
    {
        fieldValue.set( 255, 255, 255, 255 );

        // Set color.
        const char* argv = (char*)mFieldValue;
        Con::setData( TypeColorI, &fieldValue, 0, 1, &argv );
    }

    inline void getFieldValue( Point2I& fieldValue ) const
    {
        if ( dSscanf( mFieldValue, "%d %d", &fieldValue.x, &fieldValue.y ) != 2 )
        {
            // Warn.
            Con::warnf( "TamlCustomField - Reading point2I but it has an incorrect format: '%s'.", mFieldValue );
        }
    }

    inline void getFieldValue( Point2F& fieldValue ) const
    {
        if ( dSscanf( mFieldValue, "%g %g", &fieldValue.x, &fieldValue.y ) != 2 )
        {
            // Warn.
            Con::warnf( "TamlCustomField - Reading point2F but it has an incorrect format: '%s'.", mFieldValue );
        }
    }

    inline void getFieldValue( Point3I& fieldValue ) const
    {
       if ( dSscanf( mFieldValue, "%d %d %d", &fieldValue.x, &fieldValue.y, &fieldValue.z ) != 3 )
        {
            // Warn.
            Con::warnf( "TamlCustomField - Reading point3I but it has an incorrect format: '%s'.", mFieldValue );
        }
    }

    inline void getFieldValue( Point3F& fieldValue ) const
    {
        if ( dSscanf( mFieldValue, "%g %g %g", &fieldValue.x, &fieldValue.y, &fieldValue.z ) != 3 )
        {
            // Warn.
            Con::warnf( "TamlCustomField - Reading point3F but it has an incorrect format: '%s'.", mFieldValue );
        }
    }

    inline void getFieldValue( RectF& fieldValue ) const
    {
        if ( dSscanf( mFieldValue, "%g %g %g %g", &fieldValue.point.x, &fieldValue.point.y, &fieldValue.extent.x, &fieldValue.extent.y ) != 3 )
        {
            // Warn.
            Con::warnf( "TamlCustomField - Reading RectF but it has an incorrect format: '%s'.", mFieldValue );
        }
    }
    
    inline void getFieldValue( QuatF& fieldValue ) const
    {
        if ( dSscanf( mFieldValue, "%g %g %g %g", &fieldValue.x, &fieldValue.y, &fieldValue.z, &fieldValue.w ) != 4 )
        {
            // Warn.
            Con::warnf( "TamlCustomField - Reading QuatF but it has an incorrect format: '%s'.", mFieldValue );
        }
    }
    
    inline void getFieldValue( AngAxisF& fieldValue ) const
    {
        if ( dSscanf( mFieldValue, "%g %g %g %g", &fieldValue.axis.x, &fieldValue.axis.y, &fieldValue.axis.z, &fieldValue.angle ) != 4 )
        {
            // Warn.
            Con::warnf( "TamlCustomField - Reading AngAxisF but it has an incorrect format: '%s'.", mFieldValue );
        }
    }

    inline void getFieldValue( bool& fieldValue ) const
    {
        fieldValue = dAtob( mFieldValue );
    }

    inline void getFieldValue( S32& fieldValue ) const
    {
        fieldValue = dAtoi( mFieldValue );
    }

    inline void getFieldValue( U32& fieldValue ) const
    {
        fieldValue = (U32)dAtoi( mFieldValue );
    }

    inline void getFieldValue( F32& fieldValue ) const
    {
        fieldValue = dAtof( mFieldValue );
    }

    inline const char* getFieldValue( void ) const
    {
        return mFieldValue;
    }

    inline StringTableEntry getFieldName( void ) const { return mFieldName; }

    bool fieldNameBeginsWith( const char* pComparison ) const
    {
        const U32 comparisonLength = dStrlen( pComparison );
        const U32 fieldNameLength = dStrlen( mFieldName );

        if ( comparisonLength == 0 || fieldNameLength == 0 || comparisonLength > fieldNameLength )
            return false;

        StringTableEntry comparison = StringTable->insert( pComparison );

        char fieldNameBuffer[1024];

        // Sanity!
        AssertFatal( fieldNameLength < sizeof(fieldNameBuffer), "TamlCustomField: Field name is too long." );

        dStrcpy( fieldNameBuffer, mFieldName );
        fieldNameBuffer[fieldNameLength-1] = 0;
        StringTableEntry fieldName = StringTable->insert( fieldNameBuffer );

        return ( fieldName == comparison );
    }

    inline bool isValueEmpty( void ) const { return *mFieldValue == 0; }

private:
    StringTableEntry    mFieldName;
    char                mFieldValue[MAX_TAML_NODE_FIELDVALUE_LENGTH];
};

//-----------------------------------------------------------------------------

class TamlCustomNode : public IFactoryObjectReset
{
public:
    TamlCustomNode()
    {
        // Reset proxy object.
        // NOTE: This MUST be done before the state is reset otherwise we'll be touching uninitialized stuff.
        mpProxyWriteNode = NULL;
        mpProxyObject = NULL;

        resetState();
    }

    virtual ~TamlCustomNode()
    {
        // Everything should already be cleared in a state reset.
        // Touching any memory here is dangerous as this type is typically
        // held in a static factory cache until shutdown at which point
        // pretty much anything or everything could be invalid!
    }

    virtual void resetState( void )
    {
        // We don't need to delete the write node as it'll get destroyed when the compilation is reset!
        mpProxyWriteNode = NULL;
        mpProxyObject = NULL;

        // Cache the children.
        while ( mChildren.size() > 0 )
        {
            TamlCustomNodeFactory.cacheObject( mChildren.back() );
            mChildren.pop_back();
        }

        // Cache the fields.
        while( mFields.size() > 0 )
        {
            TamlCustomFieldFactory.cacheObject( mFields.back() );
            mFields.pop_back();
        }

        // Reset the node name.
        mNodeName = StringTable->EmptyString();

        // Reset node text.
        mNodeText.resetState();

        // Reset the ignore empty flag.
        mIgnoreEmpty = false;
    }

    inline TamlCustomNode* addNode( SimObject* pProxyObject )
    {
        // Sanity!
        AssertFatal( pProxyObject != NULL, "Field object cannot be NULL." );
        AssertFatal( mpProxyWriteNode == NULL, "Field write node must be NULL." );

        // Create a custom node.
        TamlCustomNode* pCustomNode = TamlCustomNodeFactory.createObject();

        // Set node name.
        pCustomNode->setNodeName( pProxyObject->getClassName() );

        // Set proxy object.
        pCustomNode->mpProxyObject = pProxyObject;

        // Store node.
        mChildren.push_back( pCustomNode );

        return pCustomNode;
    }

    inline TamlCustomNode* addNode( const char* pNodeName, const bool ignoreEmpty = true )
    {
        // Create a custom node.
        TamlCustomNode* pCustomNode = TamlCustomNodeFactory.createObject();

        // Fetch node name.
        pCustomNode->setNodeName( pNodeName );

        // Set ignore-empty flag.
        pCustomNode->setIgnoreEmpty( ignoreEmpty );

        // Store node.
        mChildren.push_back( pCustomNode );

        return pCustomNode;
    }

    inline void removeNode( const U32 index )
    {
        // Sanity!
        AssertFatal( index < (U32)mChildren.size(), "tamlCustomNode::removeNode() - Index is out of bounds." );

        // Cache the custom node.
        TamlCustomNodeFactory.cacheObject( mChildren[index] );

        // Remove it.
        mChildren.erase( index );
    }

    inline const TamlCustomNode* findNode( const char* pNodeName ) const
    {
        // Sanity!
        AssertFatal( pNodeName != NULL, "Cannot find Taml node name that is NULL." );

        // Fetch node name.
        StringTableEntry nodeName = StringTable->insert( pNodeName );

        // Find node.
        for( Vector<TamlCustomNode*>::const_iterator nodeItr = mChildren.begin(); nodeItr != mChildren.end(); ++nodeItr )
        {
            if ( (*nodeItr)->getNodeName() == nodeName )
                return (*nodeItr);
        }

        return NULL;
    }

    inline TamlCustomField* addField( const char* pFieldName, const ColorI& fieldValue )
    {
        TamlCustomField* pNodeField = TamlCustomFieldFactory.createObject();
        pNodeField->setFieldValue( pFieldName, fieldValue );
        return registerField( pNodeField );
    }

    inline TamlCustomField* addField( const char* pFieldName, const ColorF& fieldValue )
    {
        TamlCustomField* pNodeField = TamlCustomFieldFactory.createObject();
        pNodeField->setFieldValue( pFieldName, fieldValue );
        return registerField( pNodeField );
    }

    inline TamlCustomField* addField( const char* pFieldName, const Point2I& fieldValue )
    {
        TamlCustomField* pNodeField = TamlCustomFieldFactory.createObject();
        pNodeField->setFieldValue( pFieldName, fieldValue );
        return registerField( pNodeField );

    }

    inline TamlCustomField* addField( const char* pFieldName, const Point2F& fieldValue )
    {
        TamlCustomField* pNodeField = TamlCustomFieldFactory.createObject();
        pNodeField->setFieldValue( pFieldName, fieldValue );
        return registerField( pNodeField );
    }

    inline TamlCustomField* addField( const char* pFieldName, const Point3I& fieldValue )
    {
        TamlCustomField* pNodeField = TamlCustomFieldFactory.createObject();
        pNodeField->setFieldValue( pFieldName, fieldValue );
        return registerField( pNodeField );

    }

    inline TamlCustomField* addField( const char* pFieldName, const Point3F& fieldValue )
    {
        TamlCustomField* pNodeField = TamlCustomFieldFactory.createObject();
        pNodeField->setFieldValue( pFieldName, fieldValue );
        return registerField( pNodeField );
    }

    inline TamlCustomField* addField( const char* pFieldName, const RectF& fieldValue )
    {
        TamlCustomField* pNodeField = TamlCustomFieldFactory.createObject();
        pNodeField->setFieldValue( pFieldName, fieldValue );
        return registerField( pNodeField );
    }
    
    inline TamlCustomField* addField( const char* pFieldName, const QuatF& fieldValue )
    {
        TamlCustomField* pNodeField = TamlCustomFieldFactory.createObject();
        pNodeField->setFieldValue( pFieldName, fieldValue );
        return registerField( pNodeField );
    }
    
    inline TamlCustomField* addField( const char* pFieldName, const AngAxisF& fieldValue )
    {
        TamlCustomField* pNodeField = TamlCustomFieldFactory.createObject();
        pNodeField->setFieldValue( pFieldName, fieldValue );
        return registerField( pNodeField );
    }

    inline TamlCustomField* addField( const char* pFieldName, const U32 fieldValue )
    {
        TamlCustomField* pNodeField = TamlCustomFieldFactory.createObject();
        pNodeField->setFieldValue( pFieldName, fieldValue );
        return registerField( pNodeField );
    }

    inline TamlCustomField* addField( const char* pFieldName, const bool fieldValue )
    {
        TamlCustomField* pNodeField = TamlCustomFieldFactory.createObject();
        pNodeField->setFieldValue( pFieldName, fieldValue );
        return registerField( pNodeField );
    }

    inline TamlCustomField* addField( const char* pFieldName, const S32 fieldValue )
    {
        TamlCustomField* pNodeField = TamlCustomFieldFactory.createObject();
        pNodeField->setFieldValue( pFieldName, fieldValue );
        return registerField( pNodeField );
    }

    inline TamlCustomField* addField( const char* pFieldName, const float fieldValue )
    {
        TamlCustomField* pNodeField = TamlCustomFieldFactory.createObject();
        pNodeField->setFieldValue( pFieldName, fieldValue );
        return registerField( pNodeField );
    }

    inline TamlCustomField* addField( const char* pFieldName, const char* fieldValue )
    {
        TamlCustomField* pNodeField = TamlCustomFieldFactory.createObject();
        pNodeField->setFieldValue( pFieldName, fieldValue );
        return registerField( pNodeField );
    }

    inline const TamlCustomField* findField( const char* pFieldName ) const
    {
        // Sanity!
        AssertFatal( pFieldName != NULL, "Cannot find Taml field name that is NULL." );

        // Fetch field name.
        StringTableEntry fieldName = StringTable->insert( pFieldName );

        // Find node field.
        for( TamlCustomFieldVector::const_iterator fieldItr = mFields.begin(); fieldItr != mFields.end(); ++fieldItr )
        {
            if ( (*fieldItr)->getFieldName() == fieldName )
                return (*fieldItr);
        }

        return NULL;
    }

    inline void setNodeName( const char* pNodeName )
    {
        // Sanity!
        AssertFatal( pNodeName != NULL, "Cannot add a NULL node name." );

        mNodeName = StringTable->insert( pNodeName );
    }

    inline StringTableEntry getNodeName( void ) const { return mNodeName; }

    void setWriteNode( TamlWriteNode* pWriteNode );

    inline void setNodeText( const char* pNodeText )
    {
        AssertFatal( dStrlen( pNodeText ) < MAX_TAML_NODE_FIELDVALUE_LENGTH, "Custom node text is too long." );

        mNodeText.set( StringTable->EmptyString(), pNodeText );
    }
    inline const TamlCustomField& getNodeTextField( void ) const { return mNodeText; }
    inline TamlCustomField& getNodeTextField( void ) { return mNodeText; }

    inline const Vector<TamlCustomNode*>& getChildren( void ) const { return mChildren; }
    inline const TamlCustomFieldVector& getFields( void ) const { return mFields; }

    inline bool isProxyObject( void ) const { return mpProxyObject != NULL; }
    template<typename T> T* getProxyObject( const bool deleteIfNotType ) const
    {
        // Return nothing if no proxy object.
        if ( mpProxyObject == NULL )
            return NULL;

        // Cast object to specified type.
        T* pTypeCast = dynamic_cast<T*>( mpProxyObject );

        // Destroy the object if not the specified type and requested to do so.
        if ( deleteIfNotType && pTypeCast == NULL )
        {
            mpProxyObject->deleteObject();
            return NULL;
        }

        return pTypeCast;
    }
    inline const TamlWriteNode* getProxyWriteNode( void ) const { return mpProxyWriteNode; }

    inline bool isEmpty( void ) const { return mNodeText.isValueEmpty() && mFields.size() == 0 && mChildren.size() == 0; }

    inline void setIgnoreEmpty( const bool ignoreEmpty ) { mIgnoreEmpty = ignoreEmpty; }
    inline bool getIgnoreEmpty( void ) const { return mIgnoreEmpty; }

private:
    inline TamlCustomField* registerField( TamlCustomField* pCustomField )
    {
#if TORQUE_DEBUG
        // Ensure a field name conflict does not exist.
        for( Vector<TamlCustomField*>::iterator nodeFieldItr = mFields.begin(); nodeFieldItr != mFields.end(); ++nodeFieldItr )
        {
            // Skip if field name is not the same.
            if ( pCustomField->getFieldName() != (*nodeFieldItr)->getFieldName() )
                continue;

            // Warn!
            Con::warnf("Conflicting Taml node field name of '%s' in node '%s'.", pCustomField->getFieldName(), mNodeName );

            // Cache node field.
            TamlCustomFieldFactory.cacheObject( pCustomField );
            return NULL;
        }

        // Ensure the field value is not too long.
        if ( dStrlen( pCustomField->getFieldValue() ) >= MAX_TAML_NODE_FIELDVALUE_LENGTH )
        {
            // Warn.
            Con::warnf("Taml field name '%s' has a field value that is too long (Max:%d): '%s'.",
                pCustomField->getFieldName(),
                MAX_TAML_NODE_FIELDVALUE_LENGTH,
                pCustomField->getFieldValue() );

            // Cache node field.
            TamlCustomFieldFactory.cacheObject( pCustomField );
            return NULL;
        }
#endif
        // Store node field.
        mFields.push_back( pCustomField );

        return pCustomField;
    }

    inline TamlCustomField* createField( void ) const { return TamlCustomFieldFactory.createObject(); }

private:
    StringTableEntry        mNodeName;
    TamlCustomField         mNodeText;
    Vector<TamlCustomNode*> mChildren;
    TamlCustomFieldVector   mFields;
    bool                    mIgnoreEmpty;

    SimObject*              mpProxyObject;
    TamlWriteNode*          mpProxyWriteNode;
};

//-----------------------------------------------------------------------------

class TamlCustomNodes : public IFactoryObjectReset
{
public:
    TamlCustomNodes()
    {
    }

    virtual ~TamlCustomNodes()
    {
        resetState();
    }

    virtual void resetState( void )
    {
        // Cache the nodes.
        while ( mNodes.size() > 0 )
        {
            TamlCustomNodeFactory.cacheObject( mNodes.back() );
            mNodes.pop_back();
        }
    }

    inline TamlCustomNode* addNode( const char* pNodeName, const bool ignoreEmpty = true )
    {
        // Create a custom node.
        TamlCustomNode* pCustomNode = TamlCustomNodeFactory.createObject();

        // Set node name.
        pCustomNode->setNodeName( pNodeName );

        // Set ignore-empty flag.
        pCustomNode->setIgnoreEmpty( ignoreEmpty );

#if TORQUE_DEBUG
        // Ensure a node name conflict does not exist.
        for( TamlCustomNodeVector::iterator nodeItr = mNodes.begin(); nodeItr != mNodes.end(); ++nodeItr )
        {
            // Skip if node name is not the same.
            if ( pCustomNode->getNodeName() != (*nodeItr)->getNodeName() )
                continue;

            // Warn!
            Con::warnf("Conflicting Taml custom node name of '%s'.", pNodeName );

            // Cache node.
            TamlCustomNodeFactory.cacheObject( pCustomNode );
            return NULL;
        }
#endif
        // Store node.
        mNodes.push_back( pCustomNode );

        return pCustomNode;
    }

    inline void removeNode( const U32 index )
    {
        // Sanity!
        AssertFatal( index < (U32)mNodes.size(), "tamlCustomNode::removeNode() - Index is out of bounds." );

        // Cache the custom node.
        TamlCustomNodeFactory.cacheObject( mNodes[index] );

        // Remove it.
        mNodes.erase( index );
    }

    inline const TamlCustomNode* findNode( const char* pNodeName ) const
    {
        // Sanity!
        AssertFatal( pNodeName != NULL, "Cannot find Taml node name that is NULL." );

        // Fetch node name.
        StringTableEntry nodeName = StringTable->insert( pNodeName );

        // Find node.
        for( Vector<TamlCustomNode*>::const_iterator nodeItr = mNodes.begin(); nodeItr != mNodes.end(); ++nodeItr )
        {
            if ( (*nodeItr)->getNodeName() == nodeName )
                return (*nodeItr);
        }

        return NULL;
    }

    inline const TamlCustomNodeVector& getNodes( void ) const { return mNodes; }

private:
    TamlCustomNodeVector mNodes;
};

#endif // _TAML_CUSTOM_H_