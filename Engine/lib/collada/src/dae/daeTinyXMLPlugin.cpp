/*
 * Copyright 2007 Sony Computer Entertainment Inc.
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

// The user can choose whether or not to include TinyXML support in the DOM. Supporting TinyXML will
// require linking against it. By default TinyXML support isn't included.
#if defined(DOM_INCLUDE_TINYXML)

#if defined(DOM_DYNAMIC) && defined(_MSC_VER)
#pragma comment(lib, "tinyxml.lib")
#endif

#if defined(_MSC_VER)
#pragma warning(disable: 4100) // warning C4100: 'element' : unreferenced formal parameter
#endif

#include <string>
#include <tinyxml.h>
#include <dae.h>
#include <dom.h>
#include <dae/daeMetaElement.h>
#include <dae/daeErrorHandler.h>
#include <dae/daeMetaElementAttribute.h>
#include <dae/daeTinyXMLPlugin.h>
#include <dae/daeDocument.h>

using namespace std;

namespace {
	daeInt getCurrentLineNumber(TiXmlElement* element) {
		return -1;
	}
}

daeTinyXMLPlugin::daeTinyXMLPlugin()
{
  m_doc = NULL;
	supportedProtocols.push_back("*");
}

daeTinyXMLPlugin::~daeTinyXMLPlugin()
{
}

daeInt daeTinyXMLPlugin::setOption( daeString option, daeString value )
{
	return DAE_ERR_INVALID_CALL;
}

daeString daeTinyXMLPlugin::getOption( daeString option )
{
	return NULL;
}

daeElementRef daeTinyXMLPlugin::readFromFile(const daeURI& uri) {
	string file = cdom::uriToNativePath(uri.str());
	if (file.empty())
		return NULL;
	TiXmlDocument doc;
	doc.LoadFile(file.c_str());
	if (!doc.RootElement()) {
		daeErrorHandler::get()->handleError((std::string("Failed to open ") + uri.str() +
		                                     " in daeTinyXMLPlugin::readFromFile\n").c_str());
		return NULL;
	}
	return readElement(doc.RootElement(), NULL);
}

daeElementRef daeTinyXMLPlugin::readFromMemory(daeString buffer, const daeURI& baseUri) {
	TiXmlDocument doc;
	doc.Parse(buffer);
	if (!doc.RootElement()) {
		daeErrorHandler::get()->handleError("Failed to open XML document from memory buffer in "
		                                    "daeTinyXMLPlugin::readFromMemory\n");
		return NULL;
	}
	return readElement(doc.RootElement(), NULL);
}

daeElementRef daeTinyXMLPlugin::readElement(TiXmlElement* tinyXmlElement, daeElement* parentElement) {
	std::vector<attrPair> attributes;
	for (TiXmlAttribute* attrib = tinyXmlElement->FirstAttribute(); attrib != NULL; attrib = attrib->Next())
		attributes.push_back(attrPair(attrib->Name(), attrib->Value()));
		
	daeElementRef element = beginReadElement(parentElement, tinyXmlElement->Value(),
	                                         attributes, getCurrentLineNumber(tinyXmlElement));
	if (!element) {
		// We couldn't create the element. beginReadElement already printed an error message.
		return NULL;
	}

  if (tinyXmlElement->GetText() != NULL)
		readElementText(element, tinyXmlElement->GetText(), getCurrentLineNumber(tinyXmlElement));
  
  // Recurse children
  for (TiXmlElement* child = tinyXmlElement->FirstChildElement(); child != NULL; child = child->NextSiblingElement())
    element->placeElement(readElement(child, element));

	return element;
}

daeInt daeTinyXMLPlugin::write(const daeURI& name, daeDocument *document, daeBool replace)
{
	// Make sure database and document are both set
	if (!database)
		return DAE_ERR_INVALID_CALL;
	if(!document)
		return DAE_ERR_COLLECTION_DOES_NOT_EXIST;

	string fileName = cdom::uriToNativePath(name.str());
	if (fileName.empty())
	{
		daeErrorHandler::get()->handleError( "can't get path in write\n" );
		return DAE_ERR_BACKEND_IO;
	}
	// If replace=false, don't replace existing files
	if(!replace)
	{
		// Using "stat" would be better, but it's not available on all platforms
		FILE *tempfd = fopen(fileName.c_str(), "r");
		if(tempfd != NULL)
		{
			// File exists, return error
			fclose(tempfd);
			return DAE_ERR_BACKEND_FILE_EXISTS;
		}
		fclose(tempfd);
	}
	
  m_doc = new TiXmlDocument(name.getURI());
  if (m_doc)
  {
    m_doc->SetTabSize(4);

   	TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "", "" );  
	  m_doc->LinkEndChild( decl ); 

    writeElement(document->getDomRoot());

    m_doc->SaveFile(fileName.c_str());
    delete m_doc;
    m_doc = NULL;
  }
	return DAE_OK;
}

void daeTinyXMLPlugin::writeElement( daeElement* element )
{
	daeMetaElement* _meta = element->getMeta();
  if (!_meta->getIsTransparent() ) 
  {
		TiXmlElement* tiElm = new TiXmlElement( element->getElementName() );  
        
		if (m_elements.empty() == true) {
				m_doc->LinkEndChild(tiElm);
			} else {
			TiXmlElement* first = m_elements.front();
			first->LinkEndChild(tiElm);
		}
		m_elements.push_front(tiElm);

		daeMetaAttributeRefArray& attrs = _meta->getMetaAttributes();
		
		int acnt = (int)attrs.getCount();
		
		for(int i=0;i<acnt;i++) 
    {
			writeAttribute( attrs[i], element );
		}
	}
	writeValue(element);
	
	daeElementRefArray children;
	element->getChildren( children );
	for ( size_t x = 0; x < children.getCount(); x++ ) 
  {
		writeElement( children.get(x) );
	}

	if (!_meta->getIsTransparent() ) 
  {
    m_elements.pop_front();
	}
}


void daeTinyXMLPlugin::writeValue( daeElement* element )
{
	if (daeMetaAttribute* attr = element->getMeta()->getValueAttribute()) {
		std::ostringstream buffer;
		attr->memoryToString(element, buffer);
		std::string s = buffer.str();
		if (!s.empty())
			m_elements.front()->LinkEndChild( new TiXmlText(buffer.str().c_str()) );
	}
}

void daeTinyXMLPlugin::writeAttribute( daeMetaAttribute* attr, daeElement* element )
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

	std::ostringstream buffer;
	attr->memoryToString(element, buffer);
	m_elements.front()->SetAttribute(attr->getName(), buffer.str().c_str());
}

#endif // DOM_INCLUDE_TINYXML
