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
#include <dom/domGl_samplerCUBE.h>
#include <dae/daeMetaCMPolicy.h>
#include <dae/daeMetaSequence.h>
#include <dae/daeMetaChoice.h>
#include <dae/daeMetaGroup.h>
#include <dae/daeMetaAny.h>
#include <dae/daeMetaElementAttribute.h>

daeElementRef
domGl_samplerCUBE::create(DAE& dae)
{
	domGl_samplerCUBERef ref = new domGl_samplerCUBE(dae);
	return ref;
}


daeMetaElement *
domGl_samplerCUBE::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "gl_samplerCUBE" );
	meta->registerClass(domGl_samplerCUBE::create);

	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "source" );
	mea->setOffset( daeOffsetOf(domGl_samplerCUBE,elemSource) );
	mea->setElementType( domSource::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 1, 0, 1 );
	mea->setName( "wrap_s" );
	mea->setOffset( daeOffsetOf(domGl_samplerCUBE,elemWrap_s) );
	mea->setElementType( domWrap_s::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 2, 0, 1 );
	mea->setName( "wrap_t" );
	mea->setOffset( daeOffsetOf(domGl_samplerCUBE,elemWrap_t) );
	mea->setElementType( domWrap_t::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 3, 0, 1 );
	mea->setName( "wrap_p" );
	mea->setOffset( daeOffsetOf(domGl_samplerCUBE,elemWrap_p) );
	mea->setElementType( domWrap_p::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 4, 0, 1 );
	mea->setName( "minfilter" );
	mea->setOffset( daeOffsetOf(domGl_samplerCUBE,elemMinfilter) );
	mea->setElementType( domMinfilter::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 5, 0, 1 );
	mea->setName( "magfilter" );
	mea->setOffset( daeOffsetOf(domGl_samplerCUBE,elemMagfilter) );
	mea->setElementType( domMagfilter::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 6, 0, 1 );
	mea->setName( "mipfilter" );
	mea->setOffset( daeOffsetOf(domGl_samplerCUBE,elemMipfilter) );
	mea->setElementType( domMipfilter::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 7, 0, 1 );
	mea->setName( "border_color" );
	mea->setOffset( daeOffsetOf(domGl_samplerCUBE,elemBorder_color) );
	mea->setElementType( domBorder_color::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 8, 0, 1 );
	mea->setName( "mipmap_maxlevel" );
	mea->setOffset( daeOffsetOf(domGl_samplerCUBE,elemMipmap_maxlevel) );
	mea->setElementType( domMipmap_maxlevel::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 9, 0, 1 );
	mea->setName( "mipmap_bias" );
	mea->setOffset( daeOffsetOf(domGl_samplerCUBE,elemMipmap_bias) );
	mea->setElementType( domMipmap_bias::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 10, 0, -1 );
	mea->setName( "extra" );
	mea->setOffset( daeOffsetOf(domGl_samplerCUBE,elemExtra_array) );
	mea->setElementType( domExtra::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 10 );
	cm->getParent()->appendChild( cm );
	cm = cm->getParent();

	cm->setMaxOrdinal( 10 );
	meta->setCMRoot( cm );	

	meta->setElementSize(sizeof(domGl_samplerCUBE));
	meta->validate();

	return meta;
}

