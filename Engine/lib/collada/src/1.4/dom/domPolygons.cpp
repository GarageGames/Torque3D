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
#include <dom/domPolygons.h>
#include <dae/daeMetaCMPolicy.h>
#include <dae/daeMetaSequence.h>
#include <dae/daeMetaChoice.h>
#include <dae/daeMetaGroup.h>
#include <dae/daeMetaAny.h>
#include <dae/daeMetaElementAttribute.h>

daeElementRef
domPolygons::create(DAE& dae)
{
	domPolygonsRef ref = new domPolygons(dae);
	return ref;
}


daeMetaElement *
domPolygons::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "polygons" );
	meta->registerClass(domPolygons::create);

	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 0, -1 );
	mea->setName( "input" );
	mea->setOffset( daeOffsetOf(domPolygons,elemInput_array) );
	mea->setElementType( domInputLocalOffset::registerElement(dae) );
	cm->appendChild( mea );

	cm = new daeMetaChoice( meta, cm, 0, 1, 0, -1 );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "p" );
	mea->setOffset( daeOffsetOf(domPolygons,elemP_array) );
	mea->setElementType( domP::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "ph" );
	mea->setOffset( daeOffsetOf(domPolygons,elemPh_array) );
	mea->setElementType( domPolygons::domPh::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 0 );
	cm->getParent()->appendChild( cm );
	cm = cm->getParent();

	mea = new daeMetaElementArrayAttribute( meta, cm, 3002, 0, -1 );
	mea->setName( "extra" );
	mea->setOffset( daeOffsetOf(domPolygons,elemExtra_array) );
	mea->setElementType( domExtra::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 3002 );
	meta->setCMRoot( cm );	
	// Ordered list of sub-elements
	meta->addContents(daeOffsetOf(domPolygons,_contents));
	meta->addContentsOrder(daeOffsetOf(domPolygons,_contentsOrder));

	meta->addCMDataArray(daeOffsetOf(domPolygons,_CMData), 1);
	//	Add attribute: name
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "name" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domPolygons , attrName ));
		ma->setContainer( meta );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: count
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "count" );
		ma->setType( dae.getAtomicTypes().get("Uint"));
		ma->setOffset( daeOffsetOf( domPolygons , attrCount ));
		ma->setContainer( meta );
		ma->setIsRequired( true );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: material
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "material" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domPolygons , attrMaterial ));
		ma->setContainer( meta );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domPolygons));
	meta->validate();

	return meta;
}

daeElementRef
domPolygons::domPh::create(DAE& dae)
{
	domPolygons::domPhRef ref = new domPolygons::domPh(dae);
	return ref;
}


daeMetaElement *
domPolygons::domPh::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "ph" );
	meta->registerClass(domPolygons::domPh::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "p" );
	mea->setOffset( daeOffsetOf(domPolygons::domPh,elemP) );
	mea->setElementType( domP::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 1, 1, -1 );
	mea->setName( "h" );
	mea->setOffset( daeOffsetOf(domPolygons::domPh,elemH_array) );
	mea->setElementType( domPolygons::domPh::domH::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 1 );
	meta->setCMRoot( cm );	

	meta->setElementSize(sizeof(domPolygons::domPh));
	meta->validate();

	return meta;
}

daeElementRef
domPolygons::domPh::domH::create(DAE& dae)
{
	domPolygons::domPh::domHRef ref = new domPolygons::domPh::domH(dae);
	return ref;
}


daeMetaElement *
domPolygons::domPh::domH::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "h" );
	meta->registerClass(domPolygons::domPh::domH::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("ListOfUInts"));
		ma->setOffset( daeOffsetOf( domPolygons::domPh::domH , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domPolygons::domPh::domH));
	meta->validate();

	return meta;
}

