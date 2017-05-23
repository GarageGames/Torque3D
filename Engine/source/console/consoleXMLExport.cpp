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

#include "console/consoleInternal.h"
#include "console/engineAPI.h"
#include "console/consoleObject.h"
#include "console/SimXMLDocument.h"

#include "console/consoleXMLExport.h"


#if 0

namespace Con {

   XMLExport::XMLExport()
   {
      mXML = NULL;
   }

   XMLExport::~XMLExport()
   {
   }

   void XMLExport::exportBaseTypes()
   {
      mXML->pushNewElement("BaseTypes");

      ConsoleBaseType *walk = ConsoleBaseType::getListHead();
      while( walk != NULL )
      {
         mXML->pushNewElement("BaseType");

         mXML->setAttribute("name", walk->getTypeName());
         mXML->setAttribute("id", avar("%i",walk->getTypeID()));
         mXML->setAttribute("size", avar("%i",walk->getTypeSize()));
         mXML->setAttribute("doc", walk->getDocString() ? walk->getDocString() : "" );

         mXML->popElement(); // Basetype

         walk = walk->getListNext();
      }

      mXML->popElement(); // Basetypes

   }

   void XMLExport::exportEntryTypes()
   {

      const char *typeNames [] = { 
         "ScriptCallbackType", "GroupMarker", "OverloadMarker", "InvalidFunctionType",
         "ConsoleFunctionType", "StringCallbackType", "IntCallbackType", "FloatCallbackType",
         "VoidCallbackType", "BoolCallbackType"
      };

      S32 typeIds [] =  {
         Namespace::Entry::ScriptCallbackType, Namespace::Entry::GroupMarker, Namespace::Entry::InvalidFunctionType,
         Namespace::Entry::ConsoleFunctionType, Namespace::Entry::StringCallbackType, Namespace::Entry::IntCallbackType, Namespace::Entry::FloatCallbackType,
         Namespace::Entry::VoidCallbackType, Namespace::Entry::BoolCallbackType
      };

      mXML->pushNewElement("EntryTypes");

      S32 numElements = sizeof(typeIds) / sizeof ( S32);

      for (S32 i = 0; i < numElements; i++)
      {
         mXML->pushNewElement("EntryType");
         mXML->setAttribute("name", typeNames[i]);
         mXML->setAttribute("id", avar("%i", typeIds[i]));
         mXML->popElement();
      }

      mXML->popElement(); // EntryTypes
   }

   void XMLExport::exportNamespaces()
   {

      // keep track of which enumTables are in use
      Vector < const EnumTable*> enumTables;

      mXML->pushNewElement("Namespaces");

      for (Namespace *walk = Namespace::mNamespaceList; walk; walk = walk->mNext)
      {

         if ( walk->mName && !walk->isClass() )
            continue;

         const char *name = walk->mName ? walk->mName : "";

         mXML->pushNewElement("Namespace");
         mXML->setAttribute("name", name);

         Namespace *p = walk->mParent;

         mXML->pushNewElement("Parents");

         while (p)
         {
            if (p->mName == walk->mName)
            {
               p = p->mParent;
               continue;
            }

            const char* pname = p->mName ? p->mName : "";

            mXML->pushNewElement("Parent");
            mXML->setAttribute("name", pname);
            mXML->popElement(); // Parent

            p = p->mParent;
         }

         mXML->popElement(); // Parents

         // Entries (Engine/Script Methods/Functions)

         mXML->pushNewElement("Entries");

         Namespace::Entry *entry;
         VectorPtr<Namespace::Entry *> vec;

         walk->getEntryList(&vec);

         for( NamespaceEntryListIterator compItr = vec.begin(); compItr != vec.end(); compItr++ )
         {

            entry = *compItr;

            if (entry->mNamespace != walk)
               continue;

            if (entry->mNamespace->mName != walk->mName)
               continue;

            mXML->pushNewElement("Entry");

            //consistently name functions
            char functionName[512];
            dSprintf(functionName, 512, entry->mFunctionName);
            functionName[0] = dTolower(functionName[0]);

            S32 minArgs = entry->mMinArgs;
            S32 maxArgs = entry->mMaxArgs;

            if (maxArgs < minArgs)
               maxArgs = minArgs;

            mXML->setAttribute("name", functionName);
            mXML->setAttribute("minArgs", avar("%i", minArgs));
            mXML->setAttribute("maxArgs", avar("%i", maxArgs));

            const char* usage = "";
            if (entry->mUsage && entry->mUsage[0])
               usage = entry->mUsage;
            mXML->setAttribute("usage", usage);
            mXML->setAttribute("package", entry->mPackage ? entry->mPackage : "");
            mXML->setAttribute("entryType", avar("%i", entry->mType));

            mXML->popElement(); // Entry

         }

         mXML->popElement(); // Entries

         // Fields

         mXML->pushNewElement("Fields");

         AbstractClassRep *rep = walk->mClassRep;
         Vector<U32> classFields;

         if (rep)
         {
            AbstractClassRep *parentRep = rep->getParentClass();

            const AbstractClassRep::FieldList& flist = rep->mFieldList;

            for(U32 i = 0; i < flist.size(); i++)
            {
               if (parentRep)
               {
                  if (parentRep->findField(flist[i].pFieldname))
                     continue;

               }
               classFields.push_back(i);
            }

            for(U32 i = 0; i < classFields.size(); i++)
            {
               U32 index = classFields[i];

               char fieldName[256];

               dSprintf(fieldName, 256, flist[index].pFieldname);

               //consistently name fields
               fieldName[0] = dToupper(fieldName[0]);

               mXML->pushNewElement("Field");

               mXML->setAttribute("name", fieldName);
               mXML->setAttribute("type", avar("%i", flist[index].type));

// RD: temporarily deactivated; TypeEnum is no more; need to sync this up
//               if (flist[index].type == TypeEnum  && flist[index].table && dStrlen(flist[index].table->name))
//               {
//                  if (!enumTables.contains(flist[index].table))
//                     enumTables.push_back(flist[index].table);
//
//                  mXML->setAttribute("enumTable", flist[index].table->name);
//
//               }

               const char* pFieldDocs = "";
               if (flist[index].pFieldDocs && flist[index].pFieldDocs[0])
                  pFieldDocs = flist[index].pFieldDocs;

               mXML->setAttribute("docs", pFieldDocs);
               mXML->setAttribute("elementCount", avar("%i", flist[index].elementCount));

               mXML->popElement(); // Field
            }
         }

         mXML->popElement(); // Fields
         mXML->popElement(); // Namespace
      }

      mXML->popElement(); // Namespaces

      mXML->pushNewElement("EnumTables");

      // write out the used EnumTables
      for (S32 i = 0; i < enumTables.size(); i++)
      {
         mXML->pushNewElement("EnumTable");

         const EnumTable* table = enumTables[i];

         mXML->setAttribute("name", table->name);
         mXML->setAttribute("firstFlag", avar("%i", table->firstFlag));
         mXML->setAttribute("mask", avar("%i", table->mask));

         mXML->pushNewElement("Enums");

         for (S32 j = 0; j < table->size; j++)
         {
            mXML->pushNewElement("Enum");

            mXML->setAttribute("name", table->table[j].label);
            mXML->setAttribute("index", avar("%i", table->table[j].index));

            mXML->popElement(); // Enum

         }

         mXML->popElement(); //Enums

         mXML->popElement(); // EnumTable
      }

      mXML->popElement(); // EnumTables
      
   }

   void XMLExport::exportXML(String& str)
   {
      mXML = new SimXMLDocument();
      mXML->registerObject();

      mXML->addHeader();

      mXML->pushNewElement("ConsoleXML");

      exportBaseTypes();
      exportEntryTypes();
      exportNamespaces();

      mXML->popElement(); // TorqueConsole

      mXML->saveToString(str);

      // If you're having trouble with the generated XML, you can dump to a file and inspect
      // mXML->saveFile("ConsoleExport.xml");

      mXML->deleteObject();
   }

}; // namespace Con


DefineConsoleFunction( consoleExportXML, const char*, (), ,"Exports console definition XML representation" )
{
   Con::XMLExport xmlExport;
   String xml;
   xmlExport.exportXML(xml);
   char* ret = Con::getReturnBuffer(xml.length() + 1);
   dStrcpy(ret, xml.c_str());
   return ret;
}

#endif
