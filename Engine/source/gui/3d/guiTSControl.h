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

#ifndef _GUITSCONTROL_H_
#define _GUITSCONTROL_H_

#ifndef _GUICONTAINER_H_
#include "gui/containers/guiContainer.h"
#endif
#ifndef _MMATH_H_
#include "math/mMath.h"
#endif


#ifndef _MATTEXTURETARGET_H_
#include "materials/matTextureTarget.h"
#endif

class IDisplayDevice;
class GuiOffscreenCanvas;

struct CameraQuery
{
   SimObject*  object;
   F32         nearPlane;
   F32         farPlane;
   F32         fov;
   FovPort     fovPort[2]; // fov for each eye
   Point2F     projectionOffset;
   Point3F     eyeOffset[2];
   MatrixF     eyeTransforms[2];
   bool        ortho;
   bool        hasFovPort;
   bool        hasStereoTargets;
   MatrixF     cameraMatrix;
   RectI       stereoViewports[2]; // destination viewports
   GFXTextureTarget* stereoTargets[2];
   GuiCanvas* drawCanvas; // Canvas we are drawing to. Needed for VR
};

/// Abstract base class for 3D viewport GUIs.
class GuiTSCtrl : public GuiContainer
{
   typedef GuiContainer Parent;

public:
   enum RenderStyles {
      RenderStyleStandard           = 0,
      RenderStyleStereoSideBySide   = (1<<0)
   };

protected:
   static U32     smFrameCount;
   static bool    smUseLatestDisplayTransform;
   F32            mCameraZRot;
   F32            mForceFOV;

   /// A list of GuiTSCtrl which are awake and 
   /// most likely rendering.
   static Vector<GuiTSCtrl*> smAwakeTSCtrls;

   /// A scalar which controls how much of the reflection
   /// update timeslice for this viewport to get.
   F32 mReflectPriority;

   /// The current render type
   U32 mRenderStyle;

   F32         mOrthoWidth;
   F32         mOrthoHeight;

   MatrixF     mSaveModelview;
   MatrixF     mSaveProjection;
   RectI       mSaveViewport;
	Frustum		mSaveFrustum;
   
   /// The saved world to screen space scale.
   /// @see getWorldToScreenScale
   Point2F mSaveWorldToScreenScale;

   /// The last camera query set in onRender.
   /// @see getLastCameraQuery
   CameraQuery mLastCameraQuery;

   NamedTexTargetRef mStereoGuiTarget;
   GFXVertexBufferHandle<GFXVertexPCT> mStereoOverlayVB;
   GFXStateBlockRef mStereoGuiSB;
   
public:
   
   GuiTSCtrl();

   void onPreRender();
   void onRender(Point2I offset, const RectI &updateRect);
   virtual bool processCameraQuery(CameraQuery *query);

   /// Subclasses can override this to perform 3D rendering.
   virtual void renderWorld(const RectI &updateRect);

   /// Subclasses can override this to perform 2D rendering.   
   virtual void renderGui(Point2I offset, const RectI &updateRect) {}

   static void initPersistFields();
   static void consoleInit();

   virtual bool onWake();
   virtual void onSleep();

   /// Returns the last World Matrix set in onRender.
   const MatrixF& getLastWorldMatrix() const { return mSaveModelview; }

   /// Returns the last Projection Matrix set in onRender.
   const MatrixF& getLastProjectionMatrix() const { return mSaveProjection; }

   /// Returns the last Viewport Rect set in onRender.
   const RectI&   getLastViewportRect() const { return mSaveViewport; }

   /// Returns the last Frustum set in onRender.
   const Frustum&	getLastFrustum() const { return mSaveFrustum; }

   /// Returns the scale for converting world space 
   /// units to screen space units... aka pixels.
   /// @see GFXDevice::getWorldToScreenScale
   const Point2F& getWorldToScreenScale() const { return mSaveWorldToScreenScale; }

   /// Returns the last camera query set in onRender.
   const CameraQuery& getLastCameraQuery() const { return mLastCameraQuery; }   
   
   /// Returns the screen space X,Y and Z for world space point.
   /// The input z coord is depth, from 0 to 1.
   bool project( const Point3F &pt, Point3F *dest ) const; 

   /// Returns the world space point for X, Y and Z.  The ouput
   /// z coord is depth, from 0 to 1
   bool unproject( const Point3F &pt, Point3F *dest ) const;

   ///
   F32 projectRadius( F32 dist, F32 radius ) const;

   /// Returns the distance required to fit the given
   /// radius within the camera's view.
   F32 calculateViewDistance(F32 radius);

   /// Takes Points in World Space representing a Line or LineList.
   /// These will be projected into screen space and rendered with the requested
   /// width in pixels.
   ///
   /// This is a 2D drawing operation and should not be called from within
   /// renderScene without preparing the GFX for 2D rendering first.   
   ///
   /// These methods are NOT optimized for performance in any way and are only
   /// intended for debug rendering, editor rendering, or infrequent rendering.
   ///
   void drawLine( Point3F p0, Point3F p1, const ColorI &color, F32 width );
   void drawLineList( const Vector<Point3F> &points, const ColorI color, F32 width );

   static const U32& getFrameCount() { return smFrameCount; }

   bool shouldRenderChildControls() { return mRenderStyle == RenderStyleStandard; }

   void setStereoGui(GuiOffscreenCanvas *canvas);

   DECLARE_CONOBJECT(GuiTSCtrl);
   DECLARE_CATEGORY( "Gui 3D" );
   DECLARE_DESCRIPTION( "Abstract base class for controls that render a 3D viewport." );
};

typedef GuiTSCtrl::RenderStyles GuiTSRenderStyles;

DefineEnumType( GuiTSRenderStyles );

#endif // _GUITSCONTROL_H_
