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
#include <dom/domCommon_newparam_type.h>
#include <dae/daeMetaCMPolicy.h>
#include <dae/daeMetaSequence.h>
#include <dae/daeMetaChoice.h>
#include <dae/daeMetaGroup.h>
#include <dae/daeMetaAny.h>
#include <dae/daeMetaElementAttribute.h>

daeElementRef
domCommon_newparam_type::create(DAE& dae)
{
	domCommon_newparam_typeRef ref = new domCommon_newparam_type(dae);
	return ref;
}


daeMetaElement *
domCommon_newparam_type::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "common_newparam_type" );
	meta->registerClass(domCommon_newparam_type::create);

	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 0, 1 );
	mea->setName( "semantic" );
	mea->setOffset( daeOffsetOf(domCommon_newparam_type,elemSemantic) );
	mea->setElementType( domCommon_newparam_type::domSemantic::registerElement(dae) );
	cm->appendChild( mea );

	cm = new daeMetaChoice( meta, cm, 0, 1, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "float" );
	mea->setOffset( daeOffsetOf(domCommon_newparam_type,elemFloat) );
	mea->setElementType( domCommon_newparam_type::domFloat::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "float2" );
	mea->setOffset( daeOffsetOf(domCommon_newparam_type,elemFloat2) );
	mea->setElementType( domCommon_newparam_type::domFloat2::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "float3" );
	mea->setOffset( daeOffsetOf(domCommon_newparam_type,elemFloat3) );
	mea->setElementType( domCommon_newparam_type::domFloat3::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "float4" );
	mea->setOffset( daeOffsetOf(domCommon_newparam_type,elemFloat4) );
	mea->setElementType( domCommon_newparam_type::domFloat4::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "surface" );
	mea->setOffset( daeOffsetOf(domCommon_newparam_type,elemSurface) );
	mea->setElementType( domFx_surface_common::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "sampler2D" );
	mea->setOffset( daeOffsetOf(domCommon_newparam_type,elemSampler2D) );
	mea->setElementType( domFx_sampler2D_common::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 0 );
	cm->getParent()->appendChild( cm );
	cm = cm->getParent();

	cm->setMaxOrdinal( 1 );
	meta->setCMRoot( cm );	
	// Ordered list of sub-elements
	meta->addContents(daeOffsetOf(domCommon_newparam_type,_contents));
	meta->addContentsOrder(daeOffsetOf(domCommon_newparam_type,_contentsOrder));

	meta->addCMDataArray(daeOffsetOf(domCommon_newparam_type,_CMData), 1);
	//	Add attribute: sid
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "sid" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domCommon_newparam_type , attrSid ));
		ma->setContainer( meta );
		ma->setIsRequired( true );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCommon_newparam_type));
	meta->validate();

	return meta;
}

daeElementRef
domCommon_newparam_type::domSemantic::create(DAE& dae)
{
	domCommon_newparam_type::domSemanticRef ref = new domCommon_newparam_type::domSemantic(dae);
	return ref;
}


daeMetaElement *
domCommon_newparam_type::domSemantic::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "semantic" );
	meta->registerClass(domCommon_newparam_type::domSemantic::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domCommon_newparam_type::domSemantic , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCommon_newparam_type::domSemantic));
	meta->validate();

	return meta;
}

daeElementRef
domCommon_newparam_type::domFloat::create(DAE& dae)
{
	domCommon_newparam_type::domFloatRef ref = new domCommon_newparam_type::domFloat(dae);
	return ref;
}


daeMetaElement *
domCommon_newparam_type::domFloat::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "float" );
	meta->registerClass(domCommon_newparam_type::domFloat::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Float"));
		ma->setOffset( daeOffsetOf( domCommon_newparam_type::domFloat , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCommon_newparam_type::domFloat));
	meta->validate();

	return meta;
}

daeElementRef
domCommon_newparam_type::domFloat2::create(DAE& dae)
{
	domCommon_newparam_type::domFloat2Ref ref = new domCommon_newparam_type::domFloat2(dae);
	return ref;
}


daeMetaElement *
domCommon_newparam_type::domFloat2::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "float2" );
	meta->registerClass(domCommon_newparam_type::domFloat2::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Float2"));
		ma->setOffset( daeOffsetOf( domCommon_newparam_type::domFloat2 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCommon_newparam_type::domFloat2));
	meta->validate();

	return meta;
}

daeElementRef
domCommon_newparam_type::domFloat3::create(DAE& dae)
{
	domCommon_newparam_type::domFloat3Ref ref = new domCommon_newparam_type::domFloat3(dae);
	return ref;
}


daeMetaElement *
domCommon_newparam_type::domFloat3::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "float3" );
	meta->registerClass(domCommon_newparam_type::domFloat3::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Float3"));
		ma->setOffset( daeOffsetOf( domCommon_newparam_type::domFloat3 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCommon_newparam_type::domFloat3));
	meta->validate();

	return meta;
}

daeElementRef
domCommon_newparam_type::domFloat4::create(DAE& dae)
{
	domCommon_newparam_type::domFloat4Ref ref = new domCommon_newparam_type::domFloat4(dae);
	return ref;
}


daeMetaElement *
domCommon_newparam_type::domFloat4::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "float4" );
	meta->registerClass(domCommon_newparam_type::domFloat4::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Float4"));
		ma->setOffset( daeOffsetOf( domCommon_newparam_type::domFloat4 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCommon_newparam_type::domFloat4));
	meta->validate();

	return meta;
}

