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

//
// This will change to something more abstract in the near future, or will be moved into
// the level editor if there is no concise way to abstract it.
//
// This function will build out an empty frame and add it to its given parent.  The newly
// built frame control is returned
function GuiFormClass::BuildEmptyFrame(%pos, %ext, %columns, %rows, %parentID)
{
   %frame = new GuiFrameSetCtrl() 
   {
      profile = "ToolsGuiFrameSetProfile";
      horizSizing = "width";
      vertSizing = "height";
      position = %pos;
      extent = %ext;
      columns = %columns;
      rows = %rows;
      borderWidth = "5"; //"4";
      //borderColor = "192 192 192";
      absoluteFrames = "1";
      relativeResizing = "1";
      specialHighlighting = "1";
   };
   
   %parentID.add(%frame);

   return %frame;
}

//
// This will change to something more abstract in the near future, or will be moved into
// the level editor if there is no concise way to abstract it.
//
// This function will build out a form control and add it to its parent.  The newly built
// form control is returned.
function GuiFormClass::BuildFormControl( %parentID, %ContentLibrary )
{
   // Find Default 'None' Content.
   %contentNoneObj = GuiFormManager::FindFormContent( %ContentLibrary, "Scene View" );
   if( %contentNoneObj == 0 )
   {
      error("GuiFormClass::BuildFormControl - Cannot find 'Scene View' Content Object!" );
      return false;
   }

   %newFormObj = new GuiFormCtrl() 
   {
      class = "FormControlClass";
      profile = "ToolsGuiFormProfile";
      canSaveDynamicFields = 1;
      horizSizing = "width";
      vertSizing = "height";
      position = "0 0";
      extent = "640 455";
      minExtent = "20 20";
      visible = "1";
      caption = $FormClassNoContentCaption;
      collapsable = "1";
      barBehindText = "1";
      hasMenu = true;
      ContentLibrary = %ContentLibrary;
      ContentID = %contentNoneObj;
      Content   = "Scene View";
   };
   %parentID.add( %newFormObj );

   return %newFormObj;
}


//
// This will change to something more abstract in the near future, or will be moved into
// the level editor if there is no concise way to abstract it.
//
function FormControlClass::onWake( %this )
{
   if( %this.ContentLibrary !$= "" && %this.Content !$= "" )
   {
      %contentObj = GuiFormManager::FindFormContent( %this.ContentLibrary, %this.Content );
      if( %contentObj == 0 )
      {
         error("GuiFormClass::onWake - Content Library Specified But Content Not Found!" );
         return;
      }

      // Set Form Content
      //if( %this.ContentID != %contentObj || !isObject( %this.ContentID ) )
         GuiFormClass::SetFormContent( %this, %contentObj );
   }

   %parentId = %this.getParent();
   %extent = %parentID.getExtent();
   %this.setExtent( GetWord(%extent, 0), GetWord(%extent, 1) );

   GuiFormClass::BuildFormMenu( %this );

}

//
// This will change to something more abstract in the near future, or will be moved into
// the level editor if there is no concise way to abstract it.
//
function GuiFormClass::BuildFormMenu( %formObj )
{

   if( !%formObj.hasMenu )
      return;

   // Menu Name.
   %menuName = "FormMainMenu";

   // Retrieve Menu ID.
   %formMenu = %formObj.getMenuID();

   %formMenu.clearMenuItems( %menuName );

   //*** Setup the check mark bitmap index to start at the third bitmap
   %formMenu.setCheckmarkBitmapIndex(1);
   
   //*** Add a menu to the menubar
   %formMenu.addMenu( %menuName, %formObj);
   %formMenu.setMenuBitmapIndex( %menuName, 0, true, false);
   %formMenu.setMenuMargins(0, 0, 0);
   
   // Build Division Control Menu Items.
   %formMenu.addMenuItem( %menuName, "Split This View Horizontally", 1);
   %formMenu.addMenuItem( %menuName, "Split This View Vertically", 2);
   %formMenu.addMenuItem( %menuName, "-", 0);
   %formMenu.addMenuItem( %menuName, "Remove This View", 3);
   
}

//
// This will change to something more abstract in the near future, or will be moved into
// the level editor if there is no concise way to abstract it.
//
function GuiFormClass::SetFormContent( %formObj, %contentObj )
{

   // Menu Name.
   %menuName = "FormMainMenu";

   // Validate New Content.
   if( !isObject( %contentObj ) )
   {
      // Failure
      error("GuiFormClass::SetFormContent- No Valid Content Object!" );
      return 0;
   }

   // Remove any other content from the Form
   %count = %formObj.getCount();
   if(%count > 1)
   {
      // Notify Script of Content Changing.
      if( %formObj.isMethod("onContentChanging") )
         %formObj.onContentChanging( %contentObj );

      // Object 0 is Always The Menu.
      %currentContent = %formObj.getObject( 1 );
      if( isObject( %currentContent ) )
      {
         // Remove from Reference List.
         if( %formObj.contentID !$= "" )
            GuiFormManager::RemoveContentReference( %formObj.ContentLibrary, %formObj.contentID.Name, %currentContent );

         // Delete the content.
         %currentContent.delete();

         // Update form Caption
         %formObj.setCaption( $FormClassNoContentCaption );
        
      }
   }
   
   // Handle the Form content choices by obtaining the script build command
   if( %contentObj.CreateFunction !$= "" )
   {
      //*** Set the name first
      %name = %contentObj.Name;
      %formObj.setCaption(%name);

      // We set the content ID prior to calling the create function so 
      // that it may reference it's content to retrieve information about it.
      %oldContentId = %formObj.contentID;
      %formObj.contentID = %contentObj;
      
      %result = eval( %contentObj.CreateFunction @ "(" @ %formObj @ ");" );
      if(!%result)
      {
         //*** Couldn't set up the form's contents so set the name back.  We need to
         //*** do it like this to allow the form's contents to change the form caption
         //*** if the above command worked.
         %formObj.setCaption($FormClassNoContentCaption);
         
         // Restore Content ID.
         %formObj.contentID = %oldContentID;

         // Notify Script of Failure.
         if( %formObj.isMethod("onContentChangeFailure") )
            %formObj.onContentChangeFailure();
      } 
      else
      {        
         // Add to Reference List.
         %currentContent = %formObj.getObject( 1 );
         if( isObject( %currentContent ) )
            GuiFormManager::AddContentReference( %formObj.ContentLibrary, %contentObj.Name, %currentContent );

         %formObj.Content = %formObj.contentId.name;

         // Notify Script of Content Change
         if( %formObj.isMethod("onContentChanged") )
            %formObj.onContentChanged( %contentObj );
            
         return %result;
      }
   }   
   return 0;
}


//
// Create a given content library content instance on a given parent control and 
//  reference count it in the ref manager.
//
function GuiFormClass::SetControlContent( %controlObj, %contentLibrary, %contentObj )
{
   // Validate New Content.
   if( !isObject( %contentObj ) )
   {
      // Failure
      error("GuiFormClass::SetControlContent- No Valid Content Object!" );
      return 0;
   }

   // Remove any other content from the Form
   if( isObject( %controlObj.ContentID ) )
   {
      // Find Control of current content internal name on the control.
      %currentContent = %controlObj.findObjectByInternalName( %controlObj.ContentID.Name );
      
      if( isObject( %currentContent ) )
      {

         // Notify Script of Content Changing.
         if( %controlObj.isMethod("onContentChanging") )
            %controlObj.onContentChanging( %contentObj );

         // Remove from Reference List.
         GuiFormManager::RemoveContentReference( %contentLibrary, %controlObj.contentID.Name, %currentContent );

         // Delete the content.
         %currentContent.delete();

      }
   }
   
   // Handle the Form content choices by obtaining the script build command
   if( %contentObj.CreateFunction !$= "" )
   {
      %name = %contentObj.Name;

      // We set the content ID prior to calling the create function so 
      // that it may reference it's content to retrieve information about it.
      %oldContentId = %controlObj.contentID;
      %controlObj.contentID = %contentObj;
      
      %currentContent = eval( %contentObj.CreateFunction @ "(" @ %controlObj @ ");" );
      if( !isObject( %currentContent ) )
      {        
         // Restore Content ID.
         %controlObj.contentID = %oldContentID;

         // Notify Script of Failure.
         if( %controlObj.isMethod("onContentChangeFailure") )
            %controlObj.onContentChangeFailure();
      } 
      else
      {
         // Add to Reference List.
         GuiFormManager::AddContentReference( %contentLibrary, %contentObj.Name, %currentContent );

         // Store Internal Name
         %currentContent.setInternalName( %contentObj.Name );

         // Store Content
         %controlObj.Content = %controlObj.contentId.name;

         // Notify Script of Content Change
         if( %controlObj.isMethod("onContentChanged") )
            %controlObj.onContentChanged( %contentObj );
            
         return %currentContent;
      }
   }   
   return 0;
}

//
// Remove a given library content instance from a control that is housing it.
//
function GuiFormClass::ClearControlContent( %controlObj, %contentLibrary )
{
 
   // Remove any other content from the Form
   if( isObject( %controlObj.ContentID ) )
   {
      // Find Control of current content internal name on the control.
      %currentContent = %controlObj.findObjectByInternalName( %controlObj.ContentID.Name );

      if( isObject( %currentContent ) )
      {

         // Notify Script of Content Changing.
         if( %controlObj.isMethod("onContentClearing") )
            %controlObj.onContentClearing( %controlObj.ContentID );

         // Remove from Reference List.
         GuiFormManager::RemoveContentReference( %contentLibrary, %controlObj.contentID.Name, %currentContent );

         // Delete the content.
         %currentContent.delete();
      }
   }
}


//
// This will change to something more abstract in the near future, or will be moved into
// the level editor if there is no concise way to abstract it.
//
// Turn a form into a split frame and add the form back and build a new blank
// form in the empty slot.  %horizontal==ture then make a horizontal split, otherwise
// make a vertical one.
function GuiFormClass::AddFrameSplitToForm(%formid, %horizontal)
{
   %formparent = %formid.getGroup();
   
   // Get form position and size
   %pos = %formid.position;
   %ext = %formid.extent;
   %rows = "0";
   %columns = "0";
   if(%horizontal)
   {
      %framesplit = getWord(%ext,1) / 2;
      %rows = "0 " @ %framesplit;
   } 
   else
   {
      %framesplit = getWord(%ext,0) / 2;
      %columns = "0 " @ %framesplit;
   }
         
   // If the form's parent is a frame control and this form is the first control then
   // we will need to move it to the front of the other children later on.  Otherwise
   // we'll be added to the bottom of the stack and the order will get messed up.
   // This all assumes that a frame control only has two children.
   %secondctrl = -1;
   if(%formparent.getClassName() $= "GuiFrameSetCtrl")
   {
      //error("Form parent is GuiFrameSetCtrl");
      if(%formparent.getObject(0) == %formid)
      {
         // This form is the first child.
         //error("Form is at the top");
         %secondctrl = %formparent.getObject(1);
      }
   }
                  
   // If we're adding a frameset around the layout base, propogate the 
   // layoutRef and layoutObj's to the new parent.
   if( %formID.LayoutRef !$= "" )
      %LayoutRef = %formID.LayoutRef;
   else
      %LayoutRef = 0;


   // Remove form from parent, put a frame control in its place, and then add this form back to the frame
   %formparent.remove(%formid);
   %frame = GuiFormClass::BuildEmptyFrame(%pos, %ext, %columns, %rows, %formparent);

   if( %layoutRef != 0 )
   {
      %frame.LayoutRef = %LayoutRef;
      %frame.LayoutRef.layoutObj = %frame;
      %frame.setCanSave( true );
   }
   if(%secondctrl != -1)
   {
      // Move this frame to the top of its parent's children stack by removing the
      // other child and adding it back again.  This will force this second child to
      // the bottom and our new frame back to the first child (the same location the
      // original form was).  Whew!  Maybe the frame control needs to be modified to
      // handle this.
      //error("Moving to the top");
      %formparent.remove(%secondctrl);
      %formparent.add(%secondctrl);
   }
   %frame.add(%formid);
         
   // Add a blank form to the bottom frame
   GuiFormClass::BuildFormControl(%frame, %formid.ContentLibrary );
         
   //error("New parent: " @ %frame SPC "(" @ %frame.getClassName() @ ")");
}

//
// This will change to something more abstract in the near future, or will be moved into
// the level editor if there is no concise way to abstract it.
//
//*** Remove a form's frame and any other of the frame's children and put the given
//*** form in its place, effectively removing the split.  %keepform==true then remove
//*** all children of a parent Frame Set and keep the given Form.  Otherwise, remove
//*** the given Form and keep the other child.
function GuiFormClass::RemoveFrameSplit(%formid, %keepform)
{
   //*** Get the form's parent and make sure it is a frame GUI.  Other wise do nothing.
   %frameID = %formid.getGroup();
   if(%frameID.getClassName() !$= "GuiFrameSetCtrl")
      return;
   
   //*** Get frame's position and size
   %pos = %frameID.position;
   %ext = %frameID.extent;
   
   if(%keepform)
   {
      %form = %frameID.getObject(0);

      // Remove from Reference List.
      if(%form.getClassName() $= "GuiFormCtrl")
         GuiFormManager::RemoveContentReference( %form.ContentLibrary, %form.contentID.Name, %form.getObject(1) );

      //*** Go through the frame's children and remove them (which includes our form)
      %frameID.clear();    
   } 
   else
   {

      // Remove from Reference List.
      if( %formId.getClassName() $= "GuiFormCtrl" )
         GuiFormManager::RemoveContentReference( %formId.ContentLibrary, %formId.contentID.Name, %formId.getObject(1) );

      //*** Store the first child that is not the given Form
      %count = %frameID.getCount();
      for(%i=0; %i < %count; %i++)
      {
         %child = %frameID.getObject(%i);
         if(%child != %formid)
         {
            //*** This is the first child that isn't the given Form, so
            //*** swap the given %formid with this new child so we keep it
            %formid = %child;
            break;
         }
      }

      //*** Now remove all children from the frame.
      %frameID.clear();
   }
   
   //*** Obtain the frame's parent
   %frameparentID = %frameID.getGroup();
   
   //*** If the frame's parent is itself a frame, then track all of its children and add
   //*** our form into the correct location, and remove our frame in the process.  Otherwise
   //*** just delete our frame and add our form to the parent.
   if(%frameparentID.getClassName() $= "GuiFrameSetCtrl")
   {
      //*** Store the children
      %count = %frameparentID.getCount();
      %check = -1;
      for(%i=0; %i<%count; %i++)
      {
         %obj[%i] = %frameparentID.getObject(%i);
         if(%obj[%i] == %frameID)
            %check = %i;
      }
      
      //*** Clear the existing children
      %frameparentID.clear();
      
      //*** Now add them back, including our form
      for(%i=0; %i<%count; %i++)
      {
         if(%i == %check)
         {
            //*** Add our form
            %frameparentID.add(%formid);
            
         } 
         else
         {
            //*** Add the old child back
            %frameparentID.add(%obj[%i]);
         }
      }
      
   } else
   {
      // If we're about to remove a frame that has a layout ref move it to the new object (%formID)
      if( %frameID.LayoutRef !$= "" )
      {
         %formID.LayoutRef = %frameID.LayoutRef;
         // By default if a control has been added to a form it will tag itself as cannot save.
         // just to be safe, we mark it as we can since it's clearly now becoming the root of a layout.
         %formID.LayoutRef.layoutObj = %formID;
         %formID.setCanSave( true );
      }
      //*** Remove old child and add our form to the parent
      %frameparentID.remove(%frameID);
      %frameparentID.add(%formid);
      
   }
   
   //*** Finally resize the form to fit
   %formid.resize(getWord(%pos,0),getWord(%pos,1),getWord(%ext,0),getWord(%ext,1));
}


//
// This will change to something more abstract in the near future, or will be moved into
// the level editor if there is no concise way to abstract it.
//
//*** Will resize the Form's content to fit.  This is usually done at the beginning when
//*** the content is first added to the form.
function FormControlClass::sizeContentsToFit(%this, %content, %margin)
{
   %formext = %this.getExtent();
   %menupos = %this.getObject(0).getPosition();
   %menuext = %this.getObject(0).getExtent();
   
   %ctrlposx = getWord(%menupos,0) + %this.contentID.Margin;
   %ctrlposy = getWord(%menupos,1) + getWord(%menuext,1) + %this.contentID.Margin;
   %ctrlextx = getWord(%formext,0) - %ctrlposx - %this.contentID.Margin;
   %ctrlexty = getWord(%formext,1) - %ctrlposy - %this.contentID.Margin;
   
   %content.resize(%ctrlposx,%ctrlposy,%ctrlextx,%ctrlexty);
}

//
// This will change to something more abstract in the near future, or will be moved into
// the level editor if there is no concise way to abstract it.
//
function FormControlClass::onResize(%this)
{
   //*** If this form has a content child, then pass along this resize notice
   //*** to allow it to do something.
   if(%this.getCount() > 1)
   {
      %child = %this.getObject(1);
      if( isObject( %child ) )
         %this.sizeContentsToFit( %child );

      if( %child.isMethod("onResize") )
         %child.onResize(%this);
   }
}

//
// This will change to something more abstract in the near future, or will be moved into
// the level editor if there is no concise way to abstract it.
//
function FormMenuBarClass::onMenuItemSelect(%this, %menuid, %menutext, %itemid, %itemtext)
{
   %formId = %menuid; // %menuid should contain the form's ID
   
   //error("FormMenuBarClass::onMenuSelect(): " @ %menuid SPC %menutext SPC %itemid SPC %itemtext SPC "parent: " @ %formparent);

   // If the ID is less than 1000, we know it's a layout menu item
   if( %itemid < 1000 )
   {
      // Handle the standard menu choices
      switch(%itemid)
      {
         case "1": // Add horizontal split
            GuiFormClass::AddFrameSplitToForm(%formid, true);
         
         case "2": // Add vertical split
            GuiFormClass::AddFrameSplitToForm(%formid, false);        
         case "3": // Remove split and keep other child
            GuiFormClass::RemoveFrameSplit(%formid, false);
      }

      // We're Done Here.
      return;
   } 
   else
      GuiFormClass::SetFormContent( %formId, %itemId );


}