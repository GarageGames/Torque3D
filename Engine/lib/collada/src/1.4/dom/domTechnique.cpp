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
#include <dae/daeDom.h>
#include <dom/domTechnique.h>
#include <dae/daeMetaCMPolicy.h>
#include <dae/daeMetaSequence.h>
#include <dae/daeMetaChoice.h>
#include <dae/daeMetaGroup.h>
#include <dae/daeMetaAny.h>
#include <dae/daeMetaElementAttribute.h>

daeElementRef
domTechnique::create(DAE& dae)
{
	domTechniqueRef ref = new domTechnique(dae);
	return ref;
}


daeMetaElement *
domTechnique::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "technique" );
	meta->registerClass(domTechnique::create);

	daeMetaCMPolicy *cm = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	cm = new daeMetaAny( meta, cm, 0, 0, -1 );

	cm->setMaxOrdinal( 0 );
	cm->getParent()->appendChild( cm );
	cm = cm->getParent();

	cm->setMaxOrdinal( 0 );
	meta->setCMRoot( cm );	
	meta->setAllowsAny( true );
	// Ordered list of sub-elements
	meta->addContents(daeOffsetOf(domTechnique,_contents));
	meta->addContentsOrder(daeOffsetOf(domTechnique,_contentsOrder));

	//	Add attribute: xmlns
	{
		daeMetaAttribute* ma = new daeMetaAttribute;
		ma->setName( "xmlns" );
		ma->setType( dae.getAtomicTypes().get("xsAnyURI"));
		ma->setOffset( daeOffsetOf( domTechnique , attrXmlns ));
		ma->setContainer( meta );
		//ma->setIsRequired( true );
		meta->appendAttribute(ma);
	}
    
	//	Add attribute: profile
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "profile" );
		ma->setType( dae.getAtomicTypes().get("xsNMTOKEN"));
		ma->setOffset( daeOffsetOf( domTechnique , attrProfile ));
		ma->setContainer( meta );
		ma->setIsRequired( true );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domTechnique));
	meta->validate();

	return meta;
}

