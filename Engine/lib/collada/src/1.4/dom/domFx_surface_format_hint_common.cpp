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
#include <dom/domFx_surface_format_hint_common.h>
#include <dae/daeMetaCMPolicy.h>
#include <dae/daeMetaSequence.h>
#include <dae/daeMetaChoice.h>
#include <dae/daeMetaGroup.h>
#include <dae/daeMetaAny.h>
#include <dae/daeMetaElementAttribute.h>

daeElementRef
domFx_surface_format_hint_common::create(DAE& dae)
{
	domFx_surface_format_hint_commonRef ref = new domFx_surface_format_hint_common(dae);
	return ref;
}


daeMetaElement *
domFx_surface_format_hint_common::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "fx_surface_format_hint_common" );
	meta->registerClass(domFx_surface_format_hint_common::create);

	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "channels" );
	mea->setOffset( daeOffsetOf(domFx_surface_format_hint_common,elemChannels) );
	mea->setElementType( domFx_surface_format_hint_common::domChannels::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 1, 1, 1 );
	mea->setName( "range" );
	mea->setOffset( daeOffsetOf(domFx_surface_format_hint_common,elemRange) );
	mea->setElementType( domFx_surface_format_hint_common::domRange::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 2, 0, 1 );
	mea->setName( "precision" );
	mea->setOffset( daeOffsetOf(domFx_surface_format_hint_common,elemPrecision) );
	mea->setElementType( domFx_surface_format_hint_common::domPrecision::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 3, 0, -1 );
	mea->setName( "option" );
	mea->setOffset( daeOffsetOf(domFx_surface_format_hint_common,elemOption_array) );
	mea->setElementType( domFx_surface_format_hint_common::domOption::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 4, 0, -1 );
	mea->setName( "extra" );
	mea->setOffset( daeOffsetOf(domFx_surface_format_hint_common,elemExtra_array) );
	mea->setElementType( domExtra::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 4 );
	meta->setCMRoot( cm );	

	meta->setElementSize(sizeof(domFx_surface_format_hint_common));
	meta->validate();

	return meta;
}

daeElementRef
domFx_surface_format_hint_common::domChannels::create(DAE& dae)
{
	domFx_surface_format_hint_common::domChannelsRef ref = new domFx_surface_format_hint_common::domChannels(dae);
	return ref;
}


daeMetaElement *
domFx_surface_format_hint_common::domChannels::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "channels" );
	meta->registerClass(domFx_surface_format_hint_common::domChannels::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Fx_surface_format_hint_channels_enum"));
		ma->setOffset( daeOffsetOf( domFx_surface_format_hint_common::domChannels , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domFx_surface_format_hint_common::domChannels));
	meta->validate();

	return meta;
}

daeElementRef
domFx_surface_format_hint_common::domRange::create(DAE& dae)
{
	domFx_surface_format_hint_common::domRangeRef ref = new domFx_surface_format_hint_common::domRange(dae);
	return ref;
}


daeMetaElement *
domFx_surface_format_hint_common::domRange::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "range" );
	meta->registerClass(domFx_surface_format_hint_common::domRange::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Fx_surface_format_hint_range_enum"));
		ma->setOffset( daeOffsetOf( domFx_surface_format_hint_common::domRange , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domFx_surface_format_hint_common::domRange));
	meta->validate();

	return meta;
}

daeElementRef
domFx_surface_format_hint_common::domPrecision::create(DAE& dae)
{
	domFx_surface_format_hint_common::domPrecisionRef ref = new domFx_surface_format_hint_common::domPrecision(dae);
	return ref;
}


daeMetaElement *
domFx_surface_format_hint_common::domPrecision::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "precision" );
	meta->registerClass(domFx_surface_format_hint_common::domPrecision::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Fx_surface_format_hint_precision_enum"));
		ma->setOffset( daeOffsetOf( domFx_surface_format_hint_common::domPrecision , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domFx_surface_format_hint_common::domPrecision));
	meta->validate();

	return meta;
}

daeElementRef
domFx_surface_format_hint_common::domOption::create(DAE& dae)
{
	domFx_surface_format_hint_common::domOptionRef ref = new domFx_surface_format_hint_common::domOption(dae);
	return ref;
}


daeMetaElement *
domFx_surface_format_hint_common::domOption::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "option" );
	meta->registerClass(domFx_surface_format_hint_common::domOption::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Fx_surface_format_hint_option_enum"));
		ma->setOffset( daeOffsetOf( domFx_surface_format_hint_common::domOption , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domFx_surface_format_hint_common::domOption));
	meta->validate();

	return meta;
}

