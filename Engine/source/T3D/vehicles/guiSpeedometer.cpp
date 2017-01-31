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
#include "gui/controls/guiBitmapCtrl.h"
#include "console/consoleTypes.h"
#include "T3D/gameBase/gameConnection.h"
#include "T3D/vehicles/vehicle.h"
#include "T3D/player.h"
#include "gfx/primBuilder.h"

//-----------------------------------------------------------------------------
/// A Speedometer control.
/// This gui displays the speed of the current Vehicle based
/// control object. This control only works if a server
/// connection exists and its control object is a vehicle. If
/// either of these requirements is false, the control is not rendered.
class GuiSpeedometerHud : public GuiBitmapCtrl
{
   typedef GuiBitmapCtrl Parent;

   F32   mSpeed;        ///< Current speed
   F32   mMaxSpeed;     ///< Max speed at max need pos
   F32   mMaxAngle;     ///< Max pos of needle
   F32   mMinAngle;     ///< Min pos of needle
   Point2F mCenter;     ///< Center of needle rotation
   ColorF mColor;       ///< Needle Color
   F32   mNeedleLength;
   F32   mNeedleWidth;
   F32   mTailLength;

   GFXStateBlockRef mBlendSB;

public:
   GuiSpeedometerHud();

   void onRender( Point2I, const RectI &);
   static void initPersistFields();
   DECLARE_CONOBJECT( GuiSpeedometerHud );
   DECLARE_CATEGORY( "Gui Game" );
   DECLARE_DESCRIPTION( "Displays the speed of the current Vehicle-based control object." );
};


//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT( GuiSpeedometerHud );

ConsoleDocClass( GuiSpeedometerHud,
   "@brief Displays the speed of the current Vehicle based control object.\n\n"

   "This control only works if a server connection exists, and its control "
   "object is a Vehicle derived class. If either of these requirements is false, "
   "the control is not rendered.<br>"

   "The control renders the speedometer needle as a colored quad, rotated to "
   "indicate the Vehicle speed as determined by the <i>minAngle</i>, "
   "<i>maxAngle</i>, and <i>maxSpeed</i> properties. This control is normally "
   "placed on top of a GuiBitmapCtrl representing the speedometer dial.\n\n"

   "@tsexample\n"
   "new GuiSpeedometerHud()\n"
   "{\n"
   "   maxSpeed = \"100\";\n"
   "   minAngle = \"215\";\n"
   "   maxAngle = \"0\";\n"
   "   color = \"1 0.3 0.3 1\";\n"
   "   center = \"130 123\";\n"
   "   length = \"100\";\n"
   "   width = \"2\";\n"
   "   tail = \"0\";\n"
   "   //Properties not specific to this control have been omitted from this example.\n"
   "};\n"
   "@endtsexample\n\n"

   "@ingroup GuiContainers"
);

GuiSpeedometerHud::GuiSpeedometerHud()
{
   mSpeed = 0;
   mMaxSpeed = 100;
   mMaxAngle = 0;
   mMinAngle = 200;
   mCenter.set(0,0);
   mNeedleWidth = 3;
   mNeedleLength = 10;
   mTailLength = 5;
   mColor.set(1,0,0,1);
}

void GuiSpeedometerHud::initPersistFields()
{
   addGroup("Needle");

   addField("maxSpeed", TypeF32, Offset( mMaxSpeed, GuiSpeedometerHud ),
      "Maximum Vehicle speed (in Torque units per second) to represent on the "
      "speedo (Vehicle speeds greater than this are clamped to maxSpeed)." );

   addField("minAngle", TypeF32, Offset( mMinAngle, GuiSpeedometerHud ),
      "Angle (in radians) of the needle when the Vehicle speed is 0. An angle "
      "of 0 points right, 90 points up etc)." );

   addField("maxAngle", TypeF32, Offset( mMaxAngle, GuiSpeedometerHud ),
      "Angle (in radians) of the needle when the Vehicle speed is >= maxSpeed. "
      "An angle of 0 points right, 90 points up etc)." );

   addField("color", TypeColorF, Offset( mColor, GuiSpeedometerHud ),
      "Color of the needle" );

   addField("center", TypePoint2F, Offset( mCenter, GuiSpeedometerHud ),
      "Center of the needle, offset from the GuiSpeedometerHud control top "
      "left corner" );

   addField("length", TypeF32, Offset( mNeedleLength, GuiSpeedometerHud ),
      "Length of the needle from center to end" );

   addField("width", TypeF32, Offset( mNeedleWidth, GuiSpeedometerHud ),
      "Width of the needle" );

   addField("tail", TypeF32, Offset( mTailLength, GuiSpeedometerHud ),
      "Length of the needle from center to tail" );

   endGroup("Needle");

   Parent::initPersistFields();
}


//-----------------------------------------------------------------------------
/**
   Gui onRender method.
   Renders an analog speedometer needle over a specified bitmap background.
*/
void GuiSpeedometerHud::onRender(Point2I offset, const RectI &updateRect)
{
   // Must have a connection
   GameConnection* conn = GameConnection::getConnectionToServer();
   if (!conn)
      return;

   // Requires either a vehicle control object or a vehicle-mounted player		
   Vehicle* vehicle = dynamic_cast<Vehicle*>(conn->getControlObject());
   if(!vehicle){
      Player * player = dynamic_cast<Player*>(conn->getControlObject());
      if(!player) return;
      if (!player->isMounted()) return;
      vehicle = dynamic_cast<Vehicle*>(player->getObjectMount());
      if(!vehicle) return;
   }

   Parent::onRender(offset,updateRect);

   // Use the vehicle's velocity as its speed...
   mSpeed = vehicle->getVelocity().len();
   if (mSpeed > mMaxSpeed)
      mSpeed = mMaxSpeed;

    // Calculate center point if necessary and roll in offsets
   Point2F center = mCenter;
   if (mIsZero(center.x) && mIsZero(center.y))
   {
      center.x = getExtent().x / 2.0f;
      center.y = getExtent().y / 2.0f;
   }
   F32 fillOffset = GFX->getFillConventionOffset(); // Find the fill offset
   Point2F viewCenter(offset.x + fillOffset + center.x, offset.y + fillOffset + center.y);

   // Handle rotation calculations	
   F32 rotation, spinAngle;
   rotation = mMinAngle + (mMaxAngle - mMinAngle) * (mSpeed / mMaxSpeed);
   spinAngle = mDegToRad(rotation);
   MatrixF rotMatrix(EulerF(0.0, 0.0, spinAngle));

	// Set up the needle vertex list
	Point3F vertList[5];
	vertList[0].set(+mNeedleLength,-mNeedleWidth,0);
	vertList[1].set(+mNeedleLength,+mNeedleWidth,0);
	vertList[2].set(-mTailLength  ,+mNeedleWidth,0);
	vertList[3].set(-mTailLength  ,-mNeedleWidth,0);   
	vertList[4].set(+mNeedleLength,-mNeedleWidth,0); //// Get back to the start!
	
   // Create a GFXStateBlock description if one has not been set.
   if (mBlendSB.isNull())
   {
      GFXStateBlockDesc desc;
      desc.setBlend(true, GFXBlendSrcAlpha, GFXBlendInvSrcAlpha);
      desc.samplersDefined = true;
      desc.samplers[0].textureColorOp = GFXTOPDisable;
      mBlendSB = GFX->createStateBlock(desc);
   }
   GFX->setStateBlock(mBlendSB);
   GFX->setTexture(0, NULL);

   // Render the needle
   PrimBuild::color4f(mColor.red, mColor.green, mColor.blue, mColor.alpha);
   PrimBuild::begin(GFXLineStrip, 5);
   for(int k=0; k<5; k++){
      rotMatrix.mulP(vertList[k]);
      PrimBuild::vertex2f(vertList[k].x + viewCenter.x, vertList[k].y + viewCenter.y);
   }
   PrimBuild::end();
}
