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

#include <sstream>
#include <dae/daeMetaAttribute.h>
#include <dae/daeMetaElement.h>
#include <dae/daeErrorHandler.h>
#include <dae/daeDocument.h>

void daeMetaAttribute::set(daeElement* e, daeString s) {
	stringToMemory(e, s);
}

void daeMetaAttribute::copy(daeElement* to, daeElement *from) {
	_type->copy(get(from), get(to));
}

void daeMetaArrayAttribute::copy(daeElement* to, daeElement *from) {
	daeArray& fromArray = (daeArray&)*get(from);
	daeArray& toArray = (daeArray&)*get(to);
	_type->copyArray(fromArray, toArray);
}

void daeMetaAttribute::copyDefault(daeElement* element) {
	if (_defaultValue)
		_type->copy(_defaultValue, get(element));
}

void daeMetaArrayAttribute::copyDefault(daeElement* element) {
	if (_defaultValue)
		_type->copyArray((daeArray&)*_defaultValue, (daeArray&)*get(element));
}
	
daeInt daeMetaAttribute::compare(daeElement* elt1, daeElement* elt2) {
	return _type->compare(get(elt1), get(elt2));
}

daeInt daeMetaArrayAttribute::compare(daeElement* elt1, daeElement* elt2) {
	daeArray& value1 = (daeArray&)*get(elt1);
	daeArray& value2 = (daeArray&)*get(elt2);
	return _type->compareArray(value1, value2);
}

daeInt daeMetaAttribute::compareToDefault(daeElement* e) {
	if (!_defaultValue)
		return 1;
	return _type->compare(get(e), _defaultValue);
}

daeInt daeMetaArrayAttribute::compareToDefault(daeElement* e) {
	if (!_defaultValue)
		return 1;
	daeArray& value1 = (daeArray&)*get(e);
	daeArray& value2 = (daeArray&)*_defaultValue;
	return _type->compareArray(value1, value2);
}

daeMetaAttribute::daeMetaAttribute()
{
	_name = "noname";
	_offset = -1;
	_type = NULL;
	_container = NULL;
	_defaultString = "";
	_defaultValue = NULL;
	_isRequired = false;
}

daeMetaAttribute::~daeMetaAttribute() {
	if (_defaultValue)
		_type->destroy(_defaultValue);
	_defaultValue = NULL;
}

daeMetaArrayAttribute::~daeMetaArrayAttribute() {
	delete (daeArray*)_defaultValue;
	_defaultValue = NULL;
}

daeInt
daeMetaAttribute::getSize()
{
	return _type->getSize();
}
daeInt
daeMetaAttribute::getAlignment()
{
	return _type->getAlignment();
}

void daeMetaAttribute::memoryToString(daeElement* e, std::ostringstream& buffer) {
	_type->memoryToString(get(e), buffer);
}

void daeMetaAttribute::stringToMemory(daeElement* e, daeString s) {
	if (!strcmp(_name, "id") && e->getDocument())
		e->getDocument()->changeElementID(e, s);
	else if (!strcmp(_name, "sid") && e->getDocument())
		e->getDocument()->changeElementSID(e, s);

	_type->stringToMemory((daeChar*)s, get(e));
}

daeChar* daeMetaAttribute::getWritableMemory(daeElement* e) {
	return (daeChar*)e + _offset;
}

daeMemoryRef daeMetaAttribute::get(daeElement* e) {
	return getWritableMemory(e);
}

void daeMetaAttribute::setDefaultString(daeString defaultVal) {
	_defaultString = defaultVal;
	if (!_defaultValue)
		_defaultValue = _type->create();
	_type->stringToMemory((daeChar*)_defaultString.c_str(), _defaultValue);
}

void daeMetaAttribute::setDefaultValue(daeMemoryRef defaultVal) {
	if (!_defaultValue)
		_defaultValue = _type->create();
	_type->copy(defaultVal, _defaultValue);
	std::ostringstream buffer;
	_type->memoryToString(_defaultValue, buffer);
	_defaultString = buffer.str();
}

void daeMetaArrayAttribute::memoryToString(daeElement* e, std::ostringstream& buffer) {
	if (e)
		_type->arrayToString(*(daeArray*)get(e), buffer);
}

void daeMetaArrayAttribute::stringToMemory(daeElement* e, daeString s) {
	if (e)
		_type->stringToArray((daeChar*)s, *(daeArray*)get(e));
}

void daeMetaArrayAttribute::setDefaultString(daeString defaultVal) {
	_defaultString = defaultVal;
	if (!_defaultValue)
		_defaultValue = (daeMemoryRef)_type->createArray();
	_type->stringToArray((daeChar*)_defaultString.c_str(), (daeArray&)*_defaultValue);
}

void daeMetaArrayAttribute::setDefaultValue(daeMemoryRef defaultVal) {
	if (!_defaultValue)
		_defaultValue = (daeMemoryRef)_type->createArray();
	_type->copyArray((daeArray&)*defaultVal, (daeArray&)*_defaultValue);
	std::ostringstream buffer;
	_type->arrayToString((daeArray&)*_defaultValue, buffer);
	_defaultString = buffer.str();
}

daeString daeMetaAttribute::getDefaultString() {
	return _defaultString.c_str();
}

daeMemoryRef daeMetaAttribute::getDefaultValue() {
	return _defaultValue;
}

void daeMetaAttribute::setDocument(daeElement* e, daeDocument* doc) {
	_type->setDocument(get(e), doc);
}

void daeMetaArrayAttribute::setDocument(daeElement* e, daeDocument* doc) {
	_type->setDocument(*(daeArray*)get(e), doc);
}
