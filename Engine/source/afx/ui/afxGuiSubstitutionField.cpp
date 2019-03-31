
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// Arcane-FX for MIT Licensed Open Source version of Torque 3D from GarageGames
// Copyright (C) 2015 Faust Logic, Inc.
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
//
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#include "afx/arcaneFX.h"

#include "gui/editor/inspector/customField.h"
#include "gui/editor/guiInspector.h"

#include "afx/ui/afxGuiSubstitutionField.h"

IMPLEMENT_CONOBJECT( afxGuiSubstitutionField );

ConsoleDocClass( afxGuiSubstitutionField,
   "@brief A customized variation of GuiInspectorField.\n\n"

   "A customized variation of GuiInspectorField.\n"

   "@ingroup AFX\n"
);

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

afxGuiSubstitutionField::afxGuiSubstitutionField( GuiInspector* inspector,
                                                  GuiInspectorGroup* parent, 
                                                  SimFieldDictionary::Entry* field)
{
   mInspector = inspector;
   mParent = parent;
   setBounds(0,0,100,20);   

   subs_string = StringTable->insert("");
}

afxGuiSubstitutionField::afxGuiSubstitutionField()
{
   mInspector = NULL;
   mParent = NULL;
   subs_string = StringTable->insert("");
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

void afxGuiSubstitutionField::setData( const char* data, bool callbacks )
{
   if ( mField == NULL)
      return;

   const U32 numTargets = mInspector->getNumInspectObjects();

   //if( callbacks && numTargets > 1 )
   //  Con::executef( mInspector, "beginCompoundUndo" );

   if (data[0] != '$' && data[1] != '$')
   {
      data = StringTable->insert(avar("$$ %s", data), true);
   }
   else
   {
      data = StringTable->insert(data, true);
   }

   for( U32 i = 0; i < numTargets; ++ i )
   {
      SimObject* target = mInspector->getInspectObject( i );
      SimDataBlock* datablock = dynamic_cast<SimDataBlock*>(target);
      if (datablock)
         datablock->addSubstitution(mField->pFieldname, 0, data);
   }

   //if( callbacks && numTargets > 1 )
   //  Con::executef( mInspector, "endCompoundUndo" );

   // Force our edit to update
   updateValue();
}

const char* afxGuiSubstitutionField::getData( U32 inspectObjectIndex )
{
   if( mField == NULL)
      return "";

   SimObject* target = mInspector->getInspectObject(inspectObjectIndex);
   SimDataBlock* datablock = dynamic_cast<SimDataBlock*>(target);
   if (datablock)
   {
      const char* current_subs = datablock->getSubstitution(mField->pFieldname, 0);
      if (current_subs)
         return StringTable->insert(avar("$$ %s", current_subs));
   }

   return StringTable->insert( "" );
}

/// Update this controls value to reflect that of the inspected field.
void afxGuiSubstitutionField::updateValue()
{
   if( mInspector->getNumInspectObjects() > 1 )
   {
      // ??
      return;
   }

   SimObject* target = mInspector->getInspectObject(0);
   SimDataBlock* datablock = dynamic_cast<SimDataBlock*>(target);
   if (datablock)
   {
      const char* current_subs = datablock->getSubstitution(mField->pFieldname, 0);
      if (current_subs)
      {
         setValue(StringTable->insert(avar("$$ %s", current_subs)));
         return;
      }
   }

   setValue(StringTable->insert("$$ -- undefined --"));
}

void afxGuiSubstitutionField::setToolTip( StringTableEntry data )
{
   mEdit->setDataField( StringTable->insert("tooltipprofile"), NULL, "GuiToolTipProfile" );
   mEdit->setDataField( StringTable->insert("hovertime"), NULL, "1000" );
   mEdit->setDataField( StringTable->insert("tooltip"), NULL, data );
}

bool afxGuiSubstitutionField::onAdd()
{
   if( !Parent::onAdd() )
      return false;

   return true;
}

GuiControl* afxGuiSubstitutionField::constructEditControl()
{
  if (mField->doNotSubstitute)
  {
    GuiTextEditCtrl* retCtrl = new GuiTextEditCtrl();

    retCtrl->setDataField( StringTable->insert("profile"), NULL, "GuiInspectorTextEditProfile" );

    // Register the object
    retCtrl->registerObject();

    retCtrl->setText( StringTable->insert("n/a") );
    retCtrl->setActive(false);

    return retCtrl;
  }

  GuiControl* retCtrl = new GuiTextEditCtrl();

  retCtrl->setDataField( StringTable->insert("profile"), NULL, "GuiInspectorTextEditProfile" );

  // Register the object
  retCtrl->registerObject();

  char szBuffer[512];
  dSprintf( szBuffer, 512, "%d.apply(%d.getText());",getId(), retCtrl->getId() );
  retCtrl->setField("AltCommand", szBuffer );
  retCtrl->setField("Validate", szBuffer );

  return retCtrl;
}

void afxGuiSubstitutionField::setValue( const char* newValue )
{
  if (!mField->doNotSubstitute)
  {
    GuiTextEditCtrl *ctrl = dynamic_cast<GuiTextEditCtrl*>( mEdit );
    if ( ctrl != NULL )
      ctrl->setText( newValue );
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// protected:

void afxGuiSubstitutionField::_executeSelectedCallback()
{
   Con::executef( mInspector, "onFieldSelected", mCaption, ConsoleBaseType::getType(mField->type)->getTypeName(), "Substitution Statement" );
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
