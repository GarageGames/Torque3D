function HudlessPlayGui::onWake(%this)
{
   // just update the action map here
   moveMap.push();
}

function HudlessPlayGui::onSleep(%this)
{
   // pop the keymaps
   moveMap.pop();
}

function HudlessPlayGui::toggle(%this)
{
   if (%this.isAwake())
      Canvas.setContent(PlayGui);
   else
      Canvas.setContent(HudlessPlayGui);
}
