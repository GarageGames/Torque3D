#include "simpleHitboxComponent.h"

IMPLEMENT_CO_NETOBJECT_V1(SimpleHitboxComponent);

SimpleHitboxComponent::SimpleHitboxComponent() :
   // location of head, torso, legs
   mBoxHeadPercentage(0.85f),
   mBoxTorsoPercentage(0.55f),
   mBoxLeftPercentage(0),
   mBoxRightPercentage(1),
   mBoxBackPercentage(0),
   mBoxFrontPercentage(1),
   mIsProne(false)
{
}

SimpleHitboxComponent::~SimpleHitboxComponent()
{
}

void SimpleHitboxComponent::initPersistFields()
{
   addGroup("Hitbox");
   addField("headPercentage", TypeF32, Offset(mBoxHeadPercentage, SimpleHitboxComponent), "");
   addField("torsoPercentage", TypeF32, Offset(mBoxTorsoPercentage, SimpleHitboxComponent), "");
   addField("leftPercentage", TypeF32, Offset(mBoxLeftPercentage, SimpleHitboxComponent), "");
   addField("rightPercentage", TypeF32, Offset(mBoxRightPercentage, SimpleHitboxComponent), "");
   addField("backPercentage", TypeF32, Offset(mBoxBackPercentage, SimpleHitboxComponent), "");
   addField("frontPercentage", TypeF32, Offset(mBoxFrontPercentage, SimpleHitboxComponent), "");

   addField("isProne", TypeF32, Offset(mIsProne, SimpleHitboxComponent), "");
   endGroup("Hitbox");

   Parent::initPersistFields();
}

void SimpleHitboxComponent::onComponentAdd()
{
   Parent::onComponentAdd();
}

void SimpleHitboxComponent::componentAddedToOwner(Component *comp)
{
   Parent::componentAddedToOwner(comp);
}

void SimpleHitboxComponent::componentRemovedFromOwner(Component *comp)
{
   Parent::componentRemovedFromOwner(comp);
}

void SimpleHitboxComponent::processTick()
{
   Parent::processTick();
}

void SimpleHitboxComponent::interpolateTick(F32 dt)
{
   Parent::interpolateTick(dt);
}

void SimpleHitboxComponent::advanceTime(F32 dt)
{
   Parent::advanceTime(dt);
}

void SimpleHitboxComponent::getDamageLocation(const Point3F& in_rPos, const char *&out_rpVert, const char *&out_rpQuad)
{
   Point3F newPoint;
   mOwner->getWorldToObj().mulP(in_rPos, &newPoint);

   Point3F boxSize = mOwner->getObjBox().getExtents();

   F32 boxHeight = boxSize.z;
   F32 pointHeight = newPoint.z;

   if (mIsProne)
      pointHeight = newPoint.y; //this assumes we're y-forward

   F32 zTorso = mBoxTorsoPercentage;
   F32 zHead = mBoxHeadPercentage;

   zTorso *= boxHeight;
   zHead *= boxHeight;

   if (pointHeight <= zTorso)
      out_rpVert = "legs";
   else if (pointHeight <= zHead)
      out_rpVert = "torso";
   else
      out_rpVert = "head";

   if (dStrcmp(out_rpVert, "head") != 0)
   {
      if (newPoint.y >= 0.0f)
      {
         if (newPoint.x <= 0.0f)
            out_rpQuad = "front_left";
         else
            out_rpQuad = "front_right";
      }
      else
      {
         if (newPoint.x <= 0.0f)
            out_rpQuad = "back_left";
         else
            out_rpQuad = "back_right";
      }
   }
   else
   {
      F32 backToFront = boxSize.x;
      F32 leftToRight = boxSize.y;

      F32 backPoint = backToFront * mBoxBackPercentage;
      F32 frontPoint = backToFront * mBoxFrontPercentage;
      F32 leftPoint = leftToRight * mBoxLeftPercentage;
      F32 rightPoint = leftToRight * mBoxRightPercentage;

      S32 index = 0;
      if (newPoint.y < backPoint)
         index += 0;
      else if (newPoint.y >= frontPoint)
         index += 3;
      else
         index += 6;

      if (newPoint.x < leftPoint)
         index += 0;
      else if (newPoint.x >= rightPoint)
         index += 1;
      else
         index += 2;

      switch (index)
      {
         case 0: out_rpQuad = "left_back";      break;
         case 1: out_rpQuad = "middle_back";    break;
         case 2: out_rpQuad = "right_back";     break;
         case 3: out_rpQuad = "left_middle";    break;
         case 4: out_rpQuad = "middle_middle";  break;
         case 5: out_rpQuad = "right_middle";   break;
         case 6: out_rpQuad = "left_front";     break;
         case 7: out_rpQuad = "middle_front";   break;
         case 8: out_rpQuad = "right_front";    break;

         default:
            AssertFatal(0, "Bad non-tant index");
      };
   }
}

DefineEngineMethod(SimpleHitboxComponent, getDamageLocation, const char*, (Point3F pos), ,
   "@brief Get the named damage location and modifier for a given world position.\n\n"

   "the Player object can simulate different hit locations based on a pre-defined set "
   "of PlayerData defined percentages.  These hit percentages divide up the Player's "
   "bounding box into different regions.  The diagram below demonstrates how the various "
   "PlayerData properties split up the bounding volume:\n\n"

   "<img src=\"images/player_damageloc.png\">\n\n"

   "While you may pass in any world position and getDamageLocation() will provide a best-fit "
   "location, you should be aware that this can produce some interesting results.  For example, "
   "any position that is above PlayerData::boxHeadPercentage will be considered a 'head' hit, even "
   "if the world position is high in the sky.  Therefore it may be wise to keep the passed in point "
   "to somewhere on the surface of, or within, the Player's bounding volume.\n\n"

   "@note This method will not return an accurate location when the player is "
   "prone or swimming.\n\n"

   "@param pos A world position for which to retrieve a body region on this player.\n"

   "@return a string containing two words (space separated strings), where the "
   "first is a location and the second is a modifier.\n\n"

   "Posible locations:<ul>"
   "<li>head</li>"
   "<li>torso</li>"
   "<li>legs</li></ul>\n"

   "Head modifiers:<ul>"
   "<li>left_back</li>"
   "<li>middle_back</li>"
   "<li>right_back</li>"
   "<li>left_middle</li>"
   "<li>middle_middle</li>"
   "<li>right_middle</li>"
   "<li>left_front</li>"
   "<li>middle_front</li>"
   "<li>right_front</li></ul>\n"

   "Legs/Torso modifiers:<ul>"
   "<li>front_left</li>"
   "<li>front_right</li>"
   "<li>back_left</li>"
   "<li>back_right</li></ul>\n"

   "@see PlayerData::boxHeadPercentage\n"
   "@see PlayerData::boxHeadFrontPercentage\n"
   "@see PlayerData::boxHeadBackPercentage\n"
   "@see PlayerData::boxHeadLeftPercentage\n"
   "@see PlayerData::boxHeadRightPercentage\n"
   "@see PlayerData::boxTorsoPercentage\n"
)
{
   const char *buffer1;
   const char *buffer2;

   object->getDamageLocation(pos, buffer1, buffer2);

   static const U32 bufSize = 128;
   char *buff = Con::getReturnBuffer(bufSize);
   dSprintf(buff, bufSize, "%s %s", buffer1, buffer2);
   return buff;
}