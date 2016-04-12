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

function ETransformSelection::onWake( %this )
{
   // Make everything relative
   ETransformSelection-->PosRelative.setStateOn( true );
   ETransformSelection-->RotRelative.setStateOn( true );
   ETransformSelection-->ScaleRelative.setStateOn( true );
   ETransformSelection-->SizeRelative.setStateOn( false );
   
   ETransformSelection-->GetPosButton.setActive( false );
   ETransformSelection-->GetRotButton.setActive( false );
   ETransformSelection-->GetScaleButton.setActive( false );
   ETransformSelection-->GetSizeButton.setActive( false );
   
   // Size is always local
   ETransformSelection-->SizeLocal.setStateOn( true );
   ETransformSelection-->SizeLocal.setActive( false );
   
   ETransformSelection-->ScaleTabBook.selectPage( 0 );   // Scale page
   
   ETransformSelection-->ApplyButton.setActive( false );

   EWorldEditor.ETransformSelectionDisplayed = false;   
}

function ETransformSelection::onVisible( %this, %state )
{
   // If we are made visible, sync to the world editor
   // selection.
   
   if( %state )
      %this.onSelectionChanged();
}

function ETransformSelection::hideDialog( %this )
{
   %this.setVisible(false);
   EWorldEditor.ETransformSelectionDisplayed = false;
}

function ETransformSelection::ToggleVisibility( %this )
{
   if ( ETransformSelection.visible  )
   {
      ETransformSelection.setVisible(false);
      EWorldEditor.ETransformSelectionDisplayed = false;
   }
   else
   {
      ETransformSelection.setVisible(true);
      ETransformSelection.selectWindow();
      ETransformSelection.setCollapseGroup(false);
      EWorldEditor.ETransformSelectionDisplayed = true;
   }
}

function ETransformSelection::disableAllButtons( %this )
{
   ETransformSelection-->GetPosButton.setActive( false );
   ETransformSelection-->GetRotButton.setActive( false );
   ETransformSelection-->GetScaleButton.setActive( false );
   ETransformSelection-->GetSizeButton.setActive( false );
   
   ETransformSelection-->ApplyButton.setActive( false );
}

function ETransformSelection::onSelectionChanged( %this )
{
   // Count the number of selected SceneObjects.  There are
   // other object classes that could be selected, such
   // as SimGroups.
   %count = EWorldEditor.getSelectionSize();
   %sceneObjects = 0;
   %globalBoundsObjects = 0;
   for( %i=0; %i<%count; %i++)
   {
      %obj = EWorldEditor.getSelectedObject( %i );
      if( %obj.isMemberOfClass("SceneObject") )
      {
         %sceneObjects++;
         
         if( %obj.isGlobalBounds() )
         {
            %globalBoundsObjects++;
         }
      }
   }
   
   if( %sceneObjects == 0 )
   {
      // With nothing selected, disable all Get buttons
      ETransformSelection.disableAllButtons();
   }
   else if( %sceneObjects == 1 )
   {
      // With one selected, all Get buttons are active
      ETransformSelection-->GetPosButton.setActive( true );
      ETransformSelection-->GetRotButton.setActive( true );
      
      // Special case for Scale and Size for global bounds objects
      if( %globalBoundsObjects == 1 )
      {
         ETransformSelection-->GetSizeButton.setActive( false );
         ETransformSelection-->GetScaleButton.setActive( false );
      }
      else
      {
         ETransformSelection-->GetSizeButton.setActive( true );
         ETransformSelection-->GetScaleButton.setActive( true );
      }
      
      ETransformSelection-->ApplyButton.setActive( true );
   }
   else
   {
      // With more than one selected, only the position button
      // is active
      ETransformSelection-->GetPosButton.setActive( true );
      ETransformSelection-->GetRotButton.setActive( false );
      ETransformSelection-->GetScaleButton.setActive( false );
      ETransformSelection-->GetSizeButton.setActive( false );
      
      ETransformSelection-->ApplyButton.setActive( true );
      
      // If both RotRelative and RotLocal are unchecked, then go with RotLocal
      if( ETransformSelection-->RotRelative.getValue() == 0 && ETransformSelection-->RotLocal.getValue() == 0 )
      {
         ETransformSelection-->RotLocal.setStateOn( true );
      }
   }
}

function ETransformSelection::apply( %this )
{
   %position = ETransformSelection-->DoPosition.getValue();
   %p = ETransformSelection-->PosX.getValue() SPC ETransformSelection-->PosY.getValue() SPC ETransformSelection-->PosZ.getValue();
   %relativePos = ETransformSelection-->PosRelative.getValue();
   
   %rotate = ETransformSelection-->DoRotation.getValue();
   %r = mDegToRad(ETransformSelection-->Pitch.getValue()) SPC mDegToRad(ETransformSelection-->Bank.getValue()) SPC mDegToRad(ETransformSelection-->Heading.getValue());
   %relativeRot = ETransformSelection-->RotRelative.getValue();
   %rotLocal = ETransformSelection-->RotLocal.getValue();
   
   // We need to check which Tab page is active
   if( ETransformSelection-->ScaleTabBook.getSelectedPage() == 0 )
   {
      // Scale Page
      %scale = ETransformSelection-->DoScale.getValue();
      %s = ETransformSelection-->ScaleX.getValue() SPC ETransformSelection-->ScaleY.getValue() SPC ETransformSelection-->ScaleZ.getValue();
      %sRelative = ETransformSelection-->ScaleRelative.getValue();
      %sLocal = ETransformSelection-->ScaleLocal.getValue();
      
      %size = false;
   }
   else
   {
      // Size Page
      %size = ETransformSelection-->DoSize.getValue();
      %s = ETransformSelection-->SizeX.getValue() SPC ETransformSelection-->SizeY.getValue() SPC ETransformSelection-->SizeZ.getValue();
      %sRelative = ETransformSelection-->SizeRelative.getValue();
      %sLocal = ETransformSelection-->SizeLocal.getValue();
      
      %scale = false;
   }
   
   EWorldEditor.transformSelection(%position, %p, %relativePos, %rotate, %r, %relativeRot, %rotLocal, %scale ? 1 : (%size ? 2 : 0), %s, %sRelative, %sLocal);
}

function ETransformSelection::getAbsPosition( %this )
{
   %pos = EWorldEditor.getSelectionCentroid();
   ETransformSelection-->PosX.setText(getWord(%pos, 0));
   ETransformSelection-->PosY.setText(getWord(%pos, 1));
   ETransformSelection-->PosZ.setText(getWord(%pos, 2));
   
   // Turn off relative as we're populating absolute values
   ETransformSelection-->PosRelative.setValue(0);
   
   // Finally, set the Position check box as active.  The user
   // likely wants this if they're getting the position.
   ETransformSelection-->DoPosition.setValue(1);
}

function ETransformSelection::getAbsRotation( %this )
{
   %count = EWorldEditor.getSelectionSize();

   // If we have more than one SceneObject selected,
   // we must exit.
   %obj = -1;
   for( %i=0; %i<%count; %i++)
   {
      %test = EWorldEditor.getSelectedObject( %i );
      if( %test.isMemberOfClass("SceneObject") )
      {
         if( %obj != -1 )
            return;
         
         %obj = %test;
      }
   }
   
   if( %obj == -1 )
   {
      // No SceneObjects selected
      return;
   }
      
   %rot = %obj.getEulerRotation();
   ETransformSelection-->Pitch.setText(getWord(%rot, 0));
   ETransformSelection-->Bank.setText(getWord(%rot, 1));
   ETransformSelection-->Heading.setText(getWord(%rot, 2));
   
   // Turn off relative as we're populating absolute values.
   // Of course this means that we need to set local on.
   ETransformSelection-->RotRelative.setValue(0);
   ETransformSelection-->RotLocal.setValue(1);
   
   // Finally, set the Rotation check box as active.  The user
   // likely wants this if they're getting the position.
   ETransformSelection-->DoRotation.setValue(1);   
}

function ETransformSelection::getAbsScale( %this )
{
   %count = EWorldEditor.getSelectionSize();

   // If we have more than one SceneObject selected,
   // we must exit.
   %obj = -1;
   for( %i=0; %i<%count; %i++)
   {
      %test = EWorldEditor.getSelectedObject( %i );
      if( %test.isMemberOfClass("SceneObject") )
      {
         if( %obj != -1 )
            return;
         
         %obj = %test;
      }
   }
   
   if( %obj == -1 )
   {
      // No SceneObjects selected
      return;
   }
   
   %scale = %obj.scale;
   %scalex = getWord(%scale, 0);
   ETransformSelection-->ScaleX.setText(%scalex);
   if( ETransformSelectionScaleProportional.getValue() == false )
   {
      ETransformSelection-->ScaleY.setText(getWord(%scale, 1));
      ETransformSelection-->ScaleZ.setText(getWord(%scale, 2));
   }
   else
   {
      ETransformSelection-->ScaleY.setText(%scalex);
      ETransformSelection-->ScaleZ.setText(%scalex);
   }
   
   // Turn off relative as we're populating absolute values
   ETransformSelection-->ScaleRelative.setValue(0);
   
   // Finally, set the Scale check box as active.  The user
   // likely wants this if they're getting the position.
   ETransformSelection-->DoScale.setValue(1);   
}

function ETransformSelection::getAbsSize( %this )
{
   %count = EWorldEditor.getSelectionSize();

   // If we have more than one SceneObject selected,
   // we must exit.
   %obj = -1;
   for( %i=0; %i<%count; %i++)
   {
      %test = EWorldEditor.getSelectedObject( %i );
      if( %test.isMemberOfClass("SceneObject") )
      {
         if( %obj != -1 )
            return;
         
         %obj = %test;
      }
   }
   
   if( %obj == -1 )
   {
      // No SceneObjects selected
      return;
   }
      
   %size = %obj.getObjectBox();
   %scale = %obj.getScale();
   
   %sizex = (getWord(%size, 3) - getWord(%size, 0)) * getWord(%scale, 0);
   ETransformSelection-->SizeX.setText( %sizex );
   if( ETransformSelectionSizeProportional.getValue() == false )
   {
      ETransformSelection-->SizeY.setText( (getWord(%size, 4) - getWord(%size, 1)) * getWord(%scale, 1) );
      ETransformSelection-->SizeZ.setText( (getWord(%size, 5) - getWord(%size, 2)) * getWord(%scale, 2) );
   }
   else
   {
      ETransformSelection-->SizeY.setText( %sizex );
      ETransformSelection-->SizeZ.setText( %sizex );
   }
   
   // Turn off relative as we're populating absolute values
   ETransformSelection-->SizeRelative.setValue(0);
   
   // Finally, set the Size check box as active.  The user
   // likely wants this if they're getting the position.
   ETransformSelection-->DoSize.setValue(1);
}

function ETransformSelection::RotRelativeChanged( %this )
{
   if( ETransformSelection-->RotRelative.getValue() == 0 )
   {
      // With absolute rotation, it must happen locally
      ETransformSelection-->RotLocal.setStateOn( true );
   }
}

function ETransformSelection::RotLocalChanged( %this )
{
   if( ETransformSelection-->RotLocal.getValue() == 0 )
   {
      // Non-local rotation can only happen relatively
      ETransformSelection-->RotRelative.setStateOn( true );
   }
}

//-----------------------------------------------------------------------------

function ETransformSelectionScaleProportional::onClick( %this )
{
   if( %this.getValue() == 1 )
   {
      %scalex = ETransformSelection-->ScaleX.getValue();
      ETransformSelection-->ScaleY.setValue( %scalex );
      ETransformSelection-->ScaleZ.setValue( %scalex );
      
      ETransformSelection-->ScaleY.setActive( false );
      ETransformSelection-->ScaleZ.setActive( false );
   }
   else
   {
      ETransformSelection-->ScaleY.setActive( true );
      ETransformSelection-->ScaleZ.setActive( true );
   }
   
   Parent::onClick(%this);
}

function ETransformSelectionSizeProportional::onClick( %this )
{
   if( %this.getValue() == 1 )
   {
      %scalex = ETransformSelection-->SizeX.getValue();
      ETransformSelection-->SizeY.setValue( %scalex );
      ETransformSelection-->SizeZ.setValue( %scalex );
      
      ETransformSelection-->SizeY.setActive( false );
      ETransformSelection-->SizeZ.setActive( false );
   }
   else
   {
      ETransformSelection-->SizeY.setActive( true );
      ETransformSelection-->SizeZ.setActive( true );
   }
   
   Parent::onClick(%this);
}

//-----------------------------------------------------------------------------

function ETransformSelectionButtonClass::onClick( %this )
{
   %id = %this.getRoot().getFirstResponder();
   if( %id > -1 && ETransformSelection.controlIsChild(%id) )
   {
      (%id).clearFirstResponder(true);
   }
}

function ETransformSelectionCheckBoxClass::onClick( %this )
{
   %id = %this.getRoot().getFirstResponder();
   if( %id > -1 && ETransformSelection.controlIsChild(%id) )
   {
      (%id).clearFirstResponder(true);
   }
}

//-----------------------------------------------------------------------------

function ETransformSelectionTextEdit::onGainFirstResponder( %this )
{
   if( %this.isActive() )
   {
      %this.selectAllText();
   }
}

function ETransformSelectionTextEdit::onValidate( %this )
{
   if( %this.getInternalName() $= "ScaleX" && ETransformSelectionScaleProportional.getValue() == true )
   {
      // Set the Y and Z values to match
      %scalex = ETransformSelection-->ScaleX.getValue();
      ETransformSelection-->ScaleY.setValue( %scalex );
      ETransformSelection-->ScaleZ.setValue( %scalex );
   }
   
   if( %this.getInternalName() $= "SizeX" && ETransformSelectionSizeProportional.getValue() == true )
   {
      // Set the Y and Z values to match
      %sizex = ETransformSelection-->SizeX.getValue();
      ETransformSelection-->SizeY.setValue( %sizex );
      ETransformSelection-->SizeZ.setValue( %sizex );
   }
}
