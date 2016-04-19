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

// Common functions for filter text and clear button controls on tree views.
// The GuiTextEditCtrl having the filter text must have "treeView" dynamic field
// that has the ID of the associated GuiTreeViewCtrl.
// The button ctrl used to clear the text field must have a "textCtrl" dynamic field
// that has the ID of the associated filter GuiTextEditCtrl.


//---------------------------------------------------------------------------------------------

function GuiTreeViewFilterText::onWake( %this )
{
   %filter = %this.treeView.getFilterText();
   if( %filter $= "" )
      %this.setText( "\c2Filter..." );
   else
      %this.setText( %filter );
}

//---------------------------------------------------------------------------------------------

function GuiTreeViewFilterText::onGainFirstResponder( %this )
{
   %this.selectAllText();
}

//---------------------------------------------------------------------------------------------

// When Enter is pressed in the filter text control, pass along the text of the control
// as the treeview's filter.
function GuiTreeViewFilterText::onReturn( %this )
{
   %text = %this.getText();
   if( %text $= "" )
      %this.reset();
   else
      %this.treeView.setFilterText( %text );
}

//---------------------------------------------------------------------------------------------

function GuiTreeViewFilterText::reset( %this )
{
   %this.setText( "\c2Filter..." );
   %this.treeView.clearFilterText();
}

//---------------------------------------------------------------------------------------------

function GuiTreeViewFilterClearButton::onClick( %this )
{
   %this.textCtrl.reset();
}
