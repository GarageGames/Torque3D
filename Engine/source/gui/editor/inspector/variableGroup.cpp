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
#include "platform/platform.h"
#include "gui/editor/inspector/variableGroup.h"
#include "gui/editor/inspector/variableField.h"
#include "gui/editor/guiInspector.h"
#include "gui/buttons/guiIconButtonCtrl.h"
#include "console/consoleInternal.h"

extern ExprEvalState gEvalState;

//-----------------------------------------------------------------------------
// GuiInspectorVariableGroup
//-----------------------------------------------------------------------------
//
//
IMPLEMENT_CONOBJECT(GuiInspectorVariableGroup);

ConsoleDocClass( GuiInspectorVariableGroup,
   "@brief Inspector support for variable groups in a GuiVariableInspector.\n\n"
   "Editor use only.\n\n"
   "@internal"
);

GuiInspectorVariableGroup::GuiInspectorVariableGroup() 
{
}

GuiInspectorVariableGroup::~GuiInspectorVariableGroup()
{  
}

GuiInspectorField* GuiInspectorVariableGroup::constructField( S32 fieldType )
{
   return Parent::constructField(fieldType);
}

bool GuiInspectorVariableGroup::inspectGroup()
{
   // to prevent crazy resizing, we'll just freeze our stack for a sec..
   mStack->freeze(true);

   bool bNewItems = false;

   if (!mSearchString.equal(""))
   {
      Vector<String> names;

      gEvalState.globalVars.exportVariables(mSearchString, &names, NULL);

      for (U32 i = 0; i < names.size(); i++)
      {
         const String &varName = names[i];

         // If the field already exists, just update it
         GuiInspectorVariableField *field = dynamic_cast<GuiInspectorVariableField*>(findField(varName));
         if (field != NULL)
         {
            field->updateValue();
            continue;
         }

         bNewItems = true;

         field = new GuiInspectorVariableField();
         field->init(mParent, this);
         field->setInspectorField(NULL, StringTable->insert(varName));

         if (field->registerObject())
         {
            mChildren.push_back(field);
            mStack->addObject(field);
         }
         else
            delete field;
      }
   }

   for (U32 i = 0; i < mFields.size(); i++)
   {
      bNewItems = true;

      GuiInspectorField *fieldGui = findField(mFields[i]->mFieldName);
      if (fieldGui != NULL)
      {
         fieldGui->updateValue();
         continue;
      }

      //first and foremost, nab the field type and check if it's a custom field or not.
      //If it's not a custom field, proceed below, if it is, hand it off to script to be handled by the component
      if (mFields[i]->mFieldType == -1)
      {
         if (isMethod("onConstructField"))
         {
            //ensure our stack variable is bound if we need it
            Con::evaluatef("%d.stack = %d;", this->getId(), mStack->getId());

            Con::executef(this, "onConstructField", mFields[i]->mFieldName,
               mFields[i]->mFieldLabel, mFields[i]->mFieldTypeName, mFields[i]->mFieldDescription,
               mFields[i]->mDefaultValue, mFields[i]->mDataValues, mFields[i]->mOwnerObject);
         }
         continue;
      }

      bNewItems = true;

      fieldGui = constructField(mFields[i]->mFieldType);
      if (fieldGui == NULL)
         fieldGui = new GuiInspectorField();

      fieldGui->init(mParent, this);

      fieldGui->setSpecialEditField(true);

      if (mFields[i]->mOwnerObject)
      {
         fieldGui->setTargetObject(mFields[i]->mOwnerObject);
      }
      else
      {
         //check if we're binding to a global var first, if we have no owner
         if (mFields[i]->mFieldName[0] != '$')
         {
            fieldGui->setTargetObject(mParent);
         }
      }

      fieldGui->setSpecialEditVariableName(mFields[i]->mFieldName);
      fieldGui->setSpecialEditCallbackName(mFields[i]->mSetCallbackName);

      fieldGui->setInspectorField(NULL, mFields[i]->mFieldLabel);
      fieldGui->setDocs(mFields[i]->mFieldDescription);

      if(mFields[i]->mSetCallbackName != StringTable->EmptyString())
         fieldGui->setSpecialEditCallbackName(mFields[i]->mSetCallbackName);

      /*if (mFields[i]->mSetCallbackName != StringTable->EmptyString())
      {
         fieldGui->on.notify()
      }*/
         
      if (fieldGui->registerObject())
      {
#ifdef DEBUG_SPEW
         Platform::outputDebugString("[GuiInspectorVariableGroup] Adding field '%s'",
            field->pFieldname);
#endif

         if (mFields[i]->mOwnerObject)
         {
            String val = mFields[i]->mOwnerObject->getDataField(mFields[i]->mFieldName, NULL);

            if(val.isEmpty())
               fieldGui->setValue(mFields[i]->mDefaultValue);
            else
               fieldGui->setValue(val);
         }
         else
         {
            fieldGui->setValue(mFields[i]->mDefaultValue);
         }

         fieldGui->setActive(mFields[i]->mEnabled);

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
   if( bNewItems == false && !mChildren.empty() )
      return true;

   sizeToContents();

   setUpdate();

   return true;
}

void GuiInspectorVariableGroup::clearFields()
{
   mFields.clear();
}

void GuiInspectorVariableGroup::addField(VariableField* field)
{
   bool found = false;

   for (U32 i = 0; i < mFields.size(); i++)
   {
      if (mFields[i]->mFieldName == field->mFieldName)
      {
         found = true;
         break;
      }
   }

   if(!found)
      mFields.push_back(field);
}

void GuiInspectorVariableGroup::addInspectorField(GuiInspectorField* field)
{
   mStack->addObject(field);
   mChildren.push_back(field);
   mStack->updatePanes();
}

GuiInspectorField* GuiInspectorVariableGroup::createInspectorField()
{
   GuiInspectorField* newField = new GuiInspectorField();

   newField->init(mParent, this);

   newField->setSpecialEditField(true);

   if (newField->registerObject())
   {
      return newField;
   }

   return NULL;
}

DefineEngineMethod(GuiInspectorVariableGroup, createInspectorField, GuiInspectorField*, (),, "createInspectorField()")
{
   return object->createInspectorField();
}

DefineEngineMethod(GuiInspectorVariableGroup, addInspectorField, void, (GuiInspectorField* field), (nullAsType<GuiInspectorField*>()), "addInspectorField( GuiInspectorFieldObject )")
{
   object->addInspectorField(field);
}