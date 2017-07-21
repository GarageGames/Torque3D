
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// Arcane-FX for MIT Licensed Open Source version of Torque 3D from GarageGames
// Copyright (C) 2015 Faust Logic, Inc.
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
//
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// Some of the object selection code in this file is based on functionality described
// in the following resource:
//
// Object Selection in Torque by Dave Myers 
//   http://www.garagegames.com/index.php?sec=mg&mod=resource&page=view&qid=7335
//
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#include "afx/arcaneFX.h"

#include "console/engineAPI.h"
#include "gui/core/guiCanvas.h"
#include "T3D/gameBase/gameConnection.h"
#include "T3D/gameFunctions.h"

#include "afx/ui/afxTSCtrl.h"
#include "afx/afxSpellBook.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

IMPLEMENT_CONOBJECT(afxTSCtrl);

ConsoleDocClass( afxTSCtrl,
   "@brief A customized variation of GameTSCtrl.\n\n"

   "@ingroup afxGUI\n"
   "@ingroup AFX\n"
);

afxTSCtrl::afxTSCtrl()
{
  mMouse3DVec.zero();
  mMouse3DPos.zero();
  mouse_dn_timestamp = 0;
  spellbook = NULL;

  clearTargetingMode();
}

bool afxTSCtrl::processCameraQuery(CameraQuery *camq)
{
  GameUpdateCameraFov();
  return GameProcessCameraQuery(camq);
}

void afxTSCtrl::renderWorld(const RectI &updateRect)
{
  GameRenderWorld();
}

void afxTSCtrl::getCursor(GuiCursor *&cursor, bool &showCursor, const GuiEvent &lastGuiEvent)
{
  Parent::getCursor(cursor, showCursor, lastGuiEvent);

  GameConnection* conn = GameConnection::getConnectionToServer();
  if (!conn || !conn->getRolloverObj())
    return;

  GuiCanvas *pRoot = getRoot();
  if( !pRoot )
    return;

  PlatformWindow* pWindow = pRoot->getPlatformWindow();
  AssertFatal(pWindow != NULL, "GuiControl without owning platform window!  This should not be possible.");
  PlatformCursorController* pController = pWindow->getCursorController();
  AssertFatal(pController != NULL, "PlatformWindow without an owned CursorController!");

  if(pRoot->mCursorChanged != PlatformCursorController::curHand)
  {
    // We've already changed the cursor, so set it back before we change it again.
    if(pRoot->mCursorChanged != -1)
      pController->popCursor();

    // Now change the cursor shape
    pController->pushCursor(PlatformCursorController::curHand);
    pRoot->mCursorChanged = PlatformCursorController::curHand;
  }
  else if(pRoot->mCursorChanged != -1)
  {
    // Restore the cursor we changed
    pController->popCursor();
    pRoot->mCursorChanged = -1;
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

void afxTSCtrl::onMouseDown(const GuiEvent &evt)
{   
  //Con::printf("#### afxTSCtrl::onLeftMouseDown() ####");

  // save a timestamp so we can measure how long the button is down
  mouse_dn_timestamp = Platform::getRealMilliseconds();

  GuiCanvas* Canvas = getRoot();

  // clear button down status because the ActionMap is going to capture
  // the mouse and the button up will never happen
  Canvas->clearMouseButtonDown();

  // indicate that processing of event should continue (pass down to ActionMap)
  Canvas->setConsumeLastInputEvent(false);
}

void afxTSCtrl::onRightMouseDown(const GuiEvent& evt)
{
  //Con::printf("#### afxTSCtrl::onRightMouseDown() ####");

  GuiCanvas* Canvas = getRoot();

  // clear right button down status because the ActionMap is going to capture
  // the mouse and the right button up will never happen
  Canvas->clearMouseRightButtonDown();

  // indicate that processing of event should continue (pass down to ActionMap)
  Canvas->setConsumeLastInputEvent(false);
}

void afxTSCtrl::onMouseMove(const GuiEvent& evt)
{
  AssertFatal(!targeting_mode.empty(), "Error, undefined targeting mode.");
 
  Targeting targeting = targeting_mode.last();
  if (targeting.mode == arcaneFX::TARGETING_OFF || targeting.check != arcaneFX::TARGET_CHECK_ON_MOUSE_MOVE)
    return;

  performTargeting(evt.mousePoint, targeting.mode);
}

void afxTSCtrl::onRender(Point2I offset, const RectI &updateRect)
{
  GameConnection* con = GameConnection::getConnectionToServer();
#if defined(BROKEN_DAMAGEFLASH_WHITEOUT_BLACKOUT)
  bool skipRender = (!con);
#else
  bool skipRender = (!con || 
                     (con->getWhiteOut() >= 1.f) || 
                     (con->getDamageFlash() >= 1.f) || 
                     (con->getBlackOut() >= 1.f));
#endif

  if (!skipRender)
    Parent::onRender(offset, updateRect);

  GFX->setViewport(updateRect);
}

void afxTSCtrl::advanceTime(F32 dt) 
{
  AssertFatal(!targeting_mode.empty(), "Error, undefined targeting mode.");

  Targeting targeting = targeting_mode.last();
  if (targeting.mode == arcaneFX::TARGETING_OFF || targeting.check != arcaneFX::TARGET_CHECK_POLL)
    return;

  GuiCanvas* Canvas = getRoot();

  Point2I cursor_pos;
  if (Canvas && Canvas->getLastCursorPoint(cursor_pos))
  {
    performTargeting(cursor_pos, targeting.mode);
  }
};

void afxTSCtrl::performTargeting(const Point2I& mousePoint, U8 mode)
{
  GuiCanvas* Canvas = getRoot();

  if (mode != arcaneFX::TARGETING_FREE && !Canvas->isCursorON())
    return;

  MatrixF cam_xfm;
  Point3F dummy_pt;
  if (GameGetCameraTransform(&cam_xfm, &dummy_pt))    
  {      
    // get cam pos 
    Point3F cameraPoint; cam_xfm.getColumn(3,&cameraPoint); 

    // construct 3D screen point from mouse coords 
    Point3F screen_pt((F32)mousePoint.x, (F32)mousePoint.y, 1.0f);  

    // convert screen point to world point
    bool bad_cam = mIsZero(mLastCameraQuery.farPlane);
    Point3F world_pt;      
    if (!bad_cam && unproject(screen_pt, &world_pt))       
    {         
      Point3F mouseVec = world_pt - cameraPoint;         
      mouseVec.normalizeSafe();    
      
      mMouse3DPos = cameraPoint;
      mMouse3DVec = mouseVec;

      F32 selectRange = arcaneFX::sTargetSelectionRange;
      Point3F mouseScaled = mouseVec*selectRange;
      Point3F rangeEnd = cameraPoint + mouseScaled;

      if (mode == arcaneFX::TARGETING_STANDARD)
        arcaneFX::rolloverRayCast(cameraPoint, rangeEnd, arcaneFX::sTargetSelectionMask);
      else if (mode == arcaneFX::TARGETING_FREE)
        arcaneFX::freeTargetingRayCast(cameraPoint, rangeEnd, arcaneFX::sFreeTargetSelectionMask);
    }   
  }
}

void afxTSCtrl::onMouseEnter(const GuiEvent& evt)
{
  //Con::printf("#### afxTSCtrl::onMouseEnter() ####");
}

void afxTSCtrl::onMouseDragged(const GuiEvent& evt)
{
  //Con::printf("#### afxTSCtrl::onMouseDragged() ####");
}


void afxTSCtrl::onMouseLeave(const GuiEvent& evt)
{
  //Con::printf("#### afxTSCtrl::onMouseLeave() ####");
}

bool afxTSCtrl::onMouseWheelUp(const GuiEvent& evt)
{
  //Con::printf("#### afxTSCtrl::onMouseWheelUp() ####");
  Con::executef(this, "onMouseWheelUp");
  return true;
}

bool afxTSCtrl::onMouseWheelDown(const GuiEvent& evt)
{
  //Con::printf("#### afxTSCtrl::onMouseWheelDown() ####");
  Con::executef(this, "onMouseWheelDown");
  return true;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

void afxTSCtrl::setSpellBook(afxSpellBook* book)
{
  if (book != spellbook)
  {
    spellbook = book;
    Con::executef(this, "onSpellbookChange", (spellbook) ? spellbook->getIdString() : "");
  }
}

void afxTSCtrl::clearTargetingMode()
{
  targeting_mode.clear();
  pushTargetingMode(arcaneFX::TARGETING_OFF, arcaneFX::TARGET_CHECK_POLL);
}

void afxTSCtrl::pushTargetingMode(U8 mode, U8 check)
{
  switch (mode)
  {
  case arcaneFX::TARGETING_OFF:
  case arcaneFX::TARGETING_STANDARD:
  case arcaneFX::TARGETING_FREE:
    break;
  default:
    Con::errorf("afxTSCtrl::setTargetingMode() -- unknown targeting mode [%d].", mode);
    return;
  }

  switch (check)
  {
  case arcaneFX::TARGET_CHECK_POLL:
  case arcaneFX::TARGET_CHECK_ON_MOUSE_MOVE:
    break;
  default:
    Con::errorf("afxTSCtrl::setTargetingMode() -- unknown targeting check method [%d].", check);
    return;
  }
  
  Targeting targeting = { mode, check };
  targeting_mode.push_back(targeting);
}

void afxTSCtrl::popTargetingMode()
{
  if (targeting_mode.size() <= 1)
    return ;

  targeting_mode.pop_back();
}

U8 afxTSCtrl::getTargetingMode()
{
  return (targeting_mode.size() > 0) ? targeting_mode.last().mode : arcaneFX::TARGETING_OFF;
}

U8 afxTSCtrl::getTargetingCheckMethod()
{
  return (targeting_mode.size() > 0) ? targeting_mode.last().check : arcaneFX::TARGET_CHECK_POLL;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

DefineEngineMethod(afxTSCtrl, setSpellBook, void, (afxSpellBook* spellbook),,
                   "Associate a spellbook with an afxTSCtrl.\n\n"
                   "@ingroup AFX")
{
  object->setSpellBook(spellbook);
}

DefineEngineMethod(afxTSCtrl, pushTargetingMode, void, (unsigned int mode, unsigned int checkMethod), (arcaneFX::TARGET_CHECK_POLL),
                   "Push a new targeting-mode onto a statck of modes.\n\n"
                   "@ingroup AFX")
{
  object->pushTargetingMode((U8)mode, (U8)checkMethod);
}

DefineEngineMethod(afxTSCtrl, popTargetingMode, void, (),,
                   "Pop the targeting-mode off a statck of modes.\n\n"
                   "@ingroup AFX")
{
  object->popTargetingMode();
}

DefineEngineMethod(afxTSCtrl, getTargetingMode, S32, (),,
                   "Get the current targeting-mode.\n\n"
                   "@ingroup AFX")
{
  return object->getTargetingMode();
}

DefineEngineMethod(afxTSCtrl, getMouse3DVec, Point3F, (),,
                  "Get the 3D direction vector for the mouse cursor.\n\n"
                  "@ingroup AFX")
{
  return object->getMouse3DVec();
}

DefineEngineMethod(afxTSCtrl, getMouse3DPos, Point3F, (),,
                   "Get the 3D position of the mouse cursor.\n\n"
                   "@ingroup AFX")
{
  return object->getMouse3DPos();
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//