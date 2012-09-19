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
#include <dom/domAnimation.h>
#include <dae/daeMetaCMPolicy.h>
#include <dae/daeMetaSequence.h>
#include <dae/daeMetaChoice.h>
#include <dae/daeMetaGroup.h>
#include <dae/daeMetaAny.h>
#include <dae/daeMetaElementAttribute.h>

daeElementRef
domAnimation::create(DAE& dae)
{
	domAnimationRef ref = new domAnimation(dae);
	return ref;
}


daeMetaElement *
domAnimation::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "animation" );
	meta->registerClass(domAnimation::create);

	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 0, 1 );
	mea->setName( "asset" );
	mea->setOffset( daeOffsetOf(domAnimation,elemAsset) );
	mea->setElementType( domAsset::registerElement(dae) );
	cm->appendChild( mea );

	cm = new daeMetaChoice( meta, cm, 0, 1, 1, 1 );

	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, -1 );
	mea->setName( "source" );
	mea->setOffset( daeOffsetOf(domAnimation,elemSource_array) );
	mea->setElementType( domSource::registerElement(dae) );
	cm->appendChild( mea );

	cm = new daeMetaChoice( meta, cm, 1, 1, 1, 1 );

	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, -1 );
	mea->setName( "sampler" );
	mea->setOffset( daeOffsetOf(domAnimation,elemSampler_array) );
	mea->setElementType( domSampler::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 1, 1, -1 );
	mea->setName( "channel" );
	mea->setOffset( daeOffsetOf(domAnimation,elemChannel_array) );
	mea->setElementType( domChannel::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 2, 0, -1 );
	mea->setName( "animation" );
	mea->setOffset( daeOffsetOf(domAnimation,elemAnimation_array) );
	mea->setElementType( domAnimation::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 2 );
	cm->getParent()->appendChild( cm );
	cm = cm->getParent();

	mea = new daeMetaElementArrayAttribute( meta, cm, 3, 1, -1 );
	mea->setName( "animation" );
	mea->setOffset( daeOffsetOf(domAnimation,elemAnimation_array) );
	mea->setElementType( domAnimation::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 2 );
	cm->getParent()->appendChild( cm );
	cm = cm->getParent();

	cm->setMaxOrdinal( 1 );
	cm->getParent()->appendChild( cm );
	cm = cm->getParent();

	cm = new daeMetaSequence( meta, cm, 2, 1, 1 );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, -1 );
	mea->setName( "sampler" );
	mea->setOffset( daeOffsetOf(domAnimation,elemSampler_array) );
	mea->setElementType( domSampler::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 1, 1, -1 );
	mea->setName( "channel" );
	mea->setOffset( daeOffsetOf(domAnimation,elemChannel_array) );
	mea->setElementType( domChannel::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 2, 0, -1 );
	mea->setName( "animation" );
	mea->setOffset( daeOffsetOf(domAnimation,elemAnimation_array) );
	mea->setElementType( domAnimation::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 2 );
	cm->getParent()->appendChild( cm );
	cm = cm->getParent();

	mea = new daeMetaElementArrayAttribute( meta, cm, 5, 1, -1 );
	mea->setName( "animation" );
	mea->setOffset( daeOffsetOf(domAnimation,elemAnimation_array) );
	mea->setElementType( domAnimation::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 4 );
	cm->getParent()->appendChild( cm );
	cm = cm->getParent();

	mea = new daeMetaElementArrayAttribute( meta, cm, 2, 0, -1 );
	mea->setName( "extra" );
	mea->setOffset( daeOffsetOf(domAnimation,elemExtra_array) );
	mea->setElementType( domExtra::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 2 );
	meta->setCMRoot( cm );	
	// Ordered list of sub-elements
	meta->addContents(daeOffsetOf(domAnimation,_contents));
	meta->addContentsOrder(daeOffsetOf(domAnimation,_contentsOrder));

	meta->addCMDataArray(daeOffsetOf(domAnimation,_CMData), 2);
	//	Add attribute: id
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "id" );
		ma->setType( dae.getAtomicTypes().get("xsID"));
		ma->setOffset( daeOffsetOf( domAnimation , attrId ));
		ma->setContainer( meta );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: name
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "name" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domAnimation , attrName ));
		ma->setContainer( meta );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domAnimation));
	meta->validate();

	return meta;
}

