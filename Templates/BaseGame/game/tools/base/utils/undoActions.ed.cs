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

// Undo actions that are useful in multiple editors.


//=============================================================================================
//    Undo reparenting.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function UndoActionReparentObjects::create( %treeView )
{
   pushInstantGroup();
   %action = new UndoScriptAction()
   {
      class = "UndoActionReparentObjects";
      numObjects = 0;
      treeView = %treeView;
   };
   popInstantGroup();
   
   return %action;
}

//---------------------------------------------------------------------------------------------

function UndoActionReparentObjects::add( %this, %object, %oldParent, %newParent )
{
   %index = %this.numObjects;
   
   %this.objects[ %index ] = %object;
   %this.oldParents[ %index ] = %oldParent;
   %this.newParents[ %index ] = %newParent;
   
   %this.numObjects = %this.numObjects + 1;
}

//---------------------------------------------------------------------------------------------

function UndoActionReparentObjects::undo( %this )
{
   %numObjects = %this.numObjects;
   for( %i = 0; %i < %numObjects; %i ++ )
   {
      %obj = %this.objects[ %i ];
      %group = %this.oldParents[ %i ];
      
      if( isObject( %obj ) && isObject( %group ) )
         %obj.parentGroup = %group;
   }
   
   if( isObject( %this.treeView ) )
      %this.treeView.update();
}

//---------------------------------------------------------------------------------------------

function UndoActionReparentObjects::redo( %this )
{
   %numObjects = %this.numObjects;
   for( %i = 0; %i < %numObjects; %i ++ )
   {
      %obj = %this.objects[ %i ];
      %group = %this.newParents[ %i ];
      
      if( isObject( %obj ) && isObject( %group ) )
         %obj.parentGroup = %group;
   }

   if( isObject( %this.treeView ) )
      %this.treeView.update();
}
