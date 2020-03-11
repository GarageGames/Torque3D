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
/// Returns Projects API's EventManager Singleton
///
function Projects::GetEventManager()
{
   if( !isObject( $_Tools::ProjectEventManager ) )
      $_Tools::ProjectEventManager = new EventManager() { queue = "ProjectEventManager"; };
      
   return $_Tools::ProjectEventManager;
}


function Projects::DeclareProjectTarget( %projectTargetNamespace, %objectGlobalName )
{
   // At some point it would be nice to have a console method
   // on SimObject that supported validating that another object
   // implemented all the methods provided by a given namespace.
   // .validateInterface("myNamespace") or some such.
   %projectObject = new ScriptMsgListener( %objectGlobalName ) 
   { 
      class = %projectTargetNamespace; 
      superclass = ProjectBase; 
   };   
}

///
/// Public Project Events
///

/// ProjectOpened
///
/// is fired when a project has been opened and all bootstrap
/// processing has occured on the project object.  
/// At this point it is safe for addons to do post-load processing
/// such as creating new create entries and other specific modifications
/// to the editor.
Projects::GetEventManager().registerEvent( "ProjectOpened" );

/// ProjectClosed
///
/// is fired when a project is about to be closed and it's 
/// resources destroyed by the base project class.  Addons
/// should use this event to free any project specific resources
/// they have allocated, as well as saving of data where applicable.
Projects::GetEventManager().registerEvent( "ProjectClosed" );

/// ProjectDeploy 
///
/// is fired when a game is about to be run from the editor and on 
/// this event addons and third party's should without scheduling or 
/// other delaying calls, deploy any game data that the game will need
/// to it's game path.
/// 
/// Example, the core package zip code intercepts this message and
/// builds and deploys a new core.zip if is necessary
Projects::GetEventManager().registerEvent( "ProjectDeploy" );

/// Currently Unused
Projects::GetEventManager().registerEvent( "ProjectFileAdded" );
/// Currently Unused
Projects::GetEventManager().registerEvent( "ProjectFileRemoved" );

///
/// ProjectOpen Event Handler
/// - %data is the project object to be opened
function ProjectBase::onProjectOpen( %this, %data )
{
   error("onProjectOpen Handler not implemented for class -" SPC %this.class );
}

///
/// ProjectClose Event Handler
///
function ProjectBase::onProjectClose( %this, %data )
{
   error("onProjectClose Handler not implemented for class -" SPC %this.class );
}

///
/// ProjectAddFile Event Handler
///
function ProjectBase::onProjectAddFile( %this, %data )
{
   error("onProjectAddFile Handler not implemented for class -" SPC %this.class );
}

///
/// ProjectRemoveFile Event Handler
///
function ProjectBase::onProjectRemoveFile( %this, %data )
{
   error("onProjectRemoveFile Handler not implemented for class -" SPC %this.class );
}
