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

///
/// Internal Project Events
///
Projects::GetEventManager().registerEvent( "_ProjectCreate" );
Projects::GetEventManager().registerEvent( "_ProjectOpen" );
Projects::GetEventManager().registerEvent( "_ProjectClose" );
Projects::GetEventManager().registerEvent( "_ProjectAddFile" );
Projects::GetEventManager().registerEvent( "_ProjectRemoveFile" );

/// 
/// Project Context Methods
///

function ProjectBase::isActive( %this ) 
{
   if( Projects::GetEventManager().activeProject == %this.getId() )
      return true;
   else
      return false;
}

function ProjectBase::getActiveProject( %this ) 
{
   return Projects::GetEventManager().activeProject;   
}

function ProjectBase::setActive( %this ) 
{
   %activeProject = %this.getActiveProject();
   
   if( isObject( %activeProject ) )
   {
      // If another is active, properly post a close event for now THEN
      // and only then should we change the .activeProject field on the evtmgr
   }
   
   Projects::GetEventManager().activeProject = %this;
}

function ProjectBase::onAdd( %this )
{
   // Subscribe to base events
   Projects::GetEventManager().subscribe( %this, "_ProjectCreate", "_onProjectCreate" );
   Projects::GetEventManager().subscribe( %this, "_ProjectOpen", "_onProjectOpen" );
   Projects::GetEventManager().subscribe( %this, "_ProjectClose", "_onProjectClose" );
   Projects::GetEventManager().subscribe( %this, "_ProjectAddFile", "_onProjectAddFile" );
   Projects::GetEventManager().subscribe( %this, "_ProjectRemoveFile", "_onProjectRemoveFile" );
   
}

function ProjectBase::onRemove( %this )
{
   // Remove subscriptions to base events
   Projects::GetEventManager().remove( %this, "_ProjectCreate" );
   Projects::GetEventManager().remove( %this, "_ProjectOpen" );
   Projects::GetEventManager().remove( %this, "_ProjectClose" );
   Projects::GetEventManager().remove( %this, "_ProjectAddFile" );
   Projects::GetEventManager().remove( %this, "_ProjectRemoveFile" );
   
}

///
/// Internal ProjectOpen Event Handler
/// - %data is the project file path to be opened
function ProjectBase::_onProjectOpen( %this, %data )
{  
   // Sanity check calling of this 
   if( !%this.isMethod( "onProjectOpen" ) )
   {
      error("Incomplete Project Interface - onProjectOpen method is non-existent!");
      return false;
   }
     
   if( !%this.LoadProject( %data ) )
   {
      messageBox("Unable to Load Project", "The project file you're attempting to open was created with an incompatible version of this software\n\nConversion of 1.1.X projects will be addressed soon, we apologize for the inconvenience.","Ok","Error");

      return false;
   }

   %this.gamePath = filePath( %data );
   %this.projectFile = %data;
      
   %toggle = $Scripts::ignoreDSOs;
   $Scripts::ignoreDSOs = true;
      
   %this.gameResPath = %this.gamePath @ "/*";
         
   // Set current dir to game
   setCurrentDirectory( %this.gamePath );
     
   // Set ^game expando
   setScriptPathExpando("project", %this.gamePath );
   setScriptPathExpando("game", %this.gamePath @ "/game" );
   
   %this.onProjectOpen( %data );
   %this.setActive();
   
   Projects::GetEventManager().postEvent( "ProjectOpened", %this );
   
   $Scripts::ignoreDSOs = %toggle;
   $pref::lastProject = %data;
}

///
/// Internal ProjectClose Event Handler
///
function ProjectBase::_onProjectClose( %this, %data )
{
   
   Projects::GetEventManager().postEvent( "ProjectClosed", %this );
   
   // Sanity check calling of this 
   if( !%this.isMethod( "onProjectClose" ) )
      error("Incomplete Project Interface - onProjectClose method is non-existent!");
   else
      %this.onProjectClose( %data );
   
   // Reset to tools directory
   setCurrentDirectory( getMainDotCsDir() );
   
   // Remove expandos
   removeScriptPathExpando( "game" );
   removeScriptPathExpando( "project" );
}

///
/// Internal ProjectCreate Event Handler (Optionally Inherited by public interface)
///
function ProjectBase::_onProjectCreate( %this, %data )
{
   // Force a write out of the project file
   if( !%this.SaveProject( %data ) )
      return false;
      
   // Sanity check calling of this 
   if( %this.isMethod( "onProjectCreate" ) )
      %this.onProjectCreate( %data );
}


///
/// Internal ProjectAddFile Event Handler
///
function ProjectBase::_onProjectAddFile( %this, %data )
{
   // Sanity check calling of this 
   if( !%this.isMethod( "onProjectAddFile" ) )
      error("Incomplete Project Interface - onProjectAddFile method is non-existent!");
   else
      %this.onProjectAddFile( %data );

}

///
/// Internal ProjectRemoveFile Event Handler
///
function ProjectBase::_onProjectRemoveFile( %this, %data )
{
   // Sanity check calling of this 
   if( !%this.isMethod( "onProjectRemoveFile" ) )
      error("Incomplete Project Interface - onProjectRemoveFile method is non-existent!");
   else
      %this.onProjectRemoveFile( %data );

}
