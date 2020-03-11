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



//---------------------------------------------------------------------------------------------

function GuiEditorSelectDlg::toggleVisibility( %this )
{
   if( %this.isVisible() )
      %this.setVisible( false );
   else
      %this.setVisible( true );
}

//---------------------------------------------------------------------------------------------

function GuiEditorSelectDlg::getRootGroup( %this )
{
   return GuiEditor.getContentControl();
}

//---------------------------------------------------------------------------------------------

function GuiEditorSelectDlg::includeClass( %this, %className )
{
   return (    isMemberOfClass( %className, "GuiControl" )
            && !GuiEditor.isFilteredClass( %className ) );
}

//---------------------------------------------------------------------------------------------

function GuiEditorSelectDlg::selectObject( %this, %object, %val )
{
   if( %val )
      GuiEditor.addSelection( %object );
   else
      GuiEditor.removeSelection( %object );
}

//---------------------------------------------------------------------------------------------

function GuiEditorSelectDlg::clearSelection( %this )
{
   GuiEditor.clearSelection();
}

//=============================================================================================
//    Events.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function GuiEditorSelectDlg::onVisible( %this, %visible )
{
   if( !%visible )
      return;
      
   if( !%this.isInitialized )
   {
      %this.init();
      %this.isInitialized = true;
   }
   
   // Re-initialize the group list on each wake.
   
   %this.initGroupList();
}
