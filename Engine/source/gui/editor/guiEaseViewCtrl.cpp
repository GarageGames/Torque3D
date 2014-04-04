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

#include "gui/editor/guiEaseViewCtrl.h"
#include "console/consoleTypes.h"
#include "math/mMath.h"
#include "gfx/gfxDrawUtil.h"


IMPLEMENT_CONOBJECT( GuiEaseViewCtrl );

ConsoleDocClass( GuiEaseViewCtrl,
   "@brief Control to visualize an EaseF.\n\n"
   "Editor use only.\n\n"
   "@see EaseF\n\n"
   "@internal"
);

//-----------------------------------------------------------------------------

GuiEaseViewCtrl::GuiEaseViewCtrl()
{
   mEase.set(Ease::In, Ease::Cubic);
   mAxisColor.set(1,1,1,1);
   mEaseColor.set(1,1,1,1);
   mEaseWidth = 4.0f;
}

//-----------------------------------------------------------------------------

void GuiEaseViewCtrl::initPersistFields()
{
   Parent::initPersistFields();

   addField("ease", TypeEaseF, Offset( mEase, GuiEaseViewCtrl ) );
   addField("easeColor", TypeColorF, Offset( mEaseColor, GuiEaseViewCtrl ) );
   addField("easeWidth", TypeF32, Offset(mEaseWidth, GuiEaseViewCtrl ) );
   addField("axisColor", TypeColorF, Offset( mAxisColor, GuiEaseViewCtrl ) );
}

//-----------------------------------------------------------------------------

bool GuiEaseViewCtrl::onWake()
{
   if (!Parent::onWake())
		return false;
   setActive(true);
   return true;
}

//-----------------------------------------------------------------------------

void GuiEaseViewCtrl::onSleep()
{
	setActive(false);
	Parent::onSleep();
}

//-----------------------------------------------------------------------------

void GuiEaseViewCtrl::onRender(Point2I offset, const RectI &updateRect)
{
   const F32 W = getExtent().x;
   const F32 H = getExtent().y;
   const F32 plotW = W;
   const F32 plotH = H;
   const F32 zeroX = offset.x + 1.f;
   const F32 zeroY = offset.y;
   
   // Draw axis.

	GFX->getDrawUtil()->drawLine( zeroX,  zeroY + 0.0f,  zeroX,          zeroY + plotH,  mAxisColor );
	GFX->getDrawUtil()->drawLine( zeroX,  zeroY + plotH, zeroX + plotW,  zeroY + plotH,  mAxisColor );
   
	F32 numPoints = W;
	F32 lastX = zeroX;
	F32 lastY = zeroY + plotH;

   // Draw curve.

	for( S32 i = 1; i <= numPoints; ++ i )
	{
		F32 x = ( F32 ) i / ( F32 ) numPoints;
      F32 y = mEase.getValue( x, 0, 1, 1 );

		x = zeroX + x * plotW;
      y = zeroY + plotH - y * plotH;
      
		GFX->getDrawUtil()->drawLine( lastX, lastY, x, y, mEaseColor );

		lastX = x;
		lastY = y;
	}
}
