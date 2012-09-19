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
#include <dom/domConvex_mesh.h>
#include <dae/daeMetaCMPolicy.h>
#include <dae/daeMetaSequence.h>
#include <dae/daeMetaChoice.h>
#include <dae/daeMetaGroup.h>
#include <dae/daeMetaAny.h>
#include <dae/daeMetaElementAttribute.h>

daeElementRef
domConvex_mesh::create(DAE& dae)
{
	domConvex_meshRef ref = new domConvex_mesh(dae);
	return ref;
}


daeMetaElement *
domConvex_mesh::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "convex_mesh" );
	meta->registerClass(domConvex_mesh::create);

	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 0, 1 );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, -1 );
	mea->setName( "source" );
	mea->setOffset( daeOffsetOf(domConvex_mesh,elemSource_array) );
	mea->setElementType( domSource::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 1, 1, 1 );
	mea->setName( "vertices" );
	mea->setOffset( daeOffsetOf(domConvex_mesh,elemVertices) );
	mea->setElementType( domVertices::registerElement(dae) );
	cm->appendChild( mea );

	cm = new daeMetaChoice( meta, cm, 0, 2, 0, -1 );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "lines" );
	mea->setOffset( daeOffsetOf(domConvex_mesh,elemLines_array) );
	mea->setElementType( domLines::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "linestrips" );
	mea->setOffset( daeOffsetOf(domConvex_mesh,elemLinestrips_array) );
	mea->setElementType( domLinestrips::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "polygons" );
	mea->setOffset( daeOffsetOf(domConvex_mesh,elemPolygons_array) );
	mea->setElementType( domPolygons::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "polylist" );
	mea->setOffset( daeOffsetOf(domConvex_mesh,elemPolylist_array) );
	mea->setElementType( domPolylist::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "triangles" );
	mea->setOffset( daeOffsetOf(domConvex_mesh,elemTriangles_array) );
	mea->setElementType( domTriangles::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "trifans" );
	mea->setOffset( daeOffsetOf(domConvex_mesh,elemTrifans_array) );
	mea->setElementType( domTrifans::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "tristrips" );
	mea->setOffset( daeOffsetOf(domConvex_mesh,elemTristrips_array) );
	mea->setElementType( domTristrips::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 0 );
	cm->getParent()->appendChild( cm );
	cm = cm->getParent();

	mea = new daeMetaElementArrayAttribute( meta, cm, 3003, 0, -1 );
	mea->setName( "extra" );
	mea->setOffset( daeOffsetOf(domConvex_mesh,elemExtra_array) );
	mea->setElementType( domExtra::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 3003 );
	meta->setCMRoot( cm );	
	// Ordered list of sub-elements
	meta->addContents(daeOffsetOf(domConvex_mesh,_contents));
	meta->addContentsOrder(daeOffsetOf(domConvex_mesh,_contentsOrder));

	meta->addCMDataArray(daeOffsetOf(domConvex_mesh,_CMData), 1);
	//	Add attribute: convex_hull_of
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "convex_hull_of" );
		ma->setType( dae.getAtomicTypes().get("xsAnyURI"));
		ma->setOffset( daeOffsetOf( domConvex_mesh , attrConvex_hull_of ));
		ma->setContainer( meta );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domConvex_mesh));
	meta->validate();

	return meta;
}

