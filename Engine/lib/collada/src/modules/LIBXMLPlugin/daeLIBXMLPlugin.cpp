/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the SCEA Shared Source License, Version 1.0 (the "License"); you may not use this 
 * file except in compliance with the License. You may obtain a copy of the License at:
 * http://research.scea.com/scea_shared_source_license.html
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License 
 * is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or 
 * implied. See the License for the specific language governing permissions and limitations under the 
 * License. 
 */

// The user can choose whether or not to include libxml support in the DOM. Supporting libxml will
// require linking against it. By default libxml support is included.
#if defined(DOM_INCLUDE_LIBXML)

// This is a rework of the XML plugin that contains a complete interface to libxml2 "readXML"
// This is intended to be a seperate plugin but I'm starting out by rewriting it in daeLIBXMLPlugin
// because I'm not sure if all the plugin handling stuff has been tested.  Once I get a working
// plugin I'll look into renaming it so the old daeLIBXMLPlugin can coexist with it.
//
#include <string>
#include <sstream>
#include <modules/daeLIBXMLPlugin.h>
#include <dae.h>
#include <dom.h>
#include <dae/daeDatabase.h>
#include <dae/daeMetaElement.h>
#include <libxml/xmlreader.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlmemory.h>
#include <dae/daeErrorHandler.h>
#include <dae/daeMetaElementAttribute.h>

using namespace std;


// Some helper functions for working with libxml
namespace {
	daeInt getCurrentLineNumber(xmlTextReaderPtr reader) {
#if LIBXML_VERSION >= 20620
	return xmlTextReaderGetParserLineNumber(reader);
#else
	return -1;
#endif
	}

	// Return value should be freed by caller with delete[]. Passed in value should not
	// be null.
	xmlChar* utf8ToLatin1(const xmlChar* utf8) {
		int inLen = xmlStrlen(utf8);
		int outLen = (inLen+1) * 2;
		xmlChar* latin1 = new xmlChar[outLen];
		int numBytes = UTF8Toisolat1(latin1, &outLen, utf8, &inLen);
		if (numBytes < 0)
			// Failed. Return an empty string instead.
			numBytes = 0;

		latin1[numBytes] = '\0';
		return latin1;
	}

	// Return value should be freed by caller with delete[].
	xmlChar* latin1ToUtf8(const string& latin1) {
		int inLen = (int)latin1.length();
		int outLen = (inLen+1) * 2;
		xmlChar* utf8 = new xmlChar[outLen];
		int numBytes = isolat1ToUTF8(utf8, &outLen, (xmlChar*)latin1.c_str(), &inLen);
		if (numBytes < 0)
			// Failed. Return an empty string instead.
			numBytes = 0;

		utf8[numBytes] = '\0';
		return utf8;
	}

	typedef pair<daeString, daeString> stringPair;
	
	// The attributes vector passed in should be empty. If 'encoding' is anything
	// other than utf8 the caller should free the returned attribute value
	// strings. The 'freeAttrValues' function is provided for that purpose.
	void packageCurrentAttributes(xmlTextReaderPtr reader,
	                              DAE::charEncoding encoding,
	                              /* out */ vector<stringPair>& attributes) {
		int numAttributes = xmlTextReaderAttributeCount(reader);
		if (numAttributes == -1 || numAttributes == 0)
			return;
		attributes.reserve(numAttributes);
		
		while (xmlTextReaderMoveToNextAttribute(reader) == 1) {
			const xmlChar* xmlName = xmlTextReaderConstName(reader);
			const xmlChar* xmlValue = xmlTextReaderConstValue(reader);
			if (encoding == DAE::Latin1)
				attributes.push_back(stringPair((daeString)xmlName, (daeString)utf8ToLatin1(xmlValue)));
			else
				attributes.push_back(stringPair((daeString)xmlName, (daeString)xmlValue));
		}
	}

	void freeAttrValues(vector<stringPair>& pairs) {
		for(size_t i=0, size=pairs.size(); i<size; ++i) {
			delete[] pairs[i].second;
			pairs[i].second = 0;
		}
	}
}

daeLIBXMLPlugin::daeLIBXMLPlugin(DAE& dae) : dae(dae), rawRelPath(dae)
{
	supportedProtocols.push_back("*");
	xmlInitParser();
	rawFile = NULL;
	rawByteCount = 0;
	saveRawFile = false;
}

daeLIBXMLPlugin::~daeLIBXMLPlugin()
{
	 xmlCleanupParser();
}

daeInt daeLIBXMLPlugin::setOption( daeString option, daeString value )
{
	if ( strcmp( option, "saveRawBinary" ) == 0 )
	{
		if ( strcmp( value, "true" ) == 0 || strcmp( value, "TRUE" ) == 0 )
		{
			saveRawFile = true;
		}
		else
		{
			saveRawFile = false;
		}
		return DAE_OK;
	}
	return DAE_ERR_INVALID_CALL;
}

daeString daeLIBXMLPlugin::getOption( daeString option )
{
	if ( strcmp( option, "saveRawBinary" ) == 0 )
	{
		if ( saveRawFile )
		{
			return "true";
		}
		return "false";
	}
	return NULL;
}

// A simple structure to help alloc/free xmlTextReader objects
struct xmlTextReaderHelper {
	xmlTextReaderHelper(const daeURI& uri) {
		reader = xmlReaderForFile(cdom::fixUriForLibxml(uri.str()).c_str(), NULL, 0);
	}

	xmlTextReaderHelper(daeString buffer, const daeURI& baseUri) {
		reader = xmlReaderForDoc((xmlChar*)buffer, cdom::fixUriForLibxml(baseUri.str()).c_str(), NULL, 0);
	};

	~xmlTextReaderHelper() {
		if (reader)
			xmlFreeTextReader(reader);
	}

	xmlTextReaderPtr reader;
};

daeElementRef daeLIBXMLPlugin::readFromFile(const daeURI& uri) {
	xmlTextReaderHelper readerHelper(uri);
	if (!readerHelper.reader) {
		daeErrorHandler::get()->handleError((string("Failed to open ") + uri.str() +
		                                    " in daeLIBXMLPlugin::readFromFile\n").c_str());
		return NULL;
	}
	return read(readerHelper.reader);
}

daeElementRef daeLIBXMLPlugin::readFromMemory(daeString buffer, const daeURI& baseUri) {
	xmlTextReaderHelper readerHelper(buffer, baseUri);
	if (!readerHelper.reader) {
		daeErrorHandler::get()->handleError("Failed to open XML document from memory buffer in "
		                                    "daeLIBXMLPlugin::readFromMemory\n");
		return NULL;
	}
	return read(readerHelper.reader);
}

daeElementRef daeLIBXMLPlugin::read(_xmlTextReader* reader) {
	// Drop everything up to the first element. In the future, we should try to store header comments somewhere.
	while(xmlTextReaderNodeType(reader) != XML_READER_TYPE_ELEMENT)
	{
		if (xmlTextReaderRead(reader) != 1) {
			daeErrorHandler::get()->handleError("Error parsing XML in daeLIBXMLPlugin::read\n");
			return NULL;
		}
	}

	return readElement(reader, NULL);
}

daeElementRef daeLIBXMLPlugin::readElement(_xmlTextReader* reader, daeElement* parentElement) {
	assert(xmlTextReaderNodeType(reader) == XML_READER_TYPE_ELEMENT);
	daeString elementName = (daeString)xmlTextReaderConstName(reader);
	bool empty = xmlTextReaderIsEmptyElement(reader) != 0;

	vector<attrPair> attributes;
	packageCurrentAttributes(reader, dae.getCharEncoding(), /* out */ attributes);
	
	daeElementRef element = beginReadElement(parentElement, elementName, attributes, getCurrentLineNumber(reader));
	if (dae.getCharEncoding() != DAE::Utf8)
		freeAttrValues(attributes);

	if (!element) {
		// We couldn't create the element. beginReadElement already printed an error message. Just make sure
		// to skip ahead past the bad element.
		xmlTextReaderNext(reader);
		return NULL;
	}

	if (xmlTextReaderRead(reader) != 1  ||  empty)
		return element;

	int nodeType = xmlTextReaderNodeType(reader);
	while (nodeType != -1  &&  nodeType != XML_READER_TYPE_END_ELEMENT) {
		if (nodeType == XML_READER_TYPE_ELEMENT) {
			element->placeElement(readElement(reader, element));
		}
		else if (nodeType == XML_READER_TYPE_TEXT) {
			const xmlChar* xmlText = xmlTextReaderConstValue(reader);
			if (dae.getCharEncoding() == DAE::Latin1)
				xmlText = utf8ToLatin1(xmlText);
			readElementText(element, (daeString)xmlText, getCurrentLineNumber(reader));
			if (dae.getCharEncoding() == DAE::Latin1)
				delete[] xmlText;

			if (xmlTextReaderRead(reader) != 1)
				return NULL;
		}
		else {
			if (xmlTextReaderRead(reader) != 1)
				return NULL;
		}

		nodeType = xmlTextReaderNodeType(reader);
	}

	if (nodeType == XML_READER_TYPE_END_ELEMENT)
		xmlTextReaderRead(reader);

	return element;
}

daeInt daeLIBXMLPlugin::write(const daeURI& name, daeDocument *document, daeBool replace)
{
	// Make sure database and document are both set
	if (!database)
		return DAE_ERR_INVALID_CALL;
	if(!document)
		return DAE_ERR_COLLECTION_DOES_NOT_EXIST;

	// Convert the URI to a file path, to see if we're about to overwrite a file
	string file = cdom::uriToNativePath(name.str());
	if (file.empty()  &&  saveRawFile)
	{
		daeErrorHandler::get()->handleError( "can't get path in write\n" );
		return DAE_ERR_BACKEND_IO;
	}
	
	// If replace=false, don't replace existing files
	if(!replace)
	{
		// Using "stat" would be better, but it's not available on all platforms
		FILE *tempfd = fopen(file.c_str(), "r");
		if(tempfd != NULL)
		{
			// File exists, return error
			fclose(tempfd);
			return DAE_ERR_BACKEND_FILE_EXISTS;
		}
		fclose(tempfd);
	}
	if ( saveRawFile )
	{
		string rawFilePath = file + ".raw";
		if ( !replace )
		{
			rawFile = fopen(rawFilePath.c_str(), "rb" );
			if ( rawFile != NULL )
			{
				fclose(rawFile);
				return DAE_ERR_BACKEND_FILE_EXISTS;
			}
			fclose(rawFile);
		}
		rawFile = fopen(rawFilePath.c_str(), "wb");
		if ( rawFile == NULL )
		{
			return DAE_ERR_BACKEND_IO;
		}
		rawRelPath.set(cdom::nativePathToUri(rawFilePath));
		rawRelPath.makeRelativeTo( &name );
	}

	// Open the file we will write to
	writer = xmlNewTextWriterFilename(cdom::fixUriForLibxml(name.str()).c_str(), 0);
	if ( !writer ) {
		ostringstream msg;
		msg << "daeLIBXMLPlugin::write(" << name.str() << ") failed\n";
		daeErrorHandler::get()->handleError(msg.str().c_str());
		return DAE_ERR_BACKEND_IO;
	}
	xmlTextWriterSetIndentString( writer, (const xmlChar*)"\t" ); // Don't change this to spaces
	xmlTextWriterSetIndent( writer, 1 ); // Turns indentation on
    xmlTextWriterStartDocument( writer, "1.0", "UTF-8", NULL );
	
	writeElement( document->getDomRoot() );
	
	xmlTextWriterEndDocument( writer );
	xmlTextWriterFlush( writer );
	xmlFreeTextWriter( writer );

	if ( saveRawFile && rawFile != NULL )
	{
		fclose( rawFile );
	}

	return DAE_OK;
}

void daeLIBXMLPlugin::writeElement( daeElement* element )
{
	daeMetaElement* _meta = element->getMeta();

	//intercept <source> elements for special handling
	if ( saveRawFile )
	{
		if ( strcmp( element->getTypeName(), "source" ) == 0 )
		{
			daeElementRefArray children;
			element->getChildren( children );
			bool validArray = false, teqCommon = false;
			for ( unsigned int i = 0; i < children.getCount(); i++ )
			{
				if ( strcmp( children[i]->getTypeName(), "float_array" ) == 0 || 
					 strcmp( children[i]->getTypeName(), "int_array" ) == 0 )
				{
					validArray = true;
				}
				else if ( strcmp( children[i]->getTypeName(), "technique_common" ) == 0 )
				{
					teqCommon = true;
				}
			}
			if ( validArray && teqCommon )
			{
				writeRawSource( element );
				return;
			}
		}
	}

	if (!_meta->getIsTransparent() ) {
		xmlTextWriterStartElement(writer, (xmlChar*)element->getElementName());
		daeMetaAttributeRefArray& attrs = _meta->getMetaAttributes();
		
		int acnt = (int)attrs.getCount();
		
		for(int i=0;i<acnt;i++) {
			writeAttribute(attrs[i], element);
		}
	}
	writeValue(element);
	
	daeElementRefArray children;
	element->getChildren( children );
	for ( size_t x = 0; x < children.getCount(); x++ ) {
		writeElement( children.get(x) );
	}

	/*if (_meta->getContents() != NULL) {
		daeElementRefArray* era = (daeElementRefArray*)_meta->getContents()->getWritableMemory(element);
		int elemCnt = (int)era->getCount();
		for(int i = 0; i < elemCnt; i++) {
			daeElementRef elem = (daeElementRef)era->get(i);
			if (elem != NULL) {
				writeElement( elem );
			}
		}
	}
	else
	{
		daeMetaElementAttributeArray& children = _meta->getMetaElements();
		int cnt = (int)children.getCount();
		for(int i=0;i<cnt;i++) {
			daeMetaElement *type = children[i]->getElementType();
			if ( !type->getIsAbstract() ) {
				for (int c = 0; c < children[i]->getCount(element); c++ ) {
					writeElement( *(daeElementRef*)children[i]->get(element,c) );
				}
			}
		}
	}*/
	if (!_meta->getIsTransparent() ) {
		xmlTextWriterEndElement(writer);
	}
}

void daeLIBXMLPlugin::writeAttribute( daeMetaAttribute* attr, daeElement* element)
{
	//don't write if !required and is set && is default
	if ( !attr->getIsRequired() ) {
		//not required
		if ( !element->isAttributeSet( attr->getName() ) ) {
			//early out if !value && !required && !set
			return;
		}
			
		//is set
		//check for default suppression
		if (attr->compareToDefault(element) == 0) {
			// We match the default value, so exit early
			return;
		}
	}

	xmlTextWriterStartAttribute(writer, (xmlChar*)(daeString)attr->getName());
	ostringstream buffer;
	attr->memoryToString(element, buffer);
	string str = buffer.str();

	xmlChar* utf8 = (xmlChar*)str.c_str();
	if (dae.getCharEncoding() == DAE::Latin1)
		utf8 = latin1ToUtf8(str);
	xmlTextWriterWriteString(writer, utf8);
	if (dae.getCharEncoding() == DAE::Latin1)
		delete[] utf8;
	
	xmlTextWriterEndAttribute(writer);
}

void daeLIBXMLPlugin::writeValue(daeElement* element) {
	if (daeMetaAttribute* attr = element->getMeta()->getValueAttribute()) {
		ostringstream buffer;
		attr->memoryToString(element, buffer);
		string s = buffer.str();
		if (!s.empty()) {
			xmlChar* str = (xmlChar*)s.c_str();
			if (dae.getCharEncoding() == DAE::Latin1)
				str = latin1ToUtf8(s);
			xmlTextWriterWriteString(writer, (xmlChar*)s.c_str());
			if (dae.getCharEncoding() == DAE::Latin1)
				delete[] str;
		}
	}
}

void daeLIBXMLPlugin::writeRawSource( daeElement *src )
{
	daeElementRef newSrc = src->clone();
	daeElementRef array = NULL;
	daeElement *accessor = NULL;
	daeElementRefArray children;
	newSrc->getChildren( children );
	bool isInt = false;
	for ( int i = 0; i < (int)children.getCount(); i++ )
	{
		if ( strcmp( children[i]->getTypeName(), "float_array" ) == 0 )
		{
			array = children[i];
			newSrc->removeChildElement( array );
		}
		else if ( strcmp( children[i]->getTypeName(), "int_array" ) == 0 )
		{
			array = children[i];
			isInt = true;
			newSrc->removeChildElement( array );
		}
		else if ( strcmp( children[i]->getTypeName(), "technique_common" ) == 0 )
		{
			children[i]->getChildren( children );
		}
		else if ( strcmp( children[i]->getTypeName(), "accessor" ) == 0 )
		{
			accessor = children[i];
		}
	}

	daeULong *countPtr = (daeULong*)array->getAttributeValue( "count" );
	daeULong count = countPtr != NULL ? *countPtr : 0;

	daeULong *stridePtr = (daeULong*)accessor->getAttributeValue( "stride" );
	daeULong stride = stridePtr != NULL ? *stridePtr : 1;

	children.clear();
	accessor->getChildren( children );
	if ( children.getCount() > stride ) {
		*stridePtr = children.getCount();
	}

	daeFixedName newURI;
	sprintf( newURI, "%s#%ld", rawRelPath.getOriginalURI(), rawByteCount );
	accessor->setAttribute( "source", newURI );

	daeArray *valArray = (daeArray*)array->getValuePointer();

	//TODO: pay attention to precision for the array.
	if ( isInt )
	{
		for( size_t i = 0; i < count; i++ )
		{
			daeInt tmp = (daeInt)*(daeLong*)(valArray->getRaw(i));
			rawByteCount += (unsigned long)(fwrite( &tmp, sizeof(daeInt), 1, rawFile ) * sizeof(daeInt));
		}
	}
	else
	{
		for( size_t i = 0; i < count; i++ )
		{
			daeFloat tmp = (daeFloat)*(daeDouble*)(valArray->getRaw(i));
			rawByteCount += (unsigned long)(fwrite( &tmp, sizeof(daeFloat), 1, rawFile ) * sizeof(daeFloat));
		}
	}

	writeElement( newSrc );
}

#endif // DOM_INCLUDE_LIBXML
