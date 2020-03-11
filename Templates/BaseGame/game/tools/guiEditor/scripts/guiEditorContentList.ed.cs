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

// Code for the drop-down that allows selecting a GUI to edit in the Gui Editor.


if( !isDefined( "$GuiEditor::GuiFilterList" ) )
{
   /// List of named controls that are filtered out from the
   /// control list dropdown.
   $GuiEditor::GuiFilterList =
      "GuiEditorGui" TAB
      "AL_ShadowVizOverlayCtrl" TAB
      "MessageBoxOKDlg" TAB
      "MessageBoxOKCancelDlg" TAB
      "MessageBoxOKCancelDetailsDlg" TAB
      "MessageBoxYesNoDlg" TAB
      "MessageBoxYesNoCancelDlg" TAB
      "MessagePopupDlg";
}


//---------------------------------------------------------------------------------------------

function GuiEditorContentList::init( %this )
{
   %this.clear();
   %this.scanGroup( GuiGroup );
}

//---------------------------------------------------------------------------------------------

function GuiEditorContentList::scanGroup( %this, %group )
{
   foreach( %obj in %group )
   {
      if( %obj.isMemberOfClass( "GuiControl" ) )
      {
         if(%obj.getClassName() $= "GuiCanvas")
         {
            %this.scanGroup( %obj );
         }
         else 
         {
            if(%obj.getName() $= "")
               %name = "(unnamed) - " @ %obj;
            else
               %name = %obj.getName() @ " - " @ %obj;

            %skip = false;
            
            foreach$( %guiEntry in $GuiEditor::GuiFilterList )
               if( %obj.getName() $= %guiEntry )
               {
                  %skip = true;
                  break;
               }
      
            if( !%skip )
               %this.add( %name, %obj );
         }
      }
      else if( %obj.isMemberOfClass( "SimGroup" )
               &&  ( %obj.internalName !$= "EditorGuiGroup"    // Don't put our editor's GUIs in the list
                     || GuiEditor.showEditorGuis ) )   // except if explicitly requested.
      {
         // Scan nested SimGroups for GuiControls.
         
         %this.scanGroup( %obj );
      }
   }
}

//=============================================================================================
//    Event Handlers.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function GuiEditorContentList::onSelect( %this, %ctrl )
{
   GuiEditor.openForEditing( %ctrl );
}
