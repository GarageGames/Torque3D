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
#include <dom/domInt_array.h>
#include <dae/daeMetaCMPolicy.h>
#include <dae/daeMetaSequence.h>
#include <dae/daeMetaChoice.h>
#include <dae/daeMetaGroup.h>
#include <dae/daeMetaAny.h>
#include <dae/daeMetaElementAttribute.h>

daeElementRef
domInt_array::create(DAE& dae)
{
	domInt_arrayRef ref = new domInt_array(dae);
	return ref;
}


daeMetaElement *
domInt_array::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "int_array" );
	meta->registerClass(domInt_array::create);

	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("ListOfInts"));
		ma->setOffset( daeOffsetOf( domInt_array , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	//	Add attribute: id
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "id" );
		ma->setType( dae.getAtomicTypes().get("xsID"));
		ma->setOffset( daeOffsetOf( domInt_array , attrId ));
		ma->setContainer( meta );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: name
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "name" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domInt_array , attrName ));
		ma->setContainer( meta );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: count
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "count" );
		ma->setType( dae.getAtomicTypes().get("Uint"));
		ma->setOffset( daeOffsetOf( domInt_array , attrCount ));
		ma->setContainer( meta );
		ma->setIsRequired( true );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: minInclusive
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "minInclusive" );
		ma->setType( dae.getAtomicTypes().get("xsInteger"));
		ma->setOffset( daeOffsetOf( domInt_array , attrMinInclusive ));
		ma->setContainer( meta );
		ma->setDefaultString( "-2147483648");
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: maxInclusive
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "maxInclusive" );
		ma->setType( dae.getAtomicTypes().get("xsInteger"));
		ma->setOffset( daeOffsetOf( domInt_array , attrMaxInclusive ));
		ma->setContainer( meta );
		ma->setDefaultString( "2147483647");
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domInt_array));
	meta->validate();

	return meta;
}

