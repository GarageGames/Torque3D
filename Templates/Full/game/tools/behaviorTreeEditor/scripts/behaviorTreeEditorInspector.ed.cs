//-----------------------------------------------------------------------------
// Copyright (c) 2014 Guy Allard
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

//==============================================================================
// inspector field changed
//==============================================================================
function BehaviorTreeInspector::onInspectorFieldModified( %this, %object, %fieldName, %arrayIndex, %oldValue, %newValue )
{
   //echo("onInspectorFieldModified" SPC %object SPC %fieldName SPC %arrayIndex SPC %oldValue SPC %newValue);
   
   // select the correct page
   %treeRoot = BTEditor.getTreeRoot(%object);
   %targetView = -1;
   foreach(%page in BTEditorTabBook)
   {
      %view = %page-->BTView;
      if(%view.getRootNode() == %treeRoot)
         %targetView = %view;
   }
   
   if(!isObject(%targetView))
   {
      warn("OOPS - something went wrong with the inspector!");
      return;
   }
   
   %action = BTInspectorUndoAction::create(%object, %fieldName, %arrayIndex, %oldValue, %newValue);
   %action.addToManager( %targetView.getUndoManager() );
   
   BTEditor.updateUndoMenu();
}
