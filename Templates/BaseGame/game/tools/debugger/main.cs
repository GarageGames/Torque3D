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
// TCP Debugger
// To use the debugger, first call "dbgSetParameters(port, password);" from one instance of
// your game. Then, in another instance (either on the same system, or a different one) call
// "startDebugger();". Then use the gui to connect to the first instance with the port and
// password you first passed to dbgSetParameters.
//---------------------------------------------------------------------------------------------

function initializeDebugger()
{
   echo(" % - Initializing Debugger");
   
   // Load the scripts.
   exec("./scripts/debugger.ed.cs");
   
   // And the guis.
   exec("./gui/breakConditionDlg.ed.gui");
   exec("./gui/connectDlg.ed.gui");
   exec("./gui/editWatchDlg.ed.gui");
   exec("./gui/findDlg.ed.gui");
   exec("./gui/debugger.ed.gui");
   exec("./gui/watchDlg.ed.gui");
}

function destroyDebugger()
{
   if (isObject(TCPDebugger))
      TCPDebugger.delete();
}

function startDebugger()
{
   // Clean up first.
   destroyDebugger();
   
   // Create a TCP object named TCPDebugger.
   new TCPObject(TCPDebugger);
   
   // Used to get unique IDs for breakpoints and watch expressions.
   $DbgBreakId = 0;
   $DbgWatchSeq = 1;
   
   // Set up the GUI.
   DebuggerConsoleView.setActive(false);
   $GameCanvas.pushDialog(DebuggerGui);
}
