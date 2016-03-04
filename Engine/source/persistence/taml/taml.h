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

#ifndef _TAML_H_
#define _TAML_H_

#ifndef _TAML_CALLBACKS_H_
#include "persistence/taml/tamlCallbacks.h"
#endif

#ifndef _TAML_CUSTOM_H_
#include "persistence/taml/tamlCustom.h"
#endif

#ifndef _TAML_CHILDREN_H_
#include "persistence/taml/tamlChildren.h"
#endif

#ifndef _TAML_WRITE_NODE_H_
#include "persistence/taml/tamlWriteNode.h"
#endif

#ifndef _TAML_VISITOR_H_
#include "persistence/taml/tamlVisitor.h"
#endif

#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif

#ifndef _TDICTIONARY_H_
#include "core/util/tDictionary.h"
#endif

#ifndef _FILESTREAM_H_
#include "core/stream/fileStream.h"
#endif

//-----------------------------------------------------------------------------

extern StringTableEntry tamlRefIdName;
extern StringTableEntry tamlRefToIdName;
extern StringTableEntry tamlNamedObjectName;

//-----------------------------------------------------------------------------

#define TAML_SIGNATURE                  "Taml"
#define TAML_SCHEMA_VARIABLE            "$pref::T2D::TAMLSchema"
#define TAML_JSON_STRICT_VARIBLE        "$pref::T2D::JSONStrict"

class TiXmlElement;

//-----------------------------------------------------------------------------

/// @ingroup tamlGroup
/// @see tamlGroup
class Taml : public SimObject
{
public:
    enum TamlFormatMode
    {
        InvalidFormat = 0,
        XmlFormat,
        BinaryFormat,
        JSONFormat,
    };

private:
    typedef SimObject Parent;
    typedef Vector<TamlWriteNode*>                  typeNodeVector;
    typedef HashTable<SimObjectId, TamlWriteNode*>  typeCompiledHash;

    typeNodeVector      mCompiledNodes;
    typeCompiledHash    mCompiledObjects;
    U32                 mMasterNodeId;
    TamlFormatMode      mFormatMode;
    StringTableEntry    mAutoFormatXmlExtension;
    StringTableEntry    mAutoFormatBinaryExtension;
    StringTableEntry    mAutoFormatJSONExtension;
    bool                mJSONStrict;
    bool                mBinaryCompression;
    bool                mAutoFormat;
    bool                mWriteDefaults;
    bool                mProgenitorUpdate;
    char                mFilePathBuffer[1024];

private:
    void resetCompilation( void );

    TamlWriteNode* compileObject( SimObject* pSimObject, const bool forceId = false );
    void compileStaticFields( TamlWriteNode* pTamlWriteNode );
    void compileDynamicFields( TamlWriteNode* pTamlWriteNode );
    void compileChildren( TamlWriteNode* pTamlWriteNode );
    void compileCustomState( TamlWriteNode* pTamlWriteNode );
    void compileCustomNodeState( TamlCustomNode* pCustomNode );

    bool write( FileStream& stream, SimObject* pSimObject, const TamlFormatMode formatMode );
    SimObject* read( FileStream& stream, const TamlFormatMode formatMode );
    template<typename T> inline T* read( FileStream& stream, const TamlFormatMode formatMode )
    {
        SimObject* pSimObject = read( stream, formatMode );
        if ( pSimObject == NULL )
            return NULL;
        T* pObj = dynamic_cast<T*>( pSimObject );
        if ( pObj != NULL )
            return pObj;
        pSimObject->deleteObject();
        return NULL;
    }

public:
    Taml();
    virtual ~Taml() {}

    virtual bool onAdd();
    virtual void onRemove();
    static void initPersistFields();

    /// Format mode.
    inline void setFormatMode( const TamlFormatMode formatMode ) { mFormatMode = formatMode != Taml::InvalidFormat ? formatMode : Taml::XmlFormat; }
    inline TamlFormatMode getFormatMode( void ) const { return mFormatMode; }

    /// Auto-Format mode.
    inline void setAutoFormat( const bool autoFormat ) { mAutoFormat = autoFormat; }
    inline bool getAutoFormat( void ) const { return mAutoFormat; }

    /// Write defaults.
    inline void setWriteDefaults( const bool writeDefaults ) { mWriteDefaults = writeDefaults; }
    inline bool getWriteDefaults( void ) const { return mWriteDefaults; }

    /// Progenitor.
    inline void setProgenitorUpdate( const bool progenitorUpdate ) { mProgenitorUpdate = progenitorUpdate; }
    inline bool getProgenitorUpdate( void ) const { return mProgenitorUpdate; }

    /// Auto-format extensions.
    inline void setAutoFormatXmlExtension( const char* pExtension ) { mAutoFormatXmlExtension = StringTable->insert( pExtension ); }
    inline StringTableEntry getAutoFormatXmlExtension( void ) const { return mAutoFormatXmlExtension; }
    inline void setAutoFormatBinaryExtension( const char* pExtension ) { mAutoFormatBinaryExtension = StringTable->insert( pExtension ); }
    inline StringTableEntry getAutoFormatBinaryExtension( void ) const { return mAutoFormatBinaryExtension; }

    /// Compression.
    inline void setBinaryCompression( const bool compressed ) { mBinaryCompression = compressed; }
    inline bool getBinaryCompression( void ) const { return mBinaryCompression; }

    /// JSON Strict RFC4627 mode.
    inline void setJSONStrict( const bool jsonStrict ) { mJSONStrict = jsonStrict; }
    inline bool getJSONStrict( void ) const { return mJSONStrict; }

    TamlFormatMode getFileAutoFormatMode( const char* pFilename );

    const char* getFilePathBuffer( void ) const { return mFilePathBuffer; }

    /// Write.
    bool write( SimObject* pSimObject, const char* pFilename );

    /// Read.
    template<typename T> inline T* read( const char* pFilename )
    {
        SimObject* pSimObject = read( pFilename );
        if ( pSimObject == NULL )
            return NULL;
        T* pObj = dynamic_cast<T*>( pSimObject );
        if ( pObj != NULL )
            return pObj;
        pSimObject->deleteObject();
        return NULL;
    }
    SimObject* read( const char* pFilename );

    /// Parse.
    bool parse( const char* pFilename, TamlVisitor& visitor );

    /// Create type.
    static SimObject* createType( StringTableEntry typeName, const Taml* pTaml, const char* pProgenitorSuffix = NULL );

    /// Schema generation.
    static bool generateTamlSchema();

    /// Write a unrestricted custom Taml schema.
    static void WriteUnrestrictedCustomTamlSchema( const char* pCustomNodeName, const AbstractClassRep* pClassRep, TiXmlElement* pParentElement );

    /// Get format mode info.
    static TamlFormatMode getFormatModeEnum( const char* label );
    static const char* getFormatModeDescription( const TamlFormatMode formatMode );

    /// Taml callbacks.
    inline void tamlPreWrite( TamlCallbacks* pCallbacks )                                           { pCallbacks->onTamlPreWrite(); }
    inline void tamlPostWrite( TamlCallbacks* pCallbacks )                                          { pCallbacks->onTamlPostWrite(); }
    inline void tamlPreRead( TamlCallbacks* pCallbacks )                                            { pCallbacks->onTamlPreRead(); }
    inline void tamlPostRead( TamlCallbacks* pCallbacks, const TamlCustomNodes& customNodes )       { pCallbacks->onTamlPostRead( customNodes ); }
    inline void tamlAddParent( TamlCallbacks* pCallbacks, SimObject* pParentObject )                { pCallbacks->onTamlAddParent( pParentObject ); }
    inline void tamlCustomWrite( TamlCallbacks* pCallbacks, TamlCustomNodes& customNodes )          { pCallbacks->onTamlCustomWrite( customNodes ); }
    inline void tamlCustomRead( TamlCallbacks* pCallbacks, const TamlCustomNodes& customNodes )     { pCallbacks->onTamlCustomRead( customNodes ); }

    /// Declare Console Object.
    DECLARE_CONOBJECT( Taml );
};

#endif // _TAML_H_