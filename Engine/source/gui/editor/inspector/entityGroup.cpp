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
#include "gui/editor/inspector/entityGroup.h"
#include "core/strings/stringUnit.h"
#include "T3D/components/component.h"

#include "console/engineAPI.h"

IMPLEMENT_CONOBJECT(GuiInspectorEntityGroup);

ConsoleDocClass(GuiInspectorEntityGroup,
   "@brief Used to inspect an object's FieldDictionary (dynamic fields) instead "
   "of regular persistent fields.\n\n"
   "Editor use only.\n\n"
   "@internal"
   );

bool GuiInspectorEntityGroup::onAdd()
{
   if (!Parent::onAdd())
      return false;

   return true;
}

//-----------------------------------------------------------------------------
// GuiInspectorEntityGroup - add custom controls
//-----------------------------------------------------------------------------
bool GuiInspectorEntityGroup::createContent()
{
   if(!Parent::createContent())
      return false;

   Con::evaluatef("%d.stack = %d;", this->getId(), mStack->getId());

   Con::executef(this, "createContent");

   return true;
}

//-----------------------------------------------------------------------------
// GuiInspectorEntityGroup - inspectGroup override
//-----------------------------------------------------------------------------
bool GuiInspectorEntityGroup::inspectGroup()
{
   const U32 numTargets = mParent->getNumInspectObjects();
   if (numTargets == 1)
   {
      Entity* target = dynamic_cast<Entity*>(mParent->getInspectObject(0));

      Con::executef(this, "inspectObject", target->getIdString());
   }

   return true;
}

void GuiInspectorEntityGroup::updateAllFields()
{
   // We overload this to just reinspect the group.
   inspectGroup();
}

void GuiInspectorEntityGroup::onMouseMove(const GuiEvent &event)
{
   //mParent->mOverDivider = false;
}
ConsoleMethod(GuiInspectorEntityGroup, inspectGroup, bool, 2, 2, "Refreshes the dynamic fields in the inspector.")
{
   return object->inspectGroup();
}

void GuiInspectorEntityGroup::clearFields()
{
}

SimFieldDictionary::Entry* GuiInspectorEntityGroup::findDynamicFieldInDictionary(StringTableEntry fieldName)
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

void GuiInspectorEntityGroup::addDynamicField()
{
}

AbstractClassRep::Field* GuiInspectorEntityGroup::findObjectBehaviorField(Component* target, String fieldName)
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
ConsoleMethod(GuiInspectorEntityGroup, addDynamicField, void, 2, 2, "obj.addDynamicField();")
{
   object->addDynamicField();
}

ConsoleMethod(GuiInspectorEntityGroup, removeDynamicField, void, 3, 3, "")
{
}
