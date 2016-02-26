//-----------------------------------------------------------------------------
// Copyright (c) 2013 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------
#ifndef _TAML_SCRIPTBINDING_H
#define _TAML_SCRIPTBINDING_H

#include "console/engineAPI.h"

#include "persistence/taml/taml.h"

DefineEngineMethod(Taml, setFormat, void, (const char* formatName), ,  "(format) - Sets the format that Taml should use to read/write.\n"
                                            "@param format The format to use: 'xml' or 'binary'.\n"
                                            "@return No return value.")
{
   // Fetch format mode.
   const Taml::TamlFormatMode formatMode = Taml::getFormatModeEnum(formatName);
   
   // Was the format valid?
   if ( formatMode == Taml::InvalidFormat )
   {
      // No, so warn.
      Con::warnf( "Taml::setFormat() - Invalid format mode used: '%s'.", formatName );
      return;
   }

   // Set format mode.
   object->setFormatMode( formatMode );
}

//-----------------------------------------------------------------------------

DefineEngineMethod(Taml, getFormat, const char*, (), ,  "() - Gets the format that Taml should use to read/write.\n"
                                                    "@return The format that Taml should use to read/write.")
{
    // Fetch format mode.
   return Taml::getFormatModeDescription(object->getFormatMode());
}

//-----------------------------------------------------------------------------

DefineEngineMethod(Taml, setAutoFormat, void, (bool autoFormat), ,  "(autoFormat) Sets whether the format type is automatically determined by the filename extension or not.\n"
                                                "@param autoFormat Whether the format type is automatically determined by the filename extension or not.\n"
                                                "@return No return value." )
{
    object->setAutoFormat( autoFormat );
}

//-----------------------------------------------------------------------------

DefineEngineMethod(Taml, getAutoFormat, bool, (), ,  "() Gets whether the format type is automatically determined by the filename extension or not.\n"
                                                "@return Whether the format type is automatically determined by the filename extension or not." )
{
    return object->getAutoFormat();
}

//-----------------------------------------------------------------------------

DefineEngineMethod(Taml, setWriteDefaults, void, (bool writeDefaults), ,   "(writeDefaults) Sets whether to write static fields that are at their default or not.\n"
                                                    "@param writeDefaults Whether to write static fields that are at their default or not.\n"
                                                    "@return No return value." )
{
    object->setWriteDefaults( writeDefaults );
}

//-----------------------------------------------------------------------------

DefineEngineMethod(Taml, getWriteDefaults, bool, (), ,   "() Gets whether to write static fields that are at their default or not.\n"
                                                    "@return Whether to write static fields that are at their default or not." )
{
    return object->getWriteDefaults();
}

//-----------------------------------------------------------------------------

DefineEngineMethod(Taml, setProgenitorUpdate, void, (bool progenitorUpdate), ,    "(progenitorUpdate) Sets whether to update each type instances file-progenitor or not.\n"
                                                        "If not updating then the progenitor stay as the script that executed the call to Taml.\n"
                                                        "@param progenitorUpdate Whether to update each type instances file-progenitor or not.\n"
                                                        "@return No return value." )
{
    object->setProgenitorUpdate( progenitorUpdate );
}

//-----------------------------------------------------------------------------

DefineEngineMethod(Taml, getProgenitorUpdate, bool, (), ,    "() Gets whether to update each type instances file-progenitor or not.\n"
                                                        "@return Whether to update each type instances file-progenitor or not." )
{
    return object->getProgenitorUpdate();
}

//-----------------------------------------------------------------------------

DefineEngineMethod(Taml, setAutoFormatXmlExtension, void, (const char* extension), ,  "(extension) Sets the extension (end of filename) used to detect the XML format.\n"
                                                            "@param extension The extension (end of filename) used to detect the XML format.\n"
                                                            "@return No return value." )
{
    object->setAutoFormatXmlExtension( extension );
}

//-----------------------------------------------------------------------------

DefineEngineMethod(Taml, getAutoFormatXmlExtension, const char*, (), ,   "() Gets the extension (end of filename) used to detect the XML format.\n"
                                                                    "@return The extension (end of filename) used to detect the XML format." )
{
    return object->getAutoFormatXmlExtension();
}

//-----------------------------------------------------------------------------

DefineEngineMethod(Taml, setAutoFormatBinaryExtension, void, (const char* extension), ,   "(extension) Sets the extension (end of filename) used to detect the Binary format.\n"
                                                                "@param extension The extension (end of filename) used to detect the Binary format.\n"
                                                                "@return No return value." )
{
    object->setAutoFormatBinaryExtension( extension );
}

//-----------------------------------------------------------------------------

DefineEngineMethod(Taml, getAutoFormatBinaryExtension, const char*, (), ,    "() Gets the extension (end of filename) used to detect the Binary format.\n"
                                                                        "@return The extension (end of filename) used to detect the Binary format." )
{
    return object->getAutoFormatBinaryExtension();
}

//-----------------------------------------------------------------------------

DefineEngineMethod(Taml, setBinaryCompression, void, (bool compressed), ,   "(compressed) - Sets whether ZIP compression is used on binary formatting or not.\n"
                                                        "@param compressed Whether compression is on or off.\n"
                                                        "@return No return value.")
{
    // Set compression.
    object->setBinaryCompression( compressed );
}

//-----------------------------------------------------------------------------

DefineEngineMethod(Taml, getBinaryCompression, bool, (), ,  "() - Gets whether ZIP compression is used on binary formatting or not.\n"
                                                        "@return Whether ZIP compression is used on binary formatting or not.")
{
    // Fetch compression.
    return object->getBinaryCompression();
}

//-----------------------------------------------------------------------------

/*! Sets whether to write JSON that is strictly compatible with RFC4627 or not.
    @param jsonStrict Whether to write JSON that is strictly compatible with RFC4627 or not.
    @return No return value.
*/
DefineEngineMethod(Taml, setJSONStrict, void, (bool strict), , "(jsonStrict) - Sets whether to write JSON that is strictly compatible with RFC4627 or not."
    "@param jsonStrict Whether to write JSON that is strictly compatible with RFC4627 or not."
    "@return No return value.")
{
    // Set JSON Strict.
    object->setJSONStrict( strict );
}

//-----------------------------------------------------------------------------

/*! Gets whether to write JSON that is strictly compatible with RFC4627 or not.
    @return whether to write JSON that is strictly compatible with RFC4627 or not.
*/
DefineEngineMethod(Taml, getJSONStrict, bool, (), , "() - Gets whether to write JSON that is strictly compatible with RFC4627 or not."
    "@return whether to write JSON that is strictly compatible with RFC4627 or not.")
{
    // Get RFC strict.
    return object->getJSONStrict();
}

//-----------------------------------------------------------------------------

DefineEngineMethod(Taml, write, bool, (SimObject* obj, const char* filename), ,  "(object, filename) - Writes an object to a file using Taml.\n"
                                        "@param object The object to write.\n"
                                        "@param filename The filename to write to.\n"
                                        "@return Whether the write was successful or not.")
{
   // Did we find the object?
    if ( obj == NULL )
    {
        // No, so warn.
        Con::warnf( "Taml::write() - Tried to write a NULL object to file '%s'.", filename );
        return false;
    }

    return object->write( obj, filename );
}

//-----------------------------------------------------------------------------

DefineEngineMethod(Taml, read, SimObject*, (const char* filename), ,    "(filename) - Read an object from a file using Taml.\n"
                                                "@param filename The filename to read from.\n"
                                                "@return (Object) The object read from the file or an empty string if read failed.")
{
    // Read object.
    SimObject* pSimObject = object->read( filename );

    // Did we find the object?
    if ( pSimObject == NULL )
    {
        // No, so warn.
        Con::warnf( "Taml::read() - Could not read object from file '%s'.", filename );
        return NULL;
    }

    return pSimObject;
}

//-----------------------------------------------------------------------------

DefineEngineFunction(TamlWrite, bool, (SimObject* simObject, const char* filename, const char* format, bool compressed), 
                                       ("xml", true),  
                                        "(object, filename, [format], [compressed]) - Writes an object to a file using Taml.\n"
                                        "@param object The object to write.\n"
                                        "@param filename The filename to write to.\n"
                                        "@param format The file format to use.  Optional: Defaults to 'xml'.  Can be set to 'binary'.\n"
                                        "@param compressed Whether ZIP compression is used on binary formatting or not.  Optional: Defaults to 'true'.\n"
                                        "@return Whether the write was successful or not.")
{

    // Did we find the object?
    if ( simObject == NULL )
    {
        // No, so warn.
       //Con::warnf( "TamlWrite() - Could not find object '%s' to write to file '%s'.", simObject->getIdString(), filename );
       Con::warnf( "TamlWrite() - Could not find object to write to file '%s'.", filename );
        return false;
    }

    Taml taml;

    taml.setFormatMode( Taml::getFormatModeEnum(format) );  

   // Yes, so is the format mode binary?
   if ( taml.getFormatMode() == Taml::BinaryFormat )
   {
         // Yes, so set binary compression.
      taml.setBinaryCompression( compressed );
   }
   else
   {
         // No, so warn.
         Con::warnf( "TamlWrite() - Setting binary compression is only valid for XML formatting." );
   }

   // Turn-off auto-formatting.
   taml.setAutoFormat( false );

    // Write.
   return taml.write( simObject, filename );
}

//-----------------------------------------------------------------------------

DefineEngineFunction(TamlRead, const char*, (const char* filename, const char* format), ("xml"),    "(filename, [format]) - Read an object from a file using Taml.\n"
                                                "@param filename The filename to read from.\n"
                                                "@param format The file format to use.  Optional: Defaults to 'xml'.  Can be set to 'binary'.\n"
                                                "@return (Object) The object read from the file or an empty string if read failed.")
{

    // Set the format mode.
    Taml taml;

	// Yes, so set it.
    taml.setFormatMode( Taml::getFormatModeEnum(format) );  

	// Turn-off auto-formatting.
	taml.setAutoFormat( false );

    // Read object.
   SimObject* pSimObject = taml.read( filename );

    // Did we find the object?
    if ( pSimObject == NULL )
    {
        // No, so warn.
        Con::warnf( "TamlRead() - Could not read object from file '%s'.", filename );
        return StringTable->EmptyString();
    }

    return pSimObject->getIdString();
}

//-----------------------------------------------------------------------------

DefineEngineFunction(GenerateTamlSchema, bool, (), , "() - Generate a TAML schema file of all engine types.\n"
                                                "The schema file is specified using the console variable '" TAML_SCHEMA_VARIABLE "'.\n"
                                                "@return Whether the schema file was writtent or not." )
{
    // Generate the schema.
    return Taml::generateTamlSchema();
}

#endif _TAML_SCRIPTBINDING_H