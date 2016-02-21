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

function VolumetricFog::onEnterFog(%this,%obj)
{
   // This method is called whenever the control object (Camera or Player)
   // %obj enters the fog area.
   
   // echo("Control Object " @ %obj @ " enters fog " @ %this);
}

function VolumetricFog::onLeaveFog(%this,%obj)
{
   // This method is called whenever the control object (Camera or Player)
   // %obj leaves the fog area.
   
   // echo("Control Object " @ %obj @ " left fog " @ %this);
}

function VolumetricFog::Dissolve(%this,%speed,%delete)
{
   // This method dissolves the fog at speed milliseconds
   %this.isBuilding = true;
   if (%this.FogDensity > 0)
   {
      %this.setFogDensity(%this.FogDensity - 0.005);
      %this.schedule(%speed,Dissolve,%speed,%delete);
   }
   else
   {
      %this.isBuilding = false;
      %this.SetFogDensity(0.0);
      if (%delete !$= "" && %delete !$="0" && %delete !$="false")
         %this.schedule(250,delete);
   }
}

function VolumetricFog::Thicken(%this,%speed, %end_density)
{
   // This method thickens the fog at speed milliseconds to a density of %end_density

   %this.isBuilding = true;
   if (%this.FogDensity + 0.005 < %end_density)
   {
      %this.setFogDensity(%this.FogDensity + 0.005);
      %this.schedule(%speed,Thicken,%speed, %end_density);
   }
   else
   {
      %this.setFogDensity(%end_density);
      %this.isBuilding = false;
   }
}
 
function GenerateFog(%pos,%scale,%color,%density)
{
   // This function can be used to generate some fog caused by massive gunfire etc.
   // Change shape and modulation data to your likings.
   
   %fog=new VolumetricFog() {
      shapeName = "art/environment/Fog_Sphere.dts";
      fogColor = %color;
      fogDensity = "0.0";
      ignoreWater = "0";
      MinSize = "250";
      FadeSize = "750";
      texture = "art/environment/FogMod_heavy.dds";
      tiles = "1";
      modStrength = "0.2";
      PrimSpeed = "-0.01 0.04";
      SecSpeed = "0.02 0.02";
      position = %pos;
      rotation = "0 0 1 20.354";
      scale = %scale;
      canSave = "1";
      canSaveDynamicFields = "1";
   };
   
   if (isObject(%fog))
   {
      MissionCleanup.add(%fog);
      
      %fog.Thicken(500,%density);
   }
   
   return %fog;
}