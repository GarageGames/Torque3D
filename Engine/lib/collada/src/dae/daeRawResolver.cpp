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

#include <dae/daeRawResolver.h>
#include <dae.h>
#include <dae/daeURI.h>
#include <dae/daeErrorHandler.h>
#include <dae/daeUtils.h>

using namespace std;

daeRawResolver::daeRawResolver(DAE& dae) : daeURIResolver(dae)
{
}

daeRawResolver::~daeRawResolver()
{
}

daeString
daeRawResolver::getName()
{
	return "RawResolver";
}

daeElement* daeRawResolver::resolveElement(const daeURI& uri) {
	if (cdom::tolower(uri.pathExt()).find(".raw") == string::npos)
		return NULL;

	daeRawRefCache& cache = dae->getRawRefCache();
	if (daeElement* elt = cache.lookup(uri))
		return elt;
	
	string fileName = cdom::uriToNativePath(uri.str());
	if (fileName.empty())
	{
		daeErrorHandler::get()->handleError( "daeRawResolver::resolveElement() - Can't get path from URI\n" );
		return NULL;
	}
	FILE *rawFile = fopen(fileName.c_str(), "rb");
	if (rawFile == NULL )
		return NULL;
	long byteOffset = atoi( uri.getID() ); //get the fragment

	daeElement *src;
	daeElement *array;
	daeElement *accessor;
	
	accessor = uri.getContainer();
	if ( accessor == NULL )
		return NULL;
	src = accessor->getParentElement()->getParentElement();
	daeElementRefArray children;
	accessor->getChildren( children );
	bool hasInts = children[0]->getAttribute("type") == "int";

	if ( hasInts )
	{
		array = src->createAndPlace( "int_array" );
	}
	else
	{
		array = src->createAndPlace( "float_array" );
	}
	
	daeULong *countPtr = (daeULong*)accessor->getAttributeValue( "count" );
	daeULong count = countPtr != NULL ? *countPtr : 0;

	daeULong *stridePtr = (daeULong*)accessor->getAttributeValue( "stride" );
	daeULong stride = stridePtr != NULL ? *stridePtr : 1;

	*(daeULong*)(array->getAttributeValue("count")) = count*stride;
	array->setAttribute( "id", (src->getAttribute("id") + "-array").c_str() );

	daeArray *valArray = (daeArray*)array->getValuePointer();
	valArray->setCount( (size_t)(count*stride) );

	fseek( rawFile, byteOffset, SEEK_SET );
	if ( hasInts )
	{
		daeInt val;
		for ( unsigned int i = 0; i < count*stride; i++ )
		{
			fread( &val, sizeof(daeInt), 1, rawFile );
			*(daeLong*)(valArray->getRaw(i)) = (daeLong)val;
		}
	}
	else
	{
		daeFloat val;
		for ( unsigned int i = 0; i < count*stride; i++ )
		{
			fread( &val, sizeof(daeFloat), 1, rawFile );
			*(daeDouble*)(valArray->getRaw(i)) = (daeDouble)val;
		}
	}

	fclose(rawFile);
	cache.add(uri, array);
	return array;
}


daeElement* daeRawRefCache::lookup(const daeURI& uri) {
	map<string, daeElement*>::iterator iter = lookupTable.find(uri.str());
	return iter == lookupTable.end() ? NULL : iter->second;
}

void daeRawRefCache::add(const daeURI& uri, daeElement* elt) {
	lookupTable[uri.str()] = elt;
}

void daeRawRefCache::remove(const daeURI& uri) {
	lookupTable.erase(uri.str());
}

void daeRawRefCache::clear() {
	lookupTable.clear();
}
