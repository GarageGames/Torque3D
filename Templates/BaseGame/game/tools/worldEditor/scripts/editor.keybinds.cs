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

if ( isObject( EditorMap ) )
   EditorMap.delete();
   
new ActionMap(EditorMap);

//------------------------------------------------------------------------------
// Non-remapable binds
//------------------------------------------------------------------------------
EditorMap.bindCmd(keyboard, "escape", "", "Canvas.pushDialog(PauseMenu);");

//------------------------------------------------------------------------------
// Movement Keys
//------------------------------------------------------------------------------
EditorMap.bind( keyboard, a, editorMoveleft );
EditorMap.bind( keyboard, d, editorMoveright );
EditorMap.bind( keyboard, left, editorMoveleft );
EditorMap.bind( keyboard, right, editorMoveright );

EditorMap.bind( keyboard, w, editorMoveforward );
EditorMap.bind( keyboard, s, editorMovebackward );
EditorMap.bind( keyboard, up, editorMoveforward );
EditorMap.bind( keyboard, down, editorMovebackward );

EditorMap.bind( keyboard, e, editorMoveup );
EditorMap.bind( keyboard, c, editorMovedown );

EditorMap.bind( mouse, xaxis, editorYaw );
EditorMap.bind( mouse, yaxis, editorPitch );

//------------------------------------------------------------------------------
// Mouse Trigger
//------------------------------------------------------------------------------
EditorMap.bind( mouse, button0, editorClick );
EditorMap.bind( mouse, button1, editorRClick );

//------------------------------------------------------------------------------
// Camera & View functions
//------------------------------------------------------------------------------
EditorMap.bind(keyboard, "alt c", toggleCamera);

//------------------------------------------------------------------------------
// Helper Functions
//------------------------------------------------------------------------------
EditorMap.bind(keyboard, "F8", dropCameraAtPlayer);
EditorMap.bind(keyboard, "F7", dropPlayerAtCamera);

//------------------------------------------------------------------------------
// Debugging Functions
//------------------------------------------------------------------------------
GlobalActionMap.bind(keyboard, "ctrl F2", showMetrics);
GlobalActionMap.bind(keyboard, "ctrl F3", doProfile);

//------------------------------------------------------------------------------
// Misc.
//------------------------------------------------------------------------------
GlobalActionMap.bind(keyboard, "tilde", toggleConsole);

EditorMap.bind( mouse, "alt zaxis", editorWheelFadeScroll );
