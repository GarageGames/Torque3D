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

#include "gui/worldEditor/guiMissionArea.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "gfx/gfxDrawUtil.h"
#include "gfx/primBuilder.h"
#include "gfx/bitmap/gBitmap.h"
#include "gui/3d/guiTSControl.h"
#include "T3D/gameFunctions.h"
#include "terrain/terrData.h"

namespace {
   F32 round_local(F32 val)
   {
      if(val >= 0.f)
      {
         F32 floor = mFloor(val);
         if((val - floor) >= 0.5f)
            return(floor + 1.f);
         return(floor);
      }
      else
      {
         F32 ceil = mCeil(val);
         if((val - ceil) <= -0.5f)
            return(ceil - 1.f);
         return(ceil);
      }
   }
}

IMPLEMENT_CONOBJECT(GuiMissionAreaCtrl);

ConsoleDocClass( GuiMissionAreaCtrl,
   "@brief Visual representation of Mission Area Editor.\n\n"
   "@internal"
);

GuiMissionAreaCtrl::GuiMissionAreaCtrl()
{
   mHandleBitmap = StringTable->EmptyString();
   mHandleTexture = NULL;
   mHandleTextureSize = Point2I::Zero;
   mHandleTextureHalfSize = Point2F::Zero;

   mSquareBitmap = true;

   mMissionArea = 0;
   mTerrainBlock = 0;

   mMissionBoundsColor.set(255,0,0);
   mCameraColor.set(255,0,0);

   mBlendStateBlock = NULL;
   mSolidStateBlock = NULL;

   mLastHitMode = Handle_None;
   mSavedDrag = false;
}

GuiMissionAreaCtrl::~GuiMissionAreaCtrl()
{
}

//------------------------------------------------------------------------------

void GuiMissionAreaCtrl::initPersistFields()
{
   addField( "squareBitmap",        TypeBool,      Offset(mSquareBitmap, GuiMissionAreaCtrl));

   addField( "handleBitmap",        TypeFilename,  Offset( mHandleBitmap, GuiMissionAreaCtrl ),
      "Bitmap file for the mission area handles.\n");

   addField( "missionBoundsColor",  TypeColorI,    Offset(mMissionBoundsColor, GuiMissionAreaCtrl));
   addField( "cameraColor",         TypeColorI,    Offset(mCameraColor, GuiMissionAreaCtrl));

   Parent::initPersistFields();
}

//------------------------------------------------------------------------------

bool GuiMissionAreaCtrl::onAdd()
{
   if(!Parent::onAdd())
      return(false);

   GFXStateBlockDesc desc;
   desc.setCullMode(GFXCullNone);
   desc.setZReadWrite(false);
   desc.setBlend(false, GFXBlendOne, GFXBlendZero);
   mSolidStateBlock = GFX->createStateBlock( desc );

   desc.setBlend(true, GFXBlendSrcAlpha, GFXBlendInvSrcAlpha);
   mBlendStateBlock = GFX->createStateBlock( desc );

   if (*mHandleBitmap)
   {
      mHandleTexture = GFXTexHandle( mHandleBitmap, &GFXDefaultPersistentProfile, avar("%s() - mHandleTexture (line %d)", __FUNCTION__, __LINE__) );
      mHandleTextureSize = Point2I( mHandleTexture->getWidth(), mHandleTexture->getHeight() );
      mHandleTextureHalfSize = Point2F(mHandleTextureSize.x, mHandleTextureSize.y) * 0.5f;
   }
   else
   {
      mHandleTexture = NULL;
      mHandleTextureSize = Point2I::Zero;
      mHandleTextureHalfSize = Point2F::Zero;
   }

   return(true);
}

bool GuiMissionAreaCtrl::onWake()
{
   if(!Parent::onWake())
      return(false);

   //mMissionArea = const_cast<MissionArea*>(MissionArea::getServerObject());
   //if(!bool(mMissionArea))
   //   Con::warnf(ConsoleLogEntry::General, "GuiMissionAreaCtrl::onWake: no MissionArea object.");

   //mTerrainBlock = getTerrainObj();
   //if(!bool(mTerrainBlock))
   //   Con::warnf(ConsoleLogEntry::General, "GuiMissionAreaCtrl::onWake: no TerrainBlock object.");

   //if ( !bool(mMissionArea) || !bool(mTerrainBlock) )
   //   return true;

   updateTerrainBitmap();

   // make sure mission area is clamped
   setArea(getArea());

   //onUpdate();
   setActive(true);

   return(true);
}

void GuiMissionAreaCtrl::onSleep()
{
   mTextureObject = NULL;
   mMissionArea = 0;
   mTerrainBlock = 0;

   Parent::onSleep();
}

//------------------------------------------------------------------------------

void GuiMissionAreaCtrl::onMouseUp(const GuiEvent & event)
{
   if(!bool(mMissionArea))
      return;

   RectI box;
   getScreenMissionArea(box);
   S32 hit = getHitHandles(event.mousePoint, box);

   // set the current cursor
   //updateCursor(hit);
   mLastHitMode = hit;

   if(mSavedDrag)
   {
      // Let the script get a chance at it.
      Con::executef( this, "onMissionAreaModified" );
   }
   mSavedDrag = false;
}

void GuiMissionAreaCtrl::onMouseDown(const GuiEvent & event)
{
   if(!bool(mMissionArea))
      return;

   RectI box;
   getScreenMissionArea(box);

   mLastHitMode = getHitHandles(event.mousePoint, box);
   //if(mLastHitMode == Handle_Middle)
   //   setCursor(GrabCursor);
   mLastMousePoint = event.mousePoint;
}

void GuiMissionAreaCtrl::onMouseMove(const GuiEvent & event)
{
   if(!bool(mMissionArea))
      return;

   RectI box;
   getScreenMissionArea(box);
   S32 hit = getHitHandles(event.mousePoint, box);

   // set the current cursor...
   //updateCursor(hit);
   mLastHitMode = hit;
}

void GuiMissionAreaCtrl::onMouseDragged(const GuiEvent & event)
{
   if(!bool(mMissionArea))
      return;

   if(mLastHitMode == Handle_None)
      return;

   // If we haven't already saved,
   // save an undo action to get back to this state,
   // before we make any modifications.
   if ( !mSavedDrag )
   {
      submitUndo( "Modify Node" );
      mSavedDrag = true;
   }

   RectI box;
   getScreenMissionArea(box);
   Point2I mouseDiff(event.mousePoint.x - mLastMousePoint.x,
      event.mousePoint.y - mLastMousePoint.y);

   // what we drag'n?
   RectI area = getArea();
   Point2I wp = screenDeltaToWorldDelta(mouseDiff);

   if (mLastHitMode == Handle_Middle)
   {
      area.point += wp;
   }

   if (mLastHitMode & Handle_Left)
   {
      if ((area.extent.x - wp.x) >= 1)
      {
         area.point.x += wp.x;
         area.extent.x -= wp.x;
      }
   }

   if (mLastHitMode & Handle_Right)
   {
      if ((area.extent.x + wp.x) >= 1)
      {
         area.extent.x += wp.x;
      }
   }

   if (mLastHitMode & Handle_Bottom)
   {
      if ((area.extent.y - wp.y) >= 1)
      {
         area.point.y += wp.y;
         area.extent.y -= wp.y;
      }
   }

   if (mLastHitMode & Handle_Top)
   {
      if ((area.extent.y + wp.y) >= 1)
      {
         area.extent.y += wp.y;
      }
   }

   setArea(area);
   mLastMousePoint = event.mousePoint;
}

void GuiMissionAreaCtrl::onMouseEnter(const GuiEvent &)
{
   mLastHitMode = Handle_None;
   //setCursor(DefaultCursor);
}

void GuiMissionAreaCtrl::onMouseLeave(const GuiEvent &)
{
   mLastHitMode = Handle_None;
   //setCursor(DefaultCursor);
}

//------------------------------------------------------------------------------

void GuiMissionAreaCtrl::submitUndo( const UTF8 *name )
{
   // Grab the mission editor undo manager.
   UndoManager *undoMan = NULL;
   if ( !Sim::findObject( "EUndoManager", undoMan ) )
   {
      Con::errorf( "GuiRiverEditorCtrl::submitUndo() - EUndoManager not found!" );
      return;           
   }

   // Setup the action.
   GuiMissionAreaUndoAction *action = new GuiMissionAreaUndoAction( name );

   action->mMissionAreaEditor = this;

   action->mObjId = mMissionArea->getId();
   action->mArea = mMissionArea->getArea();
      
   undoMan->addAction( action );
}

//------------------------------------------------------------------------------

void GuiMissionAreaCtrl::updateTerrain()
{
   mTerrainBlock = getTerrainObj();
   updateTerrainBitmap();
}

TerrainBlock * GuiMissionAreaCtrl::getTerrainObj()
{
   SimSet * scopeAlwaysSet = Sim::getGhostAlwaysSet();
   for(SimSet::iterator itr = scopeAlwaysSet->begin(); itr != scopeAlwaysSet->end(); itr++)
   {
      TerrainBlock * terrain = dynamic_cast<TerrainBlock*>(*itr);
      if(terrain)
         return(terrain);
   }
   return(0);
}

void GuiMissionAreaCtrl::updateTerrainBitmap()
{
   GBitmap * bitmap = createTerrainBitmap();
   if( bitmap )
      setBitmapHandle( GFXTexHandle( bitmap, &GFXDefaultGUIProfile, true, String("Terrain Bitmap Update") ) );
   else
      setBitmap( "" );
}

GBitmap * GuiMissionAreaCtrl::createTerrainBitmap()
{
   if(!mTerrainBlock)
      return NULL;

   GBitmap * bitmap = new GBitmap(mTerrainBlock->getBlockSize(), mTerrainBlock->getBlockSize(), false, GFXFormatR8G8B8 );

   // get the min/max
   F32 min, max;
   mTerrainBlock->getMinMaxHeight(&min, &max);

   F32 diff = max - min;
   F32 colRange = 255.0f / diff;

   // This method allocates it's bitmap above, and does all assignment
   // in the following loop. It is not subject to 24-bit -> 32-bit conversion
   // problems, because the texture handle creation is where the conversion would
   // occur, if it occurs. Since the data in the texture is never read back, and
   // the bitmap is deleted after texture-upload, this is not a problem.
   for(S32 y = 0; y < mTerrainBlock->getBlockSize() ; y++)
   {
      for(S32 x = 0; x < mTerrainBlock->getBlockSize(); x++)
      {
         F32 height;
         height = mTerrainBlock->getHeight(Point2I(x, y));
 
         U8 col = U8((height - min) * colRange);
         ColorI color(col, col, col);
         bitmap->setColor(x, y, color);
    
      }
   }

   return(bitmap);
}

//------------------------------------------------------------------------------

void GuiMissionAreaCtrl::setMissionArea( MissionArea* area )
{
   mMissionArea = area;
   if( mMissionArea )
   {
      setArea(getArea());
   }
}

const RectI & GuiMissionAreaCtrl::getArea()
{
   if( !bool(mMissionArea) )
      return(MissionArea::smMissionArea);

   return(mMissionArea->getArea());
}

void GuiMissionAreaCtrl::setArea(const RectI & area)
{
   if( bool(mMissionArea) )
   {
      mMissionArea->setArea(area);
      mMissionArea->inspectPostApply();
      //onUpdate();
   }
}

//------------------------------------------------------------------------------

void GuiMissionAreaCtrl::drawHandle(const Point2F & pos)
{
   Point2F pnt(pos.x-mHandleTextureHalfSize.x, pos.y-mHandleTextureHalfSize.y);
   GFX->getDrawUtil()->drawBitmap(mHandleTexture, pnt);
}

void GuiMissionAreaCtrl::drawHandles(RectI & box)
{
   F32 fillOffset = GFX->getFillConventionOffset();

   F32 lx = box.point.x + fillOffset, rx = box.point.x + box.extent.x + fillOffset;
   F32 cx = (lx + rx) * 0.5f;
   F32 by = box.point.y + fillOffset, ty = box.point.y + box.extent.y + fillOffset;
   F32 cy = (ty + by) * 0.5f;

   GFX->getDrawUtil()->clearBitmapModulation();
   drawHandle(Point2F(lx, ty));
   drawHandle(Point2F(lx, cy));
   drawHandle(Point2F(lx, by));
   drawHandle(Point2F(rx, ty));
   drawHandle(Point2F(rx, cy));
   drawHandle(Point2F(rx, by));
   drawHandle(Point2F(cx, ty));
   drawHandle(Point2F(cx, by));
}

bool GuiMissionAreaCtrl::testWithinHandle(const Point2I & testPoint, S32 handleX, S32 handleY)
{
   S32 dx = testPoint.x - handleX;
   S32 dy = testPoint.y - handleY;
   return dx <= Handle_Pixel_Size && dx >= -Handle_Pixel_Size && dy <= Handle_Pixel_Size && dy >= -Handle_Pixel_Size;
}

S32 GuiMissionAreaCtrl::getHitHandles(const Point2I & mousePnt, const RectI & box)
{
   S32 lx = box.point.x, rx = box.point.x + box.extent.x - 1;
   S32 cx = (lx + rx) >> 1;
   S32 by = box.point.y, ty = box.point.y + box.extent.y - 1;
   S32 cy = (ty + by) >> 1;

   if (testWithinHandle(mousePnt, lx, ty))
      return Handle_Left | Handle_Top;
   if (testWithinHandle(mousePnt, cx, ty))
      return Handle_Top;
   if (testWithinHandle(mousePnt, rx, ty))
      return Handle_Right | Handle_Top;
   if (testWithinHandle(mousePnt, lx, by))
      return Handle_Left | Handle_Bottom;
   if (testWithinHandle(mousePnt, cx, by))
      return Handle_Bottom;
   if (testWithinHandle(mousePnt, rx, by))
      return Handle_Right | Handle_Bottom;
   if (testWithinHandle(mousePnt, lx, cy))
      return Handle_Left;
   if (testWithinHandle(mousePnt, rx, cy))
      return Handle_Right;
   if(mousePnt.x >= lx && mousePnt.x <= rx &&
      mousePnt.y >= ty && mousePnt.y <= by)
      return(Handle_Middle);

   return Handle_None;
}

//------------------------------------------------------------------------------

Point2F GuiMissionAreaCtrl::worldToScreen(const Point2F & pos)
{
   return(Point2F(mCenterPos.x + (pos.x * mScale.x), mCenterPos.y + (pos.y * mScale.y)));
}

Point2I GuiMissionAreaCtrl::worldToScreen(const Point2I &pos)
{
   return(Point2I(S32(mCenterPos.x + (pos.x * mScale.x)), S32(mCenterPos.y + (pos.y * mScale.y))));
}

Point2F GuiMissionAreaCtrl::screenToWorld(const Point2F & pos)
{
   return(Point2F((pos.x - mCenterPos.x) / mScale.x, (pos.y - mCenterPos.y) / mScale.y));
}

Point2I GuiMissionAreaCtrl::screenToWorld(const Point2I &pos)
{
   return(Point2I(S32((pos.x - mCenterPos.x) / mScale.x), S32((pos.y - mCenterPos.y) / mScale.y)));
}

Point2I GuiMissionAreaCtrl::screenDeltaToWorldDelta(const Point2I &screenPoint)
{
   return(Point2I(S32(screenPoint.x / mScale.x), S32(screenPoint.y / mScale.y)));
}

void GuiMissionAreaCtrl::setupScreenTransform(const Point2I & offset)
{
   const MatrixF & terrMat = mTerrainBlock->getTransform();
   Point3F terrPos;
   terrMat.getColumn(3, &terrPos);
   terrPos.z = 0;

   F32 terrDim = mTerrainBlock->getWorldBlockSize();

   const Point2I& extenti = getExtent( );
   Point2F extent( static_cast<F32>( extenti.x ), static_cast<F32>( extenti.y ) );

   if(mSquareBitmap)
      extent.x > extent.y ? extent.x = extent.y : extent.y = extent.x;

   // We need to negate the y-axis so we are correctly oriented with
   // positive y increase up the screen.
   mScale.set(extent.x / terrDim, -extent.y / terrDim, 0);

   Point3F terrOffset = -terrPos;
   terrOffset.convolve(mScale);

   // We need to add the y extent so we start from the bottom left of the control
   // rather than the top left.
   mCenterPos.set(terrOffset.x + F32(offset.x), terrOffset.y + F32(offset.y) + extent.y);
}

void GuiMissionAreaCtrl::getScreenMissionArea(RectI & rect)
{
   RectI area = mMissionArea->getArea();
   Point2F pos = worldToScreen(Point2F(F32(area.point.x), F32(area.point.y)));
   Point2F end = worldToScreen(Point2F(F32(area.point.x + area.extent.x), F32(area.point.y + area.extent.y)));

   //
   rect.point.x = S32(round_local(pos.x));
   rect.point.y = S32(round_local(pos.y));
   rect.extent.x = S32(round_local(end.x - pos.x));
   rect.extent.y = S32(round_local(end.y - pos.y));
}

void GuiMissionAreaCtrl::getScreenMissionArea(RectF & rect)
{
   RectI area = mMissionArea->getArea();
   Point2F pos = worldToScreen(Point2F(F32(area.point.x), F32(area.point.y)));
   Point2F end = worldToScreen(Point2F(F32(area.point.x + area.extent.x), F32(area.point.y + area.extent.y)));

   //
   rect.point.x = pos.x;
   rect.point.y = pos.y;
   rect.extent.x = end.x - pos.x;
   rect.extent.y = end.y - pos.y;
}

Point2I GuiMissionAreaCtrl::convertOrigin(const Point2I &pos)
{
   // Convert screen point to our bottom left origin
   Point2I pnt = globalToLocalCoord(pos);
   const Point2I& extent = getExtent( );
   pnt.y = extent.y - pnt.y;
   Point2I pt = localToGlobalCoord(pnt);
   return pt;
}

void GuiMissionAreaCtrl::onRender(Point2I offset, const RectI & updateRect)
{

   RectI rect(offset, getExtent());
   F32 fillOffset = GFX->getFillConventionOffset();

   setUpdate();

   // draw an x
   if(!bool(mMissionArea) || !bool(mTerrainBlock))
   {
      GFX->setStateBlock(mSolidStateBlock);
      PrimBuild::color3i( 0, 0, 0 );
      PrimBuild::begin( GFXLineList, 4 );

      PrimBuild::vertex2f( rect.point.x + fillOffset, updateRect.point.y + fillOffset );
      PrimBuild::vertex2f( rect.point.x + updateRect.extent.x + fillOffset, updateRect.point.y + updateRect.extent.y + fillOffset );
      PrimBuild::vertex2f( rect.point.x + fillOffset, updateRect.point.y + updateRect.extent.y + fillOffset );
      PrimBuild::vertex2f( rect.point.x + updateRect.extent.x + fillOffset, updateRect.point.y + fillOffset );

      PrimBuild::end();

      return;
   }

   //
   setupScreenTransform(offset);

   // draw the terrain
   if(mSquareBitmap)
      rect.extent.x > rect.extent.y ? rect.extent.x = rect.extent.y : rect.extent.y = rect.extent.x;

   GFXDrawUtil *drawer = GFX->getDrawUtil();
   drawer->clearBitmapModulation();
   drawer->drawBitmapStretch(mTextureObject, rect, GFXBitmapFlip_Y, GFXTextureFilterLinear, false);

   GFX->setStateBlock(mSolidStateBlock);
   drawer->clearBitmapModulation();

   // draw the reference axis
   PrimBuild::begin( GFXLineList, 4 );
      PrimBuild::color3i( 255, 0, 0 );
      PrimBuild::vertex2f( rect.point.x + 5 + fillOffset, rect.point.y + rect.extent.y - 5 + fillOffset );
      PrimBuild::vertex2f( rect.point.x + 25 + fillOffset, rect.point.y + rect.extent.y - 5 + fillOffset );
      PrimBuild::color3i( 0, 255, 0 );
      PrimBuild::vertex2f( rect.point.x + 5 + fillOffset, rect.point.y + rect.extent.y - 5 + fillOffset );
      PrimBuild::vertex2f( rect.point.x + 5 + fillOffset, rect.point.y + rect.extent.y - 25 + fillOffset );
   PrimBuild::end();

   RectF area;
   getScreenMissionArea(area);

   // render the mission area box
   PrimBuild::color( mMissionBoundsColor );
   PrimBuild::begin( GFXLineStrip, 5 );
      PrimBuild::vertex2f(area.point.x + fillOffset, area.point.y + fillOffset);
      PrimBuild::vertex2f(area.point.x + area.extent.x + fillOffset, area.point.y + fillOffset);
      PrimBuild::vertex2f(area.point.x + area.extent.x + fillOffset, area.point.y + area.extent.y + fillOffset);
      PrimBuild::vertex2f(area.point.x + fillOffset, area.point.y + area.extent.y + fillOffset);
      PrimBuild::vertex2f(area.point.x + fillOffset, area.point.y + fillOffset);
   PrimBuild::end();

   // render the camera
   //if(mRenderCamera)
   {
      CameraQuery camera;
      GameProcessCameraQuery(&camera);

      // farplane too far, 90' looks wrong...
      camera.fov = mDegToRad(60.f);
      camera.farPlane = 500.f;

      //
      F32 rot = camera.fov / 2;

      //
      VectorF ray;
      VectorF projRayA, projRayB;

      ray.set(camera.farPlane * -mSin(rot), camera.farPlane * mCos(rot), 0);
      camera.cameraMatrix.mulV(ray, &projRayA);

      ray.set(camera.farPlane * -mSin(-rot), camera.farPlane * mCos(-rot), 0);
      camera.cameraMatrix.mulV(ray, &projRayB);

      Point3F camPos;
      camera.cameraMatrix.getColumn(3, &camPos);

      Point2F s = worldToScreen(Point2F(camPos.x, camPos.y));
      Point2F e1 = worldToScreen(Point2F(camPos.x + projRayA.x, camPos.y + projRayA.y));
      Point2F e2 = worldToScreen(Point2F(camPos.x + projRayB.x, camPos.y + projRayB.y));

      PrimBuild::color( mCameraColor );
      PrimBuild::begin( GFXLineList, 4 );
         PrimBuild::vertex2f( s.x + fillOffset, s.y + fillOffset );
         PrimBuild::vertex2f( e1.x + fillOffset, e1.y + fillOffset );
         PrimBuild::vertex2f( s.x + fillOffset, s.y + fillOffset );
         PrimBuild::vertex2f( e2.x + fillOffset, e2.y + fillOffset );
      PrimBuild::end();
   }

   // render the handles
   RectI iArea;
   getScreenMissionArea(iArea);
   drawHandles(iArea);

   renderChildControls(offset, updateRect);
}

//------------------------------------------------------------------------------

DefineEngineMethod( GuiMissionAreaCtrl, setMissionArea, void, ( MissionArea* area ),,
   "@brief Set the MissionArea to edit.\n\n")
{
   object->setMissionArea( area );
}

DefineEngineMethod( GuiMissionAreaCtrl, updateTerrain, void, ( ),,
   "@brief Update the terrain bitmap.\n\n")
{
   object->updateTerrain();
}

//------------------------------------------------------------------------------

void GuiMissionAreaUndoAction::undo()
{
   MissionArea *ma = NULL;
   if ( !Sim::findObject( mObjId, ma ) )
      return;

   // Temporarily save the MissionArea's current data.
   RectI area = ma->getArea();

   // Restore the MissionArea properties saved in the UndoAction
   ma->setArea( mArea );
   ma->inspectPostApply();

   // Now save the previous Mission data in this UndoAction
   // since an undo action must become a redo action and vice-versa
   mArea = area;

   // Let the script get a chance at it.
   Con::executef( mMissionAreaEditor, "onUndo" );
}
