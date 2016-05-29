//-----------------------------------------------------------------------------
// Torque Game Engine
// Copyright (C) GarageGames.com, Inc.
//-----------------------------------------------------------------------------
#include "console/consoleTypes.h"
#include "T3D/components/game/triggerComponent.h"
#include "core/util/safeDelete.h"
#include "console/consoleTypes.h"
#include "console/consoleObject.h"
#include "core/stream/bitStream.h"
#include "console/engineAPI.h"
#include "sim/netConnection.h"
#include "T3D/gameBase/gameConnection.h"
#include "T3D/components/coreInterfaces.h"
#include "math/mathUtils.h"
#include "collision/concretePolyList.h"
#include "collision/clippedPolyList.h"

#include "gfx/sim/debugDraw.h"

IMPLEMENT_CALLBACK( TriggerComponent, onEnterViewCmd, void, 
   ( Entity* cameraEnt, bool firstTimeSeeing ), ( cameraEnt, firstTimeSeeing ),
   "@brief Called when an object enters the volume of the Trigger instance using this TriggerData.\n\n"

   "@param trigger the Trigger instance whose volume the object entered\n"
   "@param obj the object that entered the volume of the Trigger instance\n" );

IMPLEMENT_CALLBACK( TriggerComponent, onExitViewCmd, void, 
   ( Entity* cameraEnt ), ( cameraEnt ),
   "@brief Called when an object enters the volume of the Trigger instance using this TriggerData.\n\n"

   "@param trigger the Trigger instance whose volume the object entered\n"
   "@param obj the object that entered the volume of the Trigger instance\n" );

IMPLEMENT_CALLBACK( TriggerComponent, onUpdateInViewCmd, void, 
   ( Entity* cameraEnt ), ( cameraEnt ),
   "@brief Called when an object enters the volume of the Trigger instance using this TriggerData.\n\n"

   "@param trigger the Trigger instance whose volume the object entered\n"
   "@param obj the object that entered the volume of the Trigger instance\n" );

IMPLEMENT_CALLBACK( TriggerComponent, onUpdateOutOfViewCmd, void, 
   ( Entity* cameraEnt ), ( cameraEnt ),
   "@brief Called when an object enters the volume of the Trigger instance using this TriggerData.\n\n"

   "@param trigger the Trigger instance whose volume the object entered\n"
   "@param obj the object that entered the volume of the Trigger instance\n" );

//////////////////////////////////////////////////////////////////////////
// Constructor/Destructor
//////////////////////////////////////////////////////////////////////////

TriggerComponent::TriggerComponent() : Component()
{
   mObjectList.clear();

   mVisible = false;

   mFriendlyName = "Trigger";
   mComponentType = "Trigger";

   mDescription = getDescriptionText("Calls trigger events when a client starts and stops seeing it. Also ticks while visible to clients.");
}

TriggerComponent::~TriggerComponent()
{
   for(S32 i = 0;i < mFields.size();++i)
   {
      ComponentField &field = mFields[i];
      SAFE_DELETE_ARRAY(field.mFieldDescription);
   }

   SAFE_DELETE_ARRAY(mDescription);
}

IMPLEMENT_CO_NETOBJECT_V1(TriggerComponent);


bool TriggerComponent::onAdd()
{
   if(! Parent::onAdd())
      return false;

   return true;
}

void TriggerComponent::onRemove()
{
   Parent::onRemove();
}

//This is mostly a catch for situations where the behavior is re-added to the object and the like and we may need to force an update to the behavior
void TriggerComponent::onComponentAdd()
{
   Parent::onComponentAdd();

   CollisionInterface *colInt = mOwner->getComponent<CollisionInterface>();

   if(colInt)
   {
      colInt->onCollisionSignal.notify(this, &TriggerComponent::potentialEnterObject);
   }
}

void TriggerComponent::onComponentRemove()
{
   CollisionInterface *colInt = mOwner->getComponent<CollisionInterface>();

   if(colInt)
   {
      colInt->onCollisionSignal.remove(this, &TriggerComponent::potentialEnterObject);
   }

   Parent::onComponentRemove();
}

void TriggerComponent::componentAddedToOwner(Component *comp)
{
   if (comp->getId() == getId())
      return;

   CollisionInterface *colInt = mOwner->getComponent<CollisionInterface>();

   if (colInt)
   {
      colInt->onCollisionSignal.notify(this, &TriggerComponent::potentialEnterObject);
   }
}

void TriggerComponent::componentRemovedFromOwner(Component *comp)
{
   if (comp->getId() == getId()) //?????????
      return;

   CollisionInterface *colInt = mOwner->getComponent<CollisionInterface>();

   if (colInt)
   {
      colInt->onCollisionSignal.remove(this, &TriggerComponent::potentialEnterObject);
   }
}

void TriggerComponent::initPersistFields()
{
   Parent::initPersistFields();

   addField("visibile",   TypeBool,  Offset( mVisible, TriggerComponent ), "" );

   addField("onEnterViewCmd", TypeCommand, Offset(mEnterCommand, TriggerComponent), "");
   addField("onExitViewCmd", TypeCommand, Offset(mOnExitCommand, TriggerComponent), "");
   addField("onUpdateInViewCmd", TypeCommand, Offset(mOnUpdateInViewCmd, TriggerComponent), "");
}

U32 TriggerComponent::packUpdate(NetConnection *con, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);
   return retMask;
}

void TriggerComponent::unpackUpdate(NetConnection *con, BitStream *stream)
{
   Parent::unpackUpdate(con, stream);
}

void TriggerComponent::potentialEnterObject(SceneObject *collider)
{
   if(testObject(collider))
   {
      bool found = false;
      for(U32 i=0; i < mObjectList.size(); i++)
      {
         if(mObjectList[i]->getId() == collider->getId())
         {
            found = true;
            break;
         }
      }

      if (!found)
      {
         mObjectList.push_back(collider);

         if (!mEnterCommand.isEmpty())
         {
            String command = String("%obj = ") + collider->getIdString() + ";" + 
               String("%this = ") + getIdString() + ";" + mEnterCommand;
            Con::evaluate(command.c_str());
         }

         //onEnterTrigger_callback(this, enter);
      }
   }
}

bool TriggerComponent::testObject(SceneObject* enter)
{
   //First, test to early out
   Box3F enterBox = enter->getWorldBox();

   //if(!mOwner->getWorldBox().intersect(enterBox) || !)
   //   return false;

   //We're still here, so we should do actual work
   //We're going to be 
   ConcretePolyList mClippedList;

   SphereF sphere;
   sphere.center = (mOwner->getWorldBox().minExtents + mOwner->getWorldBox().maxExtents) * 0.5;
   VectorF bv = mOwner->getWorldBox().maxExtents - sphere.center;
   sphere.radius = bv.len();

   Entity* enterEntity = dynamic_cast<Entity*>(enter);
   if(enterEntity)
   {
      //quick early out. If the bounds don't overlap, it cannot be colliding or inside
      if (!mOwner->getWorldBox().isOverlapped(enterBox))
         return false;

      //check if the entity has a collision shape
      CollisionInterface *cI = enterEntity->getComponent<CollisionInterface>();
      if (cI)
      {
         cI->buildPolyList(PLC_Collision, &mClippedList, mOwner->getWorldBox(), sphere);

         if (!mClippedList.isEmpty())
         {
            //well, it's clipped with, or inside, our bounds
            //now to test the clipped list against our own collision mesh
            CollisionInterface *myCI = mOwner->getComponent<CollisionInterface>();

            //wait, how would we NOT have this?
            if (myCI)
            {
               //anywho, build our list and then we'll check intersections
               ClippedPolyList myList;

               MatrixF ownerTransform = mOwner->getTransform();
               myList.setTransform(&ownerTransform, mOwner->getScale());
               myList.setObject(mOwner);

               myCI->buildPolyList(PLC_Collision, &myList, enterBox, sphere);

               bool test = true;
            }
         }
      }
   }

   return mClippedList.isEmpty() == false;
}

void TriggerComponent::processTick()
{
   Parent::processTick();

   if (!isActive())
      return;

	//get our list of active clients, and see if they have cameras, if they do, build a frustum and see if we exist inside that
   mVisible = false;
   if(isServerObject())
   {
      for(U32 i=0; i < mObjectList.size(); i++)
      {
         if(!testObject(mObjectList[i]))
         {
            if (!mOnExitCommand.isEmpty())
            {
               String command = String("%obj = ") + mObjectList[i]->getIdString() + ";" +
                  String("%this = ") + getIdString() + ";" + mOnExitCommand;
               Con::evaluate(command.c_str());
            }

            mObjectList.erase(i);
            //mDataBlock->onLeaveTrigger_callback( this, remove );
            //onLeaveTrigger_callback(this, remove);
         }
      }

      /*if (!mTickCommand.isEmpty())
         Con::evaluate(mTickCommand.c_str());

      if (mObjects.size() != 0)
         onTickTrigger_callback(this);*/
   }
}

void TriggerComponent::visualizeFrustums(F32 renderTimeMS)
{
   
}

GameConnection* TriggerComponent::getConnection(S32 connectionID)
{
   for(NetConnection *conn = NetConnection::getConnectionList(); conn; conn = conn->getNext())  
   {  
      GameConnection* gameConn = dynamic_cast<GameConnection*>(conn);
  
      if (!gameConn || (gameConn && gameConn->isAIControlled()))
         continue; 

      if(connectionID == gameConn->getId())
         return gameConn;
   }

   return NULL;
}

void TriggerComponent::addClient(S32 clientID)
{
   
}

void TriggerComponent::removeClient(S32 clientID)
{

}

DefineEngineMethod( TriggerComponent, addClient, void,
                   ( S32 clientID ), ( -1 ),
                   "@brief Mount objB to this object at the desired slot with optional transform.\n\n"

                   "@param objB  Object to mount onto us\n"
                   "@param slot  Mount slot ID\n"
                   "@param txfm (optional) mount offset transform\n"
                   "@return true if successful, false if failed (objB is not valid)" )
{
   if(clientID == -1)
      return;

   object->addClient( clientID );
}

DefineEngineMethod( TriggerComponent, removeClient, void,
                   ( S32 clientID ), ( -1 ),
                   "@brief Mount objB to this object at the desired slot with optional transform.\n\n"

                   "@param objB  Object to mount onto us\n"
                   "@param slot  Mount slot ID\n"
                   "@param txfm (optional) mount offset transform\n"
                   "@return true if successful, false if failed (objB is not valid)" )
{
   if(clientID == -1)
      return;

   object->removeClient( clientID );
}

DefineEngineMethod( TriggerComponent, visualizeFrustums, void,
                   (F32 renderTime), (1000),
                   "@brief Mount objB to this object at the desired slot with optional transform.\n\n"

                   "@param objB  Object to mount onto us\n"
                   "@param slot  Mount slot ID\n"
                   "@param txfm (optional) mount offset transform\n"
                   "@return true if successful, false if failed (objB is not valid)" )
{
   object->visualizeFrustums(renderTime);
}