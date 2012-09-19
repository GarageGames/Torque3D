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
#include <dom/domGles_texcombiner_argumentAlpha_type.h>
#include <dae/daeMetaCMPolicy.h>
#include <dae/daeMetaSequence.h>
#include <dae/daeMetaChoice.h>
#include <dae/daeMetaGroup.h>
#include <dae/daeMetaAny.h>
#include <dae/daeMetaElementAttribute.h>

daeElementRef
domGles_texcombiner_argumentAlpha_type::create(DAE& dae)
{
	domGles_texcombiner_argumentAlpha_typeRef ref = new domGles_texcombiner_argumentAlpha_type(dae);
	return ref;
}


daeMetaElement *
domGles_texcombiner_argumentAlpha_type::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "gles_texcombiner_argumentAlpha_type" );
	meta->registerClass(domGles_texcombiner_argumentAlpha_type::create);


	//	Add attribute: source
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "source" );
		ma->setType( dae.getAtomicTypes().get("Gles_texcombiner_source_enums"));
		ma->setOffset( daeOffsetOf( domGles_texcombiner_argumentAlpha_type , attrSource ));
		ma->setContainer( meta );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: operand
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "operand" );
		ma->setType( dae.getAtomicTypes().get("Gles_texcombiner_operandAlpha_enums"));
		ma->setOffset( daeOffsetOf( domGles_texcombiner_argumentAlpha_type , attrOperand ));
		ma->setContainer( meta );
		ma->setDefaultString( "SRC_ALPHA");
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: unit
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "unit" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domGles_texcombiner_argumentAlpha_type , attrUnit ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGles_texcombiner_argumentAlpha_type));
	meta->validate();

	return meta;
}

