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
#include "gui/3d/guiTSControl.h"
#include "gui/core/guiOffscreenCanvas.h"

#include "console/engineAPI.h"
#include "scene/sceneManager.h"
#include "lighting/lightManager.h"
#include "gfx/sim/debugDraw.h"
#include "gfx/gfxTransformSaver.h"
#include "gfx/screenshot.h"
#include "math/mathUtils.h"
#include "gui/core/guiCanvas.h"
#include "scene/reflectionManager.h"
#include "postFx/postEffectManager.h"
#include "gfx/gfxTransformSaver.h"
#include "gfx/gfxDrawUtil.h"
#include "gfx/gfxDebugEvent.h"

GFXTextureObject *gLastStereoTexture = NULL;

#define TS_OVERLAY_SCREEN_WIDTH 0.75

IMPLEMENT_CONOBJECT( GuiTSCtrl );

ConsoleDocClass( GuiTSCtrl,
   "@brief Abstract base class for controls that render 3D scenes.\n\n"
   
   "GuiTSCtrl is the base class for controls that render 3D camera views in Torque.  The class itself "
   "does not implement a concrete scene rendering.  Use GuiObjectView to display invidiual shapes in "
   "the Gui and GameTSCtrl to render full scenes.\n\n"
   
   "@see GameTSCtrl\n"
   "@see GuiObjectView\n"
   "@ingroup Gui3D\n"
);

U32 GuiTSCtrl::smFrameCount = 0;
bool GuiTSCtrl::smUseLatestDisplayTransform = true;
Vector<GuiTSCtrl*> GuiTSCtrl::smAwakeTSCtrls;

ImplementEnumType( GuiTSRenderStyles,
   "Style of rendering for a GuiTSCtrl.\n\n"
   "@ingroup Gui3D" )
	{ GuiTSCtrl::RenderStyleStandard,         "standard"              },
	{ GuiTSCtrl::RenderStyleStereoSideBySide, "stereo side by side"   },
EndImplementEnumType;

//-----------------------------------------------------------------------------

namespace 
{
   void _drawLine( const Point3F &p0, const Point3F &p1, const ColorI &color, F32 width )
   {
      F32 x1, x2, y1, y2, z1, z2;

      x1 = p0.x;
      y1 = p0.y;
      z1 = p0.z;
      x2 = p1.x;
      y2 = p1.y;
      z2 = p1.z;

      //
      // Convert Line   a----------b
      //
      // Into Quad      v0---------v1
      //                 a         b
      //                v2---------v3
      //

      Point2F start(x1, y1);
      Point2F end(x2, y2);
      Point2F perp, lineVec;

      // handle degenerate case where point a = b
      if(x1 == x2 && y1 == y2)
      {
         perp.set(0.0f, width * 0.5f);
         lineVec.set(0.1f, 0.0f);
      }
      else
      {
         perp.set(start.y - end.y, end.x - start.x);
         lineVec.set(end.x - start.x, end.y - start.y);
         perp.normalize(width * 0.5f);
         lineVec.normalize(0.1f);
      }
      start -= lineVec;
      end   += lineVec;

      GFXVertexBufferHandle<GFXVertexPCT> verts(GFX, 4, GFXBufferTypeVolatile);
      verts.lock();

      verts[0].point.set( start.x+perp.x, start.y+perp.y, z1 );
      verts[1].point.set( end.x+perp.x, end.y+perp.y, z2 );
      verts[2].point.set( start.x-perp.x, start.y-perp.y, z1 );
      verts[3].point.set( end.x-perp.x, end.y-perp.y, z2 );

      verts[0].color = color;
      verts[1].color = color;
      verts[2].color = color;
      verts[3].color = color;

      verts.unlock();
      GFX->setVertexBuffer( verts );

      GFXStateBlockDesc desc;
      desc.setCullMode(GFXCullNone);
      desc.setZReadWrite(false);
      desc.setBlend(true, GFXBlendSrcAlpha, GFXBlendInvSrcAlpha);
      GFX->setStateBlockByDesc( desc );

      GFX->setupGenericShaders();
      GFX->drawPrimitive( GFXTriangleStrip, 0, 2 );
   }
}

//-----------------------------------------------------------------------------

GuiTSCtrl::GuiTSCtrl()
{
   mCameraZRot = 0;
   mForceFOV = 0;
   mReflectPriority = 1.0f;

   mRenderStyle = RenderStyleStandard;

   mSaveModelview.identity();
   mSaveProjection.identity();
   mSaveViewport.set( 0, 0, 10, 10 );
   mSaveWorldToScreenScale.set( 0, 0 );

   mLastCameraQuery.cameraMatrix.identity();
   mLastCameraQuery.fov = 45.0f;
   mLastCameraQuery.object = NULL;
   mLastCameraQuery.farPlane = 10.0f;
   mLastCameraQuery.nearPlane = 0.01f;

   mLastCameraQuery.projectionOffset = Point2F::Zero;
   mLastCameraQuery.hasFovPort = false;
   mLastCameraQuery.hasStereoTargets = false;

   mLastCameraQuery.ortho = false;
}

//-----------------------------------------------------------------------------

void GuiTSCtrl::initPersistFields()
{
   addGroup( "Camera" );
   
      addField("cameraZRot", TypeF32, Offset(mCameraZRot, GuiTSCtrl),
         "Z rotation angle of camera." );
      addField("forceFOV",   TypeF32, Offset(mForceFOV,   GuiTSCtrl),
         "The vertical field of view in degrees or zero to use the normal camera FOV." );
         
   endGroup( "Camera" );
   
   addGroup( "Rendering" );
   
      addField( "reflectPriority", TypeF32, Offset( mReflectPriority, GuiTSCtrl ),
         "The share of the per-frame reflection update work this control's rendering should run.\n"
         "The reflect update priorities of all visible GuiTSCtrls are added together and each control is assigned "
         "a share of the per-frame reflection update time according to its percentage of the total priority value." );

      addField("renderStyle", TYPEID< RenderStyles >(), Offset(mRenderStyle, GuiTSCtrl),
         "Indicates how this control should render its contents." );

   endGroup( "Rendering" );
   
   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

void GuiTSCtrl::consoleInit()
{
   Con::addVariable("$TSControl::frameCount", TypeS32, &smFrameCount, "The number of frames that have been rendered since this control was created.\n"
	   "@ingroup Rendering\n");
   Con::addVariable("$TSControl::useLatestDisplayTransform", TypeBool, &smUseLatestDisplayTransform, "Use the latest view transform when rendering stereo instead of the one calculated by the last move.\n"
	   "@ingroup Rendering\n");
}

//-----------------------------------------------------------------------------

bool GuiTSCtrl::onWake()
{
   if ( !Parent::onWake() )
      return false;

   // Add ourselves to the active viewport list.
   AssertFatal( !smAwakeTSCtrls.contains( this ), 
      "GuiTSCtrl::onWake - This control is already in the awake list!" );
   smAwakeTSCtrls.push_back( this );

   // For VR
   mLastCameraQuery.drawCanvas = getRoot();

   return true;
}

//-----------------------------------------------------------------------------

void GuiTSCtrl::onSleep()
{
   Parent::onSleep();

   AssertFatal( smAwakeTSCtrls.contains( this ), 
      "GuiTSCtrl::onSleep - This control is not in the awake list!" );
   smAwakeTSCtrls.remove( this );
}

//-----------------------------------------------------------------------------

void GuiTSCtrl::onPreRender()
{
   setUpdate();
}

//-----------------------------------------------------------------------------

bool GuiTSCtrl::processCameraQuery(CameraQuery *)
{
   return false;
}

//-----------------------------------------------------------------------------

void GuiTSCtrl::renderWorld(const RectI& /*updateRect*/)
{
}

//-----------------------------------------------------------------------------

F32 GuiTSCtrl::projectRadius( F32 dist, F32 radius ) const
{
   // Fixup any negative or zero distance so we
   // don't get a divide by zero.
   dist = dist > 0.0f ? dist : 0.001f;
   return ( radius / dist ) * mSaveWorldToScreenScale.y;   
}

//-----------------------------------------------------------------------------

bool GuiTSCtrl::project( const Point3F &pt, Point3F *dest ) const
{
   return MathUtils::mProjectWorldToScreen(pt,dest,mSaveViewport,mSaveModelview,mSaveProjection);
}

//-----------------------------------------------------------------------------

bool GuiTSCtrl::unproject( const Point3F &pt, Point3F *dest ) const
{
   MathUtils::mProjectScreenToWorld(pt,dest,mSaveViewport,mSaveModelview,mSaveProjection,mLastCameraQuery.farPlane,mLastCameraQuery.nearPlane);
   return true;
}

//-----------------------------------------------------------------------------

F32 GuiTSCtrl::calculateViewDistance(F32 radius)
{
   F32 fov = mLastCameraQuery.fov;
   F32 wwidth;
   F32 wheight;
   F32 renderWidth = (mRenderStyle == RenderStyleStereoSideBySide) ? F32(getWidth())*0.5f : F32(getWidth());
   F32 renderHeight = F32(getHeight());
   F32 aspectRatio = renderWidth / renderHeight;
   
   // Use the FOV to calculate the viewport height scale
   // then generate the width scale from the aspect ratio.
   if(!mLastCameraQuery.ortho)
   {
      wheight = mLastCameraQuery.nearPlane * mTan(mLastCameraQuery.fov / 2.0f);
      wwidth = aspectRatio * wheight;
   }
   else
   {
      wheight = mLastCameraQuery.fov;
      wwidth = aspectRatio * wheight;
   }

   // Now determine if we should use the width 
   // fov or height fov.
   //
   // If the window is taller than it is wide, use the 
   // width fov to keep the object completely in view.
   if (wheight > wwidth)
      fov = mAtan( wwidth / mLastCameraQuery.nearPlane ) * 2.0f;

   return radius / mTan(fov / 2.0f);
}

//-----------------------------------------------------------------------------

static FovPort CalculateFovPortForCanvas(const RectI viewport, const CameraQuery &cameraQuery)
{
   F32 wwidth;
   F32 wheight;
   F32 renderWidth = viewport.extent.x;
   F32 renderHeight = viewport.extent.y;
   F32 aspectRatio = renderWidth / renderHeight;

   // Use the FOV to calculate the viewport height scale
   // then generate the width scale from the aspect ratio.
   if(!cameraQuery.ortho)
   {
      wheight = /*cameraQuery.nearPlane * */ mTan(cameraQuery.fov / 2.0f);
      wwidth = aspectRatio * wheight;
   }
   else
   {
      wheight = cameraQuery.fov;
      wwidth = aspectRatio * wheight;
   }

   F32 hscale = wwidth * 2.0f / renderWidth;
   F32 vscale = wheight * 2.0f / renderHeight;

   F32 left = 0.0f * hscale - wwidth;
   F32 right = renderWidth * hscale - wwidth;
   F32 top = wheight - vscale * 0.0f;
   F32 bottom = wheight - vscale * renderHeight;

   FovPort fovPort;
   fovPort.upTan = top;
   fovPort.downTan = -bottom;
   fovPort.leftTan = -left;
   fovPort.rightTan = right;

   return fovPort;
}

//-----------------------------------------------------------------------------

void GuiTSCtrl::onRender(Point2I offset, const RectI &updateRect)
{
	// Save the current transforms so we can restore
   // it for child control rendering below.
   GFXTransformSaver saver;
   bool renderingToTarget = false;

   if(!processCameraQuery(&mLastCameraQuery))
   {
      // We have no camera, but render the GUI children 
      // anyway.  This makes editing GuiTSCtrl derived
      // controls easier in the GuiEditor.
      renderChildControls( offset, updateRect );
      return;
   }

   GFXTargetRef origTarget = GFX->getActiveRenderTarget();

   // Set up the appropriate render style
   U32 prevRenderStyle = GFX->getCurrentRenderStyle();
   Point2F prevProjectionOffset = GFX->getCurrentProjectionOffset();
   Point2I renderSize = getExtent();

   if(mRenderStyle == RenderStyleStereoSideBySide)
   {
      GFX->setCurrentRenderStyle(GFXDevice::RS_StereoSideBySide);
      GFX->setCurrentProjectionOffset(mLastCameraQuery.projectionOffset);
      GFX->setStereoEyeOffsets(mLastCameraQuery.eyeOffset);

      if (!mLastCameraQuery.hasStereoTargets)
      {
         // Need to calculate our current viewport here
         mLastCameraQuery.stereoViewports[0] = updateRect;
         mLastCameraQuery.stereoViewports[0].extent.x /= 2;
         mLastCameraQuery.stereoViewports[1] = mLastCameraQuery.stereoViewports[0];
         mLastCameraQuery.stereoViewports[1].point.x += mLastCameraQuery.stereoViewports[1].extent.x;
      }

      if (!mLastCameraQuery.hasFovPort)
      {
         // Need to make our own fovPort
         mLastCameraQuery.fovPort[0] = CalculateFovPortForCanvas(mLastCameraQuery.stereoViewports[0], mLastCameraQuery);
         mLastCameraQuery.fovPort[1] = CalculateFovPortForCanvas(mLastCameraQuery.stereoViewports[1], mLastCameraQuery);
      }
         
      GFX->setStereoFovPort(mLastCameraQuery.fovPort); // NOTE: this specifies fov for BOTH eyes

      GFX->setSteroViewports(mLastCameraQuery.stereoViewports);
      GFX->setStereoTargets(mLastCameraQuery.stereoTargets);

      MatrixF myTransforms[2];

      if (smUseLatestDisplayTransform)
      {
         // Use the view matrix determined from the display device
         myTransforms[0] = mLastCameraQuery.eyeTransforms[0];
         myTransforms[1] = mLastCameraQuery.eyeTransforms[1];
      }
      else
      {
         // Use the view matrix determined from the control object
         myTransforms[0] = mLastCameraQuery.cameraMatrix;
         myTransforms[1] = mLastCameraQuery.cameraMatrix;

         QuatF qrot = mLastCameraQuery.cameraMatrix;
         Point3F pos = mLastCameraQuery.cameraMatrix.getPosition();
         Point3F rotEyePos;

         myTransforms[0].setPosition(pos + qrot.mulP(mLastCameraQuery.eyeOffset[0], &rotEyePos));
         myTransforms[1].setPosition(pos + qrot.mulP(mLastCameraQuery.eyeOffset[1], &rotEyePos));
      }

      GFX->setStereoEyeTransforms(myTransforms);

      // Allow render size to originate from the render target
      if (mLastCameraQuery.stereoTargets[0])
      {
         renderSize = mLastCameraQuery.stereoViewports[0].extent;
         renderingToTarget = true;
      }
   }
   else
   {
      GFX->setCurrentRenderStyle(GFXDevice::RS_Standard);
   }

   if ( mReflectPriority > 0 )
   {
      // Get the total reflection priority.
      F32 totalPriority = 0;
      for ( U32 i=0; i < smAwakeTSCtrls.size(); i++ )
         if ( smAwakeTSCtrls[i]->isVisible() )
            totalPriority += smAwakeTSCtrls[i]->mReflectPriority;

      REFLECTMGR->update(  mReflectPriority / totalPriority,
                           getExtent(),
                           mLastCameraQuery );
   }

   if(mForceFOV != 0)
      mLastCameraQuery.fov = mDegToRad(mForceFOV);

   if(mCameraZRot)
   {
      MatrixF rotMat(EulerF(0, 0, mDegToRad(mCameraZRot)));
      mLastCameraQuery.cameraMatrix.mul(rotMat);
   }

   Frustum frustum;
   if(mRenderStyle == RenderStyleStereoSideBySide)
   {
      // NOTE: these calculations are essentially overridden later by the fov port settings when rendering each eye.
      MathUtils::makeFovPortFrustum(&frustum, mLastCameraQuery.ortho,  mLastCameraQuery.nearPlane, mLastCameraQuery.farPlane, mLastCameraQuery.fovPort[0]);
   }
   else
   {
      // set up the camera and viewport stuff:
      F32 wwidth;
      F32 wheight;
      F32 renderWidth = F32(renderSize.x);
      F32 renderHeight = F32(renderSize.y);
      F32 aspectRatio = renderWidth / renderHeight;
   
      // Use the FOV to calculate the viewport height scale
      // then generate the width scale from the aspect ratio.
      if(!mLastCameraQuery.ortho)
      {
         wheight = mLastCameraQuery.nearPlane * mTan(mLastCameraQuery.fov / 2.0f);
         wwidth = aspectRatio * wheight;
      }
      else
      {
         wheight = mLastCameraQuery.fov;
         wwidth = aspectRatio * wheight;
      }

      F32 hscale = wwidth * 2.0f / renderWidth;
      F32 vscale = wheight * 2.0f / renderHeight;

      F32 left = (updateRect.point.x - offset.x) * hscale - wwidth;
      F32 right = (updateRect.point.x + updateRect.extent.x - offset.x) * hscale - wwidth;
      F32 top = wheight - vscale * (updateRect.point.y - offset.y);
      F32 bottom = wheight - vscale * (updateRect.point.y + updateRect.extent.y - offset.y);

      frustum.set( mLastCameraQuery.ortho, left, right, top, bottom, mLastCameraQuery.nearPlane, mLastCameraQuery.farPlane );
   }

	// Manipulate the frustum for tiled screenshots
	const bool screenShotMode = gScreenShot && gScreenShot->isPending();
   if ( screenShotMode )
   {
      gScreenShot->tileFrustum( frustum );      
      GFX->setViewMatrix(MatrixF::Identity);
   }
      
   RectI tempRect = updateRect;
   
   if (!renderingToTarget)
   {
   #ifdef TORQUE_OS_MAC
      Point2I screensize = getRoot()->getWindowSize();
      tempRect.point.y = screensize.y - (tempRect.point.y + tempRect.extent.y);
   #endif

      GFX->setViewport( tempRect );
   }
   else
   {
      // Activate stereo RT
      GFX->activateStereoTarget(-1);
   }

   // Clear the zBuffer so GUI doesn't hose object rendering accidentally
   GFX->clear( GFXClearZBuffer , ColorI(20,20,20), 1.0f, 0 );
   //GFX->clear( GFXClearTarget, ColorI(255,0,0), 1.0f, 0);

   GFX->setFrustum( frustum );
   if(mLastCameraQuery.ortho)
   {
      mOrthoWidth = frustum.getWidth();
      mOrthoHeight = frustum.getHeight();
   }

   // We're going to be displaying this render at size of this control in
   // pixels - let the scene know so that it can calculate e.g. reflections
   // correctly for that final display result.
   gClientSceneGraph->setDisplayTargetResolution(renderSize);

   // Set the GFX world matrix to the world-to-camera transform, but don't 
   // change the cameraMatrix in mLastCameraQuery. This is because 
   // mLastCameraQuery.cameraMatrix is supposed to contain the camera-to-world
   // transform. In-place invert would save a copy but mess up any GUIs that
   // depend on that value.
   MatrixF worldToCamera = mLastCameraQuery.cameraMatrix;
   worldToCamera.inverse();
   GFX->setWorldMatrix( worldToCamera );

   mSaveProjection = GFX->getProjectionMatrix();
   mSaveModelview = GFX->getWorldMatrix();
   mSaveViewport = updateRect;
   mSaveWorldToScreenScale = GFX->getWorldToScreenScale();
   mSaveFrustum = GFX->getFrustum();
   mSaveFrustum.setTransform( mLastCameraQuery.cameraMatrix );

   // Set the default non-clip projection as some 
   // objects depend on this even in non-reflect cases.
   gClientSceneGraph->setNonClipProjection( mSaveProjection );

   // Give the post effect manager the worldToCamera, and cameraToScreen matrices
   PFXMGR->setFrameMatrices( mSaveModelview, mSaveProjection );

   renderWorld(updateRect);
   DebugDrawer::get()->render();

   // Render the canvas overlay if its available
   if (mRenderStyle == RenderStyleStereoSideBySide && mStereoGuiTarget.getPointer())
   {
      GFXDEBUGEVENT_SCOPE( StereoGui_Render, ColorI( 255, 0, 0 ) );
      MatrixF proj(1);
      
      Frustum originalFrustum = GFX->getFrustum();
      GFXTextureObject *texObject = mStereoGuiTarget->getTexture(0);
      const FovPort *currentFovPort = GFX->getStereoFovPort();
      const MatrixF *eyeTransforms = GFX->getStereoEyeTransforms();
      const Point3F *eyeOffset = GFX->getStereoEyeOffsets();
      Frustum gfxFrustum = originalFrustum;

      for (U32 i=0; i<2; i++)
      {
         GFX->activateStereoTarget(i);
         MathUtils::makeFovPortFrustum(&gfxFrustum, true, gfxFrustum.getNearDist(), gfxFrustum.getFarDist(), currentFovPort[i], eyeTransforms[i]);
         GFX->setFrustum(gfxFrustum);

         MatrixF eyeWorldTrans(1);
         eyeWorldTrans.setPosition(Point3F(eyeOffset[i].x,eyeOffset[i].y,eyeOffset[i].z));
         MatrixF eyeWorld(1);
         eyeWorld.mul(eyeWorldTrans);
         eyeWorld.inverse();
         
         GFX->setWorldMatrix(eyeWorld);
         GFX->setViewMatrix(MatrixF::Identity);

         if (!mStereoOverlayVB.getPointer())
         {
            mStereoOverlayVB.set(GFX, 4, GFXBufferTypeStatic);
            GFXVertexPCT *verts = mStereoOverlayVB.lock(0, 4);

            F32 texLeft   = 0.0f;
            F32 texRight  = 1.0f;
            F32 texTop    = 1.0f;
            F32 texBottom = 0.0f;

            F32 rectRatio = gfxFrustum.getWidth() / gfxFrustum.getHeight();
            F32 rectWidth = gfxFrustum.getWidth() * TS_OVERLAY_SCREEN_WIDTH;
            F32 rectHeight = rectWidth * rectRatio;

            F32 screenLeft   = -rectWidth * 0.5;
            F32 screenRight  = rectWidth * 0.5;
            F32 screenTop    = -rectHeight * 0.5;
            F32 screenBottom = rectHeight * 0.5;

            const F32 fillConv = 0.0f;
            const F32 frustumDepthAdjusted = gfxFrustum.getNearDist() + 0.012;
            verts[0].point.set( screenLeft  - fillConv, frustumDepthAdjusted, screenTop    - fillConv );
            verts[1].point.set( screenRight - fillConv, frustumDepthAdjusted, screenTop    - fillConv );
            verts[2].point.set( screenLeft  - fillConv, frustumDepthAdjusted, screenBottom - fillConv );
            verts[3].point.set( screenRight - fillConv, frustumDepthAdjusted, screenBottom - fillConv );

            verts[0].color = verts[1].color = verts[2].color = verts[3].color = ColorI(255,255,255,255);

            verts[0].texCoord.set( texLeft,  texTop );
            verts[1].texCoord.set( texRight, texTop );
            verts[2].texCoord.set( texLeft,  texBottom );
            verts[3].texCoord.set( texRight, texBottom );

            mStereoOverlayVB.unlock();
         }

         if (!mStereoGuiSB.getPointer())
         {
            // DrawBitmapStretchSR
            GFXStateBlockDesc bitmapStretchSR;
            bitmapStretchSR.setCullMode(GFXCullNone);
            bitmapStretchSR.setZReadWrite(false, false);
            bitmapStretchSR.setBlend(true, GFXBlendSrcAlpha, GFXBlendInvSrcAlpha);
            bitmapStretchSR.samplersDefined = true;

            bitmapStretchSR.samplers[0] = GFXSamplerStateDesc::getClampLinear();
            bitmapStretchSR.samplers[0].minFilter = GFXTextureFilterPoint;
            bitmapStretchSR.samplers[0].mipFilter = GFXTextureFilterPoint;
            bitmapStretchSR.samplers[0].magFilter = GFXTextureFilterPoint;

            mStereoGuiSB = GFX->createStateBlock(bitmapStretchSR);
         }

         GFX->setVertexBuffer(mStereoOverlayVB);
         GFX->setStateBlock(mStereoGuiSB);
         GFX->setTexture( 0, texObject );
         GFX->setupGenericShaders( GFXDevice::GSModColorTexture );
         GFX->drawPrimitive( GFXTriangleStrip, 0, 2 );
      }
   }

	// Restore the previous matrix state before
   // we begin rendering the child controls.
   saver.restore();

   // Restore the render style and any stereo parameters
   GFX->setActiveRenderTarget(origTarget);
   GFX->setCurrentRenderStyle(prevRenderStyle);
   GFX->setCurrentProjectionOffset(prevProjectionOffset);

   
   if(mRenderStyle == RenderStyleStereoSideBySide && gLastStereoTexture)
   {
      GFX->setClipRect(updateRect);
      GFX->getDrawUtil()->drawBitmapStretch(gLastStereoTexture, updateRect);
   }

   // Allow subclasses to render 2D elements.
   GFX->setClipRect(updateRect);
   renderGui( offset, updateRect );

   if (shouldRenderChildControls())
   {
      renderChildControls(offset, updateRect);
   }
   smFrameCount++;
}

//-----------------------------------------------------------------------------

void GuiTSCtrl::drawLine( Point3F p0, Point3F p1, const ColorI &color, F32 width )
{   
   if ( !mSaveFrustum.clipSegment( p0, p1 ) )
      return;

   MathUtils::mProjectWorldToScreen( p0, &p0, mSaveViewport, mSaveModelview, mSaveProjection );   
   MathUtils::mProjectWorldToScreen( p1, &p1, mSaveViewport, mSaveModelview, mSaveProjection );   

   p0.x = mClampF( p0.x, 0.0f, mSaveViewport.extent.x );
   p0.y = mClampF( p0.y, 0.0f, mSaveViewport.extent.y );
   p1.x = mClampF( p1.x, 0.0f, mSaveViewport.extent.x );
   p1.y = mClampF( p1.y, 0.0f, mSaveViewport.extent.y );
   p0.z = p1.z = 0.0f;

   _drawLine( p0, p1, color, width );
}

//-----------------------------------------------------------------------------

void GuiTSCtrl::drawLineList( const Vector<Point3F> &points, const ColorI color, F32 width )
{
   for ( S32 i = 0; i < points.size() - 1; i++ )
      drawLine( points[i], points[i+1], color, width );
}


void GuiTSCtrl::setStereoGui(GuiOffscreenCanvas *canvas)
{
   mStereoGuiTarget = canvas ? canvas->getTarget() : NULL;
}

//=============================================================================
//    Console Methods.
//=============================================================================
// MARK: ---- Console Methods ----

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiTSCtrl, unproject, Point3F, ( Point3F screenPosition ),,
   "Transform 3D screen-space coordinates (x, y, depth) to world space.\n"
   "This method can be, for example, used to find the world-space position relating to the current mouse cursor position.\n"
   "@param screenPosition The x/y position on the screen plus the depth from the screen-plane outwards.\n"
   "@return The world-space position corresponding to the given screen-space coordinates." )
{
   Point3F worldPos;
   object->unproject( screenPosition, &worldPos );
   return worldPos;
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiTSCtrl, project, Point3F, ( Point3F worldPosition ),,
   "Transform world-space coordinates to screen-space (x, y, depth) coordinates.\n"
   "@param worldPosition The world-space position to transform to screen-space.\n"
   "@return The " )
{
   Point3F screenPos;
   object->project( worldPosition, &screenPos );
   return screenPos;
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiTSCtrl, getWorldToScreenScale, Point2F, (),,
   "Get the ratio between world-space units and pixels.\n"
   "@return The amount of world-space units covered by the extent of a single pixel." )
{
   return object->getWorldToScreenScale();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiTSCtrl, calculateViewDistance, F32, ( F32 radius ),,
   "Given the camera's current FOV, get the distance from the camera's viewpoint at which the given radius will fit in the render area.\n"
   "@param radius Radius in world-space units which should fit in the view.\n"
   "@return The distance from the viewpoint at which the given radius would be fully visible." )
{
   return object->calculateViewDistance( radius );
}

DefineEngineMethod( GuiTSCtrl, setStereoGui, void, ( GuiOffscreenCanvas* canvas ),,
   "Sets the current stereo texture to an offscreen canvas\n"
   "@param canvas The desired canvas." )
{
   object->setStereoGui(canvas);
}
