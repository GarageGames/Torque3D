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
#include "gui/utility/guiRenderTargetVizCtrl.h"

#include "console/console.h"
#include "console/consoleTypes.h"
#include "gfx/gfxDevice.h"
#include "gfx/gfxDrawUtil.h"
#include "gui/core/guiCanvas.h"
#include "gui/core/guiDefaultControlRender.h"

#include "T3D/camera.h"

IMPLEMENT_CONOBJECT(GuiRenderTargetVizCtrl);

ConsoleDocClass(GuiRenderTargetVizCtrl,
   "@brief The most widely used button class.\n\n"

   "GuiRenderTargetVizCtrl renders seperately of, but utilizes all of the functionality of GuiBaseButtonCtrl.\n"
   "This grants GuiRenderTargetVizCtrl the versatility to be either of the 3 button types.\n\n"

   "@tsexample\n"
   "// Create a PushButton GuiRenderTargetVizCtrl that calls randomFunction when clicked\n"
   "%button = new GuiRenderTargetVizCtrl()\n"
   "{\n"
   "   profile    = \"GuiButtonProfile\";\n"
   "   buttonType = \"PushButton\";\n"
   "   command    = \"randomFunction();\";\n"
   "};\n"
   "@endtsexample\n\n"

   "@ingroup GuiButtons"
);


//-----------------------------------------------------------------------------

GuiRenderTargetVizCtrl::GuiRenderTargetVizCtrl()
{
   mTargetName = StringTable->EmptyString();

   mTarget = nullptr;
   mTargetTexture = nullptr;

   mCameraObject = nullptr;
}

//-----------------------------------------------------------------------------

bool GuiRenderTargetVizCtrl::onWake()
{
   if (!Parent::onWake())
      return false;

   return true;
}

void GuiRenderTargetVizCtrl::initPersistFields()
{
   Parent::initPersistFields();

   addField("RenderTargetName", TypeString, Offset(mTargetName, GuiRenderTargetVizCtrl), "");
   addField("cameraObject", TypeSimObjectPtr, Offset(mCameraObject, GuiRenderTargetVizCtrl), "");
}

//-----------------------------------------------------------------------------

void GuiRenderTargetVizCtrl::onRender(Point2I      offset,
   const RectI& updateRect)
{
   GFXDrawUtil* drawer = GFX->getDrawUtil();

   RectI boundsRect(offset, getExtent());

   //Draw backdrop
   GFX->getDrawUtil()->drawRectFill(boundsRect, ColorI::BLACK);

   if (mCameraObject != nullptr)
   {
      Camera* camObject = dynamic_cast<Camera*>(mCameraObject);

      camObject = dynamic_cast<Camera*>(camObject->getClientObject());

      bool servObj = camObject->isServerObject();

      if (camObject)
      {
         GFXTexHandle targ = camObject->getCameraRenderTarget();

         if (targ)
         {
            Point2I size = Point2I(targ->getWidth(), targ->getHeight());
            drawer->drawBitmapStretchSR(targ, boundsRect, RectI(Point2I::Zero, size));
         }
         return;
      }
   }
   else if (mTargetName == StringTable->EmptyString())
      return;

   NamedTexTarget* namedTarget = NamedTexTarget::find(mTargetName);
   if (namedTarget)
   {
      GFXTextureObject* theTex = namedTarget->getTexture(0);
      RectI viewport = namedTarget->getViewport();

      drawer->drawBitmapStretchSR(theTex, boundsRect, viewport);
   }
}
