function MainMenuGui::onWake(%this)
{
   if (isFunction("getWebDeployment") &&
       getWebDeployment() &&
       isObject(%this-->ExitButton))
      %this-->ExitButton.setVisible(false);
      
   MainMenuButtonContainer.hidden = false;
}

function MainMenuGui::openSinglePlayerMenu(%this)
{
   $pref::HostMultiPlayer=false;
   Canvas.pushDialog(ChooseLevelDlg);
   ChooseLevelDlg.returnGui = %this; 
   MainMenuButtonContainer.hidden = true; 
   MainMenuAppLogo.setBitmap("data/ui/art/Torque-3D-logo");
}

function MainMenuGui::openMultiPlayerMenu(%this)
{
   $pref::HostMultiPlayer=true;
   Canvas.pushDialog(ChooseLevelDlg);
   ChooseLevelDlg.returnGui = %this; 
   MainMenuButtonContainer.hidden = true; 
   MainMenuAppLogo.setBitmap("data/ui/art/Torque-3D-logo");
}

function MainMenuGui::openOptionsMenu(%this)
{
   Canvas.pushDialog(OptionsMenu);
   OptionsMenu.returnGui = %this; 
   MainMenuButtonContainer.hidden = true; 
   MainMenuAppLogo.setBitmap("data/ui/art/Torque-3D-logo");
}

function MainMenuGui::onReturnTo(%this)
{
   MainMenuButtonContainer.hidden = false;
   MainMenuAppLogo.setBitmap("data/ui/art/Torque-3D-logo-shortcut");
}