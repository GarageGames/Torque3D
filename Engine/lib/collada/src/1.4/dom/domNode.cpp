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
#include <dom/domNode.h>
#include <dae/daeMetaCMPolicy.h>
#include <dae/daeMetaSequence.h>
#include <dae/daeMetaChoice.h>
#include <dae/daeMetaGroup.h>
#include <dae/daeMetaAny.h>
#include <dae/daeMetaElementAttribute.h>

daeElementRef
domNode::create(DAE& dae)
{
	domNodeRef ref = new domNode(dae);
	return ref;
}


daeMetaElement *
domNode::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "node" );
	meta->registerClass(domNode::create);

	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 0, 1 );
	mea->setName( "asset" );
	mea->setOffset( daeOffsetOf(domNode,elemAsset) );
	mea->setElementType( domAsset::registerElement(dae) );
	cm->appendChild( mea );

	cm = new daeMetaChoice( meta, cm, 0, 1, 0, -1 );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "lookat" );
	mea->setOffset( daeOffsetOf(domNode,elemLookat_array) );
	mea->setElementType( domLookat::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "matrix" );
	mea->setOffset( daeOffsetOf(domNode,elemMatrix_array) );
	mea->setElementType( domMatrix::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "rotate" );
	mea->setOffset( daeOffsetOf(domNode,elemRotate_array) );
	mea->setElementType( domRotate::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "scale" );
	mea->setOffset( daeOffsetOf(domNode,elemScale_array) );
	mea->setElementType( domScale::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "skew" );
	mea->setOffset( daeOffsetOf(domNode,elemSkew_array) );
	mea->setElementType( domSkew::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "translate" );
	mea->setOffset( daeOffsetOf(domNode,elemTranslate_array) );
	mea->setElementType( domTranslate::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 0 );
	cm->getParent()->appendChild( cm );
	cm = cm->getParent();

	mea = new daeMetaElementArrayAttribute( meta, cm, 3002, 0, -1 );
	mea->setName( "instance_camera" );
	mea->setOffset( daeOffsetOf(domNode,elemInstance_camera_array) );
	mea->setElementType( domInstance_camera::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 3003, 0, -1 );
	mea->setName( "instance_controller" );
	mea->setOffset( daeOffsetOf(domNode,elemInstance_controller_array) );
	mea->setElementType( domInstance_controller::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 3004, 0, -1 );
	mea->setName( "instance_geometry" );
	mea->setOffset( daeOffsetOf(domNode,elemInstance_geometry_array) );
	mea->setElementType( domInstance_geometry::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 3005, 0, -1 );
	mea->setName( "instance_light" );
	mea->setOffset( daeOffsetOf(domNode,elemInstance_light_array) );
	mea->setElementType( domInstance_light::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 3006, 0, -1 );
	mea->setName( "instance_node" );
	mea->setOffset( daeOffsetOf(domNode,elemInstance_node_array) );
	mea->setElementType( domInstance_node::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 3007, 0, -1 );
	mea->setName( "node" );
	mea->setOffset( daeOffsetOf(domNode,elemNode_array) );
	mea->setElementType( domNode::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 3008, 0, -1 );
	mea->setName( "extra" );
	mea->setOffset( daeOffsetOf(domNode,elemExtra_array) );
	mea->setElementType( domExtra::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 3008 );
	meta->setCMRoot( cm );	
	// Ordered list of sub-elements
	meta->addContents(daeOffsetOf(domNode,_contents));
	meta->addContentsOrder(daeOffsetOf(domNode,_contentsOrder));

	meta->addCMDataArray(daeOffsetOf(domNode,_CMData), 1);
	//	Add attribute: id
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "id" );
		ma->setType( dae.getAtomicTypes().get("xsID"));
		ma->setOffset( daeOffsetOf( domNode , attrId ));
		ma->setContainer( meta );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: name
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "name" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domNode , attrName ));
		ma->setContainer( meta );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: sid
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "sid" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domNode , attrSid ));
		ma->setContainer( meta );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: type
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "type" );
		ma->setType( dae.getAtomicTypes().get("NodeType"));
		ma->setOffset( daeOffsetOf( domNode , attrType ));
		ma->setContainer( meta );
		ma->setDefaultString( "NODE");
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: layer
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "layer" );
		ma->setType( dae.getAtomicTypes().get("ListOfNames"));
		ma->setOffset( daeOffsetOf( domNode , attrLayer ));
		ma->setContainer( meta );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domNode));
	meta->validate();

	return meta;
}

