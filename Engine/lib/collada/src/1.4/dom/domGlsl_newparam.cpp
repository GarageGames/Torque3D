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
#include <dom/domGlsl_newparam.h>
#include <dae/daeMetaCMPolicy.h>
#include <dae/daeMetaSequence.h>
#include <dae/daeMetaChoice.h>
#include <dae/daeMetaGroup.h>
#include <dae/daeMetaAny.h>
#include <dae/daeMetaElementAttribute.h>

daeElementRef
domGlsl_newparam::create(DAE& dae)
{
	domGlsl_newparamRef ref = new domGlsl_newparam(dae);
	return ref;
}


daeMetaElement *
domGlsl_newparam::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "glsl_newparam" );
	meta->registerClass(domGlsl_newparam::create);

	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 0, -1 );
	mea->setName( "annotate" );
	mea->setOffset( daeOffsetOf(domGlsl_newparam,elemAnnotate_array) );
	mea->setElementType( domFx_annotate_common::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 1, 0, 1 );
	mea->setName( "semantic" );
	mea->setOffset( daeOffsetOf(domGlsl_newparam,elemSemantic) );
	mea->setElementType( domGlsl_newparam::domSemantic::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 2, 0, 1 );
	mea->setName( "modifier" );
	mea->setOffset( daeOffsetOf(domGlsl_newparam,elemModifier) );
	mea->setElementType( domGlsl_newparam::domModifier::registerElement(dae) );
	cm->appendChild( mea );

	cm = new daeMetaChoice( meta, cm, 0, 3, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "glsl_param_type" );
	mea->setOffset( daeOffsetOf(domGlsl_newparam,elemGlsl_param_type) );
	mea->setElementType( domGlsl_param_type::registerElement(dae) );
	cm->appendChild( new daeMetaGroup( mea, meta, cm, 0, 1, 1 ) );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "array" );
	mea->setOffset( daeOffsetOf(domGlsl_newparam,elemArray) );
	mea->setElementType( domGlsl_newarray_type::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 0 );
	cm->getParent()->appendChild( cm );
	cm = cm->getParent();

	cm->setMaxOrdinal( 3 );
	meta->setCMRoot( cm );	
	// Ordered list of sub-elements
	meta->addContents(daeOffsetOf(domGlsl_newparam,_contents));
	meta->addContentsOrder(daeOffsetOf(domGlsl_newparam,_contentsOrder));

	meta->addCMDataArray(daeOffsetOf(domGlsl_newparam,_CMData), 1);
	//	Add attribute: sid
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "sid" );
		ma->setType( dae.getAtomicTypes().get("Glsl_identifier"));
		ma->setOffset( daeOffsetOf( domGlsl_newparam , attrSid ));
		ma->setContainer( meta );
		ma->setIsRequired( true );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGlsl_newparam));
	meta->validate();

	return meta;
}

daeElementRef
domGlsl_newparam::domSemantic::create(DAE& dae)
{
	domGlsl_newparam::domSemanticRef ref = new domGlsl_newparam::domSemantic(dae);
	return ref;
}


daeMetaElement *
domGlsl_newparam::domSemantic::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "semantic" );
	meta->registerClass(domGlsl_newparam::domSemantic::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGlsl_newparam::domSemantic , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGlsl_newparam::domSemantic));
	meta->validate();

	return meta;
}

daeElementRef
domGlsl_newparam::domModifier::create(DAE& dae)
{
	domGlsl_newparam::domModifierRef ref = new domGlsl_newparam::domModifier(dae);
	return ref;
}


daeMetaElement *
domGlsl_newparam::domModifier::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "modifier" );
	meta->registerClass(domGlsl_newparam::domModifier::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Fx_modifier_enum_common"));
		ma->setOffset( daeOffsetOf( domGlsl_newparam::domModifier , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGlsl_newparam::domModifier));
	meta->validate();

	return meta;
}

