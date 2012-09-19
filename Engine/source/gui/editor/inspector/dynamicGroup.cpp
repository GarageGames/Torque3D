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
#include "gui/editor/inspector/dynamicGroup.h"
#include "gui/editor/inspector/dynamicField.h"

IMPLEMENT_CONOBJECT(GuiInspectorDynamicGroup);

ConsoleDocClass( GuiInspectorDynamicGroup,
   "@brief Used to inspect an object's FieldDictionary (dynamic fields) instead "
   "of regular persistent fields.\n\n"
   "Editor use only.\n\n"
   "@internal"
);

//-----------------------------------------------------------------------------
// GuiInspectorDynamicGroup - add custom controls
//-----------------------------------------------------------------------------
bool GuiInspectorDynamicGroup::createContent()
{
   if(!Parent::createContent())
      return false;

   // encapsulate the button in a dummy control.
   GuiControl* shell = new GuiControl();
   shell->setDataField( StringTable->insert("profile"), NULL, "GuiTransparentProfile" );
   if( !shell->registerObject() )
   {
      delete shell;
      return false;
   }

   // add a button that lets us add new dynamic fields.
   GuiBitmapButtonCtrl* addFieldBtn = new GuiBitmapButtonCtrl();
   {
      SimObject* profilePtr = Sim::findObject("InspectorDynamicFieldButton");
      if( profilePtr != NULL )
         addFieldBtn->setControlProfile( dynamic_cast<GuiControlProfile*>(profilePtr) );
		
		// FIXME Hardcoded image
      addFieldBtn->setBitmap("tools/gui/images/iconAdd.png");

      char commandBuf[64];
      dSprintf(commandBuf, 64, "%d.addDynamicField();", this->getId());
      addFieldBtn->setField("command", commandBuf);
      addFieldBtn->setSizing(horizResizeLeft,vertResizeCenter);
      //addFieldBtn->setField("buttonMargin", "2 2");
      addFieldBtn->resize(Point2I(getWidth() - 20,2), Point2I(16, 16));
      addFieldBtn->registerObject("zAddButton");
   }

   shell->resize(Point2I(0,0), Point2I(getWidth(), 28));
   shell->addObject(addFieldBtn);

   // save off the shell control, so we can push it to the bottom of the stack in inspectGroup()
   mAddCtrl = shell;
   mStack->addObject(shell);

   return true;
}

struct FieldEntry
{
   SimFieldDictionary::Entry* mEntry;
   U32 mNumTargets;
};

static S32 QSORT_CALLBACK compareEntries(const void* a,const void* b)
{
   FieldEntry& fa = *((FieldEntry *)a);
   FieldEntry& fb = *((FieldEntry *)b);
   return dStrnatcmp(fa.mEntry->slotName, fb.mEntry->slotName);
}

//-----------------------------------------------------------------------------
// GuiInspectorDynamicGroup - inspectGroup override
//-----------------------------------------------------------------------------
bool GuiInspectorDynamicGroup::inspectGroup()
{
   // clear the first responder if it's set
   mStack->clearFirstResponder();

   // Clearing the fields and recreating them will more than likely be more
   // efficient than looking up existent fields, updating them, and then iterating
   // over existent fields and making sure they still exist, if not, deleting them.
   clearFields();

   // Create a vector of the fields
   Vector< FieldEntry > flist;
   
   const U32 numTargets = mParent->getNumInspectObjects();
   for( U32 i = 0; i < numTargets; ++ i )
   {
      SimObject* target = mParent->getInspectObject( i );
      
      // Then populate with fields
      SimFieldDictionary * fieldDictionary = target->getFieldDictionary();
      for(SimFieldDictionaryIterator ditr(fieldDictionary); *ditr; ++ditr)
      {
         if( i == 0 )
         {
            flist.increment();
            flist.last().mEntry = *ditr;
            flist.last().mNumTargets = 1;
         }
         else
         {
            const U32 numFields = flist.size();
            for( U32 n = 0; n < numFields; ++ n )
               if( flist[ n ].mEntry->slotName == ( *ditr )->slotName )
               {
                  flist[ n ].mNumTargets ++;
                  break;
               }
         }
      }
   }

   dQsort( flist.address(), flist.size(), sizeof( FieldEntry ), compareEntries );

   for(U32 i = 0; i < flist.size(); i++)
   {
      if( flist[ i ].mNumTargets != numTargets )
         continue;

      SimFieldDictionary::Entry* entry = flist[i].mEntry;

      // Create a dynamic field inspector.  Can't reuse typed GuiInspectorFields as
      // these rely on AbstractClassRep::Fields.
      GuiInspectorDynamicField *field = new GuiInspectorDynamicField( mParent, this, entry );

      // Register the inspector field and add it to our lists
      if( field->registerObject() )
      {
         mChildren.push_back( field );
         mStack->addObject( field );
      }
      else
         delete field;
   }

   mStack->pushObjectToBack(mAddCtrl);

   setUpdate();

   return true;
}

void GuiInspectorDynamicGroup::updateAllFields()
{
   // We overload this to just reinspect the group.
   inspectGroup();
}

ConsoleMethod(GuiInspectorDynamicGroup, inspectGroup, bool, 2, 2, "Refreshes the dynamic fields in the inspector.")
{
   return object->inspectGroup();
}

void GuiInspectorDynamicGroup::clearFields()
{
   // save mAddCtrl
   Sim::getGuiGroup()->addObject(mAddCtrl);
   // delete everything else
   mStack->clear();
   // clear the mChildren list.
   mChildren.clear();
   // and restore.
   mStack->addObject(mAddCtrl);
}

SimFieldDictionary::Entry* GuiInspectorDynamicGroup::findDynamicFieldInDictionary( StringTableEntry fieldName )
{
   SimFieldDictionary * fieldDictionary = mParent->getInspectObject()->getFieldDictionary();

   for(SimFieldDictionaryIterator ditr(fieldDictionary); *ditr; ++ditr)
   {
      SimFieldDictionary::Entry * entry = (*ditr);

      if( entry->slotName == fieldName )
         return entry;
   }

   return NULL;
}

void GuiInspectorDynamicGroup::addDynamicField()
{
   // We can't add a field without a target
   if( !mStack )
   {
      Con::warnf("GuiInspectorDynamicGroup::addDynamicField - no target SimObject to add a dynamic field to.");
      return;
   }

   // find a field name that is not in use. 
   // But we wont try more than 100 times to find an available field.
   U32 uid = 1;
   char buf[64] = "dynamicField";
   SimFieldDictionary::Entry* entry = findDynamicFieldInDictionary(buf);
   while(entry != NULL && uid < 100)
   {
      dSprintf(buf, sizeof(buf), "dynamicField%03d", uid++);
      entry = findDynamicFieldInDictionary(buf);
   }
   
   const U32 numTargets = mParent->getNumInspectObjects();
   if( numTargets > 1 )
      Con::executef( mParent, "onBeginCompoundEdit" );

   for( U32 i = 0; i < numTargets; ++ i )
   {
      SimObject* target = mParent->getInspectObject( i );
      
      Con::evaluatef( "%d.dynamicField = \"defaultValue\";", target->getId(), buf );
 
      // Notify script.
   
      Con::executef( mParent, "onFieldAdded", target->getIdString(), buf );
   }
   
   if( numTargets > 1 )
      Con::executef( mParent, "onEndCompoundEdit" );

   // now we simply re-inspect the object, to see the new field.
   inspectGroup();
   instantExpand();
}

ConsoleMethod( GuiInspectorDynamicGroup, addDynamicField, void, 2, 2, "obj.addDynamicField();" )
{
   object->addDynamicField();
}

ConsoleMethod( GuiInspectorDynamicGroup, removeDynamicField, void, 3, 3, "" )
{
}
