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

#include "gui/buttons/guiIconButtonCtrl.h"
#include "gui/editor/guiInspector.h"
#include "gui/editor/inspector/mountingGroup.h"
#include "core/strings/stringUnit.h"
#include "T3D/entity.h"
#include "T3D/components/component.h"

//Need this to get node lists
#include "T3D/components/render/renderComponentInterface.h"

IMPLEMENT_CONOBJECT(GuiInspectorMountingGroup);

ConsoleDocClass( GuiInspectorMountingGroup,
   "@brief Used to inspect an object's FieldDictionary (dynamic fields) instead "
   "of regular persistent fields.\n\n"
   "Editor use only.\n\n"
   "@internal"
);

//-----------------------------------------------------------------------------
// GuiInspectorMountingGroup - add custom controls
//-----------------------------------------------------------------------------
GuiInspectorMountingGroup::GuiInspectorMountingGroup( StringTableEntry groupName, SimObjectPtr<GuiInspector> parent )
      : GuiInspectorGroup( groupName, parent) 
{ 
	mParentInspector = parent;

	targetMountCtrl = NULL;
	mountCtrl = NULL;
};

bool GuiInspectorMountingGroup::createContent()
{
   if(!Parent::createContent())
      return false;

   //give the necessary padding for the nested controls so it looks nice.
   setMargin(RectI(4,0,4,4));

   return true;
}

GuiControl* GuiInspectorMountingGroup::buildMenuCtrl()
{
	GuiControl* retCtrl = new GuiPopUpMenuCtrl();

   // If we couldn't construct the control, bail!
   if( retCtrl == NULL )
      return retCtrl;

   GuiPopUpMenuCtrl *menu = dynamic_cast<GuiPopUpMenuCtrl*>(retCtrl);

   // Let's make it look pretty.
   retCtrl->setDataField( StringTable->insert("profile"), NULL, "GuiPopUpMenuProfile" );
   //GuiInspectorTypeMenuBase::_registerEditControl( retCtrl );

	char szName[512];
   dSprintf( szName, 512, "IE_%s_%d_%s_Field", retCtrl->getClassName(), mParentInspector->getInspectObject()->getId(), mCaption.c_str());

   // Register the object
   retCtrl->registerObject( szName );

   // Configure it to update our value when the popup is closed
   char szBuffer[512];
   dSprintf( szBuffer, 512, "%d.apply( %d.getText() );", getId(), menu->getId() );
   menu->setField("Command", szBuffer );

	return menu;
}

bool GuiInspectorMountingGroup::buildList(Entity* ent, GuiPopUpMenuCtrl* menu)
{
   RenderComponentInterface* renderInterface = ent->getComponent<RenderComponentInterface>();

   if (renderInterface)
	{
      TSShape* shape = renderInterface->getShape();
      S32 nodeCount = shape ? shape->nodes.size() : 0;

		for(U32 i=0; i < nodeCount; i++)
		{
         menu->addEntry(shape->names[i], i);
		}

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// GuiInspectorMountingGroup - inspectGroup override
//-----------------------------------------------------------------------------
bool GuiInspectorMountingGroup::inspectGroup()
{
   // We can't inspect a group without a target!
   if( !mParent->getNumInspectObjects() )
      return false;

   // to prevent crazy resizing, we'll just freeze our stack for a sec..
   mStack->freeze(true);

   bool bNoGroup = false;

   // Un-grouped fields are all sorted into the 'general' group
   if ( dStricmp( mCaption, "General" ) == 0 )
      bNoGroup = true;
      
   // Just delete all fields and recreate them (like the dynamicGroup)
   // because that makes creating controls for array fields a lot easier
   clearFields();
   
   bool bNewItems = false;
   bool bMakingArray = false;
   GuiStackControl *pArrayStack = NULL;
   GuiRolloutCtrl *pArrayRollout = NULL;
   bool bGrabItems = false;

   AbstractClassRep* commonAncestorClass = findCommonAncestorClass();
   AbstractClassRep::FieldList& fieldList = commonAncestorClass->mFieldList;
   for( AbstractClassRep::FieldList::iterator itr = fieldList.begin();
        itr != fieldList.end(); ++ itr )
   {
      AbstractClassRep::Field* field = &( *itr );
      if( field->type == AbstractClassRep::StartGroupFieldType )
      {
         // If we're dealing with general fields, always set grabItems to true (to skip them)
         if( bNoGroup == true )
            bGrabItems = true;
         else if( dStricmp( field->pGroupname, mCaption ) == 0 )
            bGrabItems = true;
         continue;
      }
      else if ( field->type == AbstractClassRep::EndGroupFieldType )
      {
         // If we're dealing with general fields, always set grabItems to false (to grab them)
         if( bNoGroup == true )
            bGrabItems = false;
         else if( dStricmp( field->pGroupname, mCaption ) == 0 )
            bGrabItems = false;
         continue;
      }
      
      // Skip field if it has the HideInInspectors flag set.
      
      if( field->flag.test( AbstractClassRep::FIELD_HideInInspectors ) )
         continue;

      if( ( bGrabItems == true || ( bNoGroup == true && bGrabItems == false ) ) && itr->type != AbstractClassRep::DeprecatedFieldType )
      {
         if( bNoGroup == true && bGrabItems == true )
            continue;

         // If the field already exists, just update it
			GuiInspectorField *fieldGui = findField( field->pFieldname );
			if ( fieldGui != NULL )
			{
				fieldGui->updateValue();
				continue;
			}
            
			bNewItems = true;
            
			if(field->pFieldname == StringTable->insert("mountNode"))
			{
				fieldGui = new GuiInspectorNodeListField();

				Entity* e = dynamic_cast<Entity*>(mParent->getInspectObject(0));
				if(e)
					(dynamic_cast<GuiInspectorNodeListField*>(fieldGui))->setTargetEntity(e);
			}
			else
			{
				fieldGui = constructField( field->type );
				if ( fieldGui == NULL )
					fieldGui = new GuiInspectorField();
			}

			fieldGui->init( mParent, this );            
			fieldGui->setInspectorField( field );
                     
			if( fieldGui->registerObject() )
			{
				#ifdef DEBUG_SPEW
				Platform::outputDebugString( "[GuiInspectorGroup] Adding field '%s'",
					field->pFieldname );
				#endif

				mChildren.push_back( fieldGui );
				mStack->addObject( fieldGui );
			}
			else
			{
				SAFE_DELETE( fieldGui );
			}
      }
   }
   mStack->freeze(false);
   mStack->updatePanes();

   // If we've no new items, there's no need to resize anything!
   if( bNewItems == false && !mChildren.empty() )
      return true;

   sizeToContents();

   setUpdate();

   return true;
}

void GuiInspectorMountingGroup::updateAllFields()
{
   // We overload this to just reinspect the group.
   inspectGroup();
}

void GuiInspectorMountingGroup::onMouseMove(const GuiEvent &event)
{
	//mParent->mOverDivider = false;
	bool test = false;
}
ConsoleMethod(GuiInspectorMountingGroup, inspectGroup, bool, 2, 2, "Refreshes the dynamic fields in the inspector.")
{
   return object->inspectGroup();
}

void GuiInspectorMountingGroup::clearFields()
{
}

bool GuiInspectorMountingGroup::resize( const Point2I &newPosition, const Point2I &newExtent )
{
   if ( !Parent::resize( newPosition, newExtent ) )
      return false;

	//check if we're set up yet
	if(!targetMountCtrl || !mountCtrl)
		//no? bail
		return false;

	targetMountCtrl->setExtent(newExtent.x, 18);
	mountCtrl->setExtent(newExtent.x, 18);

	S32 dividerPos, dividerMargin;
   mParentInspector->getDivider( dividerPos, dividerMargin );   

   Point2I fieldExtent = Point2I(newExtent.x, 18);
   Point2I fieldPos = Point2I(newExtent.x, 18);

   S32 editWidth = dividerPos - dividerMargin;

	targetMountText->setPosition(0,0);
	targetMountText->setExtent(fieldExtent.x - dividerPos - dividerMargin, fieldExtent.y);

	targetMountNode->setPosition(fieldExtent.x - dividerPos + dividerMargin, 1);
	targetMountNode->setExtent(editWidth, fieldExtent.y - 1);

	mountText->setPosition(0,0);
	mountText->setExtent(fieldExtent.x - dividerPos - dividerMargin, fieldExtent.y);

	mountNode->setPosition(fieldExtent.x - dividerPos + dividerMargin, 1);
	mountNode->setExtent(editWidth, fieldExtent.y - 1);

   return true;
}

SimFieldDictionary::Entry* GuiInspectorMountingGroup::findDynamicFieldInDictionary( StringTableEntry fieldName )
{
   SimFieldDictionary * fieldDictionary = mParent->getInspectObject()->getFieldDictionary();

   for(SimFieldDictionaryIterator ditr(fieldDictionary); *ditr; ++ditr)
   {
      SimFieldDictionary::Entry * entry = (*ditr);

      if( entry->slotName == fieldName )
         return entry;
   }

   return NULL;
}

void GuiInspectorMountingGroup::addDynamicField()
{
}

AbstractClassRep::Field* GuiInspectorMountingGroup::findObjectComponentField(Component* target, String fieldName)
{
   AbstractClassRep::FieldList& fieldList = target->getClassRep()->mFieldList;
   for( AbstractClassRep::FieldList::iterator itr = fieldList.begin();
		itr != fieldList.end(); ++ itr )
   {
	  AbstractClassRep::Field* field = &( *itr );
	  String fldNm(field->pFieldname);
	  if(fldNm == fieldName)
		  return field;
   }
   return NULL;
}
ConsoleMethod( GuiInspectorMountingGroup, addDynamicField, void, 2, 2, "obj.addDynamicField();" )
{
   object->addDynamicField();
}

ConsoleMethod( GuiInspectorMountingGroup, removeDynamicField, void, 3, 3, "" )
{
}

//
IMPLEMENT_CONOBJECT( GuiInspectorNodeListField );

ConsoleDocClass( GuiInspectorNodeListField,
   "@brief A control that allows to edit the custom properties (text) of one or more SimObjects.\n\n"
   "Editor use only.\n\n"
   "@internal"
);

GuiInspectorNodeListField::GuiInspectorNodeListField( GuiInspector *inspector,
                                                    GuiInspectorGroup* parent, 
                                                    SimFieldDictionary::Entry* field,
																	 SimObjectPtr<Entity> target )
{
   mInspector = inspector;
   mParent = parent;
   setBounds(0,0,100,20);   
	mTargetEntity = target;
}

GuiInspectorNodeListField::GuiInspectorNodeListField()
{
   mInspector = NULL;
   mParent = NULL;
}

void GuiInspectorNodeListField::setData( const char* data, bool callbacks )
{
   mCustomValue = data;

	//We aren't updating any mounting info if we're not mounted already
	if(mTargetEntity.getObject())
	{
		Entity* target = dynamic_cast<Entity*>(mTargetEntity->getObjectMount());
		if(target)
		{
         RenderComponentInterface* renderInterface = target->getComponent<RenderComponentInterface>();
         if (renderInterface)
			{
            if (renderInterface->getShape())
				{
               S32 nodeIdx = renderInterface->getShape()->findNode(data);
				
					target->mountObject(mTargetEntity, nodeIdx, MatrixF::Identity);
					mTargetEntity->setMaskBits(Entity::MountedMask);
				}
			}
		}
	}

   // Force our edit to update
   updateValue();
}

const char* GuiInspectorNodeListField::getData( U32 inspectObjectIndex )
{
   return mCustomValue;
}

void GuiInspectorNodeListField::updateValue()
{
	mMenu->clear();
	//mMenu->addEntry("Origin");

	//if(mCustomValue.isEmpty())
	if(mTargetEntity.getObject())
	{
		Entity* target = dynamic_cast<Entity*>(mTargetEntity->getObjectMount());
		if(target)
		{
			mMenu->addEntry("Origin");
			mMenu->setActive(true);

         RenderComponentInterface* renderInterface = target->getComponent<RenderComponentInterface>();

         if (renderInterface)
			{
            TSShape* shape = renderInterface->getShape();

            S32 nodeCount = shape ? shape->nodes.size() : 0;

				for(U32 i=0; i < nodeCount; i++)
				{
               mMenu->addEntry(shape->names[i], i);
				}

				S32 targetNode = mTargetEntity->getMountNode();
				if(targetNode != -1)
				{
               String name = shape->names[targetNode];
					mCustomValue = name;
				}
				else
				{
					mCustomValue = String("Origin");
				}

				setValue( mCustomValue );
				return;
			}
		}
	}

	setValue("Not Mounted");
	mMenu->setActive(false);
}

void GuiInspectorNodeListField::setDoc( const char* doc )
{
   mDoc = StringTable->insert( doc, true );
}

void GuiInspectorNodeListField::setToolTip( StringTableEntry data )
{
   static StringTableEntry sTooltipProfile = StringTable->insert( "tooltipProfile" );
   static StringTableEntry sHoverTime = StringTable->insert( "hovertime" );
   static StringTableEntry sTooltip = StringTable->insert( "tooltip" );
   
   mEdit->setDataField( sTooltipProfile, NULL, "GuiToolTipProfile" );
   mEdit->setDataField( sHoverTime, NULL, "1000" );
   mEdit->setDataField( sTooltip, NULL, data );
}

bool GuiInspectorNodeListField::onAdd()
{
   if( !Parent::onAdd() )
      return false;

   return true;
}

void GuiInspectorNodeListField::setInspectorField( AbstractClassRep::Field *field, 
                                                  StringTableEntry caption, 
                                                  const char*arrayIndex ) 
{
   // Override the base just to be sure it doesn't get called.
   // We don't use an AbstractClassRep::Field...

	mField = field;
	mCaption = field->pFieldname;
	mDoc = field->pFieldDocs;
}

GuiControl* GuiInspectorNodeListField::constructEditControl()
{
   GuiControl* retCtrl = new GuiPopUpMenuCtrl();

	mMenu = dynamic_cast<GuiPopUpMenuCtrl*>(retCtrl);

   static StringTableEntry sProfile = StringTable->insert( "profile" );
   retCtrl->setDataField( sProfile, NULL, "ToolsGuiPopUpMenuEditProfile" );

   // Register the object
   retCtrl->registerObject();

	char szBuffer[512];
	dSprintf( szBuffer, 512, "%d.apply( %d.getText() );", getId(), mMenu->getId() );
	mMenu->setField("Command", szBuffer );

   return retCtrl;
}

void GuiInspectorNodeListField::setValue( const char* newValue )
{
   GuiPopUpMenuCtrl *ctrl = dynamic_cast<GuiPopUpMenuCtrl*>( mEdit );
   if( ctrl != NULL )
      ctrl->setText( newValue );
}

void GuiInspectorNodeListField::_executeSelectedCallback()
{
}

void GuiInspectorNodeListField::setTargetEntity(SimObjectPtr<Entity> target)
{
	mTargetEntity = target;
}