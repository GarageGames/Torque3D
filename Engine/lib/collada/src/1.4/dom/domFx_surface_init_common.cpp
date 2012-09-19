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
#include <dom/domFx_surface_init_common.h>
#include <dae/daeMetaCMPolicy.h>
#include <dae/daeMetaSequence.h>
#include <dae/daeMetaChoice.h>
#include <dae/daeMetaGroup.h>
#include <dae/daeMetaAny.h>
#include <dae/daeMetaElementAttribute.h>

daeElementRef
domFx_surface_init_common::create(DAE& dae)
{
	domFx_surface_init_commonRef ref = new domFx_surface_init_common(dae);
	return ref;
}


daeMetaElement *
domFx_surface_init_common::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "fx_surface_init_common" );
	meta->registerClass(domFx_surface_init_common::create);

	meta->setIsTransparent( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaChoice( meta, cm, 0, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "init_as_null" );
	mea->setOffset( daeOffsetOf(domFx_surface_init_common,elemInit_as_null) );
	mea->setElementType( domFx_surface_init_common::domInit_as_null::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "init_as_target" );
	mea->setOffset( daeOffsetOf(domFx_surface_init_common,elemInit_as_target) );
	mea->setElementType( domFx_surface_init_common::domInit_as_target::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "init_cube" );
	mea->setOffset( daeOffsetOf(domFx_surface_init_common,elemInit_cube) );
	mea->setElementType( domFx_surface_init_cube_common::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "init_volume" );
	mea->setOffset( daeOffsetOf(domFx_surface_init_common,elemInit_volume) );
	mea->setElementType( domFx_surface_init_volume_common::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "init_planar" );
	mea->setOffset( daeOffsetOf(domFx_surface_init_common,elemInit_planar) );
	mea->setElementType( domFx_surface_init_planar_common::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, -1 );
	mea->setName( "init_from" );
	mea->setOffset( daeOffsetOf(domFx_surface_init_common,elemInit_from_array) );
	mea->setElementType( domFx_surface_init_from_common::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 0 );
	meta->setCMRoot( cm );	
	// Ordered list of sub-elements
	meta->addContents(daeOffsetOf(domFx_surface_init_common,_contents));
	meta->addContentsOrder(daeOffsetOf(domFx_surface_init_common,_contentsOrder));

	meta->addCMDataArray(daeOffsetOf(domFx_surface_init_common,_CMData), 1);
	meta->setElementSize(sizeof(domFx_surface_init_common));
	meta->validate();

	return meta;
}

daeElementRef
domFx_surface_init_common::domInit_as_null::create(DAE& dae)
{
	domFx_surface_init_common::domInit_as_nullRef ref = new domFx_surface_init_common::domInit_as_null(dae);
	return ref;
}


daeMetaElement *
domFx_surface_init_common::domInit_as_null::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "init_as_null" );
	meta->registerClass(domFx_surface_init_common::domInit_as_null::create);

	meta->setIsInnerClass( true );

	meta->setElementSize(sizeof(domFx_surface_init_common::domInit_as_null));
	meta->validate();

	return meta;
}

daeElementRef
domFx_surface_init_common::domInit_as_target::create(DAE& dae)
{
	domFx_surface_init_common::domInit_as_targetRef ref = new domFx_surface_init_common::domInit_as_target(dae);
	return ref;
}


daeMetaElement *
domFx_surface_init_common::domInit_as_target::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "init_as_target" );
	meta->registerClass(domFx_surface_init_common::domInit_as_target::create);

	meta->setIsInnerClass( true );

	meta->setElementSize(sizeof(domFx_surface_init_common::domInit_as_target));
	meta->validate();

	return meta;
}

