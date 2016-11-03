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
#include "gui/worldEditor/editTSCtrl.h"

#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "T3D/gameBase/gameConnection.h"
#include "gui/worldEditor/editor.h"
#include "gui/core/guiCanvas.h"
#include "terrain/terrData.h"
#include "T3D/missionArea.h"
#include "gfx/primBuilder.h"
#include "gfx/gfxDrawUtil.h"
#include "gfx/gfxTransformSaver.h"
#include "gfx/gfxDebugEvent.h"
#include "scene/sceneManager.h"
#include "scene/sceneRenderState.h"
#include "renderInstance/renderBinManager.h"


IMPLEMENT_CONOBJECT(EditTSCtrl);
ConsoleDocClass( EditTSCtrl,
   "@brief 3D view control used specifically by Torque 3D's editors.\n\n"

   "For Torque 3D editors only, not for actual game development\n\n"

   "@ingroup Editors\n"

   "@internal"
);

Point3F  EditTSCtrl::smCamPos;
MatrixF  EditTSCtrl::smCamMatrix;
bool     EditTSCtrl::smCamOrtho = false;
F32      EditTSCtrl::smCamNearPlane;
F32      EditTSCtrl::smCamFOV = 75.0f;
F32      EditTSCtrl::smVisibleDistanceScale = 1.0f;
U32      EditTSCtrl::smSceneBoundsMask = EnvironmentObjectType | TerrainObjectType | WaterObjectType | CameraObjectType;
Point3F  EditTSCtrl::smMinSceneBounds = Point3F(500.0f, 500.0f, 500.0f);

EditTSCtrl::EditTSCtrl()
{
   mGizmoProfile = NULL;
   mGizmo = NULL;

   mRenderMissionArea = true;
   mMissionAreaFillColor.set(255,0,0,20);
   mMissionAreaFrameColor.set(255,0,0,128);
   mMissionAreaHeightAdjust = 5.0f;

   mConsoleFrameColor.set(255,0,0,255);
   mConsoleFillColor.set(255,0,0,120);
   mConsoleSphereLevel = 1;
   mConsoleCircleSegments = 32;
   mConsoleLineWidth = 1;
   mRightMousePassThru = true;
   mMiddleMousePassThru = true;

   mConsoleRendering = false;

   mDisplayType = DisplayTypePerspective;
   mOrthoFOV = 50.0f;
   mOrthoCamTrans.set(0.0f, 0.0f, 0.0f);

   mIsoCamAngle = mDegToRad(45.0f);
   mIsoCamRot = EulerF(0, 0, 0);

   mRenderGridPlane = true;
   mGridPlaneOriginColor = ColorI(255, 255, 255, 100);
   mGridPlaneColor = ColorI(102, 102, 102, 100);
   mGridPlaneMinorTickColor = ColorI(51, 51, 51, 100);
   mGridPlaneMinorTicks = 9;
   mGridPlaneSize = 1.0f;
   mGridPlaneSizePixelBias = 10.0f;

   mLastMousePos.set(0, 0);

   mAllowBorderMove = false;
   mMouseMoveBorder = 20;
   mMouseMoveSpeed = 0.1f;
   mLastBorderMoveTime = 0;
   mLeftMouseDown = false;
   mRightMouseDown = false;
   mMiddleMouseDown = false;
   mMiddleMouseTriggered = false;
   mMouseLeft = false;

   mBlendSB = NULL;

}

EditTSCtrl::~EditTSCtrl()
{
   mBlendSB = NULL;
}

//------------------------------------------------------------------------------

bool EditTSCtrl::onAdd()
{
   if(!Parent::onAdd())
      return(false);

   // give all derived access to the fields
   setModStaticFields(true);

   GFXStateBlockDesc blenddesc;
   blenddesc.setBlend(true, GFXBlendSrcAlpha, GFXBlendInvSrcAlpha);
   mBlendSB = GFX->createStateBlock( blenddesc );

   if ( !mGizmoProfile )
   {
      Con::errorf( "EditTSCtrl::onadd - gizmoProfile was not assigned, cannot create control!" );
      return false;
   }

   mGizmo = new Gizmo();
   mGizmo->setProfile( mGizmoProfile );
   mGizmo->registerObject();

   return true;
}

void EditTSCtrl::onRemove()
{
   Parent::onRemove();

   if ( mGizmo )
      mGizmo->deleteObject();
}

void EditTSCtrl::onRender(Point2I offset, const RectI &updateRect)
{
   // Perform possible mouse border move...
   if(mAllowBorderMove && smCamOrtho && !mLeftMouseDown && !mRightMouseDown && !mMouseLeft)
   {
      Point2I ext = getExtent();

      U32 current = Platform::getRealMilliseconds();
      bool update = false;
      F32 movex = 0.0f;
      F32 movey = 0.0f;

      Point2I localMouse = globalToLocalCoord(mLastMousePos);

      if(localMouse.x <= mMouseMoveBorder || localMouse.x >= ext.x - mMouseMoveBorder)
      {
         if(mLastBorderMoveTime != 0)
         {
            U32 dt = current - mLastBorderMoveTime;
            if(localMouse.x <= mMouseMoveBorder)
            {
               movex = mMouseMoveSpeed * dt;
            }
            else
            {
               movex = -mMouseMoveSpeed * dt;
            }
         }
         update = true;
      }

      if(localMouse.y <= mMouseMoveBorder || localMouse.y >= ext.y - mMouseMoveBorder)
      {
         if(mLastBorderMoveTime != 0)
         {
            U32 dt = current - mLastBorderMoveTime;
            if(localMouse.y <= mMouseMoveBorder)
            {
               movey = mMouseMoveSpeed * dt;
            }
            else
            {
               movey = -mMouseMoveSpeed * dt;
            }
         }
         update = true;
      }

      if(update)
      {
         mLastBorderMoveTime = current;
         calcOrthoCamOffset(movex, movey);
      }
      else
      {
         mLastBorderMoveTime = 0;
      }
   }

   updateGuiInfo();
   Parent::onRender(offset, updateRect);

}

//------------------------------------------------------------------------------

void EditTSCtrl::initPersistFields()
{
   addGroup( "Grid" );
   
      addField( "gridSize", TypeF32, Offset( mGridPlaneSize, EditTSCtrl ) );
      addField( "gridColor", TypeColorI, Offset( mGridPlaneColor, EditTSCtrl ) );
      addField( "gridOriginColor", TypeColorI, Offset( mGridPlaneOriginColor, EditTSCtrl ) );
      addField( "gridMinorTickColor", TypeColorI, Offset( mGridPlaneMinorTickColor, EditTSCtrl ) );
      addField( "renderOrthoGrid", TypeBool, Offset( mRenderGridPlane, EditTSCtrl ),
         "Whether to render the grid in orthographic axial projections." );
      addField( "renderOrthoGridPixelBias", TypeF32, Offset( mGridPlaneSizePixelBias, EditTSCtrl ),
         "Grid patch pixel size below which to switch to coarser grid resolutions." );
   
   endGroup( "Grid" );
   
   addGroup("Mission Area");	
   
      addField("renderMissionArea", TypeBool, Offset(mRenderMissionArea, EditTSCtrl));
      addField("missionAreaFillColor", TypeColorI, Offset(mMissionAreaFillColor, EditTSCtrl));
      addField("missionAreaFrameColor", TypeColorI, Offset(mMissionAreaFrameColor, EditTSCtrl));
      addField("missionAreaHeightAdjust", TypeF32, Offset(mMissionAreaHeightAdjust, EditTSCtrl),
         "How high above and below the terrain to render the mission area bounds." );
      
   endGroup("Mission Area");	

   addGroup("BorderMovement");	
   
      addField("allowBorderMove", TypeBool, Offset(mAllowBorderMove, EditTSCtrl));
      addField("borderMovePixelSize", TypeS32, Offset(mMouseMoveBorder, EditTSCtrl));
      addField("borderMoveSpeed", TypeF32, Offset(mMouseMoveSpeed, EditTSCtrl));
      
   endGroup("BorderMovement");	

   addGroup("Misc");	
   
      addField("consoleFrameColor", TypeColorI, Offset(mConsoleFrameColor, EditTSCtrl));
      addField("consoleFillColor", TypeColorI, Offset(mConsoleFillColor, EditTSCtrl));
      addField("consoleSphereLevel", TypeS32, Offset(mConsoleSphereLevel, EditTSCtrl));
      addField("consoleCircleSegments", TypeS32, Offset(mConsoleCircleSegments, EditTSCtrl));
      addField("consoleLineWidth", TypeS32, Offset(mConsoleLineWidth, EditTSCtrl));
      addField("gizmoProfile", TYPEID< GizmoProfile >(), Offset(mGizmoProfile, EditTSCtrl));
      
   endGroup("Misc");
   
   Parent::initPersistFields();
}

void EditTSCtrl::consoleInit()
{
   Con::addVariable("pref::WorldEditor::visibleDistanceScale", TypeF32, &EditTSCtrl::smVisibleDistanceScale, "Scale factor for the visible render distance.\n"
	   "@ingroup ");
   
   Con::addVariable("pref::WorldEditor::cameraFOV", TypeF32, &EditTSCtrl::smCamFOV, "Field of view for editor's perspective camera, in degrees.\n"
	   "@ingroup ");
   
   Con::setIntVariable( "$EditTsCtrl::DisplayTypeTop", DisplayTypeTop);
   Con::setIntVariable( "$EditTsCtrl::DisplayTypeBottom", DisplayTypeBottom);
   Con::setIntVariable( "$EditTsCtrl::DisplayTypeFront", DisplayTypeFront);
   Con::setIntVariable( "$EditTsCtrl::DisplayTypeBack", DisplayTypeBack);
   Con::setIntVariable( "$EditTsCtrl::DisplayTypeLeft", DisplayTypeLeft);
   Con::setIntVariable( "$EditTsCtrl::DisplayTypeRight", DisplayTypeRight);
   Con::setIntVariable( "$EditTsCtrl::DisplayTypePerspective", DisplayTypePerspective);
   Con::setIntVariable( "$EditTsCtrl::DisplayTypeIsometric", DisplayTypeIsometric);
}

//------------------------------------------------------------------------------

void EditTSCtrl::make3DMouseEvent(Gui3DMouseEvent & gui3DMouseEvent, const GuiEvent & event)
{
   (GuiEvent&)(gui3DMouseEvent) = event;
   gui3DMouseEvent.mousePoint = event.mousePoint;

   if(!smCamOrtho)
   {
      // get the eye pos and the mouse vec from that...
      Point3F screenPoint((F32)gui3DMouseEvent.mousePoint.x, (F32)gui3DMouseEvent.mousePoint.y, 1.0f);

      Point3F wp;
      unproject(screenPoint, &wp);

      gui3DMouseEvent.pos = smCamPos;
      gui3DMouseEvent.vec = wp - smCamPos;
      gui3DMouseEvent.vec.normalizeSafe();
   }
   else
   {
      // get the eye pos and the mouse vec from that...
      Point3F screenPoint((F32)gui3DMouseEvent.mousePoint.x, (F32)gui3DMouseEvent.mousePoint.y, 0.0f);

      Point3F np, fp;
      unproject(screenPoint, &np);

      gui3DMouseEvent.pos = np;
      smCamMatrix.getColumn( 1, &(gui3DMouseEvent.vec) );
   }
}

//------------------------------------------------------------------------------

TerrainBlock* EditTSCtrl::getActiveTerrain()
{
   // Find a terrain block
   SimSet* scopeAlwaysSet = Sim::getGhostAlwaysSet();
   for(SimSet::iterator itr = scopeAlwaysSet->begin(); itr != scopeAlwaysSet->end(); itr++)
   {
      TerrainBlock* block = dynamic_cast<TerrainBlock*>(*itr);
      if( block )
         return block;
   }

   return NULL;
}

//------------------------------------------------------------------------------

void EditTSCtrl::setDisplayType( S32 type )
{
   mDisplayType = type;
   
   // Disable middle-mouse pass-thru in ortho views so we can
   // use the middle mouse button for navigation.

   mMiddleMousePassThru = !isOrthoDisplayType();
   
   if( mGizmo )
   {
      // Disable gizmo's grid plane in the isometric views since
      // they will render with the grid from EditTSCtrl.  Also disable
      // the move grid as it doesn't make sense in ortho views.
      
      if( type != DisplayTypePerspective )
      {
         mGizmo->setGridPlaneEnabled( false );
         mGizmo->setMoveGridEnabled( false );
      }
      else
      {
         mGizmo->setGridPlaneEnabled( true );
         mGizmo->setMoveGridEnabled( true );
      }
   }
}

//------------------------------------------------------------------------------

void EditTSCtrl::getCursor(GuiCursor *&cursor, bool &visible, const GuiEvent &event)
{
   make3DMouseEvent(mLastEvent, event);
   get3DCursor(cursor, visible, mLastEvent);
}

void EditTSCtrl::get3DCursor(GuiCursor *&cursor, bool &visible, const Gui3DMouseEvent &event)
{
   TORQUE_UNUSED(event);
   cursor = NULL;
   visible = false;
}

void EditTSCtrl::onMouseUp(const GuiEvent & event)
{
   mLeftMouseDown = false;
   make3DMouseEvent(mLastEvent, event);
   on3DMouseUp(mLastEvent);
}

void EditTSCtrl::onMouseDown(const GuiEvent & event)
{
   mLeftMouseDown = true;
   mLastBorderMoveTime = 0;
   make3DMouseEvent(mLastEvent, event);
   on3DMouseDown(mLastEvent);
   
   setFirstResponder();
}

void EditTSCtrl::onMouseMove(const GuiEvent & event)
{
   make3DMouseEvent(mLastEvent, event);
   on3DMouseMove(mLastEvent);

   mLastMousePos = event.mousePoint;
}

void EditTSCtrl::onMouseDragged(const GuiEvent & event)
{
   make3DMouseEvent(mLastEvent, event);
   on3DMouseDragged(mLastEvent);
}

void EditTSCtrl::onMouseEnter(const GuiEvent & event)
{
   mMouseLeft = false;
   make3DMouseEvent(mLastEvent, event);
   on3DMouseEnter(mLastEvent);
}

void EditTSCtrl::onMouseLeave(const GuiEvent & event)
{
   mMouseLeft = true;
   mLastBorderMoveTime = 0;
   make3DMouseEvent(mLastEvent, event);
   on3DMouseLeave(mLastEvent);
}

void EditTSCtrl::onRightMouseDown(const GuiEvent & event)
{
   // always process the right mouse event first...

   mRightMouseDown = true;
   mLastBorderMoveTime = 0;

   make3DMouseEvent(mLastEvent, event);
   on3DRightMouseDown(mLastEvent);

   if(!mLeftMouseDown && mRightMousePassThru && mProfile->mCanKeyFocus)
   {
      GuiCanvas *pCanvas = getRoot();
      if( !pCanvas )
         return;

      PlatformWindow *pWindow = static_cast<GuiCanvas*>(getRoot())->getPlatformWindow();
      if( !pWindow )
         return;

      PlatformCursorController *pController = pWindow->getCursorController();
      if( !pController )
         return;

      // ok, gotta disable the mouse
      // script functions are lockMouse(true); Canvas.cursorOff();
      pWindow->setMouseLocked(true);
      pCanvas->setCursorON( false );

      if(mDisplayType != DisplayTypePerspective)
      {
         mouseLock();
         mLastMousePos = event.mousePoint;
         pCanvas->setForceMouseToGUI(true);
         mLastMouseClamping = pCanvas->getClampTorqueCursor();
         pCanvas->setClampTorqueCursor(false);
      }

      if(mDisplayType == DisplayTypeIsometric)
      {
         // Store the screen center point on the terrain for a possible rotation
         TerrainBlock* activeTerrain = getActiveTerrain();
         if( activeTerrain )
         {
            F32 extx, exty;
            if(event.modifier & SI_SHIFT)
            {
               extx = F32(event.mousePoint.x);
               exty = F32(event.mousePoint.y);
            }
            else
            {
               extx = getExtent().x * 0.5;
               exty = getExtent().y * 0.5;
            }
            Point3F sp(extx, exty, 0.0f); // Near plane projection
            Point3F start;
            unproject(sp, &start);

            Point3F end = start + mLastEvent.vec * 4000.0f;
            Point3F tStartPnt, tEndPnt;
            activeTerrain->getTransform().mulP(start, &tStartPnt);
            activeTerrain->getTransform().mulP(end, &tEndPnt);

            RayInfo info;
            bool result = activeTerrain->castRay(tStartPnt, tEndPnt, &info);
            if(result)
            {
               info.point.interpolate(start, end, info.t);
               mIsoCamRotCenter = info.point;
            }
            else
            {
               mIsoCamRotCenter = start;
            }
         }
         else
         {
            F32 extx = getExtent().x * 0.5;
            F32 exty = getExtent().y * 0.5;
            Point3F sp(extx, exty, 0.0f); // Near plane projection
            unproject(sp, &mIsoCamRotCenter);
         }
      }

      setFirstResponder();
   }
}

void EditTSCtrl::onRightMouseUp(const GuiEvent & event)
{
   mRightMouseDown = false;
   make3DMouseEvent(mLastEvent, event);
   on3DRightMouseUp(mLastEvent);
}

void EditTSCtrl::onRightMouseDragged(const GuiEvent & event)
{
   make3DMouseEvent(mLastEvent, event);
   on3DRightMouseDragged(mLastEvent);

   // Handle zoom of orthographic views.
   
   if( isOrthoDisplayType() )
   {
      orthoZoom( ( event.mousePoint.y - mLastMousePos.y ) * 0.5f );
      mLastMousePos = event.mousePoint;
   }
}

void EditTSCtrl::onMiddleMouseDown(const GuiEvent & event)
{
   mMiddleMouseDown = true;
   mMiddleMouseTriggered = false;
   mLastBorderMoveTime = 0;

   if(!mLeftMouseDown && !mRightMouseDown && mMiddleMousePassThru && mProfile->mCanKeyFocus)
   {
      GuiCanvas *pCanvas = getRoot();
      if( !pCanvas )
         return;

      PlatformWindow *pWindow = static_cast<GuiCanvas*>(getRoot())->getPlatformWindow();
      if( !pWindow )
         return;

      PlatformCursorController *pController = pWindow->getCursorController();
      if( !pController )
         return;

      // ok, gotta disable the mouse
      // script functions are lockMouse(true); Canvas.cursorOff();
      pWindow->setMouseLocked(true);
      pCanvas->setCursorON( false );

      // Trigger 2 is used by the camera
      MoveManager::mTriggerCount[2]++;
      mMiddleMouseTriggered = true;

      setFirstResponder();
   }
}

void EditTSCtrl::onMiddleMouseUp(const GuiEvent & event)
{
   // Trigger 2 is used by the camera
   if( mMiddleMouseTriggered )
   {
      MoveManager::mTriggerCount[2]++;
      mMiddleMouseTriggered = false;
   }

   mMiddleMouseDown = false;
}

void EditTSCtrl::onMiddleMouseDragged(const GuiEvent & event)
{
   // Handle translation of orthographic views.
   
   if( isOrthoDisplayType() )
   {
      calcOrthoCamOffset((event.mousePoint.x - mLastMousePos.x), (event.mousePoint.y - mLastMousePos.y), event.modifier);
      mLastMousePos = event.mousePoint;
   }
}

bool EditTSCtrl::onMouseWheelUp( const GuiEvent &event )
{
   // Looks like this should be zooming based on a factor of the GuiEvent.fval
   if( isOrthoDisplayType() && !event.modifier )
   {
      orthoZoom( -2.f );
      return true;
   }

   make3DMouseEvent(mLastEvent, event);
   on3DMouseWheelUp(mLastEvent);

   return false;
}

bool EditTSCtrl::onMouseWheelDown( const GuiEvent &event )
{
   // Looks like this should be zooming based on a factor of the GuiEvent.fval
   if(mDisplayType != DisplayTypePerspective && !event.modifier)
   {
      orthoZoom( 2.f );
      return true;
   }

   make3DMouseEvent(mLastEvent, event);
   on3DMouseWheelDown(mLastEvent);

   return false;
}


bool EditTSCtrl::onInputEvent(const InputEventInfo & event)
{

   if(mRightMousePassThru && event.deviceType == MouseDeviceType &&
      event.objInst == KEY_BUTTON1 && event.action == SI_BREAK)
   {
      // if the right mouse pass thru is enabled,
      // we want to reactivate mouse on a right mouse button up
      GuiCanvas *pCanvas = getRoot();
      if( !pCanvas )
         return false;

      PlatformWindow *pWindow = static_cast<GuiCanvas*>(getRoot())->getPlatformWindow();
      if( !pWindow )
         return false;

      PlatformCursorController *pController = pWindow->getCursorController();
      if( !pController )
         return false;

      pWindow->setMouseLocked(false);
      pCanvas->setCursorON( true );

      if(mDisplayType != DisplayTypePerspective)
      {
         mouseUnlock();
         pCanvas->setForceMouseToGUI(false);
         pCanvas->setClampTorqueCursor(mLastMouseClamping);
      }
   }

   if(mMiddleMousePassThru && event.deviceType == MouseDeviceType &&
      event.objInst == KEY_BUTTON2 && event.action == SI_BREAK)
   {
      // if the middle mouse pass thru is enabled,
      // we want to reactivate mouse on a middle mouse button up
      GuiCanvas *pCanvas = getRoot();
      if( !pCanvas )
         return false;

      PlatformWindow *pWindow = static_cast<GuiCanvas*>(getRoot())->getPlatformWindow();
      if( !pWindow )
         return false;

      PlatformCursorController *pController = pWindow->getCursorController();
      if( !pController )
         return false;

      pWindow->setMouseLocked(false);
      pCanvas->setCursorON( true );
   }

   // we return false so that the canvas can properly process the right mouse button up...
   return false;
}

//------------------------------------------------------------------------------

void EditTSCtrl::orthoZoom( F32 steps )
{
   //TODO: this really should be proportional
   
   mOrthoFOV += steps;

   if( mOrthoFOV < 1.0f )
      mOrthoFOV = 1.0f;
}

void EditTSCtrl::calcOrthoCamOffset(F32 mousex, F32 mousey, U8 modifier)
{
   F32 camScale = 0.01f;

   switch(mDisplayType)
   {
      case DisplayTypeTop:
         mOrthoCamTrans.x -= mousex * mOrthoFOV * camScale;
         mOrthoCamTrans.y += mousey * mOrthoFOV * camScale;
         break;

      case DisplayTypeBottom:
         mOrthoCamTrans.x -= mousex * mOrthoFOV * camScale;
         mOrthoCamTrans.y -= mousey * mOrthoFOV * camScale;
         break;

      case DisplayTypeFront:
         mOrthoCamTrans.x += mousex * mOrthoFOV * camScale;
         mOrthoCamTrans.z += mousey * mOrthoFOV * camScale;
         break;

      case DisplayTypeBack:
         mOrthoCamTrans.x -= mousex * mOrthoFOV * camScale;
         mOrthoCamTrans.z += mousey * mOrthoFOV * camScale;
         break;

      case DisplayTypeLeft:
         mOrthoCamTrans.y += mousex * mOrthoFOV * camScale;
         mOrthoCamTrans.z += mousey * mOrthoFOV * camScale;
         break;

      case DisplayTypeRight:
         mOrthoCamTrans.y -= mousex * mOrthoFOV * camScale;
         mOrthoCamTrans.z += mousey * mOrthoFOV * camScale;
         break;

      case DisplayTypeIsometric:
         if(modifier & SI_PRIMARY_CTRL)
         {
            // NOTE: Maybe move the center of rotation code to right mouse down to avoid compound errors?
            F32 rot = mDegToRad(mousex);

            Point3F campos = (mRawCamPos + mOrthoCamTrans) - mIsoCamRotCenter;
            MatrixF mat(EulerF(0, 0, rot));
            mat.mulP(campos);
            mOrthoCamTrans = (campos + mIsoCamRotCenter) - mRawCamPos;
            mIsoCamRot.z += rot;

         }
         else
         {
            mOrthoCamTrans.x -= mousex * mOrthoFOV * camScale * mCos(mIsoCamRot.z) - mousey * mOrthoFOV * camScale * mSin(mIsoCamRot.z);
            mOrthoCamTrans.y += mousex * mOrthoFOV * camScale * mSin(mIsoCamRot.z) + mousey * mOrthoFOV * camScale * mCos(mIsoCamRot.z);
         }
         break;
   }
}

void EditTSCtrl::updateGizmo()
{
   mGizmoProfile->restoreDefaultState();
}

//------------------------------------------------------------------------------

void EditTSCtrl::renderWorld(const RectI & updateRect)
{
   // Make sure that whatever the editor does, it doesn't
   // dirty the GFX transform state.
   GFXTransformSaver saver;

   updateGizmo();

   gClientSceneGraph->setDisplayTargetResolution(getExtent());

   // Use a render instance to do editor 3D scene 
   // rendering after HDR is processed and while the depth 
   // buffer is still intact.
   RenderPassManager *rpm = gClientSceneGraph->getDefaultRenderPass();
   ObjectRenderInst *inst = rpm->allocInst<ObjectRenderInst>();
   inst->type = RenderPassManager::RIT_Editor;
   inst->renderDelegate.bind(this, &EditTSCtrl::_renderScene);
   rpm->addInst(inst);

   if( mDisplayType == DisplayTypePerspective )
      gClientSceneGraph->renderScene( SPT_Diffuse );
   else
   {
      // If we are in an orthographic mode, do a special render
      // with AL, fog, and PostFX disabled.

      FogData savedFogData = gClientSceneGraph->getFogData();
      gClientSceneGraph->setFogData( FogData() );

      SceneRenderState renderState
      (
         gClientSceneGraph,
         SPT_Diffuse
      );

      gClientSceneGraph->renderScene( &renderState );
      gClientSceneGraph->setFogData( savedFogData );
   }
}

void EditTSCtrl::_renderScene( ObjectRenderInst*, SceneRenderState *state, BaseMatInstance* )
{
   GFXTransformSaver saver;

   // render through console callbacks
   SimSet * missionGroup = static_cast<SimSet*>(Sim::findObject("MissionGroup"));
   if(missionGroup)
   {
      mConsoleRendering = true;

      // [ rene, 27-Jan-10 ] This calls onEditorRender on the server objects instead
      //    of on the client objects which seems a bit questionable to me.
 
      for(SimSetIterator itr(missionGroup); *itr; ++itr)
      {
         SceneObject* object = dynamic_cast< SceneObject* >( *itr );
         if( object && object->isRenderEnabled() && !object->isHidden() )
         {
            char buf[2][16];
            dSprintf(buf[0], 16, object->isSelected() ? "true" : "false");
            dSprintf(buf[1], 16, object->isExpanded() ? "true" : "false");
            
            Con::executef( object, "onEditorRender", getIdString(), buf[0], buf[1] );
         }
      }

      mConsoleRendering = false;
   }

   // render the mission area...
   renderMissionArea();

   // Draw the grid
   if ( mRenderGridPlane )
      renderGrid();

   // render the editor stuff
   renderScene(mSaveViewport);

	// Draw the camera axis
   GFX->setClipRect(mSaveViewport);
   GFX->setStateBlock(mDefaultGuiSB);
   renderCameraAxis();
}

void EditTSCtrl::renderMissionArea()
{
   MissionArea* obj = MissionArea::getServerObject();
   if ( !obj )
      return;

   if ( !mRenderMissionArea && !obj->isSelected() )
      return;

   GFXDEBUGEVENT_SCOPE( Editor_renderMissionArea, ColorI::WHITE );

   F32 minHeight = 0.0f;
   F32 maxHeight = 0.0f;

   TerrainBlock* terrain = getActiveTerrain();
   if ( terrain )
   {
      terrain->getMinMaxHeight( &minHeight, &maxHeight );
      Point3F pos = terrain->getPosition();

      maxHeight += pos.z + mMissionAreaHeightAdjust;
      minHeight += pos.z - mMissionAreaHeightAdjust;
   }

   const RectI& area = obj->getArea();
   Box3F areaBox( area.point.x,
                  area.point.y,
                  minHeight,
                  area.point.x + area.extent.x,
                  area.point.y + area.extent.y,
                  maxHeight );

   GFXDrawUtil* drawer = GFX->getDrawUtil();

   GFXStateBlockDesc desc;
   desc.setCullMode( GFXCullNone );
   desc.setBlend( true );
   desc.setZReadWrite( false, false );

   desc.setFillModeSolid();
   drawer->drawCube( desc, areaBox, mMissionAreaFillColor );

   desc.setFillModeWireframe();
   drawer->drawCube( desc, areaBox, mMissionAreaFrameColor );
}

void EditTSCtrl::renderCameraAxis()
{
   GFXDEBUGEVENT_SCOPE( Editor_renderCameraAxis, ColorI::WHITE );

   static MatrixF sRotMat(EulerF( (M_PI_F / -2.0f), 0.0f, 0.0f));

   MatrixF camMat = mLastCameraQuery.cameraMatrix;
   camMat.mul(sRotMat);
   camMat.inverse();

   MatrixF axis;
   axis.setColumn(0, Point3F(1, 0, 0));
   axis.setColumn(1, Point3F(0, 0, 1));
   axis.setColumn(2, Point3F(0, -1, 0));
   axis.mul(camMat);

   Point3F forwardVec, upVec, rightVec;
	axis.getColumn( 2, &forwardVec );
	axis.getColumn( 1, &upVec );
	axis.getColumn( 0, &rightVec );

   Point2I pos = getPosition();
	F32 offsetx = pos.x + 20.0;
	F32 offsety = pos.y + getExtent().y - 42.0; // Take the status bar into account
	F32 scale = 15.0;

   // Generate correct drawing order
   ColorI c1(255,0,0);
   ColorI c2(0,255,0);
   ColorI c3(0,0,255);
	ColorI tc;
	Point3F *p1, *p2, *p3, *tp;
	p1 = &rightVec;
	p2 = &upVec;
	p3 = &forwardVec;
	if(p3->y > p2->y)
	{
		tp = p2; tc = c2;
		p2 = p3; c2 = c3;
		p3 = tp; c3 = tc;
	}
	if(p2->y > p1->y)
	{
		tp = p1; tc = c1;
		p1 = p2; c1 = c2;
		p2 = tp; c2 = tc;
	}

   PrimBuild::begin( GFXLineList, 6 );
		//*** Axis 1
		PrimBuild::color(c1);
		PrimBuild::vertex3f(offsetx, offsety, 0);
		PrimBuild::vertex3f(offsetx+p1->x*scale, offsety-p1->z*scale, 0);

		//*** Axis 2
		PrimBuild::color(c2);
		PrimBuild::vertex3f(offsetx, offsety, 0);
		PrimBuild::vertex3f(offsetx+p2->x*scale, offsety-p2->z*scale, 0);

		//*** Axis 3
		PrimBuild::color(c3);
		PrimBuild::vertex3f(offsetx, offsety, 0);
		PrimBuild::vertex3f(offsetx+p3->x*scale, offsety-p3->z*scale, 0);
   PrimBuild::end();
}

void EditTSCtrl::renderGrid()
{
   if( !isOrthoDisplayType() )
      return;

   GFXDEBUGEVENT_SCOPE( Editor_renderGrid, ColorI::WHITE );

   // Calculate the displayed grid size based on view
   F32 drawnGridSize = mGridPlaneSize;
   F32 gridPixelSize = projectRadius(1.0f, mGridPlaneSize);
   if(gridPixelSize < mGridPlaneSizePixelBias)
   {
      U32 counter = 1;
      while(gridPixelSize < mGridPlaneSizePixelBias)
      {
         drawnGridSize = mGridPlaneSize * counter * 10.0f;
         gridPixelSize = projectRadius(1.0f, drawnGridSize);

         ++counter;

         // No infinite loops here
         if(counter > 1000)
            break;
      }
   }

   F32 minorTickSize = 0;
   F32 gridSize = drawnGridSize;
   U32 minorTickMax = mGridPlaneMinorTicks + 1;
   if(minorTickMax > 0)
   {
      minorTickSize = drawnGridSize;
      gridSize = drawnGridSize * minorTickMax;
   }

   // Build the view-based origin
   VectorF dir;
   smCamMatrix.getColumn( 1, &dir );

   Point3F gridPlanePos = smCamPos + dir;
   Point2F size(mOrthoWidth + 2 * gridSize, mOrthoHeight + 2 * gridSize);

   GFXStateBlockDesc desc;
   desc.setBlend( true );
   desc.setZReadWrite( true, false );

   GFXDrawUtil::Plane plane = GFXDrawUtil::PlaneXY;
   switch( getDisplayType() )
   {
      case DisplayTypeTop:
      case DisplayTypeBottom:
         plane = GFXDrawUtil::PlaneXY;
         break;

      case DisplayTypeLeft:
      case DisplayTypeRight:
         plane = GFXDrawUtil::PlaneYZ;
         break;

      case DisplayTypeFront:
      case DisplayTypeBack:
         plane = GFXDrawUtil::PlaneXZ;
         break;
         
      default:
         break;
   }

   GFX->getDrawUtil()->drawPlaneGrid( desc, gridPlanePos, size, Point2F( minorTickSize, minorTickSize ), mGridPlaneMinorTickColor, plane );
   GFX->getDrawUtil()->drawPlaneGrid( desc, gridPlanePos, size, Point2F( gridSize, gridSize ), mGridPlaneColor, plane );
}

static void sceneBoundsCalcCallback(SceneObject* obj, void *key)
{
   // Early out for those objects that slipped through the mask check
   // because they belong to more than one type.
   if((obj->getTypeMask() & EditTSCtrl::smSceneBoundsMask) != 0)
      return;

   if(obj->isGlobalBounds())
      return;

   Box3F* bounds = (Box3F*)key;

   Point3F min = obj->getWorldBox().minExtents;
   Point3F max = obj->getWorldBox().maxExtents;

   #if 0 // Console messages in render loop lead to massive spam.
   if (min.x <= -5000.0f || min.y <= -5000.0f || min.z <= -5000.0f ||
       min.x >=  5000.0f || min.y >=  5000.0f || min.z >=  5000.0f)
       Con::errorf("SceneObject %d (%s : %s) has a bounds that could cause problems with a non-perspective view", obj->getId(), obj->getClassName(), obj->getName());
   #endif

   bounds->minExtents.setMin(min);
   bounds->minExtents.setMin(max);
   bounds->maxExtents.setMax(min);
   bounds->maxExtents.setMax(max);
}

bool EditTSCtrl::getCameraTransform(MatrixF* cameraMatrix)
{
   GameConnection* connection = dynamic_cast<GameConnection *>(NetConnection::getConnectionToServer());
   return (connection && connection->getControlCameraTransform(0.032f, cameraMatrix));
}

void EditTSCtrl::computeSceneBounds(Box3F& bounds)
{
   bounds.minExtents.set(1e10, 1e10, 1e10);
   bounds.maxExtents.set(-1e10, -1e10, -1e10);

   // Calculate the scene bounds
   gClientContainer.findObjects(~(smSceneBoundsMask), sceneBoundsCalcCallback, &bounds);
}

bool EditTSCtrl::processCameraQuery(CameraQuery * query)
{
   if(mDisplayType == DisplayTypePerspective)
   {
      query->ortho = false;
   }
   else
   {
      query->ortho = true;
   }

   if (getCameraTransform(&query->cameraMatrix))
   {
      query->farPlane = gClientSceneGraph->getVisibleDistance() * smVisibleDistanceScale;
      query->nearPlane = gClientSceneGraph->getNearClip();
      query->fov = mDegToRad(smCamFOV);

      if(query->ortho)
      {
         MatrixF camRot(true);
         const F32 camBuffer = 1.0f;
         Point3F camPos = query->cameraMatrix.getPosition();

         F32 isocamplanedist = 0.0f;
         if(mDisplayType == DisplayTypeIsometric)
         {
            const RectI& vp = GFX->getViewport();
            isocamplanedist = 0.25 * vp.extent.y * mSin(mIsoCamAngle);
         }

         // Calculate the scene bounds
         Box3F sceneBounds;
         computeSceneBounds(sceneBounds);

         if(!sceneBounds.isValidBox())
         {
            sceneBounds.maxExtents = camPos + smMinSceneBounds;
            sceneBounds.minExtents = camPos - smMinSceneBounds;
         }
         else
         {
            query->farPlane = getMax(smMinSceneBounds.x * 2.0f, (sceneBounds.maxExtents - sceneBounds.minExtents).len() + camBuffer * 2.0f + isocamplanedist);
         }

         mRawCamPos = camPos;
         camPos += mOrthoCamTrans;

         switch(mDisplayType)
         {
            case DisplayTypeTop:
               camRot.setColumn(0, Point3F(1.0, 0.0,  0.0));
               camRot.setColumn(1, Point3F(0.0, 0.0, -1.0));
               camRot.setColumn(2, Point3F(0.0, 1.0,  0.0));
               camPos.z = getMax(camPos.z + smMinSceneBounds.z, sceneBounds.maxExtents.z + camBuffer);
               break;

            case DisplayTypeBottom:
               camRot.setColumn(0, Point3F(1.0,  0.0,  0.0));
               camRot.setColumn(1, Point3F(0.0,  0.0,  1.0));
               camRot.setColumn(2, Point3F(0.0, -1.0,  0.0));
               camPos.z = getMin(camPos.z - smMinSceneBounds.z, sceneBounds.minExtents.z - camBuffer);
               break;

            case DisplayTypeFront:
               camRot.setColumn(0, Point3F(-1.0,  0.0,  0.0));
               camRot.setColumn(1, Point3F( 0.0, -1.0,  0.0));
               camRot.setColumn(2, Point3F( 0.0,  0.0,  1.0));
               camPos.y = getMax(camPos.y + smMinSceneBounds.y, sceneBounds.maxExtents.y + camBuffer);
               break;

            case DisplayTypeBack:
               camRot.setColumn(0, Point3F(1.0,  0.0,  0.0));
               camRot.setColumn(1, Point3F(0.0,  1.0,  0.0));
               camRot.setColumn(2, Point3F(0.0,  0.0,  1.0));
               camPos.y = getMin(camPos.y - smMinSceneBounds.y, sceneBounds.minExtents.y - camBuffer);
               break;

            case DisplayTypeLeft:
               camRot.setColumn(0, Point3F( 0.0, -1.0,  0.0));
               camRot.setColumn(1, Point3F( 1.0,  0.0,  0.0));
               camRot.setColumn(2, Point3F( 0.0,  0.0,  1.0));
               camPos.x = getMin(camPos.x - smMinSceneBounds.x, sceneBounds.minExtents.x - camBuffer);
               break;

            case DisplayTypeRight:
               camRot.setColumn(0, Point3F( 0.0,  1.0,  0.0));
               camRot.setColumn(1, Point3F(-1.0,  0.0,  0.0));
               camRot.setColumn(2, Point3F( 0.0,  0.0,  1.0));
               camPos.x = getMax(camPos.x + smMinSceneBounds.x, sceneBounds.maxExtents.x + camBuffer);
               break;

            case DisplayTypeIsometric:
               camPos.z = sceneBounds.maxExtents.z + camBuffer + isocamplanedist;
               MatrixF angle(EulerF(mIsoCamAngle, 0, 0));
               MatrixF rot(mIsoCamRot);
               camRot.mul(rot, angle);
               break;
         }

         query->cameraMatrix = camRot;
         query->cameraMatrix.setPosition(camPos);
         query->headMatrix = query->cameraMatrix;
         query->fov = mOrthoFOV;
      }

      smCamMatrix = query->cameraMatrix;
      smCamMatrix.getColumn(3,&smCamPos);
      smCamOrtho = query->ortho;
      smCamNearPlane = query->nearPlane;

      return true;
   }
   return false;
}

//------------------------------------------------------------------------------
DefineEngineMethod( EditTSCtrl, getDisplayType, S32, (),, "" )
{
   return object->getDisplayType();
}

DefineEngineMethod( EditTSCtrl, setDisplayType, void, ( S32 displayType ),, "" )
{
   object->setDisplayType( displayType );
}

DefineEngineMethod( EditTSCtrl, getOrthoFOV, F32, (),,
   "Return the FOV for orthographic views." )
{
   return object->getOrthoFOV();
}

DefineEngineMethod( EditTSCtrl, setOrthoFOV, void, ( F32 fov ),,
   "Set the FOV for to use for orthographic views." )
{
   object->setOrthoFOV( fov );
}

DefineEngineMethod( EditTSCtrl, renderBox, void, ( Point3F pos, Point3F size ),, "" )
{
   if( !object->mConsoleRendering || !object->mConsoleFillColor.alpha )
      return;

   GFXStateBlockDesc desc;
   desc.setBlend( true );

   Box3F box;
   box.set( size );
   box.setCenter( pos );

   MatrixF camera = GFX->getWorldMatrix();
   camera.inverse();

   if( box.isContained( camera.getPosition() ) )
      desc.setCullMode( GFXCullNone );

   GFX->getDrawUtil()->drawCube( desc, size, pos, object->mConsoleFillColor );
}

DefineEngineMethod(EditTSCtrl, renderSphere, void, ( Point3F pos, F32 radius, S32 sphereLevel ), ( 0 ), "" )
{
   if ( !object->mConsoleRendering || !object->mConsoleFillColor.alpha )
      return;

   // TODO: We need to support subdivision levels in GFXDrawUtil!
   if ( sphereLevel <= 0 )
      sphereLevel = object->mConsoleSphereLevel;

   GFXStateBlockDesc desc;
   desc.setBlend( true );

   MatrixF camera = GFX->getWorldMatrix();
   camera.inverse();
   
   SphereF sphere( pos, radius );
   if( sphere.isContained( camera.getPosition() ) )
      desc.setCullMode( GFXCullNone );

   GFX->getDrawUtil()->drawSphere( desc, radius, pos, object->mConsoleFillColor );
}

DefineEngineMethod( EditTSCtrl, renderCircle, void, ( Point3F pos, Point3F normal, F32 radius, S32 segments ), ( 0 ), "" )
{
   if(!object->mConsoleRendering)
      return;

   if(!object->mConsoleFrameColor.alpha && !object->mConsoleFillColor.alpha)
      return;

   if ( segments <= 0 )
      segments = object->mConsoleCircleSegments;

   normal.normalizeSafe();

   AngAxisF aa;

   F32 dotUp = mDot( normal, Point3F(0,0,1) );
   if ( dotUp == 1.0f )
      aa.set( Point3F(0,0,1), 0.0f );     // normal is 0,0,1
   else if ( dotUp == -1.0f )
      aa.set( Point3F(1,0,0), M_PI_F );   // normal is 0,0,-1
   else
   {
      mCross( normal, Point3F(0,0,1), &aa.axis );
      aa.axis.normalizeSafe();
      aa.angle = mAcos( mClampF( dotUp, -1.f, 1.f ) );
   }

   MatrixF mat;
   aa.setMatrix(&mat);

   F32 step = M_2PI / segments;
   F32 angle = 0.f;

   Vector<Point3F> points(segments);
   for(U32 i = 0; i < segments; i++)
   {
      Point3F pnt(mCos(angle), mSin(angle), 0.f);

      mat.mulP(pnt);
      pnt *= radius;
      pnt += pos;

      points.push_front(pnt);
      angle += step;
   }

   GFX->setStateBlock(object->mBlendSB);

   // framed
   if(object->mConsoleFrameColor.alpha)
   {
      // TODO: Set GFX line width (when it exists) to the value of 'object->mConsoleLineWidth'

      PrimBuild::color( object->mConsoleFrameColor );

      PrimBuild::begin( GFXLineStrip, points.size() + 1 );

      for( S32 i = 0; i < points.size(); i++ )
         PrimBuild::vertex3fv( points[i] );

      // GFX does not have a LineLoop primitive, so connect the last line
      if( points.size() > 0 )
         PrimBuild::vertex3fv( points[0] );

      PrimBuild::end();

      // TODO: Reset GFX line width here
   }

   // filled
   if(object->mConsoleFillColor.alpha)
   {
      PrimBuild::color( object->mConsoleFillColor );

      PrimBuild::begin( GFXTriangleStrip, points.size() + 2 );

      // Center point
      PrimBuild::vertex3fv( pos );

      // Edge verts
      for( S32 i = 0; i < points.size(); i++ )
         PrimBuild::vertex3fv( points[i] );

      PrimBuild::vertex3fv( points[0] );

      PrimBuild::end();
   }
}

DefineEngineMethod( EditTSCtrl, renderTriangle, void, ( Point3F a, Point3F b, Point3F c ),, "" )
{
   if(!object->mConsoleRendering)
      return;

   if(!object->mConsoleFrameColor.alpha && !object->mConsoleFillColor.alpha)
      return;

   const Point3F* pnts[3] = { &a, &b, &c };

   GFX->setStateBlock(object->mBlendSB);

   // frame
   if( object->mConsoleFrameColor.alpha )
   {
      PrimBuild::color( object->mConsoleFrameColor );
  
      // TODO: Set GFX line width (when it exists) to the value of 'object->mConsoleLineWidth'

      PrimBuild::begin( GFXLineStrip, 4 );
         PrimBuild::vertex3fv( *pnts[0] );
         PrimBuild::vertex3fv( *pnts[1] );
         PrimBuild::vertex3fv( *pnts[2] );
         PrimBuild::vertex3fv( *pnts[0] );
      PrimBuild::end();

      // TODO: Reset GFX line width here
   }

   // fill
   if( object->mConsoleFillColor.alpha )
   {
      PrimBuild::color( object->mConsoleFillColor );

      PrimBuild::begin( GFXTriangleList, 3 );
         PrimBuild::vertex3fv( *pnts[0] );
         PrimBuild::vertex3fv( *pnts[1] );
         PrimBuild::vertex3fv( *pnts[2] );
      PrimBuild::end();
   }
}

DefineEngineMethod( EditTSCtrl, renderLine, void, ( Point3F start, Point3F end, F32 lineWidth ), ( 0 ), "" )
{
   if ( !object->mConsoleRendering || !object->mConsoleFrameColor.alpha )
      return;

   // TODO: We don't support 3d lines with width... fix this!
   if ( lineWidth <= 0 )
      lineWidth = object->mConsoleLineWidth;

   GFX->getDrawUtil()->drawLine( start, end, object->mConsoleFrameColor );
}

DefineEngineMethod( EditTSCtrl, getGizmo, S32, (),, "" )
{
   return object->getGizmo()->getId();
}

DefineEngineMethod( EditTSCtrl, isMiddleMouseDown, bool, (),, "" )
{
   return object->isMiddleMouseDown();
}
