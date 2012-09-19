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

#include <dae.h>
#include <dae/daeDocument.h>
#include <dae/daeDatabase.h>


daeDocument::daeDocument(DAE& dae) : dae(&dae), uri(dae) { }

daeDocument::~daeDocument() {
}

void daeDocument::insertElement( daeElementRef element ) {
	dae->getDatabase()->insertElement( this, element.cast() );
}

void daeDocument::removeElement( daeElementRef element ) {
	dae->getDatabase()->removeElement( this, element.cast() );
}

void daeDocument::changeElementID( daeElementRef element, daeString newID ) {
	dae->getDatabase()->changeElementID( element.cast(), newID );
}

void daeDocument::changeElementSID( daeElementRef element, daeString newSID ) {
	dae->getDatabase()->changeElementSID( element.cast(), newSID );
}

void daeDocument::addExternalReference( daeURI &uri ) {
	if ( uri.getContainer() == NULL || uri.getContainer()->getDocument() != this ) {
		return;	
	}	
	daeURI tempURI( *dae, uri.getURI(), true );  // Remove fragment
   referencedDocuments.appendUnique( tempURI.getURI() );
}

DAE* daeDocument::getDAE() {
	return dae;
}

daeDatabase* daeDocument::getDatabase() {
	return dae->getDatabase();
}
