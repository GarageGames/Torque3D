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
   
function ConvexEditorGui::onWake( %this )
{   
	convexEditorToolbar-->gridSnapSizeEdit.setText(%this.getGridSnapSize());
	
	if(ConvexEditorOptionssWindow-->matPreviewBtn.bitmap $= "")
	{
		//no active material, so set one	
		ConvexEditorOptionssWindow-->matPreviewBtn.setText("");
		
		%mat = EditorSettings.Value("ConvexEditor/MaterialName");
		
		ConvexEditorOptionssWindow-->matPreviewBtn.setBitmap(%mat.diffuseMap[0]);
		
		ConvexEditorOptionssWindow.activeMaterial = %mat;
	}
}

function ConvexEditorGui::onSleep( %this )
{
}

function ConvexEditorGui::createConvexBox( %this )
{
   %obj = genericCreateObject( "ConvexShape" );
   %this.handleDeselect();
   %this.selectConvex( %obj );
   %this.dropSelectionAtScreenCenter();
}

function ConvexEditorGui::onSelectionChanged( %this, %shape, %face )
{
   //echo( "onSelectionChanged: " @ %shape SPC %face );
   
   ConvexEditorSplitFaceBtn.setActive( false );
   ConvexEditorSplitFaceBtn.ToolTip = "Split selected face [Disabled]" NL "Use Ctrl + Rotate instead for more control";
   ConvexEditorDeleteFaceBtn.setActive( false );
   ConvexEditorDeleteFaceBtn.ToolTip = "Delete selection [Disabled] (Delete)"; 
   
   if ( !isObject( %shape ) )  
   {
      ConvexEditorOptionssWindow-->defMatPreviewBtn.setText("No Brush Selected");
      ConvexEditorOptionssWindow.activeShape = "";
      return;   
   }
      
   ConvexEditorDeleteFaceBtn.setActive( true );

   ConvexEditorOptionssWindow-->defMatPreviewBtn.setText("");
   ConvexEditorOptionssWindow-->defMatPreviewBtn.setBitmap(%shape.material.diffuseMap[0]);

   ConvexEditorOptionssWindow.activeShape = %shape;
      
   if ( %face == -1 )     
   {
      ConvexEditorDeleteFaceBtn.ToolTip = "Delete selected ConvexShape (Delete)";
	
	  ConvexEditorOptionssWindow-->UOffset.setText("");
	  ConvexEditorOptionssWindow-->VOffset.setText("");
	
	  ConvexEditorOptionssWindow-->UScale.setText("");
	  ConvexEditorOptionssWindow-->VScale.setText("");
	
	  ConvexEditorOptionssWindow-->ZRotation.setText("");
   }
   else
   {
      ConvexEditorDeleteFaceBtn.ToolTip = "Delete selected Face (Delete)";
      
      ConvexEditorSplitFaceBtn.ToolTip = "Split selected face" NL "Use Ctrl + Rotate instead for more control";
      ConvexEditorSplitFaceBtn.setActive( true );
	
	  %UVOffset = %this.getSelectedFaceUVOffset();
	
	  ConvexEditorOptionssWindow-->UOffset.setText(%UVOffset.x);
	  ConvexEditorOptionssWindow-->VOffset.setText(%UVOffset.y);
	
	  %UVScale = %this.getSelectedFaceUVScale();
	
      ConvexEditorOptionssWindow-->UScale.setText(%UVScale.x);
      ConvexEditorOptionssWindow-->VScale.setText(%UVScale.y);
	
	  ConvexEditorOptionssWindow-->ZRotation.setText(ConvexEditorGui.getSelectedFaceZRot());
   }
}

function ConvexEditorUVFld::onReturn(%this)
{
   EWorldEditor.isDirty = true;

   %offset = "0 0";
   %offset.x = ConvexEditorOptionssWindow-->UOffset.getText();
   %offset.y = ConvexEditorOptionssWindow-->VOffset.getText();

   %scale = "0 0";
   %scale.x = ConvexEditorOptionssWindow-->UScale.getText();
   %scale.y = ConvexEditorOptionssWindow-->VScale.getText();

   %rot = ConvexEditorOptionssWindow-->ZRotation.getText();

   ConvexEditorGui.setSelectedFaceUVOffset(%offset);
   ConvexEditorGui.setSelectedFaceUVScale(%scale);
   ConvexEditorGui.setSelectedFaceZRot(%rot);
}

function ConvexEditorUVHorzFlipBtn::onClick(%this)
{
	EWorldEditor.isDirty = true;
   %current = ConvexEditorGui.getSelectedFaceHorzFlip();
   ConvexEditorGui.setSelectedFaceHorzFlip(!%current);
}

function ConvexEditorUVVertFlipBtn::onClick(%this)
{
	EWorldEditor.isDirty = true;
   %current = ConvexEditorGui.getSelectedFaceVertFlip();
   ConvexEditorGui.setSelectedFaceVertFlip(!%current);
}

function ConvexEditorMaterialBtn::onClick(%this)
{
	%this.getMaterialName();
}

function ConvexEditorMaterialBtn::getMaterialName(%this)
{
   materialSelector.showDialog(%this @ ".gotMaterialName", "name");
}

function ConvexEditorMaterialBtn::gotMaterialName(%this, %name)
{
   //eval(%this.object @ "." @ %this.targetField @ " = " @ %name @ ";");
   //%this.object.changeMaterial(getTrailingNumber(%this.targetField), %name);
   //%this.object.inspectorApply();
   %diffusemap = %name.diffuseMap[0];

   ConvexEditorOptionssWindow-->matPreviewBtn.setBitmap(%diffusemap);

   ConvexEditorOptionssWindow.activeMaterial = %name;
}

function ConvexEditorMaterialApplyBtn::onClick(%this)
{
	EWorldEditor.isDirty = true;
    ConvexEditorGui.setSelectedFaceMaterial(ConvexEditorOptionssWindow.activeMaterial);
    ConvexEditorGui.updateShape();
}

function ConvexEditorMaterialLiftBtn::onClick(%this)
{
   %mat = ConvexEditorGui.getSelectedFaceMaterial();
   ConvexEditorOptionssWindow.activeMaterial = %mat;
   ConvexEditorOptionssWindow-->matPreviewBtn.setBitmap(%mat.diffuseMap[0]);
}

function ConvexEditorMaterialResetBtn::onClick(%this)
{
	EWorldEditor.isDirty = true;
    ConvexEditorGui.setSelectedFaceMaterial(ConvexEditorOptionssWindow.activeShape.material);
    ConvexEditorGui.updateShape();
}

function ConvexEditorGui::toggleGridSnap(%this)
{
	%this.toggleGridSnapping();
}

function ConvexEditorGridSnapSizeFld::onReturn(%this)
{
   ConvexEditorGui.setGridSnapSize(%this.getText());
}

function ConvexEditorDefaultMaterialBtn::onClick(%this)
{
	%this.getMaterialName();
}

function ConvexEditorDefaultMaterialBtn::getMaterialName(%this)
{
   materialSelector.showDialog(%this @ ".gotMaterialName", "name");
}

function ConvexEditorDefaultMaterialBtn::gotMaterialName(%this, %name)
{
   //eval(%this.object @ "." @ %this.targetField @ " = " @ %name @ ";");
   //%this.object.changeMaterial(getTrailingNumber(%this.targetField), %name);
   //%this.object.inspectorApply();
   %diffusemap = %name.diffuseMap[0];

   ConvexEditorOptionssWindow-->defMatPreviewBtn.setBitmap(%diffusemap);

   ConvexEditorOptionssWindow.activeShape.material = %name;

   ConvexEditorGui.updateShape();
}