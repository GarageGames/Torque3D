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

#ifndef _UNDO_H_
#define _UNDO_H_

#ifndef _SIMOBJECT_H_
#include "console/simObject.h"
#endif
#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif

class UndoManager;

///
class UndoAction : public SimObject
{
   friend class UndoManager;

protected:
   // The manager this was added to.
   UndoManager* mUndoManager;

public:

   /// A brief description of the action, for display in menus and the like.
   // not private because we're exposing it to the console.
   String mActionName;

   // Required in all ConsoleObject subclasses.
   typedef SimObject Parent;
   DECLARE_CONOBJECT(UndoAction);
   static void initPersistFields();
   
   /// Create a new action, assigning it a name for display in menus et cetera.
   UndoAction(const UTF8 *actionName = " ");
   virtual ~UndoAction();

   /// Implement these methods to perform your specific undo & redo tasks. 
   virtual void undo() { };
   virtual void redo() { };
   
   /// Adds the action to the undo stack of the default UndoManager, or the provided manager.
   void addToManager(UndoManager* theMan = NULL);
};

/// An undo action that is comprised of other undo actions.
class CompoundUndoAction : public UndoAction
{
   friend class UndoManager;

protected:

   Vector< UndoAction* > mChildren;

public:

   typedef UndoAction Parent;

   CompoundUndoAction( const UTF8 *actionName = " " );
   virtual ~CompoundUndoAction();

   DECLARE_CONOBJECT(CompoundUndoAction);

   virtual void addAction( UndoAction *action );
   virtual void undo();
   virtual void redo();
   
   virtual void onDeleteNotify( SimObject* object );
   
   U32 getNumChildren() const { return mChildren.size(); }
};

///
class UndoManager : public SimObject
{
private:
   /// Default number of undo & redo levels.
   const static U32 kDefaultNumLevels = 100;

   /// The stacks of undo & redo actions. They will be capped at size mNumLevels.
   Vector<UndoAction*> mUndoStack;
   Vector<UndoAction*> mRedoStack;
   
   /// Stack for assembling compound actions.
   Vector< CompoundUndoAction* > mCompoundStack;
   
   /// Deletes all the UndoActions in a stack, then clears it.
   void clearStack(Vector<UndoAction*> &stack);
   /// Clamps a Vector to mNumLevels entries.
   void clampStack(Vector<UndoAction*> &stack);

   /// Run the removal logic on the action.
   void doRemove( UndoAction* action, bool noDelete );
   
public:
   /// Number of undo & redo levels.
   // not private because we're exposing it to the console.
   U32 mNumLevels;

   // Required in all ConsoleObject subclasses.
   typedef SimObject Parent;
   DECLARE_CONOBJECT(UndoManager);
   static void initPersistFields();

   /// Constructor. If levels = 0, we use the default number of undo levels.
   UndoManager(U32 levels = kDefaultNumLevels);
   /// Destructor. deletes and clears the undo & redo stacks.
   ~UndoManager();
   /// Accessor to the default undo manager singleton. Creates one if needed.
   static UndoManager& getDefaultManager();
   
   /// Undo last action, and put it on the redo stack.
   void undo();
   /// Redo the last action, and put it on the undo stack.
   void redo();
   
   /// Clears the undo and redo stacks.
   void clearAll();

   /// Returns the printable name of the top actions on the undo & redo stacks.
   const char* getNextUndoName();
   const char* getNextRedoName();

   S32 getUndoCount();
   S32 getRedoCount();

   const char* getUndoName(S32 index);
   const char* getRedoName(S32 index);

   UndoAction* getUndoAction(S32 index);
   UndoAction* getRedoAction(S32 index);

   /// Add an action to the top of the undo stack, and clear the redo stack.
   void addAction(UndoAction* action);
   void removeAction(UndoAction* action, bool noDelete = false);
   
   /// @name Compound Actions
   ///
   /// The compound action stack allows to redirect undos to a CompoundUndoAction
   /// and thus assemble multi-operation undos directly through the UndoManager.
   /// When the bottom-most CompoundUndoAction is popped off the stack, the compound
   /// will be moved onto the undo stack.
   ///
   /// @{
   
   /// Push a compound action called "name" onto the compound stack.  While the
   /// compound stack is not empty, all undos that are queued on the undo manager will
   /// go to the topmost compound instead of the undo stack.
   CompoundUndoAction* pushCompound( const String& name );
   
   /// Pop the topmost compound off the compound stack and add it to the undo manager.
   /// If the compound stack is still not empty, the compound will be added to the next
   /// lower compound on the stack.  Otherwise it will be recorded as a regular undo.
   void popCompound( bool discard = false );
   
   /// Return the current nesting depth of the compound stack.
   U32 getCompoundStackDepth() const { return mCompoundStack.size(); }
      
   /// @}
};


/// Script Undo Action Creation
/// 
/// Undo actions can be created in script like this:
/// 
/// ...
/// %undo = new UndoScriptAction() { class = SampleUndo; actionName = "Sample Undo"; };
/// %undo.addToManager(UndoManager);
/// ...
/// 
/// function SampleUndo::undo()
/// {
///    ...
/// }
/// 
/// function SampleUndo::redo()
/// {
///    ...
/// }
/// 
class UndoScriptAction : public UndoAction
{
public:
   typedef UndoAction Parent;

   UndoScriptAction() : UndoAction()
   {
   }

   virtual void undo() { Con::executef(this, "undo"); };
   virtual void redo() { Con::executef(this, "redo"); }

   virtual bool onAdd()
   {
      // Let Parent Do Work.
      if(!Parent::onAdd())
         return false;


      // Notify Script.
      if(isMethod("onAdd"))
         Con::executef(this, "onAdd");

      // Return Success.
      return true;
   };

   virtual void onRemove()
   {
      if (mUndoManager)
         mUndoManager->removeAction((UndoAction*)this, true);

      // notify script
      if(isMethod("onRemove"))
         Con::executef(this, "onRemove");

      Parent::onRemove();
   }

   DECLARE_CONOBJECT(UndoScriptAction);
};

#endif // _UNDO_H_
