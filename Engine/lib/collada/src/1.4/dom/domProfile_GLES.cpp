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
#include <dom/domProfile_GLES.h>
#include <dae/daeMetaCMPolicy.h>
#include <dae/daeMetaSequence.h>
#include <dae/daeMetaChoice.h>
#include <dae/daeMetaGroup.h>
#include <dae/daeMetaAny.h>
#include <dae/daeMetaElementAttribute.h>

daeElementRef
domProfile_GLES::create(DAE& dae)
{
	domProfile_GLESRef ref = new domProfile_GLES(dae);
	return ref;
}


daeMetaElement *
domProfile_GLES::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "profile_GLES" );
	meta->registerClass(domProfile_GLES::create);

	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 0, 1 );
	mea->setName( "asset" );
	mea->setOffset( daeOffsetOf(domProfile_GLES,elemAsset) );
	mea->setElementType( domAsset::registerElement(dae) );
	cm->appendChild( mea );

	cm = new daeMetaChoice( meta, cm, 0, 1, 0, -1 );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "image" );
	mea->setOffset( daeOffsetOf(domProfile_GLES,elemImage_array) );
	mea->setElementType( domImage::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "newparam" );
	mea->setOffset( daeOffsetOf(domProfile_GLES,elemNewparam_array) );
	mea->setElementType( domGles_newparam::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 0 );
	cm->getParent()->appendChild( cm );
	cm = cm->getParent();

	mea = new daeMetaElementArrayAttribute( meta, cm, 3002, 1, -1 );
	mea->setName( "technique" );
	mea->setOffset( daeOffsetOf(domProfile_GLES,elemTechnique_array) );
	mea->setElementType( domProfile_GLES::domTechnique::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 3003, 0, -1 );
	mea->setName( "extra" );
	mea->setOffset( daeOffsetOf(domProfile_GLES,elemExtra_array) );
	mea->setElementType( domExtra::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 3003 );
	meta->setCMRoot( cm );	
	// Ordered list of sub-elements
	meta->addContents(daeOffsetOf(domProfile_GLES,_contents));
	meta->addContentsOrder(daeOffsetOf(domProfile_GLES,_contentsOrder));

	meta->addCMDataArray(daeOffsetOf(domProfile_GLES,_CMData), 1);
	//	Add attribute: id
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "id" );
		ma->setType( dae.getAtomicTypes().get("xsID"));
		ma->setOffset( daeOffsetOf( domProfile_GLES , attrId ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: platform
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "platform" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domProfile_GLES , attrPlatform ));
		ma->setContainer( meta );
		ma->setDefaultString( "PC");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domProfile_GLES));
	meta->validate();

	return meta;
}

daeElementRef
domProfile_GLES::domTechnique::create(DAE& dae)
{
	domProfile_GLES::domTechniqueRef ref = new domProfile_GLES::domTechnique(dae);
	return ref;
}


daeMetaElement *
domProfile_GLES::domTechnique::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "technique" );
	meta->registerClass(domProfile_GLES::domTechnique::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 0, 1 );
	mea->setName( "asset" );
	mea->setOffset( daeOffsetOf(domProfile_GLES::domTechnique,elemAsset) );
	mea->setElementType( domAsset::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 1, 0, -1 );
	mea->setName( "annotate" );
	mea->setOffset( daeOffsetOf(domProfile_GLES::domTechnique,elemAnnotate_array) );
	mea->setElementType( domFx_annotate_common::registerElement(dae) );
	cm->appendChild( mea );

	cm = new daeMetaChoice( meta, cm, 0, 2, 0, -1 );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "image" );
	mea->setOffset( daeOffsetOf(domProfile_GLES::domTechnique,elemImage_array) );
	mea->setElementType( domImage::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "newparam" );
	mea->setOffset( daeOffsetOf(domProfile_GLES::domTechnique,elemNewparam_array) );
	mea->setElementType( domGles_newparam::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "setparam" );
	mea->setOffset( daeOffsetOf(domProfile_GLES::domTechnique,elemSetparam_array) );
	mea->setElementType( domProfile_GLES::domTechnique::domSetparam::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 0 );
	cm->getParent()->appendChild( cm );
	cm = cm->getParent();

	mea = new daeMetaElementArrayAttribute( meta, cm, 3003, 1, -1 );
	mea->setName( "pass" );
	mea->setOffset( daeOffsetOf(domProfile_GLES::domTechnique,elemPass_array) );
	mea->setElementType( domProfile_GLES::domTechnique::domPass::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 3004, 0, -1 );
	mea->setName( "extra" );
	mea->setOffset( daeOffsetOf(domProfile_GLES::domTechnique,elemExtra_array) );
	mea->setElementType( domExtra::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 3004 );
	meta->setCMRoot( cm );	
	// Ordered list of sub-elements
	meta->addContents(daeOffsetOf(domProfile_GLES::domTechnique,_contents));
	meta->addContentsOrder(daeOffsetOf(domProfile_GLES::domTechnique,_contentsOrder));

	meta->addCMDataArray(daeOffsetOf(domProfile_GLES::domTechnique,_CMData), 1);
	//	Add attribute: id
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "id" );
		ma->setType( dae.getAtomicTypes().get("xsID"));
		ma->setOffset( daeOffsetOf( domProfile_GLES::domTechnique , attrId ));
		ma->setContainer( meta );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: sid
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "sid" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domProfile_GLES::domTechnique , attrSid ));
		ma->setContainer( meta );
		ma->setIsRequired( true );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domProfile_GLES::domTechnique));
	meta->validate();

	return meta;
}

daeElementRef
domProfile_GLES::domTechnique::domSetparam::create(DAE& dae)
{
	domProfile_GLES::domTechnique::domSetparamRef ref = new domProfile_GLES::domTechnique::domSetparam(dae);
	return ref;
}


daeMetaElement *
domProfile_GLES::domTechnique::domSetparam::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "setparam" );
	meta->registerClass(domProfile_GLES::domTechnique::domSetparam::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 0, -1 );
	mea->setName( "annotate" );
	mea->setOffset( daeOffsetOf(domProfile_GLES::domTechnique::domSetparam,elemAnnotate_array) );
	mea->setElementType( domFx_annotate_common::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 1, 1, 1 );
	mea->setName( "gles_basic_type_common" );
	mea->setOffset( daeOffsetOf(domProfile_GLES::domTechnique::domSetparam,elemGles_basic_type_common) );
	mea->setElementType( domGles_basic_type_common::registerElement(dae) );
	cm->appendChild( new daeMetaGroup( mea, meta, cm, 1, 1, 1 ) );

	cm->setMaxOrdinal( 1 );
	meta->setCMRoot( cm );	

	//	Add attribute: ref
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "ref" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domProfile_GLES::domTechnique::domSetparam , attrRef ));
		ma->setContainer( meta );
		ma->setIsRequired( true );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domProfile_GLES::domTechnique::domSetparam));
	meta->validate();

	return meta;
}

daeElementRef
domProfile_GLES::domTechnique::domPass::create(DAE& dae)
{
	domProfile_GLES::domTechnique::domPassRef ref = new domProfile_GLES::domTechnique::domPass(dae);
	return ref;
}


daeMetaElement *
domProfile_GLES::domTechnique::domPass::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "pass" );
	meta->registerClass(domProfile_GLES::domTechnique::domPass::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 0, -1 );
	mea->setName( "annotate" );
	mea->setOffset( daeOffsetOf(domProfile_GLES::domTechnique::domPass,elemAnnotate_array) );
	mea->setElementType( domFx_annotate_common::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 1, 0, 1 );
	mea->setName( "color_target" );
	mea->setOffset( daeOffsetOf(domProfile_GLES::domTechnique::domPass,elemColor_target) );
	mea->setElementType( domProfile_GLES::domTechnique::domPass::domColor_target::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 2, 0, 1 );
	mea->setName( "depth_target" );
	mea->setOffset( daeOffsetOf(domProfile_GLES::domTechnique::domPass,elemDepth_target) );
	mea->setElementType( domProfile_GLES::domTechnique::domPass::domDepth_target::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 3, 0, 1 );
	mea->setName( "stencil_target" );
	mea->setOffset( daeOffsetOf(domProfile_GLES::domTechnique::domPass,elemStencil_target) );
	mea->setElementType( domProfile_GLES::domTechnique::domPass::domStencil_target::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 4, 0, 1 );
	mea->setName( "color_clear" );
	mea->setOffset( daeOffsetOf(domProfile_GLES::domTechnique::domPass,elemColor_clear) );
	mea->setElementType( domProfile_GLES::domTechnique::domPass::domColor_clear::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 5, 0, 1 );
	mea->setName( "depth_clear" );
	mea->setOffset( daeOffsetOf(domProfile_GLES::domTechnique::domPass,elemDepth_clear) );
	mea->setElementType( domProfile_GLES::domTechnique::domPass::domDepth_clear::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 6, 0, 1 );
	mea->setName( "stencil_clear" );
	mea->setOffset( daeOffsetOf(domProfile_GLES::domTechnique::domPass,elemStencil_clear) );
	mea->setElementType( domProfile_GLES::domTechnique::domPass::domStencil_clear::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 7, 0, 1 );
	mea->setName( "draw" );
	mea->setOffset( daeOffsetOf(domProfile_GLES::domTechnique::domPass,elemDraw) );
	mea->setElementType( domProfile_GLES::domTechnique::domPass::domDraw::registerElement(dae) );
	cm->appendChild( mea );

	cm = new daeMetaChoice( meta, cm, 0, 8, 0, -1 );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "gles_pipeline_settings" );
	mea->setOffset( daeOffsetOf(domProfile_GLES::domTechnique::domPass,elemGles_pipeline_settings_array) );
	mea->setElementType( domGles_pipeline_settings::registerElement(dae) );
	cm->appendChild( new daeMetaGroup( mea, meta, cm, 0, 1, 1 ) );

	cm->setMaxOrdinal( 0 );
	cm->getParent()->appendChild( cm );
	cm = cm->getParent();

	mea = new daeMetaElementArrayAttribute( meta, cm, 3009, 0, -1 );
	mea->setName( "extra" );
	mea->setOffset( daeOffsetOf(domProfile_GLES::domTechnique::domPass,elemExtra_array) );
	mea->setElementType( domExtra::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 3009 );
	meta->setCMRoot( cm );	
	// Ordered list of sub-elements
	meta->addContents(daeOffsetOf(domProfile_GLES::domTechnique::domPass,_contents));
	meta->addContentsOrder(daeOffsetOf(domProfile_GLES::domTechnique::domPass,_contentsOrder));

	meta->addCMDataArray(daeOffsetOf(domProfile_GLES::domTechnique::domPass,_CMData), 1);
	//	Add attribute: sid
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "sid" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domProfile_GLES::domTechnique::domPass , attrSid ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domProfile_GLES::domTechnique::domPass));
	meta->validate();

	return meta;
}

daeElementRef
domProfile_GLES::domTechnique::domPass::domColor_target::create(DAE& dae)
{
	domProfile_GLES::domTechnique::domPass::domColor_targetRef ref = new domProfile_GLES::domTechnique::domPass::domColor_target(dae);
	return ref;
}


daeMetaElement *
domProfile_GLES::domTechnique::domPass::domColor_target::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "color_target" );
	meta->registerClass(domProfile_GLES::domTechnique::domPass::domColor_target::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Gles_rendertarget_common"));
		ma->setOffset( daeOffsetOf( domProfile_GLES::domTechnique::domPass::domColor_target , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domProfile_GLES::domTechnique::domPass::domColor_target));
	meta->validate();

	return meta;
}

daeElementRef
domProfile_GLES::domTechnique::domPass::domDepth_target::create(DAE& dae)
{
	domProfile_GLES::domTechnique::domPass::domDepth_targetRef ref = new domProfile_GLES::domTechnique::domPass::domDepth_target(dae);
	return ref;
}


daeMetaElement *
domProfile_GLES::domTechnique::domPass::domDepth_target::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "depth_target" );
	meta->registerClass(domProfile_GLES::domTechnique::domPass::domDepth_target::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Gles_rendertarget_common"));
		ma->setOffset( daeOffsetOf( domProfile_GLES::domTechnique::domPass::domDepth_target , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domProfile_GLES::domTechnique::domPass::domDepth_target));
	meta->validate();

	return meta;
}

daeElementRef
domProfile_GLES::domTechnique::domPass::domStencil_target::create(DAE& dae)
{
	domProfile_GLES::domTechnique::domPass::domStencil_targetRef ref = new domProfile_GLES::domTechnique::domPass::domStencil_target(dae);
	return ref;
}


daeMetaElement *
domProfile_GLES::domTechnique::domPass::domStencil_target::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "stencil_target" );
	meta->registerClass(domProfile_GLES::domTechnique::domPass::domStencil_target::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Gles_rendertarget_common"));
		ma->setOffset( daeOffsetOf( domProfile_GLES::domTechnique::domPass::domStencil_target , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domProfile_GLES::domTechnique::domPass::domStencil_target));
	meta->validate();

	return meta;
}

daeElementRef
domProfile_GLES::domTechnique::domPass::domColor_clear::create(DAE& dae)
{
	domProfile_GLES::domTechnique::domPass::domColor_clearRef ref = new domProfile_GLES::domTechnique::domPass::domColor_clear(dae);
	return ref;
}


daeMetaElement *
domProfile_GLES::domTechnique::domPass::domColor_clear::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "color_clear" );
	meta->registerClass(domProfile_GLES::domTechnique::domPass::domColor_clear::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaArrayAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Fx_color_common"));
		ma->setOffset( daeOffsetOf( domProfile_GLES::domTechnique::domPass::domColor_clear , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domProfile_GLES::domTechnique::domPass::domColor_clear));
	meta->validate();

	return meta;
}

daeElementRef
domProfile_GLES::domTechnique::domPass::domDepth_clear::create(DAE& dae)
{
	domProfile_GLES::domTechnique::domPass::domDepth_clearRef ref = new domProfile_GLES::domTechnique::domPass::domDepth_clear(dae);
	return ref;
}


daeMetaElement *
domProfile_GLES::domTechnique::domPass::domDepth_clear::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "depth_clear" );
	meta->registerClass(domProfile_GLES::domTechnique::domPass::domDepth_clear::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Float"));
		ma->setOffset( daeOffsetOf( domProfile_GLES::domTechnique::domPass::domDepth_clear , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domProfile_GLES::domTechnique::domPass::domDepth_clear));
	meta->validate();

	return meta;
}

daeElementRef
domProfile_GLES::domTechnique::domPass::domStencil_clear::create(DAE& dae)
{
	domProfile_GLES::domTechnique::domPass::domStencil_clearRef ref = new domProfile_GLES::domTechnique::domPass::domStencil_clear(dae);
	return ref;
}


daeMetaElement *
domProfile_GLES::domTechnique::domPass::domStencil_clear::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "stencil_clear" );
	meta->registerClass(domProfile_GLES::domTechnique::domPass::domStencil_clear::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("xsByte"));
		ma->setOffset( daeOffsetOf( domProfile_GLES::domTechnique::domPass::domStencil_clear , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domProfile_GLES::domTechnique::domPass::domStencil_clear));
	meta->validate();

	return meta;
}

daeElementRef
domProfile_GLES::domTechnique::domPass::domDraw::create(DAE& dae)
{
	domProfile_GLES::domTechnique::domPass::domDrawRef ref = new domProfile_GLES::domTechnique::domPass::domDraw(dae);
	return ref;
}


daeMetaElement *
domProfile_GLES::domTechnique::domPass::domDraw::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "draw" );
	meta->registerClass(domProfile_GLES::domTechnique::domPass::domDraw::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Fx_draw_common"));
		ma->setOffset( daeOffsetOf( domProfile_GLES::domTechnique::domPass::domDraw , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domProfile_GLES::domTechnique::domPass::domDraw));
	meta->validate();

	return meta;
}

