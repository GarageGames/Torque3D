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

GuiInspectorComponentGroup::GuiInspectorComponentGroup(StringTableEntry groupName, SimObjectPtr<GuiInspector> parent, Component* targetComponent)
: GuiInspectorGroup(groupName, parent) 
{ 
   /*mNeedScroll=false;*/
   mTargetComponent = targetComponent;
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

   //if this isn't a component, what are we even doing here?
   if (!mTargetComponent)
      return false;

   mParent->setComponentGroupTargetId(mTargetComponent->getId());

   //first, relevent static fields
   AbstractClassRep::FieldList& fieldList = mTargetComponent->getClassRep()->mFieldList;
   for (AbstractClassRep::FieldList::iterator itr = fieldList.begin();
      itr != fieldList.end(); ++itr)
   {
      AbstractClassRep::Field* field = &(*itr);
      if (field->type == AbstractClassRep::StartGroupFieldType)
      {
         // If we're dealing with general fields, always set grabItems to true (to skip them)
         if (bNoGroup == true)
            bGrabItems = true;
         else if (dStricmp(field->pGroupname, mCaption) == 0)
            bGrabItems = true;
         continue;
      }
      else if (field->type == AbstractClassRep::EndGroupFieldType)
      {
         // If we're dealing with general fields, always set grabItems to false (to grab them)
         if (bNoGroup == true)
            bGrabItems = false;
         else if (dStricmp(field->pGroupname, mCaption) == 0)
            bGrabItems = false;
         continue;
      }

      // Skip field if it has the HideInInspectors flag set.
      if (field->flag.test(AbstractClassRep::FIELD_HideInInspectors))
         continue;

      if (field->pFieldname == StringTable->insert("locked") || field->pFieldname == StringTable->insert("class")
         || field->pFieldname == StringTable->insert("internalName"))
         continue;

      if (/*(bGrabItems == true || (bNoGroup == true && bGrabItems == false)) &&*/ itr->type != AbstractClassRep::DeprecatedFieldType)
      {
         if (bNoGroup == true && bGrabItems == true)
            continue;

         if (field->type == AbstractClassRep::StartArrayFieldType)
         {
#ifdef DEBUG_SPEW
            Platform::outputDebugString("[GuiInspectorGroup] Beginning array '%s'",
               field->pFieldname);
#endif

            // Starting an array...
            // Create a rollout for the Array, give it the array's name.
            GuiRolloutCtrl *arrayRollout = new GuiRolloutCtrl();
            GuiControlProfile *arrayRolloutProfile = dynamic_cast<GuiControlProfile*>(Sim::findObject("GuiInspectorRolloutProfile0"));

            arrayRollout->setControlProfile(arrayRolloutProfile);
            //arrayRollout->mCaption = StringTable->insert( String::ToString( "%s (%i)", field->pGroupname, field->elementCount ) );
            arrayRollout->setCaption(field->pGroupname);
            //arrayRollout->setMargin( 14, 0, 0, 0 );
            arrayRollout->registerObject();

            GuiStackControl *arrayStack = new GuiStackControl();
            arrayStack->registerObject();
            arrayStack->freeze(true);
            arrayRollout->addObject(arrayStack);

            // Allocate a rollout for each element-count in the array
            // Give it the element count name.
            for (U32 i = 0; i < field->elementCount; i++)
            {
               GuiRolloutCtrl *elementRollout = new GuiRolloutCtrl();
               GuiControlProfile *elementRolloutProfile = dynamic_cast<GuiControlProfile*>(Sim::findObject("GuiInspectorRolloutProfile0"));

               char buf[256];
               dSprintf(buf, 256, "  [%i]", i);

               elementRollout->setControlProfile(elementRolloutProfile);
               elementRollout->setCaption(buf);
               //elementRollout->setMargin( 14, 0, 0, 0 );
               elementRollout->registerObject();

               GuiStackControl *elementStack = new GuiStackControl();
               elementStack->registerObject();
               elementRollout->addObject(elementStack);
               elementRollout->instantCollapse();

               arrayStack->addObject(elementRollout);
            }

            pArrayRollout = arrayRollout;
            pArrayStack = arrayStack;
            arrayStack->freeze(false);
            pArrayRollout->instantCollapse();
            mStack->addObject(arrayRollout);

            bMakingArray = true;
            continue;
         }
         else if (field->type == AbstractClassRep::EndArrayFieldType)
         {
#ifdef DEBUG_SPEW
            Platform::outputDebugString("[GuiInspectorGroup] Ending array '%s'",
               field->pFieldname);
#endif

            bMakingArray = false;
            continue;
         }

         if (bMakingArray)
         {
            // Add a GuiInspectorField for this field, 
            // for every element in the array...
            for (U32 i = 0; i < pArrayStack->size(); i++)
            {
               FrameTemp<char> intToStr(64);
               dSprintf(intToStr, 64, "%d", i);

               // The array stack should have a rollout for each element
               // as children...
               GuiRolloutCtrl *pRollout = dynamic_cast<GuiRolloutCtrl*>(pArrayStack->at(i));
               // And the each of those rollouts should have a stack for 
               // fields...
               GuiStackControl *pStack = dynamic_cast<GuiStackControl*>(pRollout->at(0));

               // And we add a new GuiInspectorField to each of those stacks...            
               GuiInspectorField *fieldGui = constructField(field->type);
               if (fieldGui == NULL)
                  fieldGui = new GuiInspectorField();

               fieldGui->init(mParent, this);
               StringTableEntry caption = field->pFieldname;
               fieldGui->setInspectorField(field, caption, intToStr);

               if (fieldGui->registerObject())
               {
#ifdef DEBUG_SPEW
                  Platform::outputDebugString("[GuiInspectorGroup] Adding array element '%s[%i]'",
                     field->pFieldname, i);
#endif

                  mChildren.push_back(fieldGui);
                  pStack->addObject(fieldGui);
               }
               else
                  delete fieldGui;
            }

            continue;
         }

         // This is weird, but it should work for now. - JDD
         // We are going to check to see if this item is an array
         // if so, we're going to construct a field for each array element
         if (field->elementCount > 1)
         {
            // Make a rollout control for this array
            //
            GuiRolloutCtrl *rollout = new GuiRolloutCtrl();
            rollout->setDataField(StringTable->insert("profile"), NULL, "GuiInspectorRolloutProfile0");
            rollout->setCaption(String::ToString("%s (%i)", field->pFieldname, field->elementCount));
            rollout->setMargin(14, 0, 0, 0);
            rollout->registerObject();
            mArrayCtrls.push_back(rollout);

            // Put a stack control within the rollout
            //
            GuiStackControl *stack = new GuiStackControl();
            stack->setDataField(StringTable->insert("profile"), NULL, "GuiInspectorStackProfile");
            stack->registerObject();
            stack->freeze(true);
            rollout->addObject(stack);

            mStack->addObject(rollout);

            // Create each field and add it to the stack.
            //
            for (S32 nI = 0; nI < field->elementCount; nI++)
            {
               FrameTemp<char> intToStr(64);
               dSprintf(intToStr, 64, "%d", nI);

               // Construct proper ValueName[nI] format which is "ValueName0" for index 0, etc.

               String fieldName = String::ToString("%s%d", field->pFieldname, nI);

               // If the field already exists, just update it
               GuiInspectorField *fieldGui = findField(fieldName);
               if (fieldGui != NULL)
               {
                  fieldGui->updateValue();
                  continue;
               }

               bNewItems = true;

               fieldGui = constructField(field->type);
               if (fieldGui == NULL)
                  fieldGui = new GuiInspectorField();

               fieldGui->init(mParent, this);
               StringTableEntry caption = StringTable->insert(String::ToString("   [%i]", nI));
               fieldGui->setInspectorField(field, caption, intToStr);

               fieldGui->setTargetObject(mTargetComponent);

               if (fieldGui->registerObject())
               {
                  mChildren.push_back(fieldGui);
                  stack->addObject(fieldGui);
               }
               else
                  delete fieldGui;
            }

            stack->freeze(false);
            stack->updatePanes();
            rollout->instantCollapse();
         }
         else
         {
            // If the field already exists, just update it
            GuiInspectorField *fieldGui = findField(field->pFieldname);
            if (fieldGui != NULL)
            {
               fieldGui->updateValue();
               continue;
            }

            bNewItems = true;

            fieldGui = constructField(field->type);
            if (fieldGui == NULL)
               fieldGui = new GuiInspectorField();

            fieldGui->init(mParent, this);
            fieldGui->setInspectorField(field);

            fieldGui->setTargetObject(mTargetComponent);

            if (fieldGui->registerObject())
            {
#ifdef DEBUG_SPEW
               Platform::outputDebugString("[GuiInspectorGroup] Adding field '%s'",
                  field->pFieldname);
#endif
               fieldGui->setValue(mTargetComponent->getDataField(field->pFieldname, NULL));

               mChildren.push_back(fieldGui);
               mStack->addObject(fieldGui);
            }
            else
            {
               SAFE_DELETE(fieldGui);
            }
         }
      }
   }

   for (U32 i = 0; i < mTargetComponent->getComponentFieldCount(); i++)
   {
      ComponentField* field = mTargetComponent->getComponentField(i);

      //first and foremost, nab the field type and check if it's a custom field or not.
      //If it's not a custom field, proceed below, if it is, hand it off to script to be handled by the component
      if (field->mFieldType == -1)
      {
         Con::executef(this, "onConstructComponentField", mTargetComponent, field->mFieldName);
         continue;
      }

      bNewItems = true;

      GuiInspectorField *fieldGui = constructField(field->mFieldType);
      if (fieldGui == NULL)
         fieldGui = new GuiInspectorField();

      fieldGui->init(mParent, this);

      fieldGui->setTargetObject(mTargetComponent);

      AbstractClassRep::Field *refField = NULL;

      //check dynamics
      SimFieldDictionary* fieldDictionary = mTargetComponent->getFieldDictionary();
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

void GuiInspectorComponentGroup::onRightMouseUp(const GuiEvent &event)
{
   //mParent->mOverDivider = false;
   if (isMethod("onRightMouseUp"))
      Con::executef(this, "onRightMouseUp", event.mousePoint);
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

DefineConsoleMethod(GuiInspectorComponentGroup, getComponent, S32, (), ,"")
{
   return object->getComponent()->getId();
}
