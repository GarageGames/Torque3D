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

#ifndef GUI_INSPECTOR_MOUNTINGGROUP_H
#define GUI_INSPECTOR_MOUNTINGGROUP_H

#include "gui/editor/inspector/group.h"
#include "console/simFieldDictionary.h"
#include "T3D/components/component.h"
#include "gui/controls/guiPopUpCtrlEx.h"

#ifndef _GUI_INSPECTOR_TYPES_H_
#include "gui/editor/guiInspectorTypes.h"
#endif

#ifndef _ENTITY_H_
#include "T3D/entity.h"
#endif

class GuiInspectorMountingGroup;

class GuiInspectorNodeListField : public GuiInspectorField
{
   typedef GuiInspectorField Parent;
	friend class GuiInspectorMountingGroup;

public:

   GuiInspectorNodeListField( GuiInspector *inspector, GuiInspectorGroup* parent, SimFieldDictionary::Entry* field,
																	 SimObjectPtr<Entity> target  );
   GuiInspectorNodeListField();
   ~GuiInspectorNodeListField() {};

   DECLARE_CONOBJECT( GuiInspectorNodeListField );

   virtual void             setData( const char* data, bool callbacks = true );
   virtual const char*      getData( U32 inspectObjectIndex = 0 );
   virtual void             updateValue();
   virtual StringTableEntry getFieldName() { return StringTable->EmptyString(); }

   virtual void setDoc( const char* doc );
   virtual void setToolTip( StringTableEntry data );

   virtual bool onAdd();

   virtual void setInspectorField( AbstractClassRep::Field *field, 
                                   StringTableEntry caption = NULL,
                                   const char *arrayIndex = NULL );
   
   virtual GuiControl* constructEditControl();

   virtual void setValue( const char* newValue );

	void setTargetEntity(SimObjectPtr<Entity> target);

protected:

   virtual void _executeSelectedCallback();

protected:

   String mCustomValue;
   StringTableEntry mDoc;

	GuiPopUpMenuCtrl	*mMenu;

	SimObjectPtr<Entity> mTargetEntity;
};

class GuiInspectorMountingGroup : public GuiInspectorGroup
{
private:
   typedef GuiInspectorGroup Parent;
   GuiControl* mAddCtrl;

   GuiPopUpMenuCtrlEx* mAddBhvrList;

	GuiTextCtrl		*persistText;
	GuiButtonCtrl *reloadFile;
	GuiButtonCtrl *saveFile;
	GuiButtonCtrl *overwriteFile;
	GuiButtonCtrl *mBrowseButton;
	GuiControl    *filePath;

	GuiControl			*targetMountCtrl;
	GuiTextCtrl			*targetMountText;
	GuiPopUpMenuCtrl	*targetMountNode;

	GuiControl			*mountCtrl;
	GuiTextCtrl			*mountText;
	GuiPopUpMenuCtrl	*mountNode;

	GuiInspectorNodeListField* mountNodeList;
	GuiInspectorNodeListField* targetMountNodeList;

	SimObjectPtr<GuiInspector>   mParentInspector;

public:
   DECLARE_CONOBJECT(GuiInspectorMountingGroup);
   GuiInspectorMountingGroup() { /*mNeedScroll=false;*/ };
   GuiInspectorMountingGroup( StringTableEntry groupName, SimObjectPtr<GuiInspector> parent );

   //-----------------------------------------------------------------------------
   // inspectGroup is overridden in GuiInspectorMountingGroup to inspect an 
   // objects FieldDictionary (dynamic fields) instead of regular persistent
   // fields.
   bool inspectGroup();
   virtual void updateAllFields();

   void onMouseMove(const GuiEvent &event);

   // For scriptable dynamic field additions
   void addDynamicField();

   // Clear our fields (delete them)
   void clearFields();

	virtual bool resize( const Point2I &newPosition, const Point2I &newExtent );

   // Find an already existent field by name in the dictionary
   virtual SimFieldDictionary::Entry* findDynamicFieldInDictionary( StringTableEntry fieldName );

   AbstractClassRep::Field* findObjectComponentField(Component* target, String fieldName);
protected:
   // create our inner controls when we add
   virtual bool createContent();

	GuiControl* buildMenuCtrl();

	bool buildList(Entity* ent, GuiPopUpMenuCtrl* menu);
};

#endif
