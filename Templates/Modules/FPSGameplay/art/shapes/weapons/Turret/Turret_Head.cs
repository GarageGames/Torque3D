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

singleton TSShapeConstructor(Turret_HeadDAE)
{
   baseShape = "./Turret_Head.DAE";
   loadLights = "0";
};

function Turret_HeadDAE::onLoad(%this)
{
   %this.renameSequence("ambient", "timeline");
   %this.addSequence("timeline", "scan", "140", "412", "1", "0");
   %this.addSequence("timeline", "wait_deploy", "10", "11", "1", "0");
   %this.addSequence("timeline", "deploy", "10", "110", "1", "0");
   %this.addSequence("timeline", "destroyed", "840", "925", "1", "0");
   %this.addSequence("timeline", "fire", "440", "445", "1", "0");
   %this.addSequence("timeline", "fire_alt", "480", "495", "1", "0");
   %this.setSequenceCyclic("wait_deploy", "0");
   %this.setSequenceCyclic("deploy", "0");
   %this.setSequenceCyclic("destroyed", "0");
   %this.setSequenceCyclic("fire", "0");
   %this.setSequenceCyclic("fire_alt", "0");
   %this.addNode("ejectPoint", "bn_head", "0 0 0 0 0 1 1.57", "0");
}
