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
#include <dom/domGlsl_param_type.h>
#include <dae/daeMetaCMPolicy.h>
#include <dae/daeMetaSequence.h>
#include <dae/daeMetaChoice.h>
#include <dae/daeMetaGroup.h>
#include <dae/daeMetaAny.h>
#include <dae/daeMetaElementAttribute.h>

daeElementRef
domGlsl_param_type::create(DAE& dae)
{
	domGlsl_param_typeRef ref = new domGlsl_param_type(dae);
	return ref;
}


daeMetaElement *
domGlsl_param_type::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "glsl_param_type" );
	meta->registerClass(domGlsl_param_type::create);

	meta->setIsTransparent( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaChoice( meta, cm, 0, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "bool" );
	mea->setOffset( daeOffsetOf(domGlsl_param_type,elemBool) );
	mea->setElementType( domGlsl_param_type::domBool::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "bool2" );
	mea->setOffset( daeOffsetOf(domGlsl_param_type,elemBool2) );
	mea->setElementType( domGlsl_param_type::domBool2::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "bool3" );
	mea->setOffset( daeOffsetOf(domGlsl_param_type,elemBool3) );
	mea->setElementType( domGlsl_param_type::domBool3::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "bool4" );
	mea->setOffset( daeOffsetOf(domGlsl_param_type,elemBool4) );
	mea->setElementType( domGlsl_param_type::domBool4::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "float" );
	mea->setOffset( daeOffsetOf(domGlsl_param_type,elemFloat) );
	mea->setElementType( domGlsl_param_type::domFloat::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "float2" );
	mea->setOffset( daeOffsetOf(domGlsl_param_type,elemFloat2) );
	mea->setElementType( domGlsl_param_type::domFloat2::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "float3" );
	mea->setOffset( daeOffsetOf(domGlsl_param_type,elemFloat3) );
	mea->setElementType( domGlsl_param_type::domFloat3::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "float4" );
	mea->setOffset( daeOffsetOf(domGlsl_param_type,elemFloat4) );
	mea->setElementType( domGlsl_param_type::domFloat4::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "float2x2" );
	mea->setOffset( daeOffsetOf(domGlsl_param_type,elemFloat2x2) );
	mea->setElementType( domGlsl_param_type::domFloat2x2::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "float3x3" );
	mea->setOffset( daeOffsetOf(domGlsl_param_type,elemFloat3x3) );
	mea->setElementType( domGlsl_param_type::domFloat3x3::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "float4x4" );
	mea->setOffset( daeOffsetOf(domGlsl_param_type,elemFloat4x4) );
	mea->setElementType( domGlsl_param_type::domFloat4x4::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "int" );
	mea->setOffset( daeOffsetOf(domGlsl_param_type,elemInt) );
	mea->setElementType( domGlsl_param_type::domInt::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "int2" );
	mea->setOffset( daeOffsetOf(domGlsl_param_type,elemInt2) );
	mea->setElementType( domGlsl_param_type::domInt2::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "int3" );
	mea->setOffset( daeOffsetOf(domGlsl_param_type,elemInt3) );
	mea->setElementType( domGlsl_param_type::domInt3::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "int4" );
	mea->setOffset( daeOffsetOf(domGlsl_param_type,elemInt4) );
	mea->setElementType( domGlsl_param_type::domInt4::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "surface" );
	mea->setOffset( daeOffsetOf(domGlsl_param_type,elemSurface) );
	mea->setElementType( domGlsl_surface_type::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "sampler1D" );
	mea->setOffset( daeOffsetOf(domGlsl_param_type,elemSampler1D) );
	mea->setElementType( domGl_sampler1D::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "sampler2D" );
	mea->setOffset( daeOffsetOf(domGlsl_param_type,elemSampler2D) );
	mea->setElementType( domGl_sampler2D::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "sampler3D" );
	mea->setOffset( daeOffsetOf(domGlsl_param_type,elemSampler3D) );
	mea->setElementType( domGl_sampler3D::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "samplerCUBE" );
	mea->setOffset( daeOffsetOf(domGlsl_param_type,elemSamplerCUBE) );
	mea->setElementType( domGl_samplerCUBE::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "samplerRECT" );
	mea->setOffset( daeOffsetOf(domGlsl_param_type,elemSamplerRECT) );
	mea->setElementType( domGl_samplerRECT::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "samplerDEPTH" );
	mea->setOffset( daeOffsetOf(domGlsl_param_type,elemSamplerDEPTH) );
	mea->setElementType( domGl_samplerDEPTH::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "enum" );
	mea->setOffset( daeOffsetOf(domGlsl_param_type,elemEnum) );
	mea->setElementType( domGlsl_param_type::domEnum::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 0 );
	meta->setCMRoot( cm );	
	// Ordered list of sub-elements
	meta->addContents(daeOffsetOf(domGlsl_param_type,_contents));
	meta->addContentsOrder(daeOffsetOf(domGlsl_param_type,_contentsOrder));

	meta->addCMDataArray(daeOffsetOf(domGlsl_param_type,_CMData), 1);
	meta->setElementSize(sizeof(domGlsl_param_type));
	meta->validate();

	return meta;
}

daeElementRef
domGlsl_param_type::domBool::create(DAE& dae)
{
	domGlsl_param_type::domBoolRef ref = new domGlsl_param_type::domBool(dae);
	return ref;
}


daeMetaElement *
domGlsl_param_type::domBool::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "bool" );
	meta->registerClass(domGlsl_param_type::domBool::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Glsl_bool"));
		ma->setOffset( daeOffsetOf( domGlsl_param_type::domBool , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGlsl_param_type::domBool));
	meta->validate();

	return meta;
}

daeElementRef
domGlsl_param_type::domBool2::create(DAE& dae)
{
	domGlsl_param_type::domBool2Ref ref = new domGlsl_param_type::domBool2(dae);
	return ref;
}


daeMetaElement *
domGlsl_param_type::domBool2::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "bool2" );
	meta->registerClass(domGlsl_param_type::domBool2::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Glsl_bool2"));
		ma->setOffset( daeOffsetOf( domGlsl_param_type::domBool2 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGlsl_param_type::domBool2));
	meta->validate();

	return meta;
}

daeElementRef
domGlsl_param_type::domBool3::create(DAE& dae)
{
	domGlsl_param_type::domBool3Ref ref = new domGlsl_param_type::domBool3(dae);
	return ref;
}


daeMetaElement *
domGlsl_param_type::domBool3::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "bool3" );
	meta->registerClass(domGlsl_param_type::domBool3::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Glsl_bool3"));
		ma->setOffset( daeOffsetOf( domGlsl_param_type::domBool3 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGlsl_param_type::domBool3));
	meta->validate();

	return meta;
}

daeElementRef
domGlsl_param_type::domBool4::create(DAE& dae)
{
	domGlsl_param_type::domBool4Ref ref = new domGlsl_param_type::domBool4(dae);
	return ref;
}


daeMetaElement *
domGlsl_param_type::domBool4::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "bool4" );
	meta->registerClass(domGlsl_param_type::domBool4::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Glsl_bool4"));
		ma->setOffset( daeOffsetOf( domGlsl_param_type::domBool4 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGlsl_param_type::domBool4));
	meta->validate();

	return meta;
}

daeElementRef
domGlsl_param_type::domFloat::create(DAE& dae)
{
	domGlsl_param_type::domFloatRef ref = new domGlsl_param_type::domFloat(dae);
	return ref;
}


daeMetaElement *
domGlsl_param_type::domFloat::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "float" );
	meta->registerClass(domGlsl_param_type::domFloat::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Glsl_float"));
		ma->setOffset( daeOffsetOf( domGlsl_param_type::domFloat , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGlsl_param_type::domFloat));
	meta->validate();

	return meta;
}

daeElementRef
domGlsl_param_type::domFloat2::create(DAE& dae)
{
	domGlsl_param_type::domFloat2Ref ref = new domGlsl_param_type::domFloat2(dae);
	return ref;
}


daeMetaElement *
domGlsl_param_type::domFloat2::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "float2" );
	meta->registerClass(domGlsl_param_type::domFloat2::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Glsl_float2"));
		ma->setOffset( daeOffsetOf( domGlsl_param_type::domFloat2 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGlsl_param_type::domFloat2));
	meta->validate();

	return meta;
}

daeElementRef
domGlsl_param_type::domFloat3::create(DAE& dae)
{
	domGlsl_param_type::domFloat3Ref ref = new domGlsl_param_type::domFloat3(dae);
	return ref;
}


daeMetaElement *
domGlsl_param_type::domFloat3::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "float3" );
	meta->registerClass(domGlsl_param_type::domFloat3::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Glsl_float3"));
		ma->setOffset( daeOffsetOf( domGlsl_param_type::domFloat3 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGlsl_param_type::domFloat3));
	meta->validate();

	return meta;
}

daeElementRef
domGlsl_param_type::domFloat4::create(DAE& dae)
{
	domGlsl_param_type::domFloat4Ref ref = new domGlsl_param_type::domFloat4(dae);
	return ref;
}


daeMetaElement *
domGlsl_param_type::domFloat4::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "float4" );
	meta->registerClass(domGlsl_param_type::domFloat4::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Glsl_float4"));
		ma->setOffset( daeOffsetOf( domGlsl_param_type::domFloat4 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGlsl_param_type::domFloat4));
	meta->validate();

	return meta;
}

daeElementRef
domGlsl_param_type::domFloat2x2::create(DAE& dae)
{
	domGlsl_param_type::domFloat2x2Ref ref = new domGlsl_param_type::domFloat2x2(dae);
	return ref;
}


daeMetaElement *
domGlsl_param_type::domFloat2x2::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "float2x2" );
	meta->registerClass(domGlsl_param_type::domFloat2x2::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Glsl_float2x2"));
		ma->setOffset( daeOffsetOf( domGlsl_param_type::domFloat2x2 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGlsl_param_type::domFloat2x2));
	meta->validate();

	return meta;
}

daeElementRef
domGlsl_param_type::domFloat3x3::create(DAE& dae)
{
	domGlsl_param_type::domFloat3x3Ref ref = new domGlsl_param_type::domFloat3x3(dae);
	return ref;
}


daeMetaElement *
domGlsl_param_type::domFloat3x3::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "float3x3" );
	meta->registerClass(domGlsl_param_type::domFloat3x3::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Glsl_float3x3"));
		ma->setOffset( daeOffsetOf( domGlsl_param_type::domFloat3x3 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGlsl_param_type::domFloat3x3));
	meta->validate();

	return meta;
}

daeElementRef
domGlsl_param_type::domFloat4x4::create(DAE& dae)
{
	domGlsl_param_type::domFloat4x4Ref ref = new domGlsl_param_type::domFloat4x4(dae);
	return ref;
}


daeMetaElement *
domGlsl_param_type::domFloat4x4::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "float4x4" );
	meta->registerClass(domGlsl_param_type::domFloat4x4::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Glsl_float4x4"));
		ma->setOffset( daeOffsetOf( domGlsl_param_type::domFloat4x4 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGlsl_param_type::domFloat4x4));
	meta->validate();

	return meta;
}

daeElementRef
domGlsl_param_type::domInt::create(DAE& dae)
{
	domGlsl_param_type::domIntRef ref = new domGlsl_param_type::domInt(dae);
	return ref;
}


daeMetaElement *
domGlsl_param_type::domInt::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "int" );
	meta->registerClass(domGlsl_param_type::domInt::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Glsl_int"));
		ma->setOffset( daeOffsetOf( domGlsl_param_type::domInt , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGlsl_param_type::domInt));
	meta->validate();

	return meta;
}

daeElementRef
domGlsl_param_type::domInt2::create(DAE& dae)
{
	domGlsl_param_type::domInt2Ref ref = new domGlsl_param_type::domInt2(dae);
	return ref;
}


daeMetaElement *
domGlsl_param_type::domInt2::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "int2" );
	meta->registerClass(domGlsl_param_type::domInt2::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Glsl_int2"));
		ma->setOffset( daeOffsetOf( domGlsl_param_type::domInt2 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGlsl_param_type::domInt2));
	meta->validate();

	return meta;
}

daeElementRef
domGlsl_param_type::domInt3::create(DAE& dae)
{
	domGlsl_param_type::domInt3Ref ref = new domGlsl_param_type::domInt3(dae);
	return ref;
}


daeMetaElement *
domGlsl_param_type::domInt3::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "int3" );
	meta->registerClass(domGlsl_param_type::domInt3::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Glsl_int3"));
		ma->setOffset( daeOffsetOf( domGlsl_param_type::domInt3 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGlsl_param_type::domInt3));
	meta->validate();

	return meta;
}

daeElementRef
domGlsl_param_type::domInt4::create(DAE& dae)
{
	domGlsl_param_type::domInt4Ref ref = new domGlsl_param_type::domInt4(dae);
	return ref;
}


daeMetaElement *
domGlsl_param_type::domInt4::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "int4" );
	meta->registerClass(domGlsl_param_type::domInt4::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Glsl_int4"));
		ma->setOffset( daeOffsetOf( domGlsl_param_type::domInt4 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGlsl_param_type::domInt4));
	meta->validate();

	return meta;
}

daeElementRef
domGlsl_param_type::domEnum::create(DAE& dae)
{
	domGlsl_param_type::domEnumRef ref = new domGlsl_param_type::domEnum(dae);
	return ref;
}


daeMetaElement *
domGlsl_param_type::domEnum::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "enum" );
	meta->registerClass(domGlsl_param_type::domEnum::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Gl_enumeration"));
		ma->setOffset( daeOffsetOf( domGlsl_param_type::domEnum , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domGlsl_param_type::domEnum));
	meta->validate();

	return meta;
}

