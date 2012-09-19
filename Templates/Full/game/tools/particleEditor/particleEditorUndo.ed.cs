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


//=============================================================================================
//    ParticleEditor.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function ParticleEditor::createUndo( %this, %class, %desc )
{
   pushInstantGroup();
   %action = new UndoScriptAction()
   {
      class = %class;
      superClass = BaseParticleEdAction;
      actionName = %desc;
   };
   popInstantGroup();
   return %action;
}

//---------------------------------------------------------------------------------------------

function ParticleEditor::submitUndo( %this, %action )
{
   %action.addToManager( Editor.getUndoManager() );
}

//=============================================================================================
//    BaseParticleEdAction.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function BaseParticleEdAction::sync( %this )
{
   // Sync particle state.
   
   if( isObject( %this.particle ) )
   {
      %this.particle.reload();
      PE_ParticleEditor.guiSync();
      
      if( %this.particle.getId() == PE_ParticleEditor.currParticle.getId() )
         PE_ParticleEditor.setParticleDirty();
   }

   // Sync emitter state.

   if( isObject( %this.emitter ) )
   {
      %this.emitter.reload();
      
      PE_EmitterEditor.guiSync();

      if( %this.emitter.getId() == PE_EmitterEditor.currEmitter.getId() )
         PE_EmitterEditor.setEmitterDirty();
   }
}

//---------------------------------------------------------------------------------------------

function BaseParticleEdAction::redo( %this )
{
   %this.sync();
}

//---------------------------------------------------------------------------------------------

function BaseParticleEdAction::undo( %this )
{
   %this.sync();
}

//=============================================================================================
//    ActionRenameEmitter.
//=============================================================================================

//---------------------------------------------------------------------------------------------

//TODO

//=============================================================================================
//    ActionCreateNewEmitter.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function ActionCreateNewEmitter::redo( %this )
{
   %emitter = %this.emitter;
   
   // Assign name.
   
   %emitter.name = %this.emitterName;
   
   // Remove from unlisted.
   
   PE_UnlistedEmitters.remove( %emitter );
   
   // Drop it in the dropdown and select it.
   
   %popup = PEE_EmitterSelector;

   %popup.add( %emitter.getName(), %emitter.getId() );
   %popup.sort();
   %popup.setSelected( %emitter.getId() );   
   
   // Sync up.
   
   Parent::redo( %this );
}

//---------------------------------------------------------------------------------------------

function ActionCreateNewEmitter::undo( %this )
{
   %emitter = %this.emitter;
      
   // Prevent a save dialog coming up on the emitter.
   
   if( %emitter == PE_EmitterEditor.currEmitter )
      PE_EmitterEditor.setEmitterNotDirty();
   
   // Add to unlisted.

   PE_UnlistedEmitters.add( %emitter );
   
   // Remove it from in the dropdown and select prev emitter.
   
   %popup = PEE_EmitterSelector;
      
   if( isObject( %this.prevEmitter ) )
      %popup.setSelected( %this.prevEmitter.getId() );
   else
      %popup.setFirstSelected();

   %popup.clearEntry( %emitter.getId() );
      
   // Unassign name.
   
   %this.emitterName = %emitter.name;
   %emitter.name = "";

   // Sync up.
   
   Parent::undo( %this );
}

//=============================================================================================
//    ActionDeleteEmitter.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function ActionDeleteEmitter::redo( %this )
{
   %emitter = %this.emitter;
      
   // Unassign name.
   
   %this.emitterName = %emitter.name;
   %emitter.name = "";

   // Add to unlisted.
   
   PE_UnlistedEmitters.add( %emitter );
   
   // Remove from file.
   
   if(    %emitter.getFileName() !$= ""
       && %emitter.getFilename() !$= "tools/particleEditor/particleEmitterEditor.ed.cs" )
      PE_EmitterSaver.removeObjectFromFile( %emitter );
      
   // Select DefaultEmitter or first in list.
   
   %popup = PEE_EmitterSelector_Control-->PopUpMenu;

   %popup.setFirstSelected();
      
   // Remove from dropdown.

   %popup.clearEntry( %emitter );

   // Sync up.

   Parent::redo( %this );
}

//---------------------------------------------------------------------------------------------

function ActionDeleteEmitter::undo( %this )
{
   %emitter = %this.emitter;
      
   // Re-assign name.
   
   %emitter.name = %this.emitterName;

   // Remove from unlisted.
   
   PE_UnlistedEmitters.remove( %emitter );
   
   // Resave to file.
   
   if(    %this.emitterFname !$= ""
       && %this.emitterFname !$= "tools/particleEditor/particleEmitterEditor.ed.gui" )
   {
      PE_EmitterSaver.setDirty( %emitter, %this.emitterFname );
      PE_EmitterSaver.saveDirty();
   }

   // Add it to the dropdown and selet it.
   
   %popup = PEE_EmitterSelector_Control-->PopUpMenu;
   %popup.add( %emitter.getName(), %emitter.getId() );
   %popup.sort();
   %popup.setSelected( %emitter.getId() );
   
   // Sync up.
   
   Parent::undo( %this );
}

//=============================================================================================
//    ActionUpdateActiveEmitter.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function ActionUpdateActiveEmitter::redo( %this )
{
   %emitter = %this.emitter;
   %emitter.setFieldValue( %this.field, %this.newValue );
   
   Parent::redo( %this );
}

//---------------------------------------------------------------------------------------------

function ActionUpdateActiveEmitter::undo( %this )
{
   %emitter = %this.emitter;
   %emitter.setFieldValue( %this.field, %this.oldValue );
   
   Parent::undo( %this );
}

//=============================================================================================
//    ActionUpdateActiveEmitterLifeFields.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function ActionUpdateActiveEmitterLifeFields::redo( %this )
{
   %emitter = %this.emitter;
   
   %emitter.lifetimeMS = %this.newValueLifetimeMS;
   %emitter.lifetimeVarianceMS = %this.newValueLifetimeVarianceMS;
   
   Parent::redo( %this );
}

//---------------------------------------------------------------------------------------------

function ActionUpdateActiveEmitterLifeFields::undo( %this )
{
   %emitter = %this.emitter;

   %emitter.lifetimeMS = %this.oldValueLifetimeMS;
   %emitter.lifetimeVarianceMS = %this.oldValueLifetimeVarianceMS;
   
   Parent::undo( %this );
}

//=============================================================================================
//    ActionUpdateActiveEmitterAmountFields.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function ActionUpdateActiveEmitterAmountFields::redo( %this )
{
   %emitter = %this.emitter;
   
   %emitter.ejectionPeriodMS = %this.newValueEjectionPeriodMS;
   %emitter.periodVarianceMS = %this.newValuePeriodVarianceMS;
   
   Parent::redo( %this );
}

//---------------------------------------------------------------------------------------------

function ActionUpdateActiveEmitterAmountFields::undo( %this )
{
   %emitter = %this.emitter;
   
   %emitter.ejectionPeriodMS = %this.oldValueEjectionPeriodMS;
   %emitter.periodVarianceMS = %this.oldValuePeriodVarianceMS;
   
   Parent::undo( %this );
}

//=============================================================================================
//    ActionUpdateActiveEmitterSpeedFields.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function ActionUpdateActiveEmitterSpeedFields::redo( %this )
{
   %emitter = %this.emitter;
   
   %emitter.ejectionVelocity = %this.newValueEjectionVelocity;
   %emitter.velocityVariance = %this.newValueVelocityVariance;
   
   Parent::redo( %this );
}

//---------------------------------------------------------------------------------------------

function ActionUpdateActiveEmitterSpeedFields::undo( %this )
{
   %emitter = %this.emitter;

   %emitter.ejectionVelocity = %this.oldValueEjectionVelocity;
   %emitter.velocityVariance = %this.oldValueVelocityVariance;
   
   Parent::undo( %this );
}

//=============================================================================================
//    ActionCreateNewParticle.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function ActionCreateNewParticle::redo( %this )
{
   %particle = %this.particle.getName();
   %particleId = %this.particle.getId();
   %particleIndex = %this.particleIndex;
   %emitter = %this.emitter;
   
   // Remove from unlisted.
   
   PE_UnlistedParticles.remove( %particleId );
   
   // Add it to the dropdown.
   
   PEP_ParticleSelector.add( %particle, %particleId );
   PEP_ParticleSelector.sort();
   PEP_ParticleSelector.setSelected( %particleId, false );
         
   // Add particle to dropdowns in the emitter editor.
   
   for( %i = 1; %i < 5; %i ++ )
   {
      %emitterParticle = "PEE_EmitterParticle" @ %i;
      %popup = %emitterParticle-->PopupMenu;
      
      %popup.add( %particle, %particleId );
      %popup.sort();
      
      if( %i == %particleIndex + 1 )
         %popup.setSelected( %particleId );
   }
   
   // Sync up.
   
   PE_ParticleEditor.loadNewParticle();
   Parent::redo( %this );
}

//---------------------------------------------------------------------------------------------

function ActionCreateNewParticle::undo( %this )
{
   %particle = %this.particle.getName();
   %particleId = %this.particle.getId();
   %emitter = %this.emitter;
   
   // Add to unlisted.
   
   PE_UnlistedParticles.add( %particleId );
      
   // Remove from dropdown.
   
   PEP_ParticleSelector.clearEntry( %particleId );
   PEP_ParticleSelector.setFirstSelected( false );
      
   // Remove from particle dropdowns in emitter editor.
   
   for( %i = 1; %i < 5; %i ++ )
   {
      %emitterParticle = "PEE_EmitterParticle" @ %i;
      %popup = %emitterParticle-->PopUpMenu;
      
      if( %popup.getSelected() == %particleId )
         %popup.setSelected( %this.prevParticle );
         
      %popup.clearEntry( %particleId );
   }
   
   // Sync up.
   
   PE_ParticleEditor.loadNewParticle();
   Parent::undo( %this );
}

//=============================================================================================
//    ActionDeleteParticle.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function ActionDeleteParticle::redo( %this )
{
   %particle = %this.particle.getName();
   %particleId = %this.particle.getId();
   %emitter = %this.emitter;
   
   // Add to unlisted.
   
   PE_UnlistedParticles.add( %particleId );
   
   // Remove from file.
   
   if(    %particle.getFileName() !$= ""
       && %particle.getFilename() !$= "tools/particleEditor/particleParticleEditor.ed.cs" )
      PE_ParticleSaver.removeObjectFromFile( %particleId );
      
   // Remove from dropdown.
   
   PEP_ParticleSelector.clearEntry( %particleId );
   PEP_ParticleSelector.setFirstSelected();
   
   // Remove from particle selectors in emitter.
   
   for( %i = 1; %i < 5; %i ++ )
   {
      %emitterParticle = "PEE_EmitterParticle" @ %i;
      %popup = %emitterParticle-->PopUpMenu;
      
      if( %popup.getSelected() == %particleId )
      {
         %this.particleIndex = %i - 1;
         %popup.setSelected( 0 ); // Select "None".
      }
      
      %popup.clearEntry( %particleId );
   }
   
   // Sync up.
   
   PE_ParticleEditor.loadNewParticle();   
   Parent::redo( %this );
}

//---------------------------------------------------------------------------------------------

function ActionDeleteParticle::undo( %this )
{
   %particle = %this.particle.getName();
   %particleId = %this.particle.getId();
   %particleIndex = %this.particleIndex;
   %emitter = %this.emitter;
   
   // Remove from unlisted.
   
   PE_UnlistedParticles.remove( %particleId );

   // Resave to file.
   
   if(    %particle.getFilename() !$= ""
       && %particle.getFilename() !$= "tools/particleEditor/particleParticleEditor.ed.gui" )
   {
      PE_ParticleSaver.setDirty( %particle );
      PE_ParticleSaver.saveDirty();
   }
   
   // Add to dropdown.
   
   PEP_ParticleSelector.add( %particle, %particleId );
   PEP_ParticleSelector.sort();
   PEP_ParticleSelector.setSelected( %particleId );
         
   // Add particle to dropdowns in the emitter editor.
   
   for( %i = 1; %i < 5; %i ++ )
   {
      %emitterParticle = "PEE_EmitterParticle" @ %i;
      %popup = %emitterParticle-->PopUpMenu;
      
      %popup.add( %particle, %particleId );
      %popup.sort();
      
      if( %i == %particleIndex + 1 )
         %popup.setSelected( %particleId );
   }
   
   // Sync up.
   
   PE_ParticleEditor.loadNewParticle();
   Parent::undo( %This );
}

//=============================================================================================
//    ActionUpdateActiveParticle.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function ActionUpdateActiveParticle::redo( %this )
{
   %particle = %this.particle;
   %particle.setFieldValue( %this.field, %this.newValue );
   
   Parent::redo( %this );
}

function ActionUpdateActiveParticle::undo( %this )
{
   %particle = %this.particle;   
   %particle.setFieldValue( %this.field, %this.oldValue );
   
   Parent::undo( %this );
}

//=============================================================================================
//    ActionUpdateActiveParticleLifeFields.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function ActionUpdateActiveParticleLifeFields::redo( %this )
{
   %particle = %this.particle;
   
   %particle.lifetimeMS = %this.newValueLifetimeMS;
   %particle.lifetimeVarianceMS = %this.newValueLifetimeVarianceMS;
   
   Parent::redo( %this );
}

//---------------------------------------------------------------------------------------------

function ActionUpdateActiveParticleLifeFields::undo( %this )
{
   %particle = %this.particle;

   %particle.lifetimeMS = %this.oldValueLifetimeMS;
   %particle.lifetimeVarianceMS = %this.oldValueLifetimeVarianceMS;

   Parent::undo( %this );
}

//=============================================================================================
//    ActionUpdateActiveParticleSpinFields.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function ActionUpdateActiveParticleSpinFields::redo( %this )
{
   %particle = %this.particle;
   
   %particle.spinRandomMax = %this.newValueSpinRandomMax;
   %particle.spinRandomMin = %this.newValueSpinRandomMin;
   
   Parent::redo( %this );
}

//---------------------------------------------------------------------------------------------

function ActionUpdateActiveParticleSpinFields::undo( %this )
{
   %particle = %this.particle;

   %particle.spinRandomMax = %this.oldValueSpinRandomMax;
   %particle.spinRandomMin = %this.oldValueSpinRandomMin;
   
   Parent::undo( %this );
}
