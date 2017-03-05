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

singleton TSShapeConstructor(CheetahDAE)
{
   baseShape = "./Cheetah_Body.DAE";
   loadLights = "0";
   lodType = "TrailingNumber";
   neverImport = "null  EnvironmentAmbientLight";
   forceUpdateMaterials = "0";
   loadLights = "0";
};

function CheetahDAE::onLoad(%this)
{
   %this.setSequenceCyclic("ambient", "0");
   %this.renameSequence("ambient", "timeline");
   %this.addSequence("timeline", "root", "0", "1");
   %this.addSequence("timeline", "spring0", "10", "11");
   %this.addSequence("timeline", "spring1", "20", "21");
   %this.addSequence("timeline", "spring2", "30", "31");
   %this.addSequence("timeline", "spring3", "40", "41");
   %this.addSequence("timeline", "brakeLight", "50", "51");
   %this.setSequencePriority("brakeLight", "8");
   %this.setNodeTransform("cam", "5.46934e-008 -4.75632 2.89171 -0.404897 0.817636 0.409303 1.71107", "1");
   %this.removeNode("CheetahMesh300");
   %this.removeNode("CheetahMesh200");
   %this.removeNode("CheetahMesh100");
   %this.removeNode("CheetahMesh2");
   %this.removeNode("TailLightsMesh300");
   %this.removeNode("TailLightsMesh200");
   %this.removeNode("TailLightsMesh100");
   %this.removeNode("TailLightsMesh2");
   %this.removeNode("nulldetail1");
}
