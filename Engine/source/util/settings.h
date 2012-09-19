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

#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include "console/simBase.h"
#include "core/util/tVector.h"

class SimXMLDocument;

///
class Settings : public SimObject
{
private:
   FileName         mFile;
   Vector<String>   mGroupStack;
	S32              mSearchPos;
	Vector<String>   mSearchResults;

public:
   Settings();
   virtual ~Settings();

   // Required in all ConsoleObject subclasses.
   typedef SimObject Parent;
   DECLARE_CONOBJECT(Settings);
   static void initPersistFields();

   /// These will set and get the values, with an option default value passed in to the get 
   void setDefaultValue(const UTF8 *settingName, const UTF8 *settingValue, const UTF8 *settingType="");
   void setValue(const UTF8 *settingName, const UTF8 *settingValue = "");
   const UTF8 *value(const UTF8 *settingName, const UTF8 *defaultValue = "");
   void remove(const UTF8 *settingName, bool includeDefaults = false);
   void clearAllFields();
   bool write();
   bool read();
   void readLayer(SimXMLDocument *document, String groupStack = String(""));

   void beginGroup(const UTF8 *groupName, bool fromStart = false);
   void endGroup();
   void clearGroups();
   
   void buildGroupString(String &name, const UTF8 *settingName);
   const UTF8 *getCurrentGroups();

	//S32 buildSearchList(const char* pattern, bool deepSearch = false, bool defaultsSearch = false);
	const char* findFirstValue(const char* pattern, bool deepSearch = false, bool includeDefaults = false);
	const char* findNextValue();
};

class SettingSaveNode
{
public:
   Vector<SettingSaveNode*> mGroupNodes;
   Vector<SettingSaveNode*> mSettingNodes;

   String mName;
   String mValue;
   bool mIsGroup;

   SettingSaveNode(){};
   SettingSaveNode(const String &name, bool isGroup = false)
   {
      mName = name;
	   mIsGroup = isGroup;
   }
   SettingSaveNode(const String &name, const String &value)
   {
      mName = name;
	   mValue = value;
	   mIsGroup = false;
   }
   ~SettingSaveNode()
   {
      clear();
   }

   void addValue(const UTF8 *name, const UTF8 *value);
   S32 getGroupCount(const String &name);
   String getGroup(const String &name, S32 num);
   String getSettingName(const String &name);
   void buildDocument(SimXMLDocument *document, bool skipWrite = false);

   void clear();
};

#endif