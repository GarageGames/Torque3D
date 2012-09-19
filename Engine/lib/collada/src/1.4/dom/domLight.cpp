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
#include <dom/domLight.h>
#include <dae/daeMetaCMPolicy.h>
#include <dae/daeMetaSequence.h>
#include <dae/daeMetaChoice.h>
#include <dae/daeMetaGroup.h>
#include <dae/daeMetaAny.h>
#include <dae/daeMetaElementAttribute.h>

daeElementRef
domLight::create(DAE& dae)
{
	domLightRef ref = new domLight(dae);
	return ref;
}


daeMetaElement *
domLight::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "light" );
	meta->registerClass(domLight::create);

	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 0, 1 );
	mea->setName( "asset" );
	mea->setOffset( daeOffsetOf(domLight,elemAsset) );
	mea->setElementType( domAsset::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 1, 1, 1 );
	mea->setName( "technique_common" );
	mea->setOffset( daeOffsetOf(domLight,elemTechnique_common) );
	mea->setElementType( domLight::domTechnique_common::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 2, 0, -1 );
	mea->setName( "technique" );
	mea->setOffset( daeOffsetOf(domLight,elemTechnique_array) );
	mea->setElementType( domTechnique::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 3, 0, -1 );
	mea->setName( "extra" );
	mea->setOffset( daeOffsetOf(domLight,elemExtra_array) );
	mea->setElementType( domExtra::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 3 );
	meta->setCMRoot( cm );	

	//	Add attribute: id
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "id" );
		ma->setType( dae.getAtomicTypes().get("xsID"));
		ma->setOffset( daeOffsetOf( domLight , attrId ));
		ma->setContainer( meta );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: name
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "name" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domLight , attrName ));
		ma->setContainer( meta );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domLight));
	meta->validate();

	return meta;
}

daeElementRef
domLight::domTechnique_common::create(DAE& dae)
{
	domLight::domTechnique_commonRef ref = new domLight::domTechnique_common(dae);
	return ref;
}


daeMetaElement *
domLight::domTechnique_common::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "technique_common" );
	meta->registerClass(domLight::domTechnique_common::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaChoice( meta, cm, 0, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "ambient" );
	mea->setOffset( daeOffsetOf(domLight::domTechnique_common,elemAmbient) );
	mea->setElementType( domLight::domTechnique_common::domAmbient::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "directional" );
	mea->setOffset( daeOffsetOf(domLight::domTechnique_common,elemDirectional) );
	mea->setElementType( domLight::domTechnique_common::domDirectional::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "point" );
	mea->setOffset( daeOffsetOf(domLight::domTechnique_common,elemPoint) );
	mea->setElementType( domLight::domTechnique_common::domPoint::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "spot" );
	mea->setOffset( daeOffsetOf(domLight::domTechnique_common,elemSpot) );
	mea->setElementType( domLight::domTechnique_common::domSpot::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 0 );
	meta->setCMRoot( cm );	
	// Ordered list of sub-elements
	meta->addContents(daeOffsetOf(domLight::domTechnique_common,_contents));
	meta->addContentsOrder(daeOffsetOf(domLight::domTechnique_common,_contentsOrder));

	meta->addCMDataArray(daeOffsetOf(domLight::domTechnique_common,_CMData), 1);
	meta->setElementSize(sizeof(domLight::domTechnique_common));
	meta->validate();

	return meta;
}

daeElementRef
domLight::domTechnique_common::domAmbient::create(DAE& dae)
{
	domLight::domTechnique_common::domAmbientRef ref = new domLight::domTechnique_common::domAmbient(dae);
	return ref;
}


daeMetaElement *
domLight::domTechnique_common::domAmbient::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "ambient" );
	meta->registerClass(domLight::domTechnique_common::domAmbient::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "color" );
	mea->setOffset( daeOffsetOf(domLight::domTechnique_common::domAmbient,elemColor) );
	mea->setElementType( domTargetableFloat3::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 0 );
	meta->setCMRoot( cm );	

	meta->setElementSize(sizeof(domLight::domTechnique_common::domAmbient));
	meta->validate();

	return meta;
}

daeElementRef
domLight::domTechnique_common::domDirectional::create(DAE& dae)
{
	domLight::domTechnique_common::domDirectionalRef ref = new domLight::domTechnique_common::domDirectional(dae);
	return ref;
}


daeMetaElement *
domLight::domTechnique_common::domDirectional::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "directional" );
	meta->registerClass(domLight::domTechnique_common::domDirectional::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "color" );
	mea->setOffset( daeOffsetOf(domLight::domTechnique_common::domDirectional,elemColor) );
	mea->setElementType( domTargetableFloat3::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 0 );
	meta->setCMRoot( cm );	

	meta->setElementSize(sizeof(domLight::domTechnique_common::domDirectional));
	meta->validate();

	return meta;
}

daeElementRef
domLight::domTechnique_common::domPoint::create(DAE& dae)
{
	domLight::domTechnique_common::domPointRef ref = new domLight::domTechnique_common::domPoint(dae);
	return ref;
}


daeMetaElement *
domLight::domTechnique_common::domPoint::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "point" );
	meta->registerClass(domLight::domTechnique_common::domPoint::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "color" );
	mea->setOffset( daeOffsetOf(domLight::domTechnique_common::domPoint,elemColor) );
	mea->setElementType( domTargetableFloat3::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 1, 0, 1 );
	mea->setName( "constant_attenuation" );
	mea->setOffset( daeOffsetOf(domLight::domTechnique_common::domPoint,elemConstant_attenuation) );
	mea->setElementType( domTargetableFloat::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 2, 0, 1 );
	mea->setName( "linear_attenuation" );
	mea->setOffset( daeOffsetOf(domLight::domTechnique_common::domPoint,elemLinear_attenuation) );
	mea->setElementType( domTargetableFloat::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 3, 0, 1 );
	mea->setName( "quadratic_attenuation" );
	mea->setOffset( daeOffsetOf(domLight::domTechnique_common::domPoint,elemQuadratic_attenuation) );
	mea->setElementType( domTargetableFloat::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 3 );
	meta->setCMRoot( cm );	

	meta->setElementSize(sizeof(domLight::domTechnique_common::domPoint));
	meta->validate();

	return meta;
}

daeElementRef
domLight::domTechnique_common::domSpot::create(DAE& dae)
{
	domLight::domTechnique_common::domSpotRef ref = new domLight::domTechnique_common::domSpot(dae);
	return ref;
}


daeMetaElement *
domLight::domTechnique_common::domSpot::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "spot" );
	meta->registerClass(domLight::domTechnique_common::domSpot::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "color" );
	mea->setOffset( daeOffsetOf(domLight::domTechnique_common::domSpot,elemColor) );
	mea->setElementType( domTargetableFloat3::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 1, 0, 1 );
	mea->setName( "constant_attenuation" );
	mea->setOffset( daeOffsetOf(domLight::domTechnique_common::domSpot,elemConstant_attenuation) );
	mea->setElementType( domTargetableFloat::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 2, 0, 1 );
	mea->setName( "linear_attenuation" );
	mea->setOffset( daeOffsetOf(domLight::domTechnique_common::domSpot,elemLinear_attenuation) );
	mea->setElementType( domTargetableFloat::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 3, 0, 1 );
	mea->setName( "quadratic_attenuation" );
	mea->setOffset( daeOffsetOf(domLight::domTechnique_common::domSpot,elemQuadratic_attenuation) );
	mea->setElementType( domTargetableFloat::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 4, 0, 1 );
	mea->setName( "falloff_angle" );
	mea->setOffset( daeOffsetOf(domLight::domTechnique_common::domSpot,elemFalloff_angle) );
	mea->setElementType( domTargetableFloat::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 5, 0, 1 );
	mea->setName( "falloff_exponent" );
	mea->setOffset( daeOffsetOf(domLight::domTechnique_common::domSpot,elemFalloff_exponent) );
	mea->setElementType( domTargetableFloat::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 5 );
	meta->setCMRoot( cm );	

	meta->setElementSize(sizeof(domLight::domTechnique_common::domSpot));
	meta->validate();

	return meta;
}

