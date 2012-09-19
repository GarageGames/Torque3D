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

#include <dae/daeRefCountedObj.h>

daeRefCountedObj::daeRefCountedObj() : _refCount(0) { }

daeRefCountedObj::~daeRefCountedObj() { }

void daeRefCountedObj::release() const {
	if (--_refCount <= 0)
		delete this;
}

void daeRefCountedObj::ref() const {
	_refCount++;
}

void checkedRelease(const daeRefCountedObj* obj) {
	if (obj)
		obj->release();
}

void checkedRef(const daeRefCountedObj* obj) {
	if (obj)
		obj->ref();
}
