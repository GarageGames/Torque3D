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

singleton TSShapeConstructor(FP_ProxMineDAE)
{
   baseShape = "./FP_ProxMine.DAE";
   loadLights = "0";
};

function FP_ProxMineDAE::onLoad(%this)
{
   %this.renameSequence("ambient", "timeline");
   %this.addSequence("timeline", "idle", "146", "205", "1", "0");
   %this.addSequence("timeline", "Run", "236", "295", "1", "0");
   %this.addSequence("timeline", "run2sprint", "326", "341", "1", "0");
   %this.addSequence("timeline", "sprint", "376", "395", "1", "0");
   %this.addSequence("timeline", "sprint2run", "426", "441", "1", "0");
   %this.addSequence("timeline", "fire", "476", "500", "1", "0");
   %this.addSequence("timeline", "switch_out", "546", "561", "1", "0");
   %this.addSequence("timeline", "switch_in", "10", "98", "1", "0");
   %this.setSequenceCyclic("switch_in", "0");
   %this.addSequence("timeline", "fire_release", "500", "521", "1", "0");
   %this.setSequenceCyclic("fire_release", "0");
   %this.addSequence("timeline", "soldier_idle", "146", "205", "1", "0");
   %this.addSequence("timeline", "soldier_run", "236", "295", "1", "0");
   %this.addSequence("timeline", "soldier_run2sprint", "326", "341", "1", "0");
   %this.setSequenceCyclic("soldier_run2sprint", "0");
   %this.addSequence("timeline", "soldier_sprint", "376", "395", "1", "0");
   %this.addSequence("timeline", "soldier_sprint2run", "426", "441", "1", "0");
   %this.setSequenceCyclic("soldier_sprint2run", "0");
   %this.addSequence("timeline", "soldier_switch_out", "546", "561", "1", "0");
   %this.setSequenceCyclic("soldier_switch_out", "0");
   %this.addSequence("timeline", "soldier_switch_in", "10", "98", "1", "0");
   %this.setSequenceCyclic("soldier_switch_in", "0");
   %this.addSequence("timeline", "soldier_fire", "476", "500", "1", "0");
   %this.setSequenceCyclic("soldier_fire", "0");
   %this.addSequence("timeline", "soldier_fire_release", "500", "521", "1", "0");
   %this.setSequenceCyclic("soldier_fire_release", "0");
}
