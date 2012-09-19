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
#include <dom/domProfile_CG.h>
#include <dae/daeMetaCMPolicy.h>
#include <dae/daeMetaSequence.h>
#include <dae/daeMetaChoice.h>
#include <dae/daeMetaGroup.h>
#include <dae/daeMetaAny.h>
#include <dae/daeMetaElementAttribute.h>

daeElementRef
domProfile_CG::create(DAE& dae)
{
	domProfile_CGRef ref = new domProfile_CG(dae);
	return ref;
}


daeMetaElement *
domProfile_CG::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "profile_CG" );
	meta->registerClass(domProfile_CG::create);

	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 0, 1 );
	mea->setName( "asset" );
	mea->setOffset( daeOffsetOf(domProfile_CG,elemAsset) );
	mea->setElementType( domAsset::registerElement(dae) );
	cm->appendChild( mea );

	cm = new daeMetaChoice( meta, cm, 0, 1, 0, -1 );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "code" );
	mea->setOffset( daeOffsetOf(domProfile_CG,elemCode_array) );
	mea->setElementType( domFx_code_profile::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "include" );
	mea->setOffset( daeOffsetOf(domProfile_CG,elemInclude_array) );
	mea->setElementType( domFx_include_common::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 0 );
	cm->getParent()->appendChild( cm );
	cm = cm->getParent();

	cm = new daeMetaChoice( meta, cm, 1, 3002, 0, -1 );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "image" );
	mea->setOffset( daeOffsetOf(domProfile_CG,elemImage_array) );
	mea->setElementType( domImage::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "newparam" );
	mea->setOffset( daeOffsetOf(domProfile_CG,elemNewparam_array) );
	mea->setElementType( domCg_newparam::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 0 );
	cm->getParent()->appendChild( cm );
	cm = cm->getParent();

	mea = new daeMetaElementArrayAttribute( meta, cm, 6003, 1, -1 );
	mea->setName( "technique" );
	mea->setOffset( daeOffsetOf(domProfile_CG,elemTechnique_array) );
	mea->setElementType( domProfile_CG::domTechnique::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 6004, 0, -1 );
	mea->setName( "extra" );
	mea->setOffset( daeOffsetOf(domProfile_CG,elemExtra_array) );
	mea->setElementType( domExtra::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 6004 );
	meta->setCMRoot( cm );	
	// Ordered list of sub-elements
	meta->addContents(daeOffsetOf(domProfile_CG,_contents));
	meta->addContentsOrder(daeOffsetOf(domProfile_CG,_contentsOrder));

	meta->addCMDataArray(daeOffsetOf(domProfile_CG,_CMData), 2);
	//	Add attribute: id
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "id" );
		ma->setType( dae.getAtomicTypes().get("xsID"));
		ma->setOffset( daeOffsetOf( domProfile_CG , attrId ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: platform
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "platform" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domProfile_CG , attrPlatform ));
		ma->setContainer( meta );
		ma->setDefaultString( "PC");
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domProfile_CG));
	meta->validate();

	return meta;
}

daeElementRef
domProfile_CG::domTechnique::create(DAE& dae)
{
	domProfile_CG::domTechniqueRef ref = new domProfile_CG::domTechnique(dae);
	return ref;
}


daeMetaElement *
domProfile_CG::domTechnique::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "technique" );
	meta->registerClass(domProfile_CG::domTechnique::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 0, 1 );
	mea->setName( "asset" );
	mea->setOffset( daeOffsetOf(domProfile_CG::domTechnique,elemAsset) );
	mea->setElementType( domAsset::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 1, 0, -1 );
	mea->setName( "annotate" );
	mea->setOffset( daeOffsetOf(domProfile_CG::domTechnique,elemAnnotate_array) );
	mea->setElementType( domFx_annotate_common::registerElement(dae) );
	cm->appendChild( mea );

	cm = new daeMetaChoice( meta, cm, 0, 2, 0, -1 );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "code" );
	mea->setOffset( daeOffsetOf(domProfile_CG::domTechnique,elemCode_array) );
	mea->setElementType( domFx_code_profile::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "include" );
	mea->setOffset( daeOffsetOf(domProfile_CG::domTechnique,elemInclude_array) );
	mea->setElementType( domFx_include_common::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 0 );
	cm->getParent()->appendChild( cm );
	cm = cm->getParent();

	cm = new daeMetaChoice( meta, cm, 1, 3003, 0, -1 );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "image" );
	mea->setOffset( daeOffsetOf(domProfile_CG::domTechnique,elemImage_array) );
	mea->setElementType( domImage::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "newparam" );
	mea->setOffset( daeOffsetOf(domProfile_CG::domTechnique,elemNewparam_array) );
	mea->setElementType( domCg_newparam::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "setparam" );
	mea->setOffset( daeOffsetOf(domProfile_CG::domTechnique,elemSetparam_array) );
	mea->setElementType( domCg_setparam::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 0 );
	cm->getParent()->appendChild( cm );
	cm = cm->getParent();

	mea = new daeMetaElementArrayAttribute( meta, cm, 6004, 1, -1 );
	mea->setName( "pass" );
	mea->setOffset( daeOffsetOf(domProfile_CG::domTechnique,elemPass_array) );
	mea->setElementType( domProfile_CG::domTechnique::domPass::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 6005, 0, -1 );
	mea->setName( "extra" );
	mea->setOffset( daeOffsetOf(domProfile_CG::domTechnique,elemExtra_array) );
	mea->setElementType( domExtra::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 6005 );
	meta->setCMRoot( cm );	
	// Ordered list of sub-elements
	meta->addContents(daeOffsetOf(domProfile_CG::domTechnique,_contents));
	meta->addContentsOrder(daeOffsetOf(domProfile_CG::domTechnique,_contentsOrder));

	meta->addCMDataArray(daeOffsetOf(domProfile_CG::domTechnique,_CMData), 2);
	//	Add attribute: id
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "id" );
		ma->setType( dae.getAtomicTypes().get("xsID"));
		ma->setOffset( daeOffsetOf( domProfile_CG::domTechnique , attrId ));
		ma->setContainer( meta );
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: sid
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "sid" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domProfile_CG::domTechnique , attrSid ));
		ma->setContainer( meta );
		ma->setIsRequired( true );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domProfile_CG::domTechnique));
	meta->validate();

	return meta;
}

daeElementRef
domProfile_CG::domTechnique::domPass::create(DAE& dae)
{
	domProfile_CG::domTechnique::domPassRef ref = new domProfile_CG::domTechnique::domPass(dae);
	return ref;
}


daeMetaElement *
domProfile_CG::domTechnique::domPass::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "pass" );
	meta->registerClass(domProfile_CG::domTechnique::domPass::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 0, -1 );
	mea->setName( "annotate" );
	mea->setOffset( daeOffsetOf(domProfile_CG::domTechnique::domPass,elemAnnotate_array) );
	mea->setElementType( domFx_annotate_common::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 1, 0, -1 );
	mea->setName( "color_target" );
	mea->setOffset( daeOffsetOf(domProfile_CG::domTechnique::domPass,elemColor_target_array) );
	mea->setElementType( domFx_colortarget_common::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 2, 0, -1 );
	mea->setName( "depth_target" );
	mea->setOffset( daeOffsetOf(domProfile_CG::domTechnique::domPass,elemDepth_target_array) );
	mea->setElementType( domFx_depthtarget_common::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 3, 0, -1 );
	mea->setName( "stencil_target" );
	mea->setOffset( daeOffsetOf(domProfile_CG::domTechnique::domPass,elemStencil_target_array) );
	mea->setElementType( domFx_stenciltarget_common::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 4, 0, -1 );
	mea->setName( "color_clear" );
	mea->setOffset( daeOffsetOf(domProfile_CG::domTechnique::domPass,elemColor_clear_array) );
	mea->setElementType( domFx_clearcolor_common::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 5, 0, -1 );
	mea->setName( "depth_clear" );
	mea->setOffset( daeOffsetOf(domProfile_CG::domTechnique::domPass,elemDepth_clear_array) );
	mea->setElementType( domFx_cleardepth_common::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 6, 0, -1 );
	mea->setName( "stencil_clear" );
	mea->setOffset( daeOffsetOf(domProfile_CG::domTechnique::domPass,elemStencil_clear_array) );
	mea->setElementType( domFx_clearstencil_common::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 7, 0, 1 );
	mea->setName( "draw" );
	mea->setOffset( daeOffsetOf(domProfile_CG::domTechnique::domPass,elemDraw) );
	mea->setElementType( domProfile_CG::domTechnique::domPass::domDraw::registerElement(dae) );
	cm->appendChild( mea );

	cm = new daeMetaChoice( meta, cm, 0, 8, 1, -1 );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "gl_pipeline_settings" );
	mea->setOffset( daeOffsetOf(domProfile_CG::domTechnique::domPass,elemGl_pipeline_settings_array) );
	mea->setElementType( domGl_pipeline_settings::registerElement(dae) );
	cm->appendChild( new daeMetaGroup( mea, meta, cm, 0, 1, 1 ) );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "shader" );
	mea->setOffset( daeOffsetOf(domProfile_CG::domTechnique::domPass,elemShader_array) );
	mea->setElementType( domProfile_CG::domTechnique::domPass::domShader::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 0 );
	cm->getParent()->appendChild( cm );
	cm = cm->getParent();

	mea = new daeMetaElementArrayAttribute( meta, cm, 3009, 0, -1 );
	mea->setName( "extra" );
	mea->setOffset( daeOffsetOf(domProfile_CG::domTechnique::domPass,elemExtra_array) );
	mea->setElementType( domExtra::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 3009 );
	meta->setCMRoot( cm );	
	// Ordered list of sub-elements
	meta->addContents(daeOffsetOf(domProfile_CG::domTechnique::domPass,_contents));
	meta->addContentsOrder(daeOffsetOf(domProfile_CG::domTechnique::domPass,_contentsOrder));

	meta->addCMDataArray(daeOffsetOf(domProfile_CG::domTechnique::domPass,_CMData), 1);
	//	Add attribute: sid
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "sid" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domProfile_CG::domTechnique::domPass , attrSid ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domProfile_CG::domTechnique::domPass));
	meta->validate();

	return meta;
}

daeElementRef
domProfile_CG::domTechnique::domPass::domDraw::create(DAE& dae)
{
	domProfile_CG::domTechnique::domPass::domDrawRef ref = new domProfile_CG::domTechnique::domPass::domDraw(dae);
	return ref;
}


daeMetaElement *
domProfile_CG::domTechnique::domPass::domDraw::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "draw" );
	meta->registerClass(domProfile_CG::domTechnique::domPass::domDraw::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("Fx_draw_common"));
		ma->setOffset( daeOffsetOf( domProfile_CG::domTechnique::domPass::domDraw , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domProfile_CG::domTechnique::domPass::domDraw));
	meta->validate();

	return meta;
}

daeElementRef
domProfile_CG::domTechnique::domPass::domShader::create(DAE& dae)
{
	domProfile_CG::domTechnique::domPass::domShaderRef ref = new domProfile_CG::domTechnique::domPass::domShader(dae);
	return ref;
}


daeMetaElement *
domProfile_CG::domTechnique::domPass::domShader::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "shader" );
	meta->registerClass(domProfile_CG::domTechnique::domPass::domShader::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 0, -1 );
	mea->setName( "annotate" );
	mea->setOffset( daeOffsetOf(domProfile_CG::domTechnique::domPass::domShader,elemAnnotate_array) );
	mea->setElementType( domFx_annotate_common::registerElement(dae) );
	cm->appendChild( mea );

	cm = new daeMetaSequence( meta, cm, 1, 0, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "compiler_target" );
	mea->setOffset( daeOffsetOf(domProfile_CG::domTechnique::domPass::domShader,elemCompiler_target) );
	mea->setElementType( domProfile_CG::domTechnique::domPass::domShader::domCompiler_target::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 1, 0, 1 );
	mea->setName( "compiler_options" );
	mea->setOffset( daeOffsetOf(domProfile_CG::domTechnique::domPass::domShader,elemCompiler_options) );
	mea->setElementType( domProfile_CG::domTechnique::domPass::domShader::domCompiler_options::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 1 );
	cm->getParent()->appendChild( cm );
	cm = cm->getParent();

	mea = new daeMetaElementAttribute( meta, cm, 3, 1, 1 );
	mea->setName( "name" );
	mea->setOffset( daeOffsetOf(domProfile_CG::domTechnique::domPass::domShader,elemName) );
	mea->setElementType( domProfile_CG::domTechnique::domPass::domShader::domName::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementArrayAttribute( meta, cm, 4, 0, -1 );
	mea->setName( "bind" );
	mea->setOffset( daeOffsetOf(domProfile_CG::domTechnique::domPass::domShader,elemBind_array) );
	mea->setElementType( domProfile_CG::domTechnique::domPass::domShader::domBind::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 4 );
	meta->setCMRoot( cm );	

	//	Add attribute: stage
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "stage" );
		ma->setType( dae.getAtomicTypes().get("Cg_pipeline_stage"));
		ma->setOffset( daeOffsetOf( domProfile_CG::domTechnique::domPass::domShader , attrStage ));
		ma->setContainer( meta );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domProfile_CG::domTechnique::domPass::domShader));
	meta->validate();

	return meta;
}

daeElementRef
domProfile_CG::domTechnique::domPass::domShader::domCompiler_target::create(DAE& dae)
{
	domProfile_CG::domTechnique::domPass::domShader::domCompiler_targetRef ref = new domProfile_CG::domTechnique::domPass::domShader::domCompiler_target(dae);
	return ref;
}


daeMetaElement *
domProfile_CG::domTechnique::domPass::domShader::domCompiler_target::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "compiler_target" );
	meta->registerClass(domProfile_CG::domTechnique::domPass::domShader::domCompiler_target::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("xsNMTOKEN"));
		ma->setOffset( daeOffsetOf( domProfile_CG::domTechnique::domPass::domShader::domCompiler_target , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domProfile_CG::domTechnique::domPass::domShader::domCompiler_target));
	meta->validate();

	return meta;
}

daeElementRef
domProfile_CG::domTechnique::domPass::domShader::domCompiler_options::create(DAE& dae)
{
	domProfile_CG::domTechnique::domPass::domShader::domCompiler_optionsRef ref = new domProfile_CG::domTechnique::domPass::domShader::domCompiler_options(dae);
	return ref;
}


daeMetaElement *
domProfile_CG::domTechnique::domPass::domShader::domCompiler_options::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "compiler_options" );
	meta->registerClass(domProfile_CG::domTechnique::domPass::domShader::domCompiler_options::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("xsString"));
		ma->setOffset( daeOffsetOf( domProfile_CG::domTechnique::domPass::domShader::domCompiler_options , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domProfile_CG::domTechnique::domPass::domShader::domCompiler_options));
	meta->validate();

	return meta;
}

daeElementRef
domProfile_CG::domTechnique::domPass::domShader::domName::create(DAE& dae)
{
	domProfile_CG::domTechnique::domPass::domShader::domNameRef ref = new domProfile_CG::domTechnique::domPass::domShader::domName(dae);
	return ref;
}


daeMetaElement *
domProfile_CG::domTechnique::domPass::domShader::domName::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "name" );
	meta->registerClass(domProfile_CG::domTechnique::domPass::domShader::domName::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domProfile_CG::domTechnique::domPass::domShader::domName , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	//	Add attribute: source
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "source" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domProfile_CG::domTechnique::domPass::domShader::domName , attrSource ));
		ma->setContainer( meta );
		ma->setIsRequired( false );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domProfile_CG::domTechnique::domPass::domShader::domName));
	meta->validate();

	return meta;
}

daeElementRef
domProfile_CG::domTechnique::domPass::domShader::domBind::create(DAE& dae)
{
	domProfile_CG::domTechnique::domPass::domShader::domBindRef ref = new domProfile_CG::domTechnique::domPass::domShader::domBind(dae);
	return ref;
}


daeMetaElement *
domProfile_CG::domTechnique::domPass::domShader::domBind::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "bind" );
	meta->registerClass(domProfile_CG::domTechnique::domPass::domShader::domBind::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaChoice( meta, cm, 0, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "cg_param_type" );
	mea->setOffset( daeOffsetOf(domProfile_CG::domTechnique::domPass::domShader::domBind,elemCg_param_type) );
	mea->setElementType( domCg_param_type::registerElement(dae) );
	cm->appendChild( new daeMetaGroup( mea, meta, cm, 0, 1, 1 ) );

	mea = new daeMetaElementAttribute( meta, cm, 0, 1, 1 );
	mea->setName( "param" );
	mea->setOffset( daeOffsetOf(domProfile_CG::domTechnique::domPass::domShader::domBind,elemParam) );
	mea->setElementType( domProfile_CG::domTechnique::domPass::domShader::domBind::domParam::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 0 );
	meta->setCMRoot( cm );	
	// Ordered list of sub-elements
	meta->addContents(daeOffsetOf(domProfile_CG::domTechnique::domPass::domShader::domBind,_contents));
	meta->addContentsOrder(daeOffsetOf(domProfile_CG::domTechnique::domPass::domShader::domBind,_contentsOrder));

	meta->addCMDataArray(daeOffsetOf(domProfile_CG::domTechnique::domPass::domShader::domBind,_CMData), 1);
	//	Add attribute: symbol
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "symbol" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domProfile_CG::domTechnique::domPass::domShader::domBind , attrSymbol ));
		ma->setContainer( meta );
		ma->setIsRequired( true );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domProfile_CG::domTechnique::domPass::domShader::domBind));
	meta->validate();

	return meta;
}

daeElementRef
domProfile_CG::domTechnique::domPass::domShader::domBind::domParam::create(DAE& dae)
{
	domProfile_CG::domTechnique::domPass::domShader::domBind::domParamRef ref = new domProfile_CG::domTechnique::domPass::domShader::domBind::domParam(dae);
	return ref;
}


daeMetaElement *
domProfile_CG::domTechnique::domPass::domShader::domBind::domParam::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "param" );
	meta->registerClass(domProfile_CG::domTechnique::domPass::domShader::domBind::domParam::create);

	meta->setIsInnerClass( true );

	//	Add attribute: ref
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "ref" );
		ma->setType( dae.getAtomicTypes().get("xsNCName"));
		ma->setOffset( daeOffsetOf( domProfile_CG::domTechnique::domPass::domShader::domBind::domParam , attrRef ));
		ma->setContainer( meta );
		ma->setIsRequired( true );
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domProfile_CG::domTechnique::domPass::domShader::domBind::domParam));
	meta->validate();

	return meta;
}

