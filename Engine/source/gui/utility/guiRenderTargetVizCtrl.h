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
#pragma once

#ifndef GUIRENDERTARGETVIZCTRL_H
#define GUIRENDERTARGETVIZCTRL_H

#ifndef _GUICONTROL_H_
#include "gui/core/guiControl.h"
#endif
#ifndef _TEXTARGETBIN_MGR_H_
#include "renderInstance/renderTexTargetBinManager.h"
#endif

class GuiRenderTargetVizCtrl : public GuiControl
{
   typedef GuiControl Parent;

   RenderTexTargetBinManager* mRTTManager;

   SimObject* mCameraObject;

   StringTableEntry mTargetName;
   GFXFormat mTargetFormat;
   Point2I mTargetSize;

   GFXTextureTargetRef mTarget;
   GFXTexHandle mTargetTexture;

public:
   DECLARE_CONOBJECT(GuiRenderTargetVizCtrl);
   GuiRenderTargetVizCtrl();
   bool onWake();
   void onRender(Point2I offset, const RectI &updateRect);

   static void initPersistFields();
};

#endif //GUIRENDERTARGETVIZCTRL_H
