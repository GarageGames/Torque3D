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

#include <iomanip>
#include <dae/daeElement.h>
#include <dae/daeArray.h>
#include <dae/daeMetaAttribute.h>
#include <dae/daeMetaElementAttribute.h>
#include <dae/daeMetaElement.h>
#include <dae/daeDatabase.h>
#include <dae/daeErrorHandler.h>
#include <dae/daeURI.h>
#include <dae/domAny.h>
#include <dae/daeUtils.h>

using namespace std;

daeElement* daeElement::simpleAdd(daeString name, int index) {
	if (daeElementRef elt = _meta->create(name))
		return add(elt, index);
	return NULL;
}

daeElement* daeElement::add(daeString names_, int index) {
	list<string> names;
	cdom::tokenize(names_, " ", names);
	cdom::tokenIter iter = names.begin();
	daeElement* root = simpleAdd(iter->c_str(), index);
	if (!root)
		return NULL;

	iter++;
	daeElement* elt = root;
	for (; iter != names.end(); iter++) {
		elt = elt->simpleAdd(iter->c_str());
		if (!elt) {
			removeChildElement(root);
			return NULL;
		}
	}

	return elt;
}

daeElement* daeElement::add(daeElement* elt, int index) {
	if (!elt)
		return NULL;
	if (elt == this)
		return this;
	bool result = (index == -1 ? _meta->place(this, elt) : _meta->placeAt(index, this, elt));
	return result ? elt : NULL;
}

daeElement* daeElement::addBefore(daeElement* elt, daeElement* index) {
	if (!index || !elt || index->getParent() != this)
		return NULL;
	return _meta->placeBefore(index, this, elt) ? elt : NULL;
}

daeElement* daeElement::addAfter(daeElement* elt, daeElement* index) {
	if (!index || !elt || index->getParent() != this)
		return NULL;
	return _meta->placeAfter(index, this, elt) ? elt : NULL;
}

daeElementRef
daeElement::createElement(daeString className)
{
	daeElementRef elem = _meta->create(className);
	// Bug #225 work around
//	if ( elem != NULL)
//		elem->ref(); // change premature delete into memory leak.
	return elem;
}

daeElement* daeElement::createAndPlace(daeString className) {
	return add(className);
}

daeElement* daeElement::createAndPlaceAt(daeInt index, daeString className) {
	return add(className, index);
}

daeBool daeElement::placeElement(daeElement* e) {
	return add(e) != NULL;
}

daeBool daeElement::placeElementAt(daeInt index, daeElement* e) {
	return add(e, index) != NULL;
}

daeBool daeElement::placeElementBefore( daeElement *marker, daeElement *element ) {
	return addBefore(element, marker) != NULL;
}

daeBool daeElement::placeElementAfter( daeElement *marker, daeElement *element ) {
	return addAfter(element, marker) != NULL;
}

daeInt daeElement::findLastIndexOf( daeString elementName ) {
	if ( _meta->getContents() != NULL ) {
		daeElementRefArray* contents =
						(daeElementRefArray*)_meta->getContents()->getWritableMemory(this);
		for ( int i = (int)contents->getCount()-1; i >= 0; --i ) {
			if ( strcmp( contents->get(i)->getElementName(), elementName ) == 0 ) {
				return i;
			}
		}
	}
	return -1;
}

daeBool 
daeElement::removeChildElement(daeElement* element)
{
	// error traps
	if(element==NULL)
		return false;
	if(element->_parent != this)
		return false;

	return _meta->remove( this, element );
}

void daeElement::setDocument( daeDocument *c, bool notifyDocument ) {
	if( _document == c )
		return;

	// Notify our parent document if necessary.
	if ( _document != NULL && notifyDocument )
		_document->removeElement(this);
	_document = c;
	if ( _document != NULL && notifyDocument )
		_document->insertElement(this);

	// Notify our attributes
	daeMetaAttributeRefArray& metaAttrs = getMeta()->getMetaAttributes();
	for (size_t i = 0; i < metaAttrs.getCount(); i++)
		metaAttrs[i]->setDocument(this, c);

	// Notify our char data object
	if (getCharDataObject())
		getCharDataObject()->setDocument(this, c);
	
	// Notify our children
	daeElementRefArray ea;
	getChildren( ea );
	for ( size_t x = 0; x < ea.getCount(); x++ ) {
		// Since inserting and removing elements works recursively in the database,
		// we don't need to notify it about inserts/removals as we process the
		// children of this element.
		ea[x]->setDocument( c, false );
	}
}

void daeElement::deleteCMDataArray(daeTArray<daeCharArray*>& cmData) {
	for (unsigned int i = 0; i < cmData.getCount(); i++)
		delete cmData.get(i);
	cmData.clear();
}

size_t daeElement::getAttributeCount() {
	return getMeta()->getMetaAttributes().getCount();
}

namespace {
	// A helper function to get the index of an attribute given the attribute name.
	size_t getAttributeIndex(daeElement& el, daeString name) {
		if (el.getMeta()) {
			daeMetaAttributeRefArray& metaAttrs = el.getMeta()->getMetaAttributes();
			for (size_t i = 0; i < metaAttrs.getCount(); i++)
				if (metaAttrs[i]->getName()  &&  strcmp(metaAttrs[i]->getName(), name) == 0)
					return i;
		}
		return (size_t)-1;
	}
}

daeMetaAttribute* daeElement::getAttributeObject(size_t i) {
	daeMetaAttributeRefArray& attrs = getMeta()->getMetaAttributes();
	if (i >= attrs.getCount())
		return NULL;
	return attrs[i];
}	

daeMetaAttribute* daeElement::getAttributeObject(daeString name) {
	return getAttributeObject(getAttributeIndex(*this, name));
}

std::string daeElement::getAttributeName(size_t i) {
	if (daeMetaAttribute* attr = getAttributeObject(i))
		return (daeString)attr->getName();
	return "";
}

daeBool daeElement::hasAttribute(daeString name) {
	return getAttributeObject(name) != 0;
}

daeBool daeElement::isAttributeSet(daeString name) {
	size_t i = getAttributeIndex(*this, name);
	if (i != (size_t)-1)
		return _validAttributeArray[i];
	return false;
}

std::string daeElement::getAttribute(size_t i) {
	std::string value;
	getAttribute(i, value);
	return value;
}

void daeElement::getAttribute(size_t i, std::string& value) {
	value = "";
	if (daeMetaAttribute* attr = getAttributeObject(i)) {
		std::ostringstream buffer;
		attr->memoryToString(this, buffer);
		value = buffer.str();
	}
}

std::string daeElement::getAttribute(daeString name) {
	std::string value;
	getAttribute(name, value);
	return value;
}

void daeElement::getAttribute(daeString name, std::string& value) {
	getAttribute(getAttributeIndex(*this, name), value);
}

daeElement::attr::attr() { }
daeElement::attr::attr(const std::string& name, const std::string& value)
	: name(name), value(value) { }

daeTArray<daeElement::attr> daeElement::getAttributes() {
	daeTArray<daeElement::attr> attrs;
	getAttributes(attrs);
	return attrs;
}

void daeElement::getAttributes(daeTArray<attr>& attrs) {
	attrs.clear();
	for (size_t i = 0; i < getAttributeCount(); i++) {
		std::string value;
		getAttribute(i, value);
		attrs.append(attr(getAttributeName(i), value));
	}
}

daeBool daeElement::setAttribute(size_t i, daeString value) {
	if (daeMetaAttribute* attr = getAttributeObject(i)) {
		if (attr->getType()) {
			attr->stringToMemory(this, value);
			_validAttributeArray.set(i, true);
			return true;
		}
	}
	return false;
}

daeBool daeElement::setAttribute(daeString name, daeString value) {
	return setAttribute(getAttributeIndex(*this, name), value);
}

// Deprecated
daeMemoryRef daeElement::getAttributeValue(daeString name) {
	if (daeMetaAttribute* attr = getAttributeObject(name))
		return attr->get(this);
	return NULL;
}

daeMetaAttribute* daeElement::getCharDataObject() {
	if (_meta)
		return _meta->getValueAttribute();
	return NULL;
}

daeBool daeElement::hasCharData() {
	return getCharDataObject() != NULL;
}

std::string daeElement::getCharData() {
	std::string result;
	getCharData(result);
	return result;
}		

void daeElement::getCharData(std::string& data) {
	data = "";
	if (daeMetaAttribute* charDataAttr = getCharDataObject()) {
		std::ostringstream buffer;
		charDataAttr->memoryToString(this, buffer);
		data = buffer.str();
	}
}

daeBool daeElement::setCharData(const std::string& data) {
	if (daeMetaAttribute* charDataAttr = getCharDataObject()) {
		charDataAttr->stringToMemory(this, data.c_str());
		return true;
	}
	return false;
}

daeBool daeElement::hasValue() {
	return hasCharData();
}

daeMemoryRef daeElement::getValuePointer() {
	if (daeMetaAttribute* charDataAttr = getCharDataObject())
		return charDataAttr->get(this);
	return NULL;
}

void
daeElement::setup(daeMetaElement* meta)
{
	if (_meta)
		return;
	_meta = meta;
	daeMetaAttributeRefArray& attrs = meta->getMetaAttributes();
	int macnt = (int)attrs.getCount();

	_validAttributeArray.setCount(macnt, false);

	for (int i = 0; i < macnt; i++) {
		if (attrs[i]->getDefaultValue() != NULL)
			attrs[i]->copyDefault(this);
	}

	//set up the _CMData array if there is one
	if ( _meta->getMetaCMData() != NULL )
	{
		daeTArray< daeCharArray *> *CMData = (daeTArray< daeCharArray *>*)_meta->getMetaCMData()->getWritableMemory(this);
		CMData->setCount( _meta->getNumChoices() );
		for ( unsigned int i = 0; i < _meta->getNumChoices(); i++ )
		{
			CMData->set( i, new daeCharArray() );
		}
	}
}

void daeElement::init() {
	_parent = NULL;
	_document = NULL;
	_meta = NULL;
	_elementName = NULL;
	_userData = NULL;
}

daeElement::daeElement() {
	init();
}

daeElement::daeElement(DAE& dae) {
	init();
}

daeElement::~daeElement()
{
	if (_elementName) {
		delete[] _elementName;
		_elementName = NULL;
	}
}

//function used until we clarify what's a type and what's a name for an element
daeString daeElement::getTypeName() const
{
	return _meta->getName();
}
daeString daeElement::getElementName() const
{
	return _elementName ? _elementName : (daeString)_meta->getName();
}
void daeElement::setElementName( daeString nm ) {
	if ( nm == NULL ) {
		if ( _elementName ) delete[] _elementName;
		_elementName = NULL;
		return;
	}
	if ( !_elementName ) _elementName = new daeChar[128];
	strcpy( (char*)_elementName, nm );
}

daeString daeElement::getID() const {
	daeElement* this_ = const_cast<daeElement*>(this);
	if (_meta)
		if (daeMetaAttribute* idAttr = this_->getAttributeObject("id"))
			return *(daeStringRef*)idAttr->get(this_);
	return NULL;
}

daeElementRefArray daeElement::getChildren() {
	daeElementRefArray array;
	getChildren(array);
	return array;
}

void daeElement::getChildren( daeElementRefArray &array ) {
	_meta->getChildren( this, array );
}

daeSmartRef<daeElement> daeElement::clone(daeString idSuffix, daeString nameSuffix) {
	// Use the meta object system to create a new instance of this element. We need to
	// create a new meta if we're cloning a domAny object because domAnys never share meta objects.
	// Ideally we'd be able to clone the _meta for domAny objects. Then we wouldn't need
	// any additional special case code for cloning domAny. Unfortunately, we don't have a
	// daeMetaElement::clone method.
	bool any = typeID() == domAny::ID();
	daeElementRef ret = any ? domAny::registerElement(*getDAE())->create() : _meta->create();
	ret->setElementName( _elementName );

	// Copy the attributes and character data. Requires special care for domAny.
	if (any) {
		domAny* thisAny = (domAny*)this;
		domAny* retAny = (domAny*)ret.cast();
		for (daeUInt i = 0; i < (daeUInt)thisAny->getAttributeCount(); i++)
			retAny->setAttribute(thisAny->getAttributeName(i), thisAny->getAttributeValue(i));
		retAny->setValue(thisAny->getValue());
	} else {
		// Use the meta system to copy attributes
		daeMetaAttributeRefArray &attrs = _meta->getMetaAttributes();
		for (unsigned int i = 0; i < attrs.getCount(); i++) {
			attrs[i]->copy( ret, this );
			ret->_validAttributeArray[i] = _validAttributeArray[i];
		}
		if (daeMetaAttribute* valueAttr = getCharDataObject())
			valueAttr->copy( ret, this );
	}
	
	daeElementRefArray children;
	_meta->getChildren( this, children );
	for ( size_t x = 0; x < children.getCount(); x++ ) {
		ret->placeElement( children.get(x)->clone( idSuffix, nameSuffix ) );
	}

	// Mangle the id
	if (idSuffix) {
		std::string id = ret->getAttribute("id");
		if (!id.empty())
			ret->setAttribute("id", (id + idSuffix).c_str());
	}
	// Mangle the name
	if (nameSuffix) {
		std::string name = ret->getAttribute("name");
		if (!name.empty())
			ret->setAttribute("name", (name + nameSuffix).c_str());
	}
	return ret;
}


// Element comparison

namespace { // Utility functions
	int getNecessaryColumnWidth(const vector<string>& tokens) {
		int result = 0;
		for (size_t i = 0; i < tokens.size(); i++) {
			int tokenLength = int(tokens[i].length() > 0 ? tokens[i].length()+2 : 0);
			result = max(tokenLength, result);
		}
		return result;
	}

	string formatToken(const string& token) {
		if (token.length() <= 50)
			return token;
		return token.substr(0, 47) + "...";
	}
} // namespace {

daeElement::compareResult::compareResult()
	: compareValue(0),
	  elt1(NULL),
	  elt2(NULL),
	  nameMismatch(false),
	  attrMismatch(""),
	  charDataMismatch(false),
	  childCountMismatch(false) {
}

string daeElement::compareResult::format() {
	if (!elt1 || !elt2)
		return "";

	// Gather the data we'll be printing
	string name1 = formatToken(elt1->getElementName()),
	       name2 = formatToken(elt2->getElementName()),
	       type1 = formatToken(elt1->getTypeName()),
	       type2 = formatToken(elt2->getTypeName()),
	       id1 = formatToken(elt1->getAttribute("id")),
	       id2 = formatToken(elt2->getAttribute("id")),
	       attrName1 = formatToken(attrMismatch),
	       attrName2 = formatToken(attrMismatch),
	       attrValue1 = formatToken(elt1->getAttribute(attrMismatch.c_str())),
	       attrValue2 = formatToken(elt2->getAttribute(attrMismatch.c_str())),
	       charData1 = formatToken(elt1->getCharData()),
	       charData2 = formatToken(elt2->getCharData()),
	       childCount1 = formatToken(cdom::toString(elt1->getChildren().getCount())),
	       childCount2 = formatToken(cdom::toString(elt2->getChildren().getCount()));
		
	// Compute formatting information
	vector<string> col1Tokens = cdom::makeStringArray("Name", "Type", "ID",
		"Attr name", "Attr value", "Char data", "Child count", 0);
	vector<string> col2Tokens = cdom::makeStringArray("Element 1", name1.c_str(),
		type1.c_str(), id1.c_str(), attrName1.c_str(), attrValue1.c_str(),
		charData1.c_str(), childCount1.c_str(), 0);
		
	int c1w = getNecessaryColumnWidth(col1Tokens),
	    c2w = getNecessaryColumnWidth(col2Tokens);
	ostringstream msg;
	msg << setw(c1w) << left << ""            << setw(c2w) << left << "Element 1" << "Element 2\n"
			<< setw(c1w) << left << ""            << setw(c2w) << left << "---------" << "---------\n"
			<< setw(c1w) << left << "Name"        << setw(c2w) << left << name1 << name2 << endl
			<< setw(c1w) << left << "Type"        << setw(c2w) << left << type1 << type2 << endl
			<< setw(c1w) << left << "ID"          << setw(c2w) << left << id1 << id2 << endl
			<< setw(c1w) << left << "Attr name"   << setw(c2w) << left << attrName1 << attrName2 << endl
			<< setw(c1w) << left << "Attr value"  << setw(c2w) << left << attrValue1 << attrValue2 << endl
			<< setw(c1w) << left << "Char data"   << setw(c2w) << left << charData1 << charData2 << endl
			<< setw(c1w) << left << "Child count" << setw(c2w) << left << childCount1 << childCount2;

	return msg.str();
}

namespace {
	daeElement::compareResult compareMatch() {
		daeElement::compareResult result;
		result.compareValue = 0;
		return result;
	}

	daeElement::compareResult nameMismatch(daeElement& elt1, daeElement& elt2) {
		daeElement::compareResult result;
		result.elt1 = &elt1;
		result.elt2 = &elt2;
		result.compareValue = strcmp(elt1.getElementName(), elt2.getElementName());
		result.nameMismatch = true;
		return result;
	}

	daeElement::compareResult attrMismatch(daeElement& elt1, daeElement& elt2, const string& attr) {
		daeElement::compareResult result;
		result.elt1 = &elt1;
		result.elt2 = &elt2;
		result.compareValue = strcmp(elt1.getAttribute(attr.c_str()).c_str(),
		                             elt2.getAttribute(attr.c_str()).c_str());
		result.attrMismatch = attr;
		return result;
	}

	daeElement::compareResult charDataMismatch(daeElement& elt1, daeElement& elt2) {
		daeElement::compareResult result;
		result.elt1 = &elt1;
		result.elt2 = &elt2;
		result.compareValue = strcmp(elt1.getCharData().c_str(),
		                             elt2.getCharData().c_str());
		result.charDataMismatch = true;
		return result;
	}

	daeElement::compareResult childCountMismatch(daeElement& elt1, daeElement& elt2) {
		daeElement::compareResult result;
		result.elt1 = &elt1;
		result.elt2 = &elt2;
		daeElementRefArray children1 = elt1.getChildren(),
		                   children2 = elt2.getChildren();
		result.compareValue = int(children1.getCount()) - int(children2.getCount());
		result.childCountMismatch = true;
		return result;
	}

	daeElement::compareResult compareElementsSameType(daeElement& elt1, daeElement& elt2) {
		// Compare attributes
		for (size_t i = 0; i < elt1.getAttributeCount(); i++)
			if (elt1.getAttributeObject(i)->compare(&elt1, &elt2) != 0)
				return attrMismatch(elt1, elt2, elt1.getAttributeName(i));

		// Compare character data
		if (elt1.getCharDataObject())
			if (elt1.getCharDataObject()->compare(&elt1, &elt2) != 0)
				return charDataMismatch(elt1, elt2);

		// Compare children
		daeElementRefArray children1 = elt1.getChildren(),
		                   children2 = elt2.getChildren();
		if (children1.getCount() != children2.getCount())
			return childCountMismatch(elt1, elt2);
		for (size_t i = 0; i < children1.getCount(); i++) {
			daeElement::compareResult result = daeElement::compareWithFullResult(*children1[i], *children2[i]);
			if (result.compareValue != 0)
				return result;
		}
	
		return compareMatch();
	}

	daeElement::compareResult compareElementsDifferentTypes(daeElement& elt1, daeElement& elt2) {
		string value1, value2;

		// Compare attributes. Be careful because each element could have a
		// different number of attributes.
		if (elt1.getAttributeCount() > elt2.getAttributeCount())
			return attrMismatch(elt1, elt2, elt1.getAttributeName(elt2.getAttributeCount()));
		if (elt2.getAttributeCount() > elt1.getAttributeCount())
			return attrMismatch(elt1, elt2, elt2.getAttributeName(elt1.getAttributeCount()));
		for (size_t i = 0; i < elt1.getAttributeCount(); i++) {
			elt1.getAttribute(i, value1);
			elt2.getAttribute(elt1.getAttributeName(i).c_str(), value2);
			if (value1 != value2)
				return attrMismatch(elt1, elt2, elt1.getAttributeName(i));
		}

		// Compare character data
		elt1.getCharData(value1);
		elt2.getCharData(value2);
		if (value1 != value2)
			return charDataMismatch(elt1, elt2);

		// Compare children
		daeElementRefArray children1 = elt1.getChildren(),
		                   children2 = elt2.getChildren();
		if (children1.getCount() != children2.getCount())
			return childCountMismatch(elt1, elt2);
		for (size_t i = 0; i < children1.getCount(); i++) {
			daeElement::compareResult result = daeElement::compareWithFullResult(*children1[i], *children2[i]);
			if (result.compareValue != 0)
				return result;
		}
	
		return compareMatch();
	}
} // namespace {

int daeElement::compare(daeElement& elt1, daeElement& elt2) {
	return compareWithFullResult(elt1, elt2).compareValue;
}

daeElement::compareResult daeElement::compareWithFullResult(daeElement& elt1, daeElement& elt2) {
	// Check the element name
	if (strcmp(elt1.getElementName(), elt2.getElementName()) != 0)
		return nameMismatch(elt1, elt2);

	// Dispatch to a specific function based on whether or not the types are the same
	if ((elt1.typeID() != elt2.typeID())  ||  elt1.typeID() == domAny::ID())
		return compareElementsDifferentTypes(elt1, elt2);
	else
		return compareElementsSameType(elt1, elt2);
}


daeURI *daeElement::getDocumentURI() const {
	if ( _document == NULL ) {
		return NULL;
	}
	return _document->getDocumentURI();
}


daeElement::matchName::matchName(daeString name) : name(name) { }
	
bool daeElement::matchName::operator()(daeElement* elt) const {
	return strcmp(elt->getElementName(), name.c_str()) == 0;
}

daeElement::matchType::matchType(daeInt typeID) : typeID(typeID) { }

bool daeElement::matchType::operator()(daeElement* elt) const {
	return elt->typeID() == typeID;
}

daeElement* daeElement::getChild(const matchElement& matcher) {
	daeElementRefArray children;
	getChildren(children);
	for (size_t i = 0; i < children.getCount(); i++)
		if (matcher(children[i]))
			return children[i];

	return NULL;
}

daeElement* daeElement::getDescendant(const matchElement& matcher) {
	daeElementRefArray elts;
	getChildren(elts);
	
	for (size_t i = 0; i < elts.getCount(); i++) {
		// Check the current element for a match
		if (matcher(elts[i]))
			return elts[i];

		// Append the element's children to the queue
		daeElementRefArray children;
		elts[i]->getChildren(children);
		size_t oldCount = elts.getCount();
		elts.setCount(elts.getCount() + children.getCount());
		for (size_t j = 0; j < children.getCount(); j++)
			elts[oldCount + j] = children[j];
	}

	return NULL;
}

daeElement* daeElement::getAncestor(const matchElement& matcher) {
	daeElement* elt = getParent();
	while (elt) {
		if (matcher(elt))
			return elt;
		elt = elt->getParent();
	}

	return NULL;
}

daeElement* daeElement::getParent() {
	return _parent;
}

daeElement* daeElement::getChild(daeString eltName) {
	if (!eltName)
		return NULL;
	matchName test(eltName);
	return getChild(matchName(eltName));
}

daeElement* daeElement::getDescendant(daeString eltName) {
	if (!eltName)
		return NULL;
	return getDescendant(matchName(eltName));
}

daeElement* daeElement::getAncestor(daeString eltName) {
	if (!eltName)
		return NULL;
	return getAncestor(matchName(eltName));
}

DAE* daeElement::getDAE() {
	return _meta->getDAE();
}

void daeElement::setUserData(void* data) {
	_userData = data;
}

void* daeElement::getUserData() {
	return _userData;
}
