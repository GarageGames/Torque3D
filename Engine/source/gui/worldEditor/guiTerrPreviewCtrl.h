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

#ifndef _GUITERRPREVIEWCTRL_H_
#define _GUITERRPREVIEWCTRL_H_

#ifndef _GUICONTROL_H_
#include "gui/core/guiControl.h"
#endif
#ifndef _GUITSCONTROL_H_
#include "gui/3d/guiTSControl.h"
#endif
#ifndef _GFX_GFXDRAWER_H_
#include "gfx/gfxDrawUtil.h"
#endif
#ifndef _TERRAINEDITOR_H_
#include "gui/worldEditor/terrainEditor.h"
#endif


class GuiTerrPreviewCtrl : public GuiControl
{
private:
   typedef GuiControl Parent;
   GFXTexHandle mTextureHandle;
   GFXStateBlockRef mTerrainBitmapStateBlock;
   GFXStateBlockRef mControlsStateBlock;
   Point2F mRoot;
   Point2F mOrigin;
   Point2F mWorldScreenCenter;
   Point2F mCamera;
   F32     mTerrainSize;

   TerrainEditor* mTerrainEditor;

   Point2F& wrap(const Point2F &p);
   Point2F& worldToTexture(const Point2F &p);
   Point2F& worldToCtrl(const Point2F &p);


public:
   //creation methods
   DECLARE_CONOBJECT(GuiTerrPreviewCtrl);
   DECLARE_CATEGORY( "Gui Editor" );
   GuiTerrPreviewCtrl();
   static void initPersistFields();

   //Parental methods
   bool onWake();
   void onSleep();
   bool onAdd();

  void setBitmap(const GFXTexHandle&);

   void reset();
   void setRoot();
   void setRoot(const Point2F &root);
   void setOrigin(const Point2F &origin);
   const Point2F& getRoot() { return mRoot; }
   const Point2F& getOrigin() { return mOrigin; }

   //void setValue(const Point2F *center, const Point2F *camera);
   //const char *getScriptValue();
   void onPreRender();
   void onRender(Point2I offset, const RectI &updateRect);
};


#endif
