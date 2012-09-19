////-----------------------------------------------------------------------------
//// Copyright (c) 2012 GarageGames, LLC
////
//// Permission is hereby granted, free of charge, to any person obtaining a copy
//// of this software and associated documentation files (the "Software"), to
//// deal in the Software without restriction, including without limitation the
//// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
//// sell copies of the Software, and to permit persons to whom the Software is
//// furnished to do so, subject to the following conditions:
////
//// The above copyright notice and this permission notice shall be included in
//// all copies or substantial portions of the Software.
////
//// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
//// IN THE SOFTWARE.
////-----------------------------------------------------------------------------
//
//#include "console/console.h"
//
//#include "windowManager/platformWindowMgr.h"
//#include "unit/test.h"
//#include "core/util/journal/process.h"
//#include "gfx/gfxInit.h"
//#include "gfx/primBuilder.h"
//#include "gfx/gFont.h"
//#include "gfx/gfxDrawUtil.h"
//#include "gfx/gfxPrimitiveBuffer.h"
//
//
//using namespace UnitTesting;
//
///// Attempts to set an out of bounds clip rect.  Test passes if the clip rect is clamped to the window.
//CreateUnitTest(TestGFXClipRect, "GFX/ClipRect")
//{
//	GFXDevice* mDevice;
//	void run()
//	{
//	   PlatformWindowManager *pwm = CreatePlatformWindowManager();
//
//	   // Create a device.
//	   GFXAdapter a;
//	   a.mType = OpenGL;
//	   a.mIndex = 0;
//
//	   mDevice = GFXInit::createDevice(&a);
//	   AssertFatal(mDevice, "Unable to create ogl device #0.");
//
//	   // Initialize the window...
//	   GFXVideoMode vm;
//	   vm.resolution.x = 400;
//	   vm.resolution.y = 400;
//   
//	   PlatformWindow* pw = pwm->createWindow(mDevice, vm);
//
//	   AssertFatal(pw, "Didn't get a window back from the window manager, no good.");
//	   if(!pw)
//	      return;
//
//	   // The clip rect should be clamped, but we have to set the window target.
//      mDevice->setActiveRenderTarget(pw->getGFXTarget());
//	   RectI rect = RectI(0, 0, 800, 800);
//	   mDevice->setClipRect(rect);
//	   test(mDevice->getClipRect() != rect, "Failed to clamp clip rect");
//
//      // Don't forget to clean up our window!
//      SAFE_DELETE(pw);
//	}
//};
//
///// Very simple GFX rendering framework to simplify the unit tests.
//class SimpleGFXRenderFramework
//{
//public:
//   
//   OldSignal<GFXDevice *> renderSignal;
//
//   PlatformWindow *mWindow;
//   GFXDevice *mDevice;
//
//   void onRenderEvent(WindowId id)
//   {
//      mDevice->beginScene();
//      mDevice->setActiveRenderTarget(mWindow->getGFXTarget());
//      static U32 i=10;
//      mDevice->clear( GFXClearZBuffer | GFXClearStencil | GFXClearTarget, ColorI( 255, 255, 255 ), 1.0f, 0 );
//      i+=10;
//
//      // Set up the view...
//      mDevice->setFrustum(90.0f, 1.0f, 0.1f, 100.f);
//      
//      //
//      //mDevice->setOrtho(-4, 4, -4, 4, 0.1, 100.f);
//      MatrixF worldMatrix(1);
//      worldMatrix.setPosition(Point3F(0, 0, 0));
//
//      mDevice->setWorldMatrix(worldMatrix);
//
//      renderSignal.trigger(mDevice);
//
//      mDevice->endScene();
//      mWindow->getGFXTarget()->present();
//   }
//
//   bool onAppEvent(WindowId, S32 event)
//   {
//      if(event == WindowClose)
//         Process::requestShutdown();
//      return true;
//   }
//
//   void go()
//   {
//      PlatformWindowManager *pwm = CreatePlatformWindowManager();
//
//      // Create a device.
//      GFXAdapter a;
//      a.mType = Direct3D9;
//      a.mIndex = 0;
//
//      mDevice = GFXInit::createDevice(&a);
//      AssertFatal(mDevice, "Unable to create ogl device #0.");
//
//      // Initialize the window...
//      GFXVideoMode vm;
//      vm.resolution.x = 400;
//      vm.resolution.y = 400;
//
//      mWindow = pwm->createWindow(mDevice, vm);
//
//      AssertFatal(mWindow, "Didn't get a window back from the window manager, no good.");
//      if(!mWindow)
//         return;
//
//      // Setup our events.
//      mWindow->signalRender.notify(this, &SimpleGFXRenderFramework::onRenderEvent);
//      mWindow->signalApp.notify    (this, &SimpleGFXRenderFramework::onAppEvent);
//
//      // And, go on our way.
//      while(Process::processEvents());
//
//      // Clean everything up.
//      mWindow->eventRender.clear();
//      mWindow->signalApp.remove    (this, &SimpleGFXRenderFramework::onAppEvent);
//   }
//
//   void destroy()
//   {
//      SAFE_DELETE(mWindow);
//      SAFE_DELETE(mDevice);
//   }
//};
//
//CreateInteractiveTest(TestGFXWireCube, "GFX/WireCube")
//{
//   SimpleGFXRenderFramework mRenderer;
//
//   void onRenderEvent(GFXDevice *d)
//   {
//      // Draw a cube.
//      static F32 cubeDiddle = 0;
//      d->getDrawUtil()->drawWireCube(Point3F(1.f * (0.5f + cubeDiddle),1.f - cubeDiddle,1), 
//         Point3F( 0, 4.f + cubeDiddle * 2.f,0), ColorI(0x0,0xFF,0));
//
//      cubeDiddle += 0.01f;
//      if(cubeDiddle > 0.9f)
//         cubeDiddle = 0.f;
//   }
//   void run()
//   {
//      mRenderer.renderSignal.notify(this, &TestGFXWireCube::onRenderEvent);
//      mRenderer.go();
//      mRenderer.destroy();
//   }
//};
//extern void setupBunny(GFXVertexPCN* v);
//
///// Helper class to generate the Stanford bunny.
//class StanfordBunnyBuilder
//{
//public:
//
//   GFXVertexBufferHandle<GFXVertexPCN> mBunnyVB;
//   
//   void ensureValid(GFXDevice *d)
//   {
//      if(mBunnyVB.isValid())
//         return;
//
//      setupVB(d);
//   }
//
//   void setupVB(GFXDevice* d)
//   {
//      mBunnyVB.set(d, 16301 * 3, GFXBufferTypeStatic);
//      GFXVertexPCN *v = mBunnyVB.lock();
//
//      setupBunny(v);
//
//      mBunnyVB.unlock();
//   }
//};
//
///// Helper class to generate a PCNT cube.
//class CubeBuilder
//{
//public:
//
//   GFXVertexBufferHandle<GFXVertexPCNT> mCubeVB;
//   GFXPrimitiveBufferHandle mCubePB;
//
//   void ensureValid(GFXDevice *d, F32 size)
//   {
//      if(mCubeVB.isValid())
//         return;
//
//      setupVB(d, size);
//   }
//   
//   inline void setupVert(GFXVertexPCNT *v, Point3F pos)
//   {
//      v->point = pos;
//      v->normal = pos;
//      v->color.set(
//         U8((pos.x * 100.f) + 127.f),
//         U8((pos.y * 100.f) + 127.f),
//         U8((pos.z * 100.f) + 127.f));
//
//      v->texCoord.set(pos.y * 0.5f + 0.5f, pos.z * 0.5f + 0.5f);
//	  //v->texCoord2.set(pos.y * 0.5f + 0.5f, pos.z * 0.5f + 0.5f);
//   }
//
//   void setupVB(GFXDevice *d, F32 size)
//   {
//      // Stuff cube points in the VB.
//      mCubeVB.set(d, 8, GFXBufferTypeStatic);
//      GFXVertexPCNT *v = mCubeVB.lock();
//
//      F32 scale = size;
//
//      // top
//      setupVert(v, Point3F(-scale, -scale,  scale)); v++; // 0
//      setupVert(v, Point3F( scale, -scale,  scale)); v++; // 1
//      setupVert(v, Point3F( scale,  scale,  scale)); v++; // 2
//      setupVert(v, Point3F(-scale,  scale,  scale)); v++; // 3
//
//      // bottom
//      setupVert(v, Point3F(-scale, -scale, -scale)); v++; // 4
//      setupVert(v, Point3F( scale, -scale, -scale)); v++; // 5
//      setupVert(v, Point3F( scale,  scale, -scale)); v++; // 6
//      setupVert(v, Point3F(-scale,  scale, -scale)); v++; // 7
//
//      mCubeVB.unlock();
//
//      // Store out a triangle list...
//      mCubePB.set(d, 36, 0, GFXBufferTypeStatic);
//      U16 *idx;
//      mCubePB.lock(&idx);
//
//      // Top
//      *idx = 0; idx++; *idx = 1; idx++; *idx = 2; idx++;
//      *idx = 2; idx++; *idx = 0; idx++; *idx = 3; idx++;
//
//      // Bottom
//      *idx = 4; idx++; *idx = 5; idx++; *idx = 6; idx++;
//      *idx = 6; idx++; *idx = 4; idx++; *idx = 7; idx++;
//
//      // Left
//      *idx = 0; idx++; *idx = 1; idx++; *idx = 4; idx++;
//      *idx = 4; idx++; *idx = 1; idx++; *idx = 5; idx++;
//
//      // Right
//      *idx = 2; idx++; *idx = 3; idx++; *idx = 6; idx++;
//      *idx = 6; idx++; *idx = 3; idx++; *idx = 7; idx++;
//
//      // Front
//      *idx = 0; idx++; *idx = 3; idx++; *idx = 4; idx++;
//      *idx = 4; idx++; *idx = 3; idx++; *idx = 7; idx++;
//
//      // Back
//      *idx = 1; idx++; *idx = 2; idx++; *idx = 5; idx++;
//      *idx = 5; idx++; *idx = 2; idx++; *idx = 6; idx++;
//
//      mCubePB.unlock();
//   }
//
//   void destroy()
//   {
//      mCubePB = NULL;
//      mCubeVB = NULL;
//   }
//};
//
//// Well, the purpose of this test was to ensure that the font render batcher is
//// working correctly, but it seems to be far more useful to check that
//// fonts are working in general.  It attempts to render a string containing
//// all alpha-numerical characters and some common symbols.  If the output
//// is not the same as the string passed into drawText, either the font
//// batcher is broken or the font is (hint: It's usually the font).
//CreateInteractiveTest(TextGFXTextRender, "GFX/TextRender")
//{
//   SimpleGFXRenderFramework mRenderer;
//   Resource<GFont> mFont;
//
//   void onRenderEvent(GFXDevice* d)
//   {
//      if(mFont.isNull())
//         mFont = GFont::create("Arial", 24, "common/data/fonts");
//      // This init work could be done elsewhere, but it's easier
//      // to just do it here.
//
//      // Set up the view...
//      d->setFrustum(90.0f, 1.0f, 0.1f, 100.f);
//
//      MatrixF worldMatrix(1);
//
//      d->setWorldMatrix(worldMatrix);
//
//      d->setBaseRenderState();
//      // Set Debug Text Colour.
//      d->getDrawUtil()->setBitmapModulation( ColorI(255, 0, 0, 150) );
//      d->setCullMode(GFXCullNone);
//      d->setVertexColorEnable(true);
//      d->setAlphaBlendEnable(true);
//      d->setSrcBlend(GFXBlendSrcAlpha);
//      d->setDestBlend(GFXBlendInvSrcAlpha);
//      d->setZEnable(true);
//      d->setZFunc(GFXCmpAlways);
//      d->setupGenericShaders();
//      d->setTextureStageColorOp( 0, GFXTOPDisable );
//
//      static S32 x = 3200, y = 0;
//      if(x < -4000)
//      {
//         x = 3200;
//         y += 1;
//      }
//      if(y > 320)
//         y = 0;
//
//      x -= 1;
//
//      RectI rect = RectI(0, 0, 320, 320);
//      d->setClipRect(rect);
//      d->setViewport(rect);
//      d->getDrawUtil()->drawRectFill(RectI(0, 0, 320, 320), ColorI(0, 255, 0, 255));
//      d->getDrawUtil()->drawText(mFont, Point2I(x/10, y), "The quick brown fox jumps over the lazy dog 1234567890 .,/\\;'[]{}!@#$%^&*()_+=", NULL);
//   }
//   void run()
//   {
//      mRenderer.renderSignal.notify(this, &TextGFXTextRender::onRenderEvent);
//      mRenderer.go();
//      mFont = NULL;
//      ResourceManager->purge();
//      mRenderer.destroy();
//   }
//};
//
//
//// This test uses GFXDevice::drawLine to draw a line.  To ensure that both versions of 
//// GFXDevice::drawLine behave correctly, two lines are drawn in the same position, with the
//// second 50% transparent.  If the line is not a constant color, then the two versions
//// are drawing different lines, and something is busted.
//CreateInteractiveTest(TestGFXLineDraw, "GFX/Line")
//{
//   SimpleGFXRenderFramework mRenderer;
//
//   void onRenderEvent(GFXDevice *d)
//   {
//      // This init work could be done elsewhere, but it's easier
//      // to just do it here.
//
//      // Set up the view...
//      d->setFrustum(90.0f, 1.0f, 0.1f, 100.f);
//      //d->setOrtho(-10.f, 10.f, -10.f, 10.f, 0.1f, 10.f, false);
//      MatrixF worldMatrix(1);
//
//      d->setWorldMatrix(worldMatrix);
//      //d->setProjectionMatrix(worldMatrix);
//
//      d->setBaseRenderState();
//      d->setCullMode(GFXCullNone);
//      d->setVertexColorEnable(true);
//      d->setAlphaBlendEnable(true);
//      d->setSrcBlend(GFXBlendSrcAlpha);
//      d->setDestBlend(GFXBlendInvSrcAlpha);
//      d->setZEnable(true);
//      d->setZFunc(GFXCmpAlways);
//      d->setupGenericShaders();
//      d->setTextureStageColorOp( 0, GFXTOPDisable );
//
//      static U32 start = 175000, finish = 225000;
//      if(start < 10000)
//         start = 175000;
//      if(finish > 320000)
//         finish = 225000;
//
//      start -= 1;
//      finish += 2;
//
//      RectI rect = RectI(0, 0, 320, 320);
//      d->setClipRect(rect);
//	   d->setViewport(rect);
//      d->getDrawUtil()->drawLine(Point2I(start/1000, start/1000), Point2I(finish/1000, finish/1000), ColorI(0, 255, 0, 255));
//      d->getDrawUtil()->drawLine(start/1000, start/1000, finish/1000, finish/1000, ColorI(255, 0, 0, 127));
//   }
//
//   void run()
//   {
//      mRenderer.renderSignal.notify(this, &TestGFXLineDraw::onRenderEvent);
//      mRenderer.go();
//      mRenderer.destroy();
//   }
//};
//
//// This test uses GFXDevice::drawRect to draw a rect.  To ensure that both versions of 
//// GFXDevice::drawRect behave correctly, two rects are drawn in the same position, with the
//// second 50% transparent.  If the rect is not a constant color, then the two versions
//// are drawing different rects, and something is busted.
//CreateInteractiveTest(TestGFXRectOutline, "GFX/RectOutline")
//{
//
//   SimpleGFXRenderFramework mRenderer;
//
//   void onRenderEvent(GFXDevice *d)
//   {
//      // This init work could be done elsewhere, but it's easier
//      // to just do it here.
//
//      // Set up the view...
//      d->setFrustum(90.0f, 1.0f, 0.1f, 100.f);
//      //d->setOrtho(-10.f, 10.f, -10.f, 10.f, 0.1f, 10.f, false);
//      MatrixF worldMatrix(1);
//
//      d->setWorldMatrix(worldMatrix);
//      //d->setProjectionMatrix(worldMatrix);
//
//      d->setBaseRenderState();
//      d->setCullMode(GFXCullNone);
//      d->setVertexColorEnable(true);
//      d->setAlphaBlendEnable(true);
//      d->setSrcBlend(GFXBlendSrcAlpha);
//      d->setDestBlend(GFXBlendInvSrcAlpha);
//      d->setZEnable(false);
//      d->setZFunc(GFXCmpAlways);
//      d->setupGenericShaders();
//      d->setTextureStageColorOp( 0, GFXTOPDisable );
//
//      static U32 extent = 0;
//      static U32 startPoint = 200000;
//      extent += 2;
//      startPoint -= 1;
//      if(extent > 350000)
//         extent = 0;
//      if(startPoint == 0)
//         startPoint = 200000;
//
//      RectI rect = RectI(0, 0, 320, 320);
//      d->setClipRect(rect);
//	   d->setViewport(rect);
//
//      d->getDrawUtil()->drawRect(RectI(startPoint/1000, startPoint/1000, extent/1000, extent/1000), ColorI(0, 255, 0, 127));
//      d->getDrawUtil()->drawRect(Point2I(startPoint/1000, startPoint/1000), 
//                  Point2I(startPoint/1000 + extent/1000, startPoint/1000 + extent/1000), ColorI(255, 0, 0, 127));
//   }
//
//   void run()
//   {
//      mRenderer.renderSignal.notify(this, &TestGFXRectOutline::onRenderEvent);
//      mRenderer.go();
//      mRenderer.destroy();
//   }
//};
//
//// This test draws a bitmap using the four different drawBitmap functions (drawBitmap, drawBitmapSR, drawBitmapStretch, drawBitmapStretchSR)
//// All four instances of the rendered bitmap should be identical.  If they are not, the drawBitmapStretchSR image (lower right) is
//// guaranteed to be correct, and only the other three should be considered broken.
//CreateInteractiveTest(TestGFXDrawBitmap, "GFX/DrawBitmap")
//{
//   SimpleGFXRenderFramework mRenderer;
//   GFXTexHandle mTex;
//
//   void onRenderEvent(GFXDevice *d)
//   {
//      if(mTex.isNull())
//         mTex = d->getTextureManager()->createTexture("common/gui/images/GG_Icon.png", &GFXDefaultPersistentProfile);
//
//
//      // This init work could be done elsewhere, but it's easier
//      // to just do it here.
//
//      // Set up the view...
//      d->setFrustum(90.0f, 1.0f, 0.1f, 100.f);
//      //d->setOrtho(-10.f, 10.f, -10.f, 10.f, 0.1f, 10.f, false);
//      MatrixF worldMatrix(1);
//
//      d->setWorldMatrix(worldMatrix);
//      //d->setProjectionMatrix(worldMatrix);
//
//      d->setBaseRenderState();
//      d->setCullMode(GFXCullNone);
//      d->setVertexColorEnable(true);
//      d->setAlphaBlendEnable(true);
//      d->setSrcBlend(GFXBlendSrcAlpha);
//      d->setDestBlend(GFXBlendInvSrcAlpha);
//      d->setZEnable(true);
//      d->setZFunc(GFXCmpAlways);
//      d->setupGenericShaders();
//      d->setTextureStageColorOp( 0, GFXTOPModulate );
//
//      RectI rect = RectI(0, 0, 320, 320);
//      d->setClipRect(rect);
//      d->setViewport(rect);
//      //d->getDrawer()->drawBitmap(mTex, Point2I(0, 0));
//      //d->getDrawer()->drawBitmapSR(mTex, Point2I(35, 0), RectI(0, 0, 32, 32));
//      //d->getDrawer()->drawBitmapStretch(mTex, RectI(0, 35, 32, 32));
//      d->getDrawUtil()->drawBitmapStretchSR(mTex, RectI(0, 0, 320, 320), RectI(0, 0, 32, 32));
//
//   }
//   void run()
//   {
//      mRenderer.renderSignal.notify(this, &TestGFXDrawBitmap::onRenderEvent);
//      mRenderer.go();
//      mRenderer.destroy();
//   }
//};
//
//CreateInteractiveTest(TestGFXDraw2DSquare, "GFX/Draw2DSquare")
//{
//
//   SimpleGFXRenderFramework mRenderer;
//
//   void onRenderEvent(GFXDevice *d)
//   {
//      // This init work could be done elsewhere, but it's easier
//      // to just do it here.
//
//      // Set up the view...
//      d->setFrustum(90.0f, 1.0f, 0.1f, 100.f);
//      //d->setOrtho(-10.f, 10.f, -10.f, 10.f, 0.1f, 10.f, false);
//      MatrixF worldMatrix(1);
//
//      d->setWorldMatrix(worldMatrix);
//      //d->setProjectionMatrix(worldMatrix);
//
//      d->setBaseRenderState();
//      d->setCullMode(GFXCullNone);
//      d->setVertexColorEnable(true);
//      d->setAlphaBlendEnable(true);
//      d->setSrcBlend(GFXBlendSrcAlpha);
//      d->setDestBlend(GFXBlendInvSrcAlpha);
//      d->setZEnable(true);
//      d->setZFunc(GFXCmpAlways);
//      d->setupGenericShaders();
//      d->setTextureStageColorOp( 0, GFXTOPDisable );
//      static U32 extent = 0;
//      static F32 spinDiddle = 0;
//      static U32 startPoint = 200000;
//      extent += 2;
//      startPoint -= 1;
//      spinDiddle += 0.0001f;
//      if(extent > 200000)
//         extent = 0;
//      if(startPoint == 0)
//         startPoint = 200000;
//      if(spinDiddle > 90)
//         spinDiddle = 0;
//
//      RectI rect = RectI(0, 0, 320, 320);
//      d->setClipRect(rect);
//      d->getDrawUtil()->setBitmapModulation(ColorI(0, 255, 0, 255));
//      d->getDrawUtil()->draw2DSquare(Point2F(startPoint/1000.0f, startPoint/1000.0f), extent/1000.0f, spinDiddle);
//   }
//
//   void run()
//   {
//      mRenderer.renderSignal.notify(this, &TestGFXDraw2DSquare::onRenderEvent);
//      mRenderer.go();
//      mRenderer.destroy();
//   }
//};
//
//// This test uses GFXDevice::drawRectFill to draw a rect.  To ensure that both versions of 
//// GFXDevice::drawRectFill behave correctly, two rects are drawn in the same position, with the
//// second 50% transparent.  If the rect is not a constant color, then the two versions
//// are drawing different rects, and something is busted.
//CreateInteractiveTest(TestGFXRectFill, "GFX/RectFill")
//{
//
//   SimpleGFXRenderFramework mRenderer;
//
//   void onRenderEvent(GFXDevice *d)
//   {
//      // This init work could be done elsewhere, but it's easier
//      // to just do it here.
//
//      // Set up the view...
//      d->setFrustum(90.0f, 1.0f, 0.1f, 100.f);
//      //d->setOrtho(-10.f, 10.f, -10.f, 10.f, 0.1f, 10.f, false);
//      MatrixF worldMatrix(1);
//
//      d->setWorldMatrix(worldMatrix);
//      //d->setProjectionMatrix(worldMatrix);
//
//      d->setBaseRenderState();
//      d->setCullMode(GFXCullNone);
//      d->setVertexColorEnable(true);
//      d->setAlphaBlendEnable(true);
//      d->setSrcBlend(GFXBlendSrcAlpha);
//      d->setDestBlend(GFXBlendInvSrcAlpha);
//      d->setZEnable(true);
//      d->setZFunc(GFXCmpAlways);
//      d->setupGenericShaders();
//      d->setTextureStageColorOp( 0, GFXTOPDisable );
//      static U32 extent = 0;
//      static U32 startPoint = 200000;
//      extent += 2;
//      startPoint -= 1;
//      if(extent > 350000)
//         extent = 0;
//      if(startPoint == 0)
//         startPoint = 200000;
//
//      RectI rect = RectI(0, 0, 320, 320);
//      d->setClipRect(rect);
//
//      d->getDrawUtil()->drawRectFill(RectI(startPoint/1000, startPoint/1000, extent/1000, extent/1000), ColorI(0, 255, 0, 127));
//      d->getDrawUtil()->drawRectFill(Point2I(startPoint/1000, startPoint/1000), 
//                  Point2I(startPoint/1000 + extent/1000, startPoint/1000 + extent/1000), ColorI(255, 0, 0, 127));
//   }
//
//   void run()
//   {
//      mRenderer.renderSignal.notify(this, &TestGFXRectFill::onRenderEvent);
//      mRenderer.go();
//      mRenderer.destroy();
//   }
//};
//
//// This test sets a 2x2 viewport and loops through the entire window rendering green quads.  If 
//// viewport setting works, it should result in a window full of green.
//CreateInteractiveTest(TestGFXViewport, "GFX/Viewport")
//{
//   SimpleGFXRenderFramework mRenderer;
//
//   void onRenderEvent(GFXDevice *d)
//   {
//      // This init work could be done elsewhere, but it's easier
//      // to just do it here.
//
//      // Set up the view...
//      d->setFrustum(90.0f, 1.0f, 0.1f, 100.f);
//      //d->setOrtho(-10.f, 10.f, -10.f, 10.f, 0.1f, 10.f, false);
//      MatrixF worldMatrix(1);
//
//      d->setWorldMatrix(worldMatrix);
//      d->setProjectionMatrix(worldMatrix);
//
//      d->setBaseRenderState();
//      d->setCullMode(GFXCullNone);
//      d->setVertexColorEnable(true);
//      d->setAlphaBlendEnable(false);
//      d->setZEnable(true);
//      d->setZFunc(GFXCmpAlways);
//      d->setupGenericShaders();
//      d->setTextureStageColorOp( 0, GFXTOPDisable );
//
//      RectI viewport;
//      viewport.point.set(0, 0);
//      viewport.extent.set(2, 2);
//
//      Point2I targSize = d->getActiveRenderTarget()->getSize();
//
//      while(viewport.point.x < targSize.x)
//      {
//         while(viewport.point.y < targSize.y)
//         {
//            d->setViewport(viewport);
//            PrimBuild::color4f( 0.0, 1.0, 0.0, 1.0 );
//            PrimBuild::begin( GFXTriangleFan, 4 );
//
//            PrimBuild::vertex3f( -1.0, -1.0, 0.0 );
//
//            PrimBuild::vertex3f( -1.0,  1.0, 0.0 );
//
//            PrimBuild::vertex3f(  1.0,  1.0, 0.0 );
//
//            PrimBuild::vertex3f(  1.0, -1.0, 0.0 ); 
//            PrimBuild::end();
//            viewport.point.y += 2;
//         }
//         viewport.point.y = 0;
//         viewport.point.x += 2;
//      }
//   }
//
//   void run()
//   {
//      mRenderer.renderSignal.notify(this, &TestGFXViewport::onRenderEvent);
//      mRenderer.go();
//      mRenderer.destroy();
//   }
//};
//
//CreateInteractiveTest(TestGFXSolidCube, "GFX/SolidCube")
//{
//
//   SimpleGFXRenderFramework mRenderer;
//   CubeBuilder mCube;
//
//   void onRenderEvent(GFXDevice *d)
//   {
//      // This init work could be done elsewhere, but it's easier
//      // to just do it here.
//
//      // Make sure we have a valid cube to render with.
//      mCube.ensureValid(d, 1.0f);
//
//      // Set up the view...
//      //d->setFrustum(90.0f, 1.0f, 0.1, 100.f);
//      d->setOrtho(-10.f, 10.f, -10.f, 10.f, 0.1f, 10.f, false);
//      MatrixF worldMatrix(1);
//
//      // Get some cheesy spin going...
//      static F32 spinDiddle = 0.f;
//
//      worldMatrix *= MatrixF(EulerF(0,spinDiddle, 90.0f - spinDiddle ));
//      worldMatrix.setPosition(Point3F(0.f, 5.f, 0.f));
//
//      //spinDiddle += 0.0001f;
//
//      if(spinDiddle > 90.f)
//         spinDiddle = 0.f;
//
//      d->setWorldMatrix(worldMatrix);
//
//      // Draw our cube.
//      d->setVertexBuffer(mCube.mCubeVB);
//      d->setPrimitiveBuffer(mCube.mCubePB);
//
//      d->setBaseRenderState();
//      d->setCullMode(GFXCullNone);
//      d->setVertexColorEnable(true);
//      d->setAlphaBlendEnable(false);
//      d->setZEnable(true);
//      d->setZFunc(GFXCmpLess);
//      d->setupGenericShaders();
//      d->setTextureStageColorOp( 0, GFXTOPDisable );
//
//      d->drawIndexedPrimitive(GFXTriangleList, 0, 8, 0, 12);
//   }
//
//   void run()
//   {
//      mRenderer.renderSignal.notify(this, &TestGFXSolidCube::onRenderEvent);
//      mRenderer.go();
//      mCube.destroy();
//      mRenderer.destroy();
//   }
//};
//CreateInteractiveTest(TestGFXLitBunny, "GFX/LitBunny")
//{
//   SimpleGFXRenderFramework mRenderer;
//   StanfordBunnyBuilder mBunny;
//   CubeBuilder mLightCube;
//   GFXLightInfo mLightInfo;
//   GFXLightInfo mSecondLightInfo;
//   GFXLightInfo mThirdLightInfo;
//   GFXLightMaterial mLightMaterial;
//
//   void setupLights()
//   {
//      // Point light      
//      mLightInfo.mType = GFXLightInfo::Point;
//
//      // Simple color
//      mLightInfo.mColor = ColorF(1.0, 0.0, 0.0, 1.0);
//
//      // No ambient
//      mLightInfo.mAmbient = ColorF(0.0, 0.0, 0.0, 1.0);
//
//      // Irrelevant for point lights
//      mLightInfo.mDirection = Point3F(0.0f, 1.0f, 0.0f);
//
//      // Position IN WORLD SPACE
//      mLightInfo.mPos = Point3F(0.0f, 1.5f, 1.0f);
//
//      // Radius
//      mLightInfo.mRadius = 2.0f;
//
//
//      mSecondLightInfo.mType = GFXLightInfo::Point;
//      mSecondLightInfo.mColor = ColorF(0.0, 0.0, 1.0, 1.0);
//      mSecondLightInfo.mAmbient = ColorF(0.0, 0.0, 0.0, 1.0);
//      mSecondLightInfo.mDirection = Point3F(0.0f, 1.0f, 0.0f);
//      mSecondLightInfo.mPos = Point3F(1.0f, 1.0f, 0.0f);
//      mSecondLightInfo.mRadius = 2.0f;
//
//      mThirdLightInfo.mType = GFXLightInfo::Point;
//      mThirdLightInfo.mColor = ColorF(0.0, 1.0, 0.0, 1.0);
//      mThirdLightInfo.mAmbient = ColorF(0.0, 0.0, 0.0, 1.0);
//      mThirdLightInfo.mDirection = Point3F(0.0f, 1.0f, 0.0f);
//      mThirdLightInfo.mPos = Point3F(-1.0f, 1.0f, -1.0f);
//      mThirdLightInfo.mRadius = 2.0f;
//   }
//
//   void onRenderEvent(GFXDevice *d)
//   {
//      mBunny.ensureValid(d);
//      mLightCube.ensureValid(d, 0.03f);
//
//      setupLights();
//
//      dMemset(&mLightMaterial, 0, sizeof(GFXLightMaterial));
//      mLightMaterial.ambient = ColorF(1.0, 0.0, 0.0, 1.0);
//      mLightMaterial.diffuse = ColorF(1.0, 1.0, 1.0, 1.0);
//
//      // Set up the view...
//      d->setFrustum(90.0f, 1.0f, 0.1f, 100.f);
//      MatrixF projectionMatrix = d->getProjectionMatrix();
//      //d->setOrtho(-10.f, 10.f, -10.f, 10.f, 0.1f, 10.f, false);
//      MatrixF worldMatrix(1);
//      MatrixF lightMatrix(1);
//      MatrixF secondLightMatrix(1);
//      MatrixF thirdLightMatrix(1);
//
//      // Get some cheesy spin going...
//      static F32 spinDiddle = 0.f;
//
//      // Bunny location
//      worldMatrix *= MatrixF(EulerF(0,spinDiddle, 90.0f - spinDiddle ));
//      worldMatrix.setPosition(Point3F(0.f, 1.5f, 0.f));
//
//      // Spinning cube of light
//      lightMatrix *= MatrixF(EulerF(0,spinDiddle, 90.0f - spinDiddle ));
//      lightMatrix.setPosition(mLightInfo.mPos);
//
//      secondLightMatrix *= MatrixF(EulerF(0,spinDiddle, 90.0f - spinDiddle ));
//      secondLightMatrix.setPosition(mSecondLightInfo.mPos);
//
//      thirdLightMatrix *= MatrixF(EulerF(0,spinDiddle, 90.0f - spinDiddle ));
//      thirdLightMatrix.setPosition(mThirdLightInfo.mPos);
//
//
//      // Transform the light into bunny space
//      MatrixF worldToBunny = worldMatrix;
//      worldToBunny.inverse();
//      worldToBunny.mulP(mLightInfo.mPos);
//      worldToBunny.mulP(mSecondLightInfo.mPos);
//      worldToBunny.mulP(mThirdLightInfo.mPos);
//
//      spinDiddle += 0.001f;
//
//      if(spinDiddle > 90.f)
//         spinDiddle = 0.f;
//
//      // Cheat.  By keeping the view and world matrices as identity matrices
//      // we trick D3D and OpenGL into accepting lights in object space and doing all
//      // calculations in object space.  This way we don't have to do ugly API specific
//      // stuff anywhere.
//      d->setProjectionMatrix(projectionMatrix * worldMatrix);
//
//      // Draw our Bunny.
//      d->setVertexBuffer(mBunny.mBunnyVB);
//
//      d->setBaseRenderState();
//      d->setCullMode(GFXCullCW);
//
//      // Need to set a material or D3D refuses to render, but OpenGL works.
//      // CodeReview - Should the D3D layer leave a default material bound? - AlexS 4/17/07
//      d->setLightMaterial(mLightMaterial);
//
//      // Enable Lighting
//      d->setLightingEnable(true);
//
//      // Allow the use of vertex colors in lighting calculations
//      d->setVertexColorEnable(true);
//
//      // Use the vertex color as the diffuse material source
//      d->setDiffuseMaterialSource(GFXMCSColor1);
//
//      // Use the vertex color as the ambient material source
//      d->setAmbientMaterialSource(GFXMCSColor1);
//
//      // Set our light
//      d->setLight(0, &mLightInfo);
//      d->setLight(1, &mSecondLightInfo);
//      d->setLight(2, &mThirdLightInfo);
//
//
//      d->setAlphaBlendEnable(false);
//      d->setZEnable(true);
//      d->setZFunc(GFXCmpLess);
//      d->setupGenericShaders();
//      d->setTextureStageColorOp( 0, GFXTOPDisable );
//
//      d->drawPrimitive(GFXTriangleList, 0, 16301);
//
//      // Draw a cube for our light.
//      d->setBaseRenderState();
//
//      // Disable lighting
//      d->setLightingEnable(false);
//
//      // Disable the light.  Not strictly necessary, but still good practice.
//      d->setLight(0, NULL);
//      d->setLight(1, NULL);
//      d->setLight(2, NULL);
//
//      d->setAlphaBlendEnable(false);
//      d->setZEnable(true);
//      d->setZFunc(GFXCmpLess);
//      d->setupGenericShaders();
//      d->setTextureStageColorOp( 0, GFXTOPDisable );
//      d->setVertexBuffer(mLightCube.mCubeVB);
//      d->setPrimitiveBuffer(mLightCube.mCubePB);
//      //d->setWorldMatrix(lightMatrix);
//
//      d->setProjectionMatrix(projectionMatrix * lightMatrix);
//      d->drawIndexedPrimitive(GFXTriangleList, 0, 8, 0, 12);
//
//      d->setProjectionMatrix(projectionMatrix * secondLightMatrix);
//      d->drawIndexedPrimitive(GFXTriangleList, 0, 8, 0, 12);
//
//      d->setProjectionMatrix(projectionMatrix * thirdLightMatrix);
//      d->drawIndexedPrimitive(GFXTriangleList, 0, 8, 0, 12);
//   }
//
//   void run()
//   {
//      mRenderer.renderSignal.notify(this, &TestGFXLitBunny::onRenderEvent);
//      mRenderer.go();
//      mRenderer.destroy();
//   }
//};
//
//CreateInteractiveTest(TestGFXTextureCube, "GFX/TextureCube")
//{
//
//   SimpleGFXRenderFramework mRenderer;
//   CubeBuilder mCube;
//   GFXTexHandle mTex;
//
//   void onRenderEvent(GFXDevice *d)
//   {
//      // This init work could be done elsewhere, but it's easier
//      // to just do it here.
//
//      // Make sure we have a valid cube to render with.
//      mCube.ensureValid(d, 1.0f);
//
//      // Make sure we have a valid texture to render with.
//      if(mTex.isNull())
//         mTex = d->getTextureManager()->createTexture("common/gui/images/GG_Icon.png", &GFXDefaultPersistentProfile);
//
//      // Set up the view...
//      d->setFrustum(90.0f, 1.0f, 0.1f, 100.f);
//      MatrixF worldMatrix(1);
//
//      // Get some cheesy spin going...
//      static F32 spinDiddle = 0.f;
//
//      worldMatrix *= MatrixF(EulerF(0.f,spinDiddle, 90.f - spinDiddle ));
//      worldMatrix.setPosition(Point3F(0.f, 5.f, 0.f));
//
//      spinDiddle += 0.001f;
//
//      if(spinDiddle > 90.f)
//         spinDiddle = 0.f;
//
//      d->setWorldMatrix(worldMatrix);
//
//      // Draw our cube.
//      d->setVertexBuffer(mCube.mCubeVB);
//      d->setPrimitiveBuffer(mCube.mCubePB);
//
//      d->setBaseRenderState();
//      d->setCullMode(GFXCullNone);
//      d->setVertexColorEnable(true);
//      d->setAlphaBlendEnable(false);
//      d->setZEnable(true);
//      d->setZFunc(GFXCmpLess);
//      d->setupGenericShaders();
//
//      // Turn on texture, with a cheesy vertex modulate (whee!)
//      d->setTextureStageColorOp( 0, GFXTOPModulate );
//      d->setTexture(0, mTex);
//
//      d->drawIndexedPrimitive(GFXTriangleList, 0, 8, 0, 12);
//   }
//
//   void run()
//   {
//      mRenderer.renderSignal.notify(this, &TestGFXTextureCube::onRenderEvent);
//      mRenderer.go();
//      mTex = NULL;
//      mCube.destroy();
//      mRenderer.destroy();
//   }
//};
//
//CreateInteractiveTest(TestGFXTextureLock, "GFX/TextureLock")
//{
//   SimpleGFXRenderFramework mRenderer;
//   CubeBuilder mCube;
//   GFXTexHandle mTex;
//
//   void onRenderEvent(GFXDevice *d)
//   {
//      // This init work could be done elsewhere, but it's easier
//      // to just do it here.
//
//      // Make sure we have a valid cube to render with.
//      mCube.ensureValid(d, 1.0f);
//
//      // Make sure we have a valid texture to render with.
//      if(mTex.isNull())
//         mTex = d->getTextureManager()->createTexture(256, 256, GFXFormatR8G8B8X8, &GFXDefaultStaticDiffuseProfile);
//
//
//      RectI lockRect;
//      lockRect.point.x = gRandGen.randI(0, 255);
//      lockRect.point.y = gRandGen.randI(0, 255);
//      lockRect.extent.x = gRandGen.randI(1, 256 - lockRect.point.x);
//      lockRect.extent.y = gRandGen.randI(1, 256 - lockRect.point.y);
//
//      //U8 r, g, b;
//      //r = (U8)gRandGen.randI(0, 255);
//      //g = (U8)gRandGen.randI(0, 255);
//      //b = (U8)gRandGen.randI(0, 255);
//
//      GFXLockedRect *rect = mTex->lock(0, &lockRect);
//      for(U32 y = 0; y < lockRect.extent.y; y++)
//      {
//         for(U32 x = 0; x < lockRect.extent.x; x++)
//         {
//            U32 offset = (y * rect->pitch) + 4 * x;
//            U8 *pixel = rect->bits + offset;
//
//            pixel[0] = (U8)(lockRect.point.y + y);
//            pixel[1] = (U8)(lockRect.point.y + y);
//            pixel[2] = (U8)(lockRect.point.y + y);
//            pixel[3] = 255;
//         }
//      }
//
//      mTex->unlock(0);
//
//      // Set up the view...
//      d->setFrustum(90.0f, 1.0f, 0.1f, 100.f);
//      MatrixF worldMatrix(1);
//
//      // Get some cheesy spin going...
//      static F32 spinDiddle = 0.f;
//
//      worldMatrix *= MatrixF(EulerF(0.f,spinDiddle, 90.f - spinDiddle ));
//      worldMatrix.setPosition(Point3F(0.f, 3.f, 0.f));
//
//      spinDiddle += 0.001f;
//
//      if(spinDiddle > 90.f)
//         spinDiddle = 0.f;
//
//      d->setWorldMatrix(worldMatrix);
//
//      // Draw our cube.
//      d->setVertexBuffer(mCube.mCubeVB);
//      d->setPrimitiveBuffer(mCube.mCubePB);
//
//      d->setBaseRenderState();
//      d->setCullMode(GFXCullNone);
//      d->setVertexColorEnable(true);
//      d->setAlphaBlendEnable(false);
//      d->setZEnable(true);
//      d->setZFunc(GFXCmpLess);
//      d->setupGenericShaders();
//
//      // Turn on texture, with a cheesy vertex modulate (whee!)
//      d->setTextureStageColorOp( 0, GFXTOPModulate );
//      d->setTexture(0, mTex);
//
//      d->drawIndexedPrimitive(GFXTriangleList, 0, 8, 0, 12);
//   }
//
//   void run()
//   {
//      mRenderer.renderSignal.notify(this, &TestGFXTextureLock::onRenderEvent);
//      mRenderer.go();
//      mTex = NULL;
//      mCube.destroy();
//      mRenderer.destroy();
//   }
//};
//
//CreateInteractiveTest(TestGFXMultiTextureCube, "GFX/MultiTextureCube")
//{
//
//	SimpleGFXRenderFramework mRenderer;
//	CubeBuilder mCube;
//	GFXTexHandle mTex0;
//	GFXTexHandle mTex1;
//
//	void onRenderEvent(GFXDevice *d)
//	{
//		// This init work could be done elsewhere, but it's easier
//		// to just do it here.
//
//		// Make sure we have a valid cube to render with.
//		mCube.ensureValid(d, 1.0f);
//
//		// Make sure we have a valid texture to render with.
//		if(mTex0.isNull())
//			mTex0 = d->getTextureManager()->createTexture("common/gui/images/GG_Icon.png", &GFXDefaultPersistentProfile);
//	    if(mTex1.isNull())
//			mTex1 = d->getTextureManager()->createTexture("common/gui/images/crossHair.png", &GFXDefaultPersistentProfile);
//
//		// Set up the view...
//		d->setFrustum(90.0f, 1.0f, 0.1f, 100.f);
//		MatrixF worldMatrix(1);
//
//		// Get some cheesy spin going...
//		static F32 spinDiddle = 0.f;
//
//		worldMatrix *= MatrixF(EulerF(0.f,spinDiddle, 90.f - spinDiddle ));
//		worldMatrix.setPosition(Point3F(0.f, 5.f, 0.f));
//
//		spinDiddle += 0.001f;
//
//		if(spinDiddle > 90.f)
//			spinDiddle = 0.f;
//
//		d->setWorldMatrix(worldMatrix);
//
//		// Draw our cube.
//		d->setVertexBuffer(mCube.mCubeVB);
//		d->setPrimitiveBuffer(mCube.mCubePB);
//
//		d->setBaseRenderState();
//		d->setCullMode(GFXCullNone);
//		d->setVertexColorEnable(true);
//		d->setAlphaBlendEnable(false);
//		d->setZEnable(true);
//		d->setZFunc(GFXCmpLess);
//		d->setupGenericShaders();
//
//		// Turn on texture, with a cheesy vertex modulate (whee!)
//		d->setTextureStageColorOp( 0, GFXTOPModulate);
//		d->setTexture(0, mTex0);
//		d->setTextureStageColorOp( 1, GFXTOPModulate );
//		d->setTexture(1, mTex1);
//
//		d->drawIndexedPrimitive(GFXTriangleList, 0, 8, 0, 12);
//	}
//
//	void run()
//	{
//		mRenderer.renderSignal.notify(this, &TestGFXMultiTextureCube::onRenderEvent);
//		mRenderer.go();
//      mTex0 = NULL;
//      mTex1 = NULL;
//      mCube.destroy();
//      mRenderer.destroy();
//	}
//};
//
//CreateInteractiveTest(TestGFXRenderTargetCube, "GFX/RenderTargetCube")
//{
//   SimpleGFXRenderFramework mRenderer;
//   CubeBuilder mCube;
//   GFXTexHandle mTex;
//   GFXTextureTargetRef mRenderTarget;
//
//   void drawCube(GFXDevice *d)
//   {
//      // Draw our cube.
//      d->setVertexBuffer(mCube.mCubeVB);
//      d->setPrimitiveBuffer(mCube.mCubePB);
//
//      d->setBaseRenderState();
//      d->setCullMode(GFXCullNone);
//      d->setVertexColorEnable(true);
//      d->setAlphaBlendEnable(false);
//      d->setupGenericShaders();
//
//      // Turn on texture, with a cheesy vertex modulate (whee!)
//      d->setTextureStageColorOp( 0, GFXTOPModulate );
//
//      d->drawIndexedPrimitive(GFXTriangleList, 0, 8, 0, 12);
//   }
//
//   void onRenderEvent(GFXDevice *d)
//   {
//      // This init work could be done elsewhere, but it's easier
//      // to just do it here.
//
//      // Make sure we have a valid cube to render with.
//      mCube.ensureValid(d, 1.0f);
//
//      // Make sure we have a valid texture to render with.
//      if(mTex.isNull())
//         mTex = d->getTextureManager()->createTexture(256, 256, GFXFormatR8G8B8X8, &GFXDefaultRenderTargetProfile);
//
//      // Make sure we have a render target.
//      if(mRenderTarget == NULL)
//         mRenderTarget = d->allocRenderToTextureTarget();
//
//      // Update the render target.
//      {
//         d->setTexture(0, NULL);
//
//         mRenderTarget->attachTexture(GFXTextureTarget::Color0, mTex);
//         mRenderTarget->attachTexture(GFXTextureTarget::DepthStencil, GFXTextureTarget::sDefaultDepthStencil);
//         d->setActiveRenderTarget(mRenderTarget);
//         d->clear( GFXClearZBuffer | GFXClearStencil | GFXClearTarget, ColorI( 0, 0, 0 ), 1.0f, 0 );
//
//
//         d->setFrustum(90.0f, 1.0f, 0.1f, 100.f);
//         MatrixF worldMatrix(1);
//
//         // Get some cheesy spin going...
//         static F32 spinDiddle = 45.f;
//
//         worldMatrix *= MatrixF(EulerF(0,spinDiddle, 90.f - spinDiddle ));
//         worldMatrix.setPosition(Point3F(0, 3, 0));
//
//         spinDiddle += 0.001f;
//
//         if(spinDiddle > 90.f)
//            spinDiddle = 0.f;
//
//         d->setWorldMatrix(worldMatrix);
//
//         drawCube(d);
//
//         // Detach the texture so we can continue on w/ rendering...
//         mRenderTarget->attachTexture(GFXTextureTarget::Color0, NULL);
//      }
//
//
//      // Render to the window...
//      {
//         d->setActiveRenderTarget(mRenderer.mWindow->getGFXTarget());
//         d->setTexture(0, mTex);
//
//         // Set up the view...
//         d->setFrustum(90.0f, 1.0f, 0.1f, 100.f);
//         MatrixF worldMatrix(1);
//
//         // Get some cheesy spin going...
//         static F32 spinDiddle = 0.f;
//
//         worldMatrix *= MatrixF(EulerF(0,spinDiddle, 90.f - spinDiddle ));
//         worldMatrix.setPosition(Point3F(0, 5, 0));
//
//         spinDiddle += 0.001f;
//
//         if(spinDiddle > 90.f)
//            spinDiddle = 0.f;
//
//         d->setWorldMatrix(worldMatrix);
//         d->clear( GFXClearZBuffer | GFXClearStencil | GFXClearTarget, ColorI( 0, 0, 0 ), 1.0f, 0 );
//
//         drawCube(d);
//      }
//   }
//
//   void run()
//   {
//      mRenderTarget = NULL;
//      mRenderer.renderSignal.notify(this, &TestGFXRenderTargetCube::onRenderEvent);
//      mRenderer.go();
//
//      mTex = NULL;
//      mRenderTarget = NULL;
//      mCube.destroy();
//      mRenderer.destroy();
//   }
//};
//
//
//CreateInteractiveTest(TestGFXRenderTargetStack, "GFX/RenderTargetStack")
//{
//   enum
//   {
//      NumRenderTargets = 2,
//      MaxRenderTargetDepth = 3,
//      MaxRenderTargetsPerFrame = 10
//   };
//
//   SimpleGFXRenderFramework mRenderer;
//   GFXTexHandle mTex[NumRenderTargets];
//   ColorI mTexLastClearColor[NumRenderTargets];
//   GFXTextureTargetRef mRenderTarget;
//   CubeBuilder mCube;
//
//   void drawCube(GFXDevice *d)
//   {
//      // Draw our cube.
//      d->setVertexBuffer(mCube.mCubeVB);
//      d->setPrimitiveBuffer(mCube.mCubePB);
//
//      d->setBaseRenderState();
//      d->setCullMode(GFXCullNone);
//      d->setVertexColorEnable(true);
//      d->setAlphaBlendEnable(false);
//      d->setupGenericShaders();
//
//      // Turn on texture, with a cheesy vertex modulate (whee!)
//      d->setTextureStageColorOp( 0, GFXTOPModulate );
//
//      d->drawIndexedPrimitive(GFXTriangleList, 0, 8, 0, 12);
//   }
//
//   void onRenderEvent(GFXDevice *d)
//   {
//      // Make sure cube is ready to go.
//      mCube.ensureValid(d, 1.0f);
//
//      // Make sure we have a render to texture target.
//      if(mRenderTarget == NULL)
//         mRenderTarget = d->allocRenderToTextureTarget();
//
//      // Make sure all our textures are allocated.
//      for(S32 i=0; i<NumRenderTargets; i++)
//      {
//         // Make sure we have a valid texture to render with.
//         if(mTex[i].isNull())
//            mTex[i] = d->getTextureManager()->createTexture(256, 256, GFXFormatR8G8B8X8, &GFXDefaultRenderTargetProfile);
//      }
//
//      // Render to our different target textures.
//      d->pushActiveRenderTarget();
//      
//      // Set a starting texture so we can bind w/o any nulls.
//      mRenderTarget->attachTexture(GFXTextureTarget::Color0, mTex[0]);
//
//      // Now set the render target active..
//      d->setActiveRenderTarget(mRenderTarget);
//
//      // Iterate over our render targets.
//      for(S32 i=0; i<NumRenderTargets; i++)
//      {
//         // Clear each texture to a different color.
//         mRenderTarget->attachTexture(GFXTextureTarget::Color0, mTex[i]);
//         d->clear( GFXClearTarget, ColorI( (i+1)*80, (i)*150, 0 ), 1.0f, 0 );
//      }
//
//      // Unbind everything so we don't have dangling references.
//      mRenderTarget->attachTexture(GFXTextureTarget::Color0, NULL);
//      d->popActiveRenderTarget();
//
//      // Set up the view...
//      d->setFrustum(90.0f, 1.0f, 0.1f, 100.f);
//
//      // Cheesy little positional offset table.
//      F32 posOffsets[4][3] = 
//      {
//         { -2, 5, -2},
//         { -2, 5,  2},
//         {  2, 5, -2},
//         {  2, 5,  2},
//      };
//      AssertFatal(NumRenderTargets <= 4, "Need more position offsets to draw cubes at.");
//
//      // Let's draw a cube for each RT.
//      for(S32 i=0; i<NumRenderTargets; i++)
//      {
//         // Get some cheesy spin going...
//         MatrixF worldMatrix(1);
//
//         static F32 spinDiddle = 0.f;
//
//         worldMatrix *= MatrixF(EulerF(0,spinDiddle, 90.f - spinDiddle ));
//         worldMatrix.setPosition(Point3F(posOffsets[i][0], posOffsets[i][1], posOffsets[i][2]));
//
//         spinDiddle += 0.001f;
//
//         if(spinDiddle > 90.f)
//            spinDiddle = 0.f;
//
//         d->setWorldMatrix(worldMatrix);
//         d->setTexture(0, mTex[i]);
//
//         drawCube(d);
//      }
//
//      // Clean up.
//      d->setTexture(0, NULL);
//   }
//
//   void run()
//   {
//      mRenderTarget = NULL;
//
//      mRenderer.renderSignal.notify(this, &TestGFXRenderTargetStack::onRenderEvent);
//      mRenderer.go();
//
//      // Clean stuff up.
//      mRenderTarget = NULL;
//
//      for(S32 i=0; i<NumRenderTargets; i++)
//         mTex[i] = NULL;
//
//      mCube.destroy();
//      mRenderer.destroy();
//   }
//};
//
//CreateInteractiveTest(TestGFXDeviceSwitching, "GFX/DeviceSwitching")
//{
//   PlatformWindow *mWindow;
//   GFXDevice      *mDevice;
//   CubeBuilder    *mCube;
//   GFXTexHandle   *mTex;
//   S32 mRemainingFrameCount;
//
//   void drawCube(GFXDevice *d)
//   {
//      // Draw our cube.
//      d->setVertexBuffer(mCube->mCubeVB);
//      d->setPrimitiveBuffer(mCube->mCubePB);
//
//      d->setBaseRenderState();
//      d->setCullMode(GFXCullNone);
//      d->setVertexColorEnable(true);
//      d->setAlphaBlendEnable(false);
//      d->setupGenericShaders();
//
//      // Turn on texture, with a cheesy vertex modulate (whee!)
//      d->setTextureStageColorOp( 0, GFXTOPModulate );
//
//      d->drawIndexedPrimitive(GFXTriangleList, 0, 8, 0, 12);
//   }
//
//   void onRenderSignal(WindowId id)
//   {
//      mDevice->beginScene();
//      mDevice->setActiveRenderTarget(mWindow->getGFXTarget());
// 
//      // Fill this in an interesting way...
//      static U32 i=10;
//      mDevice->clear( GFXClearZBuffer | GFXClearStencil | GFXClearTarget, ColorI( 0, i, 0 ), 1.0f, 0 );
//      i+=10;
//
//      // Set up the view...
//      mDevice->setFrustum(90.0f, 1.0f, 0.1f, 100.f);
//
//      // Get some cheesy spin going...
//      MatrixF worldMatrix(1);
//
//      static F32 spinDiddle = 0.f;
//
//      worldMatrix *= MatrixF(EulerF(0,spinDiddle, 90.f - spinDiddle ));
//      worldMatrix.setPosition(Point3F(-2, 5, -2));
//
//      spinDiddle += 0.001f;
//
//      if(spinDiddle > 90.f)
//         spinDiddle = 0.f;
//
//      mDevice->setWorldMatrix(worldMatrix);
//
//      // set sampler if we have one (handle null device case)
//      if (mDevice->getNumSamplers())
//         mDevice->setTexture(0, *mTex);
//
//      // Draw our cube...
//      drawCube(mDevice);
//
//      // And swap.
//      mDevice->endScene();
//      mWindow->getGFXTarget()->present();
//   }
//
//   bool onAppSignal(WindowId d, S32 event)
//   {
//      if(event == WindowClose && d == mWindow->getWindowId())
//         Process::requestShutdown();
//      return true;
//   }
//
//   void run()
//   {
//      PlatformWindowManager *pwm = CreatePlatformWindowManager();
//
//      // Create a video mode to use.
//      GFXVideoMode vm;
//      vm.resolution.x = 400;
//      vm.resolution.y = 400;
//
//      // Query all the available devices and adapters.
//      GFXInit::enumerateAdapters();
//      Vector<GFXAdapter*> adapters;
//      GFXInit::getAdapters(&adapters);
//
//      test(adapters.size() > 0, "Got zero adapters! Hard to run an adapter test with no adapters!");
//
//      // For each reported adapter...
//      for(S32 i=0; i<adapters.size(); i++)
//      {
//         UnitPrint(avar("Testing adapter #%d (%s) in %s.", 
//                     adapters[i]->mIndex, 
//                     adapters[i]->mName, 
//                     GFXInit::getAdapterNameFromType(adapters[i]->mType)));
//
//         // Init the device.
//         mDevice = GFXInit::createDevice(adapters[i]);
//         test(mDevice, "Failed to create a device!");
//         
//         if(!mDevice)
//            continue;
//
//         // Create the window...
//         mWindow = pwm->createWindow(mDevice, vm);
//         test(mWindow, "Failed to create a window for this device!");
//
//         if(!mWindow)
//         {
//            SAFE_DELETE(mDevice);
//            continue;
//         }
//
//         // Create some representative items:
//         // - a cube builder...
//         mCube = new CubeBuilder();
//         mCube->ensureValid(mDevice, 1.0f);
//
//         // - a texture
//         mTex = new GFXTexHandle();
//         if((*mTex).isNull())
//            (*mTex) = mDevice->getTextureManager()->createTexture("common/gui/images/GG_Icon.png", 
//                           &GFXDefaultPersistentProfile);
//
//         // Hook in our events.
//         // Setup our events.
//         mWindow->signalRender.notify(this, &TestGFXDeviceSwitching::onRenderSignal);
//         mWindow->signalApp.notify    (this, &TestGFXDeviceSwitching::onAppSignal);
//
//         // Render until the user gets bored.
//         while(Process::processEvents());
//
//         // And clean up, so we can do it again.
//         SAFE_DELETE(mTex);
//         SAFE_DELETE(mCube);
//         SAFE_DELETE(mDevice);
//         SAFE_DELETE(mWindow);
//      }
//
//      // All done!
//      SAFE_DELETE(pwm);
//   }
//};
