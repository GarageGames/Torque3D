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
#include <dom/domCommon_transparent_type.h>
#include <dae/daeMetaCMPolicy.h>
#include <dae/daeMetaSequence.h>
#include <dae/daeMetaChoice.h>
#include <dae/daeMetaGroup.h>
#include <dae/daeMetaAny.h>
#include <dae/daeMetaElementAttribute.h>

daeElementRef
domCommon_transparent_type::create(DAE& dae)
{
	domCommon_transparent_typeRef ref = new domCommon_transparent_type(dae);
	return ref;
}


daeMetaElement *
domCommon_transparent_type::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "common_transparent_type" );
	meta->registerClass(domCommon_transparent_type::create);

	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	cm = new daeMetaChoice( meta, cm, 0, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "color" );
	mea->setOffset( daeOffsetOf(domCommon_transparent_type,elemColor) );
	mea->setElementType( domColor::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "param" );
	mea->setOffset( daeOffsetOf(domCommon_transparent_type,elemParam) );
	mea->setElementType( domParam::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "texture" );
	mea->setOffset( daeOffsetOf(domCommon_transparent_type,elemTexture) );
	mea->setElementType( domTexture::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 0 );
	cm->getParent()->appendChild( cm );
	cm = cm->getParent();

	cm->setMaxOrdinal( 0 );
	meta->setCMRoot( cm );	
	// Ordered list of sub-elements
	meta->addContents(daeOffsetOf(domCommon_transparent_type,_contents));
	meta->addContentsOrder(daeOffsetOf(domCommon_transparent_type,_contentsOrder));

	meta->addCMDataArray(daeOffsetOf(domCommon_transparent_type,_CMData), 1);
	//	Add attribute: opaque
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "opaque" );
		ma->setType( dae.getAtomicTypes().get("Fx_opaque_enum"));
		ma->setOffset( daeOffsetOf( domCommon_transparent_type , attrOpaque ));
		ma->setContainer( meta );
		ma->setDefaultString( "A_ONE");
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCommon_transparent_type));
	meta->validate();

	return meta;
}

