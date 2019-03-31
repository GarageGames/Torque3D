//-----------------------------------------------------------------------------  
// Gui3DProjectionCtrl  
// Doppelganger Inc  
// Orion Elenzil 200701  
//   
// This control is meant to be merely a container for other controls.  
// What's neat is that it's easy to 'attach' this control to a point in world-space  
// or, more interestingly, to an object such as a player.  
//   
// Usage:  
// * Create the Gui3DProjectionControl - by default it will be at 0, 0, 0.  
// * You can change where it's located by setting the field "offsetWorld".  
//   - note you can specify that right in the .gui file  
// * You can attach it to any SceneObject by calling "setAttachedTo()".  
//  
// Behaviour:  
// * If you're attaching it to a player, by default it will center on the player's head.  
// * If you attach it to an object, by default it will delete itself if the object is deleted.  
// * Doesn't occlude w/r/t 3D objects.  
//  
// Console Methods:  
// * SetAttachedTo(SceneObject)  
// * GetAttachedTo()  
//  
// Params:  
// * pointWorld   - read/write point in worldspace. read-only if attached to an object.  
// * offsetObject - an offset in objectspace.                                default 0, 0, 0.  
// * offsetWorld  - an offset in worldspace.                                 default 0, 0, 0.  
// * offsetScreen - an offset in screenspace.                                default 0, 0.  
// * hAlign       - horizontal alignment. 0 = left, 1 = center, 2 = right.   default center.  
// * vAlign       - vertical   alignment. 0 = top,  1 = center, 2 = bottomt. default center.  
// * useEyePoint  - H & V usage of the eyePoint, if player object.           default 0, 1. (ie - use only the vertical component)  
// * autoDelete   - self-delete when attachedTo object is deleted.           default true.  
//  
// Todo:  
// * occlusion - hide the control when its anchor point is occluded.  
// * integrate w/ zbuffer - this would actually be a change to the whole GuiControl system.  
// * allow attaching to arbitrary nodes in a skeleton.  
// * avoid projection when the object is out of the frustum.  
//  
// oxe 20070111  
//-----------------------------------------------------------------------------  
  
#include "console/console.h"  
#include "console/consoleTypes.h"  
#include "scene/sceneObject.h"  
#include "T3D/player.h"  
#include "gui/controls/gui3DProjectionCtrl.h"  
  
IMPLEMENT_CONOBJECT(Gui3DProjectionCtrl);  
  
//-----------------------------------------------------------------------------  
  
Gui3DProjectionCtrl::Gui3DProjectionCtrl()  
{  
   mTSCtrl           = NULL;  
   mAttachedTo       = NULL;  
   mAttachedToPlayer = NULL;  
   mAutoDelete       = true;  
   mHAlign           = center;  
   mVAlign           = center;  
   mUseEyePoint.x    = 0;  
   mUseEyePoint.y    = 1;  
  
   mPtWorld     .set(0, 0, 0);  
   mPtProj      .set(0, 0);  
   mOffsetObject.set(0, 0, 0);  
   mOffsetWorld .set(0, 0, 0);  
   mOffsetScreen.set(0, 0);  
}  
  
void Gui3DProjectionCtrl::initPersistFields()  
{  
   Parent::initPersistFields();  
   addGroup("3DProjection");  
   addField("pointWorld"      , TypePoint3F , Offset(mPtWorld          , Gui3DProjectionCtrl));  
   addField("offsetObject"    , TypePoint3F , Offset(mOffsetObject     , Gui3DProjectionCtrl));  
   addField("offsetWorld"     , TypePoint3F , Offset(mOffsetWorld      , Gui3DProjectionCtrl));  
   addField("offsetScreen"    , TypePoint2I , Offset(mOffsetScreen     , Gui3DProjectionCtrl));  
   addField("hAlign"          , TypeS32     , Offset(mHAlign           , Gui3DProjectionCtrl));  
   addField("vAlign"          , TypeS32     , Offset(mVAlign           , Gui3DProjectionCtrl));  
   addField("useEyePoint"     , TypePoint2I , Offset(mUseEyePoint      , Gui3DProjectionCtrl));  
   addField("autoDelete"      , TypeBool    , Offset(mAutoDelete       , Gui3DProjectionCtrl));  
   endGroup("3DProjection");  
}  
  
void Gui3DProjectionCtrl::onRender(Point2I offset, const RectI &updateRect)  
{  
   doPositioning();
   doProjection();
   doAlignment();

   Parent::onRender(offset, updateRect);  
}  
  
void Gui3DProjectionCtrl::resizeDuringRender()  
{  
   doPositioning();  
   doProjection ();  
   doAlignment  ();  
}  
  
  
  
bool Gui3DProjectionCtrl::onWake()  
{  
   // walk up the GUI tree until we find a GuiTSCtrl.  
  
   mTSCtrl               = NULL;  
   GuiControl* walkCtrl  = getParent();  
   AssertFatal(walkCtrl != NULL, "Gui3DProjectionCtrl::onWake() - NULL parent");  
   bool doMore           = true;  
  
   while (doMore)  
   {  
      mTSCtrl  = dynamic_cast<GuiTSCtrl*>(walkCtrl);  
      walkCtrl = walkCtrl->getParent();  
      doMore   = (mTSCtrl == NULL) && (walkCtrl != NULL);  
   }  
  
   if (!mTSCtrl)  
      Con::errorf("Gui3DProjectionCtrl::onWake() - no TSCtrl parent");  
  
   return Parent::onWake();  
}  
  
void Gui3DProjectionCtrl::onSleep()  
{  
   mTSCtrl = NULL;  
   return Parent::onSleep();  
}  
  
void Gui3DProjectionCtrl::onDeleteNotify(SimObject* obj)  
{  
   // - SimSet assumes that obj is a member of THIS, which in our case ain't true.  
   // oxe 20070116 - the following doesn't compile on GCC.  
   // SimSet::Parent::onDeleteNotify(obj);  
  
   if (!obj)  
   {  
      Con::warnf("Gui3DProjectionCtrl::onDeleteNotify - got NULL");  
      return;  
   }  
  
    if (obj != mAttachedTo)  
   {  
      if (mAttachedTo != NULL)  
         Con::warnf("Gui3DProjectionCtrl::onDeleteNotify - got unexpected object: %d vs. %d", obj->getId(), mAttachedTo->getId());  
      return;  
   }  
  
   if (mAutoDelete)  
      this->deleteObject();  
}  
  
//-----------------------------------------------------------------------------  
  
void Gui3DProjectionCtrl::doPositioning()  
{  
   if (mAttachedTo == NULL)  
      return;  
  
   Point3F ptBase;   // the regular position of the object.  
   Point3F ptEye;    // the render position of the eye node, if a player object.  
   Point3F pt;       // combination of ptBase and ptEye.  
  
   MatrixF mat;      // utility  
     
   mAttachedTo->getRenderTransform().getColumn(3, &ptBase);  
  
   if (mAttachedToPlayer != NULL)  
   {  
      mAttachedToPlayer->getRenderEyeTransform(&mat);  
      mat.getColumn(3, &ptEye);  
   }  
   else  
   {  
      ptEye = ptBase;  
   }  
     
   // use some components from ptEye but other position from ptBase  
   pt = ptBase;  
   if (mUseEyePoint.x != 0)  
   {  
      pt.x = ptEye.x;  
      pt.y = ptEye.y;  
   }  
   if (mUseEyePoint.y != 0)  
   {  
      pt.z = ptEye.z;  
   }  
  
   // object-space offset  
   Point3F offsetObj;  
   QuatF quat(mAttachedTo->getRenderTransform());  
   quat.mulP(mOffsetObject, &offsetObj);  
   pt += offsetObj;  
  
  
   // world-space offset  
   pt += mOffsetWorld;  
  
   mPtWorld = pt;  
}  
  
  
void Gui3DProjectionCtrl::doProjection()  
{  
   if (!mTSCtrl)  
      return;  
  
   Point3F pt;  
  
   if (!mTSCtrl->project(mPtWorld, &pt))  
      return;  
  
   mPtProj.x = (S32)(pt.x + 0.5f);  
   mPtProj.y = (S32)(pt.y + 0.5f);  
}  
  
void Gui3DProjectionCtrl::doAlignment()  
{  
   // alignment  
   Point2I offsetAlign;  
   switch(mHAlign)  
   {  
   default:  
   case center:  
      offsetAlign.x = -getBounds().extent.x / 2;  
      break;  
   case min:  
      offsetAlign.x = 0;  
      break;  
   case max:  
      offsetAlign.x = -getBounds().extent.x;  
      break;  
   }  
  
   switch(mVAlign)  
   {  
   default:  
   case center:  
      offsetAlign.y = -getBounds().extent.y / 2;  
      break;  
   case min:  
      offsetAlign.y = 0;  
      break;  
   case max:  
      offsetAlign.y = -getBounds().extent.y;  
      break;  
   }  
  
   // projected point  
   mPtScreen  = mPtProj;  
  
   // alignment offset  
   mPtScreen += offsetAlign;  
  
   // screen offset  
   mPtScreen += mOffsetScreen;  
  
// setTrgPosition(mPtScreen);  
   RectI bounds = getBounds();
   bounds.point = mPtScreen;
   setBounds(bounds);  
}  
  
//-----------------------------------------------------------------------------  
  
void Gui3DProjectionCtrl::setAttachedTo(SceneObject* obj)  
{  
   if (obj == mAttachedTo)  
      return;  
  
   if (mAttachedTo)  
      clearNotify(mAttachedTo);  
  
   mAttachedTo       = obj;  
   mAttachedToPlayer = dynamic_cast<Player*>(obj);  
  
   if (mAttachedTo)  
      deleteNotify(mAttachedTo);  
}  
  
DefineEngineMethod(Gui3DProjectionCtrl, setAttachedTo, void, (SceneObject* target), (nullAsType<SceneObject*>()), "(object)")
{
   if(target)
      object->setAttachedTo(target);
}  
  
DefineEngineMethod(Gui3DProjectionCtrl, getAttachedTo, S32, (),, "()")  
{  
   SceneObject* obj = object->getAttachedTo();  
   if (!obj)  
      return 0;  
   else  
      return obj->getId();  
}  
