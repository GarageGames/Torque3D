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

$MetricsParamArray[0] = "fps ";
$MetricsParamArray[1] = "shadow "; 
$MetricsParamArray[2] = "gfx "; 
$MetricsParamArray[3] = "sfx ";
$MetricsParamArray[4] = "terrain ";
$MetricsParamArray[5] = "groundcover ";
$MetricsParamArray[6] = "forest ";
$MetricsParamArray[7] = "net ";
$EnableProfiler = false;
$string = ""; //string used to collet the parameters for metrics function

function showMetrics(%var)
{
	$string = "";
	if(ppShowFps.getValue())
	{
		$string = $string @ $MetricsParamArray[0];
	}
	if(ppShowShadow.getValue())
	{
		$string = $string @ $MetricsParamArray[1];
	}
	if(ppShowGfx.getValue())
	{
		$string = $string @ $MetricsParamArray[2];
	}
	if(ppShowSfx.getValue())
	{
		$string = $string @ $MetricsParamArray[3];
	}
	if(ppShowTerrain.getValue())
	{
		$string = $string @ $MetricsParamArray[4];
	}
	if(ppShowForest.getValue())
	{
		$string = $string @ $MetricsParamArray[5];
	}
	if(ppShowGroundcover.getValue())
	{
		$string = $string @ $MetricsParamArray[6];
	}
	if(ppShowNet.getValue())
	{
		$string = $string @ $MetricsParamArray[7];
	}

	if(%var)
	{
		$EnableProfiler = !($EnableProfiler);

	   if($EnableProfiler)
	   {
			metrics($string);	
	   }
	   else if((false == $EnableProfiler))
	   {
		   metrics();
	   }
	}
	else if($EnableProfiler)  //will enter only when the enable/disable button was pressed
	{
		metrics($string);
	}
}

function showMetics(%var)
{
   if(%var)
   {
      metrics($string);
   }
   else if(true == $EnableProfiler)
   {
      $EnableProfiler = false;
      metrics();
   }
}

GlobalActionMap.bind(keyboard, "ctrl F2", showMetics);

%guiContent = new GuiControl(FrameOverlayGui) {
	profile = "GuiModelessDialogProfile";
	horizSizing = "right";
	vertSizing = "bottom";
	position = "0 0";
	extent = "640 480";
	minExtent = "8 8";
	visible = "True";
	setFirstResponder = "True";
	modal = "false";
	helpTag = "0";
   noCursor = true;
	new GuiConsoleTextCtrl(TextOverlayControl) {
		profile = "GuiConsoleTextProfile";
		horizSizing = "right";
		vertSizing = "bottom";
		position = "5 5";
		extent = "130 18";
		minExtent = "4 4";
		visible = "True";
		setFirstResponder = "True";
		modal = "True";
		helpTag = "0";
		expression = "10";
		command = "Canvas.popDialog(FrameOverlayGui);";
		accelerator = "escape";
	};
};

// Note:  To implement your own metrics overlay 
// just add a function with a name in the form 
// XXXXMetricsCallback which can be enabled via
// metrics( XXXX )

function fpsMetricsCallback()
{
   return "  | FPS |" @ 
          "  " @ $fps::real @ 
          "  max: " @ $fps::realMax @
          "  min: " @ $fps::realMin @
          "  mspf: " @ 1000 / $fps::real;
}

function gfxMetricsCallback()
{
   return "  | GFX |" @
          "  PolyCount: " @ $GFXDeviceStatistics::polyCount @
          "  DrawCalls: " @ $GFXDeviceStatistics::drawCalls @
          "  RTChanges: " @ $GFXDeviceStatistics::renderTargetChanges;
          
}

function terrainMetricsCallback()
{
   return "  | Terrain |" @
          "  Cells: " @ $TerrainBlock::cellsRendered @
          "  Override Cells: " @ $TerrainBlock::overrideCells @
          "  DrawCalls: " @ $TerrainBlock::drawCalls;
}

function netMetricsCallback()
{
   return "  | Net |" @
          "  BitsSent: " @ $Stats::netBitsSent @
          "  BitsRcvd: " @ $Stats::netBitsReceived @
          "  GhostUpd: " @ $Stats::netGhostUpdates;
}

function groundCoverMetricsCallback()
{
   return "  | GroundCover |" @
          "  Cells: " @ $GroundCover::renderedCells @
          "  Billboards: " @ $GroundCover::renderedBillboards @
          "  Batches: " @ $GroundCover::renderedBatches @
          "  Shapes: " @ $GroundCover::renderedShapes;
}

function forestMetricsCallback()
{
   return "  | Forest |" @
          "  Cells: " @ $Forest::totalCells @
          "  Cells Meshed: " @ $Forest::cellsRendered @
          "  Cells Billboarded: " @ $Forest::cellsBatched @
          "  Meshes: " @ $Forest::cellItemsRendered @                          
          "  Billboards: " @ $Forest::cellItemsBatched;
}

function sfxMetricsCallback() 
{
   return "  | SFX |" @
          "  Sounds: " @ $SFX::numSounds @
          "  Lists: " @ ( $SFX::numSources - $SFX::numSounds - $SFX::Device::fmodNumEventSource ) @
          "  Events: " @ $SFX::fmodNumEventSources @
          "  Playing: " @ $SFX::numPlaying @
          "  Culled: " @ $SFX::numCulled @
          "  Voices: " @ $SFX::numVoices @
          "  Buffers: " @ $SFX::Device::numBuffers @
          "  Memory: " @ ( $SFX::Device::numBufferBytes / 1024.0 / 1024.0 ) @ " MB" @
          "  Time/S: " @ $SFX::sourceUpdateTime @
          "  Time/P: " @ $SFX::parameterUpdateTime @
          "  Time/A: " @ $SFX::ambientUpdateTime;
}

function sfxSourcesMetricsCallback() 
{
   return sfxDumpSourcesToString();
}

function sfxStatesMetricsCallback()
{
   return "  | SFXStates |" @ sfxGetActiveStates();
}

function timeMetricsCallback()
{
   return "  | Time |" @ 
          "  Sim Time: " @ getSimTime() @ 
          "  Mod: " @ getSimTime() % 32;
}

function reflectMetricsCallback()
{
   return "  | REFLECT |" @
          "  Objects: " @ $Reflect::numObjects @ 
          "  Visible: " @ $Reflect::numVisible @
          "  Occluded: " @ $Reflect::numOccluded @
          "  Updated: " @ $Reflect::numUpdated @
          "  Elapsed: " @ $Reflect::elapsed NL
             
          "  Allocated: " @ $Reflect::renderTargetsAllocated @
          "  Pooled: " @ $Reflect::poolSize NL
          
          "  " @ getWord( $Reflect::textureStats, 1 ) TAB
          "  " @ getWord( $Reflect::textureStats, 2 ) @ "MB" TAB                  
          "  " @ getWord( $Reflect::textureStats, 0 );
}

function decalMetricsCallback()
{
   return "  | DECAL |" @
          " Batches: " @ $Decal::Batches @
          " Buffers: " @ $Decal::Buffers @
          " DecalsRendered: " @ $Decal::DecalsRendered;
}

function renderMetricsCallback()
{
   return "  | Render |" @
          "  Mesh: " @ $RenderMetrics::RIT_Mesh @
          "  MeshDL: " @ $RenderMetrics::RIT_MeshDynamicLighting @
          "  Shadow: " @ $RenderMetrics::RIT_Shadow @
          "  Sky: " @ $RenderMetrics::RIT_Sky @
          "  Obj: " @ $RenderMetrics::RIT_Object @
          "  ObjT: " @ $RenderMetrics::RIT_ObjectTranslucent @
          "  Decal: " @ $RenderMetrics::RIT_Decal @
          "  Water: " @ $RenderMetrics::RIT_Water @
          "  Foliage: " @ $RenderMetrics::RIT_Foliage @
          "  Trans: " @ $RenderMetris::RIT_Translucent @
          "  Custom: " @ $RenderMetrics::RIT_Custom;
}

function shadowMetricsCallback()
{   
   return "  | Shadow |" @
          "  Active: " @ $ShadowStats::activeMaps @
          "  Updated: " @ $ShadowStats::updatedMaps @
          "  PolyCount: " @ $ShadowStats::polyCount @
          "  DrawCalls: " @ $ShadowStats::drawCalls @          
          "   RTChanges: " @ $ShadowStats::rtChanges @          
          "   PoolTexCount: " @ $ShadowStats::poolTexCount @
          "   PoolTexMB: " @ $ShadowStats::poolTexMemory @ "MB";         
}

function basicShadowMetricsCallback()
{   
   return "  | Shadow |" @
          "  Active: " @ $BasicLightManagerStats::activePlugins @
          "  Updated: " @ $BasicLightManagerStats::shadowsUpdated @
          "  Elapsed Ms: " @ $BasicLightManagerStats::elapsedUpdateMs;         
}

function lightMetricsCallback()
{
   return "  | Deferred Lights |" @
          "  Active: " @ $lightMetrics::activeLights @
          "  Culled: " @ $lightMetrics::culledLights;
}

function particleMetricsCallback()
{
   return "  | Particles |" @ 
          "  # Simulated " @ $particle::numSimulated;
}
function partMetricsCallback()
{
   return particleMetricsCallback();
}

function imposterMetricsCallback()
{
   return "  | IMPOSTER |" @ 
          "   Rendered: " @ $ImposterStats::rendered @
          "   Batches: " @ $ImposterStats::batches @
          "   DrawCalls: " @ $ImposterStats::drawCalls @
          "   Polys: " @ $ImposterStats::polyCount @
          "   RtChanges: " @ $ImposterStats::rtChanges;
}

// alias
function audioMetricsCallback()
{
   return sfxMetricsCallback(); 
}

// alias
function videoMetricsCallback()
{
   return gfxMetricsCallback();
}

// Add a metrics HUD.  %expr can be a vector of names where each element
// must have a corresponding '<name>MetricsCallback()' function defined
// that will be called on each update of the GUI control.  The results
// of each function are stringed together.
//
// Example: metrics( "fps gfx" );

function metrics( %expr )
{
   %metricsExpr = "";
   if( %expr !$= "" )
   {
      for( %i = 0;; %i ++ )
      {
         %name = getWord( %expr, %i );
         if( %name $= "" )
            break;
         else
         {
            %cb = %name @ "MetricsCallback";
            if( !isFunction( %cb ) )
               error( "metrics - undefined callback: " @ %cb );
            else
            {
               %cb = %cb @ "()";
               if( %i > 0 )
                  %metricsExpr = %metricsExpr @ " NL ";
               %metricsExpr = %metricsExpr @ %cb;
            }
         }
      }
      
      if( %metricsExpr !$= "" )
         %metricsExpr = %metricsExpr @ " @ \" \"";
   }
   
   if( %metricsExpr !$= "" )
   {
      $GameCanvas.pushDialog( FrameOverlayGui, 1000 );
      TextOverlayControl.setValue( %metricsExpr );
   }
   else
      $GameCanvas.popDialog(FrameOverlayGui);
}