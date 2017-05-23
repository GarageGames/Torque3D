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

function MaterialEditorGui::createUndo(%this, %class, %desc)
{
   pushInstantGroup();
   %action = new UndoScriptAction()
   {
      class = %class;
      superClass = BaseMaterialEdAction;
      actionName = %desc;
   };
   popInstantGroup();
   return %action;
}

function MaterialEditorGui::submitUndo(%this, %action)
{
   if(!%this.preventUndo)
      %action.addToManager(Editor.getUndoManager());
}

function BaseMaterialEdAction::redo(%this)
{
   %this.redo();
}

function BaseMaterialEdAction::undo(%this)
{
}

// Generic updateActiveMaterial redo/undo

function ActionUpdateActiveMaterial::redo(%this)
{
   if( MaterialEditorPreviewWindow.isVisible() && MaterialEditorGui.currentMaterial == %this.material )
   {
      /*
      if( MaterialEditorGui.currentMaterial != %this.material )
      {
         MaterialEditorGui.currentObject = %this.object;
         MaterialEditorGui.setMode();
         MaterialEditorGui.setActiveMaterial(%this.material);
      }
      */
      eval("materialEd_previewMaterial." @ %this.field @ " = " @ %this.newValue @ ";");
      materialEd_previewMaterial.flush();
      materialEd_previewMaterial.reload();
      
      if (MaterialEditorGui.livePreview == true)
      {
         eval("%this.material." @ %this.field @ " = " @ %this.newValue @ ";");
         MaterialEditorGui.currentMaterial.flush();
         MaterialEditorGui.currentMaterial.reload();
      }      
      
      MaterialEditorGui.preventUndo = true;
      MaterialEditorGui.guiSync( materialEd_previewMaterial );
      MaterialEditorGui.setMaterialDirty();
      MaterialEditorGui.preventUndo = false;
   }
   else
   {
      eval("%this.material." @ %this.field @ " = " @ %this.newValue @ ";");
      %this.material.flush();
      %this.material.reload();
   }
}

function ActionUpdateActiveMaterial::undo(%this)
{
   MaterialEditorGui.preventUndo = true;
   if( MaterialEditorPreviewWindow.isVisible() && MaterialEditorGui.currentMaterial == %this.material )
   {
      /*
      if( MaterialEditorGui.currentMaterial != %this.material )
      {
         MaterialEditorGui.currentObject = %this.object;
         MaterialEditorGui.setMode();
         MaterialEditorGui.setActiveMaterial(%this.material);
      }
      */
      
      eval("materialEd_previewMaterial." @ %this.field @ " = " @ %this.oldValue @ ";");
      materialEd_previewMaterial.flush();
      materialEd_previewMaterial.reload();
      
      if (MaterialEditorGui.livePreview == true)
      {
         eval("%this.material." @ %this.field @ " = " @ %this.oldValue @ ";");
         MaterialEditorGui.currentMaterial.flush();
         MaterialEditorGui.currentMaterial.reload();
      }
      MaterialEditorGui.guiSync( materialEd_previewMaterial );
      MaterialEditorGui.setMaterialDirty();
   }
   else
   {
      eval("%this.material." @ %this.field @ " = " @ %this.oldValue @ ";");
      %this.material.flush();
      %this.material.reload();
   }
   
   MaterialEditorGui.preventUndo = false;
}

// Special case updateActiveMaterial redo/undo

function ActionUpdateActiveMaterialAnimationFlags::redo(%this)
{
   if( MaterialEditorPreviewWindow.isVisible() && MaterialEditorGui.currentMaterial == %this.material )
   {
      /*
      if( MaterialEditorGui.currentMaterial != %this.material )
      {
         MaterialEditorGui.currentObject = %this.object;
         MaterialEditorGui.setMode();
         MaterialEditorGui.setActiveMaterial(%this.material);
      }
      */
      
      eval("materialEd_previewMaterial.animFlags[" @ %this.layer @ "] = " @ %this.newValue @ ";");
      materialEd_previewMaterial.flush();
      materialEd_previewMaterial.reload();
      
      if (MaterialEditorGui.livePreview == true)
      {
         eval("%this.material.animFlags[" @ %this.layer @ "] = " @ %this.newValue @ ";");
         MaterialEditorGui.currentMaterial.flush();
         MaterialEditorGui.currentMaterial.reload();
      }
      MaterialEditorGui.guiSync( materialEd_previewMaterial );
      MaterialEditorGui.setMaterialDirty();      
   }
   else
   {
      eval("%this.material.animFlags[" @ %this.layer @ "] = " @ %this.newValue @ ";");
      %this.material.flush();
      %this.material.reload();
   }
}

function ActionUpdateActiveMaterialAnimationFlags::undo(%this)
{
   if( MaterialEditorPreviewWindow.isVisible() && MaterialEditorGui.currentMaterial == %this.material )
   {      
      eval("materialEd_previewMaterial.animFlags[" @ %this.layer @ "] = " @ %this.oldValue @ ";");
      materialEd_previewMaterial.flush();
      materialEd_previewMaterial.reload();
      
      if (MaterialEditorGui.livePreview == true)
      {
         eval("%this.material.animFlags[" @ %this.layer @ "] = " @ %this.oldValue @ ";");
         MaterialEditorGui.currentMaterial.flush();
         MaterialEditorGui.currentMaterial.reload();
      }
      MaterialEditorGui.guiSync( materialEd_previewMaterial );
      MaterialEditorGui.setMaterialDirty();      
   }
   else
   {
      eval("%this.material.animFlags[" @ %this.layer @ "] = " @ %this.oldValue @ ";");
      %this.material.flush();
      %this.material.reload();
   }
}

function ActionUpdateActiveMaterialName::redo(%this)
{
   %this.material.setName(%this.newName);
   MaterialEditorGui.updateMaterialReferences( MissionGroup, %this.oldName, %this.newName );

   if( MaterialEditorPreviewWindow.isVisible() && MaterialEditorGui.currentMaterial == %this.material )
   {
         MaterialEditorGui.guiSync( materialEd_previewMaterial );
         MaterialEditorGui.setMaterialDirty();
   }
}

function ActionUpdateActiveMaterialName::undo(%this)
{
   %this.material.setName(%this.oldName);
   MaterialEditorGui.updateMaterialReferences( MissionGroup, %this.newName, %this.oldName );

   if( MaterialEditorPreviewWindow.isVisible() && MaterialEditorGui.currentMaterial == %this.material )
   {
         MaterialEditorGui.guiSync( materialEd_previewMaterial );
         MaterialEditorGui.setMaterialDirty();
   }
}

function ActionRefreshMaterial::redo(%this)
{
   if( MaterialEditorPreviewWindow.isVisible() && MaterialEditorGui.currentMaterial == %this.material )
   {
      %this.material.setName( %this.newName );
     
      MaterialEditorGui.copyMaterials( %this.newMaterial, materialEd_previewMaterial );
      materialEd_previewMaterial.flush();
      materialEd_previewMaterial.reload();
      
      if (MaterialEditorGui.livePreview == true)
      {
         MaterialEditorGui.copyMaterials( %this.newMaterial , %this.material );
         %this.material.flush();
         %this.material.reload();
      }
      
      
      MaterialEditorGui.guiSync( materialEd_previewMaterial );
      MaterialEditorGui.setMaterialNotDirty();
   }
   else
   {
      MaterialEditorGui.copyMaterials( %this.newMaterial, %this.material );
      %this.material.flush();
      %this.material.reload();
   }
}

function ActionRefreshMaterial::undo(%this)
{
   if( MaterialEditorPreviewWindow.isVisible() && MaterialEditorGui.currentMaterial == %this.material )
   {
      %this.material.setName( %this.oldName );
      
      MaterialEditorGui.copyMaterials( %this.oldMaterial, materialEd_previewMaterial );
      materialEd_previewMaterial.flush();
      materialEd_previewMaterial.reload();
      
      if (MaterialEditorGui.livePreview == true)
      {
         MaterialEditorGui.copyMaterials( %this.oldMaterial, %this.material );
         %this.material.flush();
         %this.material.reload();
      }
            
      MaterialEditorGui.guiSync( materialEd_previewMaterial );
      MaterialEditorGui.setMaterialDirty();
   }
   else
   {
      MaterialEditorGui.copyMaterials( %this.oldMaterial, %this.material );
      %this.material.flush();
      %this.material.reload();
   }
}

function ActionClearMaterial::redo(%this)
{
   if( MaterialEditorPreviewWindow.isVisible() && MaterialEditorGui.currentMaterial == %this.material )
   {
      MaterialEditorGui.copyMaterials( %this.newMaterial, materialEd_previewMaterial );
      materialEd_previewMaterial.flush();
      materialEd_previewMaterial.reload();
      
      if (MaterialEditorGui.livePreview == true)
      {
         MaterialEditorGui.copyMaterials( %this.newMaterial, %this.material );
         %this.material.flush();
         %this.material.reload();
      }
      
      MaterialEditorGui.guiSync( materialEd_previewMaterial );
      MaterialEditorGui.setMaterialDirty();
   }
   else
   {
      MaterialEditorGui.copyMaterials( %this.newMaterial, %this.material );
      %this.material.flush();
      %this.material.reload();
   }
}

function ActionClearMaterial::undo(%this)
{
   if( MaterialEditorPreviewWindow.isVisible() && MaterialEditorGui.currentMaterial == %this.material )
   {
      MaterialEditorGui.copyMaterials( %this.oldMaterial, materialEd_previewMaterial );
      materialEd_previewMaterial.flush();
      materialEd_previewMaterial.reload();
      
      if (MaterialEditorGui.livePreview == true)
      {
         MaterialEditorGui.copyMaterials( %this.oldMaterial, %this.material );
         %this.material.flush();
         %this.material.reload();
      }
      
      MaterialEditorGui.guiSync( materialEd_previewMaterial );
      MaterialEditorGui.setMaterialDirty();
   }
   else
   {
      MaterialEditorGui.copyMaterials( %this.oldMaterial, %this.material );
      %this.material.flush();
      %this.material.reload();
   }
}

function ActionChangeMaterial::redo(%this)
{
   if( %this.mode $= "model" )
   {
      %this.object.changeMaterial( %this.materialTarget, %this.fromMaterial.getName(), %this.toMaterial.getName() );
      
      MaterialEditorGui.currentObject = %this.object;
   
      if( %this.toMaterial.getFilename() !$= "tools/gui/materialSelector.ed.gui" || 
            %this.toMaterial.getFilename() !$= "tools/materialEditor/scripts/materialEditor.ed.cs")
      {
         matEd_PersistMan.removeObjectFromFile(%this.toMaterial);
      }
      
      matEd_PersistMan.setDirty(%this.fromMaterial);
      matEd_PersistMan.setDirty(%this.toMaterial, %this.toMaterialNewFname);
      matEd_PersistMan.saveDirty();
      
      matEd_PersistMan.removeDirty(%this.fromMaterial);
      matEd_PersistMan.removeDirty(%this.toMaterial);
   }
   else
   {
      eval("%this.object." @ %this.materialTarget @ " = " @ %this.toMaterial.getName() @ ";");
      MaterialEditorGui.currentObject.postApply();
   }
   
   if( MaterialEditorPreviewWindow.isVisible() )
      MaterialEditorGui.setActiveMaterial( %this.toMaterial );
}

function ActionChangeMaterial::undo(%this)
{
   if( %this.mode $= "model" )
   {
      %this.object.changeMaterial( %this.materialTarget, %this.toMaterial.getName(), %this.fromMaterial.getName() );
      
      MaterialEditorGui.currentObject = %this.object;    
         
      if( %this.toMaterial.getFilename() !$= "tools/gui/materialSelector.ed.gui" || 
            %this.toMaterial.getFilename() !$= "tools/materialEditor/scripts/materialEditor.ed.cs")
      {
         matEd_PersistMan.removeObjectFromFile(%this.toMaterial);
      }
      
      
      matEd_PersistMan.setDirty(%this.fromMaterial);
      matEd_PersistMan.setDirty(%this.toMaterial, %this.toMaterialOldFname);
      matEd_PersistMan.saveDirty();
      
      matEd_PersistMan.removeDirty(%this.fromMaterial);
      matEd_PersistMan.removeDirty(%this.toMaterial);
   }
   else
   {
      eval("%this.object." @ %this.materialTarget @ " = " @ %this.fromMaterial.getName() @ ";");
      MaterialEditorGui.currentObject.postApply();
   }
   
   if( MaterialEditorPreviewWindow.isVisible() )
      MaterialEditorGui.setActiveMaterial( %this.fromMaterial );
}

function ActionCreateNewMaterial::redo(%this)
{
   if( MaterialEditorPreviewWindow.isVisible() )
   {
      if( MaterialEditorGui.currentMaterial != %this.newMaterial )
      {
         MaterialEditorGui.currentObject = "";
         MaterialEditorGui.setMode();
         MaterialEditorGui.setActiveMaterial(%this.newMaterial);
      }
      
      MaterialEditorGui.copyMaterials( %this.newMaterial, materialEd_previewMaterial );
      materialEd_previewMaterial.flush();
      materialEd_previewMaterial.reload();
      
      MaterialEditorGui.guiSync( materialEd_previewMaterial );
   }
   
   %idx = UnlistedMaterials.getIndexFromValue( %this.newMaterial.getName() );
   UnlistedMaterials.erase( %idx );
}

function ActionCreateNewMaterial::undo(%this)
{
   if( MaterialEditorPreviewWindow.isVisible() )
   {
      if( MaterialEditorGui.currentMaterial != %this.oldMaterial )
      {
         MaterialEditorGui.currentObject = "";
         MaterialEditorGui.setMode();
         MaterialEditorGui.setActiveMaterial(%this.oldMaterial);
      }
      
      MaterialEditorGui.copyMaterials( %this.oldMaterial, materialEd_previewMaterial );
      materialEd_previewMaterial.flush();
      materialEd_previewMaterial.reload();
      
      MaterialEditorGui.guiSync( materialEd_previewMaterial );
   }
   
   UnlistedMaterials.add( "unlistedMaterials", %this.newMaterial.getName() );
}

function ActionDeleteMaterial::redo(%this)
{
   if( MaterialEditorPreviewWindow.isVisible() )
   {
      if( MaterialEditorGui.currentMaterial != %this.newMaterial )
      {
         MaterialEditorGui.currentObject = "";
         MaterialEditorGui.setMode();
         MaterialEditorGui.setActiveMaterial(%this.newMaterial);
      }
      
      MaterialEditorGui.copyMaterials( %this.newMaterial, materialEd_previewMaterial );
      materialEd_previewMaterial.flush();
      materialEd_previewMaterial.reload();
      
      MaterialEditorGui.guiSync( materialEd_previewMaterial );
   }
   
   if( %this.oldMaterial.getFilename() !$= "tools/gui/materialSelector.ed.gui" || 
         %this.oldMaterial.getFilename() !$= "tools/materialEditor/scripts/materialEditor.ed.cs")
   {
      matEd_PersistMan.removeObjectFromFile(%this.oldMaterial);
   }
      
   UnlistedMaterials.add( "unlistedMaterials", %this.oldMaterial.getName() );
}

function ActionDeleteMaterial::undo(%this)
{
   if( MaterialEditorPreviewWindow.isVisible() )
   {
      if( MaterialEditorGui.currentMaterial != %this.oldMaterial )
      {
         MaterialEditorGui.currentObject = "";
         MaterialEditorGui.setMode();
         MaterialEditorGui.setActiveMaterial(%this.oldMaterial);
      }
      
      MaterialEditorGui.copyMaterials( %this.oldMaterial, materialEd_previewMaterial );
      materialEd_previewMaterial.flush();
      materialEd_previewMaterial.reload();
      
      MaterialEditorGui.guiSync( materialEd_previewMaterial );
   }
   
   matEd_PersistMan.setDirty(%this.oldMaterial, %this.oldMaterialFname);
   matEd_PersistMan.saveDirty();
   matEd_PersistMan.removeDirty(%this.oldMaterial);
   
   %idx = UnlistedMaterials.getIndexFromValue( %this.oldMaterial.getName() );
   UnlistedMaterials.erase( %idx );
}