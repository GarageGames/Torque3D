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

//-----------------------------------------------------------------------------
// File Dialog Base - Add to Sim Callback
// Purpose : Intitialize Variables and Setup State.
//-----------------------------------------------------------------------------
function FileDialogBase::onAdd( %this )
{
   // Callback function Succeed
   %this.SuccessCallback = 0;
   // Callback function Cancel
   %this.CancelCallback = 0;
   
   // Multiple Select Flag
   %this.MultipleSelect = false;
   
   // File Extensions Group
   %this.FileExtensions = new SimGroup();
   
   %this.AddFilter("*.*","All Files");
}

//-----------------------------------------------------------------------------
// File Dialog Base - Remove from Sim Callback
// Purpose : Destroy Resources.
//-----------------------------------------------------------------------------
function FileDialogBase::onRemove( %this )
{
     
   // Remove FileExtensions Group
   if ( isObject( %this.FileExtensions ) )
      %this.FileExtensions.delete();
      
   // Remove Returned Files Group
   if( isObject( %this.ReturnFiles ) )
      %this.ReturnFiles.delete();
}

//-----------------------------------------------------------------------------
// File Dialog Base - Show on Screen Callback
// Purpose : Destroy Resources.
//-----------------------------------------------------------------------------
function FileDialogBase::onWake( %this )
{
   // Necessary
   %dirTree      =  %this.findObjectByInternalName("DirectoryTree", true);
   %fileList     =  %this.findObjectByInternalName("FileList", true);
   %filterList   =  %this.findObjectByInternalName("FilterList", true);   
   %cancelButton =  %this.findObjectByInternalName("CancelButton", true);
   %okButton     =  %this.findObjectByInternalName("OkButton", true);
   
   // Optional
   %fileName     =  %this.findObjectByInternalName("FileName", true);

   // Check for functionality Components.
   if( !isObject( %dirTree ) || !isObject( %fileList ) || !isObject( %filterList ) )
   {
      error("FileDialogBase::onWake - Unable to find NECESSARY child controls.");
      return false;
   }
   
   // Check for button components.
   if( !isObject( %cancelButton ) || !isObject( %okButton ) )
   {
      error("FileDialogBase::onWake - Unable to find accept and cancel buttons!");
      return false;
   }
   
   // Tag controls so that they can navigate our dialog. 
   %dirTree.parent       = %this;
   %fileList.parent      = %this;
   %filterList.parent    = %this;
   %okButton.parent      = %this;
   %cancelButton.parent  = %this;
   
   // Tag optionals
   if( isObject( %fileName ) )
      %fileName.parent   = %this;      
   
   // Finally, make sure our ReturnFiles group is empty.
   if( isObject( %this.ReturnFiles ) )
      %this.ReturnFiles.delete();
      
   %this.ReturnFiles = new SimGroup();
   %this.add( %this.ReturnFiles );
   
   // If no filters   
   if( %this.GetFilterCount() == 0 )
      %this.addfilter("*.*","All Files");
   
   %this.PopulateFilters();
     
}

//-----------------------------------------------------------------------------
// File Dialog Base - Add a file extension filter to the list
//-----------------------------------------------------------------------------
function FileDialogBase::AddFilter( %this, %extension, %caption )
{
   if( !isObject( %this.FileExtensions ) )
   {
      error("OpenFileDialog::AddFilter - FileExtensions Group does not exist!");
      return false;
   }
   
   %filter = new ScriptObject()
   {
      extension = %extension;
      caption   = %caption;
   };
   
   // Add to filter list
   %this.FileExtensions.add( %filter );
   
   return %filter;
}

//-----------------------------------------------------------------------------
// File Dialog Base - Clear filters by file extension
//-----------------------------------------------------------------------------
function FileDialogBase::ClearFilters( %this )
{
   if( isObject( %this.FileExtensions ) )
      %this.FileExtensions.delete();
      
   %this.FileExtensions = new SimGroup();
}


//-----------------------------------------------------------------------------
// File Dialog Base - Get number of filters
//-----------------------------------------------------------------------------
function FileDialogBase::GetFilterCount( %this )
{
   if( !isObject( %this.FileExtensions ) )
      return 0;
   
   // Return Count
   return %this.FileExtensions.getCount();
   
}

//-----------------------------------------------------------------------------
// File Dialog Base - Populate dropdown with filter options
//-----------------------------------------------------------------------------
function FileDialogBase::PopulateFilters( %this )
{
   %fileExtensions = %this.FileExtensions;
   if( !isObject( %fileExtensions ) )
   {
      error("OpenFileDialog::PopulateFilters - FileExtensions Group does not exist!");
      return false;
   }   
     
   %filterList   =  %this.findObjectByInternalName("FilterList", true);
   if( !isObject( %filterList ) )
   {
      error("FileDialogBase::PopulateFilters - Filter List Dropdown not found!");
      return false;
   }
   
   // Clear filter list
   %filterList.clear();
   
   // Populate List
   for( %i = 0; %i < %fileExtensions.getCount(); %i++ )
   {
      // Fetch Filter Script Object
      %filter = %fileExtensions.getObject( %i );
      
      // Add item to list
      %filterList.add( %filter.Caption SPC "(" SPC %filter.Extension SPC ")", %filter.getID() );
   }
   
   // Set First Item to Selected.
   %filterList.setFirstSelected();
   
   
}

function FileDialogOkButton::onClick( %this )
{
   if( !isObject( %this.parent ) )
   {
      error("FileDialogBase->FileDialogOkButton::onClick - Unable to find proper parent control!  Functionality Compromised!");
      return;
   }
   
   %dirTree      =  %this.parent.findObjectByInternalName("DirectoryTree", true);
   %fileList     =  %this.parent.findObjectByInternalName("FileList", true);
   %filterList   =  %this.parent.findObjectByInternalName("FilterList", true);
   
   // Check for functionality Components.
   if( !isObject( %dirTree ) || !isObject( %fileList ) || !isObject( %filterList ) )
   {
      error("FileDialogOkButton::onClick - Unable to find NECESSARY sibling controls.");
      return;
   }
   
   //
   // Fetch Path
   //
   %path = %dirTree.getSelectedPath();

   //
   // Compose File Name   
   //
   %fileNameCtrl =  %this.parent.findObjectByInternalName("FileName", true);
   
   // FileName TextEdit?
   if( isObject( %fileNameCtrl ) )
   {
      // Get FileName from TextEdit
      %fileName     = %fileNameCtrl.getText();

      // Get Filter Object from dropdown list
      %filterObj    = %filterList.getSelected();
      
      // Validate File Extension
      if( fileExt( %fileName ) $= "" && isObject( %filterObj ) )
      {
         // Append Extension to FileName
         %fileName = %fileName @ fileExt( %filterObj.Extension );
      }        
   }
   else
      %fileName = %fileList.getSelectedFile();
      
   //
   // Build Full Path
   //
   %fullPath = %path @ "/" @ %fileName;
   
   Canvas.popDialog( %this.parent );

   // Callback      
   eval( %this.parent.SuccessCallback @ "(\"" @ %fullPath @"\");" );
   
   %parent.SuccessCallback = 0;
   
   //error("Ok");
   
}

function FileDialogCancelButton::onClick( %this )
{
   Canvas.popDialog( %this.parent );
   //error("Cancel");
}


function FileDialogDirectoryTree::onSelectPath( %this, %path )
{
   %fileList     =  %this.parent.findObjectByInternalName("FileList", true);
   %filterList   =  %this.parent.findObjectByInternalName("FilterList", true);

   
   %filterObj    = %filterList.getSelected();
   if( !isObject( %filterObj ) )
      %filter = "*.*";
   else
      %filter = %filterObj.Extension;
      
   %fileList.setPath( %path, %filter );
}


function FileDialogFilterList::onSelect( %this, %id, %text )
{
   if( !isObject( %id ) )
   {
      error("FileDialogFilterList::onSelect - Invalid Filter Object!");
      return;
   }
   
   %fileList = %this.parent.findObjectByInternalName("FileList", true);
   
   %fileList.setFilter( %id.Extension );
   
}


function FileDialogFileList::onDoubleClick( %this )
{
   //error("DoubleClick");
   %okButton = %this.parent.findObjectByInternalName("OkButton", true);
   
   if( isObject( %okButton ) )
      %okButton.performClick();
}

function FileDialogFileList::onSelect( %this, %listid, %file )
{  
   %fileNameCtrl =  %this.parent.findObjectByInternalName("FileName", true);
   
   // FileName TextEdit?
   if( !isObject( %fileNameCtrl ) )
      return;

   // Update our file name to the one selected
   %fileNameCtrl.setText( %file ); 
}

function FileDialogFileName::onReturn( %this )
{
   //error("onReturn");
   %okButton = %this.parent.findObjectByInternalName("OkButton", true);
   
   if( isObject( %okButton ) )
      %okButton.performClick();  
}
