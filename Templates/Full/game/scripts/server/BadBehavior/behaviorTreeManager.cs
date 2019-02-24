//-----------------------------------------------------------------------------
// Copyright (c) 2014 Guy Allard
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

function BehaviorTreeManager::create()
{
   if(isObject(BehaviorTreeManager))
      BehaviorTreeManager.delete();
      
   if(isObject(ServerGroup))
      pushInstantGroup(ServerGroup);
         
   new ScriptObject(BehaviorTreeManager);
   
   if(isObject(ServerGroup))
      popInstantGroup();
}

function BehaviorTreeManager::onAdd(%this)
{
   if(isObject(BehaviorTreeGroup))
      BehaviorTreeGroup.delete();
   
   if(isObject(ActiveBehaviorTreeGroup))
      ActiveBehaviorTreeGroup.delete();
   
   new SimGroup(BehaviorTreeGroup);
   new SimGroup(ActiveBehaviorTreeGroup);
   
   %this.loadTrees();
}

function BehaviorTreeManager::onRemove(%this)
{
   if(isObject(BehaviorTreeGroup))
      BehaviorTreeGroup.delete();
   
   if(isObject(ActiveBehaviorTreeGroup))
     ActiveBehaviorTreeGroup.delete();
}

function BehaviorTreeManager::loadTrees(%this)
{
   if(!isDirectory("./BehaviorTrees"))
      return;
   
   pushInstantGroup(BehaviorTreeGroup);
   
   %pattern = "./BehaviorTrees/*.cs";   
   %file = findFirstFile( %pattern );
   if ( %file $= "" )
   {
      // Try for DSOs next.
      %pattern = "./BehaviorTrees/*.cs.dso";
      %file = findFirstFile( %pattern );
   }
   
   while( %file !$= "" )
   {      
      exec( %file );
      %file = findNextFile( %pattern );
   }  
   
   popInstantGroup();
}

function BehaviorTreeManager::createTree(%this, %obj, %tree)
{
   if(!isObject(%obj))
   {
      error("BehaviorTreeManager::assignTree - object does not exist");
      return -1;
   }
   
   if(!BehaviorTreeGroup.isMember(%tree))
   {
      error("BehaviorTreeManager::assignTree - tree is not a member of BehaviorTreeGroup");
      return -1;
   }
   
   pushInstantGroup(ActiveBehaviorTreeGroup);
   %behaviorTree = new BehaviorTreeRunner() {
      rootNode = %tree;
      ownerObject = %obj;
   };
   popInstantGroup();
      
   return %behaviorTree;
}

function BehaviorTreeManager::onBehaviorTreeEditor(%this, %val)
{
   if(%val)
      onBehaviorTreeEditorStart();
   else
      onBehaviorTreeEditorStop();
}

// give an object a behavior tree
function SimObject::setBehavior(%this, %tree, %frequency)
{
   if(isObject(%this.behaviorTree))
      %this.behaviorTree.rootNode = %tree;
   else      
      %this.behaviorTree = BehaviorTreeManager.createTree(%this, %tree);

   if(%frequency)   
      %this.behaviorTree.frequency = %frequency;
}


// stop running a behavior tree on an object
function SimObject::clearBehavior(%this)
{
   if(isObject(%this.behaviorTree))
      %this.behaviorTree.clear();
}

BehaviorTreeManager::create();

