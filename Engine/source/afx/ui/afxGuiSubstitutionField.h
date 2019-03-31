
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

#ifndef _AFX_GUI_SUBSTITUTION_FIELD_H_
#define _AFX_GUI_SUBSTITUTION_FIELD_H_

#include "console/simFieldDictionary.h"
#include "gui/editor/inspector/field.h"

class afxGuiSubstitutionField : public GuiInspectorField
{
   typedef GuiInspectorField Parent;   

public:

   afxGuiSubstitutionField(GuiInspector *inspector, GuiInspectorGroup* parent, SimFieldDictionary::Entry* field);
   afxGuiSubstitutionField();
   ~afxGuiSubstitutionField() {};
   
   DECLARE_CONOBJECT(afxGuiSubstitutionField);
   DECLARE_CATEGORY("AFX");

   virtual void             setData( const char* data, bool callbacks = true );
   virtual const char*      getData( U32 inspectObjectIndex = 0 );
   virtual void             updateValue();

   virtual void setToolTip( StringTableEntry data );

   virtual bool onAdd();

   virtual GuiControl* constructEditControl();

   virtual void setValue( const char* newValue );

protected:

   virtual void _executeSelectedCallback();

protected:

   String subs_string;
};

#endif // _AFX_SUBSTITUTION_FIELD_H_
