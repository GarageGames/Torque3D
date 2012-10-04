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

// ----------------------------------------------------------------------------  
// A gui control that displays health or energy information.  Based upon the   
// stock healthBar control but rewritten by M.Hall to display a numerical value.  
// ----------------------------------------------------------------------------  
  
#include "platform/platform.h"  
#include "gui/core/guiControl.h"  
#include "console/consoleTypes.h"  
#include "T3D/gameBase/gameConnection.h"  
#include "T3D/shapeBase.h"  
#include "gfx/gfxDrawUtil.h"  
  
class GuiHealthTextHud : public GuiControl  
{  
   typedef GuiControl Parent;  
  
   bool mShowFrame;  
   bool mShowFill;  
   bool mShowEnergy;  
   bool mShowTrueHealth;  
  
   ColorF mFillColor;  
   ColorF mFrameColor;  
   ColorF mTextColor;  
   ColorF mWarnColor;  
  
   F32 mWarnLevel;  
   F32 mPulseThreshold;  
   S32 mPulseRate;  
  
   F32 mValue;  
  
public:  
   GuiHealthTextHud();  
  
   void onRender(Point2I, const RectI &);  
   static void initPersistFields();  
   DECLARE_CONOBJECT(GuiHealthTextHud);  
   DECLARE_CATEGORY("Gui Game");  
   DECLARE_DESCRIPTION("Shows the damage or energy level of the current\n"  
      "PlayerObjectType control object as a numerical text display.");  
};  
  
// ----------------------------------------------------------------------------  
  
IMPLEMENT_CONOBJECT(GuiHealthTextHud);  
  
ConsoleDocClass(GuiHealthTextHud,  
   "@brief Shows the health or energy value of the current PlayerObjectType control object.\n\n"  
   "This gui can be configured to display either the health or energy value of the current Player Object. "  
   "It can use an alternate display color if the health or drops below a set value. "  
   "It can also be set to pulse if the health or energy drops below a set value. "  
   "This control only works if a server connection exists and it's control object "  
   "is a PlayerObjectType. If either of these requirements is false, the control is not rendered.\n\n"  
  
   "@tsexample\n"  
      "\n new GuiHealthTextHud()"  
      "{\n"  
      "   fillColor = \"0.0 0.0 0.0 0.5\"; // Fills with a transparent black color\n"  
      "   frameColor = \"1.0 1.0 1.0 1.0\"; // Solid white frame color\n"  
      "   textColor = \"0.0 1.0 0.0 1.0\" // Solid green text color\n"  
      "   warningColor = \"1.0 0.0 0.0 1.0\"; // Solid red color, used when damaged\n"  
      "   showFill = \"true\";\n"  
      "   showFrame = \"true\";\n"  
      "   showTrueValue = \"false\";\n"  
      "   showEnergy = \"false\";\n"  
      "   warnThreshold = \"50\";\n"  
      "   pulseThreshold = \"25\";\n"  
      "   pulseRate = \"500\";\n"  
      "   profile = \"GuiBigTextProfile\";\n"  
      "};\n"  
   "@endtsexample\n\n"  
  
   "@ingroup GuiGame\n"  
);  
  
GuiHealthTextHud::GuiHealthTextHud()  
{  
   mShowFrame = mShowFill = true;  
   mShowEnergy = false;  
   mShowTrueHealth = false;  
  
   mFillColor.set(0, 0, 0, 0.5);  
   mFrameColor.set(1, 1, 1, 1);  
   mTextColor.set(0, 1, 0, 1);  
   mWarnColor.set(1, 0, 0, 1);  
  
   mWarnLevel = 50.0f;  
   mPulseThreshold = 25.0f;  
   mPulseRate = 0;  
     
   mValue = 0.2f;  
}  
  
void GuiHealthTextHud::initPersistFields()  
{  
   addGroup("Colors");       
   addField("fillColor", TypeColorF, Offset(mFillColor, GuiHealthTextHud), "Color for the background of the control.");  
   addField("frameColor", TypeColorF, Offset(mFrameColor, GuiHealthTextHud), "Color for the control's frame.");  
   addField("textColor", TypeColorF, Offset(mTextColor, GuiHealthTextHud), "Color for the text on this control.");  
   addField("warningColor", TypeColorF, Offset(mWarnColor, GuiHealthTextHud), "Color for the text when health is low.");    
   endGroup("Colors");          
  
   addGroup("View");      
   addField("showFill", TypeBool, Offset(mShowFill, GuiHealthTextHud), "If true, draw the background.");  
   addField("showFrame", TypeBool, Offset(mShowFrame, GuiHealthTextHud), "If true, draw the frame.");  
   addField("showTrueValue", TypeBool, Offset(mShowTrueHealth, GuiHealthTextHud), "If true, we don't hardcode maxHealth to 100.");  
   addField("showEnergy", TypeBool, Offset(mShowEnergy, GuiHealthTextHud), "If true, display the energy value rather than the damage value.");  
   endGroup("View");    
  
   addGroup("Alert");  
   addField("warnThreshold", TypeF32, Offset(mWarnLevel, GuiHealthTextHud), "The health level at which to use the warningColor.");    
   addField("pulseThreshold", TypeF32, Offset(mPulseThreshold, GuiHealthTextHud), "Health level at which to begin pulsing.");  
   addField("pulseRate", TypeS32, Offset(mPulseRate, GuiHealthTextHud), "Speed at which the control will pulse.");  
   endGroup("Alert");  
  
   Parent::initPersistFields();  
}  
  
// ----------------------------------------------------------------------------  
  
void GuiHealthTextHud::onRender(Point2I offset, const RectI &updateRect)  
{  
   // Must have a connection and player control object  
   GameConnection* conn = GameConnection::getConnectionToServer();  
   if (!conn)  
      return;  
   ShapeBase* control = dynamic_cast<ShapeBase*>(conn->getControlObject());  
   if (!control || !(control->getTypeMask() & PlayerObjectType))  
      return;  
  
   // Just grab the damage/energy right off the control object.    
   // Damage value 0 = no damage (full health).    
   if(mShowEnergy)    
      mValue = control->getEnergyLevel();  
   else    
   {  
      if (mShowTrueHealth)  
         mValue = control->getMaxDamage() - control->getDamageLevel();  
      else  
         mValue = 100 - (100 * control->getDamageValue());    
   }  
  
   // If enabled draw background first  
   if (mShowFill)  
      GFX->getDrawUtil()->drawRectFill(updateRect, mFillColor);  
  
   // Prepare text and center it  
   S32 val = (S32)mValue;    
   char buf[256];    
   dSprintf(buf, sizeof(buf), "%d", val);    
   offset.x += (getBounds().extent.x - mProfile->mFont->getStrWidth((const UTF8 *)buf)) / 2;    
   offset.y += (getBounds().extent.y - mProfile->mFont->getHeight()) / 2;    
  
   ColorF tColor = mTextColor;   
  
   // If warning level is exceeded switch to warning color  
   if(mValue < mWarnLevel)   
   {  
      tColor = mWarnColor;    
  
      // If the pulseRate is set then pulse the text if health is below the threshold  
      if (mPulseRate != 0 && mValue < mPulseThreshold)   
      {  
         U32 time = Platform::getVirtualMilliseconds();  
         F32 alpha = 2.0f * F32(time % mPulseRate) / F32(mPulseRate);  
         tColor.alpha = (alpha > 1.0f)? 2.0f - alpha: alpha;  
      }  
   }  
  
   GFX->getDrawUtil()->setBitmapModulation(tColor);    
   GFX->getDrawUtil()->drawText(mProfile->mFont, offset, buf);    
   GFX->getDrawUtil()->clearBitmapModulation();    
  
   // If enabled draw the border last  
   if (mShowFrame)  
      GFX->getDrawUtil()->drawRect(updateRect, mFrameColor);  
}  