if(isClass(GuiPlatformGenericMenuBar))
{
   exec("./guiPlatformGenericMenubar.ed.gui");
}
else
{
   %guiContent = new GuiControl(PlatformGenericMenubar) {
      profile = "GuiModelessDialogProfile";
      
      new GuiControl()
      {
         internalName = "menubar";
         extent = "1024 20";
         minExtent = "320 20";
         horizSizing = "width";
         profile = "GuiMenuBarProfile";
      };
   };
}