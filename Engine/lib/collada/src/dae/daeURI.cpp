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

#include <algorithm>
#include <dae.h>
#include <dae/daeURI.h>
#include <ctype.h>
#include <dae/daeDocument.h>
#include <dae/daeErrorHandler.h>
#include <dae/daeUtils.h>
#include <pcrecpp.h>

using namespace std;
using namespace cdom;

void daeURI::initialize() {
	reset();
	container = NULL;
}

daeURI::~daeURI() { }

daeURI::daeURI(DAE& dae) : dae(&dae) {
	initialize();
}

daeURI::daeURI(DAE& dae, const string& uriStr, daeBool nofrag) : dae(&dae) {
	initialize();

	if (nofrag) {
		size_t pos = uriStr.find_last_of('#');
		if (pos != string::npos) {
			set(uriStr.substr(0, pos));
			return;
		}
	}

	set(uriStr);
}

daeURI::daeURI(const daeURI& baseURI, const string& uriStr) : dae(baseURI.getDAE())
{
	initialize();
	set(uriStr, &baseURI);
}

daeURI::daeURI(const daeURI& copyFrom_) : dae(copyFrom_.getDAE()), container(NULL)
{
	initialize();
	copyFrom(copyFrom_);
}

daeURI::daeURI(daeElement& container_, const std::string& uriStr)
	: dae(container_.getDAE())
{
	initialize();
	container = &container_;
	set(uriStr);
}

daeURI::daeURI(DAE& dae, daeElement& container_, const string& uriStr)
	: dae(&dae)
{
	initialize();
	container = &container_;
	set(uriStr);
}

void
daeURI::copyFrom(const daeURI& copyFrom)
{
	if (!container)
		container = copyFrom.container;
	set(copyFrom.originalStr());
}

daeURI& daeURI::operator=(const daeURI& other) {
	copyFrom(other);
	return *this;
}

daeURI& daeURI::operator=(const string& uriStr) {
	set(uriStr);
	return *this;
}

void daeURI::reset() {
	// Clear everything except the container, which doesn't change for the lifetime of the daeURI
	uriString	         = "";
	originalURIString	 = "";
	_scheme            = "";
	_authority	       = "";
	_path              = "";
	_query             = "";
	_fragment          = "";
}

DAE* daeURI::getDAE() const {
	return dae;
}


const string& daeURI::str() const {
	return uriString;
}

const string& daeURI::originalStr() const {
	return originalURIString;
}

daeString daeURI::getURI() const {
	return str().c_str();
}

daeString daeURI::getOriginalURI() const {
	return originalStr().c_str();
}


namespace {
	void parsePath(const string& path,
	               /* out */ string& dir,
	               /* out */ string& baseName,
	               /* out */ string& extension) {
		// !!!steveT Currently, if we have a file name that begins with a '.', as in
		// ".emacs", that will be treated as having no base name with an extension
		// of ".emacs". We might want to change this behavior, so that the base name
		// is considered ".emacs" and the extension is empty. I think this is more
		// in line with what path parsers in other libraries/languages do, and it
		// more accurately reflects the intended structure of the file name.
		static pcrecpp::RE re("(.*/)?([^.]*)?(\\..*)?");
		dir = baseName = extension = "";
		re.FullMatch(path, &dir, &baseName, &extension);
	}
}

void daeURI::set(const string& uriStr_, const daeURI* baseURI) {
	// We make a copy of the uriStr so that set(originalURIString, ...) works properly.
	string uriStr = uriStr_;
	reset();
	originalURIString = uriStr;

	if (!parseUriRef(uriStr, _scheme, _authority, _path, _query, _fragment)) {
		reset();
		return;
	}

	validate(baseURI);
}

void daeURI::set(const string& scheme_,
                 const string& authority_,
                 const string& path_,
                 const string& query_,
                 const string& fragment_,
                 const daeURI* baseURI)
{
	set(assembleUri(scheme_, authority_, path_, query_, fragment_), baseURI);
}

void daeURI::setURI(daeString _URIString, const daeURI* baseURI) {
	string uriStr = _URIString ? _URIString : "";
	set(uriStr, baseURI);
}


const string& daeURI::scheme() const { return _scheme; }
const string& daeURI::authority() const { return _authority; }
const string& daeURI::path() const { return _path; }
const string& daeURI::query() const { return _query; }
const string& daeURI::fragment() const { return _fragment; }
const string& daeURI::id() const { return fragment(); }


namespace {
	string addSlashToEnd(const string& s) {
		return (!s.empty() && s[s.length()-1] != '/')  ?  s + '/' : s;
	}
}

void daeURI::pathComponents(string& dir, string& baseName, string& ext) const {
	parsePath(_path, dir, baseName, ext);
}

string daeURI::pathDir() const {
	string dir, base, ext;
	parsePath(_path, dir, base, ext);
	return dir;
}

string daeURI::pathFileBase() const {
	string dir, base, ext;
	parsePath(_path, dir, base, ext);
	return base;
}

string daeURI::pathExt() const {
	string dir, base, ext;
	parsePath(_path, dir, base, ext);
	return ext;
}

string daeURI::pathFile() const {
	string dir, base, ext;
	parsePath(_path, dir, base, ext);
	return base + ext;
}

void daeURI::path(const string& dir, const string& baseName, const string& ext) {
	path(addSlashToEnd(dir) + baseName + ext);
}

void daeURI::pathDir(const string& dir) {
	string tmp, base, ext;
	parsePath(_path, tmp, base, ext);
	path(addSlashToEnd(dir), base, ext);
}

void daeURI::pathFileBase(const string& baseName) {
	string dir, tmp, ext;
	parsePath(_path, dir, tmp, ext);
	path(dir, baseName, ext);
}

void daeURI::pathExt(const string& ext) {
	string dir, base, tmp;
	parsePath(_path, dir, base, tmp);
	path(dir, base, ext);
}

void daeURI::pathFile(const string& file) {
	string dir, base, ext;
	parsePath(_path, dir, base, ext);
	path(dir, file, "");
}


daeString daeURI::getScheme() const { return _scheme.c_str(); }
daeString daeURI::getProtocol() const {	return getScheme(); }
daeString daeURI::getAuthority() const { return _authority.c_str(); }
daeString daeURI::getPath() const { return _path.c_str(); }
daeString daeURI::getQuery() const { return _query.c_str(); }
daeString daeURI::getFragment() const { return _fragment.c_str(); }
daeString daeURI::getID() const { return getFragment(); }
daeBool daeURI::getPath(daeChar *dest, daeInt size) const {
	if (int(_path.length()) < size) {
		strcpy(dest, _path.c_str());
		return true;
	}
	return false;
}


void daeURI::scheme(const string& scheme_) { set(scheme_, _authority, _path, _query, _fragment); };
void daeURI::authority(const string& authority_) { set(_scheme, authority_, _path, _query, _fragment); }
void daeURI::path(const string& path_) { set(_scheme, _authority, path_, _query, _fragment); }
void daeURI::query(const string& query_) { set(_scheme, _authority, _path, query_, _fragment); }
void daeURI::fragment(const string& fragment_) { set(_scheme, _authority, _path, _query, fragment_); }
void daeURI::id(const string& id) { fragment(id); }

void
daeURI::print()
{
	fprintf(stderr,"URI(%s)\n",uriString.c_str());
	fprintf(stderr,"scheme = %s\n",_scheme.c_str());
	fprintf(stderr,"authority = %s\n",_authority.c_str());
	fprintf(stderr,"path = %s\n",_path.c_str());
	fprintf(stderr,"query = %s\n",_query.c_str());
	fprintf(stderr,"fragment = %s\n",_fragment.c_str());
	fprintf(stderr,"URI without base = %s\n",originalURIString.c_str());
	fflush(stderr);
}

namespace {
	void normalize(string& path) {
		daeURI::normalizeURIPath(const_cast<char*>(path.c_str()));
		path = path.substr(0, strlen(path.c_str()));
	}
}

void
daeURI::validate(const daeURI* baseURI)
{
	// If no base URI was supplied, use the container's document URI. If there's
	// no container or the container doesn't have a doc URI, use the application
	// base URI.
	if (!baseURI) {
		if (!container || !(baseURI = container->getDocumentURI()))
			baseURI = &dae->getBaseURI();
		if (this == baseURI)
			return;
	}

	// This is rewritten according to the updated rfc 3986
	if (!_scheme.empty()) // if defined(R.scheme) then
	{
		// Everything stays the same except path which we normalize
		// T.scheme    = R.scheme;
		// T.authority = R.authority;
		// T.path      = remove_dot_segments(R.path);
		// T.query     = R.query;
		normalize(_path);
	}
	else
	{
		if (!_authority.empty()) // if defined(R.authority) then
		{
			// Authority and query stay the same, path is normalized
			// T.authority = R.authority;
			// T.path      = remove_dot_segments(R.path);
			// T.query     = R.query;
			normalize(_path);
		}
		else
		{
			if (_path.empty())  // if (R.path == "") then
			{
				// T.path = Base.path;
				_path = baseURI->_path;

				//if defined(R.query) then
				//   T.query = R.query;
				//else
				//   T.query = Base.query;
				//endif;
				if (_query.empty())
					_query = baseURI->_query;
			}
			else
			{
				if (_path[0] == '/')  // if (R.path starts-with "/") then
				{
					// T.path = remove_dot_segments(R.path);
					normalize(_path);
				}
				else
				{
					// T.path = merge(Base.path, R.path);
					if (!baseURI->_authority.empty() && baseURI->_path.empty()) // authority defined, path empty
						_path.insert(0, "/");
					else {
						string dir, baseName, ext;
						parsePath(baseURI->_path, dir, baseName, ext);
						_path = dir + _path;
					}
					// T.path = remove_dot_segments(T.path);
					normalize(_path);
				}
				// T.query = R.query;
			}
			// T.authority = Base.authority;
			_authority = baseURI->_authority;
		}
		// T.scheme = Base.scheme;
		_scheme = baseURI->_scheme;
	}
	// T.fragment = R.fragment;

	// Reassemble all this into a string version of the URI
	uriString = assembleUri(_scheme, _authority, _path, _query, _fragment);

	// Collect external references
	if (isExternalReference())
		container->getDocument()->addExternalReference( *this );
}

daeElementRef daeURI::getElement() const {
	return internalResolveElement();
}

daeElement* daeURI::internalResolveElement() const {
	if (uriString.empty())
		return NULL;
	
	return dae->getURIResolvers().resolveElement(*this);
}

void daeURI::resolveElement() { }

void daeURI::setContainer(daeElement* cont) {
	container = cont;
	// Since we have a new container element, the base URI may have changed. Re-resolve.
	set(originalURIString);
}

daeBool daeURI::isExternalReference() const {
	if (uriString.empty())
		return false;
	
	if (container && container->getDocumentURI()) {
		daeURI* docURI = container->getDocumentURI();
		if (_path != docURI->_path ||
		    _scheme != docURI->_scheme ||
		    _authority != docURI->_authority) {
			return true;
		}
	}

	return false;
}


daeDocument* daeURI::getReferencedDocument() const {
	string doc = assembleUri(_scheme, _authority, _path, "", "");
	return dae->getDatabase()->getDocument(doc.c_str(), true);
}

daeURI::ResolveState daeURI::getState() const {
	return uriString.empty() ? uri_empty : uri_loaded;
}

void daeURI::setState(ResolveState newState) { }


// This code is loosely based on the RFC 2396 normalization code from
// libXML. Specifically it does the RFC steps 6.c->6.g from section 5.2
// The path is modified in place, there is no error return.
void daeURI::normalizeURIPath(char* path)
{
	char *cur, // location we are currently processing
	     *out; // Everything from this back we are done with

	// Return if the path pointer is null

	if (path == NULL) return;

	// Skip any initial / characters to get us to the start of the first segment

	for(cur=path; *cur == '/'; cur++);

	// Return if we hit the end of the string

	if (*cur == 0) return;

	// Keep everything we've seen so far.
    
	out = cur;

	// Analyze each segment in sequence for cases (c) and (d).

	while (*cur != 0) 
	{
		// (c) All occurrences of "./", where "." is a complete path segment, are removed from the buffer string.
		
		if ((*cur == '.') && (*(cur+1) == '/')) 
		{
			cur += 2;
			// If there were multiple slashes, skip them too
			while (*cur == '/') cur++;
			continue;
		}

		// (d) If the buffer string ends with "." as a complete path segment, that "." is removed.

		if ((*cur == '.') && (*(cur+1) == 0))
			break;

		// If we passed the above tests copy the segment to the output side

		while (*cur != '/' && *cur != 0)
		{
			*(out++) = *(cur++);
		}

		if(*cur != 0)
		{
			// Skip any occurrances of // at the end of the segment

			while ((*cur == '/') && (*(cur+1) == '/')) cur++;

			// Bring the last character in the segment (/ or a null terminator) into the output
        
			*(out++) = *(cur++);
		}
	}

	*out = 0;

    // Restart at the beginning of the first segment for the next part

	for(cur=path; *cur == '/'; cur++);
	if (*cur == 0) return;

	// Analyze each segment in sequence for cases (e) and (f).
	//
	// e) All occurrences of "<segment>/../", where <segment> is a
	//    complete path segment not equal to "..", are removed from the
	//    buffer string.  Removal of these path segments is performed
	//    iteratively, removing the leftmost matching pattern on each
	//    iteration, until no matching pattern remains.
	//
	// f) If the buffer string ends with "<segment>/..", where <segment>
	//    is a complete path segment not equal to "..", that
	//    "<segment>/.." is removed.
	//
	// To satisfy the "iterative" clause in (e), we need to collapse the
	// string every time we find something that needs to be removed.  Thus,
	// we don't need to keep two pointers into the string: we only need a
	// "current position" pointer.
	//
	while (true)
	{
		char *segp, *tmp;

		// At the beginning of each iteration of this loop, "cur" points to
		// the first character of the segment we want to examine.

		// Find the end of the current segment.  
        
		for(segp = cur;(*segp != '/') && (*segp != 0); ++segp);

		// If this is the last segment, we're done (we need at least two
		// segments to meet the criteria for the (e) and (f) cases).

		if (*segp == 0)
			break;

		// If the first segment is "..", or if the next segment _isn't_ "..",
		// keep this segment and try the next one.

		++segp;
		if (((*cur == '.') && (cur[1] == '.') && (segp == cur+3))
            || ((*segp != '.') || (segp[1] != '.')
            || ((segp[2] != '/') && (segp[2] != 0)))) 
		{
			cur = segp;
			continue;
		}

		// If we get here, remove this segment and the next one and back up
		// to the previous segment (if there is one), to implement the
		// "iteratively" clause.  It's pretty much impossible to back up
		// while maintaining two pointers into the buffer, so just compact
		// the whole buffer now.

		// If this is the end of the buffer, we're done.

		if (segp[2] == 0) 
		{
			*cur = 0;
			break;
		}

		// Strings overlap during this copy, but not in a bad way, just avoid using strcpy
		
		tmp = cur;
		segp += 3;
		while ((*(tmp++) = *(segp++)) != 0);

		// If there are no previous segments, then keep going from here.
        
		segp = cur;
		while ((segp > path) && (*(--segp) == '/'));
        
		if (segp == path)
			continue;

		// "segp" is pointing to the end of a previous segment; find it's
		// start.  We need to back up to the previous segment and start
		// over with that to handle things like "foo/bar/../..".  If we
		// don't do this, then on the first pass we'll remove the "bar/..",
		// but be pointing at the second ".." so we won't realize we can also
		// remove the "foo/..".

		for(cur = segp;(cur > path) && (*(cur-1) != '/'); cur--);
	}

	*out = 0;

	// g) If the resulting buffer string still begins with one or more
	//    complete path segments of "..", then the reference is
	//    considered to be in error. Implementations may handle this
	//    error by retaining these components in the resolved path (i.e.,
	//    treating them as part of the final URI), by removing them from
	//    the resolved path (i.e., discarding relative levels above the
	//    root), or by avoiding traversal of the reference.
	//
	// We discard them from the final path.

	if (*path == '/') 
	{
		for(cur=path; (*cur == '/') && (cur[1] == '.') && (cur[2] == '.') && ((cur[3] == '/') || (cur[3] == 0)); cur += 3);

		if (cur != path) 
		{
			for(out=path; *cur != 0; *(out++) = *(cur++));

			*out = 0;
		}
	}
	return;
}

// This function will take a resolved URI and create a version of it that is relative to
// another existing URI.  The new URI is stored in the "originalURI"
int daeURI::makeRelativeTo(const daeURI* relativeToURI)
{
	// Can only do this function if both URIs have the same scheme and authority
	if (_scheme != relativeToURI->_scheme  ||  _authority != relativeToURI->_authority)
		return DAE_ERR_INVALID_CALL;

	// advance till we find a segment that doesn't match
	const char *this_path        = getPath();
	const char *relativeTo_path  = relativeToURI->getPath();
	const char *this_slash       = this_path;
	const char *relativeTo_slash = relativeTo_path;

	while((*this_path == *relativeTo_path) && *this_path)
	{
		if(*this_path == '/')
		{
			this_slash = this_path;
			relativeTo_slash = relativeTo_path;
		}
		this_path++;
		relativeTo_path++;
	}

	// Decide how many ../ segments are needed (Filepath should always end in a /)
	int segment_count = 0;
	relativeTo_slash++;
	while(*relativeTo_slash != 0)
	{
		if(*relativeTo_slash == '/')
			segment_count ++;
		relativeTo_slash++;
	}
	this_slash++;

	string newPath;
	for (int i = 0; i < segment_count; i++)
		newPath += "../";
	newPath += this_slash;
	
	set("", "", newPath, _query, _fragment, relativeToURI);
	return(DAE_OK);
}


daeBool daeURIResolver::_loadExternalDocuments = true;

daeURIResolver::daeURIResolver(DAE& dae) : dae(&dae) { }

daeURIResolver::~daeURIResolver() { }

void daeURIResolver::setAutoLoadExternalDocuments( daeBool load ) 
{ 
	_loadExternalDocuments = load; 
}

daeBool daeURIResolver::getAutoLoadExternalDocuments() 
{ 
	return _loadExternalDocuments; 
}


daeURIResolverList::daeURIResolverList() { }

daeURIResolverList::~daeURIResolverList() {
	for (size_t i = 0; i < resolvers.getCount(); i++)
		delete resolvers[i];
}

daeTArray<daeURIResolver*>& daeURIResolverList::list() {
	return resolvers;
}

daeElement* daeURIResolverList::resolveElement(const daeURI& uri) {
	for (size_t i = 0; i < resolvers.getCount(); i++)
		if (daeElement* elt = resolvers[i]->resolveElement(uri))
			return elt;
	return NULL;
}


// Returns true if parsing succeeded, false otherwise. Parsing can fail if the uri
// reference isn't properly formed.
bool cdom::parseUriRef(const string& uriRef,
                       string& scheme,
                       string& authority,
                       string& path,
                       string& query,
                       string& fragment) {
	// This regular expression for parsing URI references comes from the URI spec:
	//   http://tools.ietf.org/html/rfc3986#appendix-B
	static pcrecpp::RE re("^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\\?([^#]*))?(#(.*))?");
	string s1, s3, s6, s8;
	if (re.FullMatch(uriRef, &s1, &scheme, &s3, &authority, &path, &s6, &query, &s8, &fragment))
		return true;

	return false;
}

namespace {
	string safeSubstr(const string& s, size_t offset, size_t length) {
		string result = s.substr(offset, min(length, s.length() - offset));
		result.resize(length, '\0');
		return result;
	}
}

string cdom::assembleUri(const string& scheme,
                         const string& authority,
                         const string& path,
                         const string& query,
                         const string& fragment,
                         bool forceLibxmlCompatible) {
	string p = safeSubstr(path, 0, 3);
	bool libxmlHack = forceLibxmlCompatible && scheme == "file";
	bool uncPath = false;
	string uri;

	if (!scheme.empty())
		uri += scheme + ":";

	if (!authority.empty() || libxmlHack || (p[0] == '/' && p[1] == '/'))
		uri += "//";
	if (!authority.empty()) {
		if (libxmlHack) {
			// We have a UNC path URI of the form file://otherMachine/file.dae.
			// Convert it to file://///otherMachine/file.dae, which is how libxml
			// does UNC paths.
			uri += "///" + authority;
			uncPath = true;
		}
		else {
			uri += authority;
		}
	}

	if (!uncPath && libxmlHack && getSystemType() == Windows) {
		// We have to be delicate in how we pass absolute path URIs to libxml on Windows.
		// If the path is an absolute path with no drive letter, add an extra slash to
		// appease libxml.
		if (p[0] == '/' && p[1] != '/' && p[2] != ':') {
			uri += "/";
		}
	}
	uri += path;
	
	if (!query.empty())
		uri += "?" + query;
	if (!fragment.empty())
		uri += "#" + fragment;

	return uri;
}

string cdom::fixUriForLibxml(const string& uriRef) {
	string scheme, authority, path, query, fragment;
	cdom::parseUriRef(uriRef, scheme, authority, path, query, fragment);
	return assembleUri(scheme, authority, path, query, fragment, true);
}


string cdom::nativePathToUri(const string& nativePath, systemType type) {
	string uri = nativePath;

	if (type == Windows) {
		// Convert "c:\" to "/c:/"
		if (uri.length() >= 2  &&  isalpha(uri[0])  &&  uri[1] == ':')
			uri.insert(0, "/");
		// Convert backslashes to forward slashes
		uri = replace(uri, "\\", "/");
	}

	// Convert spaces to %20
	uri = replace(uri, " ", "%20");

	return uri;
}

string cdom::filePathToUri(const string& filePath) {
	return nativePathToUri(filePath);
}

string cdom::uriToNativePath(const string& uriRef, systemType type) {
	string scheme, authority, path, query, fragment;
	parseUriRef(uriRef, scheme, authority, path, query, fragment);

	// Make sure we have a file scheme URI, or that it doesn't have a scheme
	if (!scheme.empty()  &&  scheme != "file")
		return "";

	string filePath;

	if (type == Windows) {
		if (!authority.empty())
			filePath += string("\\\\") + authority; // UNC path
	
		// Replace two leading slashes with one leading slash, so that
		// ///otherComputer/file.dae becomes //otherComputer/file.dae and
		// //folder/file.dae becomes /folder/file.dae
		if (path.length() >= 2  &&  path[0] == '/'  &&  path[1] == '/')
			path.erase(0, 1);

		// Convert "/C:/" to "C:/"
		if (path.length() >= 3  &&  path[0] == '/'  &&  path[2] == ':')
			path.erase(0, 1);

		// Convert forward slashes to back slashes
		path = replace(path, "/", "\\");
	}

	filePath += path;

	// Replace %20 with space
	filePath = replace(filePath, "%20", " ");
	
	return filePath;
}

string cdom::uriToFilePath(const string& uriRef) {
	return uriToNativePath(uriRef);
}
