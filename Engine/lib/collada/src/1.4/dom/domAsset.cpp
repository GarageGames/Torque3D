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
#include <dom/domAsset.h>
#include <dae/daeMetaCMPolicy.h>
#include <dae/daeMetaSequence.h>
#include <dae/daeMetaChoice.h>
#include <dae/daeMetaGroup.h>
#include <dae/daeMetaAny.h>
#include <dae/daeMetaElementAttribute.h>

daeElementRef
domAsset::create(DAE& dae)
{
	domAssetRef ref = new domAsset(dae);
	return ref;
}


daeMetaElement *
domAsset::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "asset" );
	meta->registerClass(domAsset::create);

	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementArrayAttribute( meta, cm, 0, 0, -1 );
	mea->setName( "contributor" );
	mea->setOffset( daeOffsetOf(domAsset,elemContributor_array) );
	mea->setElementType( domAsset::domContributor::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 1, 1, 1 );
	mea->setName( "created" );
	mea->setOffset( daeOffsetOf(domAsset,elemCreated) );
	mea->setElementType( domAsset::domCreated::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 2, 0, 1 );
	mea->setName( "keywords" );
	mea->setOffset( daeOffsetOf(domAsset,elemKeywords) );
	mea->setElementType( domAsset::domKeywords::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 3, 1, 1 );
	mea->setName( "modified" );
	mea->setOffset( daeOffsetOf(domAsset,elemModified) );
	mea->setElementType( domAsset::domModified::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 4, 0, 1 );
	mea->setName( "revision" );
	mea->setOffset( daeOffsetOf(domAsset,elemRevision) );
	mea->setElementType( domAsset::domRevision::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 5, 0, 1 );
	mea->setName( "subject" );
	mea->setOffset( daeOffsetOf(domAsset,elemSubject) );
	mea->setElementType( domAsset::domSubject::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 6, 0, 1 );
	mea->setName( "title" );
	mea->setOffset( daeOffsetOf(domAsset,elemTitle) );
	mea->setElementType( domAsset::domTitle::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 7, 0, 1 );
	mea->setName( "unit" );
	mea->setOffset( daeOffsetOf(domAsset,elemUnit) );
	mea->setElementType( domAsset::domUnit::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 8, 0, 1 );
	mea->setName( "up_axis" );
	mea->setOffset( daeOffsetOf(domAsset,elemUp_axis) );
	mea->setElementType( domAsset::domUp_axis::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 8 );
	meta->setCMRoot( cm );	

	meta->setElementSize(sizeof(domAsset));
	meta->validate();

	return meta;
}

daeElementRef
domAsset::domContributor::create(DAE& dae)
{
	domAsset::domContributorRef ref = new domAsset::domContributor(dae);
	return ref;
}


daeMetaElement *
domAsset::domContributor::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "contributor" );
	meta->registerClass(domAsset::domContributor::create);

	meta->setIsInnerClass( true );
	daeMetaCMPolicy *cm = NULL;
	daeMetaElementAttribute *mea = NULL;
	cm = new daeMetaSequence( meta, cm, 0, 1, 1 );

	mea = new daeMetaElementAttribute( meta, cm, 0, 0, 1 );
	mea->setName( "author" );
	mea->setOffset( daeOffsetOf(domAsset::domContributor,elemAuthor) );
	mea->setElementType( domAsset::domContributor::domAuthor::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 1, 0, 1 );
	mea->setName( "authoring_tool" );
	mea->setOffset( daeOffsetOf(domAsset::domContributor,elemAuthoring_tool) );
	mea->setElementType( domAsset::domContributor::domAuthoring_tool::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 2, 0, 1 );
	mea->setName( "comments" );
	mea->setOffset( daeOffsetOf(domAsset::domContributor,elemComments) );
	mea->setElementType( domAsset::domContributor::domComments::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 3, 0, 1 );
	mea->setName( "copyright" );
	mea->setOffset( daeOffsetOf(domAsset::domContributor,elemCopyright) );
	mea->setElementType( domAsset::domContributor::domCopyright::registerElement(dae) );
	cm->appendChild( mea );

	mea = new daeMetaElementAttribute( meta, cm, 4, 0, 1 );
	mea->setName( "source_data" );
	mea->setOffset( daeOffsetOf(domAsset::domContributor,elemSource_data) );
	mea->setElementType( domAsset::domContributor::domSource_data::registerElement(dae) );
	cm->appendChild( mea );

	cm->setMaxOrdinal( 4 );
	meta->setCMRoot( cm );	

	meta->setElementSize(sizeof(domAsset::domContributor));
	meta->validate();

	return meta;
}

daeElementRef
domAsset::domContributor::domAuthor::create(DAE& dae)
{
	domAsset::domContributor::domAuthorRef ref = new domAsset::domContributor::domAuthor(dae);
	return ref;
}


daeMetaElement *
domAsset::domContributor::domAuthor::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "author" );
	meta->registerClass(domAsset::domContributor::domAuthor::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("xsString"));
		ma->setOffset( daeOffsetOf( domAsset::domContributor::domAuthor , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domAsset::domContributor::domAuthor));
	meta->validate();

	return meta;
}

daeElementRef
domAsset::domContributor::domAuthoring_tool::create(DAE& dae)
{
	domAsset::domContributor::domAuthoring_toolRef ref = new domAsset::domContributor::domAuthoring_tool(dae);
	return ref;
}


daeMetaElement *
domAsset::domContributor::domAuthoring_tool::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "authoring_tool" );
	meta->registerClass(domAsset::domContributor::domAuthoring_tool::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("xsString"));
		ma->setOffset( daeOffsetOf( domAsset::domContributor::domAuthoring_tool , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domAsset::domContributor::domAuthoring_tool));
	meta->validate();

	return meta;
}

daeElementRef
domAsset::domContributor::domComments::create(DAE& dae)
{
	domAsset::domContributor::domCommentsRef ref = new domAsset::domContributor::domComments(dae);
	return ref;
}


daeMetaElement *
domAsset::domContributor::domComments::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "comments" );
	meta->registerClass(domAsset::domContributor::domComments::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("xsString"));
		ma->setOffset( daeOffsetOf( domAsset::domContributor::domComments , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domAsset::domContributor::domComments));
	meta->validate();

	return meta;
}

daeElementRef
domAsset::domContributor::domCopyright::create(DAE& dae)
{
	domAsset::domContributor::domCopyrightRef ref = new domAsset::domContributor::domCopyright(dae);
	return ref;
}


daeMetaElement *
domAsset::domContributor::domCopyright::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "copyright" );
	meta->registerClass(domAsset::domContributor::domCopyright::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("xsString"));
		ma->setOffset( daeOffsetOf( domAsset::domContributor::domCopyright , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domAsset::domContributor::domCopyright));
	meta->validate();

	return meta;
}

daeElementRef
domAsset::domContributor::domSource_data::create(DAE& dae)
{
	domAsset::domContributor::domSource_dataRef ref = new domAsset::domContributor::domSource_data(dae);
	return ref;
}


daeMetaElement *
domAsset::domContributor::domSource_data::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "source_data" );
	meta->registerClass(domAsset::domContributor::domSource_data::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("xsAnyURI"));
		ma->setOffset( daeOffsetOf( domAsset::domContributor::domSource_data , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domAsset::domContributor::domSource_data));
	meta->validate();

	return meta;
}

daeElementRef
domAsset::domCreated::create(DAE& dae)
{
	domAsset::domCreatedRef ref = new domAsset::domCreated(dae);
	return ref;
}


daeMetaElement *
domAsset::domCreated::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "created" );
	meta->registerClass(domAsset::domCreated::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("xsDateTime"));
		ma->setOffset( daeOffsetOf( domAsset::domCreated , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domAsset::domCreated));
	meta->validate();

	return meta;
}

daeElementRef
domAsset::domKeywords::create(DAE& dae)
{
	domAsset::domKeywordsRef ref = new domAsset::domKeywords(dae);
	return ref;
}


daeMetaElement *
domAsset::domKeywords::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "keywords" );
	meta->registerClass(domAsset::domKeywords::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("xsString"));
		ma->setOffset( daeOffsetOf( domAsset::domKeywords , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domAsset::domKeywords));
	meta->validate();

	return meta;
}

daeElementRef
domAsset::domModified::create(DAE& dae)
{
	domAsset::domModifiedRef ref = new domAsset::domModified(dae);
	return ref;
}


daeMetaElement *
domAsset::domModified::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "modified" );
	meta->registerClass(domAsset::domModified::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("xsDateTime"));
		ma->setOffset( daeOffsetOf( domAsset::domModified , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domAsset::domModified));
	meta->validate();

	return meta;
}

daeElementRef
domAsset::domRevision::create(DAE& dae)
{
	domAsset::domRevisionRef ref = new domAsset::domRevision(dae);
	return ref;
}


daeMetaElement *
domAsset::domRevision::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "revision" );
	meta->registerClass(domAsset::domRevision::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("xsString"));
		ma->setOffset( daeOffsetOf( domAsset::domRevision , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domAsset::domRevision));
	meta->validate();

	return meta;
}

daeElementRef
domAsset::domSubject::create(DAE& dae)
{
	domAsset::domSubjectRef ref = new domAsset::domSubject(dae);
	return ref;
}


daeMetaElement *
domAsset::domSubject::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "subject" );
	meta->registerClass(domAsset::domSubject::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("xsString"));
		ma->setOffset( daeOffsetOf( domAsset::domSubject , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domAsset::domSubject));
	meta->validate();

	return meta;
}

daeElementRef
domAsset::domTitle::create(DAE& dae)
{
	domAsset::domTitleRef ref = new domAsset::domTitle(dae);
	return ref;
}


daeMetaElement *
domAsset::domTitle::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "title" );
	meta->registerClass(domAsset::domTitle::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("xsString"));
		ma->setOffset( daeOffsetOf( domAsset::domTitle , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domAsset::domTitle));
	meta->validate();

	return meta;
}

daeElementRef
domAsset::domUnit::create(DAE& dae)
{
	domAsset::domUnitRef ref = new domAsset::domUnit(dae);
	return ref;
}


daeMetaElement *
domAsset::domUnit::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "unit" );
	meta->registerClass(domAsset::domUnit::create);

	meta->setIsInnerClass( true );

	//	Add attribute: meter
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "meter" );
		ma->setType( dae.getAtomicTypes().get("Float"));
		ma->setOffset( daeOffsetOf( domAsset::domUnit , attrMeter ));
		ma->setContainer( meta );
		ma->setDefaultString( "1.0");
	
		meta->appendAttribute(ma);
	}

	//	Add attribute: name
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "name" );
		ma->setType( dae.getAtomicTypes().get("xsNMTOKEN"));
		ma->setOffset( daeOffsetOf( domAsset::domUnit , attrName ));
		ma->setContainer( meta );
		ma->setDefaultString( "meter");
	
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domAsset::domUnit));
	meta->validate();

	return meta;
}

daeElementRef
domAsset::domUp_axis::create(DAE& dae)
{
	domAsset::domUp_axisRef ref = new domAsset::domUp_axis(dae);
	return ref;
}


daeMetaElement *
domAsset::domUp_axis::registerElement(DAE& dae)
{
	daeMetaElement* meta = dae.getMeta(ID());
	if ( meta != NULL ) return meta;

	meta = new daeMetaElement(dae);
	dae.setMeta(ID(), *meta);
	meta->setName( "up_axis" );
	meta->registerClass(domAsset::domUp_axis::create);

	meta->setIsInnerClass( true );
	//	Add attribute: _value
	{
		daeMetaAttribute *ma = new daeMetaAttribute;
		ma->setName( "_value" );
		ma->setType( dae.getAtomicTypes().get("UpAxisType"));
		ma->setOffset( daeOffsetOf( domAsset::domUp_axis , _value ));
		ma->setContainer( meta );
		meta->appendAttribute(ma);
	}

	meta->setElementSize(sizeof(domAsset::domUp_axis));
	meta->validate();

	return meta;
}

