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


$PE_EMITTEREDITOR_DEFAULT_FILENAME = "art/particles/managedParticleEmitterData.cs";


//=============================================================================================
//    PE_EmitterEditor.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function PE_EmitterEditor::guiSync( %this )
{
   %data = PE_EmitterEditor.currEmitter;
   
   // Sync up sliders and number boxes.
      
   if( PE_EmitterEditor-->PEE_infiniteLoop.isStateOn() )
   {
      PE_EmitterEditor-->PEE_lifetimeMS_slider.setActive( false );
      PE_EmitterEditor-->PEE_lifetimeMS_textEdit.setActive( false );
      PE_EmitterEditor-->PEE_lifetimeVarianceMS_slider.setActive( false );
      PE_EmitterEditor-->PEE_lifetimeVarianceMS_textEdit.setActive( false );
   }
   else
   {
      PE_EmitterEditor-->PEE_lifetimeMS_slider.setActive( true );
      PE_EmitterEditor-->PEE_lifetimeMS_textEdit.setActive( true );
      PE_EmitterEditor-->PEE_lifetimeVarianceMS_slider.setActive( true );
      PE_EmitterEditor-->PEE_lifetimeVarianceMS_textEdit.setActive( true );
      
      PE_EmitterEditor-->PEE_lifetimeMS_slider.setValue( %data.lifetimeMS );
      PE_EmitterEditor-->PEE_lifetimeMS_textEdit.setText( %data.lifetimeMS );
      PE_EmitterEditor-->PEE_lifetimeVarianceMS_slider.setValue( %data.lifetimeVarianceMS );
      PE_EmitterEditor-->PEE_lifetimeVarianceMS_textEdit.setText( %data.lifetimeVarianceMS );
   }
         
   PE_EmitterEditor-->PEE_ejectionPeriodMS_slider.setValue( %data.ejectionPeriodMS );
   PE_EmitterEditor-->PEE_ejectionPeriodMS_textEdit.setText( %data.ejectionPeriodMS );

   PE_EmitterEditor-->PEE_periodVarianceMS_slider.setValue( %data.periodVarianceMS );
   PE_EmitterEditor-->PEE_periodVarianceMS_textEdit.setText( %data.periodVarianceMS );
   
   PE_EmitterEditor-->PEE_ejectionVelocity_slider.setValue( %data.ejectionVelocity );
   PE_EmitterEditor-->PEE_ejectionVelocity_textEdit.setText( %data.ejectionVelocity );
   
   PE_EmitterEditor-->PEE_velocityVariance_slider.setValue( %data.velocityVariance );
   PE_EmitterEditor-->PEE_velocityVariance_textEdit.setText( %data.velocityVariance );
   
   PE_EmitterEditor-->PEE_orientParticles.setValue( %data.orientParticles );
   PE_EmitterEditor-->PEE_alignParticles.setValue( %data.alignParticles );
   PE_EmitterEditor-->PEE_alignDirection.setText( %data.alignDirection );
   
   PE_EmitterEditor-->PEE_thetaMin_slider.setValue( %data.thetaMin );
   PE_EmitterEditor-->PEE_thetaMin_textEdit.setText( %data.thetaMin );
   
   PE_EmitterEditor-->PEE_thetaMax_slider.setValue( %data.thetaMax );
   PE_EmitterEditor-->PEE_thetaMax_textEdit.setText( %data.thetaMax );
   
   PE_EmitterEditor-->PEE_phiVariance_slider.setValue( %data.phiVariance );
   PE_EmitterEditor-->PEE_phiVariance_textEdit.setText( %data.phiVariance );
   
   PE_EmitterEditor-->PEE_ejectionOffset_slider.setValue( %data.ejectionOffset );
   PE_EmitterEditor-->PEE_ejectionOffset_textEdit.setText( %data.ejectionOffset );
   
   PE_EmitterEditor-->PEE_ejectionOffsetVariance_slider.setValue( %data.ejectionOffsetVariance );
   PE_EmitterEditor-->PEE_ejectionOffsetVariance_textEdit.setText( %data.ejectionOffsetVariance );
   
   %blendTypeId = PE_EmitterEditor-->PEE_blendType.findText( %data.blendStyle );
   PE_EmitterEditor-->PEE_blendType.setSelected( %blendTypeId, false );
   
   PE_EmitterEditor-->PEE_softnessDistance_slider.setValue( %data.softnessDistance );
   PE_EmitterEditor-->PEE_softnessDistance_textEdit.setText( %data.softnessDistance );
   
   PE_EmitterEditor-->PEE_ambientFactor_slider.setValue( %data.ambientFactor );
   PE_EmitterEditor-->PEE_ambientFactor_textEdit.setText( %data.ambientFactor );
   
   PE_EmitterEditor-->PEE_softParticles.setValue( %data.softParticles );
   PE_EmitterEditor-->PEE_reverseOrder.setValue( %data.reverseOrder );
   PE_EmitterEditor-->PEE_useEmitterSizes.setValue( %data.useEmitterSizes );
   PE_EmitterEditor-->PEE_useEmitterColors.setValue( %data.useEmitterColors );
   
   PE_EmitterEditor-->PEE_glow.setValue( %data.glow );

   // Sync up particle selectors.
   
   for( %index = 0; %index < 4; %index ++ )
   {
      %ctrl = "PEE_EmitterParticle" @ ( %index + 1 );
      %popup = %ctrl-->PopUpMenu;

      %particle = getWord( %data.particles, %index );
      if( isObject( %particle ) )
         %popup.setSelected( %particle.getId(), false );
      else
         %popup.setSelected( 0, false ); // Select "None".
   }   
}

//---------------------------------------------------------------------------------------------

// Generic updateEmitter method
function PE_EmitterEditor::updateEmitter( %this, %propertyField, %value, %isSlider, %onMouseUp )
{
   PE_EmitterEditor.setEmitterDirty();
         
   %emitter = PE_EmitterEditor.currEmitter;
      
   %last = Editor.getUndoManager().getUndoAction(Editor.getUndoManager().getUndoCount() - 1);
   if( (%isSlider) && (%last.isSlider) && (!%last.onMouseUp) )
   {
      %last.field = %propertyField;
      %last.isSlider = %isSlider;
      %last.onMouseUp = %onMouseUp;
      %last.newValue = %value;
   }
   else
   {
      %action = ParticleEditor.createUndo(ActionUpdateActiveEmitter, "Update Active Emitter");
      %action.emitter = %emitter;
      %action.field = %propertyField;
      %action.isSlider = %isSlider;
      %action.onMouseUp = %onMouseUp;
      %action.newValue = %value;
      %action.oldValue = %emitter.getFieldValue( %propertyField );
      
      ParticleEditor.submitUndo( %action );
   }
   	
	%emitter.setFieldValue( %propertyField, %value );
	%emitter.reload();
}

//---------------------------------------------------------------------------------------------

// Special case updateEmitter methods
function PE_EmitterEditor::updateLifeFields( %this, %isRandom, %value, %isSlider, %onMouseUp )
{
   PE_EmitterEditor.setEmitterDirty();
   
   %emitter = PE_EmitterEditor.currEmitter;
      
   // Transfer values over to gui controls.
   
   if( %isRandom )
   {
      if( %value > 0 )
         %value++;
          
      if( %value > PE_EmitterEditor-->PEE_lifetimeMS_slider.getValue() )
      {
         PE_EmitterEditor-->PEE_lifetimeMS_textEdit.setText( %value );
         PE_EmitterEditor-->PEE_lifetimeMS_slider.setValue( %value );
      }
   }
   else
   {
      if( %value > 0 )
         %value --;
         
      if( %value < PE_EmitterEditor-->PEE_lifetimeVarianceMS_slider.getValue() )
      {
         PE_EmitterEditor-->PEE_lifetimeVarianceMS_textEdit.setText( %value );
         PE_EmitterEditor-->PEE_lifetimeVarianceMS_slider.setValue( %value );
      }
   }
   
   // Submit undo.
   
   %last = Editor.getUndoManager().getUndoAction(Editor.getUndoManager().getUndoCount() - 1);
   if( (%isSlider) && (%last.isSlider) && (!%last.onMouseUp) )
   {
      %last.isSlider = %isSlider;
      %last.onMouseUp = %onMouseUp;
      %last.newValueLifetimeMS = PE_EmitterEditor-->PEE_lifetimeMS_textEdit.getText();
      %last.newValueLifetimeVarianceMS = PE_EmitterEditor-->PEE_lifetimeVarianceMS_textEdit.getText();
   }
   else
   {
      %action = ParticleEditor.createUndo(ActionUpdateActiveEmitterLifeFields, "Update Active Emitter");
      %action.emitter = %emitter;
      %action.isSlider = %isSlider;
      %action.onMouseUp = %onMouseUp;
      
      %action.newValueLifetimeMS = PE_EmitterEditor-->PEE_lifetimeMS_textEdit.getText();
      %action.oldValueLifetimeMS = %emitter.lifetimeMS;
      
      %action.newValueLifetimeVarianceMS = PE_EmitterEditor-->PEE_lifetimeVarianceMS_textEdit.getText();
      %action.oldValueLifetimeVarianceMS = %emitter.lifetimeVarianceMS;
      
      ParticleEditor.submitUndo( %action );
   }
   
   // Set the values on the current emitter.
   
   %emitter.lifetimeMS = PE_EmitterEditor-->PEE_lifetimeMS_textEdit.getText();
   %emitter.lifetimeVarianceMS = PE_EmitterEditor-->PEE_lifetimeVarianceMS_textEdit.getText();
   %emitter.reload();
   
   // Keep the infiniteLoop checkbox up to date.
   
   PE_EmitterEditor-->PEE_infiniteLoop.setStateOn(
      %emitter.lifetimeMS == 0
   );
}

//---------------------------------------------------------------------------------------------

function PE_EmitterEditor::updateLifeFieldsInfiniteLoop( %this )
{
   %emitter = PE_EmitterEditor.currEmitter;
   %isEnabled = PE_EmitterEditor-->PEE_infiniteLoop.isStateOn();
      
   // Submit undo.
   
   %action = ParticleEditor.createUndo( ActionUpdateActiveEmitterLifeFields, "Update Active Emitter" );
   %action.emitter = %emitter;
   
   if( %isEnabled )
   {
      %action.newValueLifetimeMS = 0;
      %action.newvalueLifetimeVarianceMS = 0;
      %action.oldValueLifetimeMS = PE_EmitterEditor-->PEE_lifetimeMS_textEdit.getText();
      %action.oldValueLifetimeVarianceMS = PE_EmitterEditor-->PEE_lifetimeVarianceMS_textEdit.getText();
   }
   else
   {
      %action.newValueLifetimeMS = PE_EmitterEditor-->PEE_lifetimeMS_textEdit.getText();
      %action.newvalueLifetimeVarianceMS = PE_EmitterEditor-->PEE_lifetimeVarianceMS_textEdit.getText();
      %action.oldValueLifetimeMS = 0;
      %action.oldValueLifetimeVarianceMS = 0;
   }
            
   ParticleEditor.submitUndo( %action );
   
   // Execute action.
   
   %action.redo();
}

//---------------------------------------------------------------------------------------------

function PE_EmitterEditor::updateAmountFields( %this, %isRandom, %value, %isSlider, %onMouseUp )
{
   PE_EmitterEditor.setEmitterDirty();
   %emitter = PE_EmitterEditor.currEmitter;
      
   // Transfer values over to gui controls.
   
   if( %isRandom )
   {
      %value ++;
      if( %value > PE_EmitterEditor-->PEE_ejectionPeriodMS_slider.getValue() )
      {
         PE_EmitterEditor-->PEE_ejectionPeriodMS_textEdit.setText( %value );
         PE_EmitterEditor-->PEE_ejectionPeriodMS_slider.setValue( %value );
      }
   }
   else
   {
      %value --;
      if( %value < PE_EmitterEditor-->PEE_periodVarianceMS_slider.getValue() )
      {
         PE_EmitterEditor-->PEE_periodVarianceMS_textEdit.setText( %value );
         PE_EmitterEditor-->PEE_periodVarianceMS_slider.setValue( %value );
      }
   }
   
   // Submit undo.
   
   %last = Editor.getUndoManager().getUndoAction(Editor.getUndoManager().getUndoCount() - 1);
   if( (%isSlider) && (%last.isSlider) && (!%last.onMouseUp) )
   {
      %last.isSlider = %isSlider;
      %last.onMouseUp = %onMouseUp;
      %last.newValueEjectionPeriodMS = PE_EmitterEditor-->PEE_ejectionPeriodMS_textEdit.getText();
      %last.newValuePeriodVarianceMS = PE_EmitterEditor-->PEE_periodVarianceMS_textEdit.getText();
   }
   else
   {
      %action = ParticleEditor.createUndo(ActionUpdateActiveEmitterAmountFields, "Update Active Emitter");
      %action.emitter = %emitter;
      %action.isSlider = %isSlider;
      %action.onMouseUp = %onMouseUp;
      
      %action.newValueEjectionPeriodMS = PE_EmitterEditor-->PEE_ejectionPeriodMS_textEdit.getText();
      %action.oldValueEjectionPeriodMS = %emitter.ejectionPeriodMS;
      
      %action.newValuePeriodVarianceMS = PE_EmitterEditor-->PEE_periodVarianceMS_textEdit.getText();
      %action.oldValuePeriodVarianceMS = %emitter.periodVarianceMS;
      
      ParticleEditor.submitUndo( %action );
   }
   
   // Set the values on the current emitter.
   
   %emitter.ejectionPeriodMS = PE_EmitterEditor-->PEE_ejectionPeriodMS_textEdit.getText();
   %emitter.periodVarianceMS = PE_EmitterEditor-->PEE_periodVarianceMS_textEdit.getText();
   %emitter.reload();
}

//---------------------------------------------------------------------------------------------

function PE_EmitterEditor::updateSpeedFields( %this, %isRandom, %value, %isSlider, %onMouseUp )
{
   PE_EmitterEditor.setEmitterDirty();
   %emitter = PE_EmitterEditor.currEmitter;
      
   // Transfer values over to gui controls.
   
   if( %isRandom )
   {
      if( %value > PE_EmitterEditor-->PEE_ejectionVelocity_slider.getValue() )
      {
         PE_EmitterEditor-->PEE_ejectionVelocity_textEdit.setText( %value );
         PE_EmitterEditor-->PEE_ejectionVelocity_slider.setValue( %value );
      }
   }
   else
   {
      if( %value < PE_EmitterEditor-->PEE_velocityVariance_slider.getValue() )
      {
         PE_EmitterEditor-->PEE_velocityVariance_textEdit.setText( %value );
         PE_EmitterEditor-->PEE_velocityVariance_slider.setValue( %value );
      }
   }
   
   // Submit undo.
   
   %last = Editor.getUndoManager().getUndoAction(Editor.getUndoManager().getUndoCount() - 1);
   if( (%isSlider) && (%last.isSlider) && (!%last.onMouseUp) )
   {
      %last.isSlider = %isSlider;
      %last.onMouseUp = %onMouseUp;
      %last.newValueEjectionVelocity = PE_EmitterEditor-->PEE_ejectionVelocity_textEdit.getText();
      %last.newValueVelocityVariance = PE_EmitterEditor-->PEE_velocityVariance_textEdit.getText();
   }
   else
   {
      %action = ParticleEditor.createUndo(ActionUpdateActiveEmitterSpeedFields, "Update Active Emitter");
      %action.emitter = %emitter;
      %action.isSlider = %isSlider;
      %action.onMouseUp = %onMouseUp;
      
      %action.newValueEjectionVelocity = PE_EmitterEditor-->PEE_ejectionVelocity_textEdit.getText();
      %action.oldValueEjectionVelocity = %emitter.ejectionVelocity;
      
      %action.newValueVelocityVariance = PE_EmitterEditor-->PEE_velocityVariance_textEdit.getText();
      %action.oldValueVelocityVariance = %emitter.velocityVariance;
      
      ParticleEditor.submitUndo( %action );
   }
   
   // Set the values on the current emitter.
   
   %emitter.ejectionVelocity = PE_EmitterEditor-->PEE_ejectionVelocity_textEdit.getText();
   %emitter.velocityVariance = PE_EmitterEditor-->PEE_velocityVariance_textEdit.getText();
   %emitter.reload();
}

//---------------------------------------------------------------------------------------------

function PE_EmitterEditor::updateParticlesFields( %this )
{
   %particles = "";
   for( %i = 1; %i < 5; %i ++ )
   {
      %emitterParticle = "PEE_EmitterParticle" @ %i;
      %popup = %emitterParticle-->PopUpMenu;
      %text = %popup.getText();
      
      if( %text $= "" || %text $= "None" )
         continue;
         
      if( %particles $= "" )
         %particles = %text;
      else
         %particles = %particles SPC %text;
   }
   
   %changedEditParticle = 1;
   %currParticle = PE_ParticleEditor.currParticle.getName();

   foreach$( %particleName in %particles )
   {
      if( %particleName $= %currParticle )
      {
         %changedEditParticle = 0; 
         break;
      }
   }
   
   // True only if the currently edited particle has not been found and the 
   // ParticleEditor is dirty.
   
   if( %changedEditParticle && PE_ParticleEditor.dirty )
   {
      MessageBoxYesNoCancel("Save Particle Changes?", 
         "Do you wish to save the changes made to the <br>current particle before changing the particle?", 
         "PE_ParticleEditor.saveParticle( " @ PE_ParticleEditor.currParticle.getName() @ " ); PE_EmitterEditor.updateEmitter( \"particles\"," @ %particles @ ");", 
         "PE_ParticleEditor.saveParticleDialogDontSave( " @ PE_ParticleEditor.currParticle.getName() @ " ); PE_EmitterEditor.updateEmitter( \"particles\"," @ %particles @ ");", 
         "PE_EmitterEditor.guiSync();" );
   }
   else
   {
      PE_EmitterEditor.updateEmitter( "particles", %particles );
   }
}

//---------------------------------------------------------------------------------------------

function PE_EmitterEditor::onNewEmitter( %this )
{
   if(    isObject( PE_EmitterEditor.currEmitter )
       && PE_EmitterEditor.currEmitter $= PEE_EmitterSelector.getSelected() )
      return;
      
   //FIXME: disregards particle tab dirty state
   
   if( PE_EmitterEditor.dirty )
   {   
      
      if( PE_ParticleEditor.dirty )
      {         
         MessageBoxYesNo("Save Existing Particle?", 
            "Do you want to save changes to <br><br>" @ PE_ParticleEditor.currParticle.getName(), 
            "PE_ParticleEditor.saveParticle(" @ PE_ParticleEditor.currParticle @ ");"
         );
      }
            
      %savedEmitter = PE_EmitterEditor.currEmitter;
      MessageBoxYesNoCancel("Save Existing Emitter?", 
         "Do you want to save changes to <br><br>" @ %savedEmitter.getName(), 
         "PE_EmitterEditor.saveEmitter(" @ %savedEmitter@ "); PE_EmitterEditor.loadNewEmitter();", 
         "PE_EmitterEditor.saveEmitterDialogDontSave(" @ %savedEmitter @ "); PE_EmitterEditor.loadNewEmitter();"
      );
      

   }
   else
   {
      PE_EmitterEditor.loadNewEmitter();
   }
}

//---------------------------------------------------------------------------------------------

function PE_EmitterEditor::loadNewEmitter( %this, %emitter )
{
   if( isObject( %emitter ) )
      %current = %emitter.getId();
   else
      %current = PEE_EmitterSelector.getSelected();
   
   PE_EmitterEditor.currEmitter = %current;
   PE_EmitterEditor_NotDirtyEmitter.assignFieldsFrom( %current );
   PE_EmitterEditor_NotDirtyEmitter.originalName = %current.name;
   
   PE_EmitterEditor.guiSync();
   PE_EmitterEditor.setEmitterNotDirty();
      
   PE_ParticleEditor.loadNewParticle( getWord( %current.particles, 0 ) );
   
   ParticleEditor.updateEmitterNode();
   
   PE_EmitterEditor-->PEE_infiniteLoop.setStateOn( %current.lifetimeMS == 0 );
}
   
//---------------------------------------------------------------------------------------------

function PE_EmitterEditor::setEmitterDirty( %this )
{
   PE_EmitterEditor.text = "Emitter *";
   PE_EmitterEditor.dirty = true;
   
   %emitter = PE_EmitterEditor.currEmitter;
   
   if( %emitter.getFilename() $= "" || %emitter.getFilename() $= "tools/particleEditor/particleEmitterEditor.ed.cs" )
      PE_EmitterSaver.setDirty( %emitter, $PE_EMITTEREDITOR_DEFAULT_FILENAME );
   else
      PE_EmitterSaver.setDirty( %emitter );
}

//---------------------------------------------------------------------------------------------

function PE_EmitterEditor::setEmitterNotDirty( %this )
{   
   PE_EmitterEditor.text = "Emitter";
   PE_EmitterEditor.dirty = false;
   
   PE_EmitterSaver.clearAll();
}

//---------------------------------------------------------------------------------------------

// Create Functionality
function PE_EmitterEditor::showNewDialog( %this )
{
   //FIXME: disregards particle tab dirty state

   // Open a dialog if the current emitter is dirty.
   
   if( PE_ParticleEditor.dirty )
   {         
      MessageBoxYesNo("Save Existing Particle?", 
         "Do you want to save changes to <br><br>" @ PE_ParticleEditor.currParticle.getName(), 
         "PE_ParticleEditor.saveParticle(" @ PE_ParticleEditor.currParticle @ ");"
      );
   }   
   
   if( PE_EmitterEditor.dirty )
   {
      MessageBoxYesNoCancel("Save Emitter Changes?", 
         "Do you wish to save the changes made to the <br>current emitter before changing the emitter?", 
         "PE_EmitterEditor.saveEmitter( " @ PE_EmitterEditor.currEmitter.getName() @ " ); PE_EmitterEditor.createEmitter();", 
         "PE_EmitterEditor.saveEmitterDialogDontSave( " @ PE_EmitterEditor.currEmitter.getName() @ " ); PE_EmitterEditor.createEmitter();"
      );
   }
   else
   {
      PE_EmitterEditor.createEmitter();
   }
}

//---------------------------------------------------------------------------------------------

function PE_EmitterEditor::createEmitter( %this )
{
   // Create a new emitter.
   %emitter = getUniqueName( "newEmitter" );
   datablock ParticleEmitterData( %emitter : DefaultEmitter )
   {
   };
   
   // Submit undo.
   
   %action = ParticleEditor.createUndo( ActionCreateNewEmitter, "Create New Emitter" );
   %action.prevEmitter = PE_EmitterEditor.currEmitter;
   %action.emitter = %emitter.getId();
   %action.emitterName = %emitter;

   ParticleEditor.submitUndo( %action );

   // Execute action.
   
   %action.redo();
   
   PE_ParticleEditor.createParticle(false);
}

//---------------------------------------------------------------------------------------------

function PE_EmitterEditor::showDeleteDialog( %this )
{
   if( PE_EmitterEditor.currEmitter.getName() $= "DefaultEmitter" )
   {
      MessageBoxOK( "Error", "Cannot delete DefaultEmitter");
      return;
   }
   
   if( isObject( PE_EmitterEditor.currEmitter ) )
   {
      MessageBoxYesNoCancel("Delete Emitter?", 
         "Are you sure you want to delete<br><br>" @ PE_EmitterEditor.currEmitter.getName() @ "<br><br> Emitter deletion won't take affect until the level is exited.", 
         "PE_EmitterEditor.saveEmitterDialogDontSave( " @ PE_EmitterEditor.currEmitter.getName() @ " ); PE_EmitterEditor.deleteEmitter();"
      );
   }
}

//---------------------------------------------------------------------------------------------

function PE_EmitterEditor::deleteEmitter( %this )
{  
   %emitter = PE_EmitterEditor.currEmitter;
   
   // Create undo.
   
   %action = ParticleEditor.createUndo( ActionDeleteEmitter, "Delete Emitter" );
   %action.emitter = %emitter;
   %action.emitterFname = %emitter.getFilename();

   ParticleEditor.submitUndo( %action );
   
   // Execute action.

   %action.redo();
}

//---------------------------------------------------------------------------------------------

function PE_EmitterEditor::saveEmitter( %this, %emitter )
{   
   
   
   if ( %emitter $= "" )
      %newName = PEE_EmitterSelector_Control->TextEdit.getText();
   else
      %newName = %emitter.getName();
   
   PE_EmitterEditor.currEmitter.setName( %newName );
   PE_EmitterEditor_NotDirtyEmitter.assignFieldsFrom( %emitter );
   PE_EmitterEditor_NotDirtyEmitter.originalName = %newName;
   
   PE_EmitterSaver.saveDirty(); 
   
   PE_EmitterEditor.currEmitter = %newName.getId();
   PE_EmitterEditor.setEmitterNotDirty();
   
   ParticleEditor.createParticleList(); 
}

//---------------------------------------------------------------------------------------------

function PE_EmitterEditor::saveEmitterDialogDontSave( %this, %emitter)
{  
   %emitter.setName( PE_EmitterEditor_NotDirtyEmitter.originalName );
   %emitter.assignFieldsFrom( PE_EmitterEditor_NotDirtyEmitter );
   PE_EmitterEditor.setEmitterNotDirty();
}

//=============================================================================================
//    PEE_EmitterSelector_Control.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function PEE_EmitterSelector_Control::onRenameItem( %this )
{
   Parent::onRenameItem( %this );
   
   //FIXME: need to check for validity of name and name clashes

   PE_EmitterEditor.setEmitterDirty();
   
   // Resort menu.
   
   %this-->PopupMenu.sort();
}
