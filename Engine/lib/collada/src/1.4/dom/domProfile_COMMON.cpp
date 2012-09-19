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
#include <dom/domProfile_COMMON.h>
#include <dae/daeMetaCMPolicy.h>
#include <dae/daeMetaSequence.h>
#include <dae/daeMetaChoice.h>
#include <dae/daeMetaGroup.h>
#include <dae/daeMetaAny.h>
#include <dae/daeMetaElementAttribute.h>

daeElementRef
domProfile_COMMON::create(DAE& dae)
{
	domProfile_COMMONRef ref = new domProfile_COMMON(dae);
	return ref;
}


daeMetaElement *
domProfile_COMMON::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "profile_COMMON" );
	meta->registerClass(domProfile_COMMON::create);

	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 0, 1 );
	mea->setName( "asset" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON,elemAsset) );
	mea->setElementType( domAsset::registerElement(dae) );
	cm->appendChild( mea );

	cm = new daeMetaChoice( meta, cm, 0, 1, 0, -1 );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "image" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON,elemImage_array) );
	mea->setElementType( domImage::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "newparam" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON,elemNewparam_array) );
	mea->setElementType( domCommon_newparam_type::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 0 );
	cm->getParent()->appendChild( cm );
	cm = cm->getParent();

	mea = new daeMetaElementAttribute( meta, cm, 3002, 1, 1 );
	mea->setName( "technique" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON,elemTechnique) );
	mea->setElementType( domProfile_COMMON::domTechnique::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 3003, 0, -1 );
	mea->setName( "extra" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON,elemExtra_array) );
	mea->setElementType( domExtra::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 3003 );
	meta->setCMRoot( cm );	
	// Ordered list of sub-elements
	meta->addContents(daeOffsetOf(domProfile_COMMON,_contents));
	meta->addContentsOrder(daeOffsetOf(domProfile_COMMON,_contentsOrder));

	meta->addCMDataArray(daeOffsetOf(domProfile_COMMON,_CMData), 1);
	//	Add attribute: id
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "id" );
		ma->setType( dae.getAtomicTypes().get("xsID"));
		ma->setOffset( daeOffsetOf( domProfile_COMMON , attrId ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domProfile_COMMON));
	meta->validate();

	return meta;
}

daeElementRef
domProfile_COMMON::domTechnique::create(DAE& dae)
{
	domProfile_COMMON::domTechniqueRef ref = new domProfile_COMMON::domTechnique(dae);
	return ref;
}


daeMetaElement *
domProfile_COMMON::domTechnique::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "technique" );
	meta->registerClass(domProfile_COMMON::domTechnique::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 0, 1 );
	mea->setName( "asset" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON::domTechnique,elemAsset) );
	mea->setElementType( domAsset::registerElement(dae) );
	cm->appendChild( mea );

	cm = new daeMetaChoice( meta, cm, 0, 1, 0, -1 );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "image" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON::domTechnique,elemImage_array) );
	mea->setElementType( domImage::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "newparam" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON::domTechnique,elemNewparam_array) );
	mea->setElementType( domCommon_newparam_type::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 0 );
	cm->getParent()->appendChild( cm );
	cm = cm->getParent();

	cm = new daeMetaChoice( meta, cm, 1, 3002, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "constant" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON::domTechnique,elemConstant) );
	mea->setElementType( domProfile_COMMON::domTechnique::domConstant::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "lambert" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON::domTechnique,elemLambert) );
	mea->setElementType( domProfile_COMMON::domTechnique::domLambert::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "phong" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON::domTechnique,elemPhong) );
	mea->setElementType( domProfile_COMMON::domTechnique::domPhong::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "blinn" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON::domTechnique,elemBlinn) );
	mea->setElementType( domProfile_COMMON::domTechnique::domBlinn::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 0 );
	cm->getParent()->appendChild( cm );
	cm = cm->getParent();

	mea = new daeMetaElementArrayAttribute( meta, cm, 3003, 0, -1 );
	mea->setName( "extra" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON::domTechnique,elemExtra_array) );
	mea->setElementType( domExtra::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 3003 );
	meta->setCMRoot( cm );	
	// Ordered list of sub-elements
	meta->addContents(daeOffsetOf(domProfile_COMMON::domTechnique,_contents));
	meta->addContentsOrder(daeOffsetOf(domProfile_COMMON::domTechnique,_contentsOrder));

	meta->addCMDataArray(daeOffsetOf(domProfile_COMMON::domTechnique,_CMData), 2);
	//	Add attribute: id
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "id" );
		ma->setType( dae.getAtomicTypes().get("xsID"));
		ma->setOffset( daeOffsetOf( domProfile_COMMON::domTechnique , attrId ));
		ma->setContainer( meta );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: sid
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "sid" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domProfile_COMMON::domTechnique , attrSid ));
		ma->setContainer( meta );
		ma->setIsRequired( true );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domProfile_COMMON::domTechnique));
	meta->validate();

	return meta;
}

daeElementRef
domProfile_COMMON::domTechnique::domConstant::create(DAE& dae)
{
	domProfile_COMMON::domTechnique::domConstantRef ref = new domProfile_COMMON::domTechnique::domConstant(dae);
	return ref;
}


daeMetaElement *
domProfile_COMMON::domTechnique::domConstant::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "constant" );
	meta->registerClass(domProfile_COMMON::domTechnique::domConstant::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 0, 1 );
	mea->setName( "emission" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON::domTechnique::domConstant,elemEmission) );
	mea->setElementType( domCommon_color_or_texture_type::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 1, 0, 1 );
	mea->setName( "reflective" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON::domTechnique::domConstant,elemReflective) );
	mea->setElementType( domCommon_color_or_texture_type::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 2, 0, 1 );
	mea->setName( "reflectivity" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON::domTechnique::domConstant,elemReflectivity) );
	mea->setElementType( domCommon_float_or_param_type::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 3, 0, 1 );
	mea->setName( "transparent" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON::domTechnique::domConstant,elemTransparent) );
	mea->setElementType( domCommon_transparent_type::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 4, 0, 1 );
	mea->setName( "transparency" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON::domTechnique::domConstant,elemTransparency) );
	mea->setElementType( domCommon_float_or_param_type::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 5, 0, 1 );
	mea->setName( "index_of_refraction" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON::domTechnique::domConstant,elemIndex_of_refraction) );
	mea->setElementType( domCommon_float_or_param_type::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 5 );
	meta->setCMRoot( cm );	

	meta->setElementSize(sizeof(domProfile_COMMON::domTechnique::domConstant));
	meta->validate();

	return meta;
}

daeElementRef
domProfile_COMMON::domTechnique::domLambert::create(DAE& dae)
{
	domProfile_COMMON::domTechnique::domLambertRef ref = new domProfile_COMMON::domTechnique::domLambert(dae);
	return ref;
}


daeMetaElement *
domProfile_COMMON::domTechnique::domLambert::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "lambert" );
	meta->registerClass(domProfile_COMMON::domTechnique::domLambert::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 0, 1 );
	mea->setName( "emission" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON::domTechnique::domLambert,elemEmission) );
	mea->setElementType( domCommon_color_or_texture_type::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 1, 0, 1 );
	mea->setName( "ambient" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON::domTechnique::domLambert,elemAmbient) );
	mea->setElementType( domCommon_color_or_texture_type::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 2, 0, 1 );
	mea->setName( "diffuse" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON::domTechnique::domLambert,elemDiffuse) );
	mea->setElementType( domCommon_color_or_texture_type::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 3, 0, 1 );
	mea->setName( "reflective" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON::domTechnique::domLambert,elemReflective) );
	mea->setElementType( domCommon_color_or_texture_type::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 4, 0, 1 );
	mea->setName( "reflectivity" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON::domTechnique::domLambert,elemReflectivity) );
	mea->setElementType( domCommon_float_or_param_type::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 5, 0, 1 );
	mea->setName( "transparent" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON::domTechnique::domLambert,elemTransparent) );
	mea->setElementType( domCommon_transparent_type::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 6, 0, 1 );
	mea->setName( "transparency" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON::domTechnique::domLambert,elemTransparency) );
	mea->setElementType( domCommon_float_or_param_type::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 7, 0, 1 );
	mea->setName( "index_of_refraction" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON::domTechnique::domLambert,elemIndex_of_refraction) );
	mea->setElementType( domCommon_float_or_param_type::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 7 );
	meta->setCMRoot( cm );	

	meta->setElementSize(sizeof(domProfile_COMMON::domTechnique::domLambert));
	meta->validate();

	return meta;
}

daeElementRef
domProfile_COMMON::domTechnique::domPhong::create(DAE& dae)
{
	domProfile_COMMON::domTechnique::domPhongRef ref = new domProfile_COMMON::domTechnique::domPhong(dae);
	return ref;
}


daeMetaElement *
domProfile_COMMON::domTechnique::domPhong::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "phong" );
	meta->registerClass(domProfile_COMMON::domTechnique::domPhong::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 0, 1 );
	mea->setName( "emission" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON::domTechnique::domPhong,elemEmission) );
	mea->setElementType( domCommon_color_or_texture_type::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 1, 0, 1 );
	mea->setName( "ambient" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON::domTechnique::domPhong,elemAmbient) );
	mea->setElementType( domCommon_color_or_texture_type::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 2, 0, 1 );
	mea->setName( "diffuse" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON::domTechnique::domPhong,elemDiffuse) );
	mea->setElementType( domCommon_color_or_texture_type::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 3, 0, 1 );
	mea->setName( "specular" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON::domTechnique::domPhong,elemSpecular) );
	mea->setElementType( domCommon_color_or_texture_type::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 4, 0, 1 );
	mea->setName( "shininess" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON::domTechnique::domPhong,elemShininess) );
	mea->setElementType( domCommon_float_or_param_type::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 5, 0, 1 );
	mea->setName( "reflective" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON::domTechnique::domPhong,elemReflective) );
	mea->setElementType( domCommon_color_or_texture_type::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 6, 0, 1 );
	mea->setName( "reflectivity" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON::domTechnique::domPhong,elemReflectivity) );
	mea->setElementType( domCommon_float_or_param_type::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 7, 0, 1 );
	mea->setName( "transparent" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON::domTechnique::domPhong,elemTransparent) );
	mea->setElementType( domCommon_transparent_type::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 8, 0, 1 );
	mea->setName( "transparency" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON::domTechnique::domPhong,elemTransparency) );
	mea->setElementType( domCommon_float_or_param_type::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 9, 0, 1 );
	mea->setName( "index_of_refraction" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON::domTechnique::domPhong,elemIndex_of_refraction) );
	mea->setElementType( domCommon_float_or_param_type::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 9 );
	meta->setCMRoot( cm );	

	meta->setElementSize(sizeof(domProfile_COMMON::domTechnique::domPhong));
	meta->validate();

	return meta;
}

daeElementRef
domProfile_COMMON::domTechnique::domBlinn::create(DAE& dae)
{
	domProfile_COMMON::domTechnique::domBlinnRef ref = new domProfile_COMMON::domTechnique::domBlinn(dae);
	return ref;
}


daeMetaElement *
domProfile_COMMON::domTechnique::domBlinn::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "blinn" );
	meta->registerClass(domProfile_COMMON::domTechnique::domBlinn::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 0, 1 );
	mea->setName( "emission" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON::domTechnique::domBlinn,elemEmission) );
	mea->setElementType( domCommon_color_or_texture_type::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 1, 0, 1 );
	mea->setName( "ambient" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON::domTechnique::domBlinn,elemAmbient) );
	mea->setElementType( domCommon_color_or_texture_type::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 2, 0, 1 );
	mea->setName( "diffuse" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON::domTechnique::domBlinn,elemDiffuse) );
	mea->setElementType( domCommon_color_or_texture_type::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 3, 0, 1 );
	mea->setName( "specular" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON::domTechnique::domBlinn,elemSpecular) );
	mea->setElementType( domCommon_color_or_texture_type::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 4, 0, 1 );
	mea->setName( "shininess" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON::domTechnique::domBlinn,elemShininess) );
	mea->setElementType( domCommon_float_or_param_type::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 5, 0, 1 );
	mea->setName( "reflective" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON::domTechnique::domBlinn,elemReflective) );
	mea->setElementType( domCommon_color_or_texture_type::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 6, 0, 1 );
	mea->setName( "reflectivity" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON::domTechnique::domBlinn,elemReflectivity) );
	mea->setElementType( domCommon_float_or_param_type::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 7, 0, 1 );
	mea->setName( "transparent" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON::domTechnique::domBlinn,elemTransparent) );
	mea->setElementType( domCommon_transparent_type::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 8, 0, 1 );
	mea->setName( "transparency" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON::domTechnique::domBlinn,elemTransparency) );
	mea->setElementType( domCommon_float_or_param_type::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 9, 0, 1 );
	mea->setName( "index_of_refraction" );
	mea->setOffset( daeOffsetOf(domProfile_COMMON::domTechnique::domBlinn,elemIndex_of_refraction) );
	mea->setElementType( domCommon_float_or_param_type::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 9 );
	meta->setCMRoot( cm );	

	meta->setElementSize(sizeof(domProfile_COMMON::domTechnique::domBlinn));
	meta->validate();

	return meta;
}

