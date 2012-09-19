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
#include <dom/domCamera.h>
#include <dae/daeMetaCMPolicy.h>
#include <dae/daeMetaSequence.h>
#include <dae/daeMetaChoice.h>
#include <dae/daeMetaGroup.h>
#include <dae/daeMetaAny.h>
#include <dae/daeMetaElementAttribute.h>

daeElementRef
domCamera::create(DAE& dae)
{
	domCameraRef ref = new domCamera(dae);
	return ref;
}


daeMetaElement *
domCamera::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "camera" );
	meta->registerClass(domCamera::create);

	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 0, 1 );
	mea->setName( "asset" );
	mea->setOffset( daeOffsetOf(domCamera,elemAsset) );
	mea->setElementType( domAsset::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 1, 1, 1 );
	mea->setName( "optics" );
	mea->setOffset( daeOffsetOf(domCamera,elemOptics) );
	mea->setElementType( domCamera::domOptics::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 2, 0, 1 );
	mea->setName( "imager" );
	mea->setOffset( daeOffsetOf(domCamera,elemImager) );
	mea->setElementType( domCamera::domImager::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 3, 0, -1 );
	mea->setName( "extra" );
	mea->setOffset( daeOffsetOf(domCamera,elemExtra_array) );
	mea->setElementType( domExtra::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 3 );
	meta->setCMRoot( cm );	

	//	Add attribute: id
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "id" );
		ma->setType( dae.getAtomicTypes().get("xsID"));
		ma->setOffset( daeOffsetOf( domCamera , attrId ));
		ma->setContainer( meta );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: name
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "name" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domCamera , attrName ));
		ma->setContainer( meta );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCamera));
	meta->validate();

	return meta;
}

daeElementRef
domCamera::domOptics::create(DAE& dae)
{
	domCamera::domOpticsRef ref = new domCamera::domOptics(dae);
	return ref;
}


daeMetaElement *
domCamera::domOptics::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "optics" );
	meta->registerClass(domCamera::domOptics::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "technique_common" );
	mea->setOffset( daeOffsetOf(domCamera::domOptics,elemTechnique_common) );
	mea->setElementType( domCamera::domOptics::domTechnique_common::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 1, 0, -1 );
	mea->setName( "technique" );
	mea->setOffset( daeOffsetOf(domCamera::domOptics,elemTechnique_array) );
	mea->setElementType( domTechnique::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 2, 0, -1 );
	mea->setName( "extra" );
	mea->setOffset( daeOffsetOf(domCamera::domOptics,elemExtra_array) );
	mea->setElementType( domExtra::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 2 );
	meta->setCMRoot( cm );	

	meta->setElementSize(sizeof(domCamera::domOptics));
	meta->validate();

	return meta;
}

daeElementRef
domCamera::domOptics::domTechnique_common::create(DAE& dae)
{
	domCamera::domOptics::domTechnique_commonRef ref = new domCamera::domOptics::domTechnique_common(dae);
	return ref;
}


daeMetaElement *
domCamera::domOptics::domTechnique_common::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "technique_common" );
	meta->registerClass(domCamera::domOptics::domTechnique_common::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaChoice( meta, cm, 0, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "orthographic" );
	mea->setOffset( daeOffsetOf(domCamera::domOptics::domTechnique_common,elemOrthographic) );
	mea->setElementType( domCamera::domOptics::domTechnique_common::domOrthographic::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "perspective" );
	mea->setOffset( daeOffsetOf(domCamera::domOptics::domTechnique_common,elemPerspective) );
	mea->setElementType( domCamera::domOptics::domTechnique_common::domPerspective::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 0 );
	meta->setCMRoot( cm );	
	// Ordered list of sub-elements
	meta->addContents(daeOffsetOf(domCamera::domOptics::domTechnique_common,_contents));
	meta->addContentsOrder(daeOffsetOf(domCamera::domOptics::domTechnique_common,_contentsOrder));

	meta->addCMDataArray(daeOffsetOf(domCamera::domOptics::domTechnique_common,_CMData), 1);
	meta->setElementSize(sizeof(domCamera::domOptics::domTechnique_common));
	meta->validate();

	return meta;
}

daeElementRef
domCamera::domOptics::domTechnique_common::domOrthographic::create(DAE& dae)
{
	domCamera::domOptics::domTechnique_common::domOrthographicRef ref = new domCamera::domOptics::domTechnique_common::domOrthographic(dae);
	return ref;
}


daeMetaElement *
domCamera::domOptics::domTechnique_common::domOrthographic::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "orthographic" );
	meta->registerClass(domCamera::domOptics::domTechnique_common::domOrthographic::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	cm = new daeMetaChoice( meta, cm, 0, 0, 1, 1 );

	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "xmag" );
	mea->setOffset( daeOffsetOf(domCamera::domOptics::domTechnique_common::domOrthographic,elemXmag) );
	mea->setElementType( domTargetableFloat::registerElement(dae) );
	cm->appendChild( mea );

	cm = new daeMetaChoice( meta, cm, 1, 1, 0, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "ymag" );
	mea->setOffset( daeOffsetOf(domCamera::domOptics::domTechnique_common::domOrthographic,elemYmag) );
	mea->setElementType( domTargetableFloat::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "aspect_ratio" );
	mea->setOffset( daeOffsetOf(domCamera::domOptics::domTechnique_common::domOrthographic,elemAspect_ratio) );
	mea->setElementType( domTargetableFloat::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 0 );
	cm->getParent()->appendChild( cm );
	cm = cm->getParent();

	cm->setMaxOrdinal( 1 );
	cm->getParent()->appendChild( cm );
	cm = cm->getParent();

	cm = new daeMetaSequence( meta, cm, 2, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "ymag" );
	mea->setOffset( daeOffsetOf(domCamera::domOptics::domTechnique_common::domOrthographic,elemYmag) );
	mea->setElementType( domTargetableFloat::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 1, 0, 1 );
	mea->setName( "aspect_ratio" );
	mea->setOffset( daeOffsetOf(domCamera::domOptics::domTechnique_common::domOrthographic,elemAspect_ratio) );
	mea->setElementType( domTargetableFloat::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 1 );
	cm->getParent()->appendChild( cm );
	cm = cm->getParent();

	cm->setMaxOrdinal( 3 );
	cm->getParent()->appendChild( cm );
	cm = cm->getParent();

	mea = new daeMetaElementAttribute( meta, cm, 1, 1, 1 );
	mea->setName( "znear" );
	mea->setOffset( daeOffsetOf(domCamera::domOptics::domTechnique_common::domOrthographic,elemZnear) );
	mea->setElementType( domTargetableFloat::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 2, 1, 1 );
	mea->setName( "zfar" );
	mea->setOffset( daeOffsetOf(domCamera::domOptics::domTechnique_common::domOrthographic,elemZfar) );
	mea->setElementType( domTargetableFloat::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 2 );
	meta->setCMRoot( cm );	
	// Ordered list of sub-elements
	meta->addContents(daeOffsetOf(domCamera::domOptics::domTechnique_common::domOrthographic,_contents));
	meta->addContentsOrder(daeOffsetOf(domCamera::domOptics::domTechnique_common::domOrthographic,_contentsOrder));

	meta->addCMDataArray(daeOffsetOf(domCamera::domOptics::domTechnique_common::domOrthographic,_CMData), 2);
	meta->setElementSize(sizeof(domCamera::domOptics::domTechnique_common::domOrthographic));
	meta->validate();

	return meta;
}

daeElementRef
domCamera::domOptics::domTechnique_common::domPerspective::create(DAE& dae)
{
	domCamera::domOptics::domTechnique_common::domPerspectiveRef ref = new domCamera::domOptics::domTechnique_common::domPerspective(dae);
	return ref;
}


daeMetaElement *
domCamera::domOptics::domTechnique_common::domPerspective::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "perspective" );
	meta->registerClass(domCamera::domOptics::domTechnique_common::domPerspective::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	cm = new daeMetaChoice( meta, cm, 0, 0, 1, 1 );

	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "xfov" );
	mea->setOffset( daeOffsetOf(domCamera::domOptics::domTechnique_common::domPerspective,elemXfov) );
	mea->setElementType( domTargetableFloat::registerElement(dae) );
	cm->appendChild( mea );

	cm = new daeMetaChoice( meta, cm, 1, 1, 0, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "yfov" );
	mea->setOffset( daeOffsetOf(domCamera::domOptics::domTechnique_common::domPerspective,elemYfov) );
	mea->setElementType( domTargetableFloat::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "aspect_ratio" );
	mea->setOffset( daeOffsetOf(domCamera::domOptics::domTechnique_common::domPerspective,elemAspect_ratio) );
	mea->setElementType( domTargetableFloat::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 0 );
	cm->getParent()->appendChild( cm );
	cm = cm->getParent();

	cm->setMaxOrdinal( 1 );
	cm->getParent()->appendChild( cm );
	cm = cm->getParent();

	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "yfov" );
	mea->setOffset( daeOffsetOf(domCamera::domOptics::domTechnique_common::domPerspective,elemYfov) );
	mea->setElementType( domTargetableFloat::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 1, 0, 1 );
	mea->setName( "aspect_ratio" );
	mea->setOffset( daeOffsetOf(domCamera::domOptics::domTechnique_common::domPerspective,elemAspect_ratio) );
	mea->setElementType( domTargetableFloat::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 1 );
	cm->getParent()->appendChild( cm );
	cm = cm->getParent();

	cm->setMaxOrdinal( 3 );
	cm->getParent()->appendChild( cm );
	cm = cm->getParent();

	mea = new daeMetaElementAttribute( meta, cm, 1, 1, 1 );
	mea->setName( "znear" );
	mea->setOffset( daeOffsetOf(domCamera::domOptics::domTechnique_common::domPerspective,elemZnear) );
	mea->setElementType( domTargetableFloat::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 2, 1, 1 );
	mea->setName( "zfar" );
	mea->setOffset( daeOffsetOf(domCamera::domOptics::domTechnique_common::domPerspective,elemZfar) );
	mea->setElementType( domTargetableFloat::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 2 );
	meta->setCMRoot( cm );	
	// Ordered list of sub-elements
	meta->addContents(daeOffsetOf(domCamera::domOptics::domTechnique_common::domPerspective,_contents));
	meta->addContentsOrder(daeOffsetOf(domCamera::domOptics::domTechnique_common::domPerspective,_contentsOrder));

	meta->addCMDataArray(daeOffsetOf(domCamera::domOptics::domTechnique_common::domPerspective,_CMData), 2);
	meta->setElementSize(sizeof(domCamera::domOptics::domTechnique_common::domPerspective));
	meta->validate();

	return meta;
}

daeElementRef
domCamera::domImager::create(DAE& dae)
{
	domCamera::domImagerRef ref = new domCamera::domImager(dae);
	return ref;
}


daeMetaElement *
domCamera::domImager::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "imager" );
	meta->registerClass(domCamera::domImager::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, -1 );
	mea->setName( "technique" );
	mea->setOffset( daeOffsetOf(domCamera::domImager,elemTechnique_array) );
	mea->setElementType( domTechnique::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 1, 0, -1 );
	mea->setName( "extra" );
	mea->setOffset( daeOffsetOf(domCamera::domImager,elemExtra_array) );
	mea->setElementType( domExtra::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 1 );
	meta->setCMRoot( cm );	

	meta->setElementSize(sizeof(domCamera::domImager));
	meta->validate();

	return meta;
}

