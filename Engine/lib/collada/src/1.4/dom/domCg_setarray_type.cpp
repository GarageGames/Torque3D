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
#include <dom/domCg_setarray_type.h>
#include <dae/daeMetaCMPolicy.h>
#include <dae/daeMetaSequence.h>
#include <dae/daeMetaChoice.h>
#include <dae/daeMetaGroup.h>
#include <dae/daeMetaAny.h>
#include <dae/daeMetaElementAttribute.h>

daeElementRef
domCg_setarray_type::create(DAE& dae)
{
	domCg_setarray_typeRef ref = new domCg_setarray_type(dae);
	return ref;
}


daeMetaElement *
domCg_setarray_type::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "cg_setarray_type" );
	meta->registerClass(domCg_setarray_type::create);

	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaChoice( meta, cm, 0, 0, 0, -1 );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "cg_param_type" );
	mea->setOffset( daeOffsetOf(domCg_setarray_type,elemCg_param_type_array) );
	mea->setElementType( domCg_param_type::registerElement(dae) );
	cm->appendChild( new daeMetaGroup( mea, meta, cm, 0, 1, 1 ) );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "array" );
	mea->setOffset( daeOffsetOf(domCg_setarray_type,elemArray_array) );
	mea->setElementType( domCg_setarray_type::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "usertype" );
	mea->setOffset( daeOffsetOf(domCg_setarray_type,elemUsertype_array) );
	mea->setElementType( domCg_setuser_type::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 3000 );
	meta->setCMRoot( cm );	
	// Ordered list of sub-elements
	meta->addContents(daeOffsetOf(domCg_setarray_type,_contents));
	meta->addContentsOrder(daeOffsetOf(domCg_setarray_type,_contentsOrder));

	meta->addCMDataArray(daeOffsetOf(domCg_setarray_type,_CMData), 1);
	//	Add attribute: length
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "length" );
		ma->setType( dae.getAtomicTypes().get("xsPositiveInteger"));
		ma->setOffset( daeOffsetOf( domCg_setarray_type , attrLength ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_setarray_type));
	meta->validate();

	return meta;
}

