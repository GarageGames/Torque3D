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

#ifndef _PERSISTENCEMANAGER_H_
#define _PERSISTENCEMANAGER_H_

#ifndef _SIMOBJECT_H_
#include "console/simObject.h"
#endif

#ifndef _SIMOBJECTLIST_H_
#include "console/simObjectList.h"
#endif

#ifndef _TOKENIZER_H_
#include "core/tokenizer.h"
#endif

class PersistenceManager : public SimObject
{
public:
   struct ParsedProperty
   {
      StringTableEntry name;
      const char*      value;

      U32 arrayPos;

      S32 startLine;
      S32 endLine;

      S32 startPosition;
      S32 endPosition;

      S32 valueLine;
      S32 valuePosition;

      ParsedProperty()
      {
         name           = NULL;
         value          = NULL;

         arrayPos       = 0;

         startLine      = -1;
         endLine        = -1;

         startPosition  = -1;
         endPosition    = -1;

         valueLine      = -1;
         valuePosition  = -1;
      }
   };

   struct ParsedObject
   {
      StringTableEntry name;
      StringTableEntry className;

      ParsedObject* parentObject;

      SimObjectPtr<SimObject>  simObject;

      S32 nameLine;
      S32 namePosition;

      S32 startLine;
      S32 endLine;

      S32 startPosition;
      S32 endPosition;

      bool hasBraces;

      bool updated;

      Vector<ParsedProperty> properties;

      ParsedObject()
      {
         name      = NULL;
         className = NULL;

         parentObject = NULL;

         simObject = NULL;

         nameLine     = -1;
         namePosition = -1;

         startLine = -1;
         endLine   = -1;

         startPosition = -1;
         endPosition   = -1;

         hasBraces = true;

         updated = false;
      }
   };

   struct DirtyObject
   {
      SimObjectPtr<SimObject> *object;
      StringTableEntry fileName;

      bool isNull() const { return object->isNull(); }

      void setObject( SimObject* newObject ) { *object = newObject; }

      SimObject* getObject() const { return object->getPointer(); }

      DirtyObject()
      {
         object   = new SimObjectPtr<SimObject>();
         fileName = NULL;
      }

      ~DirtyObject()
      {
         SAFE_DELETE( object );
      }
   };

   struct RemoveField
   {
      SimObjectPtr<SimObject> object;
      StringTableEntry fieldName;
      U32 arrayPos;

      RemoveField()
      {
         object    = NULL;
         fieldName = NULL;
         arrayPos  = 0;
      }
   };

   typedef Vector<DirtyObject> DirtyList;

protected:
   typedef SimObject Parent;

   // Used to walk through the file and read out
   // the SimObject's and their properties
   Tokenizer mParser;

   // List of the objects that are flagged as dirty
   DirtyList mDirtyObjects;

   // List of fields to be removed from the objects declaration in the file
   Vector<RemoveField>     mRemoveFields;

   // Temp buffers used during file parsing
   ParsedObject*           mCurrentObject;
   Vector<ParsedObject*>   mObjectStack;

   // Buffers used on a per-file basis
   Vector<const char*>     mLineBuffer;
   Vector<ParsedObject*>   mObjectBuffer;

   // Name of the currently open file
   const char*             mCurrentFile;

   // Sort by filename
   static S32 QSORT_CALLBACK compareFiles(const void* a, const void* b);

   // Deletes and clears the line buffer
   void clearLineBuffer();

   // Deletes the objects and its properties
   void deleteObject(ParsedObject* object);
   // Deletes and clears the object buffer,
   // the object stack, and the current object
   void clearObjects();

   // Clears all of the data related to the
   // currently loaded file
   void clearFileData();

   // Updates the changed values of a dirty object
   // Also handles a new object
   void updateObject(SimObject* object, ParsedObject* parentObject = NULL);

   // Removes the current object without saving it
   void killObject();
   // Saves the current object and restores the last object in the stack (if any)
   void saveObject();
   // Parses an object from the current position in the parser
   void parseObject();

   // Reads the file into the line buffer
   bool readFile(const char* fileName);
   // Parses the ParsedObjects out of the file
   bool parseFile(const char* fileName);

   // Writes the line buffer out to the current file
   bool saveDirtyFile();

   // Attempts to look up the property in the ParsedObject
   S32 getPropertyIndex(ParsedObject* parsedObject, const char* fieldName, U32 arrayPos = 0);

   // Gets the amount of indent on the ParsedObject.
   char* getObjectIndent(ParsedObject* object);

   // Loops through all of the objects and properties that are on the same
   // line after the startPos and updates their position offests accordingly
   void updatePositions(U32 lineNumber, U32 startPos, S32 diff);

   // Loops thought all of the objects and updates their line offsets
   void updateLineOffsets(U32 startLine, S32 diff, ParsedObject* skipObject = NULL);

   // Replaces a token on a given line with a new value
   // This also calls updatePositions() to account for size
   // differences between the old token and the new token
   void updateToken(const U32 lineNumber, const U32 linePosition, const U32 oldValueLen, const char* newValue, bool addQuotes = false);

   // Gets the field value from the SimObject. Note that this does
   // allocate memory that needs to be cleaned up elsewhere
   const char* getFieldValue(SimObject* object, const char* fieldName, U32 arrayPos);

   // Attempt to find the parent object
   ParsedObject* findParentObject(SimObject* object, ParsedObject* parentObject = NULL);

   // Attempt to find the matching ParsedObject in our object buffer
   ParsedObject* findParsedObject(SimObject* object, ParsedObject* parentObject = NULL);

   // Attempt to find the matching DirtyObject for a passed SimObject in our DirtyItems list.
   DirtyObject* findDirtyObject(SimObject* object);

   // Is this field on the remove list
   bool findRemoveField(SimObject* object, const char* fieldName, U32 arrayPos = 0);

   // Helper function that allocates a new string and properly formats the data into it
   // Note that this allocates memory that needs to be cleaned up elsewhere
   const char* createNewProperty(const char* name, const char* value, bool isArray = false, U32 arrayPos = 0);

   // Test to see if there is anything valid on the line
   bool isEmptyLine(const char* line);

   // Removes a line safely from the line buffer
   void removeLine(U32 lineNumber);
   // Remove a block of text from the line buffer. It returns
   // the number of lines removed if removeEmptyLines is set to true.
   void removeTextBlock(U32 startLine, U32 endLine, U32 startPos, U32 endPos, bool removeEmptyLines);
   // Removes a ParsedObject from the line buffer
   // (everything from the startLine to the endLine)
   void removeParsedObject(ParsedObject* parsedObject);
   // Removes a property from the line buffer
   void removeField(const ParsedProperty& prop);

   // Write out properties
   // Returns the number of new lines added
   U32 writeProperties(const Vector<const char*>& properties, const U32 insertLine, const char* objectIndent);
   // Write out a new object
   ParsedObject* writeNewObject(SimObject* object, const Vector<const char*>& properties, const U32 insertLine, ParsedObject* parentObject = NULL);

public:
   PersistenceManager();
   virtual ~PersistenceManager();

   bool onAdd();
   void onRemove();

   // Adds an object to the dirty list
   // Optionally changes the object's filename
   bool setDirty(SimObject* object, const char* fileName = NULL);
   // Removes the object from the dirty list
   void removeDirty(SimObject* object);

   // Add a field to the remove list to cut it out of the object's declaration
   void addRemoveField(SimObject* object, const char* fieldName);

   // Test to see if an object is on the dirty list
   bool isDirty(SimObject* object);   

   // Returns whether or not there are dirty objects
   bool hasDirty() const { return !mDirtyObjects.empty(); }

   // Saves the dirty objects out to their respective files
   // and clears the dirty object data
   bool saveDirty();

   // Saves out a single object, if it's dirty
   bool saveDirtyObject(SimObject* object);

   // Clears all of the dirty object data
   void clearAll();

   // Removes the object from the file
   void removeObjectFromFile(SimObject* object, const char* fileName = NULL);

   // Deletes all of the objects that were created from this file
   void deleteObjectsFromFile(const char* fileName);

   // Returns a list of the dirty objects
   const DirtyList& getDirtyList() { return mDirtyObjects; }

   DECLARE_CONOBJECT(PersistenceManager);
};

#endif