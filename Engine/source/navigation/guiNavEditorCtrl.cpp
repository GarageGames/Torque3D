//-----------------------------------------------------------------------------
// Copyright (c) 2014 Daniel Buckmaster
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
#include "guiNavEditorCtrl.h"
#include "duDebugDrawTorque.h"
#include "console/engineAPI.h"

#include "console/consoleTypes.h"
#include "math/util/frustum.h"
#include "math/mathUtils.h"
#include "gfx/primBuilder.h"
#include "gfx/gfxDrawUtil.h"
#include "scene/sceneRenderState.h"
#include "scene/sceneManager.h"
#include "gui/core/guiCanvas.h"
#include "gui/buttons/guiButtonCtrl.h"
#include "gui/worldEditor/undoActions.h"
#include "T3D/gameBase/gameConnection.h"

IMPLEMENT_CONOBJECT(GuiNavEditorCtrl);

ConsoleDocClass(GuiNavEditorCtrl,
                "@brief GUI tool that makes up the Navigation Editor\n\n"
                "Editor use only.\n\n"
                "@internal"
                );

// Each of the mode names directly correlates with the Nav Editor's tool palette.
const String GuiNavEditorCtrl::mSelectMode = "SelectMode";
const String GuiNavEditorCtrl::mLinkMode = "LinkMode";
const String GuiNavEditorCtrl::mCoverMode = "CoverMode";
const String GuiNavEditorCtrl::mTileMode = "TileMode";
const String GuiNavEditorCtrl::mTestMode = "TestMode";

GuiNavEditorCtrl::GuiNavEditorCtrl()
{
   mMode = mSelectMode;
   mIsDirty = false;
   mStartDragMousePoint = InvalidMousePoint;
   mMesh = NULL;
   mCurTile = mTile = -1;
   mPlayer = mCurPlayer = NULL;
   mSpawnClass = mSpawnDatablock = "";
   mLinkStart = Point3F::Max;
   mLink = mCurLink = -1;
}

GuiNavEditorCtrl::~GuiNavEditorCtrl()
{
}

void GuiNavEditorUndoAction::undo()
{
}

bool GuiNavEditorCtrl::onAdd()
{
   if(!Parent::onAdd())
      return false;

   GFXStateBlockDesc desc;
   desc.fillMode = GFXFillSolid;
   desc.setBlend(false);
   desc.setZReadWrite(false, false);
   desc.setCullMode(GFXCullNone);

   mZDisableSB = GFX->createStateBlock(desc);

   desc.setZReadWrite(true, true);
   mZEnableSB = GFX->createStateBlock(desc);

   SceneManager::getPreRenderSignal().notify(this, &GuiNavEditorCtrl::_prepRenderImage);

   return true;
}

void GuiNavEditorCtrl::initPersistFields()
{
   addField("isDirty", TypeBool, Offset(mIsDirty, GuiNavEditorCtrl));

   addField("spawnClass", TypeRealString, Offset(mSpawnClass, GuiNavEditorCtrl),
      "Class of object to spawn in test mode.");
   addField("spawnDatablock", TypeRealString, Offset(mSpawnDatablock, GuiNavEditorCtrl),
      "Datablock to give new objects in test mode.");

   Parent::initPersistFields();
}

void GuiNavEditorCtrl::onSleep()
{
   Parent::onSleep();

   //mMode = mSelectMode;
}

void GuiNavEditorCtrl::selectMesh(NavMesh *mesh)
{
   mesh->setSelected(true);
   mMesh = mesh;
}

DefineEngineMethod(GuiNavEditorCtrl, selectMesh, void, (S32 id),,
   "@brief Select a NavMesh object.")
{
   NavMesh *obj;
   if(Sim::findObject(id, obj))
      object->selectMesh(obj);
}

S32 GuiNavEditorCtrl::getMeshId()
{
   return mMesh.isNull() ? 0 : mMesh->getId();
}

DefineEngineMethod(GuiNavEditorCtrl, getMesh, S32, (),,
   "@brief Select a NavMesh object.")
{
   return object->getMeshId();
}

S32 GuiNavEditorCtrl::getPlayerId()
{
   return mPlayer.isNull() ? 0 : mPlayer->getId();
}

DefineEngineMethod(GuiNavEditorCtrl, getPlayer, S32, (),,
   "@brief Select a NavMesh object.")
{
   return object->getPlayerId();
}

void GuiNavEditorCtrl::deselect()
{
   if(!mMesh.isNull())
      mMesh->setSelected(false);
   mMesh = NULL;
   mPlayer = mCurPlayer = NULL;
   mCurTile = mTile = -1;
   mLinkStart = Point3F::Max;
   mLink = mCurLink = -1;
}

DefineEngineMethod(GuiNavEditorCtrl, deselect, void, (),,
   "@brief Deselect whatever is currently selected in the editor.")
{
   object->deselect();
}

void GuiNavEditorCtrl::deleteLink()
{
   if(!mMesh.isNull() && mLink != -1)
   {
      mMesh->selectLink(mLink, false);
      mMesh->deleteLink(mLink);
      mLink = -1;
      Con::executef(this, "onLinkDeselected");
   }
}

DefineEngineMethod(GuiNavEditorCtrl, deleteLink, void, (),,
   "@brief Delete the currently selected link.")
{
   object->deleteLink();
}

void GuiNavEditorCtrl::setLinkFlags(const LinkData &d)
{
   if(mMode == mLinkMode && !mMesh.isNull() && mLink != -1)
   {
      mMesh->setLinkFlags(mLink, d);
   }
}

DefineEngineMethod(GuiNavEditorCtrl, setLinkFlags, void, (U32 flags),,
   "@Brief Set jump and drop properties of the selected link.")
{
   object->setLinkFlags(LinkData(flags));
}

void GuiNavEditorCtrl::buildTile()
{
   if(!mMesh.isNull() && mTile != -1)
      mMesh->buildTile(mTile);
}

DefineEngineMethod(GuiNavEditorCtrl, buildTile, void, (),,
   "@brief Build the currently selected tile.")
{
   object->buildTile();
}

void GuiNavEditorCtrl::spawnPlayer(const Point3F &pos)
{
   SceneObject *obj = (SceneObject*)Sim::spawnObject(mSpawnClass, mSpawnDatablock);
   if(obj)
   {
      MatrixF mat(true);
      mat.setPosition(pos);
      obj->setTransform(mat);
      SimObject* cleanup = Sim::findObject("MissionCleanup");
      if(cleanup)
      {
         SimGroup* missionCleanup = dynamic_cast<SimGroup*>(cleanup);
         missionCleanup->addObject(obj);
      }
      mPlayer = static_cast<AIPlayer*>(obj);
      Con::executef(this, "onPlayerSelected", Con::getIntArg(mPlayer->mLinkTypes.getFlags()));
   }
}

DefineEngineMethod(GuiNavEditorCtrl, spawnPlayer, void, (),,
                   "@brief Spawn an AIPlayer at the centre of the screen.")
{
   Point3F c;
   if(object->get3DCentre(c))
      object->spawnPlayer(c);
}

void GuiNavEditorCtrl::get3DCursor(GuiCursor *&cursor,
                                   bool &visible,
                                   const Gui3DMouseEvent &event_)
{
   //cursor = mAddNodeCursor;
   //visible = false;

   cursor = NULL;
   visible = false;

   GuiCanvas *root = getRoot();
   if(!root)
      return;

   S32 currCursor = PlatformCursorController::curArrow;

   if(root->mCursorChanged == currCursor)
      return;

   PlatformWindow *window = root->getPlatformWindow();
   PlatformCursorController *controller = window->getCursorController();

   // We've already changed the cursor,
   // so set it back before we change it again.
   if(root->mCursorChanged != -1)
      controller->popCursor();

   // Now change the cursor shape
   controller->pushCursor(currCursor);
   root->mCursorChanged = currCursor;
}

bool GuiNavEditorCtrl::get3DCentre(Point3F &pos)
{
   Point3F screen, start, end;

   screen.set(getExtent().x / 2, getExtent().y / 2, 0);
   unproject(screen, &start);

   screen.z = 1.0f;
   unproject(screen, &end);

   RayInfo ri;
   if(gServerContainer.castRay(start, end, StaticObjectType, &ri))
   {
      pos = ri.point;
      return true;
   }
   return false;
}

void GuiNavEditorCtrl::on3DMouseDown(const Gui3DMouseEvent & event)
{
   mGizmo->on3DMouseDown(event);

   if(!isFirstResponder())
      setFirstResponder();

   mouseLock();

   // Construct a LineSegment from the camera position to 1000 meters away in
   // the direction clicked.
   // If that segment hits the terrain, truncate the ray to only be that length.

   Point3F startPnt = event.pos;
   Point3F endPnt = event.pos + event.vec * 1000.0f;

   RayInfo ri;

   U8 keys = Input::getModifierKeys();
   bool shift = keys & SI_LSHIFT;
   bool ctrl = keys & SI_LCTRL;

   if(mMode == mLinkMode && !mMesh.isNull())
   {
      if(gServerContainer.castRay(startPnt, endPnt, StaticObjectType, &ri))
      {
         U32 link = mMesh->getLink(ri.point);
         if(link != -1)
         {
            if(mLink != -1)
               mMesh->selectLink(mLink, false);
            mMesh->selectLink(link, true, false);
            mLink = link;
            LinkData d = mMesh->getLinkFlags(mLink);
            Con::executef(this, "onLinkSelected", Con::getIntArg(d.getFlags()));
         }
         else
         {
            if(mLink != -1)
            {
               mMesh->selectLink(mLink, false);
               mLink = -1;
               Con::executef(this, "onLinkDeselected");
            }
            else
            {
               if(mLinkStart != Point3F::Max)
               {
                  mMesh->addLink(mLinkStart, ri.point);
                  if(!shift)
                     mLinkStart = Point3F::Max;
               }
               else
               {
                  mLinkStart = ri.point;
               }
            }
         }
      }
      else
      {
         mMesh->selectLink(mLink, false);
         mLink = -1;
         Con::executef(this, "onLinkDeselected");
      }
   }

   if(mMode == mTileMode && !mMesh.isNull())
   {
      if(gServerContainer.castRay(startPnt, endPnt, StaticShapeObjectType, &ri))
      {
         mTile = mMesh->getTile(ri.point);
         dd.clear();
         mMesh->renderTileData(dd, mTile);
      }
   }

   if(mMode == mTestMode)
   {
      // Spawn new character
      if(ctrl)
      {
         if(gServerContainer.castRay(startPnt, endPnt, StaticObjectType, &ri))
            spawnPlayer(ri.point);
      }
      // Deselect character
      else if(shift)
      {
         mPlayer = NULL;
         Con::executef(this, "onPlayerDeselected");
      }
      // Select/move character
      else
      {
         if(gServerContainer.castRay(startPnt, endPnt, PlayerObjectType, &ri))
         {
            if(dynamic_cast<AIPlayer*>(ri.object))
            {
               mPlayer = dynamic_cast<AIPlayer*>(ri.object);
               Con::executef(this, "onPlayerSelected", Con::getIntArg(mPlayer->mLinkTypes.getFlags()));
            }
         }
         else if(!mPlayer.isNull() && gServerContainer.castRay(startPnt, endPnt, StaticObjectType, &ri))
            mPlayer->setPathDestination(ri.point);
      }
   }
}

void GuiNavEditorCtrl::on3DMouseUp(const Gui3DMouseEvent & event)
{
   // Keep the Gizmo up to date.
   mGizmo->on3DMouseUp(event);

   mouseUnlock();
}

void GuiNavEditorCtrl::on3DMouseMove(const Gui3DMouseEvent & event)
{
   //if(mSelRiver != NULL && mSelNode != -1)
      //mGizmo->on3DMouseMove(event);

   Point3F startPnt = event.pos;
   Point3F endPnt = event.pos + event.vec * 1000.0f;

   RayInfo ri;

   if(mMode == mLinkMode && !mMesh.isNull())
   {
      if(gServerContainer.castRay(startPnt, endPnt, StaticObjectType, &ri))
      {
         U32 link = mMesh->getLink(ri.point);
         if(link != -1)
         {
            if(link != mLink)
            {
               if(mCurLink != -1)
                  mMesh->selectLink(mCurLink, false);
               mMesh->selectLink(link, true, true);
            }
            mCurLink = link;
         }
         else
         {
            if(mCurLink != mLink)
               mMesh->selectLink(mCurLink, false);
            mCurLink = -1;
         }
      }
      else
      {
         mMesh->selectLink(mCurLink, false);
         mCurLink = -1;
      }
   }

   // Select a tile from our current NavMesh.
   if(mMode == mTileMode && !mMesh.isNull())
   {
      if(gServerContainer.castRay(startPnt, endPnt, StaticObjectType, &ri))
         mCurTile = mMesh->getTile(ri.point);
      else
         mCurTile = -1;
   }

   if(mMode == mTestMode)
   {
      if(gServerContainer.castRay(startPnt, endPnt, PlayerObjectType, &ri))
         mCurPlayer = dynamic_cast<AIPlayer*>(ri.object);
      else
         mCurPlayer = NULL;
   }
}

void GuiNavEditorCtrl::on3DMouseDragged(const Gui3DMouseEvent & event)
{
   mGizmo->on3DMouseDragged(event);
   if(mGizmo->isDirty())
   {
      Point3F pos = mGizmo->getPosition();
      Point3F scale = mGizmo->getScale();
      const MatrixF &mat = mGizmo->getTransform();
      VectorF normal;
      mat.getColumn(2, &normal);

      //mSelRiver->setNode(pos, scale.x, scale.z, normal, mSelNode);
      mIsDirty = true;
   }
}

void GuiNavEditorCtrl::on3DMouseEnter(const Gui3DMouseEvent & event)
{
}

void GuiNavEditorCtrl::on3DMouseLeave(const Gui3DMouseEvent & event)
{
}

void GuiNavEditorCtrl::updateGuiInfo()
{
}

void GuiNavEditorCtrl::onRender(Point2I offset, const RectI &updateRect)
{
   PROFILE_SCOPE(GuiNavEditorCtrl_OnRender);

   Parent::onRender(offset, updateRect);
   return;
}

static void renderBoxOutline(const Box3F &box, const ColorI &col)
{
   if(box != Box3F::Invalid)
   {
      GFXStateBlockDesc desc;
      desc.setCullMode(GFXCullNone);
      desc.setFillModeSolid();
      desc.setZReadWrite(true, false);
      desc.setBlend(true);
      GFX->getDrawUtil()->drawCube(desc, box, ColorI(col, 20));
      desc.setFillModeWireframe();
      desc.setBlend(false);
      GFX->getDrawUtil()->drawCube(desc, box, ColorI(col, 255));
   }
}

void GuiNavEditorCtrl::renderScene(const RectI & updateRect)
{
   GFX->setStateBlock(mZDisableSB);

   // get the projected size...
   GameConnection* connection = GameConnection::getConnectionToServer();
   if(!connection)
      return;

   // Grab the camera's transform
   MatrixF mat;
   connection->getControlCameraTransform(0, &mat);

   // Get the camera position
   Point3F camPos;
   mat.getColumn(3,&camPos);

   if(mMode == mLinkMode)
   {
      if(mLinkStart != Point3F::Max)
      {
         GFXStateBlockDesc desc;
         desc.setBlend(false);
         desc.setZReadWrite(true ,true);
         MatrixF mat(true);
         mat.setPosition(mLinkStart);
         Point3F scale(0.8f, 0.8f, 0.8f);
         GFX->getDrawUtil()->drawTransform(desc, mat, &scale);
      }
   }

   if(mMode == mTileMode && !mMesh.isNull())
   {
      renderBoxOutline(mMesh->getTileBox(mCurTile), ColorI::BLUE);
      renderBoxOutline(mMesh->getTileBox(mTile), ColorI::GREEN);
      if(Con::getBoolVariable("$Nav::Editor::renderVoxels", false)) dd.renderGroup(0);
      if(Con::getBoolVariable("$Nav::Editor::renderInput", false))
      {
         dd.depthMask(false);
         dd.renderGroup(1);
         dd.depthMask(true);
      }
   }

   if(mMode == mTestMode)
   {
      if(!mCurPlayer.isNull())
         renderBoxOutline(mCurPlayer->getWorldBox(), ColorI::BLUE);
      if(!mPlayer.isNull())
         renderBoxOutline(mPlayer->getWorldBox(), ColorI::GREEN);
   }

   duDebugDrawTorque d;
   if(!mMesh.isNull())
      mMesh->renderLinks(d);
   d.render();

   // Now draw all the 2d stuff!
   GFX->setClipRect(updateRect);
}

bool GuiNavEditorCtrl::getStaticPos(const Gui3DMouseEvent & event, Point3F &tpos)
{
   // Find clicked point on the terrain

   Point3F startPnt = event.pos;
   Point3F endPnt = event.pos + event.vec * 1000.0f;

   RayInfo ri;
   bool hit;

   hit = gServerContainer.castRay(startPnt, endPnt, StaticShapeObjectType, &ri);
   tpos = ri.point;

   return hit;
}

void GuiNavEditorCtrl::setMode(String mode, bool sourceShortcut = false)
{
   mMode = mode;
   Con::executef(this, "onModeSet", mode);

   if(sourceShortcut)
      Con::executef(this, "paletteSync", mode);
}

void GuiNavEditorCtrl::submitUndo(const UTF8 *name)
{
   // Grab the mission editor undo manager.
   UndoManager *undoMan = NULL;
   if(!Sim::findObject("EUndoManager", undoMan))
   {
      Con::errorf("GuiNavEditorCtrl::submitUndo() - EUndoManager not found!");
      return;
   }

   // Setup the action.
   GuiNavEditorUndoAction *action = new GuiNavEditorUndoAction(name);

   action->mNavEditor = this;

   undoMan->addAction(action);
}

void GuiNavEditorCtrl::_prepRenderImage(SceneManager* sceneGraph, const SceneRenderState* state)
{
   /*if(isAwake() && River::smEditorOpen && mSelRiver)
   {
      ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
      ri->type = RenderPassManager::RIT_Editor;
      ri->renderDelegate.bind(this, &GuiNavEditorCtrl::_renderSelectedRiver);
      ri->defaultKey = 100;
      state->getRenderPass()->addInst(ri);
   }*/
}

ConsoleMethod(GuiNavEditorCtrl, getMode, const char*, 2, 2, "")
{
   return object->getMode();
}

ConsoleMethod(GuiNavEditorCtrl, setMode, void, 3, 3, "setMode(String mode)")
{
   String newMode = (argv[2]);
   object->setMode(newMode);
}
