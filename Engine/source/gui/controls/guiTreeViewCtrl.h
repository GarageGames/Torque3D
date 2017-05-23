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

#ifndef _GUI_TREEVIEWCTRL_H
#define _GUI_TREEVIEWCTRL_H

#include "core/bitSet.h"
#include "math/mRect.h"
#include "gfx/gFont.h"
#include "gui/core/guiControl.h"
#include "gui/core/guiArrayCtrl.h"


class GuiTextEditCtrl;

//------------------------------------------------------------------------------

class GuiTreeViewCtrl : public GuiArrayCtrl
{
   private:
      typedef GuiArrayCtrl Parent;

   public:
      /// @section GuiControl_Intro Introduction
      /// @nosubgrouping

      ///
      class Item
      {
         public:

            enum ItemState
            {
               Selected       = BIT( 0 ),
               Expanded       = BIT( 1 ),
               Marked         = BIT( 2 ), ///< Marked items are drawn with a border around them. This is
                                        ///  different than "Selected" because it can only be set by script.
               Filtered       = BIT( 3 ), ///< Whether the item is currently filtered out.
               MouseOverBmp   = BIT( 4 ),
               MouseOverText  = BIT( 5 ),
               MouseOverIcon  = BIT( 6 ),
               InspectorData  = BIT( 7 ), ///< Set if we're representing some inspector
                                        /// info (ie, use mInspectorInfo, not mScriptInfo)

               VirtualParent  = BIT( 8 ), ///< This indicates that we should be rendered as
                                        ///  a parent even though we don't have any children.
                                        ///  This is useful for preventing scenarios where
                                        ///  we might want to create thousands of
                                        ///  Items that might never be shown (for instance
                                        ///  if we're browsing the object hierarchy in
                                        ///  Torque, which might have thousands of objects).

               RebuildVisited    = BIT( 9 ), ///< Rebuild traversal for virtual parents has visited and validated this item.
               
               ShowObjectId      = BIT( 10 ),
               ShowClassName     = BIT( 11 ),
               ShowObjectName    = BIT( 12 ),
               ShowInternalName  = BIT( 13 ),
               ShowClassNameForUnnamed = BIT( 14 ),
               ForceItemName = BIT(15),
               ForceDragTarget = BIT(16),
               DenyDrag = BIT(17),
            };

            GuiTreeViewCtrl* mParentControl;
            BitSet32 mState;
            SimObjectPtr< GuiControlProfile > mProfile;
            S16 mId;
            U16 mTabLevel;
            Item* mParent;
            Item* mChild;
            Item* mNext;
            Item* mPrevious;
            String mTooltip;
            S32 mIcon; //stores the icon that will represent the item in the tree
            S32 mDataRenderWidth; /// this stores the pixel width needed
                                  /// to render the item's data in the 
                                  /// onRenderCell function to optimize
                                  /// for speed.

            Item( GuiTreeViewCtrl* parent, GuiControlProfile *pProfile );
            ~Item();

            struct ScriptTag
            {
               S8 mNormalImage;
               S8 mExpandedImage;
               StringTableEntry mText;
               StringTableEntry mValue;
            } mScriptInfo;
            struct InspectorTag
            {
               SimObjectPtr<SimObject> mObject;
            } mInspectorInfo;

            /// @name Get Methods
            /// @{

            ///
            S8 getNormalImage() const;
            S8 getExpandedImage() const;
            StringTableEntry getText();
            StringTableEntry getValue();
            inline const S16 getID() const { return mId; };
            SimObject *getObject();
            U32 getDisplayTextLength();
            S32 getDisplayTextWidth(GFont *font);
            void getDisplayText(U32 bufLen, char *buf);
            bool hasObjectBasedTooltip();
            void getTooltipText(U32 bufLen, char *buf);

            /// @}


            /// @name Set Methods
            /// @{

            /// Set whether an item is expanded or not (showing children or having them hidden)
            void setExpanded( const bool f = true );
            /// Set the image to display when an item IS expanded
            void setExpandedImage(const S8 id);
            /// Set the image to display when an item is NOT expanded
            void setNormalImage(const S8 id);
            /// Assign a SimObject pointer to an inspector data item
            void setObject(SimObject *obj);
            /// Set the items displayable text (caption)
            void setText(StringTableEntry txt);
            /// Set the items script value (data)
            void setValue(StringTableEntry val);
            /// Set the items virtual parent flag
            void setVirtualParent( bool value );
            /// Set whether the item is filtered out or not.
            void setFiltered( bool value ) { mState.set( Filtered ); }

            /// @}


            /// @name State Retrieval
            /// @{

            /// Returns true if this item is expanded. For
            /// inspector objects, the expansion is stored
            /// on the SimObject, for other things we use our
            /// bit vector.
            bool isExpanded() const;

            /// Return whether the item is current filtered out or not.
            /// @note Parent items may be filtered and yet still be visible if they have
            ///   children that are not filtered.
            bool isFiltered() const { return mState.test( Filtered ); }

            /// Returns true if an item is inspector data
            /// or false if it's just an item.
            bool isInspectorData() const { return mState.test(InspectorData); };

            /// Returns true if we've been manually set to allow dragging overrides.
            /// As it's a manually set flag, by default it is false.
            bool isDragTargetAllowed() const { return mState.test(ForceDragTarget); };

            /// Returns true if we've been manually set to allow dragging overrides.
            /// As it's a manually set flag, by default it is false.
            bool isDragAllowed() const { return !mState.test(DenyDrag); };

            /// Returns true if we should show the expand art
            /// and make the item interact with the mouse as if
            /// it were a parent.
            bool isParent() const;
            
            /// Return true if text label for inspector item should include internal name only.
            bool showInternalNameOnly() const { return mState.test( ShowInternalName ) && !mState.test( ShowObjectName | ShowClassName | ShowObjectId ); }

            /// Return true if text label for inspector item should include object name only.
            bool showObjectNameOnly() const { return mState.test( ShowObjectName ) && !mState.test( ShowInternalName | ShowClassName | ShowObjectId ); }

            /// @}

            /// @name Searching Methods
            /// @{
            
            /// Find a regular data item by it's script name.
            Item* findChildByName( const char* name );

            /// Find an inspector data item by it's SimObject pointer
            Item* findChildByValue(const SimObject *obj);

            /// Find a regular data item by it's script value
            Item* findChildByValue(StringTableEntry Value);

            /// @}
            
            /// Sort the childs of the item by their text.
            ///
            /// @param caseSensitive If true, sorting is case-sensitive.
            /// @param traverseHierarchy If true, also triggers a sort() on all child items.
            /// @param parentsFirst If true, parents are grouped before children in the resulting sort.
            void sort( bool caseSensitive = true, bool traverseHierarchy = false, bool parentsFirst = false );

         private:
            void _connectMonitors();
            void _disconnectMonitors();
      };

      friend class Item; // _onInspectorSetObjectModified

      /// @name Enums
      /// @{

      ///
      enum TreeState
      {
         RebuildVisible    = BIT(0), ///< Temporary flag, we have to rebuild the tree.
         IsInspector       = BIT(1), ///< We are mapping a SimObject hierarchy.
         IsEditable        = BIT(2), ///< We allow items to be moved around.
         ShowTreeLines     = BIT(3), ///< Should we render tree lines or just icons?
         BuildingVisTree   = BIT(4), ///< We are currently building the visible tree (prevent recursion)
      };

   protected:

  		enum
		{
         MaxIcons = 32,
		};

      enum Icons
      {
         Default1 = 0,
         SimGroup1,
         SimGroup2,
         SimGroup3,
         SimGroup4,         
         Hidden,         
         Lock1,
         Lock2,         
         Default,
         Icon31,
         Icon32
      };

      enum mDragMidPointFlags
      {
            NomDragMidPoint,
            AbovemDragMidPoint,
            BelowmDragMidPoint
      };

      ///
      enum HitFlags
      {
         OnIndent       = BIT(0),
         OnImage        = BIT(1),
         OnIcon         = BIT(2),
         OnText         = BIT(3),         
         OnRow          = BIT(4),
      };

      ///
      enum BmpIndices
      {
         BmpFirstChild,
         BmpLastChild,
         BmpChild,
         BmpExp,
         BmpExpN,
         BmpExpP,
         BmpExpPN,
         BmpCon,
         BmpConN,
         BmpConP,
         BmpConPN,
         BmpLine,
         BmpGlow,
      };

      /// @}

      /// @name Callbacks
      /// @{

      DECLARE_CALLBACK( bool, onDeleteObject, ( SimObject* object ) );
      DECLARE_CALLBACK( bool, isValidDragTarget, ( S32 id, const char* value ) );
      DECLARE_CALLBACK( void, onDefineIcons, () );
      DECLARE_CALLBACK( void, onAddGroupSelected, ( SimGroup* group ) );
      DECLARE_CALLBACK( void, onAddSelection, ( S32 itemOrObjectId, bool isLastSelection ) );
      DECLARE_CALLBACK( void, onSelect, ( S32 itemOrObjectId ) );
      DECLARE_CALLBACK( void, onInspect, ( S32 itemOrObjectId ) );
      DECLARE_CALLBACK( void, onRemoveSelection, ( S32 itemOrObjectId ) );
      DECLARE_CALLBACK( void, onUnselect, ( S32 itemOrObjectId ) );
      DECLARE_CALLBACK( void, onDeleteSelection, () );
      DECLARE_CALLBACK( void, onObjectDeleteCompleted, () );
      DECLARE_CALLBACK( void, onKeyDown, ( S32 modifier, S32 keyCode ) );
      DECLARE_CALLBACK( void, onMouseUp, ( S32 hitItemId, S32 mouseClickCount ) );
      DECLARE_CALLBACK( void, onMouseDragged, () );
      DECLARE_CALLBACK( void, onRightMouseDown, ( S32 itemId, const Point2I& mousePos, SimObject* object = NULL ) );
      DECLARE_CALLBACK( void, onRightMouseUp, ( S32 itemId, const Point2I& mousePos, SimObject* object = NULL ) );
      DECLARE_CALLBACK( void, onBeginReparenting, () );
      DECLARE_CALLBACK( void, onEndReparenting, () );
      DECLARE_CALLBACK( void, onReparent, ( S32 itemOrObjectId, S32 oldParentItemOrObjectId, S32 newParentItemOrObjectId ) );
      DECLARE_CALLBACK( void, onDragDropped, () );
      DECLARE_CALLBACK( void, onAddMultipleSelectionBegin, () );
      DECLARE_CALLBACK( void, onAddMultipleSelectionEnd, () );
      DECLARE_CALLBACK( bool, canRenameObject, ( SimObject* object ) );
      DECLARE_CALLBACK( bool, handleRenameObject, ( const char* newName, SimObject* object ) );
      DECLARE_CALLBACK( void, onClearSelection, () );

      /// @}

      ///
      Vector<Item*> mItems;
      Vector<Item*> mVisibleItems;
      Vector<Item*> mSelectedItems;

      /// Used for tracking stuff that was selected, but may not have been
      /// created at time of selection.
      Vector<S32> mSelected;

      S32 mItemCount;

      /// We do our own free list, as we we want to be able to recycle
      /// item ids and do some other clever things.
      Item* mItemFreeList;

      Item* mRoot;
      S32 mMaxWidth;
      S32 mSelectedItem;
      S32 mDraggedToItem;
      S32 mStart;      

      /// A combination of TreeState flags.
      BitSet32 mFlags;

      Item* mPossibleRenameItem;
      Item* mRenamingItem;
		Item* mTempItem;
      GuiTextEditCtrl* mRenameCtrl;

      /// Current filter that determines which items in the tree are displayed and which are hidden.
      String mFilterText;

      /// If true, a trace of actions taken by the control is logged to the console.  Can
      /// be turned on with the setDebug() script method.
      bool mDebug;

      GFXTexHandle mIconTable[MaxIcons];

      S32 mTabSize;
      S32 mTextOffset;
      bool mFullRowSelect;
      S32 mItemHeight;
      bool mDestroyOnSleep;
      bool mSupportMouseDragging;
      bool mMultipleSelections;
      bool mDeleteObjectAllowed;
      bool mDragToItemAllowed;
      bool mClearAllOnSingleSelection;   ///< When clicking on an already selected item, clear all other selections
      bool mCompareToObjectID;
      
      /// Used to hide the root tree element, defaults to true.
      bool mShowRoot;
      
      /// If true, object IDs will be included in inspector tree item labels.
      bool mShowObjectIds;
      
      /// If true, class names will be included in inspector tree item labels.
      bool mShowClassNames;
      
      /// If true, object names will be included in inspector tree item labels.
      bool mShowObjectNames;
      
      /// If true, internal names will be included in inspector tree item labels.
      bool mShowInternalNames;

      /// If true, class names will be used as object names for unnamed objects.
      bool mShowClassNameForUnnamedObjects;

      /// If true then tooltips will be automatically
      /// generated for all Inspector items
      bool mUseInspectorTooltips;

      /// If true then only render item tooltips if the item
      /// extends past the displayable width
      bool mTooltipOnWidthOnly;

      /// If true clicking on a selected item ( that is an object )
      /// will allow you to rename it.
      bool mCanRenameObjects;
      
      /// If true then object renaming operates on the internalName rather than
      /// the object name.
      bool mRenameInternal;
      
      S32 mCurrentDragCell;
      S32 mPreviousDragCell;
      S32 mDragMidPoint;
      bool mMouseDragged;
      bool mDragStartInSelection;
      Point2I mMouseDownPoint;

      StringTableEntry mBitmapBase;
      GFXTexHandle mTexRollover;
      GFXTexHandle mTexSelected;

      ColorI mAltFontColor;
      ColorI mAltFontColorHL;
      ColorI mAltFontColorSE;

      SimObjectPtr<SimObject> mRootObject;

      void _destroyChildren( Item* item, Item* parent, bool deleteObjects=true);
      void _destroyItem( Item* item, bool deleteObject=true);
      void _destroyTree();

      void _deleteItem(Item* item);

      void _buildItem(Item* item, U32 tabLevel, bool bForceFullUpdate = false);

      Item* _findItemByAmbiguousId( S32 itemOrObjectId, bool buildVirtual = true );

      void _expandObjectHierarchy( SimGroup* group );

      bool _hitTest(const Point2I & pnt, Item* & item, BitSet32 & flags);

      S32 getInspectorItemIconsWidth(Item* & item);

      virtual bool onVirtualParentBuild(Item *item, bool bForceFullUpdate = false);
      virtual bool onVirtualParentExpand(Item *item);
      virtual bool onVirtualParentCollapse(Item *item);
      virtual void onItemSelected( Item *item );
      virtual void onRemoveSelection( Item *item );
      virtual void onClearSelection() {};

      Item* addInspectorDataItem(Item *parent, SimObject *obj);
      
      virtual bool isValidDragTarget( Item* item );
      
      bool _isRootLevelItem( Item* item ) const
      {
         return ( item == mRoot && mShowRoot ) || ( item->mParent == mRoot && !mShowRoot );
      }

      /// For inspector tree views, this is hooked to the SetModificationSignal of sets
      /// so that the tree view knows when it needs to refresh.
      void _onInspectorSetObjectModified( SetModification modification, SimSet* set, SimObject* object );

   public:
      GuiTreeViewCtrl();
      virtual ~GuiTreeViewCtrl();

		//WLE Vince, Moving this into a function so I don't have to bounce off the console.  12/05/2013
		const char* getSelectedObjectList();

      /// Used for syncing the mSelected and mSelectedItems lists.
      void syncSelection();

      void lockSelection(bool lock);
      void hideSelection(bool hide);
      void toggleLockSelection();
      void toggleHideSelection();
      virtual bool canAddSelection( Item *item ) { return true; };
      void addSelection(S32 itemId, bool update = true, bool isLastSelection = true);
      const Vector< Item* >& getSelectedItems() const { return mSelectedItems; }
      const Vector< S32 >& getSelected() const { return mSelected; }

      bool isSelected(S32 itemId)
      {
         return isSelected( getItem( itemId ) );
      }
      bool isSelected(Item *item)
      {
         if ( !item )
            return false;
         return mSelectedItems.contains( item );   
      }

      void setDebug( bool value ) { mDebug = value; }

      /// Should use addSelection and removeSelection when calling from script
      /// instead of setItemSelected. Use setItemSelected when you want to select
      /// something in the treeview as it has script call backs.
      void removeSelection(S32 itemId);

      /// Sets the flag of the item with the matching itemId.
      bool setItemSelected(S32 itemId, bool select);
      bool setItemExpanded(S32 itemId, bool expand);
      bool setItemValue(S32 itemId, StringTableEntry Value);

      const char * getItemText(S32 itemId);
      const char * getItemValue(S32 itemId);
      StringTableEntry getTextToRoot(S32 itemId, const char *delimiter = "");

      Item* getRootItem() const { return mRoot; }
      Item * getItem(S32 itemId) const;
      Item * createItem(S32 icon);
      bool editItem( S32 itemId, const char* newText, const char* newValue );

      bool markItem( S32 itemId, bool mark );
      
      bool isItemSelected( S32 itemId );

      // insertion/removal
      void unlinkItem(Item * item);
      S32 insertItem(S32 parentId, const char * text, const char * value = "", const char * iconString = "", S16 normalImage = 0, S16 expandedImage = 1);
      bool removeItem(S32 itemId, bool deleteObjects=true);
      void removeAllChildren(S32 itemId); // Remove all children of the given item

      bool buildIconTable(const char * icons);

      bool setAddGroup(SimObject * obj);

      S32 getIcon(const char * iconString);

      // tree items
      const S32 getFirstRootItem() const;
      S32 getChildItem(S32 itemId);
      S32 getParentItem(S32 itemId);
      S32 getNextSiblingItem(S32 itemId);
      S32 getPrevSiblingItem(S32 itemId);
      S32 getItemCount();
      S32 getSelectedItem();
      S32 getSelectedItem(S32 index); // Given an item's index in the selection list, return its itemId
      S32 getSelectedItemsCount() {return mSelectedItems.size();} // Returns the number of selected items
      void moveItemUp( S32 itemId );
      void moveItemDown( S32 itemId );

      // misc.
      bool scrollVisible( Item *item );
      bool scrollVisible( S32 itemId );
      bool scrollVisibleByObjectId( S32 objID );
      
      void deleteSelection();
      void clearSelection();

      S32 findItemByName(const char *name);
      S32 findItemByValue(const char *name);
      S32 findItemByObjectId(S32 iObjId);

      void sortTree( bool caseSensitive, bool traverseHierarchy, bool parentsFirst );

      /// @name Filtering
      /// @{

      /// Get the current filter expression.  Only tree items whose text matches this expression
      /// are displayed.  By default, the expression is empty and all items are shown.
      const String& getFilterText() const { return mFilterText; }

      /// Set the pattern by which to filter items in the tree.  Only items in the tree whose text
      /// matches this pattern are displayed.
      void setFilterText( const String& text );

      /// Clear the current item filtering pattern.
      void clearFilterText() { setFilterText( String::EmptyString ); }

      /// @}

      // GuiControl
      bool onAdd();
      bool onWake();
      void onSleep();
      void onPreRender();
      bool onKeyDown( const GuiEvent &event );
		void onMouseDown(const GuiEvent &event);
      void onMiddleMouseDown(const GuiEvent &event);
      void onMouseMove(const GuiEvent &event);
      void onMouseEnter(const GuiEvent &event);
      void onMouseLeave(const GuiEvent &event);
      void onRightMouseDown(const GuiEvent &event);
      void onRightMouseUp(const GuiEvent &event);
      void onMouseDragged(const GuiEvent &event);
      virtual void onMouseUp(const GuiEvent &event);

      /// Returns false if the object is a child of one of the inner items.
      bool childSearch(Item * item, SimObject *obj, bool yourBaby);

      /// Find immediately available inspector items (eg ones that aren't children of other inspector items)
      /// and then update their sets
      void inspectorSearch(Item * item, Item * parent, SimSet * parentSet, SimSet * newParentSet);

		/// Find the Item associated with a sceneObject, returns true if it found one
		bool objectSearch( const SimObject *object, Item **item );

      // GuiArrayCtrl
      void onRenderCell(Point2I offset, Point2I cell, bool, bool);
      void onRender(Point2I offset, const RectI &updateRect);
      
      bool renderTooltip( const Point2I &hoverPos, const Point2I& cursorPos, const char* tipText );

      static void initPersistFields();

      void inspectObject(SimObject * obj, bool okToEdit);
	  S32 insertObject(S32 parentId, SimObject * obj, bool okToEdit);
      void buildVisibleTree(bool bForceFullUpdate = false);

      void cancelRename();
      void onRenameValidate();
      void showItemRenameCtrl( Item* item );

      DECLARE_CONOBJECT(GuiTreeViewCtrl);
      DECLARE_CATEGORY( "Gui Lists" );
      DECLARE_DESCRIPTION( "Hierarchical list of text items with optional icons.\nCan also be used to inspect SimObject hierarchies." );
};

#endif
