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
#include <dom/domFx_surface_init_cube_common.h>
#include <dae/daeMetaCMPolicy.h>
#include <dae/daeMetaSequence.h>
#include <dae/daeMetaChoice.h>
#include <dae/daeMetaGroup.h>
#include <dae/daeMetaAny.h>
#include <dae/daeMetaElementAttribute.h>

daeElementRef
domFx_surface_init_cube_common::create(DAE& dae)
{
	domFx_surface_init_cube_commonRef ref = new domFx_surface_init_cube_common(dae);
	return ref;
}


daeMetaElement *
domFx_surface_init_cube_common::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "fx_surface_init_cube_common" );
	meta->registerClass(domFx_surface_init_cube_common::create);

	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaChoice( meta, cm, 0, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "all" );
	mea->setOffset( daeOffsetOf(domFx_surface_init_cube_common,elemAll) );
	mea->setElementType( domFx_surface_init_cube_common::domAll::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "primary" );
	mea->setOffset( daeOffsetOf(domFx_surface_init_cube_common,elemPrimary) );
	mea->setElementType( domFx_surface_init_cube_common::domPrimary::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 6, 6 );
	mea->setName( "face" );
	mea->setOffset( daeOffsetOf(domFx_surface_init_cube_common,elemFace_array) );
	mea->setElementType( domFx_surface_init_cube_common::domFace::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 0 );
	meta->setCMRoot( cm );	
	// Ordered list of sub-elements
	meta->addContents(daeOffsetOf(domFx_surface_init_cube_common,_contents));
	meta->addContentsOrder(daeOffsetOf(domFx_surface_init_cube_common,_contentsOrder));

	meta->addCMDataArray(daeOffsetOf(domFx_surface_init_cube_common,_CMData), 1);
	meta->setElementSize(sizeof(domFx_surface_init_cube_common));
	meta->validate();

	return meta;
}

daeElementRef
domFx_surface_init_cube_common::domAll::create(DAE& dae)
{
	domFx_surface_init_cube_common::domAllRef ref = new domFx_surface_init_cube_common::domAll(dae);
	return ref;
}


daeMetaElement *
domFx_surface_init_cube_common::domAll::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "all" );
	meta->registerClass(domFx_surface_init_cube_common::domAll::create);

	meta->setIsInnerClass( true );

	//	Add attribute: ref
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "ref" );
		ma->setType( dae.getAtomicTypes().get("xsIDREF"));
		ma->setOffset( daeOffsetOf( domFx_surface_init_cube_common::domAll , attrRef ));
		ma->setContainer( meta );
		ma->setIsRequired( true );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domFx_surface_init_cube_common::domAll));
	meta->validate();

	return meta;
}

daeElementRef
domFx_surface_init_cube_common::domPrimary::create(DAE& dae)
{
	domFx_surface_init_cube_common::domPrimaryRef ref = new domFx_surface_init_cube_common::domPrimary(dae);
	return ref;
}


daeMetaElement *
domFx_surface_init_cube_common::domPrimary::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "primary" );
	meta->registerClass(domFx_surface_init_cube_common::domPrimary::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 0, 1 );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 6, 6 );
	mea->setName( "order" );
	mea->setOffset( daeOffsetOf(domFx_surface_init_cube_common::domPrimary,elemOrder_array) );
	mea->setElementType( domFx_surface_init_cube_common::domPrimary::domOrder::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 0 );
	meta->setCMRoot( cm );	

	//	Add attribute: ref
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "ref" );
		ma->setType( dae.getAtomicTypes().get("xsIDREF"));
		ma->setOffset( daeOffsetOf( domFx_surface_init_cube_common::domPrimary , attrRef ));
		ma->setContainer( meta );
		ma->setIsRequired( true );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domFx_surface_init_cube_common::domPrimary));
	meta->validate();

	return meta;
}

daeElementRef
domFx_surface_init_cube_common::domPrimary::domOrder::create(DAE& dae)
{
	domFx_surface_init_cube_common::domPrimary::domOrderRef ref = new domFx_surface_init_cube_common::domPrimary::domOrder(dae);
	return ref;
}


daeMetaElement *
domFx_surface_init_cube_common::domPrimary::domOrder::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "order" );
	meta->registerClass(domFx_surface_init_cube_common::domPrimary::domOrder::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Fx_surface_face_enum"));
		ma->setOffset( daeOffsetOf( domFx_surface_init_cube_common::domPrimary::domOrder , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domFx_surface_init_cube_common::domPrimary::domOrder));
	meta->validate();

	return meta;
}

daeElementRef
domFx_surface_init_cube_common::domFace::create(DAE& dae)
{
	domFx_surface_init_cube_common::domFaceRef ref = new domFx_surface_init_cube_common::domFace(dae);
	return ref;
}


daeMetaElement *
domFx_surface_init_cube_common::domFace::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "face" );
	meta->registerClass(domFx_surface_init_cube_common::domFace::create);

	meta->setIsInnerClass( true );

	//	Add attribute: ref
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "ref" );
		ma->setType( dae.getAtomicTypes().get("xsIDREF"));
		ma->setOffset( daeOffsetOf( domFx_surface_init_cube_common::domFace , attrRef ));
		ma->setContainer( meta );
		ma->setIsRequired( true );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domFx_surface_init_cube_common::domFace));
	meta->validate();

	return meta;
}

