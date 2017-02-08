#ifndef _GUIOFFSCREENCANVAS_H_
#define _GUIOFFSCREENCANVAS_H_

#include "math/mMath.h"
#include "gui/core/guiCanvas.h"
#include "core/util/tVector.h"

#ifndef _MATTEXTURETARGET_H_
#include "materials/matTextureTarget.h"
#endif

class GuiTextureDebug;

class GuiOffscreenCanvas : public GuiCanvas
{
public:
   typedef GuiCanvas Parent;
   
   GuiOffscreenCanvas();
   ~GuiOffscreenCanvas();
   
   bool onAdd();
   void onRemove();
   
   void renderFrame(bool preRenderOnly, bool bufferSwap);
   virtual void onFrameRendered();
   
   Point2I getWindowSize();

   Point2I getCursorPos();
   void setCursorPos(const Point2I &pt);
   void showCursor(bool state);
   bool isCursorShown();
   
   void _onTextureEvent( GFXTexCallbackCode code );

   void _setupTargets();
   void _teardownTargets();

   NamedTexTargetRef getTarget() { return &mNamedTarget; }

   void markDirty() { mTargetDirty = true; }

   static void initPersistFields();
   
   DECLARE_CONOBJECT(GuiOffscreenCanvas);

protected:
   GFXTextureTargetRef mTarget;
   NamedTexTarget mNamedTarget;
   GFXTexHandle mTargetTexture;

   GFXFormat mTargetFormat;
   Point2I mTargetSize;
   String mTargetName;

   bool mTargetDirty;
   bool mDynamicTarget;
   
   bool mUseDepth;
   GFXTexHandle mTargetDepth;

public:
   static Vector<GuiOffscreenCanvas*> sList;
};

#endif
