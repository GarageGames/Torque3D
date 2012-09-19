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

#ifndef _EDITTSCTRL_H_
#define _EDITTSCTRL_H_

#ifndef _GUITSCONTROL_H_
#include "gui/3d/guiTSControl.h"
#endif
#ifndef _GIZMO_H_
#include "gizmo.h"
#endif

class TerrainBlock;
class MissionArea;
class Gizmo;
class EditManager;
struct ObjectRenderInst;
class SceneRenderState;
class BaseMatInstance;


class EditTSCtrl : public GuiTSCtrl
{
      typedef GuiTSCtrl Parent;

   protected:

      void make3DMouseEvent(Gui3DMouseEvent & gui3Devent, const GuiEvent &event);

      // GuiControl
      virtual void getCursor(GuiCursor *&cursor, bool &showCursor, const GuiEvent &lastGuiEvent);
      virtual void onMouseUp(const GuiEvent & event);
      virtual void onMouseDown(const GuiEvent & event);
      virtual void onMouseMove(const GuiEvent & event);
      virtual void onMouseDragged(const GuiEvent & event);
      virtual void onMouseEnter(const GuiEvent & event);
      virtual void onMouseLeave(const GuiEvent & event);
      virtual void onRightMouseDown(const GuiEvent & event);
      virtual void onRightMouseUp(const GuiEvent & event);
      virtual void onRightMouseDragged(const GuiEvent & event);
      virtual void onMiddleMouseDown(const GuiEvent & event);
      virtual void onMiddleMouseUp(const GuiEvent & event);
      virtual void onMiddleMouseDragged(const GuiEvent & event);
      virtual bool onInputEvent(const InputEventInfo & event);
      virtual bool onMouseWheelUp(const GuiEvent &event);
      virtual bool onMouseWheelDown(const GuiEvent &event);


      virtual void updateGuiInfo() {};
      virtual void renderScene(const RectI &){};
      void renderMissionArea();
      virtual void renderCameraAxis();
      virtual void renderGrid();

      // GuiTSCtrl
      void renderWorld(const RectI & updateRect);

      void _renderScene(ObjectRenderInst*, SceneRenderState *state, BaseMatInstance*);

      /// Zoom in/out in ortho views by "steps".
      void orthoZoom( F32 steps );

   protected:
      enum DisplayType
      {
         DisplayTypeTop,
         DisplayTypeBottom,
         DisplayTypeFront,
         DisplayTypeBack,
         DisplayTypeLeft,
         DisplayTypeRight,
         DisplayTypePerspective,
         DisplayTypeIsometric,
      };

      S32      mDisplayType;
      F32      mOrthoFOV;
      Point3F  mOrthoCamTrans;
      EulerF   mIsoCamRot;
      Point3F  mIsoCamRotCenter;
      F32      mIsoCamAngle;
      Point3F  mRawCamPos;
      Point2I  mLastMousePos;
      bool     mLastMouseClamping;

      bool     mAllowBorderMove;
      S32      mMouseMoveBorder;
      F32      mMouseMoveSpeed;
      U32      mLastBorderMoveTime;

      Gui3DMouseEvent   mLastEvent;
      bool              mLeftMouseDown;
      bool              mRightMouseDown;
      bool              mMiddleMouseDown;
      bool              mMiddleMouseTriggered;
      bool              mMouseLeft;

      SimObjectPtr<Gizmo> mGizmo;
      GizmoProfile *mGizmoProfile;

   public:

      EditTSCtrl();
      ~EditTSCtrl();

      // SimObject
      bool onAdd();
      void onRemove();

      //
      bool        mRenderMissionArea;
      ColorI      mMissionAreaFillColor;
      ColorI      mMissionAreaFrameColor;
      F32         mMissionAreaHeightAdjust;

      //
      ColorI            mConsoleFrameColor;
      ColorI            mConsoleFillColor;
      S32               mConsoleSphereLevel;
      S32               mConsoleCircleSegments;
      S32               mConsoleLineWidth;

      static void initPersistFields();
      static void consoleInit();

      //
      bool              mConsoleRendering;
      bool              mRightMousePassThru;
      bool              mMiddleMousePassThru;

      // all editors will share a camera
      static Point3F    smCamPos;
      static MatrixF    smCamMatrix;
      static bool       smCamOrtho;
      static F32        smCamNearPlane;
      static F32        smCamFOV;
      static F32        smVisibleDistanceScale;

      static U32        smSceneBoundsMask;
      static Point3F    smMinSceneBounds;

      bool              mRenderGridPlane;
      ColorI            mGridPlaneColor;
      F32               mGridPlaneSize;
      F32               mGridPlaneSizePixelBias;
      S32               mGridPlaneMinorTicks;
      ColorI            mGridPlaneMinorTickColor;
      ColorI            mGridPlaneOriginColor;

      GFXStateBlockRef  mBlendSB;

      // GuiTSCtrl
      virtual bool getCameraTransform(MatrixF* cameraMatrix);
      virtual void computeSceneBounds(Box3F& bounds);
      bool processCameraQuery(CameraQuery * query);

      // guiControl
      virtual void onRender(Point2I offset, const RectI &updateRect);
      virtual void on3DMouseUp(const Gui3DMouseEvent &){};
      virtual void on3DMouseDown(const Gui3DMouseEvent &){};
      virtual void on3DMouseMove(const Gui3DMouseEvent &){};
      virtual void on3DMouseDragged(const Gui3DMouseEvent &){};
      virtual void on3DMouseEnter(const Gui3DMouseEvent &){};
      virtual void on3DMouseLeave(const Gui3DMouseEvent &){};
      virtual void on3DRightMouseDown(const Gui3DMouseEvent &){};
      virtual void on3DRightMouseUp(const Gui3DMouseEvent &){};
      virtual void on3DRightMouseDragged(const Gui3DMouseEvent &){};
      virtual void on3DMouseWheelUp(const Gui3DMouseEvent &){};
      virtual void on3DMouseWheelDown(const Gui3DMouseEvent &){};
      virtual void get3DCursor(GuiCursor *&cursor, bool &visible, const Gui3DMouseEvent &);

      virtual bool isMiddleMouseDown() {return mMiddleMouseDown;}

      S32 getDisplayType() const {return mDisplayType;}
      virtual void setDisplayType(S32 type);
      
      /// Return true if the current view is an ortho projection along one of the world axes.
      bool isOrthoDisplayType() const { return ( mDisplayType != DisplayTypePerspective && mDisplayType != DisplayTypeIsometric ); }
      
      F32 getOrthoFOV() const { return mOrthoFOV; }
      void setOrthoFOV( F32 fov ) { mOrthoFOV = fov; }

      virtual TerrainBlock* getActiveTerrain();

      virtual void calcOrthoCamOffset(F32 mousex, F32 mousey, U8 modifier=0);

      Gizmo* getGizmo() { return mGizmo; }

      /// Set flags or other Gizmo state appropriate for the current situation.
      /// For example derived classes may override this to disable certain
      /// axes of modes of manipulation.
      virtual void updateGizmo();

      DECLARE_CONOBJECT(EditTSCtrl);
      DECLARE_CATEGORY( "Gui Editor" );
};

#endif // _EDITTSCTRL_H_
