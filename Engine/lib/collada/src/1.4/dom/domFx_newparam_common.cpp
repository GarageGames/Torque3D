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
#include <dom/domFx_newparam_common.h>
#include <dae/daeMetaCMPolicy.h>
#include <dae/daeMetaSequence.h>
#include <dae/daeMetaChoice.h>
#include <dae/daeMetaGroup.h>
#include <dae/daeMetaAny.h>
#include <dae/daeMetaElementAttribute.h>

daeElementRef
domFx_newparam_common::create(DAE& dae)
{
	domFx_newparam_commonRef ref = new domFx_newparam_common(dae);
	return ref;
}


daeMetaElement *
domFx_newparam_common::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "fx_newparam_common" );
	meta->registerClass(domFx_newparam_common::create);

	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 0, -1 );
	mea->setName( "annotate" );
	mea->setOffset( daeOffsetOf(domFx_newparam_common,elemAnnotate_array) );
	mea->setElementType( domFx_annotate_common::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 1, 0, 1 );
	mea->setName( "semantic" );
	mea->setOffset( daeOffsetOf(domFx_newparam_common,elemSemantic) );
	mea->setElementType( domFx_newparam_common::domSemantic::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 2, 0, 1 );
	mea->setName( "modifier" );
	mea->setOffset( daeOffsetOf(domFx_newparam_common,elemModifier) );
	mea->setElementType( domFx_newparam_common::domModifier::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 3, 1, 1 );
	mea->setName( "fx_basic_type_common" );
	mea->setOffset( daeOffsetOf(domFx_newparam_common,elemFx_basic_type_common) );
	mea->setElementType( domFx_basic_type_common::registerElement(dae) );
	cm->appendChild( new daeMetaGroup( mea, meta, cm, 3, 1, 1 ) );

	cm->setMaxOrdinal( 3 );
	meta->setCMRoot( cm );	

	//	Add attribute: sid
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "sid" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domFx_newparam_common , attrSid ));
		ma->setContainer( meta );
		ma->setIsRequired( true );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domFx_newparam_common));
	meta->validate();

	return meta;
}

daeElementRef
domFx_newparam_common::domSemantic::create(DAE& dae)
{
	domFx_newparam_common::domSemanticRef ref = new domFx_newparam_common::domSemantic(dae);
	return ref;
}


daeMetaElement *
domFx_newparam_common::domSemantic::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "semantic" );
	meta->registerClass(domFx_newparam_common::domSemantic::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domFx_newparam_common::domSemantic , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domFx_newparam_common::domSemantic));
	meta->validate();

	return meta;
}

daeElementRef
domFx_newparam_common::domModifier::create(DAE& dae)
{
	domFx_newparam_common::domModifierRef ref = new domFx_newparam_common::domModifier(dae);
	return ref;
}


daeMetaElement *
domFx_newparam_common::domModifier::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "modifier" );
	meta->registerClass(domFx_newparam_common::domModifier::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Fx_modifier_enum_common"));
		ma->setOffset( daeOffsetOf( domFx_newparam_common::domModifier , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domFx_newparam_common::domModifier));
	meta->validate();

	return meta;
}

