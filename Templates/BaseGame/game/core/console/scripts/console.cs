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

GlobalActionMap.bind("keyboard", "tilde", "toggleConsole");

function ConsoleEntry::eval()
{
   %text = trim(ConsoleEntry.getValue());
   if(%text $= "")
      return;

   // If it's missing a trailing () and it's not a variable,
   // append the parentheses.
   if(strpos(%text, "(") == -1 && !isDefined(%text)) {
      if(strpos(%text, "=") == -1 && strpos(%text, " ") == -1) {
         if(strpos(%text, "{") == -1 && strpos(%text, "}") == -1) {
            %text = %text @ "()";
         }
      }
   }

   // Append a semicolon if need be.
   %pos = strlen(%text) - 1;
   if(strpos(%text, ";", %pos) == -1 && strpos(%text, "}") == -1) {
      %text = %text @ ";";
   }

   // Turn off warnings for assigning from void
   // and evaluate the snippet.
   if(!isDefined("$Con::warnVoidAssignment"))
      %oldWarnVoidAssignment = true;
   else
      %oldWarnVoidAssignment = $Con::warnVoidAssignment;
   $Con::warnVoidAssignment = false;

   echo("==>" @ %text);
   if(   !startsWith(%text, "function ")
       && !startsWith(%text, "datablock ")
       && !startsWith(%text, "foreach(")
       && !startsWith(%text, "foreach$(")
       && !startsWith(%text, "if(")
       && !startsWith(%text, "while(")
       && !startsWith(%text, "for(")
       && !startsWith(%text, "switch(")
       && !startsWith(%text, "switch$("))
      eval("%result = " @ %text);
   else
      eval(%text);
   $Con::warnVoidAssignment = %oldWarnVoidAssignment;

   ConsoleEntry.setValue("");

   // Echo result.
   if(%result !$= "")
      echo(%result);
}

function ToggleConsole(%make)
{
   if (%make) {
      if (ConsoleDlg.isAwake()) {
         // Deactivate the console.
         Canvas.popDialog(ConsoleDlg);
      } else {
         Canvas.pushDialog(ConsoleDlg, 99);         
      }
   }
}

function ConsoleDlg::hideWindow(%this)
{
   %this-->Scroll.setVisible(false);
}

function ConsoleDlg::showWindow(%this)
{
   %this-->Scroll.setVisible(true);
}

function ConsoleDlg::onWake(%this)
{
   ConsoleDlgErrorFilterBtn.setStateOn(ConsoleMessageLogView.getErrorFilter());
   ConsoleDlgWarnFilterBtn.setStateOn(ConsoleMessageLogView.getWarnFilter());
   ConsoleDlgNormalFilterBtn.setStateOn(ConsoleMessageLogView.getNormalFilter());
   
   ConsoleMessageLogView.refresh();
}

function ConsoleDlg::setAlpha( %this, %alpha)
{
   if (%alpha $= "")
      ConsoleScrollProfile.fillColor = $ConsoleDefaultFillColor;
   else
      ConsoleScrollProfile.fillColor = getWords($ConsoleDefaultFillColor, 0, 2) SPC %alpha * 255.0;
}

function ConsoleDlgErrorFilterBtn::onClick(%this)
{
   ConsoleMessageLogView.toggleErrorFilter();
}

function ConsoleDlgWarnFilterBtn::onClick(%this)
{
  
   ConsoleMessageLogView.toggleWarnFilter();
}

function ConsoleDlgNormalFilterBtn::onClick(%this)
{
   ConsoleMessageLogView.toggleNormalFilter();
}

function ConsoleMessageLogView::onNewMessage(%this, %errorCount, %warnCount, %normalCount)
{
   ConsoleDlgErrorFilterBtn.setText("(" @ %errorCount @ ") Errors");
   ConsoleDlgWarnFilterBtn.setText("(" @ %warnCount @ ") Warnings");
   ConsoleDlgNormalFilterBtn.setText("(" @ %normalCount @ ") Messages");
}