function createSuperTooltipTheme(%name)
{
   %theme = new ScriptObject()
   {
      class = SuperTooltipTheme;      
   };
   
   %theme.setName(%name);
   
   return %theme;
}

function SuperTooltipTheme::addStyle(%this, %name, %style)
{
   %this.styles[%name] = %style;   
}

function SuperTooltipTheme::setDefaultStyle(%this, %type, %default)
{
   %this.defaultStyles[%type] = %default;   
}

function SuperTooltipTheme::setSpacing(%this, %verticalSpace, %horizontalSpace)
{
   %this.verticalSpace = %verticalSpace;
   %this.horizontalSpace = %horizontalSpace;   
}

function SuperTooltipTheme::getStyle(%this, %name)
{
   return %this.styles[%name];   
}

function SuperTooltip::init(%this, %theme)
{
   %this.clearTooltip();
      
   if(isObject(%theme))
      %this.setTheme(%theme);
}

function SuperTooltip::clearTooltip(%this)
{
   if(%this.paramCount > 0)
   {
      for(%i=0;%i<%this.paramCount;%i++)
         %this.param[%i] = "";      
   }
   
   %this.title = "";
   %this.paramCount = 0;   
}

function SuperTooltip::processTooltip(%this, %globalPos, %verticalAlign, %horizontalAlign)
{
   if (%verticalAlign $= "")
      %verticalAlign = 1;
   if (%horizontalAlign $= "")
      %horizontalAlign = 0;
   
   %tooltipWindow = %this.findObjectByInternalName("tooltipWindow");
   
   if(isObject(%tooltipWindow))
      %tooltipMLText = %tooltipWindow.findObjectByInternalName("tooltipMLText");
   else
      return false;
      
   if(!isObject(%tooltipMLText))
      return false;
   
   %verticalSpace = %this.theme.verticalSpace;
   %horizontalSpace = %this.theme.horizontalSpace;
   
   if (%verticalAlign == 1)
      %verticalSpace = -%verticalSpace;
   if (%horizontalAlign == 1)
      %horizontalSpace = -%horizontalSpace;
   
   %text = %this.getFormatedText();
   %tooltipMLText.setText(%text);
   
   canvas.pushDialog(%this);
   
   %tooltipMLText.forceReflow();
   %MLExtent = %tooltipMLText.extent;
   %MLHeight = getWord(%MLExtent, 1);
   
   %tooltipExtent = %tooltipWindow.extent;
   %tooltipWidth = getWord(%tooltipExtent, 0);
   %tooltipHeight = %MLHeight;
   %tooltipWindow.extent = %tooltipWidth SPC %tooltipHeight;  
   
   %globalPosX = getWord(%globalPos, 0);
   %globalPosY = getWord(%globalPos, 1);
   
   %tooltipPosX = %globalPosX - (%horizontalAlign * %tooltipWidth) + %horizontalSpace;
   %tooltipPosY = %globalPosY - (%verticalAlign * %tooltipHeight) + %verticalSpace;
   
   %tooltipWindow.setPosition(%tooltipPosX, %tooltipPosY); 
   
   return true;
}

function SuperTooltip::hide(%this)
{
   canvas.popDialog(%this);   

   %this.clearTooltip();
}

function SuperTooltip::setTheme(%this, %theme)
{
   %this.theme = %theme;   
}

function SuperTooltip::setTitle(%this, %title, %style)
{
   if(%style !$= "")
      %themeStyle = %this.theme.styles[%style];
   else
      %themeStyle = %this.theme.getStyle(%this.theme.defaultStyles[Title]);
   
   %this.title = %themeStyle @ %title;      
}

function SuperTooltip::addParam(%this, %title, %text, %paramTitleStyle, %paramStyle)
{
   if(%paramTitleStyle !$= "")
      %themeTitleStyle = %this.theme.styles[%paramTitleStyle];
   else
      %themeTitleStyle = %this.theme.getStyle(%this.theme.defaultStyles[ParamTitle]);
      
   if(%paramStyle !$= "")
      %themeStyle = %this.theme.styles[%paramStyle];
   else
      %themeStyle = %this.theme.getStyle(%this.theme.defaultStyles[Param]);
   
   if (%title $= "")
      %this.param[%this.paramCount] = %themeStyle @ %text @ "\n";
   else
      %this.param[%this.paramCount] = %themeTitleStyle @ %title @ ":  " @ %themeStyle @ %text @ "\n";
   %this.paramCount++;   
}

function SuperTooltip::getFormatedText(%this)
{
   %text = %this.title @ "\n\n";
   
   for(%i=0;%i<%this.paramCount;%i++)
   {
      %text = %text @ %this.param[%i];      
   }
   
   return %text;
}