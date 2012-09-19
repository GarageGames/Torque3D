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

#include "T3D/gameTSCtrl.h"
#include "console/consoleTypes.h"
#include "T3D/gameBase/gameBase.h"
#include "T3D/gameBase/gameConnection.h"
#include "T3D/shapeBase.h"
#include "T3D/gameFunctions.h"
#include "console/engineAPI.h"

//---------------------------------------------------------------------------
// Debug stuff:
Point3F lineTestStart = Point3F(0, 0, 0);
Point3F lineTestEnd =   Point3F(0, 1000, 0);
Point3F lineTestIntersect =  Point3F(0, 0, 0);
bool gSnapLine = false;

//----------------------------------------------------------------------------
// Class: GameTSCtrl
//----------------------------------------------------------------------------
IMPLEMENT_CONOBJECT(GameTSCtrl);

// See Torque manual (.CHM) for more information
ConsoleDocClass( GameTSCtrl,
	"@brief The main 3D viewport for a Torque 3D game.\n\n"
	"@ingroup Gui3D\n");

GameTSCtrl::GameTSCtrl()
{
}

//---------------------------------------------------------------------------
bool GameTSCtrl::onAdd()
{
   if ( !Parent::onAdd() )
      return false;

#ifdef TORQUE_DEMO_WATERMARK
   mWatermark.init();
#endif

   return true;
}

//---------------------------------------------------------------------------
bool GameTSCtrl::processCameraQuery(CameraQuery *camq)
{
   GameUpdateCameraFov();
   return GameProcessCameraQuery(camq);
}

//---------------------------------------------------------------------------
void GameTSCtrl::renderWorld(const RectI &updateRect)
{
   GameRenderWorld();
}

//---------------------------------------------------------------------------
void GameTSCtrl::makeScriptCall(const char *func, const GuiEvent &evt) const
{
   // write screen position
   char *sp = Con::getArgBuffer(32);
   dSprintf(sp, 32, "%d %d", evt.mousePoint.x, evt.mousePoint.y);

   // write world position
   char *wp = Con::getArgBuffer(32);
   Point3F camPos;
   mLastCameraQuery.cameraMatrix.getColumn(3, &camPos);
   dSprintf(wp, 32, "%g %g %g", camPos.x, camPos.y, camPos.z);

   // write click vector
   char *vec = Con::getArgBuffer(32);
   Point3F fp(evt.mousePoint.x, evt.mousePoint.y, 1.0);
   Point3F ray;
   unproject(fp, &ray);
   ray -= camPos;
   ray.normalizeSafe();
   dSprintf(vec, 32, "%g %g %g", ray.x, ray.y, ray.z);

   Con::executef( (SimObject*)this, func, sp, wp, vec );
}

void GameTSCtrl::onMouseDown(const GuiEvent &evt)
{
   Parent::onMouseDown(evt);
   if( isMethod( "onMouseDown" ) )
      makeScriptCall( "onMouseDown", evt );
}

void GameTSCtrl::onRightMouseDown(const GuiEvent &evt)
{
   Parent::onRightMouseDown(evt);
   if( isMethod( "onRightMouseDown" ) )
      makeScriptCall( "onRightMouseDown", evt );
}

void GameTSCtrl::onMiddleMouseDown(const GuiEvent &evt)
{
   Parent::onMiddleMouseDown(evt);
   if( isMethod( "onMiddleMouseDown" ) )
      makeScriptCall( "onMiddleMouseDown", evt );
}

void GameTSCtrl::onMouseUp(const GuiEvent &evt)
{
   Parent::onMouseUp(evt);
   if( isMethod( "onMouseUp" ) )
      makeScriptCall( "onMouseUp", evt );
}

void GameTSCtrl::onRightMouseUp(const GuiEvent &evt)
{
   Parent::onRightMouseUp(evt);
   if( isMethod( "onRightMouseUp" ) )
      makeScriptCall( "onRightMouseUp", evt );
}

void GameTSCtrl::onMiddleMouseUp(const GuiEvent &evt)
{
   Parent::onMiddleMouseUp(evt);
   if( isMethod( "onMiddleMouseUp" ) )
      makeScriptCall( "onMiddleMouseUp", evt );
}

void GameTSCtrl::onMouseMove(const GuiEvent &evt)
{
   if(gSnapLine)
      return;

   MatrixF mat;
   Point3F vel;
   if ( GameGetCameraTransform(&mat, &vel) )
   {
      Point3F pos;
      mat.getColumn(3,&pos);
      Point3F screenPoint((F32)evt.mousePoint.x, (F32)evt.mousePoint.y, -1.0f);
      Point3F worldPoint;
      if (unproject(screenPoint, &worldPoint)) {
         Point3F vec = worldPoint - pos;
         lineTestStart = pos;
         vec.normalizeSafe();
         lineTestEnd = pos + vec * 1000;
      }
   }
}

void GameTSCtrl::onRender(Point2I offset, const RectI &updateRect)
{
   // check if should bother with a render
   GameConnection * con = GameConnection::getConnectionToServer();
   bool skipRender = !con || (con->getWhiteOut() >= 1.f) || (con->getDamageFlash() >= 1.f) || (con->getBlackOut() >= 1.f);

   if(!skipRender || true)
      Parent::onRender(offset, updateRect);

#ifdef TORQUE_DEMO_WATERMARK
   mWatermark.render(getExtent());
#endif
}

//--------------------------------------------------------------------------

DefineEngineFunction(snapToggle, void, (),,
					 "@brief Prevents mouse movement from being processed\n\n"

					 "In the source, whenever a mouse move event occurs "
					 "GameTSCtrl::onMouseMove() is called. Whenever snapToggle() "
					 "is called, it will flag a variable that can prevent this "
					 "from happening: gSnapLine. This variable is not exposed to "
					 "script, so you need to call this function to trigger it.\n\n"

					 "@tsexample\n"
					 "// Snapping is off by default, so we will toggle\n"
					 "// it on first:\n"
					 "PlayGui.snapToggle();\n\n"
					 "// Mouse movement should be disabled\n"
					 "// Let's turn it back on\n"
					 "PlayGui.snapToggle();\n"
					 "@endtsexample\n\n"

					 "@ingroup GuiGame")
{
	gSnapLine = !gSnapLine;
}
//
//ConsoleFunction( snapToggle, void, 1, 1, "()" )
//{
//   gSnapLine = !gSnapLine;
//}
