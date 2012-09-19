//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
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

#include "tinyxml/tinyxml.h"

//-----------------------------------------------------------------------------
// Console implementation of STL map.
//-----------------------------------------------------------------------------

#include "core/frameAllocator.h"
#include "console/simBase.h"
#include "console/consoleInternal.h"
#include "console/SimXMLDocument.h"
#include "console/engineAPI.h"

IMPLEMENT_CONOBJECT(SimXMLDocument);

ConsoleDocClass( SimXMLDocument,
   "@brief File I/O object used for creating, reading, and writing XML documents.\n\n"

   "A SimXMLDocument is a container of various XML nodes.  The Document level may contain "
   "a header (sometimes called a declaration), comments and child Elements.  Elements may "
   "contain attributes, data (or text) and child Elements.\n\n"

   "You build new Elements using addNewElement().  This makes the new Element the current "
   "one you're working with.  You then use setAttribute() to add attributes to the Element.  "
   "You use addData() or addText() to write to the text area of an Element."

   "@tsexample\n"
   "// Thanks to Rex Hiebert for this example\n"
   "// Given the following XML\n"
   "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\" ?>\n"
	"<DataTables>\n"
	"	<table tableName=\"2DShapes\">\n"
	"		<rec id=\"1\">Triangle</rec>\n"
	"		<rec id=\"2\">Square</rec>\n"
	"		<rec id=\"3\">Circle</rec>\n"
	"	</table>\n"
	"	<table tableName=\"3DShapes\">\n"
	"		<rec id=\"1\">Pyramid</rec>\n"
	"		<rec id=\"2\">Cube</rec>\n"
	"		<rec id=\"3\">Sphere</rec>\n"
	"	</table>\n"
	"</DataTables>\n\n"
   "// Using SimXMLDocument by itself\n"
   "function readXmlExample(%filename)\n"
	"{\n"
	"   %xml = new SimXMLDocument() {};\n"
	"   %xml.loadFile(%filename);\n\n"	   
	"   %xml.pushChildElement(\"DataTables\");\n"
	"   %xml.pushFirstChildElement(\"table\");\n"
	"   while(true)\n"
	"   {\n"
	"	  echo(\"TABLE:\" SPC %xml.attribute(\"tableName\"));\n"
	"	  %xml.pushFirstChildElement(\"rec\");\n"
	"	  while (true)\n"
	"	  {\n"
	"		 %id = %xml.attribute(\"id\");\n"
	"		 %desc = %xml.getData();\n"
	"		 echo(\"  Shape\" SPC %id SPC %desc);\n"
	"		 if (!%xml.nextSiblingElement(\"rec\")) break;\n"
	"	  }\n"
	"	  %xml.popElement();\n"
	"	  if (!%xml.nextSiblingElement(\"table\")) break;\n"
	"   }\n"
	"}\n\n"

   "// Thanks to Scott Peal for this example\n"
   "// Using FileObject in conjunction with SimXMLDocument\n"
   "// This example uses an XML file with a format of:\n"
   "// <Models>\n"
   "//    <Model category=\"\" name=\"\" path=\"\" />\n"
   "// </Models>\n"
	"function getModelsInCatagory()\n"
	"{\n"
	"   %file = \"./Catalog.xml\";\n"
	"   %fo = new FileObject();\n"
	"   %text = \"\";\n\n"
	"   if(%fo.openForRead(%file))\n"
	"   {\n"
	"	  while(!%fo.isEOF())\n"
	"	  {\n"
	"		 %text = %text @ %fo.readLine();\n"
	"		 if (!%fo.isEOF()) %text = %text @ \"\\n\";\n"
	"	  }\n"
	"   }\n"
	"   else\n"
	"   {\n"
	"	  echo(\"Unable to locate the file: \" @ %file);\n"
	"   }\n\n"
	"   %fo.delete();\n\n"
	"   %xml = new SimXMLDocument() {};\n"
	"   %xml.parse(%text);\n"
	"   // \"Get\" inside of the root element, \"Models\".\n"
	"   %xml.pushChildElement(0);\n\n"
	"   // \"Get\" into the first child element\n"
	"   if (%xml.pushFirstChildElement(\"Model\"))\n"
	"   {\n"
	"	  while (true)\n"
	"	  {\n"
	"		 // \n"
	"		 //  Here, i read the element's attributes.\n"
	"		 //  You might want to save these values in an array or call the %xml.getElementValue()\n"
	"		 //  if you have a different XML structure.\n\n"
	"		 %catagory = %xml.attribute(\"catagory\");\n"
	"		 %name = %xml.attribute(\"name\");\n"
	"		 %path = %xml.attribute(\"path\");\n\n"
	"		 // now, read the next \"Model\"\n"
	"		 if (!%xml.nextSiblingElement(\"Model\")) break;\n"
	"	  }\n"
	"   }\n"
	"}\n"
   "@endtsexample\n\n"

   "@note SimXMLDocument is a wrapper around TinyXml, a standard XML library.  If you're familiar "
   "with its concepts, you'll find they also apply here.\n\n"

   "@see FileObject\n\n"

   "@ingroup FileSystem\n"
);

// -----------------------------------------------------------------------------
// Constructor.
// -----------------------------------------------------------------------------
SimXMLDocument::SimXMLDocument():
m_qDocument(0),
m_CurrentAttribute(0)
{
}

// -----------------------------------------------------------------------------
// Destructor.
// -----------------------------------------------------------------------------
SimXMLDocument::~SimXMLDocument()
{
}

// -----------------------------------------------------------------------------
// Included for completeness.
// -----------------------------------------------------------------------------
bool SimXMLDocument::processArguments(S32 argc, const char** argv)
{
   return argc == 0;
}

// -----------------------------------------------------------------------------
// Script constructor.
// -----------------------------------------------------------------------------
bool SimXMLDocument::onAdd()
{
   if(!Parent::onAdd())
   {
      return false;
   }

   if(!m_qDocument)
   {
      m_qDocument = new TiXmlDocument();
   }
   return true;
}

// -----------------------------------------------------------------------------
// Script destructor.
// -----------------------------------------------------------------------------
void SimXMLDocument::onRemove()
{
   Parent::onRemove();
   if(m_qDocument)
   {
      m_qDocument->Clear();
      delete(m_qDocument);
   }
}

// -----------------------------------------------------------------------------
// Initialize peristent fields (datablocks).
// -----------------------------------------------------------------------------
void SimXMLDocument::initPersistFields()
{
   Parent::initPersistFields();
}

// -----------------------------------------------------------------------------
// Set this to default state at construction.
// -----------------------------------------------------------------------------
void SimXMLDocument::reset(void)
{
   m_qDocument->Clear();
   m_paNode.clear();
   m_CurrentAttribute = 0;
}

DefineEngineMethod( SimXMLDocument, reset, void, (),,
   "@brief Set this document to its default state.\n\n"
   
   "Clears all Elements from the documents.  Equivalent to using clear()\n\n"
   
   "@see clear()")
{
   object->reset();
}

// -----------------------------------------------------------------------------
// Get true if file loads successfully.
// -----------------------------------------------------------------------------
bool SimXMLDocument::loadFile(const char* rFileName)
{
   reset();
   return m_qDocument->LoadFile(rFileName);
}

DefineEngineMethod( SimXMLDocument, loadFile, bool, ( const char* fileName ),,
   "@brief Load in given filename and prepare it for use.\n\n"
   "@note Clears the current document's contents.\n\n"
   "@param fileName Name and path of XML document\n"
   "@return True if the file was loaded successfully.")
{
   return object->loadFile( fileName );
}

// -----------------------------------------------------------------------------
// Get true if file saves successfully.
// -----------------------------------------------------------------------------
bool SimXMLDocument::saveFile(const char* rFileName)
{
   return m_qDocument->SaveFile( rFileName );
}

// -----------------------------------------------------------------------------
// Get true if file saves successfully to string.
// -----------------------------------------------------------------------------
bool SimXMLDocument::saveToString(String& str)
{
   TiXmlPrinter printer;
   bool ret = m_qDocument->Accept( &printer );
   if (ret)
      str = printer.CStr();
   return ret;
}

DefineEngineMethod( SimXMLDocument, saveFile, bool, ( const char* fileName ),,
   "@brief Save document to the given file name.\n\n"
   "@param fileName Path and name of XML file to save to.\n"
   "@return True if the file was successfully saved.")
{
   return object->saveFile( fileName );
}

// -----------------------------------------------------------------------------
// Get true if XML text parses correctly.
// -----------------------------------------------------------------------------
S32 SimXMLDocument::parse(const char* rText)
{
   reset();
   m_qDocument->Parse( rText );
   return 1;
}

DefineEngineMethod( SimXMLDocument, parse, void, ( const char* xmlString ),,
   "@brief Create a document from a XML string.\n\n"
   "@note Clears the current document's contents.\n\n"
   "@param xmlString Valid XML to parse and store as a document.")
{
   object->parse( xmlString );
}

// -----------------------------------------------------------------------------
// Clear contents of XML document.
// -----------------------------------------------------------------------------
void SimXMLDocument::clear(void)
{
   reset();
}

DefineEngineMethod( SimXMLDocument, clear, void, (),,
   "@brief Set this document to its default state.\n\n"
   
   "Clears all Elements from the documents.  Equivalent to using reset()\n\n"
   
   "@see reset()")
{
   object->clear();
}

// -----------------------------------------------------------------------------
// Get error description of XML document.
// -----------------------------------------------------------------------------
const char* SimXMLDocument::getErrorDesc(void) const
{
   if(!m_qDocument)
   {
      return StringTable->insert("No document");
   }
   return m_qDocument->ErrorDesc();
}

DefineEngineMethod( SimXMLDocument, getErrorDesc, const char*, (),,
   "@brief Get last error description.\n\n"
   "@return A string of the last error message.")
{
   return object->getErrorDesc();
}

// -----------------------------------------------------------------------------
// Clear error description of this.
// -----------------------------------------------------------------------------
void SimXMLDocument::clearError(void)
{
   m_qDocument->ClearError();
}

DefineEngineMethod( SimXMLDocument, clearError, void, (),,
   "@brief Clear the last error description.\n\n")
{
   object->clearError();
}

// -----------------------------------------------------------------------------
// Get true if first child element was successfully pushed onto stack.
// -----------------------------------------------------------------------------
bool SimXMLDocument::pushFirstChildElement(const char* rName)
{
   // Clear the current attribute pointer
   m_CurrentAttribute = 0;

   // Push the first element found under the current element of the given name
   TiXmlElement* pElement;
   if(!m_paNode.empty())
   {
      const int iLastElement = m_paNode.size() - 1;
      TiXmlElement* pNode = m_paNode[iLastElement];
      if(!pNode)
      {
         return false;
      }
      pElement = pNode->FirstChildElement(rName);
   }
   else
   {
      if(!m_qDocument)
      {
         return false;
      }
      pElement = m_qDocument->FirstChildElement(rName);
   }

   if(!pElement)
   {
      return false;
   }
   m_paNode.push_back(pElement);
   return true;
}

DefineEngineMethod( SimXMLDocument, pushFirstChildElement, bool, ( const char* name ),,
   "@brief Push the first child Element with the given name onto the stack, making it the current Element.\n\n"

   "@param name String containing name of the child Element.\n"
   "@return True if the Element was found and made the current one.\n"

   "@tsexample\n"
   "// Using the following test.xml file as an example:\n"
   "// <?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\" ?>\n"
   "// <NewElement>Some text</NewElement>\n\n"

   "// Load in the file\n"
   "%x = new SimXMLDocument();\n"
   "%x.loadFile(\"test.xml\");\n\n"

   "// Make the first Element the current one\n"
   "%x.pushFirstChildElement(\"NewElement\");\n\n"

   "// Store the current Element's text ('Some text' in this example)\n"
   "// into 'result'\n"
   "%result = %x.getText();\n"
   "echo( %result );\n"
   "@endtsexample\n\n")
{
   return object->pushFirstChildElement( name );
}

// -----------------------------------------------------------------------------
// Get true if first child element was successfully pushed onto stack.
// -----------------------------------------------------------------------------
bool SimXMLDocument::pushChildElement(S32 index)
{
   // Clear the current attribute pointer
   m_CurrentAttribute = 0;

   // Push the first element found under the current element of the given name
   TiXmlElement* pElement;
   if(!m_paNode.empty())
   {
      const int iLastElement = m_paNode.size() - 1;
      TiXmlElement* pNode = m_paNode[iLastElement];
      if(!pNode)
      {
         return false;
      }
      pElement = pNode->FirstChildElement();
      for( S32 i = 0; i < index; i++ )
      {
         if( !pElement )
            return false;

         pElement = pElement->NextSiblingElement();
      }
   }
   else
   {
      if(!m_qDocument)
      {
         return false;
      }
      pElement = m_qDocument->FirstChildElement();
      for( S32 i = 0; i < index; i++ )
      {
         if( !pElement )
            return false;

         pElement = pElement->NextSiblingElement();
      }
   }

   if(!pElement)
   {
      return false;
   }
   m_paNode.push_back(pElement);
   return true;
}

DefineEngineMethod( SimXMLDocument, pushChildElement, bool, ( S32 index ),,
   "@brief Push the child Element at the given index onto the stack, making it the current one.\n\n"
   "@param index Numerical index of Element being pushed."
   "@return True if the Element was found and made the current one.\n")
{
   return object->pushChildElement( index );
}

// -----------------------------------------------------------------------------
// Convert top stack element into its next sibling element.
// -----------------------------------------------------------------------------
bool SimXMLDocument::nextSiblingElement(const char* rName)
{
   // Clear the current attribute pointer
   m_CurrentAttribute = 0;

   // Attempt to find the next sibling element
   if(m_paNode.empty())
   {
      return false;
   }
   const int iLastElement = m_paNode.size() - 1;
   TiXmlElement*& pElement = m_paNode[iLastElement];
   if(!pElement)
   {
      return false;
   }

   pElement = pElement->NextSiblingElement(rName);
   if(!pElement)
   {
      return false;
   }

   return true;
}

DefineEngineMethod( SimXMLDocument, nextSiblingElement, bool, ( const char* name ),,
   "@brief Put the next sibling Element with the given name on the stack, making it the current one.\n\n"
   "@param name String containing name of the next sibling."
   "@return True if the Element was found and made the current one.\n")
{
   return object->nextSiblingElement( name );
}

// -----------------------------------------------------------------------------
// Get element value if it exists.  Used to extract a text node from the element.
// for example.
// -----------------------------------------------------------------------------
const char* SimXMLDocument::elementValue()
{
   if(m_paNode.empty())
   {
      return StringTable->insert("");
   }
   const int iLastElement = m_paNode.size() - 1;
   TiXmlElement* pNode = m_paNode[iLastElement];
   if(!pNode)
   {
      return StringTable->insert("");
   }

   return pNode->Value();
}

DefineEngineMethod( SimXMLDocument, elementValue, const char*, (),,
   "@brief Get the Element's value if it exists.\n\n"
   "Usually returns the text from the Element.\n"
   "@return The value from the Element, or an empty string if none is found.")
{
   return object->elementValue();
}

// -----------------------------------------------------------------------------
// Pop last element off of stack.
// -----------------------------------------------------------------------------
void SimXMLDocument::popElement(void)
{
   m_paNode.pop_back();
}

DefineEngineMethod( SimXMLDocument, popElement, void, (),,
   "@brief Pop the last Element off the stack.\n\n")
{
   object->popElement();
}

// -----------------------------------------------------------------------------
// Get attribute value if it exists.
// -----------------------------------------------------------------------------
const char* SimXMLDocument::attribute(const char* rAttribute)
{
   if(m_paNode.empty())
   {
      return StringTable->insert("");
   }
   const int iLastElement = m_paNode.size() - 1;
   TiXmlElement* pNode = m_paNode[iLastElement];
   if(!pNode)
   {
      return StringTable->insert("");
   }

   if(!pNode->Attribute(rAttribute))
   {
      return StringTable->insert("");
   }

   return pNode->Attribute(rAttribute);
}

DefineEngineMethod( SimXMLDocument, attribute, const char*, ( const char* attributeName ),,
   "@brief Get a string attribute from the current Element on the stack.\n\n"
   "@param attributeName Name of attribute to retrieve.\n"
   "@return The attribute string if found.  Otherwise returns an empty string.\n")
{
   return object->attribute( attributeName );
}

// These two methods don't make a lot of sense the way TS works.  Leaving them in for backwards-compatibility.
ConsoleMethod( SimXMLDocument, attributeF32, F32, 3, 3, "(string attributeName)"
   "@brief Get float attribute from the current Element on the stack.\n\n"
   "@param attributeName Name of attribute to retrieve.\n"
   "@return The value of the given attribute in the form of a float.\n"
   "@deprecated Use attribute().")
{
   return dAtof( object->attribute( argv[2] ) );
}
ConsoleMethod(SimXMLDocument, attributeS32, S32, 3, 3, "(string attributeName)"
   "@brief Get int attribute from the current Element on the stack.\n\n"
   "@param attributeName Name of attribute to retrieve.\n"
   "@return The value of the given attribute in the form of an integer.\n"
   "@deprecated Use attribute().")
{
   return dAtoi( object->attribute( argv[2] ) );
}

// -----------------------------------------------------------------------------
// Get true if given attribute exists.
// -----------------------------------------------------------------------------
bool SimXMLDocument::attributeExists(const char* rAttribute)
{
   if(m_paNode.empty())
   {
      return false;
   }
   const int iLastElement = m_paNode.size() - 1;
   TiXmlElement* pNode = m_paNode[iLastElement];
   if(!pNode)
   {
      return false;
   }

   if(!pNode->Attribute(rAttribute))
   {
      return false;
   }

   return true;
}

DefineEngineMethod( SimXMLDocument, attributeExists, bool, ( const char* attributeName ),,
   "@brief Tests if the requested attribute exists.\n\n"
   "@param attributeName Name of attribute being queried for.\n\n"
   "@return True if the attribute exists.")
{
   return object->attributeExists( attributeName );
}

// -----------------------------------------------------------------------------
// Obtain the name of the current element's first attribute
// -----------------------------------------------------------------------------
const char* SimXMLDocument::firstAttribute()
{
   // Get the current element
   if(m_paNode.empty())
   {
      return StringTable->insert("");
   }
   const int iLastElement = m_paNode.size() - 1;
   TiXmlElement* pNode = m_paNode[iLastElement];
   if(!pNode)
   {
      return StringTable->insert("");
   }

   // Gets its first attribute, if any
   m_CurrentAttribute = pNode->FirstAttribute();
   if(!m_CurrentAttribute)
   {
      return StringTable->insert("");
   }

   return m_CurrentAttribute->Name();
}

DefineEngineMethod( SimXMLDocument, firstAttribute, const char*, (),,
   "@brief Obtain the name of the current Element's first attribute.\n\n"
   "@return String containing the first attribute's name, or an empty string if none is found.\n\n"
   "@see nextAttribute()\n"
   "@see lastAttribute()\n"
   "@see prevAttribute()")
{
   return object->firstAttribute();
}

// -----------------------------------------------------------------------------
// Obtain the name of the current element's last attribute
// -----------------------------------------------------------------------------
const char* SimXMLDocument::lastAttribute()
{
   // Get the current element
   if(m_paNode.empty())
   {
      return StringTable->insert("");
   }
   const int iLastElement = m_paNode.size() - 1;
   TiXmlElement* pNode = m_paNode[iLastElement];
   if(!pNode)
   {
      return StringTable->insert("");
   }

   // Gets its last attribute, if any
   m_CurrentAttribute = pNode->LastAttribute();
   if(!m_CurrentAttribute)
   {
      return StringTable->insert("");
   }

   return m_CurrentAttribute->Name();
}

DefineEngineMethod( SimXMLDocument, lastAttribute, const char*, (),,
   "@brief Obtain the name of the current Element's last attribute.\n\n"
   "@return String containing the last attribute's name, or an empty string if none is found.\n\n"
   "@see prevAttribute()\n"
   "@see firstAttribute()\n"
   "@see lastAttribute()\n")
{
   return object->lastAttribute();
}

// -----------------------------------------------------------------------------
// Get the name of the next attribute for the current element after a call
// to firstAttribute().
// -----------------------------------------------------------------------------
const char* SimXMLDocument::nextAttribute()
{
   if(!m_CurrentAttribute)
   {
      return StringTable->insert("");
   }

   // Gets its next attribute, if any
   m_CurrentAttribute = m_CurrentAttribute->Next();
   if(!m_CurrentAttribute)
   {
      return StringTable->insert("");
   }

   return m_CurrentAttribute->Name();
}

DefineEngineMethod( SimXMLDocument, nextAttribute, const char*, (),,
   "@brief Get the name of the next attribute for the current Element after a call to firstAttribute().\n\n"
   "@return String containing the next attribute's name, or an empty string if none is found."
   "@see firstAttribute()\n"
   "@see lastAttribute()\n"
   "@see prevAttribute()\n")
{
   return object->nextAttribute();
}

// -----------------------------------------------------------------------------
// Get the name of the previous attribute for the current element after a call
// to lastAttribute().
// -----------------------------------------------------------------------------
const char* SimXMLDocument::prevAttribute()
{
   if(!m_CurrentAttribute)
   {
      return StringTable->insert("");
   }

   // Gets its next attribute, if any
   m_CurrentAttribute = m_CurrentAttribute->Previous();
   if(!m_CurrentAttribute)
   {
      return StringTable->insert("");
   }

   return m_CurrentAttribute->Name();
}

DefineEngineMethod( SimXMLDocument, prevAttribute, const char*, (),,
   "@brief Get the name of the previous attribute for the current Element after a call to lastAttribute().\n\n"
   "@return String containing the previous attribute's name, or an empty string if none is found."
   "@see lastAttribute()\n"
   "@see firstAttribute()\n"
   "@see nextAttribute()\n")
{
   return object->prevAttribute();
}

// -----------------------------------------------------------------------------
// Set attribute of top stack element to given value.
// -----------------------------------------------------------------------------
void SimXMLDocument::setAttribute(const char* rAttribute, const char* rVal)
{
   if(m_paNode.empty())
   {
      return;
   }

   const int iLastElement = m_paNode.size() - 1;
   TiXmlElement* pElement = m_paNode[iLastElement];
   if(!pElement)
   {
      return;
   }
   pElement->SetAttribute(rAttribute, rVal);
}
DefineEngineMethod( SimXMLDocument, setAttribute, void, ( const char* attributeName, const char* value ),,
   "@brief Set the attribute of the current Element on the stack to the given value.\n\n"
   "@param attributeName Name of attribute being changed\n"
   "@param value New value to assign to the attribute\n")
{
   object->setAttribute( attributeName, value );
}

// -----------------------------------------------------------------------------
// Set attribute of top stack element to given value.
// -----------------------------------------------------------------------------
void SimXMLDocument::setObjectAttributes(const char* objectID)
{
   if( !objectID || !objectID[0] )
      return;

   if(m_paNode.empty())
      return;

   SimObject *pObject = Sim::findObject( objectID );

   if( pObject == NULL )
      return;

   const int iLastElement = m_paNode.size() - 1;
   TiXmlElement* pElement = m_paNode[iLastElement];
   if(!pElement)
      return;

   char textbuf[1024];
   TiXmlElement field( "Field" );
   TiXmlElement group( "FieldGroup" );
   pElement->SetAttribute( "Name", pObject->getName() );


   // Iterate over our filed list and add them to the XML document...
   AbstractClassRep::FieldList fieldList = pObject->getFieldList();
   AbstractClassRep::FieldList::iterator itr;
   for(itr = fieldList.begin(); itr != fieldList.end(); itr++)
   {

      if( itr->type == AbstractClassRep::DeprecatedFieldType ||
         itr->type == AbstractClassRep::StartGroupFieldType ||
         itr->type == AbstractClassRep::EndGroupFieldType) continue;

      // Not an Array
      if(itr->elementCount == 1)
      {
         // get the value of the field as a string.
         ConsoleBaseType *cbt = ConsoleBaseType::getType(itr->type);

         const char *val = Con::getData(itr->type, (void *) (((const char *)pObject) + itr->offset), 0, itr->table, itr->flag);

         // Make a copy for the field check.
         if (!val)
            continue;

         FrameTemp<char> valCopy( dStrlen( val ) + 1 );
         dStrcpy( (char *)valCopy, val );

         if (!pObject->writeField(itr->pFieldname, valCopy))
            continue;

         val = valCopy;


         expandEscape(textbuf, val);

         if( !pObject->writeField( itr->pFieldname, textbuf ) )
            continue;

         field.SetValue( "Property" );
         field.SetAttribute( "name",  itr->pFieldname );
         if( cbt != NULL )
            field.SetAttribute( "type", cbt->getTypeName() );
         else
            field.SetAttribute( "type", "TypeString" );
         field.SetAttribute( "data", textbuf );

         pElement->InsertEndChild( field );

         continue;
      }
   }

   //// IS An Array
   //for(U32 j = 0; S32(j) < f->elementCount; j++)
   //{

   //   // If the start of a group create an element for the group and
   //   // the our chache to it
   //   const char *val = Con::getData(itr->type, (void *) (((const char *)pObject) + itr->offset), j, itr->table, itr->flag);

   //   // Make a copy for the field check.
   //   if (!val)
   //      continue;

   //   FrameTemp<char> valCopy( dStrlen( val ) + 1 );
   //   dStrcpy( (char *)valCopy, val );

   //   if (!pObject->writeField(itr->pFieldname, valCopy))
   //      continue;

   //   val = valCopy;

   //      // get the value of the field as a string.
   //      ConsoleBaseType *cbt = ConsoleBaseType::getType(itr->type);
   //      const char * dstr = Con::getData(itr->type, (void *)(((const char *)pObject) + itr->offset), 0, itr->table, itr->flag);
   //      if(!dstr)
   //         dstr = "";
   //      expandEscape(textbuf, dstr);


   //      if( !pObject->writeField( itr->pFieldname, dstr ) )
   //         continue;

   //      field.SetValue( "Property" );
   //      field.SetAttribute( "name",  itr->pFieldname );
   //      if( cbt != NULL )
   //         field.SetAttribute( "type", cbt->getTypeName() );
   //      else
   //         field.SetAttribute( "type", "TypeString" );
   //      field.SetAttribute( "data", textbuf );

   //      pElement->InsertEndChild( field );
   //}

}

DefineEngineMethod( SimXMLDocument, setObjectAttributes, void, ( const char* objectID ),,
   "@brief Add the given SimObject's fields as attributes of the current Element on the stack.\n\n"
   "@param objectID ID of SimObject being copied.")
{
   object->setObjectAttributes( objectID );
}

// -----------------------------------------------------------------------------
// Create a new element and set to child of current stack element.
// New element is placed on top of element stack.
// -----------------------------------------------------------------------------
void SimXMLDocument::pushNewElement(const char* rName)
{    
   TiXmlElement cElement( rName );
   TiXmlElement* pStackTop = 0;
   if(m_paNode.empty())
   {
      pStackTop = dynamic_cast<TiXmlElement*>
         (m_qDocument->InsertEndChild( cElement ) );
   }
   else
   {
      const int iFinalElement = m_paNode.size() - 1;
      TiXmlElement* pNode = m_paNode[iFinalElement];
      if(!pNode)
      {
         return;
      }
      pStackTop = dynamic_cast<TiXmlElement*>
         (pNode->InsertEndChild( cElement ));
   }
   if(!pStackTop)
   {
      return;
   }
   m_paNode.push_back(pStackTop);
}

DefineEngineMethod( SimXMLDocument, pushNewElement, void, ( const char* name ),,
   "@brief Create a new element with the given name as child of current Element "
   "and push it onto the Element stack making it the current one.\n\n"

   "@note This differs from addNewElement() in that it adds the new Element as a "
   "child of the current Element (or a child of the document if no Element exists).\n\n"

   "@param name XML tag for the new Element.\n"

   "@see addNewElement()")
{
   object->pushNewElement( name );
}

// -----------------------------------------------------------------------------
// Create a new element and set to child of current stack element.
// New element is placed on top of element stack.
// -----------------------------------------------------------------------------
void SimXMLDocument::addNewElement(const char* rName)
{    
   TiXmlElement cElement( rName );
   TiXmlElement* pStackTop = 0;
   if(m_paNode.empty())
   {
      pStackTop = dynamic_cast<TiXmlElement*>
         (m_qDocument->InsertEndChild( cElement ));
      if(!pStackTop)
      {
         return;
      }
      m_paNode.push_back(pStackTop);
      return;
   }

   const int iParentElement = m_paNode.size() - 2;
   if(iParentElement < 0)
   {
      pStackTop = dynamic_cast<TiXmlElement*>
         (m_qDocument->InsertEndChild( cElement ));
      if(!pStackTop)
      {
         return;
      }
      m_paNode.push_back(pStackTop);
      return;
   }
   else
   {
      TiXmlElement* pNode = m_paNode[iParentElement];
      if(!pNode)
      {
         return;
      }   
      pStackTop = dynamic_cast<TiXmlElement*>
         (pNode->InsertEndChild( cElement ));
      if(!pStackTop)
      {
         return;
      }

      // Overwrite top stack position.
      const int iFinalElement = m_paNode.size() - 1;
      m_paNode[iFinalElement] = pStackTop;
      //pNode = pStackTop;
   }
}

DefineEngineMethod( SimXMLDocument, addNewElement, void, ( const char* name ),,
   "@brief Create a new element with the given name as child of current Element's "
   "parent and push it onto the Element stack making it the current one.\n\n"

   "@note This differs from pushNewElement() in that it adds the new Element to the "
   "current Element's parent (or document if there is no parent Element).  This makes "
   "the new Element a sibling of the current one.\n\n"

   "@param name XML tag for the new Element.\n"
   
   "@see pushNewElement()")
{
   object->addNewElement( name );
}

// -----------------------------------------------------------------------------
// Write XML document declaration.
// -----------------------------------------------------------------------------
void SimXMLDocument::addHeader(void)
{
   TiXmlDeclaration cDeclaration("1.0", "utf-8", "yes");
   m_qDocument->InsertEndChild(cDeclaration);
}

DefineEngineMethod( SimXMLDocument, addHeader, void, (),,
   "@brief Add a XML header to a document.\n\n"

   "Sometimes called a declaration, you typically add a standard header to "
   "the document before adding any elements.  SimXMLDocument always produces "
   "the following header:\n\n"
   "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\" ?>\n\n"
  
   "@tsexample\n"
   "// Create a new XML document with just a header and single element.\n"
   "%x = new SimXMLDocument();\n"
   "%x.addHeader();\n"
   "%x.addNewElement(\"NewElement\");\n"
   "%x.saveFile(\"test.xml\");\n\n"
   "// Produces the following file:\n"
   "// <?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\" ?>\n"
   "// <NewElement />\n"
   "@endtsexample\n\n")
{
   object->addHeader();
}

void SimXMLDocument::addComment(const char* comment)
{
   TiXmlComment cComment;
   cComment.SetValue(comment);
   m_qDocument->InsertEndChild(cComment);
}

DefineEngineMethod(SimXMLDocument, addComment, void, ( const char* comment ),,
  "@brief Add the given comment as a child of the document.\n\n"
  "@param comment String containing the comment."

   "@tsexample\n"
   "// Create a new XML document with a header, a comment and single element.\n"
   "%x = new SimXMLDocument();\n"
   "%x.addHeader();\n"
   "%x.addComment(\"This is a test comment\");\n"
   "%x.addNewElement(\"NewElement\");\n"
   "%x.saveFile(\"test.xml\");\n\n"
   "// Produces the following file:\n"
   "// <?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\" ?>\n"
   "// <!--This is a test comment-->\n"
   "// <NewElement />\n"
   "@endtsexample\n\n"
   
   "@see readComment()")
{
   object->addComment( comment );
}

const char* SimXMLDocument::readComment( S32 index )
{
   // Clear the current attribute pointer
   m_CurrentAttribute = 0;

   // Push the first element found under the current element of the given name
   if(!m_paNode.empty())
   {
      const int iLastElement = m_paNode.size() - 1;
      TiXmlElement* pNode = m_paNode[iLastElement];
      if(!pNode)
      {
         return "";
      }
      TiXmlNode* node = pNode->FirstChild();
      for( S32 i = 0; i < index; i++ )
      {
         if( !node )
            return "";

         node = node->NextSiblingElement();
      }

      if( node )
      {
         TiXmlComment* comment = node->ToComment();
         if( comment )
            return comment->Value();
      }
   }
   else
   {
      if(!m_qDocument)
      {
         return "";
      }
      TiXmlNode* node = m_qDocument->FirstChild();
      for( S32 i = 0; i < index; i++ )
      {
         if( !node )
            return "";

         node = node->NextSibling();
      }

      if( node )
      {
         TiXmlComment* comment = node->ToComment();
         if( comment )
            return comment->Value();
      }
   }
   return "";
}

DefineEngineMethod( SimXMLDocument, readComment, const char*, ( S32 index ),,
   "Gives the comment at the specified index, if any.\n\n"

   "Unlike addComment() that only works at the document level, readComment() may read "
   "comments from the document or any child Element.  The current Element (or document "
   "if no Elements have been pushed to the stack) is the parent for any comments, and the "
   "provided index is the number of comments in to read back."

   "@param index Comment index number to query from the current Element stack\n\n"
   "@return String containing the comment, or an empty string if no comment is found.\n\n"

   "@see addComment()")
{
   return object->readComment( index );
}

void SimXMLDocument::addText(const char* text)
{
   if(m_paNode.empty())
      return;

   const int iFinalElement = m_paNode.size() - 1;
   TiXmlElement* pNode = m_paNode[iFinalElement];
   if(!pNode)
      return;

   TiXmlText cText(text);
   pNode->InsertEndChild( cText );
}

DefineEngineMethod( SimXMLDocument, addText, void, ( const char* text ),,
   "@brief Add the given text as a child of current Element.\n\n"

   "Use getText() to retrieve any text from the current Element and removeText() "
   "to clear any text.\n\n"

   "addText() and addData() may be used interchangeably."

   "@param text String containing the text.\n\n"

   "@tsexample\n"
   "// Create a new XML document with a header and single element\n"
   "// with some added text.\n"
   "%x = new SimXMLDocument();\n"
   "%x.addHeader();\n"
   "%x.addNewElement(\"NewElement\");\n"
   "%x.addText(\"Some text\");\n"
   "%x.saveFile(\"test.xml\");\n\n"
   "// Produces the following file:\n"
   "// <?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\" ?>\n"
   "// <NewElement>Some text</NewElement>\n"
   "@endtsexample\n\n"

   "@see getText()\n"
   "@see removeText()\n"
   "@see addData()\n"
   "@see getData()")
{
   object->addText( text );
}

const char* SimXMLDocument::getText()
{
   if(m_paNode.empty())
      return "";

   const int iFinalElement = m_paNode.size() - 1;
   TiXmlNode* pNode = m_paNode[iFinalElement];
   if(!pNode)
      return "";

   if(!pNode->FirstChild())
      return "";

   TiXmlText* text = pNode->FirstChild()->ToText();
   if( !text )
      return "";

   return text->Value();
}

DefineEngineMethod( SimXMLDocument, getText, const char*, (),,
   "@brief Gets the text from the current Element.\n\n"

   "Use addText() to add text to the current Element and removeText() "
   "to clear any text.\n\n"

   "getText() and getData() may be used interchangeably."

   "@return String containing the text in the current Element."

   "@tsexample\n"
   "// Using the following test.xml file as an example:\n"
   "// <?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\" ?>\n"
   "// <NewElement>Some text</NewElement>\n\n"

   "// Load in the file\n"
   "%x = new SimXMLDocument();\n"
   "%x.loadFile(\"test.xml\");\n\n"

   "// Make the first Element the current one\n"
   "%x.pushFirstChildElement(\"NewElement\");\n\n"

   "// Store the current Element's text ('Some text' in this example)\n"
   "// into 'result'\n"
   "%result = %x.getText();\n"
   "echo( %result );\n"
   "@endtsexample\n\n"
   
   "@see addText()\n"
   "@see removeText()\n"
   "@see addData()\n"
   "@see getData()\n")
{
   const char* text = object->getText();
   if( !text )
      return "";

   return text;
}

void SimXMLDocument::removeText()
{
   if(m_paNode.empty())
      return;

   const int iFinalElement = m_paNode.size() - 1;
   TiXmlElement* pNode = m_paNode[iFinalElement];
   if(!pNode)
      return;

   if( !pNode->FirstChild() )
      return;

   TiXmlText* text = pNode->FirstChild()->ToText();
   if( !text )
      return;

   pNode->RemoveChild(text);
}

DefineEngineMethod( SimXMLDocument, removeText, void, (),,
   "@brief Remove any text on the current Element.\n\n"

   "Use getText() to retrieve any text from the current Element and addText() "
   "to add text to the current Element.  As getData() and addData() are equivalent "
   "to getText() and addText(), removeText() will also remove any data from the "
   "current Element.\n\n"
   
   "@see addText()\n"
   "@see getText()\n"
   "@see addData()\n"
   "@see getData()\n")
{
   object->removeText();
}

void SimXMLDocument::addData(const char* text)
{
   if(m_paNode.empty())
      return;

   const int iFinalElement = m_paNode.size() - 1;
   TiXmlElement* pNode = m_paNode[iFinalElement];
   if(!pNode)
      return;

   TiXmlText cText(text);
   pNode->InsertEndChild( cText );
}

DefineEngineMethod( SimXMLDocument, addData, void, ( const char* text ),,
   "@brief Add the given text as a child of current Element.\n\n"

   "Use getData() to retrieve any text from the current Element.\n\n"

   "addData() and addText() may be used interchangeably.  As there is no "
   "difference between data and text, you may also use removeText() to clear "
   "any data from the current Element.\n\n"

   "@param text String containing the text.\n\n"

   "@tsexample\n"
   "// Create a new XML document with a header and single element\n"
   "// with some added data.\n"
   "%x = new SimXMLDocument();\n"
   "%x.addHeader();\n"
   "%x.addNewElement(\"NewElement\");\n"
   "%x.addData(\"Some text\");\n"
   "%x.saveFile(\"test.xml\");\n\n"
   "// Produces the following file:\n"
   "// <?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\" ?>\n"
   "// <NewElement>Some text</NewElement>\n"
   "@endtsexample\n\n"

   "@see getData()"
   "@see addText()\n"
   "@see getText()\n"
   "@see removeText()\n")
{
   object->addData( text );
}

const char* SimXMLDocument::getData()
{
   if(m_paNode.empty())
      return "";

   const int iFinalElement = m_paNode.size() - 1;
   TiXmlNode* pNode = m_paNode[iFinalElement];
   if(!pNode)
      return "";

   if( !pNode->FirstChild() )
      return "";

   TiXmlText* text = pNode->FirstChild()->ToText();
   if( !text )
      return "";

   return text->Value();
}

DefineEngineMethod( SimXMLDocument, getData, const char*, (),,
   "@brief Gets the text from the current Element.\n\n"

   "Use addData() to add text to the current Element.\n\n"

   "getData() and getText() may be used interchangeably.  As there is no "
   "difference between data and text, you may also use removeText() to clear "
   "any data from the current Element.\n\n"

   "@return String containing the text in the current Element."

   "@tsexample\n"
   "// Using the following test.xml file as an example:\n"
   "// <?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\" ?>\n"
   "// <NewElement>Some data</NewElement>\n\n"

   "// Load in the file\n"
   "%x = new SimXMLDocument();\n"
   "%x.loadFile(\"test.xml\");\n\n"

   "// Make the first Element the current one\n"
   "%x.pushFirstChildElement(\"NewElement\");\n\n"

   "// Store the current Element's data ('Some data' in this example)\n"
   "// into 'result'\n"
   "%result = %x.getData();\n"
   "echo( %result );\n"
   "@endtsexample\n\n"
   
   "@see addData()\n"
   "@see addText()\n"
   "@see getText()\n"
   "@see removeText()\n")
{
   const char* text = object->getData();
   if( !text )
      return "";

   return text;
}

////EOF/////////////////////////////////////////////////////////////////////////
