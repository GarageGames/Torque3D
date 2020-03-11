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

// Code for the status bar in the Gui Editor.


//---------------------------------------------------------------------------------------------

function GuiEditorStatusBar::getMouseModeHelp( %this )
{
   %isMac = ( $platform $= "macos" );
   if( %isMac )
      %cmdCtrl = "CMD";
   else
      %cmdCtrl = "CTRL";
   
   %mouseMode = GuiEditor.getMouseMode();
   switch$( %mouseMode )
   {
      case "Selecting":
         return "";
      
      case "DragSelecting":
         return %cmdCtrl @ " to add to selection; ALT to exclude parents; CTRL+ALT to exclude children";
      
      case "MovingSelection":
         return "";
      
      case "SizingSelection":
         return "CTRL to activate snapping; ALT to move instead of resize";
      
      case "DragGuide":
         return "Drag into ruler to delete; drop to place";
   }
   
   return "";
}

//---------------------------------------------------------------------------------------------

function GuiEditorStatusBar::print( %this, %message )
{
   %this.setText( %message );
   
   %sequenceNum = %this.sequenceNum + 1;
   %this.sequenceNum = %sequenceNum;
   
   %this.schedule( 4 * 1000, "clearMessage", %sequenceNum );
}

//---------------------------------------------------------------------------------------------

function GuiEditorStatusBar::clearMessage( %this, %sequenceNum )
{
   // If we had no newer message in the meantime, clear
   // out the current text.
   
   if( %this.sequenceNum == %sequenceNum )
      %this.setText( %this.getMouseModeHelp() );
}

//=============================================================================================
//    Event Handlers.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function GuiEditorStatusBar::onWake( %this )
{
   %this.setText( %this.getMouseModeHelp() );
}
