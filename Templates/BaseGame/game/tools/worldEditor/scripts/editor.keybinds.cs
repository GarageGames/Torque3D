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

if ( isObject( editorMoveMap ) )
   editorMoveMap.delete();
   
new ActionMap(editorMoveMap);

//------------------------------------------------------------------------------
// Non-remapable binds
//------------------------------------------------------------------------------
editorMoveMap.bindCmd(keyboard, "escape", "", "Canvas.pushDialog(PauseMenu);");

//------------------------------------------------------------------------------
// Movement Keys
//------------------------------------------------------------------------------
editorMoveMap.bind( keyboard, a, editorMoveleft );
editorMoveMap.bind( keyboard, d, editorMoveright );
editorMoveMap.bind( keyboard, left, editorMoveleft );
editorMoveMap.bind( keyboard, right, editorMoveright );

editorMoveMap.bind( keyboard, w, editorMoveforward );
editorMoveMap.bind( keyboard, s, editorMovebackward );
editorMoveMap.bind( keyboard, up, editorMoveforward );
editorMoveMap.bind( keyboard, down, editorMovebackward );

editorMoveMap.bind( keyboard, e, editorMoveup );
editorMoveMap.bind( keyboard, c, editorMovedown );

editorMoveMap.bind( mouse, xaxis, editorYaw );
editorMoveMap.bind( mouse, yaxis, editorPitch );

//------------------------------------------------------------------------------
// Mouse Trigger
//------------------------------------------------------------------------------
editorMoveMap.bind( mouse, button0, editorClick );
editorMoveMap.bind( mouse, button1, editorRClick );

//------------------------------------------------------------------------------
// Camera & View functions
//------------------------------------------------------------------------------
editorMoveMap.bind(keyboard, "alt c", toggleCamera);

//------------------------------------------------------------------------------
// Helper Functions
//------------------------------------------------------------------------------
editorMoveMap.bind(keyboard, "F8", dropCameraAtPlayer);
editorMoveMap.bind(keyboard, "F7", dropPlayerAtCamera);

//------------------------------------------------------------------------------
// Debugging Functions
//------------------------------------------------------------------------------
GlobalActionMap.bind(keyboard, "ctrl F2", showMetrics);
GlobalActionMap.bind(keyboard, "ctrl F3", doProfile);

//------------------------------------------------------------------------------
// Misc.
//------------------------------------------------------------------------------
GlobalActionMap.bind(keyboard, "tilde", toggleConsole);

editorMoveMap.bind( mouse, "alt zaxis", editorWheelFadeScroll );
