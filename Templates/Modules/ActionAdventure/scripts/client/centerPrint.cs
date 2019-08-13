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

$centerPrintActive = 0;
$bottomPrintActive = 0;

// Selectable window sizes
$CenterPrintSizes[1] = 20;
$CenterPrintSizes[2] = 36;
$CenterPrintSizes[3] = 56;

// time is specified in seconds
function clientCmdCenterPrint( %message, %time, %size )
{
   // if centerprint already visible, reset text and time.
   if ($centerPrintActive) {   
      if (centerPrintDlg.removePrint !$= "")
         cancel(centerPrintDlg.removePrint);
   }
   else {
      CenterPrintDlg.visible = 1;
      $centerPrintActive = 1;
   }

   CenterPrintText.setText( "<just:center>" @ %message );
   CenterPrintDlg.extent = firstWord(CenterPrintDlg.extent) @ " " @ $CenterPrintSizes[%size];
   
   if (%time > 0)
      centerPrintDlg.removePrint = schedule( ( %time * 1000 ), 0, "clientCmdClearCenterPrint" );
}

// time is specified in seconds
function clientCmdBottomPrint( %message, %time, %size )
{
   // if bottomprint already visible, reset text and time.
   if ($bottomPrintActive) {   
      if( bottomPrintDlg.removePrint !$= "")
         cancel(bottomPrintDlg.removePrint);
   }
   else {
      bottomPrintDlg.setVisible(true);
      $bottomPrintActive = 1;
   }
   
   bottomPrintText.setText( "<just:center>" @ %message );
   bottomPrintDlg.extent = firstWord(bottomPrintDlg.extent) @ " " @ $CenterPrintSizes[%size];

   if (%time > 0)
      bottomPrintDlg.removePrint = schedule( ( %time * 1000 ), 0, "clientCmdClearbottomPrint" );
}

function BottomPrintText::onResize(%this, %width, %height)
{
   %this.position = "0 0";
}

function CenterPrintText::onResize(%this, %width, %height)
{
   %this.position = "0 0";
}

//-------------------------------------------------------------------------------------------------------

function clientCmdClearCenterPrint()
{
   $centerPrintActive = 0;
   CenterPrintDlg.visible = 0;
   CenterPrintDlg.removePrint = "";
}

function clientCmdClearBottomPrint()
{
   $bottomPrintActive = 0;
   BottomPrintDlg.visible = 0;
   BottomPrintDlg.removePrint = "";
}
