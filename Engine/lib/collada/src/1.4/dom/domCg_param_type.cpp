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
#include <dom/domCg_param_type.h>
#include <dae/daeMetaCMPolicy.h>
#include <dae/daeMetaSequence.h>
#include <dae/daeMetaChoice.h>
#include <dae/daeMetaGroup.h>
#include <dae/daeMetaAny.h>
#include <dae/daeMetaElementAttribute.h>

daeElementRef
domCg_param_type::create(DAE& dae)
{
	domCg_param_typeRef ref = new domCg_param_type(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "cg_param_type" );
	meta->registerClass(domCg_param_type::create);

	meta->setIsTransparent( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaChoice( meta, cm, 0, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "bool" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemBool) );
	mea->setElementType( domCg_param_type::domBool::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "bool1" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemBool1) );
	mea->setElementType( domCg_param_type::domBool1::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "bool2" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemBool2) );
	mea->setElementType( domCg_param_type::domBool2::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "bool3" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemBool3) );
	mea->setElementType( domCg_param_type::domBool3::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "bool4" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemBool4) );
	mea->setElementType( domCg_param_type::domBool4::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "bool1x1" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemBool1x1) );
	mea->setElementType( domCg_param_type::domBool1x1::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "bool1x2" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemBool1x2) );
	mea->setElementType( domCg_param_type::domBool1x2::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "bool1x3" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemBool1x3) );
	mea->setElementType( domCg_param_type::domBool1x3::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "bool1x4" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemBool1x4) );
	mea->setElementType( domCg_param_type::domBool1x4::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "bool2x1" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemBool2x1) );
	mea->setElementType( domCg_param_type::domBool2x1::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "bool2x2" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemBool2x2) );
	mea->setElementType( domCg_param_type::domBool2x2::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "bool2x3" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemBool2x3) );
	mea->setElementType( domCg_param_type::domBool2x3::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "bool2x4" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemBool2x4) );
	mea->setElementType( domCg_param_type::domBool2x4::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "bool3x1" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemBool3x1) );
	mea->setElementType( domCg_param_type::domBool3x1::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "bool3x2" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemBool3x2) );
	mea->setElementType( domCg_param_type::domBool3x2::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "bool3x3" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemBool3x3) );
	mea->setElementType( domCg_param_type::domBool3x3::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "bool3x4" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemBool3x4) );
	mea->setElementType( domCg_param_type::domBool3x4::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "bool4x1" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemBool4x1) );
	mea->setElementType( domCg_param_type::domBool4x1::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "bool4x2" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemBool4x2) );
	mea->setElementType( domCg_param_type::domBool4x2::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "bool4x3" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemBool4x3) );
	mea->setElementType( domCg_param_type::domBool4x3::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "bool4x4" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemBool4x4) );
	mea->setElementType( domCg_param_type::domBool4x4::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "float" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemFloat) );
	mea->setElementType( domCg_param_type::domFloat::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "float1" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemFloat1) );
	mea->setElementType( domCg_param_type::domFloat1::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "float2" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemFloat2) );
	mea->setElementType( domCg_param_type::domFloat2::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "float3" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemFloat3) );
	mea->setElementType( domCg_param_type::domFloat3::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "float4" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemFloat4) );
	mea->setElementType( domCg_param_type::domFloat4::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "float1x1" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemFloat1x1) );
	mea->setElementType( domCg_param_type::domFloat1x1::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "float1x2" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemFloat1x2) );
	mea->setElementType( domCg_param_type::domFloat1x2::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "float1x3" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemFloat1x3) );
	mea->setElementType( domCg_param_type::domFloat1x3::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "float1x4" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemFloat1x4) );
	mea->setElementType( domCg_param_type::domFloat1x4::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "float2x1" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemFloat2x1) );
	mea->setElementType( domCg_param_type::domFloat2x1::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "float2x2" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemFloat2x2) );
	mea->setElementType( domCg_param_type::domFloat2x2::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "float2x3" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemFloat2x3) );
	mea->setElementType( domCg_param_type::domFloat2x3::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "float2x4" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemFloat2x4) );
	mea->setElementType( domCg_param_type::domFloat2x4::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "float3x1" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemFloat3x1) );
	mea->setElementType( domCg_param_type::domFloat3x1::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "float3x2" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemFloat3x2) );
	mea->setElementType( domCg_param_type::domFloat3x2::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "float3x3" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemFloat3x3) );
	mea->setElementType( domCg_param_type::domFloat3x3::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "float3x4" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemFloat3x4) );
	mea->setElementType( domCg_param_type::domFloat3x4::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "float4x1" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemFloat4x1) );
	mea->setElementType( domCg_param_type::domFloat4x1::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "float4x2" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemFloat4x2) );
	mea->setElementType( domCg_param_type::domFloat4x2::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "float4x3" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemFloat4x3) );
	mea->setElementType( domCg_param_type::domFloat4x3::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "float4x4" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemFloat4x4) );
	mea->setElementType( domCg_param_type::domFloat4x4::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "int" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemInt) );
	mea->setElementType( domCg_param_type::domInt::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "int1" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemInt1) );
	mea->setElementType( domCg_param_type::domInt1::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "int2" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemInt2) );
	mea->setElementType( domCg_param_type::domInt2::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "int3" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemInt3) );
	mea->setElementType( domCg_param_type::domInt3::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "int4" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemInt4) );
	mea->setElementType( domCg_param_type::domInt4::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "int1x1" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemInt1x1) );
	mea->setElementType( domCg_param_type::domInt1x1::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "int1x2" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemInt1x2) );
	mea->setElementType( domCg_param_type::domInt1x2::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "int1x3" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemInt1x3) );
	mea->setElementType( domCg_param_type::domInt1x3::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "int1x4" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemInt1x4) );
	mea->setElementType( domCg_param_type::domInt1x4::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "int2x1" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemInt2x1) );
	mea->setElementType( domCg_param_type::domInt2x1::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "int2x2" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemInt2x2) );
	mea->setElementType( domCg_param_type::domInt2x2::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "int2x3" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemInt2x3) );
	mea->setElementType( domCg_param_type::domInt2x3::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "int2x4" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemInt2x4) );
	mea->setElementType( domCg_param_type::domInt2x4::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "int3x1" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemInt3x1) );
	mea->setElementType( domCg_param_type::domInt3x1::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "int3x2" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemInt3x2) );
	mea->setElementType( domCg_param_type::domInt3x2::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "int3x3" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemInt3x3) );
	mea->setElementType( domCg_param_type::domInt3x3::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "int3x4" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemInt3x4) );
	mea->setElementType( domCg_param_type::domInt3x4::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "int4x1" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemInt4x1) );
	mea->setElementType( domCg_param_type::domInt4x1::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "int4x2" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemInt4x2) );
	mea->setElementType( domCg_param_type::domInt4x2::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "int4x3" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemInt4x3) );
	mea->setElementType( domCg_param_type::domInt4x3::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "int4x4" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemInt4x4) );
	mea->setElementType( domCg_param_type::domInt4x4::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "half" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemHalf) );
	mea->setElementType( domCg_param_type::domHalf::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "half1" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemHalf1) );
	mea->setElementType( domCg_param_type::domHalf1::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "half2" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemHalf2) );
	mea->setElementType( domCg_param_type::domHalf2::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "half3" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemHalf3) );
	mea->setElementType( domCg_param_type::domHalf3::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "half4" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemHalf4) );
	mea->setElementType( domCg_param_type::domHalf4::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "half1x1" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemHalf1x1) );
	mea->setElementType( domCg_param_type::domHalf1x1::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "half1x2" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemHalf1x2) );
	mea->setElementType( domCg_param_type::domHalf1x2::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "half1x3" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemHalf1x3) );
	mea->setElementType( domCg_param_type::domHalf1x3::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "half1x4" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemHalf1x4) );
	mea->setElementType( domCg_param_type::domHalf1x4::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "half2x1" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemHalf2x1) );
	mea->setElementType( domCg_param_type::domHalf2x1::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "half2x2" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemHalf2x2) );
	mea->setElementType( domCg_param_type::domHalf2x2::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "half2x3" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemHalf2x3) );
	mea->setElementType( domCg_param_type::domHalf2x3::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "half2x4" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemHalf2x4) );
	mea->setElementType( domCg_param_type::domHalf2x4::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "half3x1" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemHalf3x1) );
	mea->setElementType( domCg_param_type::domHalf3x1::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "half3x2" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemHalf3x2) );
	mea->setElementType( domCg_param_type::domHalf3x2::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "half3x3" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemHalf3x3) );
	mea->setElementType( domCg_param_type::domHalf3x3::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "half3x4" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemHalf3x4) );
	mea->setElementType( domCg_param_type::domHalf3x4::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "half4x1" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemHalf4x1) );
	mea->setElementType( domCg_param_type::domHalf4x1::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "half4x2" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemHalf4x2) );
	mea->setElementType( domCg_param_type::domHalf4x2::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "half4x3" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemHalf4x3) );
	mea->setElementType( domCg_param_type::domHalf4x3::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "half4x4" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemHalf4x4) );
	mea->setElementType( domCg_param_type::domHalf4x4::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "fixed" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemFixed) );
	mea->setElementType( domCg_param_type::domFixed::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "fixed1" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemFixed1) );
	mea->setElementType( domCg_param_type::domFixed1::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "fixed2" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemFixed2) );
	mea->setElementType( domCg_param_type::domFixed2::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "fixed3" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemFixed3) );
	mea->setElementType( domCg_param_type::domFixed3::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "fixed4" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemFixed4) );
	mea->setElementType( domCg_param_type::domFixed4::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "fixed1x1" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemFixed1x1) );
	mea->setElementType( domCg_param_type::domFixed1x1::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "fixed1x2" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemFixed1x2) );
	mea->setElementType( domCg_param_type::domFixed1x2::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "fixed1x3" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemFixed1x3) );
	mea->setElementType( domCg_param_type::domFixed1x3::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "fixed1x4" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemFixed1x4) );
	mea->setElementType( domCg_param_type::domFixed1x4::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "fixed2x1" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemFixed2x1) );
	mea->setElementType( domCg_param_type::domFixed2x1::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "fixed2x2" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemFixed2x2) );
	mea->setElementType( domCg_param_type::domFixed2x2::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "fixed2x3" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemFixed2x3) );
	mea->setElementType( domCg_param_type::domFixed2x3::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "fixed2x4" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemFixed2x4) );
	mea->setElementType( domCg_param_type::domFixed2x4::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "fixed3x1" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemFixed3x1) );
	mea->setElementType( domCg_param_type::domFixed3x1::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "fixed3x2" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemFixed3x2) );
	mea->setElementType( domCg_param_type::domFixed3x2::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "fixed3x3" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemFixed3x3) );
	mea->setElementType( domCg_param_type::domFixed3x3::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "fixed3x4" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemFixed3x4) );
	mea->setElementType( domCg_param_type::domFixed3x4::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "fixed4x1" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemFixed4x1) );
	mea->setElementType( domCg_param_type::domFixed4x1::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "fixed4x2" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemFixed4x2) );
	mea->setElementType( domCg_param_type::domFixed4x2::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "fixed4x3" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemFixed4x3) );
	mea->setElementType( domCg_param_type::domFixed4x3::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "fixed4x4" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemFixed4x4) );
	mea->setElementType( domCg_param_type::domFixed4x4::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "surface" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemSurface) );
	mea->setElementType( domCg_surface_type::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "sampler1D" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemSampler1D) );
	mea->setElementType( domCg_sampler1D::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "sampler2D" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemSampler2D) );
	mea->setElementType( domCg_sampler2D::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "sampler3D" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemSampler3D) );
	mea->setElementType( domCg_sampler3D::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "samplerRECT" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemSamplerRECT) );
	mea->setElementType( domCg_samplerRECT::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "samplerCUBE" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemSamplerCUBE) );
	mea->setElementType( domCg_samplerCUBE::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "samplerDEPTH" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemSamplerDEPTH) );
	mea->setElementType( domCg_samplerDEPTH::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "string" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemString) );
	mea->setElementType( domCg_param_type::domString::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "enum" );
	mea->setOffset( daeOffsetOf(domCg_param_type,elemEnum) );
	mea->setElementType( domCg_param_type::domEnum::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 0 );
	meta->setCMRoot( cm );	
	// Ordered list of sub-elements
	meta->addContents(daeOffsetOf(domCg_param_type,_contents));
	meta->addContentsOrder(daeOffsetOf(domCg_param_type,_contentsOrder));

	meta->addCMDataArray(daeOffsetOf(domCg_param_type,_CMData), 1);
	meta->setElementSize(sizeof(domCg_param_type));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domBool::create(DAE& dae)
{
	domCg_param_type::domBoolRef ref = new domCg_param_type::domBool(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domBool::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "bool" );
	meta->registerClass(domCg_param_type::domBool::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_bool"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domBool , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domBool));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domBool1::create(DAE& dae)
{
	domCg_param_type::domBool1Ref ref = new domCg_param_type::domBool1(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domBool1::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "bool1" );
	meta->registerClass(domCg_param_type::domBool1::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_bool1"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domBool1 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domBool1));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domBool2::create(DAE& dae)
{
	domCg_param_type::domBool2Ref ref = new domCg_param_type::domBool2(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domBool2::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "bool2" );
	meta->registerClass(domCg_param_type::domBool2::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_bool2"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domBool2 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domBool2));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domBool3::create(DAE& dae)
{
	domCg_param_type::domBool3Ref ref = new domCg_param_type::domBool3(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domBool3::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "bool3" );
	meta->registerClass(domCg_param_type::domBool3::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_bool3"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domBool3 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domBool3));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domBool4::create(DAE& dae)
{
	domCg_param_type::domBool4Ref ref = new domCg_param_type::domBool4(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domBool4::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "bool4" );
	meta->registerClass(domCg_param_type::domBool4::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_bool4"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domBool4 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domBool4));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domBool1x1::create(DAE& dae)
{
	domCg_param_type::domBool1x1Ref ref = new domCg_param_type::domBool1x1(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domBool1x1::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "bool1x1" );
	meta->registerClass(domCg_param_type::domBool1x1::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_bool1x1"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domBool1x1 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domBool1x1));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domBool1x2::create(DAE& dae)
{
	domCg_param_type::domBool1x2Ref ref = new domCg_param_type::domBool1x2(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domBool1x2::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "bool1x2" );
	meta->registerClass(domCg_param_type::domBool1x2::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_bool1x2"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domBool1x2 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domBool1x2));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domBool1x3::create(DAE& dae)
{
	domCg_param_type::domBool1x3Ref ref = new domCg_param_type::domBool1x3(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domBool1x3::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "bool1x3" );
	meta->registerClass(domCg_param_type::domBool1x3::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_bool1x3"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domBool1x3 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domBool1x3));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domBool1x4::create(DAE& dae)
{
	domCg_param_type::domBool1x4Ref ref = new domCg_param_type::domBool1x4(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domBool1x4::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "bool1x4" );
	meta->registerClass(domCg_param_type::domBool1x4::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_bool1x4"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domBool1x4 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domBool1x4));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domBool2x1::create(DAE& dae)
{
	domCg_param_type::domBool2x1Ref ref = new domCg_param_type::domBool2x1(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domBool2x1::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "bool2x1" );
	meta->registerClass(domCg_param_type::domBool2x1::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_bool2x1"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domBool2x1 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domBool2x1));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domBool2x2::create(DAE& dae)
{
	domCg_param_type::domBool2x2Ref ref = new domCg_param_type::domBool2x2(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domBool2x2::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "bool2x2" );
	meta->registerClass(domCg_param_type::domBool2x2::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_bool2x2"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domBool2x2 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domBool2x2));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domBool2x3::create(DAE& dae)
{
	domCg_param_type::domBool2x3Ref ref = new domCg_param_type::domBool2x3(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domBool2x3::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "bool2x3" );
	meta->registerClass(domCg_param_type::domBool2x3::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_bool2x3"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domBool2x3 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domBool2x3));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domBool2x4::create(DAE& dae)
{
	domCg_param_type::domBool2x4Ref ref = new domCg_param_type::domBool2x4(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domBool2x4::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "bool2x4" );
	meta->registerClass(domCg_param_type::domBool2x4::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_bool2x4"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domBool2x4 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domBool2x4));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domBool3x1::create(DAE& dae)
{
	domCg_param_type::domBool3x1Ref ref = new domCg_param_type::domBool3x1(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domBool3x1::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "bool3x1" );
	meta->registerClass(domCg_param_type::domBool3x1::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_bool3x1"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domBool3x1 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domBool3x1));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domBool3x2::create(DAE& dae)
{
	domCg_param_type::domBool3x2Ref ref = new domCg_param_type::domBool3x2(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domBool3x2::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "bool3x2" );
	meta->registerClass(domCg_param_type::domBool3x2::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_bool3x2"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domBool3x2 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domBool3x2));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domBool3x3::create(DAE& dae)
{
	domCg_param_type::domBool3x3Ref ref = new domCg_param_type::domBool3x3(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domBool3x3::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "bool3x3" );
	meta->registerClass(domCg_param_type::domBool3x3::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_bool3x3"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domBool3x3 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domBool3x3));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domBool3x4::create(DAE& dae)
{
	domCg_param_type::domBool3x4Ref ref = new domCg_param_type::domBool3x4(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domBool3x4::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "bool3x4" );
	meta->registerClass(domCg_param_type::domBool3x4::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_bool3x4"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domBool3x4 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domBool3x4));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domBool4x1::create(DAE& dae)
{
	domCg_param_type::domBool4x1Ref ref = new domCg_param_type::domBool4x1(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domBool4x1::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "bool4x1" );
	meta->registerClass(domCg_param_type::domBool4x1::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_bool4x1"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domBool4x1 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domBool4x1));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domBool4x2::create(DAE& dae)
{
	domCg_param_type::domBool4x2Ref ref = new domCg_param_type::domBool4x2(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domBool4x2::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "bool4x2" );
	meta->registerClass(domCg_param_type::domBool4x2::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_bool4x2"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domBool4x2 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domBool4x2));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domBool4x3::create(DAE& dae)
{
	domCg_param_type::domBool4x3Ref ref = new domCg_param_type::domBool4x3(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domBool4x3::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "bool4x3" );
	meta->registerClass(domCg_param_type::domBool4x3::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_bool4x3"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domBool4x3 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domBool4x3));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domBool4x4::create(DAE& dae)
{
	domCg_param_type::domBool4x4Ref ref = new domCg_param_type::domBool4x4(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domBool4x4::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "bool4x4" );
	meta->registerClass(domCg_param_type::domBool4x4::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_bool4x4"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domBool4x4 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domBool4x4));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domFloat::create(DAE& dae)
{
	domCg_param_type::domFloatRef ref = new domCg_param_type::domFloat(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domFloat::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "float" );
	meta->registerClass(domCg_param_type::domFloat::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_float"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domFloat , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domFloat));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domFloat1::create(DAE& dae)
{
	domCg_param_type::domFloat1Ref ref = new domCg_param_type::domFloat1(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domFloat1::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "float1" );
	meta->registerClass(domCg_param_type::domFloat1::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_float1"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domFloat1 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domFloat1));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domFloat2::create(DAE& dae)
{
	domCg_param_type::domFloat2Ref ref = new domCg_param_type::domFloat2(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domFloat2::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "float2" );
	meta->registerClass(domCg_param_type::domFloat2::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_float2"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domFloat2 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domFloat2));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domFloat3::create(DAE& dae)
{
	domCg_param_type::domFloat3Ref ref = new domCg_param_type::domFloat3(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domFloat3::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "float3" );
	meta->registerClass(domCg_param_type::domFloat3::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_float3"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domFloat3 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domFloat3));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domFloat4::create(DAE& dae)
{
	domCg_param_type::domFloat4Ref ref = new domCg_param_type::domFloat4(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domFloat4::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "float4" );
	meta->registerClass(domCg_param_type::domFloat4::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_float4"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domFloat4 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domFloat4));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domFloat1x1::create(DAE& dae)
{
	domCg_param_type::domFloat1x1Ref ref = new domCg_param_type::domFloat1x1(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domFloat1x1::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "float1x1" );
	meta->registerClass(domCg_param_type::domFloat1x1::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_float1x1"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domFloat1x1 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domFloat1x1));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domFloat1x2::create(DAE& dae)
{
	domCg_param_type::domFloat1x2Ref ref = new domCg_param_type::domFloat1x2(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domFloat1x2::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "float1x2" );
	meta->registerClass(domCg_param_type::domFloat1x2::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_float1x2"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domFloat1x2 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domFloat1x2));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domFloat1x3::create(DAE& dae)
{
	domCg_param_type::domFloat1x3Ref ref = new domCg_param_type::domFloat1x3(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domFloat1x3::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "float1x3" );
	meta->registerClass(domCg_param_type::domFloat1x3::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_float1x3"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domFloat1x3 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domFloat1x3));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domFloat1x4::create(DAE& dae)
{
	domCg_param_type::domFloat1x4Ref ref = new domCg_param_type::domFloat1x4(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domFloat1x4::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "float1x4" );
	meta->registerClass(domCg_param_type::domFloat1x4::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_float1x4"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domFloat1x4 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domFloat1x4));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domFloat2x1::create(DAE& dae)
{
	domCg_param_type::domFloat2x1Ref ref = new domCg_param_type::domFloat2x1(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domFloat2x1::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "float2x1" );
	meta->registerClass(domCg_param_type::domFloat2x1::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_float2x1"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domFloat2x1 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domFloat2x1));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domFloat2x2::create(DAE& dae)
{
	domCg_param_type::domFloat2x2Ref ref = new domCg_param_type::domFloat2x2(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domFloat2x2::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "float2x2" );
	meta->registerClass(domCg_param_type::domFloat2x2::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_float2x2"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domFloat2x2 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domFloat2x2));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domFloat2x3::create(DAE& dae)
{
	domCg_param_type::domFloat2x3Ref ref = new domCg_param_type::domFloat2x3(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domFloat2x3::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "float2x3" );
	meta->registerClass(domCg_param_type::domFloat2x3::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_float2x3"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domFloat2x3 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domFloat2x3));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domFloat2x4::create(DAE& dae)
{
	domCg_param_type::domFloat2x4Ref ref = new domCg_param_type::domFloat2x4(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domFloat2x4::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "float2x4" );
	meta->registerClass(domCg_param_type::domFloat2x4::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_float2x4"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domFloat2x4 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domFloat2x4));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domFloat3x1::create(DAE& dae)
{
	domCg_param_type::domFloat3x1Ref ref = new domCg_param_type::domFloat3x1(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domFloat3x1::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "float3x1" );
	meta->registerClass(domCg_param_type::domFloat3x1::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_float3x1"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domFloat3x1 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domFloat3x1));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domFloat3x2::create(DAE& dae)
{
	domCg_param_type::domFloat3x2Ref ref = new domCg_param_type::domFloat3x2(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domFloat3x2::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "float3x2" );
	meta->registerClass(domCg_param_type::domFloat3x2::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_float3x2"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domFloat3x2 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domFloat3x2));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domFloat3x3::create(DAE& dae)
{
	domCg_param_type::domFloat3x3Ref ref = new domCg_param_type::domFloat3x3(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domFloat3x3::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "float3x3" );
	meta->registerClass(domCg_param_type::domFloat3x3::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_float3x3"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domFloat3x3 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domFloat3x3));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domFloat3x4::create(DAE& dae)
{
	domCg_param_type::domFloat3x4Ref ref = new domCg_param_type::domFloat3x4(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domFloat3x4::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "float3x4" );
	meta->registerClass(domCg_param_type::domFloat3x4::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_float3x4"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domFloat3x4 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domFloat3x4));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domFloat4x1::create(DAE& dae)
{
	domCg_param_type::domFloat4x1Ref ref = new domCg_param_type::domFloat4x1(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domFloat4x1::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "float4x1" );
	meta->registerClass(domCg_param_type::domFloat4x1::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_float4x1"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domFloat4x1 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domFloat4x1));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domFloat4x2::create(DAE& dae)
{
	domCg_param_type::domFloat4x2Ref ref = new domCg_param_type::domFloat4x2(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domFloat4x2::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "float4x2" );
	meta->registerClass(domCg_param_type::domFloat4x2::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_float4x2"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domFloat4x2 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domFloat4x2));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domFloat4x3::create(DAE& dae)
{
	domCg_param_type::domFloat4x3Ref ref = new domCg_param_type::domFloat4x3(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domFloat4x3::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "float4x3" );
	meta->registerClass(domCg_param_type::domFloat4x3::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_float4x3"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domFloat4x3 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domFloat4x3));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domFloat4x4::create(DAE& dae)
{
	domCg_param_type::domFloat4x4Ref ref = new domCg_param_type::domFloat4x4(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domFloat4x4::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "float4x4" );
	meta->registerClass(domCg_param_type::domFloat4x4::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_float4x4"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domFloat4x4 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domFloat4x4));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domInt::create(DAE& dae)
{
	domCg_param_type::domIntRef ref = new domCg_param_type::domInt(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domInt::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "int" );
	meta->registerClass(domCg_param_type::domInt::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_int"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domInt , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domInt));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domInt1::create(DAE& dae)
{
	domCg_param_type::domInt1Ref ref = new domCg_param_type::domInt1(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domInt1::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "int1" );
	meta->registerClass(domCg_param_type::domInt1::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_int1"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domInt1 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domInt1));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domInt2::create(DAE& dae)
{
	domCg_param_type::domInt2Ref ref = new domCg_param_type::domInt2(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domInt2::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "int2" );
	meta->registerClass(domCg_param_type::domInt2::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_int2"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domInt2 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domInt2));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domInt3::create(DAE& dae)
{
	domCg_param_type::domInt3Ref ref = new domCg_param_type::domInt3(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domInt3::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "int3" );
	meta->registerClass(domCg_param_type::domInt3::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_int3"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domInt3 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domInt3));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domInt4::create(DAE& dae)
{
	domCg_param_type::domInt4Ref ref = new domCg_param_type::domInt4(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domInt4::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "int4" );
	meta->registerClass(domCg_param_type::domInt4::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_int4"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domInt4 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domInt4));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domInt1x1::create(DAE& dae)
{
	domCg_param_type::domInt1x1Ref ref = new domCg_param_type::domInt1x1(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domInt1x1::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "int1x1" );
	meta->registerClass(domCg_param_type::domInt1x1::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_int1x1"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domInt1x1 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domInt1x1));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domInt1x2::create(DAE& dae)
{
	domCg_param_type::domInt1x2Ref ref = new domCg_param_type::domInt1x2(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domInt1x2::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "int1x2" );
	meta->registerClass(domCg_param_type::domInt1x2::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_int1x2"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domInt1x2 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domInt1x2));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domInt1x3::create(DAE& dae)
{
	domCg_param_type::domInt1x3Ref ref = new domCg_param_type::domInt1x3(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domInt1x3::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "int1x3" );
	meta->registerClass(domCg_param_type::domInt1x3::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_int1x3"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domInt1x3 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domInt1x3));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domInt1x4::create(DAE& dae)
{
	domCg_param_type::domInt1x4Ref ref = new domCg_param_type::domInt1x4(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domInt1x4::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "int1x4" );
	meta->registerClass(domCg_param_type::domInt1x4::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_int1x4"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domInt1x4 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domInt1x4));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domInt2x1::create(DAE& dae)
{
	domCg_param_type::domInt2x1Ref ref = new domCg_param_type::domInt2x1(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domInt2x1::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "int2x1" );
	meta->registerClass(domCg_param_type::domInt2x1::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_int2x1"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domInt2x1 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domInt2x1));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domInt2x2::create(DAE& dae)
{
	domCg_param_type::domInt2x2Ref ref = new domCg_param_type::domInt2x2(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domInt2x2::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "int2x2" );
	meta->registerClass(domCg_param_type::domInt2x2::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_int2x2"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domInt2x2 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domInt2x2));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domInt2x3::create(DAE& dae)
{
	domCg_param_type::domInt2x3Ref ref = new domCg_param_type::domInt2x3(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domInt2x3::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "int2x3" );
	meta->registerClass(domCg_param_type::domInt2x3::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_int2x3"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domInt2x3 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domInt2x3));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domInt2x4::create(DAE& dae)
{
	domCg_param_type::domInt2x4Ref ref = new domCg_param_type::domInt2x4(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domInt2x4::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "int2x4" );
	meta->registerClass(domCg_param_type::domInt2x4::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_int2x4"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domInt2x4 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domInt2x4));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domInt3x1::create(DAE& dae)
{
	domCg_param_type::domInt3x1Ref ref = new domCg_param_type::domInt3x1(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domInt3x1::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "int3x1" );
	meta->registerClass(domCg_param_type::domInt3x1::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_int3x1"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domInt3x1 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domInt3x1));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domInt3x2::create(DAE& dae)
{
	domCg_param_type::domInt3x2Ref ref = new domCg_param_type::domInt3x2(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domInt3x2::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "int3x2" );
	meta->registerClass(domCg_param_type::domInt3x2::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_int3x2"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domInt3x2 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domInt3x2));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domInt3x3::create(DAE& dae)
{
	domCg_param_type::domInt3x3Ref ref = new domCg_param_type::domInt3x3(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domInt3x3::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "int3x3" );
	meta->registerClass(domCg_param_type::domInt3x3::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_int3x3"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domInt3x3 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domInt3x3));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domInt3x4::create(DAE& dae)
{
	domCg_param_type::domInt3x4Ref ref = new domCg_param_type::domInt3x4(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domInt3x4::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "int3x4" );
	meta->registerClass(domCg_param_type::domInt3x4::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_int3x4"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domInt3x4 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domInt3x4));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domInt4x1::create(DAE& dae)
{
	domCg_param_type::domInt4x1Ref ref = new domCg_param_type::domInt4x1(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domInt4x1::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "int4x1" );
	meta->registerClass(domCg_param_type::domInt4x1::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_int4x1"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domInt4x1 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domInt4x1));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domInt4x2::create(DAE& dae)
{
	domCg_param_type::domInt4x2Ref ref = new domCg_param_type::domInt4x2(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domInt4x2::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "int4x2" );
	meta->registerClass(domCg_param_type::domInt4x2::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_int4x2"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domInt4x2 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domInt4x2));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domInt4x3::create(DAE& dae)
{
	domCg_param_type::domInt4x3Ref ref = new domCg_param_type::domInt4x3(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domInt4x3::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "int4x3" );
	meta->registerClass(domCg_param_type::domInt4x3::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_int4x3"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domInt4x3 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domInt4x3));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domInt4x4::create(DAE& dae)
{
	domCg_param_type::domInt4x4Ref ref = new domCg_param_type::domInt4x4(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domInt4x4::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "int4x4" );
	meta->registerClass(domCg_param_type::domInt4x4::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_int4x4"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domInt4x4 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domInt4x4));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domHalf::create(DAE& dae)
{
	domCg_param_type::domHalfRef ref = new domCg_param_type::domHalf(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domHalf::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "half" );
	meta->registerClass(domCg_param_type::domHalf::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_half"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domHalf , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domHalf));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domHalf1::create(DAE& dae)
{
	domCg_param_type::domHalf1Ref ref = new domCg_param_type::domHalf1(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domHalf1::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "half1" );
	meta->registerClass(domCg_param_type::domHalf1::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_half1"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domHalf1 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domHalf1));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domHalf2::create(DAE& dae)
{
	domCg_param_type::domHalf2Ref ref = new domCg_param_type::domHalf2(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domHalf2::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "half2" );
	meta->registerClass(domCg_param_type::domHalf2::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_half2"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domHalf2 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domHalf2));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domHalf3::create(DAE& dae)
{
	domCg_param_type::domHalf3Ref ref = new domCg_param_type::domHalf3(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domHalf3::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "half3" );
	meta->registerClass(domCg_param_type::domHalf3::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_half3"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domHalf3 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domHalf3));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domHalf4::create(DAE& dae)
{
	domCg_param_type::domHalf4Ref ref = new domCg_param_type::domHalf4(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domHalf4::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "half4" );
	meta->registerClass(domCg_param_type::domHalf4::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_half4"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domHalf4 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domHalf4));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domHalf1x1::create(DAE& dae)
{
	domCg_param_type::domHalf1x1Ref ref = new domCg_param_type::domHalf1x1(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domHalf1x1::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "half1x1" );
	meta->registerClass(domCg_param_type::domHalf1x1::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_half1x1"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domHalf1x1 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domHalf1x1));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domHalf1x2::create(DAE& dae)
{
	domCg_param_type::domHalf1x2Ref ref = new domCg_param_type::domHalf1x2(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domHalf1x2::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "half1x2" );
	meta->registerClass(domCg_param_type::domHalf1x2::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_half1x2"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domHalf1x2 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domHalf1x2));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domHalf1x3::create(DAE& dae)
{
	domCg_param_type::domHalf1x3Ref ref = new domCg_param_type::domHalf1x3(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domHalf1x3::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "half1x3" );
	meta->registerClass(domCg_param_type::domHalf1x3::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_half1x3"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domHalf1x3 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domHalf1x3));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domHalf1x4::create(DAE& dae)
{
	domCg_param_type::domHalf1x4Ref ref = new domCg_param_type::domHalf1x4(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domHalf1x4::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "half1x4" );
	meta->registerClass(domCg_param_type::domHalf1x4::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_half1x4"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domHalf1x4 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domHalf1x4));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domHalf2x1::create(DAE& dae)
{
	domCg_param_type::domHalf2x1Ref ref = new domCg_param_type::domHalf2x1(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domHalf2x1::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "half2x1" );
	meta->registerClass(domCg_param_type::domHalf2x1::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_half2x1"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domHalf2x1 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domHalf2x1));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domHalf2x2::create(DAE& dae)
{
	domCg_param_type::domHalf2x2Ref ref = new domCg_param_type::domHalf2x2(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domHalf2x2::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "half2x2" );
	meta->registerClass(domCg_param_type::domHalf2x2::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_half2x2"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domHalf2x2 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domHalf2x2));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domHalf2x3::create(DAE& dae)
{
	domCg_param_type::domHalf2x3Ref ref = new domCg_param_type::domHalf2x3(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domHalf2x3::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "half2x3" );
	meta->registerClass(domCg_param_type::domHalf2x3::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_half2x3"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domHalf2x3 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domHalf2x3));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domHalf2x4::create(DAE& dae)
{
	domCg_param_type::domHalf2x4Ref ref = new domCg_param_type::domHalf2x4(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domHalf2x4::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "half2x4" );
	meta->registerClass(domCg_param_type::domHalf2x4::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_half2x4"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domHalf2x4 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domHalf2x4));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domHalf3x1::create(DAE& dae)
{
	domCg_param_type::domHalf3x1Ref ref = new domCg_param_type::domHalf3x1(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domHalf3x1::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "half3x1" );
	meta->registerClass(domCg_param_type::domHalf3x1::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_half3x1"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domHalf3x1 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domHalf3x1));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domHalf3x2::create(DAE& dae)
{
	domCg_param_type::domHalf3x2Ref ref = new domCg_param_type::domHalf3x2(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domHalf3x2::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "half3x2" );
	meta->registerClass(domCg_param_type::domHalf3x2::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_half3x2"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domHalf3x2 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domHalf3x2));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domHalf3x3::create(DAE& dae)
{
	domCg_param_type::domHalf3x3Ref ref = new domCg_param_type::domHalf3x3(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domHalf3x3::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "half3x3" );
	meta->registerClass(domCg_param_type::domHalf3x3::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_half3x3"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domHalf3x3 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domHalf3x3));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domHalf3x4::create(DAE& dae)
{
	domCg_param_type::domHalf3x4Ref ref = new domCg_param_type::domHalf3x4(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domHalf3x4::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "half3x4" );
	meta->registerClass(domCg_param_type::domHalf3x4::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_half3x4"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domHalf3x4 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domHalf3x4));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domHalf4x1::create(DAE& dae)
{
	domCg_param_type::domHalf4x1Ref ref = new domCg_param_type::domHalf4x1(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domHalf4x1::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "half4x1" );
	meta->registerClass(domCg_param_type::domHalf4x1::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_half4x1"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domHalf4x1 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domHalf4x1));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domHalf4x2::create(DAE& dae)
{
	domCg_param_type::domHalf4x2Ref ref = new domCg_param_type::domHalf4x2(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domHalf4x2::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "half4x2" );
	meta->registerClass(domCg_param_type::domHalf4x2::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_half4x2"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domHalf4x2 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domHalf4x2));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domHalf4x3::create(DAE& dae)
{
	domCg_param_type::domHalf4x3Ref ref = new domCg_param_type::domHalf4x3(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domHalf4x3::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "half4x3" );
	meta->registerClass(domCg_param_type::domHalf4x3::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_half4x3"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domHalf4x3 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domHalf4x3));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domHalf4x4::create(DAE& dae)
{
	domCg_param_type::domHalf4x4Ref ref = new domCg_param_type::domHalf4x4(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domHalf4x4::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "half4x4" );
	meta->registerClass(domCg_param_type::domHalf4x4::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_half4x4"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domHalf4x4 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domHalf4x4));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domFixed::create(DAE& dae)
{
	domCg_param_type::domFixedRef ref = new domCg_param_type::domFixed(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domFixed::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "fixed" );
	meta->registerClass(domCg_param_type::domFixed::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_fixed"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domFixed , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domFixed));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domFixed1::create(DAE& dae)
{
	domCg_param_type::domFixed1Ref ref = new domCg_param_type::domFixed1(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domFixed1::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "fixed1" );
	meta->registerClass(domCg_param_type::domFixed1::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_fixed1"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domFixed1 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domFixed1));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domFixed2::create(DAE& dae)
{
	domCg_param_type::domFixed2Ref ref = new domCg_param_type::domFixed2(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domFixed2::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "fixed2" );
	meta->registerClass(domCg_param_type::domFixed2::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_fixed2"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domFixed2 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domFixed2));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domFixed3::create(DAE& dae)
{
	domCg_param_type::domFixed3Ref ref = new domCg_param_type::domFixed3(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domFixed3::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "fixed3" );
	meta->registerClass(domCg_param_type::domFixed3::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_fixed3"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domFixed3 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domFixed3));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domFixed4::create(DAE& dae)
{
	domCg_param_type::domFixed4Ref ref = new domCg_param_type::domFixed4(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domFixed4::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "fixed4" );
	meta->registerClass(domCg_param_type::domFixed4::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_fixed4"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domFixed4 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domFixed4));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domFixed1x1::create(DAE& dae)
{
	domCg_param_type::domFixed1x1Ref ref = new domCg_param_type::domFixed1x1(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domFixed1x1::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "fixed1x1" );
	meta->registerClass(domCg_param_type::domFixed1x1::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_fixed1x1"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domFixed1x1 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domFixed1x1));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domFixed1x2::create(DAE& dae)
{
	domCg_param_type::domFixed1x2Ref ref = new domCg_param_type::domFixed1x2(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domFixed1x2::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "fixed1x2" );
	meta->registerClass(domCg_param_type::domFixed1x2::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_fixed1x2"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domFixed1x2 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domFixed1x2));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domFixed1x3::create(DAE& dae)
{
	domCg_param_type::domFixed1x3Ref ref = new domCg_param_type::domFixed1x3(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domFixed1x3::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "fixed1x3" );
	meta->registerClass(domCg_param_type::domFixed1x3::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_fixed1x3"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domFixed1x3 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domFixed1x3));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domFixed1x4::create(DAE& dae)
{
	domCg_param_type::domFixed1x4Ref ref = new domCg_param_type::domFixed1x4(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domFixed1x4::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "fixed1x4" );
	meta->registerClass(domCg_param_type::domFixed1x4::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_fixed1x4"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domFixed1x4 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domFixed1x4));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domFixed2x1::create(DAE& dae)
{
	domCg_param_type::domFixed2x1Ref ref = new domCg_param_type::domFixed2x1(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domFixed2x1::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "fixed2x1" );
	meta->registerClass(domCg_param_type::domFixed2x1::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_fixed2x1"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domFixed2x1 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domFixed2x1));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domFixed2x2::create(DAE& dae)
{
	domCg_param_type::domFixed2x2Ref ref = new domCg_param_type::domFixed2x2(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domFixed2x2::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "fixed2x2" );
	meta->registerClass(domCg_param_type::domFixed2x2::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_fixed2x2"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domFixed2x2 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domFixed2x2));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domFixed2x3::create(DAE& dae)
{
	domCg_param_type::domFixed2x3Ref ref = new domCg_param_type::domFixed2x3(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domFixed2x3::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "fixed2x3" );
	meta->registerClass(domCg_param_type::domFixed2x3::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_fixed2x3"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domFixed2x3 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domFixed2x3));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domFixed2x4::create(DAE& dae)
{
	domCg_param_type::domFixed2x4Ref ref = new domCg_param_type::domFixed2x4(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domFixed2x4::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "fixed2x4" );
	meta->registerClass(domCg_param_type::domFixed2x4::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_fixed2x4"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domFixed2x4 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domFixed2x4));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domFixed3x1::create(DAE& dae)
{
	domCg_param_type::domFixed3x1Ref ref = new domCg_param_type::domFixed3x1(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domFixed3x1::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "fixed3x1" );
	meta->registerClass(domCg_param_type::domFixed3x1::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_fixed3x1"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domFixed3x1 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domFixed3x1));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domFixed3x2::create(DAE& dae)
{
	domCg_param_type::domFixed3x2Ref ref = new domCg_param_type::domFixed3x2(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domFixed3x2::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "fixed3x2" );
	meta->registerClass(domCg_param_type::domFixed3x2::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_fixed3x2"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domFixed3x2 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domFixed3x2));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domFixed3x3::create(DAE& dae)
{
	domCg_param_type::domFixed3x3Ref ref = new domCg_param_type::domFixed3x3(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domFixed3x3::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "fixed3x3" );
	meta->registerClass(domCg_param_type::domFixed3x3::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_fixed3x3"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domFixed3x3 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domFixed3x3));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domFixed3x4::create(DAE& dae)
{
	domCg_param_type::domFixed3x4Ref ref = new domCg_param_type::domFixed3x4(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domFixed3x4::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "fixed3x4" );
	meta->registerClass(domCg_param_type::domFixed3x4::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_fixed3x4"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domFixed3x4 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domFixed3x4));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domFixed4x1::create(DAE& dae)
{
	domCg_param_type::domFixed4x1Ref ref = new domCg_param_type::domFixed4x1(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domFixed4x1::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "fixed4x1" );
	meta->registerClass(domCg_param_type::domFixed4x1::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_fixed4x1"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domFixed4x1 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domFixed4x1));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domFixed4x2::create(DAE& dae)
{
	domCg_param_type::domFixed4x2Ref ref = new domCg_param_type::domFixed4x2(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domFixed4x2::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "fixed4x2" );
	meta->registerClass(domCg_param_type::domFixed4x2::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_fixed4x2"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domFixed4x2 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domFixed4x2));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domFixed4x3::create(DAE& dae)
{
	domCg_param_type::domFixed4x3Ref ref = new domCg_param_type::domFixed4x3(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domFixed4x3::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "fixed4x3" );
	meta->registerClass(domCg_param_type::domFixed4x3::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_fixed4x3"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domFixed4x3 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domFixed4x3));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domFixed4x4::create(DAE& dae)
{
	domCg_param_type::domFixed4x4Ref ref = new domCg_param_type::domFixed4x4(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domFixed4x4::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "fixed4x4" );
	meta->registerClass(domCg_param_type::domFixed4x4::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Cg_fixed4x4"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domFixed4x4 , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domFixed4x4));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domString::create(DAE& dae)
{
	domCg_param_type::domStringRef ref = new domCg_param_type::domString(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domString::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "string" );
	meta->registerClass(domCg_param_type::domString::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("xsString"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domString , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domString));
	meta->validate();

	return meta;
}

daeElementRef
domCg_param_type::domEnum::create(DAE& dae)
{
	domCg_param_type::domEnumRef ref = new domCg_param_type::domEnum(dae);
	return ref;
}


daeMetaElement *
domCg_param_type::domEnum::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "enum" );
	meta->registerClass(domCg_param_type::domEnum::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Gl_enumeration"));
		ma->setOffset( daeOffsetOf( domCg_param_type::domEnum , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCg_param_type::domEnum));
	meta->validate();

	return meta;
}

