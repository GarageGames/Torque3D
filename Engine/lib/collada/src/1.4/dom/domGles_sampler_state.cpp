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
#include <dom/domGles_sampler_state.h>
#include <dae/daeMetaCMPolicy.h>
#include <dae/daeMetaSequence.h>
#include <dae/daeMetaChoice.h>
#include <dae/daeMetaGroup.h>
#include <dae/daeMetaAny.h>
#include <dae/daeMetaElementAttribute.h>

daeElementRef
domGles_sampler_state::create(DAE& dae)
{
	domGles_sampler_stateRef ref = new domGles_sampler_state(dae);
	return ref;
}


daeMetaElement *
domGles_sampler_state::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "gles_sampler_state" );
	meta->registerClass(domGles_sampler_state::create);

	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 0, 1 );
	mea->setName( "wrap_s" );
	mea->setOffset( daeOffsetOf(domGles_sampler_state,elemWrap_s) );
	mea->setElementType( domGles_sampler_state::domWrap_s::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 1, 0, 1 );
	mea->setName( "wrap_t" );
	mea->setOffset( daeOffsetOf(domGles_sampler_state,elemWrap_t) );
	mea->setElementType( domGles_sampler_state::domWrap_t::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 2, 0, 1 );
	mea->setName( "minfilter" );
	mea->setOffset( daeOffsetOf(domGles_sampler_state,elemMinfilter) );
	mea->setElementType( domGles_sampler_state::domMinfilter::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 3, 0, 1 );
	mea->setName( "magfilter" );
	mea->setOffset( daeOffsetOf(domGles_sampler_state,elemMagfilter) );
	mea->setElementType( domGles_sampler_state::domMagfilter::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 4, 0, 1 );
	mea->setName( "mipfilter" );
	mea->setOffset( daeOffsetOf(domGles_sampler_state,elemMipfilter) );
	mea->setElementType( domGles_sampler_state::domMipfilter::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 5, 0, 1 );
	mea->setName( "mipmap_maxlevel" );
	mea->setOffset( daeOffsetOf(domGles_sampler_state,elemMipmap_maxlevel) );
	mea->setElementType( domGles_sampler_state::domMipmap_maxlevel::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 6, 0, 1 );
	mea->setName( "mipmap_bias" );
	mea->setOffset( daeOffsetOf(domGles_sampler_state,elemMipmap_bias) );
	mea->setElementType( domGles_sampler_state::domMipmap_bias::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 7, 0, -1 );
	mea->setName( "extra" );
	mea->setOffset( daeOffsetOf(domGles_sampler_state,elemExtra_array) );
	mea->setElementType( domExtra::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 7 );
	meta->setCMRoot( cm );	

	//	Add attribute: sid
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "sid" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_sampler_state , attrSid ));
		ma->setContainer( meta );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_sampler_state));
	meta->validate();

	return meta;
}

daeElementRef
domGles_sampler_state::domWrap_s::create(DAE& dae)
{
	domGles_sampler_state::domWrap_sRef ref = new domGles_sampler_state::domWrap_s(dae);
	return ref;
}


daeMetaElement *
domGles_sampler_state::domWrap_s::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "wrap_s" );
	meta->registerClass(domGles_sampler_state::domWrap_s::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Gles_sampler_wrap"));
		ma->setOffset( daeOffsetOf( domGles_sampler_state::domWrap_s , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_sampler_state::domWrap_s));
	meta->validate();

	return meta;
}

daeElementRef
domGles_sampler_state::domWrap_t::create(DAE& dae)
{
	domGles_sampler_state::domWrap_tRef ref = new domGles_sampler_state::domWrap_t(dae);
	return ref;
}


daeMetaElement *
domGles_sampler_state::domWrap_t::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "wrap_t" );
	meta->registerClass(domGles_sampler_state::domWrap_t::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Gles_sampler_wrap"));
		ma->setOffset( daeOffsetOf( domGles_sampler_state::domWrap_t , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_sampler_state::domWrap_t));
	meta->validate();

	return meta;
}

daeElementRef
domGles_sampler_state::domMinfilter::create(DAE& dae)
{
	domGles_sampler_state::domMinfilterRef ref = new domGles_sampler_state::domMinfilter(dae);
	return ref;
}


daeMetaElement *
domGles_sampler_state::domMinfilter::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "minfilter" );
	meta->registerClass(domGles_sampler_state::domMinfilter::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Fx_sampler_filter_common"));
		ma->setOffset( daeOffsetOf( domGles_sampler_state::domMinfilter , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_sampler_state::domMinfilter));
	meta->validate();

	return meta;
}

daeElementRef
domGles_sampler_state::domMagfilter::create(DAE& dae)
{
	domGles_sampler_state::domMagfilterRef ref = new domGles_sampler_state::domMagfilter(dae);
	return ref;
}


daeMetaElement *
domGles_sampler_state::domMagfilter::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "magfilter" );
	meta->registerClass(domGles_sampler_state::domMagfilter::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Fx_sampler_filter_common"));
		ma->setOffset( daeOffsetOf( domGles_sampler_state::domMagfilter , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_sampler_state::domMagfilter));
	meta->validate();

	return meta;
}

daeElementRef
domGles_sampler_state::domMipfilter::create(DAE& dae)
{
	domGles_sampler_state::domMipfilterRef ref = new domGles_sampler_state::domMipfilter(dae);
	return ref;
}


daeMetaElement *
domGles_sampler_state::domMipfilter::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "mipfilter" );
	meta->registerClass(domGles_sampler_state::domMipfilter::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Fx_sampler_filter_common"));
		ma->setOffset( daeOffsetOf( domGles_sampler_state::domMipfilter , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_sampler_state::domMipfilter));
	meta->validate();

	return meta;
}

daeElementRef
domGles_sampler_state::domMipmap_maxlevel::create(DAE& dae)
{
	domGles_sampler_state::domMipmap_maxlevelRef ref = new domGles_sampler_state::domMipmap_maxlevel(dae);
	return ref;
}


daeMetaElement *
domGles_sampler_state::domMipmap_maxlevel::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "mipmap_maxlevel" );
	meta->registerClass(domGles_sampler_state::domMipmap_maxlevel::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("xsUnsignedByte"));
		ma->setOffset( daeOffsetOf( domGles_sampler_state::domMipmap_maxlevel , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_sampler_state::domMipmap_maxlevel));
	meta->validate();

	return meta;
}

daeElementRef
domGles_sampler_state::domMipmap_bias::create(DAE& dae)
{
	domGles_sampler_state::domMipmap_biasRef ref = new domGles_sampler_state::domMipmap_bias(dae);
	return ref;
}


daeMetaElement *
domGles_sampler_state::domMipmap_bias::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "mipmap_bias" );
	meta->registerClass(domGles_sampler_state::domMipmap_bias::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("xsFloat"));
		ma->setOffset( daeOffsetOf( domGles_sampler_state::domMipmap_bias , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_sampler_state::domMipmap_bias));
	meta->validate();

	return meta;
}

