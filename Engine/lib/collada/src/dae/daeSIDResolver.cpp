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

#include <list>
#include <vector>
#include <sstream>
#include <algorithm>
#include <dae/daeSIDResolver.h>
#include <dae/daeIDRef.h>
#include <dae/daeAtomicType.h>
#include <dae/daeMetaAttribute.h>
#include <dae/daeMetaElement.h>
#include <dae/daeURI.h>
#include <dom/domTypes.h>
#include <dom/domConstants.h>
#include <dae/daeDocument.h>
#include <dae/daeDatabase.h>
#include <dae/daeUtils.h>
#include <dom/domSource.h>

using namespace std;


namespace {
	template<typename T>
	T nextIter(const T& iter) {
		T next = iter;
		return ++next;
	}

	template<typename T>
	T moveIter(const T& iter, int n) {
		T result = iter;
		advance(result, n);
		return result;
	}
	
	// Implements a breadth-first sid search by starting at the container element and
	// traversing downward through the element tree.
	daeElement* findSidTopDown(daeElement* container, const string& sid, const string& profile) {
		if (!container)
			return NULL;

		vector<daeElement*> elts, matchingElts;
		elts.push_back(container);

		for (size_t i = 0; i < elts.size(); i++) {
			daeElement* elt = elts[i];
			
			// Bail if we're looking for an element in a different profile
			if (!profile.empty()) {
				if (strcmp(elt->getElementName(), COLLADA_ELEMENT_TECHNIQUE_COMMON) == 0)
					continue;
				if (strcmp(elt->getElementName(), COLLADA_ELEMENT_TECHNIQUE) == 0  &&
				    profile != elt->getAttribute("profile"))
					continue;
			}
		
			// See if this is a matching element
			if (elt->getAttribute("sid") == sid)
				return elt;
			else {
				// Add the children to the list of elements to check
				daeElementRefArray children;
				elt->getChildren(children);
				for (size_t j = 0; j < children.getCount(); j++)
					elts.push_back(children[j]);
			}
		}

		return NULL;
	}

	// Returns the distance between an element and an ancestor of the element. If 'container
	// isn't an ancestor of 'elt', or if 'elt' is in a profile that doesn't match 'profile'
	// UINT_MAX is returned.
	unsigned int computeDistance(daeElement* container, daeElement* elt, const string& profile) {
		if (!container || !elt)
			return UINT_MAX;

		unsigned int distance = 0;
		do {
			// Bail if we're looking for an element in a different profile
			if (!profile.empty()) {
				if (strcmp(elt->getElementName(), COLLADA_ELEMENT_TECHNIQUE_COMMON) == 0)
					return UINT_MAX;
				if (strcmp(elt->getElementName(), COLLADA_ELEMENT_TECHNIQUE) == 0  &&
				    profile != elt->getAttribute("profile"))
					return UINT_MAX;
			}

			if (elt == container)
				return distance;
			distance++;
		} while ((elt = elt->getParentElement()) != NULL);

		return UINT_MAX;
	}

	// Implements a breadth-first sid search by using the database to find all elements
	// matching 'sid', then finding the element closest to 'container'.
	daeElement* findSidBottomUp(daeElement* container, const string& sid, const string& profile) {
		if (!container || !container->getDocument())
			return NULL;

		// Get the elements with a matching sid
		vector<daeElement*> elts;
		container->getDocument()->getDAE()->getDatabase()->sidLookup(sid, elts, container->getDocument());

		// Compute the distance from each matching element to the container element
		unsigned int minDistance = UINT_MAX;
		daeElement* closestElt = NULL;
		for (size_t i = 0; i < elts.size(); i++) {
			unsigned int distance = computeDistance(container, elts[i], profile);
			if (distance < minDistance) {
				minDistance = distance;
				closestElt = elts[i];
			}
		}

		return closestElt;
	}

	daeElement* findID(daeElement* elt, const string& id, const string& profile) {
		return elt ? elt->getDAE()->getDatabase()->idLookup(id, elt->getDocument()) : NULL;
	}
	
	void buildString(const list<string>::iterator& begin,
	                 const list<string>::iterator& end,
	                 string& result) {
		ostringstream stream;
		for (list<string>::iterator iter = begin; iter != end; iter++)
			stream << *iter;
		result = stream.str();
	}

	// Finds an element with a matching ID or sid (depending on the 'finder' function)
	// passed in. First it tries to resolve the whole ID/sid, then it tries to resolve
	// successively smaller parts. For example, consider this sid ref: "my.sid.ref".
	// First this function will try to resolve "my.sid.ref" entirely, then if that
	// fails it'll try to resolve "my.sid.", "my.sid", "my.", and "my", in that order.
	// The part that wasn't matched is returned in the 'remainingPart' parameter.
	daeElement* findWithDots(daeElement* container,
	                         const string& s,
	                         const string& profile,
	                         daeElement* (*finder)(daeElement*, const string&, const string&),
	                         list<string>& remainingPart) {
		remainingPart.clear();

		// First see if the whole thing resolves correctly
		if (daeElement* result = finder(container, s, profile))
			return result;

		// It didn't resolve. Let's tokenize it by '.'s and see if we can resolve a
		// portion of it.
		cdom::tokenize(s, ".", remainingPart, true);
		if (remainingPart.size() == 1)
			return NULL; // There were no '.'s, so the result won't be any different

		list<string>::iterator iter = moveIter(remainingPart.end(), -1);
		for (int i = int(remainingPart.size())-1; i >= 1; i--, iter--) {
			string substr;
			buildString(remainingPart.begin(), iter, substr);
			if (daeElement* result = finder(container, substr, profile)) {
				// Remove the part we matched against from the list
				remainingPart.erase(remainingPart.begin(), iter);
				return result;
			}
		}

		remainingPart.clear();
		return NULL;
	}

	daeSidRef::resolveData resolveImpl(const daeSidRef& sidRef) {
		if (sidRef.sidRef.empty() || !sidRef.refElt)
			return daeSidRef::resolveData();

		daeSidRef::resolveData result;
		string separators = "/()";
		list<string> tokens;
		cdom::tokenize(sidRef.sidRef, separators, /* out */ tokens, true);

		list<string>::iterator tok = tokens.begin();

		// The first token should be either an ID or a '.' to indicate
		// that we should start the search from the container element.
		if (tok == tokens.end())
			return daeSidRef::resolveData();

		list<string> remainingPart;
		if (*tok == ".") {
			result.elt = sidRef.refElt;
			tok++;
		}	else {
			// Try to resolve it as an ID
			result.elt = findWithDots(sidRef.refElt, *tok, sidRef.profile, findID, remainingPart);
			if (result.elt) {
				if (!remainingPart.empty()) {
					// Insert the "remaining part" from the ID resolve into our list of tokens
					tokens.erase(tokens.begin());
					tokens.splice(tokens.begin(), remainingPart);
					tok = tokens.begin();
				} else
					tok++;
			}
		}

		if (!result.elt)
			return daeSidRef::resolveData();

		// Next we have an optional list of SIDs, each one separated by "/". Once we hit one of "()",
		// we know we're done with the SID section.
		for (; tok != tokens.end() && *tok == "/"; tok++) {
			tok++; // Read the '/'
			if (tok == tokens.end())
				return daeSidRef::resolveData();

			// Find the element matching the SID
			result.elt = findWithDots(result.elt, *tok, sidRef.profile, findSidTopDown, remainingPart);
			if (!result.elt)
				return daeSidRef::resolveData();

			if (!remainingPart.empty()) {
				list<string>::iterator tmp = tok;
				tok--;
				tokens.splice(tmp, remainingPart);
				tokens.erase(tmp);
			}
		}

		// Now we want to parse the member selection tokens. It can either be
		//   (a) '.' followed by a string representing the member to access
		//   (b) '(x)' where x is a number, optionally followed by another '(x)'
		// Anything else is an error.
		string member;
		bool haveArrayIndex1 = false, haveArrayIndex2 = false;
		int arrayIndex1 = -1, arrayIndex2 = -1;
		if (tok != tokens.end()) {
			if (*tok == ".") {
				tok++;
				if (tok == tokens.end())
					return daeSidRef::resolveData();
				member = *tok;
				tok++;
			}
			else if (*tok == "(") {
				tok++;
				if (tok == tokens.end())
					return daeSidRef::resolveData();

				istringstream stream(*tok);
				stream >> arrayIndex1;
				haveArrayIndex1 = true;
				if (!stream.good() && !stream.eof())
					return daeSidRef::resolveData();
				tok++;
				if (tok == tokens.end()  ||  *tok != ")")
					return daeSidRef::resolveData();
				tok++;
			
				if (tok != tokens.end()  &&  *tok == "(") {
					tok++;
					if (tok == tokens.end())
						return daeSidRef::resolveData();

					stream.clear();
					stream.str(*tok);
					stream >> arrayIndex2;
					haveArrayIndex2 = true;
					if (!stream.good() && !stream.eof())
						return daeSidRef::resolveData();
					tok++;
					if (tok == tokens.end()  ||  *tok != ")")
						return daeSidRef::resolveData();
					tok++;
				}
			}
		}

		// We shouldn't have any tokens left. If we do it's an error.
		if (tok != tokens.end())
			return daeSidRef::resolveData();

		// At this point we've parsed a correctly formatted SID reference. The only thing left is to resolve
		// the member selection portion of the SID ref. First, see if the resolved element has a float array we
		// can use.
		if (result.elt->typeID() == domSource::ID()) {
			if (domFloat_array* floatArray = ((domSource*)result.elt)->getFloat_array())
				result.array = (daeDoubleArray*)floatArray->getCharDataObject()->get(floatArray);
		}
		else 
		{
			daeMetaAttribute *ma = result.elt->getCharDataObject();
			if ( ma != NULL ) {
				if ( ma->isArrayAttribute() && ma->getType()->getTypeEnum() == daeAtomicType::DoubleType ) {
					result.array = (daeDoubleArray*)ma->get( result.elt );
				}
			}
		}

		if( result.array ) {
			// We have an array to use for indexing. Let's see if the SID ref uses member selection.
			if (!member.empty()) {
				// Do member lookup based on the constants defined in the COMMON profile
				if (member == "ANGLE") {
					result.scalar = &(result.array->get(3));
				}	else if (member.length() == 1) {
					switch(member[0]) {
					case 'X':
					case 'R':
					case 'U':
					case 'S':
						result.scalar = &(result.array->get(0));
						break;
					case 'Y':
					case 'G':
					case 'V':
					case 'T':
						result.scalar = &(result.array->get(1));
						break;
					case 'Z':
					case 'B':
					case 'P':
						result.scalar = &(result.array->get(2));
						break;
					case 'W':
					case 'A':
					case 'Q':
						result.scalar = &(result.array->get(3));
						break;
					};
				}
			} else if (haveArrayIndex1) {
				// Use the indices to lookup a value in the array
				if (haveArrayIndex2  &&  result.array->getCount() == 16) {
					// We're doing a matrix lookup. Make sure the index is valid.
					int i = arrayIndex1*4 + arrayIndex2;
					if (i >= 0  &&  i < int(result.array->getCount()))
						result.scalar = &(result.array->get(i));
				} else {
					// Vector lookup. Make sure the index is valid.
					if (arrayIndex1 >= 0  &&  arrayIndex1 < int(result.array->getCount()))
						result.scalar = &(result.array->get(arrayIndex1));
				}
			}
		}

		// If we tried to do member selection but we couldn't resolve it to a doublePtr, fail.
		if ((!member.empty() || haveArrayIndex1)  &&  result.scalar == NULL)
			return daeSidRef::resolveData();

		// SID resolution was successful.
		return result;
	}
} // namespace {


daeSidRef::resolveData::resolveData() : elt(NULL), array(NULL), scalar(NULL) { }

daeSidRef::resolveData::resolveData(daeElement* elt, daeDoubleArray* array, daeDouble* scalar)
  : elt(elt),
    array(array),
    scalar(scalar) { }


daeSidRef::daeSidRef() : refElt(NULL) { }

daeSidRef::daeSidRef(const string& sidRef, daeElement* referenceElt, const string& profile)
  : sidRef(sidRef),
    refElt(referenceElt),
    profile(profile) { }

bool daeSidRef::operator<(const daeSidRef& other) const {
	if (refElt != other.refElt)
		return refElt < other.refElt;
	if (sidRef != other.sidRef)
		return sidRef < other.sidRef;
	return profile < other.profile;
}

daeSidRef::resolveData daeSidRef::resolve() {
	if (!refElt)
		return daeSidRef::resolveData();
	
	// First check the cache
	daeSidRef::resolveData result = refElt->getDAE()->getSidRefCache().lookup(*this);
	if (result.elt)
		return result;

	// Try to resolve as an effect-style sid ref by prepending "./" to the sid ref.
	// If that fails, try resolving as an animation-style sid ref, where the first part is an ID.
	result = resolveImpl(daeSidRef(string("./") + sidRef, refElt, profile));
	if (!result.elt)
		result = resolveImpl(*this);

	if (result.elt) // Add the result to the cache
		refElt->getDAE()->getSidRefCache().add(*this, result);

	return result;
}


daeSIDResolver::daeSIDResolver( daeElement *container, daeString target, daeString profile )
	: container(NULL)
{
	setContainer(container);
	setTarget(target);
	setProfile(profile);
}

daeString daeSIDResolver::getTarget() const {
	return target.empty() ? NULL : target.c_str();
}

void daeSIDResolver::setTarget( daeString t )
{
	target = t ? t : "";
}

daeString daeSIDResolver::getProfile() const {
	return profile.empty() ? NULL : profile.c_str();
}

void daeSIDResolver::setProfile( daeString p )
{
	profile = p ? p : "";
}

daeElement* daeSIDResolver::getContainer() const {
	return container;
}

void daeSIDResolver::setContainer(daeElement* element)
{
	container = element;
}

daeSIDResolver::ResolveState daeSIDResolver::getState() const {
	if (target.empty())
		return target_empty;

	daeSidRef::resolveData result = daeSidRef(target, container, profile).resolve();
	if (!result.elt)
		return sid_failed_not_found;
	if (result.scalar)
		return sid_success_double;
	if (result.array)
		return sid_success_array;

	return sid_success_element;
}

daeElement* daeSIDResolver::getElement()
{
	return daeSidRef(target, container, profile).resolve().elt;
}

daeDoubleArray *daeSIDResolver::getDoubleArray()
{
	return daeSidRef(target, container, profile).resolve().array;
}

daeDouble *daeSIDResolver::getDouble()
{
	return daeSidRef(target, container, profile).resolve().scalar;
}


daeSidRefCache::daeSidRefCache() : hitCount(0), missCount(0) { }

daeSidRef::resolveData daeSidRefCache::lookup(const daeSidRef& sidRef) {
	map<daeSidRef, daeSidRef::resolveData>::iterator iter = lookupTable.find(sidRef);
	if (iter != lookupTable.end()) {
		hitCount++;
		return iter->second;
	}
	missCount++;
	return daeSidRef::resolveData();
}

void daeSidRefCache::add(const daeSidRef& sidRef, const daeSidRef::resolveData& result) {
	lookupTable[sidRef] = result;
}

void daeSidRefCache::clear() {
	lookupTable.clear();
	hitCount = missCount = 0;
}

bool daeSidRefCache::empty() {
	return lookupTable.empty();
}

int daeSidRefCache::misses() {
	return missCount;
}

int daeSidRefCache::hits() {
	return hitCount;
}
