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
#include "console/engineAPI.h"
#include "gfx/gfxDevice.h"
#include "gfx/gfxDrawUtil.h"

#include "gui/editor/guiRectHandles.h"

IMPLEMENT_CONOBJECT(GuiRectHandles);

ConsoleDocClass( GuiRectHandles,
   "@brief Draws a box with handles for the user to manipulate.\n\n"
   "Editor use only.\n\n"
   "@internal"
);

//--------------------------------------------------------------------------
GuiRectHandles::GuiRectHandles() : GuiControl()
{
   mHandleRect.set(0.0f, 0.0f, 1.0f, 1.0f);
   mHandleSize = 10;
   mUseCustomColor = false;
   mHandleColor.set(100,100,100);
   mHitHandle = 0;
}

GuiRectHandles::~GuiRectHandles()
{
}

//--------------------------------------------------------------------------
void GuiRectHandles::initPersistFields()
{
   addField("handleRect",     TypeRectF,  Offset(mHandleRect,  GuiRectHandles),     "RectF of handle's box." );
   addField("handleSize",     TypeS32,    Offset(mHandleSize,  GuiRectHandles),     "Size of handles in pixels." );
   addField("useCustomColor", TypeBool,   Offset(mUseCustomColor,  GuiRectHandles), "Use given custom color for handles." );
   addField("handleColor",    TypeColorI, Offset(mHandleColor,  GuiRectHandles),    "Use given custom color for handles." );

   Parent::initPersistFields();
}

//--------------------------------------------------------------------------

void GuiRectHandles::onMouseUp(const GuiEvent &event)
{
   mHitHandle = 0;
}

void GuiRectHandles::onMouseDown(const GuiEvent &event)
{
   // The handles range from 0-1, so scale to fit within the
   // control's bounds.
   const Point2I& extent = getExtent();
   Point2I pos(extent.x*mHandleRect.point.x, extent.y*mHandleRect.point.y);
   Point2I size(extent.x*mHandleRect.extent.x, extent.y*mHandleRect.extent.y);
   RectI box(pos, size);

   Point2I localMousePoint = globalToLocalCoord(event.mousePoint);

   // Check if mouse is within handle rect
   if(!box.pointInRect(localMousePoint))
   {
      mHitHandle = 0;
      return;
   }

   Point2I normalizedMouse = localMousePoint - pos;
   Point2I halfSize = size / 2;
   S32 halfHandleSize = mHandleSize / 2;
   if(normalizedMouse.y < mHandleSize)
   {
      // Top handles
      if(normalizedMouse.x < mHandleSize)
         mHitHandle = 1;
      else if(normalizedMouse.x >= (size.x-mHandleSize))
         mHitHandle = 3;
      else if(normalizedMouse.x >= (halfSize.x-halfHandleSize) && normalizedMouse.x < (halfSize.x+halfHandleSize))
         mHitHandle = 2;
   }
   else if(normalizedMouse.y >= (size.y-mHandleSize))
   {
      // Bottom handles
      if(normalizedMouse.x < mHandleSize)
         mHitHandle = 7;
      else if(normalizedMouse.x >= (size.x-mHandleSize))
         mHitHandle = 5;
      else if(normalizedMouse.x >= (halfSize.x-halfHandleSize) && normalizedMouse.x < (halfSize.x+halfHandleSize))
         mHitHandle = 6;
   }
   else if(normalizedMouse.y >= (halfSize.y-halfHandleSize) && normalizedMouse.y < (halfSize.y+halfHandleSize))
   {
      // Middle handles
      if(normalizedMouse.x < mHandleSize)
         mHitHandle = 8;
      else if(normalizedMouse.x >= (size.x-mHandleSize))
         mHitHandle = 4;
      else if(normalizedMouse.x >= (halfSize.x-halfHandleSize) && normalizedMouse.x < (halfSize.x+halfHandleSize))
         mHitHandle = 9;
   }

   mHitPoint = localMousePoint;
}

void GuiRectHandles::onMouseDragged(const GuiEvent &event)
{
   if(mHitHandle == 0)
      return;

   // The handles range from 0-1, so scale to fit within the
   // control's bounds.
   const Point2I& extent = getExtent();

   Point2I localMousePoint = globalToLocalCoord(event.mousePoint);

   Point2I diffI = localMousePoint - mHitPoint;
   Point2F diffF(diffI.x/F32(extent.x), diffI.y/F32(extent.y));

   RectF box(mHandleRect);
   bool postMoveExtentX = false;
   bool postMoveExtentY = false;
   bool keepExtent = false;

   switch(mHitHandle)
   {
      case 1:
      {
         // Top left
         box.point += diffF;
         postMoveExtentX = true;
         postMoveExtentY = true;
         break;
      }

      case 2:
      {
         // Top middle
         box.point.y += diffF.y;
         postMoveExtentY = true;
         break;
      }

      case 3:
      {
         // Top right
         box.point.y += diffF.y;
         box.extent.x += diffF.x;
         postMoveExtentY = true;
         break;
      }

      case 4:
      {
         // Middle right
         box.extent.x += diffF.x;
         break;
      }

      case 5:
      {
         // Bottom right
         box.extent += diffF;
         break;
      }

      case 6:
      {
         // Bottom middle
         box.extent.y += diffF.y;
         break;
      }

      case 7:
      {
         // Bottom left
         box.point.x += diffF.x;
         box.extent.y += diffF.y;
         postMoveExtentX = true;
         break;
      }

      case 8:
      {
         // Middle left
         box.point.x += diffF.x;
         postMoveExtentX = true;
         break;
      }

      case 9:
      {
         // Centre
         box.point += diffF;
         keepExtent = true;
         break;
      }

      default:
         break;
   }

   // Position limits
   if(box.point.x < 0.0f)
      box.point.x = 0.0f;
   else if(box.point.x > 1.0f)
      box.point.x = 1.0f;

   if(box.point.y < 0.0f)
      box.point.y = 0.0f;
   else if(box.point.y > 1.0f)
      box.point.y = 1.0f;

   // Move any extent to counter a change in handle position.  Do this
   // after the limits above to make sure the extent doesn't accidentally
   // grow when the position is clamped.
   if(postMoveExtentX)
      box.extent.x += mHandleRect.point.x - box.point.x;
   if(postMoveExtentY)
      box.extent.y += mHandleRect.point.y - box.point.y;

   // Extent limits
   if(box.extent.x < 0.0f)
      box.extent.x = 0.0f;
   else if(box.extent.x > 1.0f)
      box.extent.x = 1.0f;
   if(box.point.x+box.extent.x > 1.0f)
   {
      if(keepExtent)
         box.point.x = 1.0f-box.extent.x;
      else
         box.extent.x = 1.0f-box.point.x;
   }

   if(box.extent.y < 0.0f)
      box.extent.y = 0.0f;
   else if(box.extent.y > 1.0f)
      box.extent.y = 1.0f;
   if(box.point.y+box.extent.y > 1.0f)
   {
      if(keepExtent)
         box.point.y = 1.0f-box.extent.y;
      else
         box.extent.y = 1.0f-box.point.y;
   }

   mHandleRect = box;
   mHitPoint = localMousePoint;

   if( isMethod( "onHandleRectChange" ) )
      Con::executef(this, "onHandleRectChange" );
}

//--------------------------------------------------------------------------
void GuiRectHandles::onRender(Point2I offset, const RectI &updateRect)
{
   Parent::onRender( offset, updateRect );

   ColorI handleColor = mProfile->mBorderColor;
   if(mUseCustomColor)
      handleColor = mHandleColor;

   // The handles range from 0-1, so scale to fit within the
   // control's bounds.
   const Point2I& extent = getExtent();
   Point2I pos(extent.x*mHandleRect.point.x, extent.y*mHandleRect.point.y);
   Point2I size(extent.x*mHandleRect.extent.x, extent.y*mHandleRect.extent.y);
   RectI box(offset+pos, size);

   GFXDrawUtil* drawUtil = GFX->getDrawUtil();

   // Draw border
   drawUtil->drawRect(box, handleColor);

   // Draw each handle
   Point2I handleSize(mHandleSize, mHandleSize);
   RectI handleRect(box.point, handleSize);
   drawUtil->drawRectFill(handleRect, handleColor);      // Upper left
   handleRect.point = Point2I(box.point.x+size.x-handleSize.x, box.point.y);
   drawUtil->drawRectFill(handleRect, handleColor);      // Upper right
   handleRect.point = Point2I(box.point.x, box.point.y+size.y-handleSize.y);
   drawUtil->drawRectFill(handleRect, handleColor);      // Lower left
   handleRect.point = Point2I(box.point.x+size.x-handleSize.x, box.point.y+size.y-handleSize.y);
   drawUtil->drawRectFill(handleRect, handleColor);      // Lower right

   Point2I halfSize = size / 2;
   Point2I halfHandleSize = handleSize / 2;
   handleRect.point = Point2I(box.point.x+halfSize.x-halfHandleSize.x, box.point.y);
   drawUtil->drawRectFill(handleRect, handleColor);      // Upper middle
   handleRect.point = Point2I(box.point.x+halfSize.x-halfHandleSize.x, box.point.y+size.y-handleSize.y);
   drawUtil->drawRectFill(handleRect, handleColor);      // Lower middle
   handleRect.point = Point2I(box.point.x, box.point.y+halfSize.y-halfHandleSize.y);
   drawUtil->drawRectFill(handleRect, handleColor);      // Left middle
   handleRect.point = Point2I(box.point.x+size.x-handleSize.x, box.point.y+halfSize.y-halfHandleSize.y);
   drawUtil->drawRectFill(handleRect, handleColor);      // Right middle

   handleRect.point = Point2I(box.point.x+halfSize.x-halfHandleSize.x, box.point.y+halfSize.y-halfHandleSize.y);
   drawUtil->drawRectFill(handleRect, handleColor);      // Middle

   renderChildControls(offset, updateRect);
}
