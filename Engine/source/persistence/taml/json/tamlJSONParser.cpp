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

#include "persistence/taml/json/tamlJSONParser.h"
#include "persistence/taml/tamlVisitor.h"
#include "console/console.h"
#include "core/stream/fileStream.h"
#include "core/frameAllocator.h"

// Debug Profiling.
#include "platform/profiler.h"

//-----------------------------------------------------------------------------

bool TamlJSONParser::accept( const char* pFilename, TamlVisitor& visitor )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlJSONParser_Accept);

    // Sanity!
    AssertFatal( pFilename != NULL, "Cannot parse a NULL filename." );

    // Expand the file-path.
    char filenameBuffer[1024];
    Con::expandToolScriptFilename( filenameBuffer, sizeof(filenameBuffer), pFilename );

    FileStream stream;

    // File open for read?
    if ( !stream.open( filenameBuffer, Torque::FS::File::Write ) )
    {
        // No, so warn.
        Con::warnf("TamlJSONParser::parse() - Could not open filename '%s' for parse.", filenameBuffer );
        return false;
    }

    // Read JSON file.
    const U32 streamSize = stream.getStreamSize();
    FrameTemp<char> jsonText( streamSize + 1 );
    if ( !stream.read( streamSize, jsonText ) )
    {
        // Warn!
        Con::warnf("TamlJSONParser::parse() - Could not load Taml JSON file from stream.");
        return false;
    }

    // Create JSON document.
    rapidjson::Document inputDocument;
    inputDocument.Parse<0>( jsonText );

    // Close the stream.
    stream.close();

    // Check the document is valid.
    if ( inputDocument.GetType() != rapidjson::kObjectType )
    {
        // Warn!
        Con::warnf("TamlJSONParser::parse() - Load Taml JSON file from stream but was invalid.");
        return false;
    }

    // Set parsing filename.
    setParsingFilename( filenameBuffer );

    // Flag document as not dirty.
    mDocumentDirty = false;

    // Fetch the root.
    rapidjson::Value::MemberIterator rootItr = inputDocument.MemberBegin();

    // Parse root value.
    parseType( rootItr, visitor, true );

    // Reset parsing filename.
    setParsingFilename( StringTable->EmptyString() );

    // Finish if the document is not dirty.
    if ( !mDocumentDirty )
        return true;

    // Open for write?

    if ( !stream.open( filenameBuffer, Torque::FS::File::Write ) )
    {
        // No, so warn.
        Con::warnf("TamlJSONParser::parse() - Could not open filename '%s' for write.", filenameBuffer );
        return false;
    }

    // Create output document.
    rapidjson::Document outputDocument;
    outputDocument.AddMember( rootItr->name, rootItr->value, outputDocument.GetAllocator() );

    // Write document to stream.
    rapidjson::PrettyWriter<FileStream> jsonStreamWriter( stream );
    outputDocument.Accept( jsonStreamWriter );

    // Close the stream.
    stream.close();

    return true;
}

//-----------------------------------------------------------------------------

inline bool TamlJSONParser::parseType( rapidjson::Value::MemberIterator& memberItr, TamlVisitor& visitor, const bool isRoot )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlJSONParser_ParseType);

    // Fetch name and value.
    const rapidjson::Value& typeName = memberItr->name;
    rapidjson::Value& typeValue = memberItr->value;

    // Create a visitor property state.
    TamlVisitor::PropertyState propertyState;
    propertyState.setObjectName( typeName.GetString(), isRoot );

    // Parse field members.
    for( rapidjson::Value::MemberIterator fieldMemberItr = typeValue.MemberBegin(); fieldMemberItr != typeValue.MemberEnd(); ++fieldMemberItr )
    {
        // Fetch value.
        const rapidjson::Value& fieldName = fieldMemberItr->name;
        rapidjson::Value& fieldValue = fieldMemberItr->value;
        
        // Skip if not a field.
        if ( fieldValue.IsObject() )
            continue;

        char valueBuffer[4096];
        if ( !parseStringValue( valueBuffer, sizeof(valueBuffer), fieldValue, fieldName.GetString() ) )
        {
            // Warn.
            Con::warnf( "TamlJSONParser::parseTyoe() - Could not interpret value for field '%s'", fieldName.GetString() );
            continue;
        }

        // Configure property state.
        propertyState.setProperty( fieldName.GetString(), valueBuffer );

        // Visit this attribute.
        const bool visitStatus = visitor.visit( *this, propertyState );

        // Was the property value changed?
        if ( propertyState.getPropertyValueDirty() )
        {
            // Yes, so update the attribute.
            fieldValue.SetString( propertyState.getPropertyValue() );

            // Flag the document as dirty.
            mDocumentDirty = true;
        }

        // Finish if requested.
        if ( !visitStatus )
            return false;
    }

    // Finish if only the root is needed.
    if ( visitor.wantsRootOnly() )
        return false;

    // Parse children and custom node members.
    for( rapidjson::Value::MemberIterator objectMemberItr = typeValue.MemberBegin(); objectMemberItr != typeValue.MemberEnd(); ++objectMemberItr )
    {
        // Fetch name and value.
        const rapidjson::Value& objectValue = objectMemberItr->value;
        
        // Skip if not an object.
        if ( !objectValue.IsObject() )
            continue;

        // Parse the type.
        if ( !parseType( objectMemberItr, visitor, false ) )
            return false;
    }

    return true;
}

//-----------------------------------------------------------------------------

inline bool TamlJSONParser::parseStringValue( char* pBuffer, const S32 bufferSize, const rapidjson::Value& value, const char* pName )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlJSONParser_ParseStringValue);

    // Handle field value appropriately.

    if ( value.IsString() )
    {
        dSprintf( pBuffer, bufferSize, "%s", value.GetString() );
        return true;
    }

    if ( value.IsNumber() )
    {
        if ( value.IsInt() )
        {
            dSprintf( pBuffer, bufferSize, "%d", value.GetInt() );
            return true;
        }

        if ( value.IsUint() )
        {
            dSprintf( pBuffer, bufferSize, "%d", value.GetUint() );
            return true;
        }

        if ( value.IsInt64() )
        {
            dSprintf( pBuffer, bufferSize, "%d", value.GetInt64() );
            return true;
        }

        if ( value.IsUint64() )
        {
            dSprintf( pBuffer, bufferSize, "%d", value.GetUint64() );
            return true;
        }

        if ( value.IsDouble() )
        {
            dSprintf( pBuffer, bufferSize, "%f", value.GetDouble() );
            return true;
        }
    }

    if ( value.IsBool() )
    {
        dSprintf( pBuffer, bufferSize, "%d", value.GetBool() );
        return true;
    }

    // Failed to get value type.
    Con::warnf( "Taml: Encountered a field '%s' but its value is an unknown type.", pName );
    return false;
}

