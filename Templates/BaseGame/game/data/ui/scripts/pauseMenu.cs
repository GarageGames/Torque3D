function PauseMenu::onWake(%this)
{
   $timescale = 0;
}

function PauseMenu::onSleep(%this)
{
   $timescale = 1;
}

function PauseMenu::openOptionsMenu(%this)
{
   Canvas.pushDialog(OptionsMenu);
   OptionsMenu.returnGui = %this; 
   PauseOptionsMain.hidden = true; 
}

function PauseMenu::openControlsMenu(%this)
{
   Canvas.pushDialog(OptionsMenu);
   OptionsMenu.returnGui = %this; 
   PauseOptionsMain.hidden = true; 
   OptionsMain.hidden = true;
   ControlsMenu.hidden = false;
}

function PauseMenu::onReturnTo(%this)
{
   PauseOptionsMain.hidden = false;
}