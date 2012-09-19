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
#include <dom/domSource.h>
#include <dae/daeMetaCMPolicy.h>
#include <dae/daeMetaSequence.h>
#include <dae/daeMetaChoice.h>
#include <dae/daeMetaGroup.h>
#include <dae/daeMetaAny.h>
#include <dae/daeMetaElementAttribute.h>

daeElementRef
domSource::create(DAE& dae)
{
	domSourceRef ref = new domSource(dae);
	return ref;
}


daeMetaElement *
domSource::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "source" );
	meta->registerClass(domSource::create);

	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 0, 1 );
	mea->setName( "asset" );
	mea->setOffset( daeOffsetOf(domSource,elemAsset) );
	mea->setElementType( domAsset::registerElement(dae) );
	cm->appendChild( mea );

	cm = new daeMetaChoice( meta, cm, 0, 1, 0, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "IDREF_array" );
	mea->setOffset( daeOffsetOf(domSource,elemIDREF_array) );
	mea->setElementType( domIDREF_array::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "Name_array" );
	mea->setOffset( daeOffsetOf(domSource,elemName_array) );
	mea->setElementType( domName_array::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "bool_array" );
	mea->setOffset( daeOffsetOf(domSource,elemBool_array) );
	mea->setElementType( domBool_array::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "float_array" );
	mea->setOffset( daeOffsetOf(domSource,elemFloat_array) );
	mea->setElementType( domFloat_array::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "int_array" );
	mea->setOffset( daeOffsetOf(domSource,elemInt_array) );
	mea->setElementType( domInt_array::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 0 );
	cm->getParent()->appendChild( cm );
	cm = cm->getParent();

	mea = new daeMetaElementAttribute( meta, cm, 2, 0, 1 );
	mea->setName( "technique_common" );
	mea->setOffset( daeOffsetOf(domSource,elemTechnique_common) );
	mea->setElementType( domSource::domTechnique_common::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 3, 0, -1 );
	mea->setName( "technique" );
	mea->setOffset( daeOffsetOf(domSource,elemTechnique_array) );
	mea->setElementType( domTechnique::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 3 );
	meta->setCMRoot( cm );	
	// Ordered list of sub-elements
	meta->addContents(daeOffsetOf(domSource,_contents));
	meta->addContentsOrder(daeOffsetOf(domSource,_contentsOrder));

	meta->addCMDataArray(daeOffsetOf(domSource,_CMData), 1);
	//	Add attribute: id
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "id" );
		ma->setType( dae.getAtomicTypes().get("xsID"));
		ma->setOffset( daeOffsetOf( domSource , attrId ));
		ma->setContainer( meta );
		ma->setIsRequired( true );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: name
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "name" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domSource , attrName ));
		ma->setContainer( meta );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domSource));
	meta->validate();

	return meta;
}

daeElementRef
domSource::domTechnique_common::create(DAE& dae)
{
	domSource::domTechnique_commonRef ref = new domSource::domTechnique_common(dae);
	return ref;
}


daeMetaElement *
domSource::domTechnique_common::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "technique_common" );
	meta->registerClass(domSource::domTechnique_common::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "accessor" );
	mea->setOffset( daeOffsetOf(domSource::domTechnique_common,elemAccessor) );
	mea->setElementType( domAccessor::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 0 );
	meta->setCMRoot( cm );	

	meta->setElementSize(sizeof(domSource::domTechnique_common));
	meta->validate();

	return meta;
}

