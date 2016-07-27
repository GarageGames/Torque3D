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

#include "gui/buttons/guiIconButtonCtrl.h"
#include "gui/editor/guiInspector.h"
#include "gui/editor/inspector/componentGroup.h"
#include "core/strings/stringUnit.h"
#include "T3D/components/component.h"
#include "gui/editor/inspector/field.h"

#include "console/engineAPI.h"

IMPLEMENT_CONOBJECT(GuiInspectorComponentGroup);

ConsoleDocClass(GuiInspectorComponentGroup,
   "@brief Used to inspect an object's FieldDictionary (dynamic fields) instead "
   "of regular persistent fields.\n\n"
   "Editor use only.\n\n"
   "@internal"
   );

GuiInspectorComponentGroup::GuiInspectorComponentGroup(StringTableEntry groupName, SimObjectPtr<GuiInspector> parent)
: GuiInspectorGroup(groupName, parent) 
{ 
   /*mNeedScroll=false;*/
};

bool GuiInspectorComponentGroup::onAdd()
{
   if (!Parent::onAdd())
      return false;

   return true;
}

//-----------------------------------------------------------------------------
// GuiInspectorComponentGroup - add custom controls
//-----------------------------------------------------------------------------
bool GuiInspectorComponentGroup::createContent()
{
   if(!Parent::createContent())
      return false;

   Con::evaluatef("%d.stack = %d;", this->getId(), mStack->getId());

   Con::executef(this, "createContent");

   return true;
}

//-----------------------------------------------------------------------------
// GuiInspectorComponentGroup - inspectGroup override
//-----------------------------------------------------------------------------
bool GuiInspectorComponentGroup::inspectGroup()
{
   // We can't inspect a group without a target!
   if (!mParent || !mParent->getNumInspectObjects())
      return false;

   clearFields();

   // to prevent crazy resizing, we'll just freeze our stack for a sec..
   mStack->freeze(true);

   bool bNoGroup = false;

   bool bNewItems = false;
   bool bMakingArray = false;
   GuiStackControl *pArrayStack = NULL;
   GuiRolloutCtrl *pArrayRollout = NULL;
   bool bGrabItems = false;

   Component* comp = dynamic_cast<Component*>(getInspector()->getInspectObject(0));

   //if this isn't a component, what are we even doing here?
   if (!comp)
      return false;

   for (U32 i = 0; i < comp->getComponentFieldCount(); i++)
   {
      ComponentField* field = comp->getComponentField(i);

      bNewItems = true;

      GuiInspectorField *fieldGui = constructField(field->mFieldType);
      if (fieldGui == NULL)
         fieldGui = new GuiInspectorField();

      fieldGui->init(mParent, this);

      AbstractClassRep::Field *refField;

      //check dynamics
      SimFieldDictionary* fieldDictionary = comp->getFieldDictionary();
      SimFieldDictionaryIterator itr(fieldDictionary);

      while (*itr)
      {
         SimFieldDictionary::Entry* entry = *itr;
         if (entry->slotName == field->mFieldName)
         {
            AbstractClassRep::Field f;
            f.pFieldname = StringTable->insert(field->mFieldName);

            if (field->mFieldDescription)
               f.pFieldDocs = field->mFieldDescription;

            f.type = field->mFieldType;
            f.offset = -1;
            f.elementCount = 1;
            f.validator = NULL;
            f.flag = 0; //change to be the component type

            f.setDataFn = &defaultProtectedSetFn;
            f.getDataFn = &defaultProtectedGetFn;
            f.writeDataFn = &defaultProtectedWriteFn;

            f.pFieldDocs = field->mFieldDescription;

            f.pGroupname = "Component Fields";

            ConsoleBaseType* conType = ConsoleBaseType::getType(field->mFieldType);
            AssertFatal(conType, "ConsoleObject::addField - invalid console type");
            f.table = conType->getEnumTable();

            tempFields.push_back(f);

            refField = &f;

            break;
         }
         ++itr;
      }

      if (!refField)
         continue;

      fieldGui->setInspectorField(&tempFields[tempFields.size() - 1]);

      if (fieldGui->registerObject())
      {
#ifdef DEBUG_SPEW
         Platform::outputDebugString("[GuiInspectorGroup] Adding field '%s'",
            field->pFieldname);
#endif

         mChildren.push_back(fieldGui);
         mStack->addObject(fieldGui);
      }
      else
      {
         SAFE_DELETE(fieldGui);
      }
   }

   mStack->freeze(false);
   mStack->updatePanes();

   // If we've no new items, there's no need to resize anything!
   if (bNewItems == false && !mChildren.empty())
      return true;

   sizeToContents();

   setUpdate();

   return true;
}

void GuiInspectorComponentGroup::updateAllFields()
{
   // We overload this to just reinspect the group.
   inspectGroup();
}

void GuiInspectorComponentGroup::onMouseMove(const GuiEvent &event)
{
   //mParent->mOverDivider = false;
}
ConsoleMethod(GuiInspectorComponentGroup, inspectGroup, bool, 2, 2, "Refreshes the dynamic fields in the inspector.")
{
   return object->inspectGroup();
}

void GuiInspectorComponentGroup::clearFields()
{
   // delete everything else
   mStack->clear();

   // clear the mChildren list.
   mChildren.clear();
}

SimFieldDictionary::Entry* GuiInspectorComponentGroup::findDynamicFieldInDictionary(StringTableEntry fieldName)
{
   SimFieldDictionary * fieldDictionary = mParent->getInspectObject()->getFieldDictionary();

   for (SimFieldDictionaryIterator ditr(fieldDictionary); *ditr; ++ditr)
   {
      SimFieldDictionary::Entry * entry = (*ditr);

      if (entry->slotName == fieldName)
         return entry;
   }

   return NULL;
}

void GuiInspectorComponentGroup::addDynamicField()
{
}

AbstractClassRep::Field* GuiInspectorComponentGroup::findObjectBehaviorField(Component* target, String fieldName)
{
   AbstractClassRep::FieldList& fieldList = target->getClassRep()->mFieldList;
   for (AbstractClassRep::FieldList::iterator itr = fieldList.begin();
      itr != fieldList.end(); ++itr)
   {
      AbstractClassRep::Field* field = &(*itr);
      String fldNm(field->pFieldname);
      if (fldNm == fieldName)
         return field;
   }
   return NULL;
}
ConsoleMethod(GuiInspectorComponentGroup, addDynamicField, void, 2, 2, "obj.addDynamicField();")
{
   object->addDynamicField();
}

ConsoleMethod(GuiInspectorComponentGroup, removeDynamicField, void, 3, 3, "")
{
}
