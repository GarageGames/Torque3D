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
   return NULL;
}

bool GuiInspectorVariableGroup::inspectGroup()
{
   // to prevent crazy resizing, we'll just freeze our stack for a sec..
   mStack->freeze(true);

   clearFields();

   Vector<String> names;

   gEvalState.globalVars.exportVariables( mSearchString, &names, NULL );

   bool bNewItems = false;

   for ( U32 i = 0; i < names.size(); i++ )
   {      
      const String &varName = names[i];

      // If the field already exists, just update it
      GuiInspectorVariableField *field = dynamic_cast<GuiInspectorVariableField*>( findField( varName ) );
      if ( field != NULL )
      {
         field->updateValue();
         continue;
      }

      bNewItems = true;

      field = new GuiInspectorVariableField();
      field->init( mParent, this );            
      field->setInspectorField( NULL, StringTable->insert( varName ) );

      if ( field->registerObject() )
      {
         mChildren.push_back( field );
         mStack->addObject( field );
      }
      else
         delete field;         
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
