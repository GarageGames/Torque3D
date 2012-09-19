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

#include <sstream>
#include <dae.h>
#include <dae/daeStandardURIResolver.h>
#include <dae/daeDatabase.h>
#include <dae/daeURI.h>
#include <dae/daeIOPlugin.h>
#include <dae/daeErrorHandler.h>

using namespace std;

daeStandardURIResolver::daeStandardURIResolver(DAE& dae)
	: daeURIResolver(dae) { }

daeStandardURIResolver::~daeStandardURIResolver() { }

daeString
daeStandardURIResolver::getName()
{
	return "XMLResolver";
}

namespace {
	void printErrorMsg(const daeURI& uri) {
		ostringstream msg;
		msg << "daeStandardURIResolver::resolveElement() - Failed to resolve " << uri.str() << endl;
		daeErrorHandler::get()->handleError(msg.str().c_str());
	}
}

daeElement* daeStandardURIResolver::resolveElement(const daeURI& uri) {
	daeDocument* doc = uri.getReferencedDocument();
	if (!doc) {
		dae->open(uri.str());
		doc = uri.getReferencedDocument();
		if (!doc) {
			printErrorMsg(uri);
			return NULL;
		}
	}

	daeElement* elt = dae->getDatabase()->idLookup(uri.id(), doc);
	if (!elt)
		printErrorMsg(uri);

	return elt;
}
