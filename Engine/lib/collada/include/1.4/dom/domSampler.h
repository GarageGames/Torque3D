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

#ifndef __domSampler_h__
#define __domSampler_h__

#include <dae/daeDocument.h>
#include <dom/domTypes.h>
#include <dom/domElements.h>

#include <dom/domInputLocal.h>
class DAE;

/**
 * The sampler element declares an N-dimensional function used for animation.
 * Animation function curves  are represented by 1-D sampler elements in COLLADA.
 * The sampler defines sampling points and how to  interpolate between them.
 */
class domSampler : public daeElement
{
public:
	virtual COLLADA_TYPE::TypeEnum getElementType() const { return COLLADA_TYPE::SAMPLER; }
	static daeInt ID() { return 654; }
	virtual daeInt typeID() const { return ID(); }
protected:  // Attribute
/**
 *  The id attribute is a text string containing the unique identifier of
 * this element. This value  must be unique within the instance document.
 * Optional attribute. 
 */
	xsID attrId;

protected:  // Element
/**
 * The input element must occur at least one time. These inputs are local
 * inputs. @see domInput
 */
	domInputLocal_Array elemInput_array;

public:	//Accessors and Mutators
	/**
	 * Gets the id attribute.
	 * @return Returns a xsID of the id attribute.
	 */
	xsID getId() const { return attrId; }
	/**
	 * Sets the id attribute.
	 * @param atId The new value for the id attribute.
	 */
	void setId( xsID atId ) { *(daeStringRef*)&attrId = atId; _validAttributeArray[0] = true; 
		if( _document != NULL ) _document->changeElementID( this, attrId );
	}

	/**
	 * Gets the input element array.
	 * @return Returns a reference to the array of input elements.
	 */
	domInputLocal_Array &getInput_array() { return elemInput_array; }
	/**
	 * Gets the input element array.
	 * @return Returns a constant reference to the array of input elements.
	 */
	const domInputLocal_Array &getInput_array() const { return elemInput_array; }
protected:
	/**
	 * Constructor
	 */
	domSampler(DAE& dae) : daeElement(dae), attrId(), elemInput_array() {}
	/**
	 * Destructor
	 */
	virtual ~domSampler() {}
	/**
	 * Overloaded assignment operator
	 */
	virtual domSampler &operator=( const domSampler &cpy ) { (void)cpy; return *this; }

public: // STATIC METHODS
	/**
	 * Creates an instance of this class and returns a daeElementRef referencing it.
	 * @return a daeElementRef referencing an instance of this object.
	 */
	static DLLSPEC daeElementRef create(DAE& dae);
	/**
	 * Creates a daeMetaElement object that describes this element in the meta object reflection framework.
	 * If a daeMetaElement already exists it will return that instead of creating a new one. 
	 * @return A daeMetaElement describing this COLLADA element.
	 */
	static DLLSPEC daeMetaElement* registerElement(DAE& dae);
};


#endif
