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
#include <dom/domTapered_capsule.h>
#include <dae/daeMetaCMPolicy.h>
#include <dae/daeMetaSequence.h>
#include <dae/daeMetaChoice.h>
#include <dae/daeMetaGroup.h>
#include <dae/daeMetaAny.h>
#include <dae/daeMetaElementAttribute.h>

daeElementRef
domTapered_capsule::create(DAE& dae)
{
	domTapered_capsuleRef ref = new domTapered_capsule(dae);
	return ref;
}


daeMetaElement *
domTapered_capsule::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "tapered_capsule" );
	meta->registerClass(domTapered_capsule::create);

	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "height" );
	mea->setOffset( daeOffsetOf(domTapered_capsule,elemHeight) );
	mea->setElementType( domTapered_capsule::domHeight::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 1, 1, 1 );
	mea->setName( "radius1" );
	mea->setOffset( daeOffsetOf(domTapered_capsule,elemRadius1) );
	mea->setElementType( domTapered_capsule::domRadius1::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 2, 1, 1 );
	mea->setName( "radius2" );
	mea->setOffset( daeOffsetOf(domTapered_capsule,elemRadius2) );
	mea->setElementType( domTapered_capsule::domRadius2::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 3, 0, -1 );
	mea->setName( "extra" );
	mea->setOffset( daeOffsetOf(domTapered_capsule,elemExtra_array) );
	mea->setElementType( domExtra::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 3 );
	meta->setCMRoot( cm );	

	meta->setElementSize(sizeof(domTapered_capsule));
	meta->validate();

	return meta;
}

daeElementRef
domTapered_capsule::domHeight::create(DAE& dae)
{
	domTapered_capsule::domHeightRef ref = new domTapered_capsule::domHeight(dae);
	return ref;
}


daeMetaElement *
domTapered_capsule::domHeight::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "height" );
	meta->registerClass(domTapered_capsule::domHeight::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Float"));
		ma->setOffset( daeOffsetOf( domTapered_capsule::domHeight , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domTapered_capsule::domHeight));
	meta->validate();

	return meta;
}

daeElementRef
domTapered_capsule::domRadius1::create(DAE& dae)
{
	domTapered_capsule::domRadius1Ref ref = new domTapered_capsule::domRadius1(dae);
	return ref;
}


daeMetaElement *
domTapered_capsule::domRadius1::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "radius1" );
	meta->registerClass(domTapered_capsule::domRadius1::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Float2"));
		ma->setOffset( daeOffsetOf( domTapered_capsule::domRadius1 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domTapered_capsule::domRadius1));
	meta->validate();

	return meta;
}

daeElementRef
domTapered_capsule::domRadius2::create(DAE& dae)
{
	domTapered_capsule::domRadius2Ref ref = new domTapered_capsule::domRadius2(dae);
	return ref;
}


daeMetaElement *
domTapered_capsule::domRadius2::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "radius2" );
	meta->registerClass(domTapered_capsule::domRadius2::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Float2"));
		ma->setOffset( daeOffsetOf( domTapered_capsule::domRadius2 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domTapered_capsule::domRadius2));
	meta->validate();

	return meta;
}

