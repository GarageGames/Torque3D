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
#include <dom/domRigid_constraint.h>
#include <dae/daeMetaCMPolicy.h>
#include <dae/daeMetaSequence.h>
#include <dae/daeMetaChoice.h>
#include <dae/daeMetaGroup.h>
#include <dae/daeMetaAny.h>
#include <dae/daeMetaElementAttribute.h>

daeElementRef
domRigid_constraint::create(DAE& dae)
{
	domRigid_constraintRef ref = new domRigid_constraint(dae);
	return ref;
}


daeMetaElement *
domRigid_constraint::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "rigid_constraint" );
	meta->registerClass(domRigid_constraint::create);

	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "ref_attachment" );
	mea->setOffset( daeOffsetOf(domRigid_constraint,elemRef_attachment) );
	mea->setElementType( domRigid_constraint::domRef_attachment::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 1, 1, 1 );
	mea->setName( "attachment" );
	mea->setOffset( daeOffsetOf(domRigid_constraint,elemAttachment) );
	mea->setElementType( domRigid_constraint::domAttachment::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 2, 1, 1 );
	mea->setName( "technique_common" );
	mea->setOffset( daeOffsetOf(domRigid_constraint,elemTechnique_common) );
	mea->setElementType( domRigid_constraint::domTechnique_common::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 3, 0, -1 );
	mea->setName( "technique" );
	mea->setOffset( daeOffsetOf(domRigid_constraint,elemTechnique_array) );
	mea->setElementType( domTechnique::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 4, 0, -1 );
	mea->setName( "extra" );
	mea->setOffset( daeOffsetOf(domRigid_constraint,elemExtra_array) );
	mea->setElementType( domExtra::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 4 );
	meta->setCMRoot( cm );	

	//	Add attribute: sid
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "sid" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domRigid_constraint , attrSid ));
		ma->setContainer( meta );
		ma->setIsRequired( true );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: name
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "name" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domRigid_constraint , attrName ));
		ma->setContainer( meta );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domRigid_constraint));
	meta->validate();

	return meta;
}

daeElementRef
domRigid_constraint::domRef_attachment::create(DAE& dae)
{
	domRigid_constraint::domRef_attachmentRef ref = new domRigid_constraint::domRef_attachment(dae);
	return ref;
}


daeMetaElement *
domRigid_constraint::domRef_attachment::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "ref_attachment" );
	meta->registerClass(domRigid_constraint::domRef_attachment::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaChoice( meta, cm, 0, 0, 0, -1 );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "translate" );
	mea->setOffset( daeOffsetOf(domRigid_constraint::domRef_attachment,elemTranslate_array) );
	mea->setElementType( domTranslate::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "rotate" );
	mea->setOffset( daeOffsetOf(domRigid_constraint::domRef_attachment,elemRotate_array) );
	mea->setElementType( domRotate::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 0, -1 );
	mea->setName( "extra" );
	mea->setOffset( daeOffsetOf(domRigid_constraint::domRef_attachment,elemExtra_array) );
	mea->setElementType( domExtra::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 3000 );
	meta->setCMRoot( cm );	
	// Ordered list of sub-elements
	meta->addContents(daeOffsetOf(domRigid_constraint::domRef_attachment,_contents));
	meta->addContentsOrder(daeOffsetOf(domRigid_constraint::domRef_attachment,_contentsOrder));

	meta->addCMDataArray(daeOffsetOf(domRigid_constraint::domRef_attachment,_CMData), 1);
	//	Add attribute: rigid_body
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "rigid_body" );
		ma->setType( dae.getAtomicTypes().get("xsAnyURI"));
		ma->setOffset( daeOffsetOf( domRigid_constraint::domRef_attachment , attrRigid_body ));
		ma->setContainer( meta );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domRigid_constraint::domRef_attachment));
	meta->validate();

	return meta;
}

daeElementRef
domRigid_constraint::domAttachment::create(DAE& dae)
{
	domRigid_constraint::domAttachmentRef ref = new domRigid_constraint::domAttachment(dae);
	return ref;
}


daeMetaElement *
domRigid_constraint::domAttachment::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "attachment" );
	meta->registerClass(domRigid_constraint::domAttachment::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaChoice( meta, cm, 0, 0, 0, -1 );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "translate" );
	mea->setOffset( daeOffsetOf(domRigid_constraint::domAttachment,elemTranslate_array) );
	mea->setElementType( domTranslate::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "rotate" );
	mea->setOffset( daeOffsetOf(domRigid_constraint::domAttachment,elemRotate_array) );
	mea->setElementType( domRotate::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 0, -1 );
	mea->setName( "extra" );
	mea->setOffset( daeOffsetOf(domRigid_constraint::domAttachment,elemExtra_array) );
	mea->setElementType( domExtra::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 3000 );
	meta->setCMRoot( cm );	
	// Ordered list of sub-elements
	meta->addContents(daeOffsetOf(domRigid_constraint::domAttachment,_contents));
	meta->addContentsOrder(daeOffsetOf(domRigid_constraint::domAttachment,_contentsOrder));

	meta->addCMDataArray(daeOffsetOf(domRigid_constraint::domAttachment,_CMData), 1);
	//	Add attribute: rigid_body
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "rigid_body" );
		ma->setType( dae.getAtomicTypes().get("xsAnyURI"));
		ma->setOffset( daeOffsetOf( domRigid_constraint::domAttachment , attrRigid_body ));
		ma->setContainer( meta );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domRigid_constraint::domAttachment));
	meta->validate();

	return meta;
}

daeElementRef
domRigid_constraint::domTechnique_common::create(DAE& dae)
{
	domRigid_constraint::domTechnique_commonRef ref = new domRigid_constraint::domTechnique_common(dae);
	return ref;
}


daeMetaElement *
domRigid_constraint::domTechnique_common::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "technique_common" );
	meta->registerClass(domRigid_constraint::domTechnique_common::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 0, 1 );
	mea->setName( "enabled" );
	mea->setOffset( daeOffsetOf(domRigid_constraint::domTechnique_common,elemEnabled) );
	mea->setElementType( domRigid_constraint::domTechnique_common::domEnabled::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 1, 0, 1 );
	mea->setName( "interpenetrate" );
	mea->setOffset( daeOffsetOf(domRigid_constraint::domTechnique_common,elemInterpenetrate) );
	mea->setElementType( domRigid_constraint::domTechnique_common::domInterpenetrate::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 2, 0, 1 );
	mea->setName( "limits" );
	mea->setOffset( daeOffsetOf(domRigid_constraint::domTechnique_common,elemLimits) );
	mea->setElementType( domRigid_constraint::domTechnique_common::domLimits::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 3, 0, 1 );
	mea->setName( "spring" );
	mea->setOffset( daeOffsetOf(domRigid_constraint::domTechnique_common,elemSpring) );
	mea->setElementType( domRigid_constraint::domTechnique_common::domSpring::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 3 );
	meta->setCMRoot( cm );	

	meta->setElementSize(sizeof(domRigid_constraint::domTechnique_common));
	meta->validate();

	return meta;
}

daeElementRef
domRigid_constraint::domTechnique_common::domEnabled::create(DAE& dae)
{
	domRigid_constraint::domTechnique_common::domEnabledRef ref = new domRigid_constraint::domTechnique_common::domEnabled(dae);
	return ref;
}


daeMetaElement *
domRigid_constraint::domTechnique_common::domEnabled::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "enabled" );
	meta->registerClass(domRigid_constraint::domTechnique_common::domEnabled::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Bool"));
		ma->setOffset( daeOffsetOf( domRigid_constraint::domTechnique_common::domEnabled , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	//	Add attribute: sid
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "sid" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domRigid_constraint::domTechnique_common::domEnabled , attrSid ));
		ma->setContainer( meta );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domRigid_constraint::domTechnique_common::domEnabled));
	meta->validate();

	return meta;
}

daeElementRef
domRigid_constraint::domTechnique_common::domInterpenetrate::create(DAE& dae)
{
	domRigid_constraint::domTechnique_common::domInterpenetrateRef ref = new domRigid_constraint::domTechnique_common::domInterpenetrate(dae);
	return ref;
}


daeMetaElement *
domRigid_constraint::domTechnique_common::domInterpenetrate::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "interpenetrate" );
	meta->registerClass(domRigid_constraint::domTechnique_common::domInterpenetrate::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Bool"));
		ma->setOffset( daeOffsetOf( domRigid_constraint::domTechnique_common::domInterpenetrate , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	//	Add attribute: sid
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "sid" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domRigid_constraint::domTechnique_common::domInterpenetrate , attrSid ));
		ma->setContainer( meta );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domRigid_constraint::domTechnique_common::domInterpenetrate));
	meta->validate();

	return meta;
}

daeElementRef
domRigid_constraint::domTechnique_common::domLimits::create(DAE& dae)
{
	domRigid_constraint::domTechnique_common::domLimitsRef ref = new domRigid_constraint::domTechnique_common::domLimits(dae);
	return ref;
}


daeMetaElement *
domRigid_constraint::domTechnique_common::domLimits::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "limits" );
	meta->registerClass(domRigid_constraint::domTechnique_common::domLimits::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 0, 1 );
	mea->setName( "swing_cone_and_twist" );
	mea->setOffset( daeOffsetOf(domRigid_constraint::domTechnique_common::domLimits,elemSwing_cone_and_twist) );
	mea->setElementType( domRigid_constraint::domTechnique_common::domLimits::domSwing_cone_and_twist::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 1, 0, 1 );
	mea->setName( "linear" );
	mea->setOffset( daeOffsetOf(domRigid_constraint::domTechnique_common::domLimits,elemLinear) );
	mea->setElementType( domRigid_constraint::domTechnique_common::domLimits::domLinear::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 1 );
	meta->setCMRoot( cm );	

	meta->setElementSize(sizeof(domRigid_constraint::domTechnique_common::domLimits));
	meta->validate();

	return meta;
}

daeElementRef
domRigid_constraint::domTechnique_common::domLimits::domSwing_cone_and_twist::create(DAE& dae)
{
	domRigid_constraint::domTechnique_common::domLimits::domSwing_cone_and_twistRef ref = new domRigid_constraint::domTechnique_common::domLimits::domSwing_cone_and_twist(dae);
	return ref;
}


daeMetaElement *
domRigid_constraint::domTechnique_common::domLimits::domSwing_cone_and_twist::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "swing_cone_and_twist" );
	meta->registerClass(domRigid_constraint::domTechnique_common::domLimits::domSwing_cone_and_twist::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 0, 1 );
	mea->setName( "min" );
	mea->setOffset( daeOffsetOf(domRigid_constraint::domTechnique_common::domLimits::domSwing_cone_and_twist,elemMin) );
	mea->setElementType( domTargetableFloat3::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 1, 0, 1 );
	mea->setName( "max" );
	mea->setOffset( daeOffsetOf(domRigid_constraint::domTechnique_common::domLimits::domSwing_cone_and_twist,elemMax) );
	mea->setElementType( domTargetableFloat3::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 1 );
	meta->setCMRoot( cm );	

	meta->setElementSize(sizeof(domRigid_constraint::domTechnique_common::domLimits::domSwing_cone_and_twist));
	meta->validate();

	return meta;
}

daeElementRef
domRigid_constraint::domTechnique_common::domLimits::domLinear::create(DAE& dae)
{
	domRigid_constraint::domTechnique_common::domLimits::domLinearRef ref = new domRigid_constraint::domTechnique_common::domLimits::domLinear(dae);
	return ref;
}


daeMetaElement *
domRigid_constraint::domTechnique_common::domLimits::domLinear::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "linear" );
	meta->registerClass(domRigid_constraint::domTechnique_common::domLimits::domLinear::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 0, 1 );
	mea->setName( "min" );
	mea->setOffset( daeOffsetOf(domRigid_constraint::domTechnique_common::domLimits::domLinear,elemMin) );
	mea->setElementType( domTargetableFloat3::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 1, 0, 1 );
	mea->setName( "max" );
	mea->setOffset( daeOffsetOf(domRigid_constraint::domTechnique_common::domLimits::domLinear,elemMax) );
	mea->setElementType( domTargetableFloat3::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 1 );
	meta->setCMRoot( cm );	

	meta->setElementSize(sizeof(domRigid_constraint::domTechnique_common::domLimits::domLinear));
	meta->validate();

	return meta;
}

daeElementRef
domRigid_constraint::domTechnique_common::domSpring::create(DAE& dae)
{
	domRigid_constraint::domTechnique_common::domSpringRef ref = new domRigid_constraint::domTechnique_common::domSpring(dae);
	return ref;
}


daeMetaElement *
domRigid_constraint::domTechnique_common::domSpring::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "spring" );
	meta->registerClass(domRigid_constraint::domTechnique_common::domSpring::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 0, 1 );
	mea->setName( "angular" );
	mea->setOffset( daeOffsetOf(domRigid_constraint::domTechnique_common::domSpring,elemAngular) );
	mea->setElementType( domRigid_constraint::domTechnique_common::domSpring::domAngular::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 1, 0, 1 );
	mea->setName( "linear" );
	mea->setOffset( daeOffsetOf(domRigid_constraint::domTechnique_common::domSpring,elemLinear) );
	mea->setElementType( domRigid_constraint::domTechnique_common::domSpring::domLinear::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 1 );
	meta->setCMRoot( cm );	

	meta->setElementSize(sizeof(domRigid_constraint::domTechnique_common::domSpring));
	meta->validate();

	return meta;
}

daeElementRef
domRigid_constraint::domTechnique_common::domSpring::domAngular::create(DAE& dae)
{
	domRigid_constraint::domTechnique_common::domSpring::domAngularRef ref = new domRigid_constraint::domTechnique_common::domSpring::domAngular(dae);
	return ref;
}


daeMetaElement *
domRigid_constraint::domTechnique_common::domSpring::domAngular::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "angular" );
	meta->registerClass(domRigid_constraint::domTechnique_common::domSpring::domAngular::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 0, 1 );
	mea->setName( "stiffness" );
	mea->setOffset( daeOffsetOf(domRigid_constraint::domTechnique_common::domSpring::domAngular,elemStiffness) );
	mea->setElementType( domTargetableFloat::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 1, 0, 1 );
	mea->setName( "damping" );
	mea->setOffset( daeOffsetOf(domRigid_constraint::domTechnique_common::domSpring::domAngular,elemDamping) );
	mea->setElementType( domTargetableFloat::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 2, 0, 1 );
	mea->setName( "target_value" );
	mea->setOffset( daeOffsetOf(domRigid_constraint::domTechnique_common::domSpring::domAngular,elemTarget_value) );
	mea->setElementType( domTargetableFloat::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 2 );
	meta->setCMRoot( cm );	

	meta->setElementSize(sizeof(domRigid_constraint::domTechnique_common::domSpring::domAngular));
	meta->validate();

	return meta;
}

daeElementRef
domRigid_constraint::domTechnique_common::domSpring::domLinear::create(DAE& dae)
{
	domRigid_constraint::domTechnique_common::domSpring::domLinearRef ref = new domRigid_constraint::domTechnique_common::domSpring::domLinear(dae);
	return ref;
}


daeMetaElement *
domRigid_constraint::domTechnique_common::domSpring::domLinear::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "linear" );
	meta->registerClass(domRigid_constraint::domTechnique_common::domSpring::domLinear::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 0, 1 );
	mea->setName( "stiffness" );
	mea->setOffset( daeOffsetOf(domRigid_constraint::domTechnique_common::domSpring::domLinear,elemStiffness) );
	mea->setElementType( domTargetableFloat::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 1, 0, 1 );
	mea->setName( "damping" );
	mea->setOffset( daeOffsetOf(domRigid_constraint::domTechnique_common::domSpring::domLinear,elemDamping) );
	mea->setElementType( domTargetableFloat::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 2, 0, 1 );
	mea->setName( "target_value" );
	mea->setOffset( daeOffsetOf(domRigid_constraint::domTechnique_common::domSpring::domLinear,elemTarget_value) );
	mea->setElementType( domTargetableFloat::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 2 );
	meta->setCMRoot( cm );	

	meta->setElementSize(sizeof(domRigid_constraint::domTechnique_common::domSpring::domLinear));
	meta->validate();

	return meta;
}

