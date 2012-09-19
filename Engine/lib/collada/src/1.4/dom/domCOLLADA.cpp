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
#include <dom/domCOLLADA.h>
#include <dae/daeMetaCMPolicy.h>
#include <dae/daeMetaSequence.h>
#include <dae/daeMetaChoice.h>
#include <dae/daeMetaGroup.h>
#include <dae/daeMetaAny.h>
#include <dae/daeMetaElementAttribute.h>

extern daeString COLLADA_VERSION;
extern daeString COLLADA_NAMESPACE;

daeElementRef
domCOLLADA::create(DAE& dae)
{
	domCOLLADARef ref = new domCOLLADA(dae);
	ref->_meta = dae.getMeta(domCOLLADA::ID());
	ref->setAttribute("version", COLLADA_VERSION );
	ref->setAttribute("xmlns", COLLADA_NAMESPACE );
	ref->_meta = NULL;
	return ref;
}


daeMetaElement *
domCOLLADA::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "COLLADA" );
	meta->registerClass(domCOLLADA::create);

	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "asset" );
	mea->setOffset( daeOffsetOf(domCOLLADA,elemAsset) );
	mea->setElementType( domAsset::registerElement(dae) );
	cm->appendChild( mea );

	cm = new daeMetaChoice( meta, cm, 0, 1, 0, -1 );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "library_animations" );
	mea->setOffset( daeOffsetOf(domCOLLADA,elemLibrary_animations_array) );
	mea->setElementType( domLibrary_animations::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "library_animation_clips" );
	mea->setOffset( daeOffsetOf(domCOLLADA,elemLibrary_animation_clips_array) );
	mea->setElementType( domLibrary_animation_clips::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "library_cameras" );
	mea->setOffset( daeOffsetOf(domCOLLADA,elemLibrary_cameras_array) );
	mea->setElementType( domLibrary_cameras::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "library_controllers" );
	mea->setOffset( daeOffsetOf(domCOLLADA,elemLibrary_controllers_array) );
	mea->setElementType( domLibrary_controllers::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "library_geometries" );
	mea->setOffset( daeOffsetOf(domCOLLADA,elemLibrary_geometries_array) );
	mea->setElementType( domLibrary_geometries::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "library_effects" );
	mea->setOffset( daeOffsetOf(domCOLLADA,elemLibrary_effects_array) );
	mea->setElementType( domLibrary_effects::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "library_force_fields" );
	mea->setOffset( daeOffsetOf(domCOLLADA,elemLibrary_force_fields_array) );
	mea->setElementType( domLibrary_force_fields::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "library_images" );
	mea->setOffset( daeOffsetOf(domCOLLADA,elemLibrary_images_array) );
	mea->setElementType( domLibrary_images::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "library_lights" );
	mea->setOffset( daeOffsetOf(domCOLLADA,elemLibrary_lights_array) );
	mea->setElementType( domLibrary_lights::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "library_materials" );
	mea->setOffset( daeOffsetOf(domCOLLADA,elemLibrary_materials_array) );
	mea->setElementType( domLibrary_materials::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "library_nodes" );
	mea->setOffset( daeOffsetOf(domCOLLADA,elemLibrary_nodes_array) );
	mea->setElementType( domLibrary_nodes::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "library_physics_materials" );
	mea->setOffset( daeOffsetOf(domCOLLADA,elemLibrary_physics_materials_array) );
	mea->setElementType( domLibrary_physics_materials::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "library_physics_models" );
	mea->setOffset( daeOffsetOf(domCOLLADA,elemLibrary_physics_models_array) );
	mea->setElementType( domLibrary_physics_models::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "library_physics_scenes" );
	mea->setOffset( daeOffsetOf(domCOLLADA,elemLibrary_physics_scenes_array) );
	mea->setElementType( domLibrary_physics_scenes::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "library_visual_scenes" );
	mea->setOffset( daeOffsetOf(domCOLLADA,elemLibrary_visual_scenes_array) );
	mea->setElementType( domLibrary_visual_scenes::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 0 );
	cm->getParent()->appendChild( cm );
	cm = cm->getParent();

	mea = new daeMetaElementAttribute( meta, cm, 3002, 0, 1 );
	mea->setName( "scene" );
	mea->setOffset( daeOffsetOf(domCOLLADA,elemScene) );
	mea->setElementType( domCOLLADA::domScene::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 3003, 0, -1 );
	mea->setName( "extra" );
	mea->setOffset( daeOffsetOf(domCOLLADA,elemExtra_array) );
	mea->setElementType( domExtra::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 3003 );
	meta->setCMRoot( cm );	
	// Ordered list of sub-elements
	meta->addContents(daeOffsetOf(domCOLLADA,_contents));
	meta->addContentsOrder(daeOffsetOf(domCOLLADA,_contentsOrder));

	meta->addCMDataArray(daeOffsetOf(domCOLLADA,_CMData), 1);	//	Add attribute: xmlns
	{
		daeMetaAttribute* ma = new daeMetaAttribute;
		ma->setName( "xmlns" );
		ma->setType( dae.getAtomicTypes().get("xsAnyURI"));
		ma->setOffset( daeOffsetOf( domCOLLADA , attrXmlns ));
		ma->setContainer( meta );
		//ma->setIsRequired( true );
		meta->appendAttribute(ma);
	}
    
	//	Add attribute: version
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "version" );
		ma->setType( dae.getAtomicTypes().get("VersionType"));
		ma->setOffset( daeOffsetOf( domCOLLADA , attrVersion ));
		ma->setContainer( meta );
		ma->setIsRequired( true );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: xml_base
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "xml_base" );
		ma->setType( dae.getAtomicTypes().get("xsAnyURI"));
		ma->setOffset( daeOffsetOf( domCOLLADA , attrXml_base ));
		ma->setContainer( meta );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domCOLLADA));
	meta->validate();

	return meta;
}

daeElementRef
domCOLLADA::domScene::create(DAE& dae)
{
	domCOLLADA::domSceneRef ref = new domCOLLADA::domScene(dae);
	return ref;
}


daeMetaElement *
domCOLLADA::domScene::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "scene" );
	meta->registerClass(domCOLLADA::domScene::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 0, -1 );
	mea->setName( "instance_physics_scene" );
	mea->setOffset( daeOffsetOf(domCOLLADA::domScene,elemInstance_physics_scene_array) );
	mea->setElementType( domInstanceWithExtra::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 1, 0, 1 );
	mea->setName( "instance_visual_scene" );
	mea->setOffset( daeOffsetOf(domCOLLADA::domScene,elemInstance_visual_scene) );
	mea->setElementType( domInstanceWithExtra::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 2, 0, -1 );
	mea->setName( "extra" );
	mea->setOffset( daeOffsetOf(domCOLLADA::domScene,elemExtra_array) );
	mea->setElementType( domExtra::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 2 );
	meta->setCMRoot( cm );	

	meta->setElementSize(sizeof(domCOLLADA::domScene));
	meta->validate();

	return meta;
}

