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

#include <sstream>
#include <dae.h>
#include <dom.h>
#include <dae/daeDatabase.h>
#include <dae/daeIOPluginCommon.h>
#include <dae/daeMetaElement.h>
#include <dae/daeErrorHandler.h>
#include <dae/daeMetaElementAttribute.h>

using namespace std;

daeIOPluginCommon::daeIOPluginCommon()
	: database(NULL),
		topMeta(NULL)
{
}

daeIOPluginCommon::~daeIOPluginCommon()
{
}

daeInt daeIOPluginCommon::setMeta(daeMetaElement *_topMeta)
{
	topMeta = _topMeta;
	return DAE_OK;
}

void daeIOPluginCommon::setDatabase(daeDatabase* _database)
{
	database = _database;
}

// This function needs to be re-entrant, it can be called recursively from inside of resolveAll
// to load files that the first file depends on.
daeInt daeIOPluginCommon::read(const daeURI& uri, daeString docBuffer)
{
	// Make sure topMeta has been set before proceeding
	if (topMeta == NULL) 
	{
		return DAE_ERR_BACKEND_IO;
	}

	// Generate a version of the URI with the fragment removed
	daeURI fileURI(*uri.getDAE(), uri.str(), true);

	//check if document already exists
	if ( database->isDocumentLoaded( fileURI.getURI() ) )
	{
		return DAE_ERR_COLLECTION_ALREADY_EXISTS;
	}

	daeElementRef domObject = docBuffer ?
		readFromMemory(docBuffer, fileURI) :
		readFromFile(fileURI); // Load from URI

	if (!domObject) {
		string msg = docBuffer ?
			"Failed to load XML document from memory\n" :
			string("Failed to load ") + fileURI.str() + "\n";
		daeErrorHandler::get()->handleError(msg.c_str());
		return DAE_ERR_BACKEND_IO;
	}

	// Insert the document into the database, the Database will keep a ref on the main dom, so it won't get deleted
	// until we clear the database

	daeDocument *document = NULL;

	int res = database->insertDocument(fileURI.getURI(),domObject,&document);
	if (res!= DAE_OK)
		return res;

	return DAE_OK;
}

daeElementRef daeIOPluginCommon::beginReadElement(daeElement* parentElement,
																									daeString elementName,
																									const vector<attrPair>& attributes,
																									daeInt lineNumber) {
	daeMetaElement* parentMeta = parentElement ? parentElement->getMeta() : topMeta;
	daeElementRef element = parentMeta->create(elementName);
	
	if(!element)
	{
		ostringstream msg;
		msg << "The DOM was unable to create an element named " << elementName << " at line "
			     << lineNumber << ". Probably a schema violation.\n";
		daeErrorHandler::get()->handleWarning( msg.str().c_str() );
		return NULL;
	}

	// Process the attributes
	for (size_t i = 0; i < attributes.size(); i++) {
		daeString name  = attributes[i].first,
			        value = attributes[i].second;
		if (!element->setAttribute(name, value)) {
			ostringstream msg;
			msg << "The DOM was unable to create an attribute " << name << " = " << value
				  << " at line " << lineNumber << ".\nProbably a schema violation.\n";
			daeErrorHandler::get()->handleWarning(msg.str().c_str());
		}
	}
		
	if (parentElement == NULL) {
		// This is the root element. Check the COLLADA version.
		daeURI *xmlns = (daeURI*)(element->getMeta()->getMetaAttribute( "xmlns" )->getWritableMemory( element ));
		if ( strcmp( xmlns->getURI(), COLLADA_NAMESPACE ) != 0 ) {
			// Invalid COLLADA version
			daeErrorHandler::get()->handleError("Trying to load an invalid COLLADA version for this DOM build!");
			return NULL;
		}
	}
	
	return element;
}

bool daeIOPluginCommon::readElementText(daeElement* element, daeString text, daeInt elementLineNumber) {
	if (element->setCharData(text))
		return true;
	
	ostringstream msg;
	msg << "The DOM was unable to set a value for element of type " << element->getTypeName()
			<< " at line " << elementLineNumber << ".\nProbably a schema violation.\n";
	daeErrorHandler::get()->handleWarning(msg.str().c_str());
	return false;
}
