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

function ForestBrushTool::onActivated( %this )
{
   ForestEditToolbar.setVisible( true );
   %this.syncBrushToolbar();
}

function ForestBrushTool::onDeactivated( %this )
{
   ForestEditToolbar.setVisible( false );
}

function ForestBrushTool::syncBrushToolbar( %this )
{
   %size = %this.size;
   ForestBrushSizeSliderCtrlContainer->slider.setValue( %size );
   ForestBrushSizeTextEditContainer-->textEdit.setValue( %size );
      
   %pres = %this.pressure;   
   ForestBrushPressureSliderCtrlContainer->slider.setValue( %pres );
   ForestBrushPressureTextEditContainer-->textEdit.setValue( mCeil(100 * %pres) @ "%" );
   
   %hard = %this.hardness;
   ForestBrushHardnessSliderCtrlContainer->slider.setValue( %hard );
   ForestBrushHardnessTextEditContainer-->textEdit.setValue( mCeil(100 * %hard) @ "%"); 
}

function ForestBrushTool::onMouseDown( %this )
{
   ForestEditTabBook.selectPage( 0 );
}

function ForestSelectionTool::onActivated( %this )
{
}

function ForestSelectionTool::onDeactivated( %this )
{
   %this.clearSelection();
}