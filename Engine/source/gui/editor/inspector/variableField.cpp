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
#include "gui/editor/inspector/variableField.h"

#include "gui/buttons/guiIconButtonCtrl.h"
#include "gui/editor/guiInspector.h"
#include "core/util/safeDelete.h"
#include "gfx/gfxDrawUtil.h"

//-----------------------------------------------------------------------------
// GuiInspectorVariableField
//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(GuiInspectorVariableField);

ConsoleDocClass( GuiInspectorVariableField,
   "@brief Inspector support for variables in a GuiVariableInspector.\n\n"
   "Editor use only.\n\n"
   "@internal"
);

GuiInspectorVariableField::GuiInspectorVariableField() 
{
}

GuiInspectorVariableField::~GuiInspectorVariableField()
{
}

bool GuiInspectorVariableField::onAdd()
{    
   if( !mInspector )
   {
      Con::errorf("GuiInspectorVariableField::onAdd - Fail - No inspector");
      return false;
   }

   setInspectorProfile();   

   // Hack: skip our immediate parent
   if ( !Parent::Parent::onAdd() )
      return false;  

   {
      GuiTextEditCtrl *edit = new GuiTextEditCtrl();

      edit->setDataField( StringTable->insert("profile"), NULL, "GuiInspectorTextEditProfile" );

      edit->registerObject();

      char szBuffer[512];
      dSprintf( szBuffer, 512, "%d.apply(%d.getText());",getId(), edit->getId() );
      edit->setField("AltCommand", szBuffer );
      edit->setField("Validate", szBuffer );

      mEdit = edit;
   }

   setBounds(0,0,100,18);

   // Add our edit as a child
   addObject( mEdit );

   // Calculate Caption and EditCtrl Rects
   updateRects();   

   // Force our editField to set it's value
   updateValue();

   return true;
}

void GuiInspectorVariableField::setData( const char* data, bool callbacks )
{   
   if ( !mCaption || mCaption[0] == 0 )
      return;

   Con::setVariable( mCaption, data );   

   // Force our edit to update
   updateValue();
}

const char* GuiInspectorVariableField::getData( U32 inspectObjectIndex )
{
   if ( !mCaption || mCaption[0] == 0 )
      return "";
      
   return Con::getVariable( mCaption );
}

void GuiInspectorVariableField::setValue( const char* newValue )
{
   GuiTextEditCtrl *ctrl = dynamic_cast<GuiTextEditCtrl*>( mEdit );
   if( ctrl != NULL )
      ctrl->setText( newValue );
}

void GuiInspectorVariableField::updateValue()
{
   if ( !mCaption || mCaption[0] == 0 ) 
      return;
   
   setValue( getData() );
}