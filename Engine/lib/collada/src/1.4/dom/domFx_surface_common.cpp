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
#include <dom/domFx_surface_common.h>
#include <dae/daeMetaCMPolicy.h>
#include <dae/daeMetaSequence.h>
#include <dae/daeMetaChoice.h>
#include <dae/daeMetaGroup.h>
#include <dae/daeMetaAny.h>
#include <dae/daeMetaElementAttribute.h>

daeElementRef
domFx_surface_common::create(DAE& dae)
{
	domFx_surface_commonRef ref = new domFx_surface_common(dae);
	return ref;
}


daeMetaElement *
domFx_surface_common::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "fx_surface_common" );
	meta->registerClass(domFx_surface_common::create);

	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 0, 1 );
	mea->setName( "fx_surface_init_common" );
	mea->setOffset( daeOffsetOf(domFx_surface_common,elemFx_surface_init_common) );
	mea->setElementType( domFx_surface_init_common::registerElement(dae) );
	cm->appendChild( new daeMetaGroup( mea, meta, cm, 0, 0, 1 ) );

	mea = new daeMetaElementAttribute( meta, cm, 1, 0, 1 );
	mea->setName( "format" );
	mea->setOffset( daeOffsetOf(domFx_surface_common,elemFormat) );
	mea->setElementType( domFx_surface_common::domFormat::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 2, 0, 1 );
	mea->setName( "format_hint" );
	mea->setOffset( daeOffsetOf(domFx_surface_common,elemFormat_hint) );
	mea->setElementType( domFx_surface_format_hint_common::registerElement(dae) );
	cm->appendChild( mea );

	cm = new daeMetaChoice( meta, cm, 0, 3, 0, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "size" );
	mea->setOffset( daeOffsetOf(domFx_surface_common,elemSize) );
	mea->setElementType( domFx_surface_common::domSize::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "viewport_ratio" );
	mea->setOffset( daeOffsetOf(domFx_surface_common,elemViewport_ratio) );
	mea->setElementType( domFx_surface_common::domViewport_ratio::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 0 );
	cm->getParent()->appendChild( cm );
	cm = cm->getParent();

	mea = new daeMetaElementAttribute( meta, cm, 4, 0, 1 );
	mea->setName( "mip_levels" );
	mea->setOffset( daeOffsetOf(domFx_surface_common,elemMip_levels) );
	mea->setElementType( domFx_surface_common::domMip_levels::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 5, 0, 1 );
	mea->setName( "mipmap_generate" );
	mea->setOffset( daeOffsetOf(domFx_surface_common,elemMipmap_generate) );
	mea->setElementType( domFx_surface_common::domMipmap_generate::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 6, 0, -1 );
	mea->setName( "extra" );
	mea->setOffset( daeOffsetOf(domFx_surface_common,elemExtra_array) );
	mea->setElementType( domExtra::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 6 );
	meta->setCMRoot( cm );	
	// Ordered list of sub-elements
	meta->addContents(daeOffsetOf(domFx_surface_common,_contents));
	meta->addContentsOrder(daeOffsetOf(domFx_surface_common,_contentsOrder));

	meta->addCMDataArray(daeOffsetOf(domFx_surface_common,_CMData), 1);
	//	Add attribute: type
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "type" );
		ma->setType( dae.getAtomicTypes().get("Fx_surface_type_enum"));
		ma->setOffset( daeOffsetOf( domFx_surface_common , attrType ));
		ma->setContainer( meta );
		ma->setIsRequired( true );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domFx_surface_common));
	meta->validate();

	return meta;
}

daeElementRef
domFx_surface_common::domFormat::create(DAE& dae)
{
	domFx_surface_common::domFormatRef ref = new domFx_surface_common::domFormat(dae);
	return ref;
}


daeMetaElement *
domFx_surface_common::domFormat::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "format" );
	meta->registerClass(domFx_surface_common::domFormat::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("xsToken"));
		ma->setOffset( daeOffsetOf( domFx_surface_common::domFormat , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domFx_surface_common::domFormat));
	meta->validate();

	return meta;
}

daeElementRef
domFx_surface_common::domSize::create(DAE& dae)
{
	domFx_surface_common::domSizeRef ref = new domFx_surface_common::domSize(dae);
	return ref;
}


daeMetaElement *
domFx_surface_common::domSize::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "size" );
	meta->registerClass(domFx_surface_common::domSize::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Int3"));
		ma->setOffset( daeOffsetOf( domFx_surface_common::domSize , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domFx_surface_common::domSize));
	meta->validate();

	return meta;
}

daeElementRef
domFx_surface_common::domViewport_ratio::create(DAE& dae)
{
	domFx_surface_common::domViewport_ratioRef ref = new domFx_surface_common::domViewport_ratio(dae);
	return ref;
}


daeMetaElement *
domFx_surface_common::domViewport_ratio::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "viewport_ratio" );
	meta->registerClass(domFx_surface_common::domViewport_ratio::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Float2"));
		ma->setOffset( daeOffsetOf( domFx_surface_common::domViewport_ratio , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domFx_surface_common::domViewport_ratio));
	meta->validate();

	return meta;
}

daeElementRef
domFx_surface_common::domMip_levels::create(DAE& dae)
{
	domFx_surface_common::domMip_levelsRef ref = new domFx_surface_common::domMip_levels(dae);
	return ref;
}


daeMetaElement *
domFx_surface_common::domMip_levels::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "mip_levels" );
	meta->registerClass(domFx_surface_common::domMip_levels::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("xsUnsignedInt"));
		ma->setOffset( daeOffsetOf( domFx_surface_common::domMip_levels , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domFx_surface_common::domMip_levels));
	meta->validate();

	return meta;
}

daeElementRef
domFx_surface_common::domMipmap_generate::create(DAE& dae)
{
	domFx_surface_common::domMipmap_generateRef ref = new domFx_surface_common::domMipmap_generate(dae);
	return ref;
}


daeMetaElement *
domFx_surface_common::domMipmap_generate::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "mipmap_generate" );
	meta->registerClass(domFx_surface_common::domMipmap_generate::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("xsBoolean"));
		ma->setOffset( daeOffsetOf( domFx_surface_common::domMipmap_generate , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domFx_surface_common::domMipmap_generate));
	meta->validate();

	return meta;
}

