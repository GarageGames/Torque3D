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

#include "dae/daeDatabase.h"
using namespace std;

daeDatabase::daeDatabase(DAE& dae) : dae(dae) { }

DAE* daeDatabase::getDAE() {
	return &dae;
}

daeDocument* daeDatabase::getDoc(daeUInt index) {
	return getDocument(index);
}

daeElement* daeDatabase::idLookup(const string& id, daeDocument* doc) {
	vector<daeElement*> elts = idLookup(id);
	for (size_t i = 0; i < elts.size(); i++)
		if (elts[i]->getDocument() == doc)
			return elts[i];
	return NULL;
}

vector<daeElement*> daeDatabase::typeLookup(daeInt typeID, daeDocument* doc) {
	vector<daeElement*> result;
	typeLookup(typeID, result);
	return result;
}

vector<daeElement*> daeDatabase::sidLookup(const string& sid, daeDocument* doc) {
	vector<daeElement*> result;
	sidLookup(sid, result, doc);
	return result;
}
