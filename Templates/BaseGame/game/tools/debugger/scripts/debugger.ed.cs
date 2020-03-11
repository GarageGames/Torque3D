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

//---------------------------------------------------------------------------------------------
// onLine is invoked whenever the TCP object receives a line from the server. Treat the first
// word as a "command" and dispatch to an appropriate handler.
//---------------------------------------------------------------------------------------------
function TCPDebugger::onLine(%this, %line)
{
   echo("Got line=>" @ %line);
   %cmd = firstWord(%line);
   %rest = restWords(%line);
   
   if (%cmd $= "PASS") {
      %this.handlePass(%rest);
   }
   else if(%cmd $= "COUT") {
      %this.handleLineOut(%rest);
   }
   else if(%cmd $= "FILELISTOUT") {
      %this.handleFileList(%rest);
   }
   else if(%cmd $= "BREAKLISTOUT") {
      %this.handleBreakList(%rest);
   }
   else if(%cmd $= "BREAK") {
      %this.handleBreak(%rest);
   }
   else if(%cmd $= "RUNNING") {
      %this.handleRunning();
   }
   else if(%cmd $= "EVALOUT") {
      %this.handleEvalOut(%rest);
   }
   else {
      %this.handleError(%line);
   }
}

// Handler for PASS response.
function TCPDebugger::handlePass(%this, %message)
{
   if (%message $= "WrongPass") {
      DebuggerConsoleView.print("Disconnected - wrong password.");   
      %this.disconnect();
   }
   else if(%message $= "Connected.") {
      DebuggerConsoleView.print("Connected.");
      DebuggerStatus.setValue("CONNECTED");
      %this.send("FILELIST\r\n");
   }
}

// Handler for COUT response.
function TCPDebugger::handleLineOut(%this, %line)
{
   DebuggerConsoleView.print(%line);
}

// Handler for FILELISTOUT response.
function TCPDebugger::handleFileList(%this, %line)
{
   DebuggerFilePopup.clear();
   %word = 0;
   while ((%file = getWord(%line, %word)) !$= "") {
      %word++;
      DebuggerFilePopup.add(%file, %word);
   }
}

// Handler for BREAKLISTOUT response.
function TCPDebugger::handleBreakList(%this, %line)
{
   %file = getWord(%line, 0);
   if (%file != $DebuggerFile) {
      return;
   }
   %pairs = getWord(%line, 1);
   %curLine = 1;
   DebuggerFileView.clearBreakPositions();
   
   // Set the possible break positions.
   for (%i = 0; %i < %pairs; %i++) {
      %skip = getWord(%line, %i * 2 + 2);
      %breaks = getWord(%line, %i * 2 + 3);
      %curLine += %skip;
      for (%j = 0; %j < %breaks; %j++) {
         DebuggerFileView.setBreakPosition(%curLine);
         %curLine++;
      }
   }

   // Now set the actual break points.
   for (%i = 0; %i < DebuggerBreakPoints.rowCount(); %i++) {
      %breakText = DebuggerBreakPoints.getRowText(%i);
      %breakLine = getField(%breakText, 0);
      %breakFile = getField(%breakText, 1);
      if (%breakFile == $DebuggerFile) {
         DebuggerFileView.setBreak(%breakLine);
      }
   }
}

// Handler for BREAK response.
function TCPDebugger::handleBreak(%this, %line)
{
   DebuggerStatus.setValue("BREAK");
   
   // Query all the watches.
   for (%i = 0; %i < DebuggerWatchView.rowCount(); %i++) {
      %id = DebuggerWatchView.getRowId(%i);
      %row = DebuggerWatchView.getRowTextById(%id);
      %expr = getField(%row, 0);
      %this.send("EVAL " @ %id @ " 0 " @ %expr @ "\r\n");
   }

   // Update the call stack window.
   DebuggerCallStack.clear();

   %file = getWord(%line, 0);
   %lineNumber = getWord(%line, 1);
   %funcName = getWord(%line, 2);
   
   DbgOpenFile(%file, %lineNumber, true);

   %nextWord = 3;
   %rowId = 0;
   %id = 0;
   while(1) {
      DebuggerCallStack.setRowById(%id, %file @ "\t" @ %lineNumber @ "\t" @ %funcName);
      %id++;
      %file = getWord(%line, %nextWord);
      %lineNumber = getWord(%line, %nextWord + 1);
      %funcName = getWord(%line, %nextWord + 2);
      %nextWord += 3;
      if (%file $= "") {
         break;
      }
   }
}

// Handler for RUNNING response.
function TCPDebugger::handleRunning(%this)
{
   DebuggerFileView.setCurrentLine(-1, true);
   DebuggerCallStack.clear();
   DebuggerStatus.setValue("RUNNING...");
}

// Handler for EVALOUT response.
function TCPDebugger::handleEvalOut(%this, %line)
{
   %id = firstWord(%line);
   %value = restWords(%line);

   // See if it's the cursor watch, or from the watch window.
   if (%id < 0) {
      DebuggerCursorWatch.setText(DebuggerCursorWatch.expr SPC "=" SPC %value);
   }
   else {
      %row = DebuggerWatchView.getRowTextById(%id);
      if (%row $= "") {
         return;
      }
      %expr = getField(%row, 0);
      DebuggerWatchView.setRowById(%id, %expr @ "\t" @ %value);
   }
}

// Handler for unrecognized response.
function TCPDebugger::handleError(%this, %line)
{
   DebuggerConsoleView.print("ERROR - bogus message: " @ %line);
}

// Print a line of response from the server.
function DebuggerConsoleView::print(%this, %line)
{
   %row = %this.addRow(0, %line);
   %this.scrollVisible(%row);
}

// When entry in file list selected, open the file.
function DebuggerFilePopup::onSelect(%this, %id, %text)
{
   DbgOpenFile(%text, 0, false);
}

// When entry on call stack selected, open the file and go to the line.
function DebuggerCallStack::onAction(%this)
{
   %id = %this.getSelectedId();
   if (%id == -1) {
      return;
   }
   %text = %this.getRowTextById(%id);
   %file = getField(%text, 0);
   %line = getField(%text, 1);
   
   DbgOpenFile(%file, %line, %id == 0);
}

// Add a breakpoint at the selected spot, if it doesn't already exist.
function DebuggerBreakPoints::addBreak(%this, %file, %line, %clear, %passct, %expr)
{
   // columns 0 = line, 1 = file, 2 = expr
   %textLine = %line @ "\t" @ %file @ "\t" @ %expr @ "\t" @ %passct @ "\t" @ %clear;
   %selId = %this.getSelectedId();
   %selText = %this.getRowTextById(%selId);
   if ((getField(%selText, 0) $= %line) && (getField(%selText, 1) $= %file)) {
      %this.setRowById(%selId, %textLine);
   }
   else {
      %this.addRow($DbgBreakId, %textLine);
      $DbgBreakId++;
   }
}

// Remove the selected breakpoint.
function DebuggerBreakPoints::removeBreak(%this, %file, %line)
{
   for (%i = 0; %i < %this.rowCount(); %i++) {
      %id = %this.getRowId(%i);
      %text = %this.getRowTextById(%id);
      if ((getField(%text, 0) $= %line) && (getField(%text, 1) $= %file)) {
         %this.removeRowById(%id);
         return;
      }
   }
}

// Remove all breakpoints.
function DebuggerBreakPoints::clearBreaks(%this)
{
   while (%this.rowCount()) {
      %id = %this.getRowId(0);
      %text = %this.getRowTextById(%id);
      %file = getField(%text, 1);
      %line = getField(%text, 0);
      DbgRemoveBreakPoint(%file, %line);
   }
}

// Go to file & line for the selected breakpoint.
function DebuggerBreakPoints::onAction(%this)
{
   %id = %this.getSelectedId();
   if (%id == -1) {
      return;
   }
   %text = %this.getRowTextById(%id);
   %line = getField(%text, 0);
   %file = getField(%text, 1);
   
   DbgOpenFile(%file, %line, false);
}

// Handle breakpoint removal executed from the file-view GUI.
function DebuggerFileView::onRemoveBreakPoint(%this, %line)
{
   %file = $DebuggerFile;
   DbgRemoveBreakPoint(%file, %line);
}

// Handle breakpoint addition executed from the file-view GUI.
function DebuggerFileView::onSetBreakPoint(%this, %line)
{
   %file = $DebuggerFile;
   DbgSetBreakPoint(%file, %line, 0, 0, true);
}

//---------------------------------------------------------------------------------------------
// Various support functions.
//---------------------------------------------------------------------------------------------

// Add a watch expression.
function DbgWatchDialogAdd()
{
   %expr = WatchDialogExpression.getValue();
   if (%expr !$= "") {
      DebuggerWatchView.setRowById($DbgWatchSeq, %expr @"\t(unknown)");
      TCPDebugger.send("EVAL " @ $DbgWatchSeq @ " 0 " @ %expr @ "\r\n");
      $DbgWatchSeq++;
   }
   Canvas.popDialog(DebuggerWatchDlg);
}

// Edit a watch expression.
function DbgWatchDialogEdit()
{
   %newValue = EditWatchDialogValue.getValue();
   %id = DebuggerWatchView.getSelectedId();
   if (%id >= 0) {
      %row = DebuggerWatchView.getRowTextById(%id);
      %expr = getField(%row, 0);
      if (%newValue $= "") {
         %assignment = %expr @ " = \"\"";
      }
      else {
         %assignment = %expr @ " = " @ %newValue;
      }
      TCPDebugger.send("EVAL " @ %id  @ " 0 " @ %assignment @ "\r\n");
   }
   Canvas.popDialog(DebuggerEditWatchDlg);
}

// Set/change the singular "cursor watch" expression.
function DbgSetCursorWatch(%expr)
{
   DebuggerCursorWatch.expr = %expr;
   if (DebuggerCursorWatch.expr $= "") {
      DebuggerCursorWatch.setText("");
   }
   else {
      TCPDebugger.send("EVAL -1 0 " @ DebuggerCursorWatch.expr @ "\r\n");
   }
}

// Connect to the server with the given addr/port/password.
function DbgConnect()
{
   %address = DebuggerConnectAddress.getValue();
   %port = DebuggerConnectPort.getValue();
   %password = DebuggerConnectPassword.getValue();

   if ((%address !$= "" ) && (%port !$= "" ) && (%password !$= "" )) {
      TCPDebugger.connect(%address @ ":" @ %port);
      TCPDebugger.schedule(5000, send, %password @ "\r\n");
      TCPDebugger.password = %password;
   }

   Canvas.popDialog(DebuggerConnectDlg);
}

// Put a condition on a breakpoint.
function DbgBreakConditionSet()
{
   // Read the condition.
   %condition = BreakCondition.getValue();
   %passct = BreakPassCount.getValue();
   %clear = BreakClear.getValue();
   if (%condition $= "") {
      %condition = "true";
   }
   if (%passct $= "") {
      %passct = "0";
   }
   if (%clear $= "") {
      %clear = "false";
   }
   
   // Set the condition.
   %id = DebuggerBreakPoints.getSelectedId();
   if (%id != -1) {
      %bkp = DebuggerBreakPoints.getRowTextById(%id);
      DbgSetBreakPoint(getField(%bkp, 1), getField(%bkp, 0), %clear, %passct, %condition);
   }

   Canvas.popDialog(DebuggerBreakConditionDlg);
}

// Open a file, go to the indicated line, and optionally select the line.
function DbgOpenFile(%file, %line, %selectLine)
{
   if (%file !$= "") {
      // Open the file in the file view.
      if (DebuggerFileView.open(%file)) {
         // Go to the line.
         DebuggerFileView.setCurrentLine(%line, %selectLine);
         // Get the breakpoints for this file.
         if (%file !$= $DebuggerFile) {
            TCPDebugger.send("BREAKLIST " @ %file @ "\r\n");
            $DebuggerFile = %file;
         }
      }
   }
}

// Search in the fileview GUI.
function DbgFileViewFind()
{
   %searchString = DebuggerFindStringText.getValue();
   DebuggerFileView.findString(%searchString);

   Canvas.popDialog(DebuggerFindDlg);
}

// Set a breakpoint, optionally with condition.
function DbgSetBreakPoint(%file, %line, %clear, %passct, %expr)
{
   if (!%clear) {
      if (%file == $DebuggerFile) {
         DebuggerFileView.setBreak(%line);
      }
   }
   DebuggerBreakPoints.addBreak(%file, %line, %clear, %passct, %expr);
   TCPDebugger.send("BRKSET " @ %file @ " " @ %line @ " " @ %clear @ " " @ %passct @ " " @ %expr @ "\r\n");
}

// Remove a breakpoint.
function DbgRemoveBreakPoint(%file, %line)
{
   if (%file == $DebuggerFile) {
      DebuggerFileView.removeBreak(%line);
   }
   TCPDebugger.send("BRKCLR " @ %file @ " " @ %line @ "\r\n");
   DebuggerBreakPoints.removeBreak(%file, %line);
}

// Remove whatever breakpoint is selected in the breakpoints GUI.
function DbgDeleteSelectedBreak()
{
   %selectedBreak = DebuggerBreakPoints.getSelectedId();
   %rowNum = DebuggerBreakPoints.getRowNumById(%selectedWatch);
   if (%rowNum >= 0) {
      %breakText = DebuggerBreakPoints.getRowText(%rowNum);
      %breakLine = getField(%breakText, 0);
      %breakFile = getField(%breakText, 1);
      DbgRemoveBreakPoint(%breakFile, %breakLine);
   }
}

// Send an expression to the server for evaluation.
function DbgConsoleEntryReturn()
{
   %msg = DbgConsoleEntry.getValue();
   if (%msg !$= "") {
      DebuggerConsoleView.print("%" @ %msg);
      if (DebuggerStatus.getValue() $= "NOT CONNECTED") {
         DebuggerConsoleView.print("*** Not connected.");
      }
      else if (DebuggerStatus.getValue() $= "BREAK") {
         DebuggerConsoleView.print("*** Target is in BREAK mode.");
      }
      else {
         TCPDebugger.send("CEVAL " @ %msg @ "\r\n");
      }
   }
   DbgConsoleEntry.setValue("");
}

// Print a line from the server.
function DbgConsolePrint(%status)
{
   DebuggerConsoleView.print(%status);
}

// Delete the currently selected watch expression.
function DbgDeleteSelectedWatch()
{
   %selectedWatch = DebuggerWatchView.getSelectedId();
   %rowNum = DebuggerWatchView.getRowNumById(%selectedWatch);
   DebuggerWatchView.removeRow(%rowNum);
}

// Evaluate all the watch expressions.
function DbgRefreshWatches()
{
   for (%i = 0; %i < DebuggerWatchView.rowCount(); %i++) {
      %id = DebuggerWatchView.getRowId(%i);
      %row = DebuggerWatchView.getRowTextById(%id);
      %expr = getField(%row, 0);
      TCPDebugger.send("EVAL " @ %id @ " 0 " @ %expr @ "\r\n");
   }
}

//---------------------------------------------------------------------------------------------
// Incremental execution functions
// These just send commands to the server.
//---------------------------------------------------------------------------------------------
function dbgStepIn()
{
   TCPDebugger.send("STEPIN\r\n");
}

function dbgStepOut()
{
   TCPDebugger.send("STEPOUT\r\n");
}

function dbgStepOver()
{
   TCPDebugger.send("STEPOVER\r\n");
}

function dbgContinue()
{
   TCPDebugger.send("CONTINUE\r\n");
}
