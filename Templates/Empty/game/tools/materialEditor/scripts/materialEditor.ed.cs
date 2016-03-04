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

// Material Editor originally created by Dave Calabrese and Travis Vroman of Gaslight Studios

function MaterialEditorGui::establishMaterials(%this)
{
   //Cubemap used to preview other cubemaps in the editor.
   singleton CubemapData( matEdCubeMapPreviewMat )
   {
      cubeFace[0] = "tools/materialeditor/gui/cube_xNeg";
      cubeFace[1] = "tools/materialeditor/gui/cube_xPos";
      cubeFace[2] = "tools/materialeditor/gui/cube_ZNeg";
      cubeFace[3] = "tools/materialeditor/gui/cube_ZPos";
      cubeFace[4] = "tools/materialeditor/gui/cube_YNeg";
      cubeFace[5] = "tools/materialeditor/gui/cube_YPos";
      parentGroup = "RootGroup";
   };
   
   //Material used to preview other materials in the editor.
   singleton Material(materialEd_previewMaterial)
   {
      mapTo = "matEd_mappedMat";
      diffuseMap[0] = "tools/materialeditor/gui/matEd_mappedMat";
   };

   singleton CustomMaterial( materialEd_justAlphaMaterial )
   {
      mapTo = "matEd_mappedMatB";
      texture[0] = materialEd_previewMaterial.diffuseMap[0];
   };

   //Custom shader to allow the display of just the alpha channel. 
   singleton ShaderData( materialEd_justAlphaShader )
   {
      DXVertexShaderFile 	= "shaders/alphaOnlyV.hlsl";
      DXPixelShaderFile 	= "shaders/alphaOnlyP.hlsl";
      pixVersion = 1.0;
   };
}

function MaterialEditorGui::open(%this)
{
   MaterialEditorGui.establishMaterials();

	// We hide these specific windows here due too there non-modal nature.
	// These guis are also pushed onto Canvas, which means they shouldn't be parented
	// by editorgui
	materialSelector.setVisible(0);
	matEdSaveDialog.setVisible(0);
	
	MaterialEditorPropertiesWindow-->matEd_cubemapEditBtn.setVisible(0);
	
	//Setup our dropdown menu contents.
   //Blending Modes
   MaterialEditorPropertiesWindow-->blendingTypePopUp.clear();
   MaterialEditorPropertiesWindow-->blendingTypePopUp.add(None,0);
   MaterialEditorPropertiesWindow-->blendingTypePopUp.add(Mul,1);
   MaterialEditorPropertiesWindow-->blendingTypePopUp.add(Add,2);
   MaterialEditorPropertiesWindow-->blendingTypePopUp.add(AddAlpha,3);
   MaterialEditorPropertiesWindow-->blendingTypePopUp.add(Sub,4);
   MaterialEditorPropertiesWindow-->blendingTypePopUp.add(LerpAlpha,5);
   MaterialEditorPropertiesWindow-->blendingTypePopUp.setSelected( 0, false );

   //Reflection Types
   MaterialEditorPropertiesWindow-->reflectionTypePopUp.clear();
   MaterialEditorPropertiesWindow-->reflectionTypePopUp.add("None",0);	
   MaterialEditorPropertiesWindow-->reflectionTypePopUp.add("cubemap",1);	
   MaterialEditorPropertiesWindow-->reflectionTypePopUp.setSelected( 0, false );

   //Sounds
   MaterialEditorPropertiesWindow-->footstepSoundPopup.clear();
   MaterialEditorPropertiesWindow-->impactSoundPopup.clear();

   %sounds = "<None>" TAB "<Soft>" TAB "<Hard>" TAB "<Metal>" TAB "<Snow>";    // Default sounds

   // Get custom sound datablocks
   foreach (%db in DataBlockSet)
   {
      if (%db.isMemberOfClass("SFXTrack"))
         %sounds = %sounds TAB %db.getName();
   }

   %count = getFieldCount(%sounds);
   for (%i = 0; %i < %count; %i++)
   {
      %name = getField(%sounds, %i);
      MaterialEditorPropertiesWindow-->footstepSoundPopup.add(%name);
      MaterialEditorPropertiesWindow-->impactSoundPopup.add(%name);
   }

   //Preview Models
   matEd_quickPreview_Popup.clear();
   matEd_quickPreview_Popup.add("Cube",0);
   matEd_quickPreview_Popup.add("Sphere",1);
   matEd_quickPreview_Popup.add("Pyramid",2);
   matEd_quickPreview_Popup.add("Cylinder",3);
   matEd_quickPreview_Popup.add("Torus",4);
   matEd_quickPreview_Popup.add("Knot",5);  
   matEd_quickPreview_Popup.setSelected( 0, false );
   matEd_quickPreview_Popup.selected = matEd_quickPreview_Popup.getText();
   
   MaterialEditorPropertiesWindow-->MaterialLayerCtrl.clear();		
   MaterialEditorPropertiesWindow-->MaterialLayerCtrl.add("Layer 0",0);		
   MaterialEditorPropertiesWindow-->MaterialLayerCtrl.add("Layer 1",1);		
   MaterialEditorPropertiesWindow-->MaterialLayerCtrl.add("Layer 2",2);		
   MaterialEditorPropertiesWindow-->MaterialLayerCtrl.add("Layer 3",3);	
   MaterialEditorPropertiesWindow-->MaterialLayerCtrl.setSelected( 0, false );
   
   //Sift through the RootGroup and find all loaded material items.
   MaterialEditorGui.updateAllFields();
   MaterialEditorGui.updatePreviewObject();

   // If no selected object; go to material mode. And edit the last selected material
   MaterialEditorGui.setMode();
   
   MaterialEditorGui.preventUndo = true;   
   
   if( MaterialEditorGui.currentMode $= "Mesh" )
      MaterialEditorGui.prepareActiveObject( true );
   else
      MaterialEditorGui.prepareActiveMaterial( "", true );
      
   MaterialEditorGui.preventUndo = false;   
      
}

function MaterialEditorGui::quit(%this)
{
   // if we quit, restore with notDirty
   if(MaterialEditorGui.materialDirty)
   {
      //keep on doing this
      MaterialEditorGui.copyMaterials( notDirtyMaterial, materialEd_previewMaterial );
      MaterialEditorGui.copyMaterials( notDirtyMaterial, MaterialEditorGui.currentMaterial );
      MaterialEditorGui.guiSync( materialEd_previewMaterial );
      
      materialEd_previewMaterial.flush();
      materialEd_previewMaterial.reload();
      MaterialEditorGui.currentMaterial.flush();
      MaterialEditorGui.currentMaterial.reload();
   }
   
   if( isObject(MaterialEditorGui.currentMaterial) )
   {
      MaterialEditorGui.lastMaterial = MaterialEditorGui.currentMaterial.getName();
   }

   MaterialEditorGui.setMaterialNotDirty();
   
   // First delete the model so that it releases
   // material instances that use the preview materials.
   matEd_previewObjectView.deleteModel();
   
   // Now we can delete the preview materials and shaders
   // knowing that there are no matinstances using them.
   matEdCubeMapPreviewMat.delete();
   materialEd_previewMaterial.delete();
   materialEd_justAlphaMaterial.delete();
   materialEd_justAlphaShader.delete();
}

function MaterialEditorGui::openFile( %this, %fileType )
{
	switch$(%fileType)
	{
		case "Texture":         %filters = MaterialEditorGui.textureFormats;
		
		                if(MaterialEditorGui.lastTextureFile $= "")
		                  %defaultFileName = "*.*";
                      else
							   %defaultFileName = MaterialEditorGui.lastTextureFile;
							   
							     %defaultPath = MaterialEditorGui.lastTexturePath;
							     
      case "Model":           %filters = MaterialEditorGui.modelFormats;
							 %defaultFileName = "*.dts";
							     %defaultPath = MaterialEditorGui.lastModelPath;
	}
	
      %dlg = new OpenFileDialog()
      {
         Filters        = %filters;
         DefaultPath    = %defaultPath;
         DefaultFile    = %defaultFileName;
         ChangePath     = false;
         MustExist      = true;
      };
            
      %ret = %dlg.Execute();
      if(%ret)
      {
         switch$(%fileType)
	      {
	         case "Texture":
               MaterialEditorGui.lastTexturePath = filePath( %dlg.FileName );
               MaterialEditorGui.lastTextureFile = %filename = %dlg.FileName;
               
            case "Model":
               MaterialEditorGui.lastModelPath = filePath( %dlg.FileName );
               MaterialEditorGui.lastModelFile = %filename = %dlg.FileName;
	      }
      }
      
      %dlg.delete();
      
      if(!%ret)
         return;
		else
		   return makeRelativePath( %filename, getMainDotCsDir() );
}

//==============================================================================
// SubMaterial(Material Target) -- Supports different ways to grab the 
// material from the dropdown list. We're here either because-
// 1. We have switched over from another editor with an object locked in the 
//    $Tools::materialEditorList variable
// 2. We have selected an object using the Object Editor via the Material Editor 

function SubMaterialSelector::onSelect( %this )
{
   %material = "";   
   
   if( MaterialEditorGui.currentMeshMode $= "Model" )
      %material = getMapEntry( %this.getText() );
   else
      %material = MaterialEditorGui.currentObject.getFieldValue( %this.getText() );
   
   %origMat = %material;   
   if(%material$="")   
        %origMat = %material = %this.getText();
   // if there is no material attached to that objects material field or the 
   // object does not have a valid method to grab a material
   if( !isObject( %material ) )
   {
      // look for a newMaterial name to grab
      // addiitonally, convert "." to "_" in case we have something like: "base.texname" as a material name   
      // at the end we will have generated material name: "base_texname_mat"   
      %material = getUniqueName( strreplace(%material, ".", "_") @ "_mat" );

      new Material(%material) 
      {
         diffuseMap[0] = %origMat;   
         mapTo = %origMat;
         parentGroup = RootGroup;
      };
      
      eval( "MaterialEditorGui.currentObject." @ strreplace(%this.getText(),".","_") @ " = " @ %material @ ";");
      
      if( MaterialEditorGui.currentObject.isMethod("postApply") )
         MaterialEditorGui.currentObject.postApply();
   }

   MaterialEditorGui.prepareActiveMaterial( %material.getId() );
}

//==============================================================================
// Select object logic (deciding material/target mode)

function MaterialEditorGui::setMode( %this )
{
   MatEdMaterialMode.setVisible(0);
   MatEdTargetMode.setVisible(0);
   
   if( isObject(MaterialEditorGui.currentObject) )
   {
      MaterialEditorGui.currentMode = "Mesh";
      MatEdTargetMode.setVisible(1);
   }
   else
   {
      MaterialEditorGui.currentMode = "Material";
      MatEdMaterialMode.setVisible(1);
      EWorldEditor.clearSelection();
   }
}

function MaterialEditorGui::prepareActiveObject( %this, %override )
{
   %obj = $Tools::materialEditorList;
   if( MaterialEditorGui.currentObject == %obj && !%override)
      return;
   
   // TSStatics and ShapeBase objects should have getModelFile methods
   if( %obj.isMethod( "getModelFile" ) )
   {
      MaterialEditorGui.currentObject = %obj;
      
      SubMaterialSelector.clear();
      MaterialEditorGui.currentMeshMode = "Model";
      
      MaterialEditorGui.setMode();
      
      for(%j = 0; %j < MaterialEditorGui.currentObject.getTargetCount(); %j++)
      {
         %target = MaterialEditorGui.currentObject.getTargetName(%j);
         %count = SubMaterialSelector.getCount();
         SubMaterialSelector.add(%target);
      }
   }
   else // Other classes that support materials if possible
   {
      %canSupportMaterial = false;
      for( %i = 0; %i < %obj.getFieldCount(); %i++ )
      {
         %fieldName = %obj.getField(%i);
         
         if( %obj.getFieldType(%fieldName) !$= "TypeMaterialName" )
            continue;
      
         if( !%canSupportMaterial )
         {
            MaterialEditorGui.currentObject = %obj;
            SubMaterialSelector.clear();
            SubMaterialSelector.add(%fieldName, 0);
         }
         else
         {
            %count = SubMaterialSelector.getCount();
            SubMaterialSelector.add(%fieldName, %count);
         }
         %canSupportMaterial = true;
      }
      
      if( !%canSupportMaterial ) // Non-relevant classes get returned
         return;

      MaterialEditorGui.currentMeshMode = "EditorShape";
      MaterialEditorGui.setMode();
   }
   
   %id = SubMaterialSelector.findText( MaterialEditorGui.currentMaterial.mapTo );
   if( %id != -1 )
      SubMaterialSelector.setSelected( %id );
   else
      SubMaterialSelector.setSelected(0);	   
}

//==============================================================================
// Helper functions to help create categories and manage category lists

function MaterialEditorGui::updateAllFields(%this)
{
	matEd_cubemapEd_availableCubemapList.clear();
}

function MaterialEditorGui::updatePreviewObject(%this)
{   
   %newModel = matEd_quickPreview_Popup.getValue();

   switch$(%newModel)
	{
		case "sphere":
         matEd_quickPreview_Popup.selected = %newModel;
         matEd_previewObjectView.setModel("tools/materialeditor/gui/spherePreview.dts");
         matEd_previewObjectView.setOrbitDistance(4);
				
		case "cube":
         matEd_quickPreview_Popup.selected = %newModel;
         matEd_previewObjectView.setModel("tools/materialeditor/gui/cubePreview.dts");
         matEd_previewObjectView.setOrbitDistance(5);
				
		case "pyramid":
         matEd_quickPreview_Popup.selected = %newModel;
         matEd_previewObjectView.setModel("tools/materialeditor/gui/pyramidPreview.dts");
         matEd_previewObjectView.setOrbitDistance(5);
				
		case "cylinder":
         matEd_quickPreview_Popup.selected = %newModel;
         matEd_previewObjectView.setModel("tools/materialeditor/gui/cylinderPreview.dts");
         matEd_previewObjectView.setOrbitDistance(4.2);
				
		case "torus":
         matEd_quickPreview_Popup.selected = %newModel;
         matEd_previewObjectView.setModel("tools/materialeditor/gui/torusPreview.dts");
         matEd_previewObjectView.setOrbitDistance(4.2);
				
		case "knot":
         matEd_quickPreview_Popup.selected = %newModel;
         matEd_previewObjectView.setModel("tools/materialeditor/gui/torusknotPreview.dts");
	}
}

//==============================================================================
// Helper functions to help load and update the preview and active material

// Finds the selected line in the material list, then makes it active in the editor.
function MaterialEditorGui::prepareActiveMaterial(%this, %material, %override)
{
   // If were not valid, grab the first valid material out of the materialSet
   if( !isObject(%material) )
      %material = MaterialSet.getObject(0);
	
	// Check made in order to avoid loading the same material. Overriding
	// made in special cases
	if(%material $= MaterialEditorGui.lastMaterial && !%override)
	{
	   return;
	}
   else
   {
      if(MaterialEditorGui.materialDirty )
      { 
         MaterialEditorGui.showSaveDialog( %material );
         return;
      }
		   
      MaterialEditorGui.setActiveMaterial(%material);
   }
}

// Updates the preview material to use the same properties as the selected material,
// and makes that material active in the editor.
function MaterialEditorGui::setActiveMaterial( %this, %material )
{
   // Warn if selecting a CustomMaterial (they can't be properly previewed or edited)
   if ( isObject( %material ) && %material.isMemberOfClass( "CustomMaterial" ) )
   {
      MessageBoxOK( "Warning", "The selected Material (" @ %material.getName() @
         ") is a CustomMaterial, and cannot be edited using the Material Editor." );
      return;
   }

   MaterialEditorGui.currentMaterial = %material;
   MaterialEditorGui.lastMaterial = %material;
   
   // we create or recreate a material to hold in a pristine state
   singleton Material(notDirtyMaterial)
   {
      mapTo = "matEd_mappedMat";
      diffuseMap[0] = "tools/materialEditor/gui/matEd_mappedMat";
   };
   
   // Converts the texture files into absolute paths.
   MaterialEditorGui.convertTextureFields();

   // If we're allowing for name changes, make sure to save the name seperately
   %this.originalName = MaterialEditorGui.currentMaterial.name;

   // Copy materials over to other references
   MaterialEditorGui.copyMaterials( MaterialEditorGui.currentMaterial, materialEd_previewMaterial );
   MaterialEditorGui.copyMaterials( MaterialEditorGui.currentMaterial, notDirtyMaterial );
   MaterialEditorGui.guiSync( materialEd_previewMaterial );   
     
   // Necessary functionality in order to render correctly
   materialEd_previewMaterial.flush();
   materialEd_previewMaterial.reload();
   MaterialEditorGui.currentMaterial.flush();
   MaterialEditorGui.currentMaterial.reload();
      
   MaterialEditorGui.setMaterialNotDirty();
}

function MaterialEditorGui::isMatEditorMaterial(%this, %material)
{
   return ( %material.getFilename() $= "" ||
            %material.getFilename() $= "tools/gui/materialSelector.ed.gui" ||
            %material.getFilename() $= "tools/materialEditor/scripts/materialEditor.ed.cs" );
}

function MaterialEditorGui::setMaterialNotDirty(%this)
{
   %propertyText = "Material Properties";
   %previewText = "Material Preview";
   MaterialEditorPropertiesWindow.text = %propertyText;
   MaterialEditorPreviewWindow.text = %previewText;
   
   MaterialEditorGui.materialDirty = false;
   matEd_PersistMan.removeDirty(MaterialEditorGui.currentMaterial);
}

function MaterialEditorGui::setMaterialDirty(%this)
{ 
   %propertyText = "Material Properties *";
   %previewText = "Material Preview *";
   MaterialEditorPropertiesWindow.text = %propertyText;
   MaterialEditorPreviewWindow.text = %previewText;
   
   MaterialEditorGui.materialDirty = true;
   
   // materials created in the material selector are given that as its filename, so we run another check
   if( MaterialEditorGui.isMatEditorMaterial( MaterialEditorGui.currentMaterial ) )
   {
      if( MaterialEditorGui.currentMaterial.isAutoGenerated() )
      {
         %obj = MaterialEditorGui.currentObject;
         
         if( %obj.shapeName !$= "" ) 
            %shapePath = %obj.shapeName;
         else if( %obj.isMethod("getDatablock") )
         {
            if( %obj.getDatablock().shapeFile !$= "" )
               %shapePath = %obj.getDatablock().shapeFile;
         }
         
         //creating toPath
         %k = 0;
         while( strpos( %shapePath, "/", %k ) != -1 )
         {
            %pos = strpos( %shapePath, "/", %k );
            %k = %pos + 1;
         }
         %savePath = getSubStr( %shapePath , 0 , %k );
         %savePath = %savePath @ "materials.cs";
         
         matEd_PersistMan.setDirty(MaterialEditorGui.currentMaterial, %savePath);
      }
      else
      {
         matEd_PersistMan.setDirty(MaterialEditorGui.currentMaterial, "art/materials.cs");
      }
   }
   else
      matEd_PersistMan.setDirty(MaterialEditorGui.currentMaterial);
}

function MaterialEditorGui::convertTextureFields(%this)
{
   // Find the absolute paths for the texture filenames so that
   // we can properly wire up the preview materials and controls.
   
   for(%diffuseI = 0; %diffuseI < 4; %diffuseI++)
   {
      %diffuseMap = MaterialEditorGui.currentMaterial.diffuseMap[%diffuseI];         
      %diffuseMap = MaterialEditorGui.searchForTexture(MaterialEditorGui.currentMaterial, %diffuseMap);      
      MaterialEditorGui.currentMaterial.diffuseMap[%diffuseI] = %diffuseMap;
   }
   
   for(%normalI = 0; %normalI < 4; %normalI++)
   {
      %normalMap = MaterialEditorGui.currentMaterial.normalMap[%normalI];
      %normalMap = MaterialEditorGui.searchForTexture(MaterialEditorGui.currentMaterial, %normalMap);
      MaterialEditorGui.currentMaterial.normalMap[%normalI] = %normalMap;
   }
   
   for(%overlayI = 0; %overlayI < 4; %overlayI++)
   {
      %overlayMap = MaterialEditorGui.currentMaterial.overlayMap[%overlayI];
      %overlayMap = MaterialEditorGui.searchForTexture(MaterialEditorGui.currentMaterial, %overlayMap);      
      MaterialEditorGui.currentMaterial.overlayMap[%overlayI] = %overlayMap;
   }
   
   for(%detailI = 0; %detailI < 4; %detailI++)
   {
      %detailMap = MaterialEditorGui.currentMaterial.detailMap[%detailI];
      %detailMap = MaterialEditorGui.searchForTexture(MaterialEditorGui.currentMaterial, %detailMap);      
      MaterialEditorGui.currentMaterial.detailMap[%detailI] = %detailMap;
   }
   
   for(%detailNormalI = 0; %detailNormalI < 4; %detailNormalI++)
   {
      %detailNormalMap = MaterialEditorGui.currentMaterial.detailNormalMap[%detailNormalI];
      %detailNormalMap = MaterialEditorGui.searchForTexture(MaterialEditorGui.currentMaterial, %detailNormalMap);      
      MaterialEditorGui.currentMaterial.detailNormalMap[%detailNormalI] = %detailNormalMap;
   }
   
   for(%lightI = 0; %lightI < 4; %lightI++)
   {
      %lightMap = MaterialEditorGui.currentMaterial.lightMap[%lightI];      
      %lightMap = MaterialEditorGui.searchForTexture(MaterialEditorGui.currentMaterial, %lightMap);
      MaterialEditorGui.currentMaterial.lightMap[%lightI] = %lightMap;
   }
   
   for(%toneI = 0; %toneI < 4; %toneI++)
   {
      %toneMap = MaterialEditorGui.currentMaterial.toneMap[%toneI];      
      %toneMap = MaterialEditorGui.searchForTexture(MaterialEditorGui.currentMaterial, %toneMap);
      MaterialEditorGui.currentMaterial.toneMap[%toneI] = %toneMap;
   }
   
   for(%specI = 0; %specI < 4; %specI++)
   {
      %specMap = MaterialEditorGui.currentMaterial.specularMap[%specI];      
      %specMap = MaterialEditorGui.searchForTexture(MaterialEditorGui.currentMaterial, %specMap);
      MaterialEditorGui.currentMaterial.specularMap[%specI] = %specMap;
   }
}

// still needs to be optimized further
function MaterialEditorGui::searchForTexture(%this,%material, %texture)
{   
   if( %texture !$= "" )
   {
      // set the find signal as false to start out with 
      %isFile = false;
      // sete the formats we're going to be looping through if need be
      %formats = ".png .jpg .dds .bmp .gif .jng .tga";
      
      // if the texture contains the correct filepath and name right off the bat, lets use it
      if( isFile(%texture) )
         %isFile = true;
      else
      {
         
         for( %i = 0; %i < getWordCount(%formats); %i++)
         {
            %testFileName = %texture @ getWord( %formats, %i );
            if(isFile(%testFileName))
            {
               %isFile = true;
               break;
            }
         }
      }
      
      // if we didn't grab a proper name, lets use a string logarithm
      if( !%isFile )
      {
         %materialDiffuse = %texture;
         %materialDiffuse2 = %texture;
         
         %materialPath = %material.getFilename();
         
         if( strchr( %materialDiffuse, "/") $= "" )
         {
            %k = 0;
            while( strpos( %materialPath, "/", %k ) != -1 )
            {
               %count = strpos( %materialPath, "/", %k );
               %k = %count + 1;
            }
         
            %materialsCs = getSubStr( %materialPath , %k , 99 );
            %texture =  strreplace( %materialPath, %materialsCs, %texture );
         }
         else
            %texture =  strreplace( %materialPath, %materialPath, %texture );
         
         
         // lets test the pathing we came up with
         if( isFile(%texture) )
            %isFile = true;
         else
         {
            for( %i = 0; %i < getWordCount(%formats); %i++)
            {
               %testFileName = %texture @ getWord( %formats, %i );
               if(isFile(%testFileName))
               {
                  %isFile = true;
                  break;
               }
            }
         }
         
         // as a last resort to find the proper name
         // we have to resolve using find first file functions very very slow
         if( !%isFile )
         {
            %k = 0;
            while( strpos( %materialDiffuse2, "/", %k ) != -1 )
            {
               %count = strpos( %materialDiffuse2, "/", %k );
               %k = %count + 1;
            }
            
            %texture =  getSubStr( %materialDiffuse2 , %k , 99 );
            for( %i = 0; %i < getWordCount(%formats); %i++)
            {
               %searchString = "*" @ %texture @ getWord( %formats, %i );
               %testFileName = findFirstFile( %searchString );
               if( isFile(%testFileName) )
               {
                  %texture = %testFileName;
                  %isFile = true;
                  break;
               }
            }
         }
         
         return %texture;
      }
      else
         return %texture; //Texture exists and can be found - just return the input argument.
   }

   return ""; //No texture associated with this property.
}

function MaterialEditorGui::updateLivePreview(%this,%preview)
{
   // When checkbox is selected, preview the material in real time, if not; then don't
   if( %preview )
      MaterialEditorGui.copyMaterials( materialEd_previewMaterial, MaterialEditorGui.currentMaterial );
   else
      MaterialEditorGui.copyMaterials( notDirtyMaterial, MaterialEditorGui.currentMaterial );
   
   MaterialEditorGui.currentMaterial.flush();
   MaterialEditorGui.currentMaterial.reload();
}

function MaterialEditorGui::copyMaterials( %this, %copyFrom, %copyTo)
{
   // Make sure we copy and restore the map to.
   %mapTo = %copyTo.mapTo;
   %copyTo.assignFieldsFrom( %copyFrom );
   %copyTo.mapTo = %mapTo;

}

function MaterialEditorGui::guiSync( %this, %material )
{
   %this.preventUndo = true;
   //Setup our headers
   if( MaterialEditorGui.currentMode $= "material" )
   {
      MatEdMaterialMode-->selMaterialName.setText(MaterialEditorGui.currentMaterial.name);
      MatEdMaterialMode-->selMaterialMapTo.setText(MaterialEditorGui.currentMaterial.mapTo);
   }
   else
   {
      if( MaterialEditorGui.currentObject.isMethod("getModelFile") )
      {
         %sourcePath = MaterialEditorGui.currentObject.getModelFile();
         if( %sourcePath !$= "" )
         {
            MatEdTargetMode-->selMaterialMapTo.ToolTip = %sourcePath;
            %sourceName = fileName(%sourcePath);
            MatEdTargetMode-->selMaterialMapTo.setText(%sourceName);
            MatEdTargetMode-->selMaterialName.setText(MaterialEditorGui.currentMaterial.name);
         }
      }
      else
      {
         %info = MaterialEditorGui.currentObject.getClassName();
         MatEdTargetMode-->selMaterialMapTo.ToolTip = %info;
         MatEdTargetMode-->selMaterialMapTo.setText(%info);
         MatEdTargetMode-->selMaterialName.setText(MaterialEditorGui.currentMaterial.name);
      }
   }
   
   MaterialEditorPropertiesWindow-->alphaRefTextEdit.setText((%material).alphaRef);
   MaterialEditorPropertiesWindow-->alphaRefSlider.setValue((%material).alphaRef);
   MaterialEditorPropertiesWindow-->doubleSidedCheckBox.setValue((%material).doubleSided);
   MaterialEditorPropertiesWindow-->transZWriteCheckBox.setValue((%material).translucentZWrite);
   MaterialEditorPropertiesWindow-->alphaTestCheckBox.setValue((%material).alphaTest);
   MaterialEditorPropertiesWindow-->castShadows.setValue((%material).castShadows);
   MaterialEditorPropertiesWindow-->castDynamicShadows.setValue((%material).castDynamicShadows);
   MaterialEditorPropertiesWindow-->translucentCheckbox.setValue((%material).translucent);
   
   switch$((%material).translucentBlendOp)
   {
           case "None": %selectedNum = 0;
            case "Mul": %selectedNum = 1;
            case "Add": %selectedNum = 2;
       case "AddAlpha": %selectedNum = 3;
            case "Sub": %selectedNum = 4;
      case "LerpAlpha": %selectedNum = 5;
   }
   MaterialEditorPropertiesWindow-->blendingTypePopUp.setSelected(%selectedNum);
   
   if((%material).cubemap !$= "")
   {
      MaterialEditorPropertiesWindow-->matEd_cubemapEditBtn.setVisible(1);
      MaterialEditorPropertiesWindow-->reflectionTypePopUp.setSelected(1);
   }
   else if((%material).dynamiccubemap)
   {
      MaterialEditorPropertiesWindow-->matEd_cubemapEditBtn.setVisible(0);
      MaterialEditorPropertiesWindow-->reflectionTypePopUp.setSelected(2);
   }
   else if((%material).planarReflection)
   {
      MaterialEditorPropertiesWindow-->matEd_cubemapEditBtn.setVisible(0);
      MaterialEditorPropertiesWindow-->reflectionTypePopUp.setSelected(3);
   }
   else
   {
      MaterialEditorPropertiesWindow-->matEd_cubemapEditBtn.setVisible(0);
      MaterialEditorPropertiesWindow-->reflectionTypePopUp.setSelected(0);
   }

   MaterialEditorPropertiesWindow-->effectColor0Swatch.color = (%material).effectColor[0];
   MaterialEditorPropertiesWindow-->effectColor1Swatch.color = (%material).effectColor[1];
   MaterialEditorPropertiesWindow-->showFootprintsCheckbox.setValue((%material).showFootprints);
   MaterialEditorPropertiesWindow-->showDustCheckbox.setValue((%material).showDust);
   MaterialEditorGui.updateSoundPopup("Footstep", (%material).footstepSoundId, (%material).customFootstepSound);
   MaterialEditorGui.updateSoundPopup("Impact", (%material).impactSoundId, (%material).customImpactSound);

   //layer specific controls are located here
   %layer = MaterialEditorGui.currentLayer;
   
   if((%material).diffuseMap[%layer] $= "") 
   {
      MaterialEditorPropertiesWindow-->diffuseMapNameText.setText( "None" );
      MaterialEditorPropertiesWindow-->diffuseMapDisplayBitmap.setBitmap( "tools/materialeditor/gui/unknownImage" );
   }
   else
   {
      MaterialEditorPropertiesWindow-->diffuseMapNameText.setText( (%material).diffuseMap[%layer] );      
      MaterialEditorPropertiesWindow-->diffuseMapDisplayBitmap.setBitmap( (%material).diffuseMap[%layer] );
   }
 
   if((%material).normalMap[%layer] $= "") 
   {
      MaterialEditorPropertiesWindow-->normalMapNameText.setText( "None" );
      MaterialEditorPropertiesWindow-->normalMapDisplayBitmap.setBitmap( "tools/materialeditor/gui/unknownImage" );
   }
   else
   {
      MaterialEditorPropertiesWindow-->normalMapNameText.setText( (%material).normalMap[%layer] );
      MaterialEditorPropertiesWindow-->normalMapDisplayBitmap.setBitmap( (%material).normalMap[%layer] );
   }
   
   if((%material).overlayMap[%layer] $= "") 
   {
      MaterialEditorPropertiesWindow-->overlayMapNameText.setText( "None" );
      MaterialEditorPropertiesWindow-->overlayMapDisplayBitmap.setBitmap( "tools/materialeditor/gui/unknownImage" );
   }
   else
   {
      MaterialEditorPropertiesWindow-->overlayMapNameText.setText( (%material).overlayMap[%layer] );
      MaterialEditorPropertiesWindow-->overlayMapDisplayBitmap.setBitmap( (%material).overlayMap[%layer] );
   }
   
   if((%material).detailMap[%layer] $= "") 
   {
      MaterialEditorPropertiesWindow-->detailMapNameText.setText( "None" );
      MaterialEditorPropertiesWindow-->detailMapDisplayBitmap.setBitmap( "tools/materialeditor/gui/unknownImage" );
   }
   else
   {
      MaterialEditorPropertiesWindow-->detailMapNameText.setText( (%material).detailMap[%layer] );
      MaterialEditorPropertiesWindow-->detailMapDisplayBitmap.setBitmap( (%material).detailMap[%layer] );
   }
   
   if((%material).detailNormalMap[%layer] $= "") 
   {
      MaterialEditorPropertiesWindow-->detailNormalMapNameText.setText( "None" );
      MaterialEditorPropertiesWindow-->detailNormalMapDisplayBitmap.setBitmap( "tools/materialeditor/gui/unknownImage" );
   }
   else
   {
      MaterialEditorPropertiesWindow-->detailNormalMapNameText.setText( (%material).detailNormalMap[%layer] );
      MaterialEditorPropertiesWindow-->detailNormalMapDisplayBitmap.setBitmap( (%material).detailNormalMap[%layer] );
   }
   
   if((%material).lightMap[%layer] $= "") 
   {
      MaterialEditorPropertiesWindow-->lightMapNameText.setText( "None" );
      MaterialEditorPropertiesWindow-->lightMapDisplayBitmap.setBitmap( "tools/materialeditor/gui/unknownImage" );
   }
   else
   {
      MaterialEditorPropertiesWindow-->lightMapNameText.setText( (%material).lightMap[%layer] );
      MaterialEditorPropertiesWindow-->lightMapDisplayBitmap.setBitmap( (%material).lightMap[%layer] );
   }
   
   if((%material).toneMap[%layer] $= "") 
   {
      MaterialEditorPropertiesWindow-->toneMapNameText.setText( "None" );
      MaterialEditorPropertiesWindow-->toneMapDisplayBitmap.setBitmap( "tools/materialeditor/gui/unknownImage" );
   }
   else
   {
      MaterialEditorPropertiesWindow-->toneMapNameText.setText( (%material).toneMap[%layer] );
      MaterialEditorPropertiesWindow-->toneMapDisplayBitmap.setBitmap( (%material).toneMap[%layer] );
   }
   
   if((%material).specularMap[%layer] $= "") 
   {
      MaterialEditorPropertiesWindow-->specMapNameText.setText( "None" );
      MaterialEditorPropertiesWindow-->specMapDisplayBitmap.setBitmap( "tools/materialeditor/gui/unknownImage" );
   }
   else
   {
      MaterialEditorPropertiesWindow-->specMapNameText.setText( (%material).specularMap[%layer] );
      MaterialEditorPropertiesWindow-->specMapDisplayBitmap.setBitmap( (%material).specularMap[%layer] );
   }
   
   MaterialEditorPropertiesWindow-->accuScaleTextEdit.setText((%material).accuScale[%layer]);
   MaterialEditorPropertiesWindow-->accuScaleTextEdit.setText((%material).accuScale[%layer]);
   MaterialEditorPropertiesWindow-->accuDirectionTextEdit.setText((%material).accuDirection[%layer]);
   MaterialEditorPropertiesWindow-->accuDirectionTextEdit.setText((%material).accuDirection[%layer]);
   MaterialEditorPropertiesWindow-->accuStrengthTextEdit.setText((%material).accuStrength[%layer]);
   MaterialEditorPropertiesWindow-->accuStrengthTextEdit.setText((%material).accuStrength[%layer]);
   MaterialEditorPropertiesWindow-->accuCoverageTextEdit.setText((%material).accuCoverage[%layer]);
   MaterialEditorPropertiesWindow-->accuCoverageTextEdit.setText((%material).accuCoverage[%layer]);
   MaterialEditorPropertiesWindow-->accuSpecularTextEdit.setText((%material).accuSpecular[%layer]);
   MaterialEditorPropertiesWindow-->accuSpecularTextEdit.setText((%material).accuSpecular[%layer]);
   
   MaterialEditorPropertiesWindow-->detailScaleTextEdit.setText( getWord((%material).detailScale[%layer], 0) );
   MaterialEditorPropertiesWindow-->detailNormalStrengthTextEdit.setText( getWord((%material).detailNormalMapStrength[%layer], 0) );
   
   MaterialEditorPropertiesWindow-->colorTintSwatch.color = (%material).diffuseColor[%layer];
   MaterialEditorPropertiesWindow-->specularColorSwatch.color = (%material).specular[%layer];     
   
   MaterialEditorPropertiesWindow-->specularPowerTextEdit.setText((%material).specularPower[%layer]);
   MaterialEditorPropertiesWindow-->specularPowerSlider.setValue((%material).specularPower[%layer]);
   MaterialEditorPropertiesWindow-->specularStrengthTextEdit.setText((%material).specularStrength[%layer]);
   MaterialEditorPropertiesWindow-->specularStrengthSlider.setValue((%material).specularStrength[%layer]);
   MaterialEditorPropertiesWindow-->pixelSpecularCheckbox.setValue((%material).pixelSpecular[%layer]);
   MaterialEditorPropertiesWindow-->glowCheckbox.setValue((%material).glow[%layer]);
   MaterialEditorPropertiesWindow-->emissiveCheckbox.setValue((%material).emissive[%layer]);
   MaterialEditorPropertiesWindow-->parallaxTextEdit.setText((%material).parallaxScale[%layer]);
   MaterialEditorPropertiesWindow-->parallaxSlider.setValue((%material).parallaxScale[%layer]);

   MaterialEditorPropertiesWindow-->useAnisoCheckbox.setValue((%material).useAnisotropic[%layer]);
   MaterialEditorPropertiesWindow-->vertLitCheckbox.setValue((%material).vertLit[%layer]);
   MaterialEditorPropertiesWindow-->vertColorSwatch.color = (%material).vertColor[%layer];
   MaterialEditorPropertiesWindow-->subSurfaceCheckbox.setValue((%material).subSurface[%layer]);
   MaterialEditorPropertiesWindow-->subSurfaceColorSwatch.color = (%material).subSurfaceColor[%layer];
   MaterialEditorPropertiesWindow-->subSurfaceRolloffTextEdit.setText((%material).subSurfaceRolloff[%layer]);
   MaterialEditorPropertiesWindow-->minnaertTextEdit.setText((%material).minnaertConstant[%layer]);

   // Animation properties
   MaterialEditorPropertiesWindow-->RotationAnimation.setValue(0);
   MaterialEditorPropertiesWindow-->ScrollAnimation.setValue(0);
   MaterialEditorPropertiesWindow-->WaveAnimation.setValue(0);
   MaterialEditorPropertiesWindow-->ScaleAnimation.setValue(0);
   MaterialEditorPropertiesWindow-->SequenceAnimation.setValue(0);
   
   %flags = (%material).getAnimFlags(%layer);
   %wordCount = getWordCount( %flags );
   for(%i = 0; %i != %wordCount; %i++)
   {
      switch$(getWord( %flags, %i))
      {
         case "$rotate": MaterialEditorPropertiesWindow-->RotationAnimation.setValue(1);
         case "$scroll": MaterialEditorPropertiesWindow-->ScrollAnimation.setValue(1);
         case "$wave": MaterialEditorPropertiesWindow-->WaveAnimation.setValue(1);
         case "$scale": MaterialEditorPropertiesWindow-->ScaleAnimation.setValue(1);
         case "$sequence": MaterialEditorPropertiesWindow-->SequenceAnimation.setValue(1);
      }
   }
   
   MaterialEditorPropertiesWindow-->RotationTextEditU.setText( getWord((%material).rotPivotOffset[%layer], 0) );
   MaterialEditorPropertiesWindow-->RotationTextEditV.setText( getWord((%material).rotPivotOffset[%layer], 1) );
   MaterialEditorPropertiesWindow-->RotationSpeedTextEdit.setText( (%material).rotSpeed[%layer] );
   MaterialEditorPropertiesWindow-->RotationSliderU.setValue( getWord((%material).rotPivotOffset[%layer], 0) );
   MaterialEditorPropertiesWindow-->RotationSliderV.setValue( getWord((%material).rotPivotOffset[%layer], 1) );
   MaterialEditorPropertiesWindow-->RotationSpeedSlider.setValue( (%material).rotSpeed[%layer] );
   MaterialEditorPropertiesWindow-->RotationCrosshair.setPosition( 45*mAbs(getWord((%material).rotPivotOffset[%layer], 0))-2, 45*mAbs(getWord((%material).rotPivotOffset[%layer], 1))-2 );
   
   MaterialEditorPropertiesWindow-->ScrollTextEditU.setText( getWord((%material).scrollDir[%layer], 0) );
   MaterialEditorPropertiesWindow-->ScrollTextEditV.setText( getWord((%material).scrollDir[%layer], 1) );
   MaterialEditorPropertiesWindow-->ScrollSpeedTextEdit.setText( (%material).scrollSpeed[%layer] );
   MaterialEditorPropertiesWindow-->ScrollSliderU.setValue( getWord((%material).scrollDir[%layer], 0) );
   MaterialEditorPropertiesWindow-->ScrollSliderV.setValue( getWord((%material).scrollDir[%layer], 1) );
   MaterialEditorPropertiesWindow-->ScrollSpeedSlider.setValue( (%material).scrollSpeed[%layer] );
   MaterialEditorPropertiesWindow-->ScrollCrosshair.setPosition( -(23 * getWord((%material).scrollDir[%layer], 0))+20, -(23 * getWord((%material).scrollDir[%layer], 1))+20);
   
   %waveType = (%material).waveType[%layer];
   for( %radioButton = 0; %radioButton < MaterialEditorPropertiesWindow-->WaveButtonContainer.getCount(); %radioButton++ )
   {
      if( %waveType $= MaterialEditorPropertiesWindow-->WaveButtonContainer.getObject(%radioButton).waveType )
         MaterialEditorPropertiesWindow-->WaveButtonContainer.getObject(%radioButton).setStateOn(1);
   }
   
   MaterialEditorPropertiesWindow-->WaveTextEditAmp.setText( (%material).waveAmp[%layer] );
   MaterialEditorPropertiesWindow-->WaveTextEditFreq.setText( (%material).waveFreq[%layer] );
   MaterialEditorPropertiesWindow-->WaveSliderAmp.setValue( (%material).waveAmp[%layer] );
   MaterialEditorPropertiesWindow-->WaveSliderFreq.setValue( (%material).waveFreq[%layer] );

   %numFrames = mRound( 1 / (%material).sequenceSegmentSize[%layer] );

   MaterialEditorPropertiesWindow-->SequenceTextEditFPS.setText( (%material).sequenceFramePerSec[%layer] );
   MaterialEditorPropertiesWindow-->SequenceTextEditSSS.setText( %numFrames );
   MaterialEditorPropertiesWindow-->SequenceSliderFPS.setValue( (%material).sequenceFramePerSec[%layer] );
   MaterialEditorPropertiesWindow-->SequenceSliderSSS.setValue( %numFrames );
   
   // Accumulation
   MaterialEditorPropertiesWindow-->accuCheckbox.setValue((%material).accuEnabled[%layer]);   
   
   %this.preventUndo = false;
}

//=======================================
// Material Update Functionality

function MaterialEditorGui::changeLayer( %this, %layer )
{
   if( MaterialEditorGui.currentLayer == getWord(%layer, 1) )
      return;
      
   MaterialEditorGui.currentLayer = getWord(%layer, 1); 
   MaterialEditorGui.guiSync( materialEd_previewMaterial );
}

function MaterialEditorGui::updateActiveMaterial(%this, %propertyField, %value, %isSlider, %onMouseUp)
{
   MaterialEditorGui.setMaterialDirty();

   if(%value $= "")
      %value = "\"\"";
   
   // Here is where we handle undo actions with slider controls. We want to be able to
   // undo every onMouseUp; so we overrite the same undo action when necessary in order
   // to achieve this desired effect.
   %last = Editor.getUndoManager().getUndoAction(Editor.getUndoManager().getUndoCount() - 1);
   if((%last != -1) && (%last.isSlider) && (!%last.onMouseUp))
   {
      %last.field = %propertyField;
      %last.isSlider = %isSlider;
      %last.onMouseUp = %onMouseUp;
      %last.newValue = %value;
   }
   else
   {
      %action = %this.createUndo(ActionUpdateActiveMaterial, "Update Active Material");
      %action.material = MaterialEditorGui.currentMaterial;
      %action.object = MaterialEditorGui.currentObject;
      %action.field = %propertyField;
      %action.isSlider = %isSlider;
      %action.onMouseUp = %onMouseUp;
      %action.newValue = %value;
      eval( "%action.oldValue = " @ MaterialEditorGui.currentMaterial @ "." @ %propertyField @ ";");
      %action.oldValue = "\"" @ %action.oldValue @ "\"";
      MaterialEditorGui.submitUndo( %action );
   }
   
   eval("materialEd_previewMaterial." @ %propertyField @ " = " @ %value @ ";");
   materialEd_previewMaterial.flush();
   materialEd_previewMaterial.reload();
   
   if (MaterialEditorGui.livePreview == true)
   {
      eval("MaterialEditorGui.currentMaterial." @ %propertyField @ " = " @ %value @ ";");
      MaterialEditorGui.currentMaterial.flush();
      MaterialEditorGui.currentMaterial.reload();
   }
}

function MaterialEditorGui::updateActiveMaterialName(%this, %name)
{
   %action = %this.createUndo(ActionUpdateActiveMaterialName, "Update Active Material Name");
   %action.material =  MaterialEditorGui.currentMaterial;
   %action.object = MaterialEditorGui.currentObject;
   %action.oldName = MaterialEditorGui.currentMaterial.getName();
   %action.newName = %name;
   MaterialEditorGui.submitUndo( %action );

   MaterialEditorGui.currentMaterial.setName(%name);

   // Some objects (ConvexShape, DecalRoad etc) reference Materials by name => need
   // to find and update all these references so they don't break when we rename the
   // Material.
   MaterialEditorGui.updateMaterialReferences( MissionGroup, %action.oldName, %action.newName );
}

function MaterialEditorGui::updateMaterialReferences( %this, %obj, %oldName, %newName )
{
   if ( %obj.isMemberOfClass( "SimSet" ) )
   {
      // invoke on children
      %count = %obj.getCount();
      for ( %i = 0; %i < %count; %i++ )
         %this.updateMaterialReferences( %obj.getObject( %i ), %oldName, %newName );
   }
   else
   {
      %objChanged = false;

      // Change all material fields that use the old material name
      %count = %obj.getFieldCount();
      for( %i = 0; %i < %count; %i++ )
      {
         %fieldName = %obj.getField( %i );
         if ( ( %obj.getFieldType( %fieldName ) $= "TypeMaterialName" ) && ( %obj.getFieldValue( %fieldName ) $= %oldName ) )
         {
            eval( %obj @ "." @ %fieldName @ " = " @ %newName @ ";" );
            %objChanged = true;
         }
      }

      EWorldEditor.isDirty |= %objChanged;
      if ( %objChanged && %obj.isMethod( "postApply" ) )
         %obj.postApply();
   }
}

// Global Material Options

function MaterialEditorGui::updateReflectionType( %this, %type )
{
	if( %type $= "None" )
	{
	   MaterialEditorPropertiesWindow-->matEd_cubemapEditBtn.setVisible(0);
	   //Reset material reflection settings on the preview materials
	   MaterialEditorGui.updateActiveMaterial( "cubeMap", "" );
	   MaterialEditorGui.updateActiveMaterial( "dynamicCubemap" , false );
	   MaterialEditorGui.updateActiveMaterial( "planarReflection", false );
	}
	else
	{
      if(%type $= "cubeMap")	 
      {
         MaterialEditorPropertiesWindow-->matEd_cubemapEditBtn.setVisible(1);
	      MaterialEditorGui.updateActiveMaterial( %type, materialEd_previewMaterial.cubemap );
      }
      else
      {
         MaterialEditorGui.updateActiveMaterial( %type, true );
      }
	}
}

// Per-Layer Material Options

// For update maps
// %action : 1 = change map
// %action : 0 = remove map

function MaterialEditorGui::updateTextureMap( %this, %type, %action )
{
   %layer = MaterialEditorGui.currentLayer;

   %bitmapCtrl = MaterialEditorPropertiesWindow.findObjectByInternalName( %type @ "MapDisplayBitmap", true );
   %textCtrl = MaterialEditorPropertiesWindow.findObjectByInternalName( %type @ "MapNameText", true );

   if( %action )
   {
      %texture = MaterialEditorGui.openFile("texture");
      if( %texture !$= "" )
      {
         %bitmapCtrl.setBitmap(%texture);
      
         %bitmap = %bitmapCtrl.bitmap;
         %bitmap = strreplace(%bitmap,"tools/materialEditor/scripts/","");
         %bitmapCtrl.setBitmap(%bitmap);
         %textCtrl.setText(%bitmap);
         MaterialEditorGui.updateActiveMaterial(%type @ "Map[" @ %layer @ "]","\"" @ %bitmap @ "\"");	
      }
   }
   else
   {
      %textCtrl.setText("None");
      %bitmapCtrl.setBitmap("tools/materialeditor/gui/unknownImage");
      MaterialEditorGui.updateActiveMaterial(%type @ "Map[" @ %layer @ "]","");
   }
}

function MaterialEditorGui::updateDetailScale(%this,%newScale)
{
   %layer = MaterialEditorGui.currentLayer;
   
   %detailScale = "\"" @ %newScale SPC %newScale @ "\"";
   MaterialEditorGui.updateActiveMaterial("detailScale[" @ %layer @ "]", %detailScale);
}

function MaterialEditorGui::updateDetailNormalStrength(%this,%newStrength)
{
   %layer = MaterialEditorGui.currentLayer;
   
   %detailStrength = "\"" @ %newStrength @ "\"";
   MaterialEditorGui.updateActiveMaterial("detailNormalMapStrength[" @ %layer @ "]", %detailStrength);
}

function MaterialEditorGui::updateSpecMap(%this,%action)
{
   %layer = MaterialEditorGui.currentLayer;
   
   if( %action )
   {
      %texture = MaterialEditorGui.openFile("texture");
      if( %texture !$= "" )
      {
         MaterialEditorGui.updateActiveMaterial("pixelSpecular[" @ MaterialEditorGui.currentLayer @ "]", 0);
         
         MaterialEditorPropertiesWindow-->specMapDisplayBitmap.setBitmap(%texture);
      
         %bitmap = MaterialEditorPropertiesWindow-->specMapDisplayBitmap.bitmap;
         %bitmap = strreplace(%bitmap,"tools/materialEditor/scripts/","");
         MaterialEditorPropertiesWindow-->specMapDisplayBitmap.setBitmap(%bitmap);
         MaterialEditorPropertiesWindow-->specMapNameText.setText(%bitmap);
         MaterialEditorGui.updateActiveMaterial("specularMap[" @ %layer @ "]","\"" @ %bitmap @ "\"");
      }
   }
   else
   {
      MaterialEditorPropertiesWindow-->specMapNameText.setText("None");
      MaterialEditorPropertiesWindow-->specMapDisplayBitmap.setBitmap("tools/materialeditor/gui/unknownImage");
      MaterialEditorGui.updateActiveMaterial("specularMap[" @ %layer @ "]","");
   }
   
   MaterialEditorGui.guiSync( materialEd_previewMaterial );
}

function MaterialEditorGui::updateRotationOffset(%this, %isSlider, %onMouseUp)
{
   %layer = MaterialEditorGui.currentLayer;
	%X = MaterialEditorPropertiesWindow-->RotationTextEditU.getText();
	%Y = MaterialEditorPropertiesWindow-->RotationTextEditV.getText();
	MaterialEditorPropertiesWindow-->RotationCrosshair.setPosition(45*mAbs(%X)-2, 45*mAbs(%Y)-2);
	
	MaterialEditorGui.updateActiveMaterial("rotPivotOffset[" @ %layer @ "]","\"" @ %X SPC %Y @ "\"",%isSlider,%onMouseUp);
}

function MaterialEditorGui::updateRotationSpeed(%this, %isSlider, %onMouseUp)
{
   %layer = MaterialEditorGui.currentLayer;
   %speed = MaterialEditorPropertiesWindow-->RotationSpeedTextEdit.getText();
   MaterialEditorGui.updateActiveMaterial("rotSpeed[" @ %layer @ "]",%speed,%isSlider,%onMouseUp);
}

function MaterialEditorGui::updateScrollOffset(%this, %isSlider, %onMouseUp)
{
   %layer = MaterialEditorGui.currentLayer;
	%X = MaterialEditorPropertiesWindow-->ScrollTextEditU.getText();
	%Y = MaterialEditorPropertiesWindow-->ScrollTextEditV.getText();
	MaterialEditorPropertiesWindow-->ScrollCrosshair.setPosition( -(23 * %X)+20, -(23 * %Y)+20);
	
	MaterialEditorGui.updateActiveMaterial("scrollDir[" @ %layer @ "]","\"" @ %X SPC %Y @ "\"",%isSlider,%onMouseUp);	
}

function MaterialEditorGui::updateScrollSpeed(%this, %isSlider, %onMouseUp)
{
   %layer = MaterialEditorGui.currentLayer;
   %speed = MaterialEditorPropertiesWindow-->ScrollSpeedTextEdit.getText();
   MaterialEditorGui.updateActiveMaterial("scrollSpeed[" @ %layer @ "]",%speed,%isSlider,%onMouseUp);
}

function MaterialEditorGui::updateWaveType(%this)
{
   for( %radioButton = 0; %radioButton < MaterialEditorPropertiesWindow-->WaveButtonContainer.getCount(); %radioButton++ )
   {
      if( MaterialEditorPropertiesWindow-->WaveButtonContainer.getObject(%radioButton).getValue() == 1 )
         %type = MaterialEditorPropertiesWindow-->WaveButtonContainer.getObject(%radioButton).waveType;
   }
   
   %layer = MaterialEditorGui.currentLayer;
   MaterialEditorGui.updateActiveMaterial("waveType[" @ %layer @ "]", %type);
}

function MaterialEditorGui::updateWaveAmp(%this, %isSlider, %onMouseUp)
{
   %layer = MaterialEditorGui.currentLayer;
   %amp = MaterialEditorPropertiesWindow-->WaveTextEditAmp.getText();
   MaterialEditorGui.updateActiveMaterial("waveAmp[" @ %layer @ "]", %amp, %isSlider, %onMouseUp);
}

function MaterialEditorGui::updateWaveFreq(%this, %isSlider, %onMouseUp)
{
   %layer = MaterialEditorGui.currentLayer;
   %freq = MaterialEditorPropertiesWindow-->WaveTextEditFreq.getText();
   MaterialEditorGui.updateActiveMaterial("waveFreq[" @ %layer @ "]", %freq, %isSlider, %onMouseUp);
}

function MaterialEditorGui::updateSequenceFPS(%this, %isSlider, %onMouseUp)
{
   %layer = MaterialEditorGui.currentLayer;
	%fps = MaterialEditorPropertiesWindow-->SequenceTextEditFPS.getText();
	MaterialEditorGui.updateActiveMaterial("sequenceFramePerSec[" @ %layer @ "]", %fps, %isSlider, %onMouseUp);	
}

function MaterialEditorGui::updateSequenceSSS(%this, %isSlider, %onMouseUp)
{
   %layer = MaterialEditorGui.currentLayer;
   %sss = 1 / MaterialEditorPropertiesWindow-->SequenceTextEditSSS.getText();
   MaterialEditorGui.updateActiveMaterial("sequenceSegmentSize[" @ %layer @ "]", %sss, %isSlider, %onMouseUp);
}

function MaterialEditorGui::updateAnimationFlags(%this)
{
   MaterialEditorGui.setMaterialDirty();
   %single = true;
   
   if(MaterialEditorPropertiesWindow-->RotationAnimation.getValue() == true)
   {
      if(%single == true)
	      %flags = %flags @ "$Rotate";
	   else
		  %flags = %flags @ " | $Rotate";
		  
      %single = false;
   }
   if(MaterialEditorPropertiesWindow-->ScrollAnimation.getValue() == true)
   {
      if(%single == true)
	      %flags = %flags @ "$Scroll";
	   else
		  %flags = %flags @ " | $Scroll";
		  
      %single = false;
   }
   if(MaterialEditorPropertiesWindow-->WaveAnimation.getValue() == true)
   {
      if(%single == true)
	      %flags = %flags @ "$Wave";
	   else
		  %flags = %flags @ " | $Wave";
		  
      %single = false;
   }
   if(MaterialEditorPropertiesWindow-->ScaleAnimation.getValue() == true)
   {
      if(%single == true)
	      %flags = %flags @ "$Scale";
	   else
		  %flags = %flags @ " | $Scale";
		  
      %single = false;
   }
   if(MaterialEditorPropertiesWindow-->SequenceAnimation.getValue() == true)
   {
      if(%single == true)
	      %flags = %flags @ "$Sequence";
	   else
		  %flags = %flags @ " | $Sequence";
		  
      %single = false;
   }
   
   if(%flags $= "")
      %flags = "\"\"";
      
   %action = %this.createUndo(ActionUpdateActiveMaterialAnimationFlags, "Update Active Material");
   %action.material = MaterialEditorGui.currentMaterial;
   %action.object = MaterialEditorGui.currentObject;
   %action.layer = MaterialEditorGui.currentLayer; 
      
   %action.newValue = %flags;
   
   %oldFlags = MaterialEditorGui.currentMaterial.getAnimFlags(MaterialEditorGui.currentLayer);
   if(%oldFlags $= "")
      %oldFlags = "\"\"";
      
   %action.oldValue = %oldFlags;
   MaterialEditorGui.submitUndo( %action );
   
   eval("materialEd_previewMaterial.animFlags[" @ MaterialEditorGui.currentLayer @ "] = " @ %flags @ ";");
   materialEd_previewMaterial.flush();
   materialEd_previewMaterial.reload();
   
   if (MaterialEditorGui.livePreview == true)
   {
      eval("MaterialEditorGui.currentMaterial.animFlags[" @ MaterialEditorGui.currentLayer @ "] = " @ %flags @ ";");
      MaterialEditorGui.currentMaterial.flush();
      MaterialEditorGui.currentMaterial.reload();
   }
}

//==============================================================================
// Color Picker Helpers - They are all using colorPicker.ed.gui in order to function
// These functions are mainly passed callbacks from getColorI/getColorF callbacks

function MaterialEditorGui::syncGuiColor(%this, %guiCtrl, %propname, %color)
{
   %layer = MaterialEditorGui.currentLayer;

   %r = getWord(%color,0);
   %g = getWord(%color,1);
   %b = getWord(%color,2);
   %a = getWord(%color,3);

   %colorSwatch = (%r SPC %g SPC %b SPC %a);
   %color = "\"" @ %r SPC %g SPC %b SPC %a @ "\"";

   %guiCtrl.color = %colorSwatch;
	MaterialEditorGui.updateActiveMaterial(%propName, %color);
}

//These two functions are focused on object/layer specific functionality
function MaterialEditorGui::updateColorMultiply(%this,%color)
{
   %propName = "diffuseColor[" @ MaterialEditorGui.currentLayer @ "]";
   %this.syncGuiColor(MaterialEditorPropertiesWindow-->colorTintSwatch, %propName, %color);
}

function MaterialEditorGui::updateSpecularCheckbox(%this,%value)
{
   MaterialEditorGui.updateActiveMaterial("pixelSpecular[" @ MaterialEditorGui.currentLayer @ "]", %value);   
   MaterialEditorGui.guiSync( materialEd_previewMaterial );
}

function MaterialEditorGui::updateSpecular(%this, %color)
{
   %propName = "specular[" @ MaterialEditorGui.currentLayer @ "]";
   %this.syncGuiColor(MaterialEditorPropertiesWindow-->specularColorSwatch, %propName, %color);
}

function MaterialEditorGui::updateSubSurfaceColor(%this, %color)
{
   %propName = "subSurfaceColor[" @ MaterialEditorGui.currentLayer @ "]";
   %this.syncGuiColor(MaterialEditorPropertiesWindow-->subSurfaceColorSwatch, %propName, %color);
}

function MaterialEditorGui::updateEffectColor0(%this, %color)
{
   %this.syncGuiColor(MaterialEditorPropertiesWindow-->effectColor0Swatch, "effectColor[0]", %color);
}

function MaterialEditorGui::updateEffectColor1(%this, %color)
{
   %this.syncGuiColor(MaterialEditorPropertiesWindow-->effectColor1Swatch, "effectColor[1]", %color);
}

function MaterialEditorGui::updateBehaviorSound(%this, %type, %sound)
{
   %defaultId = -1;
   %customName = "";

   switch$ (%sound)
   {
      case "<Soft>":    %defaultId = 0;
      case "<Hard>":    %defaultId = 1;
      case "<Metal>":   %defaultId = 2;
      case "<Snow>":    %defaultId = 3;
      default:          %customName = %sound;
   }

   %this.updateActiveMaterial(%type @ "SoundId", %defaultId);
   %this.updateActiveMaterial("custom" @ %type @ "Sound", %customName);
}

function MaterialEditorGui::updateSoundPopup(%this, %type, %defaultId, %customName)
{
   %ctrl = MaterialEditorPropertiesWindow.findObjectByInternalName( %type @ "SoundPopup", true );

   switch (%defaultId)
   {
      case 0:        %name = "<Soft>";
      case 1:        %name = "<Hard>";
      case 2:        %name = "<Metal>";
      case 3:        %name = "<Snow>";
      default:
         if (%customName $= "")
            %name = "<None>";
         else
            %name = %customName;
   }

   %r = %ctrl.findText(%name);
   if (%r != -1)
      %ctrl.setSelected(%r, false);
   else
      %ctrl.setText(%name);
}

//These two functions are focused on environment specific functionality
function MaterialEditorGui::updateLightColor(%this, %color)
{
   matEd_previewObjectView.setLightColor(%color);
   matEd_lightColorPicker.color = %color;
}

function MaterialEditorGui::updatePreviewBackground(%this,%color)
{   
   matEd_previewBackground.color = %color;
   MaterialPreviewBackgroundPicker.color = %color;
}

function MaterialEditorGui::updateAmbientColor(%this,%color)
{
   matEd_previewObjectView.setAmbientLightColor(%color);
   matEd_ambientLightColorPicker.color = %color;
}

//==============================================================================
// Commands for the Cubemap Editor

function MaterialEditorGui::selectCubemap(%this)
{
   %cubemap = MaterialEditorGui.currentCubemap;
   if(!isObject(%cubemap))
      return;
   
   MaterialEditorGui.updateActiveMaterial( "cubemap", %cubemap.name );
   MaterialEditorGui.hideCubemapEditor();
}

function MaterialEditorGui::cancelCubemap(%this)
{
   %cubemap = MaterialEditorGui.currentCubemap;
   
   %idx = matEd_cubemapEd_availableCubemapList.findItemText( %cubemap.getName() );
   matEd_cubemapEd_availableCubemapList.setItemText( %idx, notDirtyCubemap.originalName );      
   %cubemap.setName( notDirtyCubemap.originalName );
   
   MaterialEditorGui.copyCubemaps( notDirtyCubemap, %cubemap );
   MaterialEditorGui.copyCubemaps( notDirtyCubemap, matEdCubeMapPreviewMat);
   
   %cubemap.updateFaces();
   matEdCubeMapPreviewMat.updateFaces();
}

function MaterialEditorGui::showCubemapEditor(%this)
{
   if (matEd_cubemapEditor.isVisible())
      return;

   MaterialEditorGui.currentCubemap = "";
   
   matEd_cubemapEditor.setVisible(1);
   new PersistenceManager(matEd_cubemapEdPerMan);
   MaterialEditorGui.setCubemapNotDirty();
   
   for( %i = 0; %i < RootGroup.getCount(); %i++ )
   {
      if( RootGroup.getObject(%i).getClassName()!$= "CubemapData" )
         continue;
         
      for( %k = 0; %k < UnlistedCubemaps.count(); %k++ )
      {
         %unlistedFound = 0;
         if( UnlistedCubemaps.getValue(%k) $= RootGroup.getObject(%i).name )
         {
            %unlistedFound = 1;
            break;
         }
      }
   
      if( %unlistedFound )
         continue;
      
      matEd_cubemapEd_availableCubemapList.addItem( RootGroup.getObject(%i).name );
   }
   
   singleton CubemapData(notDirtyCubemap);
   
   // if there was no cubemap, pick the first, select, and bail, these are going to take
   // care of themselves in the selected function
   if( !isObject( MaterialEditorGui.currentMaterial.cubemap ) )
   {
      if( matEd_cubemapEd_availableCubemapList.getItemCount() > 0 )
      {
         matEd_cubemapEd_availableCubemapList.setSelected(0, true);
         return;
      }
      else 
      {  
         // if there are no cubemaps, then create one, select, and bail
         %cubemap = MaterialEditorGui.createNewCubemap();
         matEd_cubemapEd_availableCubemapList.addItem( %cubemap.name );
         matEd_cubemapEd_availableCubemapList.setSelected(0, true);
         return;
      }
   }

   // do not directly change activeMat!
   MaterialEditorGui.currentCubemap = MaterialEditorGui.currentMaterial.cubemap.getId();
   %cubemap = MaterialEditorGui.currentCubemap;
   
   notDirtyCubemap.originalName = %cubemap.getName();
   MaterialEditorGui.copyCubemaps( %cubemap, notDirtyCubemap);
   MaterialEditorGui.copyCubemaps( %cubemap, matEdCubeMapPreviewMat);
   MaterialEditorGui.syncCubemap( %cubemap );
}

function MaterialEditorGui::hideCubemapEditor(%this,%cancel)
{
   if(%cancel)
      MaterialEditorGui.cancelCubemap();
      
   matEd_cubemapEd_availableCubemapList.clearItems();
   matEd_cubemapEdPerMan.delete();
   matEd_cubemapEditor.setVisible(0);
}

// create category and update current material if there is one
function MaterialEditorGui::addCubemap( %this,%cubemapName )
{
   if( %cubemapName $= "" )
   {
      MessageBoxOK( "Error", "Can not create a cubemap without a valid name.");
      return;
   }
   
   for(%i = 0; %i < RootGroup.getCount(); %i++)
   {
      if( %cubemapName $= RootGroup.getObject(%i).getName() )
      {
         MessageBoxOK( "Error", "There is already an object with the same name.");
         return;
      }
   }

   // Create and select a new cubemap
   %cubemap = MaterialEditorGui.createNewCubemap( %cubemapName );
   %idx = matEd_cubemapEd_availableCubemapList.addItem( %cubemap.name );
   matEd_cubemapEd_availableCubemapList.setSelected( %idx, true );

   // material category text field to blank
   matEd_addCubemapWindow-->cubemapName.setText("");
}

function MaterialEditorGui::createNewCubemap( %this, %cubemap )
{
   if( %cubemap $= "" )
   {
      for(%i = 0; ; %i++)
      {
         %cubemap = "newCubemap_" @ %i;
         if( !isObject(%cubemap) )
            break;
      }
   }
   
   new CubemapData(%cubemap) 
   {
      cubeFace[0] = "tools/materialeditor/gui/cube_xNeg";
      cubeFace[1] = "tools/materialeditor/gui/cube_xPos";
      cubeFace[2] = "tools/materialeditor/gui/cube_ZNeg";
      cubeFace[3] = "tools/materialeditor/gui/cube_ZPos";
      cubeFace[4] = "tools/materialeditor/gui/cube_YNeg";
      cubeFace[5] = "tools/materialeditor/gui/cube_YPos";

      parentGroup = RootGroup;
   };

   matEd_cubemapEdPerMan.setDirty( %cubemap, "art/materials.cs" );
   matEd_cubemapEdPerMan.saveDirty();

   return %cubemap;
}

function MaterialEditorGui::setCubemapDirty(%this)
{ 
   %propertyText = "Create Cubemap *";
   matEd_cubemapEditor.text = %propertyText;
   matEd_cubemapEditor.dirty = true;
   matEd_cubemapEditor-->saveCubemap.setActive(true);

   %cubemap = MaterialEditorGui.currentCubemap;

   // materials created in the materail selector are given that as its filename, so we run another check
   if( MaterialEditorGui.isMatEditorMaterial( %cubemap ) )
      matEd_cubemapEdPerMan.setDirty(%cubemap, "art/materials.cs");
   else
      matEd_cubemapEdPerMan.setDirty(%cubemap);
}

function MaterialEditorGui::setCubemapNotDirty(%this)
{
   %propertyText= strreplace("Create Cubemap" , "*" , "");
   matEd_cubemapEditor.text = %propertyText;
   matEd_cubemapEditor.dirty = false;
   matEd_cubemapEditor-->saveCubemap.setActive(false);

   %cubemap = MaterialEditorGui.currentCubemap;
   matEd_cubemapEdPerMan.removeDirty(%cubemap);
}

function MaterialEditorGui::showDeleteCubemapDialog(%this)
{
   %idx = matEd_cubemapEd_availableCubemapList.getSelectedItem();
   %cubemap = matEd_cubemapEd_availableCubemapList.getItemText( %idx );
   %cubemap = %cubemap.getId();
   
   if( %cubemap == -1 || !isObject(%cubemap) )
      return;
      
   if( isObject( %cubemap ) )
   {
      MessageBoxYesNoCancel("Delete Cubemap?", 
         "Are you sure you want to delete<br><br>" @ %cubemap.getName() @ "<br><br> Cubemap deletion won't take affect until the engine is quit.", 
         "MaterialEditorGui.deleteCubemap( " @ %cubemap @ ", " @ %idx @ " );", 
         "", 
         "" );
   }
}

function MaterialEditorGui::deleteCubemap( %this, %cubemap, %idx )
{
   matEd_cubemapEd_availableCubemapList.deleteItem( %idx );
   
   UnlistedCubemaps.add( "unlistedCubemaps", %cubemap.getName() );
      
   if( !MaterialEditorGui.isMatEditorMaterial( %cubemap ) )
   {
      matEd_cubemapEdPerMan.removeDirty( %cubemap );
      matEd_cubemapEdPerMan.removeObjectFromFile( %cubemap );
   }

   if( matEd_cubemapEd_availableCubemapList.getItemCount() > 0 )
   {
      matEd_cubemapEd_availableCubemapList.setSelected(0, true);
   }
   else 
   {  
      // if there are no cubemaps, then create one, select, and bail
      %cubemap = MaterialEditorGui.createNewCubemap();
      matEd_cubemapEd_availableCubemapList.addItem( %cubemap.getName() );
      matEd_cubemapEd_availableCubemapList.setSelected(0, true);
   }
}

function matEd_cubemapEd_availableCubemapList::onSelect( %this, %id, %cubemap )
{
   %cubemap = %cubemap.getId();
   if( MaterialEditorGui.currentCubemap $= %cubemap )
      return;
   
   if( matEd_cubemapEditor.dirty )
   {         
      %savedCubemap = MaterialEditorGui.currentCubemap;
      MessageBoxYesNoCancel("Save Existing Cubemap?", 
      "Do you want to save changes to <br><br>" @ %savedCubemap.getName(), 
      "MaterialEditorGui.saveCubemap(" @ true @ ");", 
      "MaterialEditorGui.saveCubemapDialogDontSave(" @ %cubemap @ ");",
      "MaterialEditorGui.saveCubemapDialogCancel();" );
   }
   else
      MaterialEditorGui.changeCubemap( %cubemap );
}

function MaterialEditorGui::showSaveCubemapDialog( %this )
{
   %cubemap = MaterialEditorGui.currentCubemap;
   if( !isObject(%cubemap) )
      return;
      
   MessageBoxYesNoCancel("Save Cubemap?", 
      "Do you want to save changes to <br><br>" @ %cubemap.getName(), 
      "MaterialEditorGui.saveCubemap( " @ %cubemap @ " );", 
      "", 
      "" );
}

function MaterialEditorGui::saveCubemap( %this, %cubemap )
{   
   notDirtyCubemap.originalName = %cubemap.getName();
   MaterialEditorGui.copyCubemaps( %cubemap, notDirtyCubemap );
   MaterialEditorGui.copyCubemaps( %cubemap, matEdCubeMapPreviewMat);

   matEd_cubemapEdPerMan.saveDirty(); 

   MaterialEditorGui.setCubemapNotDirty();
}

function MaterialEditorGui::saveCubemapDialogDontSave( %this, %newCubemap)
{  
   //deal with old cubemap first
   %oldCubemap = MaterialEditorGui.currentCubemap;
   
   %idx = matEd_cubemapEd_availableCubemapList.findItemText( %oldCubemap.getName() );
   matEd_cubemapEd_availableCubemapList.setItemText( %idx, notDirtyCubemap.originalName );      
   %oldCubemap.setName( notDirtyCubemap.originalName );
   
   MaterialEditorGui.copyCubemaps( notDirtyCubemap, %oldCubemap);
   MaterialEditorGui.copyCubemaps( notDirtyCubemap, matEdCubeMapPreviewMat);
   MaterialEditorGui.syncCubemap( %oldCubemap ); 
   
   MaterialEditorGui.changeCubemap( %newCubemap );
}

function MaterialEditorGui::saveCubemapDialogCancel( %this )
{ 
   %cubemap = MaterialEditorGui.currentCubemap;
   %idx = matEd_cubemapEd_availableCubemapList.findItemText( %cubemap.getName() );
   matEd_cubemapEd_availableCubemapList.clearSelection();
   matEd_cubemapEd_availableCubemapList.setSelected( %idx, true );
}

function MaterialEditorGui::changeCubemap( %this, %cubemap )
{  
   MaterialEditorGui.setCubemapNotDirty();
   MaterialEditorGui.currentCubemap = %cubemap;
   
   notDirtyCubemap.originalName = %cubemap.getName();
   MaterialEditorGui.copyCubemaps( %cubemap, notDirtyCubemap);
   MaterialEditorGui.copyCubemaps( %cubemap, matEdCubeMapPreviewMat);
   MaterialEditorGui.syncCubemap( %cubemap );  
}

function MaterialEditorGui::editCubemapImage( %this, %face )
{   
   MaterialEditorGui.setCubemapDirty();
   
   %cubemap = MaterialEditorGui.currentCubemap;
   %bitmap = MaterialEditorGui.openFile("texture");
   if( %bitmap !$= "" && %bitmap !$= "tools/materialEditor/gui/cubemapBtnBorder" )
   {
      %cubemap.cubeFace[%face] = %bitmap;
      MaterialEditorGui.copyCubemaps( %cubemap, matEdCubeMapPreviewMat);
      MaterialEditorGui.syncCubemap( %cubemap );
   }
}

function MaterialEditorGui::editCubemapName( %this, %newName )
{   
   MaterialEditorGui.setCubemapDirty();
   
   %cubemap = MaterialEditorGui.currentCubemap;
   
   %idx = matEd_cubemapEd_availableCubemapList.findItemText( %cubemap.getName() );
   matEd_cubemapEd_availableCubemapList.setItemText( %idx, %newName );   
   %cubemap.setName(%newName);
   
   MaterialEditorGui.syncCubemap( %cubemap );
}

function MaterialEditorGui::syncCubemap( %this, %cubemap )
{
   %xpos = MaterialEditorGui.searchForTexture(%cubemap.getName(), %cubemap.cubeFace[0]);
   if( %xpos !$= "" )
      matEd_cubemapEd_XPos.setBitmap( %xpos );
   
   %xneg = MaterialEditorGui.searchForTexture(%cubemap.getName(), %cubemap.cubeFace[1]);
   if( %xneg !$= "" )
      matEd_cubemapEd_XNeg.setBitmap( %xneg );      
   
   %yneg = MaterialEditorGui.searchForTexture(%cubemap.getName(), %cubemap.cubeFace[2]);
   if( %yneg !$= "" )
      matEd_cubemapEd_YNeG.setBitmap( %yneg );
   
   %ypos = MaterialEditorGui.searchForTexture(%cubemap.getName(), %cubemap.cubeFace[3]);
   if( %ypos !$= "" )
      matEd_cubemapEd_YPos.setBitmap( %ypos );
   
   %zpos = MaterialEditorGui.searchForTexture(%cubemap.getName(), %cubemap.cubeFace[4]);
   if( %zpos !$= "" )
      matEd_cubemapEd_ZPos.setBitmap( %zpos );
   
   %zneg = MaterialEditorGui.searchForTexture(%cubemap.getName(), %cubemap.cubeFace[5]);
   if( %zneg !$= "" )
      matEd_cubemapEd_ZNeg.setBitmap( %zneg );
   
   matEd_cubemapEd_activeCubemapNameTxt.setText(%cubemap.getName());
   
   %cubemap.updateFaces();
   matEdCubeMapPreviewMat.updateFaces();
}

function MaterialEditorGui::copyCubemaps( %this, %copyFrom, %copyTo)
{
   %copyTo.cubeFace[0] = %copyFrom.cubeFace[0];
   %copyTo.cubeFace[1] = %copyFrom.cubeFace[1];
   %copyTo.cubeFace[2] = %copyFrom.cubeFace[2];
   %copyTo.cubeFace[3] = %copyFrom.cubeFace[3];
   %copyTo.cubeFace[4] = %copyFrom.cubeFace[4];
   %copyTo.cubeFace[5] = %copyFrom.cubeFace[5];
}


//==============================================================================
// showSaveDialog logic

function MaterialEditorGui::showSaveDialog( %this, %toMaterial )
{
   MessageBoxYesNoCancel("Save Material?", 
      "The material " @ MaterialEditorGui.currentMaterial.getName() @ " has unsaved changes. <br>Do you want to save?", 
      "MaterialEditorGui.saveDialogSave(" @ %toMaterial @ ");", 
      "MaterialEditorGui.saveDialogDontSave(" @ %toMaterial @ ");", 
      "MaterialEditorGui.saveDialogCancel();" );
}

function MaterialEditorGui::showMaterialChangeSaveDialog( %this, %toMaterial )
{
   %fromMaterial = MaterialEditorGui.currentMaterial;
   
   MessageBoxYesNoCancel("Save Material?", 
      "The material " @ %fromMaterial.getName() @ " has unsaved changes. <br>Do you want to save before changing the material?", 
      "MaterialEditorGui.saveDialogSave(" @ %toMaterial @ "); MaterialEditorGui.changeMaterial(" @ %fromMaterial @ ", " @ %toMaterial @ ");", 
      "MaterialEditorGui.saveDialogDontSave(" @ %toMaterial @ "); MaterialEditorGui.changeMaterial(" @ %fromMaterial @ ", " @ %toMaterial @ ");", 
      "MaterialEditorGui.saveDialogCancel();" );
}

/*
function MaterialEditorGui::showCreateNewMaterialSaveDialog( %this, %toMaterial )
{
   MessageBoxYesNoCancel("Save Material?", 
      "The material " @ MaterialEditorGui.currentMaterial.getName() @ " has unsaved changes. <br>Do you want to save before changing the material?", 
      "MaterialEditorGui.save(); MaterialEditorGui.createNewMaterial(" @ %toMaterial @ ");", 
      "MaterialEditorGui.saveDialogDontSave(" @ %toMaterial @ "); MaterialEditorGui.changeMaterial(" @ %toMaterial @ ");", 
      "MaterialEditorGui.saveDialogCancel();" );
}
*/

function MaterialEditorGui::saveDialogCancel( %this )
{
   MaterialEditorGui.guiSync( materialEd_previewMaterial );
}

function MaterialEditorGui::saveDialogDontSave( %this, %material )
{
   MaterialEditorGui.currentMaterial.setName( %this.originalName );
   
   //restore to defaults
   MaterialEditorGui.copyMaterials( notDirtyMaterial, MaterialEditorGui.currentMaterial );
   MaterialEditorGui.copyMaterials( notDirtyMaterial, materialEd_previewMaterial );
   MaterialEditorGui.guiSync( materialEd_previewMaterial );
   
   materialEd_previewMaterial.flush();
   materialEd_previewMaterial.reload();
   MaterialEditorGui.currentMaterial.flush();
   MaterialEditorGui.currentMaterial.reload();
   
   MaterialEditorGui.setMaterialNotDirty();
   
   MaterialEditorGui.setActiveMaterial( %material );
}

function MaterialEditorGui::saveDialogSave( %this, %material )
{
   MaterialEditorGui.save();
   MaterialEditorGui.setActiveMaterial( %material );
}

function MaterialEditorGui::save( %this )
{
   if( MaterialEditorGui.currentMaterial.getName() $= "" )
   {
      MessageBoxOK("Cannot perform operation", "Saved materials cannot be named \"\". A name must be given before operation is performed" );
      return;   
   }
   
   // Update the live object regardless in this case
   MaterialEditorGui.updateLivePreview(true);
   
   %currentMaterial = MaterialEditorGui.currentMaterial;
   if( %currentMaterial == -1 )
   {
      MessageBoxOK("Cannot perform operation", "Could not locate material" );
      return;
   }

   // Specifically for materials autogenerated from shapes.
   if( %currentMaterial.isAutoGenerated() ) 
      %currentMaterial.setAutoGenerated( false ); 
     
   // Save the material using the persistence manager
   matEd_PersistMan.saveDirty();   
   
   // Clean up the Material Editor
   MaterialEditorGui.copyMaterials( materialEd_previewMaterial, notDirtyMaterial );
   MaterialEditorGui.setMaterialNotDirty();
}

//==============================================================================
// Create New and Delete Material

function MaterialEditorGui::createNewMaterial( %this )
{
   %action = %this.createUndo(ActionCreateNewMaterial, "Create New Material");
   %action.object = "";
   
   %material = getUniqueName( "newMaterial" );
   new Material(%material) 
   {
      diffuseMap[0] = "core/art/warnmat";
      mapTo = "unmapped_mat";
      parentGroup = RootGroup;
   };
   
   %action.newMaterial = %material.getId();
   %action.oldMaterial = MaterialEditorGui.currentMaterial;
   
   MaterialEditorGui.submitUndo( %action );
   
   MaterialEditorGui.currentObject = "";
   MaterialEditorGui.setMode();
   MaterialEditorGui.prepareActiveMaterial( %material.getId(), true );
}

function MaterialEditorGui::deleteMaterial( %this )
{
   %action = %this.createUndo(ActionDeleteMaterial, "Delete Material");
   %action.object = MaterialEditorGui.currentObject;
   %action.currentMode = MaterialEditorGui.currentMode;
   
   /*
   if( MaterialEditorGui.currentMode $= "Mesh" )
   {
      %materialTarget = SubMaterialSelector.text;
      %action.materialTarget = %materialTarget;   
      
      //create the stub material
      %toMaterial = getUniqueName( "newMaterial" );
      new Material(%toMaterial) 
      {
         diffuseMap[0] = "core/art/warnmat";
         mapTo = "unmapped_mat";
         parentGroup = RootGroup;
      };
         
      %action.toMaterial = %toMaterial.getId();   
      %action.fromMaterial = MaterialEditorGui.currentMaterial;
      %action.fromMaterialOldFname = MaterialEditorGui.currentMaterial.getFilename();
   }
   else
   {
      // Grab first material we see; if theres not one, create one
      %toMaterial = MaterialSet.getObject(0);
      if( !isObject( %toMaterial ) )
      {
         %toMaterial = getUniqueName( "newMaterial" );
         new Material(%toMaterial) 
         {
            diffuseMap[0] = "core/art/warnmat";
            mapTo = "unmapped_mat";
            parentGroup = RootGroup;
         };    
      }
   
      %action.toMaterial = %toMaterial.getId();
      %action.fromMaterial = MaterialEditorGui.currentMaterial;
   }
   */
   
   // Grab first material we see; if theres not one, create one
   %newMaterial = getUniqueName( "newMaterial" );
   new Material(%newMaterial) 
   {
      diffuseMap[0] = "core/art/warnmat";
      mapTo = "unmapped_mat";
      parentGroup = RootGroup;
   };

   // Setup vars
   %action.newMaterial = %newMaterial.getId();
   %action.oldMaterial = MaterialEditorGui.currentMaterial;
   %action.oldMaterialFname = MaterialEditorGui.currentMaterial.getFilename();
   
   // Submit undo
   MaterialEditorGui.submitUndo( %action );
   
   // Delete the material from file
   if( !MaterialEditorGui.isMatEditorMaterial( MaterialEditorGui.currentMaterial ) )
   {
      matEd_PersistMan.removeObjectFromFile(MaterialEditorGui.currentMaterial);
      matEd_PersistMan.removeDirty(MaterialEditorGui.currentMaterial);
   }
   
   // Delete the material as seen through the material selector.
   UnlistedMaterials.add( "unlistedMaterials", MaterialEditorGui.currentMaterial.getName() );
   
   // Loadup another material
   MaterialEditorGui.currentObject = "";
   MaterialEditorGui.setMode();
   MaterialEditorGui.prepareActiveMaterial( %newMaterial.getId(), true );
}

//==============================================================================
// Clear and Refresh Material

function MaterialEditorGui::clearMaterial(%this)
{
   %action = %this.createUndo(ActionClearMaterial, "Clear Material");
   %action.material = MaterialEditorGui.currentMaterial;
   %action.object = MaterialEditorGui.currentObject;
   
   pushInstantGroup();
   %action.oldMaterial = new Material();
   %action.newMaterial = new Material();   
   popInstantGroup();
   
   MaterialEditorGui.submitUndo( %action );
   
   MaterialEditorGui.copyMaterials( MaterialEditorGui.currentMaterial, %action.oldMaterial );
   
   %tempMat = new Material()
                  {
                     name = "tempMaterial";
                     mapTo = "unmapped_mat";
                     parentGroup = RootGroup;
                  };
   
   MaterialEditorGui.copyMaterials( %tempMat, materialEd_previewMaterial );
   MaterialEditorGui.guiSync( materialEd_previewMaterial );
      
   materialEd_previewMaterial.flush();
   materialEd_previewMaterial.reload();
   
   if (MaterialEditorGui.livePreview == true)
   {
      MaterialEditorGui.copyMaterials( %tempMat, MaterialEditorGui.currentMaterial );
      MaterialEditorGui.currentMaterial.flush();
      MaterialEditorGui.currentMaterial.reload();
   }
   
   MaterialEditorGui.setMaterialDirty();
   
   %tempMat.delete();
}

function MaterialEditorGui::refreshMaterial(%this)
{
   %action = %this.createUndo(ActionRefreshMaterial, "Refresh Material");
   %action.material = MaterialEditorGui.currentMaterial;
   %action.object = MaterialEditorGui.currentObject;
   
   pushInstantGroup();
   %action.oldMaterial = new Material();
   %action.newMaterial = new Material();   
   popInstantGroup();
   
   MaterialEditorGui.copyMaterials( MaterialEditorGui.currentMaterial, %action.oldMaterial );
   MaterialEditorGui.copyMaterials( notDirtyMaterial, %action.newMaterial );
   
   %action.oldName = MaterialEditorGui.currentMaterial.getName();
   %action.newName = %this.originalName;
   
   MaterialEditorGui.submitUndo( %action );
   
   MaterialEditorGui.currentMaterial.setName( %this.originalName );
   MaterialEditorGui.copyMaterials( notDirtyMaterial, materialEd_previewMaterial );
   MaterialEditorGui.guiSync( materialEd_previewMaterial );
      
   materialEd_previewMaterial.flush();
   materialEd_previewMaterial.reload();
   
   if (MaterialEditorGui.livePreview == true)
   {
      MaterialEditorGui.copyMaterials( notDirtyMaterial, MaterialEditorGui.currentMaterial );
      MaterialEditorGui.currentMaterial.flush();
      MaterialEditorGui.currentMaterial.reload();
   }
   
   MaterialEditorGui.setMaterialNotDirty();
}

//==============================================================================
// Switching and Changing Materials

function MaterialEditorGui::switchMaterial( %this, %material )
{
   //MaterialEditorGui.currentMaterial = %material.getId();
   MaterialEditorGui.currentObject = "";
   MaterialEditorGui.setMode();
   MaterialEditorGui.prepareActiveMaterial( %material.getId(), true );
}

/*------------------------------------------------------------------------------
 This changes the map to's of possibly two materials (%fromMaterial, %toMaterial)
 and updates the engines libraries accordingly in order to make this change per 
 object/per objects instances/per target. Before this functionality is enacted, 
 there is a popup beforehand that will ask if you are sure if you want to make
 this change. Making this change will physically alter possibly two materials.cs 
 files in order to move the (%fromMaterial, %toMaterial), replacing the 
 (%fromMaterials)'s mapTo to "unmapped_mat".
-------------------------------------------------------------------------------*/

function MaterialEditorGui::changeMaterial(%this, %fromMaterial, %toMaterial)
{
   %action = %this.createUndo(ActionChangeMaterial, "Change Material");
   %action.object = MaterialEditorGui.currentObject;
   
   %materialTarget = SubMaterialSelector.text;
   %action.materialTarget = %materialTarget;   
   
   %action.fromMaterial = %fromMaterial;    
   %action.toMaterial = %toMaterial;   
   %action.toMaterialOldFname = %toMaterial.getFilename();
   %action.object =  MaterialEditorGui.currentObject;

   if( MaterialEditorGui.currentMeshMode $= "Model" ) // Models
   {
      %action.mode = "model";      
      
      MaterialEditorGui.currentObject.changeMaterial( %materialTarget, %fromMaterial.getName(), %toMaterial.getName() );
      
      if( MaterialEditorGui.currentObject.shapeName !$= "" ) 
         %sourcePath = MaterialEditorGui.currentObject.shapeName;
      else if( MaterialEditorGui.currentObject.isMethod("getDatablock") )
      {
         if( MaterialEditorGui.currentObject.getDatablock().shapeFile !$= "" )
            %sourcePath = MaterialEditorGui.currentObject.getDatablock().shapeFile;
      }
      
      // Creating "to" path
      %k = 0;
      while( strpos( %sourcePath, "/", %k ) != -1 )
      {
         %count = strpos( %sourcePath, "/", %k );
         %k = %count + 1;
      }
      %fileName = getSubStr( %sourcePath , 0 , %k );
      %fileName = %fileName @ "materials.cs";
      
      %action.toMaterialNewFname = %fileName;
      
      MaterialEditorGui.prepareActiveMaterial( %toMaterial, true );
      if( !MaterialEditorGui.isMatEditorMaterial( %toMaterial ) )
      {
         matEd_PersistMan.removeObjectFromFile(%toMaterial);
      }
      
      matEd_PersistMan.setDirty(%fromMaterial);
      matEd_PersistMan.setDirty(%toMaterial, %fileName);
      matEd_PersistMan.saveDirty();
      
      matEd_PersistMan.removeDirty(%fromMaterial);
      matEd_PersistMan.removeDirty(%toMaterial);
   }
   else // EditorShapes
   {
      %action.mode = "editorShapes";

      eval("MaterialEditorGui.currentObject." @ SubMaterialSelector.getText() @ " = " @ %toMaterial.getName() @ ";");
      if( MaterialEditorGui.currentObject.isMethod("postApply") )
         MaterialEditorGui.currentObject.postApply();
      
      MaterialEditorGui.prepareActiveMaterial( %toMaterial, true );
   }
   
   MaterialEditorGui.submitUndo( %action );
}

//==============================================================================
// Image thumbnail right-clicks.

// not yet functional
function MaterialEditorMapThumbnail::onRightClick( %this )
{
   if( !isObject( "MaterialEditorMapThumbnailPopup" ) )
      new PopupMenu( MaterialEditorMapThumbnailPopup )
      {
         superClass = "MenuBuilder";
         isPopup = true;
         
         item[ 0 ] = "Open File" TAB "" TAB "openFile( %this.filePath );";
         item[ 1 ] = "Open Folder" TAB "" TAB "openFolder( filePath( %this.filePath ) );";
         
         filePath = "";
      };
      
   // Find the text control containing the filename.
   
   %textCtrl = %this.parentGroup.findObjectByInternalName( %this.fileNameTextCtrl, true );
   if( !%textCtrl )
      return;
      
   %fileName = %textCtrl.getText();
   %fullPath = makeFullPath( %fileName, getMainDotCsDir() );
      
   // Construct a full path.
   
   %isValid = isFile( %fullPath );
   if( !%isValid )
   {
      if( isFile( %fileName ) )
      {
         %fullPath = %fileName;
         %isValid = true;
      }
      else
      {
         // Try material-relative path.
         
         %material = MaterialEditorGui.currentMaterial;
         if( isObject( %material ) )
         {
            %materialPath = filePath( makeFullPath( %material.getFilename(), getMainDotCsDir() ) );
            %fullPath = makeFullPath( %fileName, %materialPath );
            %isValid = isFile( %fullPath );
         }
      }
   }
   
   %popup = MaterialEditorMapThumbnailPopup;
   %popup.enableItem( 0, %isValid );
   %popup.enableItem( 1, %isValid );
   %popup.filePath = %fullPath;
   
   %popup.showPopup( Canvas );
}

// Accumulation
function MaterialEditorGui::updateAccuCheckbox(%this, %value)
{
   MaterialEditorGui.updateActiveMaterial("accuEnabled[" @ MaterialEditorGui.currentLayer @ "]", %value);   
   MaterialEditorGui.guiSync( materialEd_previewMaterial );
}
