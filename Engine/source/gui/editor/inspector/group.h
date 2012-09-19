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

#ifndef _GUI_INSPECTOR_GROUP_H_
#define _GUI_INSPECTOR_GROUP_H_

#include "gui/core/guiCanvas.h"
#include "gui/controls/guiTextEditCtrl.h"
#include "gui/buttons/guiBitmapButtonCtrl.h"
#include "gui/containers/guiRolloutCtrl.h"

// Forward refs
class GuiInspector;
class GuiInspectorField;


/// The GuiInspectorGroup control is a helper control that the inspector
/// makes use of which houses a collapsible pane type control for separating
/// inspected objects fields into groups.  The content of the inspector is 
/// made up of zero or more GuiInspectorGroup controls inside of a GuiStackControl
///
class GuiInspectorGroup : public GuiRolloutCtrl
{
private:
   typedef GuiRolloutCtrl Parent;
public:
   // Members
   SimObjectPtr<GuiInspector>          mParent;
   Vector<GuiInspectorField*>          mChildren;
   GuiStackControl*                    mStack;
   Vector<GuiRolloutCtrl*>             mArrayCtrls;

   // Constructor/Destructor/Conobject Declaration
   GuiInspectorGroup();
   GuiInspectorGroup( const String& groupName, SimObjectPtr<GuiInspector> parent );
   virtual ~GuiInspectorGroup();
   
   DECLARE_CONOBJECT(GuiInspectorGroup);
   DECLARE_CATEGORY( "Gui Editor" );

   virtual GuiInspectorField* constructField( S32 fieldType );
   virtual GuiInspectorField* findField( const char *fieldName );

   // Publicly Accessible Information about this group
   const String& getGroupName() const { return mCaption; };
   SimObjectPtr<GuiInspector> getInspector() { return mParent; };

   bool onAdd();
   virtual bool inspectGroup();

   virtual void animateToContents();

   void clearFields();

   virtual bool updateFieldValue( StringTableEntry fieldName, const char *arrayIdx );
   virtual void updateAllFields();
   
   U32 getNumFields() const { return mChildren.size(); }

protected:
   // overridable method that creates our inner controls.
   virtual bool createContent();
   
   /// Determine the class that is a common ancestor to all inspected objects.
   AbstractClassRep* findCommonAncestorClass();
};

#endif // _GUI_INSPECTOR_GROUP_H_
