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

#ifndef __DAE_LIBXMLPLUGIN__
#define __DAE_LIBXMLPLUGIN__

#include <vector>
#include <dae/daeElement.h>
#include <dae/daeURI.h>
#include <dae/daeIOPluginCommon.h>

struct _xmlTextReader;
struct _xmlTextWriter;
class DAE;

/**
 * The @c daeLIBXMLPlugin class derives from @c daeIOPluginCommon and implements an XML
 * input/output backend using libxml2 as a parser. When using this plugin, DAE::load() expects
 * an rfc 2396 compliant URI,  any URI supported by libxml2 should be properly 
 * handled including ones with network schemes and authority.  If the URI contains a fragment it will be ignored
 * and the entire referenced document will be loaded.  DAE::saveAs will only
 * handle a filename path at present (ie: no scheme or authority).
 */
class DLLSPEC daeLIBXMLPlugin : public daeIOPluginCommon
{
public:
	// Constructor / destructor
	/**
	 * Constructor.
	 */
	daeLIBXMLPlugin(DAE& dae);
	/**
	 * Destructor.
	 */
	virtual ~daeLIBXMLPlugin();

	// Operations
	virtual daeInt write(const daeURI& name, daeDocument *document, daeBool replace);

	/**
	 * setOption allows you to set options for this IOPlugin. Which options a plugin supports is
	 * dependent on the plugin itself. There is currently no list of options that plugins are
	 * suggested to implement. daeLibXML2Plugin supports only one option, "saveRawBinary". Set to 
	 * "true" to save float_array data as a .raw binary file. The daeRawResolver will convert the 
	 * data back into COLLADA domFloat_array elements upon load.
	 * @param option The option to set.
	 * @param value The value to set the option.
	 * @return Returns DAE_OK upon success.
	 */
	virtual daeInt setOption( daeString option, daeString value );

	/**
	 * getOption retrieves the value of an option from this IOPlugin. Which options a plugin supports is
	 * dependent on the plugin itself.
	 * @param option The option to get.
	 * @return Returns the string value of the option or NULL if option is not valid.
	 */
	virtual daeString getOption( daeString option );

private:
	DAE& dae;
	
	_xmlTextWriter *writer;

	FILE *rawFile;
	unsigned long rawByteCount;
	daeURI rawRelPath;
	bool saveRawFile;

	virtual daeElementRef readFromFile(const daeURI& uri);
	virtual daeElementRef readFromMemory(daeString buffer, const daeURI& baseUri);
	daeElementRef read(_xmlTextReader* reader);
	daeElementRef readElement(_xmlTextReader* reader, daeElement* parentElement);

	void writeElement( daeElement* element ); 
	void writeAttribute( daeMetaAttribute* attr, daeElement* element);
	void writeValue(daeElement* element);

	void writeRawSource( daeElement* src );
};

#endif //__DAE_LIBXMLPLUGIN__
