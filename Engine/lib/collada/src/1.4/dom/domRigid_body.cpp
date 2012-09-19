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
#include <dom/domRigid_body.h>
#include <dae/daeMetaCMPolicy.h>
#include <dae/daeMetaSequence.h>
#include <dae/daeMetaChoice.h>
#include <dae/daeMetaGroup.h>
#include <dae/daeMetaAny.h>
#include <dae/daeMetaElementAttribute.h>

daeElementRef
domRigid_body::create(DAE& dae)
{
	domRigid_bodyRef ref = new domRigid_body(dae);
	return ref;
}


daeMetaElement *
domRigid_body::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "rigid_body" );
	meta->registerClass(domRigid_body::create);

	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "technique_common" );
	mea->setOffset( daeOffsetOf(domRigid_body,elemTechnique_common) );
	mea->setElementType( domRigid_body::domTechnique_common::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 1, 0, -1 );
	mea->setName( "technique" );
	mea->setOffset( daeOffsetOf(domRigid_body,elemTechnique_array) );
	mea->setElementType( domTechnique::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 2, 0, -1 );
	mea->setName( "extra" );
	mea->setOffset( daeOffsetOf(domRigid_body,elemExtra_array) );
	mea->setElementType( domExtra::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 2 );
	meta->setCMRoot( cm );	

	//	Add attribute: sid
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "sid" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domRigid_body , attrSid ));
		ma->setContainer( meta );
		ma->setIsRequired( true );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: name
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "name" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domRigid_body , attrName ));
		ma->setContainer( meta );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domRigid_body));
	meta->validate();

	return meta;
}

daeElementRef
domRigid_body::domTechnique_common::create(DAE& dae)
{
	domRigid_body::domTechnique_commonRef ref = new domRigid_body::domTechnique_common(dae);
	return ref;
}


daeMetaElement *
domRigid_body::domTechnique_common::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "technique_common" );
	meta->registerClass(domRigid_body::domTechnique_common::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 0, 1 );
	mea->setName( "dynamic" );
	mea->setOffset( daeOffsetOf(domRigid_body::domTechnique_common,elemDynamic) );
	mea->setElementType( domRigid_body::domTechnique_common::domDynamic::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 1, 0, 1 );
	mea->setName( "mass" );
	mea->setOffset( daeOffsetOf(domRigid_body::domTechnique_common,elemMass) );
	mea->setElementType( domTargetableFloat::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 2, 0, 1 );
	mea->setName( "mass_frame" );
	mea->setOffset( daeOffsetOf(domRigid_body::domTechnique_common,elemMass_frame) );
	mea->setElementType( domRigid_body::domTechnique_common::domMass_frame::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 3, 0, 1 );
	mea->setName( "inertia" );
	mea->setOffset( daeOffsetOf(domRigid_body::domTechnique_common,elemInertia) );
	mea->setElementType( domTargetableFloat3::registerElement(dae) );
	cm->appendChild( mea );

	cm = new daeMetaChoice( meta, cm, 0, 4, 0, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "instance_physics_material" );
	mea->setOffset( daeOffsetOf(domRigid_body::domTechnique_common,elemInstance_physics_material) );
	mea->setElementType( domInstance_physics_material::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "physics_material" );
	mea->setOffset( daeOffsetOf(domRigid_body::domTechnique_common,elemPhysics_material) );
	mea->setElementType( domPhysics_material::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 0 );
	cm->getParent()->appendChild( cm );
	cm = cm->getParent();

	mea = new daeMetaElementArrayAttribute( meta, cm, 5, 1, -1 );
	mea->setName( "shape" );
	mea->setOffset( daeOffsetOf(domRigid_body::domTechnique_common,elemShape_array) );
	mea->setElementType( domRigid_body::domTechnique_common::domShape::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 5 );
	meta->setCMRoot( cm );	
	// Ordered list of sub-elements
	meta->addContents(daeOffsetOf(domRigid_body::domTechnique_common,_contents));
	meta->addContentsOrder(daeOffsetOf(domRigid_body::domTechnique_common,_contentsOrder));

	meta->addCMDataArray(daeOffsetOf(domRigid_body::domTechnique_common,_CMData), 1);
	meta->setElementSize(sizeof(domRigid_body::domTechnique_common));
	meta->validate();

	return meta;
}

daeElementRef
domRigid_body::domTechnique_common::domDynamic::create(DAE& dae)
{
	domRigid_body::domTechnique_common::domDynamicRef ref = new domRigid_body::domTechnique_common::domDynamic(dae);
	return ref;
}


daeMetaElement *
domRigid_body::domTechnique_common::domDynamic::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "dynamic" );
	meta->registerClass(domRigid_body::domTechnique_common::domDynamic::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Bool"));
		ma->setOffset( daeOffsetOf( domRigid_body::domTechnique_common::domDynamic , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	//	Add attribute: sid
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "sid" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domRigid_body::domTechnique_common::domDynamic , attrSid ));
		ma->setContainer( meta );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domRigid_body::domTechnique_common::domDynamic));
	meta->validate();

	return meta;
}

daeElementRef
domRigid_body::domTechnique_common::domMass_frame::create(DAE& dae)
{
	domRigid_body::domTechnique_common::domMass_frameRef ref = new domRigid_body::domTechnique_common::domMass_frame(dae);
	return ref;
}


daeMetaElement *
domRigid_body::domTechnique_common::domMass_frame::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "mass_frame" );
	meta->registerClass(domRigid_body::domTechnique_common::domMass_frame::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaChoice( meta, cm, 0, 0, 1, -1 );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "translate" );
	mea->setOffset( daeOffsetOf(domRigid_body::domTechnique_common::domMass_frame,elemTranslate_array) );
	mea->setElementType( domTranslate::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "rotate" );
	mea->setOffset( daeOffsetOf(domRigid_body::domTechnique_common::domMass_frame,elemRotate_array) );
	mea->setElementType( domRotate::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 3000 );
	meta->setCMRoot( cm );	
	// Ordered list of sub-elements
	meta->addContents(daeOffsetOf(domRigid_body::domTechnique_common::domMass_frame,_contents));
	meta->addContentsOrder(daeOffsetOf(domRigid_body::domTechnique_common::domMass_frame,_contentsOrder));

	meta->addCMDataArray(daeOffsetOf(domRigid_body::domTechnique_common::domMass_frame,_CMData), 1);
	meta->setElementSize(sizeof(domRigid_body::domTechnique_common::domMass_frame));
	meta->validate();

	return meta;
}

daeElementRef
domRigid_body::domTechnique_common::domShape::create(DAE& dae)
{
	domRigid_body::domTechnique_common::domShapeRef ref = new domRigid_body::domTechnique_common::domShape(dae);
	return ref;
}


daeMetaElement *
domRigid_body::domTechnique_common::domShape::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "shape" );
	meta->registerClass(domRigid_body::domTechnique_common::domShape::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 0, 1 );
	mea->setName( "hollow" );
	mea->setOffset( daeOffsetOf(domRigid_body::domTechnique_common::domShape,elemHollow) );
	mea->setElementType( domRigid_body::domTechnique_common::domShape::domHollow::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 1, 0, 1 );
	mea->setName( "mass" );
	mea->setOffset( daeOffsetOf(domRigid_body::domTechnique_common::domShape,elemMass) );
	mea->setElementType( domTargetableFloat::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 2, 0, 1 );
	mea->setName( "density" );
	mea->setOffset( daeOffsetOf(domRigid_body::domTechnique_common::domShape,elemDensity) );
	mea->setElementType( domTargetableFloat::registerElement(dae) );
	cm->appendChild( mea );

	cm = new daeMetaChoice( meta, cm, 0, 3, 0, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "instance_physics_material" );
	mea->setOffset( daeOffsetOf(domRigid_body::domTechnique_common::domShape,elemInstance_physics_material) );
	mea->setElementType( domInstance_physics_material::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "physics_material" );
	mea->setOffset( daeOffsetOf(domRigid_body::domTechnique_common::domShape,elemPhysics_material) );
	mea->setElementType( domPhysics_material::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 0 );
	cm->getParent()->appendChild( cm );
	cm = cm->getParent();

	cm = new daeMetaChoice( meta, cm, 1, 4, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "instance_geometry" );
	mea->setOffset( daeOffsetOf(domRigid_body::domTechnique_common::domShape,elemInstance_geometry) );
	mea->setElementType( domInstance_geometry::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "plane" );
	mea->setOffset( daeOffsetOf(domRigid_body::domTechnique_common::domShape,elemPlane) );
	mea->setElementType( domPlane::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "box" );
	mea->setOffset( daeOffsetOf(domRigid_body::domTechnique_common::domShape,elemBox) );
	mea->setElementType( domBox::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "sphere" );
	mea->setOffset( daeOffsetOf(domRigid_body::domTechnique_common::domShape,elemSphere) );
	mea->setElementType( domSphere::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "cylinder" );
	mea->setOffset( daeOffsetOf(domRigid_body::domTechnique_common::domShape,elemCylinder) );
	mea->setElementType( domCylinder::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "tapered_cylinder" );
	mea->setOffset( daeOffsetOf(domRigid_body::domTechnique_common::domShape,elemTapered_cylinder) );
	mea->setElementType( domTapered_cylinder::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "capsule" );
	mea->setOffset( daeOffsetOf(domRigid_body::domTechnique_common::domShape,elemCapsule) );
	mea->setElementType( domCapsule::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "tapered_capsule" );
	mea->setOffset( daeOffsetOf(domRigid_body::domTechnique_common::domShape,elemTapered_capsule) );
	mea->setElementType( domTapered_capsule::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 0 );
	cm->getParent()->appendChild( cm );
	cm = cm->getParent();

	cm = new daeMetaChoice( meta, cm, 2, 5, 0, -1 );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "translate" );
	mea->setOffset( daeOffsetOf(domRigid_body::domTechnique_common::domShape,elemTranslate_array) );
	mea->setElementType( domTranslate::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "rotate" );
	mea->setOffset( daeOffsetOf(domRigid_body::domTechnique_common::domShape,elemRotate_array) );
	mea->setElementType( domRotate::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 0 );
	cm->getParent()->appendChild( cm );
	cm = cm->getParent();

	mea = new daeMetaElementArrayAttribute( meta, cm, 3006, 0, -1 );
	mea->setName( "extra" );
	mea->setOffset( daeOffsetOf(domRigid_body::domTechnique_common::domShape,elemExtra_array) );
	mea->setElementType( domExtra::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 3006 );
	meta->setCMRoot( cm );	
	// Ordered list of sub-elements
	meta->addContents(daeOffsetOf(domRigid_body::domTechnique_common::domShape,_contents));
	meta->addContentsOrder(daeOffsetOf(domRigid_body::domTechnique_common::domShape,_contentsOrder));

	meta->addCMDataArray(daeOffsetOf(domRigid_body::domTechnique_common::domShape,_CMData), 3);
	meta->setElementSize(sizeof(domRigid_body::domTechnique_common::domShape));
	meta->validate();

	return meta;
}

daeElementRef
domRigid_body::domTechnique_common::domShape::domHollow::create(DAE& dae)
{
	domRigid_body::domTechnique_common::domShape::domHollowRef ref = new domRigid_body::domTechnique_common::domShape::domHollow(dae);
	return ref;
}


daeMetaElement *
domRigid_body::domTechnique_common::domShape::domHollow::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "hollow" );
	meta->registerClass(domRigid_body::domTechnique_common::domShape::domHollow::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Bool"));
		ma->setOffset( daeOffsetOf( domRigid_body::domTechnique_common::domShape::domHollow , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	//	Add attribute: sid
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "sid" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domRigid_body::domTechnique_common::domShape::domHollow , attrSid ));
		ma->setContainer( meta );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domRigid_body::domTechnique_common::domShape::domHollow));
	meta->validate();

	return meta;
}

