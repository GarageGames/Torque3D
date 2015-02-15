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


function ESnapOptions::onWake( %this )
{
   ESnapOptionsTabTerrain-->NoAlignment.setStateOn(1);
   
   ESnapOptionsTabSoft-->NoAlignment.setStateOn(1);
   ESnapOptionsTabSoft-->RenderSnapBounds.setStateOn(1);
   ESnapOptionsTabSoft-->SnapBackfaceTolerance.setText(EWorldEditor.getSoftSnapBackfaceTolerance());
}

function ESnapOptions::hideDialog( %this )
{
   %this.setVisible(false);
   SnapToBar-->snappingSettingsBtn.setStateOn(false); 
}

function ESnapOptions::ToggleVisibility()
{
   if ( ESnapOptions.visible  )
   {
      ESnapOptions.setVisible(false);
      SnapToBar-->snappingSettingsBtn.setStateOn(false); 
   }
   else
   {
      ESnapOptions.setVisible(true);
      ESnapOptions.selectWindow();
      ESnapOptions.setCollapseGroup(false);
      SnapToBar-->snappingSettingsBtn.setStateOn(true); 
   }
}

function ESnapOptions::setTerrainSnapAlignment( %this, %val )
{
   EWorldEditor.setTerrainSnapAlignment(%val);
}

function ESnapOptions::setSoftSnapAlignment( %this, %val )
{
   EWorldEditor.setSoftSnapAlignment(%val);
}

function ESnapOptions::setSoftSnapSize( %this )
{
   %val = ESnapOptions-->SnapSize.getText();
   
   EWorldEditor.setSoftSnapSize(%val);
   EWorldEditor.syncGui();
}

function ESnapOptions::setGridSnapSize( %this )
{
   %val = ESnapOptions-->GridSize.getText();
   
   EWorldEditor.setGridSize( %val );
}

function ESnapOptions::toggleRenderSnapBounds( %this )
{
   EWorldEditor.softSnapRender( ESnapOptionsTabSoft-->RenderSnapBounds.getValue() );
}

function ESnapOptions::toggleRenderSnappedTriangle( %this )
{
   EWorldEditor.softSnapRenderTriangle( ESnapOptionsTabSoft-->RenderSnappedTriangle.getValue() );
}

function ESnapOptions::getSoftSnapBackfaceTolerance( %this )
{
   %val = ESnapOptions-->SnapBackfaceTolerance.getText();
   
   EWorldEditor.setSoftSnapBackfaceTolerance(%val);
}
