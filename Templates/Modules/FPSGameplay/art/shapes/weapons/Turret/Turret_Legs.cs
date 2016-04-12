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

singleton TSShapeConstructor(Turret_LegsDAE)
{
   baseShape = "./Turret_Legs.DAE";
   loadLights = "0";
   lodType = "DetectDTS";
};

function Turret_LegsDAE::onLoad(%this)
{
   %this.renameSequence("ambient", "timeline");
   %this.addSequence("timeline", "root", "110", "111", "1", "0");
   %this.addSequence("timeline", "scan", "140", "412", "1", "0");
   %this.addSequence("timeline", "throw", "10", "11", "1", "0");
   %this.addSequence("timeline", "deploy", "10", "110", "1", "0");
   %this.addSequence("timeline", "destroyed", "840", "925", "1", "0");
   %this.addSequence("timeline", "light_recoil", "440", "445", "1", "0");
   %this.addSequence("timeline", "medium_recoil", "480", "495", "1", "0");
   %this.setSequenceCyclic("throw", "0");
   %this.setSequenceCyclic("deploy", "0");
   %this.setSequenceCyclic("destroyed", "0");
   %this.setSequenceCyclic("light_recoil", "0");
   %this.setSequenceCyclic("medium_recoil", "0");
   %this.setSequenceBlend("light_recoil", "1", "root", "0");
   %this.setSequenceBlend("medium_recoil", "1", "root", "0");
   %this.setMeshSize("ColCapsule-1 2", "-1");
}
