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

#include "gui/core/guiControl.h"
#include "gui/controls/guiBitmapCtrl.h"
#include "console/consoleTypes.h"
#include "scene/sceneManager.h"
#include "T3D/gameBase/gameConnection.h"
#include "T3D/shapeBase.h"
#include "gfx/gfxDrawUtil.h"
#include "console/engineAPI.h"


//-----------------------------------------------------------------------------
/// Vary basic cross hair hud.
/// Uses the base bitmap control to render a bitmap, and decides whether
/// to draw or not depending on the current control object and it's state.
/// If there is ShapeBase object under the cross hair and it's named,
/// then a small health bar is displayed.
class GuiCrossHairHud : public GuiBitmapCtrl
{
   typedef GuiBitmapCtrl Parent;

   ColorF   mDamageFillColor;
   ColorF   mDamageFrameColor;
   Point2I  mDamageRectSize;
   Point2I  mDamageOffset;

protected:
   void drawDamage(Point2I offset, F32 damage, F32 opacity);

public:
   GuiCrossHairHud();

   void onRender( Point2I, const RectI &);
   static void initPersistFields();
   DECLARE_CONOBJECT( GuiCrossHairHud );
   DECLARE_CATEGORY( "Gui Game" );
   DECLARE_DESCRIPTION( "Basic cross hair hud. Reacts to state of control object.\n"
      "Also displays health bar for named objects under the cross hair." );
};

/// Valid object types for which the cross hair will render, this
/// should really all be script controlled.
static const U32 ObjectMask = PlayerObjectType | VehicleObjectType;


//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT( GuiCrossHairHud );

ConsoleDocClass( GuiCrossHairHud,
   "@brief Basic cross hair hud. Reacts to state of control object. Also displays health bar for named objects under the cross hair.\n\n"
   "Uses the base bitmap control to render a bitmap, and decides whether to draw or not depending "
   "on the current control object and it's state. If there is ShapeBase object under the cross hair "
   "and it's named, then a small health bar is displayed.\n\n"
     
   "@tsexample\n"
		"\n new GuiCrossHairHud()"
		"{\n"
		"	damageFillColor = \"1.0 0.0 0.0 1.0\"; // Fills with a solid red color\n"
		"	damageFrameColor = \"1.0 1.0 1.0 1.0\"; // Solid white frame color\n"
		"	damageRect = \"15 5\";\n"
		"	damageOffset = \"0 -10\";\n"
		"};\n"
   "@endtsexample\n"
   
   "@ingroup GuiGame\n"
);

GuiCrossHairHud::GuiCrossHairHud()
{
   mDamageFillColor.set( 0.0f, 1.0f, 0.0f, 1.0f );
   mDamageFrameColor.set( 1.0f, 0.6f, 0.0f, 1.0f );
   mDamageRectSize.set(50, 4);
   mDamageOffset.set(0,32);
}

void GuiCrossHairHud::initPersistFields()
{
   addGroup("Damage");		
   addField( "damageFillColor", TypeColorF, Offset( mDamageFillColor, GuiCrossHairHud ), "As the health bar depletes, this color will represent the health loss amount." );
   addField( "damageFrameColor", TypeColorF, Offset( mDamageFrameColor, GuiCrossHairHud ), "Color for the health bar's frame." );
   addField( "damageRect", TypePoint2I, Offset( mDamageRectSize, GuiCrossHairHud ), "Size for the health bar portion of the control." );
   addField( "damageOffset", TypePoint2I, Offset( mDamageOffset, GuiCrossHairHud ), "Offset for drawing the damage portion of the health control." );
   endGroup("Damage");
   Parent::initPersistFields();
}


//-----------------------------------------------------------------------------

void GuiCrossHairHud::onRender(Point2I offset, const RectI &updateRect)
{
   // Must have a connection and player control object
   GameConnection* conn = GameConnection::getConnectionToServer();
   if (!conn)
      return;
   GameBase* control = dynamic_cast<GameBase*>(conn->getCameraObject());
   if (!control || !(control->getTypeMask() & ObjectMask) || !conn->isFirstPerson())
      return;

   // Parent render.
   Parent::onRender(offset,updateRect);

   // Get control camera info
   MatrixF cam;
   Point3F camPos;
   conn->getControlCameraTransform(0,&cam);
   cam.getColumn(3, &camPos);

   // Extend the camera vector to create an endpoint for our ray
   Point3F endPos;
   cam.getColumn(1, &endPos);
   endPos *= gClientSceneGraph->getVisibleDistance();
   endPos += camPos;

   // Collision info. We're going to be running LOS tests and we
   // don't want to collide with the control object.
   static U32 losMask = TerrainObjectType | ShapeBaseObjectType;
   control->disableCollision();

   RayInfo info;
   if (gClientContainer.castRay(camPos, endPos, losMask, &info)) {
      // Hit something... but we'll only display health for named
      // ShapeBase objects.  Could mask against the object type here
      // and do a static cast if it's a ShapeBaseObjectType, but this
      // isn't a performance situation, so I'll just use dynamic_cast.
      if (ShapeBase* obj = dynamic_cast<ShapeBase*>(info.object))
         if (obj->getShapeName()) {
            offset.x = updateRect.point.x + updateRect.extent.x / 2;
            offset.y = updateRect.point.y + updateRect.extent.y / 2;
            drawDamage(offset + mDamageOffset, obj->getDamageValue(), 1);
         }
   }

   // Restore control object collision
   control->enableCollision();
}


//-----------------------------------------------------------------------------
/**
   Display a damage bar ubove the shape.
   This is a support funtion, called by onRender.
*/
void GuiCrossHairHud::drawDamage(Point2I offset, F32 damage, F32 opacity)
{
   mDamageFillColor.alpha = mDamageFrameColor.alpha = opacity;

   // Damage should be 0->1 (0 being no damage,or healthy), but
   // we'll just make sure here as we flip it.
   damage = mClampF(1 - damage, 0, 1);

   // Center the bar
   RectI rect(offset, mDamageRectSize);
   rect.point.x -= mDamageRectSize.x / 2;

   // Draw the border
   GFX->getDrawUtil()->drawRect(rect, mDamageFrameColor);

   // Draw the damage % fill
   rect.point += Point2I(1, 1);
   rect.extent -= Point2I(1, 1);
   rect.extent.x = (S32)(rect.extent.x * damage);
   if (rect.extent.x == 1)
      rect.extent.x = 2;
   if (rect.extent.x > 0)
      GFX->getDrawUtil()->drawRectFill(rect, mDamageFillColor);
}
