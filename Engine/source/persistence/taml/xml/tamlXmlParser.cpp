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

#include "persistence/taml/xml/tamlXmlParser.h"
#include "persistence/taml/tamlVisitor.h"
#include "console/console.h"

// Debug Profiling.
#include "platform/profiler.h"

#ifndef _FILESTREAM_H_
#include "core/stream/fileStream.h"
#endif

//-----------------------------------------------------------------------------

bool TamlXmlParser::accept( const char* pFilename, TamlVisitor& visitor )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlXmlParser_Accept);

    // Sanity!
    AssertFatal( pFilename != NULL, "Cannot parse a NULL filename." );

    // Expand the file-path.
    char filenameBuffer[1024];
    // TODO: Make sure this is a proper substitute for
    // Con::expandPath (T2D)
    Con::expandToolScriptFilename( filenameBuffer, sizeof(filenameBuffer), pFilename );
    /** T2D uses a custom version of TinyXML that supports FileStream.
      * We don't so we can't do this
      *
    FileStream stream;

#ifdef TORQUE_OS_ANDROID
    if (strlen(pFilename) > strlen(filenameBuffer)) {
    	strcpy(filenameBuffer, pFilename);
    }
#endif

    // File open for read?
    if ( !stream.open( filenameBuffer, Torque::FS::File::AccessMode::Read ) )
    {
        // No, so warn.
        Con::warnf("TamlXmlParser::parse() - Could not open filename '%s' for parse.", filenameBuffer );
        return false;
    }
    
     */

    TiXmlDocument xmlDocument;

    // Load document from stream.
    if ( !xmlDocument.LoadFile( filenameBuffer ) )
    {
        // Warn!
        Con::warnf("TamlXmlParser: Could not load Taml XML file from stream.");
        return false;
    }

    // Close the stream.
    // stream.close();

    // Set parsing filename.
    setParsingFilename( filenameBuffer );

    // Flag document as not dirty.
    mDocumentDirty = false;

    // Parse root element.
    parseElement( xmlDocument.RootElement(), visitor );

    // Reset parsing filename.
    setParsingFilename( StringTable->EmptyString() );

    // Finish if the document is not dirty.
    if ( !mDocumentDirty )
        return true;

    // Open for write?
    /*if ( !stream.open( filenameBuffer, FileStream::StreamWrite ) )
    {
        // No, so warn.
        Con::warnf("TamlXmlParser::parse() - Could not open filename '%s' for write.", filenameBuffer );
        return false;
    }*/

    // Yes, so save the document.
    if ( !xmlDocument.SaveFile( filenameBuffer ) )
    {
        // Warn!
        Con::warnf("TamlXmlParser: Could not save Taml XML document.");
        return false;
    }

    // Close the stream.
    //stream.close();

    return true;
}

//-----------------------------------------------------------------------------

inline bool TamlXmlParser::parseElement( TiXmlElement* pXmlElement, TamlVisitor& visitor )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlXmlParser_ParseElement);

    // Parse attributes (stop processing if instructed).
    if ( !parseAttributes( pXmlElement, visitor ) )
        return false;

    // Finish if only the root is needed.
    if ( visitor.wantsRootOnly() )
        return false;

    // Fetch any children.
    TiXmlNode* pChildXmlNode = pXmlElement->FirstChild();

    // Do we have any element children?
    if ( pChildXmlNode != NULL && pChildXmlNode->Type() == TiXmlNode::TINYXML_ELEMENT )
    {
        // Iterate children.
        for ( TiXmlElement* pChildXmlElement = dynamic_cast<TiXmlElement*>( pChildXmlNode ); pChildXmlElement; pChildXmlElement = pChildXmlElement->NextSiblingElement() )
        {
            // Parse element (stop processing if instructed).
            if ( !parseElement( pChildXmlElement, visitor ) )
                return false;
        }
    }

    return true;
}

//-----------------------------------------------------------------------------

inline bool TamlXmlParser::parseAttributes( TiXmlElement* pXmlElement, TamlVisitor& visitor )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlXmlParser_ParseAttribute);

    // Calculate if element is at the root or not.
    const bool isRoot = pXmlElement->GetDocument()->RootElement() == pXmlElement;

    // Create a visitor property state.
    TamlVisitor::PropertyState propertyState;
    propertyState.setObjectName( pXmlElement->Value(), isRoot );

    // Iterate attributes.
    for ( TiXmlAttribute* pAttribute = pXmlElement->FirstAttribute(); pAttribute; pAttribute = pAttribute->Next() )
    {
        // Configure property state.
        propertyState.setProperty( pAttribute->Name(), pAttribute->Value() );

        // Visit this attribute.
        const bool visitStatus = visitor.visit( *this, propertyState );

        // Was the property value changed?
        if ( propertyState.getPropertyValueDirty() )
        {
            // Yes, so update the attribute.
            pAttribute->SetValue( propertyState.getPropertyValue() );

            // Flag the document as dirty.
            mDocumentDirty = true;
        }

        // Finish if requested.
        if ( !visitStatus )
            return false;
    }

    return true;
}
