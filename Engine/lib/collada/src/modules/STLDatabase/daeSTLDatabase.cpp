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

#include <modules/daeSTLDatabase.h>
#include <dae/daeMetaElement.h>

using namespace std;

daeSTLDatabase::daeSTLDatabase(DAE& dae) : daeDatabase(dae)
{ }

daeSTLDatabase::~daeSTLDatabase()
{
	clear();
}

daeInt daeSTLDatabase::setMeta(daeMetaElement *_topMeta)
{
	topMeta = _topMeta;
	return DAE_OK;
}

daeBool
daeSTLDatabase::isDocumentLoaded(daeString name)
{
	daeDocument* document = getDocument(name);
	if(document)
		return(true);
	else
		return(false);
}

// Element Types of all Elements
daeUInt daeSTLDatabase::getTypeCount()
{
	return (daeUInt)elements.size();
}


daeString daeSTLDatabase::getTypeName(daeUInt index)
{
	daeUInt count = 0;
	
	map<string, vector< daeElement* > >::iterator iter = elements.begin();
	map<string, vector< daeElement* > >::iterator end = elements.end();
	while ( iter != end )
	{
		if ( count == index )
		{
			return (*iter).first.c_str();
		}
		++count;
		++iter;
	}

	return NULL;
}

// Documents
daeInt daeSTLDatabase::insertDocument(const char *name, daeElement* dom, daeDocument** document)
{
	return createDocument( name, dom, document );
}
daeInt daeSTLDatabase::createDocument(const char *name, daeElement* dom, daeDocument** document)
{
	// If a document already exists with the same name, error
	if(isDocumentLoaded(name))
	{
		if (document)
			*document = NULL;
		return DAE_ERR_COLLECTION_ALREADY_EXISTS;
	}
	
	// Make a new document
	daeDocument *newDocument = new daeDocument(dae);
	newDocument->getDocumentURI()->setURI(name);
	newDocument->setDomRoot(dom);
	// Push the connection into the database
	documents.push_back(newDocument);
	
	if (document)
		*document = newDocument;

	return DAE_OK;
}
// !!!GAC revised version of insertDocument, creates a domCollada and fills it in for you.
daeInt daeSTLDatabase::insertDocument(const char *name, daeDocument** document)
{
	return createDocument( name, document );
}
daeInt daeSTLDatabase::createDocument(const char *name, daeDocument** document)
{

	// If a document already exists with the same name, error
	if(isDocumentLoaded(name))
	{
		if (document)
			*document = NULL;
		return DAE_ERR_COLLECTION_ALREADY_EXISTS;
	}
	// Make the new document
	daeDocument *newDocument = new daeDocument(dae);
	// Make a domCOLLADA to be the root of this new document (this makes a reference so the domCOLLADA won't delete itself
	daeElementRef myCOLLADA = topMeta->create();
	myCOLLADA->setDocument(newDocument);
	newDocument->getDocumentURI()->setURI(name);
	newDocument->setDomRoot(myCOLLADA);

	// Add this document to the list.
	documents.push_back(newDocument);
	// If the user gave us a place to put the document, send it back to them.
	if (document)
		*document = newDocument;
	
	return DAE_OK;
}

daeInt daeSTLDatabase::insertDocument( daeDocument *c ) {
	documents.push_back(c);
	insertElement( c, c->getDomRoot() );
	return DAE_OK;
}

daeInt daeSTLDatabase::removeDocument(daeDocument *document)
{	
	vector< daeDocument* >::iterator iter = documents.begin();
	while ( iter != documents.end() ) {
		if ( (*iter) == document ) {
			//delete all of its children
			removeElement( *iter, (*iter)->getDomRoot() );
            delete *iter; // sthomas (see bug 1466019)
			iter = documents.erase(iter);
		}
		else {
            iter++;
		}
	}
	return DAE_OK;
}

daeUInt daeSTLDatabase::getDocumentCount()
{
	return (daeUInt)documents.size();
}

daeDocument* daeSTLDatabase::getDocument(daeUInt index)
{
	if (index<documents.size())
		return (documents[index]);
	else
		return NULL;
}

daeDocument* daeSTLDatabase::getDocument(daeString name_, bool skipUriNormalization)
{
	string name = name_;
	if (!skipUriNormalization) {
		// Normalize the input string to an absolute URI with no fragment
		name = daeURI(dae, name, true).str();
	}

	// Try to find a document that matches
	daeDocument *document;
	int documentCount	= getDocumentCount();
	for (int i=0;i<documentCount;i++)
	{
		document = getDocument(i);
		if(document->getDocumentURI()->str() == name)
			return(document);
	}
	return(NULL);
}

daeString daeSTLDatabase::getDocumentName(daeUInt index)
{
	if (index<documents.size())
		return getDocument(index)->getDocumentURI()->getURI();
	else
		return NULL;
}

// Elements
daeInt daeSTLDatabase::insertElement(daeDocument* document,daeElement* element)
{
	insertChildren( document, element );
	
	map<string, vector< daeElement* > >::iterator iter = elements.find( string( element->getTypeName() ) );
	if ( iter != elements.end() )
	{
		(*iter).second.push_back( element );
	}
	else
	{
		vector< daeElement* > vec;
		vec.push_back( element );
		elements.insert( make_pair( string( element->getTypeName() ), vec ) );
	}

	// Insert into the type ID map
	typeMap.insert(make_pair(element->typeID(), element));

	//insert into IDMap if element has an ID. IDMap is used to speed up URI resolution
	if ( element->getID() != NULL ) {
		elementsIDMap.insert( make_pair( string( element->getID() ), element ) );
	}

	// Insert into sid map if the element has a sid
// 	string sid = element->getAttribute("sid");
// 	if (!sid.empty())
// 		sidMap.insert(sidMapPair(sid, element));

	dae.getSidRefCache().clear();

	return DAE_OK;
}

daeInt daeSTLDatabase::insertChildren( daeDocument *c, daeElement *element )
{
	daeElementRefArray era;
	element->getChildren( era );
	for ( unsigned int i = 0; i < era.getCount(); i++ ) {
		insertElement( c, era[i] );
	}
	return DAE_OK;
}

daeInt daeSTLDatabase::removeElement(daeDocument* document,daeElement* element)
{
	if ( !element ) {
		return DAE_ERR_INVALID_CALL;
	}
	removeChildren( document, element );
	
	map<string, vector< daeElement* > >::iterator iter = elements.find( string( element->getTypeName() ) );
	if ( iter != elements.end() )
	{
		vector< daeElement* > &vec = (*iter).second;
		vector< daeElement* >::iterator i = vec.begin();
		vector< daeElement* >::iterator end = vec.end();
		while( i != end )
		{
			if ( (*i) == element )
			{
				vec.erase( i );
				break;
			}
			++i;
		}
	}

	typeMapRange range = typeMap.equal_range(element->typeID());
	for (typeMapIter iter = range.first; iter != range.second; iter++) {
		if (iter->second == element) {
			typeMap.erase(iter);
			break;
		}
	}

	if ( element->getID() != NULL ) {
		idMapRange range = elementsIDMap.equal_range( string( element->getID() ) );
		multimap<string, daeElement* >::iterator iter = range.first;
		while( iter != range.second ) {
			if ( (*iter).second == element ) {
				elementsIDMap.erase( iter );
				break;
			}
			++iter;
		}
	}

// 	string sid = element->getAttribute("sid");
// 	if (!sid.empty()) {
// 		pair<sidMapIter, sidMapIter> range = sidMap.equal_range(sid);
// 		for (sidMapIter iter = range.first; iter != range.second; iter++) {
// 			if (iter->second == element) {
// 				sidMap.erase(iter);
// 				break;
// 			}
// 		}
// 	}

	dae.getSidRefCache().clear();
	
	return DAE_OK;
}

daeInt daeSTLDatabase::removeChildren( daeDocument *c, daeElement *element )
{
	daeElementRefArray era;
	element->getChildren( era );
	for ( unsigned int i = 0; i < era.getCount(); i++ ) {
		removeElement( c, era[i] );	
	}
	return DAE_OK;
}

daeInt daeSTLDatabase::changeElementID( daeElement* element, daeString newID )
{
	if ( !element ) {
		return DAE_ERR_INVALID_CALL;
	}

	// Remove the current entry in the ID map if the element has an ID
	if ( element->getID() != NULL ) {
		pair< multimap<string, daeElement* >::iterator, multimap<string, daeElement* >::iterator> range;
		range = elementsIDMap.equal_range( string( element->getID() ) );
		multimap<string, daeElement* >::iterator iter = range.first;
		while( iter != range.second ) {
			if ( (*iter).second == element ) {
				elementsIDMap.erase( iter );
				break;
			}
			++iter;
		}
	}

	// Add an entry to the ID map if the element will have an ID
	if ( newID != NULL ) {
		elementsIDMap.insert( make_pair( string( newID ), element ) );
	}

	dae.getSidRefCache().clear();

	return DAE_OK;
}

daeInt daeSTLDatabase::changeElementSID(daeElement* element, daeString newSID) {
	if (!element)
		return DAE_ERR_INVALID_CALL;

// 	// Remove the current entry in the sid map if the element has a sid
// 	string sid = element->getAttribute("sid");
// 	if (!sid.empty()) {
// 		pair<sidMapIter, sidMapIter> range = sidMap.equal_range(sid);
// 		for (sidMapIter iter = range.first; iter != range.second; iter++) {
// 			if (iter->second == element) {
// 				sidMap.erase(iter);
// 				break;
// 			}
// 		}
// 	}

// 	// Add an entry to the sid map if the element will have a sid
// 	if ( newSID != NULL )
// 		sidMap.insert(sidMapPair(newSID, element));

	dae.getSidRefCache().clear();

	return DAE_OK;
}

daeInt daeSTLDatabase::clear()
{
	elements.clear();
	typeMap.clear();
	elementsIDMap.clear();
	sidMap.clear();
	int i;
	for (i=0;i<(int)documents.size();i++)
		delete documents[i];
	documents.clear(); //this will free the daeElement
	dae.getRawRefCache().clear();
	dae.getSidRefCache().clear();
	return DAE_OK;
}

daeUInt daeSTLDatabase::getElementCount(daeString name,daeString type,daeString file)
{	
	// If none of the search keys was specified, return the total element count in the database
	if ( !name && !type && !file ) 
	{
		daeUInt count = 0;
		map< string, vector< daeElement*> >::iterator iter = elements.begin();
		map< string, vector< daeElement*> >::iterator end = elements.end();
		while( iter != end )
		{
			count += (daeUInt)(*iter).second.size();
			++iter;
		}
		return count;
	}

	if ( name ) 
	{ 
		// name specified
		int count = 0;
		if ( file ) 
		{ 
			// If a document URI was a search key (in file) resolve it to a text URI with no fragment
			daeURI tempURI(dae, file,true);
			daeDocument *col = getDocument( tempURI.getURI() );
			if ( col == NULL ) {
				return 0;
			}
			// a document was specified
			pair< multimap< string, daeElement* >::iterator, multimap< string, daeElement* >::iterator > range;
			range = elementsIDMap.equal_range( string( name ) );
			multimap< string, daeElement* >::iterator i = range.first;
			while ( i != range.second )
			{
				if ( col == (*i).second->getDocument() )
				{
					count++;
				}
				++i;
			}
			return count;
		}
		else 
		{ 
			//no file specified - just name
			return (daeUInt)elementsIDMap.count( string( name ) );
		}
	}

	if ( type ) 
	{ 
		// type specified
		map< string, vector< daeElement*> >::iterator iter = elements.find( string( type ) );
		if ( iter == elements.end() )
		{
			return 0;
		}

		int count = 0;
		if ( file ) 
		{ 
			// If a document URI was a search key (in file) resolve it to a text URI with no fragment
			daeURI tempURI(dae, file,true);
			daeDocument *col = getDocument( tempURI.getURI() );
			if ( col == NULL ) {
				return 0;
			}
			// a document was specified
			vector< daeElement* > &vec = (*iter).second;
			vector< daeElement* >::iterator i = vec.begin();
			vector< daeElement* >::iterator end = vec.end();
			while( i != end )
			{
				if ( col == (*i)->getDocument() )
				{
					++count;
				}
				++i;
			}
			return count;
		}
		else 
		{ 
			//no file specified - just type
			return (daeUInt)(*iter).second.size();
		}
	}

	//if you get here only a file was specified
	daeURI tempURI(dae, file,true);
	daeDocument *col = getDocument( tempURI.getURI() );
	if ( col == NULL ) {
		return 0;
	}
	//a document was specified
	int count = 0;
	map< string, vector< daeElement*> >::iterator iter = elements.begin();
	map< string, vector< daeElement*> >::iterator end = elements.end();
	while( iter != end )
	{
		vector< daeElement* > &vec = (*iter).second;
		vector< daeElement* >::iterator i = vec.begin();
		vector< daeElement* >::iterator end2 = vec.end();
		while( i != end2 )
		{
			if( col == (*i)->getDocument() )
			{
				++count;
			}
			++i;
		}
		++iter;
	}
	return count;

}

daeInt daeSTLDatabase::getElement(daeElement** pElement,daeInt index,daeString name,daeString type,daeString file)
{	
	// If the index is out of range, there can be no match
	if ( index < 0 ) 
	{
		return DAE_ERR_QUERY_NO_MATCH;
	}

	// If no name, type or file was specified we return the element at "index" - SLOW
	if ( !name && !type && !file ) 
	{
		daeUInt count = 0;
		map< string, vector< daeElement*> >::iterator iter = elements.begin();
		map< string, vector< daeElement*> >::iterator end = elements.end();
		while( iter != end )
		{
			count += (daeUInt)(*iter).second.size();
			if ( (daeInt)count > index )
			{
				*pElement = (*iter).second[index - (count - (*iter).second.size())] ;
				return DAE_OK;
			}
			++iter;
		}
		return DAE_ERR_QUERY_NO_MATCH;
	}
	
	if ( name ) 
	{ 
		//name specified
		int count = 0;
		if ( file ) 
		{ 
			// If a document URI was a search key (in file) resolve it to a text URI with no fragment
			daeURI tempURI(dae, file, true);
			daeDocument *col = getDocument( tempURI.getURI() );
			if ( col == NULL ) {
				*pElement = NULL;
				return DAE_ERR_QUERY_NO_MATCH;
			}
			//a document was specified
			pair< multimap< string, daeElement* >::iterator, multimap< string, daeElement* >::iterator> range;
			range = elementsIDMap.equal_range( string( name ) );
			multimap< string, daeElement* >::iterator i = range.first;
			while ( i != range.second )
			{
				if ( col == (*i).second->getDocument() )
				{
					if ( count == index )
					{
						*pElement = (*i).second;
						return DAE_OK;
					}
					count++;
				}
				++i;
			}
			*pElement = NULL;
			return DAE_ERR_QUERY_NO_MATCH;
		}
		else 
		{ 
			//no document specified
			multimap< string, daeElement* >::iterator i = elementsIDMap.find( string( name ) );
			if ( index > (daeInt)elementsIDMap.count( string( name ) ) || i == elementsIDMap.end() )
			{
				*pElement = NULL;
				return DAE_ERR_QUERY_NO_MATCH;
			}
			for ( int x = 0; x < index; x++ )
			{
				++i;
			}
			*pElement = i->second;
			return DAE_OK;
		}
	}
	
	if ( type ) 
	{ 
		map< string, vector< daeElement*> >::iterator iter = elements.find( string( type ) );
		if ( iter == elements.end() )
		{
			*pElement = NULL;
			return DAE_ERR_QUERY_NO_MATCH;
		}
		//type specified
		int count = 0;
		if ( file ) 
		{ 
			// If a document URI was a search key (in file) resolve it to a text URI with no fragment
			daeURI tempURI(dae, file, true);
			daeDocument *col = getDocument( tempURI.getURI() );
			if ( col == NULL ) {
				return DAE_ERR_QUERY_NO_MATCH;
			}
			//a document was specified
			// a document was specified
			vector< daeElement* > &vec = (*iter).second;
			vector< daeElement* >::iterator i = vec.begin();
			vector< daeElement* >::iterator end = vec.end();
			while( i != end )
			{
				if ( col == (*i)->getDocument() )
				{
					if ( count == index )
					{
						*pElement = (*i);
						return DAE_OK;
					}
					++count;
				}
				++i;
			}
			return DAE_ERR_QUERY_NO_MATCH;
		}
		else 
		{ 
			//no document specified
			if ( index >= (daeInt)(*iter).second.size() )
			{
				*pElement = NULL;
				return DAE_ERR_QUERY_NO_MATCH;
			}
			*pElement = (*iter).second[index];
			return DAE_OK;
		}
	}

	//if you get here only the file was specified - SLOW
	daeURI tempURI(dae, file, true);
	daeDocument *col = getDocument( tempURI.getURI() );
	if ( col == NULL ) {
		return DAE_ERR_QUERY_NO_MATCH;
	}
	//a document was specified
	int count = 0;
	map< string, vector< daeElement*> >::iterator iter = elements.begin();
	map< string, vector< daeElement*> >::iterator end = elements.end();
	while( iter != end )
	{
		vector< daeElement* > &vec = (*iter).second;
		vector< daeElement* >::iterator i = vec.begin();
		vector< daeElement* >::iterator end2 = vec.end();
		while( i != end2 )
		{
			if( col == (*i)->getDocument() ) 
			{
				if( count == index ) 
				{
					*pElement = (*i);
					return DAE_OK;
				}
				++count;
			}
			++i;
		}
		++iter;
	}
	return DAE_ERR_QUERY_NO_MATCH;

}

vector<daeElement*> daeSTLDatabase::idLookup(const string& id) {
	vector<daeElement*> matchingElements;
	idMapRange range = elementsIDMap.equal_range(id);
	for (idMapIter iter = range.first; iter != range.second; iter++)
		matchingElements.push_back(iter->second);
	return matchingElements;
}

void daeSTLDatabase::typeLookup(daeInt typeID,
                                vector<daeElement*>& matchingElements,
                                daeDocument* doc) {
	matchingElements.clear();
	typeMapRange range = typeMap.equal_range(typeID);
	for (typeMapIter iter = range.first; iter != range.second; iter++)
		if (!doc  ||  doc == iter->second->getDocument())
			matchingElements.push_back(iter->second);
}

void daeSTLDatabase::sidLookup(const string& sid,
                               vector<daeElement*>& matchingElements,
                               daeDocument* doc) {
	matchingElements.clear();
	if (!sid.empty()) {
		sidMapRange range = sidMap.equal_range(sid);
		for (sidMapIter iter = range.first; iter != range.second; iter++)
			if (!doc  ||  doc == iter->second->getDocument())
				matchingElements.push_back(iter->second);
	}
}
