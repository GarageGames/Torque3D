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
#include <dom/domEffect.h>
#include <dae/daeMetaCMPolicy.h>
#include <dae/daeMetaSequence.h>
#include <dae/daeMetaChoice.h>
#include <dae/daeMetaGroup.h>
#include <dae/daeMetaAny.h>
#include <dae/daeMetaElementAttribute.h>

daeElementRef
domEffect::create(DAE& dae)
{
	domEffectRef ref = new domEffect(dae);
	return ref;
}

#include <dom/domProfile_GLSL.h>
#include <dom/domProfile_COMMON.h>
#include <dom/domProfile_CG.h>
#include <dom/domProfile_GLES.h>

daeMetaElement *
domEffect::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "effect" );
	meta->registerClass(domEffect::create);

	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 0, 1 );
	mea->setName( "asset" );
	mea->setOffset( daeOffsetOf(domEffect,elemAsset) );
	mea->setElementType( domAsset::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 1, 0, -1 );
	mea->setName( "annotate" );
	mea->setOffset( daeOffsetOf(domEffect,elemAnnotate_array) );
	mea->setElementType( domFx_annotate_common::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 2, 0, -1 );
	mea->setName( "image" );
	mea->setOffset( daeOffsetOf(domEffect,elemImage_array) );
	mea->setElementType( domImage::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 3, 0, -1 );
	mea->setName( "newparam" );
	mea->setOffset( daeOffsetOf(domEffect,elemNewparam_array) );
	mea->setElementType( domFx_newparam_common::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 4, 1, -1 );
	mea->setName( "fx_profile_abstract" );
	mea->setOffset( daeOffsetOf(domEffect,elemFx_profile_abstract_array) );
	mea->setElementType( domFx_profile_abstract::registerElement(dae) );
	cm->appendChild( mea );

    
	mea = new daeMetaElementArrayAttribute( meta, cm, 4, 1, -1 );
	mea->setName( "profile_GLSL" );
	mea->setOffset( daeOffsetOf(domEffect,elemFx_profile_abstract_array) );
	mea->setElementType( domProfile_GLSL::registerElement(dae) );
	cm->appendChild( mea );

    
	mea = new daeMetaElementArrayAttribute( meta, cm, 4, 1, -1 );
	mea->setName( "profile_COMMON" );
	mea->setOffset( daeOffsetOf(domEffect,elemFx_profile_abstract_array) );
	mea->setElementType( domProfile_COMMON::registerElement(dae) );
	cm->appendChild( mea );

    
	mea = new daeMetaElementArrayAttribute( meta, cm, 4, 1, -1 );
	mea->setName( "profile_CG" );
	mea->setOffset( daeOffsetOf(domEffect,elemFx_profile_abstract_array) );
	mea->setElementType( domProfile_CG::registerElement(dae) );
	cm->appendChild( mea );

    
	mea = new daeMetaElementArrayAttribute( meta, cm, 4, 1, -1 );
	mea->setName( "profile_GLES" );
	mea->setOffset( daeOffsetOf(domEffect,elemFx_profile_abstract_array) );
	mea->setElementType( domProfile_GLES::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 5, 0, -1 );
	mea->setName( "extra" );
	mea->setOffset( daeOffsetOf(domEffect,elemExtra_array) );
	mea->setElementType( domExtra::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 5 );
	meta->setCMRoot( cm );	
	// Ordered list of sub-elements
	meta->addContents(daeOffsetOf(domEffect,_contents));
	meta->addContentsOrder(daeOffsetOf(domEffect,_contentsOrder));


	//	Add attribute: id
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "id" );
		ma->setType( dae.getAtomicTypes().get("xsID"));
		ma->setOffset( daeOffsetOf( domEffect , attrId ));
		ma->setContainer( meta );
		ma->setIsRequired( true );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: name
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "name" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domEffect , attrName ));
		ma->setContainer( meta );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domEffect));
	meta->validate();

	return meta;
}

