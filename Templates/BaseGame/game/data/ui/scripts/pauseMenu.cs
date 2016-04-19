function PauseMenu::onWake(%this)
{
}

function PauseMenu::onSleep(%this)
{
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