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
#include <dae/daeMetaElement.h>
#include <dae/daeElement.h>
#include <dae/daeDocument.h>
#include <dae/domAny.h>
#include <dae/daeMetaCMPolicy.h>
#include <dae/daeMetaElementAttribute.h>

daeElementRef
daeMetaElement::create() 
{
	daeElementRef ret =  (*_createFunc)(dae);
	ret->setup(this);
		
	return ret;
}

daeElementRef
daeMetaElement::create(daeString s)
{
	daeMetaElement* me = NULL;
	if ( strcmp( s, _name ) == 0 ) {
		//looking for this meta
		me = this;
	}
	else if ( _contentModel != NULL ) {
		me = _contentModel->findChild(s);
	}
	if (me != NULL) {
		daeElementRef ret = me->create();
		if ( strcmp(s, me->getName() ) != 0 ) {
			ret->setElementName(s);
		}
		return ret;
	}
	if ( getAllowsAny() ) {
		daeElementRef ret = domAny::registerElement(dae)->create();
		ret->setElementName(s);
		return ret;
	}
	return NULL;
}

daeMetaElement::daeMetaElement(DAE& dae) : dae(dae)
{
	_name = "noname";
	_createFunc = NULL;
	_elementSize = sizeof(daeElement);
	_metaValue = NULL;
	_metaContents = NULL;
	_metaContentsOrder = NULL; // sthomas
	_metaID = NULL;
	_isTrackableForQueries = true;
	_usesStringContents = false;
	_isTransparent = false;
	_isAbstract = false;
	_allowsAny = false;
	_innerClass = false;
	_contentModel = NULL;
	_metaCMData = NULL;
	_numMetaChoices = 0;
}

daeMetaElement::~daeMetaElement()
{
	delete _metaContents;
	delete _contentModel;
	delete _metaContentsOrder;
	delete _metaCMData;
}

DAE* daeMetaElement::getDAE() {
	return &dae;
}

void daeMetaElement::setCMRoot( daeMetaCMPolicy *cm )
{
	if (_contentModel) 
		delete _contentModel;
	_contentModel = cm;
}

void
daeMetaElement::addContents(daeInt offset)
{
	daeMetaElementArrayAttribute* meaa = new daeMetaElementArrayAttribute( this, NULL, 0, 1, -1 );
	meaa->setType(dae.getAtomicTypes().get("element"));
	meaa->setName("contents");
	meaa->setOffset(offset);
	meaa->setContainer( this);
	_metaContents = meaa;
}
void
daeMetaElement::addContentsOrder(daeInt offset)
{
	daeMetaArrayAttribute* meaa = new daeMetaArrayAttribute();
	meaa->setType(dae.getAtomicTypes().get("uint"));
	meaa->setName("contentsOrder");
	meaa->setOffset(offset);
	meaa->setContainer( this);

    if (_metaContentsOrder)
        delete _metaContentsOrder;

	_metaContentsOrder = meaa;
}

void daeMetaElement::addCMDataArray(daeInt offset, daeUInt numChoices)
{
	daeMetaArrayAttribute* meaa = new daeMetaArrayAttribute();
	meaa->setType(dae.getAtomicTypes().get("int"));
	meaa->setName("CMData");
	meaa->setOffset(offset);
	meaa->setContainer( this);

    if (_metaCMData)
        delete _metaCMData;

	_metaCMData = meaa;

	_numMetaChoices = numChoices;
}


/*void
daeMetaElement::appendArrayElement(daeMetaElement* element, daeInt offset, daeString name)
{
	daeMetaElementArrayAttribute* meaa = new daeMetaElementArrayAttribute;
	meaa->setType(daeAtomicType::get("element"));
	if ( name ) {
		meaa->setName(name);
	}
	else {
		meaa->setName(element->getName());
	}
	meaa->setOffset(offset);
	meaa->setContainer(this);
	meaa->setElementType( element);
	_metaElements.append(meaa);
}
void
daeMetaElement::appendElement(daeMetaElement* element, daeInt offset, daeString name)
{
	daeMetaElementAttribute* meaa = new daeMetaElementAttribute;
	meaa->setType(daeAtomicType::get("element"));
	if ( name ) {
		meaa->setName(name);
	}
	else {
		meaa->setName(element->getName());
	}
	meaa->setOffset( offset);
	meaa->setContainer( this );
	meaa->setElementType( element );
	_metaElements.append(meaa);
}*/

void
daeMetaElement::appendAttribute(daeMetaAttribute* attr)
{
	if (attr == NULL)
		return;

	if (strcmp(attr->getName(),"_value") == 0) {
		_metaValue = attr;
	}
	else
		_metaAttributes.append(attr);

	if ((attr->getName() != NULL) &&
		(strcmp(attr->getName(),"id") == 0)) {
		_metaID = attr;
		_isTrackableForQueries = true;
	}
}

void
daeMetaElement::validate()
{
	if (_elementSize == 0)
	{
		daeInt place=0;
		unsigned int i;
		for(i=0;i<_metaAttributes.getCount();i++) {
			place += _metaAttributes[i]->getSize();
			int align = _metaAttributes[i]->getAlignment();
			place += align;
			place &= (~(align-1));
		}
		_elementSize = place;
	}
}
	
daeMetaAttribute*
daeMetaElement::getMetaAttribute(daeString s)
{
	int cnt = (int)_metaAttributes.getCount();
	int i;
	for(i=0;i<cnt;i++)
		if (strcmp(_metaAttributes[i]->getName(),s) == 0)
			return _metaAttributes[i];
	return NULL;
}


// void daeMetaElement::releaseMetas()
// {
// 	_metas().clear();
// 	size_t count = _classMetaPointers().getCount();
// 	for ( size_t i = 0; i < count; i++ )
// 	{
// 		*(_classMetaPointers()[i]) = NULL;
// 	}
// 	_classMetaPointers().clear();
// 	if (mera)
// 	{
// 		delete mera;
// 		mera = NULL;
// 	}
// 	if (mes)
// 	{
// 		delete mes;
// 		mes = NULL;
// 	}
// }

daeBool daeMetaElement::place(daeElement *parent, daeElement *child, daeUInt *ordinal )
{
	if (child->getMeta()->getIsAbstract() || parent->getMeta() != this ) {
		return false;
	}
	daeUInt ord;
	daeElement *retVal = _contentModel->placeElement( parent, child, ord );
	if ( retVal != NULL ) {
		//update document pointer
		child->setDocument( parent->getDocument() );
		retVal->setDocument( parent->getDocument() );
		//add to _contents array
		if (_metaContents != NULL) {
			daeElementRefArray* contents =
				(daeElementRefArray*)_metaContents->getWritableMemory(parent);
			daeUIntArray* contentsOrder =
				(daeUIntArray*)_metaContentsOrder->getWritableMemory(parent);
			daeBool needsAppend = true;
			size_t cnt = contentsOrder->getCount();
			for ( size_t x = 0; x < cnt; x++ ) {
				if ( contentsOrder->get(x) > ord ) {
					contents->insertAt( x, retVal );
					contentsOrder->insertAt( x, ord );
					needsAppend = false;
					break;
				}
			}
			if ( needsAppend ) {
				contents->append(retVal);
				contentsOrder->append( ord );
			}
		}
		if ( ordinal != NULL ) {
			*ordinal = ord;
		}
	}
	return retVal!=NULL;
}

daeBool daeMetaElement::placeAt( daeInt index, daeElement *parent, daeElement *child )
{
	if (child->getMeta()->getIsAbstract() || parent->getMeta() != this || index < 0 ) {
		return false;
	}
	daeUInt ord;
	daeElement *retVal = _contentModel->placeElement( parent, child, ord );
	if ( retVal != NULL ) {
		//add to _contents array
		if (_metaContents != NULL) {
			daeElementRefArray* contents =
				(daeElementRefArray*)_metaContents->getWritableMemory(parent);
			daeUIntArray* contentsOrder =
				(daeUIntArray*)_metaContentsOrder->getWritableMemory(parent);
			daeBool validLoc;
			if ( index > 0 ) {
				validLoc = contentsOrder->get(index) >= ord && contentsOrder->get(index) <= ord;
			}
			else {
				if ( contentsOrder->getCount() == 0 ) {
					validLoc = true;
				}
				else {
					validLoc = contentsOrder->get(index) >= ord;
				}
			}
			if ( validLoc ) {
				contents->insertAt( index, retVal );
				contentsOrder->insertAt( index, ord );
			}
			else {
				_contentModel->removeElement( parent, retVal );
				retVal = NULL;
			}
		}
	}
	if ( retVal != NULL ) {
		//update document pointer
		child->setDocument( parent->getDocument() );
		retVal->setDocument( parent->getDocument() );
	}
	return retVal!=NULL;
}

daeBool daeMetaElement::placeBefore( daeElement *marker, daeElement *parent, daeElement *child, daeUInt *ordinal )
{
	if (child->getMeta()->getIsAbstract() || parent->getMeta() != this ) {
		return false;
	}
	daeUInt ord;
	daeElement *retVal = _contentModel->placeElement( parent, child, ord, 0, marker, NULL );
	if ( retVal != NULL ) {
		//add to _contents array
		if (_metaContents != NULL) {
			daeElementRefArray* contents =
				(daeElementRefArray*)_metaContents->getWritableMemory(parent);
			daeUIntArray* contentsOrder =
				(daeUIntArray*)_metaContentsOrder->getWritableMemory(parent);
			size_t index(0);
			daeBool validLoc = false;
			if ( contents->find( marker, index ) == DAE_OK ) {
				if ( index > 0 ) {
					daeUInt gt = contentsOrder->get(index-1);
					daeUInt lt = contentsOrder->get(index);
					validLoc = gt <= ord && lt >= ord;
				}
				else {
					validLoc = contentsOrder->get(index) >= ord;
				}
			}
			if ( validLoc ) {
				contents->insertAt( index, retVal );
				contentsOrder->insertAt( index, ord );
				if ( ordinal != NULL ) {
					*ordinal = ord;
				}
			}
			else {
				_contentModel->removeElement( parent, retVal );
				retVal = NULL;
			}
		}
	}
	if ( retVal != NULL ) {
		//update document pointer
		child->setDocument( parent->getDocument() );
		retVal->setDocument( parent->getDocument() );
	}
	return retVal!=NULL;
}

daeBool daeMetaElement::placeAfter( daeElement *marker, daeElement *parent, daeElement *child, daeUInt *ordinal )
{
	if (child->getMeta()->getIsAbstract() || parent->getMeta() != this ) {
		return false;
	}
	daeUInt ord;
	daeElement *retVal = _contentModel->placeElement( parent, child, ord, 0, NULL, marker );
	if ( retVal != NULL ) {
		//add to _contents array
		if (_metaContents != NULL) {
			daeElementRefArray* contents =
				(daeElementRefArray*)_metaContents->getWritableMemory(parent);
			daeUIntArray* contentsOrder =
				(daeUIntArray*)_metaContentsOrder->getWritableMemory(parent);
			size_t index(0);
			daeBool validLoc = false;
			if ( contents->find( marker, index ) == DAE_OK ) {
				if ( index < contentsOrder->getCount()-1 ) {
					validLoc = contentsOrder->get(index) <= ord && contentsOrder->get(index+1) >= ord;
				}
				else {
					validLoc = contentsOrder->get(index) <= ord;
				}
			}
			if ( validLoc ) {
				contents->insertAt( index+1, retVal );
				contentsOrder->insertAt( index+1, ord );
				if ( ordinal != NULL ) {
					*ordinal = ord;
				}
			}
			else {
				_contentModel->removeElement( parent, retVal );
				retVal = NULL;
			}
		}
	}
	if ( retVal != NULL ) {
		//update document pointer
		child->setDocument( parent->getDocument() );
		retVal->setDocument( parent->getDocument() );
	}
	return retVal!=NULL;
}

daeBool daeMetaElement::remove(daeElement *parent, daeElement *child)
{
	if ( parent->getMeta() != this ) {
		return false;
	}
	//prevent child from being deleted
	daeElementRef el( child );
	if ( _contentModel->removeElement( parent, child ) ) {
		if ( _metaContents != NULL) 
		{
			daeElementRefArray* contents = (daeElementRefArray*)_metaContents->getWritableMemory(parent);
			daeUIntArray* contentsOrder = (daeUIntArray*)_metaContentsOrder->getWritableMemory(parent);
			size_t idx(0);
			if ( contents->remove(child, &idx) == DAE_OK ) {
				contentsOrder->removeIndex( idx );
			}
		}
		if ( child->getDocument() ) {
			child->getDocument()->removeElement( child );
		}

		// Clear the child's parent pointer
		child->setParentElement( NULL );

		return true;
	}
	return false;
}

void daeMetaElement::getChildren( daeElement* parent, daeElementRefArray &array )
{
	if ( parent->getMeta() != this ) {
		return;
	}
	if ( _metaContents != NULL ) {
		daeElementRefArray* contents = (daeElementRefArray*)_metaContents->getWritableMemory(parent);
		for ( size_t x = 0; x < contents->getCount(); x++ ) {
			array.append( contents->get(x) );
		}
	}
	else if ( _contentModel != NULL ) {
		_contentModel->getChildren( parent, array );
	}
}

// daeMetaElementRefArray &daeMetaElement::_metas()
// {
// 	if (!mera)
// 	{
// 		mera = new daeMetaElementRefArray();
// 	}
// 	return *mera;
// }

// daeTArray< daeMetaElement** > &daeMetaElement::_classMetaPointers()
// {
// 	if (!mes)
// 	{
// 		mes = new daeTArray< daeMetaElement** >();
// 	}
// 	return *mes;
// }

