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
#include <dom/domFloat_array.h>
#include <dae/daeMetaCMPolicy.h>
#include <dae/daeMetaSequence.h>
#include <dae/daeMetaChoice.h>
#include <dae/daeMetaGroup.h>
#include <dae/daeMetaAny.h>
#include <dae/daeMetaElementAttribute.h>

daeElementRef
domFloat_array::create(DAE& dae)
{
	domFloat_arrayRef ref = new domFloat_array(dae);
	return ref;
}


daeMetaElement *
domFloat_array::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "float_array" );
	meta->registerClass(domFloat_array::create);

	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("ListOfFloats"));
		ma->setOffset( daeOffsetOf( domFloat_array , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	//	Add attribute: id
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "id" );
		ma->setType( dae.getAtomicTypes().get("xsID"));
		ma->setOffset( daeOffsetOf( domFloat_array , attrId ));
		ma->setContainer( meta );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: name
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "name" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domFloat_array , attrName ));
		ma->setContainer( meta );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: count
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "count" );
		ma->setType( dae.getAtomicTypes().get("Uint"));
		ma->setOffset( daeOffsetOf( domFloat_array , attrCount ));
		ma->setContainer( meta );
		ma->setIsRequired( true );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: digits
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "digits" );
		ma->setType( dae.getAtomicTypes().get("xsShort"));
		ma->setOffset( daeOffsetOf( domFloat_array , attrDigits ));
		ma->setContainer( meta );
		ma->setDefaultString( "6");
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: magnitude
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "magnitude" );
		ma->setType( dae.getAtomicTypes().get("xsShort"));
		ma->setOffset( daeOffsetOf( domFloat_array , attrMagnitude ));
		ma->setContainer( meta );
		ma->setDefaultString( "38");
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domFloat_array));
	meta->validate();

	return meta;
}

