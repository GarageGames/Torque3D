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

#include "console/console.h"
#include "console/consoleTypes.h"
#include "terrain/terrData.h"
#include "gui/worldEditor/guiTerrPreviewCtrl.h"
#include "gfx/primBuilder.h"
#include "T3D/gameFunctions.h"

IMPLEMENT_CONOBJECT(GuiTerrPreviewCtrl);

ConsoleDocClass( GuiTerrPreviewCtrl,
   "@brief Very old GUI used for terrain preview\n\n"
   "Deprecated\n\n"
   "@internal"
);

GuiTerrPreviewCtrl::GuiTerrPreviewCtrl(void) : mTerrainEditor(NULL), mTerrainSize(2048.0f)
{
   mRoot.set( 0, 0 );
   mOrigin.set( 0, 0 );
   mWorldScreenCenter.set( mTerrainSize*0.5f, mTerrainSize*0.5f );
   mControlsStateBlock = NULL;
   mTerrainBitmapStateBlock = NULL;
}

bool GuiTerrPreviewCtrl::onAdd()
{
   if(Parent::onAdd() == false)
   {
      return false;
   }

   SimObject* inTerrEditor = Sim::findObject("ETerrainEditor");
   if(!inTerrEditor)
   {
      Con::errorf(ConsoleLogEntry::General, "TerrainEditor::onAdd: failed to load Terrain Editor");
      return false;
   }

   mTerrainEditor = dynamic_cast<TerrainEditor*>(inTerrEditor);

   GFXStateBlockDesc desc;

   desc.setBlend(false, GFXBlendOne, GFXBlendZero);

   desc.samplersDefined = true;
   desc.samplers[0].addressModeU = GFXAddressWrap;
   desc.samplers[0].addressModeV = GFXAddressWrap;
   desc.samplers[0].textureColorOp = GFXTOPSelectARG1;
   desc.samplers[0].colorArg1 = GFXTATexture;
   desc.setCullMode(GFXCullNone);
   desc.setZReadWrite(false);

   mTerrainBitmapStateBlock = GFX->createStateBlock(desc);

   desc.samplers[0].textureColorOp = GFXTOPDisable;

   mControlsStateBlock = GFX->createStateBlock(desc);

   return true;
}

void GuiTerrPreviewCtrl::initPersistFields()
{
   Parent::initPersistFields();
}


ConsoleMethod( GuiTerrPreviewCtrl, reset, void, 2, 2, "Reset the view of the terrain.")
{
   object->reset();
}

ConsoleMethod( GuiTerrPreviewCtrl, setRoot, void, 2, 2, "Add the origin to the root and reset the origin.")
{
   object->setRoot();
}

ConsoleMethod( GuiTerrPreviewCtrl, getRoot, const char *, 2, 2, "Return a Point2F representing the position of the root.")
{
   Point2F p = object->getRoot();

   static char rootbuf[32];
   dSprintf(rootbuf,sizeof(rootbuf),"%g %g", p.x, -p.y);
   return rootbuf;
}

ConsoleMethod( GuiTerrPreviewCtrl, setOrigin, void, 4, 4, "(float x, float y)"
              "Set the origin of the view.")
{
   object->setOrigin( Point2F( dAtof(argv[2]), -dAtof(argv[3]) ) );
}

ConsoleMethod( GuiTerrPreviewCtrl, getOrigin, const char*, 2, 2, "Return a Point2F containing the position of the origin.")
{
   Point2F p = object->getOrigin();

   static char originbuf[32];
   dSprintf(originbuf,sizeof(originbuf),"%g %g", p.x, -p.y);
   return originbuf;
}

ConsoleMethod( GuiTerrPreviewCtrl, getValue, const char*, 2, 2, "Returns a 4-tuple containing: root_x root_y origin_x origin_y")
{
   Point2F r = object->getRoot();
   Point2F o = object->getOrigin();

   static char valuebuf[64];
   dSprintf(valuebuf,sizeof(valuebuf),"%g %g %g %g", r.x, -r.y, o.x, -o.y);
   return valuebuf;
}

ConsoleMethod( GuiTerrPreviewCtrl, setValue, void, 3, 3, "Accepts a 4-tuple in the same form as getValue returns.\n\n"
              "@see GuiTerrPreviewCtrl::getValue()")
{
   Point2F r,o;
   dSscanf(argv[2],"%g %g %g %g", &r.x, &r.y, &o.x, &o.y);
   r.y = -r.y;
   o.y = -o.y;
   object->reset();
   object->setRoot(r);
   object->setOrigin(o);
}

bool GuiTerrPreviewCtrl::onWake()
{
   if (! Parent::onWake())
      return false;

   return true;
}

void GuiTerrPreviewCtrl::onSleep()
{
   Parent::onSleep();
}

void GuiTerrPreviewCtrl::setBitmap(const GFXTexHandle &handle)
{
   mTextureHandle = handle;
}


void GuiTerrPreviewCtrl::reset()
{
   mRoot.set(0,0);
   mOrigin.set(0,0);
}

void GuiTerrPreviewCtrl::setRoot()
{
   mRoot += mOrigin;
   mOrigin.set(0,0);
}

void GuiTerrPreviewCtrl::setRoot(const Point2F &p)
{
   mRoot = p;
}

void GuiTerrPreviewCtrl::setOrigin(const Point2F &p)
{
   mOrigin = p;
}


Point2F& GuiTerrPreviewCtrl::wrap(const Point2F &p)
{
   static Point2F result;
   result = p;

   while (result.x < 0.0f)
      result.x += mTerrainSize;
   while (result.x > mTerrainSize)
      result.x -= mTerrainSize;
   while (result.y < 0.0f)
      result.y += mTerrainSize;
   while (result.y > mTerrainSize)
      result.y -= mTerrainSize;

   return result;
}

Point2F& GuiTerrPreviewCtrl::worldToTexture(const Point2F &p)
{
   static Point2F result;
   result = wrap( p + mRoot ) / mTerrainSize;
   return result;
}


Point2F& GuiTerrPreviewCtrl::worldToCtrl(const Point2F &p)
{
   static Point2F result;
   result = wrap( p - mCamera - mWorldScreenCenter );
   result *= getWidth() / mTerrainSize;
   return result;
}


void GuiTerrPreviewCtrl::onPreRender()
{
   setUpdate();
}

void GuiTerrPreviewCtrl::onRender(Point2I offset, const RectI &updateRect)
{
   CameraQuery query;
	GameProcessCameraQuery(&query);
   Point3F cameraRot;
   TerrainBlock *terrBlock = NULL;

	MatrixF matrix = query.cameraMatrix;
   matrix.getColumn(3,&cameraRot);           // get Camera translation
   mCamera.set(cameraRot.x, -cameraRot.y);
	matrix.getRow(1,&cameraRot);              // get camera rotation

   if (mTerrainEditor != NULL)
      terrBlock = mTerrainEditor->getActiveTerrain();

   if (!terrBlock)
      return;

   for(U32 i = 0; i < GFX->getNumSamplers(); i++)
      GFX->setTexture(i, NULL);
   
   GFX->disableShaders();
   
   Point2F terrPos(terrBlock->getPosition().x, terrBlock->getPosition().y);

   mTerrainSize = terrBlock->getWorldBlockSize();


   //----------------------------------------- RENDER the Terrain Bitmap
   if (mTextureHandle)
   {

      GFXTextureObject *texture = (GFXTextureObject*)mTextureHandle;
      if (texture)
      {
         //GFX->setLightingEnable(false);
         GFX->setStateBlock(mTerrainBitmapStateBlock);
         GFX->setTexture(0, texture);

         Point2F screenP1(offset.x - 0.5f, offset.y + 0.5f);
         Point2F screenP2(offset.x + getWidth() - 0.5f, offset.y + getWidth() + 0.5f);
         Point2F textureP1( worldToTexture( mCamera - terrPos ) - Point2F(0.5f, 0.5f));
         Point2F textureP2(textureP1 + Point2F(1.0f, 1.0f));

         // the texture if flipped horz to reflect how the terrain is really drawn
         PrimBuild::color3f(1.0f, 1.0f, 1.0f);
         PrimBuild::begin(GFXTriangleFan, 4);
            PrimBuild::texCoord2f(textureP1.x, textureP2.y);
            PrimBuild::vertex2f(screenP1.x, screenP2.y);       // left bottom

            
            PrimBuild::texCoord2f(textureP2.x, textureP2.y);
            PrimBuild::vertex2f(screenP2.x, screenP2.y);       // right bottom
            PrimBuild::texCoord2f(textureP2.x, textureP1.y);
            PrimBuild::vertex2f(screenP2.x, screenP1.y);       // right top

            PrimBuild::texCoord2f(textureP1.x, textureP1.y);
            PrimBuild::vertex2f(screenP1.x, screenP1.y);       // left top
         PrimBuild::end();
      }
   }
   //Draw blank texture
   else
   {
      RectI rect(offset.x, offset.y, getWidth(), getHeight());
       GFX->getDrawUtil()->drawRect(rect, ColorI(0,0,0));
   }

   GFX->setStateBlock(mControlsStateBlock);

   //----------------------------------------- RENDER the '+' at the center of the Block

   PrimBuild::color4f(1.0f, 1.0f, 1.0f, 1.0f);
   Point2F center( worldToCtrl(terrPos + Point2F(mTerrainSize * 0.5f, mTerrainSize * 0.5f)) );
   S32 y;
   for (y=-1; y<=1; y++)
   {
      F32 yoffset = offset.y + y*256.0f;
      for (S32 x=-1; x<=1; x++)
      {
         F32 xoffset = offset.x + x*256.0f;
         PrimBuild::begin(GFXLineList, 4);
            PrimBuild::vertex2f(xoffset + center.x, yoffset + center.y-5);
            PrimBuild::vertex2f(xoffset + center.x, yoffset + center.y+6);
            PrimBuild::vertex2f(xoffset + center.x-5, yoffset + center.y);
            PrimBuild::vertex2f(xoffset + center.x+6, yoffset + center.y);
         PrimBuild::end();
      }
   }

   //----------------------------------------- RENDER the Block Corners
   Point2F cornerf( worldToCtrl(terrPos)  + Point2F(0.125f, 0.125f));
   Point2I corner=Point2I((S32)cornerf.x,(S32)cornerf.y);
   for (y=-1; y<=1; y++)
   {
      S32 yoffset = offset.y + y*256;
      for (S32 x=-1; x<=1; x++)
      {
         S32 xoffset = offset.x + x*256;
         PrimBuild::begin(GFXLineStrip, 3);
            PrimBuild::color4f(1.0f, 1.0f, 1.0f, 0.3f);
            PrimBuild::vertex2i(xoffset + corner.x, yoffset + corner.y-128);
            PrimBuild::color4f(1.0f, 1.0f, 1.0f, 0.7f);
            PrimBuild::vertex2i(xoffset + corner.x, yoffset + corner.y);
            PrimBuild::color4f(1.0f, 1.0f, 1.0f, 0.3f);
            PrimBuild::vertex2i(xoffset + corner.x+128, yoffset + corner.y);
         PrimBuild::end();
         PrimBuild::begin(GFXLineStrip, 3);
            PrimBuild::color4f(1.0f, 1.0f, 1.0f, 0.3f);
            PrimBuild::vertex2i(xoffset + corner.x, yoffset + corner.y+128);
            PrimBuild::color4f(1.0f, 1.0f, 1.0f, 0.7f);
            PrimBuild::vertex2i(xoffset + corner.x, yoffset + corner.y);
            PrimBuild::color4f(1.0f, 1.0f, 1.0f, 0.3f);
            PrimBuild::vertex2i(xoffset + corner.x-128, yoffset + corner.y);
         PrimBuild::end();
      }
   }

  //----------------------------------------- RENDER the Viewcone
   Point2F pointA(cameraRot.x * -40, cameraRot.y * -40);
	Point2F pointB(-pointA.y, pointA.x);

	F32 tann = mTan(0.5f);
	Point2F point1( pointA + pointB * tann );
	Point2F point2( pointA - pointB * tann );

	center.set((F32)(offset.x + getWidth() / 2), (F32)(offset.y + getHeight() / 2 ));
   PrimBuild::begin(GFXLineStrip, 3);
	   PrimBuild::color4f(1.0f, 0.0f, 0.0f, 0.7f);
	   PrimBuild::vertex2i((S32)(center.x + point1.x), (S32)(center.y + point1.y));
	   PrimBuild::color4f(1.0f, 0.0f, 0.0f, 1.0f);
	   PrimBuild::vertex2i((S32)center.x,(S32)center.y);
	   PrimBuild::color4f(1.0f, 0.0f, 0.0f, 0.7f);
	   PrimBuild::vertex2i((S32)(center.x + point2.x), (S32)(center.y + point2.y));
   PrimBuild::end();


   renderChildControls(offset, updateRect);
}

