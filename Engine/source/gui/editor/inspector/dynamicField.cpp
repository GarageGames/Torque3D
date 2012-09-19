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

#include "gui/editor/inspector/dynamicField.h"
#include "gui/editor/inspector/dynamicGroup.h"
#include "gui/editor/guiInspector.h"
#include "gui/buttons/guiIconButtonCtrl.h"

//-----------------------------------------------------------------------------
// GuiInspectorDynamicField - Child class of GuiInspectorField 
//-----------------------------------------------------------------------------
IMPLEMENT_CONOBJECT( GuiInspectorDynamicField );

ConsoleDocClass( GuiInspectorDynamicField,
   "@brief Custom field type for dynamic variable modification on SimObjects.\n\n"
   "Editor use only.\n\n"
   "@internal"
);

GuiInspectorDynamicField::GuiInspectorDynamicField( GuiInspector *inspector,
                                                    GuiInspectorGroup* parent, 
                                                    SimFieldDictionary::Entry* field )
 : mRenameCtrl( NULL ),
   mDeleteButton( NULL )
{
   mInspector = inspector;
   mParent = parent;
   mDynField = field;
   setBounds(0,0,100,20);   
}

void GuiInspectorDynamicField::setData( const char* data, bool callbacks )
{
   if ( mDynField == NULL )
      return;
   
   const U32 numTargets = mInspector->getNumInspectObjects();
   if( callbacks && numTargets > 1 )
      Con::executef( mInspector, "beginCompoundUndo" );
      
   // Setting an empty string will kill the field.
   const bool isRemoval = !data[ 0 ];
      
   for( U32 i = 0; i < numTargets; ++ i )
   {
      SimObject* target = mInspector->getInspectObject( i );

      // Callback on the inspector when the field is modified
      // to allow creation of undo/redo actions.
      const char *oldData = target->getDataField( mDynField->slotName, NULL );
      if ( !oldData )
         oldData = "";
      if ( dStrcmp( oldData, data ) != 0 )
      {
         target->inspectPreApply();
         
         if( callbacks )
         {
            if( isRemoval )
               Con::executef( mInspector, "onFieldRemoved", target->getIdString(), mDynField->slotName );
            else
               Con::executef( mInspector, "onInspectorFieldModified", target->getIdString(), mDynField->slotName, oldData, data );
         }

         target->setDataField( mDynField->slotName, NULL, data );

         // give the target a chance to validate
         target->inspectPostApply();
      }
   }
   
   if( callbacks && numTargets > 1 )
      Con::executef( mInspector, "endCompoundUndo" );

   // Force our edit to update
   updateValue();
}

const char* GuiInspectorDynamicField::getData( U32 inspectObjectIndex )
{
   if( mDynField == NULL )
      return "";

   return mInspector->getInspectObject( inspectObjectIndex )->getDataField( mDynField->slotName, NULL );
}

void GuiInspectorDynamicField::renameField( const char* newFieldName )
{
   newFieldName = StringTable->insert( newFieldName );
   
   if ( mDynField == NULL || mParent == NULL || mEdit == NULL )
   {
      Con::warnf("GuiInspectorDynamicField::renameField - No target object or dynamic field data found!" );
      return;
   }

   if ( !newFieldName )
   {
      Con::warnf("GuiInspectorDynamicField::renameField - Invalid field name specified!" );
      return;
   }

   // Only proceed if the name has changed
   if ( dStricmp( newFieldName, getFieldName() ) == 0 )
      return;
      
   // Grab a pointer to our parent and cast it to GuiInspectorDynamicGroup
   GuiInspectorDynamicGroup *group = dynamic_cast<GuiInspectorDynamicGroup*>(mParent);
   if ( group == NULL )
   {
      Con::warnf("GuiInspectorDynamicField::renameField - Unable to locate GuiInspectorDynamicGroup parent!" );
      return;
   }
   
   const U32 numTargets = mInspector->getNumInspectObjects();
   if( numTargets > 1 )
      Con::executef( mInspector, "onBeginCompoundEdit" );
      
   const char* oldFieldName = getFieldName();
   SimFieldDictionary::Entry* newEntry = NULL;
   
   for( U32 i = 0; i < numTargets; ++ i )
   {
      SimObject* target = mInspector->getInspectObject( i );
      
      // Make sure the new field is not already defined as a static field
      // on the object.
      
      if( target->isField( newFieldName ) )
      {
         // New field is already defined.  If we can, let the scripts handle
         // the error.  Otherwise, just emit an error on the console and proceed.
         
         if( numTargets == 1 && mInspector->isMethod( "onFieldRenameAlreadyDefined" ) )
            Con::executef( mInspector, "onFieldRenameAlreadyDefined", target->getIdString(), oldFieldName, newFieldName );
         else
            Con::errorf( "GuiInspectorDynamicField::renameField - field '%s' is already defined on %i:%s (%s)",
               newFieldName, target->getId(), target->getClassName(), target->getName() );
               
         // Reset the text entry.
               
         if( mRenameCtrl )
            mRenameCtrl->setText( oldFieldName );
            
         continue;
      }
      
      char currentValue[1024] = {0};
      // Grab our current dynamic field value (we use a temporary buffer as this gets corrupted upon Con::eval)
      dSprintf( currentValue, sizeof( currentValue ), "%s", target->getDataField( oldFieldName, NULL ) );

      // Unset the old field and set the new field.
      
      target->setDataField( oldFieldName, NULL, "" );
      target->setDataField( newFieldName, NULL, currentValue );

      // Notify script.
      
      Con::executef( mInspector, "onFieldRenamed", target->getIdString(), oldFieldName, newFieldName );
      
      // Look up the new SimFieldDictionary entry.
      
      if( !newEntry )
      {
         newEntry = target->getFieldDictionary()->findDynamicField( newFieldName );
         if( !newEntry )
         {
            Con::warnf( "GuiInspectorDynamicField::renameField - could not find new field '%s' on object %i:%s (%s)",
               newFieldName, target->getId(), target->getClassName(), target->getName() );
         }
         
         mDynField = newEntry;
      }
   }

   if( numTargets > 1 )
      Con::executef( mInspector, "onEndCompoundEdit" );
      
   // Lastly we need to reassign our validate field for our value edit control
   char szBuffer[1024];
   dSprintf( szBuffer, sizeof( szBuffer ), "%d.apply(%d.getText());", getId(), mEdit->getId() );
   mEdit->setField("validate", szBuffer );

   if( mDeleteButton )
   {
      dSprintf(szBuffer, sizeof( szBuffer ), "%d.apply("");%d.inspectGroup();", getId(), newFieldName, group->getId());
      mDeleteButton->setField("Command", szBuffer);
   }
}

bool GuiInspectorDynamicField::onAdd()
{
   if( !Parent::onAdd() )
      return false;

   //pushObjectToBack(mEdit);

   // Create our renaming field
   mRenameCtrl = new GuiTextEditCtrl();
   mRenameCtrl->setDataField( StringTable->insert("profile"), NULL, "GuiInspectorDynamicFieldProfile" );

   char szName[512];
   dSprintf( szName, 512, "IE_%s_%d_%s_Rename", mRenameCtrl->getClassName(), mInspector->getInspectObject()->getId(), getFieldName() );
   mRenameCtrl->registerObject( szName );

   // Our command will evaluate to :
   //
   //    if( (editCtrl).getText() !$= "" )
   //       (field).renameField((editCtrl).getText());
   //
   char szBuffer[1024];
   dSprintf( szBuffer, sizeof( szBuffer ), "if( %d.getText() !$= \"\" ) %d.renameField(%d.getText());", mRenameCtrl->getId(), getId(), mRenameCtrl->getId() );
   mRenameCtrl->setText( getFieldName() );
   mRenameCtrl->setField("Validate", szBuffer );
   addObject( mRenameCtrl );

   // Resize the name control to fit in our caption rect
   mRenameCtrl->resize( mCaptionRect.point, mCaptionRect.extent );

   // Resize the value control to leave space for the delete button
   mEdit->resize( mValueRect.point, mValueRect.extent);

   // Clear out any caption set from Parent::onAdd
   // since we are rendering the fieldname with our 'rename' control.
   mCaption = StringTable->insert( "" );

   // Create delete button control
   mDeleteButton = new GuiBitmapButtonCtrl();

	SimObject* profilePtr = Sim::findObject("InspectorDynamicFieldButton");
   if( profilePtr != NULL )
		mDeleteButton->setControlProfile( dynamic_cast<GuiControlProfile*>(profilePtr) );

   dSprintf( szBuffer, sizeof( szBuffer ),
      "%d.apply(\"\");%d.schedule(1,\"inspectGroup\");",
      getId(),
      mParent->getId() );

   // FIXME Hardcoded image
   mDeleteButton->setField( "Bitmap", "tools/gui/images/iconDelete" );
   mDeleteButton->setField( "Text", "X" );
   mDeleteButton->setField( "Command", szBuffer );
   mDeleteButton->setSizing( horizResizeLeft, vertResizeCenter );
	mDeleteButton->resize(Point2I(getWidth() - 20,2), Point2I(16, 16));
   mDeleteButton->registerObject();

   addObject(mDeleteButton);

   return true;
}

bool GuiInspectorDynamicField::updateRects()
{   
   Point2I fieldExtent = getExtent();   
   S32 dividerPos, dividerMargin;
   mInspector->getDivider( dividerPos, dividerMargin );

   S32 editWidth = dividerPos - dividerMargin;

   mEditCtrlRect.set( fieldExtent.x - dividerPos + dividerMargin, 1, editWidth, fieldExtent.y - 1 );
   mCaptionRect.set( 0, 0, fieldExtent.x - dividerPos - dividerMargin, fieldExtent.y );
   mValueRect.set( mEditCtrlRect.point, mEditCtrlRect.extent - Point2I( 20, 0 ) );
   mDeleteRect.set( fieldExtent.x - 20, 2, 16, fieldExtent.y - 4 );

   // This is probably being called during Parent::onAdd
   // so our special controls haven't been created yet but are just about to
   // so we just need to calculate the extents.
   if ( mRenameCtrl == NULL )
      return false;

   bool sized0 = mRenameCtrl->resize( mCaptionRect.point, mCaptionRect.extent );
   bool sized1 = mEdit->resize( mValueRect.point, mValueRect.extent );
   bool sized2 = mDeleteButton->resize(Point2I(getWidth() - 20,2), Point2I(16, 16));

   return ( sized0 || sized1 || sized2 );
}

void GuiInspectorDynamicField::setInspectorField( AbstractClassRep::Field *field, 
                                                  StringTableEntry caption, 
                                                  const char*arrayIndex ) 
{
   // Override the base just to be sure it doesn't get called.
   // We don't use an AbstractClassRep::Field...

//    mField = field; 
//    mCaption = StringTable->EmptyString();
//    mRenameCtrl->setText( getFieldName() );
}

void GuiInspectorDynamicField::_executeSelectedCallback()
{
   ConsoleBaseType* type = mDynField->type;
   if ( type )
      Con::executef( mInspector, "onFieldSelected", mDynField->slotName, type->getTypeName() );
   else
      Con::executef( mInspector, "onFieldSelected", mDynField->slotName, "TypeDynamicField" );
}

ConsoleMethod( GuiInspectorDynamicField, renameField, void, 3,3, "field.renameField(newDynamicFieldName);" )
{
   object->renameField( argv[ 2 ] );
}
