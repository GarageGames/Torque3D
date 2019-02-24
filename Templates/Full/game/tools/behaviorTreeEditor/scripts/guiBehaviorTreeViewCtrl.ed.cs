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

function GuiBehaviorTreeViewCtrl::getRootNode(%this)
{
   %rootId = %this.getFirstRootItem(); 
   %rootObj = %rootId ? %this.getItemValue(%rootId) : -1;
   return %rootObj;
}

function GuiBehaviorTreeViewCtrl::getUndoManager( %this )
{
   if( !isObject( %this.undoManager ) )
   {
      %this.undoManager = new UndoManager(){ numLevels=20; };
      %this.undoManager.setClassNamespace("BTEditorUndoManager");
   }

   return %this.undoManager;
}

function GuiBehaviorTreeViewCtrl::onRemove(%this)
{
   if(isObject(%this.undoManager))
      %this.undoManager.delete();
}

function GuiBehaviorTreeViewCtrl::expandAll(%this)
{
   for(%i=1; %i<=%this.getItemCount(); %i++)
   {
      %this.expandItem(%i);
      %this.buildVisibleTree();
   }
}

function GuiBehaviorTreeViewCtrl::collapseAll(%this)
{
   for(%i=1; %i<=%this.getItemCount(); %i++)
      %this.expandItem(%i, false);
   %this.buildVisibleTree();
}

function GuiBehaviorTreeViewCtrl::refresh(%this)
{
   %root = %this.getRootNode();
   %this.open(%root);
   %this.expandAll();
   %this.selectItem(1);
}

//==============================================================================
// DELETE
//==============================================================================
// onDeleteSelection is called prior to deleting the selected object. 
function GuiBehaviorTreeViewCtrl::onDeleteSelection(%this)
{
   if(%this.getSelectedItem() > 1) // not root
      BTDeleteUndoAction::submit(%this.getSelectedObject());
   
   %this.clearSelection();
   
   BTEditorStatusBar.print( "Node deleted" );
}

//==============================================================================
// SELECT
//==============================================================================
function GuiBehaviorTreeViewCtrl::onSelect(%this, %item)
{
   BehaviorTreeInspector.inspect(%item);
}

function GuiBehaviorTreeViewCtrl::onBeginReparenting(%this)
{
   if( isObject( %this.reparentUndoAction ) )
      %this.reparentUndoAction.delete();
      
   %action = BTReparentUndoAction::create( %this );
   %action.node = %this.getSelectedObject();
   %parent = %action.node.getGroup();
   %action.oldPosition = 0;
   
   if(isObject(%parent))
      %action.oldPosition = %parent.getObjectIndex(%action.node);

   %this.reparentUndoAction = %action;
}

function GuiBehaviorTreeViewCtrl::onReparent(%this, %item, %old, %new)
{
   if( !isObject(%this.reparentUndoAction) ||
       %this.reparentUndoAction.node != %item )
   {
      warn( "Reparenting undo is borked :(" );
      if(isObject(%this.reparentUndoAction))
      {
         %this.reparentUndoAction.delete();
         %this.reparentUndoAction="";
      }
   }
   else
   {       
      %this.reparentUndoAction.oldParent = %old;
      %this.reparentUndoAction.newParent = %new;
      %this.reparentUndoAction.newPosition = %new.getObjectIndex(%item);
   }
}

function GuiBehaviorTreeViewCtrl::onEndReparenting( %this )
{
   %action = %this.reparentUndoAction;
   %this.reparentUndoAction = "";
   
   // Check that the reparenting went as planned, and undo it right now if not
   if(%action.node.getGroup() != %action.newParent)
   {
      %action.undo();
      %action.delete();
   }
   else
   {
      %action.addToManager( %this.getUndoManager() );
      BTEditorStatusBar.print( "Moved node" );
   }
}

function GuiBehaviorTreeViewCtrl::canAdd(%this, %obj, %target)
{
   if(!isObject(%target))
      return false;
      
   if( !%target.isMemberOfClass( "SimGroup" ) )
      return false;
      
   return %target.acceptsAsChild(%obj);
}

function GuiBehaviorTreeViewCtrl::isValidDragTarget(%this, %id, %obj)
{
   %selObj = %this.getSelectedObject();
   
   if(!%selObj)
      return false;
   
   return %this.canAdd(%selObj, %obj);
}

function GuiBehaviorTreeViewCtrl::onMouseUp(%this, %itemid, %count)
{
   %this.mouseDragging=true;  
}

function GuiBehaviorTreeViewCtrl::onRightMouseDown(%this, %item, %pos, %obj)
{
   //echo("onRightMouseDown" TAB %item TAB %pos TAB %obj);

   %selObj = %this.getSelectedObject();
   if(!isObject(%selObj))
      return;
   
   %this.mouseDragging=false;

   if(isObject(BTEditPopup))
      BTEditPopup.delete();
   
   // construct the popup
   %popup = new PopupMenu(BTEditPopup) {
      superClass = "MenuBuilder";
      isPopup = true;
   };
   
   // current node
   %popup.addItem(0, %selObj.getClassName() SPC "[" @ %selObj.internalName @ "]" TAB "" TAB "");
   %popup.enableItem(0, false);
   
   // divider
   %popup.addItem(1, "-");
   
   // add node submenu
   %popup.addItem(2, "Add node" TAB %this.makePopup(false, %selObj) TAB "");
   %popup.enableItem(2, %this.validate(false, %selObj));

   // insert node submenu
   %popup.addItem(3, "Insert node" TAB %this.makePopup(true, %selObj) TAB "");
   %popup.enableItem(3, %this.validate(true, %selObj));
   
   // divider
   %popup.addItem(4, "-");
   
   // delete node
   %popup.addItem(5, "Delete node" TAB "" TAB %this @ ".deleteSelection();");
   
   // excise node (delete single node, reparenting children)
   %popup.addItem(6, "Excise node" TAB "" TAB %this @ ".exciseSelection();");
   %popup.enableItem(6, (%this.getSelectedObject().getCount() == 1) &&
                        (%this.getSelectedObject() != %this.getRootNode()) );
   
   BTEditPopup.showPopup( Canvas );
}

function GuiBehaviorTreeViewCtrl::onRightMouseUp(%this, %item, %pos, %obj)
{
   //echo("onRightMouseUp" TAB %item TAB %pos TAB %obj);
}

function GuiBehaviorTreeViewCtrl::makePopup(%this, %insert, %selObj)
{
   %popup = new PopupMenu() {
      superClass = "MenuBuilder";
      isPopup = true;
   };
   
   for(%i=0; %i<BTNodeTypes.getCount(); %i++)
   {
      %nodeGroup = BTNodeTypes.getObject(%i);
      %type = %nodeGroup.internalName;
      %submenu = new PopupMenu() {
         superClass = "MenuBuilder";
         isPopup=true;
      };
      
      %popup.addItem(%i, %type TAB %submenu TAB "");
      %enabled = %this.validate(%insert, %selObj, %type);
      %popup.enableItem(%i, %enabled);
      
      if(%enabled)
      {
         for(%j=0; %j<%nodeGroup.getCount(); %j++)
         {
            %nodeType = %nodeGroup.getObject(%j).nodeType;
            %cmd = %this @ "." @ (%insert ? "insert" : "add") @ "Node(" @ %selObj @ "," @ %nodeType @ ");";
            %submenu.addItem(%j, %nodeType TAB "" TAB %cmd);
         }
      }
   }
   
   return %popup;
}

function GuiBehaviorTreeViewCtrl::validate(%this, %insert, %selObj, %newType)
{
   %selObjClass = %selObj.getClassName();
   %selObjBaseType = BTEditor.getBaseNodeType(%selObjClass);
   %selObjHasChild = %selObjBaseType !$= "Leaf" ? %selObj.getCount() > 0 : false;
   %newTypeBaseType = BTEditor.getBaseNodeType(%newType);
   
   if(!%insert)
   {
      return !( (%selObjClass $= "Root" && %selObjHasChild) ||
                (%selObjBaseType $= "Leaf") ||
                (%selObjBaseType $= "Decorator" && %selObjHasChild) );
   }
   else
   {
      return !( (%selObjClass $= "Root") ||
                (%newTypeBaseType $= "Leaf") );  
   }
}


function GuiBehaviorTreeViewCtrl::addNode(%this, %selObj, %type)
{
   pushInstantGroup(%selObj);
   %newNode = new(%type)();
   popInstantGroup();
   
   %this.expandItem(%this.findItemByObjectId(%selObj.getId()));   
   %this.buildVisibleTree();
      
   if(isObject(%newNode))
      BTCreateUndoAction::submit( %newNode );
   
   // have to delay selection a bit
   %this.schedule(10, selectItem, %this.findItemByObjectId(%newNode.getId()));
}

function GuiBehaviorTreeViewCtrl::insertNode(%this, %selObj, %type)
{
   // steps are
   // 1)  Remove selected object from tree
   // 2)  Create the new object as a child of the selected object's parent
   // 3)  Move the new object into the correct position
   // 4)  Make the new object the parent of the old object

   // need a compound undo for this process   
   %this.getUndoManager().pushCompound( "Insert new" SPC %type );
   
   // step 1
   // remove selected object from tree (add it to BehaviorTreeGroup)
   %parent = %selObj.getGroup();
   %pos = %parent.getObjectIndex(%selObj);
   BehaviorTreeGroup.add(%selObj);
   
   // Undo for step 1
   %action = BTReparentUndoAction::create(%this);
   %action.node = %selObj;   
   %action.oldPosition = %pos;
   %action.oldParent = %parent;
   %action.newParent = BehaviorTreeGroup;
   %action.newPosition = 0;
   %action.addToManager(%this.getUndoManager());
   
   // Step 2
   // Create new object as child of selected object's parent
   pushInstantGroup(%parent);
   %newNode = new(%type)();
   popInstantGroup();
   
   // undo for step 2
   if(isObject(%newNode))
      BTCreateUndoAction::submit( %newNode );
   
   // step 3
   // move new object to correct position
   if(%pos < %parent.getCount() - 1)
   {
      %parent.reorderChild(%newNode, %parent.getObject(%pos));
      
      // undo for step 3
      %action = BTReparentUndoAction::create(%this);
      %action.node = %newNode;
      %action.oldPosition = 0;
      %action.oldParent = %parent;
      %action.newParent = %parent;
      %action.newPosition = %pos;
      %action.addToManager(%this.getUndoManager());
   }
   
   // step 4
   // Make the new object the parent of the old object
   %newNode.add(%selObj);
   
   // undo for step 4
   %action = BTReparentUndoAction::create(%this);
   %action.node = %selObj;
   %action.oldPosition = 0;
   %action.oldParent = BehaviorTreeGroup;
   %action.newParent = %newNode;
   %action.newPosition = 0;
   %action.addToManager(%this.getUndoManager());
   
   // finalize the compound undo   
   %this.getUndoManager().popCompound();
   
   // re-draw 
   %this.refresh();
}

// remove a single node from within the hierarchy, reparenting its children
function GuiBehaviorTreeViewCtrl::exciseSelection(%this)
{
   // steps are:
   // 1) remove the selected node from the tree
   // 2) re-parent the selected node's child to the old parent
   // 3) delete the selected node
   %selObj = %this.getSelectedObject();
   if(%selObj.getCount() != 1 || %selObj == %this.getRootNode())
   {
      warn("Excision got messy");
      return;
   }
   
   %child = %selObj.getObject(0);
   %parent = %selObj.getGroup();
   %pos = %parent.getObjectIndex(%selObj);
   
   // need a compound undo for this process   
   %this.getUndoManager().pushCompound( "Excise" SPC %type );
   
   // step 1
   // remove selected object from tree (add it to BehaviorTreeGroup)
   BehaviorTreeGroup.add(%selObj);
   
   // Undo for step 1
   %action = BTReparentUndoAction::create(%this);
   %action.node = %selObj;   
   %action.oldPosition = %pos;
   %action.oldParent = %parent;
   %action.newParent = BehaviorTreeGroup;
   %action.newPosition = 0;
   %action.addToManager(%this.getUndoManager());
   
   // step 2
   // re-parent the selected node's child to the old parent
   %parent.add(%child);
   if(%pos < %parent.getCount() - 1)
   {
      %parent.reorderChild(%child, %parent.getObject(%pos));
   }
   
   // undo for step 2
   %action = BTReparentUndoAction::create(%this);
   %action.node = %child;   
   %action.oldPosition = 0;
   %action.oldParent = %selObj;
   %action.newParent = %parent;
   %action.newPosition = %pos;
   %action.addToManager(%this.getUndoManager());
   
   // step 3 
   // delete the selected node
   %action = new BTDeleteUndoAction();
   %action.deleteObject( %selObj );
   %action.addToManager(%this.getUndoManager());
   
   // finalize the compound undo   
   %this.getUndoManager().popCompound();
   
   // re-draw 
   %this.refresh();
   
   %this.selectItem(%this.findItemByObjectId(%child.getId()));
}