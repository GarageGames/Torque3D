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
#include <dom/domCommon_color_or_texture_type.h>
#include <dae/daeMetaCMPolicy.h>
#include <dae/daeMetaSequence.h>
#include <dae/daeMetaChoice.h>
#include <dae/daeMetaGroup.h>
#include <dae/daeMetaAny.h>
#include <dae/daeMetaElementAttribute.h>

daeElementRef
domCommon_color_or_texture_type::create(DAE& dae)
{
	domCommon_color_or_texture_typeRef ref = new domCommon_color_or_texture_type(dae);
	return ref;
}


daeMetaElement *
domCommon_color_or_texture_type::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "common_color_or_texture_type" );
	meta->registerClass(domCommon_color_or_texture_type::create);

	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaChoice( meta, cm, 0, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "color" );
	mea->setOffset( daeOffsetOf(domCommon_color_or_texture_type,elemColor) );
	mea->setElementType( domCommon_color_or_texture_type::domColor::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "param" );
	mea->setOffset( daeOffsetOf(domCommon_color_or_texture_type,elemParam) );
	mea->setElementType( domCommon_color_or_texture_type::domParam::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "texture" );
	mea->setOffset( daeOffsetOf(domCommon_color_or_texture_type,elemTexture) );
	mea->setElementType( domCommon_color_or_texture_type::domTexture::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 0 );
	meta->setCMRoot( cm );	
	// Ordered list of sub-elements
	meta->addContents(daeOffsetOf(domCommon_color_or_texture_type,_contents));
	meta->addContentsOrder(daeOffsetOf(domCommon_color_or_texture_type,_contentsOrder));

	meta->addCMDataArray(daeOffsetOf(domCommon_color_or_texture_type,_CMData), 1);
	meta->setElementSize(sizeof(domCommon_color_or_texture_type));
	meta->validate();

	return meta;
}

daeElementRef
domCommon_color_or_texture_type::domColor::create(DAE& dae)
{
	domCommon_color_or_texture_type::domColorRef ref = new domCommon_color_or_texture_type::domColor(dae);
	return ref;
}


daeMetaElement *
domCommon_color_or_texture_type::domColor::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "color" );
	meta->registerClass(domCommon_color_or_texture_type::domColor::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Fx_color_common"));
		ma->setOffset( daeOffsetOf( domCommon_color_or_texture_type::domColor , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	//	Add attribute: sid
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "sid" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domCommon_color_or_texture_type::domColor , attrSid ));
		ma->setContainer( meta );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCommon_color_or_texture_type::domColor));
	meta->validate();

	return meta;
}

daeElementRef
domCommon_color_or_texture_type::domParam::create(DAE& dae)
{
	domCommon_color_or_texture_type::domParamRef ref = new domCommon_color_or_texture_type::domParam(dae);
	return ref;
}


daeMetaElement *
domCommon_color_or_texture_type::domParam::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "param" );
	meta->registerClass(domCommon_color_or_texture_type::domParam::create);

	meta->setIsInnerClass( true );

	//	Add attribute: ref
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "ref" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domCommon_color_or_texture_type::domParam , attrRef ));
		ma->setContainer( meta );
		ma->setIsRequired( true );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCommon_color_or_texture_type::domParam));
	meta->validate();

	return meta;
}

daeElementRef
domCommon_color_or_texture_type::domTexture::create(DAE& dae)
{
	domCommon_color_or_texture_type::domTextureRef ref = new domCommon_color_or_texture_type::domTexture(dae);
	return ref;
}


daeMetaElement *
domCommon_color_or_texture_type::domTexture::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "texture" );
	meta->registerClass(domCommon_color_or_texture_type::domTexture::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 0, 1 );
	mea->setName( "extra" );
	mea->setOffset( daeOffsetOf(domCommon_color_or_texture_type::domTexture,elemExtra) );
	mea->setElementType( domExtra::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 0 );
	meta->setCMRoot( cm );	

	//	Add attribute: texture
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "texture" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domCommon_color_or_texture_type::domTexture , attrTexture ));
		ma->setContainer( meta );
		ma->setIsRequired( true );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: texcoord
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "texcoord" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domCommon_color_or_texture_type::domTexture , attrTexcoord ));
		ma->setContainer( meta );
		ma->setIsRequired( true );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCommon_color_or_texture_type::domTexture));
	meta->validate();

	return meta;
}

