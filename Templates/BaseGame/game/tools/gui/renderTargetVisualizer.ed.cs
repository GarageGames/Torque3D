function ToggleRenderTargetVisualizer()
{
   if(RenderTargetVisualizer.isAwake())
   {
      Canvas.popDialog(RenderTargetVisualizer); 
   }
   else
   {
      Canvas.pushDialog(RenderTargetVisualizer);
   }
}

function RenderTargetVisualizer::onWake(%this)
{
   %targetsList = getNamedTargetList();
   
   %targetsCount = getWordCount(%targetsList);
   for(%i=0; %i < %targetsCount; %i++)
   {
      %targetName = getWord(%targetsList, %i);  
      RenderTargetsList.add(%targetName, %i);
   }
   
   RenderTargetsList.setSelected( 0, false );
   RenderTargetVizCtrl.RenderTargetName = RenderTargetsList.getValue();
}

function RenderTargetsList::updateTarget(%this)
{
   %target = RenderTargetsList.getValue();

   RenderTargetVizCtrl.RenderTargetName = %target;
}