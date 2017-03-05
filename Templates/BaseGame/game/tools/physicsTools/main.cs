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


function physicsToggleSimulation()
{
   %isEnabled = physicsSimulationEnabled();
   if ( %isEnabled )
   {
      physicsStateText.setText( "Simulation is paused." );
      physicsStopSimulation( "client" );
      physicsStopSimulation( "server" );
   }
   else
   {
      physicsStateText.setText( "Simulation is unpaused." );
      physicsStartSimulation( "client" );
      physicsStartSimulation( "server" );   
   }
}

function initializePhysicsTools()
{
   echo( " % - Initializing Physics Tools" );

   if ( !physicsPluginPresent() )
   {
      echo( "No physics plugin exists." );
      return;
   }
      
   globalactionmap.bindCmd( keyboard, "alt t", "physicsToggleSimulation();", "" );      
   globalactionmap.bindCmd( keyboard, "alt r", "physicsRestoreState();", "" );   
      
   new ScriptObject( PhysicsEditorPlugin )
   {
      superClass = "EditorPlugin";
      editorGui = EWorldEditor;
   };
}

function destroyPhysicsTools()
{
}

function PhysicsEditorPlugin::onWorldEditorStartup( %this )
{      
   new PopupMenu( PhysicsToolsMenu )
   {
      superClass = "MenuBuilder";
      //class = "PhysXToolsMenu";

      barTitle = "Physics";
                                 
      item[0] = "Start Simulation" TAB "Ctrl-Alt P" TAB "physicsStartSimulation( \"client\" );physicsStartSimulation( \"server\" );";         
      //item[1] = "Stop Simulation" TAB "" TAB "physicsSetTimeScale( 0 );";
      item[1] = "-";
      item[2] = "Speed 25%" TAB "" TAB "physicsSetTimeScale( 0.25 );";
      item[3] = "Speed 50%" TAB "" TAB "physicsSetTimeScale( 0.5 );";
      item[4] = "Speed 100%" TAB "" TAB "physicsSetTimeScale( 1.0 );";
      item[5] = "-";
      item[6] = "Reload NXBs" TAB "" TAB "";
   };
      
   // Add our menu.
   EditorGui.menuBar.insert( PhysicsToolsMenu, EditorGui.menuBar.dynamicItemInsertPos );
         
   // Add ourselves to the window menu.
   //EditorGui.addToWindowMenu( "Road and Path Editor", "", "RoadEditor" );   
}

function PhysicsToolsMenu::onMenuSelect(%this)
{
   %isEnabled = physicsSimulationEnabled();

   %itemText = !%isEnabled ? "Start Simulation" : "Pause Simulation";
   %itemCommand = !%isEnabled ? "physicsStartSimulation( \"client\" );physicsStartSimulation( \"server\" );" : "physicsStopSimulation( \"client\" );physicsStopSimulation( \"server\" );";
   
   %this.setItemName( 0, %itemText );
   %this.setItemCommand( 0, %itemCommand );
}

function PhysicsEditorPlugin::onEditorWake( %this )
{
   // Disable physics when entering
   // the editor.  Will be re-enabled
   // when the editor is closed.
   physicsStopSimulation( "client" );
   physicsStopSimulation( "server" );
   physicsRestoreState(); 
}

function PhysicsEditorPlugin::onEditorSleep( %this )
{
   physicsStoreState();   
   
   %currentTimeScale = physicsGetTimeScale();
   if ( %currentTimeScale == 0.0 )
      physicsSetTimeScale( 1.0 );
      
   physicsStartSimulation( "client" );   
   physicsStartSimulation( "server" );
}
