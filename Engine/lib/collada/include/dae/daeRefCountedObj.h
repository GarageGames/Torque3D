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

#ifndef daeRefCountedObj_h
#define daeRefCountedObj_h

#include <dae/daeTypes.h>
#include <dae/daePlatform.h>

class DLLSPEC daeRefCountedObj {
protected:
	mutable daeInt _refCount;

public:
	daeRefCountedObj();
	virtual ~daeRefCountedObj();

	/**
	 * Decrements the reference count and deletes the object if reference count is zero.
	 * @note Should not be used externally if daeSmartRefs are being used, they call it
	 * automatically.
	 */
	void release() const;

	/**
	 * Increments the reference count of this element.
	 * @note Should not be used externally if daeSmartRefs are being used, they call it
	 * automatically.
	 */
	void ref() const;
};

void DLLSPEC checkedRelease(const daeRefCountedObj* obj);
void DLLSPEC checkedRef(const daeRefCountedObj* obj);

#endif
