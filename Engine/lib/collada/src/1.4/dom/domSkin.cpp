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
#include <dom/domSkin.h>
#include <dae/daeMetaCMPolicy.h>
#include <dae/daeMetaSequence.h>
#include <dae/daeMetaChoice.h>
#include <dae/daeMetaGroup.h>
#include <dae/daeMetaAny.h>
#include <dae/daeMetaElementAttribute.h>

daeElementRef
domSkin::create(DAE& dae)
{
	domSkinRef ref = new domSkin(dae);
	return ref;
}


daeMetaElement *
domSkin::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "skin" );
	meta->registerClass(domSkin::create);

	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 0, 1 );
	mea->setName( "bind_shape_matrix" );
	mea->setOffset( daeOffsetOf(domSkin,elemBind_shape_matrix) );
	mea->setElementType( domSkin::domBind_shape_matrix::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 1, 3, -1 );
	mea->setName( "source" );
	mea->setOffset( daeOffsetOf(domSkin,elemSource_array) );
	mea->setElementType( domSource::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 2, 1, 1 );
	mea->setName( "joints" );
	mea->setOffset( daeOffsetOf(domSkin,elemJoints) );
	mea->setElementType( domSkin::domJoints::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 3, 1, 1 );
	mea->setName( "vertex_weights" );
	mea->setOffset( daeOffsetOf(domSkin,elemVertex_weights) );
	mea->setElementType( domSkin::domVertex_weights::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 4, 0, -1 );
	mea->setName( "extra" );
	mea->setOffset( daeOffsetOf(domSkin,elemExtra_array) );
	mea->setElementType( domExtra::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 4 );
	meta->setCMRoot( cm );	

	//	Add attribute: source
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "source" );
		ma->setType( dae.getAtomicTypes().get("xsAnyURI"));
		ma->setOffset( daeOffsetOf( domSkin , attrSource ));
		ma->setContainer( meta );
		ma->setIsRequired( true );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domSkin));
	meta->validate();

	return meta;
}

daeElementRef
domSkin::domBind_shape_matrix::create(DAE& dae)
{
	domSkin::domBind_shape_matrixRef ref = new domSkin::domBind_shape_matrix(dae);
	return ref;
}


daeMetaElement *
domSkin::domBind_shape_matrix::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "bind_shape_matrix" );
	meta->registerClass(domSkin::domBind_shape_matrix::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Float4x4"));
		ma->setOffset( daeOffsetOf( domSkin::domBind_shape_matrix , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domSkin::domBind_shape_matrix));
	meta->validate();

	return meta;
}

daeElementRef
domSkin::domJoints::create(DAE& dae)
{
	domSkin::domJointsRef ref = new domSkin::domJoints(dae);
	return ref;
}


daeMetaElement *
domSkin::domJoints::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "joints" );
	meta->registerClass(domSkin::domJoints::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 2, -1 );
	mea->setName( "input" );
	mea->setOffset( daeOffsetOf(domSkin::domJoints,elemInput_array) );
	mea->setElementType( domInputLocal::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 1, 0, -1 );
	mea->setName( "extra" );
	mea->setOffset( daeOffsetOf(domSkin::domJoints,elemExtra_array) );
	mea->setElementType( domExtra::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 1 );
	meta->setCMRoot( cm );	

	meta->setElementSize(sizeof(domSkin::domJoints));
	meta->validate();

	return meta;
}

daeElementRef
domSkin::domVertex_weights::create(DAE& dae)
{
	domSkin::domVertex_weightsRef ref = new domSkin::domVertex_weights(dae);
	return ref;
}


daeMetaElement *
domSkin::domVertex_weights::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "vertex_weights" );
	meta->registerClass(domSkin::domVertex_weights::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 2, -1 );
	mea->setName( "input" );
	mea->setOffset( daeOffsetOf(domSkin::domVertex_weights,elemInput_array) );
	mea->setElementType( domInputLocalOffset::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 1, 0, 1 );
	mea->setName( "vcount" );
	mea->setOffset( daeOffsetOf(domSkin::domVertex_weights,elemVcount) );
	mea->setElementType( domSkin::domVertex_weights::domVcount::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 2, 0, 1 );
	mea->setName( "v" );
	mea->setOffset( daeOffsetOf(domSkin::domVertex_weights,elemV) );
	mea->setElementType( domSkin::domVertex_weights::domV::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 3, 0, -1 );
	mea->setName( "extra" );
	mea->setOffset( daeOffsetOf(domSkin::domVertex_weights,elemExtra_array) );
	mea->setElementType( domExtra::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 3 );
	meta->setCMRoot( cm );	

	//	Add attribute: count
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "count" );
		ma->setType( dae.getAtomicTypes().get("Uint"));
		ma->setOffset( daeOffsetOf( domSkin::domVertex_weights , attrCount ));
		ma->setContainer( meta );
		ma->setIsRequired( true );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domSkin::domVertex_weights));
	meta->validate();

	return meta;
}

daeElementRef
domSkin::domVertex_weights::domVcount::create(DAE& dae)
{
	domSkin::domVertex_weights::domVcountRef ref = new domSkin::domVertex_weights::domVcount(dae);
	return ref;
}


daeMetaElement *
domSkin::domVertex_weights::domVcount::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "vcount" );
	meta->registerClass(domSkin::domVertex_weights::domVcount::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("ListOfUInts"));
		ma->setOffset( daeOffsetOf( domSkin::domVertex_weights::domVcount , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domSkin::domVertex_weights::domVcount));
	meta->validate();

	return meta;
}

daeElementRef
domSkin::domVertex_weights::domV::create(DAE& dae)
{
	domSkin::domVertex_weights::domVRef ref = new domSkin::domVertex_weights::domV(dae);
	return ref;
}


daeMetaElement *
domSkin::domVertex_weights::domV::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "v" );
	meta->registerClass(domSkin::domVertex_weights::domV::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("ListOfInts"));
		ma->setOffset( daeOffsetOf( domSkin::domVertex_weights::domV , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domSkin::domVertex_weights::domV));
	meta->validate();

	return meta;
}

