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

function initLightingSystems()
{
   echo( "\n--------- Initializing Lighting Systems ---------" );

   // First exec the scripts for the different light managers
   // in the lighting folder.
   
   %pattern = "./lighting/*/init.cs";   
   %file = findFirstFile( %pattern );
   if ( %file $= "" )
   {
      // Try for DSOs next.
      %pattern = "./lighting/*/init.cs.dso";
      %file = findFirstFile( %pattern );
   }
   
   while( %file !$= "" )
   {      
      exec( %file );
      %file = findNextFile( %pattern );
   }
   
   // Try the perfered one first.
   %success = setLightManager( $pref::lightManager );
   if ( !%success )
   {
      // The perfered one fell thru... so go thru the default
      // light managers until we find one that works.
      %lmCount = getFieldCount( $lightManager::defaults );
      for ( %i = 0; %i < %lmCount; %i++ )
      {         
         %lmName = getField( $lightManager::defaults, %i );
         %success = setLightManager( %lmName );
         if ( %success )
            break;
      }
   }
   
   // Did we completely fail to initialize a light manager?   
   if ( !%success )
   {
      // If we completely failed to initialize a light 
      // manager then the 3d scene cannot be rendered.
      quitWithErrorMessage( "Failed to set a light manager!" );
   }
      
   echo( "\n" );
}

//---------------------------------------------------------------------------------------------

function onLightManagerActivate( %lmName )
{
   $pref::lightManager = %lmName;
   echo( "Using " @ $pref::lightManager );
   
   // Call activation callbacks.
   
   %activateNewFn = "onActivate" @ getWord( %lmName, 0 ) @ "LM";   
   if( isFunction( %activateNewFn ) )
      eval( %activateNewFn @ "();" );
}

//---------------------------------------------------------------------------------------------

function onLightManagerDeactivate( %lmName )
{
   // Call deactivation callback.
   
   %deactivateOldFn = "onDeactivate" @ getWord( %lmName, 0 ) @ "LM";
   if( isFunction( %deactivateOldFn ) )
      eval( %deactivateOldFn @ "();" );
}
