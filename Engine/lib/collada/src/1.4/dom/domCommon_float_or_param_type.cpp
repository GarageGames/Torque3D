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
#include <dom/domCommon_float_or_param_type.h>
#include <dae/daeMetaCMPolicy.h>
#include <dae/daeMetaSequence.h>
#include <dae/daeMetaChoice.h>
#include <dae/daeMetaGroup.h>
#include <dae/daeMetaAny.h>
#include <dae/daeMetaElementAttribute.h>

daeElementRef
domCommon_float_or_param_type::create(DAE& dae)
{
	domCommon_float_or_param_typeRef ref = new domCommon_float_or_param_type(dae);
	return ref;
}


daeMetaElement *
domCommon_float_or_param_type::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "common_float_or_param_type" );
	meta->registerClass(domCommon_float_or_param_type::create);

	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaChoice( meta, cm, 0, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "float" );
	mea->setOffset( daeOffsetOf(domCommon_float_or_param_type,elemFloat) );
	mea->setElementType( domCommon_float_or_param_type::domFloat::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "param" );
	mea->setOffset( daeOffsetOf(domCommon_float_or_param_type,elemParam) );
	mea->setElementType( domCommon_float_or_param_type::domParam::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 0 );
	meta->setCMRoot( cm );	
	// Ordered list of sub-elements
	meta->addContents(daeOffsetOf(domCommon_float_or_param_type,_contents));
	meta->addContentsOrder(daeOffsetOf(domCommon_float_or_param_type,_contentsOrder));

	meta->addCMDataArray(daeOffsetOf(domCommon_float_or_param_type,_CMData), 1);
	meta->setElementSize(sizeof(domCommon_float_or_param_type));
	meta->validate();

	return meta;
}

daeElementRef
domCommon_float_or_param_type::domFloat::create(DAE& dae)
{
	domCommon_float_or_param_type::domFloatRef ref = new domCommon_float_or_param_type::domFloat(dae);
	return ref;
}


daeMetaElement *
domCommon_float_or_param_type::domFloat::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "float" );
	meta->registerClass(domCommon_float_or_param_type::domFloat::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Float"));
		ma->setOffset( daeOffsetOf( domCommon_float_or_param_type::domFloat , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	//	Add attribute: sid
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "sid" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domCommon_float_or_param_type::domFloat , attrSid ));
		ma->setContainer( meta );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCommon_float_or_param_type::domFloat));
	meta->validate();

	return meta;
}

daeElementRef
domCommon_float_or_param_type::domParam::create(DAE& dae)
{
	domCommon_float_or_param_type::domParamRef ref = new domCommon_float_or_param_type::domParam(dae);
	return ref;
}


daeMetaElement *
domCommon_float_or_param_type::domParam::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "param" );
	meta->registerClass(domCommon_float_or_param_type::domParam::create);

	meta->setIsInnerClass( true );

	//	Add attribute: ref
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "ref" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domCommon_float_or_param_type::domParam , attrRef ));
		ma->setContainer( meta );
		ma->setIsRequired( true );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCommon_float_or_param_type::domParam));
	meta->validate();

	return meta;
}

