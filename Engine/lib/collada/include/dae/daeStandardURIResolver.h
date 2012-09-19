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

#ifndef __DAE_STANDARD_URI_RESOLVER__
#define __DAE_STANDARD_URI_RESOVLER__

#include <string>
#include "dae/daeURI.h"
class DAE;

/**
 * The @c daeStandardURIResolver class derives from @c daeURIResolver and implements
 * the default XML backend resolver.
 */
class daeStandardURIResolver : public daeURIResolver
{
public:
	/**
	 * Constructor.
	 * @param database The @c daeDatabase used.
	 * @param plugin The @c daeIOPlugin used.
	 */
	DLLSPEC daeStandardURIResolver(DAE& dae);
	/**
	 * Destructor.
	 */
	DLLSPEC ~daeStandardURIResolver();

public: // Abstract Interface
	virtual DLLSPEC daeElement* resolveElement(const daeURI& uri);
	virtual DLLSPEC daeString getName();
};

#endif //__DAE_STANDARD_URI_RESOLVER__
