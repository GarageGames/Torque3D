function toolsDebugViz::create( %this )
{
   exec("./shadowViz.gui");
   
   exec("./lightViz.cs");
   exec("./shadowViz.cs");
}

function toolsDebugViz::destroy( %this )
{
   echo("Shutting down the visualizer!");
}