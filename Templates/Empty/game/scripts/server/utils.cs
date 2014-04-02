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


// Landing all shapes and prefabs to the terrain.
// # Objects which marked as 'locked' we are skipping.
function Utils::landing( %group ) {

    if (%group $= "") {
        %group = "MissionGroup";
    }

    for (%i = 0; %i < %group.getCount(); %i++) {
        %obj = %group.getObject( %i );
        if ( !isObject( %obj ) ) { continue; }

        %cn = %obj.getClassName();
        if ( (%cn $= "SimGroup") || (%cn $= "SceneGroup") ) {
            //echo( "\nBegin processing " @ %cn @ " '" @ %obj.name @ "'..." );
			%oldName = %obj.name;
            if (%obj.name $= "") {
                %obj.name = "n" @ %obj.getID();
            }
            Utils::landing( %obj.name );
			%obj.name = %oldName;
            //echo( "End processing the group '" @ %obj.name @ "'.\n" );
            continue;
        }

        %skip = %obj.locked;
        %hasPrefab   = (%cn $= "Prefab");
        %hasTSStatic = (%cn $= "TSStatic");
        %need = !%skip && (%hasPrefab || %hasTSStatic);
        //echo( %cn @ " -> " @ (%need ? "Process..." : "Skip.") );
        if ( !%need ) { continue; }

        %x = %obj.position.x;
        %y = %obj.position.y;
        %h = getTerrainHeight( %x SPC %y );
        %newPos = %obj.position.x SPC %obj.position.y SPC %h;

        %obj.position = %newPos;

        //echo( %cn @ " " @ %obj.getID() @ " is landing to " @ %newPos @ "." );

    } // for (%i = 0; ...

    EWorldEditor.isDirty = true;
}
