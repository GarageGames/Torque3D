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

#include "console/engineAPI.h"
#include "gui/editor/guiInspector.h"
#include "gui/editor/inspector/field.h"
#include "gui/editor/inspector/group.h"
#include "gui/buttons/guiIconButtonCtrl.h"
#include "gui/editor/inspector/dynamicGroup.h"
#include "gui/containers/guiScrollCtrl.h"
#include "gui/editor/inspector/customField.h"

#ifdef TORQUE_EXPERIMENTAL_EC
#include "gui/editor/inspector/entityGroup.h"
#include "gui/editor/inspector/mountingGroup.h"
#include "gui/editor/inspector/componentGroup.h"
#endif

IMPLEMENT_CONOBJECT(GuiInspector);

ConsoleDocClass( GuiInspector,
   "@brief A control that allows to edit the properties of one or more SimObjects.\n\n"
   "Editor use only.\n\n"
   "@internal"
);


//#define DEBUG_SPEW


//-----------------------------------------------------------------------------

GuiInspector::GuiInspector()
 : mDividerPos( 0.35f ),
   mDividerMargin( 5 ),
   mOverDivider( false ),
   mMovingDivider( false ),
   mHLField( NULL ),
   mShowCustomFields( true )
{
   mPadding = 1;
}

//-----------------------------------------------------------------------------

GuiInspector::~GuiInspector()
{
   clearGroups();
}

//-----------------------------------------------------------------------------

void GuiInspector::initPersistFields()
{
   addGroup( "Inspector" );
   
      addField( "dividerMargin", TypeS32, Offset( mDividerMargin, GuiInspector ) );

      addField( "groupFilters", TypeRealString, Offset( mGroupFilters, GuiInspector ), 
         "Specify groups that should be shown or not. Specifying 'shown' implicitly does 'not show' all other groups. Example string: +name -otherName" );

      addField( "showCustomFields", TypeBool, Offset( mShowCustomFields, GuiInspector ),
         "If false the custom fields Name, Id, and Source Class will not be shown." );
         
   endGroup( "Inspector" );

   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

void GuiInspector::onRemove()
{
   clearGroups();
   Parent::onRemove();
}

//-----------------------------------------------------------------------------

void GuiInspector::onDeleteNotify( SimObject *object )
{
   Parent::onDeleteNotify( object );
   
   if( isInspectingObject( object ) )
      removeInspectObject( object );
}

//-----------------------------------------------------------------------------

void GuiInspector::parentResized(const RectI &oldParentRect, const RectI &newParentRect)
{
   GuiControl *parent = getParent();
   if ( parent && dynamic_cast<GuiScrollCtrl*>(parent) != NULL )
   {
      GuiScrollCtrl *scroll = dynamic_cast<GuiScrollCtrl*>(parent);
      setWidth( ( newParentRect.extent.x - ( scroll->scrollBarThickness() + 4  ) ) );
   }
   else
      Parent::parentResized(oldParentRect,newParentRect);
}

//-----------------------------------------------------------------------------

bool GuiInspector::resize( const Point2I &newPosition, const Point2I &newExtent )
{
   //F32 dividerPerc = (F32)getWidth() / (F32)mDividerPos;

   bool result = Parent::resize( newPosition, newExtent );

   //mDividerPos = (F32)getWidth() * dividerPerc;

   updateDivider();

   return result;
}

//-----------------------------------------------------------------------------

GuiControl* GuiInspector::findHitControl( const Point2I &pt, S32 initialLayer )
{
   if ( mOverDivider || mMovingDivider )
      return this;

   return Parent::findHitControl( pt, initialLayer );
}

//-----------------------------------------------------------------------------

void GuiInspector::getCursor( GuiCursor *&cursor, bool &showCursor, const GuiEvent &lastGuiEvent )
{
   GuiCanvas *pRoot = getRoot();
   if( !pRoot )
      return;

   S32 desiredCursor = mOverDivider ? PlatformCursorController::curResizeVert : PlatformCursorController::curArrow;

   // Bail if we're already at the desired cursor
   if ( pRoot->mCursorChanged == desiredCursor )
      return;

   PlatformWindow *pWindow = static_cast<GuiCanvas*>(getRoot())->getPlatformWindow();
   AssertFatal(pWindow != NULL,"GuiControl without owning platform window!  This should not be possible.");
   PlatformCursorController *pController = pWindow->getCursorController();
   AssertFatal(pController != NULL,"PlatformWindow without an owned CursorController!");

   // Now change the cursor shape
   pController->popCursor();
   pController->pushCursor(desiredCursor);
   pRoot->mCursorChanged = desiredCursor;
}

//-----------------------------------------------------------------------------

void GuiInspector::onMouseMove(const GuiEvent &event)
{
   if ( collideDivider( globalToLocalCoord( event.mousePoint ) ) )
      mOverDivider = true;
   else
      mOverDivider = false;
}

//-----------------------------------------------------------------------------

void GuiInspector::onMouseDown(const GuiEvent &event)
{
   if ( mOverDivider )
   {
      mMovingDivider = true;
   }
}

//-----------------------------------------------------------------------------

void GuiInspector::onMouseUp(const GuiEvent &event)
{
   mMovingDivider = false;   
}

//-----------------------------------------------------------------------------

void GuiInspector::onMouseDragged(const GuiEvent &event)
{
   if ( !mMovingDivider )   
      return;

   Point2I localPnt = globalToLocalCoord( event.mousePoint );

   S32 inspectorWidth = getWidth();

   // Distance from mouse/divider position in local space
   // to the right edge of the inspector
   mDividerPos = inspectorWidth - localPnt.x;
   mDividerPos = mClamp( mDividerPos, 0, inspectorWidth );

   // Divide that by the inspectorWidth to get a percentage
   mDividerPos /= inspectorWidth;

   updateDivider();
}

//-----------------------------------------------------------------------------

GuiInspectorGroup* GuiInspector::findExistentGroup( StringTableEntry groupName )
{
   // If we have no groups, it couldn't possibly exist
   if( mGroups.empty() )
      return NULL;

   // Attempt to find it in the group list
   Vector<GuiInspectorGroup*>::iterator i = mGroups.begin();

   for( ; i != mGroups.end(); i++ )
   {
      if( dStricmp( (*i)->getGroupName(), groupName ) == 0 )
         return *i;
   }

   return NULL;
}

//-----------------------------------------------------------------------------

void GuiInspector::updateFieldValue( StringTableEntry fieldName, StringTableEntry arrayIdx )
{
   // We don't know which group contains the field of this name,
   // so ask each group in turn, and break when a group returns true
   // signifying it contained and updated that field.

   Vector<GuiInspectorGroup*>::iterator groupIter = mGroups.begin();

   for( ; groupIter != mGroups.end(); groupIter++ )
   {   
      if ( (*groupIter)->updateFieldValue( fieldName, arrayIdx ) )
         break;
   }
}

//-----------------------------------------------------------------------------

void GuiInspector::clearGroups()
{
   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[GuiInspector] Clearing %i (%s)", getId(), getName() );
   #endif
   
   // If we have no groups, there's nothing to clear!
   if( mGroups.empty() )
      return;

   mHLField = NULL;
   
   if( isMethod( "onClear" ) )
      Con::executef( this, "onClear" );

   Vector<GuiInspectorGroup*>::iterator i = mGroups.begin();

   freeze(true);

   // Delete Groups
   for( ; i != mGroups.end(); i++ )
   {
      if((*i) && (*i)->isProperlyAdded())
         (*i)->deleteObject();
   }   

   mGroups.clear();

   freeze(false);
   updatePanes();
}

//-----------------------------------------------------------------------------

bool GuiInspector::isInspectingObject( SimObject* object )
{
   const U32 numTargets = mTargets.size();
   for( U32 i = 0; i < numTargets; ++ i )
      if( mTargets[ i ] == object )
         return true;
         
   return false;
}

//-----------------------------------------------------------------------------

void GuiInspector::inspectObject( SimObject *object )
{  
   if( mTargets.size() > 1 || !isInspectingObject( object ) )
      clearInspectObjects();
         
   addInspectObject( object );
}

//-----------------------------------------------------------------------------

void GuiInspector::clearInspectObjects()
{
   const U32 numTargets = mTargets.size();
   for( U32 i = 0; i < numTargets; ++ i )
      clearNotify( mTargets[ i ] );
      
   clearGroups();
   mTargets.clear();
}

//-----------------------------------------------------------------------------

void GuiInspector::addInspectObject( SimObject* object, bool autoSync )
{   
   // If we are already inspecting the object, just update the groups.
   
   if( isInspectingObject( object ) )
   {
      #ifdef DEBUG_SPEW
      Platform::outputDebugString( "[GuiInspector] Refreshing view of %i:%s (%s)",
         object->getId(), object->getClassName(), object->getName() );
      #endif

      Vector<GuiInspectorGroup*>::iterator i = mGroups.begin();
      for ( ; i != mGroups.end(); i++ )
         (*i)->updateAllFields();

      return;
   }

   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[GuiInspector] Adding %i:%s (%s) to inspect set",
      object->getId(), object->getClassName(), object->getName() );
   #endif

   // Give users a chance to customize fields on this object
   if( object->isMethod("onDefineFieldTypes") )
      Con::executef( object, "onDefineFieldTypes" );

   // Set Target
   mTargets.push_back( object );
   deleteNotify( object );
   
	if( autoSync )
		refresh();
}

//-----------------------------------------------------------------------------

void GuiInspector::removeInspectObject( SimObject* object )
{
   const U32 numTargets = mTargets.size();
   for( U32 i = 0; i < numTargets; ++ i )
      if( mTargets[ i ] == object )
      {      
         // Delete all inspector data *before* removing the target so that apply calls
         // triggered by edit controls losing focus will not find the inspect object
         // gone.

         clearGroups();
         
         #ifdef DEBUG_SPEW
         Platform::outputDebugString( "[GuiInspector] Removing %i:%s (%s) from inspect set",
            object->getId(), object->getClassName(), object->getName() );
         #endif

         mTargets.erase( i );
         clearNotify( object );
         
         // Refresh the inspector except if the system is going down.

         if( !Sim::isShuttingDown() )
            refresh();
         
         return;
      }   
}

//-----------------------------------------------------------------------------

void GuiInspector::setName( StringTableEntry newName )
{
   if( mTargets.size() != 1 )
      return;

   StringTableEntry name = StringTable->insert(newName);

   // Only assign a new name if we provide one
   mTargets[ 0 ]->assignName(name);
}

//-----------------------------------------------------------------------------

bool GuiInspector::collideDivider( const Point2I &localPnt )
{   
   RectI divisorRect( getWidth() - getWidth() * mDividerPos - mDividerMargin, 0, mDividerMargin * 2, getHeight() ); 

   if ( divisorRect.pointInRect( localPnt ) )
      return true;

   return false;
}

//-----------------------------------------------------------------------------

void GuiInspector::updateDivider()
{
   for ( U32 i = 0; i < mGroups.size(); i++ )
      for ( U32 j = 0; j < mGroups[i]->mChildren.size(); j++ )
         mGroups[i]->mChildren[j]->updateRects(); 

   //setUpdate();
}

//-----------------------------------------------------------------------------

void GuiInspector::getDivider( S32 &pos, S32 &margin )
{   
   pos = (F32)getWidth() * mDividerPos;
   margin = mDividerMargin;   
}

//-----------------------------------------------------------------------------

void GuiInspector::setHighlightField( GuiInspectorField *field )
{
   if ( mHLField == field )
      return;

   if ( mHLField.isValid() )
      mHLField->setHLEnabled( false );
   mHLField = field;

   // We could have been passed a null field, meaning, set no field highlighted.
   if ( mHLField.isNull() )
      return;

   mHLField->setHLEnabled( true );
}

//-----------------------------------------------------------------------------

bool GuiInspector::isGroupFiltered( const char *groupName ) const
{
   // Internal and Ungrouped always filtered, we never show them.   
   if ( dStricmp( groupName, "Internal" ) == 0 ||
        dStricmp( groupName, "Ungrouped" ) == 0 ||
		  dStricmp( groupName, "AdvCoordManipulation" ) == 0)
      return true;

   // Normal case, determine if filtered by looking at the mGroupFilters string.
   String searchStr;

   // Is this group explicitly show? Does it immediately follow a + char.
   searchStr = String::ToString( "+%s", groupName );
   if ( mGroupFilters.find( searchStr ) != String::NPos )
      return false;   

   // Were there any other + characters, if so, we are implicitly hidden.   
   if ( mGroupFilters.find( "+" ) != String::NPos )
      return true;

   // Is this group explicitly hidden? Does it immediately follow a - char.
   searchStr = String::ToString( "-%s", groupName );
   if ( mGroupFilters.find( searchStr ) != String::NPos )
      return true;   

   return false;
}

//-----------------------------------------------------------------------------

bool GuiInspector::isGroupExplicitlyFiltered( const char *groupName ) const
{  
   String searchStr;

   searchStr = String::ToString( "-%s", groupName );

   if ( mGroupFilters.find( searchStr ) != String::NPos )
      return true;   

   return false;
}

//-----------------------------------------------------------------------------

void GuiInspector::setObjectField( const char *fieldName, const char *data )
{
   GuiInspectorField *field;

   for ( S32 i = 0; i < mGroups.size(); i++ )
   {
      field = mGroups[i]->findField( fieldName );

      if( field )
      {
         field->setData( data );
         return;
      }
   }
}

//-----------------------------------------------------------------------------

static SimObject *sgKeyObj = NULL;

bool findInspectors( GuiInspector *obj )
{   
   if ( obj->isAwake() && obj->isInspectingObject( sgKeyObj ) )
      return true;

   return false;
}

//-----------------------------------------------------------------------------

GuiInspector* GuiInspector::findByObject( SimObject *obj )
{
   sgKeyObj = obj;

   Vector< GuiInspector* > found;
   Sim::getGuiGroup()->findObjectByCallback( findInspectors, found );

   if ( found.empty() )
      return NULL;

   return found.first();
}

//-----------------------------------------------------------------------------

void GuiInspector::refresh()
{
   clearGroups();
   
   // Remove any inspect object that happened to have
   // already been killed.
   
   for( U32 i = 0; i < mTargets.size(); ++ i )
      if( !mTargets[ i ] )
      {
         mTargets.erase( i );
         -- i;
      }
      
   if( !mTargets.size() )
      return;
   
   // Special group for fields which should appear at the top of the
   // list outside of a rollout control.
   
   GuiInspectorGroup *ungroup = NULL;
   if( mTargets.size() == 1 )
   {
      ungroup = new GuiInspectorGroup( "Ungrouped", this );
      ungroup->setHeaderHidden( true );
      ungroup->setCanCollapse( false );

      ungroup->registerObject();
      mGroups.push_back( ungroup );
      addObject( ungroup );
   }

   // Put the 'transform' group first
   GuiInspectorGroup *transform = new GuiInspectorGroup( "Transform", this );

   transform->registerObject();
   mGroups.push_back(transform);
   addObject(transform);

   // Always create the 'general' group (for fields without a group)      
   GuiInspectorGroup *general = new GuiInspectorGroup( "General", this );

   general->registerObject();
   mGroups.push_back(general);
   addObject(general);

#ifdef TORQUE_EXPERIMENTAL_EC
   //Entity inspector group
   if (mTargets.first()->getClassRep()->isSubclassOf("Entity"))
   {
      GuiInspectorEntityGroup *components = new GuiInspectorEntityGroup("Components", this);
      if (components != NULL)
      {
         components->registerObject();
         mGroups.push_back(components);
         addObject(components);
      }

      //Mounting group override
      GuiInspectorGroup *mounting = new GuiInspectorMountingGroup("Mounting", this);
      if (mounting != NULL)
      {
         mounting->registerObject();
         mGroups.push_back(mounting);
         addObject(mounting);
      }
   }

   if (mTargets.first()->getClassRep()->isSubclassOf("Component"))
   {
      //Build the component field groups as the component describes it
      Component* comp = dynamic_cast<Component*>(mTargets.first().getPointer());

      if (comp->getComponentFieldCount() > 0)
      {
         GuiInspectorComponentGroup *compGroup = new GuiInspectorComponentGroup("Component Fields", this);
         compGroup->registerObject();
         mGroups.push_back(compGroup);
         addObject(compGroup);
      }
   }
#endif

   // Create the inspector groups for static fields.

   for( TargetVector::iterator iter = mTargets.begin(); iter != mTargets.end(); ++ iter )
   {
      AbstractClassRep::FieldList &fieldList = ( *iter )->getModifiableFieldList();

      // Iterate through, identifying the groups and create necessary GuiInspectorGroups
      for( AbstractClassRep::FieldList::iterator itr = fieldList.begin(); itr != fieldList.end(); itr++ )
      {      
         if ( itr->type == AbstractClassRep::StartGroupFieldType )
         {
            GuiInspectorGroup* group = findExistentGroup( itr->pGroupname );
            
            if( !group && !isGroupFiltered( itr->pGroupname ) )
            {
               GuiInspectorGroup *group = new GuiInspectorGroup( itr->pGroupname, this );

               group->registerObject();
               if( !group->getNumFields() )
               {
                  #ifdef DEBUG_SPEW
                  Platform::outputDebugString( "[GuiInspector] Removing empty group '%s'",
                     group->getCaption().c_str() );
                  #endif
                     
                  // The group ended up having no fields.  Remove it.
                  group->deleteObject();
               }
               else
               {
                  mGroups.push_back( group );
                  addObject( group );
               }
            }
         }
      }
   }

   // Deal with dynamic fields
   if ( !isGroupFiltered( "Dynamic Fields" ) )
   {
      GuiInspectorGroup *dynGroup = new GuiInspectorDynamicGroup( "Dynamic Fields", this);

      dynGroup->registerObject();
      mGroups.push_back( dynGroup );
      addObject( dynGroup );
   }

   if( mShowCustomFields && mTargets.size() == 1 )
   {
      SimObject* object = mTargets.first();
      
      // Add the SimObjectID field to the ungrouped group.
      
      GuiInspectorCustomField* field = new GuiInspectorCustomField();
      field->init( this, ungroup );

      if( field->registerObject() )
      {
         ungroup->mChildren.push_back( field );
         ungroup->mStack->addObject( field );
         
         static StringTableEntry sId = StringTable->insert( "id" );
         
         field->setCaption( sId );
         field->setData( object->getIdString() );
         field->setDoc( "SimObjectId of this object. [Read Only]" );
      }
      else
         delete field;

      // Add the Source Class field to the ungrouped group.
      
      field = new GuiInspectorCustomField();
      field->init( this, ungroup );

      if( field->registerObject() )
      {
         ungroup->mChildren.push_back( field );
         ungroup->mStack->addObject( field );
         
         StringTableEntry sSourceClass = StringTable->insert( "Source Class", true );
         field->setCaption( sSourceClass );
         field->setData( object->getClassName() );

         Namespace* ns = object->getClassRep()->getNameSpace();
         field->setToolTip( Con::getNamespaceList( ns ) );

         field->setDoc( "Native class of this object. [Read Only]" );
      }
      else
         delete field;
   }


   // If the general group is still empty at this point ( or filtered ), kill it.
   if ( isGroupFiltered( "General" ) || general->mStack->size() == 0 )
   {
      for(S32 i=0; i<mGroups.size(); i++)
      {
         if ( mGroups[i] == general )
         {
            mGroups.erase(i);
            general->deleteObject();
            updatePanes();

            break;
         }
      }
   }

   // If transform turns out to be empty or filtered, remove it
   if( isGroupFiltered( "Transform" ) || transform->mStack->size() == 0 )
   {
      for(S32 i=0; i<mGroups.size(); i++)
      {
         if ( mGroups[i] == transform )
         {
            mGroups.erase(i);
            transform->deleteObject();
            updatePanes();

            break;
         }
      }
   }

   // If ungrouped is empty or explicitly filtered, remove it.   
   if( ungroup && ( isGroupExplicitlyFiltered( "Ungrouped" ) || ungroup->getNumFields() == 0 ) )
   {
      for(S32 i=0; i<mGroups.size(); i++)
      {
         if ( mGroups[i] == ungroup )
         {
            mGroups.erase(i);
            ungroup->deleteObject();
            updatePanes();

            break;
         }
      }
   }
   
   // If the object cannot be renamed, deactivate the name field if we have it.
   
   if( ungroup && getNumInspectObjects() == 1 && !getInspectObject()->isNameChangeAllowed() )
   {
      GuiInspectorField* nameField = ungroup->findField( "name" );
      if( nameField )
         nameField->setActive( false );
   }
}

//-----------------------------------------------------------------------------

void GuiInspector::sendInspectPreApply()
{
   const U32 numObjects = getNumInspectObjects();
   for( U32 i = 0; i < numObjects; ++ i )
      getInspectObject( i )->inspectPreApply();
}

//-----------------------------------------------------------------------------

void GuiInspector::sendInspectPostApply()
{
   const U32 numObjects = getNumInspectObjects();
   for( U32 i = 0; i < numObjects; ++ i )
      getInspectObject( i )->inspectPostApply();
}

//=============================================================================
//    Console Methods.
//=============================================================================
// MARK: ---- Console Methods ----

//-----------------------------------------------------------------------------
DefineEngineMethod( GuiInspector, inspect, void, (const char* simObject), (""),
   "Inspect the given object.\n"
   "@param simObject Object to inspect.")
{
   SimObject * target = Sim::findObject(simObject);
   if(!target)
   {
      if(dAtoi(simObject) > 0)
         Con::warnf("%s::inspect(): invalid object: %s", object->getClassName(), simObject);

      object->clearInspectObjects();
      return;
   }

   object->inspectObject(target);
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiInspector, addInspect, void, (const char* simObject, bool autoSync), (true),
   "Add the object to the list of objects being inspected.\n"
   "@param simObject Object to add to the inspection."
   "@param autoSync Auto sync the values when they change.")
{
   SimObject* obj;
   if( !Sim::findObject( simObject, obj ) )
   {
      Con::errorf( "%s::addInspect(): invalid object: %s", object->getClassName(), simObject );
      return;
   }

	object->addInspectObject( obj, autoSync );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiInspector, removeInspect, void, (const char* simObject), ,
   "Remove the object from the list of objects being inspected.\n"
   "@param simObject Object to remove from the inspection.")
{
   SimObject* obj;
   if( !Sim::findObject( simObject, obj ) )
   {
      Con::errorf( "%s::removeInspect(): invalid object: %s", object->getClassName(), simObject );
      return;
   }

   object->removeInspectObject( obj );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiInspector, refresh, void, (), ,
   "Re-inspect the currently selected object.\n")
{
   if ( object->getNumInspectObjects() == 0 )
      return;

   SimObject *target = object->getInspectObject();
   if ( target )
      object->inspectObject( target );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiInspector, getInspectObject, const char*, (S32 index), (0),
   "Returns currently inspected object.\n"
   "@param index Index of object in inspection list you want to get."
   "@return object being inspected.")
{
      
   if( index < 0 || index >= object->getNumInspectObjects() )
   {
      Con::errorf( "GuiInspector::getInspectObject() - index out of range: %i", index );
      return "";
   }
   
   return object->getInspectObject( index )->getIdString();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiInspector, getNumInspectObjects, S32, (), ,
   "Return the number of objects currently being inspected.\n"
   "@return number of objects currently being inspected.")
{
   return object->getNumInspectObjects();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiInspector, setName, void, (const char* newObjectName), ,
	"Rename the object being inspected (first object in inspect list).\n"
	"@param newObjectName new name for object being inspected.")
{
   object->setName(newObjectName);
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiInspector, apply, void, (), ,
	"Force application of inspected object's attributes.\n")
{
   object->sendInspectPostApply();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiInspector, setObjectField, void, (const char* fieldname, const char* data ), ,
	"Set a named fields value on the inspected object if it exists. This triggers all the usual callbacks that would occur if the field had been changed through the gui..\n"
	"@param fieldname Field name on object we are inspecting we want to change."
	"@param data New Value for the given field.")
{
   object->setObjectField( fieldname, data );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiInspector, findByObject, S32, (SimObject* obj), ,
	"Returns the id of an awake inspector that is inspecting the passed object if one exists\n"
	"@param object Object to find away inspector for."
	"@return id of an awake inspector that is inspecting the passed object if one exists, else NULL or 0.")
{
   if ( !obj)
      return NULL;

   SimObject *inspector = GuiInspector::findByObject(obj);

   if ( !inspector )
      return NULL;

   return inspector->getId();
}
