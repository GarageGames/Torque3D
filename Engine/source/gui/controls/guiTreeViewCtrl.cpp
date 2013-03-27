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

#include "platform/platform.h"
#include "gui/controls/guiTreeViewCtrl.h"

#include "core/frameAllocator.h"
#include "core/strings/findMatch.h"
#include "gui/containers/guiScrollCtrl.h"
#include "gui/worldEditor/editorIconRegistry.h"
#include "console/consoleTypes.h"
#include "console/console.h"
#include "gui/core/guiTypes.h"
#include "gfx/gfxDrawUtil.h"
#include "gui/controls/guiTextEditCtrl.h"
#ifdef TORQUE_TOOLS
   #include "gui/editor/editorFunctions.h"
#endif
#include "console/engineAPI.h"


IMPLEMENT_CONOBJECT(GuiTreeViewCtrl);

ConsoleDocClass( GuiTreeViewCtrl,
	"@brief Hierarchical list of text items with optional icons.\n\n"

	"Can also be used to inspect SimObject hierarchies, primarily within editors.\n\n"

   "GuiTreeViewCtrls can either display arbitrary user-defined trees or can be used to display SimObject hierarchies where "
   "each parent node in the tree is a SimSet or SimGroup and each leaf node is a SimObject.\n\n"

   "Each item in the tree has a text and a value.  For trees that display SimObject hierarchies, the text for each item "
   "is automatically derived from objects while the value for each item is the ID of the respective SimObject.  For trees "
   "that are not tied to SimObjects, both text and value of each item are set by the user.\n\n"

   "Additionally, items in the tree can have icons.\n\n"

   "Each item in the tree has a distinct numeric ID that is unique within its tree.  The ID of the root item, which is always "
   "present on a tree, is 0.\n\n"

	"@tsexample\n"
	"new GuiTreeViewCtrl(DatablockEditorTree)\n"
	"{\n"
	"	tabSize = \"16\";\n"
   "	textOffset = \"2\";\n"
	"	fullRowSelect = \"0\";\n"
	"	itemHeight = \"21\";\n"
	"	destroyTreeOnSleep = \"0\";\n"
	"	MouseDragging = \"0\";\n"
	"	MultipleSelections = \"1\";\n"
	"	DeleteObjectAllowed = \"1\";\n"
	"	DragToItemAllowed = \"0\";\n"
	"	ClearAllOnSingleSelection = \"1\";\n"
	"	showRoot = \"1\";\n"
	"	internalNamesOnly = \"0\";\n"
	"	objectNamesOnly = \"0\";\n"
	"	compareToObjectID = \"0\";\n"
	"	Profile = \"GuiTreeViewProfile\";\n"
	"	tooltipprofile = \"GuiToolTipProfile\";\n"
	"	hovertime = \"1000\";\n"
	"};\n"
	"@endtsexample\n\n"

	"@ingroup GuiContainers\n");

IMPLEMENT_CALLBACK( GuiTreeViewCtrl, onDeleteObject, bool, ( SimObject* object ), ( object ), "" );
IMPLEMENT_CALLBACK( GuiTreeViewCtrl, isValidDragTarget, bool, ( S32 id, const char* value ), ( id, value ), "" );
IMPLEMENT_CALLBACK( GuiTreeViewCtrl, onDefineIcons, void, (), (), "" );
IMPLEMENT_CALLBACK( GuiTreeViewCtrl, onAddGroupSelected, void, ( SimGroup* group ), ( group ), "" );
IMPLEMENT_CALLBACK( GuiTreeViewCtrl, onAddSelection, void, ( S32 itemOrObjectId, bool isLastSelection ), ( itemOrObjectId, isLastSelection ), "" );
IMPLEMENT_CALLBACK( GuiTreeViewCtrl, onSelect, void, ( S32 itemOrObjectId ), ( itemOrObjectId ), "" );
IMPLEMENT_CALLBACK( GuiTreeViewCtrl, onInspect, void, ( S32 itemOrObjectId ), ( itemOrObjectId ), "" );
IMPLEMENT_CALLBACK( GuiTreeViewCtrl, onRemoveSelection, void, ( S32 itemOrObjectId ), ( itemOrObjectId ), "" );
IMPLEMENT_CALLBACK( GuiTreeViewCtrl, onUnselect, void, ( S32 itemOrObjectId ), ( itemOrObjectId ), "" );
IMPLEMENT_CALLBACK( GuiTreeViewCtrl, onDeleteSelection, void, (), (), "" );
IMPLEMENT_CALLBACK( GuiTreeViewCtrl, onObjectDeleteCompleted, void, (), (), "" );
IMPLEMENT_CALLBACK( GuiTreeViewCtrl, onKeyDown, void, ( S32 modifier, S32 keyCode ), ( modifier, keyCode ), "" );
IMPLEMENT_CALLBACK( GuiTreeViewCtrl, onMouseUp, void, ( S32 hitItemId, S32 mouseClickCount ), ( hitItemId, mouseClickCount ), "" );
IMPLEMENT_CALLBACK( GuiTreeViewCtrl, onMouseDragged, void, (), (), "" );
IMPLEMENT_CALLBACK( GuiTreeViewCtrl, onRightMouseDown, void, ( S32 itemId, const Point2I& mousePos, SimObject* object ), ( itemId, mousePos, object ), "" );
IMPLEMENT_CALLBACK( GuiTreeViewCtrl, onRightMouseUp, void, ( S32 itemId, const Point2I& mousePos, SimObject* object ), ( itemId, mousePos, object ), "" );
IMPLEMENT_CALLBACK( GuiTreeViewCtrl, onBeginReparenting, void, (), (), "" );
IMPLEMENT_CALLBACK( GuiTreeViewCtrl, onEndReparenting, void, (), (), "" );
IMPLEMENT_CALLBACK( GuiTreeViewCtrl, onReparent, void, ( S32 itemOrObjectId, S32 oldParentItemOrObjectId, S32 newParentItemOrObjectId ), ( itemOrObjectId, oldParentItemOrObjectId, newParentItemOrObjectId ), "" );
IMPLEMENT_CALLBACK( GuiTreeViewCtrl, onDragDropped, void, (), (), "" );
IMPLEMENT_CALLBACK( GuiTreeViewCtrl, onAddMultipleSelectionBegin, void, (), (), "" );
IMPLEMENT_CALLBACK( GuiTreeViewCtrl, onAddMultipleSelectionEnd, void, (), (), "" );
IMPLEMENT_CALLBACK( GuiTreeViewCtrl, canRenameObject, bool, ( SimObject* object ), ( object ), "" );
IMPLEMENT_CALLBACK( GuiTreeViewCtrl, handleRenameObject, bool, ( const char* newName, SimObject* object ), ( newName, object ), "" );
IMPLEMENT_CALLBACK( GuiTreeViewCtrl, onClearSelection, void, (), (), "" );


static S32 QSORT_CALLBACK itemCompareCaseSensitive( const void *a, const void *b )
{
   GuiTreeViewCtrl::Item* itemA = *( ( GuiTreeViewCtrl::Item** ) a );
   GuiTreeViewCtrl::Item* itemB = *( ( GuiTreeViewCtrl::Item** ) b );

   char bufferA[ 1024 ];
   char bufferB[ 1024 ];

   itemA->getDisplayText( sizeof( bufferA ), bufferA );
   itemB->getDisplayText( sizeof( bufferB ), bufferB );

   return dStrnatcmp( bufferA, bufferB );
}

static S32 QSORT_CALLBACK itemCompareCaseInsensitive( const void *a, const void *b )
{
   GuiTreeViewCtrl::Item* itemA = *( ( GuiTreeViewCtrl::Item** ) a );
   GuiTreeViewCtrl::Item* itemB = *( ( GuiTreeViewCtrl::Item** ) b );

   char bufferA[ 1024 ];
   char bufferB[ 1024 ];

   itemA->getDisplayText( sizeof( bufferA ), bufferA );
   itemB->getDisplayText( sizeof( bufferB ), bufferB );

   return dStrnatcasecmp( bufferA, bufferB );
}

static void itemSortList( GuiTreeViewCtrl::Item*& firstChild, bool caseSensitive, bool traverseHierarchy, bool parentsFirst )
{
   // Sort the children.
   // Do this in a separate scope, so we release the buffers before
   // recursing.

   {
      Vector< GuiTreeViewCtrl::Item* > parents;
      Vector< GuiTreeViewCtrl::Item* > items;

      // Put all items into the two vectors.

      for( GuiTreeViewCtrl::Item* item = firstChild; item != NULL; item = item->mNext )
         if( parentsFirst && item->isParent() )
            parents.push_back( item );
         else
            items.push_back( item );

      // Sort both vectors.

      dQsort( parents.address(), parents.size(), sizeof( GuiTreeViewCtrl::Item* ), caseSensitive ? itemCompareCaseSensitive : itemCompareCaseInsensitive );
      dQsort( items.address(), items.size(), sizeof( GuiTreeViewCtrl::Item* ), caseSensitive ? itemCompareCaseSensitive : itemCompareCaseInsensitive );

      // Wipe current child chain then reconstruct it in reverse
      // as we prepend items.

      firstChild = NULL;

      // Add child items.

      for( U32 i = items.size(); i > 0; -- i )
      {
         GuiTreeViewCtrl::Item* child = items[ i - 1 ];

         child->mNext = firstChild;         
         if( firstChild )
            firstChild->mPrevious = child;

         firstChild = child;
      }

      // Add parent child items, if requested.

      for( U32 i = parents.size(); i > 0; -- i )
      {
         GuiTreeViewCtrl::Item* child = parents[ i - 1 ];

         child->mNext = firstChild;
         if( firstChild )
            firstChild->mPrevious = child;

         firstChild = child;
      }

      firstChild->mPrevious = NULL;
   }

   // Traverse hierarchy, if requested.

   if( traverseHierarchy )
   {
      GuiTreeViewCtrl::Item* child = firstChild;
      while( child )
      {
         if( child->isParent() )
            child->sort( caseSensitive, traverseHierarchy, parentsFirst );

         child = child->mNext;
      }
   }
}


//=============================================================================
//    GuiTreeViewCtrl::Item.
//=============================================================================
// MARK: ---- GuiTreeViewCtrl::Item ----

//-----------------------------------------------------------------------------

GuiTreeViewCtrl::Item::Item( GuiTreeViewCtrl* parent, GuiControlProfile *pProfile )
{
   AssertFatal( pProfile != NULL , "Cannot create a tree item without a valid tree and control profile!");
   mParentControl       = parent;
   mState               = 0;
   mId                  = -1;
   mTabLevel            = 0;
   mIcon                = 0;
   mDataRenderWidth     = 0;
   mParent              = NULL;
   mChild               = NULL;
   mNext                = NULL;
   mPrevious            = NULL;
   mProfile             = pProfile;

   mScriptInfo.mNormalImage   = BmpCon;
   mScriptInfo.mExpandedImage = BmpExp;
   mScriptInfo.mText          = NULL;
   mScriptInfo.mValue         = NULL;
}

//-----------------------------------------------------------------------------

GuiTreeViewCtrl::Item::~Item()
{
   _disconnectMonitors();
}

//-----------------------------------------------------------------------------

void GuiTreeViewCtrl::Item::_connectMonitors()
{
   if( mInspectorInfo.mObject != NULL )
   {
      SimSet* set = dynamic_cast< SimSet* >( mInspectorInfo.mObject.getPointer() );
      if( set )
         set->getSetModificationSignal().notify( mParentControl, &GuiTreeViewCtrl::_onInspectorSetObjectModified );
   }
}

//-----------------------------------------------------------------------------

void GuiTreeViewCtrl::Item::_disconnectMonitors()
{
   if( mInspectorInfo.mObject != NULL )
   {
      SimSet* set = dynamic_cast< SimSet* >( mInspectorInfo.mObject.getPointer() );
      if( set )
         set->getSetModificationSignal().remove( mParentControl, &GuiTreeViewCtrl::_onInspectorSetObjectModified );
   }
}

//-----------------------------------------------------------------------------

void GuiTreeViewCtrl::Item::setNormalImage(S8 id)
{
   if(mState.test(InspectorData))
   {
      Con::errorf("Tried to set normal image %d for item %d, which is InspectorData!", id, mId);
      return;
   }

   mScriptInfo.mNormalImage = id;
}

//-----------------------------------------------------------------------------

void GuiTreeViewCtrl::Item::setExpandedImage(S8 id)
{
   if(mState.test(InspectorData))
   {
      Con::errorf("Tried to set expanded image %d for item %d, which is InspectorData!", id, mId);
      return;
   }

   mScriptInfo.mExpandedImage = id;
}

//-----------------------------------------------------------------------------

void GuiTreeViewCtrl::Item::setText(StringTableEntry txt)
{
   if(mState.test(InspectorData))
   {
      Con::errorf("Tried to set text for item %d, which is InspectorData!", mId);
      return;
   }

   mScriptInfo.mText = txt;


   // Update Render Data
   if( !mProfile.isNull() )
      mDataRenderWidth = getDisplayTextWidth( mProfile->mFont );

}

//-----------------------------------------------------------------------------

void GuiTreeViewCtrl::Item::setValue(StringTableEntry val)
{
   if(mState.test(InspectorData))
   {
      Con::errorf("Tried to set value for item %d, which is InspectorData!", mId);
      return;
   }

   mScriptInfo.mValue = const_cast<char*>(val); // mValue really ought to be a StringTableEntry

   // Update Render Data
   if( !mProfile.isNull() )
      mDataRenderWidth = getDisplayTextWidth( mProfile->mFont );

}

//-----------------------------------------------------------------------------

S8 GuiTreeViewCtrl::Item::getNormalImage() const
{
   if(mState.test(InspectorData))
   {
      Con::errorf("Tried to get the normal image for item %d, which is InspectorData!", mId);
      return 0; // fail safe for width determinations
   }

   return mScriptInfo.mNormalImage;
}

//-----------------------------------------------------------------------------

S8 GuiTreeViewCtrl::Item::getExpandedImage() const
{
   if(mState.test(InspectorData))
   {
      Con::errorf("Tried to get the expanded image for item %d, which is InspectorData!", mId);
      return 0; // fail safe for width determinations
   }

   return mScriptInfo.mExpandedImage;
}

//-----------------------------------------------------------------------------

StringTableEntry GuiTreeViewCtrl::Item::getText()
{
   if(mState.test(InspectorData))
   {
      Con::errorf("Tried to get the text for item %d, which is InspectorData!", mId);
      return NULL;
   }

   return ( mScriptInfo.mText ) ? mScriptInfo.mText : StringTable->EmptyString();
}

//-----------------------------------------------------------------------------

StringTableEntry GuiTreeViewCtrl::Item::getValue()
{
   if(mState.test(InspectorData))
   {
      Con::errorf("Tried to get the value for item %d, which is InspectorData!", mId);
      return NULL;
   }

   return ( mScriptInfo.mValue ) ? mScriptInfo.mValue : StringTable->EmptyString();
}

//-----------------------------------------------------------------------------

void GuiTreeViewCtrl::Item::setObject(SimObject *obj)
{
   if(!mState.test(InspectorData))
   {
      Con::errorf("Tried to set the object for item %d, which is not InspectorData!", mId);
      return;
   }
   
   _disconnectMonitors();
   mInspectorInfo.mObject = obj;
   _connectMonitors();

   // Update Render Data
   if( !mProfile.isNull() )
      mDataRenderWidth = getDisplayTextWidth( mProfile->mFont );
}

//-----------------------------------------------------------------------------

SimObject *GuiTreeViewCtrl::Item::getObject()
{
   if(!mState.test(InspectorData))
   {
      Con::errorf("Tried to get the object for item %d, which is not InspectorData!", mId);
      return NULL;
   }

   return mInspectorInfo.mObject;
}

//-----------------------------------------------------------------------------

U32 GuiTreeViewCtrl::Item::getDisplayTextLength()
{
   if( mState.test( InspectorData ) )
   {
      SimObject *obj = getObject();
      if( !obj )
         return 0;

      StringTableEntry name = obj->getName();
      StringTableEntry internalName = obj->getInternalName();
      StringTableEntry className = obj->getClassName();      

      if( showInternalNameOnly() )
      {
         if( internalName && internalName[ 0 ] )
            return dStrlen( internalName );
         else
            return dStrlen( "(none)" );
      }
      else if( showObjectNameOnly() )
      {
         if( name && name[ 0 ] )
            return dStrlen( name );
         else if( mState.test( ShowClassNameForUnnamed ) )
            return dStrlen( className );
         else
            return dStrlen( "(none)" );
      }

      dsize_t len = 0;
      if( mState.test( ShowObjectId ) )
         len += dStrlen( obj->getIdString() ) + 2; // '<id>: '
      if( mState.test( ShowClassName ) )
      {
         if( name && name[ 0 ] )
            len += dStrlen( className ) + 3; // '<class> - '
         else
            len += dStrlen( className );
      }
      if( mState.test( ShowObjectName ) )
      {
         if( name && name[ 0 ] )
            len += dStrlen( name );
         else if( mState.test( ShowClassNameForUnnamed ) )
            len += dStrlen( className );
      }
      if( mState.test( ShowInternalName ) )
      {
         if( internalName && internalName[ 0 ] )
            len += dStrlen( internalName ) + 3; // ' [<internalname>]'
      }

      return len;
   }

   StringTableEntry pText = getText();
   if( pText == NULL )
      return 0;

   return dStrlen( pText );
}

//-----------------------------------------------------------------------------

void GuiTreeViewCtrl::Item::getDisplayText(U32 bufLen, char *buf)
{
   FrameAllocatorMarker txtAlloc;

   if( mState.test( InspectorData ) )
   {
      SimObject *pObject = getObject();
      if( pObject )
      {
         const char* pObjName = pObject->getName();
         const char* pInternalName = pObject->getInternalName();

         bool hasInternalName = pInternalName && pInternalName[0];
         bool hasObjectName = pObjName && pObjName[0];

         const char* pClassName = pObject->getClassName();

         if( showInternalNameOnly() )
            dSprintf( buf, bufLen, "%s", hasInternalName ? pInternalName : "(none)" );
			else if( showObjectNameOnly() )
         {
            if( !hasObjectName && mState.test( ShowClassNameForUnnamed ) )
               dSprintf( buf, bufLen, "%s", pClassName );
            else
               dSprintf( buf, bufLen, "%s", hasObjectName ? pObjName : "(none)" );
         }
         else
         {
            char* ptr = buf;
            int len = bufLen;
            
            if( mState.test( ShowObjectId ) )
            {
               S32 n = dSprintf( ptr, len, "%d: ", pObject->getId() );
               ptr += n;
               len -= n;
            }
            
            if( mState.test( ShowClassName ) )
            {
               S32 n;
               if( hasObjectName && mState.test( ShowObjectName ) )
                  n = dSprintf( ptr, len, "%s - ", pClassName );
               else
                  n = dSprintf( ptr, len, "%s", pClassName );
               
               ptr += n;
               len -= n;
            }
            
            if( mState.test( ShowObjectName ) )
            {
               S32 n = 0;
               if( hasObjectName )
                  n = dSprintf( ptr, len, "%s", pObjName );
               else if( mState.test( ShowClassNameForUnnamed ) )
                  n = dSprintf( ptr, len, "%s", pClassName );
                  
               ptr += n;
               len -= n;
            }
            
            if( hasInternalName && mState.test( ShowInternalName ) )
               dSprintf( ptr, len, " [%s]", pInternalName );
         }
      }
      else
         buf[ 0 ] = '\0';
   }
   else
   {
      // Script data! (copy it in)
      dStrncpy(buf, getText(), bufLen);
   }
}

//-----------------------------------------------------------------------------

S32 GuiTreeViewCtrl::Item::getDisplayTextWidth(GFont *font)
{
   if( !font )
      return 0;

   FrameAllocatorMarker txtAlloc;
   U32 bufLen = getDisplayTextLength();
   if( bufLen == 0 )
      return 0;

   // Add space for the string terminator
   bufLen++;

   char *buf = (char*)txtAlloc.alloc(bufLen);
   getDisplayText(bufLen, buf);

   return font->getStrWidth(buf);
}

//-----------------------------------------------------------------------------

bool GuiTreeViewCtrl::Item::hasObjectBasedTooltip()
{
   if(mState.test(Item::InspectorData))
   {
      SimObject *pObject = getObject();
      if(pObject)
      {
         const char* pClassName = pObject->getClassName();

         // Retrieve custom tooltip string
         String method("GetTooltip");
         method += pClassName;
         if(mParentControl->isMethod(method.c_str()))
         {
            return true;
         }
      }
   }

   return false;
}

//-----------------------------------------------------------------------------

void GuiTreeViewCtrl::Item::getTooltipText(U32 bufLen, char *buf)
{
   getDisplayText(bufLen, buf);

   if(mState.test(Item::InspectorData))
   {
      SimObject *pObject = getObject();
      if(pObject)
      {
         const char* pClassName = pObject->getClassName();

         // Retrieve custom tooltip string
         String method("GetTooltip");
         method += pClassName;
         if(mParentControl->isMethod(method.c_str()))
         {
            const char* tooltip = Con::executef( mParentControl, method.c_str(), pObject->getIdString() );
            dsize_t len = dStrlen(buf);
            S32 newBufLen = bufLen-len;
            if(dStrlen(tooltip) > 0 && newBufLen > 0)
            {
               dSprintf(buf+len, newBufLen, "\n%s", tooltip);
            }
         }
      }
   }
}

//-----------------------------------------------------------------------------

bool GuiTreeViewCtrl::Item::isParent() const
{
   if(mState.test(VirtualParent))
   {
      if( !isInspectorData() )
         return true;

      // Does our object have any children?
      if(mInspectorInfo.mObject)
      {
         SimSet *pSimSet = dynamic_cast<SimSet*>( (SimObject*)mInspectorInfo.mObject);
         if ( pSimSet != NULL && pSimSet->size() > 0)
            return pSimSet->size();
      }
   }

   // Otherwise, just return whether the child list is populated.
   return mChild;
}

//-----------------------------------------------------------------------------

bool GuiTreeViewCtrl::Item::isExpanded() const
{
   if(mState.test(InspectorData))
      return mInspectorInfo.mObject ? mInspectorInfo.mObject->isExpanded() : false;
   else
      return mState.test(Expanded);
}

//-----------------------------------------------------------------------------

void GuiTreeViewCtrl::Item::setExpanded(bool f)
{
   if( mState.test(InspectorData) )
   {
      if( !mInspectorInfo.mObject.isNull() )
         mInspectorInfo.mObject->setExpanded(f);
   }
   else
      mState.set(Expanded, f);
}

//-----------------------------------------------------------------------------

void GuiTreeViewCtrl::Item::setVirtualParent( bool value )
{
   mState.set(VirtualParent, value);
}

//-----------------------------------------------------------------------------

GuiTreeViewCtrl::Item* GuiTreeViewCtrl::Item::findChildByName( const char* name )
{
   Item* child = mChild;
   while( child )
   {
      if( dStricmp( child->mScriptInfo.mText, name ) == 0 )
         return child;
         
      child = child->mNext;
   }
   
   return NULL;
}

//-----------------------------------------------------------------------------

GuiTreeViewCtrl::Item *GuiTreeViewCtrl::Item::findChildByValue(const SimObject *obj)
{
   // Iterate over our children and try to find the given
   // SimObject

   Item *pResultObj = mChild;

   while(pResultObj)
   {
      // CodeReview this check may need to be removed
      // if we want to use the tree for data that
      // isn't related to SimObject based objects with
      // arbitrary values associated with them [5/5/2007 justind]

      // Skip non-inspector data stuff.
      if(pResultObj->mState.test(InspectorData))
      {
         if(pResultObj->getObject() == obj)
            break; // Whoa.
      }
      pResultObj = pResultObj->mNext;
   }
   // If the loop terminated we are NULL, otherwise we have the result in res.
   return pResultObj;
}

//-----------------------------------------------------------------------------

GuiTreeViewCtrl::Item *GuiTreeViewCtrl::Item::findChildByValue( StringTableEntry Value )
{
   // Iterate over our children and try to find the given Value
   // Note : This is a case-insensitive search
   Item *pResultObj = mChild;

   while(pResultObj)
   {

      // check the script value of the item against the specified value
      if( pResultObj->mScriptInfo.mValue != NULL && dStricmp( pResultObj->mScriptInfo.mValue, Value ) == 0 )
         return pResultObj;
      pResultObj = pResultObj->mNext;
   }
   // If the loop terminated we didn't find an item with the specified script value
   return NULL;
}

//-----------------------------------------------------------------------------

void GuiTreeViewCtrl::Item::sort( bool caseSensitive, bool traverseHierarchy, bool parentsFirst )
{
   itemSortList( mChild, caseSensitive, traverseHierarchy, parentsFirst );
}

//=============================================================================
//    GuiTreeViewCtrl.
//=============================================================================
// MARK: ---- GuiTreeViewCtrl ----

//-----------------------------------------------------------------------------

GuiTreeViewCtrl::GuiTreeViewCtrl()
{
   VECTOR_SET_ASSOCIATION(mItems);
   VECTOR_SET_ASSOCIATION(mVisibleItems);
   VECTOR_SET_ASSOCIATION(mSelectedItems);
   VECTOR_SET_ASSOCIATION(mSelected);

   mItemFreeList  =  NULL;
   mRoot          =  NULL;
   mItemCount     =  0;
   mSelectedItem  =  0;
   mStart         =  0;
   mPossibleRenameItem = NULL;
   mRenamingItem = NULL;
	mTempItem = NULL;
   mRenameCtrl = NULL;

   mDraggedToItem =  0;
   mCurrentDragCell = 0;
   mPreviousDragCell = 0;
   mDragMidPoint = NomDragMidPoint;
   mMouseDragged = false;
   mDebug = false;

   // persist info..
   mTabSize = 16;
   mTextOffset = 2;
   mFullRowSelect = false;
   mItemHeight = 20;

   //
   setSize(Point2I(1, 0));

   // Set up default state
   mFlags.set(ShowTreeLines);
   mFlags.set(IsEditable, false);

   mDestroyOnSleep = true;
   mSupportMouseDragging = true;
   mMultipleSelections = true;
   mDeleteObjectAllowed = true;
   mDragToItemAllowed = true;
   mShowRoot = true;
   mUseInspectorTooltips = false;
   mTooltipOnWidthOnly = false;
   mCompareToObjectID = true;
   mShowObjectIds = true;
   mShowClassNames = true;
   mShowObjectNames = true;
   mShowInternalNames = true;
   mShowClassNameForUnnamedObjects = false;
   mFlags.set(RebuildVisible);

   mCanRenameObjects = true;
   mRenameInternal = false;

   mClearAllOnSingleSelection = true;

   mBitmapBase       = StringTable->insert("");
   mTexRollover      = NULL;
   mTexSelected      = NULL;
   
   mRenderTooltipDelegate.bind( this, &GuiTreeViewCtrl::renderTooltip );
}

//-----------------------------------------------------------------------------

GuiTreeViewCtrl::~GuiTreeViewCtrl()
{
   _destroyTree();
}

//------------------------------------------------------------------------------

void GuiTreeViewCtrl::initPersistFields()
{
   addGroup( "TreeView" );

      addField( "tabSize",              TypeS32,    Offset(mTabSize,              GuiTreeViewCtrl));
      addField( "textOffset",           TypeS32,    Offset(mTextOffset,           GuiTreeViewCtrl));
      addField( "fullRowSelect",        TypeBool,   Offset(mFullRowSelect,        GuiTreeViewCtrl));
      addField( "itemHeight",           TypeS32,    Offset(mItemHeight,           GuiTreeViewCtrl));
      addField( "destroyTreeOnSleep",   TypeBool,   Offset(mDestroyOnSleep,       GuiTreeViewCtrl),
         "If true, the entire tree item hierarchy is deleted when the control goes to sleep." );
      addField( "mouseDragging",        TypeBool,   Offset(mSupportMouseDragging, GuiTreeViewCtrl));
      addField( "multipleSelections",   TypeBool,   Offset(mMultipleSelections,   GuiTreeViewCtrl),
         "If true, multiple items can be selected concurrently." );
      addField( "deleteObjectAllowed",  TypeBool,   Offset(mDeleteObjectAllowed,  GuiTreeViewCtrl));
      addField( "dragToItemAllowed",    TypeBool,   Offset(mDragToItemAllowed,    GuiTreeViewCtrl));
      addField( "clearAllOnSingleSelection",  TypeBool,   Offset(mClearAllOnSingleSelection,  GuiTreeViewCtrl));
      addField( "showRoot",             TypeBool,   Offset(mShowRoot,             GuiTreeViewCtrl),
         "If true, the root item is shown in the tree." );
      addField( "useInspectorTooltips", TypeBool,   Offset(mUseInspectorTooltips,    GuiTreeViewCtrl));
      addField( "tooltipOnWidthOnly",   TypeBool,   Offset(mTooltipOnWidthOnly,    GuiTreeViewCtrl));

   endGroup( "TreeView" );
   
   addGroup( "Inspector Trees" );
   
      addField( "showObjectIds", TypeBool, Offset( mShowObjectIds, GuiTreeViewCtrl ),
         "If true, item text labels for objects will include object IDs." );
      addField( "showClassNames", TypeBool, Offset( mShowClassNames, GuiTreeViewCtrl ),
         "If true, item text labels for objects will include class names." );
      addField( "showObjectNames", TypeBool, Offset( mShowObjectNames, GuiTreeViewCtrl ),
         "If true, item text labels for objects will include object names." );
      addField( "showInternalNames", TypeBool, Offset( mShowInternalNames, GuiTreeViewCtrl ),
         "If true, item text labels for obje ts will include internal names." );
      addField( "showClassNameForUnnamedObjects", TypeBool, Offset( mShowClassNameForUnnamedObjects, GuiTreeViewCtrl ),
         "If true, class names will be used as object names for unnamed objects." );
      addField( "compareToObjectID",    TypeBool,   Offset(mCompareToObjectID,    GuiTreeViewCtrl));
      addField( "canRenameObjects",     TypeBool,   Offset(mCanRenameObjects,     GuiTreeViewCtrl),       
         "If true clicking on a selected item ( that is an object and not the root ) will allow you to rename it." );
      addField( "renameInternal",       TypeBool,   Offset(mRenameInternal,       GuiTreeViewCtrl),
         "If true then object renaming operates on the internalName rather than the object name." );

   endGroup( "Inspector Trees" );

   Parent::initPersistFields();
}

//------------------------------------------------------------------------------

GuiTreeViewCtrl::Item * GuiTreeViewCtrl::getItem(S32 itemId) const
{
   if ( itemId > 0 && itemId <= mItems.size() )
      return mItems[itemId-1];
   return NULL;
}

//------------------------------------------------------------------------------

GuiTreeViewCtrl::Item * GuiTreeViewCtrl::createItem(S32 icon)
{
   Item * pNewItem = NULL;

   // grab from the free list?
   if( mItemFreeList )
   {
      pNewItem = mItemFreeList;
      mItemFreeList = pNewItem->mNext;

      // re-add to vector
      mItems[ pNewItem->mId - 1 ] = pNewItem;
   }
   else
   {
      pNewItem = new Item( this, mProfile );

      AssertFatal( pNewItem != NULL, "Fatal : unable to allocate tree item!");

      mItems.push_back( pNewItem );

      // set the id
      pNewItem->mId = mItems.size();
   }

   // reset
   if (icon)
      pNewItem->mIcon = icon;
   else
      pNewItem->mIcon = Default; //default icon to stick next to an item

   pNewItem->mState = Item::ShowObjectId | Item::ShowClassName | Item::ShowObjectName | Item::ShowInternalName;
   pNewItem->mTabLevel = 0;

   // Null out item pointers
   pNewItem->mNext = 0;
   pNewItem->mPrevious = 0;
   pNewItem->mChild = 0;
   pNewItem->mParent = 0;

   mItemCount++;

   return pNewItem;
}

//------------------------------------------------------------------------------

void GuiTreeViewCtrl::_destroyChildren( Item* item, Item* parent, bool deleteObjects )
{
   if ( !item  || item == parent || !mItems[item->mId-1] )
      return;

   // destroy depth first, then siblings from last to first
   if ( item->isParent() && item->mChild )
      _destroyChildren(item->mChild, item, deleteObjects);
   if( item->mNext )
      _destroyChildren(item->mNext, parent, deleteObjects);

   // destroy the item
   _destroyItem( item, deleteObjects );
}

//-----------------------------------------------------------------------------

void GuiTreeViewCtrl::_destroyItem( Item* item, bool deleteObject )
{
   if(!item)
      return;

   if(item->isInspectorData())
   {
      // make sure the SimObjectPtr is clean!
      SimObject *pObject = item->getObject();
      if( pObject && pObject->isProperlyAdded() )
      {
         bool skipDelete = !deleteObject;

         if( !skipDelete && isMethod( "onDeleteObject" ) )
            skipDelete = onDeleteObject_callback( pObject );

         if ( !skipDelete )
            pObject->deleteObject();
      }

      item->setObject( NULL );
   }

   // Remove item from the selection
   if (mSelectedItem == item->mId)
      mSelectedItem = 0;
   for ( S32 i = 0; i < mSelectedItems.size(); i++ ) 
   {
      if ( mSelectedItems[i] == item ) 
      {
         mSelectedItems.erase( i );
         break;
      }
   }
   item->mState.clear();

   // unlink
   if( item->mPrevious )
      item->mPrevious->mNext = item->mNext;
   if( item->mNext )
      item->mNext->mPrevious = item->mPrevious;
   if( item->mParent && ( item->mParent->mChild == item ) )
      item->mParent->mChild = item->mNext;

   // remove from vector
   mItems[item->mId-1] = 0;

   // set as root free item
   item->mNext = mItemFreeList;
   mItemFreeList = item;
   mItemCount--;
}

//------------------------------------------------------------------------------

void GuiTreeViewCtrl::_deleteItem(Item *item)
{
   removeItem(item->mId);
}

//------------------------------------------------------------------------------

void GuiTreeViewCtrl::_destroyTree()
{
   // clear the item list
   for(U32 i = 0; i < mItems.size(); i++)
   {
      Item *pFreeItem = mItems[ i ];
      if( pFreeItem != NULL )
         delete pFreeItem;
   }

   mItems.clear();

   // clear the free list
   while(mItemFreeList)
   {
      Item *next = mItemFreeList->mNext;
      delete mItemFreeList;
      mItemFreeList = next;
   }

   mVisibleItems.clear();
   mSelectedItems.clear();

   //
   mRoot          = NULL;
   mItemFreeList  = NULL;
   mItemCount     = 0;
   mSelectedItem  = 0;
   mDraggedToItem = 0;

   mRenamingItem = NULL;
   mTempItem = NULL;
   mPossibleRenameItem = NULL;
}

//------------------------------------------------------------------------------

void GuiTreeViewCtrl::_onInspectorSetObjectModified( SetModification modification, SimSet* set, SimObject* object )
{
   // Don't bother searching for the Item to see if it is actually visible and instead just
   // mark our tree state as dirty so we get a rebuild on the next render.

   mFlags.set( RebuildVisible );
}

//------------------------------------------------------------------------------

GuiTreeViewCtrl::Item* GuiTreeViewCtrl::_findItemByAmbiguousId( S32 itemOrObjectId, bool buildVirtual )
{
   Item* item = getItem( itemOrObjectId );
   if( item )
      return item;

   SimObject* object = Sim::findObject( itemOrObjectId );
   if( object )
   {
      // If we should expand virtual trees in order to find the item,
      // do so now.
      if( buildVirtual )
      {
         if( mFlags.test( RebuildVisible ) )
            buildVisibleTree();

         SimGroup* group = object->getGroup();
         if( group )
            _expandObjectHierarchy( group );
      }

      if( objectSearch( object, &item ) )
         return item;
   }

   return NULL;
}

//------------------------------------------------------------------------------

void GuiTreeViewCtrl::_expandObjectHierarchy( SimGroup* group )
{
   SimGroup* parent = group->getGroup();
   if( parent && !parent->isExpanded() )
      _expandObjectHierarchy( parent );

   if( !group->isExpanded() )
   {
      Item* item;
      if( objectSearch( group, &item ) )
      {
         item->setExpanded();
         onVirtualParentBuild( item, false );
      }
   }
}

//------------------------------------------------------------------------------

void GuiTreeViewCtrl::_buildItem( Item* item, U32 tabLevel, bool bForceFullUpdate )
{
   if (!item || !mActive || !isVisible() || !mProfile  )
      return;

   // If it's inspector data, make sure we still have it, if not, kill it.
   if(item->isInspectorData() && !item->getObject() )
   {
      removeItem(item->mId);
      return;
   }

   // If it's a virtual parent, give a chance to update itself...
   if(item->mState.test( Item::VirtualParent) )
   {
      // If it returns false the item has been removed.

      if( !onVirtualParentBuild( item, bForceFullUpdate ) )
         return;
   }

   // If we have a filter pattern, sync the item's filtering status to it.

   if( !getFilterText().isEmpty() )
   {
      // Determine the filtering status by looking for the filter
      // text in the item's display text.

      char displayText[ 2048 ];
      item->getDisplayText( sizeof( displayText ), displayText );
      if( !dStristr( displayText, mFilterText ) )
      {
         item->mState.set( Item::Filtered );

         // If it's not a parent, we're done.  Otherwise, there may be children
         // that are not filtered so we need to process them first.

         if( !item->isParent() )
            return;
      }
      else
         item->mState.clear( Item::Filtered );
   }
   else
      item->mState.clear( Item::Filtered );

   // Is this the root item?
   const bool isRoot = item == mRoot;

   // Add non-root items or the root if we're supposed to show it.

   if( ( mShowRoot || !isRoot ) &&
       !item->isFiltered() )
   {
      item->mTabLevel = tabLevel;
      mVisibleItems.push_back( item );

      if( mProfile != NULL )
      {
         mProfile->incLoadCount();
         
         S32 width = mTextOffset + ( mTabSize * item->mTabLevel ) + getInspectorItemIconsWidth( item ) + item->getDisplayTextWidth( mProfile->mFont );

         // check image
         S32 image = BmpChild;
         if ( item->isInspectorData() )
            image = item->isExpanded() ? BmpExp : BmpCon;
         else
            image = item->isExpanded() ? item->getExpandedImage() : item->getNormalImage();

         if ( ( image >= 0 ) && ( image < mProfile->mBitmapArrayRects.size() ) )
            width += mProfile->mBitmapArrayRects[image].extent.x;

         if ( width > mMaxWidth )
            mMaxWidth = width;
            
         mProfile->decLoadCount();
      }
   }

   // If expanded or a hidden root, add all the
   // children items as well.

   if (  item->isExpanded() || 
         bForceFullUpdate ||
         ( isRoot && !mShowRoot ) )
   {
      Item* child = item->mChild;
      while ( child )
      {
         // Bit of a hack so we can safely remove items as we
         // traverse.
         Item *pChildTemp = child;
         child = child->mNext;

         _buildItem( pChildTemp, tabLevel + 1, bForceFullUpdate );
      }
   }
}

//------------------------------------------------------------------------------

void GuiTreeViewCtrl::buildVisibleTree(bool bForceFullUpdate)
{
   // Recursion Prevention.
   if( mFlags.test( BuildingVisTree ) )
      return;
   mFlags.set( BuildingVisTree, true );

   if( mDebug )
      Con::printf( "Rebuilding visible tree" );

   mMaxWidth = 0;
   mVisibleItems.clear();

   // If we're filtering, force a full update.

   if( !mFilterText.isEmpty() )
      bForceFullUpdate = true;

   // Update the flags.
   mFlags.clear(RebuildVisible);

   // build the root items
   Item *traverse = mRoot;
   while(traverse)
   {
      _buildItem(traverse, 0, bForceFullUpdate);
      traverse = traverse->mNext;
   }

   // adjust the GuiArrayCtrl
   mCellSize.set( mMaxWidth + mTextOffset, mItemHeight );
   setSize(Point2I(1, mVisibleItems.size()));
   syncSelection();

   // Done Recursing.
   mFlags.clear( BuildingVisTree );
}

//------------------------------------------------------------------------------

bool GuiTreeViewCtrl::scrollVisible( S32 itemId )
{
   Item* item = getItem(itemId);
   if(item)
      return scrollVisible(item);

   return false;
}

//-----------------------------------------------------------------------------

bool GuiTreeViewCtrl::scrollVisible( Item *item )
{
   // Now, make sure it's visible (ie, all parents expanded)
   Item *parent = item->mParent;

   if( !item->isInspectorData() && item->mState.test(Item::VirtualParent) )
      onVirtualParentExpand(item);

   while(parent)
   {
      parent->setExpanded(true);

      if( !parent->isInspectorData() && parent->mState.test(Item::VirtualParent) )
         onVirtualParentExpand(parent);

      parent = parent->mParent;
   }

   // Get our scroll-pappy, if any.
   GuiScrollCtrl *pScrollParent = dynamic_cast<GuiScrollCtrl*>( getParent() );

   if ( !pScrollParent )
   {
      Con::warnf("GuiTreeViewCtrl::scrollVisible - parent control is not a GuiScrollCtrl!");
      return false;
   }

   // And now, build the visible tree so we know where we have to scroll.
   if( mFlags.test( RebuildVisible ) )
      buildVisibleTree();

   // All done, let's figure out where we have to scroll...
   for(S32 i=0; i<mVisibleItems.size(); i++)
   {
      if(mVisibleItems[i] == item)
      {
         // Fetch X Details.
         const S32 xPos   = pScrollParent->getChildRelPos().x;
         const S32 xWidth = ( mMaxWidth - xPos );

         // Scroll to View the Item.
         // Note: Delta X should be 0 so that we maintain the X axis position.
         pScrollParent->scrollRectVisible( RectI( xPos, i * mItemHeight, xWidth, mItemHeight ) );
         return true;
      }
   }

   // If we got here, it's probably bad...
   Con::errorf("GuiTreeViewCtrl::scrollVisible - was unable to find specified item in visible list!");
   return false;
}

//------------------------------------------------------------------------------

S32 GuiTreeViewCtrl::insertItem(S32 parentId, const char * text, const char * value, const char * iconString, S16 normalImage, S16 expandedImage)
{
   if( ( parentId < 0 ) || ( parentId > mItems.size() ) )
   {
      Con::errorf(ConsoleLogEntry::General, "GuiTreeViewCtrl::insertItem: invalid parent id!");
      return 0;
   }

   if((parentId != 0) && (mItems[parentId-1] == 0))
   {
      Con::errorf(ConsoleLogEntry::General, "GuiTreeViewCtrl::insertItem: parent item invalid!");
      return 0;
   }


   const char * pItemText  = ( text != NULL ) ? text : "";
   const char * pItemValue = ( value != NULL ) ? value : "";

   S32 icon = getIcon(iconString);

   // create an item (assigns id)
   Item * pNewItem = createItem(icon);
   if( pNewItem == NULL )
      return 0;

   pNewItem->setText( StringTable->insert( pItemText, true ) );
   pNewItem->setValue( StringTable->insert( pItemValue, true ) );
   
   pNewItem->setNormalImage( normalImage );
   pNewItem->setExpandedImage( expandedImage );

   // root level?
   if(parentId == 0)
   {
      // insert back
      if( mRoot != NULL )
      {
         Item * pTreeTraverse = mRoot;
         while( pTreeTraverse != NULL && pTreeTraverse->mNext != NULL )
            pTreeTraverse = pTreeTraverse->mNext;

         pTreeTraverse->mNext = pNewItem;
         pNewItem->mPrevious = pTreeTraverse;
      }
      else
         mRoot = pNewItem;

      mFlags.set(RebuildVisible);
   }
   else if( mItems.size() >= ( parentId - 1 ) )
   {
      Item * pParentItem = mItems[parentId-1];

      // insert back
      if( pParentItem != NULL && pParentItem->mChild)
      {
         Item * pTreeTraverse = pParentItem->mChild;
         while( pTreeTraverse != NULL && pTreeTraverse->mNext != NULL )
            pTreeTraverse = pTreeTraverse->mNext;

         pTreeTraverse->mNext = pNewItem;
         pNewItem->mPrevious = pTreeTraverse;
      }
      else
         pParentItem->mChild = pNewItem;

      pNewItem->mParent = pParentItem;

      if( pParentItem->isExpanded() )
         mFlags.set(RebuildVisible);
   }

   return pNewItem->mId;
}

//------------------------------------------------------------------------------

bool GuiTreeViewCtrl::removeItem( S32 itemId, bool deleteObjects )
{
   if( isSelected( itemId ) )
      removeSelection( itemId );
      
   // tree?
   if(itemId == 0)
   {
      //RD: this does not delete objects and thus isn't coherent with the semantics of this method
      
      _destroyTree();
      return(true);
   }

   Item * item = getItem(itemId);
   if(!item)
   {
      //Con::errorf(ConsoleLogEntry::General, "GuiTreeViewCtrl::removeItem: invalid item id!");
      return false;
   }

   // root?
   if(item == mRoot)
      mRoot = item->mNext;

   // Dispose of any children...
   if (item->mChild)
      _destroyChildren( item->mChild, item, deleteObjects );

   // Kill the item...
   _destroyItem( item, deleteObjects );

   // Update the rendered tree...
   mFlags.set(RebuildVisible);

   return true;
}


//-----------------------------------------------------------------------------

void GuiTreeViewCtrl::removeAllChildren(S32 itemId)
{
   Item * item = getItem(itemId);
   if(item)
   {
      _destroyChildren(item->mChild, item);
   }
}
//------------------------------------------------------------------------------

const S32 GuiTreeViewCtrl::getFirstRootItem() const
{
   return (mRoot ? mRoot->mId : 0);
}

//------------------------------------------------------------------------------

S32 GuiTreeViewCtrl::getChildItem(S32 itemId)
{
   Item * item = getItem(itemId);
   if(!item)
   {
      Con::errorf(ConsoleLogEntry::General, "GuiTreeViewCtrl::getChild: invalid item id!");
      return(0);
   }

   return(item->mChild ? item->mChild->mId : 0);
}

//-----------------------------------------------------------------------------

S32 GuiTreeViewCtrl::getParentItem(S32 itemId)
{
   Item * item = getItem(itemId);
   if(!item)
   {
      Con::errorf(ConsoleLogEntry::General, "GuiTreeViewCtrl::getParent: invalid item id!");
      return(0);
   }

   return(item->mParent ? item->mParent->mId : 0);
}

//-----------------------------------------------------------------------------

S32 GuiTreeViewCtrl::getNextSiblingItem(S32 itemId)
{
   Item * item = getItem(itemId);
   if(!item)
   {
      Con::errorf(ConsoleLogEntry::General, "GuiTreeViewCtrl::getNextSibling: invalid item id!");
      return(0);
   }

   return(item->mNext ? item->mNext->mId : 0);
}

//-----------------------------------------------------------------------------

S32 GuiTreeViewCtrl::getPrevSiblingItem(S32 itemId)
{
   Item * item = getItem(itemId);
   if(!item)
   {
      Con::errorf(ConsoleLogEntry::General, "GuiTreeViewCtrl::getPrevSibling: invalid item id!");
      return(0);
   }

   return(item->mPrevious ? item->mPrevious->mId : 0);
}

//------------------------------------------------------------------------------

bool GuiTreeViewCtrl::isValidDragTarget( Item* item )
{
   bool isValid = true;
   
   // If this is inspector data, first make sure the item accepts all
   // selected objects as children.  This prevents bad surprises when
   // certain SimSet subclasses reject children and start shoving them
   // off to places of their own choosing.

   if( item->isInspectorData() )
   {
      if( mDebug )
         Con::printf( "Checking %i:%s as drag-parent",
            item->getObject()->getId(), item->getObject()->getClassName() );
      
      SimSet* set = dynamic_cast< SimSet*>( item->getObject() );
      if( set )
      {
         for( U32 i = 0; i < mSelectedItems.size(); ++ i )
         {
            Item* selectedItem = mSelectedItems[ i ];
            
            if( mDebug )
               Con::printf( "Checking %i:%s as drag-object",
                  selectedItem->getObject()->getId(),
                  selectedItem->getObject()->getClassName() );
            
            if(    selectedItem->isInspectorData()
                && !set->acceptsAsChild( selectedItem->getObject() ) )
               return false;
         }
      }
   }
      
   if( isMethod( "isValidDragTarget" ) )
   {
      // We have a callback.  Exclusively leave the decision whether
      // the item is a valid drag target to it.
      
      isValid = isValidDragTarget_callback( item->mId, getItemValue( item->mId ) );
   }
   else
   {
      // Make the item a valid drag target if it either already is
      // a parent (including VirtualParents) or if dragging to non-parent
      // items is explicitly allowed.
      
      isValid = item->isParent() || mDragToItemAllowed;
   }
   
   return isValid;
}

//------------------------------------------------------------------------------

S32 GuiTreeViewCtrl::getItemCount()
{
   return(mItemCount);
}

//-----------------------------------------------------------------------------

S32 GuiTreeViewCtrl::getSelectedItem()
{
   return mSelectedItem;
}

//------------------------------------------------------------------------------

void GuiTreeViewCtrl::moveItemUp( S32 itemId )
{
   GuiTreeViewCtrl::Item* pItem = getItem( itemId );
   if ( !pItem )
   {
      Con::errorf( ConsoleLogEntry::General, "GuiTreeViewCtrl::moveItemUp: invalid item id!");
      return;
   }

   Item * pParent   = pItem->mParent;
   Item * pPrevItem = pItem->mPrevious;
   if ( pPrevItem == NULL || pParent == NULL )
   {
      Con::errorf( ConsoleLogEntry::General, "GuiTreeViewCtrl::moveItemUp: Unable to move item up, bad data!");
      return;
   }

   //  Diddle the linked list!
   if ( pPrevItem->mPrevious )
      pPrevItem->mPrevious->mNext = pItem;
   else if ( pItem->mParent )
      pItem->mParent->mChild = pItem;

   if ( pItem->mNext )
      pItem->mNext->mPrevious = pPrevItem;

   pItem->mPrevious = pPrevItem->mPrevious;
   pPrevItem->mNext = pItem->mNext;
   pItem->mNext = pPrevItem;
   pPrevItem->mPrevious = pItem;

   // Update SimObjects if Appropriate.
   SimObject * pSimObject = NULL;
   SimSet    * pParentSet = NULL;

   // Fetch Current Add Set
   if( pParent->isInspectorData() )
      pParentSet = dynamic_cast<SimSet*>( pParent->getObject() );
   else
   {
      // parent is probably script data so we search up the tree for a
      // set to put our object in
      Item * pTraverse = pItem->mParent;
      while ( pTraverse != NULL && !pTraverse->isInspectorData() )
         pTraverse = pTraverse->mParent;

      // found an ancestor who is an inspectorData?
      if (pTraverse != NULL)
         pParentSet = pTraverse->isInspectorData() ? dynamic_cast<SimSet*>( pTraverse->getObject() ) : NULL;
   }

   // Reorder the item and make sure that the children of the item get updated
   // correctly prev item may be script... so find a prevItem if there is.
   // We only need to reorder if there you move it above an inspector item.
   if ( pSimObject != NULL && pParentSet != NULL )
   {
      Item * pTraverse = pItem->mNext;

      while(pTraverse)
      {
         if (pTraverse->isInspectorData())
            break;
         pTraverse = pTraverse->mNext;
      }

      if (pTraverse && pItem->getObject() &&  pTraverse->getObject())
         pParentSet->reOrder(pItem->getObject(), pTraverse->getObject());
   }

   mFlags.set(RebuildVisible);
}
//-----------------------------------------------------------------------------

void GuiTreeViewCtrl::moveItemDown( S32 itemId )
{
   GuiTreeViewCtrl::Item* item = getItem( itemId );
   if ( !item )
   {
      Con::errorf( ConsoleLogEntry::General, "GuiTreeViewCtrl::moveItemDown: invalid item id!");
      return;
   }

   Item* nextItem = item->mNext;
   if ( !nextItem )
   {
      Con::errorf( ConsoleLogEntry::General, "GuiTreeViewCtrl::moveItemDown: no next sibling?");
      return;
   }

   //  Diddle the linked list!
   if ( nextItem->mNext )
      nextItem->mNext->mPrevious = item;

   if ( item->mPrevious )
      item->mPrevious->mNext = nextItem;
   else if ( item->mParent )
      item->mParent->mChild = nextItem;

   item->mNext = nextItem->mNext;
   nextItem->mPrevious = item->mPrevious;
   item->mPrevious = nextItem;
   nextItem->mNext = item;

   // And update the simobjects if apppropriate...
   SimObject * simobj = NULL;
   if (item->isInspectorData())
      simobj = item->getObject();

   SimSet *parentSet = NULL;

   // grab the current parentSet if there is any...
   if(item->mParent->isInspectorData())
      parentSet = dynamic_cast<SimSet*>(item->mParent->getObject());
   else
   {
      // parent is probably script data so we search up the tree for a
      // set to put our object in
      Item * temp = item->mParent;
      while (temp && !temp->isInspectorData())
         temp = temp->mParent;

      // found an ancestor who is an inspectorData?
      parentSet = (temp && temp->isInspectorData()) ? dynamic_cast<SimSet*>(temp->getObject()) : NULL;
   }

   // Reorder the item and make sure that the children of the item get updated
   // correctly prev item may be script... so find a prevItem if there is.
   // We only need to reorder if there you move it above an inspector item.
   if (simobj && parentSet)
   {
      Item * temp = item->mPrevious;

      while(temp)
      {
         if (temp->isInspectorData())
            break;
         temp = temp->mPrevious;
      }

      if (temp && item->getObject() && temp->getObject())
         parentSet->reOrder(temp->getObject(), item->getObject());
   }

   mFlags.set(RebuildVisible);
}

//------------------------------------------------------------------------------

bool GuiTreeViewCtrl::onAdd()
{
   if( !Parent::onAdd() )
      return false;
      
   // If we have dynamic fields, convert the "internalNamesOnly" and "objectNamesOnly"
   // legacy fields.
      
   if( getFieldDictionary() )
   {
      static StringTableEntry sInternalNamesOnly = StringTable->insert( "internalNamesOnly" );
      static StringTableEntry sObjectNamesOnly = StringTable->insert( "objectNamesOnly" );
      
      const char* internalNamesOnly = getDataField( sInternalNamesOnly, NULL );
      if( internalNamesOnly && internalNamesOnly[ 0 ] && dAtob( internalNamesOnly ) )
      {
         mShowObjectIds = false;
         mShowClassNames = false;
         mShowObjectNames = false;
         mShowInternalNames = true;
      }

      const char* objectNamesOnly = getDataField( sObjectNamesOnly, NULL );
      if( objectNamesOnly && objectNamesOnly[ 0 ] && dAtob( objectNamesOnly ) )
      {
         mShowObjectIds = false;
         mShowClassNames = false;
         mShowObjectNames = true;
         mShowInternalNames = false;
      }
   }
      
   return true;
}

//------------------------------------------------------------------------------

bool GuiTreeViewCtrl::onWake()
{
   if(!Parent::onWake() || !mProfile->constructBitmapArray())
      return false;

   // If destroy on sleep, then we have to give things a chance to rebuild.
   if(mDestroyOnSleep)
   {
      onDefineIcons_callback();
   }

   // Update the row height, if appropriate.
   if(mProfile->mAutoSizeHeight)
   {
      // make sure it's big enough for both bitmap AND font...
      mItemHeight = getMax((S32)mFont->getHeight(), (S32)mProfile->mBitmapArrayRects[0].extent.y);
   }

   return true;
}

//-----------------------------------------------------------------------------

void GuiTreeViewCtrl::onSleep()
{
   Parent::onSleep();

   // If appropriate, blast the tree. (We probably rebuild it on wake.)
   if( mDestroyOnSleep )
      _destroyTree();

   if ( mRenameCtrl )
   {
      mRenameCtrl->deleteObject();
      mRenameCtrl = NULL;
   }
}

//-----------------------------------------------------------------------------

bool GuiTreeViewCtrl::buildIconTable(const char * icons)
{
   // Icons should be designated by the bitmap/png file names (minus the file extensions)
   // and separated by colons (:). This list should be synchronized with the Icons enum.

   // Figure the size of the buffer we need...
   const char* temp = dStrchr( icons, '\t' );
   U32 textLen = temp ? ( temp - icons ) : dStrlen( icons );

   // Allocate temporary space.
   FrameAllocatorMarker txtBuff;
   char* drawText = (char*)txtBuff.alloc(sizeof(char) * (textLen + 4));
   dStrncpy( drawText, icons, textLen );
   drawText[textLen] = '\0';

   U32 numIcons = 0;
   char buf[ 1024 ];
   char* pos = drawText;

   // Count the number of icons and store them.
   while( *pos && numIcons < MaxIcons )
   {
      char* start = pos;
      while( *pos && *pos != ':' )
         pos ++;
      
      const U32 len = pos - start;
      if( len )
      {
         dStrncpy( buf, start, getMin( sizeof( buf ) / sizeof( buf[ 0 ] ) - 1, len ) );
         buf[ len ] = '\0';
                  
         mIconTable[ numIcons ] = GFXTexHandle( buf, &GFXDefaultPersistentProfile, avar( "%s() - mIconTable[%d] (line %d)", __FUNCTION__, numIcons, __LINE__ ) );
      }
      else
         mIconTable[ numIcons ] = GFXTexHandle();
      
      numIcons ++;
      if( *pos )
         pos ++;
   }

   return true;
}

//------------------------------------------------------------------------------

void GuiTreeViewCtrl::onPreRender()
{
   Parent::onPreRender();

   S32 nRootItemId = getFirstRootItem();
   if( nRootItemId == 0 )
      return;

   Item *pRootItem = getItem( nRootItemId );
   if( pRootItem == NULL )
      return;

   // Update every render in case new objects are added
   if(mFlags.test(RebuildVisible))
   {
      buildVisibleTree();
	  mFlags.clear(RebuildVisible);
   }
}

//------------------------------------------------------------------------------

bool GuiTreeViewCtrl::_hitTest(const Point2I & pnt, Item* & item, BitSet32 & flags)
{
   // Initialize some things.
   const Point2I pos = globalToLocalCoord(pnt);
   flags.clear();
   item = 0;

   // get the hit cell
   Point2I cell((pos.x < 0 ? -1 : pos.x / mCellSize.x),
                (pos.y < 0 ? -1 : pos.y / mCellSize.y));

   // valid?
   if((cell.x < 0 || cell.x >= mSize.x) ||
      (cell.y < 0 || cell.y >= mSize.y))
      return false;

   flags.set(OnRow);

   // Grab the cell.
   if (cell.y >= mVisibleItems.size())
      return false; //Invalid cell, so don't do anything

   item = mVisibleItems[cell.y];

   S32 min = mTabSize * item->mTabLevel;

   // left of icon/text?
   if(pos.x < min)
   {
      flags.set(OnIndent);
      return true;
   }

   // check image
   S32 image = BmpChild;

   if(item->isInspectorData())
      image = item->isExpanded() ? BmpExp : BmpCon;
   else
      image = item->isExpanded() ? item->getExpandedImage() : item->getNormalImage();

   if((image >= 0) && (image < mProfile->mBitmapArrayRects.size()))
      min += mProfile->mBitmapArrayRects[image].extent.x;

   // Is it on the image?
   if(pos.x < min)
   {
      flags.set(OnImage);
      return(true);
   }

   // Check the icon.
   min += getInspectorItemIconsWidth( item );

   if ( pos.x < min )
   {
      flags.set(OnIcon);
      return true;
   }

   // Check the text.

   min += mProfile->mTextOffset.x;

   FrameAllocatorMarker txtAlloc;
   U32 bufLen = item->getDisplayTextLength() + 1;
   char *buf = (char*)txtAlloc.alloc(bufLen);
   item->getDisplayText(bufLen, buf);

   min += mProfile->mFont->getStrWidth(buf);
   if(pos.x < min)
      flags.set(OnText);

   return true;
}

//-----------------------------------------------------------------------------

S32 GuiTreeViewCtrl::getInspectorItemIconsWidth(Item* & item)
{
   S32 width = 0;

   if( item->isInspectorData() )
   {
      // Based on code in onRenderCell()

      S32 icon = Lock1;
      S32 icon2 = Hidden;

      if (item->getObject() && item->getObject()->isLocked())
      {
         if (mIconTable[icon])
         {
            width += mIconTable[icon].getWidth();
         }
      }

      if (item->getObject() && item->getObject()->isHidden())
      {
         if (mIconTable[icon2])
         {
            width += mIconTable[icon2].getWidth();
         }
      }

      GFXTexHandle iconHandle;
      if ( ( item->mIcon != -1 ) && mIconTable[item->mIcon] )
         iconHandle = mIconTable[item->mIcon];
   #ifdef TORQUE_TOOLS
      else
         iconHandle = gEditorIcons.findIcon( item->getObject() );
   #endif

      if ( iconHandle.isValid() )
      {
         width += iconHandle.getWidth();
      }
   }
   else
   {
      S32 icon = item->isExpanded() ? item->mScriptInfo.mExpandedImage : item->mScriptInfo.mNormalImage;
      if ( ( icon != -1 ) && mIconTable[icon] )
      {
         width += mIconTable[icon].getWidth();
      }
   }

   return width;
}

//-----------------------------------------------------------------------------

bool GuiTreeViewCtrl::setAddGroup(SimObject * obj)
{
   // make sure we're talking about a group.
   SimGroup * grp = dynamic_cast<SimGroup*>(obj);

   if(grp)
   {
      onAddGroupSelected_callback( grp );
      return true;
   }
   return false;
}

//-----------------------------------------------------------------------------

void GuiTreeViewCtrl::syncSelection()
{
   // for each visible item check to see if it is on the mSelected list.
   // if it is then make sure that it is on the mSelectedItems list as well.
   for (S32 i = 0; i < mVisibleItems.size(); i++) 
   {
      for (S32 j = 0; j < mSelected.size(); j++) 
      {
         if (mVisibleItems[i]->mId == mSelected[j]) 
         {
            // check to see if it is on the visible items list.
            bool addToSelectedItems = true;
            for (S32 k = 0; k < mSelectedItems.size(); k++) 
            {
               if (mSelected[j] == mSelectedItems[k]->mId) 
               {
                  // don't add it
                  addToSelectedItems = false;
               }
            }
            if (addToSelectedItems) 
            {
               mVisibleItems[i]->mState.set(Item::Selected, true);
               mSelectedItems.push_front(mVisibleItems[i]);
               break;
            }
         } 
         else if (mVisibleItems[i]->isInspectorData()) 
         {
			if(mCompareToObjectID)
			{
			   if (mVisibleItems[i]->getObject() && mVisibleItems[i]->getObject()->getId() == mSelected[j]) 
			   {
				  // check to see if it is on the visible items list.
				  bool addToSelectedItems = true;
				  for (S32 k = 0; k < mSelectedItems.size(); k++) 
				  {
					 if (mSelectedItems[k]->isInspectorData() && mSelectedItems[k]->getObject() )
					 {
						if (mSelected[j] == mSelectedItems[k]->getObject()->getId()) 
						{
						   // don't add it
						   addToSelectedItems = false;
						}
					 } 
					 else 
					 {
						if (mSelected[j] == mSelectedItems[k]->mId) 
						{
						   // don't add it
						   addToSelectedItems = false;
						}
					 }
				  }
				  if (addToSelectedItems) 
				  {
					 mVisibleItems[i]->mState.set(Item::Selected, true);
					 mSelectedItems.push_front(mVisibleItems[i]);
					 break;
				  }
			   }
			}
         }

      }

   }
}

//-----------------------------------------------------------------------------

void GuiTreeViewCtrl::removeSelection( S32 itemOrObjectId )
{
   if (mDebug)
      Con::printf( "removeSelection %i", itemOrObjectId );

   Item* item = _findItemByAmbiguousId( itemOrObjectId, false ); 
   if (!item) 
      return;

   // Make sure we have a true item ID even if we started with
   // an object ID.
   S32 itemId = item->getID();

   S32 objectId = -1;
   if ( item->isInspectorData() && item->getObject() )   
      objectId = item->getObject()->getId();   

   // Remove from vector of selected object ids if it exists there
   if ( objectId != -1 )
   {
      for ( S32 i = 0; i < mSelected.size(); i++ ) 
      {
         if ( objectId == mSelected[i] || itemId == mSelected[i] ) 
         {
            mSelected.erase( i );
            break;
         }
      }
   }
   else
   {
      for ( S32 i = 0; i < mSelected.size(); i++ ) 
      {
         if ( itemId == mSelected[i] ) 
         {
            mSelected.erase( i );
            break;
         }
      }
   }

   item->mState.set(Item::Selected, false);
   
   // Remove from vector of selected items if it exists there.
   for ( S32 i = 0; i < mSelectedItems.size(); i++ ) 
   {
      if ( mSelectedItems[i] == item ) 
      {
         mSelectedItems.erase( i );
         break;
      }
   }

   // Callback.
   onRemoveSelection( item );
}

//-----------------------------------------------------------------------------

void GuiTreeViewCtrl::addSelection( S32 itemOrObjectId, bool update, bool isLastSelection )
{
   if (mDebug)
      Con::printf( "addSelection %i", itemOrObjectId );

   Item* item = _findItemByAmbiguousId( itemOrObjectId );

   // Add Item?
   if ( !item || isSelected( item ) || !canAddSelection( item ) )
   {
      // Nope.
      return;
   }

   const S32 itemId = item->getID();
	
	// Ok, we have an item to select which isn't already selected....

   // Do we want to allow more than one selected item?
   if( !mMultipleSelections )
      clearSelection();

	// Add this object id to the vector of selected objectIds
   // if it is not already.
   bool foundMatch = false;
   for ( S32 i = 0; i < mSelected.size(); i++)
   {
      if ( mSelected[i] == itemId )
         foundMatch = true;
   }
   
   if ( !foundMatch )
      mSelected.push_front(itemId);

   item->mState.set(Item::Selected, true);

   if( mSelected.size() == 1 )
   {
      onItemSelected( item );
   }

   // Callback Start
   // Set and add the selection to the selected items group
	item->mState.set(Item::Selected, true);
	mSelectedItems.push_front(item);

   if ( item->isInspectorData() && 
        item->getObject() )
   { 
      SimObject *obj = item->getObject();
		
      onAddSelection_callback( obj->getId(), isLastSelection );
   }
   else
   {
      onAddSelection_callback( item->mId, isLastSelection );
   }
	// Callback end

   mFlags.set( RebuildVisible );
   if( update )
   {
      // Also make it so we can see it if we didn't already.
      scrollVisible( item );
   }
}


//-----------------------------------------------------------------------------

void GuiTreeViewCtrl::onItemSelected( Item *item )
{
   mSelectedItem = item->getID();

   if (item->isInspectorData())
   {
      SimObject* object = item->getObject();
	   if( object )
         onSelect_callback( object->getId() );
      if( !item->isParent() && object )
         onInspect_callback( object->getId() );
   }
   else
   {
      onSelect_callback( item->mId );
      if( !item->isParent() )
         onInspect_callback( item->mId );
   }
}

//-----------------------------------------------------------------------------

void GuiTreeViewCtrl::onRemoveSelection( Item *item )
{
   S32 id = item->mId;

   if( item->isInspectorData() &&
       item->getObject() )
   {      
      SimObject* obj = item->getObject();
      id = obj->getId();
      //obj->setSelected( false );
   }

   if( isMethod( "onRemoveSelection" ) )
      onRemoveSelection_callback( id );
   else
      onUnselect_callback( id );
}

//-----------------------------------------------------------------------------

bool GuiTreeViewCtrl::setItemSelected(S32 itemId, bool select)
{
   Item * item = getItem(itemId);
   if( isSelected( item ) == select )
      return true;

   if (select)
   {
      if (mDebug) Con::printf("setItemSelected called true");

      mSelected.push_front(itemId);
   }
   else
   {
      if (mDebug) Con::printf("setItemSelected called false");

      // remove it from the mSelected list
      for (S32 j = 0; j <mSelected.size(); j++)
      {
         if (item)
         {
            if (item->isInspectorData())
            {
               if (item->getObject())
               {
                  if(item->getObject()->getId() == mSelected[j])
                  {
                     mSelected.erase(j);
                     break;
                  }
               }
               else
               {
                  // Zombie, kill it!
                  mSelected.erase(j);
                  j--;
                  break;
               }
            }
         }

         if (mSelected[j] == itemId)
         {
            mSelected.erase(j);
            break;
         }
      }
   }

   if(!item)
   {
      // maybe what we were passed wasn't an item id but an object id.
      for (S32 i = 0; i <mItems.size(); i++)
      {
         if (mItems[i] != 0)
         {
            if (mItems[i]->isInspectorData())
            {
               if (mItems[i]->getObject())
               {
                  if(mItems[i]->getObject()->getId() == itemId)
                  {
                     item = mItems[i];
                     break;
                  }
               }
               else
               {
                  // It's a zombie, blast it.
                  mItems.erase(i);
                  i--;
               }
            }
         }
      }

      if (!item)
      {
         //Con::errorf(ConsoleLogEntry::General, "GuiTreeViewCtrl::setItemSelected: invalid item id! Perhaps it isn't visible yet.");
         return(false);
      }
   }

   mFlags.set( RebuildVisible );

   if(select)
   {
      addSelection( item->mId );
      onItemSelected( item );
   }
   else
   {
      // deselect the item, if it's present.
      item->mState.set(Item::Selected, false);

      if (item->isInspectorData() && item->getObject())
         onUnselect_callback( item->getObject()->getId() );
      else
         onUnselect_callback( item->mId );

      // remove it from the selected items list
      for (S32 i = 0; i < mSelectedItems.size(); i++)
      {
         if (mSelectedItems[i] == item)
         {
            mSelectedItems.erase(i);
            break;
         }
      }
   }

   setUpdate();
   return(true);
}

//-----------------------------------------------------------------------------

// Given an item's index in the selection list, return its itemId
S32 GuiTreeViewCtrl::getSelectedItem(S32 index)
{
   if(index >= 0 && index < getSelectedItemsCount())
   {
      return mSelectedItems[index]->mId;
   }

   return -1;
}

//-----------------------------------------------------------------------------

bool GuiTreeViewCtrl::setItemExpanded(S32 itemId, bool expand)
{
   Item * item = getItem(itemId);
   if(!item)
   {
      Con::errorf(ConsoleLogEntry::General, "GuiTreeViewCtrl::setItemExpanded: invalid item id!");
      return(false);
   }

   if(item->isExpanded() == expand)
      return(true);

   // expand parents
   if(expand)
   {
      while(item)
      {
         if(item->mState.test(Item::VirtualParent))
            onVirtualParentExpand(item);

         item->setExpanded(true);
         item = item->mParent;
      }
   }
   else
   {
      if(item->mState.test(Item::VirtualParent))
         onVirtualParentCollapse(item);

      item->setExpanded(false);
   }
   return(true);
}


//-----------------------------------------------------------------------------

bool GuiTreeViewCtrl::setItemValue(S32 itemId, StringTableEntry Value)
{
   Item * item = getItem(itemId);
   if(!item)
   {
      Con::errorf(ConsoleLogEntry::General, "GuiTreeViewCtrl::setItemValue: invalid item id!");
      return(false);
   }

   item->setValue( ( Value ) ? Value : "" );

   return(true);
}

//-----------------------------------------------------------------------------

const char * GuiTreeViewCtrl::getItemText(S32 itemId)
{
   Item * item = getItem(itemId);
   if(!item)
   {
      Con::errorf(ConsoleLogEntry::General, "GuiTreeViewCtrl::getItemText: invalid item id!");
      return("");
   }

   return(item->getText() ? item->getText() : "");
}

//-----------------------------------------------------------------------------

const char * GuiTreeViewCtrl::getItemValue(S32 itemId)
{
   Item * item = getItem(itemId);
   if(!item)
   {
      Con::errorf(ConsoleLogEntry::General, "GuiTreeViewCtrl::getItemValue: invalid item id!");
      return("");
   }

   if(item->mState.test(Item::InspectorData))
   {
      // If it's InspectorData, we let people use this call to get an object reference.
      return item->mInspectorInfo.mObject->getIdString();
   }
   else
   {
      // Just return the script value...
      return item->getValue();
   }
}

//-----------------------------------------------------------------------------

bool GuiTreeViewCtrl::editItem( S32 itemId, const char* newText, const char* newValue )
{
   Item* item = getItem( itemId );
   if ( !item )
   {
      Con::errorf(ConsoleLogEntry::General, "GuiTreeViewCtrl::editItem: invalid item id: %d!", itemId);
      return false;
   }

   if ( item->mState.test(Item::InspectorData) )
   {
      Con::errorf(ConsoleLogEntry::General, "GuiTreeViewCtrl::editItem: item %d is inspector data and may not be modified!", itemId);
      return false;
   }

   item->setText( StringTable->insert( newText, true ) );
   item->setValue( StringTable->insert( newValue, true ) );

   // Update the widths and such:
   mFlags.set(RebuildVisible);
   return true;
}

//-----------------------------------------------------------------------------

bool GuiTreeViewCtrl::markItem( S32 itemId, bool mark )
{
   Item *item = getItem( itemId );
   if ( !item )
   {
      Con::errorf(ConsoleLogEntry::General, "GuiTreeViewCtrl::markItem: invalid item id: %d!", itemId);
      return false;
   }

   item->mState.set(Item::Marked, mark);
   return true;
}

//-----------------------------------------------------------------------------

bool GuiTreeViewCtrl::isItemSelected( S32 itemId )
{
   for( U32 i = 0, num = mSelectedItems.size(); i < num; ++ i )
      if( mSelectedItems[ i ]->mId == itemId )
         return true;
         
   return false;
}

//-----------------------------------------------------------------------------

void GuiTreeViewCtrl::deleteSelection()
{
   onDeleteSelection_callback();

   if (mSelectedItems.empty())
   {
      for (S32 i = 0; i < mSelected.size(); i++)
      {
         S32 objectId = mSelected[i];

         // find the object
         SimObject* obj = Sim::findObject(objectId);
         if ( !obj )
            continue;

         bool skipDelete = onDeleteObject_callback( obj );
         if ( !skipDelete )
            obj->deleteObject();
      }
   }
   else
   {
	  Vector<Item*> delSelection;
	  delSelection = mSelectedItems;
	  mSelectedItems.clear();
      while (!delSelection.empty())
      {
         Item * item = delSelection.front();
         setItemSelected(item->mId,false);
         if ( item->mParent )
            _deleteItem( item );
         
		 delSelection.pop_front();      
      }
   }

   mSelected.clear();
   mSelectedItems.clear();
   mSelectedItem = 0;

   onObjectDeleteCompleted_callback();
}

//------------------------------------------------------------------------------

// keyboard movement of items is restricted to just one item at a time
// if more than one item is selected then movement operations are not performed
bool GuiTreeViewCtrl::onKeyDown( const GuiEvent& event )
{
   if ( !mVisible || !mActive || !mAwake )
      return false;

   // All the keyboard functionality requires a selected item, so if none exists...

   // Deal with enter and delete
   if ( event.modifier == 0 )
   {
      if ( event.keyCode == KEY_RETURN )
      {
         execAltConsoleCallback();
         return true;
      }

      if ( event.keyCode == KEY_DELETE && mDeleteObjectAllowed )
      {
         // Don't delete the root!
         if (mSelectedItems.empty())
            return true;

         //this may be fighting with the world editor delete
         deleteSelection();
         return true;
      }
 
	  //call a generic bit of script that will let the subclass know that a key was pressed
     onKeyDown_callback( event.modifier, event.keyCode );
   }

   // only do operations if only one item is selected
   if ( mSelectedItems.empty() || (mSelectedItems.size() > 1))
      return false;

   Item* item = mSelectedItems.first();

   if ( !item )
      return false;

   // The Alt key lets you move items around!
   if ( mFlags.test(IsEditable) && event.modifier & SI_ALT )
   {
      switch ( event.keyCode )
      {
      case KEY_UP:
         // Move us up.
         if ( item->mPrevious )
         {
            moveItemUp( item->mId );
            scrollVisible(item);
         }
         return true;

      case KEY_DOWN:
         // Move the item under us up.
         if ( item->mNext )
         {
            moveItemUp( item->mNext->mId );
            scrollVisible(item);
         }
         return true;

      case KEY_LEFT:
         if ( item->mParent )
         {
            if ( item->mParent->mParent )
            {
               // Ok, we have both an immediate parent, and a grandparent.

               // The goal of left-arrow alt is to become the child of our
               // grandparent, ie, to become a sibling of our parent.

               // First, unlink item from its siblings.
               if ( item->mPrevious )
                  item->mPrevious->mNext = item->mNext;
               else
                  item->mParent->mChild = item->mNext;

               if ( item->mNext )
                  item->mNext->mPrevious = item->mPrevious;

               // Now, relink as the next sibling of our parent.
               item->mPrevious = item->mParent;
               item->mNext = item->mParent->mNext;

               // If there was already a next sibling, deal with that case.
               if ( item->mNext )
                  item->mNext->mPrevious = item;
               item->mParent->mNext = item;

               // Snag the current parent set if any...
               SimSet *parentSet = NULL;

               if(item->mParent->isInspectorData())
                  parentSet = dynamic_cast<SimSet*>(item->mParent->getObject());
               else
               {
                  // parent is probably script data so we search up the tree for a
                  // set to put our object in
                  Item * temp = item->mParent;
                  while (!temp->isInspectorData())
                     temp = temp->mParent;
                  // found a ancestor who is an inspectorData
                  if (temp->isInspectorData())
                     parentSet = dynamic_cast<SimSet*>(temp->getObject());
                  else parentSet = NULL;
               }

               // Get our active SimObject if any
               SimObject *simObj = NULL;
               if(item->isInspectorData())
                  simObj = item->getObject();

               // Remove from the old parentset...
               if(simObj && parentSet) {
                  if (parentSet->size()>0)
                  {
                     SimObject *lastObject = parentSet->last();
                     parentSet->removeObject(simObj);
                     parentSet->reOrder(lastObject);
                  } else
                     parentSet->removeObject(simObj);
               }

               // And finally, update our item
               item->mParent = item->mParent->mParent;

               // Snag the newparent set if any...
               SimSet *newParentSet = NULL;

               if(item->mParent->isInspectorData())
                  newParentSet = dynamic_cast<SimSet*>(item->mParent->getObject());
               else
               {
                  // parent is probably script data so we search up the tree for a
                  // set to put our object in
                  Item * temp = item->mParent;
                  while (!temp->isInspectorData())
                     temp = temp->mParent;
                  // found a ancestor who is an inspectorData
                  if (temp->isInspectorData())
                     newParentSet = dynamic_cast<SimSet*>(temp->getObject());
                  else newParentSet = NULL;
               }
               if(simObj && newParentSet)
               {

                  newParentSet->addObject(simObj);
                  Item * temp = item->mNext;
                  // item->mNext may be script, so find an inspector item to reorder with if any

                  if (temp) {
                     do {
                        if (temp->isInspectorData())
                           break;
                        temp = temp->mNext;
                     } while (temp);
                     if (temp && item->getObject() && temp->getObject()) //do we still have a item->mNext? If not then don't bother reordering
                        newParentSet->reOrder(item->getObject(), temp->getObject());
                  }

               } else if (!simObj&&newParentSet) {
                  // our current item is script data. but it may have children who
                  // is inspector data who need an updated set
                  if (item->mChild)
                     inspectorSearch(item->mChild, item, parentSet, newParentSet);

               }

               // And update everything hurrah.
               buildVisibleTree();
               scrollVisible(item);
            }
         }
         return true;

      case KEY_RIGHT:
         if ( item->mPrevious )
         {
            // Make the item the last child of its previous sibling.

            // First, unlink from the current position in the list
            item->mPrevious->mNext = item->mNext;

            if ( item->mNext )
               item->mNext->mPrevious = item->mPrevious;

            // Get the object we're poking with.
            SimObject *simObj = NULL;
            SimSet *parentSet = NULL;
            if(item->isInspectorData())
               simObj = item->getObject();
            if(item->mParent->isInspectorData())
               parentSet = dynamic_cast<SimSet*>(item->mParent->getObject());
            else {
               // parent is probably script data so we search up the tree for a
               // set to put our object in
               Item * temp = item->mParent;
               while (!temp->isInspectorData())
                  temp = temp->mParent;
               // found an ancestor who is an inspectorData
               if (temp->isInspectorData())
                  parentSet = dynamic_cast<SimSet*>(temp->getObject());
            }

            // If appropriate, remove from the current SimSet.
            if(parentSet && simObj) {
               if (parentSet->size()>0)
               {
                  SimObject *lastObject = parentSet->last();
                  parentSet->removeObject(simObj);
                  parentSet->reOrder(lastObject);
               } else
                  parentSet->removeObject(simObj);
            }


            // Now, make our previous sibling our parent...
            item->mParent = item->mPrevious;
            item->mNext = NULL;

            // And sink us down to the end of its siblings, if appropriate.
            if ( item->mParent->mChild )
            {
               Item* temp = item->mParent->mChild;
               while ( temp->mNext )
                  temp = temp->mNext;

               temp->mNext = item;
               item->mPrevious = temp;
            }
            else
            {
               // only child...<sniff>
               item->mParent->mChild = item;
               item->mPrevious = NULL;
            }

            // Make sure the new parent is expanded:
            if ( !item->mParent->mState.test( Item::Expanded ) )
               setItemExpanded( item->mParent->mId, true );

            // Snag the new parent simset if any.
            SimSet *newParentSet = NULL;

            // new parent might be script. so figure out what set we need to add it to.
            if(item->mParent->isInspectorData())
               newParentSet = dynamic_cast<SimSet*>(item->mParent->getObject());
            else
            {
               // parent is probably script data so we search up the tree for a
               // set to put our object in
               if (mDebug) Con::printf("oh nos my parent is script!");
               Item * temp = item->mParent;
               while (!temp->isInspectorData())
                  temp = temp->mParent;
               // found a ancestor who is an inspectorData
               if (temp->isInspectorData())
                  newParentSet = dynamic_cast<SimSet*>(temp->getObject());
               else newParentSet = NULL;
            }
            // Add the item's SimObject to the new parent simset, at the end.
            if(newParentSet && simObj)
               newParentSet->addObject(simObj);
            else if (!simObj&&newParentSet&&parentSet) {
               // our current item is script data. but it may have children who
               // is inspector data who need an updated set

               if (item->mChild) {
                  inspectorSearch(item->mChild, item, parentSet, newParentSet);
               }

            }
            scrollVisible(item);
         }
         return true;
      
      default:
         break;
      }
   }

   // Explorer-esque navigation...
   switch( event.keyCode )
   {
   case KEY_UP:
      // Select previous visible item:
      if ( item->mPrevious )
      {
         item = item->mPrevious;
         while ( item->isParent() && item->isExpanded() )
         {
            item = item->mChild;
            while ( item->mNext )
               item = item->mNext;
         }
         clearSelection();
         addSelection( item->mId );
         return true;
      }

      // or select parent:
      if ( item->mParent )
      {
         clearSelection();
         addSelection( item->mParent->mId );
         return true;
      }

      return false;
      break;

   case KEY_DOWN:
      // Selected child if it is visible:
      if ( item->isParent() && item->isExpanded() )
      {
         clearSelection();
         addSelection( item->mChild->mId );
         return true;
      }
      // or select next sibling (recursively):
      do
      {
         if ( item->mNext )
         {
            clearSelection();
            addSelection( item->mNext->mId );
            return true;
         }

         item = item->mParent;
      } while ( item );

      return false;
      break;

   case KEY_LEFT:
      // Contract current menu:
      if ( item->isExpanded() )
      {
         setItemExpanded( item->mId, false );
         scrollVisible(item);
         return true;
      }
      // or select parent:
      if ( item->mParent )
      {
         clearSelection();
         addSelection( item->mParent->mId );
         return true;
      }

      return false;
      break;

   case KEY_RIGHT:
      // Expand selected item:
      if ( item->isParent() )
      {
         if ( !item->isExpanded() )
         {
            setItemExpanded( item->mId, true );
            scrollVisible(item);
            return true;
         }

         // or select child:
         clearSelection();
         addSelection( item->mChild->mId );
         return true;
      }

      return false;
      break;
   
   default:
      break;
   }

   // Not processed, so pass the event on:
   return Parent::onKeyDown( event );
}

//------------------------------------------------------------------------------

// on mouse up look at the current item and check to see if it is valid
// to move the selected item(s) to it.
void GuiTreeViewCtrl::onMouseUp(const GuiEvent &event)
{
   if( !mActive || !mAwake || !mVisible )
      return;

   if( isMethod("onMouseUp") )
   {
      BitSet32 hitFlags = 0;
      Item* item;
      
      S32 hitItemId = -1;
      if( _hitTest( event.mousePoint, item, hitFlags ) )
         hitItemId = item->mId;
         
      onMouseUp_callback( hitItemId, event.mouseClickCount );
   }

   mouseUnlock();

   if ( mSelectedItems.empty())
   {
      mDragMidPoint = NomDragMidPoint;
      return;
   }

	BitSet32 hitFlags = 0;
   Item *item;
	bool hitCheck = _hitTest( event.mousePoint, item, hitFlags );
   mRenamingItem = NULL;

	if( hitCheck )
	{
		if ( event.mouseClickCount == 1 && !mMouseDragged && mPossibleRenameItem != NULL )
		{
			if ( item == mPossibleRenameItem )
            showItemRenameCtrl( item );
		}
		else // If mouseUp occurs on the same item as mouse down
		{
			bool wasSelected = isSelected( item );
			bool multiSelect = getSelectedItemsCount() > 1;
			if( wasSelected && multiSelect && item == mTempItem )
			{
				clearSelection();
				addSelection( item->mId );
			}
		}
	}

   mPossibleRenameItem = NULL;
   
   if (!mMouseDragged) 
      return;

   Item* newItem = NULL;
   Item* newItem2 = NULL;

   if (mFlags.test(IsEditable))
   {
      Parent::onMouseMove( event );
         
      BitSet32 hitFlags = 0;
      if( !_hitTest( event.mousePoint, newItem2, hitFlags ) )
      {
         if( !mShowRoot )
            newItem2 = mRoot;
         else
         {
            if( mDebug )
               Con::printf( "Nothing hit" );
               
            mDragMidPoint = NomDragMidPoint;
            return;
         }
      }

      newItem2->mState.clear(Item::MouseOverBmp | Item::MouseOverText );
      
      // If the hit item is the visible root, make sure
      // we don't allow dragging above.
      
      if( newItem2 == mRoot && mDragMidPoint == AbovemDragMidPoint )
      {
         if( mDebug )
            Con::printf( "Rejecting to make child sibling of root" );
            
         mDragMidPoint = NomDragMidPoint;
         return;
      }
      
      // if the newItem isn't in the mSelectedItemList then continue.

      Vector<Item *>::iterator k;
      for(k = mSelectedItems.begin(); k != mSelectedItems.end(); k++) 
      {
         newItem = newItem2;
         
         if (*(k) == newItem) 
         {
            mDragMidPoint = NomDragMidPoint;
            return;
         }

         Item * temp = *(k);
         Item * grandpaTemp = newItem->mParent;
         
         // grandpa check, kick out if an item would be its own ancestor
         while (grandpaTemp)
         {
            if (temp == grandpaTemp)
            {
               if (mDebug)
               {
                  Con::printf("grandpa check");

                  if (temp->isInspectorData())
                     Con::printf("temp's name: %s",temp->getObject()->getName());

                  if (grandpaTemp->isInspectorData())
                     Con::printf("grandpa's name: %s",grandpaTemp->getObject()->getName());
               }

               mDragMidPoint = NomDragMidPoint;
               return;
            }

            grandpaTemp = grandpaTemp->mParent;
         }
      }
      
      // Notify script for undo.

      onBeginReparenting_callback();
      
      // Reparent the items.

      for (S32 i = 0; i <mSelectedItems.size();i++) 
      {
         newItem = newItem2;
         Item * item = mSelectedItems[i];

         if (mDebug) Con::printf("----------------------------");
      
         // clear old highlighting of the item
         item->mState.clear(Item::MouseOverBmp | Item::MouseOverText );

         // move the selected item to the newItem
         Item* oldParent = item->mParent;
         // Snag the current parent set if any for future reference
         SimSet *parentSet = NULL;

         if(oldParent->isInspectorData())
            parentSet = dynamic_cast<SimSet*>(oldParent->getObject());
         else 
         {
            // parent is probably script data so we search up the tree for a
            // set to put our object in
            Item * temp = oldParent;
            while (temp) 
            {
               if (temp->isInspectorData())
                  break;
               temp = temp->mParent;
            }
            // found an ancestor who is an inspectorData
            if (temp) 
            {
               if (temp->isInspectorData())
                  parentSet = dynamic_cast<SimSet*>(temp->getObject());
            }
         }
         
         // unlink from the current position in the list
         unlinkItem(item);

         // update the parent's children

         // check if we an only child
         if (item->mParent->mChild == item)
         {
            if (item->mNext)
               item->mParent->mChild = item->mNext;
            else
               item->mParent->mChild = NULL;
         }

         if (mDragMidPoint != NomDragMidPoint)
         {

            //if it is below an expanded tree, place as last item in the tree
            //if it is below a parent who isn't expanded put below it

            // position the item above or below another item
            if (mDragMidPoint == AbovemDragMidPoint)
            {
               // easier to treat everything as "Below the mDragMidPoint" so make some adjustments
               if (mDebug) Con::printf("adding item above mDragMidPoint");

               // above the mid point of an item, so grab either the parent
               // or the previous sibling

               // does the item have a previous sibling?
               if (newItem->mPrevious)
               {
                  newItem = newItem->mPrevious;

                  if (mDebug) Con::printf("treating as if below an item that isn't expanded");

                  // otherwise add below that item as a sibling
                  item->mParent = newItem->mParent;
                  item->mPrevious = newItem;
                  item->mNext = newItem->mNext;
                  if (newItem->mNext)
                     newItem->mNext->mPrevious = item;
                  newItem->mNext = item;                  
               } 
               else
               {
                  if (mDebug) Con::printf("treating as if adding below the parent of the item");

                  // instead we add as the first item below the newItem's parent
                  item->mParent = newItem->mParent;
                  item->mNext = newItem;
                  item->mPrevious = NULL;
                  newItem->mPrevious = item;
                  item->mParent->mChild = item;
               }
            }
            else if (mDragMidPoint == BelowmDragMidPoint)
            {
               if ((newItem->isParent())&&(newItem->isExpanded()))
               {
                  if (mDebug) Con::printf("adding item to an expanded parent below the mDragMidPoint");

                  item->mParent = newItem;

                  // then add the new item as a child
                  item->mNext = newItem->mChild;
                  if (newItem->mChild)
                     newItem->mChild->mPrevious = item;
                  item->mParent->mChild = item;
                  item->mPrevious = NULL;
               }
               else if ((!newItem->mNext)&&(newItem->mParent)&&(newItem->mParent->mParent)) 
               {
                  // add below it's parent.
                  if (mDebug) Con::printf("adding below a tree");

                  item->mParent = newItem->mParent->mParent;
                  item->mNext = newItem->mParent->mNext;
                  item->mPrevious = newItem->mParent;

                  if (newItem->mParent->mNext)
                     newItem->mParent->mNext->mPrevious = item;

                  newItem->mParent->mNext = item;
               }
               else 
               {
                  // adding below item not as a child
                  if (mDebug) Con::printf("adding item below the mDragMidPoint of an item");

                  item->mParent = newItem->mParent;
                  // otherwise the item is a sibling
                  if (newItem->mNext)
                     newItem->mNext->mPrevious = item;
                  item->mNext = newItem->mNext;
                  item->mPrevious = newItem;
                  newItem->mNext = item;
               }
            }
         }
         // if we're not allowed to add to items, then try to add to the parent of the hit item.
         // if we are, just add to the item we hit.
         else 
         {
            if (mDebug) 
            {
               if (item->isInspectorData() && item->getObject())
                  Con::printf("Item: %i",item->getObject()->getId());
               if (newItem->isInspectorData() && newItem->getObject())
                  Con::printf("Parent: %i",newItem->getObject()->getId());
               Con::printf("dragged onto an item");
            }
            
            // If the hit item is not a valid drag target,
            // then try to add to the parent.
            
            if( !isValidDragTarget( newItem ) )
            {
                // add to the item's parent.
               if(!newItem->mParent || !newItem->mParent->isParent())
               {
                  if(mDebug)
                     Con::printf("could not find the parent of that item. dragging to an item is not allowed, kicking out.");
                  mDragMidPoint = NomDragMidPoint;
                  continue;
               }
               newItem = newItem->mParent;
            }

            // new parent is the item in the current cell
            item->mParent = newItem;

            // adjust children if any
            if (newItem->mChild)
            {
               if (mDebug) Con::printf("not the first child");

               // put it at the top of the list (easier to find if there are many children)
               if (newItem->mChild)
                  newItem->mChild->mPrevious = item;
               item->mNext = newItem->mChild;
               newItem->mChild = item;
               item->mPrevious = NULL;
            }
            else 
            {
               if (mDebug) Con::printf("first child");

               // only child
               newItem->mChild = item;
               item->mNext = NULL;
               item->mPrevious = NULL;
            }
         }

         // expand the item we added to, if it isn't expanded already
         if( !item->mParent->mState.test( Item::Expanded ) )
            setItemExpanded( item->mParent->mId, true );

         //----------------------------------------------------------------
         // handle objects

         // Get our active SimObject if any
         SimObject *simObj = NULL;
         if(item->isInspectorData()) 
         {
            simObj = item->getObject();
         }

         // Remove from the old parentset
         if((simObj && parentSet)&&(oldParent != item->mParent))
         {
            if (mDebug) Con::printf("removing item from old parentset");
         
            // hack to get around the way removeObject takes the last item of the set
            // and moves it into the place of the object we removed
            if (parentSet->size()>0)
            {
               SimObject *lastObject = parentSet->last();
               parentSet->removeObject(simObj);
               parentSet->reOrder(lastObject);
            }
            else
            {
               parentSet->removeObject(simObj);
            }
         }

         // Snag the newparent set if any...
         SimSet *newParentSet = NULL;

         if(item->mParent->isInspectorData()) 
         {
            if (mDebug) Con::printf("getting a new parent set");

            SimObject * tmpObj = item->mParent->getObject();
            newParentSet = dynamic_cast<SimSet*>(tmpObj);
         }
         else
         {
            // parent is probably script data so we search up the tree for a
            // set to put our object in
            if (mDebug) Con::printf("oh nos my parent is script!");

            Item * temp = item->mParent;
            while (temp) 
            {
               if (temp->isInspectorData())
                  break;
               temp = temp->mParent;
            }
            
            // found a ancestor who is an inspectorData
            if (temp) 
            {
               if (temp->isInspectorData())
                  newParentSet = dynamic_cast<SimSet*>(temp->getObject());
            } 
            else 
            {
               newParentSet = NULL;
            }
         }

         if(simObj && newParentSet)
         {
            if (mDebug) Con::printf("simobj and new ParentSet");

            if (oldParent != item->mParent)
               newParentSet->addObject(simObj);

            //order the objects in the simset according to their
            //order in the tree view control
            if(!item->mNext)
            {
               if( item->mPrevious )
               {
                  //bring to the end of the set
                  SimObject *prevObject = item->mPrevious->getObject();
                  if (prevObject && item->getObject()) 
                  {
                     newParentSet->reOrder(item->getObject(), prevObject);
                  }
               }
            }
            else
            {
               //reorder within the set
               SimObject *nextObject = item->mNext->getObject();
               if(nextObject && item->getObject())
               {
                  newParentSet->reOrder(item->getObject(), nextObject);
               }
            }
         } 
         else if (!simObj&&newParentSet) 
         {
            // our current item is script data. but it may have children who
            // is inspector data who need an updated set
            if (mDebug) Con::printf("no simobj but new parentSet");
            if (item->mChild)
               inspectorSearch(item->mChild, item, parentSet, newParentSet);

         }
         else if (simObj&&!newParentSet) 
         {
            if (mDebug) Con::printf("simobject and no new parent set");
         }
         else
            if (mDebug) Con::printf("no simobject and no new parent set");

         // Notify script.

         if( item->isInspectorData() )
            onReparent_callback(
               item->getObject()->getId(),
               oldParent->getObject()->getId(),
               item->mParent->getObject()->getId()
            );
         else
            onReparent_callback(
               item->mId,
               oldParent->mId,
               item->mParent->mId
            );
      }
      
      onEndReparenting_callback();

      // And update everything.
      scrollVisible(newItem);

      onDragDropped_callback();

      buildVisibleTree(false);
   }

   mDragMidPoint = NomDragMidPoint;
}

//------------------------------------------------------------------------------

void GuiTreeViewCtrl::onMouseDragged(const GuiEvent &event)
{
   if( mDragStartInSelection )
      onMouseDragged_callback();
      
	if(!mSupportMouseDragging)
      return;
      
   if( !mActive || !mAwake || !mVisible )
      return;

   if (mSelectedItems.size() == 0)
      return;
      
   // Give us a little delta before we actually start a mouse drag so that
   // if the user moves the mouse a little while clicking, he/she does not
   // accidentally trigger a drag.
      
   if( mFabs( ( mMouseDownPoint - event.mousePoint ).len() ) <= 4.f )
      return;
      
   Point2I pt = globalToLocalCoord(event.mousePoint);
   Parent::onMouseMove(event);
   mouseLock();
   mMouseDragged = true;
   
   // If the drag is outside of our visible area,
   // start scrolling.
   
   GuiScrollCtrl* scrollCtrl = dynamic_cast< GuiScrollCtrl* >( getParent() );
   if( scrollCtrl && !scrollCtrl->isPointVisible( pt ) )
   {
      S32 widthDelta = 0;
      S32 heightDelta = 0;
      
      if( pt.x < scrollCtrl->getChildRelPos().x )
         widthDelta = pt.x - scrollCtrl->getChildRelPos().x;
      else if( pt.x > scrollCtrl->getChildRelPos().x + scrollCtrl->getContentExtent().x )
         widthDelta = pt.x - scrollCtrl->getChildRelPos().x - scrollCtrl->getContentExtent().x;

      if( pt.y < scrollCtrl->getChildRelPos().y )
         heightDelta = pt.y - scrollCtrl->getChildRelPos().y;
      else if( pt.y > scrollCtrl->getChildRelPos().y + scrollCtrl->getContentExtent().y )
         heightDelta = pt.y - scrollCtrl->getChildRelPos().y - scrollCtrl->getContentExtent().y;
         
      const F32 SCROLL_RATIO = 0.5f;
      scrollCtrl->scrollDelta( S32( F32( widthDelta ) * SCROLL_RATIO ), S32( F32( heightDelta ) * SCROLL_RATIO ) );
   }
   
   // whats our mDragMidPoint?
   mCurrentDragCell = mMouseOverCell.y;
   S32 midpCell = mCurrentDragCell * mItemHeight + (mItemHeight/2);
   S32 currentY = pt.y;
   S32 yDiff = currentY-midpCell;
   S32 variance = (mItemHeight/5);
   if( mPreviousDragCell >= 0 && mPreviousDragCell < mVisibleItems.size() )
      mVisibleItems[mPreviousDragCell]->mState.clear( Item::MouseOverBmp | Item::MouseOverText | Item::MouseOverIcon );

   bool hoverItem = false;

   if (mAbs(yDiff) <= variance)
   {
      mDragMidPoint = NomDragMidPoint;
      // highlight the current item
      // hittest to detect whether we are on an item
      // ganked from onMouseMouse

      // used for tracking what our last cell was so we can clear it.
      mPreviousDragCell = mCurrentDragCell;
      if (mCurrentDragCell >= 0)
      {

         Item* item = NULL;
         BitSet32 hitFlags = 0;
         if ( !_hitTest( event.mousePoint, item, hitFlags ) )
            return;
         
         // If the item is a valid drag target, activate the item
         // highlighting.

         if( isValidDragTarget( item ) )
         {
            hoverItem = true;

            if ( hitFlags.test( OnImage ) )
               item->mState.set( Item::MouseOverBmp );

            if ( hitFlags.test( OnText ) )
               item->mState.set( Item::MouseOverText );

            if ( hitFlags.test( OnIcon ) )
               item->mState.set( Item::MouseOverIcon );

            // Always redraw the entire mouse over item, since we are distinguishing
            // between the bitmap and the text:
            setUpdateRegion( Point2I( mMouseOverCell.x * mCellSize.x, mMouseOverCell.y * mCellSize.y ), mCellSize );
         }
      }
   }

   if ( !hoverItem )
   {
      //above or below an item?
      if (yDiff < 0)
         mDragMidPoint = AbovemDragMidPoint;
      else
         mDragMidPoint = BelowmDragMidPoint;
   }
}

//-----------------------------------------------------------------------------

void GuiTreeViewCtrl::onMiddleMouseDown(const GuiEvent & event)
{
   //for debugging items
   if (mDebug) {
      Item* item;
      BitSet32 hitFlags = 0;
      _hitTest( event.mousePoint, item, hitFlags );
      Con::printf("debugging %d", item->mId);
      Point2I pt = globalToLocalCoord(event.mousePoint);
      if (item->isInspectorData() && item->getObject()) {
         Con::printf("object data:");
         Con::printf("name:%s",item->getObject()->getName());
         Con::printf("className:%s",item->getObject()->getClassName());
      }
      Con::printf("contents of mSelectedItems:");
      for(S32 i = 0; i < mSelectedItems.size(); i++) {
         if (mSelectedItems[i]->isInspectorData()) {
            Con::printf("%d",mSelectedItems[i]->getObject()->getId());
         } else
            Con::printf("wtf %d", mSelectedItems[i]);
      }
      Con::printf("contents of mSelected");
      for (S32 j = 0; j < mSelected.size(); j++) {
         Con::printf("%d", mSelected[j]);
      }
      S32 mCurrentDragCell = mMouseOverCell.y;
      S32 midpCell = (mCurrentDragCell) * mItemHeight + (mItemHeight/2);
      S32 currentY = pt.y;
      S32 yDiff = currentY-midpCell;
      Con::printf("cell info: (%d,%d) mCurrentDragCell=%d est=(%d,%d,%d) ydiff=%d",pt.x,pt.y,mCurrentDragCell,mCurrentDragCell*mItemHeight, midpCell, (mCurrentDragCell+1)*mItemHeight,yDiff);
   }
}

//-----------------------------------------------------------------------------

void GuiTreeViewCtrl::onMouseDown(const GuiEvent & event)
{
   if( !mActive || !mAwake || !mVisible )
   {
      Parent::onMouseDown(event);
      return;
   }
   if ( mProfile->mCanKeyFocus )
      setFirstResponder();

   Item * item = 0;
   BitSet32 hitFlags;
   mDragMidPoint = NomDragMidPoint;

   mMouseDragged = false;
   mMouseDownPoint = event.mousePoint;

   //
   if(!_hitTest(event.mousePoint, item, hitFlags))
      return;

   mPossibleRenameItem = NULL;
   mRenamingItem = NULL;
	mTempItem = NULL;
      
   //
   if( event.modifier & SI_MULTISELECT )
   {
      bool selectFlag = item->mState.test(Item::Selected);
      if (selectFlag == true)
      {
         // already selected, so unselect it and remove it
         removeSelection(item->mId);
      }
      else
      {
         addSelection(item->mId);
      }
   }
   else if( event.modifier & SI_RANGESELECT && mMultipleSelections )
   {
      // is something already selected?
      S32 firstSelectedIndex = 0;
      Item * firstItem = NULL;
      if (!mSelectedItems.empty())
      {
         firstItem = mSelectedItems.front();
         for (S32 i = 0; i < mVisibleItems.size();i++)
         {
            if (mVisibleItems[i] == mSelectedItems.front())
            {
               firstSelectedIndex = i;
               break;
            }
         }
         S32 mCurrentDragCell = mMouseOverCell.y;
         if (mVisibleItems[firstSelectedIndex] != firstItem )
         {
            /*
            Con::printf("something isn't right...");
            if (mVisibleItems[firstSelectedIndex]->isInspectorData())
            Con::printf("visibleItem %s",mVisibleItems[firstSelectedIndex]->getObject()->getName());
            if (firstItem->isInspectorData())
            Con::printf("firstItem %s",firstItem->getObject()->getName());
            */
         }
         else
         {
            // select the cells
            onAddMultipleSelectionBegin_callback();
            if ((mCurrentDragCell) < firstSelectedIndex)
            {
               //select up
               for (S32 j = (mCurrentDragCell); j < firstSelectedIndex; j++)
               {
						if( j != (firstSelectedIndex - 1) )
							addSelection(mVisibleItems[j]->mId, false, false);
						else
							addSelection(mVisibleItems[j]->mId, false);
               }
            }
            else
            {
               // select down
               for (S32 j = firstSelectedIndex+1; j < (mCurrentDragCell+1); j++)
               {
						if( j != mCurrentDragCell )
							addSelection(mVisibleItems[j]->mId, false, false);
						else
							addSelection(mVisibleItems[j]->mId, false);
               }
            }

            // Scroll to view the last selected cell.
            scrollVisible( mVisibleItems[mCurrentDragCell] );

            onAddMultipleSelectionEnd_callback();
         }
      }
   }
   else if ( event.modifier & SI_PRIMARY_ALT )
   {
      if ( item->isInspectorData() && item->getObject() )
         setAddGroup(item->getObject());
   }
   else if ( !hitFlags.test(OnImage) )
   {      
		mTempItem = item;

      bool wasSelected = isSelected( item );
      bool multiSelect = getSelectedItemsCount() > 1;
		
		if( !wasSelected || !multiSelect )
		{
			if ( mClearAllOnSingleSelection )
				clearSelection();

			if ( !wasSelected || mClearAllOnSingleSelection )
				addSelection( item->mId );

			if ( wasSelected &&
				  !multiSelect &&
				  mCanRenameObjects &&
				  hitFlags.test(OnText) && 
				  mFlags.test(IsEditable) && 
				  item->isInspectorData() && 
				  item->getObject() &&
              item->getObject()->isNameChangeAllowed() &&
				  item != mRoot &&
				  event.mouseClickCount == 1 )
			{
				mPossibleRenameItem = item;

				if ( isMethod( "canRenameObject" ) )
				{
					if( canRenameObject_callback( item->getObject() ) )
						mPossibleRenameItem = NULL;
				}
			}
		}
      
   }

   if ( ( hitFlags.test( OnText ) || hitFlags.test( OnIcon ) ) && 
        event.mouseClickCount > 1 )
      execAltConsoleCallback();

   // For dragging, note if hit is in selection.
   
   mDragStartInSelection = isItemSelected( item->mId );

   if(!item->isParent())
      return;

   //
   if ( mFullRowSelect || hitFlags.test( OnImage ) )
   {
      item->setExpanded(!item->isExpanded());
      if( !item->isInspectorData() && item->mState.test(Item::VirtualParent) )
         onVirtualParentExpand(item);
      
      mFlags.set( RebuildVisible );
      scrollVisible(item);
   }
}

//------------------------------------------------------------------------------

void GuiTreeViewCtrl::onMouseMove( const GuiEvent &event )
{
   if ( mMouseOverCell.y >= 0 && mVisibleItems.size() > mMouseOverCell.y)
      mVisibleItems[mMouseOverCell.y]->mState.clear( Item::MouseOverBmp | Item::MouseOverText | Item::MouseOverIcon);

   Parent::onMouseMove( event );

   if ( mMouseOverCell.y >= 0 )
   {
      Item* item = NULL;
      BitSet32 hitFlags = 0;
      if ( !_hitTest( event.mousePoint, item, hitFlags ) )
         return;

      if ( hitFlags.test( OnImage ) )
         item->mState.set( Item::MouseOverBmp );

      if ( hitFlags.test( OnText ) )
         item->mState.set( Item::MouseOverText );

      if ( hitFlags.test( OnIcon ) )
         item->mState.set( Item::MouseOverIcon );

      // Always redraw the entire mouse over item, since we are distinguishing
      // between the bitmap and the text:
      setUpdateRegion( Point2I( mMouseOverCell.x * mCellSize.x, mMouseOverCell.y * mCellSize.y ), mCellSize );
   }
}

//------------------------------------------------------------------------------

void GuiTreeViewCtrl::onMouseEnter( const GuiEvent &event )
{
   Parent::onMouseEnter( event );
   onMouseMove( event );
}

//------------------------------------------------------------------------------

void GuiTreeViewCtrl::onMouseLeave( const GuiEvent &event )
{
   if ( mMouseOverCell.y >= 0 && mVisibleItems.size() > mMouseOverCell.y)
      mVisibleItems[mMouseOverCell.y]->mState.clear( Item::MouseOverBmp | Item::MouseOverText | Item::MouseOverIcon );

   Parent::onMouseLeave( event );
}

//------------------------------------------------------------------------------

void GuiTreeViewCtrl::onRightMouseDown(const GuiEvent & event)
{
   if(!mActive)
   {
      Parent::onRightMouseDown(event);
      return;
   }

   Item * item = NULL;
   BitSet32 hitFlags;

   //
   if(!_hitTest(event.mousePoint, item, hitFlags))
      return;

   //

   if (item->isInspectorData() && item->getObject())
      onRightMouseDown_callback( item->mId, event.mousePoint, item->getObject() );
   else
      onRightMouseDown_callback( item->mId, event.mousePoint );
}

//-----------------------------------------------------------------------------

void GuiTreeViewCtrl::onRightMouseUp(const GuiEvent & event)
{
   Item *item = NULL;
   BitSet32 hitFlags;

   if ( !_hitTest( event.mousePoint, item, hitFlags ) )
      return;

   if ( hitFlags.test( OnText ) || hitFlags.test( OnIcon ) )
   {
      if ( !isItemSelected( item->getID() ) )
      {
          clearSelection();
          addSelection( item->getID() );
      }

      if (item->isInspectorData() && item->getObject())
         onRightMouseUp_callback( item->mId, event.mousePoint, item->getObject() );
      else
         onRightMouseUp_callback( item->mId, event.mousePoint );
   }
   else
   {
      clearSelection();
   }

   Parent::onRightMouseUp(event);
}

//------------------------------------------------------------------------------

void GuiTreeViewCtrl::onRender(Point2I offset, const RectI &updateRect)
{
   if ( !mRenamingItem && mRenameCtrl )   
   {
      mRenameCtrl->deleteObject();
      mRenameCtrl = NULL;
   }
   
   // Get all our contents drawn!
   Parent::onRender(offset,updateRect);

   // Deal with drawing the drag & drop line, if any...
   GFX->setClipRect(updateRect);

   // only do it if we have a mDragMidPoint
   if (mDragMidPoint == NomDragMidPoint || !mSupportMouseDragging )
      return;

   ColorF greyLine(0.5,0.5,0.5,1);
   Point2F squarePt;

   // CodeReview: LineWidth is not supported in Direct3D. This is lame. [5/10/2007 Pat]
   // draw mDragMidPoint lines with a diamond
   if (mDragMidPoint == AbovemDragMidPoint)
   {
      S32 tempY = mItemHeight*mCurrentDragCell+offset.y ;
      squarePt.y = (F32)tempY;
      squarePt.x = 125.f+offset.x;
      GFX->getDrawUtil()->drawLine(0+offset.x, tempY, 250+offset.x, tempY,greyLine);
      GFX->getDrawUtil()->draw2DSquare(squarePt, 6, 90 );

   }
   if (mDragMidPoint == BelowmDragMidPoint)
   {
      S32 tempY2 = mItemHeight*(mCurrentDragCell+1) +offset.y;
      squarePt.y = (F32)tempY2;
      squarePt.x = 125.f+offset.x;
      GFX->getDrawUtil()->drawLine(0+offset.x, tempY2, 250+offset.x, tempY2,greyLine);
      GFX->getDrawUtil()->draw2DSquare(squarePt,6, 90 );
   }
}

//-----------------------------------------------------------------------------

void GuiTreeViewCtrl::onRenderCell(Point2I offset, Point2I cell, bool, bool )
{
   if( !mVisibleItems.size() )
      return;

   // Do some sanity checking and data retrieval.
   AssertFatal(cell.y < mVisibleItems.size(), "GuiTreeViewCtrl::onRenderCell: invalid cell");
   Item * item = mVisibleItems[cell.y];

   // If there's no object, deal with it.
   if(item->isInspectorData())
      if(!item->getObject())
         return;

   RectI drawRect( offset, mCellSize );
   GFXDrawUtil *drawer = GFX->getDrawUtil();
   drawer->clearBitmapModulation();

   FrameAllocatorMarker txtBuff;
   
   // Ok, we have the item. There are a few possibilities at this point:
   //    - We need to draw inheritance lines and a treeview-chosen icon
   //       OR
   //    - We have to draw an item-dependent icon
   //    - If we're mouseover, we have to highlight it.
   //
   //    - We have to draw the text for the item
   //       - Taking into account various mouseover states
   //       - Taking into account the value (set or not)
   //       - If it's an inspector data, we have to do some custom rendering
   //       - ADDED: If it is being renamed, we also have custom rendering.

   // Ok, first draw the tab and icon.

   // Do we draw the tree lines?
   if( mFlags.test(ShowTreeLines) )
   {
      drawRect.point.x += ( mTabSize * item->mTabLevel );
      Item* parent = item->mParent;
      for ( S32 i = item->mTabLevel; ( parent && i > 0 ); i-- )
      {
         drawRect.point.x -= mTabSize;
         if ( parent->mNext )
            drawer->drawBitmapSR( mProfile->mTextureObject, drawRect.point, mProfile->mBitmapArrayRects[BmpLine] );

         parent = parent->mParent;
      }
   }

   // Now, the icon...
   drawRect.point.x = offset.x + mTabSize * item->mTabLevel;

   // First, draw the rollover glow, if it's an inner node.
   if ( item->isParent() && item->mState.test( Item::MouseOverBmp ) )
      drawer->drawBitmapSR( mProfile->mTextureObject, drawRect.point, mProfile->mBitmapArrayRects[BmpGlow] );

   // Now, do we draw a treeview-selected item or an item dependent one?
   S32 newOffset = 0; // This is stored so we can render glow, then update render pos.

   S32 bitmap = 0;

   // Ok, draw the treeview lines as appropriate.
   
   bool drawBitmap = true;
   if ( !item->isParent() )
   {
      if( mFlags.test( ShowTreeLines ) )
      {
         if(    ( item->mNext && item->mPrevious )
             || ( item->mNext && item->mParent && ( !_isRootLevelItem( item ) || mShowRoot ) ) )
            bitmap = BmpChild;
         else if( item->mNext && ( !item->mParent || !mShowRoot ) )
            bitmap = BmpFirstChild;
         else if( item->mPrevious || ( item->mParent && !_isRootLevelItem( item ) ) )
            bitmap = BmpLastChild;
         else
            drawBitmap = false;
      }
      else
         drawBitmap = false;
   }
   else
   {
      bitmap = item->isExpanded() ? BmpExp : BmpCon;

      if( mFlags.test( ShowTreeLines ) )
      {
         // Shift indices to show versions with tree lines.
         
         if ( item->mParent || item->mPrevious )
            bitmap += ( item->mNext ? 3 : 2 );
         else
            bitmap += ( item->mNext ? 1 : 0 );
      }
   }

   if( ( bitmap >= 0 ) && ( bitmap < mProfile->mBitmapArrayRects.size() ) )
   {
      if( drawBitmap )
         drawer->drawBitmapSR( mProfile->mTextureObject, drawRect.point, mProfile->mBitmapArrayRects[bitmap] );
      newOffset = mProfile->mBitmapArrayRects[bitmap].extent.x;
   }

   if(item->isInspectorData())
   {
      // draw lock icon if need be
      S32 icon = Lock1;
      S32 icon2 = Hidden;

      if (item->getObject() && item->getObject()->isLocked())
      {
         if (mIconTable[icon])
         {
            //drawRect.point.x = offset.x + mTabSize * item->mTabLevel + mIconTable[icon].getWidth();
            drawRect.point.x += mIconTable[icon].getWidth();
            drawer->drawBitmap( mIconTable[icon], drawRect.point );
         }
      }

      if (item->getObject() && item->getObject()->isHidden())
      {
         if (mIconTable[icon2])
         {
            //drawRect.point.x = offset.x + mTabSize * item->mTabLevel + mIconTable[icon].getWidth();
            drawRect.point.x += mIconTable[icon2].getWidth();
            drawer->drawBitmap( mIconTable[icon2], drawRect.point );
         }
      }

      SimObject * pObject = item->getObject();
      SimGroup  * pGroup  = ( pObject == NULL ) ? NULL : dynamic_cast<SimGroup*>( pObject );

      // If this item is a VirtualParent we can use the generic SimGroup123 icons.
      // However if there is already an icon in the EditorIconRegistry for this
      // exact class (not counting parent class icons) we want to use that instead.
      bool hasClassIcon = false;
#ifdef TORQUE_TOOLS
      hasClassIcon = gEditorIcons.hasIconNoRecurse( pObject );
#endif

      // draw the icon associated with the item
      if ( !hasClassIcon && item->mState.test(Item::VirtualParent))
      {
         if ( pGroup != NULL)
         {
            if (item->isExpanded())
               item->mIcon = SimGroup1;
            else
               item->mIcon = SimGroup2;
         }
         else
            item->mIcon = SimGroup2;
      }
      
      if ( !hasClassIcon && item->mState.test(Item::Marked))
      {
         if (item->isInspectorData())
         {
            if ( pGroup != NULL )
            {
               if (item->isExpanded())
                  item->mIcon = SimGroup3;
               else
                  item->mIcon = SimGroup4;
            }
         }
      }

      GFXTexHandle iconHandle;

      if ( ( item->mIcon != -1 ) && mIconTable[item->mIcon] )
         iconHandle = mIconTable[item->mIcon];
#ifdef TORQUE_TOOLS
      else
         iconHandle = gEditorIcons.findIcon( item->getObject() );
#endif

      if ( iconHandle.isValid() )
      {
         S32 iconHeight = (mItemHeight - iconHandle.getHeight()) / 2;
         S32 oldHeight = drawRect.point.y;
         if(iconHeight > 0)
            drawRect.point.y += iconHeight;
         drawRect.point.x += iconHandle.getWidth();
         drawer->drawBitmap( iconHandle, drawRect.point );
         drawRect.point.y = oldHeight;
      }
   }
   else
   {
      S32 icon = item->isExpanded() ? item->mScriptInfo.mExpandedImage : item->mScriptInfo.mNormalImage;
      if ( icon )
      {
         if (mIconTable[icon])
         {
            S32 iconHeight = (mItemHeight - mIconTable[icon].getHeight()) / 2;
            S32 oldHeight = drawRect.point.y;
            if(iconHeight > 0)
               drawRect.point.y += iconHeight;
            drawRect.point.x += mIconTable[icon].getWidth();
            drawer->drawBitmap( mIconTable[icon], drawRect.point );
            drawRect.point.y = oldHeight;
         }
      }
   }

   // Ok, update offset so we can render some text!
   drawRect.point.x += newOffset;

   // Ok, now we're off to rendering the actual data for the treeview item.

   U32 bufLen = 1024; //item->mDataRenderWidth + 1;
   char *displayText = (char *)txtBuff.alloc(bufLen);
   displayText[bufLen-1] = 0;
   item->getDisplayText(bufLen, displayText);

   // Draw the rollover/selected bitmap, if one was specified.
   drawRect.extent.x = mProfile->mFont->getStrWidth( displayText ) + ( 2 * mTextOffset );
   if ( item->mState.test( Item::Selected ) && mTexSelected )
      drawer->drawBitmapStretch( mTexSelected, drawRect );
   else if ( item->mState.test( Item::MouseOverText ) && mTexRollover )
      drawer->drawBitmapStretch( mTexRollover, drawRect );

   // Offset a bit so as to space text properly.
   drawRect.point.x += mTextOffset;

   // Determine what color the font should be.
   ColorI fontColor;

   fontColor = item->mState.test( Item::Selected ) ? mProfile->mFontColorSEL :
             ( item->mState.test( Item::MouseOverText ) ? mProfile->mFontColorHL : mProfile->mFontColor );

   if (item->mState.test(Item::Selected))
   {
      drawer->drawRectFill(drawRect, mProfile->mFillColorSEL);
   }
   else if (item->mState.test(Item::MouseOverText))
   {
      drawer->drawRectFill(drawRect, mProfile->mFillColorHL);
   }

   if( item->mState.test(Item::MouseOverText) )
   {
		fontColor	=	mProfile->mFontColorHL;
   }

   drawer->setBitmapModulation( fontColor );

   // Center the text horizontally.
   S32 height = (mItemHeight - mProfile->mFont->getHeight()) / 2;

   if(height > 0)
      drawRect.point.y += height;

   // JDD - offset by two pixels or so to keep the text from rendering RIGHT ONTOP of the outline
   drawRect.point.x += 2;

   drawer->drawText( mProfile->mFont, drawRect.point, displayText, mProfile->mFontColors );

   if ( mRenamingItem == item && mRenameCtrl )
   {
      Point2I ctrPos = globalToLocalCoord( drawRect.point );
      ctrPos.y -= height;
      ctrPos.x -= 2;

      Point2I ctrExtent( getWidth() - ctrPos.x, drawRect.extent.y );

      mRenameCtrl->setPosition( ctrPos );
      mRenameCtrl->setExtent( ctrExtent );
      mRenameCtrl->setVisible( true );
   }
}

//------------------------------------------------------------------------------

bool GuiTreeViewCtrl::renderTooltip( const Point2I &hoverPos, const Point2I& cursorPos, const char* tipText )
{
   Item* item;
   BitSet32 flags = 0;
   char buf[ 2048 ];
   if( _hitTest( cursorPos, item, flags ) && (!item->mTooltip.isEmpty() || mUseInspectorTooltips) )
   {
      bool render = true;

      if( mTooltipOnWidthOnly && !item->hasObjectBasedTooltip() )
      {
         // Only render tooltip if the item's text is cut off with its
         // parent scroll control, unless there is custom object-based
         // tooltip information.
         GuiScrollCtrl *pScrollParent = dynamic_cast<GuiScrollCtrl*>( getParent() );
         if ( pScrollParent )
         {
            Point2I textStart;
            Point2I textExt;

            const Point2I pos = globalToLocalCoord(cursorPos);
            textStart.y = pos.y / mCellSize.y;
            textStart.y *= mCellSize.y;

            // The following is taken from _hitTest()
            textStart.x = mTabSize * item->mTabLevel;
            S32 image = BmpChild;
            if((image >= 0) && (image < mProfile->mBitmapArrayRects.size()))
               textStart.x += mProfile->mBitmapArrayRects[image].extent.x;
            textStart.x += mTextOffset;

            textStart.x += getInspectorItemIconsWidth( item );

            FrameAllocatorMarker txtAlloc;
            U32 bufLen = item->getDisplayTextLength() + 1;
            char *buf = (char*)txtAlloc.alloc(bufLen);
            item->getDisplayText(bufLen, buf);
            textExt.x = mProfile->mFont->getStrWidth(buf);
            textExt.y = mProfile->mFont->getHeight();

            if( pScrollParent->isRectCompletelyVisible(RectI(textStart, textExt)) )
               render = false;
         }
      }

      if( render )
      {
         if( mUseInspectorTooltips )
         {
            item->getTooltipText( sizeof( buf ), buf );
            tipText = buf;
         }
         else
         {
            tipText = item->mTooltip.c_str();
         }
      }
   }
      
   return defaultTooltipRender( cursorPos, cursorPos, tipText );
}

//------------------------------------------------------------------------------

void GuiTreeViewCtrl::clearSelection()
{
   if( mDebug ) Con::printf( "clearSelection called" );
   
   while ( !mSelectedItems.empty() )
   {
      removeSelection( mSelectedItems.last()->mId );
   }

   mSelectedItems.clear();
   mSelected.clear();

   onClearSelection();   
   onClearSelection_callback();
}

//-----------------------------------------------------------------------------

void GuiTreeViewCtrl::lockSelection(bool lock)
{
   for(U32 i = 0; i < mSelectedItems.size(); i++)
   {
      if(mSelectedItems[i]->isInspectorData())
         mSelectedItems[i]->getObject()->setLocked(lock);
   }
}

//-----------------------------------------------------------------------------

void GuiTreeViewCtrl::hideSelection(bool hide)
{
   for(U32 i = 0; i < mSelectedItems.size(); i++)
   {
      if(mSelectedItems[i]->isInspectorData())
         mSelectedItems[i]->getObject()->setHidden(hide);
   }
}

//-----------------------------------------------------------------------------

void GuiTreeViewCtrl::toggleLockSelection()
{
   for(U32 i = 0; i < mSelectedItems.size(); i++)
   {
      if( mSelectedItems[i]->isInspectorData() )
      {
         SimObject* object = mSelectedItems[ i ]->getObject();
         object->setLocked( !object->isLocked() );
      }
   }
}

//-----------------------------------------------------------------------------

void GuiTreeViewCtrl::toggleHideSelection()
{
   for(U32 i = 0; i < mSelectedItems.size(); i++)
   {
      if( mSelectedItems[i]->isInspectorData() )
      {
         SimObject* object = mSelectedItems[ i ]->getObject();
         object->setHidden( !object->isHidden() );
      }
   }
}

//------------------------------------------------------------------------------

// handles icon assignments
S32 GuiTreeViewCtrl::getIcon(const char * iconString)
{
   return -1;   
}

//-----------------------------------------------------------------------------

GuiTreeViewCtrl::Item* GuiTreeViewCtrl::addInspectorDataItem(Item *parent, SimObject *obj)
{
   S32 icon = getIcon(obj->getClassName());
   Item *item = createItem(icon);
   item->mState.set(Item::InspectorData);

   // Set the item text label flags.
   
   if( !mShowObjectIds )
      item->mState.clear( Item::ShowObjectId );
   else
      item->mState.set( Item::ShowObjectId );
   if( !mShowClassNames )
      item->mState.clear( Item::ShowClassName );
   else
      item->mState.set( Item::ShowClassName );
   if( !mShowObjectNames )
      item->mState.clear( Item::ShowObjectName );
   else
      item->mState.set( Item::ShowObjectName );
   if( !mShowInternalNames )
      item->mState.clear( Item::ShowInternalName );
   else
      item->mState.set( Item::ShowInternalName );
   if( mShowClassNameForUnnamedObjects )
      item->mState.set( Item::ShowClassNameForUnnamed );

   // Deal with child objects...
   if(dynamic_cast<SimSet*>(obj))
      item->mState.set(Item::VirtualParent);

   // Actually store the data!
   item->setObject(obj);

   // Now add us to the data structure...
   if(parent)
   {
      // Add as child of parent.
      if(parent->mChild)
      {
         Item * traverse = parent->mChild;
         while(traverse->mNext)
            traverse = traverse->mNext;

         traverse->mNext = item;
         item->mPrevious = traverse;
      }
      else
         parent->mChild = item;

      item->mParent = parent;
   }
   else
   {
      // If no parent, add to root.
      item->mNext = mRoot;
      mRoot = item;
      item->mParent = NULL;
   }

   mFlags.set(RebuildVisible);
   return item;
}

//-----------------------------------------------------------------------------

void GuiTreeViewCtrl::unlinkItem(Item * item)
{
   if (item->mPrevious)
      item->mPrevious->mNext = item->mNext;

   if (item->mNext)
      item->mNext->mPrevious = item->mPrevious;
}

//-----------------------------------------------------------------------------

bool GuiTreeViewCtrl::childSearch(Item * item, SimObject *obj, bool yourBaby)
{
   Item * temp = item->mChild;
   while (temp)
   {
      //do you have my baby?
      if (temp->isInspectorData())
      {
         if (temp->getObject() == obj)
            yourBaby = false; //probably a child of an inner script
      }
      yourBaby = childSearch(temp,obj, yourBaby);
      temp = temp->mNext;
   }
   return yourBaby;
}

//-----------------------------------------------------------------------------

void GuiTreeViewCtrl::inspectorSearch(Item * item, Item * parent, SimSet * parentSet, SimSet * newParentSet)
{
   if (!parentSet||!newParentSet)
      return;

   if (item == parent->mNext)
      return;

   if (item)
   {
      if (item->isInspectorData())
      {
         // remove the object from the parentSet and add it to the newParentSet
         SimObject* simObj = item->getObject();

         if (parentSet->size())
         {
            SimObject *lastObject = parentSet->last();
            parentSet->removeObject(simObj);
            parentSet->reOrder(lastObject);
         }
         else
            parentSet->removeObject(simObj);

         newParentSet->addObject(simObj);

         if (item->mNext)
         {
            inspectorSearch(item->mNext, parent, parentSet, newParentSet);
            return;
         }
         else
         {
            // end of children so backing up
            if (item->mParent == parent)
               return;
            else
            {
               inspectorSearch(item->mParent->mNext, parent, parentSet, newParentSet);
               return;
            }
         }
      }

      if (item->mChild)
      {
         inspectorSearch(item->mChild, parent, parentSet, newParentSet);
         return;
      }

      if (item->mNext)
      {
         inspectorSearch(item->mNext, parent, parentSet, newParentSet);
         return;
      }
   }
}

//-----------------------------------------------------------------------------

bool GuiTreeViewCtrl::objectSearch( const SimObject *object, Item **item )
{
	for ( U32 i = 0; i < mItems.size(); i++ )
	{
		Item *pItem = mItems[i];

      if ( !pItem )
         continue;

		SimObject *pObj = pItem->getObject();

		if ( pObj && pObj == object )
		{
			*item = pItem;
			return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------

bool GuiTreeViewCtrl::onVirtualParentBuild(Item *item, bool bForceFullUpdate)
{
   if(!item->mState.test(Item::InspectorData))
      return true;

   // Blast an item if it doesn't have a corresponding SimObject...
   if(item->mInspectorInfo.mObject == NULL)
   {
      removeItem(item->mId);
      return false;
   }

   // Skip the next stuff unless we're expanded...
	 if(!item->isExpanded() && !bForceFullUpdate && !( item == mRoot && !mShowRoot ) )
      return true;

   // Verify that we have all the kids we should in here...
   SimSet *srcObj = dynamic_cast<SimSet*>(&(*item->mInspectorInfo.mObject));

   // If it's not a SimSet... WTF are we doing here?
   if(!srcObj)
      return true;

   // This is slow but probably ok.
   for( SimSet::iterator i = srcObj->begin(); i != srcObj->end(); ++ i )
   {
      SimObject *obj = *i;

      // If we can't find it, add it.
      // unless it has a parent that is a child that is a script
      Item *res = item->findChildByValue(obj);

      bool foundChild = true;

      // search the children. if any of them are the parent of the object then don't add it.
      foundChild = childSearch(item,obj,foundChild);

      if(!res && foundChild)
      {
         if (mDebug) Con::printf( "adding object %i to item %i", obj->getId(), item->mId );
         res = addInspectorDataItem(item, obj);
      }
      
      if( res )
         res->mState.set( Item::RebuildVisited );
   }
   
   // Go through our items and purge those that have disappeared from
   // the set.
   
   for( Item* ptr = item->mChild; ptr != NULL; )
   {
      Item* next = ptr->mNext;
      if( !ptr->mState.test( Item::RebuildVisited ) )
      {
         if( mDebug ) Con::printf( "removing item %i for object %i that is no longer in the set",
            ptr->mId, ptr->getObject()->getId() );
            
         removeItem( ptr->mId, false );
      }
      else
         ptr->mState.clear( Item::RebuildVisited );
         
      ptr = next;
   }

   return true;
}

//-----------------------------------------------------------------------------

bool GuiTreeViewCtrl::onVirtualParentExpand(Item *item)
{
   // Do nothing...
   return true;
}

//-----------------------------------------------------------------------------

bool GuiTreeViewCtrl::onVirtualParentCollapse(Item *item)
{
   // Do nothing...
   return true;
}

//-----------------------------------------------------------------------------

void GuiTreeViewCtrl::inspectObject( SimObject* obj, bool okToEdit )
{
   _destroyTree();

   mFlags.set( IsEditable, okToEdit );
   mFlags.set( IsInspector );

   onDefineIcons_callback();

   addInspectorDataItem( NULL, obj );
}

//-----------------------------------------------------------------------------

S32 GuiTreeViewCtrl::findItemByName(const char *name)
{
   for (S32 i = 0; i < mItems.size(); i++) 
      if (mItems[i] && dStrcmp(mItems[i]->getText(),name) == 0) 
         return mItems[i]->mId;

   return 0;
}

//-----------------------------------------------------------------------------

S32 GuiTreeViewCtrl::findItemByValue(const char *name)
{
   for (S32 i = 0; i < mItems.size(); i++) 
      if (mItems[i] && dStrcmp(mItems[i]->getValue(),name) == 0) 
         return mItems[i]->mId;

   return 0;
}

//-----------------------------------------------------------------------------

void GuiTreeViewCtrl::sortTree( bool caseSensitive, bool traverseHierarchy, bool parentsFirst )
{
   itemSortList( mRoot, caseSensitive, traverseHierarchy, parentsFirst );
}

//-----------------------------------------------------------------------------

StringTableEntry GuiTreeViewCtrl::getTextToRoot( S32 itemId, const char * delimiter )
{
   Item * item = getItem(itemId);

   if(!item)
   {
      Con::errorf(ConsoleLogEntry::General, "GuiTreeViewCtrl::getTextToRoot: invalid start item id!");
      return StringTable->insert("");
   }

   if(item->isInspectorData())
   {
      Con::errorf(ConsoleLogEntry::General, "GuiTreeViewCtrl::getTextToRoot: cannot get text to root of inspector data items");
      return StringTable->insert("");
   }

   char bufferOne[1024];
   char bufferTwo[1024];
   char bufferNodeText[128];

   dMemset( bufferOne, 0, sizeof(bufferOne) );
   dMemset( bufferTwo, 0, sizeof(bufferTwo) );

   dStrcpy( bufferOne, item->getText() );

   Item *prevNode = item->mParent;
   while ( prevNode )
   {
      dMemset( bufferNodeText, 0, sizeof(bufferNodeText) );
      dStrcpy( bufferNodeText, prevNode->getText() );
      dSprintf( bufferTwo, 1024, "%s%s%s",bufferNodeText, delimiter, bufferOne );
      dStrcpy( bufferOne, bufferTwo );
      dMemset( bufferTwo, 0, sizeof(bufferTwo) );
      prevNode = prevNode->mParent;
   }

   // Return the result, StringTable-ized.
   return StringTable->insert( bufferOne, true );
}

//-----------------------------------------------------------------------------

void GuiTreeViewCtrl::setFilterText( const String& text )
{
   mFilterText = text;

   // Trigger rebuild.
   mFlags.set( RebuildVisible );
}

//=============================================================================
//    Console Methods.
//=============================================================================
// MARK: ---- Console Methods ----

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiTreeViewCtrl, findItemByName, S32, ( const char* text ),,
   "Get the ID of the item whose text matches the given @a text.\n\n"
   "@param text Item text to match.\n"
   "@return ID of the item or -1 if no item matches the given text." )
{
   return object->findItemByName( text );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiTreeViewCtrl, findItemByValue, S32, ( const char* value ),,
   "Get the ID of the item whose value matches @a value.\n\n"
   "@param value Value text to match.\n"
   "@return ID of the item or -1 if no item has the given value." )
{
   return object->findItemByValue( value );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiTreeViewCtrl, findChildItemByName, S32, ( S32 parentId, const char* childName ),,
   "Get the child item of the given parent item whose text matches @a childName.\n\n"
   "@param parentId Item ID of the parent in which to look for the child.\n"
   "@param childName Text of the child item to find.\n"
   "@return ID of the child item or -1 if no child in @a parentId has the given text @a childName.\n\n"
   "@note This method does not recurse, i.e. it only looks for direct children." )
{
   if( parentId == 0 )
   {
      if( !object->getRootItem() )
         return 0;
         
      GuiTreeViewCtrl::Item* root = object->getRootItem();
      while( root )
      {
         if( dStricmp( root->getText(), childName ) == 0 )
            return root->getID();
            
         root = root->mNext;
      }
      
      return 0;
   }
   else
   {
      GuiTreeViewCtrl::Item* item = object->getItem( parentId );
      
      if( !item )
      {
         Con::errorf( "GuiTreeViewCtrl.findChildItemByName - invalid parent ID '%i'", parentId );
         return 0;
      }
      
      GuiTreeViewCtrl::Item* child = item->findChildByName( childName );
      if( !child )
         return 0;
      
      return child->mId;
   }
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiTreeViewCtrl, insertItem, S32, ( S32 parentId, const char* text, const char* value, const char* icon, S32 normalImage, S32 expandedImage ), ( "", "", 0, 0 ),
   "Add a new item to the tree.\n\n"
   "@param parentId Item ID of parent to which to add the item as a child.  0 is root item.\n"
   "@param text Text to display on the item in the tree.\n"
   "@param value Behind-the-scenes value of the item.\n"
   "@param icon\n"
   "@param normalImage\n"
   "@param expandedImage\n"
   "@return The ID of the newly added item." )
{
   return object->insertItem( parentId, text, value, icon, normalImage, expandedImage );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiTreeViewCtrl, lockSelection, void, ( bool lock ), ( true ),
   "Set whether the current selection can be changed by the user or not.\n\n"
   "@param lock If true, the current selection is frozen and cannot be changed.  If false, "
      "the selection may be modified." )
{
   object->lockSelection( lock );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiTreeViewCtrl, hideSelection, void, ( bool state ), ( true ),
   "Call SimObject::setHidden( @a state ) on all objects in the current selection.\n\n"
   "@param state Visibility state to set objects in selection to." )
{
   object->hideSelection( state );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiTreeViewCtrl, toggleLockSelection, void, (),,
   "Toggle the locked state of all objects in the current selection." )
{
   object->toggleLockSelection();

}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiTreeViewCtrl, toggleHideSelection, void, (),,
   "Toggle the hidden state of all objects in the current selection." )
{
   object->toggleHideSelection();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiTreeViewCtrl, clearSelection, void, (),,
   "Unselect all currently selected items." )
{
   object->clearSelection();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiTreeViewCtrl, deleteSelection, void, (),,
   "Delete all items/objects in the current selection." )
{
   object->deleteSelection();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiTreeViewCtrl, addSelection, void, ( S32 id, bool isLastSelection ), ( true ),
   "Add an item/object to the current selection.\n\n"
   "@param id ID of item/object to add to the selection.\n"
   "@param isLastSelection Whether there are more pending items/objects to be added to the selection.  If false, "
      "the control will defer refreshing the tree and wait until addSelection() is called with this parameter set "
      "to true." )
{
   object->addSelection( id, isLastSelection, isLastSelection );
}

ConsoleMethod(GuiTreeViewCtrl, addChildSelectionByValue, void, 4, 4, "addChildSelectionByValue(TreeItemId parent, value)")
{
   S32 id = dAtoi(argv[2]);
   GuiTreeViewCtrl::Item* parentItem = object->getItem(id);
   GuiTreeViewCtrl::Item* child = parentItem->findChildByValue(argv[3]);
   object->addSelection(child->getID());
}

ConsoleMethod(GuiTreeViewCtrl, removeSelection, void, 3, 3, "(deselects an item)")
{
   S32 id = dAtoi(argv[2]);
   object->removeSelection(id);
}

ConsoleMethod(GuiTreeViewCtrl, removeChildSelectionByValue, void, 4, 4, "removeChildSelectionByValue(TreeItemId parent, value)")
{
   S32 id = dAtoi(argv[2]);
   GuiTreeViewCtrl::Item* parentItem = object->getItem(id);
   if(parentItem)
   {
      GuiTreeViewCtrl::Item* child = parentItem->findChildByValue(argv[3]);
	  if(child)
	  {
         object->removeSelection(child->getID());
	  }
   }
}

ConsoleMethod(GuiTreeViewCtrl, selectItem, bool, 3, 4, "(TreeItemId item, bool select=true)")
{
   S32 id = dAtoi(argv[2]);
   bool select = true;
   if(argc == 4)
      select = dAtob(argv[3]);

   return(object->setItemSelected(id, select));
}

ConsoleMethod(GuiTreeViewCtrl, expandItem, bool, 3, 4, "(TreeItemId item, bool expand=true)")
{
   S32 id = dAtoi(argv[2]);
   bool expand = true;
   if(argc == 4)
      expand = dAtob(argv[3]);
   return(object->setItemExpanded(id, expand));
}

ConsoleMethod(GuiTreeViewCtrl, markItem, bool, 3, 4, "(TreeItemId item, bool mark=true)")
{
   S32 id = dAtoi(argv[2]);
   bool mark = true;
   if(argc == 4)
      mark = dAtob(argv[3]);
   return object->markItem(id, mark);
}

// Make the given item visible.
ConsoleMethod(GuiTreeViewCtrl, scrollVisible, void, 3, 3, "(TreeItemId item)")
{
   object->scrollVisible(dAtoi(argv[2]));
}

ConsoleMethod(GuiTreeViewCtrl, buildIconTable, bool, 3,3, "(builds an icon table)")
{   
   const char * icons = argv[2];
   return object->buildIconTable(icons);
}

ConsoleMethod( GuiTreeViewCtrl, open, void, 3, 4, "(SimSet obj, bool okToEdit=true) Set the root of the tree view to the specified object, or to the root set.")
{
   SimSet *treeRoot = NULL;
   SimObject* target = Sim::findObject(argv[2]);

   bool okToEdit = true;

   if (argc == 4)
      okToEdit = dAtob(argv[3]);

   if (target)
      treeRoot = dynamic_cast<SimSet*>(target);

   if (! treeRoot)
      Sim::findObject(RootGroupId, treeRoot);

   object->inspectObject(treeRoot,okToEdit);
}

ConsoleMethod( GuiTreeViewCtrl, setItemTooltip, void, 4, 4, "( int id, string text ) - Set the tooltip to show for the given item." )
{
   int id = dAtoi( argv[ 2 ] );
   
   GuiTreeViewCtrl::Item* item = object->getItem( id );
   if( !item )
   {
      Con::errorf( "GuiTreeViewCtrl::setTooltip() - invalid item id '%i'", id );
      return;
   }
   
   item->mTooltip = argv[ 3 ];
}

ConsoleMethod( GuiTreeViewCtrl, setItemImages, void, 5, 5, "( int id, int normalImage, int expandedImage ) - Sets the normal and expanded images to show for the given item." )
{
   int id = dAtoi( argv[ 2 ] );

   GuiTreeViewCtrl::Item* item = object->getItem( id );
   if( !item )
   {
      Con::errorf( "GuiTreeViewCtrl::setItemImages() - invalid item id '%i'", id );
      return;
   }

   item->setNormalImage((S8)dAtoi(argv[3]));
   item->setExpandedImage((S8)dAtoi(argv[4]));
}

ConsoleMethod( GuiTreeViewCtrl, isParentItem, bool, 3, 3, "( int id ) - Returns true if the given item contains child items." )
{
   int id = dAtoi( argv[ 2 ] );
   if( !id && object->getItemCount() )
      return true;
   
   GuiTreeViewCtrl::Item* item = object->getItem( id );
   if( !item )
   {
      Con::errorf( "GuiTreeViewCtrl::isParentItem - invalid item id '%i'", id );
      return false;
   }
   
   return item->isParent();
}

ConsoleMethod(GuiTreeViewCtrl, getItemText, const char *, 3, 3, "(TreeItemId item)")
{
   return(object->getItemText(dAtoi(argv[2])));
}

ConsoleMethod(GuiTreeViewCtrl, getItemValue, const char *, 3, 3, "(TreeItemId item)")
{
   return(object->getItemValue(dAtoi(argv[2])));
}

ConsoleMethod(GuiTreeViewCtrl, editItem, bool, 5, 5, "(TreeItemId item, string newText, string newValue)")
{
   return(object->editItem(dAtoi(argv[2]), argv[3], argv[4]));
}

ConsoleMethod(GuiTreeViewCtrl, removeItem, bool, 3, 3, "(TreeItemId item)")
{
   return(object->removeItem(dAtoi(argv[2])));
}

ConsoleMethod(GuiTreeViewCtrl, removeAllChildren, void, 3, 3, "removeAllChildren(TreeItemId parent)")
{
   object->removeAllChildren(dAtoi(argv[2]));
}
ConsoleMethod(GuiTreeViewCtrl, clear, void, 2, 2, "() - empty tree")
{
   object->removeItem(0);
}

ConsoleMethod(GuiTreeViewCtrl, getFirstRootItem, S32, 2, 2, "Get id for root item.")
{
   return(object->getFirstRootItem());
}

ConsoleMethod(GuiTreeViewCtrl, getChild, S32, 3, 3, "(TreeItemId item)")
{
   return(object->getChildItem(dAtoi(argv[2])));
}

ConsoleMethod(GuiTreeViewCtrl, buildVisibleTree, void, 2, 3, "Build the visible tree")
{
   bool forceFullUpdate = false;
   if( argc > 2 )
      forceFullUpdate = dAtob( argv[ 2 ] );
      
   object->buildVisibleTree( forceFullUpdate );
}

//FIXME: [rene 11/09/09 - This clashes with GuiControl.getParent(); bad thing; should be getParentItem]
ConsoleMethod(GuiTreeViewCtrl, getParent, S32, 3, 3, "(TreeItemId item)")
{
   return(object->getParentItem(dAtoi(argv[2])));
}

ConsoleMethod(GuiTreeViewCtrl, getNextSibling, S32, 3, 3, "(TreeItemId item)")
{
   return(object->getNextSiblingItem(dAtoi(argv[2])));
}

ConsoleMethod(GuiTreeViewCtrl, getPrevSibling, S32, 3, 3, "(TreeItemId item)")
{
   return(object->getPrevSiblingItem(dAtoi(argv[2])));
}

ConsoleMethod(GuiTreeViewCtrl, getItemCount, S32, 2, 2, "")
{
   return(object->getItemCount());
}

ConsoleMethod(GuiTreeViewCtrl, getSelectedItem, S32, 2, 3, "( int index=0 ) - Return the selected item at the given index.")
{
   S32 index = 0;
   if( argc > 2 )
      index = dAtoi( argv[ 2 ] );
      
   return ( object->getSelectedItem( index ) );
}

ConsoleMethod(GuiTreeViewCtrl, getSelectedObject, S32, 2, 3, "( int index=0 ) - Return the currently selected SimObject at the given index in inspector mode or -1")
{
   S32 index = 0;
   if( argc > 2 )
      index = dAtoi( argv[ 2 ] );

   GuiTreeViewCtrl::Item *item = object->getItem( object->getSelectedItem( index ) );
   if( item != NULL && item->isInspectorData() )
   {
      SimObject *obj = item->getObject();
      if( obj != NULL )
         return obj->getId();
   }

   return -1;
}

ConsoleMethod(GuiTreeViewCtrl, getSelectedObjectList, const char*, 2, 2, 
              "Returns a space sperated list of all selected object ids.")
{
   char* buff = Con::getReturnBuffer(1024);
   dSprintf(buff,1024,"");

   const Vector< GuiTreeViewCtrl::Item* > selectedItems = object->getSelectedItems();
   for(int i = 0; i < selectedItems.size(); i++)
   {
      GuiTreeViewCtrl::Item *item = selectedItems[i];

      if ( item->isInspectorData() && item->getObject() )
      {      
         S32 id  = item->getObject()->getId();
         //get the current length of the buffer
         U32	len = dStrlen(buff);
         //the start of the buffer where we want to write
         char* buffPart = buff+len;
         //the size of the remaining buffer (-1 cause dStrlen doesn't count the \0)
         S32 size	=	1024-len-1;
         //write it:
         if(size < 1)
         {
            Con::errorf("GuiTreeViewCtrl::getSelectedItemList - Not enough room to return our object list");
            return buff;
         }

         dSprintf(buffPart,size,"%d ", id);
      }
   }

   return buff;
}

ConsoleMethod(GuiTreeViewCtrl, moveItemUp, void, 3, 3, "(TreeItemId item)")
{
   object->moveItemUp( dAtoi( argv[2] ) );
}

ConsoleMethod(GuiTreeViewCtrl, getSelectedItemsCount, S32, 2, 2, "")
{
   return ( object->getSelectedItemsCount() );
}



ConsoleMethod(GuiTreeViewCtrl, moveItemDown, void, 3, 3, "(TreeItemId item)")
{
   object->moveItemDown( dAtoi( argv[2] ) );
}

//-----------------------------------------------------------------------------

ConsoleMethod(GuiTreeViewCtrl, getTextToRoot, const char*,4,4,"(TreeItemId item,Delimiter=none) gets the text from the current node to the root, concatenating at each branch upward, with a specified delimiter optionally")
{
   if ( argc < 4 )
   {
      Con::warnf("GuiTreeViewCtrl::getTextToRoot - Invalid number of arguments!");
      return ("");
   }
   S32 itemId = dAtoi( argv[2] );
   StringTableEntry delimiter = argv[3];

   return object->getTextToRoot( itemId, delimiter );
}

ConsoleMethod(GuiTreeViewCtrl, getSelectedItemList,const char*, 2,2,"returns a space seperated list of mulitple item ids")
{
	char* buff = Con::getReturnBuffer(1024);
	dSprintf(buff,1024,"");

   const Vector< S32 >& selected = object->getSelected();
	for(int i = 0; i < selected.size(); i++)
	{
		S32 id  = selected[i];
		//get the current length of the buffer
		U32	len = dStrlen(buff);
		//the start of the buffer where we want to write
		char* buffPart = buff+len;
		//the size of the remaining buffer (-1 cause dStrlen doesn't count the \0)
		S32 size	=	1024-len-1;
		//write it:
		if(size < 1)
		{
			Con::errorf("GuiTreeViewCtrl::getSelectedItemList - Not enough room to return our object list");
			return buff;
		}

		dSprintf(buffPart,size,"%d ", id);
	}
//mSelected

	return buff;
}

S32 GuiTreeViewCtrl::findItemByObjectId(S32 iObjId)
{  
   for (S32 i = 0; i < mItems.size(); i++)
   {    
      if ( !mItems[i] )
         continue;

      SimObject* pObj = mItems[i]->getObject();
	   if( pObj && pObj->getId() == iObjId )
		   return mItems[i]->mId;
   }

   return -1;
}

//------------------------------------------------------------------------------
ConsoleMethod(GuiTreeViewCtrl, findItemByObjectId, S32, 3, 3, "(find item by object id and returns the mId)")
{
   return(object->findItemByObjectId(dAtoi(argv[2])));
}

//------------------------------------------------------------------------------
bool GuiTreeViewCtrl::scrollVisibleByObjectId(S32 objID)
{
   S32 itemID = findItemByObjectId(objID);

   if(itemID == -1)
   {
      // we did not find the item in our current items
      // we should try to find and show the parent of the item.
      SimObject *obj = Sim::findObject(objID);
      if(!obj || !obj->getGroup())
         return false;
    
      // if we can't show the parent, we fail.
      if(! scrollVisibleByObjectId(obj->getGroup()->getId()) )
         return false;
      
      // get the parent. expand the parent. rebuild the tree. this ensures that
      // we'll be able to find the child item we're targeting.
      S32 parentID = findItemByObjectId(obj->getGroup()->getId());
      AssertFatal(parentID != -1, "We were able to show the parent, but could not then find the parent. This should not happen.");
      Item *parentItem = getItem(parentID);
      parentItem->setExpanded(true);
      buildVisibleTree();
      
      // NOW we should be able to find the object. if not... something's wrong.
      itemID = findItemByObjectId(objID);
      AssertWarn(itemID != -1,"GuiTreeViewCtrl::scrollVisibleByObjectId() found the parent, but can't find it's immediate child. This should not happen.");
      if(itemID == -1)
         return false;
   }
   
   // ok, item found. scroll to it.
   mFlags.set( RebuildVisible );
   scrollVisible(itemID);
   
   return true;
}

//------------------------------------------------------------------------------
ConsoleMethod(GuiTreeViewCtrl, scrollVisibleByObjectId, S32, 3, 3, "(show item by object id. returns true if sucessful.)")
{
   return(object->scrollVisibleByObjectId(dAtoi(argv[2])));
}

//------------------------------------------------------------------------------

//FIXME: this clashes with SimSet.sort()
ConsoleMethod( GuiTreeViewCtrl, sort, void, 2, 6, "( int parent, bool traverseHierarchy=false, bool parentsFirst=false, bool caseSensitive=true ) - Sorts all items of the given parent (or root).  With 'hierarchy', traverses hierarchy." )
{
   S32 parent = 0;
   if( argc >= 3 )
      parent = dAtoi( argv[ 2 ] );

   bool traverseHierarchy = false;
   bool parentsFirst = false;
   bool caseSensitive = true;
   
   if( argc >= 4 )
      traverseHierarchy = dAtob( argv[ 3 ] );
   if( argc >= 5 )
      parentsFirst = dAtob( argv[ 4 ] );
   if( argc >= 6 )
      caseSensitive = dAtob( argv[ 5 ] );
      
   if( !parent )
      object->sortTree( caseSensitive, traverseHierarchy, parentsFirst );
   else
   {
      GuiTreeViewCtrl::Item* item = object->getItem( parent );
      if( !item )
      {
         Con::errorf( "GuiTreeViewCtrl::sort - no item '%i' in tree", parent );
         return;
      }
      
      item->sort( caseSensitive, traverseHierarchy, parentsFirst );
   }
}

void GuiTreeViewCtrl::cancelRename()
{
   if ( !mRenamingItem || !mRenameCtrl )
      return;

   mRenamingItem = NULL;  

   if ( mRenameCtrl )
      mRenameCtrl->clearFirstResponder();
}

void GuiTreeViewCtrl::onRenameValidate()
{
   if ( !mRenamingItem || !mRenameCtrl )
      return;

   char data[ GuiTextCtrl::MAX_STRING_LENGTH+1 ];
   mRenameCtrl->getText( data );

   SimObject *obj = mRenamingItem->getObject();

   mRenamingItem = NULL;  

   // Object could have been deleted in the interum.
   if ( !obj )
      return;
   
   if( isMethod( "handleRenameObject" ) && handleRenameObject_callback( data, obj ) )
      return;

   if ( mRenameInternal )
      obj->setInternalName( data );   
   else
#ifdef TORQUE_TOOLS
   if ( validateObjectName( data, obj ) )
#endif
      obj->assignName( data ); 
}

void GuiTreeViewCtrl::showItemRenameCtrl( Item* item )
{
   SimObject *renameObj = item->getObject();
   
   mRenamingItem = item;
   
   if ( !mRenameCtrl )
   {
      mRenameCtrl = new GuiTextEditCtrl;
      mRenameCtrl->registerObject();
      addObject( mRenameCtrl );        
      
      if ( mRenameInternal )
         mRenameCtrl->setText( renameObj->getInternalName() );
      else
         mRenameCtrl->setText( renameObj->getName() );

      mRenameCtrl->setFirstResponder();
      mRenameCtrl->setSinkAllKeys(true);
      mRenameCtrl->selectAllText();
      mRenameCtrl->setCursorPos(0);
      
      char cmd[256];
      dSprintf( cmd, 256, "%i.onRenameValidate();", getId() );
      mRenameCtrl->setField( "validate", cmd );

      dSprintf( cmd, 256, "%i.cancelRename();", getId() );
      mRenameCtrl->setField( "escapeCommand", cmd );
   }
}

ConsoleMethod( GuiTreeViewCtrl, cancelRename, void, 2, 2, "For internal use." )
{
   object->cancelRename();
}

ConsoleMethod( GuiTreeViewCtrl, onRenameValidate, void, 2, 2, "For internal use." )
{
   object->onRenameValidate();
}

ConsoleMethod( GuiTreeViewCtrl, showItemRenameCtrl, void, 3, 3, "( TreeItemId id ) - Show the rename text field for the given item (only one at a time)." )
{
   S32 id = dAtoi( argv[ 2 ] );
   GuiTreeViewCtrl::Item* item = object->getItem( id );
   if( !item )
   {
      Con::errorf( "GuiTreeViewCtrl::showItemRenameCtrl - invalid item id '%i'", id );
      return;
   }
   
   object->showItemRenameCtrl( item );
}

ConsoleMethod( GuiTreeViewCtrl, setDebug, void, 2, 3, "( bool value=true ) - Enable/disable debug output." )
{
   bool value = true;
   if( argc > 2 )
      value = dAtob( argv[ 2 ] );
      
   object->setDebug( value );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiTreeViewCtrl, isItemSelected, bool, ( S32 id ),,
   "Check whether the given item is currently selected in the tree.\n\n"
   "@param id Item/object ID.\n"
   "@return True if the given item/object is currently selected in the tree." )
{
   const Vector< GuiTreeViewCtrl::Item* >& selectedItems = object->getSelectedItems();
	for( S32 i = 0; i < selectedItems.size(); ++ i )
      if( selectedItems[ i ]->mId == id )
         return true;
         
   return false;
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiTreeViewCtrl, getFilterText, const char*, (),,
   "Get the current filter expression.  Only tree items whose text matches this expression "
   "are displayed.  By default, the expression is empty and all items are shown.\n\n"
   "@return The current filter pattern or an empty string if no filter pattern is currently active.\n\n"
   "@see setFilterText\n"
   "@see clearFilterText" )
{
   return object->getFilterText();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiTreeViewCtrl, setFilterText, void, ( const char* pattern ),,
   "Set the pattern by which to filter items in the tree.  Only items in the tree whose text "
   "matches this pattern are displayed.\n\n"
   "@param pattern New pattern based on which visible items in the tree should be filtered.  If empty, all items become visible.\n\n"
   "@see getFilterText\n"
   "@see clearFilterText" )
{
   object->setFilterText( pattern );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiTreeViewCtrl, clearFilterText, void, (),,
   "Clear the current item filtering pattern.\n\n"
   "@see setFilterText\n"
   "@see getFilterText" )
{
   object->clearFilterText();
}
