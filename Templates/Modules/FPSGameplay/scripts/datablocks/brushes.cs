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

// This is the default save location for any ForestBrush(s) created in 
// the Forest Editor.
// This script is executed from ForestEditorPlugin::onWorldEditorStartup().

//--- OBJECT WRITE BEGIN ---
new SimGroup(ForestBrushGroup) {
   canSaveDynamicFields = "1";

   new ForestBrush() {
      internalName = "ExampleForestBrush";
      canSaveDynamicFields = "1";

      new ForestBrushElement() {
         internalName = "ExampleElement";
         canSaveDynamicFields = "1";
         ForestItemData = "ExampleForestMesh";
         probability = "1";
         rotationRange = "360";
         scaleMin = "1";
         scaleMax = "2";
         scaleExponent = "0.2";
         sinkMin = "0";
         sinkMax = "0.1";
         sinkRadius = "0.25";
         slopeMin = "0";
         slopeMax = "30";
         elevationMin = "-10000";
         elevationMax = "10000";
         clumpCountMin = "1";
         clumpCountMax = "1";
         clumpCountExponent = "1";
         clumpRadius = "10";
      };
   };
};
//--- OBJECT WRITE END ---
