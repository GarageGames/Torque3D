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

#ifndef _GUI_INSPECTOR_VARIABLEFIELD_H_
#define _GUI_INSPECTOR_VARIABLEFIELD_H_

#ifndef _GUI_INSPECTOR_FIELD_H_
#include "gui/editor/inspector/field.h"
#endif

class GuiInspectorGroup;
class GuiInspector;


class GuiInspectorVariableField : public GuiInspectorField
{
   friend class GuiInspectorField;

public:

   typedef GuiInspectorField Parent;

   GuiInspectorVariableField();
   virtual ~GuiInspectorVariableField();

   DECLARE_CONOBJECT( GuiInspectorVariableField );
   DECLARE_CATEGORY( "Gui Editor" );

   virtual bool onAdd();


   virtual void setValue( const char* newValue );
   virtual const char* getValue() { return NULL; }
   virtual void updateValue();
   virtual void setData( const char* data, bool callbacks = true );
   virtual const char* getData( U32 inspectObjectIndex = 0 );
   virtual void updateData() {};

protected:
   StringTableEntry mVariableName;
   StringTableEntry mSetCallbackName;
   SimObject* mOwnerObject;
};

#endif // _GUI_INSPECTOR_VARIABLEFIELD_H_
