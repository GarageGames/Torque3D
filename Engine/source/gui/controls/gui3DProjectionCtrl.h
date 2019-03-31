//-----------------------------------------------------------------------------  
// Gui3DProjectionCtrl  
// Doppelganger Inc  
// Orion Elenzil 200701  
//  
//   
//-----------------------------------------------------------------------------  
  
#ifndef _GUI3DPROJECTIONCTRL_H_  
#define _GUI3DPROJECTIONCTRL_H_  
  
#include "gui/core/guiTypes.h"  
#include "gui/core/guiControl.h"  
#include "gui/3d/guiTSControl.h"  
#include "scene/sceneObject.h"  
#include "T3D/player.h"  
  
class Gui3DProjectionCtrl : public GuiControl  
{  
  
//-----------------------------------------------------------------------------  
// stock stuff  
public:  
   Gui3DProjectionCtrl();  
   typedef GuiControl Parent;  
  
   DECLARE_CONOBJECT(Gui3DProjectionCtrl);  
  
   static  void initPersistFields     ();  
  
//-----------------------------------------------------------------------------  
// more interesting stuff  
  
   GuiTSCtrl*                mTSCtrl;           /// must be a child of one of these.  
   SimObjectPtr<SceneObject> mAttachedTo;       /// optional object we're attached to.  
   SimObjectPtr<Player>      mAttachedToPlayer; /// same pointer as mAttachedTo, but conveniently casted to player.  
  
   Point3F                   mPtWorld;          /// the worldspace point which we're projecting  
   Point2I                   mPtProj;           /// the screenspace projected point. - note there are further modifiers before   
   Point2I                   mPtScreen;  
  
   Point3F                   mOffsetObject;     /// object-space offset applied first  to the attached point to obtain mPtWorld.  
   Point3F                   mOffsetWorld;      /// world-space  offset applied second to the attached point to obtain mPtWorld.  
   Point2I                   mOffsetScreen;     /// screen-space offset applied to mPtProj. note we still have centering, etc.  
  
   enum alignment  
   {  
      min    = 0,  
      center = 1,  
      max    = 2  
   };  
  
   alignment                 mHAlign;           /// horizontal alignment  
   alignment                 mVAlign;           /// horizontal alignment  
  
   bool                      mAutoDelete;       /// optionally self-delete when mAttachedTo is deleted.  
   Point2I                   mUseEyePoint;      /// optionally use the eye point. x != 0 -> horiz.  y != 0 -> vert.  
  
  
   virtual void   onRender            (Point2I offset, const RectI &updateRect);  
   virtual void   resizeDuringRender  ();  
  
   virtual bool   onWake              ();  
   virtual void   onSleep             ();  
   virtual void   onDeleteNotify      (SimObject *object);  
  
   void           doPositioning       ();  
   void           doProjection        ();  
   void           doAlignment         ();  
  
   void           setAttachedTo       (SceneObject*  obj);  
   SceneObject*   getAttachedTo       ()                 { return mAttachedTo; }  
   void           setWorldPt          (Point3F& pt)      { mPtWorld = pt;      }  
   Point3F        getWorldPt          ()                 { return mPtWorld;    }  
};  
  
#endif //_GUI3DPROJECTIONCTRL_H_ 