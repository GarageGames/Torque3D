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

function validateDatablockName(%name)
{
   // remove whitespaces at beginning and end
   %name = trim( %name );
   
   // remove numbers at the beginning
   %numbers = "0123456789";   
   while( strlen(%name) > 0 )
   {
      // the first character
      %firstChar = getSubStr( %name, 0, 1 );
      // if the character is a number remove it
      if( strpos( %numbers, %firstChar ) != -1 )
      {
         %name = getSubStr( %name, 1, strlen(%name) -1 );
         %name = ltrim( %name );
      }
      else
         break;
   }
   
   // replace whitespaces with underscores
   %name = strreplace( %name, " ", "_" );
   
   // remove any other invalid characters
   %invalidCharacters = "-+*/%$&§=()[].?\"#,;!~<>|°^{}";
   %name = stripChars( %name, %invalidCharacters );
   
   if( %name $= "" )
      %name = "Unnamed";
   
   return %name;
}

//--------------------------------------------------------------------------
// Finds location of %word in %text, starting at %start.  Works just like strPos
//--------------------------------------------------------------------------

function wordPos(%text, %word, %start)
{
   if (%start $= "") %start = 0;
   
   if (strpos(%text, %word, 0) == -1) return -1;
   %count = getWordCount(%text);
   if (%start >= %count) return -1;
   for (%i = %start; %i < %count; %i++)
   {
      if (getWord( %text, %i) $= %word) return %i;
   }
   return -1;
}

//--------------------------------------------------------------------------
// Finds location of %field in %text, starting at %start.  Works just like strPos
//--------------------------------------------------------------------------

function fieldPos(%text, %field, %start)
{
   if (%start $= "") %start = 0;
   
   if (strpos(%text, %field, 0) == -1) return -1;
   %count = getFieldCount(%text);
   if (%start >= %count) return -1;
   for (%i = %start; %i < %count; %i++)
   {
      if (getField( %text, %i) $= %field) return %i;
   }
   return -1;
}

//--------------------------------------------------------------------------
// returns the text in a file with "\n" at the end of each line
//--------------------------------------------------------------------------

function loadFileText( %file)
{
   %fo = new FileObject();
   %fo.openForRead(%file);
   %text = "";
   while(!%fo.isEOF())
   {
      %text = %text @ %fo.readLine();
      if (!%fo.isEOF()) %text = %text @ "\n";
   }

   %fo.delete();
   return %text;
}

function setValueSafe(%dest, %val)
{
   %cmd = %dest.command;
   %alt = %dest.altCommand;
   %dest.command = "";
   %dest.altCommand = "";

   %dest.setValue(%val);
   
   %dest.command = %cmd;
   %dest.altCommand = %alt;
}

function shareValueSafe(%source, %dest)
{
   setValueSafe(%dest, %source.getValue());
}

function shareValueSafeDelay(%source, %dest, %delayMs)
{
   schedule(%delayMs, 0, shareValueSafe, %source, %dest);
}


//------------------------------------------------------------------------------
// An Aggregate Control is a plain GuiControl that contains other controls, 
// which all share a single job or represent a single value.
//------------------------------------------------------------------------------

// AggregateControl.setValue( ) propagates the value to any control that has an 
// internal name.
function AggregateControl::setValue(%this, %val, %child)
{
   for(%i = 0; %i < %this.getCount(); %i++)
   {
      %obj = %this.getObject(%i);
      if( %obj == %child )
         continue;
         
      if(%obj.internalName !$= "")
         setValueSafe(%obj, %val);
   }
}

// AggregateControl.getValue() uses the value of the first control that has an
// internal name, if it has not cached a value via .setValue
function AggregateControl::getValue(%this)
{
   for(%i = 0; %i < %this.getCount(); %i++)
   {
      %obj = %this.getObject(%i);
      if(%obj.internalName !$= "")
      {
         //error("obj = " @ %obj.getId() @ ", " @ %obj.getName() @ ", " @ %obj.internalName );
         //error(" value = " @ %obj.getValue());
         return %obj.getValue();
      }
   }
}

// AggregateControl.updateFromChild( ) is called by child controls to propagate
// a new value, and to trigger the onAction() callback.
function AggregateControl::updateFromChild(%this, %child)
{
   %val = %child.getValue();
   if(%val == mCeil(%val)){
      %val = mCeil(%val);
   }else{
      if ( %val <= -100){
         %val = mCeil(%val);
      }else if ( %val <= -10){
         %val = mFloatLength(%val, 1);
      }else if ( %val < 0){
         %val = mFloatLength(%val, 2);
      }else if ( %val >= 1000){
         %val = mCeil(%val);
      }else if ( %val >= 100){
         %val = mFloatLength(%val, 1);
      }else if ( %val >= 10){
         %val = mFloatLength(%val, 2);
      }else if ( %val > 0){
         %val = mFloatLength(%val, 3);
      }
   }
   %this.setValue(%val, %child);
   %this.onAction();
}

// default onAction stub, here only to prevent console spam warnings.
function AggregateControl::onAction(%this) 
{
}

// call a method on all children that have an internalName and that implement the method.
function AggregateControl::callMethod(%this, %method, %args)
{
   for(%i = 0; %i < %this.getCount(); %i++)
   {
      %obj = %this.getObject(%i);
      if(%obj.internalName !$= "" && %obj.isMethod(%method))
         eval(%obj @ "." @ %method @ "( " @ %args @ " );");
   }

}

// A function used in order to easily parse the MissionGroup for classes . I'm pretty 
// sure at this point the function can be easily modified to search the any group as well.
function parseMissionGroup( %className, %childGroup )
{
   if( getWordCount( %childGroup ) == 0)
      %currentGroup = "MissionGroup";
   else
      %currentGroup = %childGroup;
      
   for(%i = 0; %i < (%currentGroup).getCount(); %i++)
   {      
      if( (%currentGroup).getObject(%i).getClassName() $= %className )
         return true;
      
      if( (%currentGroup).getObject(%i).getClassName() $= "SimGroup" )
      {
         if( parseMissionGroup( %className, (%currentGroup).getObject(%i).getId() ) )
            return true;         
      }
   } 
}

// A variation of the above used to grab ids from the mission group based on classnames
function parseMissionGroupForIds( %className, %childGroup )
{
   if( getWordCount( %childGroup ) == 0)
      %currentGroup = "MissionGroup";
   else
      %currentGroup = %childGroup;
      
   for(%i = 0; %i < (%currentGroup).getCount(); %i++)
   {      
      if( (%currentGroup).getObject(%i).getClassName() $= %className )
         %classIds = %classIds @ (%currentGroup).getObject(%i).getId() @ " ";
      
      if( (%currentGroup).getObject(%i).getClassName() $= "SimGroup" )
         %classIds = %classIds @ parseMissionGroupForIds( %className, (%currentGroup).getObject(%i).getId());
   } 
   return trim( %classIds );
}

//------------------------------------------------------------------------------
// Altered Version of TGB's QuickEditDropDownTextEditCtrl
//------------------------------------------------------------------------------

function QuickEditDropDownTextEditCtrl::onRenameItem( %this )
{
}

function QuickEditDropDownTextEditCtrl::updateFromChild( %this, %ctrl )
{
   if( %ctrl.internalName $= "PopUpMenu" )
   {
      %this->TextEdit.setText( %ctrl.getText() );
   }
   else if ( %ctrl.internalName $= "TextEdit" )
   {
      %popup = %this->PopupMenu;
      %popup.changeTextById( %popup.getSelected(), %ctrl.getText() );
      %this.onRenameItem();
   }
}
