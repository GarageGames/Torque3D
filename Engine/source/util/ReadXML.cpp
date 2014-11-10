


#include "util/ReadXML.h"

#include "console/consoleTypes.h"
#include "console/SimXMLDocument.h"
#include "console/engineAPI.h"

IMPLEMENT_CONOBJECT(ReadXML);

ConsoleDocClass( ReadXML,
				"@brief Class used for writing out preferences and settings for editors\n\n"
				"Not intended for game development, for editors or internal use only.\n\n "
				"@internal");

ReadXML::ReadXML(void)
{
	mFileName = "";
}


ReadXML::~ReadXML(void)
{
}

void ReadXML::initPersistFields()
{
	addField("fileName", TypeStringFilename, Offset(mFileName, ReadXML), "The file path and name to be saved to and loaded from.");

   Parent::initPersistFields();
}

bool ReadXML::readFile()
{
	SimXMLDocument *document = new SimXMLDocument();
	document->registerObject();
	bool output;
	if(document->loadFile(mFileName))
	{

		// set our base element
		if(document->pushFirstChildElement(getName()))
		{
			setModStaticFields( true );
			readLayer(document);
			output = true;
		}
		else
			output = false;
	}
	else
		output = false;

	document->unregisterObject();
	return output;
}

void ReadXML::readLayer(SimXMLDocument *document)
{
   for(S32 i=0; document->pushChildElement(i); i++)
   {
	  const UTF8 *type = document->elementValue();
	  const UTF8 *name = document->attribute("name");
	  const UTF8 *value = document->getText();
	  
	  static SimObject* object;
	  if(dStrcmp(type, "Group") == 0)
	  {
		  const UTF8 *fileName = document->attribute("fileName");
		  const S32 lineNumber = (dAtoi)(document->attribute("lineNumber"));
		  object = Sim::findObject(fileName, lineNumber);
		  if(object)
		  {
			  readLayer(document);
		  }
	  } 
	  else if(dStrcmp(type, "Setting") == 0)
	  {
		  const UTF8 *id = document->attribute("id");
		  if(object)
		  {
			  if( dStricmp(id, "") )
				  object->setDataField( name, id, value);
			  else
				  object->setDataField( name, NULL, value);
		  }
	  }
	  
	  document->popElement();
   }
}


DefineConsoleMethod(ReadXML, readFile, bool, (), , "readXMLObj.readFile();")
{   
   return object->readFile();
}
