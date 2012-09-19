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
#include <dom/domGles_texture_unit.h>
#include <dae/daeMetaCMPolicy.h>
#include <dae/daeMetaSequence.h>
#include <dae/daeMetaChoice.h>
#include <dae/daeMetaGroup.h>
#include <dae/daeMetaAny.h>
#include <dae/daeMetaElementAttribute.h>

daeElementRef
domGles_texture_unit::create(DAE& dae)
{
	domGles_texture_unitRef ref = new domGles_texture_unit(dae);
	return ref;
}


daeMetaElement *
domGles_texture_unit::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "gles_texture_unit" );
	meta->registerClass(domGles_texture_unit::create);

	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 0, 1 );
	mea->setName( "surface" );
	mea->setOffset( daeOffsetOf(domGles_texture_unit,elemSurface) );
	mea->setElementType( domGles_texture_unit::domSurface::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 1, 0, 1 );
	mea->setName( "sampler_state" );
	mea->setOffset( daeOffsetOf(domGles_texture_unit,elemSampler_state) );
	mea->setElementType( domGles_texture_unit::domSampler_state::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 2, 0, 1 );
	mea->setName( "texcoord" );
	mea->setOffset( daeOffsetOf(domGles_texture_unit,elemTexcoord) );
	mea->setElementType( domGles_texture_unit::domTexcoord::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 3, 0, -1 );
	mea->setName( "extra" );
	mea->setOffset( daeOffsetOf(domGles_texture_unit,elemExtra_array) );
	mea->setElementType( domExtra::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 3 );
	meta->setCMRoot( cm );	

	//	Add attribute: sid
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "sid" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_texture_unit , attrSid ));
		ma->setContainer( meta );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_texture_unit));
	meta->validate();

	return meta;
}

daeElementRef
domGles_texture_unit::domSurface::create(DAE& dae)
{
	domGles_texture_unit::domSurfaceRef ref = new domGles_texture_unit::domSurface(dae);
	return ref;
}


daeMetaElement *
domGles_texture_unit::domSurface::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "surface" );
	meta->registerClass(domGles_texture_unit::domSurface::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_texture_unit::domSurface , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_texture_unit::domSurface));
	meta->validate();

	return meta;
}

daeElementRef
domGles_texture_unit::domSampler_state::create(DAE& dae)
{
	domGles_texture_unit::domSampler_stateRef ref = new domGles_texture_unit::domSampler_state(dae);
	return ref;
}


daeMetaElement *
domGles_texture_unit::domSampler_state::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "sampler_state" );
	meta->registerClass(domGles_texture_unit::domSampler_state::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_texture_unit::domSampler_state , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_texture_unit::domSampler_state));
	meta->validate();

	return meta;
}

daeElementRef
domGles_texture_unit::domTexcoord::create(DAE& dae)
{
	domGles_texture_unit::domTexcoordRef ref = new domGles_texture_unit::domTexcoord(dae);
	return ref;
}


daeMetaElement *
domGles_texture_unit::domTexcoord::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "texcoord" );
	meta->registerClass(domGles_texture_unit::domTexcoord::create);

	meta->setIsInnerClass( true );

	//	Add attribute: semantic
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "semantic" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_texture_unit::domTexcoord , attrSemantic ));
		ma->setContainer( meta );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_texture_unit::domTexcoord));
	meta->validate();

	return meta;
}

