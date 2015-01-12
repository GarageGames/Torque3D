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

// The TSShapeConstructor object allows you to apply a set of transformations
// to a 3space shape after it is loaded by Torque, but _before_ the shape is used
// by any other object (eg. Player, StaticShape etc). The sort of transformations
// available include adding, renaming and removing nodes and sequences. This GUI
// is a visual wrapper around TSShapeConstructor which allows you to build up the
// transformation set without having to get your hands dirty with TorqueScript.
//
// Removing a node, sequence, mesh or detail poses a problem. These operations
// permanently delete a potentially large amount of data scattered throughout
// the shape, and there is no easy way to restore it if the user 'undoes' the
// delete. Although it is possible to store the deleted data somewhere and restore
// it on undo, it is not easy to get right, and ugly as hell to implement. For
// example, removing a node would require storing the node name, the
// translation/rotation/scale matters bit for each sequence, all node transform
// keyframes, the IDs of any objects that were attached to the node, skin weights
// etc, then restoring all that data into the original place on undo. Frankly,
// TSShape was never designed to be modified dynamically like that.
//
// So......currently we wimp out completely and just don't support undo for those
// remove operations. Lame, I know, but the best I can do for now.
//
// This file implements all of the actions that can be applied by the GUI. Each
// action has 3 methods:
//
//    doit: called the first time the action is performed
//    undo: called to undo the action
//    redo: called to redo the action (usually the same as doit)
//
// In each case, the appropriate change is made to the shape, and the GUI updated.
//
// TSShapeConstructor keeps track of all the changes made and provides a simple
// way to save the modifications back out to a script file.

// The ShapeEditor uses its own UndoManager
if ( !isObject( ShapeEdUndoManager ) )
   new UndoManager( ShapeEdUndoManager );

function ShapeEdUndoManager::updateUndoMenu( %this, %editMenu )
{
   %undoName = %this.getNextUndoName();
   %redoName = %this.getNextRedoName();
   
   %editMenu.setItemName( 0, "Undo " @ %undoName );
   %editMenu.setItemName( 1, "Redo " @ %redoName );
   
   %editMenu.enableItem( 0, %undoName !$= "" );
   %editMenu.enableItem( 1, %redoName !$= "" );
}

//------------------------------------------------------------------------------
// Helper functions for creating and applying GUI operations

function ShapeEditor::createAction( %this, %class, %desc )
{
   pushInstantGroup();
   %action = new UndoScriptAction()
   {
      class = %class;
      superClass = BaseShapeEdAction;
      actionName = %desc;
      done = 0;
   };
   popInstantGroup();
   return %action;
}

function ShapeEditor::doAction( %this, %action )
{
   if ( %action.doit() )
   {
      ShapeEditor.setDirty( true );
      %action.addToManager( ShapeEdUndoManager );
   }
   else
   {
      MessageBoxOK( "Error", %action.actionName SPC "failed. Check the console for error messages.", "" );
   }
}

function BaseShapeEdAction::redo( %this )
{
   // Default redo action is the same as the doit action
   if ( %this.doit() )
   {
      ShapeEditor.setDirty( true );
   }
   else
   {
      MessageBoxOK( "Error", "Redo" SPC %this.actionName SPC "failed. Check the console for error messages.", "" );
   }
}

function BaseShapeEdAction::undo( %this )
{
   ShapeEditor.setDirty( true );
}

//------------------------------------------------------------------------------

function ShapeEditor::doRemoveShapeData( %this, %type, %name )
{
   // Removing data from the shape cannot be undone => so warn the user first
   MessageBoxYesNo( "Warning", "Deleting a " @ %type @ " cannot be undone. Do " @
      "you want to continue?", "ShapeEditor.doRemove" @ %type @ "( \"" @ %name @ "\" );", "" );
}

//------------------------------------------------------------------------------
// Add node
function ShapeEditor::doAddNode( %this, %nodeName, %parentName, %transform )
{
   %action = %this.createAction( ActionAddNode, "Add node" );
   %action.nodeName = %nodeName;
   %action.parentName = %parentName;
   %action.transform = %transform;

   %this.doAction( %action );
}

function ActionAddNode::doit( %this )
{
   if ( ShapeEditor.shape.addNode( %this.nodeName, %this.parentName, %this.transform ) )
   {
      ShapeEdPropWindow.update_onNodeAdded( %this.nodeName, -1 );
      return true;
   }
   return false;
}

function ActionAddNode::undo( %this )
{
   Parent::undo( %this );

   if ( ShapeEditor.shape.removeNode( %this.nodeName ) )
      ShapeEdPropWindow.update_onNodeRemoved( %this.nodeName, 1 );
}

//------------------------------------------------------------------------------
// Remove node
function ShapeEditor::doRemoveNode( %this, %nodeName )
{
   %action = %this.createAction( ActionRemoveNode, "Remove node" );
   %action.nodeName =%nodeName;
   %action.nodeChildIndex = ShapeEdNodeTreeView.getChildIndexByName( %nodeName );

   // Need to delete all child nodes of this node as well, so recursively collect
   // all of the names.
   %action.nameList = %this.getNodeNames( %nodeName, "" );
   %action.nameCount = getFieldCount( %action.nameList );
   for ( %i = 0; %i < %action.nameCount; %i++ )
      %action.names[%i] = getField( %action.nameList, %i );

   %this.doAction( %action );
}

function ActionRemoveNode::doit( %this )
{
   for ( %i = 0; %i < %this.nameCount; %i++ )
      ShapeEditor.shape.removeNode( %this.names[%i] );

   // Update GUI
   ShapeEdPropWindow.update_onNodeRemoved( %this.nameList, %this.nameCount );

   return true;
}

function ActionRemoveNode::undo( %this )
{
   Parent::undo( %this );
}

//------------------------------------------------------------------------------
// Rename node
function ShapeEditor::doRenameNode( %this, %oldName, %newName )
{
   %action = %this.createAction( ActionRenameNode, "Rename node" );
   %action.oldName = %oldName;
   %action.newName = %newName;

   %this.doAction( %action );
}

function ActionRenameNode::doit( %this )
{
   if ( ShapeEditor.shape.renameNode( %this.oldName, %this.newName ) )
   {
      ShapeEdPropWindow.update_onNodeRenamed( %this.oldName, %this.newName );
      return true;
   }
   return false;
}

function ActionRenameNode::undo( %this )
{
   Parent::undo( %this );

   if ( ShapeEditor.shape.renameNode( %this.newName, %this.oldName ) )
      ShapeEdPropWindow.update_onNodeRenamed( %this.newName, %this.oldName );
}

//------------------------------------------------------------------------------
// Set node parent
function ShapeEditor::doSetNodeParent( %this, %name, %parent )
{
   if ( %parent $= "<root>" )
      %parent = "";

   %action = %this.createAction( ActionSetNodeParent, "Set parent node" );
   %action.nodeName = %name;
   %action.parentName = %parent;
   %action.oldParentName = ShapeEditor.shape.getNodeParentName( %name );

   %this.doAction( %action );
}

function ActionSetNodeParent::doit( %this )
{
   if ( ShapeEditor.shape.setNodeParent( %this.nodeName, %this.parentName ) )
   {
      ShapeEdPropWindow.update_onNodeParentChanged( %this.nodeName );
      return true;
   }
   return false;
}

function ActionSetNodeParent::undo( %this )
{
   Parent::undo( %this );

   if ( ShapeEditor.shape.setNodeParent( %this.nodeName, %this.oldParentName ) )
      ShapeEdPropWindow.update_onNodeParentChanged( %this.nodeName );
}

//------------------------------------------------------------------------------
// Edit node transform
function ShapeEditor::doEditNodeTransform( %this, %nodeName, %newTransform, %isWorld, %gizmoID )
{
   // If dragging the 3D gizmo, combine all movement into a single action. Undoing
   // that action will return the node to where it was when the gizmo drag started.
   %last = ShapeEdUndoManager.getUndoAction( ShapeEdUndoManager.getUndoCount() - 1 );
   if ( ( %last != -1 ) && ( %last.class $= ActionEditNodeTransform ) &&
      ( %last.nodeName $= %nodeName ) && ( %last.gizmoID != -1 ) && ( %last.gizmoID == %gizmoID ) )
   {
      // Use the last action to do the edit, and modify it so it only applies
      // the latest transform
      %last.newTransform = %newTransform;
      %last.isWorld = %isWorld;
      %last.doit();
      ShapeEditor.setDirty( true );
   }
   else
   {
      %action = %this.createAction( ActionEditNodeTransform, "Edit node transform" );
      %action.nodeName = %nodeName;
      %action.newTransform = %newTransform;
      %action.isWorld = %isWorld;
      %action.gizmoID = %gizmoID;
      %action.oldTransform = %this.shape.getNodeTransform( %nodeName, %isWorld );

      %this.doAction( %action );
   }
}

function ActionEditNodeTransform::doit( %this )
{
   ShapeEditor.shape.setNodeTransform( %this.nodeName, %this.newTransform, %this.isWorld );
   ShapeEdPropWindow.update_onNodeTransformChanged();
   return true;
}

function ActionEditNodeTransform::undo( %this )
{
   Parent::undo( %this );

   ShapeEditor.shape.setNodeTransform( %this.nodeName, %this.oldTransform, %this.isWorld );
   ShapeEdPropWindow.update_onNodeTransformChanged();
}

//------------------------------------------------------------------------------
// Add sequence
function ShapeEditor::doAddSequence( %this, %seqName, %from, %start, %end )
{
   %action = %this.createAction( ActionAddSequence, "Add sequence" );
   %action.seqName = %seqName;
   %action.origFrom = %from;
   %action.from = %from;
   %action.start = %start;
   %action.end = %end;

   %this.doAction( %action );
}

function ActionAddSequence::doit( %this )
{
   // If adding this sequence from an existing sequence, make a backup copy of
   // the existing sequence first, so we can edit the start/end frames later
   // without having to worry if the original source sequence has changed.
   if ( ShapeEditor.shape.getSequenceIndex( %this.from ) >= 0 )
   {
      %this.from = ShapeEditor.getUniqueName( "sequence", "__backup__" @ %this.origFrom @ "_" );
      ShapeEditor.shape.addSequence( %this.origFrom, %this.from );
   }

   // Add the sequence
   $collada::forceLoadDAE = EditorSettings.value( "forceLoadDAE" );
   %success = ShapeEditor.shape.addSequence( %this.from, %this.seqName, %this.start, %this.end );
   $collada::forceLoadDAE = false;

   if ( %success )
   {
      ShapeEdPropWindow.update_onSequenceAdded( %this.seqName, -1 );
      return true;
   }
   return false;
}

function ActionAddSequence::undo( %this )
{
   Parent::undo( %this );

   // Remove the backup sequence if one was created
   if ( %this.origFrom !$= %this.from )
   {
      ShapeEditor.shape.removeSequence( %this.from );
      %this.from = %this.origFrom;
   }

   // Remove the actual sequence
   if ( ShapeEditor.shape.removeSequence( %this.seqName ) )
      ShapeEdPropWindow.update_onSequenceRemoved( %this.seqName );
}

//------------------------------------------------------------------------------
// Remove sequence

function ShapeEditor::doRemoveSequence( %this, %seqName )
{
   %action = %this.createAction( ActionRemoveSequence, "Remove sequence" );
   %action.seqName = %seqName;

   %this.doAction( %action );
}

function ActionRemoveSequence::doit( %this )
{
   if ( ShapeEditor.shape.removeSequence( %this.seqName ) )
   {
      ShapeEdPropWindow.update_onSequenceRemoved( %this.seqName );
      return true;
   }
   return false;
}

function ActionRemoveSequence::undo( %this )
{
   Parent::undo( %this );
}

//------------------------------------------------------------------------------
// Rename sequence
function ShapeEditor::doRenameSequence( %this, %oldName, %newName )
{
   %action = %this.createAction( ActionRenameSequence, "Rename sequence" );
   %action.oldName = %oldName;
   %action.newName = %newName;

   %this.doAction( %action );
}

function ActionRenameSequence::doit( %this )
{
   if ( ShapeEditor.shape.renameSequence( %this.oldName, %this.newName ) )
   {
      ShapeEdPropWindow.update_onSequenceRenamed( %this.oldName, %this.newName );
      return true;
   }
   return false;
}

function ActionRenameSequence::undo( %this )
{
   Parent::undo( %this );

   if ( ShapeEditor.shape.renameSequence( %this.newName, %this.oldName ) )
      ShapeEdPropWindow.update_onSequenceRenamed( %this.newName, %this.oldName );
}

//------------------------------------------------------------------------------
// Edit sequence source data ( parent, start or end )
function ShapeEditor::doEditSeqSource( %this, %seqName, %from, %start, %end )
{
   %action = %this.createAction( ActionEditSeqSource, "Edit sequence source data" );
   %action.seqName = %seqName;
   %action.origFrom = %from;
   %action.from = %from;
   %action.start = %start;
   %action.end = %end;

   // To support undo, the sequence will be renamed instead of removed (undo just
   // removes the added sequence and renames the original back). Generate a unique
   // name for the backed up sequence
   %action.seqBackup = ShapeEditor.getUniqueName( "sequence", "__backup__" @ %action.seqName @  "_" );

   // If editing an internal sequence, the source is the renamed backup
   if ( %action.from $= %action.seqName )
      %action.from = %action.seqBackup;

   %this.doAction( %action );
}

function ActionEditSeqSource::doit( %this )
{
   // If changing the source to an existing sequence, make a backup copy of
   // the existing sequence first, so we can edit the start/end frames later
   // without having to worry if the original source sequence has changed.
   if ( !startswith( %this.from, "__backup__" ) &&
        ShapeEditor.shape.getSequenceIndex( %this.from ) >= 0 )
   {
      %this.from = ShapeEditor.getUniqueName( "sequence", "__backup__" @ %this.origFrom @ "_" );
      ShapeEditor.shape.addSequence( %this.origFrom, %this.from );
   }

   // Get settings we want to retain
   %priority = ShapeEditor.shape.getSequencePriority( %this.seqName );
   %cyclic = ShapeEditor.shape.getSequenceCyclic( %this.seqName );
   %blend = ShapeEditor.shape.getSequenceBlend( %this.seqName );

   // Rename this sequence (instead of removing it) so we can undo this action
   ShapeEditor.shape.renameSequence( %this.seqName, %this.seqBackup );

   // Add the new sequence
   if ( ShapeEditor.shape.addSequence( %this.from, %this.seqName, %this.start, %this.end ) )
   {
      // Restore original settings
      if ( ShapeEditor.shape.getSequencePriority ( %this.seqName ) != %priority )
         ShapeEditor.shape.setSequencePriority( %this.seqName, %priority );
      if ( ShapeEditor.shape.getSequenceCyclic( %this.seqName ) != %cyclic )
         ShapeEditor.shape.setSequenceCyclic( %this.seqName, %cyclic );

      %newBlend = ShapeEditor.shape.getSequenceBlend( %this.seqName );
      if ( %newBlend !$= %blend )
      {
         // Undo current blend, then apply new one
         ShapeEditor.shape.setSequenceBlend( %this.seqName, 0, getField( %newBlend, 1 ), getField( %newBlend, 2 ) );
         if ( getField( %blend, 0 ) == 1 )
            ShapeEditor.shape.setSequenceBlend( %this.seqName, getField( %blend, 0 ), getField( %blend, 1 ), getField( %blend, 2 ) );
      }

      if ( ShapeEdSequenceList.getSelectedName() $= %this.seqName )
      {
         ShapeEdSequenceList.editColumn( %this.seqName, 3, %this.end - %this.start + 1 );
         ShapeEdPropWindow.syncPlaybackDetails();
      }

      return true;
   }
   return false;
}

function ActionEditSeqSource::undo( %this )
{
   Parent::undo( %this );

   // Remove the source sequence backup if one was created
   if ( ( %this.from !$= %this.origFrom ) && ( %this.from !$= %this.seqBackup ) )
   {
      ShapeEditor.shape.removeSequence( %this.from );
      %this.from = %this.origFrom;
   }

   // Remove the added sequence, and rename the backup back
   if ( ShapeEditor.shape.removeSequence( %this.seqName ) &&
      ShapeEditor.shape.renameSequence( %this.seqBackup, %this.seqName ) )
   {
      if ( ShapeEdSequenceList.getSelectedName() $= %this.seqName )
      {
         ShapeEdSequenceList.editColumn( %this.seqName, 3, %this.end - %this.start + 1 );
         ShapeEdPropWindow.syncPlaybackDetails();
      }
   }
}

//------------------------------------------------------------------------------
// Edit cyclic flag
function ShapeEditor::doEditCyclic( %this, %seqName, %cyclic )
{
   %action = %this.createAction( ActionEditCyclic, "Toggle cyclic flag" );
   %action.seqName = %seqName;
   %action.cyclic = %cyclic;

   %this.doAction( %action );
}

function ActionEditCyclic::doit( %this )
{
   if ( ShapeEditor.shape.setSequenceCyclic( %this.seqName, %this.cyclic ) )
   {
      ShapeEdPropWindow.update_onSequenceCyclicChanged( %this.seqName, %this.cyclic );
      return true;
   }
   return false;
}

function ActionEditCyclic::undo( %this )
{
   Parent::undo( %this );

   if ( ShapeEditor.shape.setSequenceCyclic( %this.seqName, !%this.cyclic ) )
      ShapeEdPropWindow.update_onSequenceCyclicChanged( %this.seqName, !%this.cyclic );
}

//------------------------------------------------------------------------------
// Edit blend properties
function ShapeEditor::doEditBlend( %this, %seqName, %blend, %blendSeq, %blendFrame )
{
   %action = %this.createAction( ActionEditBlend, "Edit blend properties" );
   %action.seqName = %seqName;
   %action.blend = %blend;
   %action.blendSeq = %blendSeq;
   %action.blendFrame = %blendFrame;

   // Store the current blend settings
   %oldBlend = ShapeEditor.shape.getSequenceBlend( %seqName );
   %action.oldBlend = getField( %oldBlend, 0 );
   %action.oldBlendSeq = getField( %oldBlend, 1 );
   %action.oldBlendFrame = getField( %oldBlend, 2 );

   // Use new values if the old ones do not exist ( for blend sequences embedded
   // in the DTS/DSQ file )
   if ( %action.oldBlendSeq $= "" )
      %action.oldBlendSeq = %action.blendSeq;
   if ( %action.oldBlendFrame $= "" )
      %action.oldBlendFrame = %action.blendFrame;

   %this.doAction( %action );
}

function ActionEditBlend::doit( %this )
{
   // If we are changing the blend reference ( rather than just toggling the flag )
   // we need to undo the current blend first.
   if ( %this.blend && %this.oldBlend )
   {
      if ( !ShapeEditor.shape.setSequenceBlend( %this.seqName, false, %this.oldBlendSeq, %this.oldBlendFrame ) )
         return false;
   }

   if ( ShapeEditor.shape.setSequenceBlend( %this.seqName, %this.blend, %this.blendSeq, %this.blendFrame ) )
   {
      ShapeEdPropWindow.update_onSequenceBlendChanged( %this.seqName, %this.blend,
         %this.oldBlendSeq, %this.oldBlendFrame, %this.blendSeq, %this.blendFrame );
      return true;
   }
   return false;
}

function ActionEditBlend::undo( %this )
{
   Parent::undo( %this );

   // If we are changing the blend reference ( rather than just toggling the flag )
   // we need to undo the current blend first.
   if ( %this.blend && %this.oldBlend )
   {
      if ( !ShapeEditor.shape.setSequenceBlend( %this.seqName, false, %this.blendSeq, %this.blendFrame ) )
         return;
   }

   if ( ShapeEditor.shape.setSequenceBlend( %this.seqName, %this.oldBlend, %this.oldBlendSeq, %this.oldBlendFrame ) )
   {
      ShapeEdPropWindow.update_onSequenceBlendChanged( %this.seqName, !%this.blend,
         %this.blendSeq, %this.blendFrame, %this.oldBlendSeq, %this.oldBlendFrame );
   }
}

//------------------------------------------------------------------------------
// Edit sequence priority
function ShapeEditor::doEditSequencePriority( %this, %seqName, %newPriority )
{
   %action = %this.createAction( ActionEditSequencePriority, "Edit sequence priority" );
   %action.seqName = %seqName;
   %action.newPriority = %newPriority;
   %action.oldPriority = %this.shape.getSequencePriority( %seqName );

   %this.doAction( %action );
}

function ActionEditSequencePriority::doit( %this )
{
   if ( ShapeEditor.shape.setSequencePriority( %this.seqName, %this.newPriority ) )
   {
      ShapeEdPropWindow.update_onSequencePriorityChanged( %this.seqName );
      return true;
   }
   return false;
}

function ActionEditSequencePriority::undo( %this )
{
   Parent::undo( %this );

   if ( ShapeEditor.shape.setSequencePriority( %this.seqName, %this.oldPriority ) )
      ShapeEdPropWindow.update_onSequencePriorityChanged( %this.seqName );
}

//------------------------------------------------------------------------------
// Edit sequence ground speed
function ShapeEditor::doEditSequenceGroundSpeed( %this, %seqName, %newSpeed )
{
   %action = %this.createAction( ActionEditSequenceGroundSpeed, "Edit sequence ground speed" );
   %action.seqName = %seqName;
   %action.newSpeed = %newSpeed;
   %action.oldSpeed = %this.shape.getSequenceGroundSpeed( %seqName );

   %this.doAction( %action );
}

function ActionEditSequenceGroundSpeed::doit( %this )
{
   if ( ShapeEditor.shape.setSequenceGroundSpeed( %this.seqName, %this.newSpeed ) )
   {
      ShapeEdPropWindow.update_onSequenceGroundSpeedChanged( %this.seqName );
      return true;
   }
   return false;
}

function ActionEditSequenceGroundSpeed::undo( %this )
{
   Parent::undo( %this );

   if ( ShapeEditor.shape.setSequenceGroundSpeed( %this.seqName, %this.oldSpeed ) )
      ShapeEdPropWindow.update_onSequenceGroundSpeedChanged( %this.seqName );
}

//------------------------------------------------------------------------------
// Add trigger
function ShapeEditor::doAddTrigger( %this, %seqName, %frame, %state )
{
   %action = %this.createAction( ActionAddTrigger, "Add trigger" );
   %action.seqName = %seqName;
   %action.frame = %frame;
   %action.state = %state;

   %this.doAction( %action );
}

function ActionAddTrigger::doit( %this )
{
   if ( ShapeEditor.shape.addTrigger( %this.seqName, %this.frame, %this.state ) )
   {
      ShapeEdPropWindow.update_onTriggerAdded( %this.seqName, %this.frame, %this.state );
      return true;
   }
   return false;
}

function ActionAddTrigger::undo( %this )
{
   Parent::undo( %this );

   if ( ShapeEditor.shape.removeTrigger( %this.seqName, %this.frame, %this.state ) )
      ShapeEdPropWindow.update_onTriggerRemoved( %this.seqName, %this.frame, %this.state );
}

//------------------------------------------------------------------------------
// Remove trigger
function ShapeEditor::doRemoveTrigger( %this, %seqName, %frame, %state )
{
   %action = %this.createAction( ActionRemoveTrigger, "Remove trigger" );
   %action.seqName = %seqName;
   %action.frame = %frame;
   %action.state = %state;

   %this.doAction( %action );
}

function ActionRemoveTrigger::doit( %this )
{
   if ( ShapeEditor.shape.removeTrigger( %this.seqName, %this.frame, %this.state ) )
   {
      ShapeEdPropWindow.update_onTriggerRemoved( %this.seqName, %this.frame, %this.state );
      return true;
   }
   return false;
}

function ActionRemoveTrigger::undo( %this )
{
   Parent::undo( %this );

   if ( ShapeEditor.shape.addTrigger( %this.seqName, %this.frame, %this.state ) )
      ShapeEdPropWindow.update_onTriggerAdded( %this.seqName, %this.frame, %this.state );
}

//------------------------------------------------------------------------------
// Edit trigger
function ShapeEditor::doEditTrigger( %this, %seqName, %oldFrame, %oldState, %frame, %state )
{
   %action = %this.createAction( ActionEditTrigger, "Edit trigger" );
   %action.seqName = %seqName;
   %action.oldFrame = %oldFrame;
   %action.oldState = %oldState;
   %action.frame = %frame;
   %action.state = %state;

   %this.doAction( %action );
}

function ActionEditTrigger::doit( %this )
{
   if ( ShapeEditor.shape.addTrigger( %this.seqName, %this.frame, %this.state ) &&
      ShapeEditor.shape.removeTrigger( %this.seqName, %this.oldFrame, %this.oldState ) )
   {
      ShapeEdTriggerList.updateItem( %this.oldFrame, %this.oldState, %this.frame, %this.state );
      return true;
   }
   return false;
}

function ActionEditTrigger::undo( %this )
{
   Parent::undo( %this );

   if ( ShapeEditor.shape.addTrigger( %this.seqName, %this.oldFrame, %this.oldState ) &&
      ShapeEditor.shape.removeTrigger( %this.seqName, %this.frame, %this.state ) )
      ShapeEdTriggerList.updateItem( %this.frame, %this.state, %this.oldFrame, %this.oldState );
}

//------------------------------------------------------------------------------
// Rename detail
function ShapeEditor::doRenameDetail( %this, %oldName, %newName )
{
   %action = %this.createAction( ActionRenameDetail, "Rename detail" );
   %action.oldName = %oldName;
   %action.newName = %newName;

   %this.doAction( %action );
}

function ActionRenameDetail::doit( %this )
{
   if ( ShapeEditor.shape.renameDetailLevel( %this.oldName, %this.newName ) )
   {
      ShapeEdPropWindow.update_onDetailRenamed( %this.oldName, %this.newName );
      return true;
   }
   return false;
}

function ActionRenameDetail::undo( %this )
{
   Parent::undo( %this );
   if ( ShapeEditor.shape.renameDetailLevel( %this.newName, %this.oldName ) )
      ShapeEdPropWindow.update_onDetailRenamed( %this.newName, %this.oldName );
}

//------------------------------------------------------------------------------
// Edit detail size
function ShapeEditor::doEditDetailSize( %this, %oldSize, %newSize )
{
   %action = %this.createAction( ActionEditDetailSize, "Edit detail size" );
   %action.oldSize = %oldSize;
   %action.newSize = %newSize;

   %this.doAction( %action );
}

function ActionEditDetailSize::doit( %this )
{
   %dl = ShapeEditor.shape.setDetailLevelSize( %this.oldSize, %this.newSize );
   if ( %dl != -1 )
   {
      ShapeEdPropWindow.update_onDetailSizeChanged( %this.oldSize, %this.newSize );
      return true;
   }
   return false;
}

function ActionEditDetailSize::undo( %this )
{
   Parent::undo( %this );
   %dl = ShapeEditor.shape.setDetailLevelSize( %this.newSize, %this.oldSize );
   if ( %dl != -1 )
      ShapeEdPropWindow.update_onDetailSizeChanged( %this.newSize, %this.oldSize );
}

//------------------------------------------------------------------------------
// Rename object
function ShapeEditor::doRenameObject( %this, %oldName, %newName )
{
   %action = %this.createAction( ActionRenameObject, "Rename object" );
   %action.oldName = %oldName;
   %action.newName = %newName;

   %this.doAction( %action );
}

function ActionRenameObject::doit( %this )
{
   if ( ShapeEditor.shape.renameObject( %this.oldName, %this.newName ) )
   {
      ShapeEdPropWindow.update_onObjectRenamed( %this.oldName, %this.newName );
      return true;
   }
   return false;
}

function ActionRenameObject::undo( %this )
{
   Parent::undo( %this );
   if ( ShapeEditor.shape.renameObject( %this.newName, %this.oldName ) )
      ShapeEdPropWindow.update_onObjectRenamed( %this.newName, %this.oldName );
}

//------------------------------------------------------------------------------
// Edit mesh size
function ShapeEditor::doEditMeshSize( %this, %meshName, %size )
{
   %action = %this.createAction( ActionEditMeshSize, "Edit mesh size" );
   %action.meshName = stripTrailingNumber( %meshName );
   %action.oldSize = getTrailingNumber( %meshName );
   %action.newSize = %size;

   %this.doAction( %action );
}

function ActionEditMeshSize::doit( %this )
{
   if ( ShapeEditor.shape.setMeshSize( %this.meshName SPC %this.oldSize, %this.newSize ) )
   {
      ShapeEdPropWindow.update_onMeshSizeChanged( %this.meshName, %this.oldSize, %this.newSize );
      return true;
   }
   return false;
}

function ActionEditMeshSize::undo( %this )
{
   Parent::undo( %this );
   if ( ShapeEditor.shape.setMeshSize( %this.meshName SPC %this.newSize, %this.oldSize ) )
      ShapeEdPropWindow.update_onMeshSizeChanged( %this.meshName, %this.oldSize, %this.oldSize );
}

//------------------------------------------------------------------------------
// Edit billboard type
function ShapeEditor::doEditMeshBillboard( %this, %meshName, %type )
{
   %action = %this.createAction( ActionEditMeshBillboard, "Edit mesh billboard" );
   %action.meshName = %meshName;
   %action.oldType = %this.shape.getMeshType( %meshName );
   %action.newType = %type;

   %this.doAction( %action );
}

function ActionEditMeshBillboard::doit( %this )
{
   if ( ShapeEditor.shape.setMeshType( %this.meshName, %this.newType ) )
   {
      switch$ ( ShapeEditor.shape.getMeshType( %this.meshName ) )
      {
         case "normal":          ShapeEdDetails-->bbType.setSelected( 0, false );
         case "billboard":       ShapeEdDetails-->bbType.setSelected( 1, false );
         case "billboardzaxis":  ShapeEdDetails-->bbType.setSelected( 2, false );
      }
      return true;
   }
   return false;
}

function ActionEditMeshBillboard::undo( %this )
{
   Parent::undo( %this );
   if ( ShapeEditor.shape.setMeshType( %this.meshName, %this.oldType ) )
   {
      %id = ShapeEdDetailTree.getSelectedItem();
      if ( ( %id > 1 ) && ( ShapeEdDetailTree.getItemText( %id ) $= %this.meshName ) )
      {
         switch$ ( ShapeEditor.shape.getMeshType( %this.meshName ) )
         {
            case "normal":          ShapeEdDetails-->bbType.setSelected( 0, false );
            case "billboard":       ShapeEdDetails-->bbType.setSelected( 1, false );
            case "billboardzaxis":  ShapeEdDetails-->bbType.setSelected( 2, false );
         }
      }
   }
}

//------------------------------------------------------------------------------
// Edit object node
function ShapeEditor::doSetObjectNode( %this, %objName, %node )
{
   %action = %this.createAction( ActionSetObjectNode, "Set object node" );
   %action.objName = %objName;
   %action.oldNode = %this.shape.getObjectNode( %objName );
   %action.newNode = %node;

   %this.doAction( %action );
}

function ActionSetObjectNode::doit( %this )
{
   if ( ShapeEditor.shape.setObjectNode( %this.objName, %this.newNode ) )
   {
      ShapeEdPropWindow.update_onObjectNodeChanged( %this.objName );
      return true;
   }
   return false;
}

function ActionSetObjectNode::undo( %this )
{
   Parent::undo( %this );
   if ( ShapeEditor.shape.setObjectNode( %this.objName, %this.oldNode ) )
      ShapeEdPropWindow.update_onObjectNodeChanged( %this.objName );
}

//------------------------------------------------------------------------------
// Remove mesh
function ShapeEditor::doRemoveMesh( %this, %meshName )
{
   %action = %this.createAction( ActionRemoveMesh, "Remove mesh" );
   %action.meshName = %meshName;

   %this.doAction( %action );
}

function ActionRemoveMesh::doit( %this )
{
   if ( ShapeEditor.shape.removeMesh( %this.meshName ) )
   {
      ShapeEdPropWindow.update_onMeshRemoved( %this.meshName );
      return true;
   }
   return false;
}

function ActionRemoveMesh::undo( %this )
{
   Parent::undo( %this );
}

//------------------------------------------------------------------------------
// Add meshes from file
function ShapeEditor::doAddMeshFromFile( %this, %filename, %size )
{
   %action = %this.createAction( ActionAddMeshFromFile, "Add mesh from file" );
   %action.filename = %filename;
   %action.size = %size;

   %this.doAction( %action );
}

function ActionAddMeshFromFile::doit( %this )
{
   %this.meshList = ShapeEditor.addLODFromFile( ShapeEditor.shape, %this.filename, %this.size, 1 );
   if ( %this.meshList !$= "" )
   {
      %count = getFieldCount( %this.meshList );
      for ( %i = 0; %i < %count; %i++ )
         ShapeEdPropWindow.update_onMeshAdded( getField( %this.meshList, %i ) );

      ShapeEdMaterials.updateMaterialList();

      return true;
   }
   return false;
}

function ActionAddMeshFromFile::undo( %this )
{
   // Remove all the meshes we added
   %count = getFieldCount( %this.meshList );
   for ( %i = 0; %i < %count; %i ++ )
   {
      %name = getField( %this.meshList, %i );
      ShapeEditor.shape.removeMesh( %name );
      ShapeEdPropWindow.update_onMeshRemoved( %name );
   }
   ShapeEdMaterials.updateMaterialList();
}

//------------------------------------------------------------------------------
// Add/edit collision geometry
function ShapeEditor::doEditCollision( %this, %type, %target, %depth, %merge, %concavity,
                                       %maxVerts, %boxMax, %sphereMax, %capsuleMax )
{
   %colData = ShapeEdColWindow.lastColSettings;

   %action = %this.createAction( ActionEditCollision, "Edit shape collision" );

   %action.oldType = getField( %colData, 0 );
   %action.oldTarget = getField( %colData, 1 );
   %action.oldDepth = getField( %colData, 2 );
   %action.oldMerge = getField( %colData, 3 );
   %action.oldConcavity = getField( %colData, 4 );
   %action.oldMaxVerts = getField( %colData, 5 );
   %action.oldBoxMax = getField( %colData, 6 );
   %action.oldSphereMax = getField( %colData, 7 );
   %action.oldCapsuleMax = getField( %colData, 8 );

   %action.newType = %type;
   %action.newTarget = %target;
   %action.newDepth = %depth;
   %action.newMerge = %merge;
   %action.newConcavity = %concavity;
   %action.newMaxVerts = %maxVerts;
   %action.newBoxMax = %boxMax;
   %action.newSphereMax = %sphereMax;
   %action.newCapsuleMax = %capsuleMax;

   %this.doAction( %action );
}

function ActionEditCollision::updateCollision( %this, %type, %target, %depth, %merge, %concavity,
                                               %maxVerts, %boxMax, %sphereMax, %capsuleMax )
{
   %colDetailSize = -1;
   %colNode = "Col" @ %colDetailSize;

   // TreeView items are case sensitive, but TSShape names are not, so fixup case
   // if needed
   %index = ShapeEditor.shape.getNodeIndex( %colNode );
   if ( %index != -1 )
      %colNode = ShapeEditor.shape.getNodeName( %index );

   // First remove the old detail and collision nodes
   %meshList = ShapeEditor.getDetailMeshList( %colDetailSize );
   %meshCount = getFieldCount( %meshList );
   if ( %meshCount > 0 )
   {
      ShapeEditor.shape.removeDetailLevel( %colDetailSize );
      for ( %i = 0; %i < %meshCount; %i++ )
         ShapeEdPropWindow.update_onMeshRemoved( getField( %meshList, %i ) );
   }

   %nodeList = ShapeEditor.getNodeNames( %colNode, "" );
   %nodeCount = getFieldCount( %nodeList );
   if ( %nodeCount > 0 )
   {
      for ( %i = 0; %i < %nodeCount; %i++ )
         ShapeEditor.shape.removeNode( getField( %nodeList, %i ) );
      ShapeEdPropWindow.update_onNodeRemoved( %nodeList, %nodeCount );
   }

   // Add the new node and geometry
   if ( %type $= "" )
      return;

   if ( !ShapeEditor.shape.addCollisionDetail( %colDetailSize, %type, %target,
                                                %depth, %merge, %concavity, %maxVerts,
                                                %boxMax, %sphereMax, %capsuleMax ) )
      return false;

   // Update UI
   %meshList = ShapeEditor.getDetailMeshList( %colDetailSize );
   ShapeEdPropWindow.update_onNodeAdded( %colNode, ShapeEditor.shape.getNodeCount() );    // will also add child nodes
   %count = getFieldCount( %meshList );
   for ( %i = 0; %i < %count; %i++ )
      ShapeEdPropWindow.update_onMeshAdded( getField( %meshList, %i ) );

   ShapeEdColWindow.lastColSettings = %type TAB %target TAB %depth TAB %merge TAB
      %concavity TAB %maxVerts TAB %boxMax TAB %sphereMax TAB %capsuleMax;
   ShapeEdColWindow.update_onCollisionChanged();

   return true;
}

function ActionEditCollision::doit( %this )
{
   ShapeEdWaitGui.show( "Generating collision geometry..." );
   %success = %this.updateCollision( %this.newType, %this.newTarget, %this.newDepth, %this.newMerge,
                                     %this.newConcavity, %this.newMaxVerts, %this.newBoxMax,
                                     %this.newSphereMax, %this.newCapsuleMax );
   ShapeEdWaitGui.hide();

   return %success;
}

function ActionEditCollision::undo( %this )
{
   Parent::undo( %this );

   ShapeEdWaitGui.show( "Generating collision geometry..." );
   %this.updateCollision( %this.oldType, %this.oldTarget, %this.oldDepth, %this.oldMerge,
                          %this.oldConcavity, %this.oldMaxVerts, %this.oldBoxMax,
                          %this.oldSphereMax, %this.oldCapsuleMax );
   ShapeEdWaitGui.hide();
}

//------------------------------------------------------------------------------
// Remove Detail

function ShapeEditor::doRemoveDetail( %this, %size )
{
   %action = %this.createAction( ActionRemoveDetail, "Remove detail level" );
   %action.size = %size;

   %this.doAction( %action );
}

function ActionRemoveDetail::doit( %this )
{
   %meshList = ShapeEditor.getDetailMeshList( %this.size );
   if ( ShapeEditor.shape.removeDetailLevel( %this.size ) )
   {
      %meshCount = getFieldCount( %meshList );
      for ( %i = 0; %i < %meshCount; %i++ )
         ShapeEdPropWindow.update_onMeshRemoved( getField( %meshList, %i ) );
      return true;
   }
   return false;
}

function ActionRemoveDetail::undo( %this )
{
   Parent::undo( %this );
}

//------------------------------------------------------------------------------
// Update bounds
function ShapeEditor::doSetBounds( %this )
{
   %action = %this.createAction( ActionSetBounds, "Set bounds" );
   %action.oldBounds = ShapeEditor.shape.getBounds();
   %action.newBounds = ShapeEdShapeView.computeShapeBounds();

   %this.doAction( %action );
}

function ActionSetBounds::doit( %this )
{
   return ShapeEditor.shape.setBounds( %this.newBounds );
}

function ActionSetBounds::undo( %this )
{
   Parent::undo( %this );

   ShapeEditor.shape.setBounds( %this.oldBounds );
}

//------------------------------------------------------------------------------
// Add/edit imposter
function ShapeEditor::doEditImposter( %this, %dl, %detailSize, %bbEquatorSteps, %bbPolarSteps,
                                       %bbDetailLevel, %bbDimension, %bbIncludePoles, %bbPolarAngle )
{
   %action = %this.createAction( ActionEditImposter, "Edit imposter" );
   %action.oldDL = %dl;
   if ( %action.oldDL != -1 )
   {
      %action.oldSize = ShapeEditor.shape.getDetailLevelSize( %dl );
      %action.oldImposter = ShapeEditor.shape.getImposterSettings( %dl );
   }
   %action.newSize = %detailSize;
   %action.newImposter = "1" TAB %bbEquatorSteps TAB %bbPolarSteps TAB %bbDetailLevel TAB
                           %bbDimension TAB %bbIncludePoles TAB %bbPolarAngle;

   %this.doAction( %action );
}

function ActionEditImposter::doit( %this )
{
   // Unpack new imposter settings
   for ( %i = 0; %i < 7; %i++ )
      %val[%i] = getField( %this.newImposter, %i );
  
   ShapeEdWaitGui.show( "Generating imposter bitmaps..." );

   // Need to de-highlight the current material, or the imposter will have the
   // highlight effect baked in!
   ShapeEdMaterials.updateSelectedMaterial( false );

   %dl = ShapeEditor.shape.addImposter( %this.newSize, %val[1], %val[2], %val[3], %val[4], %val[5], %val[6] );
   ShapeEdWaitGui.hide();

   // Restore highlight effect
   ShapeEdMaterials.updateSelectedMaterial( ShapeEdMaterials-->highlightMaterial.getValue() );

   if ( %dl != -1 )
   {
      ShapeEdShapeView.refreshShape();
      ShapeEdShapeView.currentDL = %dl;
      ShapeEdAdvancedWindow-->detailSize.setText( %this.newSize );
      ShapeEdDetails-->meshSize.setText( %this.newSize );
      ShapeEdDetails.update_onDetailsChanged();

      return true;
   }
   return false;
}

function ActionEditImposter::undo( %this )
{
   Parent::undo( %this );

   // If this was a new imposter, just remove it. Otherwise restore the old settings
   if ( %this.oldDL < 0 )
   {
      if ( ShapeEditor.shape.removeImposter() )
      {
         ShapeEdShapeView.refreshShape();
         ShapeEdShapeView.currentDL = 0;
         ShapeEdDetails.update_onDetailsChanged();
      }
   }
   else
   {
      // Unpack old imposter settings
      for ( %i = 0; %i < 7; %i++ )
         %val[%i] = getField( %this.oldImposter, %i );

      ShapeEdWaitGui.show( "Generating imposter bitmaps..." );

      // Need to de-highlight the current material, or the imposter will have the
      // highlight effect baked in!
      ShapeEdMaterials.updateSelectedMaterial( false );

      %dl = ShapeEditor.shape.addImposter( %this.oldSize, %val[1], %val[2], %val[3], %val[4], %val[5], %val[6] );
      ShapeEdWaitGui.hide();

      // Restore highlight effect
      ShapeEdMaterials.updateSelectedMaterial( ShapeEdMaterials-->highlightMaterial.getValue() );

      if ( %dl != -1 )
      {
         ShapeEdShapeView.refreshShape();
         ShapeEdShapeView.currentDL = %dl;
         ShapeEdAdvancedWindow-->detailSize.setText( %this.oldSize );
         ShapeEdDetails-->meshSize.setText( %this.oldSize );
      }
   }
}

//------------------------------------------------------------------------------
// Remove imposter
function ShapeEditor::doRemoveImposter( %this )
{
   %action = %this.createAction( ActionRemoveImposter, "Remove imposter" );
   %dl = ShapeEditor.shape.getImposterDetailLevel();
   if ( %dl != -1 )
   {
      %action.oldSize = ShapeEditor.shape.getDetailLevelSize( %dl );
      %action.oldImposter = ShapeEditor.shape.getImposterSettings( %dl );
      %this.doAction( %action );
   }
}

function ActionRemoveImposter::doit( %this )
{
   if ( ShapeEditor.shape.removeImposter() )
   {
      ShapeEdShapeView.refreshShape();
      ShapeEdShapeView.currentDL = 0;
      ShapeEdDetails.update_onDetailsChanged();

      return true;
   }
   return false;
}

function ActionRemoveImposter::undo( %this )
{
   Parent::undo( %this );

   // Unpack the old imposter settings
   for ( %i = 0; %i < 7; %i++ )
      %val[%i] = getField( %this.oldImposter, %i );

   ShapeEdWaitGui.show( "Generating imposter bitmaps..." );
   %dl = ShapeEditor.shape.addImposter( %this.oldSize, %val[1], %val[2], %val[3], %val[4], %val[5], %val[6] );
   ShapeEdWaitGui.hide();

   if ( %dl != -1 )
   {
      ShapeEdShapeView.refreshShape();
      ShapeEdShapeView.currentDL = %dl;
      ShapeEdAdvancedWindow-->detailSize.setText( %this.oldSize );
      ShapeEdDetails-->meshSize.setText( %this.oldSize );
      ShapeEdDetails.update_onDetailsChanged();
   }
}
