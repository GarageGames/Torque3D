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

function EditorLightingMenu::onAdd( %this )
{
   Parent::onAdd( %this );
   
   // Get the light manager names.
   %lightManagers = getLightManagerNames();

   // Where we gonna insert them?
   %this.lmFirstIndex = %this.getItemCount();
   
   // Add the light mangers to the lighting menu.
   %count = getFieldCount( %lightManagers );
   for ( %i = 0; %i < %count; %i++ )
   {      
      %lm = getField( %lightManagers, %i );

      // Store a reverse lookup of the light manager
      // name to the menu index... used in onMenuSelect.
      %index = %this.lmFirstIndex + %i;
      %this.lmToIndex[ %lm ] = %index;      
            
      // The command just sets the light manager.
      %cmd = "setLightManager(\"" @ %lm @ "\"); $pref::lightManager = \"" @ %lm @ "\";";
      
      // Add it.
      %this.addItem( %index, %lm TAB "" TAB %cmd );
   }
   
   // Store for later in EditorLightingMenu.
   %this.lmLastIndex = %index;
}

function EditorLightingMenu::onMenuSelect( %this )
{
   %lm = getActiveLightManager();
   %index = %this.lmToIndex[ %lm ];
   %this.checkRadioItem( %this.lmFirstIndex, %this.lmLastIndex, %index );
   
   //%selSize = EWorldEditor.getSelectionSize();
   %this.enableItem( 1, true /*%selSize == 1*/ );   
}
